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
#include <time.h>
#include <sqlite3.h>

#include "types.h"
#include "logfile.h"
#include "regexp.h"
#include "handleopts.h"
#include "database.h"
#include "torrentdownload.h"
#include "simplefilter.h"

/*
 * Nodup filternames
 */
#define NODUP_NONE_NAME     "none"
#define NODUP_LINK_NAME     "link"
#define NODUP_UNIQUE_NAME   "unique"
#define NODUP_NEWER_NAME    "newer"

/*
 * Nodup filters are defined here
 */
#define NODUP_NONE    ""
#define NODUP_LINK    "SELECT title FROM downloaded WHERE link=?1"
#define NODUP_UNIQUE  "SELECT title FROM downloaded WHERE link=?1 OR (season=?2 AND episode=?3 AND IREGEXP('REPLACE_TITLE', title))"
#define NODUP_NEWER   "SELECT title FROM downloaded WHERE link=?1 OR ((season=?2 AND episode>=?3) OR season>?2) AND IREGEXP('REPLACE_TITLE', title)"

/*
 * Filter that is used to convert the simple filter into SQL.
 */
static char *sqlfilter="SELECT link, title, pubdate, category, season, episode FROM newtorrents WHERE "
		"IREGEXP(?1, title) AND "
		"(size < ?2 OR ?2 = 0) AND "
		"(size > ?3 OR ?3 = 0) AND "
		"(season >= ?4 OR ?4 = 0) AND "
		"(season > ?4 OR episode >= ?5 OR ?5 = 0) AND "
		"(NOT IREGEXP(?6, title) OR ?6 = '') AND "
		"IREGEXP(?7, category) AND "
		"IREGEXP(?8, source) AND "
		"new = 'Y'";


/*
 * name is name of the inodup filter 
 * title is the titlefilter to insert
 * **nodup is the allocated buffer that is returned.
 * Nohup *inodup should be passed with value NULL
 * Returns 0 on succes, -1 when no fitting filter was found.
 */
static int findnodup(char *name, char *title, char **nodup)
{
  *nodup=NULL;

  /*
   * Find the correct filter.
   */
  if(strcmp(name, NODUP_NONE_NAME) == 0)
  {
    rsstalloccopy(nodup, NODUP_NONE, strlen(NODUP_NONE));
  }
  if(strcmp(name, NODUP_LINK_NAME) == 0)
  {
    rsstalloccopy(nodup, NODUP_LINK, strlen(NODUP_LINK));
  }
  if(strcmp(name, NODUP_UNIQUE_NAME) == 0)
  {
    rsstalloccopy(nodup, NODUP_UNIQUE, strlen(NODUP_UNIQUE));
  }
  if(strcmp(name, NODUP_NEWER_NAME) == 0)
  {
    rsstalloccopy(nodup, NODUP_NEWER, strlen(NODUP_NEWER));
  }

  /*
   * When no valid nodup name is provided, return -1
   */
  if(*nodup == NULL){
    return -1;
  }

  /*
   * Insert the correct TITLE filter
   */
  rsststrrepl(nodup, "REPLACE_TITLE", title);

  return 0;
}


/*
 * Validate Arguments
 * takes opts_struct struct as argument
 * return on correct arguments, -1 on invalid arguments
 */
static int validearguments(opts_struct *opts)
{
  int   rc=0;
  int   retval=0;
  char  *nodup=NULL;

  /*
   * Validate nodup argument
   */
  rc = findnodup(opts->simplenodup, opts->simpletitle, &nodup);
  free(nodup);
  if(rc == -1) {
    rsstwritelog(LOG_ERROR, "Nodup name '%s' is not valid. %s:%d", opts->simplenodup, __FILE__, __LINE__);
    fprintf(stderr, "Nodup name '%s' is not valid.", opts->simplenodup);
    retval=-1;
  }

  return retval;
}


/*
 * optstosimple
 * Takes takes a opts_struct argument and a simplefilter_struct as argument.
 * returns 0 on succes, -1 on error.
 */
