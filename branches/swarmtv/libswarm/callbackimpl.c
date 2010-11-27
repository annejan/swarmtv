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
#include <string.h>
#include <time.h>

#include "swarm.h"
#include "callback.h"


/*
 * Add a routine that is executed on a RSS download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddownrsscallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.downloadrss;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * execute routines that are handling on RSS download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecdownrsscallbacks(rsstor_handle *handle, struct_download *rssdownmsg)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.downloadrss;

	/*
	 * call the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, rssdownmsg);

	/*
	 * Return the execute value
	 */
	return rc;
}

/*
 * Add a routine that is executed on a torrent download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddowntorcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.downloadtorrent;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * execute routines that are handling on torrent download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecdowntordownrsscallbacks(rsstor_handle *handle, struct_download *rssdownmsg)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.downloadtorrent;

	/*
	 * call the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, rssdownmsg);

	/*
	 * Return the execute value
	 */
	return rc;
}


/*
 * Add a routine that is executed on start of update event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddstartupcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.startupdate;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * execute routines that are handling on torrent download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecstartupcallbacks(rsstor_handle *handle)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.startupdate;

	/*
	 * call the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, handle);

	/*
	 * Return the execute value
	 */
	return rc;
}


/*
 * Add a routine that handles start of update cycle
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddendupcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.endupdate;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * execute routines that handle end of update cycle
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * time 			time in seconds until next update
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecendupcallbacks(rsstor_handle *handle, int time)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.endupdate;

	/*
	 * call the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, &time);

	/*
	 * Return the execute value
	 */
	return rc;
}


/*
 * Call the logfile register routine
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddlogmsgcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.logmessage;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * Call the logfile callback exe
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * logmsg			structure holding the logmsg
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexeclogmsgcallbacks(rsstor_handle *handle, struct_logmsg *logmsg)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.logmessage;

	/*
	 * call the the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, logmsg);

	/*
	 * Return the execute value
	 */
	return rc;
}


/*
 * Call the RSS download register routine
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddrssdownloadcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.rssdownload;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * Call the RSS download callback exe
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * logmsg			structure holding the logmsg
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecrssdownloadcallbacks(rsstor_handle *handle)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.rssdownload;

	/*
	 * call the the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, handle);

	/*
	 * Return the execute value
	 */
	return rc;
}


/*
 * Call the apply filters register routine
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddapplyfilterscallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.applyfilters;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * Call the apply filters callback exe
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * logmsg			structure holding the logmsg
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecapplyfilterscallbacks(rsstor_handle *handle)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.applyfilters;

	/*
	 * call the the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, handle);

	/*
	 * Return the execute value
	 */
	return rc;
}

/*
 * Call the wrap up register routine
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddwrapupcallback(rsstor_handle *handle, rsstcallbackfnct callback)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.wrapup;

	/*
	 * Register the callback.
	 */
	rc = rsstaddcallback(callstruct, callback, handle);

	/*
	 * Return the value returned by add callback
	 */
	return rc;
}


/*
 * Call the wrapup callback exe
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * logmsg			structure holding the logmsg
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecwrapupcallbacks(rsstor_handle *handle)
{
	int rc=0;
	struct_callback *callstruct=NULL;

	/*
	 * Get callback structure pointer.
	 */
	callstruct = handle->callback.wrapup;

	/*
	 * call the the callbacks.
	 */
	rc = rsstexecallbacks(callstruct, handle);

	/*
	 * Return the execute value
	 */
	return rc;
}

