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

#define _XOPEN_SOURCE /* glibc2 needs this */

#include <stdio.h>
#include <sqlite3.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <pcre.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "curlfile.h"
#include "regexp.h"
#include "torrentdb.h"
#include "logfile.h"
#include "config.h"
#include "torrentparse.h"
#include "database.h"

/*
 * Unknown type
 */
#define UNKNOWNTYPE "unknown"
#define OVECTSIZE   15

/*
 * Xpath queries.
 */
static char *toitems="/rss/channel/item";
static char *totitle="title";
static char *todescription="description";
static char *tolink="link";
static char *topubdate="pubDate";
static char *toenclosure="enclosure";

/*
 * Same as xmlXPathEvalExpression, but preserves current node.
 * This enables relative xpaths.
 */
static xmlXPathObjectPtr myXmlXPathEval( const xmlChar * str, xmlXPathContextPtr ctxt)
{
  xmlNode *temp;
  xmlXPathObjectPtr xpathObj;

  temp = ctxt->node;

  xpathObj = xmlXPathEvalExpression(str, ctxt); 

  ctxt->node = temp;

  return xpathObj;
}

/*
 * Execute the xpath query and return the pointer to the string in that node.
 * return 0 when okay, -1 when failed.
 */
static int getxpathstring(const xmlChar * str, xmlXPathContextPtr ctxt, unsigned char **string)
{
  xmlXPathObjectPtr xpathObj;
  xmlNode *node;

  *string = NULL;

  /*
   * Evaluate xpath
   */
  xpathObj = myXmlXPathEval(str, ctxt);

  /*
   * When no results are found fail.
   */
  if( xpathObj->nodesetval->nodeNr == 0) {
    xmlXPathFreeObject(xpathObj);
    return -1;
  }

  /*
   * get the pointer to the first node.
   */
  node = xpathObj->nodesetval->nodeTab[0];

  /*
   * Set pointer to contentstring in child
   */
	if(node->children == NULL) {
    xmlXPathFreeObject(xpathObj);
		return -1;
	}
  *string = BAD_CAST node->children->content;

  /*
   * Clean up 
   */
  xmlXPathFreeObject(xpathObj);

  /*
   * All gone well
   */
  return 0;
}

/*
 * Execute the xpath query and return the pointer to the string in that node.
 * return 0 when okay, -1 when failed.
 */
static xmlNode *getxpathnode(const xmlChar * str, xmlXPathContextPtr ctxt)
{
  xmlXPathObjectPtr xpathObj;
  xmlNode *node;

  /*
   * Evaluate xpath
   */
  xpathObj = myXmlXPathEval(str, ctxt);

  /*
   * When no results are found fail.
   */
  if( xpathObj->nodesetval->nodeNr == 0) {
    return NULL;
  }

  /*
   * get the pointer to the first node.
   */
  node = xpathObj->nodesetval->nodeTab[0];

  /*
   * Clean up 
   */
  xmlXPathFreeObject(xpathObj);

  /*
   * All gone well
   */
  return node;
}


/*
 * Gets the timestring.
 * Exports time_t value
 */
static int disectdate(char *date, time_t *pubdate)
{
  char      *rc;
  struct tm pubtm;
  time_t    now;

  /*
   * init struct.
   */
  memset(&pubtm, 0, sizeof(pubtm));
  mktime(&pubtm);

  // char *strptime(const char *buf, const char *format, struct tm *tm);
  // Fri, 02 Oct 2009 17:23:06 -0500
  // %a, %d %b %Y %H:%M:%S 
  // 2009-10-05
  // %Y-%m-%d
  // When all failes use time 'now'
  rc = strptime(date, "%a, %d %b %Y %H:%M:%S", &pubtm);
  if(rc == NULL) {
    writelog(LOG_DEBUG, "Converting date '%s'.", date);
    rc = strptime(date, "%Y-%m-%d", &pubtm);
  }
  if(rc == NULL) {
    /*
     * When all else fails it's a safe bet to set the date to 'now'
     */
    writelog(LOG_DEBUG, "Converting date '%s' failed, used 'now' as substitute.", date);
    now = time ( NULL );
    localtime_r ( &now, &pubtm );

  }

  pubtm.tm_isdst = -1;      /* Not set by strptime(); tells mktime()
                               to determine whether daylight saving time
                               is in effect */

  /*
   * struct tm to time_t
   */
  *pubdate = mktime(&pubtm);

  writelog(LOG_DEBUG, "Converted ctime: %s", ctime(pubdate));

  /*
   * success
   */
  return 0; 
}


