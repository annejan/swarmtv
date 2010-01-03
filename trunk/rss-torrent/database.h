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
 * path to database file.
 */
#define  DBFILE "./rss.db"

/*
 * Open database, and add regexp functionality.
 */
int initdatabase(
    const char *filename,   /* Database filename (UTF-8) */
    sqlite3 **ppDb          /* OUT: SQLite db handle */
    ); 

/*
 * Do a query, and bind value's to the query in one step
 * This is a simplified version as it only returns 1 value.
 * that value is the first string of the first colomn.
 * the value returned must be a TEXT value.
 * the returned value will be put into text
 * make sure to free text after use
 */
int dosingletextquery(sqlite3 *db, const char *query, unsigned char const **text);

/*
 * Execute a query
 * Don't expect any output back
 */
int executequery(sqlite3 *db, const char *query);

/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 */
int printquery(sqlite3 *db, const char *query);

/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * return 0 when okay
 * return -1 on error
 */
int printquery(sqlite3 *db, const char *query);

