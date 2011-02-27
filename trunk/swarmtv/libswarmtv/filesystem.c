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
#ifdef __MINGW32__
#include <windows.h>
#define _WIN32_IE 0x0400
#include "shlobj.h"
#endif

#include "regexp.h"

#define TESTFILENAME "test.file"

/*
 * Complete path
 * when the first char of filename is a ~ it gets replaced by 
 * the path to the home directory
 * free destination path afterwards
 * Arguments
 * origpath	The path containing '~' and stuff
 * destpath	The path completed and all
 */
void rsstcompletepath(const char *origpath, char **destpath)
{
  int   lenght=0;
  #ifdef __MINGW32__
  char homedir[MAX_PATH];
  #else
  char *homedir=NULL;
  #endif

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
#ifdef __MINGW32__
    /* let's assume Windows gives me a folder
     * and not check for 0 every step of the way
     */

    SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, homedir);
    /* might need some UNICODE safety here !! */
    //LPSTR homefolder=NULL;
    //int cw=lstrlenW(homefolder);
    //int cc=WideCharToMultiByte(CP_ACP, 0, homefolder, cw, NULL, 0, NULL, NULL);
    /* now we know how many chars */
    //cc=WideCharToMultiByte(CP_ACP, 0, homefolder, cw, homedir, cc, NULL, NULL);
    //homedir[cc]='\0';
    //delete homefolder;
#else
    homedir = getenv("HOME");
#endif

    /*
     * if it does insert home path in place of '~'
     */
    lenght = strlen(origpath)-1;
    lenght += strlen(homedir);
    lenght++; // we add an extra '/' in the path

    /*
     * Allocate and build output
     */
    *destpath = calloc(lenght+1, 1);
    sprintf(*destpath, "%s/%s", homedir, origpath+1);
  } else { 
    /*
     * if not copy origpath to destpath and allocate memory
     */
		rsstalloccopy(destpath, (char*) origpath, strlen(origpath));
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
int rsstfsexists(const char *path) 
{
  struct stat st;
  int 		rc=0;
	char 		*fullpath=NULL;

	/*
	 * Get full path
	 */
  rsstcompletepath(path, &fullpath);

  /*
   * Stat the file
   */
  rc = stat(fullpath, &st);

	/*
	 * Cleanup
	 */
	free(fullpath);

  return rc;
}

/*
 * Test if a directory is writable
 * This is done by creating a test file named "test.file",
 * and deleting it afterwards.
 * Arguments
 * path		Path to file to be tested.
 * Return
 * 0 on success, -1 on failure (not writable)
 */
int rssttestwrite(const char *path)
{
  int   rc=0;
  int   fsock=0;
  int   lenght=0;
  char  *filename=NULL;
	char	*fullpath=NULL;

	/*
	 * Full path
	 */
	rsstcompletepath(path, &fullpath);

  /*
   * Create path
   */
  lenght = strlen(fullpath);
  lenght += strlen(TESTFILENAME);
  lenght++; // we add an extra '/' in the path
  filename = calloc(lenght+1, 1);
  sprintf(filename, "%s/%s", fullpath, TESTFILENAME);

  /*
   * Create test file
   */
  fsock = creat(filename, S_IRWXU);
  if(fsock != 0) {
    /*
     * Close the file.
     */
    close(fsock);

    /*
     * Delete file when creation was successful
     */
    rc = remove(filename); 
  }

  free(filename);
	free(fullpath);
 
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
 * 0 on success, -1 on failure
 */
int rsstmakedir(char *path)
{
	int rc;

	/*
	 * Create directory depending on users umask.
	 */
	#ifdef __MINGW32__  
	rc = mkdir(path);
	#else
	rc = mkdir(path, 0777);
	#endif

	return rc;
}

