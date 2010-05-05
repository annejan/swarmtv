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
void rsstprintfilters(rsstor_handle *handle, char *appname) 
{
	filter_container *container=NULL;
	int								count=0;
	int								rc=0;


	/*
	 * Get container with SQL filters.
	 */
	rc = rsstgetallfilter(handle, &container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Get all filters failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Loop through the SQLs.
	 */
	for(count=0; count < container->nr; count++)
	{
		/*
		 * Print the SQL filters.
		 * int   id;       // Id of the filter
		 * char *name;     // Name of the filter
		 * char *filter;   // SQL of the filter
		 * char *nodup;    // SQL of the avoiding duplicates filter
		 */
		printf ( "# %s \\\n -F \'%s:%s\' \\\n -T \'%s\'\n", 
				appname,
				container->filter[count].name,  
				container->filter[count].filter, 
				container->filter[count].nodup);
	}

	/*
	 * Free container
	 */
	rc = rsstfreefiltercontainer(container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Freeing filter container failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * All done
	 */
	return;
}


/*
 * Print filter in a way it could be modified and reentered
 * @Arguments 
 * handle RSS-torrent handle
 * appname		The name of the executable
 * filtername	The name of the filter to print. 	
 */
int rsstprintshellfilter(rsstor_handle *handle, char *appname, char *filtername)
{
  int           		rc=0;
	filter_container 	*container=NULL;
	int								retval=0;

	/*
	 * Retieve container
	 */
	rc = rsstgetfilterbyname(handle, filtername, &container);
	if(rc != 0){
		retval=-1;
	}

	if(retval == 0) {
		/*
		 * Print the filter
		 */
		printf ( "# %s \\\n -F \'%s:%s\' \\\n -T \'%s\'\n", 
				appname,
				container->filter[0].name,  
				container->filter[0].filter, 
				container->filter[0].nodup);

		/*
		 * Free container
		 */
		rc = rsstfreefiltercontainer(container);
		if(rc != 0) {
			rsstwritelog(LOG_ERROR, "Freeing filter container failed! %s:%d", __FILE__, __LINE__);
			retval = -1;
		}
	}

	/*
	 * All done
	 */
	return retval;
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

  /*
   * Footer
   */
  printf("\n#############\n");
}

/*
 * Print a simple filter struct in shell format
 * @Arguments
 *
 * @return
 */
static void printsimplestruct(simplefilter_struct *simple)
{
  unsigned char 			maxsizestring[BUFSIZE+1];
  unsigned char 			minsizestring[BUFSIZE+1];

  /*
   * Print the components that are set
   */
  printf("# rsstorrent --add-simple='%s' --nodup='%s' ", simple->name, simple->nodup); 

  if(strlen((char*)simple->title) > 0) {
    printf("--title='%s' ", simple->title);
  }

  /*
   * Exclude
   */
  if(strlen((char*)simple->exclude) != 0){
    printf("--exclude='%s' ", simple->exclude);
  }

  /*
   * Category
   */
  if(strlen((char*)simple->category) != 0){
    printf("--category='%s' ", simple->category);
  }

  /*
   * Category
   */
  if(strlen((char*)simple->source) != 0){
    printf("--source='%s' ", simple->source);
  }

  /*
   * Maxsize
   */
  if(simple->maxsize != 0){
    rsstsizetohuman(simple->maxsize, (char*) maxsizestring);

    printf("--max-size='%s' ", maxsizestring);
  }

  /*
   * Minsize
   */
  if(simple->minsize != 0){
    rsstsizetohuman(simple->minsize, (char*) minsizestring);

    printf("--min-size='%s' ", minsizestring);
  }

  /*
   * From season
   */
  if(simple->fromseason != 0) {
    printf("--from-season='%d' ", simple->fromseason);
  }

  /*
   * From episode
   */
  if(simple->fromepisode != 0) {
    printf("--from-episode='%d' ", simple->fromepisode);
  }

  /*
   * Close print row
   */
  putchar('\n');
}

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 */
void rsstprintsimple(rsstor_handle *handle, char *filtername)
{
  int           			rc=0;
	simplefilter_container *simplefilter=NULL;

	/*
	 * Get simple container limit of -1 gets all records.
	 */
	rc = rsstgetsimplefilter(handle, &simplefilter, filtername);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstgetsimplefilter failed %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Print result
	 */
	printsimplestruct(simplefilter->simplefilter);

	/*
	 * Free container
	 */
	rc = rsstfreesimplefiltercontainer(simplefilter);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstfreesimplefiltercontainer failed %s:%d", __FILE__, __LINE__);
		return;
	}
}


/*
 * Print all simple filters in shell format.
 */
void rsstprintallsimple(rsstor_handle *handle)
{
  int           					rc=0;
	int											count=0;
	simplefilter_container *simplefilter=NULL;

	/*
	 * Get simple container limit of -1 gets all records.
	 */
	rc = rsstgetallsimplefilter(handle, &simplefilter, -1, 0);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstgetsimplefilter failed %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Print result
	 */
	for(count=0; count < simplefilter->nr; count++) {
		printsimplestruct((simplefilter->simplefilter)+count);
	}

	/*
	 * Free container
	 */
	rc = rsstfreesimplefiltercontainer(simplefilter);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstfreesimplefiltercontainer failed %s:%d", __FILE__, __LINE__);
		return;
	}

#if 0
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
#endif
}


