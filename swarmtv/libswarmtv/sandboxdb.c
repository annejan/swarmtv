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
 *  Program written by Paul Honig 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sqlite3.h>

#include "types.h"
#include "sandboxdb.h"
#include "logfile.h"
//#include "database.h"
#include "databaseimpl.h"
#include "filesystem.h"


/*
 * Create a filecopy of the database.
 * Arguments 
 * outfile		Destination path
 * infile			Source path
 * returns
 * 0 on success -1 on failure
 */
static int copyfile(char *outfile, char *infile)
{
  int inF, ouF;
  char line[512];
  char *fullinfile=NULL;
  char *fulloutfile=NULL;
  int bytes;
  int rc;

  rsstcompletepath(outfile, &fulloutfile);
  rsstcompletepath(infile, &fullinfile);

  /*
   * Open input and output file.
   */
  if((inF = open(fullinfile, O_RDONLY)) == -1) {
    rsstwritelog(LOG_ERROR, "Could not open read file '%s' %s:%d", infile, __FILE__, __LINE__);
    free(fulloutfile);
    free(fullinfile);
    return -1;
  }

  if((ouF = open(fulloutfile, O_WRONLY | O_CREAT, S_IRWXU )) == -1) {
    rsstwritelog(LOG_ERROR, "Could not open write file '%s' %s:%d", outfile, __FILE__, __LINE__);
    close(inF);
    free(fulloutfile);
    free(fullinfile);
    return -1;
  }

  /*
   * free temp filenames
   */
  free(fulloutfile);
  free(fullinfile);

  /*
   * Do the copying
   */
  while((bytes = read(inF, line, sizeof(line))) > 0){
    rc = write(ouF, line, bytes);
    if(rc == -1) {
      rsstwritelog(LOG_ERROR, "Error writing to file '%s' %s:%d", outfile, __FILE__, __LINE__);
      break;
    }
  }
  if(bytes == -1){
    rsstwritelog(LOG_ERROR, "Error reading from file '%s' %s:%d", infile, __FILE__, __LINE__);
  }

  /*
   * Do the cleanup
   */
  close(inF);
  close(ouF);

  return 0;
}


/*
 * Create copy of database
 * and create a sqlite3 pointer to the database copy.
 * arguments
 * sourcedbname		path to source database
 * sandboxdbname	path to destination database
 * return
 * returns pointer to sandbox db.
 */
sandboxdb *rsstcreatesandbox(char *sourcedbname, char *sandboxdbname)
{
  int       		 rc=0;
  sandboxdb 		*sandbox=NULL;
	rsstor_handle	 handle;

  /*
   * Init struct
   */
  sandbox = calloc(sizeof(sandboxdb), 1);
  sandbox->filename = calloc(strlen(sandboxdbname)+1, 1);
  strcpy(sandbox->filename, sandboxdbname);
  
  /*
   * Create copy of database.
   */
  rc = copyfile(sandboxdbname, sourcedbname);
  if(rc == -1) {
    return NULL;
  }

  /*
   * create pointer to database
   */
  rc = rsstinitdatabase(sandboxdbname, &handle); 
  if(rc != SQLITE_OK) {
    rsstwritelog(LOG_ERROR, "Sandbox database initialization failed %s:%d", __FILE__, __LINE__);
    return NULL;
  }

	/*
	 * Get db pointer to sandbox
	 */
	sandbox->db = handle.db;

  /*
   * Done !
   */
  return sandbox;
}


/*
 * Close the database.
 * delete the sandboxed database.
 * Arguments
 * Sandbox	Pointer the sandbox database
 * Return
 * 0 on succes otherwise -1
 */
int rsstclosesandbox(sandboxdb *sandbox)
{
  int   rc=0;
  char *fullpath=NULL;

  rsstcompletepath(sandbox->filename, &fullpath);

  /*
   * Close the sqlite database.
   * No way to get dbpointer here
   */
  sqlite3_close(sandbox->db);

  /*
   * Delete file
   */
  rc = remove(fullpath);
  if(rc != 0) {
    rsstwritelog(LOG_ERROR, "Removing sandbox db file '%s' failed %s:%d", sandbox->filename, __FILE__, __LINE__);
  }

  /*
   * Free struct
   */
  free(sandbox->filename);
  free(sandbox);

  free(fullpath);
  return 0;
}

