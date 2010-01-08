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

/*
 * This function implement pcre functionality to the queries.
 * called every time sqlite crosses an regexp.
 */
static void regexpfunc(sqlite3_context *db, int num, sqlite3_value **sqlite3_value){
  int   rc;
  const char *var1;
  const char *var2;
  pcre *re = NULL; 
  const char *error = NULL; 
  int errOffset = 0; 
  int match;

  /*
   * sanity check
   */
  if(num != 2) {
    writelog(LOG_ERROR, "on line: %d, in file: %s, the wrong number of arguments were called: %d",
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
   * Compile regular expression
   */
  re = pcre_compile( var1, 0 /* default options */, &error, 
      &errOffset, NULL); 
  if (re == NULL) { 
    writelog(LOG_ERROR, "Regexp compilation failed at " 
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
      writelog(LOG_ERROR, "An unrecognized bit was set in the " 
          "options argument %s:%d", __FILE__, __LINE__); 
      break; 
    case PCRE_ERROR_NOMEMORY: 
      writelog(LOG_ERROR, "Not enough memory available. %s:%d", __FILE__, __LINE__); 
      break; 
    default: 
      if (rc < 0) { 
        writelog(LOG_ERROR, "A regexp match error " 
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
 * Open database, and add regexp functionality.
 */
int initdatabase(
    const char *filename,   /* Database filename (UTF-8) */
    sqlite3   **ppDb)       /* OUT: SQLite db handle */
{
  int rc; /* return code */
  char       *zErrMsg = 0;
  char       *dbpath = NULL;
  
  /*
   * Complete the filename is it contains a ~ as homedir
   */
  completepath(filename, &dbpath);
  
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
 */
int dosingletextquery(sqlite3 *db, const char *query, const unsigned char **text) 
{
  sqlite3_stmt *ppStmt;
  const char *pzTail;
  int         rc;
  int         step_rc;
  char       *zErrMsg = 0;
  const unsigned char  *temptext;

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
  }

  /* 
   * Get the first value discard the others.
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
    *text = (unsigned char *) calloc(strlen((char *)temptext)+1, 1);
    strcpy((char*) *text,(char*) temptext);

    break;
  default:
    writelog(LOG_ERROR, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
    writelog(LOG_ERROR, "in statement : \'%s\'", query);

    *text=NULL;
    break;
  }



  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  return step_rc;
}

#if 0
/*
 * Test a query
 * Don't expect any output back
 * returns 1 on 1 row returned
 * return 0 on no rows returned
 * returns -1 on error
 */
int testquery(sqlite3 *db, const char *query) 
{
  sqlite3_stmt *ppStmt=NULL;
  const char *pzTail=NULL;
  int         rc=0;
  int         retval=0; 
  int         step_rc=0;
  char       *zErrMsg = 0;

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
    retval=-1;
  }

  /* 
   * Get the first value discard the others.
   */
  if(retval == 0){
    step_rc = sqlite3_step(ppStmt);
    switch(step_rc){
      case SQLITE_ROW:
        retval=1;
        break;
      case SQLITE_DONE:
        retval=0;
        break;
      default:
        writelog(LOG_ERROR, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
        writelog(LOG_ERROR, "in statement : \'%s\'", query);
        retval=-1;
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);

  return retval;
}
#endif

/*
 * Execute a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * returns 1 on 1 row returned
 * return 0 on no rows returned
 * returns -1 on error
 */
int executequery(sqlite3 *db, const char *query, char *fmt, ...) 
{
  sqlite3_stmt *ppStmt=NULL;
  const char *pzTail=NULL;
  va_list     ap;
  int         rc=0;
  int         retval=0; 
  int         step_rc=0;
  char       *zErrMsg = 0;
  char        *s=NULL;
  int          d=0;
  double       f=0.0;
  //char         c=' ';
  int          count=0;
  int          changes=0;

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
    writelog(LOG_ERROR, "sqlite3_prepare_v2 %s:%d", __FILE__, __LINE__);
    writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
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
          writelog(LOG_ERROR, "sqlite3_bind_text failed on argument '%d'\n'%s'\n'%s' %s:%d", 
              count, query, fmt, __FILE__, __LINE__);  
          writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'd':            /* int */
        d = va_arg(ap, int);
        rc = sqlite3_bind_int(ppStmt, count, d);
        if( rc!=SQLITE_OK ){
          writelog(LOG_ERROR, "sqlite3_bind_int failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      case 'f':            /* int */
        f = va_arg(ap, double);
        rc = sqlite3_bind_double(ppStmt, count, f);
        if( rc!=SQLITE_OK ){
          writelog(LOG_ERROR, "sqlite3_bind_double failed on argument '%d'\n'%s'\n'%s' %s:%d",
              count, query, fmt, __FILE__, __LINE__);  
          writelog(LOG_ERROR, "SQL error: %s", zErrMsg);
          sqlite3_free(zErrMsg);
          retval=-1;
        }
        break;
      default:
        writelog(LOG_ERROR, "Unknown format '%c' on position '%d'\nQuery: '%s'\nFmt: '%s'",
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
        writelog(LOG_ERROR, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
        writelog(LOG_ERROR, "in statement : \'%s\'", query);
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
 * Prints columns from query to standard out.
 * third argumnt is the number of rows returned.
 * return 0 when okay
 * return -1 on error
 */
int printquery(sqlite3 *db, const char *query)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  int           cols;
  char          *zErrMsg = 0;
  int           count=0;
  const unsigned char *text;

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
   * Get number of columns
   * int sqlite3_column_count(sqlite3_stmt *pStmt);
   */
  cols = sqlite3_column_count(ppStmt);


  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {

    for(count=0; count<cols; count++){
      /*
       * Print the content of the row
       */
      text = sqlite3_column_text(ppStmt, count);
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
  rc = sqlite3_finalize(ppStmt);

  /*
   * All gone well
   */
  return rc;
}
