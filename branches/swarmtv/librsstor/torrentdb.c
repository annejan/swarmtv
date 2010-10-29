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

#include "types.h"
#include "config.h"
#include "database.h"
#include "logfile.h"
#include "torrentdownload.h"
#include "lastdownloaded.h"

#define 	BUFSIZE 	20

/*
 * Free strings in newtorrents_struct 
 * Be sure to free the struct yourself.
 * @Arguments 
 * newtor structure pointer
 * @Return 
 * void, exits on failure
 */
void rsstfreenewtor(newtorrents_struct *newtor)
{
	free(newtor->title);
	free(newtor->link);
	free(newtor->category);
	free(newtor->source);
}


/*
 * Single out unlikely season/episode numbers
 * @Arguments
 * newtor New torrent struct to delete unusual season/episode combinations from.
 */
static void rsstfilterseasonepisode(newtorrents_struct *newtor)
{
	int magicnumbers[] = {264, 720, 1080, 0};
	int count=0;
	int season = newtor->season;
	int	episode	= newtor->episode;

	/*
	 * Make sure no 264 enters in season or episode
	 */
	while(magicnumbers[count] != 0){
		/*
		 * if season or episode is one of the forbidden numbers, reset both
		 */
		if(season == magicnumbers[count] || episode == magicnumbers[count]){
			newtor->season = 0;
			newtor->episode = 0;
		}
	
		/*
		 * next
		 */
		count++;
	}
}


/*
 * Add a torrent to the newtorrent table.
 * @Arguments
 * newtor structure holding the values for the record to be added
 * @Return
 * 0 on success, exits on -1 on failure
 */
int rsstaddnewtorrent(rsstor_handle *handle, newtorrents_struct *newtor)
{
  int           rc=0;
	sqlite3      *db=NULL;

	/*
	 * Get db pointer;
	 */
	db = handle->db;

  char *query = "INSERT INTO newtorrents (title, link, pubdate, category, source, season, episode, seeds, peers, size, metatype, new) "
                "VALUES (?1, ?2, date(?3, 'unixepoch'), ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, 'Y')";
  
	/*
	 * Remove numbers that refer to video encoding but could end up as season/episode numbers.
	 */
	rsstfilterseasonepisode(newtor);

  // DEBUG
  rsstwritelog(LOG_DEBUG, "############\n"
      "title:    %s\n"
      "link:     %s\n"
      "pubdate:  %s\n"
      "category: %s\n"
			"source:   %s\n"
      "season:   %d\n"
      "episode:  %d\n"
      "seeds:    %d\n"
      "peers:    %d\n"
      "size:     %ld\n"
      "metatype: %s\n",
      	newtor->title, newtor->link, ctime(&(newtor->pubdate)), newtor->category, newtor->source, 
				newtor->season, newtor->episode, newtor->seeds, newtor->peers, newtor->size, newtor->metatype);

  /*
   * execute query
   */
  rc = rsstexecutequery(db, query, "ssdssddddfs", 
			newtor->title, newtor->link, newtor->pubdate, newtor->category, newtor->source, newtor->season, 
			newtor->episode, newtor->seeds, newtor->peers, (double)(newtor->size), newtor->metatype);
  switch(rc) {
    case ROWS_EMPTY:
    case ROWS_CHANGED:
      // print nothing all is okay
      break;
    case ROWS_CONSTRAINT:
      rsstwritelog(LOG_DEBUG, "Torrent already in DB"); 
      break;
    default:
      rsstwritelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }

  /*
   * no errors
   */
  return 0;
} 


/*
 * Add a torrent to the downloaded table.
 * Arguments
 * downed			pointer to struct holding values to add to the db.
 * simulate		0 to log addition, 1 adds anyway, but does not log at all.
 */
