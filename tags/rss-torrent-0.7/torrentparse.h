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

#ifndef TORRENTPARSE
#define TORRENTPARSE

/*
 * Should be big enough for all interesing data
 */
#define DATALEN  1024

/*
 * Struct holding useful torrentproperties.
 */
typedef struct {
  size_t pieces_length;     // Size of pieces
  size_t size;            // Size of files enclosed
} torprops;

/*
 * Struct to hold state info and torrent props
 */
typedef struct {
  torprops  *props;       // Torrent properties found go here 
  char      prevtype;    // Can be 's' string, 'd' int or 'k' key
  char      prevkey[DATALEN+1];     // Previously found key.
  long      prevint;      // previous int val
  char      prevstring[DATALEN+1];  // Previous string
} bencstate;

/*
 * Provide the url to the torrent,
 * returns a struct containing some of the props of the torrent.
 * free struct afterwards
 * @arguments
 * url url to get torrent from
 * props structure holding the torrentproperties
 * @return
 * returns -1 on failure to parse url, otherwise 0 is returned.
 */
int gettorrentinfo(char *url, torprops **props);

/*
 * Free torrentprop struct.
 * @arguments
 * props structure to be freed
 */
void freetorprops(torprops *props);

#endif