int rsstoptstosimple(opts_struct *opts, simplefilter_struct *simple)
{
  /*
   * Copy strings
   */
  rsstalloccopy(&(simple->name),   opts->simplename,   strlen(opts->simplename));
  rsstalloccopy(&(simple->nodup),  opts->simplenodup,  strlen(opts->simplenodup));

  /*
   * Title filter is optional
   */
  if(opts->simpletitle != NULL) {
    rsstalloccopy(&(simple->title),  opts->simpletitle,  strlen(opts->simpletitle));
  } else {
    rsstalloccopy(&(simple->title), "", 1);
  }
  if(opts->simpleexclude != NULL) {
    rsstalloccopy(&(simple->exclude),  opts->simpleexclude,  strlen(opts->simpleexclude));
  } else {
    rsstalloccopy(&(simple->exclude), "", 1);
  }
  if(opts->simplecategory != NULL) {
    rsstalloccopy(&(simple->category),  opts->simplecategory,  strlen(opts->simplecategory));
  } else {
    rsstalloccopy(&(simple->category), "", 1);
  }
  if(opts->simplesource != NULL) {
    rsstalloccopy(&(simple->source),  opts->simplesource,  strlen(opts->simplesource));
  } else {
    rsstalloccopy(&(simple->source), "", 1);
  }
  if(opts->simpleseason != NULL) {
    simple->fromseason=atoi(opts->simpleseason); 
  } 
  if(opts->simpleepisode != NULL) {
    simple->fromepisode=atoi(opts->simpleepisode); 
  } 

  /*
   * Convert units 
   */
  if(opts->simplemaxsize != NULL) {
    rssthumantosize(opts->simplemaxsize, &(simple->maxsize));
  } else {
    simple->maxsize = 0;
  }
  if(opts->simpleminsize != NULL) {
    rssthumantosize(opts->simpleminsize, &(simple->minsize));
  } else {
    simple->minsize = 0;
  }

  /*
   * Done
   */
  return 0;
}


/*
 * Free simplefilter_struct content
 * returns nothing
 * The structure itself is not freed only the content inside
 */
static void freestructsimple(simplefilter_struct *simple)
{
  /*
   * free strings
   */
  free(simple->name);
  free(simple->title);
  free(simple->exclude);
  free(simple->category);
  free(simple->nodup);

  /*
   * Null all
   */
  memset(simple, 0, sizeof(simplefilter_struct));
}


/*
 * Add simple filter adds the filter to the database
 * Arguments  : simplefilter_struct * 
 * returns    : 0 when added succesfully
 * returns    : -1 when adding failed
 */
int rsstinsertsimplefilter(rsstor_handle *handle, simplefilter_struct *simple)
{
  int 			rc=0;
	sqlite3  *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Query and format used to insert the simple filter into the database
   */
  static char *query = "insert into 'simplefilters' "
		"(name, title, exclude, category, maxsize, minsize, nodup, fromseason, fromepisode, source) "
		"VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)";
  static char *fmt   = "ssssffsdds";

  /*
   * Call database execute query function
   */
  rc = rsstexecutequery(db, query, fmt, 
      simple->name,
      simple->title,
			simple->exclude,
			simple->category,
      simple->maxsize,
      simple->minsize,
      simple->nodup,
      simple->fromseason,
      simple->fromepisode,
			simple->source);

  /*
   * Handle output
   */
  //if(rc != 1) {
  if(rc < 1) {
    return -1;
  } else {
    return 0;
  }
}


/*
 * Check Filter
 * When the filter exists, return 1
 * else 0
 */
static int checksimple(sqlite3 *db, const char *name)
{
  int rc;

  char *query = "select * from simplefilters where name=?1";

  /*
   * execute query
   */
  rc = rsstexecutequery(db, query, "s", name);

  return rc;
}


/*
 * Add simple filter
 * returns 0 on succes, else -1
 */
int rsstaddsimplefilter(rsstor_handle *handle, opts_struct *opts)
{
  int rc=0;
  int retval=0;
  simplefilter_struct simple;
	sqlite3 *db=NULL;

  memset(&simple, 0, sizeof(simplefilter_struct));

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Make sure the entries are valid.
   */
  rc = validearguments(opts);
  if(rc != 0){
    return -1;
  }

  /*
   * Translate argument to usable data
   */
  rc = rsstoptstosimple(opts, &simple);
  if(rc != 0){
    retval=-1;
  }

  /*
   * When allready there delete the previous simple filter
   */
  rc = checksimple(db, opts->simplename);
  if(rc == 1){
    rsstdelsimple(handle, opts->simplename);
  }

  /*
   * Add Record
   */
  rsstinsertsimplefilter(handle, &simple);

  /*
   * Done.
   */
  freestructsimple(&simple);
  return 0;
}


/*
 * List Simple filters.
 * @Arguments
 * handle RSS-torrent handle
 */
void rsstlistsimple(rsstor_handle *handle)
{
  int rc=0;
	sqlite3 *db=NULL;

	/*
	 * get db pointer
	 */
	db = handle->db;

  /*
   * Query to list Simple filters
   */
  const char *query="select name from simplefilters";

  printf( "Please use --print-simple to see the content of the fiters.\n"
  "#############\n"
  "Filters.\n"
  "#############\n");


  rc = rsstprintquery(db, query, NULL);
  if(rc != 0) {
    fprintf(stderr, "Listing simple filters failed!\n");
  }

  printf("#############\n");
}


