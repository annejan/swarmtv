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

#include "database.h"
#include "logfile.h"

#define  MAXLENGHT 400

/*
 * Get value of a config object.
 * Make sure the free the value returned in the value pointer.
 * arguments
 * prop name of config property to retrieve
 * value pointer to the pointer that is going to hold the retrieved value.
 * returns
 * 0 when the value was found, otherwise -1.
 */
int configgetproperty(sqlite3 *db, char *prop, char **value) 
{
  char  *query;
  int   rc=0;
  
  query="select value from config where prop = ?1";

  rc =  dosingletextquery(db, (unsigned char const**) value, query, "s", prop);
  if(rc == 0){
    return 0;
  } 
  writelog(LOG_ERROR, "Config value '%s' not found!", prop);

  return -1;
}

/*
 * Get value of a config object.
 */
int configgetint(sqlite3 *db, char *prop, int *number) 
{
  char *value;
  int rc=0;

  rc = configgetproperty(db, prop,(char**) &value);
  if(rc == 0){
    *number = atoi(value);
    free(value);
    return 0;
  }
  writelog(LOG_ERROR, "Config value '%s' not found!", prop);

  return -1;
}

/*
 * Get value of a config object.
 */
int configgetlong(sqlite3 *db, char *prop, long *number) 
{
  char *value;
  int rc=0;

  rc = configgetproperty(db, prop,(char**) &value);
  if(rc == 0){
    *number = atol(value);
    free(value);
    return 0;
  }
  writelog(LOG_ERROR, "Config value '%s' not found!", prop);

  return -1;
}

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printconfigitems(sqlite3 *db) 
{
  /*
   * header
   */
  printf("#############\n");
  printf("Available config items.\n");
  printf("#############\n");
  // select prop, value from config 
  printquery(db, "select prop, value, descr from config");

  /*
   * Footer
   */
  printf("\n#############\n");
}

/*
 * Set config item
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int setconfigitem(sqlite3 *db, const char *prop, const char *value)
{
  sqlite3_stmt  *ppStmt=NULL;
  const char    *pzTail=NULL;
  int           rc=0;
  char          *zErrMsg=NULL;

  /*
   * Init query
   */
  const char* query = "update config set value=?1 where config.prop=?2";

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
    return -1;
  }

  /*
   * bind property and value to the query
   *
   */
  rc = sqlite3_bind_text(ppStmt, 1, value, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    //fprintf(stderr, "sqlite3_bind_text failed on value\n");  
    writelog(LOG_ERROR, "sqlite3_bind_text failed on value %s:%d", __FILE__, __LINE__);
    return -1;
  }
  rc = sqlite3_bind_text(ppStmt, 2, prop, -1, SQLITE_TRANSIENT);
  if( rc!=SQLITE_OK ){
    //fprintf(stderr, "sqlite3_bind_text failed on prop\n");  
    writelog(LOG_ERROR, "sqlite3_bind_text failed on prop %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Evaluate the result.
   */
  rc = sqlite3_step(ppStmt);
  if( rc!=SQLITE_DONE ) {
    //fprintf(stderr, "sqlite3_step\n");
    //fprintf(stderr, "SQL error: %s\n", zErrMsg);
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
    //fprintf(stderr, "Finalize failed in %s at line %d.!", __FILE__, __LINE__);
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
