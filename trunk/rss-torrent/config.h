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

/*
 * The config items
 */
#define CONF_TORRENTDIR "torrentdir"
#define CONF_LOGFILE    "logfile"
#define CONF_REFRESH    "refresh"
#define CONF_RETAIN     "retain"
#define CONF_DEFPARSER  "default_parser"
#define CONF_LOCKFILE   "lockfile"
#define CONF_SMTPTO     "smtp_to"
#define CONF_SMTPFROM   "smtp_from"
#define CONF_SMTPHOST   "smtp_host"
#define CONF_MIN_SIZE   "min_size"
#define CONF_SMTPENABLE "smtp_enable"


/*
 * Get value of a config object.
 * Make sure the free the value returned in the value pointer.
 * @arguments
 * prop name of config property to retrieve
 * value pointer to the pointer that is going to hold the retrieved value.
 * @returns
 * 0 when the value was found, otherwise -1.
 */
int rsstconfiggetproperty(sqlite3 *db, char *prop, char **value);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rsstprintconfigitems(sqlite3 *db); 

/*
 * Set config item
 * @arguments
 * prop name of the propertie to change
 * value new value to enter
 * @returns
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstsetconfigitem(sqlite3 *db, const char *prop, const char *value);

/*
 * Get value of a config object.
 * @arguments
 * prop name of the propertie to change
 * number pointer to place to store value of prop
 * @returns
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstconfiggetint(sqlite3 *db, char *prop, int *number);

/*
 * Get value of a config object.
 * @arguments
 * prop name of the propertie to change
 * number pointer to place to store value of prop
 * @returns
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int rsstconfiggetlong(sqlite3 *db, char *prop, long *number);

