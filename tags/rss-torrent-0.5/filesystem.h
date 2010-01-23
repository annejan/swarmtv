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
 * Complete path
 * when the first char of filename is a ~ it gets replaced by 
 * the path to the homedir
 * free destpath afterwards
 */
void completepath(const char *origpath, char **destpath);

/*
 * Test if a file or directory exists
 * return 0 when found, -1 if not found
 */
int fsexists(char *path);

/*
 * Test if a directory is writable
 * This is done by creating a testfile named "test.file",
 * and deleting it afterwards.
 */
int testwrite(const char *path);

