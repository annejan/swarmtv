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

#ifdef RSST_DBUS_GLIB_ENABLE

/*
 * The DBUS_GLIB includes
 */
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <swarm.h>

#endif

/*
 * The none DBUS glib mainloop
 */
#ifndef RSST_DBUS_GLIB_ENABLE

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

#endif

/*
 * The DBUS supporting main loop
 */
#ifdef RSST_DBUS_GLIB_ENABLE

/*
 * Runcycle data struct
 */
typedef struct {
  rsstor_handle *handle;
  GMainLoop     *loop;
  int            onetime;
  int            timewait;
} runcycledata;

/*
 * Glib callback routine to call when the timer triggers
 */
static gboolean rssfglibcycle(runcycledata *data)
{
  int            rc=0;
  int            onetime=0;
  rsstor_handle *handle=NULL;
  GMainLoop     *loop=NULL;
  time_t         after=0;
  time_t         before=0;
  time_t         timeleft=0;
  time_t         runtime=0;
  int            timewait=0;

  /*
   * Get time beforehand
   */
  before = time(NULL);

  /*
   * Get variables from data structure
   */
  handle=data->handle;
  onetime=data->onetime;
  timewait=data->timewait;
  loop=data->loop;

  /*
   * Store loop pointer into handle data pointer
   */
  handle->data=loop;

  /*
   * Call the routine to do the work
   */
  rc = runcycle(handle);

  /*
   * Calculate sleep time left.
   */
  after = time(NULL);
  runtime = (after - before);
  timeleft = timewait - runtime;
  if(timeleft < 0) {
    timeleft = timewait;
  }

  /*
   * Handle run one time
   */
  if(onetime != 0) {
    /* 
     * Tell the main loop to quit 
     */
    rsstwritelog(LOG_NORMAL, "Single run done in %d seconds.", runtime);
    g_main_loop_quit (loop);
  } else {
    /*
     * Reschedule for a new cycle
     */
    rsstwritelog(LOG_NORMAL, "Sleeping %d seconds.", timeleft);
    g_timeout_add_seconds(timeleft, (GSourceFunc)rssfglibcycle, data);
  }
  return FALSE;
}


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
  int             rc=0;
  GMainLoop      *loop=NULL;
  DBusConnection *bus=NULL;
  DBusError       error;
  int             timewait=0;
  runcycledata    data;

  /*
   * Get the interval to run
   */
  rc = rsstconfiggetint(handle, CONF_REFRESH, &timewait);
  if(onetime == 0) {
    rsstwritelog(LOG_NORMAL, "Starting daemon, refresh %ds", timewait);
  } else {
    rsstwritelog(LOG_NORMAL, "Running once.");
  }

  /*
   * initialize the mainloop
   */
  loop = g_main_loop_new (NULL, FALSE);

  /*
   * Fill runcycledata structure
   */
  data.handle=handle;
  data.onetime=onetime;
  data.timewait=timewait;
  data.loop=loop;

  dbus_error_init (&error);
  bus = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {
    g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
    dbus_error_free (&error);
    return 1;
  }
  dbus_connection_setup_with_g_main (bus, NULL);

#if 0
  /* listening to messages from all objects as no path is specified */
  dbus_bus_add_match (bus, PING_MATCH_RULE, &error);
  dbus_connection_add_filter (bus, signal_filter, loop, NULL);
#endif

  /* 
   * Call the runcycle 
   */
  g_timeout_add_seconds(1, (GSourceFunc)rssfglibcycle, &data);

  /*
   * Run main loop
   */
  g_main_loop_run (loop);

  /*
   * Free main loop
   */
  dbus_connection_unref(bus);
  g_main_loop_unref(loop);

  /*
   * Done.
   */
  return 0;
}

#endif
