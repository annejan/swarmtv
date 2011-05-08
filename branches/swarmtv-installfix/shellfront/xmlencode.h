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

/*
 * Struct download xml encode
 * @Arguments
 *
 * @return
 * 0 on success, otherwise -1
 *
 *	int   id;			   ID of the RSS/torrent depends newtorrents id/downloaded id  
 *  char *name;      Name of the source 
 *  char *url;       URL of the source  
 *  char *parser;    Parser used to parse the source 
 *  char *metatype;  The metafiles source is going to provide 
 *  char *errstr;    Will be set when status == -1 
 *	int status;		   0 when download was successful, else -1 
 */
int rssfrsstoxml(struct_download *down, xmlChar **xmlbuff, int *buffersize);

/*
 * Free XML Document 
 * @arguments
 * xmlbuff pointer to the XMl buffer to be freed.
 */
void rssffreexmldoc(xmlChar *xmlbuff);

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
int rssfrsstoxml(struct_download *down, xmlChar **xmlbuff, int *buffersize);

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
int rssfsimpletoxml(simplefilter_struct *simple, xmlChar **xmlbuff, int *buffersize);

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
int rssfsqltoxml(filter_struct *filter, xmlChar **xmlbuff, int *buffersize);

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
int rssfdownedtoxml(downloaded_struct *downed, xmlChar **xmlbuff, int *buffersize);

/*
 * Encode struct_diskusage
 * METAFILETYPE  metatype;  Metatype of the test 
 * int           limit;     Limit set
 * int           use;       current usage
 * @arguments
 * usage      Structure to encode  
 * xmlbuff    Pointer to buffer containing XML  
 * buffersize size of the XML buffer
 * @return
 * 0 on success, otherwise -1
 */
int rssfusagetoxml(struct_diskusage *usage, xmlChar **xmlbuff, int *buffersize);

