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
void rssfprintversion(void);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssfprintconfigitems(rsstor_handle *handle); 

/*
 * List all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssflistfilters(rsstor_handle *handle);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssfprintfilters(rsstor_handle *handle, char *appname);

/*
 * Print filter in a way it could be modified and reentered
 * @arguments
 * appname the name of the rssforrent executable
 * filtername the filter name to print
 */
void rssfprintshellfilter(rsstor_handle *handle, char *appname, char *filtername);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssfprintsources(rsstor_handle *handle); 

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 * @variables
 * filtername name to print
 */
void rssfprintsimple(rsstor_handle *handle, char *filtername, char *execname);

/*
 * Print a simple filter struct in shell format
 * @Arguments
 * simple pointer to simple filter struct
 * @return
 */
void printsimplestruct(char *execname, simplefilter_struct *simple);

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 * @Arguments
 * execname name of this executable
 */
void rssfprintallsimple(rsstor_handle *handle, char *execname);

/*
 * Do filter test
 * show first 10 matches
 * @arguments
 * opts Takes opts_struct * as argument.
 * @return
 * return 0 on success, return -1 on failure.
 */
int rssfdosimpletest(simplefilter_struct *filter);

/*
 * Print all simple filters in shell format.
 * @Arguments
 * handle RSS-torrent handle
 */
void rssflistallsimple(rsstor_handle *handle);

/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on success, return -1 on failure.
 */
int rssffindtorrentids(opts_struct *opts);

/*
 * Print the last downloaded content per filter
 * @Arguments
 * handle RSS-torrent handle
 */
void rsstprintlastdowned(rsstor_handle *handle);
