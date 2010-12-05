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
 * Include the rsstor structs in the types
 * Not the most best solution ever, but will work for now.
 */
#include "swarmtv.h"

/*
 * real = Download torrent, send mails and log download
 * sim  = Just run the database code, no downloading no logging.
 */
typedef enum {real=0, sim} SIM;

/*
 * Enum holdig filter types
 */
typedef enum {sql=0, simple} FILTER_TYPE;

/*
 * Struct holding file info
 */
struct FtpFile {
  const char *filename;
  FILE *stream;
};

/*
 * Structure holding useful torrent properties.
 */
typedef enum {undefined=0, torrent, nzb} METAFILETYPE;

/*
 * meta file properties
 * Data extracted from the torrent/nzb is stored in this struct
 */
typedef struct {
  METAFILETYPE type;        // Tells what kind of meta file is presented
  size_t pieces_length;     // Size of pieces
  size_t size;              // Size of files enclosed
  size_t file_nr;           // Number of files in meta-file
} metafileprops;

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
 * Sandbox struct
 */
typedef struct {
  sqlite3* db;
  char *filename;
} sandboxdb;

