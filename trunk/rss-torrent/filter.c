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
#include <pcre.h>
#include <sqlite3.h>

#include "curlfile.h"
#include "database.h"
#include "logfile.h"
#include "rssfilter/rsstorrent/rsstorrentfilter.h"
#include "rssfilter/defaultrss/defaultrss.h"
#include "rssfilter/twitter/twitter.h"

#define  MAXLENGHT 400

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printfilters(sqlite3 *db) 
{
  /*
   * header
   */
  printf("#############\n");
  printf("Filters.\n");
  printf("#############\n");

  printquery(db, "select name, filter, nodouble from 'filters'");

  /*
   * Footer
   */
  printf("\n#############\n");
}

/*
 * Del all filters.
 * Deletes all filters from filtertable.
 * On succes 0 is returned.
 */
int delallfilters(sqlite3 *db)
{
  int           rc;

  /*
   * Init query
   */
  const char* query = "delete from 'filters'";

  printf("Attempting to delete all filters \n");

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = executequery(db, query, NULL);
  switch(rc) {
    case(ROWS_CHANGED):
      return 0;
      break;
    case(ROWS_EMPTY):
      printf("No filters left, delete all did nothing %s:%d\n", __FILE__, __LINE__);
      writelog(LOG_ERROR, "No filters left, delete all did nothing %s:%d", __FILE__, __LINE__);
      return -1;
      break;
    default: 
      writelog(LOG_ERROR, "Query error during delallfilters %s:%d",  __FILE__, __LINE__);
      return -1;
  }
}

/*
 * del filter item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int delfilter(sqlite3 *db, const char *name)
{
  int         rc=0;

  /*
   * Init query
   */
  const char* query = "delete from 'filters' where name=?1";

  printf("Attempting to delete filter '%s'\n", name);

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = executequery(db, query, "s", name);
  switch(rc) {
    case(ROWS_CHANGED):
      return 0;
      break;
    case(ROWS_EMPTY):
      fprintf(stderr, "Could not deletei filter '%s' %s:%di\n", name,  __FILE__, __LINE__);
      writelog(LOG_ERROR, "Could not delete filter '%s' %s:%d", name,  __FILE__, __LINE__);
      return -1;
      break;
    default: 
      writelog(LOG_ERROR, "Query error during delfilter %s:%d",  __FILE__, __LINE__);
      return -1;
  }
}

/*
 * Check Filter
 * When the filter exists, return 1
 * else 0
 */
static int checkfilter(sqlite3 *db, const char *name)
{
  int rc;

  char *query = "select * from filters where name=?1";

  /*
   * execute query
   */
  rc = executequery(db, query, "s", name);

  return rc;
}

/*
 * Add filter item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addfilter(sqlite3 *db, const char *name, const char *filter, const char *doublefilter)
{
  int         rc;
  const char *locdouble; // holds pointer to doublefilter or empty
  char        query[MAXLENGHT+1];

  /*
   * No filters can be added that have the name all.
   */
  if(strcmp(name, "all") == 0){
    writelog(LOG_NORMAL, "No filter can be added with name 'all'");
    fprintf(stderr, "No filter can be added with name 'all'\n");
    return -1;
  }

  /*
   * Init query, to an insert when not found, to a update when existing
   */
  memset(query, 0, MAXLENGHT+1);
  rc = checkfilter(db, name);
  switch(rc) {
    case 0:
      strncpy(query, "INSERT INTO 'filters' (name, filter, nodouble) VALUES(?1, ?2, ?3)", MAXLENGHT);
      break;
    case 1:
      writelog(LOG_NORMAL, "filter '%s' exists, updating.", name);
      printf("filter '%s' exists, updating.\n", name);
      strncpy(query, "UPDATE 'filters' SET filter=?2, nodouble=?3 WHERE name=?1", MAXLENGHT);
      break;
    default:
      writelog(LOG_ERROR, "Filter table corrupt !! %s:%d", __FILE__, __LINE__);
      fprintf(stderr, "Filter table in database corrupt!\n");
      return -1;
  }

  /*
   * Debug
   */
  writelog(LOG_NORMAL, "new filter name : %s", name);
  writelog(LOG_NORMAL, "filter : %s", filter);
  writelog(LOG_NORMAL, "nodouble filter : %s", doublefilter);

  /*
   * if no 'no double' filter is provided insert an empty string
   */
  if(doublefilter == NULL){
    locdouble = "";
  } else {
    locdouble = doublefilter;
  }

  /*
   * Execute query to add filter
   * When no rows are changed return error.
   */
  rc = executequery(db, query, "sss", name, filter, doublefilter);
  if(rc != ROWS_CHANGED) {
    writelog(LOG_ERROR, "No rows changed, inserting filter failed. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * All gone well.
   */
  return 0;
}

/*
 * Apply a filter to the downloaded RSS code.
 * This routine holdes the refferences to different kind of filters.
 * (For now only rsstorrent.com format)
 */
int filterdownload(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
  int rc;

  /*
   * compare the filter string and pass the downloaded file to the correct filtering routine.
   */
  if(strcmp(filter, "rsstorrent") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = rsstorrentfilter(db, name, url, filter, rssfile); 
    return 0;
  }

  if(strcmp(filter, "defaultrss") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = defaultrss(db, name, url, filter, rssfile); 
    return 0;
  }

  if(strcmp(filter, "twitter") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = twitter(db, name, url, filter, rssfile); 
    return 0;
  }

  /*
   * When no filter found.
   */
  writelog(LOG_ERROR, "No filter found '%s', ignoring file %s:%d", filter, __FILE__, __LINE__);
  return -1;
}

/*
 * Print filter in a way it could be modified and reentered
 */
void printshellfilter(sqlite3 *db, char *appname, char *filtername)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           cols;
  char          *zErrMsg = 0;
  const unsigned char *filterstring;
  const unsigned char *nodoublestring;

  char *query =  "select filter, nodouble from 'filters' where name=?1";

  // Get filter from filters

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
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return;
  }

  /*
   * bind property and value to the query
   */
  rc = sqlite3_bind_text(ppStmt, 1, filtername, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d" __FILE__, __LINE__);  
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
    printf("Filter with name '%s' not found\n", filtername);
    return;
  }

  /*
   * Get Values
   */
  filterstring = sqlite3_column_text(ppStmt, 0);
  nodoublestring = sqlite3_column_text(ppStmt, 1);

  /*
   * Print the shell line 
   */
  printf ( "#%s \\\n -F \"%s:%s\" \\\n -T \"%s\"\n", 
    appname, filtername,  filterstring, nodoublestring);

  /*
   * Done with query, finalizing.
   */
  sqlite3_finalize(ppStmt);

  /*
   * All gone well
   */
  return;
  
  // print filter shell line 
}
