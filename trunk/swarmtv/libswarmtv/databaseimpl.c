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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pcre.h>
#include <sqlite3.h>
#include <stdarg.h>

#include "types.h"
#include "database.h"
#include "databaseimpl.h"
#include "logfile.h"
#include "filesystem.h"
#include "regexp.h"
#include "setup.h"
#include "sandboxdb.h"
#include "testfilter.h"
#include "simplefilter.h"
#include "torrentdb.h"
#include "database.h"



/*
 * Database create script version 4
 */
static const char *dbinitscript = 
"BEGIN TRANSACTION"
""
"-- Drop tables\n"
"drop table if exists version;\n"
"drop table if exists newtorrents;\n"
"drop table if exists downloaded;\n"
"drop table if exists filters;\n"
"drop table if exists sources;\n"
"drop table if exists config;\n"
"drop table if exists simplefilters;\n"
"\n"
"-- Create versioning field\n"
"create table version (version INTEGER);\n"
"INSERT INTO 'version' VALUES(4);\n"
"\n"
"-- Create the newtorrents table\n"
"create table newtorrents (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE, "
"category TEXT, season INTEGER, episode INTEGER, seeds INTEGER DEFAULT 0, peers INTEGER DEFAULT 0, size INTEGER, source TEXT DEFAULT 'unknown', metatype TEXT DEFAULT 'torrent', new TEXT DEFAULT 'Y');\n"
"\n"
"-- Create the downloaded table\n"
"create table downloaded (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE, "
"category TEXT, season INTEGER, episode INTEGER, date DATE, metatype TEXT DEFAULT 'torrent');\n"
"\n"
"-- Create the filters table\n"
"create table filters (id INTEGER PRIMARY KEY, name TEXT UNIQUE, filter TEXT, nodouble TEXT DEFAULT '');\n"
"CREATE TABLE simplefilters (id INTEGER PRIMARY KEY, name TEXT UNIQUE, title TEXT, exclude TEXT, category TEXT, source TEXT, maxsize INTEGER DEFAULT 0, "
"minsize INTEGER DEFAULT 0, nodup TEXT NOT NULL, fromseason INTEGER DEFAULT 0, fromepisode INTEGER DEFAULT 0, metatype TEXT DEFAULT '');\n"
"\n"
"-- Create the sources table\n"
"create table sources (id INTEGER PRIMARY KEY, name TEXT UNIQUE, url TEXT, parser TEXT, metatype TEXT DEFAULT 'torrent');\n"
"\n"
"-- Create the lastdownloaded table\n"
"CREATE TABLE lastdownload (id INTEGER PRIMARY KEY, simple_id INTEGER UNIQUE, sql_id INTEGER UNIQUE, downloaded_id INTEGER);"
"\n"
"-- Create the config table, and fill out the table with the 'standard' values\n"
"create table config (id INTEGER PRIMARY KEY, prop TEXT UNIQUE, value TEXT, descr TEXT);\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('torrentdir', '~/torrents', 'Path the downloaded Torrents are placed in.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzbdir', '~/nzb', 'Path the downloaded NZBs are placed in.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('logfile', '~/.swarmtv/swarmtv.log', 'Path to logfile.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('lockfile', '~/.swarmtv/lockfile.pid', 'Path to lockfile.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('refresh', '3600', 'Seconds between refreshes.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('retain', '30', 'The number of days source information is retained.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('default_parser', 'defaultrss', 'The default RSS filter to add to new rss sources.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_enable', 'N', '`Y` is send email notifications on new download, `N` is don`t.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_to', 'foo@bar.nl', 'Host to send the notifications to.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_from', 'user@somehost.nl', 'The from email-address in the mail headers.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_host', 'smtp.foobar.nl:25', 'The STMP server used to send the notifications.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('min_size', '4000000', 'When size is smaller then this, download torrent and check.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('tor_mon_dir', '~', 'Path downloaded Torrents end up.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('tor_mon_enable', 'n', 'Enable/disable tor partition usage monitering.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzb_mon_dir', '~', 'Path downloaded NZBs end up.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzb_mon_enable', 'n', 'Enable/disable nzb partition usage monitering.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('mon_limit', '90', 'Stop downloading when the disk usage exceeds.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_enable', 'n', 'Enable proxy support.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_url', '', 'URL of the HTTP proxy.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_userpass', '', 'User name and password of the proxy.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_type', '', 'Set the type of proxy to use default is HTTP.');\n"
""
"COMMIT"
"\n";

/*
 * Database update script v1 to v2
 */
static const char *updatev1tov2 = 
"BEGIN TRANSACTION"
""
"-- Add parser column to newtorrents table\n"
"ALTER TABLE newtorrents ADD COLUMN source TEXT DEFAULT 'unknown';\n"
""
"-- Add parser column to simplefilters table\n"
"ALTER TABLE simplefilters ADD COLUMN source TEXT DEFAULT '';\n"
""
"-- Up database version from version 1 to 2\n"
"update version set version = '2';\n"
""
"COMMIT"
"\n";

/*
 * Database update script v2 to v3
 */
