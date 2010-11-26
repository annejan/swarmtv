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

#include <swarm.h>

/*
 * Main loop, dispatches tasks
 * When onetime != 0 run once then exit
 * @Argument
 * onetime when ! 0 runloop only runs one time.
 * @Return
 * 0 for now.
 */
int rssfrunloop(rsstor_handle *handle, LOOPMODE onetime)
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
  while(1 == 1){
    /*
     * Call callback to signal start of update
     */
    if(rc != 0){
      rsstwritelog(LOG_ERROR, "Error returned by 'startup' callback. %s:%d", __FILE__, __LINE__);
    }

    before = time(NULL);

    /*
     * Call the routine to do the work
     */
    rc = runcycle(handle);

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
		if(rc != 0){
			rsstwritelog(LOG_ERROR, "Error returned by 'endup' callback. %s:%d", __FILE__, __LINE__);
		}

    /*
     * Run once.
     */
    if(onetime == (LOOPMODE) once) {
      rsstwritelog(LOG_NORMAL,"Run Done.", timeleft); 
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
