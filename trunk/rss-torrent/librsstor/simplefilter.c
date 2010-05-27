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
static int validearguments(simplefilter_struct *filter)
{
  int   rc=0;
  int   retval=0;
  char  *nodup=NULL;

  /*
   * Validate nodup argument
   */
  rc = findnodup(filter->nodup, filter->title, &nodup);
  free(nodup);
  if(rc == -1) {
    rsstwritelog(LOG_ERROR, "Nodup name '%s' is not valid. %s:%d", filter->nodup, __FILE__, __LINE__);
    fprintf(stderr, "Nodup name '%s' is not valid.", filter->nodup);
    retval=-1;
  }

  return retval;
}


#if 0
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
   * NULL all
   */
  memset(simple, 0, sizeof(simplefilter_struct));
}
#endif


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
int rsstaddsimplefilter(rsstor_handle *handle, simplefilter_struct *simple)
{
  int rc=0;
	sqlite3 *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Make sure the entries are valid.
   */
  rc = validearguments(simple);
  if(rc != 0){
    return -1;
  }

  /*
   * When allready there delete the previous simple filter
   */
  rc = checksimple(db, simple->name);
  if(rc == 1){
    rsstdelsimple(handle, simple->name);
  }

  /*
   * Add Record
   */
  rsstinsertsimplefilter(handle, simple);

	/*
	 * Done
	 */
  return 0;
}

#if 0
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
#endif


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

