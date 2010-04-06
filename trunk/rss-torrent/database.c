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

#include "database.h"
#include "logfile.h"
#include "filesystem.h"
#include "regexp.h"
#include "setup.h"

/*
 * Database version.
 * When the current version does not match the version in the version field
 * The database is recreated
 */
#define DB_VERSION 2

/*
 * End of line character
 */
#define SCRIPT_EOL "\n"

/*
 * Memic to indicate comments.
 */
#define SCRIPT_SYM "--"

/*
 * Database create script version 2
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
"INSERT INTO 'version' VALUES(2);\n"
"\n"
"-- Create the newtorrents table\n"
"create table newtorrents (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE, "
"category TEXT, season INTEGER, episode INTEGER, seeds INTEGER DEFAULT 0, peers INTEGER DEFAULT 0, size INTEGER, source TEXT DEFAULT 'unknown', new TEXT DEFAULT 'Y');\n"
"\n"
"-- Create the downloaded table\n"
"create table downloaded (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE, "
"category TEXT, season INTEGER, episode INTEGER, date DATE);\n"
"\n"
"-- Create the filters table\n"
"create table filters (id INTEGER PRIMARY KEY, name TEXT UNIQUE, filter TEXT, nodouble TEXT DEFAULT '');\n"
"CREATE TABLE simplefilters (id INTEGER PRIMARY KEY, name TEXT UNIQUE, title TEXT, exclude TEXT, category TEXT, source TEXT, maxsize INTEGER DEFAULT 0, "
"minsize INTEGER DEFAULT 0, nodup TEXT NOT NULL, fromseason INTEGER DEFAULT 0, fromepisode INTEGER DEFAULT 0 );\n"
"\n"
"-- Create the sources table\n"
"create table sources (id INTEGER PRIMARY KEY, name TEXT UNIQUE, url TEXT, parser TEXT);\n"
"\n"
"-- Create the config table, and fillout the table with the 'standard' values\n"
"create table config (id INTEGER PRIMARY KEY, prop TEXT UNIQUE, value TEXT, descr TEXT);\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('torrentdir', '~/torrents', 'Path the downloaded torrents are placed in.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('logfile', '~/.rsstorrent/rsstorrent.log', 'Path to logfile.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('lockfile', '~/.rsstorrent/lockfile.pid', 'Path to lockfile.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('refresh', '3600', 'Seconds between refreshes.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('retain', '30', 'The number of days source information is retained.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('default_parser', 'defaultrss', 'The default rss filter to add to new rss sources.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_enable', 'N', '`Y` is send email notifications on new download, `N` is don`t.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_to', 'foo@bar.nl', 'Host to send the notifications to.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_from', 'user@somehost.nl', 'The from email-address in the mail headers.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('smtp_host', 'smtp.foobar.nl:25', 'The STMP server used to send the notifications.');\n"
"INSERT INTO 'config' (prop, value, descr) VALUES('min_size', '4000000', 'When size is smaller then this, download torrent and check.');\n"
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
"-- Up databaseversion from version 1 to 2\n"
"update version set version = '2';\n"
""
"COMMIT"
"\n";

/*
 * Executing script
 * This function executes a script.
 * Each line should be seperated by a '\n' char.
 * @Arguments 
 * script pointer so buffer holding script.
 * @ returns
 * 0 on succes
 * -1 on fail
 */
static int dbexecscript(sqlite3 *db, const char *script)
{
	char 	*latest=NULL;
	char 	*local=NULL;
	char 	*line=NULL;
	int		rc=0;
	int		retval=0;
	int		linenr=0;

	/*
	 * Copy to local buffer for strtok to modify
	 */
	rsstalloccopy(&local, script, strlen(script));

	/*
	 * Use strtok to seperate the lines
	 */
	char *strtok(char *str, const char *delim);
	line = strtok_r(local, SCRIPT_EOL, &latest);
	while(line != NULL && retval == 0){
		linenr++;

		/*
		 * Ignore comments
		 */
		if(strncmp(line, SCRIPT_SYM, strlen(SCRIPT_SYM)-1) == 0) {
			line = strtok_r(NULL, SCRIPT_EOL, &latest);
			continue;
		}

		/*
		 * Execute line by line
		 */
		rc = rsstexecutequery(db, line, NULL);
		if(rc < 0) {
			/*
			 * On error print report, and break loop.
			 */
			rsstwritelog(LOG_ERROR, "Error in script on line %d : %s", linenr, line);
			retval = -1;
		}

		/*
		 * Next line
		 */
		line = strtok_r(NULL, SCRIPT_EOL, &latest);
	}

	/*
	 * Done.
	 */
	free(local);
	return retval;
}


