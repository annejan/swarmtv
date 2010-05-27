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
 * @arguments
 * newtor newtor structure pointer
 */
void rsstfreenewtor(newtorrents_struct *newtor);

/*
 * Add a torrent to the newtorrent table.
 * @arguments
 * newtor structure holding the values for the record to be added
 * @returns
 * 0 on succes, exits on -1 on failure
 */
int rsstaddnewtorrent(sqlite3 *db, newtorrents_struct *newtor);

/*
 * Add a torrent to the downloaded table.
 * @arguments
 * downed			pointer to struct holding values to add to the db.
 * simulate		0 to log addition, 1 adds anyway, but does not log at all. (used for filtertest)
 */
void rsstadddownloaded(sqlite3 *db, downloaded_struct *downed, SIM  simulate);

/*
 * Delete from downloaded table
 * @arguments
 * handle rsstor handle 
 * id 	id of the downloaded torrent to delete from downed table
 * @returns
 * 0 	On success
 * -1 on failure
 */
int rsstdeldownloaded(rsstor_handle *handle, int id);

/*
 * When Torrents are prosessed, they are no longer new
 * this method removes the new flag
 */
void rsstnonewtorrents(sqlite3 *db);

/*
 * Delete all newtorrents entris older the x days
 * This function returns 0 on succes, -1 SQL on failure.
 * @arguments
 * days max age a newtorrent may become before deletion
 * @return
 * returns 0 on succes, -1 on error.
 */
int rsstdeloldnewtorents(sqlite3 *db, unsigned int days);

