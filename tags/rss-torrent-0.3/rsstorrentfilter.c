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

#include "curlfile.h"
#include "regexp.h"
#include "torrentdb.h"
#include "logfile.h"

/*
 * Xpath queries.
 */
static char *toitems="/rss/channel/item";
static char *totitle="title";
static char *todescription="description";
static char *tolink="link";
static char *topubdate="pubDate";

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
    return -1;
  }

  /*
   * get the pointer to the first node.
   */
  node = xpathObj->nodesetval->nodeTab[0];

  /*
   * Set pointer to contentstring in child
   */
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
 * Calculates size from string to size_t in bytes
 * format <float> <mb/gb/kb>
 * Be warned float is used, zo roundingerrors occure
 */
static int calcsize(char *size, size_t *sizeout)
{
  float  sizefloat;
  char   sizemod[5];
  int    rc;

  /*
   * Get size and modifier.
   */
  rc = sscanf(size, "%f %2s", &sizefloat, sizemod);
  if(rc != 2) {
    *sizeout = 0;
    return -1;
  }

  /*
   * calculate size
   */
  if(!strcmp(sizemod, "kb") || !strcmp(sizemod, "KB")) {
    *sizeout = sizefloat * 1e3;
  } 
  if(!strcmp(sizemod, "mb") || !strcmp(sizemod, "MB")) {
    *sizeout = sizefloat * 1e6;
  } 
  if(!strcmp(sizemod, "gb") || !strcmp(sizemod, "GB")) {
    *sizeout = sizefloat * 1e9;
  } 
  //printf("Size: %.2f, in: %s, became: %ld\n", sizefloat, sizemod, *sizeout); 

  /*
   * All gone well.
   */
  return 0;
}


/*
 * Disect the description field from the xml.
 * return the components.
 * Make sure to free the category string after use.
 */
static int disectdescription(unsigned char *desc, unsigned char** category, int *seeds, int *peers, size_t *size) {
  /*
   * Example description.
   * Category: Anime, Seeds: 20, Peers: 103, Size: 353.3 MBs
   * extract: Category, seeds, peers, size
   * execute regexp /Category: ([^,]*), Seeds: ([^,]*), Peers: ([^,]*), Size: (.*)$/
   */
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[10];
  int   rc;
  int   i;
  char  *sizechar;
  char  *peerschar;
  char  *seedschar;


  /*
   * Compile the regexp to split the description
   */
  p = pcre_compile("^Category: ([^,]*), Seeds: ([^,]*), Peers: ([^,]*), Size: (.*)$", 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d", errmsg, errpos, __FILE__, __LINE__);
  }

  /*
   * execute regexp
   */
  rc = pcre_exec (
      p,                   /* the compiled pattern */
      0,                    /* no extra data - pattern was not studied */
      (char*)desc,                  /* the string to match */
      strlen((char*)desc),          /* the length of the string */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      ovector,              /* output vector for substring information */
      sizeof(ovector));           /* number of elements in the output vector */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
        printf("decription of unknown format.\n");
        printf("%s\n", desc);
        break;

      default:
        printf("Error while matching: %d\n", rc);
        break;
    }
    free(p);
    return -1;
  }

  /*
   * extract both strings.
   */
  i = 1;
  *category = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf((char*)*category, "%.*s", ovector[2*i+1] - ovector[2*i], desc + ovector[2*i]);
  i = 2;
  seedschar = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(seedschar, "%.*s", ovector[2*i+1] - ovector[2*i], desc + ovector[2*i]);
  i = 3;
  peerschar = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(peerschar, "%.*s", ovector[2*i+1] - ovector[2*i], desc + ovector[2*i]);
  i = 4;
  sizechar = calloc(1, ovector[2*i+1]-ovector[2*i]+1);
  sprintf(sizechar, "%.*s", ovector[2*i+1] - ovector[2*i], desc + ovector[2*i]);


  /*
   * convert seeds and peers
   */
  *seeds = atoi(seedschar);
  *peers = atoi(peerschar);
  rc = calcsize(sizechar, size);

 //printf("category: %s\nseeds: %d\npeers: %d\nsizechar: %ld\n",(char*) *category, *seeds, *peers, *size);

  /*
   * Get the 3 strings and put them in the output strings.
   * Both need to be freed afterwards
   */
  free(seedschar);
  free(peerschar);
  free(sizechar);
  pcre_free(p);

  return 0;
}

/*
 * Gets the timestring.
 * Exports time_t value
 */
int disectdate(char *date, time_t *pubdate)
{
  char *rc;
  struct tm pubtm;

  /*
   * init struct.
   */
  memset(&pubtm, 0, sizeof(pubtm));
  mktime(&pubtm);

  // char *strptime(const char *buf, const char *format, struct tm *tm);
  // 09/29/2009 02:00 AM
  // %d/%m/%Y %I:%M %p
  rc = strptime(date, "%Y-%m-%d", &pubtm);
  if(rc == NULL) {
    writelog(LOG_ERROR, "Failed to convert time '%s' %s:%d", date, __FILE__, __LINE__);
    return -1;
  }

  /*
   * struct tm to time_t
   */
  *pubdate = mktime(&pubtm);

 //printf("ctime: %s\n", ctime(pubdate));

  /*
   * success
   */
  return 0; 
}

/*
 * Disect the name field from the xml.
 * return the components.
 * Make sure to free the category string after use.
 */
