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
 *  Program written by Paul Honig 2009 - 2010
 */
#include <sqlite3.h>

#ifndef RSSTOR
#define RSSTOR

/*
 * Version of program
 */
#define RSST_VERSION "0.9"

/*
 * Enums used by RSS-torrent
 */

/*
 * Loopmode 
 */
typedef enum {loop=0, once} LOOPMODE;

/*
 * Return values for execute query inside RSS-torrent
 * ROWS_ERROR error occurred
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
 * The debug routine handle 3 debug levels
 * LOG_DEBUG 	for debug information
 * LOG_NORMAL for output messages
 * LOG_ERROR  for error messages
 */
#define LOG_DEBUG   1
#define LOG_NORMAL  2
#define LOG_ERROR   3

/*
 * == Structures containing data from RSS-torrent database.
 */

/*
 * Function pointer used in this callback implementation.
 * @Arguments
 * Data 		is data defined during the registration of the callback.
 * Calldata The data passed from the calling function.
 * @Return
 */
typedef int (*rsstcallbackfnct)(void *data, void *calldata);

/*
 * Structure to store callbacks in
 */
typedef struct{
	int 	nr;										/* Number of functions to call. */
	rsstcallbackfnct *callback; /* Array of function pointers. 	*/
	void 						 **data;		/* Array of data-pointers. 			*/
} struct_callback;

/*
 * Download RSS/torrent structure
 */
typedef struct {
	int id;				/*ID of the RSS/torrent depends newtorrents id/downloaded id*/  
	int status;		/*0 when download was successful, else -1 */
	char *error;	/*Error message telling what has failed in text */
} struct_download;

/*
 * Log file entry structure
 */
typedef struct {
	char *msg;		/* Pointer to the message being logged */
	int	 *sev;		/* Severity of log entry */
} struct_logmsg;

/*
 * Callback pointers.
 */
typedef struct {
	struct_callback *downloadrss; 		/* When a RSS file is downloaded */
  struct_callback *downloadtorrent; /* When a Torrent is downloaded */
	struct_callback *startupdate; 		/* Emitted at the start of an update cycle */
  struct_callback *rssdownload;     /* Emitted at the start of the RSS download */
  struct_callback *applyfilters;    /* Emitted at the start of the Applying of the filters */
  struct_callback *wrapup;          /* Emitted at the start of the wrap up stage */
	struct_callback *endupdate; 			/* emitted at the end of an update cycle */
	struct_callback *logmessage; 			/* When ever a logmessage is created, this method is called */
} struct_callbacks;	

/*
 * RSS-torrent handle.
 * This handle will change when other database types are implemented, 
 * so do not reference to components in this struct.
 */
typedef struct {
	sqlite3 				 *db;				// RSS-torrent database handle.
	int			 					lockfile; // Lock file handle.
	struct_callbacks 	callback;	// Struct containing the callbacks
  void             *data;     // Data pointer, to be used by the client to pass data though callbacks.
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
  char  *metatype;
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
	char *title;        // Title from the RSS feed
	char *link;         // The link to the metafile
	char *pubdate;      // The data the meta file was released 
	char *category;     // The category from the RSS feed
  char *metatype;     // The type of content Torrent or NZB
  char *baretitle;    // The bare title of the show/content 
	int  season;        // The season number
	int  episode;       // The episode number
} downloaded_struct;  

/*
 * Downloaded container
 */
typedef struct {
	int nr;
	downloaded_struct *downloaded;
} downloaded_container;

/*
 * Last downloaded 
 */
typedef struct {
  char *filtername;
  char *filtertype;
  downloaded_struct *downloaded;
} lastdowned_struct;

/*
 * Last downloaded container
 */
typedef struct {
  int nr;
  lastdowned_struct *lastdownloaded;
} lastdowned_container;

/*
 * Statistics structure to be filled.
 */
typedef struct {
  int     metafile;   // Number of meta files in newtorrents table.
  int     downloaded; // Number of downloaded meta files in the downloaded table.
  int     sources;    // Number sources in the database.
  char    *version;  // Version string of software version.
  int     database;   // Database version number.
  int     simples;    // Number of simple filters.
  int     sqls;       // SQL filter count.
  size_t  dbsize;     // Size of the current database file.
} stats_struct;

/*
 * The config item names
 */
