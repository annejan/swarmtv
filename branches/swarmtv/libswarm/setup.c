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
#include <time.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <libxml/parser.h>
#include <swarm.h>

#include "types.h"
#include "logfile.h"
#include "filesystem.h"
#include "setup.h"
#include "database.h"
#include "databaseimpl.h"
#include "callback.h"

/*
 * Test if rsstorrentdir exists
 * @returns
 * return 0 when found, -1 if not found
 */
static int testrsstorrentdir()
{
	int 	rc=0;
	char 	*fullpath=NULL;

	rsstcompletepath(RSS_BASEDIR, &fullpath);

	rc = rsstfsexists(fullpath);

	free(fullpath);

	return rc;
}


/*
 * Create the torrent dir if rsstorrentdir does not exist
 * @returns
 * return 0 when found, -1 if not found
 */
static int makersstorrentdir()
{
	int 	rc=0;
	char 	*fullpath=NULL;

	/*
	 * Resolv path and create directory
	 */
	rsstcompletepath(RSS_BASEDIR, &fullpath);
	rc = rsstmakedir(fullpath);

	free(fullpath);
	return rc;
}


/*
 * Initialize the callback structures
 * @Arguments
 * callbacks Structure holding the callback structures to be initialized.
 * @return
 * 0 when all was initialized well
 */
static int rsstinitcallbacks(struct_callbacks *callbacks)
{
  unsigned int count=0;
	int retval=0;
  struct_callback **deref;
  struct_callback **walk;

  /*
   * Do a dirty cast
   */
  deref = (struct_callback**) callbacks;

  /*
   * Walk through the callbackstruct until all is initialized
   */
  for(count=0; count < (enum_callbacks) lastelement; count++)
  {
    walk = deref+count;
    retval |= rsstinitcallback(walk);
  }

	/*
	 * When all did go well, 0 is returned
	 */
	return retval;
}


/*
 * Free the callback structures
 * @Arguments
 * callbacks Structure holding the callback structures to be Freed.
 * @return
 * 0 when all was initialized well
 */
void rsstfreecallbacks(struct_callbacks *callbacks)
{
  unsigned int count=0;
  struct_callback **deref;
  struct_callback *walk;

  /*
   * Do a dirty cast
   */
  deref = (struct_callback**) callbacks;

  /*
   * Walk through the callbackstruct until all is initialized
   */
  for(count=0; count < (enum_callbacks) lastelement; count++)
  {
    walk = deref[count];
    rsstfreecallback(walk);
  }
}


/*
 * Initialize RSS-torrent handle
 * and make sure we have all subsystems operational
 * @Return
 * Pointer to handle on success, NULL on failure
 */
rsstor_handle *initrsstor()
{
	int rc=0;
	rsstor_handle *handle=NULL;

	/*
	 * Retrieve complete path
	 */
	rc = testrsstorrentdir();
	if(rc != 0) {
		rc= makersstorrentdir();
		if(rc != 0) {
			return NULL;
		}
	}

	/*
	 * Allocate structure
	 */
	handle = calloc(1, sizeof(rsstor_handle));

  /*
   * Initialize the database
   */
  rc = rsstinitdatabase(RSST_DBFILE, handle);  
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "Initializing db : \'%s\' failed %s:%d\n", RSST_DBFILE, __FILE__, __LINE__);
		return NULL;
  }

  /*
   * Initialize lib curl
   */
  curl_global_init(CURL_GLOBAL_ALL);

  /*
   * open logfile
   */
  rc = rsstinitlogdb(handle->db);
  if(rc != 0) {
		return NULL;
  }

	/*
	 * Initialize the callbackstructures
	 */
	rc = rsstinitcallbacks(&(handle->callback));
	if(rc != 0) {
    fprintf(stderr, "Allocation callbacks failed! %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Return handle to struct
	 */
	return handle;
}


/*
 * Free RSS-torrent handle
 * @Arguments
 * handle pointer to RSS-torrent structure
 */
void freersstor(rsstor_handle *handle)
{
  if(handle == NULL){
    return;
  }

	/*
	 * Free the callback structures
	 */
	rsstfreecallbacks(&(handle->callback));

  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();

  /* 
   * we're done with libcurl, so clean it up
   */ 
  curl_global_cleanup();

  /*
   * Close the sqlite database.
   * No way to get dbpointer here
   */
  sqlite3_close(handle->db);

  /*
   * Close logfile.
   */
  rsstcloselog();
	/*
	 * Free the handle struct
	 */
	free(handle);
}

