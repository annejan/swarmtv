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

// Number of output vectoritems
#define   OVECSIZE 20
#define   MATCHSIZE 20

/*
 * Used for unitconversion see humantosize and sizetohuman
 */
static const char* units[]    = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
static const char* unitmatch  = "BKMGTPEZY";

/*
 * This function copies and allocates the destination memory.
 * don't forget to free the destination after use.
 */
int alloccopy(char **dest, char *src, size_t size)
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
	 * Return 0 on succes, otherwise one-zero
	 */
	return !*dest;
}

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

/*
 * Simple routine to compare a string to a regexp
 */
int comregexp(char *regexp, char *string)
{
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[MATCHSIZE];
  int   rc;
  int   outval = -1;

  /*
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    //writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
    //    errmsg, errpos, __FILE__, __LINE__);
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
  switch (rc) {
    case 0:
      outval = 1;
      break;
    case PCRE_ERROR_NOMATCH:
      outval = 0;
      break;

    default:
      fprintf(stderr, "Error evaluating password!\n");
      //writelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
      break;
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
int capturefirstmatch(char *regexp, int flag, char *string, char **match)
{
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[MATCHSIZE];
  int   rc;
  int   i;

  /*
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    //writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d",
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
        //writelog(LOG_ERROR, "String could not be split. %s:%d", __FILE__, __LINE__);
        //fprintf(stderr, "No match found '%s'. %s:%d", regexp, __FILE__, __LINE__);
        break;

      default:
        fprintf(stderr, "Error in regexp matching '%s'. %s:%d\n", regexp, __FILE__, __LINE__);
        //writelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
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
 * Extract username and password from url
 * Accepts passwords in the url https://<user>:<password>@<host>:<port>/<path>
 * returns 0 on no username/passwd
 * When returning 1 cleanurl and userpass are both set, and should be freed after use.
 */
int getusernamepassword(char *url, char **cleanurl, char **userpass)
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
   * Compile the regexp te split the two strings.
   */
  p = pcre_compile(regexp, 0, &errmsg, &errpos, 0);
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
	 * Match, get url and userpass
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

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns the char * to the converted string.
 */
char* sizetohuman(size_t size/*in bytes*/, char *buf) 
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
int humantosize(char *buf, double *size) 
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
    writelog(LOG_ERROR, "Invalid pointer passed to humantosize function. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Initialize stuff
   */
  memset(upcasenum, 0, BUFSIZE+1);
  strncpy(upcasenum, buf, BUFSIZE);

  /*
   * transform the humanreadable string to a power of 1024
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
   * Calulate response
   */
  *size = (double) tempsize * pow(1024, power);

  return 0;
}

/*
 * Credits to Gilles Kohl for providing the base code.
 *
 * strrepl: Replace OldStr by NewStr in string Str.
 *
 * The empty string ("") is found at the beginning of every string.
 *
 * Returns:  0  When replace succesful
 *          -1  When no replace was done
 *
 * **Str must not be on strack, because it gets reallocced.
 */ 
int strrepl(char **Str, char *OldStr, char *NewStr)
{
  size_t OldLen;
  size_t NewLen;
  size_t TotalNew;
  char *p, *q;

  /*
   * When not found return origional string
   */
  if(NULL == (p = strstr(*Str, OldStr)))
  {
    return -1;
  }
  
  /*
   * Calculate the size of the new string
   */
  OldLen = strlen(OldStr);
  NewLen = strlen(NewStr);
  TotalNew = (strlen(*Str) + NewLen - OldLen + 1);
  
  /*
   * Alloc space for new string
   */
  if(TotalNew > strlen(*Str)){
    *Str=realloc(*Str, TotalNew);
    Str[TotalNew]='\0';
  }

  /*
   * Build new string
   */
  memmove(q = p+NewLen, p+OldLen, strlen(p+OldLen)+1);
  memcpy(p, NewStr, NewLen);
  return 0;
}


#if 0
/*
 * Replacel all occurences of OldStr te NewStr in Str
 * returns 0 
 */
int strreplall(char **Str, char *OldStr, char *NewStr)
{
  int rc = 0;

  while(rc == 0) {
    rc = strrepl(Str, OldStr, NewStr);
  }
  
  /*
   * return last value
   */
  return 0;
}
#endif