/*
 * del filter item
 * When the name is not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstdelallsimple(rsstor_handle *handle)
{
  int         rc=0;
	sqlite3    *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Init query
   */
  const char* query = "delete from 'simplefilters'";

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = rsstexecutequery(db, query, NULL);
  switch(rc) {
    case(ROWS_CHANGED):
      return 0;
      break;
    case(ROWS_EMPTY):
      fprintf(stderr, "No simplefilters in list.\n");
      rsstwritelog(LOG_ERROR, "No simplefilters in list.\n");
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during delallfilter %s:%d",  __FILE__, __LINE__);
      return -1;
  }
}

/*
 * Print all simple filters in shell format.
 */
void rsstprintallsimple(rsstor_handle *handle)
{
  sqlite3_stmt  *ppStmt=NULL;
  const char    *pzTail=NULL;
  int           rc=0;
  int           step_rc=0;
  int           cols=0;
  char          *zErrMsg=0;
  int           count=0;
  const unsigned char *text=NULL;
	sqlite3				*db=NULL;

	char *query="select name from simplefilters";

	/*
	 * Get db pointer
	 */
	db=handle->db;

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
    return;
  }

  /*
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  cols = sqlite3_column_count(ppStmt);


  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
		/*
		 * Print the content of the row
		 */
		text = sqlite3_column_text(ppStmt, count);
		rsstprintsimple(handle, (char*) text);
	}

	/*
	 * Done with query, finalizing.
	 */
  rc = sqlite3_finalize(ppStmt);
}


/*
 * del filter item
 * When the name is not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstdelsimple(rsstor_handle *handle, const char *name)
{
  int         rc=0;
	sqlite3		 *db=NULL;

  /*
   * Init query
   */
  const char* query = "delete from 'simplefilters' where name=?1";

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = rsstexecutequery(db, query, "s", name);
  switch(rc) {
    case(ROWS_CHANGED):
      return 0;
      break;
    case(ROWS_EMPTY):
      fprintf(stderr, "Could not delete filter '%s' %s:%di\n", name,  __FILE__, __LINE__);
      rsstwritelog(LOG_ERROR, "Could not delete filter '%s' %s:%d", name,  __FILE__, __LINE__);
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during delfilter %s:%d",  __FILE__, __LINE__);
      return -1;
  }
}


/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 */
void rsstprintsimple(rsstor_handle *handle, char *filtername)
{
  sqlite3_stmt  			*ppStmt=NULL;
  const char    			*pzTail=NULL;
  int           			rc=0;
  int           			cols=0;
  char          			*zErrMsg = 0;
  const unsigned char *titlestring=NULL;
  unsigned char 			maxsizestring[BUFSIZE+1];
  double              maxsizedouble=0;
  unsigned char 			minsizestring[BUFSIZE+1];
  double              minsizedouble=0;
  const unsigned char *nodupstring=NULL;
  unsigned int        seasonint=0;
  unsigned int        episodeint=0;
  const unsigned char *excludestring=NULL;
  const unsigned char *categorystring=NULL;
  const unsigned char *sourcestring=NULL;
	sqlite3							*db=NULL;


  char *query =  "select title, maxsize, minsize, nodup, fromseason, fromepisode, exclude, category, source from 'simplefilters' where name=?1";

	/*
	 * Get db pointer
	 */
	db=handle->db;

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
    return;
  }

  /*
   * bind property and value to the query
   */
  rc = sqlite3_bind_text(ppStmt, 1, filtername, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d" __FILE__, __LINE__);  
    return;
  }

  /*
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  cols = sqlite3_column_count(ppStmt);

  /*
   * Execute query.
   */
  rc = sqlite3_step(ppStmt);
  if(rc == SQLITE_DONE) {
    printf("Simple Filter with name '%s' not found\n", filtername);
    return;
  }

  /*
   * Get Values
   */
  titlestring     = sqlite3_column_text(ppStmt,   0);
  maxsizedouble   = sqlite3_column_double(ppStmt, 1);
  minsizedouble   = sqlite3_column_double(ppStmt, 2);
  nodupstring     = sqlite3_column_text(ppStmt,   3);
  seasonint       = sqlite3_column_int(ppStmt,    4);
  episodeint      = sqlite3_column_int(ppStmt,    5);
  excludestring   = sqlite3_column_text(ppStmt,   6);
  categorystring  = sqlite3_column_text(ppStmt,   7);
  sourcestring  	= sqlite3_column_text(ppStmt,   8);

  /*
   * Print the components that are set
   */
  printf("rsstorrent --add-simple='%s' --nodup='%s' ", filtername, nodupstring); 

  if(strlen((char*)titlestring) > 0) {
    printf("--title='%s' ", titlestring);
  }

  /*
   * Exclude
   */
  if(strlen((char*)excludestring) != 0){
    printf("--exclude='%s' ", excludestring);
  }

  /*
   * Category
   */
  if(strlen((char*)categorystring) != 0){
    printf("--category='%s' ", categorystring);
  }

  /*
   * Category
   */
  if(strlen((char*)sourcestring) != 0){
    printf("--source='%s' ", sourcestring);
  }

  /*
   * Maxsize
   */
  if(maxsizedouble != 0){
    rsstsizetohuman(maxsizedouble, (char*) maxsizestring);

    printf("--max-size='%s' ", maxsizestring);
  }

  /*
   * Minsize
   */
  if(minsizedouble != 0){
    rsstsizetohuman(minsizedouble, (char*) minsizestring);

    printf("--min-size='%s' ", minsizestring);
  }

  /*
   * From season
   */
  if(seasonint != 0) {
    printf("--from-season='%d' ", seasonint);
  }

  /*
   * From episode
   */
  if(episodeint != 0) {
    printf("--from-episode='%d' ", episodeint);
  }

  /*
   * Close print row
   */
  putchar('\n');

  /*
   * Done with query, finalizing.
   */
  sqlite3_finalize(ppStmt);
}


