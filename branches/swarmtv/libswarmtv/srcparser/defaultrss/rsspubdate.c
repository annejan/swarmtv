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
#include "disectdate.h"
#include "disectdescription.h"
#include "defaultrss.h"

/*
 * Use the rss data to fill out the pubdate field in the new torrent struct.
 * Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsspubdate(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int 		rc=0;
	char 		*retdate=NULL;
	time_t	founddate=0;

	/*
	 * Get the pubdate from the pubdate node. 
	 */
	if(rssdata->pubdate != 0){
		newtor->pubdate=rssdata->pubdate;

		return 0;
	}

	/*
	 * Get the date from description. "Episode Date:" 
	 */
	rc = disectdescription(rssdata->description, "Episode Date", &retdate);
	if(rc == 0){
		/*
		 * Disect date and store
		 */
		rc = rssdisectdate(retdate, &founddate);
		free(retdate);	
		rssdata->pubdate=founddate;

		return 0;
	}

	/*
	 * Not found
	 */
	return -1;
}

