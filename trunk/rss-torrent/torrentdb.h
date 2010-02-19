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
 * Free strings in newtorrents_struct 
 * Be sure to free the struct yourself.
 * Arguments 
 * newtor structure pointer
 * returns void, exits on failure
 */
void freenewtor(newtorrents_struct *newtor);

/*
 * Add a torrent to the newtorrent table.
 * Arguments
 * newtor structure holding the values for the record to be added
 * Returns
 * 0 on succes, exits on -1 on failure
 */
int addnewtorrent(sqlite3 *db, newtorrents_struct *newtor);

/*
 * Add a torrent to the downloaded table.
 * Arguments
 * downed			pointer to struct holding values to add to the db.
 * simulate		0 to log addition, 1 adds anyway, but does not log at all. (used for filtertest)
 */
void adddownloaded(sqlite3 *db, downloaded_struct *downed, SIM  simulate);

/*
 * When Torrents are prosessed, they are no longer new
 * this method removes the new flag
 */
void nonewtorrents(sqlite3 *db);

/*
 * Delete all newtorrents entris older the x days
 * This function returns 0 on succes, -1 SQL on failure.
 */
int deloldnewtorents(sqlite3 *db, unsigned int days);

