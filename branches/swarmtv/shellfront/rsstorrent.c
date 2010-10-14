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
 * Bit of a hack.. But needed for cleanup
 */
static rsstor_handle *cleanhandle = NULL;


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
      freersstor(cleanhandle);

      exit(0);
      break;    
  } 
}

#if 0
/*
 * @@Debug
 */
static int rssfcallbackfnct(void *data, void *calldata)
{
	/*
	 * Print a silly message
	 */
	printf("Callback for starting RSS update cycle called.\n");

	return 0;
}

/*
 * @@Debug
 */
static int rssfcallbackendfnct(void *data, void *calldata)
{
	int *timew = (int*)calldata;
	/*
	 * Print a silly message
	 */
	printf("Callback for ending RSS update cycle called, time wait '%d'.\n", *timew);

	return 0;
}

/*
 * @@Debug
 */
static int rssfcallbackrssfnct(void *data, void *calldata)
{
	/*
	 * Print a silly message
	 */
	printf("Callback RSS is downloaded.\n");

	return 0;
}
#endif 


/*
 * Temporary main function.
 */
int main(int argc, char **argv){
	rsstor_handle *handle=NULL;
  //int         	 rc=0;

	/*
	 * Handle signals the nice way.
	 */
  signal(SIGHUP,Signal_Handler); /* hangup signal */
  signal(SIGTERM,Signal_Handler); /* software termination signal from kill */
  signal(SIGINT,Signal_Handler); /* software termination signal CTRL+C */
  signal(SIGPIPE,Signal_Handler); /* software termination signal CTRL+C */

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
#if 0
		/*
		 * Register a test callback
		 */
		rsstaddstartupcallback(handle, rssfcallbackfnct);
		rsstaddendupcallback(handle, rssfcallbackendfnct);
		rsstadddownrsscallback(handle, rssfcallbackrssfnct);
#endif

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

