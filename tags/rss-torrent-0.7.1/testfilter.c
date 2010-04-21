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

#include "types.h"
#include "sandboxdb.h"
#include "logfile.h"
#include "database.h"
#include "torrentdb.h"
#include "handleopts.h"
#include "simplefilter.h"
#include "torrentdownload.h"

#define  DBSANDBOX "~/.rsstorrent/sandbox.db"

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
  rc = executequery(sandbox->db, delfilterquery, NULL);
  if(rc == ROWS_ERROR){
    writelog(LOG_ERROR, "Deleting downloaded failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * use function to add records to the downloaded table.
   */
  applyfilter(sandbox->db, "Sandbox", nodouble, 1, filter,  NULL);

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
  rc = executequery(sandbox->db, query, NULL);
  if(rc != ROWS_CHANGED){
    writelog(LOG_ERROR, "Setting of new flag failed %s:%d", __FILE__, __LINE__);
    retval=-1;
  }

  return retval;
}

/*
 * Prepare the filter test database
 */
static sandboxdb *initfiltertest()
{
  int       rc;
  sandboxdb *sandbox;

  /*
   * create sandbox
   */
  sandbox = createsandbox(DBFILE, DBSANDBOX);
  if(sandbox == NULL) {
    writelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    fprintf(stderr, "Sandbox creaton failed %s:%d\n", __FILE__, __LINE__);
    return NULL;
  }

  /*
   * Prepare newtorrents
   */
  rc = setnewtorrentflags(sandbox);
  if(rc != 0) {
    closesandbox(sandbox);
    return NULL;
  }

  return sandbox;
}


/*
 * Do filter test
 * show first 10 matches
 */
int dofiltertest(char *filter, char *nodouble)
{
  int rc=0;
  sandboxdb *sandbox;
  char *query="select title, season, episode, pubdate from downloaded"; // get values from downloaded table

  /*
   * Init sandbok db
   */
  sandbox = initfiltertest();
  if(sandbox == NULL){
    writelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute testfilter
   */
  rc = createdownloaded(sandbox, filter, nodouble);
  if(rc != 0){
    printf("Execution of testfilter failed.\n");
    writelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  printf("Title                     | Season                    | Episode                   | Pubdate\n");
  rc = printquery(sandbox->db, query);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    writelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = closesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    writelog(LOG_ERROR, "Closing sandbox falied %s:%d", __FILE__, __LINE__);
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
  char 	*delquery="DELETE FROM downloaded";
  char 	*delsimplequery="DELETE FROM simplefilters";

  /*
   * Clean downloaded.
   */
  rc = executequery(sandbox->db, delquery, NULL);
  if(rc == ROWS_ERROR){
    writelog(LOG_ERROR, "Deleting downloaded failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	/*
	 * Empty simple filters in sandbox
	 */
  rc = executequery(sandbox->db, delsimplequery, NULL);
  if(rc == ROWS_ERROR){
    writelog(LOG_ERROR, "Deleting simplefilters failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

	/*
	 * Add filter to sandbox
	 */
	addsimplefilter(sandbox->db, filter);

	/*
	 * Execute filter
	 * with simulate 1, to run the simplefilters only in the database.
	 */
	downloadsimple(sandbox->db, (SIM) sim);

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
int dosimpletest(opts_struct *opts)
{
  int rc=0;
  sandboxdb *sandbox;
  char *query="select title, season, episode, pubdate from downloaded"; // get values from downloaded table

  /*
   * Init sandbok db
   */
  sandbox = initfiltertest();
  if(sandbox == NULL){
    writelog(LOG_ERROR, "Sandbox creaton failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Execute testfilter
   */
	rc = createsimpledownloaded(sandbox, opts);
  if(rc != 0){
    printf("Execution of testfilter failed.\n");
    writelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Print content of downloaded
   */
  printf("Title                     | Season                    | Episode                   | Pubdate\n");
  rc = printquery(sandbox->db, query);
  if(rc != 0){
    printf("Listing of download queue failed.\n");
    writelog(LOG_ERROR, "Execution of testfilter failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * cleanup sandbox
   */
  rc = closesandbox(sandbox);
  if(rc != 0){
    printf("Closing sandbox failed.\n");
    writelog(LOG_ERROR, "Closing sandbox falied %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}