static const char *updatev2tov3 = 
"BEGIN TRANSACTION"
""
"-- Add filetype to newtorrents table (Might be renamed all togethera)\n"
"ALTER TABLE newtorrents ADD COLUMN metatype TEXT DEFAULT 'torrent';\n"
""
"-- Add nzb Monitor directory to config-options\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzbdir', '~/nzb', 'Path the downloaded NZBs are placed in.');\n"
""
"-- Add meta type to source table\n"
"ALTER TABLE sources  ADD COLUMN metatype TEXT DEFAULT 'torrent';\n"
""
"-- Add metatype column to simplefilters table\n"
"ALTER TABLE simplefilters ADD COLUMN metatype TEXT DEFAULT '';\n"
""
"-- Make sure to include the metatype the file came from in the downloaded table.\n"
"ALTER TABLE downloaded ADD COLUMN metatype TEXT DEFAULT 'torrent';\n"
""
"-- Up database version from version 2 to 3\n"
"update version set version = '3';\n"
""
"-- Add the lastdownloaded tablei\n"
"CREATE TABLE lastdownload (id INTEGER PRIMARY KEY, simple_id INTEGER UNIQUE, sql_id INTEGER UNIQUE, downloaded_id INTEGER);\n"
""
"COMMIT"
"\n";

static const char *updatev3tov4 =
"BEGIN TRANSACTION"
""
"-- Inserting new properties\n" 
"INSERT INTO 'config' (prop, value, descr) VALUES('tor_mon_dir', '~', 'Path downloaded Torrents end up.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('tor_mon_enable', 'n', 'Enable/disable tor partition usage monitering.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzb_mon_dir', '~', 'Path downloaded NZBs end up.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('nzb_mon_enable', 'n', 'Enable/disable nzb partition usage monitering.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('mon_limit', '90', 'Stop downloading when the disk usage exceeds.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_enable', 'n', 'Enable proxy support.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_url', '', 'URL of the HTTP proxy.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_userpass', '', 'User name and password of the proxy.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('proxy_type', '', 'Set the type of proxy to use default is HTTP.');\n"
""
"-- Up database version from version 3 to 4\n"
"update version set version = '4';\n"
""
"COMMIT"
"\n";


/*
 * Get database version.
 * @arguments
 * version pointer to int holding database version number
 * @return
 * 0 on success, -1 on failure
 */
static int getdbversion(sqlite3 *db, int *version)
{
	int 	rc=0;
	char *text=NULL;

	const char *existquery="SELECT * FROM sqlite_master WHERE name = 'version' AND type = 'table'";
	const char *query="SELECT version FROM version";
	
	/*
	 * Test if version table exists
	 */
	rc = rsstexecutequery(db, existquery, NULL);
	free(text);
	if(rc != 1) {
		/*
		 * version table not there
		 */
		*version=0;
		return -1;
	}

	/*
	 * Execute query.
	 * translate value when found
	 */
	rc = rsstdosingletextquery(db, (unsigned char const**)&text, query, NULL);
	if(rc == 0) {
		*version = atoi(text);
	}

	/*
	 * Return rc from dosingeltextquery
	 */
	free(text);
	return rc;
}

/*
 * Create a complete database if needed, otherwise run update scripts.
 * @arguments 
 * version current database version
 * @return
 * 0 on success, -1 on failure
 */
static int fixdb(rsstor_handle *handle, int version) 
{
  int 			rc=0;
	sqlite3  *db=NULL;

	/*
	 * get db pointer
	 */
	db = handle->db;

  /*
   * When version is < 1 database state unknown reinitialize whole db
   */
  if(version < 1) {
    rc = rsstrundbinitscript(handle);
    return rc;
  } 

  /*
   * When the database version < 2 run this script to update it to version 2.
   */
  if(version < 2) {
    rsstwritelog(LOG_NORMAL, "Updating database from version 1 to version 2.");
    rc = dbexecscript(db, updatev1tov2); 
    if(rc < 0) {
      rsstwritelog(LOG_ERROR, "Update script version 1 to version 2 Failed!");
      rsstwritelog(LOG_ERROR, "Update aborted.");
      return rc;
    }
  }
  if(version < 3) {
    rsstwritelog(LOG_NORMAL, "Updating database from version 2 to version 3.");
    rc = dbexecscript(db, updatev2tov3); 
    if(rc < 0) {
      rsstwritelog(LOG_ERROR, "Update script version 2 to version 3 Failed!");
      rsstwritelog(LOG_ERROR, "Update aborted.");
      return rc;
    }
  }
  if(version < 4) {
    rsstwritelog(LOG_NORMAL, "Updating database from version 3 to version 4.");
    rc = dbexecscript(db, updatev3tov4); 
    if(rc < 0) {
      rsstwritelog(LOG_ERROR, "Update script version 3 to version 4 Failed!");
      rsstwritelog(LOG_ERROR, "Update aborted.");
      return rc;
    }
  }

  /*
   * All update scripts done, return result
   */
  return rc;
}

/*
 * Run the Database init script.
 * @return
 * 0 on success, -1 on failure
 */
int rsstrundbinitscript(rsstor_handle *handle)
{
	int rc=0;

	/*
	 * Execute query
	 */
	rc = dbexecscript(handle->db, dbinitscript); 

	/*
	 * return result
	 */
	return rc;
}


/*
 * This function implements PCRE functionality to the queries.
 * called every time Sqlite crosses an regexp.
 */
