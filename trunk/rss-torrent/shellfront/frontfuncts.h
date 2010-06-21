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
 * Split options
 * When options come in as <name>:<value> split them 
 * the first ':' found is the one plitting name and value
 * When the splitting failed '-1' is returned
 */
int rssfsplitnameval(char *input,char **name, char **value);

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns the char * to the converted string.
 * @arguments
 * size sie to be converted
 * buf the buffer the human readable size is stored
 * @return 
 * returns pointer to the converted size
 */
char* rssfsizetohuman(size_t size/*in bytes*/, char *buf);

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns 0
 * size in bytes is returned in argument size
 * @arguments
 * buf string of size in human format
 * size converted size
 * @return
 * -1 on error
 * 0 on success
 */
int rssfhumantosize(char *buf, double *size); 

/*
 * optstosimple
 * Takes takes a opts_struct argument and a simplefilter_struct as argument.
 * @Arguments
 * opts the opts structure to retrieve the arguments from.
 * simple the simple filter struct to store the filters settings in.
 * @Return
 * returns 0 on succes, -1 on error.
 */
int rssfoptstosimple(opts_struct *opts, simplefilter_struct *simple);

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
int rssfalloccopy(char **dest, const char *src, const size_t size);

/*
 * Get the last season and episode.
 * Adding them to the simplefilter struct.
 * @Argument
 * simple
 * @Return 
 * 0 on success
 * -1 on error
 */
int insertseasonepisode(simplefilter_struct *filter);
