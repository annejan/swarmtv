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
#include <time.h>
#include <stdarg.h>
#include <sqlite3.h>

#include "config.h"

#define LOG_DEBUG   1
#define LOG_NORMAL  2
#define LOG_ERROR   3

/*
 * Define loglevels
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


static FILE *log=NULL;

int initlog(char *logpath);

/*
 * Open logfile getting path from database
 */
int initlogdb(sqlite3 *db)
{
  char  *configpath = NULL;
  int   rc;

  /*
   * Get path
   * int configgetproperty(sqlite3 *db, char *prop, char **value);
   */
  rc = configgetproperty(db, CONF_LOGFILE, &configpath);

  /*
   * open log file
   */
  rc = initlog(configpath);

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
 * Initialize logfile
 * Return 0 when succes
 * Return !0 when fail
 */
int initlog(char *logpath)
{
  log = fopen(logpath,"a");

  /*
   * Return code.
   */
  return (int) !log;
}

/*
 * Log an entry
 */
int writelog(int level, char *str,...)
{
  char s[30];
  size_t i;
  struct tm tim;
  time_t now;

  if(level > LOG_LEVEL) {

    /*
     * log is null, file is not open.
     */
    if (log==NULL) {
      fprintf(stderr, "Logfile not initialized!\n");
      return -1;
    }

    /*
     * What time is now ?
     */
    now = time(NULL);
    tim = *(localtime(&now));
    i = strftime(s,30,"%H:%M:%S",&tim);

    /*
     * Build string
     */
    fprintf(log,"%s ",s);
    switch(level) {
      case LOG_DEBUG:
        fprintf(log,"(D)");
        break;
      case LOG_NORMAL:
        fprintf(log,"(N)");
        break;
      case LOG_ERROR:
        fprintf(log,"(E)");
        break;
    }
    fprintf(log," : ");

    va_list arglist;
    va_start(arglist,str);
    vfprintf(log,str,arglist);
    va_end(arglist);
    fprintf(log," \n");


    fflush(log);
  }

  return 1;
}


/*
 * Close logfile
 */
void closelog()
{
  /*
   * Closing logfile.
   */
  fclose(log);
}
