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
 * Structure holding the callback the parser calls.
 * When a callbackpointer is NULL, no function is called.
 */
typedef struct {
	void *data; // Store use data in this struct.
	int (*newtweet)(void *data); // Called on the start of parsing a record
	int (*gettext)(void *data, char *string); // Called the the twittertext is found
	int (*getcreatedate)(void *data, char *string); // Called when createdate is found
	int (*endtweet)(void *data); // Called on the start of parsing a record
	int (*donetweet)(void *data); // Called on the start of parsing a record
} twitparse_callback;

/*
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int twitparse(twitparse_callback *call, char *url, MemoryStruct *buffer);

