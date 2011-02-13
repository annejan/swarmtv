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
#include "filehandler/filehandler.h"
#include "filesystem.h"
#include "database.h"
#include "callback.h"
#include "lastdownloaded.h"
#include "baretitle.h"

/*
 * When libesmtp is included add header here.
 */
//#ifdef RSST_ESMTP_ENABLE
//  #include "mailmsg.h"
//#endif
/*
 * Max message and subject lenght for notification email
 */
#define MAXMSGLEN 2024


/*
 * Apply the filters from the query.
 * when simulate is set to 'sim' no actual downloads are performed
 */
void rsstapplyfilter(rsstor_handle *handle, char *name, FILTER_TYPE type, char* nodouble, char *filtertitle, SIM simulate, char *filter, char *fmt, ...);

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
static int dodownload(rsstor_handle *handle, downloaded_struct *downed);

/*
 * Get the filters from the database.
 * apply the filters.
 * then download the results.
 * return
 * return 0 on succes, -1 on failure
 */
int rsstdownloadtorrents(rsstor_handle *handle)
{
  sqlite3_stmt  *ppStmt;
  const char    *pzTail;
  int           rc;
  int           step_rc;
  char          *zErrMsg = 0;
  filter_struct sql_data;

  const char *query = "select id, name, filter, nodouble from filters";

  /*
   * Prepare the sqlite statement
   */
  rc = sqlite3_prepare_v2(
      handle->db,        /* Database handle */
      query,            /* SQL statement, UTF-8 encoded */
      strlen(query),    /* Maximum length of zSql in bytes. */
      &ppStmt,          /* OUT: Statement handle */
      &pzTail           /* OUT: Pointer to unused portion of zSql */
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
    sql_data.id = sqlite3_column_int(ppStmt, 0);
    sql_data.name = (char*) sqlite3_column_text(ppStmt, 1);
    sql_data.filter = (char*) sqlite3_column_text(ppStmt, 2);
    sql_data.nodup = (char*) sqlite3_column_text(ppStmt, 3);
    
    /*
     * Send callbacks we are about to apply filter
     */
    rsstexecallbacks(handle, applysqlfilt, &sql_data);

    /*
     * call apply filter
     */
    rsstapplyfilter(handle, sql_data.name, sql, sql_data.nodup, NULL, (SIM) real, sql_data.filter, NULL);
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
  char          *bareregexp=NULL;
  char          *zErrMsg = 0;

  /*
   * Generate bare title regexp
   */
  rsstmakebareregexp(downed->baretitle, &bareregexp);

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
  rc = sqlite3_bind_text(ppStmt, 5, bareregexp, -1, SQLITE_TRANSIENT);

  /*
   * Execute query
   */
  step_rc = sqlite3_step(ppStmt);

  /*
   * Cleanup 
   */
  sqlite3_finalize(ppStmt);
  free(bareregexp);

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
 * Use the filtername and type to get the filterid, use the link to get the downloaded id.
 * @Arguments
 * handle RSS-torrent handle
 * downed Downloaded structure holding the downloaded data.
 * filtername the name of the simple or sql filter 
 * type the filter type sql or simple
 * @Return
 * 0 on success -1 on failure
 */
int rsstupdatelastdowned(rsstor_handle *handle, downloaded_struct *downed, FILTER_TYPE type)
{
  int rc=0;
  int downloadedid=0;
  int filterid=0;
  char *value=NULL;
  char *downedquery = "select id from downloaded where link = ?1 LIMIT 1";
  char *simpleidquery = "SELECT id FROM simplefilters WHERE name = ?1 LIMIT 1";
  char *sqlidquery = "SELECT id FROM filters WHERE name = ?1 LIMIT 1";

  /*
   * Get downloaded id
   */
  rc = rsstdosingletextquery(handle->db, (unsigned char const**)&value, downedquery, "s", downed->link);
  if(rc == 0){
    downloadedid = (int) atoi(value);
    free(value);
    value=NULL;
  } else {
    rsstwritelog(LOG_ERROR, "Could not get downloaded ID for link '%s'  %s:%d", downed->link, __FILE__, __LINE__);
    free(value);
    return -1;
  }

  /*
   * Get filter id and add the entry to the database.
   */
  switch(type){
    case sql:
      rc = rsstdosingletextquery(handle->db, (unsigned char const**)&value, sqlidquery, "s", downed->filter);
      if(rc != 0){
        rsstwritelog(LOG_ERROR, "Quering for sql filter id failed  %s:%d", __FILE__, __LINE__);
        free(value);
        return -1;
      }

      break;
    case simple:
      rc = rsstdosingletextquery(handle->db, (unsigned char const**)&value, simpleidquery, "s", downed->filter);
      if(rc != 0){
        rsstwritelog(LOG_ERROR, "Quering for simple filter id failed %s:%d", __FILE__, __LINE__);
        free(value);
        return -1;
      }

      break;
    default:
      rsstwritelog(LOG_ERROR, "Unknown filter type %s:%d", __FILE__, __LINE__);
      free(value);
      return -1;
  }
  filterid = (int) atoi(value);
  free(value);
  value=NULL;

  /*
   * Call the registration.
   */
  rc = rsstaddlastdownload(handle, filterid, downloadedid, type);
  if(rc != 0){
		rsstwritelog(LOG_ERROR, "Adding download to lastdownload table failed %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Done 
   */
  return 0;
}


/*
 * Handle new results
 */
static void rssthandlenewresults(rsstor_handle *handle, FILTER_TYPE type, downloaded_struct *downed, SIM simulate)
{
	int downsuccess=0;
	char errorstr[MAXMSGLEN+1];

	/*
	 * Make sure the string is terminated
	 */
	errorstr[MAXMSGLEN] = '\0';

	/*
	 * call apply filter
	 * When the routine is called in simulated mode, no downloads are done
	 */
	if(simulate == (SIM) real) {

		/*
		 * Download torrent when download is successful send email.
		 */
		downsuccess = dodownload(handle, downed);
		if(downsuccess == 0) {
			/*
			 * Send email and call callbacks
			 */
			//rsstsendemail(handle->db, filter, downed);
      rsstexecallbacks(handle, downloadtorrent, downed);
		} else {
			/*
			 * Call the torrent download callback and report error
       * @@ FIX make sure this gets handled in the future.
			 */
			//snprintf(errorstr, MAXMSGLEN, "Torrent download failed '%s', '%s'", downed->title, downed->link);
			//rsstexetorcallback(handle, downed->id, -1, errorstr);

		}
	}

	/*
	 * Only when download is success or simulation is enabled add found torrents to downloaded table.
	 */
	if(downsuccess == 0){
		/*
		 * When download has succeeded add entry to downloaded table
		 * Double download attempt will not occur as a newtorrent entry is only new once.
		 */
		rsstadddownloaded(handle, downed, simulate);

    /*
     * Update the lastdownloaded table
     */
    rsstupdatelastdowned(handle, downed, type);
	}
}


/*
 * Handle filter results
 */
static void rssthandlefiltresults(rsstor_handle *handle, sqlite3_stmt *ppStmt, char *filtername, FILTER_TYPE type, char* nodouble, char *titleregexp, SIM simulate)
{
	int 							rc=0;
	int							  step_rc=0;
	downloaded_struct downed;

	/*
	 * loop until the end of the dataset is found
	 */
	while( SQLITE_DONE != (step_rc = sqlite3_step(ppStmt))) {
		/*
		 * Get name and query of the filters
		 */
		memset(&downed, 0, sizeof(downloaded_struct));
    downed.id        =  sqlite3_column_int(ppStmt, 0);
		downed.link      = (char*) sqlite3_column_text(ppStmt, 1);
		downed.title     = (char*) sqlite3_column_text(ppStmt, 2);
		downed.pubdate   = (char*) sqlite3_column_text(ppStmt, 3);
		downed.category  = (char*) sqlite3_column_text(ppStmt, 4);
    downed.metatype  = (char*) sqlite3_column_text(ppStmt, 5);
		downed.season    =  sqlite3_column_int(ppStmt, 6);
		downed.episode   =  sqlite3_column_int(ppStmt, 7);
    downed.filter    =  filtername;
    downed.type      =  type;

    /*
     * Use the downed.title to isolate to name in the title.
     * Form this into a regexp, and provide that to the double test.
     * downed.baretitle
     */
    rc = rsstfillbaretitle(&downed); 
    if(rc == -1) {
      rsstwritelog(LOG_ERROR, "Extracting bare title failed! '%s' %s:%d", downed.title, __FILE__, __LINE__);
    }

		/*
		 * Test if episode is already there
		 */
		rc = testdouble(handle->db, nodouble, titleregexp, &downed);
		if(rc == 0) {
			/*
			 * Handle results that are no duplicate.
			 */
			rssthandlenewresults(handle, type, &downed, simulate);
		} else {
			rsstwritelog(LOG_DEBUG, "%s Season %d Episode %d is a duplicate %s:%d", 
					downed.title, downed.episode, downed.season, __FILE__, __LINE__);
		}

    /*
     * Free bare title
     */
    free(downed.baretitle);
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
void rsstapplyfilter(rsstor_handle *handle, char *name, FILTER_TYPE type, char* nodouble, char *titleregexp, SIM simulate, char *filter, char *fmt, ...)
{
	sqlite3_stmt  *ppStmt=NULL;
	const char    *pzTail=NULL;
	int           rc=0;
	char          *zErrMsg=NULL;
	va_list     	ap;
	int						retval=0;
	int						count=0;
	char     	   *s=NULL;
	int       	  d=0;
	double        f=0.0;

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
			handle->db,           /* Database handle */
			filter,            		/* SQL statement, UTF-8 encoded */
			strlen(filter),    		/* Maximum length of zSql in bytes. */
			&ppStmt,             	/* OUT: Statement handle */
			&pzTail              	/* OUT: Pointer to unused portion of zSql */
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
		 * Handle query results, filter out doubles, and take according action.
		 */
		rssthandlefiltresults(handle, ppStmt, name, type, nodouble, titleregexp, simulate);
	}

	/*
	 * Done with query, finalizing.
	 */
	rc = sqlite3_finalize(ppStmt);
}


/*
 * Create from settings and filename a complete path
 * @Arguments
 * handle
 * downed
 * path
 * metatype
 * fullpath
 * @Return
 * 0 when all was succesfull, -1 when things gone wrong
 */
static int rsstgetdownloadpath(rsstor_handle *handle, downloaded_struct *downed, METAFILETYPE type, char *filename)
{
	char *path = NULL;
	char *fullpath = NULL;
  char *extension = NULL;

	/*
	 * get path to put torrent in
	 */
  if(type == torrent) {
    rsstconfiggetproperty(handle, CONF_TORRENTDIR, &path);
    extension = "torrent";
  } 
  else if(type == nzb) {
    rsstconfiggetproperty(handle, CONF_NZBDIR, &path);
    extension = "nzb";
  } else {
    rsstwritelog(LOG_ERROR, "Meta file type unknown for '%s'. %s:%d", downed->title, __FILE__, __LINE__);
    return -1;
  }

  /*
   * If the meta file path is not set error
   */
  if(strlen(path) == 0) {
    if(type == torrent) {
      rsstwritelog(LOG_NORMAL, "Trying to download Torrent while no download path is set!");
    }
    if(type == nzb) {
      rsstwritelog(LOG_NORMAL, "Trying to download NZB while no download path is set!");
    }
    return -1;
  }

  /*
   * Create full path from abbreviations
   */
	rsstcompletepath(path, &fullpath);

	/*
	 * Create filename.
	 */
	snprintf(filename, 255, "%s/%sS%dE%dR%s.%s", 
			fullpath, downed->title, downed->season, downed->episode, downed->pubdate, extension); 

	/*
	 * Cleanup
	 */
	free(path);
	free(fullpath);

  return 0;
}


/*
 * Do download.
 * take url, create name and call curl routine
 * @Arguments 
 * 
 */
static int dodownload(rsstor_handle *handle, downloaded_struct *downed) 
{
	char filename[256];
	char *path = NULL;
	char *fullpath = NULL;
  int   rc=0;
	int   retval=0;
  METAFILETYPE type=undefined;

	/*
	 * get path to put torrent in
	 */
	rsstconfiggetproperty(handle, CONF_TORRENTDIR, &path);
	rsstcompletepath(path, &fullpath);

	/*
	 * Create filename.
	 */
	snprintf(filename, 150, "%s/%sS%dE%dR%s.torrent", 
			fullpath, downed->title, downed->season, downed->episode, downed->pubdate); 

  /*
   * Free the path
   */
  free(path);

  /*
   * Define METAFILE type
   */
  rc = metafilestrtotype(downed->metatype, &type);
  if(rc != 0) {
    rsstwritelog(LOG_ERROR, "Meta file type unknown for '%s'. %s:%d", downed->title, __FILE__, __LINE__);
    retval = -1;
  }

  /*
   * Determine full Path, and commence download when success
   */
  rc = rsstgetdownloadpath(handle, downed, type, filename);
  if(rc == 0 && retval == 0) {
    /*
     * download
     */
    retval = rsstfindmetafilewrite(type, downed->link, filename);
  } else {
    /*
     * Meta type unknown, this is problematic
     */
    rsstwritelog(LOG_ERROR, "Download failed! %s:%d", __FILE__, __LINE__);
    retval=-1;
  }

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

	static char *manualquery = "select id, link, title, pubdate, category, season, episode, metatype from newtorrents where id = ?1";

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
    downed.id        = sqlite3_column_int(ppstmt, 0);
		downed.link      = (char*) sqlite3_column_text(ppstmt, 1);
		downed.title     = (char*) sqlite3_column_text(ppstmt, 2);
		downed.pubdate   = (char*) sqlite3_column_text(ppstmt, 3);
		downed.category  = (char*) sqlite3_column_text(ppstmt, 4);
		downed.season    =  sqlite3_column_int(ppstmt, 5);
		downed.episode   =  sqlite3_column_int(ppstmt, 6);
    downed.metatype  = (char*) sqlite3_column_text(ppstmt, 7);

		/*
		 * Execute download
		 */
		rc = dodownload(handle, &downed);
		if(rc == 0) {
			/* 
			 * Download successfull
			 * Add download to downloaded database
			 */
			rsstadddownloaded(handle, &downed, 0);
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
int rssttestmetafiledir(rsstor_handle *handle)
{
  int retval=0;
	int torexist=0;
	int nzbexist=0;
  int torwritable=0;
  int nzbwritable=0;
  int torpathlen=0;
  int nzbpathlen=0;
	char *torpath = NULL;
	char *nzbpath = NULL;
	char *fullpath = NULL;
	sqlite3 *db=NULL;

	/*
	 * Get database pointer
	 */
	db = handle->db;

	/*
	 *  Get all the information into the variables
	 */
	rsstconfiggetproperty(handle, CONF_TORRENTDIR, &torpath);
	rsstconfiggetproperty(handle, CONF_NZBDIR, &nzbpath);
	torexist = rsstfsexists(torpath);
	nzbexist = rsstfsexists(nzbpath);
  torpathlen = strlen(torpath);
  nzbpathlen = strlen(nzbpath);

  /*
   * Print the errors
   */
	if(torexist != 0 && torpathlen != 0) {
		rsstwritelog(LOG_ERROR, "Torrent directory '%s' does not exist!", torpath);
		rsstwritelog(LOG_ERROR, 
				"Please create the directory, or alter torrent directory by setting 'torrentdir' in the config. (--set-config \"torrentdir:<path>\"), "
        "clear the setting to disable NZB download.");
	}
  if(nzbexist !=0 && nzbpathlen != 0) {
		rsstwritelog(LOG_ERROR, "NZB directory '%s' does not exist!", nzbpath);
		rsstwritelog(LOG_ERROR, 
				"Please create the directory, or alter NZB directory by setting 'nzbdir' in the config. (--set-config \"nzbdir:<path>\"), "
        "clear the setting to disable NZB download.");
  }

	/*
	 * Test if the directory is writable to us
	 */
	if(torexist == 0 && torpathlen != 0) {
		torwritable |= rssttestwrite(torpath);
		if(torwritable != 0) {
			rsstwritelog(LOG_ERROR, "Torrent directory '%s' is not writable!", torpath);
		}
	}
	if(nzbexist == 0 && nzbpathlen != 0) {
		nzbwritable |= rssttestwrite(nzbpath);
		if(nzbwritable != 0) {
			rsstwritelog(LOG_ERROR, "NZB directory '%s' is not writable!", nzbpath);
		}
	}

  /*
   * Return != 0 when both torrent and NZB are invalid
   */
  if(((torexist != 0 || torwritable != 0) && torpathlen != 0) ||
      ((nzbexist != 0 || nzbwritable != 0) && nzbpathlen != 0)) {
    retval = -1;
  }
  if(torpathlen == 0 && nzbpathlen == 0) {
    rsstwritelog(LOG_ERROR, "No torrent or NZB directory was set, please specify at least one.");
    retval = -1;
  }


  /*
   * Cleanup
   */
  free(torpath);
  free(nzbpath);
  free(fullpath);

  return retval;
}

