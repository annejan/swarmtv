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

#include "logfile.h"

/*
 * Gets the timestring.
 * Exports time_t value
 */
int disectdate(char *date, time_t *pubdate)
{
  char      *rc;
  struct tm pubtm;
  time_t    now;

  /*
   * init struct.
   */
  memset(&pubtm, 0, sizeof(pubtm));
  mktime(&pubtm);

  // char *strptime(const char *buf, const char *format, struct tm *tm);
  // Fri, 02 Oct 2009 17:23:06 -0500
  // %a, %d %b %Y %H:%M:%S 
  // 2009-10-05
  // %Y-%m-%d
  // When all failes use time 'now'
  rc = strptime(date, "%a, %d %b %Y %H:%M:%S", &pubtm);
  if(rc == NULL) {
    rsstwritelog(LOG_DEBUG, "Converting date '%s'.\n", date);
    rc = strptime(date, "%Y-%m-%d", &pubtm);
  }
  if(rc == NULL) {
    /*
     * When all else fails it's a safe bet to set the date to 'now'
     */
    rsstwritelog(LOG_DEBUG, "Converting date '%s' failed, used 'now' as substitute.", date);
    now = time ( NULL );
    localtime_r ( &now, &pubtm );

  }

  pubtm.tm_isdst = -1;      /* Not set by strptime(); tells mktime()
                               to determine whether daylight saving time
                               is in effect */

  /*
   * struct tm to time_t
   */
  *pubdate = mktime(&pubtm);

  rsstwritelog(LOG_DEBUG, "Converted ctime: %s\n", ctime(pubdate));

  /*
   * success
   */
  return 0; 
}
