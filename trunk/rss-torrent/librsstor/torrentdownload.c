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
#include "config.h"
#include "curlfile.h"
#include "torrentdb.h"
#include "logfile.h"
#include "mailmsg.h"
#include "findtorrent.h"
#include "filesystem.h"
#include "database.h"

/*
 * Max message and subject lenght for notification email
 */
#define MAXMSGLEN 1024

/*
 * Apply the filters from the query.
 * when simulate is set to 'sim' no actual downloads are performed
 */
void rsstapplyfilter(sqlite3 *db, char *name, char* nodouble, char *filtertitle, SIM simulate, char *filter, char *fmt, ...);

/*
 * Test for double downloads.
 * Queries need to be provided by the user.
 * return 1 if double, 0 if new
 */
static int testdouble(sqlite3 *db, char *nodouble, char *titleregexp, downloaded_struct *downed);

/*
 * Do download.
 * take url, create name and call curl routine
 */
static int dodownload(sqlite3 *db, downloaded_struct *downed);

/*
 * Get the filters from the database.
 * apply the filters.
 * then download the results.
 * return
 * return 0 on succes, -1 on failure
 */
int rsstdownloadtorrents(sqlite3 *db)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;
  char          *name;
  char          *filter;
  char          *nodouble;

  const char *query = "select name, filter, nodouble from filters";

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
    return -1;
  }

  /*
   * loop until the end of the dataset is found
   */
  while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
    /*
     * Get name and query of the filters
     */
    name = (char*) sqlite3_column_text(ppStmt, 0);
    filter = (char*) sqlite3_column_text(ppStmt, 1);
    nodouble = (char*) sqlite3_column_text(ppStmt, 2);

    /*
     * call apply filter
     */
    rsstapplyfilter(db, name, nodouble, NULL, (SIM) real, filter, NULL);
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


/*
 * Test for double downloads.
 * Queries need to be provided by the user.
 * return 1 if double, 0 if new
 */
static int testdouble(sqlite3 *db, char *nodouble, char *titleregexp, downloaded_struct *downed)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;

  /*
   * prepare query
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      nodouble,            /* SQL statement, UTF-8 encoded */
      strlen(nodouble),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "Testdouble query failed: %s %s:%d", nodouble, __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }

  /*
   * Bind values
   * torrentdb.c:116:  rc = sqlite3_bind_text(ppStmt, 4, category, -1, SQLITE_TRANSIENT);
   * torrentdb.c:117:  rc = sqlite3_bind_int(ppStmt, 5, season);
   */
  rc = sqlite3_bind_text(ppStmt, 1, downed->link, -1, SQLITE_TRANSIENT);
  rc = sqlite3_bind_int(ppStmt, 2, downed->season);
  rc = sqlite3_bind_int(ppStmt, 3, downed->episode);
	if(titleregexp != NULL) {
		rc = sqlite3_bind_text(ppStmt, 4, titleregexp, -1, SQLITE_TRANSIENT);
	}

  /*
   * Execute query
   */
  step_rc = sqlite3_step(ppStmt);

  /*
   * Cleanup 
   */
  sqlite3_finalize(ppStmt);

  /*
   * return result
   */
  if(step_rc == SQLITE_ROW) {
    return 1; // ROW was found so It's a duplicate
  } else { 
    return 0; // None found we has fresh meat :)
  }
}


/*
 * Apply the filters from the query.
 * when simulate is set !=0 no actual downloads are performed
 * arguments :
 * db					: Sqlite3 pointer
 * *name			: Filter name
 * *nodouble	: SQL for the nodouble filter
 * simulate		: When 1 simulation mode 0, no simualtion
 * *filter		: Filter SQL 
 * *fmt				:	Format of the arguments to insert into the filter sql 
 * ...				:	Arguments for the filter SQL.
 */
