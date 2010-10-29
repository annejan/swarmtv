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

#include "types.h"
#include "regexp.h"
#include "logfile.h"

#include "database.h"
#include "databaseimpl.h"

/*
 * Free a last downloaded structure.
 * @Arguments
 * downed pointer to the downloaded structure
 */
void rsstfreelastdowned(lastdowned_struct *downed)
{
  /*
   * Free the strings
   */
  free(downed->filtername);
  free(downed->filtertype);
  rsstfreedownloaded(downed->downloaded); 
  free(downed->downloaded);
}

/*
 * Free the downloaded container and its contents
 * When the container it self is allocated, it should be freed separately
 * @Arguments
 * container pointer to the downed container to be freed
 */
void rsstfreelastdownedcontainer(lastdowned_container *container)
{
  int count=0;

  /*
   * Free all elements
   */
  while(count < container->nr){
    rsstfreelastdowned(container->lastdownloaded+count);
    count++;
  }
}


/*
 * Add an entrie to the last entry 
 */
static int rsstaddlastdowned(sqlite3_stmt *ppStmt, int count, char *type, lastdowned_container *container)
{
  lastdowned_struct *lastdownload=NULL;
  downloaded_struct *downloaded=NULL;
  char *name=NULL;
  char *link=NULL;
  char *title=NULL;
  char *pubdate=NULL;
  char *category=NULL;
  char *metatype=NULL;
  int season=0;
  int episode=0;

  /*
   * Get the values from the query
   */
  name      = (char*) sqlite3_column_text(ppStmt, 0);
  link      = (char*) sqlite3_column_text(ppStmt, 1);
  title     = (char*) sqlite3_column_text(ppStmt, 2);
  pubdate   = (char*) sqlite3_column_text(ppStmt, 3);
  category  = (char*) sqlite3_column_text(ppStmt, 4);
  metatype  = (char*) sqlite3_column_text(ppStmt, 5);
  season    =  sqlite3_column_int(ppStmt, 6);
  episode   =  sqlite3_column_int(ppStmt, 7);

  /*
   * Allocate and reallocate the structures
   */
  container->nr=count+1;
  container->lastdownloaded = realloc(container->lastdownloaded, sizeof(lastdowned_struct)*container->nr);

  /*
   * Allocate the downloaded structure
   */
  container->lastdownloaded[count].downloaded = calloc(1, sizeof(downloaded_struct));

  /*
   * Initialize the pointers
   */
  lastdownload = &(container->lastdownloaded[count]);
  downloaded = lastdownload->downloaded;


  /*
   * Add the data to the container
   * int rsstalloccopy(char **dest, const char *src, const size_t size);
   */
  rsstalloccopy(&(lastdownload->filtername), name, strlen(name));
  rsstalloccopy(&(lastdownload->filtertype), type, strlen(type));
  rsstalloccopy(&(downloaded->link), link, strlen(link));
  rsstalloccopy(&(downloaded->title), title, strlen(title));
  rsstalloccopy(&(downloaded->pubdate), pubdate, strlen(pubdate));
  rsstalloccopy(&(downloaded->category), category, strlen(category));
  rsstalloccopy(&(downloaded->metatype), metatype, strlen(metatype));
  downloaded->season = season;
  downloaded->episode = episode;

  /*
   * All done
   */
  return 0;
}


/*
 * Get information of the last downloads and the filters that produced them
 * @Arguments
 * handle RSS-torrent handle 
 * container the pointer pointer to the container
 * @return
 * 0 on success, -1 on failure
 */
