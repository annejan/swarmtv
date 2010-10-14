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

#include "types.h"
#include "regexp.h"
#include "curlfile.h"
#include "findnzb.h"
#include "nzbparse.h"

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
   return 0;
}

