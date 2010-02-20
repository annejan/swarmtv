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
#include "logfile.h"

#include "rssparse.h"

/*
 * Xpath queries.
 */
static char *toitems				= "/rss/channel/item";
static char *totitle				= "title";
static char *tolink					= "link";
static char *totorrentlink	= "torrentLink";
static char *tocategory			= "category";
static char *topubdate			= "pubDate";
static char *todescription	= "description";
static char *toenclosure		= "enclosure";
static char *tocomments			= "comments";
static char *topeers				= "peers";
static char *toseeds				= "seeds";
static char *tosize					= "size";
static char *toguid					= "guid";


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
 * Get the properties of this node.
 * <enclosure url="http://torrent.zoink.it/Conan.O.Brien.2009.09.28.Drew.Barrymore.HDTV.XViD-YesTV.[eztv].torrent" length="367366144" type="application/x-bittorrent" />
 */
static int disectenclosure(xmlNode *encnode, rssparse_callback *call)
{
  struct 	_xmlAttr *prop=NULL;
  char   	*name=NULL;
  char   	*attr=NULL;
	size_t 	torsize=0;

  /*
   * when no enclosure is not found return
   */
  if(encnode == NULL) {
    /*
     * warn about no enclosure
     */
    writelog(LOG_DEBUG, "No enclosure found %s:%d", __FILE__, __LINE__);
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
			if(call->enclosureurl != NULL){
				call->enclosureurl(call->data, attr);
			}
    }
    else if(!strcmp(name, "length")) {
      /*
       * transform to size_t
       */
      torsize = atol(attr);
      writelog(LOG_DEBUG, "length : %s %ld", attr, torsize);
			if(call->enclosurelenght != NULL){
				call->enclosurelenght(call->data, torsize);
			}
    }
    else if(!strcmp(name, "type")) {
      /*
       * copy type to string
       */
      writelog(LOG_DEBUG, "Type : '%s'", attr);
			if(call->enclosuretype != NULL){
				call->enclosuretype(call->data, attr);
			}
    } 
    else {
      writelog(LOG_ERROR, "unknown enclosuretype: '%s' : ,%s'", name, attr);
    }

    prop = prop->next;
  }

  return 0;
}


/*
 * Routine to call parser
 * Arguments 
 * call		Pointer to a struct holding the callback routines, all unused should be NULL
 * url		the URL of the RSS feed.
 * buffer	The buffer holding the RSS content.
 */
int rssparse(rssparse_callback *call, char *url, MemoryStruct *buffer)
{
  xmlDocPtr           doc=NULL; /* the resulting document tree */
  xmlNode             *root_element=NULL;
  xmlNode             *encnode;
  xmlXPathContextPtr  xpathCtx=NULL; 
  xmlXPathObjectPtr   xpathObj=NULL; 
  xmlNodeSetPtr       nodeset=NULL;
	unsigned char			 *value=NULL;
	long								filesize=0;
	int									integer=0;
	int									count=0;
	int 								rc=0;
	
  /*
   * Use the imput to feed libxml2
   */
  doc = xmlReadMemory(buffer->memory, buffer->size, url, NULL, 0);
  if (doc == NULL) {
    writelog(LOG_ERROR, "Failed to parse RSS document: '%s' %s:%d", url, __FILE__, __LINE__);
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
     * Set current node for XPath queries.
     */
    xpathCtx->node = nodeset->nodeTab[count];

		/*
		 * new record
		 */
		if(call->start != NULL) {
			call->start(call->data);
		}

		/*
		 * Get node op enclosure
		 * toenclosure
		 */
		encnode = getxpathnode(BAD_CAST toenclosure, xpathCtx);
		rc = disectenclosure(encnode, call);
		if(rc != 0) {
			/*
			 * When an error occures ignore record
			 */
			writelog(LOG_ERROR, "enclosure failed not found");
			continue;
		}


		/*
		 * Look for title
		 */
		rc = getxpathstring(BAD_CAST totitle, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No title found");
    } else {
			if(call->title != NULL){
				call->title(call->data, (char*) value);
			}
		}
		value=NULL; 

		/*
		 * Look for link
		 */
    rc = getxpathstring(BAD_CAST tolink, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_ERROR, "No link found");
    } else {
			if(call->link != NULL){
				call->link(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for torrentlink
		 */
    rc = getxpathstring(BAD_CAST totorrentlink, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No torrentlink found");
    } else {
			if(call->torrentlink != NULL){
				call->torrentlink(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for category
		 */
    rc = getxpathstring(BAD_CAST tocategory, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No categorystr found");
    } else {
			if(call->category != NULL){
				call->category(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for pubdate
		 */
    rc = getxpathstring(BAD_CAST topubdate, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No pubdatestr found");
    } else {
			if(call->pubdate != NULL){
				call->pubdate(call->data, (char*) value);
			}
		}
		value=NULL;

    /*
     * Look for desciption
     */
    rc = getxpathstring(BAD_CAST todescription, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No description found");
    } else {
			if(call->description != NULL){
				call->description(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for comments
		 */
    rc = getxpathstring(BAD_CAST tocomments, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No commentsstr found");
    } else {
			if(call->comments != NULL){
				call->comments(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for guid
		 */
    rc = getxpathstring(BAD_CAST toguid, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No guidstr found");
    } else {
			if(call->guid != NULL){
				call->guid(call->data, (char*) value);
			}
		}
		value=NULL;

		/*
		 * Look for seeds
		 */
    rc = getxpathstring(BAD_CAST toseeds, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No seedsstr found");
    } else {
			if(call->seeds != NULL){
				integer=atoi((const char*) value);
				call->seeds(call->data, integer);
			}
		}
		value=NULL;

		/*
		 * Look for peers
		 */
    rc = getxpathstring(BAD_CAST topeers, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No peersstr found");
    } else {
			if(call->peers != NULL){
				integer=atoi((const char*) value);
				call->peers(call->data, integer);
			}
		}
		value=NULL;
		

		/*
		 * Look for size
		 */
    rc = getxpathstring(BAD_CAST tosize, xpathCtx, &value);
    if( rc < 0 ) {
      writelog(LOG_DEBUG, "No sizestr found");
    } else {
			if(call->size != NULL){
				filesize = atol((const char*) value);
				call->size(call->data, filesize);
			}
		}
		value=NULL;

    /*
     * Add gatered data to the database avoiding duplicates.
     */
		if(call->end != NULL) {
			call->end(call->data);
		}
  }

	/*
   * When done cleanup the our mess.
   */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

	/*
	 * All gone well.
	 */
  return 0;
}