/*
 * Get the properties of this node.
 * <enclosure url="http://torrent.zoink.it/Conan.O.Brien.2009.09.28.Drew.Barrymore.HDTV.XViD-YesTV.[eztv].torrent" length="367366144" type="application/x-bittorrent" />
 */
static int disectenclosure(xmlNode *encnode, size_t *torsize, unsigned char **type)
{
  struct _xmlAttr *prop;
  char   *name;
  char   *attr;

  /*
   * when no enclosure is found, set defaults
   */
  if(encnode == NULL) {
    /*
     * warn about no enclosure
     */
    writelog(LOG_DEBUG, "No enclosure found %s:%d", __FILE__, __LINE__);
    /*
     * Unknown size and type
     */
    *torsize=0;
    *type = calloc(strlen(UNKNOWNTYPE)+1, 1);
    strcpy((char*)*type, UNKNOWNTYPE);

    return 0;
  }

  /*
   * set pointer to properties
   */
  prop = encnode->properties;

  while(prop != NULL) {
    /*
     * get the name
     */
    name = (char*) prop->name;
    attr = (char*) prop->children->content;

    /*
     * Get the content
     */
    if(!strcmp(name, "url")) {
      /*
       * ignore for now, we have the link-node
       */
    }
    else if(!strcmp(name, "length")) {
      /*
       * transform to size_t
       */
      *torsize = atol(attr);
      writelog(LOG_DEBUG, "length : %s %ld", attr, *torsize);
    }
    else if(!strcmp(name, "type")) {
      /*
       * copy type to string
       */
      *type = calloc(strlen(attr)+1, 1);
      strcpy((char*)*type, attr);
      writelog(LOG_DEBUG, "Type : '%s'", *type);
    } 
    else {
      writelog(LOG_ERROR, "unknown enclosuretype: '%s' : ,%s'", name, attr);
    }

    prop = prop->next;
  }
  

  return 0;
}


/*
 * Disect the description field from the xml.
 * return the components.
 * Make sure to free the showname, episodetitle string after use.
 */
static int disectdescription(unsigned char *desc, unsigned char** showname, unsigned char** episodetitle, time_t *episodedate, int *season, int *episode) {
  char *token;
  char *value;
  char *name;
  char *cur;

  /*
   * When no description is found, do nothing, just return
   */
  if(desc == NULL) {
    return 0;
  }

  /*
   * Example description.
   * Show Name: Parks and Recreation; Episode Title: Beauty Pageant; Season: 2; Episode: 3
   * Show Name: Conan O Brien; Episode Title: Michael Moore; Episode Date: 2009-10-01
   * 
   */
  token = strtok ((char*)desc,";");
  while (token != NULL)
  {
    /*
     * Split the substring
     */
    cur=token;
    while(*cur != ':') {
      if(*cur == '\0'){
        break;
      }
      cur++;
    }
    *cur = '\0';
    name = token;
    value = cur+1;
    cleanupstring(name);
    cleanupstring(value);
    
    /*
     * Get the different settings and return set them
     */
    if(!strcmp("Show Name", name) || 
      !strcmp("Filename", name)) {
      *showname=calloc(strlen(value)+1, 1);
      strcpy((char*) *showname, value);
      writelog(LOG_DEBUG, "Showname: %s", *showname);
    }
    else if(!strcmp("Order", name)) {
      // Do nothing here no idea what order is supposed to do.
      //*order=calloc(strlen(value)+1, 1);
     // strcpy((char*) *order, value);
      //printf("Order: %s\n", *order);
    }
    else if(!strcmp("Episode Title", name) || 
            !strcmp("Show Title", name)) {
      *episodetitle=calloc(strlen(value)+1, 1);
      strcpy((char*) *episodetitle, value);
      writelog(LOG_DEBUG, "Episode title: %s", *episodetitle);
    }
    else if(!strcmp("Season", name)) {
      *season = atoi(value);
      writelog(LOG_DEBUG, "Season: %s %d", value, *season);
    }
    else if(!strcmp("Episode", name)) {
      *episode = atoi(value);
      writelog(LOG_DEBUG, "Episode: %s %d", value, *episode);
    }
    else if(!strcmp("Episode Date", name)) {
      disectdate(value, episodedate);
    } else {
      writelog(LOG_ERROR, "description attribute '%s' : '%s'='%s' not known.", token, name, value);
    }
    
    token = strtok (NULL, ";");
  }

  /*
   * Compile the regexp to split the description
   * Show Name: BBC Upgrade Me; Episode Title: N/A; Episode Date: 
   * Show Name: Supernatural; Episode Title: N/A; Season: 5; Episode: 4
   */

  return 0;
}

