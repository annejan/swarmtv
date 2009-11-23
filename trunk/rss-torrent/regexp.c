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
#include <pcre.h>
#include <string.h>

#include "logfile.h"

// Number of output vectoritems
#define   OVECSIZE 10

/*
 * Split options
 * When options come in as <name>:<value> split them 
 * the first ':' found is the one plitting name and value
 * When the splitting failed '-1' is returned
 */
int splitnameval(char *input,char **name, char **value) 
{
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[OVECSIZE];
  int   rc;
  int   i;

  /*
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile("^([^:]+):(.+)$", 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
        errmsg, errpos, __FILE__, __LINE__);
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   /* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      input,                  /* the string to match */
      strlen(input),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      OVECSIZE);           /* number of elements in the output vector */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
        writelog(LOG_ERROR, "String could not be split. %s:%d", __FILE__, __LINE__);
        break;

      default:
        writelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
        break;
    }
    free(p);
    return -1;
  }

  /*
   * extract both strings.
   */
  i = 1;
  *name = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*name, "%.*s", ovector[2*i+1] - ovector[2*i], input + ovector[2*i]);
  i = 2;
  *value = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*value, "%.*s", ovector[2*i+1] - ovector[2*i], input + ovector[2*i]);


  /*
   * Get the 2 strings and put them in the output strings.
   * Both need to be freed afterwards
   */

  pcre_free(p);

  return 0;
}

/*
 * Cleanup strings from XML
 */
void cleanupstring(char *string)
{
  char *cur;

  /*
   * check if sane
   */
  if(strlen(string) == 0){
    return;
  }

  /*
   * translate unwanted chars to spaces.
   */
  cur = string;
  while(*cur != '\0') {
    switch(*cur){
      case '.':
      case '_':
        *cur = ' ';
        break;
    }
    cur++;
  }

  /*
   * Remove spaces at end
   */
  if(*(cur-1) == ' '){
    *(cur-1) = '\0';
  }

  /*
   * Remove spaces at begin
   */
  if(*string == ' ') {
    memmove(string, string+1, strlen(string)-1);
    *(string+strlen(string)-1) = '\0';
  }
}

