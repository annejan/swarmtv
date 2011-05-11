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
 *  Program written by Paul Honig 2009 - 2010
 */

/*
 * Runcycle data struct
 */
typedef struct {
  GMainLoop      *loop;
  int             onetime;
  int             timewait;
  DBusConnection *bus;
} runcycledata;

/*
 * Initialize the dbus connection.
 * Handle the connection of the callback routines to the dbus emitters.
 * @return
 * 0 on success, -1 on failure.
 */
int rssfdbusinit(DBusConnection **bus);

/*
 * Clean up dbus instance
 * @arguments
 * bus pointer to the dbus connection we want to free
 */
void rssfdbusfree(DBusConnection *bus);

/*
 * Connect swarmtv events to dbus emits
 * @arguments
 * handle swarmtv handle
 */
void rssfinitcallbacks(rsstor_handle *handle);
