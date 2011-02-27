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
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "types.h"
#include "logfile.h"
#include "config.h"
#include "regexp.h"
#include "defaultrss.h"
#include "filehandler/filehandler.h"
#include "disectdescription.h"
#include "database.h"

#ifdef __MINGW32__
#include "workarounds.h"
#endif

/*
 * Used for unit conversion see humantosize and sizetohuman
 */
#define BUFSIZE  20
//static const char* units[]    = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
static const char* unitmatch  = "BKMGTPEZY";

/*
 * Remove any XML strings from the string
 * free **out after use.
 * @Arguments
 * in
 * out
 * @return
 */
static int rsstremoveescaped(char *in, double *size, char **out)
{
  char *cread=NULL;
  char *cwrite=NULL;
  char *cout=NULL;
  int copy=1; // 1 is copy, 0 is don't 

  cread=in;
  cwrite=calloc(1, ((size_t)*size) +1);
  cout=cwrite;
  if(cwrite == NULL)
  {
    return 0;
  }

  while(*cread != '\0'){
    // Copy off on start escape
    if(*cread == '&')
    {
      copy=0;
    }

    // When copy is on, copy the data
    if(copy == 1)
    {
      *cwrite=*cread;
      cwrite++;
    }

    // Copy on end of escape
    if(*cread == ';')
    {
      copy=1;
    }
    
    cread++;
  }

  *out=cout;
  
  return 0;
}

/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns 0 and -1 on error
 * size in bytes is returned in argument size
 */
int rssthumantosize(char *buf, double *size) 
{
  char    upcasenum[BUFSIZE+1];
  char    *unit=NULL;
  char    *stripped=NULL;
  int     i=0;
  long double  tempsize=0.0;
  int     power=0;
  int     rc=0;

  /*
   * When buf or size = NULL, return -1
   */
  if( buf == NULL || size == NULL){
    rsstwritelog(LOG_ERROR, "Invalid pointer passed to humantosize function. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Remove any XML escaped characters.
   */
  rc = rsstremoveescaped(buf, size, &stripped);
  if(rc != 0){
    rsstwritelog(LOG_ERROR, "Could not parse escaped strings.k%s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Initialize stuff
   */
  memset(upcasenum, 0, BUFSIZE+1);
  strncpy(upcasenum, stripped, BUFSIZE);
  free(stripped);

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
 * Get the size from the description
 * @Arguments
 * rssdata Structure containing data from record.
 * size found size from description
 * @Return
 * returns 0 on success, otherwise -1
 */
static int rsstsizefromdesription(rssdatastruct *rssdata, size_t *foundsize)
{
  int retval=-1;
  int rc=0;
  char *descstr=NULL;
  char *token=NULL;
  char *origptr=NULL;
  char *sizestr=NULL;
  double descsize=0.0;

  const char *sizetoken="size";
  const char *delim="\n";

  /*
   * Sanity check
   */
  if(rssdata->description == NULL){
    return -1;
  }

  /*
   * Copy description
   */
  rsstalloccopy(&descstr, rssdata->description, strlen(rssdata->description));
  token=descstr;
  origptr=descstr;

  /*
   * Look at description, find size
   */
  token = strsep(&descstr, delim); 
  while(token != NULL){
    /*
     * Find a line starting with "Size"
     */
    if(strncasecmp(token, sizetoken, strlen(sizetoken)) == 0) {

      /*
       * When found, put into size to human.
       */
      sizestr=token+strlen(sizetoken);
      rc = rssthumantosize(sizestr, &descsize); 
      if(rc == 0){
        retval=0;
        *foundsize = descsize; 
      } else {
        rsstwritelog(LOG_ERROR, "Could not get size from '%s' %s:%d", sizestr, __FILE__, __LINE__);
     }

      break;
    }
    
    token = strsep(&descstr, delim); 
  }

  /*
   * Clean up
   */
  free(origptr);

  /*
   * All done
   */
  return retval;
}

static int rsstsizefromdatabase(rssdatastruct *rssdata, size_t *size) 
{
  int rc=0;
  char *result=NULL;
  rsstor_handle *handle=NULL;
  char *link=NULL;

  /* 
   * Query to retrieve the size from the database
   */
  char *query="select size from newtorrents where link=?1";

  /*
   * Get the handle
   */
  handle = rssdata->handle;

  /*
   * Choose between link and torrentlink
   */
  if(rssdata->torrentlink != NULL) {
    link=rssdata->torrentlink;
  } else {
    link=rssdata->link;
  }

  /*
   * execute query to retrieve the size stored in the database
   */
  rc = rsstdosingletextquery(handle->db, (unsigned char const**) &result, query, "s", link);
  if(rc != 0 || result == NULL) {
    /*
     * not yet in database.
     */
    return -1;
  }

  /*
   * When found set the new size 
   */
  *size = (size_t) atol(result);
  free(result);

  /*
   * Success !
   */
  return 0;
}

/*
 * Use the rss data to fill out the seeds and peers field in the new torrent struct.
 * @Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * @Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsssize(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int				    rc=0;
	size_t 		    i_size=0;
	size_t 		    i_length=0;
  size_t        i_contlength=0;
	long			    min_config=0;
	metafileprops	*props=NULL;
  char          *torlink=NULL;
	int				    retval=0;
  METAFILETYPE  type=undefined;

	/*
	 * Get from size node.
	 */
	i_size = rssdata->size;

	/*
	 * Get size from enclosure length
	 */
	i_length = rssdata->enclosurelength;

  /*
   * Get the content length
   */
  i_contlength = rssdata->contentlength;

	/*
	 * Pick the biggest.
	 */
	if(i_size > i_length) {
		newtor->size = i_size;
	} else {
		newtor->size = i_length;
	}
  if(newtor->size < i_contlength) {
    newtor->size = i_contlength;
  }

  /*
   * convert metafile type
   */
  rc = metafilestrtotype(newtor->metatype, &type);
  if(rc != 0){
		rsstwritelog(LOG_ERROR, "Not corrent meta type '%s' %s:%d", 
      newtor->metatype, __FILE__, __LINE__ );
		return -1;
  }

	/*
	 * When smaller than 'min_size'
	 */
	rc = rsstconfiggetlong(rssdata->handle, CONF_MIN_SIZE , &min_config);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Configuration variable '%s' not set!", CONF_MIN_SIZE);
		return -1;
	}

  /*
   * When smaller than min_size get size from the database
   */
	if( newtor->size < (size_t) min_config) {
    rsstsizefromdatabase(rssdata, &(newtor->size)); 
  }

  /*
   * Get the size from the description
   */
	if( newtor->size < (size_t) min_config) {
    rc = rsstsizefromdesription(rssdata, &(newtor->size));
  }

  /*
   * When still smaller than size resort to downloading the metafile and getting the size that way.
   */
	if( newtor->size < (size_t) min_config) {
    if(rssdata->torrentlink != NULL){
      torlink = rssdata->torrentlink;
    } else {
      torlink = rssdata->link;
    }
    printf("Downloading to double check: '%s'\n", torlink);
		/*
		 * Download the torrent to verify the length
		 */
		rc = rsstgetmetafileinfo(type, torlink, &props);
		if(rc == 0) {
			newtor->size = props->size;	
		} else {
			retval = -1;
		}
		rsstfreemetafileprops(props);
	} 

	return retval;
}

