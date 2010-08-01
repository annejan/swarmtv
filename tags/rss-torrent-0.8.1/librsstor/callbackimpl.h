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

/*
 * Add a routine that is executed on a RSS download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddownrsscallback(rsstor_handle *handle, rsstcallbackfnct callback);


/*
 * execute routines that are handling on RSS download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecdownrsscallbacks(rsstor_handle *handle, struct_download *rssdownmsg);


/*
 * Add a routine that is executed on a torrent download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddowntorcallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * execute routines that are handling on torrent download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecdowntordownrsscallbacks(rsstor_handle *handle, struct_download *rssdownmsg);

/*
 * Add a routine that is executed on start of update event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddstartupcallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * execute a routine at the start of a RSS-torrent pointer
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecstartupcallbacks(rsstor_handle *handle);


/*
 * Add a routine that handles start of update cycle
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddendupcallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * execute routines that handle end of update cycle
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * time 			time in seconds until next update
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecendupcallbacks(rsstor_handle *handle, int time);

/*
 * Add a routine that is executed on a RSS download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddownrsscallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * execute routines that are handling on RSS download events
 * @Arguments
 * handle			handle to RSS-torrent pointer
 * rssdownmsg	structure holding the download-information
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecdownrssdownrsscallbacks(rsstor_handle *handle, struct_download *rssdownmsg);

#if 0
/*
 * Call the logfile register routine
 * @arguments
 * callstruct Structure to add new pointer to
 * handle			handle to RSS-torrent pointer
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddlogmsgcallback(rsstor_handle *handle, rsstcallbackfnct callback, void *data);


/*
 * Call the logfile callback exe
 * @Arguments
 * callstruct
 * @return
 *
 */
int rsstexeclogmsgcallbacks(rsstor_handle *handle, struct_logmsg *logmsg);
	
#endif

