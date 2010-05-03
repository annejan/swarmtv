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
#include <sqlite3.h>

#ifndef RSSTOR
#define RSSTOR

/*
 * Enums used by RSS-torrent
 */

/*
 * Loopmode 
 */
typedef enum {loop=0, once} LOOPMODE;

/*
 * Structures containing data from rsstorrent database.
 */

/*
 * Rsstorrent handle.
 * This handle will change when other database types are implemented, 
 * so do not reference to components in this struct.
 */
typedef struct {
	sqlite3 *db;			 // RSS-torrent database handle.
	int			 lockfile; // Lock file handle.
} rsstor_handle;

/*
 * Struct holding the values to add to the database
 */
typedef struct {
	int   id;				// Id of the filter
	char *name;			// Simple filter name
	char *title;		// Simple title regexp
	char *exclude;	// Simple exclude regexp
  char *category; // Simple category
	char *source;		// Source the newtorrent originated from
	double maxsize;	// Simple max size
	double minsize;	// Simple minimal size
	char *nodup;	// Simple no double filter type
	int fromseason;		// From what season to download
	int fromepisode;	// From episode
} simplefilter_struct;

/*
 * Simple filters container
 */
typedef struct {
	int nr;
	simplefilter_struct *simplefilter;
} simplefilter_container;

/*
 * Filter container
 */
typedef struct {
	int 	id;				// Id of the filter
	char *name;			// Name of the filter
	char *filter;		// SQL of the filter
	char *nodup;		// SQL of the avoiding duplicates filter
} filter_struct;

/*
 * Simple filters container
 */
typedef struct {
	int nr;
	filter_struct *filter;
} filter_container;

/*
 * Struct holding the values to enter into the newtorrents table.
 */
typedef struct {
	int   id;
	char *title;
	char *link;
	time_t pubdate;
	char *category;
	char *source;
	int		season;
	int		episode;
	int		seeds;
	int		peers;
  size_t	size;
	// 'new' is set by the routine. 
} newtorrents_struct;

/*
 * Simple filters container
 */
typedef struct {
	int nr;
	newtorrents_struct *newtorrent;
} newtorrents_container;

/*
 * Struct holding the values to enter into the downloaded table
 */
typedef struct {
	int   id;
	char *title;
	char *link;
	char *pubdate;
	char *category;
	int  season;
	int  episode;
} downloaded_struct;

/*
 * Simple filters container
 */
typedef struct {
	int nr;
	downloaded_struct *downloaded;
} downloaded_container;

/*
 * Config record container
 */
typedef struct {
	int 	id;
	char *name;
	char *value;
	char *description;
} config_struct;

/*
 * Config record container
 */
typedef struct {
	int            nr;
	config_struct  *config;
} config_container;

/*
 * source
 */
typedef struct {
	int   id;
	char *name;
	char *url;
	char *parser;
} source_struct;

/*
 * Sources container
 */
typedef struct {
	int            nr;
	source_struct *source;
} source_container;

/*
 * == Functions to manipulate the config settings
 */

/*
 * Initialize RSS-torrent handle
 * @Return
 * Pointer to handle on success, NULL on failure
 */
rsstor_handle *initrsstor();

/*
 * Free RSS-torrent handle
 * @Arguments
 * handle pointer to RSS-torrent structure
 */
void freersstor(rsstor_handle *handle);

/*
 * Lock the rsstorrent lockfile
 * This routine gets the path of the lockfile from the config settings.
 * @Arguments
 * handle RSS-torrent handle
 */
void rsstlock(rsstor_handle *handle);

/*
 * Free lockfile
 * This routine frees up the lockfile and leaves the database for other instances of RSS-torrent
 * @Arguments 
 * handle RSS-torrent handle
 * @Return
 * 0 on success otherwise -1
 */
int rsstunlock(rsstor_handle *handle);

/*
 * Get all config settings
 * @Arguments
 * handle RSS-torrent handle
 * configitems The container to store the configitems in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallconfig(rsstor_handle *handle, config_container **configitems);

/*
 * Delete content from confeg_container struct
 * @Arguments
 * container Pointer to configcontainer to free content of
 * @Return
 * 0 on success, -1 on failure
 */
int rsstfreeconfigcontainer(config_container *container);

/*
 * Set config item
 * @arguments
 * handle RSS-torrent handle
 * prop name of the propertie to change
 * value new value to enter
 * @returns
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstsetconfigitem(rsstor_handle *handle, const char *prop, const char *value);

/*
 * == Functions to manipulate sources
 */

/*
 * Get all RSS-sources
 * @arguments
 * handle RSS-torrent handle
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
 * Add source item
 * @arguments
 * handle RSS-torrent handle
 * name filtername
 * url source url
 * parsertype parser type
 * @return
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstaddsource(rsstor_handle *handle, const char *name, const char *url, char *parsertype);

/*
 * del source item
 * @arguments
 * handle RSS-torrent handle
 * name sourcename to delete
 * @return
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstdelsource(rsstor_handle *handle, const char *name);

/*
 * == Functions to manipulate downloaded database
 */

/*
 * Get downloaded torrents
 * @arguments
 * handle RSS-torrent handle
 * downloaded The container to store the downloaded in
 * limit amount of records to retrieve
 * offset number of records to skip at beginning
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
 * Delete from downloaded table
 * @arguments
 * handle RSS-torrent handle
 * id id of the downloaded torrent to delete from downed table
 * @returns
 * 0 	On success
 * -1 on failure
 */
int rsstdeldownloaded(rsstor_handle *handle, int id);

/*
 * == Functions to manipulate newtorrents
 */

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
 * This routine function downloads the torrent indicated by ID.
 * The routine looks throught the newtorrents table to pick the torrent by id.
 * @arguments
 * handle RSS-torrent handle
 * id	The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyid(rsstor_handle *handle, int torid);

/*
 * Functions to manipulate the SQL filters
 */

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
 * Delete a filter
 * TO BE DONE
 */

/*
 * Add a filter
 * TO BE DONE
 */

/*
 * == Functions to run RSS-torrent
 */

/*
 * Main loop, dispatches tasks
 * @arguments
 * handle RSS-torrent handle
 * onetime Set loop for infinate loop, once for running the step just once
 * @return
 * 0 for now.
 */
int rsstrunloop(rsstor_handle *handle, LOOPMODE onetime);


#endif
