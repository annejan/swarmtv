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
 *  http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */


/*
 * Fork process to daemon.
 * @arguments
 * path path to file to write standard in and out to
 */
void rssfdaemonize(char *path);

/*
 * Lock the rsstorrent lockfile
 * This routine gets the path of the lockfile from the config settings.
 * @Arguments
 * handle RSS-torrent handle
 */
void rssflock(rsstor_handle *handle);