void rsstapplyfilter(sqlite3 *db, char *name, char* nodouble, char *titleregexp, SIM simulate, char *filter, char *fmt, ...)
{
  sqlite3_stmt  *ppStmt=NULL;
  const char    *pzTail=NULL;
  int           rc=0;
  int           step_rc=0;
  char          *zErrMsg=NULL;
  char          message[MAXMSGLEN+1];
  va_list     	ap;
	int						retval=0;
	int						count=0;
  char     	   *s=NULL;
  int       	  d=0;
  double        f=0.0;
	downloaded_struct downed;

  /*
   * NULL = no arguments.
   */
  if(fmt == NULL) {
    fmt = "";
  }

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      db,                 /* Database handle */
      filter,            /* SQL statement, UTF-8 encoded */
      strlen(filter),    /* Maximum length of zSql in bytes. */
      &ppStmt,             /* OUT: Statement handle */
      &pzTail              /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    rsstwritelog(LOG_ERROR, "Filter '%s' failed %s:%d", name, __FILE__, __LINE__);
    rsstwritelog(LOG_ERROR, "'%s'", filter);
    rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
		retval=-1;
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
                count, filter, fmt, __FILE__, __LINE__);  
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
                count, filter, fmt, __FILE__, __LINE__);  
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
                count, filter, fmt, __FILE__, __LINE__);  
            rsstwritelog(LOG_ERROR, "SQL error: %s", zErrMsg);
            sqlite3_free(zErrMsg);
            retval=-1;
          }
          break;
        default:
          rsstwritelog(LOG_ERROR, "Unknown format '%c' on position '%d'\nQuery: '%s'\nFmt: '%s'",
              *fmt, count, filter, fmt);
          retval=-1;
      }
    }
    va_end(ap);
  }


  if(retval == 0) {

    /*
     * Filters should allways first query for the link!
     */

    /*
     * loop until the end of the dataset is found
     */
    while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
      /*
       * Get name and query of the filters
       */
			memset(&downed, 0, sizeof(downloaded_struct));
      downed.link      = (char*) sqlite3_column_text(ppStmt, 0);
      downed.title     = (char*) sqlite3_column_text(ppStmt, 1);
      downed.pubdate   = (char*) sqlite3_column_text(ppStmt, 2);
      downed.category  = (char*) sqlite3_column_text(ppStmt, 3);
      downed.season    =  sqlite3_column_int(ppStmt, 4);
      downed.episode   =  sqlite3_column_int(ppStmt, 5);

      /*
       * Test if episode is already there
       */
			rc = testdouble(db, nodouble, titleregexp, &downed);
      if(rc == 0) {

        /*
         * Add a torrent to the downloaded table.
         */
        rsstadddownloaded(db, &downed, simulate);

        /*
         * call apply filter
         * when in a sandbox simulate = 1, no downloads are done.
         */
        if(simulate == (SIM) real) {
          /*
           * Download torrent
           */
          rc = dodownload(db, &downed);
					if(rc == 0) {
						/*
						 * Send email
						 */
						snprintf(message, MAXMSGLEN, "Downloading %s S%dE%d", downed.title, downed.season, downed.episode);
						rsstsendrssmail(db, message, message);
					}
        }
      } else {
        rsstwritelog(LOG_DEBUG, "%s Season %d Episode %d is a duplicate %s:%d", 
						downed.title, downed.episode, downed.season, __FILE__, __LINE__);
      }
    }
  }

  /*
   * Done with query, finalizing.
   */
  rc = sqlite3_finalize(ppStmt);
}




/*
 * Do download.
 * take url, create name and call curl routine
 * @Arguments 
 * 
 */
static int dodownload(sqlite3 *db, downloaded_struct *downed) 
{
  char filename[151];
  char *path = NULL;
  char *fullpath = NULL;
	int  retval=0;
	rsstor_handle handle;

	/*
	 * REMOVE IN THE FUTURE
	 */
	handle.db=db;
  
  /*
   * get path to put torrent in
   */
  rsstconfiggetproperty(&handle, CONF_TORRENTDIR, &path);
  rsstcompletepath(path, &fullpath);

  /*
   * Create filename.
   */
  snprintf(filename, 150, "%s/%sS%dE%dR%s.torrent", 
			fullpath, downed->title, downed->season, downed->episode, downed->pubdate); 

  /*
   * download
   */
  retval = rsstfindtorrentwrite(downed->link, filename);

  /*
   * Cleanup
   */
  free(path);
  free(fullpath);

	return retval;
}