static void genregexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value, int opt){
  int   			rc=0;
  const char 	*var1=NULL;
  const char 	*var2=NULL;
  pcre 				*re = NULL; 
  const char 	*error = NULL; 
  int 				errOffset = 0; 
  int 				match=0;

  /*
   * sanity check
   */
  if(num != 2) {
    rsstwritelog(LOG_ERROR, "on line: %d, in file: %s, the wrong number of arguments were called: %d",
      __LINE__,
      __FILE__,
      num);
    exit(1);
  }

  /*
   * Extract the values from the function call.
   */
  var1 = (const char*) sqlite3_value_text(sqlite3_value[0]);
  var2 = (const char*) sqlite3_value_text(sqlite3_value[1]);

	/*
	 * When NULL is provided, match fails.
	 */
	if(var2 == NULL) {
		sqlite3_result_int(db, 0);
		return;
	}

  /*
   * Compile regular expression
   */
  re = pcre_compile( var1, opt, &error, 
      &errOffset, NULL); 
  if (re == NULL) { 
    rsstwritelog(LOG_ERROR, "Regexp compilation failed at " 
        "offset %d: %s %s:%d\n", errOffset, error, __FILE__, __LINE__); 
    exit(1); 
  } 

  /* 
   * Do the match and interpret outcome.
   */
  rc = pcre_exec(re, NULL, var2, strlen(var2), 0, 
      0 /* default options */, NULL, 0); 
  switch (rc) { 
    case PCRE_ERROR_NOMATCH: 
      match = 0; 
      break; 
    case PCRE_ERROR_BADOPTION: 
      rsstwritelog(LOG_ERROR, "An unrecognized bit was set in the " 
          "options argument %s:%d", __FILE__, __LINE__); 
      break; 
    case PCRE_ERROR_NOMEMORY: 
      rsstwritelog(LOG_ERROR, "Not enough memory available. %s:%d", __FILE__, __LINE__); 
      break; 
    default: 
      if (rc < 0) { 
        rsstwritelog(LOG_ERROR, "A regexp match error " 
            "occurred: %d %s:%d", rc, __FILE__, __LINE__); 
      } 
      else { 
        match = 1; 
      } 
      break; 
  } 

  /*
   * Close the regexp
   */
  pcre_free(re);

  /*
   * return result.
   */
  sqlite3_result_int(db, match);
}


/*
 * This function implement PCRE functionality to the queries.
 * called every time Sqlite crosses an regexp.
 */
static void regexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value)
{
	genregexpfunc(db, num, sqlite3_value, 0);
}


/*
 * This function implement PCRE functionality to the queries.
 * called every time Sqlite crosses an regexp.
 * This function matches case insensitive.
 */
static void iregexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value)
{
	genregexpfunc(db, num, sqlite3_value, PCRE_CASELESS);
}


/*
 * Open database, and add regexp functionality.
 * make sure the handle pointer is pointing to an valid address
 * @Arguments
 * filename Database filename
 * handle RSS-torrent handle
 * @Returns
 *
 */
int rsstinitdatabase(const char *filename, rsstor_handle *handle)
{
	int         rc=0; /* return code */
	int					version=0;
	char       *zErrMsg = 0;
	char       *dbpath = NULL;
	sqlite3    *db=NULL;

	/*
	 * Complete the filename is it contains a ~ as home dir
	 */
	rsstcompletepath(filename, &dbpath);

	/*
   * Open the Sqlite database.
   */
  rc = sqlite3_open(dbpath, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return !SQLITE_OK;
  }

	/*
	 * Store db pointer
	 */
	handle->db = db;

  /*
   * free db path
   */
  free(dbpath);

	/*
	 * Test if database is initialized and of the right version.
	 */
	rc = getdbversion(db, &version);
	if(rc != 0 || version != RSST_DB_VERSION){
		/*
		 * Create new DB
		 */
		printf("Running create database script.\n");
    rc = fixdb(handle, version); 
		if(rc == -1){
			fprintf(stderr, "Can't open database, init script failed!\n");
			sqlite3_close(db);
			return !SQLITE_OK;
		}
	}

  /* 
   * Add regexp function.
   */
  typedef struct sqlite3_value sqlite3_value;
  rc = sqlite3_create_function(
      db,
      "regexp",       // name of the function
      2,              // number of arguments
      SQLITE_UTF8,    // Kind of encoding we expect
      NULL,
      regexpfunc,     // function to call 
      NULL,           // 
      NULL
      );
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_create_function\n");
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return !SQLITE_OK;
  }

  rc = sqlite3_create_function(
      db,
      "iregexp",       // name of the function
      2,              // number of arguments
      SQLITE_UTF8,    // Kind of encoding we expect
      NULL,
      iregexpfunc,     // function to call 
      NULL,           // 
      NULL
			);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_create_function\n");
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return !SQLITE_OK;
  }

  /*
   * All went well.
   */
  return SQLITE_OK;
}

/*
 * Reallocate if not big enough 
 * @Arguments
 * buffer Pointer to buffer
 * bufnr pointer to number of elements in the buffer
 * occupied number of elements occupied
 * structsize size of the structure in place
 * @returns
 * pointer to new area, null or fail.
 */
static void *rsstmakespace(void *buffer, int *bufnr, int occupied, size_t structsize)
{
	size_t newsize=0;

	/*
	 * Reallocate twice the size when we run out of space
	 */
	if(occupied == *bufnr) {
		*bufnr = *bufnr * 2;
		newsize = *bufnr * structsize;
		buffer = realloc(buffer, newsize);
		if(buffer == NULL) {
			rsstwritelog(LOG_ERROR, "Realloc failed ! %s:%d", __FILE__, __LINE__);
			return NULL;
		}
	}

	return buffer;
}


