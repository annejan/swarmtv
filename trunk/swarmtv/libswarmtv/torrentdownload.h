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
 * Get the filters from the database.
 * apply the filters.
 * then download the results.
 * @return
 * SQLITE_DONE on success, -1 on error.
 */
int rsstdownloadtorrents(rsstor_handle *handle);

/*
 * Apply the filters from the query.
 * when simulate is set !=0 no actual downloads are performed
 * @arguments 
 * *handle			: RSS-torrent handle
 * *name				: Filter name
 * *nodouble		: SQL for the nodouble filter
 * *titleregexp : Title regexp from the simple filter. Set NULL to ignore 
 * simulate			: When 1 simulation mode 0, no simualtion
 * *filter			: Filter SQL 
 * *fmt					:	Format of the arguments to insert into the filter sql 
 * ...					:	Arguments for the filter SQL.
 */
void rsstapplyfilter(rsstor_handle *handle, char *name, FILTER_TYPE type, char* nodouble, char *titleregexp, SIM simulate, char *filter, char *fmt, ...);


/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks throught the newtorrents table to pick the torrent by id.
 * @arguments
 * torid The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyidstr(rsstor_handle *handle, char *torid);

/*
 * This routine function downloads the torrent indicated by ID.
 * The routine looks throught the newtorrents table to pick the torrent by id.
 * @arguments
 * id	The id that points to the torrent
 * @returns
 * 0 on success
 * -1 on failure
 */
int rsstdownloadbyid(rsstor_handle *handle, int torid);

/*
 * Test torrentdir
 * @return
 * 0 when writabe, -1 if not.
 */
int rssttestmetafiledir(rsstor_handle *handle);