static int disecttitle(unsigned char *title, unsigned char** name, int *season, int *episode) {
  /*
   * Example description.
   * Category: Anime, Seeds: 20, Peers: 103, Size: 353.3 MBs
   * extract: Category, seeds, peers, size
   * execute regexp /Category: ([^,]*), Seeds: ([^,]*), Peers: ([^,]*), Size: (.*)$/
   */
  pcre *p;
  const char *errmsg;
  int   errpos;
  int   ovector[10];
  int   rc;
  int   i;
  char *seasonstr;
  char *episodestr;


  /*
   * Compile the regexp to retrieve name, season and episode 
   * try :
   * ^(.*)(([sS]([0-9][0-9]?)))?[eE]([0-9][0-9]?))?   name, season, episode
   */
  p = pcre_compile("^(.*)[sS]([0-9][0-9]?)[eE]([0-9][0-9]?)", 0, &errmsg, &errpos, 0);
  if (p == NULL) {
    /* should not happen, because init1 has already tested and set to NULL on error */
    writelog(LOG_ERROR, "Ouch! Can't compile regular expression: %s (char %i) %s:%d", errmsg, errpos, __FILE__, __LINE__);
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
      sizeof(ovector));           /* number of elements in the output vector */
  if (rc < 0) {
    switch (rc) {
      case PCRE_ERROR_NOMATCH:
        //printf("Using whole string\n");
        *name = calloc(1, strlen((char*)title)+1);
        strcpy((char*) *name, (char*) title);
        *season = 0;
        episode = 0;
        break;

      default:
        writelog(LOG_ERROR, "Error while matching: %d %s:%d", rc, __FILE__, __LINE__);
        exit(1);
    }
    free(p);
    return 0;
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

 //printf("number of strings found %d, title %s season %d episode %d\n", rc, *name, *season, *episode);

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
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int rsstorrentfilter(sqlite3 *db, char *name, char *url, char *filter, MemoryStruct *rssfile)
{
  xmlDocPtr doc = NULL; /* the resulting document tree */
  xmlNode *root_element = NULL;
  //xmlNode *cur_node = NULL;
  xmlXPathContextPtr  xpathCtx; 
  xmlXPathObjectPtr   xpathObj; 
  xmlNodeSetPtr       nodeset;
  int                 count;
  int                 rc;
  unsigned char       *title;
  unsigned char       *description;
  unsigned char       *link;
  unsigned char       *pubdatestr;
  unsigned char       *category;
  int                 seeds;
  int                 peers;
  size_t              size;
  int                 season;
  int                 episode;
  unsigned char       *cuttitle;
  time_t              pubdate;

  /*
   * Use the imput to feed libxml2
   */
  doc = xmlReadMemory(rssfile->memory, rssfile->size, url, NULL, 0);
  if (doc == NULL) {
    writelog(LOG_ERROR, "Failed to parse document '%s' %s:%d", url, __FILE__, __LINE__);
    return -1;
  }
  root_element = xmlDocGetRootElement(doc);

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    writelog(LOG_ERROR,"Error: unable to create new XPath context %s:%d", __FILE__, __LINE__);
    xmlFreeDoc(doc); 
    return(-1);
  }

  /*
   * Get the item nodes in channel.
   */
  //xpathObj = xmlXPathEvalExpression(BAD_CAST toitems, xpathCtx);
  xpathObj = myXmlXPathEval(BAD_CAST toitems, xpathCtx);
  if(xpathObj == NULL) {
    writelog(LOG_ERROR, "Error: unable to evaluate xpath expression \"%s\" %s:%d", toitemsi, __FILE__, __LINE__);
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc); 
    return(-1);
  }

  /*
   * Get the nodeset returned from the query
   */
  nodeset = xpathObj->nodesetval;


  /*
   * print elements
   */
  //print_element_names(root_element);

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


    /*
     * Set current node for XPath queries.
     */
    xpathCtx->node = nodeset->nodeTab[count];

    //printf("nodename[%d]: %s\n", count, nodeset->nodeTab[count]->name);
    /*
     * Get the strings we need
     */
    rc = getxpathstring(BAD_CAST totitle, xpathCtx, &title);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No title found in '%s' %s:%d", url, __FILE__, __LINE__);
    }
    //printf("title: %s\n", title);

    rc = getxpathstring(BAD_CAST todescription, xpathCtx, &description);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No description found '%s' %s:%d", url, __FILE__, __LINE__);
    }
    //printf("description: %s\n", description);

    rc = getxpathstring(BAD_CAST tolink, xpathCtx, &link);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No link found '%s' %s:%d", url, __FILE__, __LINE__);
    }
    //printf("link: %s\n", link);

    rc = getxpathstring(BAD_CAST topubdate, xpathCtx, &pubdatestr);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No pubdatestr found '%s' %s:%d", url, __FILE__, __LINE__);
    }
    //printf("pubdatestr: %s\n", pubdatestr);

    /*
     * split the strings
     */
    rc = disectdescription(description, &category, &seeds, &peers, &size);
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "description failed: %s %s:%d", description, __FILE__, __LINE__);
      continue;
    }
    rc = disecttitle(title, &cuttitle, &season, &episode);
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "name failed: %s %s:%d", title, __FILE__, __LINE__);
      continue;
    }
    rc = disectdate((char*)pubdatestr, &pubdate); 
    if(rc != 0) {
      /*
       * When an error occures ignore record
       */
      writelog(LOG_ERROR, "date failed: %s %s:%d", description, __FILE__, __LINE__);
      continue;
    }

    /*
     * Add gatered data to the database avoiding duplicates.
     */
    
    rc = addnewtorrent(db,
               (char*) cuttitle,
               (char*) link,
               pubdate,
               (char*) category,
               season,
               episode,
               seeds,
               peers,
               size);

    /*
     * Clean up mess
     */
    free(category);
    free(cuttitle);
  }

  /*
   * When done cleanup the our mess.
   */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  return 0;
}
