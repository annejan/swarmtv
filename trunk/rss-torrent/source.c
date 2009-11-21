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

#include "config.h"
#include "database.h"
#include "logfile.h"

#define  MAXLENGHT 400

/*
 * Get value of a config object.
 */
int getsource(sqlite3 *db, char *prop, char **url) 
{
  char  query[MAXLENGHT+1];
  int   rc;

  snprintf(query, MAXLENGHT, "select url from source where name = '%s'", prop);

  rc =  dosingletextquery(db, query, (unsigned char const**) url);

  return rc;
}

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printsources(sqlite3 *db) 
{
  /*
   * header
   */
  printf("#############\n");
  printf("Filters.\n");
  printf("Name : url : filter\n");
  printf("#############\n");

  printquery(db, "select name, url, filter from sources");

  /*
   * Footer
   */
  printf("\n#############\n");
}

/*
 * del source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int delsource(sqlite3 *db, const char *name)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;

  /*
   * Init query
   */
  const char* query = "delete from 'sources' where name=?1";

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
  rc = sqlite3_bind_text(ppStmt, 1, name, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d", __FILE__, __LINE__);  
    //return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    printf("Could not delete source: %s\n", name);
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
 * Add source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addsource(sqlite3 *db, const char *name, const char *url, char *filtertype)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;
  char       *localfilter;

  /*
   * Init query
   */
  const char* query = "INSERT INTO 'sources' (name, url, filter) VALUES(?2, ?1, ?3)";


  /*
   * When filtertype is not set use the default.
   */
  if(filtertype == NULL){
    configgetproperty(db, CONF_DEFFILTER, &localfilter);
  } else {
    localfilter = calloc(1, strlen(filtertype));
    strcpy(localfilter, filtertype);
  }

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
  rc = sqlite3_bind_text(ppStmt, 1, url, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on url %s:%d", __FILE__, __LINE__);  
    //return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 2, name, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on name %s:%d", __FILE__, __LINE__);  
    //return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 3, localfilter, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    writelog(LOG_ERROR, "sqlite3_bind_text failed on filtertype %s:%d", __FILE__, __LINE__);  
    //return -1;
  }

  /*
   * free filtertype.
   */
  free(localfilter);

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    printf("Could not add source: %s, url: %s\n", name, url);
    //fprintf(stderr, "sqlite3_step\n");
    //fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
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
 * Change source item
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int changesource(sqlite3 *db, const char *name, const char *url)
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  char       *zErrMsg = 0;

  /*
   * Init query
   */
  const char* query = "update source set value=?1 where config.prop=?2";

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
    writelog(LOG_ERROR, "sqlite3_prepare_v2");
    writelog(LOG_ERROR, "SQL error: %s %s:%d", zErrMsg, __FILE__, __LINE__);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * bind property and value to the query
   *
   */
  rc = sqlite3_bind_text(ppStmt, 1, url, -1, SQLITE_TRANSIENT);
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
