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

#define   BUFSIZE 20

/*
 * This function copies and allocates the destination memory.
 * don't forget to free the destination after use.
 * @arguments
 * dest pointer to the destination the string is copied to
 * src pointer to the source string
 * size the size of the source string
 * @return
 * 0 when okay, otherwise !0
 */
int rsstalloccopy(char **dest, const char *src, const size_t size);

/*
 * Cleanup strings from XML
 * @arguments
 * string pointer to string to clean up
 */
void rsstcleanupstring(char *string);

/*
 * Simple routine to compare a string to a regexp
 * @arguments
 * regexp regexp to match to.
 * string string to be matched.
 * @returns
 * 1 when match
 * 0 when no match
 * -1 on error
 */
int rsstcomregexp(char *regexp, char *string);

/*
 * This routine returns a string pointing to the first captured string.
 * @arguments
 * regexp regexp to capture
 * flag regexp flag to set match mode (look at pcre_exec)
 * string string to capture match from
 * match pointer to store match
 * @return
 * 0 on success
 * -1 on error
 */
int rsstcapturefirstmatch(char *regexp, int flag, char *string, char **match);

/*
 * Extract user name and password from URL
 * Accepts passwords in the URL https://<user>:<password>@<host>:<port>/<path>
 * returns 0 on no user name/password
 * When returning 1 cleanurl and userpass are both set, and should be freed.
 * @arguments
 * url the URL to split
 * cleanurl the URL without the user name and password
 * userpass user name and password in <user name>:<password> format.
 * @return
 * return 1 when user name and password are found
 * else return 0
 */
int rsstgetusernamepassword(char *url, char **cleanurl, char **userpass);

