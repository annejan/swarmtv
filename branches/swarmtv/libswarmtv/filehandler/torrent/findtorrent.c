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
#include <time.h>
#include <sqlite3.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "types.h"
#include "regexp.h"
#include "curlfile.h"
#include "logfile.h"
#include "findtorrent.h"

/*
 * MIME strings 
 */
#define TORRENT_MIME  "application/x-bittorrent"
#define SECONDARY_MIME "application/octet-stream"
#define HTML_MIME     "text/html"
#define HTTPPATTERN   "http://"
#define HTTPSPATTERN   "https://"

#define XPATH_TO_A    "//a[@href]" 

#define NODOWN_EXT  "\\.(gif|png|jpg|js|rss|css|xml|jsf|exe)$"
#define NODOWN_TEXT  "(/faq/|/forum/|/login/|/showlist/|calendar|php[^?]|news)"


/*
 * This function returns 1 when the file matches one of the uninteresting formatting.
 * Else 0 is returned.
 */
static int uninteresting(char *url)
{
  int rc=0;
  
  /*
   * Do the matching.
   */
  rc = rsstcomregexp(NODOWN_EXT, url);
  rc |= rsstcomregexp(NODOWN_TEXT, url);

  /*
   * return result.
   */
  return rc;
}


/*
 * This is an HTML document parser, that prints all <a href=".."/> values
 */
static xmlXPathObjectPtr htmlextacthrefs(xmlDocPtr doc, const xmlChar* xpathExpr)
{
  xmlXPathContextPtr xpathCtx; 
  xmlXPathObjectPtr xpathObj;

  /* Create Xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    fprintf(stderr,"Error: unable to create new XPath context\n");
    return(NULL);
  }

  /* Evaluate Xpath expression */
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(xpathObj == NULL) {
    fprintf(stderr,"Error: unable to evaluate Xpath expression \"%s\"\n", XPATH_TO_A);
    xmlXPathFreeContext(xpathCtx); 
    return(NULL);
  }

  /* dump the resulting document */
  //xmlDocDump(stdout, doc);

  /*
   * cleanup
   */
  xmlXPathFreeContext(xpathCtx); 

  /*
   * gone well
   */
  return(xpathObj);
}


/*
 * Extract base name from URL
 * basename is the output string for the base name
 * Don't forget to free after use.
 */
static void getbasename(char *url, char **basename) 
{
  char    *cur=NULL;
  size_t  basesize=0;
  int     found=0;

  *basename = NULL;

  /*
   * find 3th '/'
   */
  cur = strrchr(url, '/');

  /*
   * When not found check if the string starts with 'http://'
   * then return whole string
   */
  if(found==1 && 
     ( strncmp(HTTPPATTERN, url, strlen(HTTPPATTERN))==0 ||
       strncmp(HTTPSPATTERN, url, strlen(HTTPSPATTERN))==0  ) ){
    cur=url+strlen(url);  
  }

  /*
   * Copy the base name to the output buffer
   */
  basesize=cur-url;    
	rsstalloccopy(basename, url, basesize);
}


/*
 * Resolve relative path
 * when link is a relative link link is completed.
 * (and reallocated)
 */
static void resolvrel(char *fullurl, char *link, char **fulllink)
{
  char *basename  = NULL;
  //char *fulllink  = NULL;
  size_t fullsize = 0;

  /*
   * Test if link starts with 'http://'
   * If it does link is full link
   */
  if(strncmp( HTTPPATTERN, link, strlen(HTTPPATTERN)) == 0 ||
    strncmp( HTTPSPATTERN, link, strlen(HTTPSPATTERN))==0){
    fullsize = strlen(link);
    *fulllink = calloc(1, fullsize+2);
    snprintf(*fulllink, fullsize+1, "%s/", link);
  }
  else {
    /*
     * When a relative link is found, make sure 
     * Get base path
     */
    getbasename(fullurl, &basename);

    /*
     * append base path with link 
     * realloc into new space
     */
    fullsize = strlen(basename) + strlen(link);
    *fulllink = calloc(1, fullsize+3);
    snprintf(*fulllink, fullsize+2, "%s/%s/", basename, link);
  }

  //printf("Full link : '%s' '%s' '%s' '%s'\n", fullurl, basename, link, *fulllink);

  /*
   * clean up
   */
  free(basename);
}


