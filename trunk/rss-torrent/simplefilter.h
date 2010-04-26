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
 * Add simple filter
 * @variables
 * opts structure holding the options
 * @returns 
 * 0 on succes, else -1
 */
int rsstaddsimplefilter(rsstor_handle *handle, opts_struct *opts);

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 */
void rsstlistsimple(rsstor_handle *handle);

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

/*
 *  * del filter item
 *   * When the name is not found -1 is returned.
 *    * On succes 0 is returned.
 *     */
int rsstdelallsimple(rsstor_handle *handle);

/*
 * del filter item
 * When allready existing -1 is returned.
 * @variables
 * name filtername to delete
 * @returns
 * On succes 0 is returned.
 */
int rsstdelsimple(rsstor_handle *handle, const char *name);

/*
 * Apply filters
 * Runs through all filters in simplefilters table.
 * Calls SQL filters routines for further handling.
 * @variables 
 * simultate 0 for real behaviour, 1 for simulation mode.
 * @return
 * -1 on error, 0 on success
 */
int rsstdownloadsimple(sqlite3 *db, SIM simulate);

/*
 * Add simple filter adds the filter to the database
 * Arguments  : simplefilter_struct * 
 * returns    : 0 when added succesfully
 * returns    : -1 when adding failed
 */
int rsstinsertsimplefilter(rsstor_handle *handle, simplefilter_struct *simple);

/*
 * optstosimple
 * Takes takes a opts_struct argument and a simplefilter_struct as argument.
 * @Arguments
 * opts the opts structure to retrieve the arguments from.
 * simple the simple filter struct to store the filters settings in.
 * @Return
 * returns 0 on succes, -1 on error.
 */
int rsstoptstosimple(opts_struct *opts, simplefilter_struct *simple);

