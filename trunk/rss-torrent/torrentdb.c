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
#include <time.h>

#include "config.h"
#include "database.h"
#include "logfile.h"


/*
 * Add a torrent to the newtorrent table.
 */
int addnewtorrent(sqlite3 *db,
               char *title,
               char *link,
               time_t pubdate,
               char *category,
               int  season,
               int  episode,
               int  seeds,
               int  peers,
               size_t size)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  char          *zErrMsg = 0;


  char *query = "INSERT INTO newtorrents (title, link, pubdate, category, season, episode, seeds, peers, size, new) "
                "VALUES (?1, ?2, date(?3, 'unixepoch'), ?4, ?5, ?6, ?7, ?8, ?9, 'Y')";
  
  // DEBUG
  writelog(LOG_DEBUG, "############"
      "title:    %s\n"
      "link:     %s\n"
      "pubdate:  %s\n"
      "category: %s\n"
      "season:   %d\n"
      "episode:  %d\n"
      "seeds:    %d\n"
      "peers:    %d\n"
      "size:     %ld\n",
      title, link, ctime(&pubdate), category, season, episode, seeds, peers, size);

  /*
   *  create table newtorrents (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE,
   *  category TEXT, season INTEGER, episode INTEGER, seeds INTEGER, peers INTEGER, size INTEGER, new TEXT);
   *
   * title:     WWE.Raw.09.28.09.DSR.XviD-XWT.avi  [1/2]
   * link:     http://www.rsstorrents.com/detail.php?id=387360
   * pubdate:  1254178800
   * category: TVShows
   * season:   0
   * episode:  2
   * seeds:    0
   * peers:    0
   * size:     781229980
   * sleep for 299s.
   *
   * INSERT INTO newtorrent ( title, link, pubdate, category, season, episode, seeds, peers, size, new) 
   * VALUES ( 'WWE.Raw.09.28.09.DSR.XviD-XWT.avi', 'http://www.rsstorrents.com/detail.php?id=387360',
   *          datetime(1254178800), 'TVShows', '0', '2', '0', '0', '781229980', 'Y');
   *
   * INSERT INTO newtorrent ( title, link, pubdate, category, season, episode, seeds, peers, size, new)
   * VALUES ( \1, \2, datetime(\3), \4, \5, \6, \7, \8, \9, 'Y' );
   */
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
    writelog(LOG_ERROR, "sqlite3_prepare_v2 returned %d %s:%d", rc, __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Bind value's
   */
  // int sqlite3_bind_int(sqlite3_stmt*, int, int);
  // int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
  rc = sqlite3_bind_text(ppStmt, 1, title, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_text(ppStmt, 2, link,  -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_int(ppStmt, 3, pubdate);
  rc = sqlite3_bind_text(ppStmt, 4, category, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_int(ppStmt, 5, season);
  rc = sqlite3_bind_int(ppStmt, 6, episode);
  rc = sqlite3_bind_int(ppStmt, 7, seeds);
  rc = sqlite3_bind_int(ppStmt, 8, peers);
  rc = sqlite3_bind_int(ppStmt, 9, size);

  /*
   * execute query, when failed return -1
   */
  rc = sqlite3_step(ppStmt);
  switch(rc) {
    case SQLITE_DONE:
      // print nothing all is okay
      break;
    case SQLITE_CONSTRAINT:
      writelog(LOG_DEBUG, "Torrent allready in DB"); 
      break;
    default:
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
  if(rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
    writelog(LOG_ERROR, "Finalize failed %d! %s:%d", rc, __FILE__, __LINE__);
    exit(1);
  }

  /*
   * no errors
   */
  return 0;
} 


/*
 * Add a torrent to the downloaded table.
 */
int adddownloaded(sqlite3 *db,
               char *title,
               char *link,
               char *pubdate,
               char *category,
               int  season,
               int  episode)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  char          *zErrMsg = 0;
  time_t        now;



  char *query = "INSERT INTO downloaded (title, link, pubdate, category, season, episode, date) "
                "VALUES (?1, ?2, ?3, ?4, ?5, ?6,  datetime(?7, 'unixepoch', 'localtime'))";
  

  // DEBUG
  writelog(LOG_NORMAL, "##### Download #######"
      "title:    %s\n"
      "link:     %s\n"
      "pubdate:  %s\n"
      "category: %s\n"
      "season:   %d\n"
      "episode:  %d\n",
      title, link, pubdate, category, season, episode);

  /*
   *  create table newtorrents (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE,
   *  category TEXT, season INTEGER, episode INTEGER, seeds INTEGER, peers INTEGER, size INTEGER, new TEXT);
   *
   * title:     WWE.Raw.09.28.09.DSR.XviD-XWT.avi  [1/2]
   * link:     http://www.rsstorrents.com/detail.php?id=387360
   * pubdate:  1254178800
   * category: TVShows
   * season:   0
   * episode:  2
   * seeds:    0
   * peers:    0
   * size:     781229980
   * sleep for 299s.
   *
   * INSERT INTO newtorrent ( title, link, pubdate, category, season, episode, seeds, peers, size, new) 
   * VALUES ( 'WWE.Raw.09.28.09.DSR.XviD-XWT.avi', 'http://www.rsstorrents.com/detail.php?id=387360',
   *          datetime(1254178800), 'TVShows', '0', '2', '0', '0', '781229980', 'Y');
   *
   * INSERT INTO newtorrent ( title, link, pubdate, category, season, episode, seeds, peers, size, new)
   * VALUES ( \1, \2, datetime(\3), \4, \5, \6, \7, \8, \9, 'Y' );
   */
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
    writelog(LOG_ERROR, "sqlite3_prepare_v2 returned %d %s:%d", rc, __FILE__, __LINE__);
    writelog(LOG_ERROR, "On query: %s", query);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Bind value's
   */
  // int sqlite3_bind_int(sqlite3_stmt*, int, int);
  // int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
  now = time(NULL);
  rc = sqlite3_bind_text(ppStmt, 1, title, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_text(ppStmt, 2, link,  -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_text(ppStmt, 3, pubdate, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_text(ppStmt, 4, category, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_int(ppStmt, 5, season);
  rc = sqlite3_bind_int(ppStmt, 6, episode);
  rc = sqlite3_bind_int(ppStmt, 7, now);

  /*
   * execute query, when failed return -1
   */
  rc = sqlite3_step(ppStmt);
  switch(rc) {
    case SQLITE_DONE:
      // print nothing all is okay
      break;
    case SQLITE_CONSTRAINT:
      writelog(LOG_ERROR, "Torrent '%s' allready downloaded %s:%d", link, __FILE__, __LINE__); 
      break;
    default:
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
  if(rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
    writelog(LOG_ERROR, "Finalize failed %d! %s:%d", rc, __FILE__, __LINE__);
    exit(1);
  }

  /*
   * no errors
   */
  return 0;
} 


/*
 * When Torrents are prosessed, they are no longer new
 * this method removes the new flag
 */
void nonewtorrents(sqlite3 *db)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  char          *zErrMsg = 0;

  const char *query = "UPDATE newtorrents SET new='N' WHERE new='Y'";

  writelog(LOG_DEBUG,"New torrents are marked old now !");

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
    writelog(LOG_ERROR, "sqlite3_prepare_v2 returned %d %s:%d", rc, __FILE__, __LINE__);
    writelog(LOG_ERROR, "On query: %s", query);
    sqlite3_free(zErrMsg);
    return;
  }

  /*
   * execute query.
   */
  rc = sqlite3_step(ppStmt);
  if(rc != SQLITE_DONE){
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }


  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
  if(rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
    writelog(LOG_ERROR, "Finalize failed %d! %s:%d", rc, __FILE__, __LINE__);
    exit(1);
  }

  /*
   * no errors
   */
  return;
}