/*
 * Get value of href attribute from node
 */
static char *gethrefvalue(xmlNode *node)
{
  struct _xmlAttr *prop;
  
  prop = node->properties;

  /*
   * find href or NULL
   */
  while(prop != NULL){
    if(strcmp("href", (char*) prop->name) == 0){
      return (char*) prop->children->content;
    } 

    /*
     * to next node
     */
    prop=prop->next;
  } 

  /*
   * Not found
   */
  return NULL;
}


/*
 * Decide if a downloaded file is a torrent
 * @Arguments
 * url URL of the downloaded file
 * mime the mime type of the file
 * @returns
 * 0 on no torrent
 * 1 on torrent found
 */
static int testiftorrent(char *url, char *mimetype)
{
	/*
	 * found torrent mime
	 */
	if(strncmp(mimetype, TORRENT_MIME, strlen(TORRENT_MIME)) == 0) {
		return 1;
	}

	/*
	 * Is a torrent but with generic mime type.
	 */
	if(strncmp(mimetype, SECONDARY_MIME, strlen(SECONDARY_MIME)) == 0 &&
			strstr(url, ".torrent") != NULL) {
		return 1;
	}

	/*
	 * not a torrent
	 */
	return 0;
}


/*
 * This is a recursive function.
 * It scans for torrent files, and will return the first one it encounters.
 * Note this might not be the correct torrent at all :)
 * A counter prevents the recursing from getting out of hand.
 * The torrent will be contained in torbuffer.
 * The URL the torrent was found in torrenturl.
 * Don't forget to free buffer and torrenturl !
 */
