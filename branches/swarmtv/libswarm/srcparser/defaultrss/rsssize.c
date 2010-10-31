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
#include <sqlite3.h>
#include <time.h>
#include <stdlib.h>

#include "types.h"
#include "logfile.h"
#include "config.h"
#include "defaultrss.h"
#include "filehandler/filehandler.h"
#include "disectdescription.h"

/*
 * Use the rss data to fill out the seeds and peers field in the new torrent struct.
 * Arguments
 * newtor		: New torrent structure.
 * rssdata	: The struct holding the raw rssdata.
 * Returns	
 * 0 on succes, -1 on failure.
 * Errors are logged.
 */
int rsssize(newtorrents_struct *newtor, rssdatastruct *rssdata)
{
	int				    rc=0;
	size_t 		    i_size=0;
	size_t 		    i_length=0;
	long			    min_config=0;
	metafileprops	*props=NULL;
	int				    retval=0;
  METAFILETYPE  type=undefined;

	/*
	 * Get from size node.
	 */
	i_size = rssdata->size;

	/*
	 * Get size from enclosurelength
	 */
	i_length = rssdata->enclosurelength;

	/*
	 * Pick the biggest.
	 */
	if(i_size > i_length) {
		newtor->size = i_size;
	} else {
		newtor->size = i_length;
	}

  /*
   * convert metafiletype
   */
  rc = metafilestrtotype(newtor->metatype, &type);
  if(rc != 0){
		rsstwritelog(LOG_ERROR, "Not corrent meta type '%s' %s:%d", 
      newtor->metatype, __FILE__, __LINE__ );
		return -1;
  }

	/*
	 * When smaller than 'min_size'
	 */
	rc = rsstconfiggetlong(rssdata->handle, CONF_MIN_SIZE , &min_config);
	if(rc != 0){
		rsstwritelog(LOG_ERROR, "Configuration variable '%s' not set!", CONF_MIN_SIZE);
		return -1;
	}

	if( newtor->size < (size_t) min_config) {
		/*
		 * Download the torrent to verify the length
		 */
		rc = rsstgetmetafileinfo(type, rssdata->link, &props);
		if(rc == 0) {
			newtor->size = props->size;	
		} else {
			retval = -1;
		}
		rsstfreemetafileprops(props);
	}

	return retval;
}

