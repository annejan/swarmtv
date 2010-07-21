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
#include <time.h>

#include "types.h"
#include "curlfile.h"
#include "database.h"
#include "logfile.h"

#define  MAXLENGHT 400


/*
 * Del all filters.
 * Deletes all filters from filter table.
 * On success 0 is returned.
 */
int rsstdelallfilters(rsstor_handle *handle)
{
	sqlite3	*db=NULL;
  int      rc=0;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * Initialize query
   */
  const char* query = "delete from 'filters'";

  printf("Attempting to delete all filters \n");

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = rsstexecutequery(db, query, NULL);
  switch(rc) {
    case(ROWS_CHANGED):
      return 0;
      break;
    case(ROWS_EMPTY):
			printf("No filters left, delete all did nothing %s:%d\n", __FILE__, __LINE__);
			rsstwritelog(LOG_ERROR, "No filters left, delete all did nothing %s:%d", __FILE__, __LINE__);
			return -1;
			break;
		default: 
			rsstwritelog(LOG_ERROR, "Query error during delallfilters %s:%d",  __FILE__, __LINE__);
			return -1;
	}
}

/*
 * Del filter item
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstdelfilter(rsstor_handle *handle, const char *name)
{
	sqlite3    *db=NULL;
	int         rc=0;

	/*
	 * Get db pointer.
	 */
	db = handle->db;

	/*
	 * Initialize query
	 */
	const char* query = "delete from 'filters' where name=?1";

	printf("Attempting to delete filter '%s'\n", name);

	/*
	 * Execute query
	 * When name is all, delete all filters.
	 */
	rc = rsstexecutequery(db, query, "s", name);
	switch(rc) {
		case(ROWS_CHANGED):
			return 0;
			break;
		case(ROWS_EMPTY):
			fprintf(stderr, "Could not delete filter '%s' %s:%di\n", name,  __FILE__, __LINE__);
			rsstwritelog(LOG_ERROR, "Could not delete filter '%s' %s:%d", name,  __FILE__, __LINE__);
			return -1;
			break;
		default: 
			rsstwritelog(LOG_ERROR, "Query error during delfilter %s:%d",  __FILE__, __LINE__);
			return -1;
	}
}

/*
 * Check Filter
 * When the filter exists, return 1
 * else 0
 */
static int checkfilter(sqlite3 *db, const char *name)
{
	int rc=0;

	char *query = "select * from filters where name=?1";

	/*
	 * execute query
	 */
	rc = rsstexecutequery(db, query, "s", name);

	return rc;
}

/*
 * Add filter item
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstaddfilter(rsstor_handle *handle, const char *name, const char *filter, const char *doublefilter)
{
	int         rc=0;
	const char *locdouble=NULL; // holds pointer to double filter or empty
	char        query[MAXLENGHT+1];
	sqlite3 	 *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

	/*
	 * No filters can be added that have the name all.
	 */
	if(strcmp(name, "all") == 0){
		rsstwritelog(LOG_NORMAL, "No filter can be added with name 'all'");
		fprintf(stderr, "No filter can be added with name 'all'\n");
		return -1;
	}

	/*
	 * Init query, to an insert when not found, to a update when existing
	 */
	memset(query, 0, MAXLENGHT+1);
	rc = checkfilter(db, name);
	switch(rc) {
		case 0:
			strncpy(query, "INSERT INTO 'filters' (name, filter, nodouble) VALUES(?1, ?2, ?3)", MAXLENGHT);
			break;
		case 1:
			rsstwritelog(LOG_NORMAL, "filter '%s' exists, updating.", name);
			printf("filter '%s' exists, updating.\n", name);
			strncpy(query, "UPDATE 'filters' SET filter=?2, nodouble=?3 WHERE name=?1", MAXLENGHT);
			break;
		default:
      rsstwritelog(LOG_ERROR, "Filter table corrupt !! %s:%d", __FILE__, __LINE__);
      fprintf(stderr, "Filter table in database corrupt!\n");
      return -1;
  }

  /*
   * Debug
   */
  rsstwritelog(LOG_NORMAL, "new filter name : %s", name);
  rsstwritelog(LOG_NORMAL, "filter : %s", filter);
  rsstwritelog(LOG_NORMAL, "nodouble filter : %s", doublefilter);

  /*
   * if no 'no double' filter is provided insert an empty string
   */
  if(doublefilter == NULL){
    locdouble = "";
  } else {
    locdouble = doublefilter;
  }

  /*
   * Execute query to add filter
   * When no rows are changed return error.
   */
  rc = rsstexecutequery(db, query, "sss", name, filter, doublefilter);
  if(rc != ROWS_CHANGED) {
    rsstwritelog(LOG_ERROR, "No rows changed, inserting filter failed. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * All gone well.
   */
  return 0;
}


