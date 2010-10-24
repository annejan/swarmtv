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
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "types.h"
#include "regexp.h"
#include "curlfile.h"
#include "logfile.h"
#include "nzbparse.h"
#include "findnzb.h"


/*
 * Test if the downloaded file really is an NZB 
 * @arguments
 * @return
 * -1 when not an NZB
 * 0 when the file is and NZB
 */
static int testifnzb(xmlNode *root)
{
  char *doctype=NULL;
  //int  rc=0;

  /*
   * Sanity checks
   */
  if(root == NULL || root->name == NULL)
  {
    /*
     * No XML structure is no NZB :)
     */
    rsstwritelog(LOG_ERROR, "Document pointed not sane! %s:%d\n", __FILE__, __LINE__);
    return -1;
  }

  /*
   * Retrieve the DOCTYPE
   */
  doctype = (char*) root->name;

  /*
   * Test if the doctype is 'nzb'
   */
  if(strcmp(doctype, "nzb") != 0) {
    /*
     * Not an NZB
     */
    rsstwritelog(LOG_DEBUG, "Doctype not NZB: '%s' %s:%d", __FILE__, __LINE__);
    return -1;
  } 

  /*
   * We found an NZB
   */
  return 0;
}

/*
 * Parse XML Document from buffer
 * @arguments
 * url  The URL that gets included in the document as the source
 * buffer The buffer containing the (hopefully) XML-data
 * doc Do
 * root_element
 * @return
 *
 */
int rsstparsebufxml(char *url, MemoryStruct *buffer, xmlNode **root_element)
{
  xmlDocPtr doc=NULL;

  /*
   * Read from memory
   */
  doc = xmlReadMemory(buffer->memory, buffer->size, url, NULL, 0);
  if (doc == NULL) {
    rsstwritelog(LOG_ERROR, "Failed to parse RSS document: '%s' %s:%d", url, __FILE__, __LINE__);
    *root_element=NULL;
    return -1;
  }
  *root_element = xmlDocGetRootElement(doc);

  /*
   * All gone well
   */
  return 0;
}

/*
 * This is a recursive function.
 * It scans for NZB files, and will return the first one it encounters.
 * Note this might not be the correct NZB at all :)
 * A counter prevents the recursing from getting out of hand.
 * The NZB will be contained in nzbbuffer.
 * The URL the NZB was found in NZB URL.
 * Don't forget to free buffer and NZB URL !
 * @arguments
 * url the URL to start searching.
 * nzburl the URL found containing the NZB.
 * nzbbuffer the buffer the NZB is returned in when found. 
 * recurse the number of recursions to do to find the NZB.
 * @return
 * -1 when nothing was found
 * 0 when the NZB is found
 */
int rsstfindnzb(char *url, char **nzburl, MemoryStruct **nzbbuffer)
{
  int          rc=0;
  xmlNode     *root=NULL;
  MemoryStruct buffer;
  int          doctype=0;

  /*
   * Initialize stuff
   */
  memset(&buffer, 0, sizeof(MemoryStruct));

  /*
   * Download URL into buffer.
   */
  rc = rsstdownloadtobuffer( url, &buffer);
  if(rc != 0) {
    rsstwritelog(LOG_NORMAL,  "%s: Failed to download.", url);
	  rsstfreedownload(&buffer);
    return 0;
  }

  /*
   * Parse into XML-document
   */
  rc = rsstparsebufxml(url, &buffer, &root);

  /*
   * Get document type
   */
  doctype = testifnzb(root);

  /*
   * When the type is NZB, copy the URL and return 0
   */
  if(doctype == 0){
    /*
     * Set URL to current URL
     */
    rsstalloccopy(nzburl, url, strlen(url));

    /*
     * Set output buffer
     */
    rsstalloccopy((void*)nzbbuffer, (void*) &buffer, sizeof(MemoryStruct));

    /*
     * Success, return 0
     */
    return 0;
  }

  /*
   * make sure we clean up
   * If not give up for now and return -1 (not found)
   */
  rsstfreedownload(&buffer);

  rsstwritelog(LOG_NORMAL, "Document not an NZB file, URL: '%s' %s:%d\n", url, __FILE__, __LINE__);

  return -1;
}


/*
 * Finds and writes NZB to file
 * @arguments
 * url the URL to start looking for a NZB.
 * name the path to store the NZB on disk.
 * @return
 * 0 on success
 * -1 when NZB was not found or could not be stored.
 */
int rsstfindnzbwrite(char *url, char *name)
{
  int             rc = 0;
  int             rv = 0;
  char            *nzburl = NULL;
  MemoryStruct    *buffer = NULL;

  rsstwritelog(LOG_DEBUG, "Attempting Writing nzb '%s' to file '%s'", url, name);

  /*
   * Get the buffer and URL to the torrent in there
   */
  rc = rsstfindnzb(url, &nzburl, &buffer);
  if(rc != 1) {
    rsstwritelog(LOG_NORMAL, "NZB not found in '%s'", url);
    rv=-1;
  }

  /*
   * Print to result
   */
  if(rv == 0 && strcmp(url, nzburl) != 0) {
    rsstwritelog(LOG_NORMAL, "Original URL : %s", url);
    rsstwritelog(LOG_NORMAL, "NZB URL   : %s", nzburl);
  }

  /*
   * Save file to name
   */
  if(rv == 0) {
    rc = rsstwritebuffer(name, buffer);
    if(rc != 0){
      rsstwritelog(LOG_ERROR, "Writing NZB '%s' failed.", name);
      rv = -1;
    }
  }

  /*
   * free the result
   */
  free(nzburl);
  rsstfreedownload(buffer);
  free(buffer);

  return rv;
}

