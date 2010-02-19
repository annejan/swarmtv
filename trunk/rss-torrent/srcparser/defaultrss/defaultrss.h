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
 * RSS data structure
 */
typedef struct {
	sqlite3 *db;
	char 		*title;
	char		*link;
	char		*category;
	time_t	 pubdate;
	char		*description;
	char		*comments;
	char		*guid;
	int			seeds;
	int			peers;
	size_t	size;
	size_t	enclosurelength;
	char		*enclosuretype;
	char		*enclosureurl;
	int			verified;
} rssdatastruct;

/*
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int defaultrss(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile);

