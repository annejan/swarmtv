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
 *  Program written by Paul Honig 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "swarm.h"
#include "callback.h"

/*
 * Initialize the struct_callback.
 * @Arguments
 * callstruct pointer to structure to initialize
 * @return
 * 0 on success, -1 when initialisation fails
 */
int rsstinitcallback(struct_callback **callstruct)
{
	/*
	 * Allocate structure
	 */
	*callstruct = calloc(1, sizeof(struct_callback));
	if(*callstruct == NULL) {
		fprintf(stderr, "Allocation failed of callback structure failed. %s:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Done
	 */
	return 0;
}

/*
 * Free the callback structure
 * @Arguments
 * callstruct callback structure to free
 */
void rsstfreecallback(struct_callback *callstruct)
{
	/*
	 * Free should allow passing NULL pointer to it
	 */
	if(callstruct == NULL) {
		return;
	}

	/*
	 * free array of function pointers
	 */
	free(callstruct->callback);
	free(callstruct->data);

	/*
	 * Free callback structure itself
	 */
	free(callstruct);
}

/*
 * Add pointer.
 * @arguments
 * handle swarmtv handle
 * enumcall the name of the callback funct
 * callback pointer to the callback function
 * data   pointer that will be returned as the data pointer.
 * @return
 * 0 on successful addition, -1 on error
 */
int rsstaddcallback(rsstor_handle *handle, enum_callbacks callenum, rsstcallbackfnct callback, void *data)
{
	int                newcount=0;
  struct_callbacks   *callbacks=NULL;
  struct_callback   **callcol=NULL;
  struct_callback   *callstruct=NULL;

  /*
   * Get pointer
   */
  callbacks = &(handle->callback);
  callcol = (struct_callback**) callbacks;
  callstruct = callcol[callenum];

	/*
	 * Set new count
	 */
	newcount = callstruct->nr + 1;

	/*
	 * Reallocate the array to store the function pointer
	 */
	callstruct->callback = realloc(callstruct->callback, sizeof(rsstcallbackfnct) * newcount);
	if(callstruct->callback == NULL) {
		fprintf(stderr, "Allocation of callback pointer array failed! %s:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * Reallocate the array to store the data pointer
	 */
	callstruct->data = realloc(callstruct->data, sizeof(void*) * newcount);
	if(callstruct->callback == NULL) {
		fprintf(stderr, "Allocation of callback pointer array failed! %s:%d\n", __FILE__, __LINE__);
		return -1;
	}


	/*
	 * Add the new pointers
	 */
	callstruct->callback[newcount-1] = *callback;
	callstruct->data[newcount-1] = data;

	/*
	 * Add one to the pointer number
	 */
	callstruct->nr = newcount;

	/*
	 * All is done
	 */
	return 0;
}


/*
 * Call callbacks.
 * @arguments
 * callstruct	Structure of callbacks to execute one by one
 * data				Structure containing callback data (should be casted to proper structure)
 * @return
 * 0 on success, !0 when on of the called functions did not return 0
 */
int rsstexecallstruct(struct_callback *callstruct, void *calldata)
{
	int count=0;
	int retval=0;

	/*
	 * Walk past all pointers, and call the function pointers
	 */
	for(count=0; count < callstruct->nr; count++)
	{
		/*
		 * Or the return value, when one is !0 the return value != 0
		 */
		retval |= callstruct->callback[count](callstruct->data[count], calldata);
	}

	/*
	 * Return the results.
	 */
	return retval;
}


/*
 * execute routines that are handling on RSS download events
 * @Arguments
 * handle     Handle to RSS-torrent pointer
 * callenum   Name name of the callback to call
 * load       payload to provide together with the callback
 * @return
 * return 0 when all called functions returned 0, otherwise != 0
 */
int rsstexecallbacks(rsstor_handle *handle, enum_callbacks callenum, void *load)
{
  int rc=0;
  struct_callbacks *callbacks=NULL;
  struct_callback **callcol=NULL;
  struct_callback  *callstruct=NULL;

  /*
   * Get callback structure pointer.
   */
  callbacks = &(handle->callback);
  callcol = (struct_callback**) callbacks;
  callstruct = callcol[callenum];

  /*
   * call the callbacks.
   */
  rc = rsstexecallstruct(callstruct, load);

  /*
   * Return the execute value
   */
  return rc;
}

