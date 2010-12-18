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
#include <time.h>
#include <pcre.h>
#include <sqlite3.h>

#include "types.h"
#include "config.h"
#include "database.h"
#include "logfile.h"
#include "regexp.h"

#define  MAXLENGHT 400

/*
 * Delete source item by name
 * @arguments
 * handle RSS-torrent handle
 * id source id to delete
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstdelsourceid(rsstor_handle *handle, const int id)
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
  const char* query = "DELETE FROM 'sources' WHERE id=?1";

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = rsstexecutequery(db, query, "d", id);
  switch(rc) {
    case(ROWS_CHANGED):
      printf("Source '%d' deleted.\n", id);
      rsstwritelog(LOG_DEBUG, "Source '%d' deleted.", id);
      return 0;
      break;
    case(ROWS_EMPTY):
      printf("Source '%d' not found, could not delete.\n", id);
      rsstwritelog(LOG_DEBUG, "Source '%d' not found, could not delete.", id);
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during deletesource '%s':%d",  __FILE__, __LINE__);
      return -1;
  }

}

/*
 * Delete source item
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstdelsourcename(rsstor_handle *handle, const char *name)
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
 * Test the metatype string.
 * @arguments
 * metatype string holding the metatype to be entered
 * @return
 * 0 when supported, -1 when not found
 */
static int rssttestmetatype(char *metatype)
{
  int count=0;
  char **supported;

  /*
   * Get supported type from library
   */
  supported = getsupportedmetatypes();

  /*
   * Is the metatype valid ?
   */
  while(supported[count] != NULL) {
    if(!strcmp(supported[count], metatype)) {
      return 0;
    }
    count++;
  }

  return -1;
}


/*
 * Add source item
 * When already existing the source is updated.
 * On success 0 is returned.
 */
int rsstaddsource(rsstor_handle *handle, source_struct *source)
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
  const char* query = "INSERT OR REPLACE INTO 'sources' (name, url, parser, metatype) VALUES(?2, ?1, ?3, ?4)";

  /*
   * Test the metatype is supported
   */
  rc = rssttestmetatype(source->metatype);
  if(rc == -1) {
    rsstwritelog(LOG_ERROR, "Adding source failed, metatype '%s' is not supported.", source->metatype);
    return -1;    
  } 

  /*
   * When parser type is not set use the default.
   */
  if(source->parser == NULL){
    rsstconfiggetproperty(handle, CONF_DEFPARSER, &localparser);
  } else {
		rsstalloccopy(&localparser, source->parser, strlen(source->parser));
  }
  
  //printf("Adding:%s, url:%s, parser type:%s\n", source->name, source->url, localparser);
  rsstwritelog(LOG_DEBUG, "Adding:%s, url:%s, parsertype:%s", source->name, source->url, localparser);

  /*
   * Execute query
   */
  rc = rsstexecutequery(db, query, "ssss", source->url, source->name, localparser, source->metatype);

  /*
   * free parser type.
   */
  free(localparser);

  /*
   * Act on add filter
   */
  switch(rc) {
    case(ROWS_CHANGED):
      printf("Source '%s' added succesfully.\n", source->name);
      rsstwritelog(LOG_DEBUG, "Source '%s' added succesfully.", source->name);
      return 0;
      break;
    case(ROWS_CONSTRAINT):
    case(ROWS_EMPTY):
      fprintf(stderr, "Could not add source '%s', does the source allready exist?\n", source->name);
      rsstwritelog(LOG_ERROR, "Could not add source '%s'. %s:%d", source->name, __FILE__, __LINE__);
      return -1;
      break;
    default: 
      rsstwritelog(LOG_ERROR, "Query error during addsource '%s'. %s:%d", source->name,  __FILE__, __LINE__);
      return -1;
  }
}

