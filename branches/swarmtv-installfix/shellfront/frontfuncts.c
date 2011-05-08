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
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <pcre.h>
#include <ctype.h>
#include <math.h>

#include <swarmtv.h>

#include <handleopts.h>

#define OVECSIZE 20
#define BUFSIZE  20

/*
 * Used for unit conversion see humantosize and sizetohuman
 */
static const char* units[]    = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
static const char* unitmatch  = "BKMGTPEZY";

/*
 * Convert strings to lowercase
 * @arguments
 * string string to convert
 */
void rssflowercase(char string[])
{
	int  i = 0;
	while ( string[i] )
	{
		string[i] = tolower(string[i]);
		i++;
	}
	return;
}


/*
 * This function copies and allocates the destination memory.
 * don't forget to free the destination after use.
 */
int rssfalloccopy(char **dest, const char *src, const size_t size)
{
	/*
	 * Allocate the memory
	 */
	*dest=calloc(1, size+1);

	/*
	 * Copy the string to it.
	 */
	memcpy(*dest, src, size);

	/*
	 * Return 0 on success, otherwise one-zero
	 */
	return !*dest;
}


/*
 * Split options
 * When options come in as <name>:<value> split them 
 * the first ':' found is the one splitting name and value
 * When the splitting failed '-1' is returned
 */
int rssfsplitnameval(char *input,char **name, char **value) 
{
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[OVECSIZE];
  int   rc;
  int   i;

  /*
   * Compile the regexp to split the two strings.
   */
  p = pcre_compile("^([^:]+):(.*)$", 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init has already tested and set to NULL on error */
    rsstwritelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
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
        rsstwritelog(LOG_NORMAL, "String could not be split. %s:%d", __FILE__, __LINE__);
        break;

      default:
        rsstwritelog(LOG_NORMAL, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
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
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns the char * to the converted string.
 */
char* rssfsizetohuman(size_t size/*in bytes*/, char *buf) 
{
  int i = 0;
  double tempsize;

  tempsize = size;

  while (tempsize / 1024 > 1) {
    tempsize /= 1024;
    i++;
  }
  snprintf(buf, BUFSIZE, "%.*f %s", i, (double) tempsize, units[i]);
  return buf;
}


/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns 0 and -1 on error
 * size in bytes is returned in argument size
 */
int rssfhumantosize(char *buf, double *size) 
{
  char    upcasenum[BUFSIZE+1];
  char    *unit=NULL;
  int     i=0;
  long double  tempsize=0.0;
  int     power=0;

  /*
   * When buf or size = NULL, return -1
   */
  if( buf == NULL || size == NULL){
    rsstwritelog(LOG_ERROR, "Invalid pointer passed to humantosize function. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Initialize stuff
   */
  memset(upcasenum, 0, BUFSIZE+1);
  strncpy(upcasenum, buf, BUFSIZE);

  /*
   * transform the human readable string to a power of 1024
   */
  for( i = 0; upcasenum[ i ]; i++) 
  {
    upcasenum[ i ] = toupper( upcasenum[ i ] );
  }

  /*
   * returns a pointer to the first occurrence in string s1 of any character from string s2, or a null pointer if no character from s2 exists in s1
   */
  unit = strpbrk(upcasenum, unitmatch);

  /*
   * Get size 
   */
  tempsize = atof(upcasenum);
  
  /*
   * when no unit is found use a power of 1024^0
   * Otherwise calculate number of bytes
   */
  if(unit != NULL){
    /*
     * Calculate the number of bytes out.
     */
    while(*(unitmatch + power) != '\0'){
      if(*unit == *(unitmatch + power)) {
        break;
      }
      power++;
    }
  }

  /*
   * Calculate response
   */
  *size = (double) tempsize * pow(1024, power);

  return 0;
}


/*
 * Get the last season and episode.
 * Adding them to the simple filter struct.
 * @Argument
 * simple
 * @Return 
 * 0 on success
 * -1 on error
 */
int insertseasonepisode(simplefilter_struct *filter)
{
	int rc=0;
	int season=0;
	int episode=0;

	/*
	 * If season and/or fromepisode is set, don't apply autoseason
	 */
	if(filter->fromseason == 0 && filter->fromepisode == 0){
		rc = rsstgetnewestepisode(filter, &season, &episode);
		if(rc < 0) {
			return -1;
		}
		rsstwritelog(LOG_DEBUG, "Last Season: '%d' Last Episode: '%d'\n", season, episode);
		filter->fromseason = season;
		filter->fromepisode = episode;
	}

	return 0;
}


/*
 * optstosimple
 * Takes takes a opts_struct argument and a simplefilter_struct as argument.
 * returns 0 on success, -1 on error.
 */
int rssfoptstosimple(opts_struct *opts, simplefilter_struct *simple)
{
	int rc=0;

  /*
   * Copy strings
   */
  rssfalloccopy(&(simple->name),   opts->simplename,   strlen(opts->simplename));
  rssfalloccopy(&(simple->nodup),  opts->simplenodup,  strlen(opts->simplenodup));

  /*
   * Title filter is optional
   */
  if(opts->simpletitle != NULL) {
    rssfalloccopy(&(simple->title),  opts->simpletitle,  strlen(opts->simpletitle));
  } else {
    rssfalloccopy(&(simple->title), "", 1);
  }
  if(opts->simpleexclude != NULL) {
    rssfalloccopy(&(simple->exclude),  opts->simpleexclude,  strlen(opts->simpleexclude));
  } else {
    rssfalloccopy(&(simple->exclude), "", 1);
  }
  if(opts->simplecategory != NULL) {
    rssfalloccopy(&(simple->category),  opts->simplecategory,  strlen(opts->simplecategory));
  } else {
    rssfalloccopy(&(simple->category), "", 1);
  }
  if(opts->simplesource != NULL) {
    rssfalloccopy(&(simple->source),  opts->simplesource,  strlen(opts->simplesource));
  } else {
    rssfalloccopy(&(simple->source), "", 1);
  }
  if(opts->simpleseason != NULL) {
    simple->fromseason=atoi(opts->simpleseason); 
  } 
  if(opts->simpleepisode != NULL) {
    simple->fromepisode=atoi(opts->simpleepisode); 
  } 

	/*
	 * When auto from-season and from-episode fill out is enabled, do so
	 */
	if(opts->autoseasep == 1) {
		rc = insertseasonepisode(simple);
		if(rc != 0){
			fprintf(stderr, "No prior season/episode number found in database.\n");
		}
	}

  /*
   * Convert units 
   */
  if(opts->simplemaxsize != NULL) {
    rssfhumantosize(opts->simplemaxsize, &(simple->maxsize));
  } else {
    simple->maxsize = 0;
  }
  if(opts->simpleminsize != NULL) {
    rssfhumantosize(opts->simpleminsize, &(simple->minsize));
  } else {
    simple->minsize = 0;
  }

  /*
   * Done
   */
  return 0;
}

