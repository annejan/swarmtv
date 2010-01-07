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
  int           rc;


  char *query = "INSERT INTO newtorrents (title, link, pubdate, category, season, episode, seeds, peers, size, new) "
                "VALUES (?1, ?2, date(?3, 'unixepoch'), ?4, ?5, ?6, ?7, ?8, ?9, 'Y')";
  
  // DEBUG
  writelog(LOG_DEBUG, "############\n"
      "title:    %s\n"
      "link:     %s\n"
      "pubdate:  %s\n"
      "category: %s\n"
      "season:   %d\n"
      "episode:  %d\n"
      "seeds:    %d\n"
      "peers:    %d\n"
      "size:     %ln",
      title, link, ctime(&pubdate), category, season, episode, seeds, peers, size);

  /*
   * execute query
   */
  rc = executequery(db, query, "ssdsddddd", title, link, pubdate, category, season, episode, seeds, peers, size);
  switch(rc) {
    case ROWS_EMPTY:
    case ROWS_CHANGED:
      // print nothing all is okay
      break;
    case ROWS_CONSTRAINT:
      writelog(LOG_DEBUG, "Torrent allready in DB"); 
      break;
    default:
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
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
void adddownloaded(sqlite3 *db,
               char *title,
               char *link,
               char *pubdate,
               char *category,
               int  season,
               int  episode,
               int  simulate)
{
  int           rc;
  time_t        now;

  char *query = "INSERT INTO downloaded (title, link, pubdate, category, season, episode, date) "
                "VALUES (?1, ?2, ?3, ?4, ?5, ?6,  datetime(?7, 'unixepoch', 'localtime'))";

  /*
   * Do not log downloading when we are testing filters.
   */
  if(simulate == 0){
    writelog(LOG_NORMAL, "##### Download #######\n"
        "title:    %s\n"
        "link:     %s\n"
        "pubdate:  %s\n"
        "category: %s\n"
        "season:   %d\n"
        "episode:  %d",
        title, link, pubdate, category, season, episode);
  }

  /*
   * execute query, when failed return -1
   */
  rc = executequery(db, query, "ssssddd", title, link, pubdate, category, season, episode, now); 
  switch(rc) {
    case ROWS_EMPTY:
    case ROWS_CHANGED:
      // print nothing all is okay
      break;
    case ROWS_CONSTRAINT:
      writelog(LOG_ERROR, "Torrent '%s' allready downloaded, please check no double filters for '%s'. %s:%d", link, title, __FILE__, __LINE__); 
      break;
    default:
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }
} 

/*
 * When Torrents are prosessed, they are no longer new
 * this method removes the new flag
 */
void nonewtorrents(sqlite3 *db)
{
  int           rc;

  const char *query = "UPDATE newtorrents SET new='N' WHERE new='Y'";

  writelog(LOG_DEBUG,"New torrents are marked old now !");

  rc = executequery(db, query, NULL); 
  if(rc == SQLITE_ERROR){
      writelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }

  /*
   * no errors
   */
  return;
}
