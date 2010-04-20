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
#include "torrentdb.h"
#include "handleopts.h"
#include "simplefilter.h"
#include "torrentdownload.h"
#include "regexp.h"

#define  DBSANDBOX "~/.rsstorrent/sandbox.db"

#define  FINDNAME 	"filter"
#define  FINDNODUP	"none"
/*
 * The max number of results in a search result.
 */
#define  PAGE_LIMIT "100"

/*
 * Copy content to downloaded
 * Prevent doubles from occuring.
 * match all records that match the filter, and copy them to the no double
 */
static int createdownloaded(sandboxdb *sandbox, char *filter, char *nodouble)
{
  int rc=0;
  char *delfilterquery="DELETE FROM downloaded";

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
  rsstapplyfilter(sandbox->db, "Sandbox", nodouble, 1, filter,  NULL);

  return 0;
}

/*
 * Set all newtorrents flags to new, to imitate new arrivals
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
  sandbox = rsstcreatesandbox(DBFILE, DBSANDBOX);
  if(sandbox == NULL) {
    rsstwritelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    fprintf(stderr, "Sandbox creaton failed %s:%d\n", __FILE__, __LINE__);
    return NULL;
  }

  /*
   * Prepare newtorrents
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
   * Init sandbok db
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute testfilter
   */
  rc = createdownloaded(sandbox, filter, nodouble);
  if(rc != 0){
    printf("Execution of testfilter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  printf(" Id                        | Title                     | Season                    | Episode\n");
  rc = rsstprintquery(sandbox->db, query, NULL);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox falied %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}

/*
 * Cleanout the sandbox for filter processing
 * @Arguments
 * sandbox the sandbox to cleanout
 * @retun
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
    rsstwritelog(LOG_ERROR, "Deleting simplefilters failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	return 0;
}


/*
 * Copy content to downloaded
 * Prevent doubles from occuring.
 * match all records that match the filter, and copy them to the no double
 */
static int createsimpledownloaded(sandboxdb *sandbox, opts_struct *filter)
{
  int 	rc=0;
	char 	*nodupsql=NULL;

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
	rsstaddsimplefilter(sandbox->db, filter);

	/*
	 * Execute filter
	 * with simulate 1, to run the simplefilters only in the database.
	 */
	rsstdownloadsimple(sandbox->db, (SIM) sim);

	/*
	 * Clean up
	 */
	free(nodupsql);

  return 0;
}


/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on succes, return -1 on failure.
 */
int rsstdosimpletest(opts_struct *opts)
{
  int rc=0;
  sandboxdb *sandbox;
  char *query="select title, season, episode from downloaded;"; // get values from downloaded table
	char *names[]={"Title", "Season", "Episode", "Torrent"};

  /*
   * Init sandbok db
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute testfilter
   */
	rc = createsimpledownloaded(sandbox, opts);
  if(rc != 0){
    printf("Execution of testfilter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  rc = rsstprintquerylist(sandbox->db, query, names, NULL);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox falied %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}


/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on succes, return -1 on failure.
 */
int rsstfindtorrentids(opts_struct *opts)
{
  int rc=0;
	int retval=0;
	int count=0;
	char humansize[20];
	simplefilter_struct filter;
	newtorrents_container *newtorrents=NULL;

	/*
	 * NULL filter
	 */
	memset(&filter, 0, sizeof(simplefilter_struct));

	/*
	 * Add bogus name and nodup to filter
	 */
	if(opts->simplename == NULL) {
		rsstalloccopy(&(opts->simplename), FINDNAME, strlen(FINDNAME));
	}
	if(opts->simplenodup == NULL) {
		rsstalloccopy(&(opts->simplenodup), FINDNODUP, strlen(FINDNODUP));
	}

  /*
   * Translate argument to usable data
   */
  rc = rsstoptstosimple(opts, &filter);
  if(rc != 0){
    return -1;
  }
	
	rc = rsstfindnewtorrents(&filter, &newtorrents, 100, 0);
  if(rc != 0){
    retval=-1;
  }

	if(retval == 0){
		/*
		 * Print results
		 */
		for(count=0; count < newtorrents->nr; count++){
			rsstsizetohuman(newtorrents->newtorrent[count].size, humansize);
			printf("id: %d, name: %s , size: %s\n", 
					newtorrents->newtorrent[count].id,
					newtorrents->newtorrent[count].title,
					humansize);
			printf("url: %s\n\n", 
					newtorrents->newtorrent[count].link);
		}
	}
	
	rc = rsstfreenewtorrentscontainer(newtorrents);
  if(rc != 0){
    retval=-1;
  }

	return retval;
#if 0
  sandboxdb *sandbox;
  char *query="SELECT newtorrents.id, downloaded.title, downloaded.season, downloaded.episode FROM newtorrents, downloaded "
							"WHERE newtorrents.link = downloaded.link ORDER BY newtorrents.id LIMIT " PAGE_LIMIT ""; // get values from downloaded table
	char *names[]={"Id", "Title", "Season", "Episode", "Torrent"};


  /*
   * Init sandbok db
   */
  sandbox = rsstinitfiltertest();
  if(sandbox == NULL){
    rsstwritelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute testfilter
   */
	rc = createsimpledownloaded(sandbox, opts);
  if(rc != 0){
    printf("Execution of testfilter failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  rc = rsstprintquerylist(sandbox->db, query, names, NULL);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = rsstclosesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    rsstwritelog(LOG_ERROR, "Closing sandbox falied %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
#endif
}

/*
 * This routine retrieves the records from the downloaded table.
 * A selection is based on name.
 * This routine should not be here namewise, but sinds the other function ended up here
 * I'll place it here for now.
 * @Arguments
 * db
 * optarg
 * @return
 * 0 on success
 * -1 on error
 */
int rsstfinddowned(sqlite3 *db, char *optarg)
{
	int rc=0;

	static char *query  = "SELECT id, season, episode, title FROM downloaded WHERE IREGEXP(?1, title);";
	static char *names[] = {"Id", "Season", "Episode", "Title", "Downloaded torrent"};

	/*
	 * print results
	 */
  rc = rsstprintquerylist(db, query, names, "s", optarg);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    rsstwritelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	return 0;
}

