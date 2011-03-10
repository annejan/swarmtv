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

#ifdef __MINGW32__
#include "../../types.h"  /* looks damn ugly! */
#else
#include "types.h"
#endif
#include "logfile.h"
#include "regexp.h"
#include "defaultrss.h"
#include "disectdescription.h"

/*
 * Use the rss data to fill out the title field in the new torrent struct.
 * Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsstitle(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int 	rc=0;
	char 	*temp=NULL;

	/*
	 * Get title from title field
	 */
	if(rssdata->title != NULL && strlen(rssdata->title) != 0) {
		rsstalloccopy(&(newtor->title), rssdata->title, strlen(rssdata->title));
		rsstcleanupstring(newtor->title);
	}

	/*
	 * Try to get the title from the description first
	 */
	if(newtor->title ==NULL){
		rc = disectdescription(rssdata->description, "Show Name", &temp);
		if(rc == 0) {
			newtor->title = temp;
		}
	}

	if(newtor->title != NULL){
		rsstcleanupstring(newtor->title);
		return 0;
	}

	/*
	 * Not found
	 */
	return -1;
}

