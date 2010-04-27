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

/*
 * Print version 
 */
void rsstprintversion(void);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintconfigitems(rsstor_handle *handle); 

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintfilters(rsstor_handle *handle);

/*
 * Print filter in a way it could be modified and reentered
 * @arguments
 * appname the name of the rsstorrent executable
 * filtername the filtername to print
 */
void rsstprintshellfilter(rsstor_handle *handle, char *appname, char *filtername);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintsources(rsstor_handle *handle); 

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 * @variables
 * filtername name to print
 */
void rsstprintsimple(rsstor_handle *handle, char *filtername);

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 */
void rsstprintallsimple(rsstor_handle *handle);

