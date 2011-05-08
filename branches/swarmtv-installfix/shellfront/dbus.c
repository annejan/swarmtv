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
 *  Program written by Paul Honig 2009 - 2010
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>

/*
 * The DBUS_GLIB includes
 */
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

/*
 * LibXML2
 */
#include <libxml/parser.h>

#include <swarmtv.h>

#include "runloop.h"
#include "dbus.h"
#include "xmlencode.h"

#ifdef RSST_ESMTP_ENABLE
#include "mailmsg.h"
#endif

//path  the path to the object emitting the signal
#define RSSFDBUSPATH "/"
//interface   the interface the signal is emitted from
#define RSSFDBUSINTF "nl.swarmtv.dbus"
//name  name of the signal 
//#define RSSFDBUSNAME "event"
#define MAXSIZE 30


/*
 * Send A message via dbus
 */
static int rssfsenddbusmsg(DBusConnection *bus, char *name, char *msg)
{
  DBusMessage *message = NULL;

  /*
   * When not initialized do nothing.
   */
  if(bus == NULL){
    return 0;
  }

  /* Create a new signal "Ping" on the "com.burtonini.dbus.Signal" interface,
   *    * from the object "/com/burtonini/dbus/ping". */
  message = dbus_message_new_signal (RSSFDBUSPATH, RSSFDBUSINTF, name);
  /* Append the message to the signal */
  dbus_message_append_args (message,
      DBUS_TYPE_STRING, &msg,
      DBUS_TYPE_INVALID);
  /* Send the signal */
  dbus_connection_send (bus, message, NULL);
  /* Free the signal now we have finished with it */
  dbus_message_unref (message);
  /* Make sure message is sent. */
  //dbus_connection_flush (bus);
  /* Return TRUE to tell the event loop we want to be called again */
  return 0;
}


/*
 * @@Debug
 */
static int rssfcallbackstartfnct(void *data, void *calldata)
{
  rsstor_handle   *handle=NULL;
  DBusConnection  *bus=NULL;
  runcycledata    *rundata=NULL;

  /*
   * To shut the compiler up
   */
  calldata=NULL;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;

	/*
	 * Call the dbus method to send the message through.
	 */
  rssfsenddbusmsg(bus, "start", "");

	/*
	 * Print a silly message
	 */
	//printf("Callback start.\n");

	return 0;
}

/*
 * @@Debug
 */
static int rssfcallbackendfnct(void *data, void *calldata)
{
  rsstor_handle   *handle=NULL;
  DBusConnection  *bus=NULL;
  runcycledata    *rundata=NULL;

  /*
   * To shut the compiler up
   */
  calldata=NULL;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;

	/*
	 * Call the dbus method to send the message through.
	 */
  rssfsenddbusmsg(bus, "end", "");

	/*
	 * Print a silly message
	 */
	//printf("Callback for ending RSS update cycle called.\n");

	return 0;
}

/*
 * @@Debug
 */
static int rssfcallbackrssfnct(void *data, void *calldata)
{
  rsstor_handle   *handle=NULL;
  DBusConnection  *bus=NULL;
  runcycledata    *rundata=NULL;
  struct_download *down=NULL;
  xmlChar         *xmlbuf=NULL;
  int              buffersize=0;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;
  down=(struct_download*) calldata;


	/*
	 * Call the dbus method to send the message through.
	 */
  rssfrsstoxml(down, &xmlbuf, &buffersize);
  rssfsenddbusmsg(bus, "rss", (char*) xmlbuf);
  rssffreexmldoc(xmlbuf);

	return 0;
}


/*
 * @@Debug
 */
static int rssfcallbacksimplefnct(void *data, void *calldata)
{
  rsstor_handle   *handle=NULL;
  DBusConnection  *bus=NULL;
  runcycledata    *rundata=NULL;
  simplefilter_struct *simple=NULL;
  xmlChar         *xmlbuf=NULL;
  int              buffersize=0;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;
  simple=(simplefilter_struct*) calldata;


	/*
	 * Call the dbus method to send the message through.
	 */
  rssfsimpletoxml(simple, &xmlbuf, &buffersize);
  rssfsenddbusmsg(bus, "simple", (char*) xmlbuf);
  rssffreexmldoc(xmlbuf);

	return 0;
}