int rsstgetlastdownloaded(rsstor_handle *handle, lastdowned_container *container)
{
  int rc=0;
  int count=0;
  int retval=0;
  sqlite3_stmt *ppstmtsimple=NULL;
  sqlite3_stmt *ppstmtsql=NULL;

  /*
   * Initialize 
   */
  memset(container, 0, sizeof(lastdowned_container));

  /*
   * Queries
   * Get the following values
   * name link title pubdate category metatype season episode
   */
  char *simplequery=
    "SELECT spl.name, dow.link, dow.title, dow.pubdate, dow.category, dow.metatype, dow.season, dow.episode "
    "FROM simplefilters spl, downloaded as dow, lastdownload as last " 
    "WHERE spl.id = last.simple_id and dow.id = last.downloaded_id";
  char *sqlquery=
    "SELECT flt.name, dow.link, dow.title, dow.pubdate, dow.category, dow.metatype, dow.season, dow.episode "
    "FROM filters flt, downloaded as dow, lastdownload as last "
    "WHERE flt.id = last.simple_id and dow.id = last.downloaded_id";

  /*
   * Execute the query to get all simple filter last downloads
   */
  rc = rsstexecqueryresult(handle->db, &ppstmtsimple, simplequery, NULL);
  if(rc != 0){
    fprintf(stderr, "Query for retrieving simple filter last downloads. %s:%d\n", __FILE__, __LINE__);
    retval=-1;
  }

  /*
   * Execute the query to get all sql filter last downloads
   */
  if(retval == 0){
    rc = rsstexecqueryresult(handle->db, &ppstmtsql, sqlquery, NULL);
    if(rc != 0){
      fprintf(stderr, "Query for retrieving sql filter last downloads.");
      retval=-1;
    }
  }

  /*
   * Add the result numbers together and allocate the space.
   */
  if(retval == 0){
    while( SQLITE_DONE != sqlite3_step(ppstmtsimple)) {
      rc = rsstaddlastdowned(ppstmtsimple, count, "simple", container);
      if(rc != 0){
        rsstwritelog(LOG_ERROR, "Record '%d' type '%s' could not be added.", count, "simple" ); 
        retval=-1;
        break;
      }
      count++;
    }
  }

  /*
   * Add the resulting information into the structures
   */
  if(retval == 0){
    while( SQLITE_DONE != sqlite3_step(ppstmtsql)) {
      rc = rsstaddlastdowned(ppstmtsql, count, "sql", container);
      if(rc != 0){
        rsstwritelog(LOG_ERROR, "Record '%d' type '%s' could not be added.", count, "sql" ); 
        retval=-1;
        break;
      }
      count++;
    }
  }

  /*
   * Clean up
   */
  sqlite3_finalize(ppstmtsimple);
  sqlite3_finalize(ppstmtsql);

  return retval;
}

/*
 * Add a downloaded record to the lastdownload table
 * @arguments
 * handle RSS-torrent handle
 * filterid The id of the sql/simple filter
 * downedid The id from the downloaded 
 * type simple or sql filter
 * @return
 * 0 in success, else -1
 */
int rsstaddlastdownload(rsstor_handle *handle, int filterid, int downedid, FILTER_TYPE type)
{
  int rc=0;
  int retval=0;
  char *query=NULL;
  //sqlite3_stmt *ppstmt=NULL;

  /*
   * Queries used to update the SQL and SIMPLE lastdownload entries
   */
  char *simplequery = "INSERT OR REPLACE INTO lastdownload (simple_id, downloaded_id) VALUES (?1, ?2); COMMIT";
  char *sqlquery = "INSERT OR REPLACE INTO lastdownload (sql_id, downloaded_id) VALUES (?1, ?2); COMMIT";

  /*
   * Test the type of filter we need to handle
   */
  switch(type) {
    case simple:
      query=simplequery;
      break;
    case sql:
      query=sqlquery;
      break;
    default:
      rsstwritelog(LOG_ERROR, "Last downloaded invoked with unknown type %s:%d\n", __FILE__, __LINE__);
      return -1;
  }

  /*
   * Execute the query to get all simple filter last downloads
   */
  rc = rsstexecutequery(handle->db, query, "dd", filterid, downedid);
  if(rc < 0){
    rsstwritelog(LOG_ERROR, "Query for retrieving simple filter last downloads. %s:%d\n", __FILE__, __LINE__);
    retval=-1;
  } else {
    /*
     * @@DEBUG
     */
    printf("Query executed successful !\n");
  }

  return retval;
}