static int titleregexp(char *regexp, unsigned char *title, unsigned char** name, int *season, int *episode)
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
  char *seasonstr;
  char *episodestr;


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

  //PCRE_UNGREEDY


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
  *name = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf((char*)*name, "%.*s", ovector[2*i+1] - ovector[2*i], title + ovector[2*i]);
  i = 2;
  seasonstr = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(seasonstr, "%.*s", ovector[2*i+1] - ovector[2*i], title + ovector[2*i]);
  i = 3;
  episodestr = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(episodestr, "%.*s", ovector[2*i+1] - ovector[2*i], title + ovector[2*i]);


  /*
   * postprocess information.
   */
  *season = atoi(seasonstr);
  *episode = atoi(episodestr);
  cleanupstring((char*) *name);

  writelog(LOG_DEBUG, "number of strings found '%d', title '%s' season '%d' episode '%d'", rc, *name, *season, *episode);
  //printf("Title '%s'\nRegexp '%s'\nNumber of strings found '%d', title '%s' season '%s' episode '%s'\n", title, regexp, rc, *name, seasonstr, episodestr);

  /*
   * convert seeds and peers
   */

  /*
   * Get the 3 strings and put them in the output strings.
   * Both need to be freed afterwards
   */
  free(seasonstr);
  free(episodestr);
  pcre_free(p);

  return 0;
}

/*
 * Disect the name field from the xml.
 * return the components.
 * Make sure to free the name, season string after use.
 */
