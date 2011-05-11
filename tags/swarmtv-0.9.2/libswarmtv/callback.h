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
 * Initialize the struct_callback.
 * @Arguments
 * callstruct pointer to structure to initialize
 * @return
 * 0 on success, -1 when initialisation fails
 */
int rsstinitcallback(struct_callback **callstruct);

/*
 * Free the callback structure
 * @Arguments
 * callstruct callback structure to free
 */
void rsstfreecallback(struct_callback *callstruct);


/*
 * Add pointer.
 * @arguments
 * handle swarmtv handle
 * enumcall the name of the callback funct
 * callback pointer to the callback function
 * data   pointer that will be returned as the data pointer.
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddcallback(rsstor_handle *handle, enum_callbacks enumcall, rsstcallbackfnct callback, void *data);


/*
 * execute routines that are handling on RSS download events
 * @Arguments
 * handle     Handle to RSS-torrent pointer
 * callenum   Name name of the callback to call
 * load       payload to provide together with the callback
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecallbacks(rsstor_handle *handle, enum_callbacks callenum, void *load);

