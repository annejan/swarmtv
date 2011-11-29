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
#include <curl/curl.h>
#include <sqlite3.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#ifdef __MINGW32__
#include "../../types.h"  /* looks damn ugly! */
#else
#include "types.h"
#endif
#include "logfile.h"
#include "curlfile.h"
#include "twitparse.h"


static char *tostatus="/statuses/status";
static char *totext="text";
static char *tocreatedat="created_at";

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
 * 'string' should not be freed, as it is pointing to a string inside the DOM-tree.
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
 * filter to handle incomming files from http://www.rsstorrents.com
 */
int twitparse(twitparse_callback *call, char *url, MemoryStruct *buffer)
{
  xmlDocPtr           doc = NULL; /* the resulting document tree */
  //xmlNode             *root_element = NULL;
  //xmlNode *cur_node = NULL;
  xmlXPathContextPtr  xpathCtx; 
  xmlXPathObjectPtr   xpathObj; 
  xmlNodeSetPtr       nodeset;
  int                 count = 0;
	unsigned char				*text;
	unsigned char				*date;
  int                 rc = 0;

  /*
   * Use the imput to feed libxml2
   */
  doc = xmlReadMemory(buffer->memory, buffer->size, url, NULL, 0);
  if (doc == NULL) {
    //rsstwritelog(LOG_ERROR, "Failed to parse document %s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Failed to parse document %s:%d\n", __FILE__, __LINE__);
    return -1;
  }
  //root_element = xmlDocGetRootElement(doc);

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    //rsstwritelog(LOG_ERROR, "Error: unable to create new XPath context %s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Error: unable to create new XPath context %s:%d\n", __FILE__, __LINE__);
    xmlFreeDoc(doc); 
    return(-1);
  }

  /*
   * Get the item nodes in channel.
   */
  xpathObj = myXmlXPathEval(BAD_CAST tostatus, xpathCtx);
  if(xpathObj == NULL) {
    //rsstwritelog(LOG_ERROR, "Error: unable to evaluate xpath expression \"%s\" %s:%d\n", toitems, __FILE__, __LINE__);
    fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\" %s:%d\n", tostatus, __FILE__, __LINE__);
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
		 * Call the newtwit hook
		 */
		if(call->newtweet != NULL){
			call->newtweet(call->data);
		}

		/*
		 * Get text
		 */
		rc = getxpathstring(BAD_CAST totext, xpathCtx, &text);
		if( rc < 0 ) {
			
		} else {
			//rsstwritelog(LOG_DEBUG, "title: %s", title);
		 if(call->gettext != NULL){
				rc = call->gettext(call->data, (char*) text);
				if(rc < 0) {
					rsstwritelog(LOG_ERROR, "gettext returned '%d'. %s:%d", rc, __FILE__, __LINE__);
				}
		 }
		}

		/*
		 * Get created_at
		 */
		rc = getxpathstring(BAD_CAST tocreatedat, xpathCtx, &date);
		if( rc < 0 ) {
		} else {
			if(call->getcreatedate != NULL) {
				rc = call->getcreatedate(call->data, (char*) date); // Called when createdate is found
				if(rc < 0) {
					rsstwritelog(LOG_ERROR, "getcreatedate returned '%d'. %s:%d", rc, __FILE__, __LINE__);
				}
			}
		}

		/*
		 * Call the endtweet hook
		 */
		if(call->endtweet != NULL){
			call->endtweet(call->data);
		}
	}

	/*
	 * Call the donetweet hook parsing done.
	 */
	if(call->donetweet != NULL){
		call->donetweet(call->data);
	}

	/*
	 * When done cleanup the our mess.
	 */
	xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(xpathCtx);
	xmlFreeDoc(doc);

	return 0;
}
