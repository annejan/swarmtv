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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <magic.h>

/*
 * Place of magic file
 */
#define MAGIC_FILENAME  "/usr/share/misc/magic"

/*
 * magic pointer
 */
static magic_t magic=NULL;

/*
 * Initialize the magic 
 */
int initmagic() 
{
  int rc;

  /*
   * Sanity check
   * when already initialized ignore a second call
   */
  if(magic != NULL) {
    return 0;
  }

  /*
   * Open context
   */
  magic = magic_open(MAGIC_COMPRESS | MAGIC_MIME);

  /*
   * Load the magic file with file definitions
   */
  magic_load(magic, MAGIC_FILENAME); 

  /*
   * Check if all went well
   */
  rc = magic_errno(magic);
  if(rc != 0) {
    fprintf(stderr, "magic error \n");
    fprintf(stderr, "%s\n", magic_error(magic));
    return rc; 
  }

  return 0;
}


/*
 * This routine determines the filetype and returns a string describing the filetype.
 * Don't forget to free the filtetype string after use.
 */
int getcontenttype(char *buffer, size_t size, char **filetype) 
{
  int rc;

  /*
   * check sanity
   */
  if(magic == NULL) {
    fprintf(stderr, "magic not initialized yet !\n");
    return -1;
  }

  /*
   * init output
   */
  *filetype = NULL;

  /*
   * try match
   */
  const char* type = magic_buffer(magic, buffer, size); 

  /*
   * Check for errors
   */
  rc = magic_errno(magic);
  if(rc != 0) {
    fprintf(stderr, "magic error \n");
    fprintf(stderr, "%s\n", magic_error(magic));
    return rc; 
  }

  /*
   * check for valid output
   */
  if(type == NULL) {
    return -1;
  }

  /*
   * When valid string is returned and no errors did occur return 
   * mimetype in return buffer.
   */
  *filetype = calloc(1, strlen(type)+1);
  strcpy(*filetype, type);

  /*
   * All gone well
   */
  return 0;
}

/*
 * Close the magic context
 */
int magicclose(void) 
{
  if(magic == NULL) {
    return -1;
  }

  magic_close(magic);
  
  /*
   * Pointer back to start
   */
  magic=NULL;

  return 0;
}
