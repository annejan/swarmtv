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
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "database.h"
#include "logfile.h"
#include "filesystem.h"
#include "database.h"
#include "databaseimpl.h"

/*
 * Get database size
 * @Arguments
 * size the size of the database file
 * @return
 * 0 when all has gone well, -1 on error
 */
static int rsstgetdbsize(size_t *dbsize)
{
  struct stat dbstat;
  int         rc=0;
  int         retval=0;
  char        *dbpath=NULL;

  /*
   * Get the complete path to the DB
   */
  rsstcompletepath(RSST_DBFILE, &dbpath);

  /*
   * Get the information
   */
  rc = stat(dbpath, &dbstat);
  if(rc == 0){
    *dbsize=dbstat.st_size;
  } else {
    rsstwritelog(LOG_ERROR, "Could not retrieve size of '%s'! %s:%d", dbpath,  __FILE__, __LINE__);
    retval=-1;
  }

  /*
   * Clean up
   */
  free(dbpath);

  return retval;
}

/*
 * Get meta file count
 * @Arguments
 * count metafile count number of entries in newtorrents file.
 * @Return
 * 0 return on success, -1 on failure
 */
static int rsstgetmetafile(rsstor_handle *handle, int *count)
{
  int retval=0;
  char *countstr=NULL;

  /*
   * Query
   */
  char *query = "select count(*) from newtorrents";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &countstr, query, NULL);
  if(retval == 0){
    /*
     * Get results
     */
    *count = atoi(countstr);
  }

  /*
   * Free stuff
   */
  free(countstr);

  /*
   * Done
   */
  return retval;
}


/*
 * Get downloaded count
 * @Arguments
 * downloaded
 * @Return
 * 0 return on success, -1 on failure
 */
static int rsstgetdownloadedcount(rsstor_handle *handle, int *count)
{
  int retval=0;
  char *countstr=NULL;

  /*
   * Query
   */
  char *query = "select count(*) from downloaded";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &countstr, query, NULL);
  if(retval == 0) {
    /*
     * Get results
     */
    *count = atoi(countstr);
  }

  /*
   * Free stuff
   */
  free(countstr);

  /*
   * Done
   */
  return retval;
}


/*
 * Get sources count
 * @Arguments
 * sources
 * @Return
 * 0 return on success, -1 on failure
 */
static int rsstgetsources(rsstor_handle *handle, int *count)
{
  int retval=0;
  char *countstr=NULL;

  /*
   * Query
   */
  char *query = "select count(*) from sources";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &countstr, query, NULL);
  if(retval == 0) {
    /*
     * Get results
     */
    *count = atoi(countstr);
  }

  /*
   * Free stuff
   */
  free(countstr);

  /*
   * Done
   */
  return retval;
}


/*
 * Get sources count
 * @Arguments
 * version
 * @Return
 * 0 return on success, -1 on failure
 */
static void rsstgetversion(char **version)
{
  /*
   * Set version from header
   */
  *version = RSST_VERSION;  
}


/*
 * Get database version
 * @Arguments
 * version database version
 * @return 
 * returns 0 on success, -1 on failure
 */
static int rsstgetdbversion(rsstor_handle *handle, int *version)
{
  int retval=0;
  char *versionstr=NULL;

  /*
   * Query
   */
  char *query = "select version from version";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &versionstr, query, NULL);
  if(retval == 0) {
    /*
     * Get results
     */
    *version = atoi(versionstr);
  }

  /*
   * Free stuff
   */
  free(versionstr);

  /*
   * Done
   */
  return retval;
}

/*
 * Get number of simple filters
 * @Arguments 
 * simple number of simple filters in the database
 * @return
 * 0 when success, -1 when failed.
 */
static int rsstgetsimplecount(rsstor_handle *handle, int *count)
{
  int retval=0;
  char *countstr=NULL;

  /*
   * Query
   */
  char *query = "select count(*) from simplefilters";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &countstr, query, NULL);
  if(retval == 0) {
    /*
     * Get results
     */
    *count = atoi(countstr);
  }

  /*
   * Free stuff
   */
  free(countstr);

  /*
   * Done
   */
  return retval;
}


/*
 * Get number of simple filters
 * @Arguments 
 * simple number of simple filters in the database
 * @return
 * 0 when success, -1 when failed.
 */
static int rsstgetsqlcount(rsstor_handle *handle, int *count)
{
  int retval=0;
  char *countstr=NULL;

  /*
   * Query
   */
  char *query = "select count(*) from filters";

  /*
   * execute query
   */
  retval = rsstdosingletextquery(handle->db, (unsigned char const**) &countstr, query, NULL);
  if(retval == 0) {
    /*
     * Get results
     */
    *count = atoi(countstr);
  }

  /*
   * Free stuff
   */
  free(countstr);

  /*
   * Done
   */
  return retval;
}


/*
 * Get statistics from application
 * No free for the returned struct yet
 * @Arguments
 * stats statistics structure
 * @Returns
 * 0 when okay, -1 on error
 */
int rsstgetstats(rsstor_handle *handle, stats_struct *stats)
{
  int retval=0;

  /*
   *  int metafile;   // Number of meta files in newtorrents table.
   *  int downloaded; // Number of downloaded meta files in the downloaded table.
   *  int sources;    // Number sources in the database.
   *  char *version;  // Version string of software version.
   *  int database;   // Database version number.
   *  int simples;    // Number of simple filters.
   *  int sqls;       // SQL filter count.
   */

  /*
   * initialize the structure
   */
  memset(stats, 0, sizeof(stats));

  /*
   * Fill the variables 
   */
  retval = rsstgetmetafile(handle, &(stats->metafile));
  retval |= rsstgetdownloadedcount(handle,  &(stats->downloaded));
  retval |= rsstgetsources(handle, &(stats->sources));
  rsstgetversion(&(stats->version));
  retval |= rsstgetdbversion(handle, &(stats->database));
  retval |= rsstgetsimplecount(handle,&(stats->simples));
  retval |= rsstgetsqlcount(handle, &(stats->sqls));
  retval |= rsstgetdbsize(&(stats->dbsize));

  /*
   * Done return orred value.
   */
  return retval;
}
