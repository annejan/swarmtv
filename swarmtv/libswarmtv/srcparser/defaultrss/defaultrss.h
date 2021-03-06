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
	rsstor_handle *handle;
	char 		*source;
	char 		*title;
	char		*link;
	char		*torrentlink;
	char		*category;
	time_t	 pubdate;
	char		*description;
	char		*comments;
	char		*guid;
	int			 seeds;
	int			 peers;
	size_t	 size;
	size_t	 enclosurelength;
  size_t   contentlength;
	char		*enclosuretype;
	char		*enclosureurl;
  char    *metatype;
	int			 verified;
} rssdatastruct;

/*
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int defaultrss(rsstor_handle *handle, char *name, char *url, char *filter, char *metatype, MemoryStruct *rssfile);

