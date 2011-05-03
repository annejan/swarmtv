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

#ifndef CURLFILE
#define CURLFILE

/*
 *  * HTTP header defines
 *   * Not complete by a long shot, just what I need for now.
 *    */
#define HTTP_CONTENTTYPE  "Content-Type"
#define HTTP_LENGHT       "Content-Length"
#define HTTP_SERVER       "Server"

/*
 * Download url and put the resulting data in chunk.
 * @arguments
 * url 
 * chunk 
 * @returns
 */
int rsstdownloadtobuffer(rsstor_handle *handle, char *url, MemoryStruct *chunk);

/*
 * Free the download and clean the leftovers
 * @arguments
 * chuck pointer to MemoryStruct to free content of.
 */
void rsstfreedownload(MemoryStruct *chunk);

/*
 * Download to file.
 * @arguments
 * url the url to download the content from
 * path the path to store the file (including filename)
 * @returns
 * 0 when all goes well
 * -1 when not found
 */
int rsstdownloadtofile(rsstor_handle *handle, char *url, char *path);

/*
 * extract from HTTP-header
 * This method extracts a given field from the http-header.
 * The pointer returned contains the value.
 * after use free the returned string
 * @arguments 
 * name 	the name of the variable you wish to retrieve.
 * value	the pointer to store the value of the variable in.
 * chunk	the memory structure holding the header.
 * @returns
 * 0 on succes, -1 on failure
 */
int rsstgetheadersvalue(char *name, char **value, MemoryStruct *chunk);

/*
 * Write retrieved buffer to file.
 * @arguments
 * filename is the path to write the file to
 * buffer to write to file
 * @returns
 * 0 on success, else -1
 */
int rsstwritebuffer(char *filename, MemoryStruct *buffer);

#endif