/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
static int rsststoreconfigcontainer(sqlite3_stmt *result, config_container *container)
{
	int 		count=0;
	int 		allocrecords=0;
	char 	 *column=NULL;

	/*
	 * prealloc for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->config = calloc(allocrecords, sizeof(config_struct));
	if(container->config == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

  /*
   * loop until the end of the dataset is found
	 * Copy results to struct
   */
  while( SQLITE_DONE != sqlite3_step(result)) {
		/*
		 * Store values
		 */
		container->config[count].id = sqlite3_column_int(result, 0);
		column = (char*) sqlite3_column_text(result, 1);
		rsstalloccopy(&(container->config[count].name), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 2);
		rsstalloccopy(&(container->config[count].value), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 3);
		rsstalloccopy(&(container->config[count].description), column, strlen(column));
		count++;

		/*
		 * Realloc goes here
		 */
		container->config = rsstmakespace(container->config, &allocrecords, count, sizeof(config_struct));
  }

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}

/*
 * Get all config settings
 * @Arguments
 * configitems The container to store the config items in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallconfig(rsstor_handle *handle, config_container **configitems)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	config_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * Get db pointer.
	 */
	db = handle->db;

	/*
	 * Query to retrieve config items
	 */
	const char *query = "select id, prop, value, descr from config";

	/*
	 * Alloc the container
	 */
	localitems = calloc(1, sizeof(config_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, NULL);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststoreconfigcontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*configitems = localitems;

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);
	return retval;
}

/*
 * Free config struct
 * @Arguments
 * config pointer to config structure
 */
static void rsstfreeconfig(config_struct *config)
{
	free(config->name);
	free(config->value);
	free(config->description);
}

/*
 * Delete content from config_container struct
 * @Arguments
 * container Pointer to configcontainer to free content of
 * @Return
 * 0 on success, -1 on failure
 */
int rsstfreeconfigcontainer(config_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container->config == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreeconfig(container->config+count);
	}

	/*
	 * free the container itself
	 */
	free(container->config);
	free(container);

	return 0;
}

/*
 * Free config struct
 * @Arguments
 * config pointer to config structure
 */
static void rsstfreefilter(filter_struct *filter)
{
	free(filter->name);
	free(filter->filter);
	free(filter->nodup);
}

/*
 * Delete content from config_container struct
 * @Arguments
 * container Pointer to configcontainer to free content of
 * @Return
 * 0 on success, -1 on failure
 */
int rsstfreefiltercontainer(filter_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container->filter == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreefilter(container->filter+count);
	}

	/*
	 * free the container itself
	 */
	free(container->filter);
	free(container);

	return 0;
}

/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
static int rsststorefiltercontainer(sqlite3_stmt *result, filter_container *container)
{
	int 		count=0;
	int 		allocrecords=0;
	char 	 *column=NULL;

	/*
	 * prealloc for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->filter = calloc(allocrecords, sizeof(filter_struct));
	if(container->filter == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

  /*
   * loop until the end of the dataset is found
	 * Copy results to struct
   */
  while( SQLITE_DONE != sqlite3_step(result)) {
		/*
		 * Store values
		 */
		container->filter[count].id = sqlite3_column_int(result, 0);
		column = (char*) sqlite3_column_text(result, 1);
		rsstalloccopy(&(container->filter[count].name), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 2);
		rsstalloccopy(&(container->filter[count].filter), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 3);
		rsstalloccopy(&(container->filter[count].nodup), column, strlen(column));
		count++;

		/*
		 * Realloc goes here
		 */
		container->filter = rsstmakespace(container->filter, &allocrecords, count, sizeof(filter_struct));
  }

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}

/*
 * Get all SQL filter settings
 * @Arguments
 * handle RSS-torrent handle
 * container The container to store the container in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallfilter(rsstor_handle *handle, filter_container **container)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	filter_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * Get db pointer.
	 */
	db = handle->db;

	/*
	 * Query to retrieve filter items
	 */
	const char *query = "select id, name, filter, nodouble from 'filters'";

	/*
	 * Alloc the container
	 */
	localitems = calloc(1, sizeof(filter_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, NULL);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "Sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststorefiltercontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*container = localitems;

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);
	return retval;
}

/*
 * Get all SQL filter settings
 * @Arguments
 * handle RSS-torrent handle
 * container The container to store the container in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetfilterbyname(rsstor_handle *handle, char *name, filter_container **container)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	filter_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * Sanity check
	 */
	if(name == NULL){
		return -1;
	}

	/*
	 * Get db pointer.
	 */
	db = handle->db;

	/*
	 * Query to retrieve filter items
	 */
	const char *query = "select id, name, filter, nodouble from 'filters' where name = ?1";

	/*
	 * Alloc the container
	 */
	localitems = calloc(1, sizeof(filter_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, "s", name);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststorefiltercontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*container = localitems;

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);
	return retval;
}


