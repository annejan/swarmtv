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

#ifdef __MINGW32__
#include "../../types.h"  /* looks damn ugly! */
#else
#include "types.h"
#endif
#include "logfile.h"
#include "defaultrss.h"
#include "regexp.h"

#define CATEGORY_UNKNOWN "unknown"

/*
 * Use the rss data to fill out the category field in the new torrent struct.
 * Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsscategory(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	/*
	 * Try to get a value from the category tag
	 */
	if(rssdata->category != NULL) {
		rsstalloccopy(&(newtor->category), rssdata->category, strlen(rssdata->category));
		return 0;
	}

	/*
	 * When not found, put in "unknown"
	 */
	rsstalloccopy(&(newtor->category), CATEGORY_UNKNOWN, strlen(CATEGORY_UNKNOWN));
	return 0;
}