/*
 * @@Debug
 */
static int rssfcallbacksqlfnct(void *data, void *calldata)
{
  rsstor_handle   *handle=NULL;
  DBusConnection  *bus=NULL;
  runcycledata    *rundata=NULL;
  filter_struct   *sql=NULL;
  xmlChar         *xmlbuf=NULL;
  int              buffersize=0;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;
  sql=(filter_struct*) calldata;

	/*
	 * Call the dbus method to send the message through.
	 */
  rssfsqltoxml(sql, &xmlbuf, &buffersize);
  rssfsenddbusmsg(bus, "sql", (char*) xmlbuf);
  rssffreexmldoc(xmlbuf);

	return 0;
}


/*
 * @@Debug
 */
static int rssfcallbackdownedfnct(void *data, void *calldata)
{
  rsstor_handle     *handle=NULL;
  DBusConnection    *bus=NULL;
  runcycledata      *rundata=NULL;
  downloaded_struct *down=NULL;
  xmlChar           *xmlbuf=NULL;
  int                buffersize=0;

  /*
   * Retrieve pointers
   */
  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;
  down=(downloaded_struct*) calldata;

#ifdef RSST_ESMTP_ENABLE
  /*
   * Send email when enabled
   */
  rsstsendemail(handle, down);
#endif

	/*
	 * Call the dbus method to send the message through.
	 */
  rssfdownedtoxml(down, &xmlbuf, &buffersize);
  rssfsenddbusmsg(bus, "downed", (char*) xmlbuf);
  rssffreexmldoc(xmlbuf);

	return 0;
}

/*
 * Executed when the download partition is full 
 */
static int rssfcallbackfullfnct(void *data, void *calldata)
{
  rsstor_handle     *handle=NULL;
  DBusConnection    *bus=NULL;
  runcycledata      *rundata=NULL;
  struct_diskusage  *usage=NULL;
  xmlChar           *xmlbuf=NULL;
  int                buffersize=0;

  handle=(rsstor_handle*) data;
  rundata=(runcycledata*) handle->data;
  bus=rundata->bus;
  usage=(struct_diskusage*) calldata;

	/*
	 * Call the dbus method to send the message through.
	 */
  rssfusagetoxml(usage, &xmlbuf, &buffersize);
  rssfsenddbusmsg(bus, "diskfull", (char*) xmlbuf);
  rssffreexmldoc(xmlbuf);

	return 0;
}

/*
 * Connect swarmtv events to dbus emits
 * @arguments
 * handle swarmtv handle
 */
void rssfinitcallbacks(rsstor_handle *handle)
{
		/*
		 * Register a test callback
		 */
    rsstaddcallback(handle, startcycle,       rssfcallbackstartfnct,  handle);
    rsstaddcallback(handle, endcycle,         rssfcallbackendfnct,    handle);
    rsstaddcallback(handle, rssdownload,      rssfcallbackrssfnct,    handle);
    rsstaddcallback(handle, downloadtorrent,  rssfcallbackdownedfnct, handle);
    rsstaddcallback(handle, applysimplefilt,  rssfcallbacksimplefnct, handle);
    rsstaddcallback(handle, applysqlfilt,     rssfcallbacksqlfnct,    handle);
    rsstaddcallback(handle, diskfull,     rssfcallbackfullfnct,    handle);
}

/*
 * Initialize the dbus connection.
 * Handle the connection of the callback routines to the dbus emitters.
 * @return
 * 0 on success, -1 on failure.
 */
int rssfdbusinit(DBusConnection **bus)
{
  DBusError       error;

  /*
   * Initialize the DBUS connection.
   */
  dbus_error_init (&error);
  *bus = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (dbus_error_is_set(&error)) {
    rsstwritelog(LOG_ERROR, "Connection Error (%s)", error.message);
    dbus_error_free(&error);
    return -1;
  }
  dbus_connection_setup_with_g_main (*bus, NULL);

  return 0;
}

/*
 * Clean up dbus instance
 * @arguments
 * bus pointer to the dbus connection we want to free
 */
void rssfdbusfree(DBusConnection *bus)
{
  if(bus != NULL) {
    dbus_connection_unref(bus);
  }
}

