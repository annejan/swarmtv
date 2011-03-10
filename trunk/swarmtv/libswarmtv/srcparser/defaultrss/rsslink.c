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
#include <unistd.h>
#include <stdlib.h>

#ifdef __MINGW32__
#include "../../types.h"  /* looks damn ugly! */
#else
#include "types.h"
#endif
#include "logfile.h"
#include "regexp.h"
#include "filehandler/torrent/findtorrent.h"
#include "defaultrss.h"
#include "disectdescription.h"

/*
 * Use the rss data to fill out the link field in the new torrent struct.
 * Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsslink(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int 	rc=0;
	char *link=NULL;	

	/*
	 * when link and enclosureurl are NULL
	 */
	if( rssdata->link == NULL && 
			rssdata->enclosureurl == NULL &&
			rssdata->torrentlink == NULL)
	{
		return -1;
	}

	/*
	 *    * Get the link from the enclosureurl.
	 *       */
	if(rssdata->torrentlink != NULL) {
		rc = rsstalloccopy(&link, rssdata->torrentlink, strlen(rssdata->torrentlink));
		if(rc != 0) {
			rsstwritelog(LOG_ERROR, "Alloc failed %s:%d", __FILE__, __LINE__);
			exit(1);
		}
	}

	/*
	 * Get the link from the enclosureurl.
	 */
	if(link == NULL){
		if(rssdata->enclosureurl != NULL) {
			rc = rsstalloccopy(&link, rssdata->enclosureurl, strlen(rssdata->enclosureurl));
			if(rc != 0) {
				rsstwritelog(LOG_ERROR, "Alloc failed %s:%d", __FILE__, __LINE__);
				exit(1);
			}
		}
	}

	/*
	 * Get the link from the link node
	 */
	if(link == NULL){
		if(rssdata->link != NULL && strlen(rssdata->link) != 0)
		{
			rc = rsstalloccopy(&link, rssdata->link, strlen(rssdata->link));
			if(rc != 0){
				rsstwritelog(LOG_ERROR, "Alloc failed %s:%d", __FILE__, __LINE__);
				exit(1);
			}
		}
	}

	/*
	 * Last resort, use the guid
	 */
	if(link == NULL){
		if(rssdata->guid != NULL) {
			rc = rsstalloccopy(&link, rssdata->guid, strlen(rssdata->guid));
			if(rc == 0) {
				rsstwritelog(LOG_ERROR, "Alloc failed %s:%d", __FILE__, __LINE__);
				exit(1);
			}
		}
	}

	/*
	 * not found
	 */
	if(link == NULL){
		return -1;
	}

	/*
	 * When enclosure type is nog 'application/x-bittorrent' get real torrent link.
	 */


	/*
	 * Store value in struct
	 */
	newtor->link = link;

	return 0;
}

