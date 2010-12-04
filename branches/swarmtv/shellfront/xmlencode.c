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
 *  Program written by Paul Honig 2009 - 2010
 */

#define  NUMLENGHT 30

#include <stdio.h>
#include <libxml/parser.h>

#include <swarm.h>


/*
 * Create document and add root node
 */
static void rssfnewdocument(xmlDocPtr *doc, char *rootname)
{
  xmlNodePtr n;

  /*
   * Create the document.
   */
  *doc = xmlNewDoc(BAD_CAST "1.0");
  n = xmlNewNode(NULL, BAD_CAST rootname);
  xmlDocSetRootElement(*doc, n);
}


/*
 * Add a new child node, containing int value
 * @arguments
 * parent   parent node 
 * ns       namespace the node will be in
 * name     name of the node
 * num      int to add to the node
 */
static void rssfxmlnewintchild(xmlNodePtr parent, xmlNsPtr ns, const xmlChar * name, int num)
{
  char numstr[NUMLENGHT+1];
  
  /*
   * int to string
   */
  numstr[NUMLENGHT]='\0';
  snprintf(numstr, NUMLENGHT, "%d", num);

  /*
   * Add child node
   */
  xmlNewTextChild   (parent, ns, name, BAD_CAST numstr);
}


/*
 * Add a new child node, containing int value
 * @arguments
 * parent   parent node 
 * ns       namespace the node will be in
 * name     name of the node
 * num      double to add to the node
 */
static void rssfxmlnewdoublechild(xmlNodePtr parent, xmlNsPtr ns, const xmlChar * name, double num)
{
  char numstr[NUMLENGHT+1];
  
  /*
   * int to string
   */
  numstr[NUMLENGHT]='\0';
  snprintf(numstr, NUMLENGHT, "%.0f", num);

  /*
   * Add child node
   */
  xmlNewTextChild   (parent, ns, name, BAD_CAST numstr);
}


/*
 * Free XML Document 
 * @arguments
 * xmlbuff pointer to the XMl buffer to be freed.
 */
void rssffreexmldoc(xmlChar *xmlbuff)
{
  /*
   * Clean up
   */
  xmlFree(xmlbuff);
}


/*
 * Struct download xml encode
 *	int   id;			   ID of the RSS/torrent depends newtorrents id/downloaded id  
 *  char *name;      Name of the source 
 *  char *url;       URL of the source  
 *  char *parser;    Parser used to parse the source 
 *  char *metatype;  The metafiles source is going to provide 
 *  char *errstr;    Will be set when status == -1 
 *	int status;		   0 when download was successful, else -1 
 * @Arguments
 * down       Structure to encode
 * xmlbuff    Pointer to buffer containing XML
 * buffersize Size of the XML buffer
 * @return
 * 0 on success, otherwise -1
 */
int rssfrsstoxml(struct_download *down, xmlChar **xmlbuff, int *buffersize)
{
  xmlDocPtr   doc=NULL;
  xmlNodePtr  root=NULL;
  xmlNodePtr  swarmtv=NULL;

  rssfnewdocument(&doc, "swarmtv");

  root = xmlDocGetRootElement(doc);
  swarmtv =  xmlNewTextChild(root, NULL, BAD_CAST "rss", NULL);

  /*
   * Add data
   */
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "id"   , down->id);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "name" , BAD_CAST down->name);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "url" , BAD_CAST down->url);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "parser" , BAD_CAST down->parser);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "metatype" , BAD_CAST down->metatype);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "errstr" , BAD_CAST down->errstr);
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "status"   , down->status);

  /*
   * Dump generated document to string
   */
  xmlDocDumpFormatMemory(doc, xmlbuff, buffersize, 1);

  /*
   * Free associated memory.
   */
  xmlFreeDoc(doc);

  return (0);
}


/*
 * simplefilter_struct
 *	int   id;				 Id of the filter
 *	char *name;			 Simple filter name
 *	char *title;		 Simple title regexp
 *	char *exclude;	 Simple exclude regexp
 *  char *category;  Simple category
 *	char *source;		 Source the newtorrent originated from
 *	double maxsize;	 Simple max size
 *	double minsize;	 Simple minimal size
 *	char *nodup;	 Simple no double filter type
 *	int fromseason;		 From what season to download
 *	int fromepisode;	 From episode
 * @Arguments
 * simple     Structure to encode
 * xmlbuff    Pointer to buffer containing XML
 * buffersize Size of the XML buffer
 * @return
 * 0 on success, otherwise -1
 */
