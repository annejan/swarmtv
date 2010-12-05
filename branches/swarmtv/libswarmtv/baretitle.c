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
 *  Program written by Paul Honig 2009 - 2010
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <sqlite3.h>
#include <time.h>

#include "types.h"
#include "regexp.h"
#include "logfile.h"

/*
 * Isolate the title from the title captured from the RSS-title
 * @Arguments
 * downed      Title provided by the RSS feed
 * @Return
 * 0 on success otherwise -1
 */
int rsstfillbaretitle(downloaded_struct *downed)
{
  int   rc=0;
  int   retval=0;
  char *title=NULL;
  char *baretitle=NULL;

  /*
   * Regexp to strip bare title from title.
   */
  char *bareregexp="([^0-9]*?)[ ]?[sS]?[0-9]";

  /*
   * initialize local pointer
   */
  title=downed->title;

  /*
   * Get the bare title
   */
  rc = rsstcapturefirstmatch(bareregexp, 0, title, &baretitle);
  if(rc == PCRE_ERROR_NOMATCH){
    /*
     * When the regexp failed to match take the whole title.
     */
    rsstalloccopy(&baretitle, title, strlen(title));
  } 
  else if(rc < 0){
    /*
     * Problems with regexp.
     */
    rsstwritelog(LOG_ERROR, "Regep failed in baretitle %s:%d", __FILE__, __LINE__);
    retval = -1;
  }

  //printf("title:      '%s'\n", title);
  //printf("baretitle:  '%s'\n", baretitle);

  /*
   * write back bare title.
   */
  downed->baretitle=baretitle; 

  /*
   * return outcome.
   */
  return retval;
}


/*
 * Transform the bare title to a regexp matching the bare title
 * @Arguments
 * baretitle Title to transform
 * bareregexp regexp to return 
 */
void rsstmakebareregexp(char *baretitle, char **bareregexp)
{
  int regsize=0;

  /*
   * Check sanity
   */
  if(baretitle == 0) {
    rsstwritelog(LOG_ERROR, "NULL pointer passed to rsstmakebareregexp function. %s:%d", __FILE__, __LINE__);
    return;
  }

  /*
   * Front and back additions
   */
  char *frontreg  = "^";
  char *endreg    = "";

  /*
   * Calculate size of the regexp
   */
  regsize = strlen(frontreg);
  regsize += strlen(endreg);
  regsize += strlen(baretitle);
  regsize++;

  /*
   * Allocate memory and print the regexp there
   */
  *bareregexp = calloc(1, regsize);
  snprintf(*bareregexp, regsize, "%s%s%s", frontreg, baretitle, endreg);

  /*
   * @@DEBUG
   */
  //printf("bareregexp: '%s'\n", *bareregexp);

  /*
   * Done
   */
}
