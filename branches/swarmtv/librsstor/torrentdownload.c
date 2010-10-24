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
#include "callbackimpl.h"

/*
 * When libesmtp is included add header here.
 */
#ifdef RSST_ESMTP_ENABLE
  #include "mailmsg.h"
#endif

/*
 * Max message and subject lenght for notification email
 */
#define MAXMSGLEN 2024

/*
 * Apply the filters from the query.
 * when simulate is set to 'sim' no actual downloads are performed
 */
void rsstapplyfilter(rsstor_handle *handle, char *name, char* nodouble, char *filtertitle, SIM simulate, char *filter, char *fmt, ...);

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
  char          *name;
  char          *filter;
  char          *nodouble;

  const char *query = "select name, filter, nodouble from filters";

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
    name = (char*) sqlite3_column_text(ppStmt, 0);
    filter = (char*) sqlite3_column_text(ppStmt, 1);
    nodouble = (char*) sqlite3_column_text(ppStmt, 2);

    /*
     * call apply filter
     */
    rsstapplyfilter(handle, name, nodouble, NULL, (SIM) real, filter, NULL);
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
 * Send email containing information about torrent
 * @arguments 
 * db	database pointers
 * downed 
 * @return
 *
 */
static void rsstsendemail(sqlite3 *db, char *filtername, downloaded_struct *downed)
{
	char 					header[MAXMSGLEN+1];
	char          message[MAXMSGLEN+1];

#ifdef RSST_ESMTP_ENABLE
	/*
	 * Build header and email-message
	 */
	snprintf(header, MAXMSGLEN, 
			"Downloading %s S%dE%d", 
			downed->title, downed->season, downed->episode);
	snprintf(message, MAXMSGLEN, 
			"Downloading %s S%dE%d\n"
      "\n--------------------------------\n"
			"URL: %s\n"
      "This content was found by Filter: %s\n"
      "Download method used: %s\n"
      "\n--------------------------------", 
			downed->title, downed->season, downed->episode, downed->link, filtername, downed->metatype);

  /*
   * Call the send mail routine.
   */
	rsstsendrssmail(db, header, message);
#endif
}

/*
 * Call the torrentdownload callback
 * @Arguments
 * handle	RSS-torrent handle
 * id			Id of the torrent being downloaded
 * status	status 0 for success -1 for fail
 * error	string containing error message, NULL when status is 0
 * @Return
 * returns 0 when all called routines return 0 otherwise !0 is returned.
 */
static int rsstexetorcallback(rsstor_handle *handle, int id, int status, char *error)
{
	int							rc=0;
	struct_download callback;

	/*
	 * Build callback struct
	 */
	callback.id = id;				
	callback.status = status;	
	callback.error = error;

	/*
	 * Call callback
	 */
	rc = rsstexecdowntordownrsscallbacks(handle, &callback);

	return rc;
}


/*
 * Handle new results
 */
static void rssthandlenewresults(rsstor_handle *handle, char *filtername, downloaded_struct *downed, SIM simulate)
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
			rsstsendemail(handle->db, filtername, downed);
			rsstexetorcallback(handle, downed->id, 0, NULL);
		} else {
			/*
			 * Call the torrent download callback and report error
			 */
			snprintf(errorstr, MAXMSGLEN, "Torrent download failed '%s', '%s'", downed->title, downed->link);
			rsstexetorcallback(handle, downed->id, -1, errorstr);
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
	}

	/*
	 * Done
	 */
}


/*
 * Handle filter results
 */
static void rssthandlefiltresults(rsstor_handle *handle, sqlite3_stmt *ppStmt, char *filtername, char* nodouble, char *titleregexp, SIM simulate)
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
		downed.link      = (char*) sqlite3_column_text(ppStmt, 0);
		downed.title     = (char*) sqlite3_column_text(ppStmt, 1);
		downed.pubdate   = (char*) sqlite3_column_text(ppStmt, 2);
		downed.category  = (char*) sqlite3_column_text(ppStmt, 3);
    downed.metatype  = (char*) sqlite3_column_text(ppStmt, 4);
		downed.season    =  sqlite3_column_int(ppStmt, 5);
		downed.episode   =  sqlite3_column_int(ppStmt, 6);

		/*
		 * Test if episode is already there
		 */
		rc = testdouble(handle->db, nodouble, titleregexp, &downed);
		if(rc == 0) {
			/*
			 * Handle results that are no duplicate.
			 */
			rssthandlenewresults(handle, filtername, &downed, simulate);
		} else {
			rsstwritelog(LOG_DEBUG, "%s Season %d Episode %d is a duplicate %s:%d", 
					downed.title, downed.episode, downed.season, __FILE__, __LINE__);
		}
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
void rsstapplyfilter(rsstor_handle *handle, char *name, char* nodouble, char *titleregexp, SIM simulate, char *filter, char *fmt, ...)
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
		rssthandlefiltresults(handle, ppStmt, name, nodouble, titleregexp, simulate);
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
    rsstwritelog(LOG_ERROR, "'%s' Did not have meta type !", fullpath);
    retval=-1;
  }

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

	static char *manualquery = "select link, title, pubdate, category, season, episode, metatype from newtorrents where id = ?1";

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
    downed.metatype  = (char*) sqlite3_column_text(ppstmt, 6);

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

