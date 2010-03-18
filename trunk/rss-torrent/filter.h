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
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printfilters(sqlite3 *db);


/*
 * Del all filters.
 * Deletes all filters from filtertable.
 * @returns
 * On succes 0 is returned.
 */
int delallfilters(sqlite3 *db);

/*
 * del source item
 * When allready existing -1 is returned.
 * @arguments
 * name the name of the SQL filter to delete
 * @return
 * On succes 0 is returned.
 */
int delfilter(sqlite3 *db, const char *name);


/*
 * Add source item
 * @arguments
 * name name of the filter
 * filter SQL query to sort out the candidates
 * doublefilter SQL query to root out the candidates that are already downloaded.
 * @return
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addfilter(sqlite3 *db, const char *name, const char *filter, const char *doublefilter);


/*
 * Print filter in a way it could be modified and reentered
 * @arguments
 * appname the name of the rsstorrent executable
 * filtername the filtername to print
 */
void printshellfilter(sqlite3 *db, char *appname, char *filtername);

