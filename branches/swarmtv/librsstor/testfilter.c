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
 *  Program written by Paul Honig 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include <string.h>

#include "types.h"
#include "sandboxdb.h"
#include "logfile.h"
#include "database.h"
#include "databaseimpl.h"
#include "torrentdb.h"
#include "simplefilter.h"
#include "torrentdownload.h"
#include "regexp.h"

/*
 * The sandbox is created in a separate file, path is set here.
 */
#define  RSST_DBSANDBOX "~/.rsstorrent/sandbox.db"

/*
 * The max number of results in a search result.
 */
#define  PAGE_LIMIT "100"

/*
 * Structure to hold season and episode
 */
typedef struct {
	int season;
	int episode;
} season_episode;


/*
 * Copy content to downloaded
 * Prevent doubles from occurring.
 * match all records that match the filter, and copy them to the no double
 */
static int createdownloaded(sandboxdb *sandbox, char *filter, char *nodouble)
{
  int rc=0;
  char *delfilterquery="DELETE FROM downloaded";
	rsstor_handle handle;

	/*
	 * Initialize handle
	 */
	memset(&handle, 0, sizeof(rsstor_handle));
	handle.db=sandbox->db;

  /*
   * Clean downloaded.
   */
  rc = rsstexecutequery(sandbox->db, delfilterquery, NULL);
  if(rc == ROWS_ERROR){
    rsstwritelog(LOG_ERROR, "Deleting downloaded failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * use function to add records to the downloaded table.
   */
  rsstapplyfilter(&handle, "Sandbox", simple, nodouble, NULL, 1, filter,  NULL);

  return 0;
}

/*
 * Set all 'newtorrents' flags to new, to imitate new arrivals
 */
static int setnewtorrentflags(sandboxdb *sandbox)
{
  int   rc=0;
  int   retval=0;
  char *query = "UPDATE newtorrents SET new='Y'";  

  /*
   * do query
   */
  rc = rsstexecutequery(sandbox->db, query, NULL);
  if(rc != ROWS_CHANGED){
    rsstwritelog(LOG_ERROR, "Setting of new flag failed %s:%d", __FILE__, __LINE__);
    retval=-1;
  }

  return retval;
}

/*
 * Prepare the filter test database
 */
sandboxdb *rsstinitfiltertest()
{
  int       rc;
  sandboxdb *sandbox;

  /*
   * create sandbox
   */
  sandbox = rsstcreatesandbox(RSST_DBFILE, RSST_DBSANDBOX);
  if(sandbox == NULL) {
    rsstwritelog(LOG_ERROR, "Sandbox creation failed %s:%d", __FILE__, __LINE__);
    fprintf(stderr, "Sandbox creation failed %s:%d\n", __FILE__, __LINE__);
    return NULL;
  }

  /*
   * Prepare new torrents
   */
  rc = setnewtorrentflags(sandbox);
  if(rc != 0) {
    rsstclosesandbox(sandbox);
    return NULL;
  }

  return sandbox;
}


/*
 * Do filter test
 * show first 10 matches
 */
int rsstdofiltertest(char *filter, char *nodouble)
{
  int rc=0;
  sandboxdb *sandbox;
  char *query="select newtorrents.id, downloaded.title, downloaded.season, downloaded.episode from newtorrents, downloaded "
							"where newtorrents.link = downloaded.link order by newtorrents.id"; // get values from downloaded table

  /*
   * Initialize sandbox database
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creation failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute test filter
   */
  rc = createdownloaded(sandbox, filter, nodouble);
  if(rc != 0){
    printf("Execution of test filter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  printf(" Id                        | Title                     | Season                    | Episode\n");
  rc = rsstprintquery(sandbox->db, query, NULL);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}


/*
 * Clean-up the sandbox for filter processing
 * @Arguments
 * sandbox the sandbox to clean-up
 * @return
 * 0 on success, otherwise -1
 */
int rsstcleanoutdb(sandboxdb *sandbox)
{
  int 	rc=0;
  char 	*delquery="DELETE FROM downloaded";
  char 	*delsimplequery="DELETE FROM simplefilters";

  /*
   * Clean downloaded.
   */
  rc = rsstexecutequery(sandbox->db, delquery, NULL);
  if(rc == ROWS_ERROR){
    rsstwritelog(LOG_ERROR, "Deleting downloaded failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	/*
	 * Empty simple filters in sandbox
	 */
  rc = rsstexecutequery(sandbox->db, delsimplequery, NULL);
  if(rc == ROWS_ERROR){
    rsstwritelog(LOG_ERROR, "Deleting simple filters failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	return 0;
}


/*
 * Copy content to downloaded
 * Prevent doubles from occurring.
 * match all records that match the filter, and copy them to the no double
 */
static int createsimpledownloaded(sandboxdb *sandbox, simplefilter_struct *filter)
{
  int 					 rc=0;
	char 					*nodupsql=NULL;
	rsstor_handle  handle;

	/*
	 * Get sandbox db
	 */
	handle.db=sandbox->db;

	/*
	 * Remove unwanted data from sandbox.
	 */
	rc = rsstcleanoutdb(sandbox);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "Cleaning out sandbox failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Add filter to sandbox
	 */
	rsstaddsimplefilter(&handle, filter);

	/*
	 * Execute filter
	 * with simulate 1, to run the simple filters only in the database.
	 */
	rsstdownloadsimple(&handle, (SIM) sim);

	/*
	 * Clean up
	 */
	free(nodupsql);

  return 0;
}


/*
 * Get the newest season and episode
 * @arguments
 * filter Simple filter structure
 * Season newest season
 * Episode newest episode
 * @return
 * 0 on success otherwise 1
 * 1 When season and episode are found
 * -1 on error
 */
int rsstgetnewestepisode(simplefilter_struct *filter, int *season, int *episode)
{
  int rc=0;
  sandboxdb *sandbox=NULL;
	sqlite3_stmt *ppstmt=NULL;
	int retval=0;
	char *newestquery="SELECT season, episode FROM downloaded ORDER BY season DESC, episode DESC  LIMIT 1"; // Get the newest episode 

	/*
	 * Initialize season and episode
	 */
	*season = 0;
	*episode = 0;

  /*
   * Initialize sandbox database
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creation failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute test filter
   */
	rc = createsimpledownloaded(sandbox, filter);
  if(rc != 0){
    printf("Execution of test filter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute query to get newest season and episode numbers
   */
	rc = rsstexecqueryresult(sandbox->db, &ppstmt, newestquery, NULL);

	/*
	 * Extract numbers
	 */
	rc = sqlite3_step(ppstmt);
	
	/*
	 * We found an entry
	 */
	if(rc == SQLITE_ROW){
		*season = sqlite3_column_int(ppstmt, 0);
		*episode = sqlite3_column_int(ppstmt, 1);
		retval=1;
	}

	/*
	 * Free results
	 */
	sqlite3_finalize(ppstmt);

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	/*
	 * Done.
	 */
  return retval;
}


/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on success, return -1 on failure.
 */
int rsstdosimpletest(simplefilter_struct *filter)
{
  int rc=0;
  sandboxdb *sandbox;
  char *query="select title, season, episode from downloaded;"; // get values from downloaded table
	char *names[]={"Title", "Season", "Episode", "Torrent"};

  /*
   * Initializing sandbox database
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creation failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute test filter
   */
	rc = createsimpledownloaded(sandbox, filter);
  if(rc != 0){
    printf("Execution of test filter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  rc = rsstprintquerylist(sandbox->db, query, names, NULL);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}


/*
 * This routine retrieves the records from the downloaded table.
 * A selection is based on name.
 * This routine should not be here name wise, though all dependencies reside here.
 * I'll place it here for now.
 * @Arguments
 * db
 * optarg
 * @return
 * 0 on success
 * -1 on error
 */
int rsstfinddowned(rsstor_handle *handle, char *optarg)
{
	int rc=0;
	sqlite3 *db=NULL;

	static char *query  = "SELECT id, season, episode, title FROM downloaded WHERE IREGEXP(?1, title);";
	static char *names[] = {"Id", "Season", "Episode", "Title", "Downloaded torrent"};

	/*
	 * Get db pointer
	 */
	db = handle->db;

	/*
	 * print results
	 */
  rc = rsstprintquerylist(db, query, names, "s", optarg);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of test filter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	return 0;
}