/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks throught the newtorrents table to pick the torrent by id.
 * @arguments
 * torid	The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyid(rsstor_handle *handle, int torid)
{
	int 							rc=0;
	sqlite3_stmt 			*ppstmt=NULL;
	downloaded_struct downed;
	int								retval=0;
	int								step_rc=0;
	sqlite3          *db=NULL;

	/*
	 * Get db pointer.
	 */
	db = handle->db;

	static char *manualquery = "select link, title, pubdate, category, season, episode from newtorrents where id = ?1";

	/*
	 * Retrieve record from newtor table
	 */
	rc = rsstexecqueryresult(db, &ppstmt, manualquery, "d", torid);

	/*
	 * Get the first record
	 */
	step_rc = sqlite3_step(ppstmt);
	if(step_rc != SQLITE_ROW) {
		/*
		 * No Records found
		 */
		retval = -1;
	}

	if(retval == 0) {
		/*
		 * initiate download
		 */
		memset(&downed, 0, sizeof(downloaded_struct));
		downed.link      = (char*) sqlite3_column_text(ppstmt, 0);
		downed.title     = (char*) sqlite3_column_text(ppstmt, 1);
		downed.pubdate   = (char*) sqlite3_column_text(ppstmt, 2);
		downed.category  = (char*) sqlite3_column_text(ppstmt, 3);
		downed.season    =  sqlite3_column_int(ppstmt, 4);
		downed.episode   =  sqlite3_column_int(ppstmt, 5);

		/*
		 * Execute download
		 */
		rc = dodownload(db, &downed);
		if(rc == 0) {
			/* 
			 * Download successfull
			 * Add download to downloaded database
			 */
			rsstadddownloaded(db, &downed, 0);
		} else {
			/*
			 * Download failed torrent not found
			 */
			retval = -1;
		}	
	}

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppstmt);

	return retval;
}

/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks throught the newtorrents table to pick the torrent by id.
 * @arguments
 * torid The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyidstr(rsstor_handle *handle, char *torid)
{
	int rc=0;
	int i_torid=0;

	/*
	 * convert torid to int
	 */
	i_torid=atoi(torid);

	/*
	 * call real routine
	 */
	rc = rsstdownloadbyid(handle, i_torid);
	if(rc == 0){
		/*
		 * Download successfull
		 */
		printf("Torrentdownload successful.\n");
	} else {
		/*
		 * Download failed
		 */
		printf("Torrentdownload failed.\n");
	}

	return rc;
}


/*
 * Test torrentdir
 * returns 
 * 0 on succes otherwise -1
 */
int rssttesttorrentdir(rsstor_handle *handle)
{
  int rc=0;
  char *path = NULL;
  char *fullpath = NULL;
	sqlite3 *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

  /*
   * get path to put torrent in
   */
  rsstconfiggetproperty(handle, CONF_TORRENTDIR, &path);
  
  /*
   * Test if the directory exists
   */
  rc = rsstfsexists(path);
  if(rc != 0) {
    rsstwritelog(LOG_ERROR, "Torrent directory '%s' does not exist!", path);
		rsstwritelog(LOG_ERROR, 
				"Please create the directory, or alter torrent directory by setting 'torrentdir' in the config. (--set-config \"torrentdir:<path>\")");
  }

  /*
   * Test if the directry is writable to us
   */
  if(rc == 0) {
    rc |= rssttestwrite(path);
    if(rc != 0) {
      rsstwritelog(LOG_ERROR, "Torrent directory '%s' is not writable!", path);
    }
  }

  /*
   * Cleanup
   */
  free(path);
  free(fullpath);

  return rc;
}

