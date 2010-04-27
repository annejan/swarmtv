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
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "config.h"
#include "filter.h"
#include "simplefilter.h"
#include "source.h"
#include "database.h"
#include "logfile.h"
#include "regexp.h"

/*
 * Program version
 */
#define PROGVERSION "0.8"

/*
 * Print version 
 */
void rsstprintversion(void)
{
	printf("RSSTorrent by Paul Honig 2009-2010\n");
	printf("Version %s \n", PROGVERSION);
}

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 * @Arguments
 * handle RSS-torrent handle
 */
void rsstprintconfigitems(rsstor_handle *handle) 
{
	int 							rc=0;
	int								count=0;
	config_container *container=NULL;

  /*
   * header
   */
  printf("#############\n");
  printf("Available config items.\n");
  printf("#############\n");
	// New interface through the new dal 
	
	/*
	 * Get config values.
	 */
	rc = rsstgetallconfig(handle, &container);
	if(rc != 0){
		fprintf(stderr, "Retrieving of config failed !\n");
		exit(1);
	}	

	/*
	 * Print config values.
	 */
	for(count=0; count < container->nr; count++) {
		printf("%-25s : %-25s : %s\n", 
				container->config[count].name,
				container->config[count].value,
				container->config[count].description);
	}

	/*
	 * Free the results
	 */
	rc = rsstfreeconfigcontainer(container);
	if(rc != 0) {
		fprintf(stderr, "Freeing of the container failed !\n");
		exit(1);
	}

  // select prop, value from config 
  //rsstprintquery(db, "select prop, value, descr from config", NULL);

  /*
   * Footer
   */
  printf("\n#############\n");
}


/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintfilters(rsstor_handle *handle) 
{
	sqlite3 *db=NULL;

	/*
	 * Get db pointer
	 */
	db  = handle->db;

  /*
   * header
   */
  printf("#############\n");
  printf("Filters.\n");
  printf("#############\n");

  rsstprintquery(db, "select name, filter, nodouble from 'filters'", NULL);

  /*
   * Footer
   */
  printf("\n#############\n");
}


/*
 * Print filter in a way it could be modified and reentered
 * @Arguments 
 * handle RSS-torrent handle
 * appname		The name of the executable
 * filtername	The name of the filter to print. 	
 */
void rsstprintshellfilter(rsstor_handle *handle, char *appname, char *filtername)
{
  sqlite3_stmt  *ppStmt=NULL;
  const char    *pzTail=NULL;
  int           rc=0;
  int           cols=0;
  char          *zErrMsg = 0;
  const unsigned char *filterstring=NULL;
  const unsigned char *nodoublestring=NULL;
	sqlite3				*db=NULL;

  char *query =  "select filter, nodouble from 'filters' where name=?1";
	
	/*
	 * Get db pointer
	 */
	db = handle->db;

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

/*
 * Print all available source items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintsources(rsstor_handle *handle) 
{
	int rc=0;
	int count=0;
	source_container *container=NULL;

  /*
   * header
   */
  printf("#############\n");
  printf("Sources\n");
  printf("Name : url : parser\n");
  printf("#############\n");

	/*
	 * Get source values.
	 */
	rc = rsstgetallsources(handle, &container);
	if(rc != 0){
		fprintf(stderr, "Retrieving of source failed !\n");
		exit(1);
	}	

	/*
	 * Print source values.
	 */
	for(count=0; count < container->nr; count++) {
		printf("%-25s : %-15s : %s\n", 
				container->source[count].name,
				container->source[count].parser,
				container->source[count].url);
	}

	/*
	 * Free the results
	 */
	rc = rsstfreesourcecontainer(container);
	if(rc != 0) {
		fprintf(stderr, "Freeing of the container failed !\n");
		exit(1);
	}

  //rsstprintquery(db, "select name, url, parser from sources", NULL);

  /*
   * Footer
   */
  printf("\n#############\n");
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


