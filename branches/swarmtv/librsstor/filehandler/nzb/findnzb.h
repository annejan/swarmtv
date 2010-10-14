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

#ifndef FINDNZB
#define FINDNZB

/*
 * This is a recursive function.
 * It scans for NZB files, and will return the first one it encounters.
 * Note this might not be the correct NZB at all :)
 * A counter prevents the recursing from getting out of hand.
 * The NZB will be contained in nzbbuffer.
 * The URL the NZB was found in NZB URL.
 * Don't forget to free buffer and NZB URL !
 * @arguments
 * url the URL to start searching.
 * nzburl the URL found containing the NZB.
 * nzbbuffer the buffer the NZB is returned in when found. 
 * recurse the number of recursions to do to find the NZB.
 * @return
 * 0 when nothing is found
 * 1 when the NZB was found
 */
int rsstfindnzb(char *url, char **nzburl, MemoryStruct **nzbbuffer, int recurse);

/*
 * Finds and writes NZB to file
 * @arguments
 * url the URL to start looking for a NZB.
 * name the path to store the NZB on disk.
 * @return
 * 0 on success
 * -1 when NZB was not found or could not be stored.
 */
int rsstfindnzbwrite(char *url, char *name);

#endif

