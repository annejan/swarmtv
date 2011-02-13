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
#include <math.h>
#include <stdlib.h>
#include <pcre.h>
#include <string.h>
#include <ctype.h>

#include "logfile.h"
#include "regexp.h"

/*
 * Number of output vector items
 */
#define   OVECSIZE 20
#define   MATCHSIZE 20



/*
 * This function copies and allocates the destination memory.
 * don't forget to free the destination after use.
 */
int rsstalloccopy(char **dest, const char *src, const size_t size)
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
 * Cleanup strings from XML
 */
void rsstcleanupstring(char *string)
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
   * Remove spaces at begin
   */
  if(*string == ' ') {
    memmove(string, string+1, strlen(string)-1);
    *(string+strlen(string)-1) = '\0';
  }
}


/*
 * Simple routine to compare a string to a regexp
 * @Arguments
 * regexp   Regular expression to match.
 * string   String to match regular expression on.
 * @Returns
 * 0 no match
 * 1 match
 * -1 error
 */
int rsstcomregexp(char *regexp, char *string)
{
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[MATCHSIZE];
  int   rc;
  int   outval = -1;

  /*
   * Compile the regexp to split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* 
		 * Should not happen, because init has already tested and set to NULL on error 
		 */ 
    rsstwritelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
        errmsg, errpos, __FILE__, __LINE__);
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   /* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      string,                  /* the string to match */
      strlen(string),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      MATCHSIZE);           /* number of elements in the output vector */
  /*
   * handle output
   */
  if(rc == PCRE_ERROR_NOMATCH) {
    outval=0;
  } 
  else if (rc == PCRE_ERROR_MATCHLIMIT) {
    rsstwritelog(LOG_ERROR, "PCRE produced PCRE_ERROR_MATCHLIMIT for regexp '%s' and text '%s'", regexp, string);
    outval=-1;
  } else {
    /*
     * Found a match !
     */
    outval=1;
  }

  /*
   * Clean up !
   */
  pcre_free(p);

  return outval;
}


/*
 * This routine returns a string pointing to the first captured string.
 */
int rsstcapturefirstmatch(char *regexp, int flag, char *string, char **match)
{
  pcre *p=NULL;
  const char *errmsg=NULL;
  int   errpos=0;
  int   ovector[MATCHSIZE];
  int   rc=0;
  int   i=0;

  /*
   * Compile the regexp the split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init has already tested and set to NULL on error */
    //rsstwritelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
    //    errmsg, errpos, __FILE__, __LINE__);
    fprintf(stderr, "Ouch! Can't compile regular expression: %s (char %i) %s:%d", errmsg, errpos, __FILE__, __LINE__);
    return -1;
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                    /* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      string,                /* the string to match */
      strlen(string),        /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      flag,                 /* default options */
      ovector,              /* output vector for substring information */
      MATCHSIZE);     /* number of elements in the output vector */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
        //rsstwritelog(LOG_ERROR, "String could not be split. %s:%d", __FILE__, __LINE__);
        //fprintf(stderr, "No match found '%s'. %s:%d", regexp, __FILE__, __LINE__);
        break;

      default:
        fprintf(stderr, "Error in regexp matching '%s'. %s:%d\n", regexp, __FILE__, __LINE__);
        //rsstwritelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
        break;
    }
    free(p);
    return rc;
  }

  /*
   * extract both strings.
   */
  i = 1;
  *match = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*match, "%.*s", ovector[2*i+1] - ovector[2*i], string + ovector[2*i]);

  //printf("o'%s' r'%s' b%d e%d s%d v'%s'\n", string, regexp,  ovector[2*i], ovector[2*i+1], ovector[2*i+1]-ovector[2*i], *match);

  /*
   * Get the 2 strings and put them in the output strings.
   * Both need to be freed afterwards
   */

  pcre_free(p);

  return 0;
}


/*
 * Extract user name and password from URL
 * Accepts passwords in the URL https://<user>:<password>@<host>:<port>/<path>
 * returns 0 on no user name/passwd
 * When returning 1 cleanurl and userpass are both set, and should be freed after use.
 */
int rsstgetusernamepassword(char *url, char **cleanurl, char **userpass)
{
  pcre 		*p=NULL;
  int   	ovector[OVECSIZE];
  const char *errmsg=NULL;
  int   	errpos=0;
	char 		*regexp="^(https?://)([^:]+:[^@]+)@(.*)$";
	char 		*protocol=NULL;
	char 		*hostpath=NULL;
	int 		i=0;
	size_t	urlsize=0;	
	int			rc=0;

	/*
	 * Init input variables.
	 */
	*cleanurl=NULL;
	*userpass=NULL;

  /*
   * Compile the regexp to split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init has already tested and set to NULL on error */
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
      url,                  /* the string to match */
      strlen(url),          /* the length of the string */
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
				 * no user name/password match
				 */
				pcre_free(p);
				return 0;
        break;
      default:
        rsstwritelog(LOG_ERROR, "Error while matching URL. %d %s:%d", rc, __FILE__, __LINE__);
        break;
    }
    free(p);
    return -1;
  }

	/*
	 * Match, get URL and userpass
	 */
  i = 1;
  protocol = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(protocol, "%.*s", ovector[2*i+1] - ovector[2*i], url + ovector[2*i]);
  i = 2;
  *userpass = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(*userpass, "%.*s", ovector[2*i+1] - ovector[2*i], url + ovector[2*i]);
  i = 3;
  hostpath = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(hostpath, "%.*s", ovector[2*i+1] - ovector[2*i], url + ovector[2*i]);

	/*
	 * Set cleanurl and userpass
	 * return 1
	 */
	urlsize = strlen(protocol) + strlen(hostpath) + 1;
  *cleanurl = calloc(1, urlsize);
	snprintf(*cleanurl, urlsize, "%s%s", protocol, hostpath);

	/*
	 * free tempstuff
	 */
	pcre_free(p);
	free(protocol);
	free(hostpath);
	
	return 1;
}

