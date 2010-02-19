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
#include <time.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>

#include "types.h"
#include "curlfile.h"
#include "logfile.h"
#include "twitparse.h"
#include "twitter.h"
#include "splittext.h"
#include "torrentparse.h"
#include "torrentdb.h"
#include "parsedate.h"

static int newtweet(void *data)
{
	twitterdata *local = (twitterdata*) data;

	writelog(LOG_DEBUG, "New Tweet\n");

	/*
	 * Clear tweet struct
	 */
	memset(&(local->tweet), 0, sizeof(tweetdata));

	return 0;
}

/*
 * Called on end of tweet-info
 */
static int endtweet(void *data)
{
	twitterdata *local = (twitterdata*) data;
	sqlite3 *db = local->db;
	torprops *props=NULL;
	int					rc=0;
	//char 			*name=NULL;
	//char			*link=NULL;
	//int				season=0;
	//int				episode=0;
	//time_t		pubdate=0;
	newtorrents_struct newtor; 

	/*
	 * Clean newtor
	 */
	memset(&newtor, 0, sizeof(newtorrents_struct));

	/*
	 * Use data to create database record.
	 */
	rc = splittext(local->tweet.text, &(newtor.title), &(newtor.link), &(newtor.season), &(newtor.episode));
	if(rc < 0){
		/*
		 * Text could not be split, no torrent record.
		 */
		return -1;
	}

	/*
	 * Process data
	 */
	rc = gettorrentinfo(newtor.link, &props);
	if(rc < 0){
		writelog(LOG_DEBUG, "Download failed for '%s'\n", newtor.link);
		/*
		 * No torrent.
		 */
		return -1;
	}

	/*
	 * Print stuff.
	 */
	writelog(LOG_DEBUG, "link: %s, title: %s, season: %d, episode: %d, size: %ld\n", 
			newtor.link, newtor.title, newtor.season, newtor.episode,(long) props->size); 
	
	/*
	 * Parse date to string
	 */
	disectdate(local->tweet.createdate, &(newtor.pubdate));

	/*
	 * Add a torrent to the newtorrents table.
	 */
	addnewtorrent(db, &newtor);

	/*
	 * cleanup
	 */
	freetorprops(props);
	freenewtor(&newtor);

	writelog(LOG_DEBUG, "End Tweet\n");

	return 0;
}

static int twittertext(void *data, char *string)
{
	twitterdata *local = (twitterdata*) data;
	//sqlite3 *db = local->db;

	/*
	 * Debugging : print the twittertext we found.
	 */
	writelog(LOG_DEBUG, "Twitter text : '%s'\n", string);
	local->tweet.text=string;

	return 0;
}

static int twittecreaterdate(void *data, char *string)
{
	twitterdata *local = (twitterdata*) data;
	//sqlite3 *db = local->db;

	writelog(LOG_DEBUG, "Twitter createdate: '%s'\n", string);
	local->tweet.createdate=string;

	return 0;
}

/*
 * filter to handle incomming files from a twitter timeline
 */
int twitter(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
	twitparse_callback twitcallback;
	twitterdata				 twitdata;
	int								 rc=0;
	int								 retval=0;

	/*
	 * All to NULL
	 */
	memset(&twitcallback, 0, sizeof(twitparse_callback));
	memset(&twitdata, 0, sizeof(twitterdata));

	/*
	 * Init data.
	 */
	twitdata.db=db;

	/*
	 * Initialize parser callbacks.
	 */
	twitcallback.gettext = twittertext;
	twitcallback.getcreatedate = twittecreaterdate;
	twitcallback.newtweet = newtweet;
	twitcallback.endtweet = endtweet;
	twitcallback.data = &twitdata;

	/*
	 * Call the filter to transform the timeline into database entries.
	 * when failed return -1
	 */
	rc = twitparse(&twitcallback, url, rssfile);
	if(rc != 0){
		writelog(LOG_ERROR, "Twitter stream url '%s' name '%s' filter '%s' fialed ! %s:%d", url, name, filter, __FILE__, __LINE__);
		retval = -1;
	}

	/*
	 * Done.
	 */
	return retval;
}

