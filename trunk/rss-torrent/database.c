#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <sqlite3.h>

#include "logfile.h"

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

  
  /*
   * Open the sqlite database.
   */
  rc = sqlite3_open(filename, ppDb);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*ppDb));
    sqlite3_close(*ppDb);
    return !SQLITE_OK;
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