void rsstadddownloaded(rsstor_handle *handle, downloaded_struct *downed, SIM  simulate)
{
  int        rc=0;
  time_t     now=0;
	sqlite3 	*db=NULL;

	db = handle->db;

  char *query = "INSERT INTO downloaded (title, link, pubdate, category, season, episode, metatype, date) "
                "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7,  datetime(?8, 'unixepoch', 'localtime'))";

  /*
   * The time is now, now is the time
   */
  now = time(NULL);

  /*
   * Do not log downloading when we are testing filters.
   */
  if(simulate == (SIM) real){
    rsstwritelog(LOG_NORMAL, "##### Download #######\n"
        "title:    %s\n"
        "link:     %s\n"
        "pubdate:  %s\n"
        "category: %s\n"
        "metatype: %s\n"
        "season:   %d\n"
        "episode:  %d",
        downed->title, downed->link, downed->pubdate, downed->category, downed->metatype, downed->season, downed->episode);
  }

  /*
   * execute query, when failed return -1
   */
  rc = rsstexecutequery(db, query, "ssssddsd", downed->title, downed->link, downed->pubdate, downed->category, 
			downed->season, downed->episode, downed->metatype, now); 
  switch(rc) {
    case ROWS_EMPTY:
    case ROWS_CHANGED:
      // print nothing all is okay
      break;
    case ROWS_CONSTRAINT:
      rsstwritelog(LOG_ERROR, "Torrent '%s' already downloaded, please check no double filters for '%s'. %s:%d", 
					downed->link, downed->title, __FILE__, __LINE__); 
      break;
    default:
      rsstwritelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }
} 

/*
 * When Torrents are processed, they are no longer new
 * this method removes the new flag
 */
void rsstnonewtorrents(rsstor_handle *handle)
{
  int        rc=0;
	sqlite3 	*db=NULL;

	db=handle->db;

  const char *query = "UPDATE newtorrents SET new='N' WHERE new='Y'";

  rsstwritelog(LOG_DEBUG,"New torrents are marked old now !");

  rc = rsstexecutequery(db, query, NULL); 
  if(rc == SQLITE_ERROR){
      rsstwritelog(LOG_ERROR, "SQL statement failed %d! %s:%d", rc, __FILE__, __LINE__);
      exit(1);
  }

  /*
   * no errors
   */
  return;
}

/*
 * Delete all newtorrents entries older the x days
 * This function returns 0 on success, -1 SQL on failure.
 * Arguments
 * days 	The number of days records get retained in the newtorrents table.
 * returns
 * -1 on error, 0 on success
 */
int rsstdeloldnewtorents(rsstor_handle *handle, unsigned int days)
{
	char daystr[BUFSIZE+1];
	int  rc=0;
	sqlite3 *db=NULL;

	db = handle->db;

	/*
	 * Query to delete old entries.
	 */
	char *query="DELETE from newtorrents where pubdate <  date('now', ?1)";

	/*
	 * Build string
	 */
	memset(daystr, 0, BUFSIZE+1);
	snprintf(daystr, BUFSIZE, "-%d days", days);

	/*
	 * Execute query
	 */
	rc = rsstexecutequery(db, query, "s", daystr);
	if(rc == -1) {
		return -1;
	}
	
	/*
	 * Log if we deleted old newtorrents records.
	 */
	if(rc == 1) {
		rsstwritelog(LOG_DEBUG, "Deleted old torrents.");
	} else {
		rsstwritelog(LOG_DEBUG, "No newtorrent records have aged enough.");
	}

	return 0;
}


/*
 * Delete from downloaded table
 * @arguments
 * id 	id of the downloaded torrent to delete from downed table
 * @returns
 * 0 	On success
 * -1 on failure
 */
int rsstdeldownloaded(rsstor_handle *handle, int id)
{
	int      rc=0;
	sqlite3 *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;
	
	static char *query="delete from downloaded where id=?1;";
	
	/*
	 * Execute query to delete 
	 */
	rc = rsstexecutequery(db, query, "d", id);

	return rc;
}
