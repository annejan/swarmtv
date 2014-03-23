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
#define _XOPEN_SOURCE /* glibc2 needs this */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#ifdef __MINGW32__
#include "../../types.h"  /* looks damn ugly! */
#else
#include "types.h"
#endif
#include "logfile.h"
#include "rssparse.h"
#include "defaultrss.h"

#ifdef __MINGW32__
#include "workarounds.h"
#endif

/*
 * Standard date decoding
 */
static int disectdate(char *date, struct tm *pubtm)
{
	char *rc=NULL;

  // %a, %d %b %Y %H:%M:%S 
  // When all failes use time 'now'
  rc = strptime(date, "%a, %d %b %Y %H:%M:%S", pubtm);
	if(rc != NULL) {
		rsstwritelog(LOG_DEBUG, "Converted date '%s'.", date);
	}

	return !rc;
}

/*
 * Simple date decoding
 */
static int simpledisectdate(char *date, struct tm *pubtm)
{
	char *rc=NULL;

  // 2009-10-05
  // %Y-%m-%d
	rc = strptime(date, "%Y-%m-%d", pubtm);
	if(rc != NULL) {
		rsstwritelog(LOG_DEBUG, "Converted date '%s'.", date);
	}

	return !rc;
}

/*
 * Get current time
 */
static void nowdate(struct tm *pubtm)
{
	time_t    now=0;

	/*
	 * Get time now.
	 */
	now = time ( NULL );
	localtime_r ( &now, pubtm );
}

/*
 * Gets the timestring.
 * Exports time_t value
 */
int rssdisectdate(char *date, time_t *pubdate)
{
  int    rc=0;
	int		 retval=0;
  struct tm pubtm;

  /*
   * init struct.
   */
  memset(&pubtm, 0, sizeof(pubtm));
  mktime(&pubtm);

	/*
	 * Disect the date
	 */
	if(date == NULL){
		rsstwritelog(LOG_DEBUG, "NULL passed to rssdisectdate using now.");
		nowdate(&pubtm);
		retval=1;
	}
  if(retval == 0) {
		rc = disectdate(date, &pubtm);
		if(rc == 0){
			retval=1;
		}
	}
  if(retval == 0) {
		rc = simpledisectdate(date, &pubtm);
		if(rc == 0){
			retval=1;
		}
  }
  if(retval == 0) {
    /*
     * When all else fails it's a safe bet to set the date to 'now'
     */
    rsstwritelog(LOG_DEBUG, "Converting date '%s' failed, used 'now' as substitute.", date);
		nowdate(&pubtm);
  }

	/*
	 * Not set by strptime(); tells mktime()
	 * to determine whether daylight saving time
	 * is in effect
	 */
  pubtm.tm_isdst = -1;      

  /*
   * struct tm to time_t
   */
  *pubdate = mktime(&pubtm);

  rsstwritelog(LOG_DEBUG, "Converted ctime: %s", ctime(pubdate));

  /*
   * success
   */
  return 0; 
}

