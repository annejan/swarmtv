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
#include <sqlite3.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <libxml/parser.h>
#include <signal.h>

#include "types.h"
#include "database.h"
#include "logfile.h"
#include "handleopts.h"
#include "setup.h"

/*
 * Bit of a hack.. but needed for cleanup
 */
static sqlite3 *cleandb;

/*
 * Clean up 
 */
void cleanup(){
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
  sqlite3_close(cleandb);

  /*
   * Close logfile.
   */
  rsstcloselog();
}


/*
 * Handle standard signals
 */
void Signal_Handler(int sig) /* signal handler function */
{
  switch(sig){
    case SIGHUP:
    case SIGPIPE:
      /* Do nothing */
      break;    
    case SIGINT:
    case SIGTERM:
      /* finalize the server */
      printf("Cleaning up\n");
      cleanup();
      exit(0);
      break;    
  } 
}


/*
 * Temporary main function.
 */
int main(int argc, char **argv){
	rsstor_handle handle;
  sqlite3    *db=NULL;
  int         rc=0;

	/*
	 * Handle signals the nice way.
	 */
  signal(SIGHUP,Signal_Handler); /* hangup signal */
  signal(SIGTERM,Signal_Handler); /* software termination signal from kill */
  signal(SIGINT,Signal_Handler); /* software termination signal CTRL+C */
  signal(SIGPIPE,Signal_Handler); /* software termination signal CTRL+C */

	/*
	 * Test if basedir is present
	 */
	rc = rsstinitrsstorrent(); 
	if(rc != 0) {
    fprintf(stderr, "Initializing basedir : \'%s\' failed", RSS_BASEDIR);
    exit(1);
  }

  /*
   * Initialize the database
   */
  rc = rsstinitdatabase( DBFILE, &db);  
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "Initializing db : \'%s\' failed\n", argv[1]);
    exit(1);
  }
  cleandb=db;

	/*
	 * Create handle from db pointer.
	 * REMOVE IN THE FUTURE
	 */
	handle.db = db;

  /*
   * Initialize lib curl
   */
  curl_global_init(CURL_GLOBAL_ALL);

  /*
   * open logfile
   */
  rc = rsstinitlogdb(db);
  if(rc != 0) {
    fprintf(stderr, "Can't open logfile!\n");
    exit(1);
  }
  rsstwritelog(LOG_DEBUG, "Start rss-torrent");

	/*
	 * Handle commandline options
	 */
	rssthandleopts(&handle, argc, argv);

  /*
   * Cleanup the rest of the libaries
   */
  cleanup();

  return 0;
}

