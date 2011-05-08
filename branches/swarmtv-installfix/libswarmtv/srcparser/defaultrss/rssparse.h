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
 * Struct with callback functions.
 */
typedef struct {
	int (*category)(void *data, char *string); // Called if the category is found
	int (*comments)(void *data, char *string); // Called if the comments is found
	void *data; // Data struct pointer
	int (*description)(void *data, char *string); // Called if the description is found
	int (*enclosurelength)(void *data, size_t torsize); // Called if enclosure 
	int (*enclosuretype)(void *data, char *string); // Called if enclosure 
	int (*enclosureurl)(void *data, char *string); // Called if the enclosure url is found
	int (*end)(void *data); // End of RSS entry
	int (*guid)(void *data, char *string); // Called if the guid is found
	int (*leechs)(void *data, char *string); // Called when a number of leechs is found
	int (*link)(void *data, char *string); // Called if the link is found
	int (*torrentlink)(void *data, char *string); // Called if the link is found
	int (*peers)(void *data, int peers); // Called when a number of peers is found
	int (*pubdate)(void *data, char *string); // Called if the pubdate is found
	int (*seeds)(void *data, int seeds); // Called when a number of seeds is found
	int (*size)(void *data, size_t size); // Called when a number of size is found
	int (*contentlength)(void *data, size_t size); // Called when a number of size is found
	int	(*start)(void *data); // Start new RSS entry 
	int (*title)(void *data, char *string); // Called if the title is found
	int (*verified)(void *data, char *string); // Called if verified node is found
} rssparse_callback;

/*
 * Routine to call parser
 * Arguments 
 * call		Pointer to a struct holding the callback routines, all unused should be NULL
 * url		the URL of the RSS feed.
 * buffer	The buffer holding the RSS content.
 * returns 0 on succes, else -1
 */
int rssparse(rssparse_callback *call, char *url, MemoryStruct *buffer);

