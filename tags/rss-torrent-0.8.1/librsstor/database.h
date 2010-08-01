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
 * Open database, and add regexp functionality.
 * @Arguments
 * filename Filename of database to open 
 * handle RSS-torrent handle
 * @returns 
 * returns SQLITE_OK when all did go well.
 */
int rsstinitdatabase(const char *filename, rsstor_handle *handle); 

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
 * Run the Database init script.
 * @return
 * 0 on succes, -1 on failure
 */
int rsstrundbinitscript(rsstor_handle *handle);

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
 * Database abstraction functions
 */

/*
 * Get all config settings
 * @Arguments
 * configitems The container to store the configitems in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallconfig(rsstor_handle *handle, config_container **configitems);

/*
 * Delete content from config_container struct
 * @Arguments
 * container Pointer to configcontainer to free content of
 * @Return
 * 0 on success, -1 on failure
 */
int rsstfreeconfigcontainer(config_container *container);


/*
 * Get downloaded torrents
 * @arguments
 * downloaded The container to store the downloaded in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetdownloaded(rsstor_handle *handle, downloaded_container **downloaded, int limit, int offset);

/*
 * Delete content from source_container struct
 * @Arguments
 * container downloaded container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreedownloadedcontainer(downloaded_container *container);

/*
 * Get all RSS-sources
 * @arguments
 * sources The container to store the sources in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallsources(rsstor_handle *handle, source_container **sources);

/*
 * Delete content from source_container struct
 * @Arguments
 * container sources container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreesourcecontainer(source_container *sources);

/*
 * Find newtorrents entries
 * @Arguments
 * filter simplefilterstruct to filter out the newtorrent entries we want
 * newtorrents container handling newtorrents entries
 * limit is the amount of rows we want to retrieve
 * offset is the amount of rows we want to skip at the start
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfindnewtorrents(simplefilter_struct *filter, newtorrents_container **newtorrents, int limit, int offset);

/*
 * Delete content from newtorrents_container
 * @Arguments
 * container newtorrents container the content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreenewtorrentscontainer(newtorrents_container *newtorrents);

/*
 * Delete content from config_container struct
 * @Arguments
 * container Pointer to configcontainer to free content of
 * @Return
 * 0 on success, -1 on failure
 */
int rsstfreefiltercontainer(filter_container *container);

/*
 * Get all SQL filter settings
 * @Arguments
 * handle RSS-torrent handle
 * container The container to store the SQL filters in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallfilter(rsstor_handle *handle, filter_container **filteritem);

/*
 * Get all SQL filter settings
 * @Arguments
 * handle RSS-torrent handle
 * container The container to store the container in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetfilterbyname(rsstor_handle *handle, char *name, filter_container **container);

/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallsimplefilter(rsstor_handle *handle, simplefilter_container **simplefilter, int limit, int offset);

/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetsimplefilter(rsstor_handle *handle, simplefilter_container **simplefilter, char *name);

/*
 * Free simplefilter structure
 * @Arguments
 * simplefilter pointer to simplefilter struct to be freeed
 */
void rsstfreesimplefilter(simplefilter_struct *simplefilter);

/*
 * Delete content from source_container struct
 * @Arguments
 * container simplefilter container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreesimplefiltercontainer(simplefilter_container *container);

