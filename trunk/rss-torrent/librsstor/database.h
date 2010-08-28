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
 * Database version.
 * When the current version does not match the version in the version field
 * The database is recreated
 */
#define RSST_DB_VERSION 2

/*
 * Number of elements to allocate initially for returning results
 */
#define RSST_START_ELEMENTS 10

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
 * Execute a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * @arguments
 * ppstmt pointer carying the results of the query
 * query pointer to query string:
 * fmt format string
 * ... arguments to fill out in query
 * @returns
 * returns 1 on 1 row returned
 * return 0 on no rows returned
 * returns -1 on error
 */
int rsstexecqueryresult(sqlite3 *db, sqlite3_stmt **ppstmt, const char *query, char *fmt, ...);

/*
 * Execute a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * @arguments
 * ppstmt pointer carying the results of the query
 * query pointer to query string:
 * fmt format string
 * ap va_list argument list
 * @returns
 * returns 1 on 1 row returned
 * return 0 on no rows returned
 * returns -1 on error
 */
int rsstexecqueryresultva(sqlite3 *db, sqlite3_stmt **ppstmt, const char *query, char *fmt, va_list ap);

/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * @Arguments
 * query the query that retrieves the values to print
 * fmt format string that goes with the query
 * ... Arguments
 * @Returns
 * return 0 when okay
 * return -1 on error
 */
int rsstprintquery(sqlite3 *db, const char *query, char *fmt, ...);


/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * Arguments
 * query	Query to print.
 * names	The names that should be printed infront of values.
 * fmt		Format string describing the. 
 * Return
 * return 0 when okay.
 * return -1 on error.
 */
int rsstprintquerylist(sqlite3 *db, const char *query, char *names[], char *fmt, ...);

/*
 * Executing script
 * This function executes a script.
 * Each line should be separated by a '\n' char.
 * @Arguments 
 * script pointer so buffer holding script.
 * @ returns
 * 0 on success
 * -1 on fail
 */
int dbexecscript(sqlite3 *db, const char *script);