/*
 * This function implement pcre functionality to the queries.
 * called every time sqlite crosses an regexp.
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
   * Extract the values from the functioncall.
   */
  var1 = (const char*) sqlite3_value_text(sqlite3_value[0]);
  var2 = (const char*) sqlite3_value_text(sqlite3_value[1]);

	/*
	 * When NULL is provided, match failes.
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
            "occured: %d %s:%d", rc, __FILE__, __LINE__); 
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
 * This function implement pcre functionality to the queries.
 * called every time sqlite crosses an regexp.
 */
static void regexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value)
{
	genregexpfunc(db, num, sqlite3_value, 0);
}


/*
 * This function implement pcre functionality to the queries.
 * called every time sqlite crosses an regexp.
 * This function matches case insensative.
 */
static void iregexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value)
{
	genregexpfunc(db, num, sqlite3_value, PCRE_CASELESS);
}


/*
 * Get database version.
 * @arguments
 * version pointer to int holding database version number
 * @return
 * 0 on succes, -1 on failure
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
 * Create a complete database if needed, otherwise run updatescripts.
 * @arguments 
 * version current datbase version
 * @return
 * 0 on succes, -1 on failure
 */
static int fixdb(sqlite3 *db, int version) 
{
  int rc=0;

  /*
   * When version is < 1 database state unknown reinitialize whole db
   */
  if(version < 1) {
    rc = rsstrundbinitscript(db);
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

  /*
   * All updatescripts done, return result
   */
  return rc;
}

/*
 * Run the Database init script.
 * @return
 * 0 on succes, -1 on failure
 */
int rsstrundbinitscript(sqlite3 *db)
{
	int rc=0;

	/*
	 * Execute query
	 */
	rc = dbexecscript(db, dbinitscript); 

	/*
	 * return result
	 */
	return rc;
}


/*
 * Open database, and add regexp functionality.
 */
int rsstinitdatabase(
		const char *filename,   /* Database filename (UTF-8) */
		sqlite3   **ppDb)       /* OUT: SQLite db handle */
{
	int         rc=0; /* return code */
	int					version=0;
	char       *zErrMsg = 0;
	char       *dbpath = NULL;

	/*
	 * Complete the filename is it contains a ~ as homedir
	 */
	rsstcompletepath(filename, &dbpath);

	/*
   * Open the sqlite database.
   */
  rc = sqlite3_open(dbpath, ppDb);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*ppDb));
    sqlite3_close(*ppDb);
    return !SQLITE_OK;
  }

  /*
   * free dbpath
   */
  free(dbpath);

	/*
	 * Test if database is initialized and of the right version.
	 */
	rc = getdbversion(*ppDb, &version);
	if(rc != 0 || version != DB_VERSION){
		/*
		 * Create new DB
		 */
		printf("Running create databasescript.\n");
    rc = fixdb(*ppDb, version); 
		if(rc == -1){
			fprintf(stderr, "Can't open database, initscript failed!\n");
			sqlite3_close(*ppDb);
			return !SQLITE_OK;
		}
	}

  /* 
   * Add regexp function.
   */
  typedef struct sqlite3_value sqlite3_value;
  rc = sqlite3_create_function(
      *ppDb,
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
      *ppDb,
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
 * Do a query, and bind value's to the query in one step
 * This is a simplified version as it only returns 1 value.
 * that value is the first string of the first colomn.
 * the value returned must be a TEXT value.
 * the returned value will be put into text
 * make sure to free text after use
 * passing NULL to the fmt argument means no arguments.
 * @Arguments
 * text retrieved from the query
 * query pointer to the query string
 * fmt arguments pointer
 * ... other arguments
 * @return
 * 0 on succes, -1 on failure
 */
int rsstdosingletextquery(sqlite3 *db, const unsigned char **text, const char *query, char *fmt, ...) 
{
  sqlite3_stmt 	*ppStmt=NULL;
  const char 		*pzTail=NULL;
  int         	rc=0;
  int         	step_rc=0;
  char       		*zErrMsg = 0;
  const unsigned char  *temptext=NULL;
  va_list     	ap;
  int         	retval=0;
  int         	count=0;
  char        	*s=NULL;
  int          	d=0;
  double       	f=0.0;

  /*
   * NULL = no arguments.
   */
  if(fmt == NULL) {
    fmt = "";
  }

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    retval = -1;
  }

  /*
   * Handle the arguments
   */
  if(retval == 0) {
    va_start(ap, fmt);
    while (*fmt != '\0' && retval == 0){
      count++; // next item
      switch(*fmt++) {
        case 's':            /* string */
          s = va_arg(ap, char *);
          rc = sqlite3_bind_text(ppStmt, count, s, -1, SQLITE_TRANSIENT);
          if( rc!=SQLITE_OK ){
            rsstwritelog(LOG_ERROR, "sqlite3_bind_text failed on argument '%d'\n'%s'\n'%s' %s:%d", 
                count, query, fmt, __FILE__, __LINE__);  
            rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
            sqlite3_free(zErrMsg);
            retval=-1;
          }
          break;
        case 'd':            /* int */
          d = va_arg(ap, int);
          rc = sqlite3_bind_int(ppStmt, count, d);
          if( rc!=SQLITE_OK ){
            rsstwritelog(LOG_ERROR, "sqlite3_bind_int failed on argument '%d'\n'%s'\n'%s' %s:%d",
                count, query, fmt, __FILE__, __LINE__);  
            rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
            sqlite3_free(zErrMsg);
            retval=-1;
          }
          break;
        case 'f':            /* int */
          f = va_arg(ap, double);
          rc = sqlite3_bind_double(ppStmt, count, f);
          if( rc!=SQLITE_OK ){
            rsstwritelog(LOG_ERROR, "sqlite3_bind_double failed on argument '%d'\n'%s'\n'%s' %s:%d",
                count, query, fmt, __FILE__, __LINE__);  
            rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
            sqlite3_free(zErrMsg);
            retval=-1;
          }
          break;
        default:
          rsstwritelog(LOG_ERROR, "Unknown format '%c' on position '%d'\nQuery: '%s'\nFmt: '%s'",
              *fmt, count, query, fmt);
          retval=-1;
      }
    }
    va_end(ap);
  }

  /* 
   * Get the first value discard the others.
   */
  if(retval == 0) {
    /*
     * Execute query
     */
    step_rc = sqlite3_step(ppStmt);

    switch(step_rc){
      case SQLITE_ROW:
        /*
         * context to output
         */
        temptext = sqlite3_column_text(ppStmt, 0);

        /*
         * Move result to premanent own location.
         */
				rsstalloccopy((char**)text,(char*) temptext, strlen((char*)temptext));

        break;
      case SQLITE_DONE:
        *text=NULL;
        break;
      default:
        rsstwritelog(LOG_ERROR, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
        rsstwritelog(LOG_ERROR, "in statement : \'%s\'", query);

        *text=NULL;
        retval=-1;
        break;
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  return retval;
}

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
int rsstexecutequery(sqlite3 *db, const char *query, char *fmt, ...) 
{
  sqlite3_stmt 	*ppStmt=NULL;
  const char 		*pzTail=NULL;
  va_list     	ap;
  int         	rc=0;
  int         	retval=0; 
  int        		step_rc=0;
  char       		*zErrMsg = 0;
  char        	*s=NULL;
  int          	d=0;
  double       	f=0.0;
  int          	count=0;
  int          	changes=0;

  /*
   * fmt pointer to NULL is do not substitutes
   */
  if(fmt == NULL){
    fmt = "";
  }

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    retval=-1;
  }

  /*
   * Handle the arguments
   */
  va_start(ap, fmt);
  while (*fmt != '\0' && retval == 0){
    count++; // next item
    switch(*fmt++) {
      case 's':            /* string */
        s = va_arg(ap, char *);
        rc = sqlite3_bind_text(ppStmt, count, s, -1, SQLITE_TRANSIENT);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_text failed on argument '%d'\n'%s'\n'%s' %s:%d", 
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'd':            /* int */
        d = va_arg(ap, int);
        rc = sqlite3_bind_int(ppStmt, count, d);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_int failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'f':            /* int */
        f = va_arg(ap, double);
        rc = sqlite3_bind_double(ppStmt, count, f);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_double failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      default:
        rsstwritelog(LOG_ERROR, "Unknown format '%c' on position '%d'\nQuery: '%s'\nFmt: '%s'",
          *fmt, count, query, fmt);
        retval=-1;
    }
  }
  va_end(ap);

  /* 
   * Get the first value discard the others.
   */
  if(retval == 0){
    step_rc = sqlite3_step(ppStmt);
    switch(step_rc){
      case SQLITE_ROW:
        retval=ROWS_FOUND;
        break;
      case SQLITE_DONE:
        retval=ROWS_EMPTY;
        break;
      case SQLITE_CONSTRAINT:
        retval=ROWS_CONSTRAINT; 
        break;
      default:
        rsstwritelog(LOG_ERROR, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
        rsstwritelog(LOG_ERROR, "in statement : \'%s\'", query);
        retval=ROWS_ERROR;
    }
  }

  /*
   * If rows were changes return ROWS_CHANGED
   */
  if(retval == 0){
    changes = sqlite3_changes(db);
    if(changes > 0){
      retval=ROWS_CHANGED;
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  return retval;
}

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
int rsstexecqueryresult(sqlite3 *db, sqlite3_stmt **ppstmt, const char *query, char *fmt, ...)
{
	int rc=0;
	va_list ap;

	/*
	 * Execute real function
	 */
	va_start(ap, fmt);
	rc = rsstexecqueryresultva(db, ppstmt, query, fmt, ap);
	va_end(ap);

	/*
	 * Done
	 */
	return rc;
}

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
int rsstexecqueryresultva(sqlite3 *db, sqlite3_stmt **ppstmt, const char *query, char *fmt, va_list ap)
{
  //sqlite3_stmt 	*ppStmt=NULL;
  const char 		*pzTail=NULL;
  int         	rc=0;
  int         	retval=0; 
  char       		*zErrMsg = 0;
  char        	*s=NULL;
  int          	d=0;
  double       	f=0.0;
  int          	count=0;

  /*
   * fmt pointer to NULL is do not substitutes
   */
  if(fmt == NULL){
    fmt = "";
  }

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      ppstmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    retval=-1;
  }

  /*
   * Handle the arguments
   */
  while (*fmt != '\0' && retval == 0){
    count++; // next item
    switch(*fmt++) {
      case 's':            /* string */
        s = va_arg(ap, char *);
        rc = sqlite3_bind_text(*ppstmt, count, s, -1, SQLITE_TRANSIENT);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_text failed on argument '%d'\n'%s'\n'%s' %s:%d", 
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'd':            /* int */
        d = va_arg(ap, int);
        rc = sqlite3_bind_int(*ppstmt, count, d);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_int failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'f':            /* int */
        f = va_arg(ap, double);
        rc = sqlite3_bind_double(*ppstmt, count, f);
        if( rc!=SQLITE_OK ){
          rsstwritelog(LOG_ERROR, "sqlite3_bind_double failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      default:
        rsstwritelog(LOG_ERROR, "Unknown format '%c' on position '%d'\nQuery: '%s'\nFmt: '%s'",
          *fmt, count, query, fmt);
        retval=-1;
    }
  }

  return retval;
}

/*
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * Arguments
 * query	query to print.
 * Return
 * return 0 when okay.
 * return -1 on error.
 */
int rsstprintquery(sqlite3 *db, const char *query, char *fmt, ...)
{
  sqlite3_stmt  *ppstmt=NULL;
  int           rc=0;
  int           step_rc=0;
  int           cols=0;
  char          *zErrMsg=0;
  int           count=0;
  const unsigned char *text=NULL;
	va_list				ap;

  /*
   * Prepare the sqlite statement
   */
	va_start(ap, fmt);
	rc = rsstexecqueryresultva(db, &ppstmt, query, fmt, ap);
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }
	va_end(ap);

  /*
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  cols = sqlite3_column_count(ppstmt);

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppstmt))) {

    for(count=0; count<cols; count++){
      /*
       * Print the content of the row
       */
      text = sqlite3_column_text(ppstmt, count);
      printf("%-25s", text);
      if(count+1 < cols){
        printf(" : ");
      }
    }

    /*
     * new line at end of record.
     */
    if(step_rc == SQLITE_ROW){
      printf("\n");
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);

  /*
   * All gone well
   */
  return rc;
}


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
int rsstprintquerylist(sqlite3 *db, const char *query, char *names[], char *fmt, ...)
{
  sqlite3_stmt  *ppstmt=NULL;
  int           rc=0;
  int           step_rc=0;
  int           cols=0;
  char          *zErrMsg=0;
  int           count=0;
  const unsigned char *text=NULL;
	va_list				ap;

  /*
   * Prepare the sqlite statement
   */
	va_start(ap, fmt);
	rc = rsstexecqueryresultva(db, &ppstmt, query, fmt, ap);
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }
	va_end(ap);

  /*
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  cols = sqlite3_column_count(ppstmt);

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppstmt))) {

    for(count=0; count<cols; count++){
      /*
       * Print the content of the row
       */
      if(count == 0){
        printf("%s\n", names[cols]);
      }
      text = sqlite3_column_text(ppstmt, count);
      printf("%-15s : %s\n", names[count], text);
    }

    /*
     * new line at end of record.
     */
    if(step_rc == SQLITE_ROW){
      printf("\n");
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppstmt);

  /*
   * All gone well
   */
  return rc;
}
