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
 * Free a last downloaded structure.
 * @Arguments
 * downed pointer to the downloaded structure
 */
void rsstfreelastdowned(lastdowned_struct *downed);

/*
 * Free the downloaded container and its contents
 * When the container it self is allocated, it should be freed separately
 * @Arguments
 * container pointer to the downed container to be freed
 */
void rsstfreelastdownedcontainer(lastdowned_container *container);

/*
 * Get information of the last downloads and the filters that produced them
 * @Arguments
 * handle RSS-torrent handle 
 * container the pointer pointer to the container
 * @return
 * 0 on success, -1 on failure
 */
int rsstgetlastdownloaded(rsstor_handle *handle, lastdowned_container *container);

/*
 * Add a downloaded record to the lastdownload table
 * @arguments
 * handle RSS-torrent handle
 * filterid The id of the sql/simple filter
 * downedid The id from the downloaded 
 * type simple or sql filter
 * @return
 * 0 in success, else -1
 */
int rsstaddlastdownload(rsstor_handle *handle, int filterid, int downedid, FILTER_TYPE type);
