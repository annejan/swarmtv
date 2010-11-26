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
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>

#include "types.h"
#include "curlfile.h"
#include "config.h"
#include "filter.h"
#include "runloop.h"
#include "torrentdownload.h"
#include "simplefilter.h"
#include "logfile.h"
#include "torrentdb.h"
#include "callback.h"
#include "callbackimpl.h"

/*
 * Parser includes
 */
#include "srcparser/defaultrss/defaultrss.h"
#include "srcparser/twitter/twitter.h"

#define true (1==1)
#define ERRORLEN 1024

/*
 * Query to get sources to download.
 */
const char *query="select name, url, parser, metatype, id from sources";

/*
 * Apply a filter to the downloaded RSS code.
 * This routine holds the references to different kind of filters.
 * (For now only RSS-torrent.com format)
 * Return
 * 0 when okay, on error -1
 */
static int parserdownload(rsstor_handle *handle, char *name, char *url, char *metatype, char *filter, MemoryStruct *rssfile)
{
  int rc;
	
  /*
   * compare the filter string and pass the downloaded file to the correct filtering routine.
   */
  if(strcmp(filter, "defaultrss") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = defaultrss(handle, name, url, filter, metatype, rssfile); 
    return 0;
  }

  if(strcmp(filter, "twitter") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = twitter(handle, name, url, filter, metatype, rssfile); 
    return 0;
  }

  /*
   * When no filter found.
   */
  rsstwritelog(LOG_ERROR, "No filter found '%s', ignoring file %s:%d", filter, __FILE__, __LINE__);
  return -1;
}

/*
 * Get the config value for the amount of time we want to retain data.
 * Delete the older data.
 * Exits status 1 on error
 */
static void deleteold(rsstor_handle *handle)
{
	int rc;
	int days;

	/*
	 * Get the config value.
	 */
	rc = rsstconfiggetint(handle, CONF_RETAIN, &days);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Config value %s not set ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}


	/*
	 * Delete the content.
	 */
	rc = rsstdeloldnewtorents(handle, (unsigned int) days);
	if(rc == -1) {
		rsstwritelog(LOG_ERROR, "Database inconsistent, could not remove old newtorrent entries!", __FILE__, __LINE__);
		exit(1);
	}
}

/*
 * Call RSS download callback
 * @arguments
 * handle 	RSS-torrent handle
 * id				id of RSS that was downloaded
 * status		0 if okay, otherwise -1, error is set when status is -1
 * error		String containing printable error, when status is okay set to NULL
 * @return
 * 0 if all callbacks returned 0, otherwise !0
 */
static int rsstrssdownloadcallback(rsstor_handle *handle, int id, int status, char *error)
{
	int rc=0;
	struct_download down;

	/*
	 * Build struct
	 */
	down.id=id;
	down.status=status;
	down.error=error;

	/*
	 * Execute callbacks
	 */
	rc = rsstexecdownrsscallbacks(handle, &down);

	return rc;
}

/*
 * Do the main work here
 * Download the feeds and pars them.
 */
static void dowork(rsstor_handle *handle){
  /*
   * Use the database query to get the sources.
   */
  sqlite3_stmt            *ppStmt=NULL;
  const char              *pzTail=NULL;
  int                     rc=0;
  int                     step_rc=0;
  char                    *zErrMsg=NULL;
  char           					*name=NULL;
  char           					*url=NULL;
  char                    *metatype=NULL;
  char           					*parser=NULL;
	char										errstr[ERRORLEN+1];
	int											id=0;
  MemoryStruct            rssfile;

  /*
   * Prepare the Sqlite statement
   */
  rc = sqlite3_prepare_v2(
      handle->db,                 /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return;
  }

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
    metatype= (char *) sqlite3_column_text(ppStmt, 3);
		id      = 				 sqlite3_column_int (ppStmt, 4);
  
    rc = rsstdownloadtobuffer(url, &rssfile);
    if(rc == 0) {
      /*
       * Download succeeded.
       */
      rsstwritelog(LOG_DEBUG, "Download succeeded for %s : %s : %s", name, url, metatype);

      /*
       * Filter the stuff and add it to the database.
       */
      rc = parserdownload(handle, name, url, metatype, parser, &rssfile);
      if(rc == 0) {
				/*
				 * Callback RSS download is okay
				 */
				rsstrssdownloadcallback(handle, id, 0, NULL);

			} else {
				/*
				 * Call RSS download has failed because the content could not be parsed
				 */
				snprintf(errstr, ERRORLEN, "RSS source '%s' '%s' failed to download.", name, url);
				rsstrssdownloadcallback(handle, id, -1, errstr);
        rsstwritelog(LOG_ERROR, "Filtering failed for %s : %s %s:%d", name, url, __FILE__, __LINE__);
      }
      
    } else {
			/*
			 * Call RSS download has failed, because file could not be retrieved.
			 */
      rsstwritelog(LOG_ERROR, "Download failed for %s : %s %s:%d", name, url, __FILE__, __LINE__);
    }
    rsstfreedownload(&rssfile);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
}

