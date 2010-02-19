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
#include <sqlite3.h>

#include "types.h"
#include "logfile.h"
#include "regexp.h"

/*
 * disectdescription
 * Arguments
 * description	Description string
 * key					Keyword to extract 
 * retrieved		Retrieved string (free after usage)
 * return values
 * return 0 when found, -1 when not found
 */
int disectdescription(char *description, char *key, char **retrieved)
{
	int 	rc=0;
	char *desc=NULL;
	char *token=NULL;
	char *split=NULL;
	char *fkey=NULL;
	char *fvalue=NULL;

	*retrieved=NULL;

	/*
	 * When not set return -1
	 */
	if(description == NULL){
		return -1;
	}

	/*
	 * Move description to local buffer
	 */
	alloccopy(&desc, description, strlen(description));


	/*
	 * Getopt through the string
	 */
	token = strtok(desc, ";");
	while (token != NULL)
	{
		rc=0;
		fkey=NULL;
		fvalue=NULL;

		/*
		 * Split key and value.
		 */
		split = strchr(token, ':');
		if(split == NULL) {
			printf("token: %s\n", split);
			token = strtok (NULL, ";");
			continue;
		}

		/*
		 * Get both values
		 */
		rc =  alloccopy(&fkey, token,	split-token);
		if(rc != 0){
			writelog(LOG_ERROR, "Alloc failed on %s:%d", __FILE__, __LINE__);
			exit(1);
		}
		rc |= alloccopy(&fvalue, split+1, strlen(split+1));
		if(rc != 0){
			writelog(LOG_ERROR, "Alloc failed on %s:%d", __FILE__, __LINE__);
			exit(1);
		}
		cleanupstring(fkey);
		cleanupstring(fvalue);

		/*
		 * When found copyalloc to retrieved
		 */
		if(fkey != NULL && strcmp(fkey, key) == 0){
			rc = alloccopy(retrieved, split+1, strlen(split+1));

			free(fkey);
			free(fvalue);
			free(desc);

			return 0;
		}

		/*
		 * Clean up for next cycle
		 */
		free(fkey);
		free(fvalue);
		token = strtok (NULL, ";");
	}

	/*
	 * failed return -1 after cleanup.
	 */
	free(desc);
	return -1;
}

