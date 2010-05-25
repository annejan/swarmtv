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
#include <rsstor.h>

#include "types.h"
#include "logfile.h"
#include "filesystem.h"
#include "setup.h"
#include "database.h"

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
 * Call this function to test if rsstorrent needs setting up to do.
 * When it does the function initializes the files that need to be in place in order to run rsstorrent.
 * @Arguments none
 * @returns 	0 when succes otherwise -1
 */
int rsstinitrsstorrent()
{
	int		rc=0;
	int		retval=0;

	/*
	 * Retrieve complete path
	 */
	rc = testrsstorrentdir();
	if(rc != 0) {
		rc= makersstorrentdir();
		if(rc != 0) {
			retval = -1;
		}
	}

	/*
	 * Done
	 */
	return retval;
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
	 * Allocate structure
	 */
	handle = calloc(1, sizeof(rsstor_handle));

  /*
   * Initialize the database
   */
  rc = rsstinitdatabase(DBFILE, handle);  
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "Initializing db : \'%s\' failed\n", DBFILE);
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
    fprintf(stderr, "Can't open logfile!\n");
		return NULL;
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
