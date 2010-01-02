/*
 *  This program is free software; you can redistribute it and/or modify
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Please refer to the LICENSE.txt file that comes along with this source file
 *  or to http://www.gnu.org/licenses/gpl.txt for a full version of the license.
 *
 *  Program written by Paul Honig 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <sqlite3.h>

#include "config.h"
#include "curlfile.h"
#include "torrentdb.h"
#include "logfile.h"
#include "mailmsg.h"
#include "findtorrent.h"

/*
 * Max message and subject lenght for notification email
 */
#define MAXMSGLEN 1024

/*
 * Apply the filters from the query.
 */
static void applyfilter(sqlite3 *db, char *name, char *filter, char *nodouble);

/*
 * Test for double downloads.
 * Queries need to be provided by the user.
 * return 1 if double, 0 if new
 */
static int testdouble(sqlite3 *db, char *nodouble, char *link, int season, int episode);

/*
 * Do download.
 * take url, create name and call curl routine
 */
static void dodownload(sqlite3 *db, char *link, char *title, int season, int episode, char *pubdate);

/*
 * Get the filters from the database.
 * apply the filters.
 * then download the results.
 */
int downloadtorrents(sqlite3 *db)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;
  char          *name;
  char          *filter;
  char          *nodouble;

  const char *query = "select name, filter, nodouble from filters";

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
    /*
     * Get name and query of the filters
     */
    name = (char*) sqlite3_column_text(ppStmt, 0);
    filter = (char*) sqlite3_column_text(ppStmt, 1);
    nodouble = (char*) sqlite3_column_text(ppStmt, 2);

    /*
     * call apply filter
     */
    applyfilter(db, name, filter, nodouble);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  /*
   * Torrents are no longer new
   */
  nonewtorrents(db);

  /*
   * All gone well
   */
  return rc;
}

/*
 * Test for double downloads.
 * Queries need to be provided by the user.
 * return 1 if double, 0 if new
 */
static int testdouble(sqlite3 *db, char *nodouble, char *link, int season, int episode)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;

  /*
   * prepare query
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      nodouble,            /* SQL statement, UTF-8 encoded */
      strlen(nodouble),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Testdouble query failed: %s %s:%d", nodouble, __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Bind values
   * torrentdb.c:116:  rc = sqlite3_bind_text(ppStmt, 4, category, -1, SQLITE_TRANSIENT);
   * torrentdb.c:117:  rc = sqlite3_bind_int(ppStmt, 5, season);
   */
  rc = sqlite3_bind_text(ppStmt, 1, link, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_int(ppStmt, 2, season);
  rc = sqlite3_bind_int(ppStmt, 3, episode);

  /*
   * Execute query
   */
  step_rc = sqlite3_step(ppStmt);

  /*
   * Cleanup 
   */
  sqlite3_finalize(ppStmt);

  /*
   * return result
   */
  if(step_rc == SQLITE_ROW) {
    return 1; // ROW was found so It's a duplicate
  } else { 
    return 0; // None found we has fresh meat :)
  }
}

/*
 * Apply the filters from the query.
 */
static void applyfilter(sqlite3 *db, char *name, char *filter, char* nodouble)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;
  char          *link;
  char          *title;
  char          *pubdate;
  char          *category;
  int           season;
  int           episode;
  char          message[MAXMSGLEN+1];

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      filter,            /* SQL statement, UTF-8 encoded */
      strlen(filter),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Filter '%s' failed %s:%d", name, __FILE__, __LINE__);
    writelog(LOG_ERROR, "'%s'", filter);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  else {

    /*
     * Filters should allways first query for the link!
     */

    /*
     * loop until the end of the dataset is found
     */
    while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
      /*
       * Get name and query of the filters
       */
      link      = (char*) sqlite3_column_text(ppStmt, 0);
      title     = (char*) sqlite3_column_text(ppStmt, 1);
      pubdate   = (char*) sqlite3_column_text(ppStmt, 2);
      category  = (char*) sqlite3_column_text(ppStmt, 3);
      season    =  sqlite3_column_int(ppStmt, 4);
      episode   =  sqlite3_column_int(ppStmt, 5);

      /*
       * Test if episode is already there
       */
      if(testdouble(db, nodouble, link, season, episode) == 0) {
        /*
         * When enabled send an email.
         */
        snprintf(message, MAXMSGLEN, "Downloading %s S%dE%d", title, season, episode);
        sendrssmail(db, message, message);
        
        /*
         * Add a torrent to the downloaded table.
         */
        adddownloaded(db, title, link, pubdate, category, season, episode);

        /*
         * call apply filter
         */
        dodownload(db, link, title, season, episode, pubdate);
      } else {
        writelog(LOG_DEBUG, "%s Season %d Episode %d is a duplicate %s:%d", title, episode, season, __FILE__, __LINE__);
      }
    }

  }
  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
}


/*
 * Do download.
 * take url, create name and call curl routine
 */
static void dodownload(sqlite3 *db, char *link, char *title, int season, int episode, char *pubdate) 
{
  char filename[151];
  char *path = NULL;

  
  /*
   * get path to put torrent in
   */
  configgetproperty(db, CONF_TORRENTDIR, &path);

  /*
   * Create filename.
   */
  snprintf(filename, 150, "%s/%sS%dE%dR%s.torrent", path, title, season, episode, pubdate); 

  //downloadtofile(link, filename);
  findtorrentwrite(link, filename);

  free(path);
}

