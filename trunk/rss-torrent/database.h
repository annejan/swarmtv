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
#define  DBFILE "~/.rsstorrent/rss.db"

/*
 * Return values for executequery
 * ROWS_ERROR error occured
 * ROWS_EMPTY no rows found or changed
 * ROWS_FOUND select found rows
 * ROWS_CHANGED insert or update changed rows
 */
#define ROWS_ERROR      -1
#define ROWS_EMPTY      0
#define ROWS_FOUND      1
#define ROWS_CHANGED    2  
#define ROWS_CONSTRAINT 3  


/*
 * Open database, and add regexp functionality.
 * @returns 
 * returns SQLITE_OK when all did go well.
 */
int rsstinitdatabase(
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
 * @Arguments
 * text retrieved from the query
 * query pointer to the query string
 * fmt arguments pointer
 * ... other arguments
 * @return
 * 0 on succes, -1 on failure
 */
int rsstdosingletextquery(sqlite3 *db, const unsigned char **text, const char *query, char *fmt, ...);

/*
 * Execute a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * @arguments
 * query pointer to query string
 * fmt format string
 * ... arguments to fill out in query
 * @returns
 * returns 1 on 1 row returned
 * return 0 on no rows returned
 * returns -1 on error
 */
int rsstexecutequery(sqlite3 *db, const char *query, char *fmt, ...);

/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * @Arguments
 * query the query that retrieves the values to print
 * @Returns
 * return 0 when okay
 * return -1 on error
 */
int rsstprintquery(sqlite3 *db, const char *query);

/*
 * Run the Database init script.
 * @return
 * 0 on succes, -1 on failure
 */
int rsstrundbinitscript(sqlite3 *db);

