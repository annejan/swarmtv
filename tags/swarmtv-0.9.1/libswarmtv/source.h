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
 * Add source item
 * @arguments
 * name filtername
 * url source url
 * parsertype parser type
 * metatype meta file type (torrent/nzb)
 * @return
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstaddsource(rsstor_handle *handle, source_struct *source);
//int rsstaddsource(rsstor_handle *handle, const char *name, const char *url, char *parsertype, char *metatype);

/*
 * Delete source item by name
 * @arguments
 * handle RSS-torrent handle
 * name source id to delete
 * @return
 * When already existing -1 is returned.
 * On success 0 is returned.
 */
int rsstdelsourceid(rsstor_handle *handle, const int *id);

/*
 * del source item
 * @arguments
 * name sourcename to delete
 * @return
 * When allready existing -1 is returned.
 * On succes 0 is returned.
 */
int rsstdelsourcename(rsstor_handle *handle, const char *name);

