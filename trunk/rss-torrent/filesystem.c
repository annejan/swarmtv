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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "regexp.h"

#define TESTFILENAME "test.file"

/*
 * Complete path
 * when the first char of filename is a ~ it gets replaced by 
 * the path to the homedir
 * free destpath afterwards
 * Arguments
 * origpath	The path containing '~' and stuff
 * destpath	The path compleded and all
 */
void completepath(const char *origpath, char **destpath)
{
  int   lenght=0;
  char *homedir=NULL;

	/*
	 * Test input
	 */
	if(origpath == NULL) {
		return;
	}

  /*
   * Test if the string starts with a '~'
   */
  if(*origpath == '~') {
    homedir = getenv("HOME");

    /*
     * if it does insert home path in place of '~'
     */
    lenght = strlen(origpath)-1;
    lenght += strlen(homedir);
    lenght++; // we add an extra '/' in the path

    /*
     * Alloc and build output
     */
    *destpath = calloc(lenght+1, 1);
    sprintf(*destpath, "%s/%s", homedir, origpath+1);
  } else { 
    /*
     * if not copy origpath to destpath and allocate memory
     */
		alloccopy(destpath, (char*) origpath, strlen(origpath));
  }
}


/*
 * Test if a file or directory exists
 * return 0 when found, -1 if not found
 * Arguments
 * path			
 * returns
 * 0 on success, -1 on error (or when not found).
 */
int fsexists(const char *path) 
{
  struct stat st;
  int rc=0;

  /*
   * Stat the file
   */
  rc = stat(path, &st);

  return rc;
}

/*
 * Test if a directory is writable
 * This is done by creating a testfile named "test.file",
 * and deleting it afterwards.
 * Arguments
 * path		Path to file to be tested.
 * Return
 * 0 on succes, -1 on failure (not writable)
 */
int testwrite(const char *path)
{
  int   rc=0;
  int   fsock=0;
  int   lenght=0;
  char  *filename=NULL;

  /*
   * Create path
   */
  lenght = strlen(path);
  lenght += strlen(TESTFILENAME);
  lenght++; // we add an extra '/' in the path
  filename = calloc(lenght+1, 1);
  sprintf(filename, "%s/%s", path, TESTFILENAME);

  /*
   * Create testfile
   */
  fsock = creat(filename, S_IRWXU);
  if(fsock != 0) {
    /*
     * Close the file.
     */
    close(fsock);

    /*
     * Delete file when creation was successfull
     */
    rc = remove(filename); 
  }

  free(filename);
 
  /*
   * check tests
   */
  if(fsock != 0 && rc == 0) {
    return 0;
  } else {
    return -1;
  }
}


/*
 * Create directory
 * Arguments
 * path path to directory
 * returns
 * 0 on succes, -1 on failure
 */
int makedir(char *path)
{
	int rc;

	/*
	 * Create directory depending on users umask.
	 */
	rc = mkdir(path, 0777);

	return rc;
}
