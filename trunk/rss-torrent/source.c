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
#include "regexp.h"

#define  MAXLENGHT 400

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
  printf("Sources\n");
  printf("Name : url : parser\n");
  printf("#############\n");

  printquery(db, "select name, url, parser from sources");

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
  int         rc;

  /*
   * Init query
   */
  const char* query = "DELETE FROM 'sources' WHERE name=?1";

  /*
   * Execute query
   * When name is all, delete all filters.
   */
  rc = executequery(db, query, "s", name);
  switch(rc) {
    case(ROWS_CHANGED):
      printf("Source '%s' deleted.\n", name);
      writelog(LOG_DEBUG, "Source '%s' deleted.", name);
      return 0;
      break;
    case(ROWS_EMPTY):
      printf("Source '%s' not found, could not delete.\n", name);
      writelog(LOG_DEBUG, "Source '%s' not found, could not delete.", name);
      return -1;
      break;
    default: 
      writelog(LOG_ERROR, "Query error during deletesource '%s':%d",  __FILE__, __LINE__);
      return -1;
  }
}

/*
 * Add source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addsource(sqlite3 *db, const char *name, const char *url, char *parsertype)
{
  int         rc;
  char       *localparser;

  /*
   * Init query
   */
  const char* query = "INSERT INTO 'sources' (name, url, parser) VALUES(?2, ?1, ?3)";


  /*
   * When parsertype is not set use the default.
   */
  if(parsertype == NULL){
    configgetproperty(db, CONF_DEFPARSER, &localparser);
  } else {
		alloccopy(&localparser, parsertype, strlen(parsertype));
  }
  
  printf("Adding:%s, url:%s, parser type:%s\n", name, url, localparser);
  writelog(LOG_DEBUG, "Adding:%s, url:%s, parsertype:%s", name, url, localparser);

  /*
   * Execute query
   */
  rc = executequery(db, query, "sss", url, name, localparser);

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
      writelog(LOG_DEBUG, "Source '%s' added succesfully.", name);
      return 0;
      break;
    case(ROWS_CONSTRAINT):
    case(ROWS_EMPTY):
      fprintf(stderr, "Could not add source '%s', does the source allready exist?\n", name);
      writelog(LOG_ERROR, "Could not add source '%s'. %s:%d", name, __FILE__, __LINE__);
      return -1;
      break;
    default: 
      writelog(LOG_ERROR, "Query error during addsource '%s'. %s:%d", name,  __FILE__, __LINE__);
      return -1;
  }
}
