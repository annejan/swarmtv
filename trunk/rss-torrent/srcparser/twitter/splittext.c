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
#include <pcre.h>
#include <sqlite3.h>

#include "curlfile.h"
#include "logfile.h"
#include "twitparse.h"

#define   OVECSIZE 20

/*
 * Get data and link.
 */
int getdataandlink(char *text, char **data, char **link)
{
  pcre 		*p=NULL;
  int   	ovector[OVECSIZE];
  const char *errmsg=NULL;
  int   	errpos=0;
	static char *splitregexp="^(.*) - (https?://[^ ]*).*$";
	int 		i=0;
	int			rc=0;

	/*
	 * Init input variables.
	 */
	*data=NULL;
	*link=NULL;

  /*
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile(splitregexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    fprintf(stderr, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
        errmsg, errpos, __FILE__, __LINE__);
		return -1;
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   	/* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      text,                  /* the string to match */
      strlen(text),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      OVECSIZE);           	/* number of elements in the output vector */

	/*
	 * No match is return 0
	 */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
				/*
				 * no username/password match
				 */
				pcre_free(p);
				return 0;
        break;
      default:
        writelog(LOG_ERROR, "Error while matching url. %d %s:%d", rc, __FILE__, __LINE__);
        break;
    }
    free(p);
    return -1;
  }

	/*
	 * Match, get data and url
	 */
  i = 1;
  *data = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*data, "%.*s", ovector[2*i+1] - ovector[2*i], text + ovector[2*i]);
  i = 2;
  *link = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*link, "%.*s", ovector[2*i+1] - ovector[2*i], text + ovector[2*i]);

	/*
	 * free tempstuff
	 */
	pcre_free(p);
	
	return 1;
}

/*
 * Extract data from text.
 */
int getnamelinkepisode(char *data, char **name, int *season, int *episode)
{
  pcre 		*p=NULL;
  int   	ovector[OVECSIZE];
  const char *errmsg=NULL;
  int   	errpos=0;
	static char *dataregexp="(.*) ?[sS ]([0-9]?[0-9])[eExX ]([0-9]?[0-9])";
	char   	*seasonstr=NULL;
	char		*episodestr=NULL;
	int 		i=0;
	int			rc=0;

	/*
	 * Init input variables.
	 */
	*name=NULL;
	*season=0;
	*episode=0;

  /*
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile(dataregexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    fprintf(stderr, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
        errmsg, errpos, __FILE__, __LINE__);
		return -1;
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   	/* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      data,                  /* the string to match */
      strlen(data),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      OVECSIZE);           	/* number of elements in the output vector */

	/*
	 * No match is return 0
	 */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
				/*
				 * no username/password match
				 */
				pcre_free(p);
				return 0;
        break;
      default:
        writelog(LOG_ERROR, "Error while matching url. %d %s:%d", rc, __FILE__, __LINE__);
        break;
    }
    free(p);
    return -1;
  }

	/*
	 * Match, get data and url
	 */
  i = 1;
  *name = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*name, "%.*s", ovector[2*i+1] - ovector[2*i], data + ovector[2*i]);
  i = 2;
  seasonstr = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(seasonstr, "%.*s", ovector[2*i+1] - ovector[2*i], data + ovector[2*i]);
  i = 3;
  episodestr = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(episodestr, "%.*s", ovector[2*i+1] - ovector[2*i], data + ovector[2*i]);

	/*
	 * translate to int.
	 */
	*episode = atoi(episodestr);
	*season = atoi(seasonstr);

	/*
	 * free tempstuff
	 */
	free(seasonstr);
	free(episodestr);
	pcre_free(p);
	
	return 1;
}


/*
 * Examines the tweet, and splits it into 
 * name
 * link
 * season
 * episode
 * returns 0 on succes -1 on fail.
 */
int splittext(char *text, char **name, char **link, int *season, int *episode) 
{
	char *data=NULL;
	int		rc=0;

	/*
	 * Split the initial data and link.
	 */ 
	rc = getdataandlink(text, &data, link);
	if(rc != 1) {
		writelog(LOG_DEBUG, "Tweet does not hold and url: '%s'", text);
		return -1;
	}

	/*
	 * Split the data, into name, season, episode.
	 */
	rc = getnamelinkepisode(data, name, season, episode);
	if(rc != 1) {
		writelog(LOG_DEBUG, "Tweet does not contain season and episode info: '%s'", text);
		return -1;
	}


	/*
	 * free stuff.
	 */
	free(data);

	return 0;
}
