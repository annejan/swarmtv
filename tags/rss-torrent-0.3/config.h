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
#define CONF_DEFFILTER  "default_filter"
#define CONF_LOCKFILE   "lockfile"
#define CONF_SMTPTO     "smtp_to"
#define CONF_SMTPFROM   "smtp_from"
#define CONF_SMTPHOST   "smtp_host"
#define CONF_SMTPENABLE "smtp_enable"


/*
 * Get value of a config object.
 * When the property does not exist, *value is set to NULL
 * returncode is returncode of the sqlite query
 */
int configgetproperty(sqlite3 *db, char *prop, char **value);

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printconfigitems(sqlite3 *db); 

/*
 * Set config item
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int setconfigitem(sqlite3 *db, char *prop, char *value);

/*
 * Get value of a config object.
 */
int configgetint(sqlite3 *db, char *prop, int *number);