#define CONF_TORRENTDIR "torrentdir"
#define CONF_NZBDIR     "nzbdir"
#define CONF_LOGFILE    "logfile"
#define CONF_REFRESH    "refresh"
#define CONF_RETAIN     "retain"
#define CONF_DEFPARSER  "default_parser"
#define CONF_LOCKFILE   "lockfile"
#define CONF_SMTPTO     "smtp_to"
#define CONF_SMTPFROM   "smtp_from"
#define CONF_SMTPHOST   "smtp_host"
#define CONF_MIN_SIZE   "min_size"
#define CONF_SMTPENABLE "smtp_enable"

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
  char *metatype;
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
 * Run the Database init script.
 * @return
 * 0 on success, -1 on failure
 */
int rsstrundbinitscript(rsstor_handle *handle);

/*
 * Free RSS-torrent handle
 * @Arguments
 * handle pointer to RSS-torrent structure
 */
void freersstor(rsstor_handle *handle);

/*
 * Get statistics from application
 * No free for the returned struct yet
 * @Arguments
 * stats statistics structure
 * @Returns
 * 0 when okay, -1 on error
 */
int rsstgetstats(rsstor_handle *handle, stats_struct *stats);

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
 * Delete content from config_container struct
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
 * prop name of the property to change
 * value new value to enter
 * @returns
 * When not found -1 is returned.
 * On success 0 is returned.
 */
int rsstsetconfigitem(rsstor_handle *handle, const char *prop, const char *value);

/*
 * Get value of a config object.
 * Make sure the free the value returned in the value pointer.
 * @arguments
 * prop name of config property to retrieve
 * value pointer to the pointer that is going to hold the retrieved value.
 * @returns
 * 0 when the value was found, otherwise -1.
 */
int rsstconfiggetproperty(rsstor_handle *handle, char *prop, char **value);

/*
 * Get value of a config object.
 * @arguments
 * prop name of the propertie to change
 * number pointer to place to store value of prop
 * @returns
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstconfiggetint(rsstor_handle *handle, char *prop, int *number);

/*
 * == Functions to manipulate sources
 */

/*
 * Get supported meta file types
 * @return
 * Returns a pointer to the names of the supported meta types.
 */
char **getsupportedmetatypes(); 

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
 * name filter name
 * url source URL
 * parsertype parser type
 * metatype meta file type (torrent/nzb)
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstaddsource(rsstor_handle *handle, const char *name, const char *url, char *parsertype, char *metatype );

/*
 * Delete source item
 * @arguments
 * handle RSS-torrent handle
 * name source name to delete
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
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
 * This routine retrieves the records from the downloaded table.
 * A selection is based on name.
 * @Arguments
 * db
 * optarg
 * @return
 * 0 on success
 * -1 on error
 */
int rsstfinddowned(rsstor_handle *handle, char *optarg);

/*
 * Function to download torrent
 */

/*
 * Test torrentdir
 * @return
 * 0 when writabe, -1 if not.
 */
int rssttesttorrentdir(rsstor_handle *handle);

/*
 * Get the newest season and episode
 * @arguments
 * filter Simple filter struct
 * Season newest season
 * Episode newest episode
 * @return
 * 0 on success otherwise 1
 * 1 When season and episode are found
 * -1 on error
 * @comments
 * This function is this file because all supporting functions are in this file.
 */
int rsstgetnewestepisode(simplefilter_struct *filter, int *season, int *episode);

/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on success, return -1 on failure.
 */
int rsstfindtorrentids(simplefilter_struct *filter);

/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks through the newtorrents table to pick the torrent by id.
 * @arguments
 * id	The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyid(rsstor_handle *handle, int torid);

/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks through the newtorrents table to pick the torrent by id.
 * @arguments
 * torid The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyidstr(rsstor_handle *handle, char *torid);

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
 * The routine looks through the newtorrents table to pick the torrent by id.
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
 * Get all SQL filter settings
 * @Arguments
 * handle RSS-torrent handle
 * container The container to store the container in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetfilterbyname(rsstor_handle *handle, char *name, filter_container **container);

/*
 * Get information of the last downloads and the filters that produced them
 * @Arguments
 * handle RSS-torrent handle 
 * container the pointer pointer to the container
 * @return
 * 0 on success, -1 on failure
 */
int rsstgetlastdownloaded(rsstor_handle *handle, lastdowned_container *container);

/*
 * Free the downloaded container and its contents
 * When the container it self is allocated, it should be freed separately
 * @Arguments
 * container pointer to the downed container to be freed
 */
