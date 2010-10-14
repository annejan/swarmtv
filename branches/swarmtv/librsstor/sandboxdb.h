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
 * Create copy of database
 * and create a sqlite3 pointer to the database copy.
 * @arguments
 * sourcedbname source path to database
 * sandboxdbname detination path to database
 * @return
 * pointer not sandbox database
 */
sandboxdb *rsstcreatesandbox(char *sourcedbname, char *sandboxdbname);

/*
 * Close the database.
 * delete the sandboxed database.
 * @arguments
 * db pointer to sandbox db
 * @returns
 * 0 on succes otherwise -1 
 */
int rsstclosesandbox(sandboxdb *db);

