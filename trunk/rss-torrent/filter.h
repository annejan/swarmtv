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

#include "curlfile.h"

/*
 * Get value of a config object.
 */
int getfilter(sqlite3 *db, char *prop, char **url);


/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void printfilters(sqlite3 *db);

/*
 * Del all filters.
 * Deletes all filters from filtertable.
 * On succes 0 is returned.
 */
int delallfilters(sqlite3 *db);

/*
 * del source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int delfilter(sqlite3 *db, const char *name);


/*
 * Add source item
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int addfilter(sqlite3 *db, const char *name, const char *filter, const char *doublefilter);


/*
 * Change source item
 * When not found -1 is returned.
 * On succes 0 is returned.
 */
int changefilter(sqlite3 *db, const char *name, const char *filter);

/*
 * Apply a filter to the downloaded RSS code.
 * This routine holdes the refferences to different kind of filters.
 * (For now only rsstorrent.com format)
 */
int parserdownload(sqlite3 *db, char * name, char * url, char * filter, MemoryStruct *rssfile);

/*
 * Print filter in a way it could be modified and reentered
 */
void printshellfilter(sqlite3 *db, char *appname, char *filtername);

