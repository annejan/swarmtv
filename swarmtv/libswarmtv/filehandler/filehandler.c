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

#ifdef __MINGW32__
#include "../types.h"  /* looks stupid! */
#else
#include "types.h"
#endif
#include "regexp.h"
#include "curlfile.h"
#include "filehandler.h"
#include "torrent/findtorrent.h"
#include "torrent/torrentparse.h"
#include "nzb/findnzb.h"
#include "nzb/nzbparse.h"

/*
 * char array holding the types of metafiles supported by librsstorrent
 */
char *supported_metatypes[] ={
  "torrent",
  "nzb",
  NULL
};

/*
 * Get supported meta file types
 * @return
 * Returns a pointer to the names of the supported meta types.
 */
char **getsupportedmetatypes()
{
  return supported_metatypes;
}

/*
 * Convert string into METAFILETYPE 
 * @arguments
 * metastr META file string
 * metatype metafile output string
 * @return
 * 0 on success, -1 when not match is found.
 */
int metafilestrtotype(char *metastr, METAFILETYPE *type)
{
  int retval=0;
  
  if(!strcmp(metastr, "torrent")) {
    *type=torrent;
  }
  else if(!strcmp(metastr, "nzb")) {
    *type=nzb;
  }
  else {
    *type=undefined;
    retval=-1;
  } 

  return retval;
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
int rsstgetmetafileinfo(rsstor_handle *handle, METAFILETYPE type, char *url, metafileprops **props)
{
  int rc=0;

  /*
   * Switch between the 2 meta file types.
   */
  switch(type){
    case torrent:
      rc = rsstgettorrentinfo(handle, url, props);
      break;
    case nzb:
      rc = rsstgetnzbinfo(handle, url, props);
      break;
    case undefined:
      /*
       * When this happens, just fail
       */
      rsstwritelog(LOG_ERROR, "'url' requested metafile info without valid meta file type %s:%d",
        url, __FILE__, __LINE__);
      *props=NULL;
      rc=-1;
      break;
  } 

  return rc;
}


/*
 * Free torrentprop structure.
 * @arguments
 * props structure to be freed
 */
void rsstfreemetafileprops(metafileprops *props)
{
  /*
   * For now just free the structure.
   */
  free(props);
}


/*
 * This is a recursive function.
 * It scans for torrent files, and will return the first one it encounteres.
 * Note this might not be the correct torrent at all :)
 * A counter prevents the recursing from getting out of hand.
 * The torrent wil be contained in torbuffer.
 * The url the torrent was found in torrenturl.
 * Don't forget to free buffer and torrenturl !
 * @arguments
 * url the url to start searching.
 * torrenturl the url found containing the torrent.
 * torbuffer the buffer the torrent is returned in when found. 
 * recurse the number of recursions to do to find the torrent.
 * @return
 * 0 when nothing is found
 * 1 when the torrent was found
 */
int rsstfindmetafile(rsstor_handle *handle, METAFILETYPE type, char *url, char **torrenturl, MemoryStruct **torbuffer, int recurse)
{
  int rc=0;

  switch(type){
    case torrent:
      rc = rsstfindtorrent(handle, url, torrenturl, torbuffer, recurse);
      break;
    case nzb:
      rc = rsstfindnzb(handle, url, torrenturl, torbuffer);
      break;
    case undefined:
      /*
       * When this happens, just fail
       */
      rsstwritelog(LOG_ERROR, "'url' requested metafile info without valid meta file type %s:%d",
        url, __FILE__, __LINE__);
      *torrenturl=NULL;
      *torbuffer=NULL;
      rc=-1;
      break;
  } 

  return rc;
}


/*
 * Finds and writes torrent to file
 * @arguments
 * url the url to start looking for a torrent.
 * name the path to store the torrent on disk.
 * @return
 * 0 on success
 * -1 when torrent was not found or could not be stored.
 */
int rsstfindmetafilewrite(rsstor_handle *handle, METAFILETYPE type, char *url, char *name, char *filtername)
{
  int rc=0;

  switch(type){
    case torrent:
      rc = rsstfindtorrentwrite(handle, url, name, filtername);
      break;
    case nzb:
      rc = rsstfindnzbwrite(handle, url, name, filtername);
      break;
    case undefined:
      /*
       * When this happens, just fail
       */
      rsstwritelog(LOG_ERROR, "'url' requested metafile info without valid meta file type %s:%d",
        url, __FILE__, __LINE__);
      rc=-1;
      break;
  } 

  return rc;
}

