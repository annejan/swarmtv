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
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <stdlib.h>

#include "types.h"
#include "logfile.h"
#include "curlfile.h"
#include "regexp.h"
#include "torrentdb.h"
#include "defaultrss.h"
#include "disectdate.h"
#include "rssparse.h"
#include "rsslink.h"
#include "rsstitle.h"
#include "rsspubdate.h"
#include "rsscategory.h"
#include "rssseasonepisode.h"
#include "rssseedspeers.h"
#include "rsssize.h"

/*
 * Free all allocated strings in structure.
 */
static void	freerssdata(rssdatastruct *rssdata)
{
	free(rssdata->title);
	free(rssdata->link);
	free(rssdata->category);
	free(rssdata->description);
	free(rssdata->comments);
	free(rssdata->guid);
	free(rssdata->enclosuretype);
	free(rssdata->enclosureurl);
}

/*
 * Handle title
 */
static int handletitle(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->title), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle link
 */
static int handlelink(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->link), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle category
 */
static int handlecategory(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->category), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle pubdate
 */
static int handlepubdate(void *data, char *string)
{
	int rc = 0;
	time_t pubdate=0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Decode date
	 */
	rc = rssdisectdate(string, &pubdate);
	if(rc != 0){
		writelog(LOG_ERROR, "pubdate '%s' could not be decoded.", string);	
	}

	/*
	 * Set pubdate
	 */
	rssdata->pubdate = pubdate;

	/*
	 * Done.
	 */
	return 0;
}

/*
 * Handle description
 */
static int handledescription(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->description), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}


/*
 * Handle comments
 */
static int handlecomments(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->comments), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle guid
 */
static int handleguid(void *data, char *string)
{
	int rc = 0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Store into the struct
	 */
	rc = alloccopy(&(rssdata->guid), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle start
 */
static int handlestart(void *data)
{
	sqlite3 *db;
	rssdatastruct *rssdata = (rssdatastruct *) data;


	/*
	 * NULL all pointers
	 */
	db=rssdata->db;
	memset(rssdata, 0, sizeof(rssdatastruct));
	rssdata->db=db;

	return 0;
}


static int handleenclosurelenght(void *data, size_t newtor)
{
	rssdatastruct *rssdata = (rssdatastruct *) data;
	/*
	 * set lenght in struct
	 */
	rssdata->enclosurelenght = newtor;

	/*
	 * Dummy for now.
	 */
	return 0;
}


static int handleenclosuretype(void *data, char *string)
{
	int rc=0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Set the onclosure type
	 */
	rc = alloccopy(&(rssdata->enclosuretype), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

static int handleenclosereurl(void *data, char *string)
{
	int rc=0;
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Set enclosure url
	 */
	rc = alloccopy(&(rssdata->enclosureurl), string, strlen(string));
	if(rc != 0) {
		writelog(LOG_ERROR, "Alloc failed at %s:%d", __FILE__, __LINE__);
		return  -1;
	}

	return 0;
}

/*
 * Handle seeds
 */
static int handleseeds(void *data, int seeds)
{
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Set the seeds value.
	 */
	rssdata->seeds=seeds;

	/*
	 * Dummy 
	 */
	return 0;
}

/*
 * Handle peers
 */
static int handlepeers(void *data, int peers)
{
	rssdatastruct *rssdata = (rssdatastruct *) data;

	/*
	 * Set the peers value.
	 */
	rssdata->peers=peers;

	/*
	 * Dummy
	 */
	return 0;
}

/*
 * Handle end
 */
static int handleend(void *data)
{
	int 								rc 		 = 0;
	int									ignore = 0;
	newtorrents_struct 	newtor;

	/*
	 * init structs
	 */
	rssdatastruct *rssdata = (rssdatastruct *) data;
	memset(&newtor, 0, sizeof(newtorrents_struct));

	/*
	 * Disect the data.
	 */
	newtor.pubdate = rssdata->pubdate;
	
	/*
	 * Fill out title
	 */
	rc = rsstitle(&newtor, rssdata);
	if(rc != 0){
		ignore=1;
	}

	/*
	 * Fill out link
	 */
	if(ignore == 0) {
		rc = rsslink(&newtor, rssdata);
		if(rc != 0){
			ignore=1;
		}
	}

	/*
	 * Fill out pubdate
	 */
	if(ignore == 0) {
		rc = rsspubdate(&newtor, rssdata);
	}

	/*
	 * Category
	 */
	if(ignore == 0) {
		rc = rsscategory(&newtor, rssdata);
	}

	/*
	 * Set season and episode
	 */
	if(ignore == 0) {
		rc = rssseasonepisode(&newtor, rssdata);
	}
	
	/*
	 * Set seeds and peers
	 */
	if(ignore == 0) {
		rc = rssseedspeers(&newtor, rssdata);
	}

	/*
	 * Set size 
	 */
	if(ignore == 0) {
		rc = rsssize(&newtor, rssdata);
		if(rc != 0){
			ignore=1;
		}
	}

	/*
	 * enter the new record.
	 */
	if(ignore == 0) {
		rc = addnewtorrent(rssdata->db, &newtor);
		if(rc != 0) {
			writelog(LOG_ERROR, "Failed to add newtorrent %s:%d", __FILE__, __LINE__);
			return -1;
		}
	}

	/*
	 * Free all stored strings
	 */
	freerssdata(rssdata);

	/*
	 * Free newtorrents strings
	 */
	freenewtor(&newtor);

	/*
	 * All gone well !
	 */
	return 0;
}

/*
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int defaultrss(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
	int								retval=0;
	int								rc=0;
	rssparse_callback callback;
	rssdatastruct			data;

	/*
	 * Make sure all pointers are set to NULL by default.
	 */
	memset(&callback, 0, sizeof(rssparse_callback));
	memset(&data, 0, sizeof(rssdatastruct));

	/*
	 * Add functions to data
	 */
	data.db = db;

	/*
	 * Initialize the callback struct.
	 */
	callback.data							= &data;
	callback.start 						= handlestart;
	callback.title 						= handletitle;
	callback.link	 						= handlelink;
	callback.category					= handlecategory;
	callback.pubdate					= handlepubdate;
	callback.description			= handledescription;
	callback.enclosurelenght 	= handleenclosurelenght;
	callback.enclosuretype		= handleenclosuretype;
	callback.enclosureurl			=	handleenclosereurl;
	callback.comments					= handlecomments;
	callback.seeds						= handleseeds;
	callback.peers						= handlepeers;
	callback.guid							= handleguid;
	callback.end							= handleend;

	/*
	 * Call the parser
	 */
	rc = rssparse(&callback, url, rssfile);
	if(rc != 0) {
		writelog(LOG_ERROR, "RSS could not be parsed '%s' '%s' '%s'", url, name, filter);
		retval=-1;
	}

	/*
	 * When we reach here all gone well.
	 */
	return retval;
}

