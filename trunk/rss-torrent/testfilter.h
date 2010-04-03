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
 * Do filter test
 * show first 10 matches
 * @arguments
 * filter filter SQL
 * nodouble nodouble SQL
 * @return
 * -1 on error
 * 0 on success
 */
int rsstdofiltertest(char *filter, char *nodouble);

/*
 * Do filter test
 * show first 10 matches
 * @arguments
 * opts Takes opts_struct * as argument.
 * @return
 * return 0 on succes, return -1 on failure.
 */
int rsstdosimpletest(opts_struct *opts);

/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on succes, return -1 on failure.
 */
int rsstfindtorrentids(opts_struct *opts);

/*
 * This routine retrieves the records from the downloaded table.
 * A selection is based on name.
 * This routine should not be here namewise, but sinds the other function ended up here
 * I'll place it here for now.
 * @Arguments
 * db
 * optarg
 * @return
 * 0 on success
 * -1 on error
 */
int rsstfinddowned(sqlite3 *db, char *optarg);

