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

#include "types.h"
#include "config.h"
#include "database.h"
#include "logfile.h"
#include "regexp.h"

#define  MAXLENGHT 400

/*
 * Print all available source items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintsources(sqlite3 *db) 
{
	int rc=0;
	int count=0;
	source_container *container=NULL;
	rsstor_handle handle;

	/*
	 * REMOVE IN FUTURE !!!
	 */
	handle.db = db;

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
	rc = rsstgetallsources(&handle, &container);
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
 * del source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstdelsource(rsstor_handle *handle, const char *name)
{
  int         rc=0;
	sqlite3    *db=NULL;

	/*
	 * Get database pointer
	 */
	db = handle->db;

  /*
   * Init query
   */
  const char* query = "DELETE FROM 'sources' WHERE name=?1";

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = rsstexecutequery(db, query, "s", name);
  switch(rc) {
    case(ROWS_CHANGED):
      printf("Source '%s' deleted.\n", name);
      rsstwritelog(LOG_DEBUG, "Source '%s' deleted.", name);
      return 0;
      break;
    case(ROWS_EMPTY):
      printf("Source '%s' not found, could not delete.\n", name);
      rsstwritelog(LOG_DEBUG, "Source '%s' not found, could not delete.", name);
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during deletesource '%s':%d",  __FILE__, __LINE__);
      return -1;
  }
}

/*
 * Add source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstaddsource(rsstor_handle *handle, const char *name, const char *url, char *parsertype)
{
  int         rc=0;
  char       *localparser=NULL;
	sqlite3    *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Init query
   */
  const char* query = "INSERT INTO 'sources' (name, url, parser) VALUES(?2, ?1, ?3)";


  /*
   * When parsertype is not set use the default.
   */
  if(parsertype == NULL){
    rsstconfiggetproperty(db, CONF_DEFPARSER, &localparser);
  } else {
		rsstalloccopy(&localparser, parsertype, strlen(parsertype));
  }
  
  printf("Adding:%s, url:%s, parser type:%s\n", name, url, localparser);
  rsstwritelog(LOG_DEBUG, "Adding:%s, url:%s, parsertype:%s", name, url, localparser);

  /*
   * Execute query
   */
  rc = rsstexecutequery(db, query, "sss", url, name, localparser);

  /*
   * free parsertype.
   */
  free(localparser);

  /*
   * Act on addfilter
   */
  switch(rc) {
    case(ROWS_CHANGED):
      printf("Source '%s' added succesfully.\n", name);
      rsstwritelog(LOG_DEBUG, "Source '%s' added succesfully.", name);
      return 0;
      break;
    case(ROWS_CONSTRAINT):
    case(ROWS_EMPTY):
      fprintf(stderr, "Could not add source '%s', does the source allready exist?\n", name);
      rsstwritelog(LOG_ERROR, "Could not add source '%s'. %s:%d", name, __FILE__, __LINE__);
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during addsource '%s'. %s:%d", name,  __FILE__, __LINE__);
      return -1;
  }
}
