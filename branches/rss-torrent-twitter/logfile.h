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

#define LOG_DEBUG   1
#define LOG_NORMAL  2
#define LOG_ERROR   3

/*
 * External include.
 */
#include <sqlite3.h>

/*
 * Open logfile getting path from database
 */
int initlogdb(sqlite3 *db);

/*
 * Initialize logfile
 */
int initlog(char *logpath);

/*
 * Log an entry
 * LOG_DEBUG, LOG_NORMAL, LOG_ERROR
 */
int writelog(int level, char *str,...);

/*
 * Close logfile
 */
void closelog();
