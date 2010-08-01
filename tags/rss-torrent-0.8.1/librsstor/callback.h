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
 * callstruct Structure to add new pointer to
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddcallback(struct_callback *callstruct, rsstcallbackfnct callback, void *data);

/*
 * Call callbacks.
 * @arguments
 * callstruct	Structure of callbacks to execute one by one
 * data				Structure containing callback data (should be casted to proper structure)
 * @return
 * 0 on success, -1 when on of the called functions did not return 0
 */
int rsstexecallbacks(struct_callback *callstruct, void *data);

