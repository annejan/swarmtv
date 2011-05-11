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
#include "logfile.h"
#include "filesystem.h"
#include "regexp.h"
#include "setup.h"
#include "sandboxdb.h"
#include "testfilter.h"
#include "simplefilter.h"
#include "torrentdb.h"

#ifdef __MINGW32__
#define strtok_r( _s, _sep, _lasts ) \
( *(_lasts) = strtok( (_s), (_sep) ) )
#endif /* !__MINGW32__ */

/*
 * End of line character used by execute script routine.
 */
#define SCRIPT_EOL "\n"

/*
 * Memic to indicate comments used by execute script routine.
 */
#define SCRIPT_SYM "--"

/*
 * This is a fix for systems that use the old sqlite3 library without
 * sqlite3_prepare_v2. Centos 5.6 is one of them.
 */
int rsst_sqlite3_prepare(  
    sqlite3 *db,            /* Database handle */
    const char *zSql,       /* SQL statement, UTF-8 encoded */
    int nByte,              /* Maximum length of zSql in bytes. */
    sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
    const char **pzTail     /* OUT: Pointer to unused portion of zSql */
    )
{
  int rc=0;
#if SQLITE_VERSION_NUMBER >= 3003009
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      zSql,            /* SQL statement, UTF-8 encoded */
      nByte,    /* Maximum length of zSql in bytes. */
      ppStmt,             /* OUT: Statement handle */
      pzTail              /* OUT: Pointer to unused portion of zSql */
      );
#else 
  rc = sqlite3_prepare(
      db,                 /* Database handle */
      zSql,            /* SQL statement, UTF-8 encoded */
      nByte,    /* Maximum length of zSql in bytes. */
      ppStmt,             /* OUT: Statement handle */
      pzTail              /* OUT: Pointer to unused portion of zSql */
      );
#endif
  return rc;
}

/*
 * Executing script
 * This function executes a script.
 * Each line should be separated by a '\n' char.
 * @Arguments 
 * script pointer so buffer holding script.
 * @ returns
 * 0 on success
 * -1 on fail
 */
int dbexecscript(sqlite3 *db, const char *script)
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
	 * Use strtok to separate the lines
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
 * Do a query, and bind value's to the query in one step
 * This is a simplified version as it only returns 1 value.
 * that value is the first string of the first column.
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
 * 0 on success, -1 on failure
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
   * Prepare the Sqlite statement
   */
  rc = rsst_sqlite3_prepare(
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
  int           errnum=0;
  const char   *errstring=NULL;

  /*
   * fmt pointer to NULL is do not substitutes
   */
  if(fmt == NULL){
    fmt = "";
  }

  /*
   * Prepare the Sqlite statement
   */
  rc = rsst_sqlite3_prepare(
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
        errnum = sqlite3_errcode(db);
        errstring = sqlite3_errmsg(db);
        /*
         * Workaround for older sqlite3 libraries, I hope they will upgrade soon to v2 compatible versions of sqlite
         */
#ifndef sqlite_prepare_v2 
        rsstwritelog(LOG_DEBUG, "sqlite3_step, %d %s:%d", step_rc, __FILE__, __LINE__);
        rsstwritelog(LOG_DEBUG, "in statement : \'%s\'", query);
#else
        rsstwritelog(LOG_ERROR, "sqlite3_step, %s %d %s:%d", errstring, step_rc, __FILE__, __LINE__);
        rsstwritelog(LOG_ERROR, "in statement : \'%s\'", query);
#endif
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
 * Prepare a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * @arguments
 * ppstmt pointer carrying the results of the query
 * query pointer to query string:
 * fmt format string
 * ... arguments to fill out in query
 * @returns
 * return 0 when all is okay
 * returns -1  when the query fails
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
 * Prepare a query
 * query, format, arguments 
 * format string accepts 
 * d = int , s = string, f = double, NULL pointer when no arguments.
 * @arguments
 * ppstmt pointer carrying the results of the query
 * query pointer to query string:
 * fmt format string
 * ap va_list argument list
 * @returns
 * return 0 on success
 * returns -1 when the query fails
 */
int rsstexecqueryresultva(sqlite3 *db, sqlite3_stmt **ppstmt, const char *query, char *fmt, va_list ap)
{
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
   * Prepare the Sqlite statement
   */
  rc = rsst_sqlite3_prepare(
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
      case 'f':            /* float */
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
 * third argument is the number of rows returned.
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
  char          *zErrMsg=NULL;
  int           count=0;
  const unsigned char *text=NULL;
	va_list				ap;

  /*
   * Prepare the Sqlite statement
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
  sqlite3_finalize(ppstmt);

  /*
   * All gone well
   */
  return rc;
}


/*
 * Prints columns from query to standard out.
 * third argument is the number of rows returned.
 * Arguments
 * query	Query to print.
 * names	The names that should be printed in front of values.
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
  char          *zErrMsg=NULL;
  int           count=0;
  const unsigned char *text=NULL;
	va_list				ap;

  /*
   * Prepare the Sqlite statement
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

