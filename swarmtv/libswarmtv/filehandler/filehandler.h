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

#ifndef FILEHANDLER
#define FILEHANDLER


/*
 * Get supported meta file types
 */
char **getsupportedmetatypes();

/*
 * Provide the URL to the NZB,
 * returns a structure containing some of the props of the NZB.
 * free structure afterwards
 * @arguments
 * url URL to get NZB from
 * props structure holding the NZB properties
 * @return
 * returns -1 on failure to parse URL, otherwise 0 is returned.
 */
int rsstgetmetafileinfo(rsstor_handle *handle, METAFILETYPE type, char *url, metafileprops **props);

/*
 * Free torrentprop structure.
 * @arguments
 * props structure to be freed
 */
void rsstfreemetafileprops(metafileprops *props);

/*
 * This is a recursive function.
 * It scans for torrent files, and will return the first one it encounteres.
 * Note this might not be the correct torrent at all :)
 * A counter prevents the recursing from getting out of hand.
 * The torrent wil be contained in torbuffer.
 * The url the torrent was found in torrenturl.
 * Don't forget to free buffer and torrenturl !
 * @arguments
 * url the url to start searching.
 * torrenturl the url found containing the torrent.
 * torbuffer the buffer the torrent is returned in when found. 
 * recurse the number of recursions to do to find the torrent.
 * @return
 * 0 when nothing is found
 * 1 when the torrent was found
 */
int rsstfindmetafile(rsstor_handle *handle, METAFILETYPE type, char *url, char **torrenturl, MemoryStruct **torbuffer, int recurse);

/*
 * Finds and writes torrent to file
 * @arguments
 * url the url to start looking for a torrent.
 * name the path to store the torrent on disk.
 * @return
 * 0 on success
 * -1 when torrent was not found or could not be stored.
 */
int rsstfindmetafilewrite(rsstor_handle *handle, METAFILETYPE type, char *url, char *name, char *filtername);

/*
 * Convert string into METAFILETYPE 
 * @arguments
 * metastr META file string
 * metatype metafile output string
 * @return
 * 0 on success, -1 when not match is found.
 */
int metafilestrtotype(char *metastr, METAFILETYPE *type);

#endif
