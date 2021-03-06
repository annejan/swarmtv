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
#define RSST_DB_VERSION 4

/*
 * path to database file.
 */
#define  RSST_DBFILE "~/.swarmtv/swarm.db"

/*
 * Run the Database init script.
 * @return
 * 0 on succes, -1 on failure
 */
int rsstrundbinitscript(rsstor_handle *handle);

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
 * Get downloaded torrents
 * @arguments
 * downloaded The container to store the downloaded in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetdownloaded(rsstor_handle *handle, downloaded_container **downloaded, int limit, int offset);

/*
 * Free downloaded structure
 * @Arguments
 * downloaded pointer to downloaded struct to be freed
 */
void rsstfreedownloaded(downloaded_struct *downloaded);

/*
 * Free source structure
 * @Arguments
 * source pointer to source struct to be freed
 */
void rsstfreesource(source_struct *source);

/*
 * Delete content from source_container struct
 * @Arguments
 * container downloaded container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreedownloadedcontainer(downloaded_container *container);

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
 * name name of the simple filter
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetsimplefiltername(rsstor_handle *handle, simplefilter_container **simplefilter, char *name);

/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * id id of the simple filter
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetsimplefilterid(rsstor_handle *handle, simplefilter_struct **simplefilter, int id);

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
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
int rsststorenewtorrentcontainer(sqlite3_stmt *result, newtorrents_container *container);

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
 * Get newtorrent information providing its ID
 * @Arguments
 * handle Swarmtv Handle
 * newtorid id number of torrent to retrieve
 * newtorrent structure holding information
 * @Return
 * Returns 0 when found, -1 on not found or error
 */
int rsstnewtorrentsbyid(rsstor_handle *handle, int newtorid, newtorrents_struct *newtorrent);

/*
 * Find newtorrents entries
 * @Arguments
 * handle SwarmTv Handle
 * title title to match to
 * newtorrents container handling newtorrents entries
 * limit is the amount of rows we want to retrieve
 * offset is the amount of rows we want to skip at the start
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfindnewtorrentsbytitle(rsstor_handle *handle, char *title, newtorrents_container **newtorrents, int limit, int offset);

/*
 * Delete content from newtorrents_container
 * @Arguments
 * container newtorrents container the content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreenewtorrentscontainer(newtorrents_container *newtorrents);

