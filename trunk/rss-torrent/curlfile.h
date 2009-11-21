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

#ifndef CURLFILE
#define CURLFILE

/*
 * Struct holding file info
 */
struct FtpFile {
  const char *filename;
  FILE *stream;
};

/*
 * Struct holing pointer and size of downloaded file.
 */
typedef struct{
  char *memory;
  size_t size;
  char *header;
  size_t headersize;
} MemoryStruct; 


/*
 * Download url and put the resulting data in chunk.
 */
int downloadtobuffer(char *url, MemoryStruct *chunk);

/*
 * Free the download and clean the leftovers
 */
void freedownload(MemoryStruct *chunk);

/*
 * Download to file.
 */
int downloadtofile(char *url, char *path);

#endif
