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
 * @arguments
 * origpath the original path
 * destpath the expanded (complete) path
 */
void rsstcompletepath(const char *origpath, char **destpath);

/*
 * Test if a file or directory exists
 * @arguments
 * path path to check
 * @returns
 * return 0 when found, -1 if not found
 */
int rsstfsexists(char *path);

/*
 * Test if a directory is writable
 * This is done by creating a testfile named "test.file",
 * and deleting it afterwards.
 * @arguments
 * path path to check
 * @returns
 * return 0 when found, -1 if not found
 */
int rssttestwrite(const char *path);

/*
 * Create directory
 * @arguments
 * path path to directory
 * @returns
 * 0 on succes, -1 on failure
 */
int rsstmakedir(char *path);

/*
 * Get the usage of the partition the pointed directory resides in
 * @arguments
 * path the path to get the usage for
 * usage the usage in percents 0 - 100
 * @returns
 * 0 on success, -1 on failure
 */
int rsstdiskusage(char *path, int *usage);