int rssfsimpletoxml(simplefilter_struct *simple, xmlChar **xmlbuff, int *buffersize)
{
  xmlDocPtr   doc=NULL;
  xmlNodePtr  root=NULL;
  xmlNodePtr  swarmtv=NULL;

  rssfnewdocument(&doc, "swarmtv");

  root = xmlDocGetRootElement(doc);
  swarmtv =  xmlNewTextChild(root, NULL, BAD_CAST "simple", NULL);

  /*
   * Add data
   */
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "id"   , simple->id);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "name" , BAD_CAST simple->name);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "title" , BAD_CAST simple->title);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "exclude" , BAD_CAST simple->exclude);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "category" , BAD_CAST simple->category);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "source" , BAD_CAST simple->source);
  rssfxmlnewdoublechild(swarmtv, NULL, BAD_CAST "maxsize", simple->maxsize);
  rssfxmlnewdoublechild(swarmtv, NULL, BAD_CAST "minsize", simple->minsize);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "nodup" , BAD_CAST simple->nodup);
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "fromseason"   , simple->fromseason);
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "fromepisode"   , simple->fromepisode);
  
  /*
   * Dump generated document to string
   */
  xmlDocDumpFormatMemory(doc, xmlbuff, buffersize, 1);

  /*
   * Free associated memory.
   */
  xmlFreeDoc(doc);

  return (0);
}


/*
 * Encode filter_struct
 *	int 	id;				 Id of the filter
 *	char *name;			 Name of the filter
 *	char *filter;		 SQL of the filter
 *	char *nodup;		 SQL of the avoiding duplicates filter
 * @Arguments
 * filter     Structure to encode
 * xmlbuff    Pointer to buffer containing XML
 * buffersize Size of the XML buffer
 * @return
 * 0 on success, otherwise -1
 */
int rssfsqltoxml(filter_struct *filter, xmlChar **xmlbuff, int *buffersize)
{
  xmlDocPtr   doc=NULL;
  xmlNodePtr  root=NULL;
  xmlNodePtr  swarmtv=NULL;

  rssfnewdocument(&doc, "swarmtv");

  root = xmlDocGetRootElement(doc);
  swarmtv =  xmlNewTextChild(root, NULL, BAD_CAST "sql", NULL);

  /*
   * Add data
   */
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "id"      , filter->id);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "name"    , BAD_CAST filter->name);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "filter"  , BAD_CAST filter->filter);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "nodup"   , BAD_CAST filter->nodup);

  /*
   * Dump generated document to string
   */
  xmlDocDumpFormatMemory(doc, xmlbuff, buffersize, 1);

  /*
   * Free associated memory.
   */
  xmlFreeDoc(doc);

  return (0);
}

/*
 * Encode downloaded_struct
 *	int   id;           
 *	char *title;         Title from the RSS feed
 *	char *link;          The link to the metafile
 *	char *pubdate;       The data the meta file was released 
 *	char *category;      The category from the RSS feed
 *  char *metatype;      The type of content Torrent or NZB
 *  char *baretitle;     The bare title of the show/content 
 *	int  season;         The season number
 *	int  episode;        The episode number
 * @Arguments
 * filter     Structure to encode
 * xmlbuff    Pointer to buffer containing XML
 * buffersize Size of the XML buffer
 * @return
 * 0 on success, otherwise -1
 */
int rssfdownedtoxml(downloaded_struct *downed, xmlChar **xmlbuff, int *buffersize)
{
  xmlDocPtr   doc=NULL;
  xmlNodePtr  root=NULL;
  xmlNodePtr  swarmtv=NULL;

  rssfnewdocument(&doc, "swarmtv");

  root = xmlDocGetRootElement(doc);
  swarmtv =  xmlNewTextChild(root, NULL, BAD_CAST "downed", NULL);

  /*
   * Add data
   */
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "id"        , downed->id);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "title"     , BAD_CAST downed->title);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "link"      , BAD_CAST downed->link);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "pubdate"   , BAD_CAST downed->pubdate);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "category"  , BAD_CAST downed->category);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "metatype"  , BAD_CAST downed->metatype);
  xmlNewTextChild   (swarmtv, NULL, BAD_CAST "baretitle" , BAD_CAST downed->baretitle);
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "season"    , downed->season);
  rssfxmlnewintchild(swarmtv, NULL, BAD_CAST "episode"   , downed->episode);

  /*
   * Dump generated document to string
   */
  xmlDocDumpFormatMemory(doc, xmlbuff, buffersize, 1);

  /*
   * Free associated memory.
   */
  xmlFreeDoc(doc);

  return (0);
}

