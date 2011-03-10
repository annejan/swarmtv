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

#ifdef __MINGW32__
#include "../libswarmtv/types.h"
#else
#include "types.h".
#endif
#include "database.h"
#include "logfile.h"
#include "handleopts.h"
#include "setup.h"

/*
 * Bit of a hack.. But needed for cleanup
 */
static rsstor_handle *cleanhandle = NULL;


/*
 * Handle standard signals
 */
void Signal_Handler(int sig) /* signal handler function */
{
  switch(sig){
#ifndef __MINGW32__
    case SIGHUP:
    case SIGPIPE:
      /* Do nothing */
      break;    
#endif
    case SIGINT:
    case SIGTERM:
      /* finalize the server */
      printf("Cleaning up\n");
      freersstor(cleanhandle);

      exit(0);
      break;    
  } 
}

/*
 * Temporary main function.
 */
int main(int argc, char **argv){
	rsstor_handle *handle=NULL;
  //int         	 rc=0;

	/*
	 * Handle signals the nice way.
	 */
#ifndef __MINGW32__
  signal(SIGHUP,Signal_Handler); /* hangup signal */
  signal(SIGPIPE,Signal_Handler); /* software termination signal CTRL+C */
#endif
  signal(SIGTERM,Signal_Handler); /* software termination signal from kill */
  signal(SIGINT,Signal_Handler); /* software termination signal CTRL+C */

	/*
	 * Init RSS tor
	 */
	handle = initrsstor();

  /*
   * Could not think of a better way to pass this pointer to the signal handler routines.
   */
  cleanhandle = handle;

	if(handle == NULL) {
		/*
		 * When handle could not be initialized, exit here
		 */
		fprintf(stderr, "Initiazation failed, exiting !\n");
	} else {

		/*
		 * Handle command line options
		 */
		rssthandleopts(handle, argc, argv);
	}

	/*
	 * Cleanup the rest of the libraries
	 */
  freersstor(handle);

	return 0;
}