/*
 * Apply filters
 * Runs through all filters in simplefilters table.
 * Calls SQL filters routines for further handling.
 * arguments :
 * db pointer to db to use
 * simultate 0 for real behaviour, 1 for simulation mode.
 */
int rsstdownloadsimple(sqlite3 *db, SIM simulate)
{
  sqlite3_stmt  *ppStmt=NULL;
  const char    *pzTail=NULL;
  int           rc=0;
  int           step_rc=0;
  char          *zErrMsg = 0;
  char          *name=NULL;
  char          *title=NULL;
  char          *exclude=NULL;
  char          *category=NULL;
  char          *source=NULL;
  char          *nodup=NULL;
 	double        maxsize=0.0;
  double        minsize=0.0;
  int           season=0;
  int           episode=0;
  char          *sqlnodup=NULL;


	/*
	 * Query to retrieve filters from simplefilters table.
	 */ 
	char *query = "select name, title, maxsize, minsize, nodup, fromseason, fromepisode, exclude, category, source from simplefilters";

	/*
	 * Prepare the sqlite statement
	 */
	rc = sqlite3_prepare_v2(
			db,                 /* Database handle */
      query,            	/* SQL statement, UTF-8 encoded */
      strlen(query),    	/* Maximum length of zSql in bytes. */
      &ppStmt,            /* OUT: Statement handle */
      &pzTail             /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
    sqlnodup=NULL;

		/*
		 * Get name and query of the filters
     */
    name     = (char*) sqlite3_column_text(ppStmt, 0);
    title    = (char*) sqlite3_column_text(ppStmt, 1);
    maxsize  = sqlite3_column_double(ppStmt, 2);
    minsize  = sqlite3_column_double(ppStmt, 3);
    nodup    = (char*) sqlite3_column_text(ppStmt, 4);
    season   = sqlite3_column_int(ppStmt,    5);
    episode  = sqlite3_column_int(ppStmt,    6);
    exclude  = (char*) sqlite3_column_text(ppStmt, 7);
    category = (char*) sqlite3_column_text(ppStmt, 8);
    source   = (char*) sqlite3_column_text(ppStmt, 9);

    /*
     * Generate SQL-filter and SQL-nodup
     */
    rc = findnodup(nodup, title, &sqlnodup);
    if(rc != 0) {
    free(sqlnodup);
      rsstwritelog(LOG_ERROR, "Simple filter '%s' does not have a valid nodup value. %s:%d", name, __FILE__, __LINE__);
      continue;
    }

		/*
		 * Log SQL used for handling filters.
		 */
		rsstwritelog(LOG_DEBUG, "%s : %s", name, sqlfilter);
		rsstwritelog(LOG_DEBUG, "%s : %s", name, sqlnodup);

    /*
     * call apply filter
     */
    rsstapplyfilter(db, name, sqlnodup, simulate, sqlfilter, 
				"sffddsss", title, maxsize, minsize, season, episode, exclude, category, source);

    /*
     * Cleanup
     */
    free(sqlnodup);
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  /*
   * All gone well
   */
  return 0;
}

