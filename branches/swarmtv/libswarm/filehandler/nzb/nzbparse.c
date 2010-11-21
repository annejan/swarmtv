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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "types.h"
#include "regexp.h"
#include "curlfile.h"
#include "findnzb.h"
#include "nzbparse.h"

/*
 * Get Document type
 * @Arguments
 * doc the XML-tree to get the doctype from
 * @return
 * pointer to the string, NULL if error
 */
char *getdocumenttype(xmlDocPtr doc)
{
  char *doctype=NULL;

  /*
   * Sanity checks
   */
  if(doc == NULL || doc->children == NULL){
    return NULL;
  }

  /*
   * Get name of main node in the XML-document
   */
  doctype = (char*) doc->children->name;

  return doctype;
}




/*
 * This function parses the XML data in the MemoryStruct
 * @arguments
 * chuck the memory structure containing the downloaded information.
 * doc XML document DOM tree 
 * @return
 * 0 on success -1 on failure
 */
static int parsefrommem(char *url, MemoryStruct *chunk, xmlDocPtr *doc) 
{
  /*
   * Sanity check
   */
  if(chunk == NULL) {
    rsstwritelog(LOG_DEBUG, "NULL pointer provided as chunk pointer. %s:%d", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Parse document in memory, using url as docbase RFC 2396,
   */
  *doc = xmlReadMemory(chunk->memory, chunk->size, url, NULL, 0);
  if (*doc == NULL) {
    rsstwritelog(LOG_DEBUG, "Failed to parse document %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return 0;
}


/*
 * Find desired attribute, and return value contained within attribute
 * @Arguments
 * attr  Pointer to first attribute
 * label Attribute label 
 * value value contained in the Attribute
 * @Return
 * returns 0 if all okay, otherwise -1
 */
static int getattributeval(xmlAttrPtr attr, char *label, char **value)
{
    if(attr == NULL) {
      return -1;
    }

    /*
     * Find the value
     */
    while(attr != NULL) { 
      if(!strcmp((char*)attr->name, label)) { 
        *value = (char*) attr->children->content;
        return 0;
      }
      attr = attr->next; 
    }

    /*
     * Nothing found in the attr collection.
     */
    return -1;
}


/*
 * Get total size from nzb
 * @arguments
 * doc libxml2 document pointer
 * @ return
 * 0 on success otherwise -1
 */
static int getnzbsize(xmlDocPtr doc, size_t *nzbsize, size_t *piece_nr)
{
  xmlXPathContextPtr xpathCtx=NULL;
  xmlXPathObjectPtr xpathObj=NULL;
  char *sizestr=NULL;
  size_t cumulative=0;
  int count=0;
  xmlAttrPtr attr=NULL;
  int rc=0;

  /*
   * XPath to get the segment nodes despite of the namespace involved.
   */
  char *sizesxpath="//*[local-name()='segment'][@bytes]";

  /* Create Xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    fprintf(stderr,"Error: unable to create new XPath context\n");
    return(-1);
  }

  /*
   * Query all size objects
   */
  xpathObj = xmlXPathEvalExpression(BAD_CAST sizesxpath, xpathCtx);
  if(xpathObj == NULL) {
    fprintf(stderr,"Error: unable to evaluate Xpath expression \"%s\"\n", sizesxpath);
    xmlXPathFreeContext(xpathCtx);
    return(-1);
  }

  /*
   * Add all sizes together
   */
  for (count=0; count < xpathObj->nodesetval->nodeNr; count++) {
    attr = xpathObj->nodesetval->nodeTab[count]->properties;
    rc = getattributeval(attr, "bytes", &sizestr);
    if(rc == 0) {
      //printf("keyword: %s\n", sizestr);
      cumulative += atoi(sizestr);
    }
  }
  
  /*
   * store the value
   */
  *nzbsize = cumulative;
  *piece_nr = xpathObj->nodesetval->nodeNr;

  /*
   * cleanup
   */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);

  return 0;
}


/*
 * Provide the URL to the NZB,
 * returns a structure containing some of the props of the NZB.
 * free structure afterwards
 * @arguments
 * url URL to get NZB from
 * props structure holding the NZB properties
 * @return
 * returns -1 on failure to parse URL, otherwise 0 is returned.
 */
int rsstgetnzbinfo(char *url, metafileprops **props)
{
  int            rc=0;
  int            retval=0;
  char          *nzburl=NULL;
  MemoryStruct  *nzbbuffer=NULL;
  xmlDocPtr      doc=NULL;

  /*
   * Initialize 
   */
  *props=NULL;

  /*
   * Download data
   */
  rc = rsstfindnzb(url, &nzburl, &nzbbuffer);
  if(rc != 0){
    rsstwritelog(LOG_DEBUG, "Downloading NZB '%s' failed.", url);
    retval=-1;
  }

  if(retval == 0) {
    /*
     * Parse the data
     */
    rc = parsefrommem(url, nzbbuffer, &doc);
      if(rc != 0){
        rsstwritelog(LOG_DEBUG, "Parsing NZB '%s' failed.", url);
        *props=NULL;
        retval=-1;
      }
  }

  /*
   * Use the doc structure to fill the metafileprops struct
   */
  if(retval == 0) {
    #if 0
    typedef struct {
      METAFILETYPE type;        // Tells what kind of meta file is presented
      size_t pieces_length;     // Size of pieces
      size_t size;              // Size of files enclosed
      size_t file_nr;           // Number of files in meta-file
    } metafileprops;
    #endif

    /*
     * Allocate the metafileprops structure
     */
    *props = calloc(1, sizeof(metafileprops));

    /*
     * Provide type of content
     */
    (*props)->type = nzb;

    /*
     * Get size and pieces number.
     * Use them to calculate the pieces length.
     */
    rc = getnzbsize(doc, &((*props)->size), &((*props)->file_nr));
    (*props)->pieces_length = (*props)->size/(*props)->file_nr;
  }

  /*
   * Clean up mess
   */ 
  free(nzburl);
  rsstfreedownload(nzbbuffer);
  free(nzbbuffer);
  xmlFreeDoc(doc);

  /*
   * Done
   */
  return retval;
}

