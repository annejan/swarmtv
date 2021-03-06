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
 * Open log file getting path from database
 * @return
 * 0 when successful
 * !0 when fail
 */
int rsstinitlogdb(sqlite3 *db);

/*
 * Initialize log file
 * @arguments
 * logpath the path to store the log
 * @return
 * 0 when successful
 * !0 when fail
 */
int rsstinitlog(char *logpath);

/*
 * Log an entry
 * LOG_DEBUG, LOG_NORMAL, LOG_ERROR
 * @arguments
 * level the log level (see log level defines)
 * str the string to log 
 * ... the arguments to fill out in the log line
 * @return
 * returns 1 for now in all cases
 */
int rsstwritelog(int level, char *str,...);

/*
 * Close log file
 */
void rsstcloselog();
