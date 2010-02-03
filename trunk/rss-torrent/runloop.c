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
#include <unistd.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>

#include "curlfile.h"
#include "config.h"
#include "filter.h"
#include "handleopts.h"
#include "torrentdownload.h"
#include "simplefilter.h"
#include "logfile.h"
#include "torrentdb.h"

/*
 * Parser includes
 */
#include "srcparser/defaultrss/defaultrss.h"
#include "srcparser/twitter/twitter.h"

#define true (1==1)

/*
 * Query to get sources to download.
 */
const char *query="select name, url, parser from sources";

/*
 * Apply a filter to the downloaded RSS code.
 * This routine holdes the refferences to different kind of filters.
 * (For now only rsstorrent.com format)
 */
static int parserdownload(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
  int rc;

  /*
   * compare the filter string and pass the downloaded file to the correct filtering routine.
   */
  if(strcmp(filter, "defaultrss") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = defaultrss(db, name, url, filter, rssfile); 
    return 0;
  }

  if(strcmp(filter, "twitter") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = twitter(db, name, url, filter, rssfile); 
    return 0;
  }

  /*
   * When no filter found.
   */
  writelog(LOG_ERROR, "No filter found '%s', ignoring file %s:%d", filter, __FILE__, __LINE__);
  return -1;
}

/*
 * Do the main work here
 */
static void dowork(sqlite3 *db){
  /*
   * Use the database query to get the sources.
   */
  sqlite3_stmt            *ppStmt;
  const char              *pzTail;
  int                     rc;
  int                     step_rc;
  char                    *zErrMsg = 0;
  char           *name;
  char           *url;
  char           *parser;
  MemoryStruct            rssfile;

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
    return;
  }

  /*
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  //cols = sqlite3_column_count(ppStmt);

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {

    /*
     * Get the values
     */
    name    = (char *) sqlite3_column_text(ppStmt, 0);
    url     = (char *) sqlite3_column_text(ppStmt, 1);
    parser  = (char *) sqlite3_column_text(ppStmt, 2); 
  
    rc = downloadtobuffer(url, &rssfile);
    if(rc == 0) {
      /*
       * Download succeded.
       */
      writelog(LOG_DEBUG, "Download succeded for %s : %s", name, url);

      /*
       * Filter the stuff and add it to the database.
       */
      rc = parserdownload(db, name, url, parser, &rssfile);
      if(rc != 0) {
        writelog(LOG_ERROR, "Filtering failed for %s : %s %s:%d", name, url, __FILE__, __LINE__);
      }
      
    } else {
      writelog(LOG_ERROR, "Download failed for %s : %s %s:%d", name, url, __FILE__, __LINE__);
    }
    freedownload(&rssfile);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
  /*
   * For every source download and parse it.
   */
}


/*
 * Main loop, dispatches tasks
 * When onetime != 0 run once then exit
 */
int runloop(sqlite3 *db, int onetime)
{
  int rc;
  int timewait;
  time_t before;
  time_t after;
  int    timeleft;

  rc = configgetint(db, CONF_REFRESH, &timewait);
  writelog(LOG_NORMAL, "Starting daemon, refresh %ds", timewait);

  while(true){
    before = time(NULL);
    /*
     * work through the sources and process them
     */
    writelog(LOG_NORMAL,"Downloading RSS feed(s)");
    dowork(db);
 
    /*
     * Calculate sleep time left.
     */
    after = time(NULL);
    timeleft = timewait - (after - before);
    if(timeleft < 0) {
      timeleft = 0;
    }

    writelog(LOG_NORMAL,"Checking for new torrents to download.");
    downloadtorrents(db);
    downloadsimple(db, 0);

    /*
     * Torrents are no longer new
     */
    nonewtorrents(db);

    /*
     * Run once.
     */
    if(onetime != 0) {
      break;
    }

    writelog(LOG_NORMAL,"Refresh done, sleeping %d seconds.", timeleft); 

    /*
     * Sleep timeout
     */
    sleep(timeleft); 
  } 

  /*
   * done.
   */
  return 0;
}
