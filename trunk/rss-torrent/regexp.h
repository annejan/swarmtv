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
 */
int alloccopy(char **dest, char *src, size_t size);

/*
 * Split options
 * When options come in as <name>:<value> split them 
 * the first ':' found is the one plitting name and value
 * When the splitting failed '-1' is returned
 */
int splitnameval(char *input, char **name, char **value);

/*
 * Cleanup strings from XML
 */
void cleanupstring(char *string);

/*
 * Simple routine to compare a string to a regexp
 */
int comregexp(char *regexp, char *string);

/*
 * This routine returns a string pointing to the first captured string.
 */
int capturefirstmatch(char *regexp, int flag, char *string, char **match);

/*
 * Extract username and password from url
 * Accepts passwords in the url https://<user>:<password>@<host>:<port>/<path>
 * returns 0 on no username/passwd
 * When returning 1 cleanurl and userpass are both set, and should be freed.
 */
int getusernamepassword(char *url, char **cleanurl, char **userpass);

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns the char * to the converted string.
 */
char* sizetohuman(size_t size/*in bytes*/, char *buf);

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns 0
 * size in bytes is returned in argument size
 */
int humantosize(char *buf, size_t *size); 

/*
 * strrepl: Replace OldStr by NewStr in string Str.
 *
 * The empty string ("") is found at the beginning of every string.
 *
 * Returns:  0  When replace succesful
 *          -1  When no replace was done
 *
 * **Str must not be on strack, because it gets reallocced.
 */ 
int strrepl(char **Str, char *OldStr, char *NewStr);

/*
 * Replacel all occurences of OldStr te NewStr in Str
 * returns 0 
 */
int strreplall(char **Str, char *OldStr, char *NewStr);