void rsstfreelastdownedcontainer(lastdowned_container *container);

/*
 * Add source item
 * @Arguments
 * handle RSS-torrent handle
 * name name of the filter
 * filter SQL query to sort out the candidates
 * nodupfilter SQL query to root out the candidates that are already downloaded.
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstaddfilter(rsstor_handle *handle, const char *name, const char *filter, const char *nodupfilter);

/*
 * Add simple filter
 * @variables
 * opts structure holding the options
 * @returns 
 * 0 on success, else -1
 */
int rsstaddsimplefilter(rsstor_handle *handle, simplefilter_struct *filter);

/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * limit max number of simple filters retrieved
 * offset the number of simple filters to skip
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
int rsstgetsimplefilterid(rsstor_handle *handle, simplefilter_container **simplefilter, int id);

/*
 * Free simplefilter structure
 * @Arguments
 * simplefilter pointer to simplefilter struct to be freed
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
 * Do filter test
 * show first 10 matches
 * @arguments
 * filter filter SQL
 * nodup nonup SQL
 * @return
 * -1 on error
 * 0 on success
 */
int rsstdofiltertest(char *filter, char *nodup);

/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on success, return -1 on failure.
 */
int rsstdosimpletest(simplefilter_struct *filter);

/*
 * Del filter filter
 * When already existing -1 is returned.
 * @variables
 * name filter name to delete
 * @returns
 * On success 0 is returned.
 */
int rsstdelsimple(rsstor_handle *handle, const char *name);

/*
 * delete filter item
 * When the name is not found -1 is returned.
 * On success 0 is returned.
 */
int rsstdelallsimple(rsstor_handle *handle);

/*
 * Delete SQL filter
 * When already existing -1 is returned.
 * @arguments
 * name the name of the SQL filter to delete
 * @return
 * On success 0 is returned.
 */
int rsstdelfilter(rsstor_handle *handle, const char *name);

/*
 * Delete all filters.
 * Deletes all filters from filtertable.
 * @returns
 * On success 0 is returned.
 */
int rsstdelallfilters(rsstor_handle *handle);

/*
 * Add source item
 * @Arguments
 * handle RSS-torrent handle
 * name name of the filter
 * filter SQL query to sort out the candidates
 * nodupfilter SQL query to root out the candidates that are already downloaded.
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstaddfilter(rsstor_handle *handle, const char *name, const char *filter, const char *nodupfilter);

/*
 * == Functions to run RSS-torrent
 */

/*
 * Main loop, dispatches tasks
 * @arguments
 * handle RSS-torrent handle
 * onetime Set loop for infinite loop, once for running the step just once
 * @return
 * 0 for now.
 */
int rsstrunloop(rsstor_handle *handle, LOOPMODE onetime);

/*
 * Do a cycle of workload
 * As runloop is going to be depricated.
 * A runloop should be implemented by the calling program.
 * @Arguments
 * handle
 * @return
 * 0 for now
 */
int runcycle(rsstor_handle *handle);

/*
 * == Logi file writing routines
 */

/*
 * Loglevels
 */
#define LOG_DEBUG   1
#define LOG_NORMAL  2
#define LOG_ERROR   3

/*
 * Log an entry
 * LOG_DEBUG, LOG_NORMAL, LOG_ERROR
 * @arguments
 * level the log level (see log level defines)
 * str the string to log 
 * ... the arguments to fill out in the log line
 * @return
 * returns 1 for now in all cases
 */
int rsstwritelog(int level, char *str,...);

/*
 * == Mail routines
 */

/*
 * Uses the mail routine to send a test mail.
 * Arguments :
 * testxt, test message to send.
 */
void rssttestmail(rsstor_handle *handle, char *testtxt);

/*
 * Callback registration functions
 */

/*
 * Add a routine that is executed on a RSS download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddownrsscallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * Add a routine that is executed on a torrent download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddowntorcallback(rsstor_handle *handle, rsstcallbackfnct callback);


/*
 * Add a routine that is executed on start of update event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddstartupcallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * Add a routine that handles start of update cycle
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddendupcallback(rsstor_handle *handle, rsstcallbackfnct callback);

/*
 * Add a routine that is executed on a RSS download event
 * @arguments
 * handle			handle to RSS-torrent pointer
 * callback 	Function pointer to the routine to add
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstadddownrsscallback(rsstor_handle *handle, rsstcallbackfnct callback);

#endif