static int disecttitle(unsigned char *title, unsigned char** name, int *season, int *episode) 
{
  /*
   * Example description.
   * Category: Anime, Seeds: 20, Peers: 103, Size: 353.3 MBs
   * extract: Category, seeds, peers, size
   * execute regexp /Category: ([^,]*), Seeds: ([^,]*), Peers: ([^,]*), Size: (.*)$/
   */
  int   rc;

  if(title == NULL) {
    writelog(LOG_ERROR, "NULL pointer passed for title %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Try the S01E23 notation, this is common.
   */
  rc = titleregexp("^(.*)[sS]([0-9][0-9]?) ?[eE] ?([0-9][0-9]?)", title, name, season, episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at specific regexp %s:%d", __FILE__, __LINE__);
    return 0;
  }

  /*
   * Try the 01x23 notation, this is common.
   */
  rc = titleregexp("^(.*[^0-9])([0-9][0-9]?) ?[xX] ?([0-9][0-9]?)", title, name, season, episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at specific regexp %s:%d", __FILE__, __LINE__);
    return 0;
  }

  /*
   * Try a more generic match on anything that has an S followed by 2 seperate numbers
   */
  rc = titleregexp("^([^0-9]+)[Ss][[:space:]]*([0-9][0-9]?)[^0-9]+([0-9][0-9]?)[^0-9]*$", title, name, season, episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at generic regexp %s:%d", __FILE__, __LINE__);
    return 0;
  }

  /*
   * Try a more generic match on anything that has 2 seperate numbers
   */
  rc = titleregexp("^([^0-9]+)([0-9][0-9]?)[^0-9]+([0-9][0-9]?)[^0-9]*$", title, name, season, episode);
  if(rc == 0) {
    writelog(LOG_DEBUG, "Found title match at generic regexp %s:%d", __FILE__, __LINE__);
    return 0;
  }

  /*
   * when none match we use the whole string as title
   */
  writelog(LOG_DEBUG, "Using whole string as title '%s'", title);
  *name = calloc(1, strlen((char*)title)+1);
  strcpy((char*) *name, (char*) title);

  return 0;
}

/*
 * Checks if a link was present in the database
 * returns 1 on presense, arg size holds size
 * returns 0 on not present.
 */
static int linkpresent(sqlite3 *db, char *url, long *size)
{
  int    retval=0;
  char   *sizestring;
  char   *query;
  int    rc;
  
  /*
   * Try to fetch size associated with link.
   */
  query="select size from newtorrents where link=?1";

  /*
   * Check if the torrent has been downloaded before
   */
  rc = dosingletextquery(db, (unsigned char const**) &sizestring, query, "s", url); 
  if(rc == -1) {
    /*
     * When this goes wrong we are fubar
     */
    writelog(LOG_ERROR, "Query on newtorrenttable failed! %s:%d", __FILE__, __LINE__);
    exit(1);
  }
  /*
   * Did we find something ?
   */
  if(sizestring == NULL) {
    /*
     * Nothing
     */
    *size=0;
    retval=-1;
  } else {
    /*
     * Found a size
     */
    *size=atol(sizestring);
  }
  free(sizestring);

  return retval;
}

/*
 * Check if torrent size is correct
 * When the torrent is below the threshold set in config,
 * the torrent is downloaded and the size is used from there.
 * This is added because the piratebaf includes the size of the
 * torrents not the content in there rss.
 */
static int checksize(sqlite3 *db, size_t *size, char *url)
{
  int retval=0;
  int rc=0;
  long threshold=0;
  long foundsize=0;
  torprops *props;

  /*
   * check against threshold
   */
  configgetlong(db, "min_size", &threshold);
  if(*size > (size_t)threshold){
    return 0;
  }

  /*
   * Check if link is present in database, when it is use that size.
   */
  rc = linkpresent(db, url, &foundsize);
  if(rc == 0) {
    *size = foundsize; 
    return 0;
  }

  writelog(LOG_DEBUG, "'%s' smaller then min_size, downloading to check.", url);

  /*
   * Get torrent info
   */
  rc = gettorrentinfo(url, &props);
  if(rc != 0){
    writelog(LOG_ERROR, "Getting torrent info failed for '%s'.", url);
    retval=-1;
  } else {
    *size = props->size;
  }

  /*
   * Free struct
   */
  freetorprops(props);

  return retval;
}


/*
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int defaultrss(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
  xmlDocPtr           doc = NULL; /* the resulting document tree */
  xmlNode             *root_element = NULL;
  xmlNode             *encnode;
  //xmlNode *cur_node = NULL;
  xmlXPathContextPtr  xpathCtx; 
  xmlXPathObjectPtr   xpathObj; 
  xmlNodeSetPtr       nodeset;
  int                 count = 0;
  int                 rc = 0;
  unsigned char       *title = NULL;
  unsigned char       *description = NULL;
  unsigned char       *link = NULL;
  unsigned char       *pubdatestr = NULL;
  unsigned char       *showname = NULL;
  unsigned char       *episodetitle = NULL;
  unsigned char       *cuttitle = NULL;
  unsigned char       *type = NULL;
  int                 seeds = 0;
  int                 peers = 0;
  size_t              size = 0;
  int                 season = 0;
  int                 episode = 0;
  time_t              pubdate = 0;
  time_t              episodedate = 0;

  /*
   * Use the imput to feed libxml2
   */
  doc = xmlReadMemory(rssfile->memory, rssfile->size, url, NULL, 0);
  if (doc == NULL) {
    writelog(LOG_ERROR, "Failed to parse document name '%s' filter %s %s:%d", name, filter, __FILE__, __LINE__);
    return -1;
  }
  root_element = xmlDocGetRootElement(doc);

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    writelog(LOG_ERROR, "Error: unable to create new XPath context %s:%d", __FILE__, __LINE__);
    xmlFreeDoc(doc); 
    return(-1);
  }

  /*
   * Get the item nodes in channel.
   */
  xpathObj = myXmlXPathEval(BAD_CAST toitems, xpathCtx);
  if(xpathObj == NULL) {
    writelog(LOG_ERROR, "Error: unable to evaluate xpath expression \"%s\" %s:%d", toitems, __FILE__, __LINE__);
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc); 
    return(-1);
  }

  /*
   * Get the nodeset returned from the query
   */
  nodeset = xpathObj->nodesetval;

  /*
   * Loop through the nodes we need.
   */
  for(count = 0; count < nodeset->nodeNr; count++) {
    /*
     * Clear reused ints.
     */
    seeds=0;
    peers=0;
    size=0;
    season=0;
    episode=0;
    pubdate=0;
    type=NULL;
    title=NULL;
    description=NULL;
    link=NULL;
    pubdatestr=NULL;
    showname=NULL;
    episodetitle=NULL;
    cuttitle=NULL;
    type=NULL;


    /*
     * Set current node for XPath queries.
     */
    xpathCtx->node = nodeset->nodeTab[count];

    writelog(LOG_DEBUG, "nodename[%d]: %s", count, nodeset->nodeTab[count]->name);
    /*
     * Get the strings we need
     */
    rc = getxpathstring(BAD_CAST totitle, xpathCtx, &title);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No title found");
    }
    writelog(LOG_DEBUG, "title: %s", title);

    rc = getxpathstring(BAD_CAST todescription, xpathCtx, &description);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No description found");
    }
    writelog(LOG_DEBUG, "description: %s", description);

    rc = getxpathstring(BAD_CAST tolink, xpathCtx, &link);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No link found");
    }
    writelog(LOG_DEBUG, "link: %s", link);

    rc = getxpathstring(BAD_CAST topubdate, xpathCtx, &pubdatestr);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No pubdatestr found");
    }
    writelog(LOG_DEBUG, "pubdatestr: %s", pubdatestr);

    /*
     * Get node op enclosure
     * toenclosure
     */
    encnode = getxpathnode(BAD_CAST toenclosure, xpathCtx );

    rc = disecttitle(title, &cuttitle, &season, &episode);
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "name failed: %s", title);
      continue;
    }

    /*
     * split the strings
     */
    rc = disectdescription(description, &showname, &episodetitle, &episodedate, &season, &episode);
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "description failed: %s", description);
      continue;
    }

    rc = disectdate((char*)pubdatestr, &pubdate); 
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "date failed: %s", description);
      continue;
    }
    rc = disectenclosure(encnode, &size, &type);
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "enclosure failed: %s", description);
      continue;
    }

    /*
     * Check if torrent size is correct
     * When the torrent is below the threshold set in config,
     * the torrent is downloaded and the size is used from there.
     * This is added because the piratebaf includes the size of the 
     * torrents not the content in there rss.
     */
    rc = checksize(db, &size, (char*) link);

    /*
     * Add gatered data to the database avoiding duplicates.
     */
    
    rc = addnewtorrent(db,
               (char*) cuttitle,
               (char*) link,
               pubdate,
               (char*) "none",
               season,
               episode,
               0,
               0,
               size);
    


    /*
     * Clean up mess
     */
    free(showname);
    free(episodetitle);
    free(cuttitle);
    free(type);
  }

  /*
   * When done cleanup the our mess.
   */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  return 0;
}