/*
 * Do a cycle of workload
 * @Arguments
 * handle
 * @return
 * 0 for now
 */
int runcycle(rsstor_handle *handle)
{

  int 			rc=0;

  /*
   * Call callback to signal start of update
   */
  rc = rsstexecstartupcallbacks(handle);
  if(rc != 0){
    rsstwritelog(LOG_ERROR, "Error returned by 'startup' callback. %s:%d", __FILE__, __LINE__);
  }

  /*
   * work through the sources and process them
   */
  dowork(handle);

  /*
   * Execute SQL and simple filters on new entries.
   */
  rsstdownloadtorrents(handle);
  rsstdownloadsimple(handle, 0);

  /*
   * Torrents are no longer new
   */
  rsstnonewtorrents(handle);
  deleteold(handle);

  /*
   * Call callback to signal start of update, -1 as parameter, as we don't know anything about time
   */
  rc = rsstexecendupcallbacks(handle, -1);

  return 0;
}

/*
 * Main loop, dispatches tasks
 * When onetime != 0 run once then exit
 * @Argument
 * onetime when ! 0 runloop only runs one time.
 * @Return
 * 0 for now.
 */
int rsstrunloop(rsstor_handle *handle, LOOPMODE onetime)
{
  int 			rc=0;
  int				timewait=0;
  time_t 		before=0;
  time_t 		after=0;
  int    		timeleft=0;

  rc = rsstconfiggetint(handle, CONF_REFRESH, &timewait);
  if(onetime == 0) {
    rsstwritelog(LOG_NORMAL, "Starting daemon, refresh %ds", timewait);
  } else {
    rsstwritelog(LOG_NORMAL, "Running once.");
  }

  /*
   * Keep running until...
   */
  while(true){
    /*
     * Call callback to signal start of update
     */
    rc = rsstexecstartupcallbacks(handle);
    if(rc != 0){
      rsstwritelog(LOG_ERROR, "Error returned by 'startup' callback. %s:%d", __FILE__, __LINE__);
    }

    before = time(NULL);
    /*
     * work through the sources and process them
     */
    rsstwritelog(LOG_DEBUG,"Downloading RSS feed(s)");
    dowork(handle);
 

		/*
		 * Execute SQL and simple filters on new entries.
		 */
    rsstwritelog(LOG_DEBUG,"Checking for new torrents to download.");
    rsstdownloadtorrents(handle);
    rsstdownloadsimple(handle, 0);

    /*
     * Torrents are no longer new
     */
    rsstnonewtorrents(handle);
		deleteold(handle);

    /*
     * Calculate sleep time left.
     */
    after = time(NULL);
    timeleft = timewait - (after - before);
    if(timeleft < 0) {
      timeleft = timewait;
    }

		/*
		 * Call callback to signal start of update
		 * Time left is also given when run once
		 */
		rc = rsstexecendupcallbacks(handle, timeleft);
		if(rc != 0){
			rsstwritelog(LOG_ERROR, "Error returned by 'endup' callback. %s:%d", __FILE__, __LINE__);
		}

    /*
     * Run once.
     */
    if(onetime == (LOOPMODE) once) {
      break;
    }

		/*
		 * Inform user
		 */
    rsstwritelog(LOG_NORMAL,"Refresh done, sleeping %d seconds.", timeleft); 

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