/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
static int rsststoredownloadedcontainer(sqlite3_stmt *result, downloaded_container *container)
{
	int 		count=0;
	int 		allocrecords=0;
	char 	 *column=NULL;

	/*
	 * prealloc for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->downloaded = calloc(allocrecords, sizeof(downloaded_struct));
	if(container->downloaded == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

  /*
   * loop until the end of the dataset is found
	 * Copy results to struct
   */
  while( SQLITE_DONE != sqlite3_step(result)) {
		/*
		 * Store values
		 */
		container->downloaded[count].id = sqlite3_column_int(result, 0);
		column = (char*) sqlite3_column_text(result, 1);
		rsstalloccopy(&(container->downloaded[count].title), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 2);
		rsstalloccopy(&(container->downloaded[count].link), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 3);
		rsstalloccopy(&(container->downloaded[count].pubdate), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 4);
		rsstalloccopy(&(container->downloaded[count].category), column, strlen(column));
		container->downloaded[count].season = sqlite3_column_int(result, 5);
		container->downloaded[count].episode = sqlite3_column_int(result, 6);
		column = (char*)sqlite3_column_text(result, 7);
		rsstalloccopy(&(container->downloaded[count].downdate), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 8);
		rsstalloccopy(&(container->downloaded[count].metatype), column, strlen(column));
    container->downloaded[count].baretitle=NULL;
    container->downloaded[count].filter=NULL;
		count++;

		/*
		 * Reallocate goes here
		 */
		container->downloaded = rsstmakespace(container->downloaded, &allocrecords, count, sizeof(downloaded_struct));
	}

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}


/*
 * Get downloaded torrents
 * @arguments
 * downloaded The container to store the downloaded in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetdownloaded(rsstor_handle *handle, downloaded_container **downloaded, int limit, int offset)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	downloaded_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * handle
	 */
	db = handle->db;

	/*
	 * Query to retrieve downloaded items
	 * int   id;
	 * char *title;
	 * char *link;
	 * char *pubdate;
	 * char *category;
	 * int  season;
	 * int  episode;
	 */
	const char *query = "SELECT id, title, link, pubdate, category, season, episode, date, metatype "
    "FROM downloaded "
    "ORDER BY date DESC LIMIT ?1 OFFSET ?2";

	/*
	 * Allocate the container
	 */
	localitems = calloc(1, sizeof(downloaded_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, "dd", limit, offset);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststoredownloadedcontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*downloaded = localitems;

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppstmt);
	return retval;
}


/*
 * Free downloaded structure
 * @Arguments
 * downloaded pointer to downloaded struct to be freed
 */
void rsstfreedownloaded(downloaded_struct *downloaded)
{
  if(downloaded == NULL) {
    return;
  }

	/*
	 * char *title;
	 * char *link;
	 * char *pubdate;
	 * char *category;
	 */
	free(downloaded->title);
	free(downloaded->link);
	free(downloaded->pubdate);
	free(downloaded->category);
  free(downloaded->metatype);
  free(downloaded->baretitle);
  free(downloaded->downdate);
  free(downloaded->filter);
}


/*
 * Delete content from source_container struct
 * @Arguments
 * container downloaded container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreedownloadedcontainer(downloaded_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container->downloaded == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreedownloaded(container->downloaded+count);
	}

	/*
	 * free the array itself
	 */
	free(container->downloaded);
	free(container);

	return 0;
}

/*
 * Store the database result into a simple struct
 */
static void rsststoresimplestruct(sqlite3_stmt *result, simplefilter_struct *simple)
{
	char 	 *column=NULL;

  /*
   * Store values
   */
  simple->id = sqlite3_column_int(result, 0);
  column = (char*) sqlite3_column_text(result, 1);
  rsstalloccopy(&(simple->name), column, strlen(column));
  column = (char*) sqlite3_column_text(result, 2);
  rsstalloccopy(&(simple->title), column, strlen(column));
  column = (char*) sqlite3_column_text(result, 3);
  rsstalloccopy(&(simple->exclude), column, strlen(column));
  column = (char*)sqlite3_column_text(result, 4);
  rsstalloccopy(&(simple->category), column, strlen(column));
  column = (char*)sqlite3_column_text(result, 5);
  rsstalloccopy(&(simple->source), column, strlen(column));
  simple->maxsize = sqlite3_column_double(result, 6);
  simple->minsize = sqlite3_column_double(result, 7);
  column = (char*)sqlite3_column_text(result, 8);
  rsstalloccopy(&(simple->nodup), column, strlen(column));
  simple->fromseason  = sqlite3_column_double(result, 9);
  simple->fromepisode = sqlite3_column_double(result, 10);
}


/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
static int rsststoresimplecontainer(sqlite3_stmt *result, simplefilter_container *container)
{
	int 		count=0;
	int 		allocrecords=0;
  simplefilter_struct *simplestruct=NULL;

	/*
	 * reallocate for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->simplefilter = calloc(allocrecords, sizeof(simplefilter_struct));
	if(container->simplefilter == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

  /*
   * loop until the end of the dataset is found
	 * Copy results to struct
   */
  while( SQLITE_DONE != sqlite3_step(result)) {
    /*
     * Fill the simple structure
     */
    simplestruct=&(container->simplefilter[count]);
    rsststoresimplestruct(result, simplestruct);

		count++;

		/*
		 * Reallocate goes here
		 */
		container->simplefilter = rsstmakespace(container->simplefilter, &allocrecords, count, sizeof(simplefilter_struct));
	}

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}