int rsstfindtorrent(char *url, char **torrenturl, MemoryStruct **torbuffer, int recurse) 
{
  int               rc = 0;
  MemoryStruct      buffer;
  char              *filetype=NULL;
  char              *linkaddr=NULL; 
  char              *fulllink=NULL;
  xmlXPathObjectPtr allas=NULL;
  xmlDocPtr         html=NULL; 
  xmlNodeSetPtr     nodeset=NULL;
  int               count=0;

  /*
   * When uninteresting stuff is found, ignore
   */
  if( uninteresting(url) != 0) {
    return 0;
  }

  /*
   * Initialize stuff
   */
  memset(&buffer, 0, sizeof(MemoryStruct));
  rsstwritelog(LOG_DEBUG, "Scan URL: '%s'", url);

  /*
   * Make sure no garbage is in here
   */
  *torrenturl=NULL;
  *torbuffer=NULL;

  /*
   * Download content
   */
  rc = rsstdownloadtobuffer( url, &buffer);
  if(rc != 0) {
    //rsstwritelog(LOG_NORMAL,  "%s: Failed to download.", url);
	  rsstfreedownload(&buffer);
    return 0;
  }

  /*
   * try to get the Content type from the header
   */
  rc = rsstgetheadersvalue(HTTP_CONTENTTYPE, &filetype, &buffer);
  if(rc == -1) {
    /*
     * No header found ignore this link.
     */
		rsstfreedownload(&buffer);
    free(filetype);
		return 0;
  }

  /*
   * When we find a torrent, return 1 and set buffer + url
   */
	rc = testiftorrent(url, filetype);
  if(rc == 1) {
    /*
     * Copy and allocate the url + buffer
     * Both should be freed after use !
     */
		rsstalloccopy(torrenturl, url, strlen(url));
    *torbuffer = calloc(1, sizeof(MemoryStruct)+1);
    memcpy(*torbuffer, &buffer, sizeof(MemoryStruct));

    /*
     * NO other way, clean up here
     */
    free(filetype);
    return 1;
  }

  /*
   * When recursion = 0 
   * We are not interested in other content then torrents, so returning 0
   */
  if(recurse <= 0) {
		rsstfreedownload(&buffer); 
    free(filetype);
    return 0;
  }

  /*
   * When html, call this method again on every html and torrent link in the document.
   * ignore : .gif, .jpg, .jpeg, .png
   */
  if(strncmp(filetype, HTML_MIME, strlen(HTML_MIME)) == 0){
    /*
     * Parse document through html parser
     */
    html =  htmlReadDoc((unsigned char*)(buffer.memory), "", NULL, 
        HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR | HTML_PARSE_NOBLANKS | HTML_PARSE_RECOVER);
    if(html == NULL) {
      rsstwritelog(LOG_NORMAL, "%s: failed to create reader, although MIME suggested html\n", url);      
      rsstfreedownload(&buffer);
			free(filetype);
      return 0;
    }

    /*
     * Find all <a href=.. > nodes
     */
    allas = htmlextacthrefs(html,(unsigned const char*) XPATH_TO_A);
    nodeset = allas->nodesetval;

    /*
     * Go through nodes and see which ones are interesting
     */
    while(count < nodeset->nodeNr) {
      /*
       * Get href value that seem relevant
       */
      linkaddr = gethrefvalue(nodeset->nodeTab[count]);

      /*
       * pass them through the recursion one by one
       * If one returns 1, break the loop and return that pointer to the output buffer.
       */
      resolvrel(url, linkaddr, &fulllink);

      if(strlen(fulllink) == 0) {
        rsstwritelog(LOG_DEBUG, "url: '%s' linkaddr: '%s' fulllink: '%s'", url, linkaddr, fulllink);
      }

      switch(*fulllink){
        case'?':
        case'#':
          xmlFreeDoc(html);
          free(fulllink);
          xmlXPathFreeObject(allas);
          free(filetype);
          rsstfreedownload(&buffer);
          return 0;
      }

      /*
       * recurse
       * When one of the children returns 1, return 1 to.
       */
      rc = rsstfindtorrent(fulllink, torrenturl, torbuffer, recurse-1); 
      if(rc == 1) {
        /*
         * We found the torrent !
         * Clean up and return.
         */
        xmlFreeDoc(html);
        free(fulllink);
        xmlXPathFreeObject(allas);
        free(filetype);
        return 1;
      }

      /*
       * Clean up in loop, and move on
       */
      free(fulllink);
      count++;
    }
  }

  rsstwritelog(LOG_NORMAL, "Nothing found at '%s'", url);

  /*
   * Clean up
   */
  xmlXPathFreeObject(allas);
  xmlFreeDoc(html); 
  free(filetype);
  rsstfreedownload(&buffer);

  return 0;
}


/*
 * Finds and writes torrent to file
 */
int rsstfindtorrentwrite(char *url, char *name)
{
  int             rc = 0;
  int             rv = 0;
  char            *torurl = NULL;
  MemoryStruct    *buffer = NULL;

  rsstwritelog(LOG_DEBUG, "Writing torrent '%s' to file '%s'", url, name);

  /*
   * Get the buffer and URL to the torrent in there
   */
  rc = rsstfindtorrent(url, &torurl, &buffer, RECURSE);
  if(rc != 1) {
    rsstwritelog(LOG_NORMAL, "Torrent not found in %s", url);
		rsstfreedownload(buffer);
    rv=-1;
  }

  /*
   * Print to result
   */
  //int rsstwritelog(int level, char *str,...);
  if(rv == 0 && strcmp(url, torurl) != 0) {
    rsstwritelog(LOG_NORMAL, "Original URL : %s", url);
    rsstwritelog(LOG_NORMAL, "Torrent URL   : %s", torurl);
  }

  /*
   * Save file to name
   */
  if(rv == 0) {
    rc = rsstwritebuffer(name, buffer);
    if(rc != 0){
      rsstwritelog(LOG_ERROR, "Writing torrent '%s' failed.", name);
      rv = -1;
    }
  }

  /*
   * free the result
   */
  free(torurl);
  rsstfreedownload(buffer);
  free(buffer);

  return rv;
}

