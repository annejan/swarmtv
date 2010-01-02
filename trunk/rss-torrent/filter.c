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
#include "rssfilter/eztv/eztvfilter.h"

#define  MAXLENGHT 400

/*
 * Get value of a config object.
 */
int getfilter(sqlite3 *db, char *prop, char **url) 
{
  char  query[MAXLENGHT+1];
  int   rc;

  snprintf(query, MAXLENGHT, "select 'filter' from 'filters' where 'name' = '%s'", prop);

  rc =  dosingletextquery(db, query, (unsigned char const**) url);

  return rc;
}

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
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  char          *zErrMsg = 0;
  int           rc;

  /*
   * Init query
   */
  const char* query = "delete from 'filters'";

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,              /* SQL statement, UTF-8 encoded */
      strlen(query),      /* Maximum length of zSql in bytes. */
      &ppStmt,            /* OUT: Statement handle */
      &pzTail             /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d",  __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    writelog(LOG_ERROR, "Could not delete all filters %s:%d", __FILE__, __LINE__);
    //return -1;
  }

  /*
   * Free statement
   */
  rc = sqlite3_finalize(ppStmt);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Finalize failed in %s at line %d.!", __FILE__, __LINE__);
    exit(1);
  }


  /*
   * If the number of rows modified = 0 the configitem was not found.
   */
  rc = sqlite3_changes(db);
  if(rc == 0) {
    return -1;
  }

  /*
   * All gone well.
   */
  return 0;
}

/*
 * del filter item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int delfilter(sqlite3 *db, const char *name)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;

  /*
   * Init query
   */
  const char* query = "delete from 'filters' where name=?1";

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,              /* SQL statement, UTF-8 encoded */
      strlen(query),      /* Maximum length of zSql in bytes. */
      &ppStmt,            /* OUT: Statement handle */
      &pzTail             /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d",  __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * bind property and value to the query
   *
   */
  rc = sqlite3_bind_text(ppStmt, 1, name, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d" __FILE__, __LINE__);  
    //return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    writelog(LOG_ERROR, "Could not delete filter: %s %s:%d", name, __FILE__, __LINE__);
    //return -1;
  }

  /*
   * Free statement
   */
  rc = sqlite3_finalize(ppStmt);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Finalize failed in %s at line %d.!", __FILE__, __LINE__);
    exit(1);
  }


  /*
   * If the number of rows modified = 0 the configitem was not found.
   */
  rc = sqlite3_changes(db);
  if(rc == 0) {
    return -1;
  }

  /*
   * All gone well.
   */
  return 0;
}

/*
 * Add filter item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addfilter(sqlite3 *db, const char *name, const char *filter, const char *doublefilter)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;
  const char *locdouble; // holds pointer to doublefilter or empty
  int         changes;

  /*
   * Init query
   */
  const char* query = "INSERT INTO 'filters' (name, filter, nodouble) VALUES(?1, ?2, ?3)";

  /*
   * Debug
   */
  writelog(LOG_NORMAL, "new filter name : %s", name);
  writelog(LOG_NORMAL, "filter : %s", filter);
  writelog(LOG_NORMAL, "nodouble filter : %s", doublefilter);


  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,              /* SQL statement, UTF-8 encoded */
      strlen(query),      /* Maximum length of zSql in bytes. */
      &ppStmt,            /* OUT: Statement handle */
      &pzTail             /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * if no 'no double' filter is provided insert an empty string
   */
  if(doublefilter == NULL){
    locdouble = "";
  } else {
    locdouble = doublefilter;
  }

  /*
   * bind property and value to the query
   *
   */
  rc = sqlite3_bind_text(ppStmt, 1, name, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on url %s:%d", __FILE__, __LINE__);  
    //return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 2, filter, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d", __FILE__, __LINE__);  
    //return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 3, doublefilter, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on doublefilter %s:%d", __FILE__, __LINE__);  
    //return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    writelog(LOG_ERROR, "Could not add filter: %s, filter: %s %s:%d", name, filter, __FILE__, __LINE__);
    //fprintf(stderr, "sqlite3_step\n");
    //fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    //return -1;
  }

  /*
   * If the number of rows modified = 0 the configitem was not found.
   */
  changes = sqlite3_changes(db);

  /*
   * Free statement
   */
  rc = sqlite3_finalize(ppStmt);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Finalize failed in %s at line %d.!", __FILE__, __LINE__);
    exit(1);
  }

  /*
   * When no changes, return -1
   */
  if(changes == 0) {
    return -1;
  }

  /*
   * All gone well.
   */
  return 0;
}

/*
 * Change filter item
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int changefilter(sqlite3 *db, const char *name, const char *filter)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;

  /*
   * Init query
   */
  const char* query = "update filters set value=?1 where config.prop=?2";

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      query,              /* SQL statement, UTF-8 encoded */
      strlen(query),      /* Maximum length of zSql in bytes. */
      &ppStmt,            /* OUT: Statement handle */
      &pzTail             /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * bind property and value to the query
   *
   */
  rc = sqlite3_bind_text(ppStmt, 1, filter, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on url %s:%d", __FILE__, __LINE__);  
    return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 2, name, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d", __FILE__, __LINE__);  
    return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    writelog(LOG_ERROR, "sqlite3_step %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Free statement
   */
  rc = sqlite3_finalize(ppStmt);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "Finalize failed in %s at line %d.!", __FILE__, __LINE__);
    exit(1);
  }


  /*
   * If the number of rows modified = 0 the configitem was not found.
   */
  rc = sqlite3_changes(db);
  if(rc == 0) {
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

  if(strcmp(filter, "eztv") == 0) {
    //printf("Found a file for filter %s\n", filter);
    rc = eztvfilter(db, name, url, filter, rssfile); 
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
   *
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