/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallsimplefilter(rsstor_handle *handle, simplefilter_container **simplefilter, int limit, int offset)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	simplefilter_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * handle
	 */
	db = handle->db;

	/*
	 * Query to retrieve simplefilter items
	 */
	const char *query =  "SELECT id, name, title, exclude, category, source, maxsize, minsize, nodup, fromseason, fromepisode "
											 "FROM 'simplefilters' ORDER BY name LIMIT ?1 OFFSET ?2";

	/*
	 * Allocate the container
	 */
	localitems = calloc(1, sizeof(simplefilter_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, "dd", limit, offset);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "Rsstexecqueryresult failed %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststoresimplecontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*simplefilter = localitems;

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppstmt);
	return retval;
}


/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * id id of the simple filter
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetsimplefilterid(rsstor_handle *handle, simplefilter_struct **simplefilter, int id)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	simplefilter_struct *localitem=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * handle
	 */
	db = handle->db;

	/*
	 * Query to retrieve simplefilter items
	 */
	const char *query =  "SELECT id, name, title, exclude, category, source, maxsize, minsize, nodup, fromseason, fromepisode "
											 "FROM 'simplefilters' WHERE id=?1";

	/*
	 * Allocate the container
	 */
	localitem = calloc(1, sizeof(simplefilter_struct));
	if(localitem == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, "d", id);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "Rsstexecqueryresult failed %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
   * Do one step before executing
   */
  if(SQLITE_DONE != sqlite3_step(ppstmt)) {

    /*
     * Set output.
     */
    rsststoresimplestruct(ppstmt, localitem);
    *simplefilter = localitem;
  } else {
    *simplefilter = NULL;
    retval=-1;
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);
  return retval;
}


/*
 * Get simplefilter torrents
 * @arguments
 * simplefilter The container to store the simplefilter in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetsimplefiltername(rsstor_handle *handle, simplefilter_container **simplefilter, char *name)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	simplefilter_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * handle
	 */
	db = handle->db;

	/*
	 * Query to retrieve simplefilter items
	 */
	const char *query =  "SELECT id, name, title, exclude, category, source, maxsize, minsize, nodup, fromseason, fromepisode "
											 "FROM 'simplefilters' WHERE name=?1";

	/*
	 * Allocate the container
	 */
	localitems = calloc(1, sizeof(simplefilter_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, "s", name);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "Rsstexecqueryresult failed %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststoresimplecontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*simplefilter = localitems;

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppstmt);
	return retval;
}


/*
 * Free simplefilter structure
 * @Arguments
 * simplefilter pointer to simplefilter struct to be freed
 */
void rsstfreesimplefilter(simplefilter_struct *simplefilter)
{
	/*
	 * Free all strings
	 */
	free(simplefilter->name);
	free(simplefilter->title);
	free(simplefilter->exclude);
	free(simplefilter->category);
	free(simplefilter->source);
	free(simplefilter->nodup);
}


/*
 * Delete content from source_container struct
 * @Arguments
 * container simplefilter container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreesimplefiltercontainer(simplefilter_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container->simplefilter == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreesimplefilter(container->simplefilter+count);
	}

	/*
	 * free the array itself
	 */
	free(container->simplefilter);
	free(container);

	return 0;
}


/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
static int rsststoresourcecontainer(sqlite3_stmt *result, source_container *container)
{
	int 		count=0;
	int 		allocrecords=0;
	char 	 *column=NULL;

	/*
	 * reallocate for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->source = calloc(allocrecords, sizeof(source_struct));
	if(container->source == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * loop until the end of the dataset is found
	 * Copy results to struct
	 */
	while( SQLITE_DONE != sqlite3_step(result)) {
		/*
		 * Store values
		 * int   id;
		 * char *name;
		 * char *url;
		 * char *parser;
     * char *metatype;
		 */
		container->source[count].id = sqlite3_column_int(result, 0);
		column = (char*) sqlite3_column_text(result, 1);
		rsstalloccopy(&(container->source[count].name), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 2);
		rsstalloccopy(&(container->source[count].url), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 3);
		rsstalloccopy(&(container->source[count].parser), column, strlen(column));
		column = (char*)sqlite3_column_text(result, 4);
		rsstalloccopy(&(container->source[count].metatype), column, strlen(column));
		count++;

		/*
		 * reallocate goes here
		 */
		container->source = rsstmakespace(container->source, &allocrecords, count, sizeof(source_struct));
	}

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}


/*
 * Get RSS-source by id
 * @arguments
 * handle RSS-torrent handle
 * id source id to get
 * source structure containing the retrieved struct
 * @Return
 * 0 when okay, -1 on error
 */
