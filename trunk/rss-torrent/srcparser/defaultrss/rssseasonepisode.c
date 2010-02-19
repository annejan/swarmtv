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
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <pcre.h>

#include "types.h"
#include "logfile.h"
#include "defaultrss.h"
#include "disectdescription.h"

#define OVECTSIZE 20

/*
 * Gets season and episode from string using regexp
 * Arguments
 * regexp 	regexp string.
 * season		variable to store season in.
 * episode	variable to srore episode in.
 * Returns
 * 0 on match, -1 on nomatch.
 */
static int titleregexp(char *regexp, char *title, int *season, int *episode)
{
  /*
   * Example description.
   * Category: Anime, Seeds: 20, Peers: 103, Size: 353.3 MBs
   * extract: Category, seeds, peers, size
   * execute regexp /Category: ([^,]*), Seeds: ([^,]*), Peers: ([^,]*), Size: (.*)$/
   */
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[OVECTSIZE];
  int   rc;
  int   i;
  char *s_season;
  char *s_episode;

  /*
   * Compile the regexp to retrieve name, season and episode 
   * try :
   * ^(.*)(([sS]([0-9][0-9]?)))?[eE]([0-9][0-9]?))?   name, season, episode
   */
  p = pcre_compile((char*) regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i)", errmsg, errpos);
    exit(1);
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   /* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      (char*)title,                  /* the string to match */
      strlen((char*)title),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      OVECTSIZE);           /* number of elements in the output vector */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
        break;

      default:
        writelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
        exit(1);
    }
    free(p);
    return -1;
  }

  /*
   * extract both strings.
   */
  i = 1;
  s_season = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(s_season, "%.*s", ovector[2*i+1] - ovector[2*i], title + ovector[2*i]);
  i = 2;
  s_episode = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(s_episode, "%.*s", ovector[2*i+1] - ovector[2*i], title + ovector[2*i]);

  /*
   * postprocess information.
   */
  *season = atoi(s_season);
  *episode = atoi(s_episode);

  writelog(LOG_DEBUG, "number of strings found '%d', season '%d' episode '%d'", rc, *season, *episode);

  /*
   * Get the 3 strings and put them in the output strings.
   * Both need to be freed afterwards
   */
  free(s_season);
  free(s_episode);
  pcre_free(p);

  return 0;
}


/*
 * Get season from title
 * Arguments
 * season 	pointer to season int
 * rssdata	RAW rssdata
 * returns
 * 0 on success, otherwise -1
 */
int rssseasonepisode(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int rc=0;
	int foundseason=0;
	int foundepisode=0;
	char *s_season=NULL;
	char *s_episode=NULL;
	int	i_season=0;
	int i_episode=0;

	/*
	 * Try to get the season en episode nummer from the description
	 */
	foundseason  = disectdescription(rssdata->description, "Season", &s_season);
	foundepisode = disectdescription(rssdata->description, "Episode", &s_episode);
	if(foundepisode == 0 && foundseason == 0){
		i_season = atoi(s_season);
		i_episode = atoi(s_episode);

		newtor->season = i_season;
		newtor->episode = i_episode;

		/*
		 * cleanup
		 */
		free(s_season);
		free(s_episode);

		return 0;
	}
	/*
	 * Free anyway to prevent any memoryleaks
	 */
	free(s_season); 
	free(s_episode);

	/*
	 * Match S[0-9]E[0-9]
	 */
	rc = titleregexp("^.*[sS]([0-9][0-9]?) ?[eE] ?([0-9][0-9]?)", rssdata->title, &i_season, &i_episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at specific regexp %s:%d", __FILE__, __LINE__);
		newtor->season 	= i_season;
		newtor->episode = i_episode;
    return 0;
  }

	/*
	 * Match [0-9][0-9]x[0-9][0-9]
	 */
	rc = titleregexp("^.*[^0-9]([0-9][0-9]?) ?[xX] ?([0-9][0-9]?)", rssdata->title, &i_season, &i_episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at specific regexp %s:%d", __FILE__, __LINE__);
		newtor->season 	= i_season;
		newtor->episode = i_episode;
    return 0;
  }

	/*
	 * if that failes try any pair of numbers seperated by a charakter
	 */
	rc = titleregexp("^[^0-9]+[Ss][[:space:]]*([0-9][0-9]?)[^0-9]+([0-9][0-9]?)[^0-9]*$", rssdata->title, &i_season, &i_episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at specific regexp %s:%d", __FILE__, __LINE__);
		newtor->season 	= i_season;
		newtor->episode = i_episode;
    return 0;
  }

	return -1;
}

