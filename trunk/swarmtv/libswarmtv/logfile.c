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
#include <time.h>
#include <stdarg.h>
#include <sqlite3.h>

#include "swarmtv.h"
#include "types.h"
#include "config.h"
#include "filesystem.h"
#include "callback.h"

#define LOG_DEBUG   1
#define LOG_NORMAL  2
#define LOG_ERROR   3

/*
 * Define log levels
 */
#ifdef DEBUG
  #define LOG_LEVEL 0
#endif

#ifdef ERROR
  #define LOG_LEVEL 2
#endif

#ifndef DEBUG
#ifndef ERROR
  #define LOG_LEVEL 1
#endif
#endif



int rsstinitlog(char *logpath);

/*
 * Open log file getting path from database
 */
int rsstinitlogdb(sqlite3 *db)
{
  char  *configpath = NULL;
  int   rc=9;
	rsstor_handle handle;

	/*
	 * REMOVE IN THE FUTURE
	 */
  memset(&handle, 0, sizeof(rsstor_handle));
	handle.db = db;

  /*
   * Get path
   * int configgetproperty(sqlite3 *db, char *prop, char **value);
   */
  rc =rsstconfiggetproperty(&handle, CONF_LOGFILE, &configpath);

  /*
   * open log file
   */
  if(rc == 0){
    rc = rsstinitlog(configpath);
  }

  /*
   * clean up
   */
  free(configpath);

  /*
   * Return rc
   */
  return rc;
}


/*
 * Initialize log file
 * @Arguments
 * logpath path to the logfile
 * @return Return 0 when success, Return -1 when fail
 */
int rsstinitlog(char *logpath)
{
	FILE *log=NULL;
  char *fullpath=NULL;
  int retval=0;

  rsstcompletepath(logpath, &fullpath);

  log = fopen(fullpath,"a+");
  if(log != NULL) {
    fclose(log);
  } else {
    fprintf(stderr, "Could not initiate logfile at '%s'\n", fullpath);
    fprintf(stderr, "log to default out instead.\n");
    log=stdout;
  }

  free(fullpath);

  /*
   * Return code.
   */
  return retval;
}

/*
 * Log an entry
 */
int rsstwritelog(int level, char *str,...)
{
  char            s[30];
  //size_t          i=0;
  //struct tm       tim;
  //time_t          now=0;

  if(level > LOG_LEVEL) {
    /*
     * What time is now ?
     */
    //now = time(NULL);
    //tim = *(localtime(&now));
    //i = strftime(s,30,"%H:%M:%S",&tim);

    /*
     * Build string
     */
    printf("%s ",s);
    switch(level) {
      case LOG_DEBUG:
        printf("(D)");
        break;
      case LOG_NORMAL:
        printf("(N)");
        break;
      case LOG_ERROR:
        printf("(E)");
        break;
    }
    printf(" : ");

    /*
     * Put the error out there
     */
    va_list arglist;
    va_start(arglist,str);
    vprintf(str,arglist);
    va_end(arglist);
    printf(" \n");

		fflush(stdout);
  }

  return 1;
}


/*
 * Close log file
 */
void rsstcloselog()
{
  /*
   * Closing log file.
   */
}