int rsstgetsource(rsstor_handle *handle, int id, source_struct **source)
{
  int            rc=0;
  source_struct *sourcestr=NULL;
	sqlite3_stmt  *ppstmt=NULL;
	char 	        *column=NULL;
	char          *zErrMsg=NULL;

	/*
	 * Query to retrieve source items
	 */
	const char *query = "SELECT id, name, url, parser, metatype FROM sources WHERE id=?1";

	/*
	 * Alloc the container
	 */
	sourcestr = calloc(1, sizeof(source_struct));
	if(sourcestr == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(handle->db, &ppstmt, query, "d", id);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
  sqlite3_step(ppstmt);

  /*
   * Store retrieved data
   */
  sourcestr->id = sqlite3_column_int(ppstmt, 0);
  column = (char*) sqlite3_column_text(ppstmt, 1);
  rsstalloccopy(&(sourcestr->name), column, strlen(column));
  column = (char*)sqlite3_column_text(ppstmt, 2);
  rsstalloccopy(&(sourcestr->url), column, strlen(column));
  column = (char*)sqlite3_column_text(ppstmt, 3);
  rsstalloccopy(&(sourcestr->parser), column, strlen(column));
  column = (char*)sqlite3_column_text(ppstmt, 4);
  rsstalloccopy(&(sourcestr->metatype), column, strlen(column));
  
  /*
   * Set structure to be returned.
   */
  *source = sourcestr;

  /*
   * Clean up
   */
	rc = sqlite3_finalize(ppstmt);

  return 0;
}


/*
 * Get all RSS-sources
 * @arguments
 * sources The container to store the sources in
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstgetallsources(rsstor_handle *handle, source_container **sources)
{
	int 					rc=0;
	int 					retval=0;
	sqlite3_stmt *ppstmt=NULL;
	source_container *localitems=NULL;
	char         *zErrMsg=NULL;
	sqlite3      *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

	/*
	 * Query to retrieve source items
	 */
	const char *query = "SELECT id, name, url, parser, metatype FROM sources ORDER BY name";

	/*
	 * Alloc the container
	 */
	localitems = calloc(1, sizeof(source_container));
	if(localitems == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		exit(1);
	}

	/*
	 * Execute query
	 */
	rc = rsstexecqueryresult(db, &ppstmt, query, NULL);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Store result into container
	 */
	rc = rsststoresourcecontainer(ppstmt, localitems);

	/*
	 * Set output.
	 */
	*sources = localitems;

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppstmt);
	return retval;
}


/*
 * Free source structure
 * @Arguments
 * source pointer to source struct to be freed
 */
void rsstfreesource(source_struct *source)
{
	free(source->name);
	free(source->url);
	free(source->parser);
  free(source->metatype);
}


/*
 * Delete content from source_container struct
 * @Arguments
 * container sources container content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreesourcecontainer(source_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container->source == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreesource(container->source+count);
	}

	/*
	 * free the array itself
	 */
	free(container->source);
	free(container);

	return 0;
}

/*
 * Store newtorrent result into newtorrents_struct
 * @arguments
 * newtor newtorrents_struct
 * result sqlstatement set to the current record to store
 */
void rsststorenewtorrentstruct(sqlite3_stmt *result, newtorrents_struct *newstruct)
{
  char 	 *column=NULL;

  /*
   * Store values
   */
  newstruct->id = sqlite3_column_int(result, 0);
  column = (char*) sqlite3_column_text(result, 1);
  rsstalloccopy(&(newstruct->title), column, strlen(column));
  column = (char*)sqlite3_column_text(result, 2);
  rsstalloccopy(&(newstruct->link), column, strlen(column));
  newstruct->pubdate = sqlite3_column_int(result, 3);
  column = (char*)sqlite3_column_text(result, 4);
  rsstalloccopy(&(newstruct->category), column, strlen(column));
  column = (char*)sqlite3_column_text(result, 5);
  rsstalloccopy(&(newstruct->source), column, strlen(column));
  column = (char*)sqlite3_column_text(result, 6);
  rsstalloccopy(&(newstruct->metatype), column, strlen(column));
  newstruct->season = sqlite3_column_int(result, 7);
  newstruct->episode = sqlite3_column_int(result,8);
  newstruct->seeds = sqlite3_column_int(result, 9);
  newstruct->peers = sqlite3_column_int(result, 10);
  newstruct->size = sqlite3_column_double(result, 11);
}

/*
 * Store database result into struct
 * @Arguments 
 * result
 * container
 * @returns
 * 0 on success otherwise -1
 */
int rsststorenewtorrentcontainer(sqlite3_stmt *result, newtorrents_container *container)
{
	int 		count=0;
	int 		allocrecords=0;

	/*
	 * reallocate for START_ELEMENTS number of records
	 */ 
	allocrecords = RSST_START_ELEMENTS;
	container->newtorrent = calloc(allocrecords, sizeof(newtorrents_struct));
	if(container->newtorrent == NULL) {
		rsstwritelog(LOG_ERROR, "Calloc failed ! %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * loop until the end of the dataset is found
	 * Copy results to struct
	 */
	while( SQLITE_DONE != sqlite3_step(result)) {
    /*
     * Fill struct
     */
    rsststorenewtorrentstruct(result, &(container->newtorrent[count]));

		/*
		 * Move to next record & reallocate 
		 */
		count++;
		container->newtorrent = rsstmakespace(container->newtorrent, &allocrecords, count, sizeof(newtorrents_struct));
	}

	/*
	 * Save number of records retrieved
	 */
	container->nr=count;

	return 0;
}

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
int rsstfindnewtorrents(simplefilter_struct *filter, newtorrents_container **newtorrents, int limit, int offset) 
{
	int 					 rc=0;
	sandboxdb 		*sandbox=NULL;
	sqlite3_stmt 	*ppstmt=NULL;
	char         	*zErrMsg=NULL;
	rsstor_handle  handle;

	/*
	 * init handle;
	 */
	memset(&handle, 0, sizeof(rsstor_handle));

	/*
	 * Query to retrieve the data from the sandbox after all the work is done.
	 */
	char *query="SELECT newtorrents.id, downloaded.title, newtorrents.link, newtorrents.pubdate, newtorrents.category, "
		"newtorrents.source, downloaded.metatype, downloaded.season, downloaded.episode, newtorrents.seeds, "
    "newtorrents.peers, newtorrents.size "
    "FROM newtorrents, downloaded "
		"WHERE newtorrents.link = downloaded.link "
    "ORDER BY newtorrents.id LIMIT ?1 OFFSET ?2"; // get values from downloaded table

	/*
	 * Create sandbox
	 */
	sandbox = rsstinitfiltertest();
	if(sandbox == NULL){
		rsstwritelog(LOG_ERROR, "Sandbox creation failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Dirty but needed
	 */
	handle.db=sandbox->db;

	/*
	 * Remove unwanted data from sandbox.
	 */
	rc = rsstcleanoutdb(sandbox);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Cleaning out sandbox failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Add simple filter
	 */
	rc = rsstinsertsimplefilter(&handle, filter);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Inserting simple filter failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Execute filter
	 * with simulate 1, to run the simplefilters only in the database.
	 */
	rc = rsstdownloadsimple(&handle, (SIM) sim);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Executing filter failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Get records using query
	 */
	rc = rsstexecqueryresult(sandbox->db, &ppstmt, query, "dd", limit, offset);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Allocate and fill container
	 */
	*newtorrents = calloc(1, sizeof(newtorrents_container));
	rc = rsststorenewtorrentcontainer(ppstmt, *newtorrents);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Storing in newtor failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Done with Sqlite result set
	 */
	sqlite3_finalize(ppstmt);

	/*
	 * Close sandbox
	 */
	rc = rsstclosesandbox(sandbox);
	if(rc != 0){
		printf("Closing sandbox failed.\n");
		rsstwritelog(LOG_ERROR, "Closing sandbox failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	return 0;
}


/*
 * Get newtorrent information providing its ID
 * @Arguments
 * handle Swarmtv Handle
 * id id number of torrent to retrieve
 * newtorrent structure holding information
 * @Return
 * Returns 0 when found, -1 on not found or error
 */
int rsstnewtorrentsbyid(rsstor_handle *handle, int newtorid, newtorrents_struct *newtorrent)
{
  int            rc=0;
	sqlite3_stmt 	*ppstmt=NULL;
	char         	*zErrMsg=NULL;

  /*
   * Query To filter newtorrents on filter.
   */
	char *query="SELECT newtorrents.id, newtorrents.title, newtorrents.link, strftime('%s', newtorrents.pubdate), newtorrents.category, "
		"newtorrents.source, newtorrents.metatype, newtorrents.season, newtorrents.episode, newtorrents.seeds, "
    "newtorrents.peers, newtorrents.size "
    "FROM newtorrents "
		"WHERE newtorrents.id = ?1";

	/*
	 * Get records using query
	 */
	rc = rsstexecqueryresult(handle->db, &ppstmt, query, "d", newtorid);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

  /*
   * Did we find anything ?
   */
	if( SQLITE_DONE == sqlite3_step(ppstmt)) {
		rsstwritelog(LOG_ERROR, "No records found for id: %d %s:%d", newtorid, __FILE__, __LINE__);
    return -1;
  }

  /*
   * Store result in newtorrents struct
   */
  rsststorenewtorrentstruct(ppstmt, newtorrent);

  /*
   * Clean up
   */
	sqlite3_finalize(ppstmt);

  return 0;
}


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
int rsstfindnewtorrentsbytitle(rsstor_handle *handle, char *title, newtorrents_container **newtorrents, int limit, int offset)
{
  int            rc=0;
	sqlite3_stmt 	*ppstmt=NULL;
	char         	*zErrMsg=NULL;

  /*
   * Query To filter newtorrents on filter.
   */
	char *query="SELECT newtorrents.id, newtorrents.title, newtorrents.link, newtorrents.pubdate, newtorrents.category, "
		"newtorrents.source, newtorrents.metatype, newtorrents.season, newtorrents.episode, newtorrents.seeds, "
    "newtorrents.peers, newtorrents.size "
    "FROM newtorrents "
		"WHERE IREGEXP(?1, newtorrents.title) "
    "ORDER BY newtorrents.id DESC LIMIT ?2 OFFSET ?3"; // get values from downloaded table

	/*
	 * Get records using query
	 */
	rc = rsstexecqueryresult(handle->db, &ppstmt, query, "sdd", title, limit, offset);
	if( rc!=SQLITE_OK ){
		rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
		rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	/*
	 * Allocate and fill container
	 */
	*newtorrents = calloc(1, sizeof(newtorrents_container));
	rc = rsststorenewtorrentcontainer(ppstmt, *newtorrents);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Storing in newtor failed %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Done with Sqlite result set
	 */
	sqlite3_finalize(ppstmt);

  return 0;
}


/*
 * Delete content from newtorrents_container
 * @Arguments
 * container newtorrents container the content needs freeing
 * @Return
 * Returns 0 on success -1 on failure
 */
int rsstfreenewtorrentscontainer(newtorrents_container *container)
{
	int count=0;

	/*
	 * Sanity checks
	 */
	if(container == NULL || container->newtorrent == NULL) {
		return -1;
	}

	/*
	 * Free config values structures in container
	 */
	for(count=0; count < container->nr; count++)
	{
		rsstfreenewtor(container->newtorrent + count);
	}

	/*
	 * free the array itself
	 */
	free(container->newtorrent);

	/*
	 * NULL all pointers.
	 */
	memset(container, 0, sizeof(source_container));

	return 0;
}

