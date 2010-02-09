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
#include <assert.h>
#include <string.h>
#include <sqlite3.h>

#include "types.h"
#include "curlfile.h"
#include "tbl.h"
#include "torrentparse.h"
#include "findtorrent.h"
#include "logfile.h"

typedef struct str { size_t len; char *str; } str_t;

/*
 * Find the key's we need and use them to fill the info
 */
static void setintstate(bencstate *state, long value){
  /*
   * no key ? nothing to do here
   */
  if(state->prevtype != 'k') {
    return;
  }

  //printf("'Prevkey '%s'\n", state->prevkey);

  /*
   * Add size to total
   */
  if(strcmp( "length", state->prevkey) == 0) {
    state->props->size += value;
  }

  /*
   * Get pieces lenght
   */
  if( strcmp("piece length", state->prevkey) == 0) {
    state->props->pieces_length = value;
  }
}

/*
 * Parse int attributes
 */
static int tbl_integer(void *ctx, long value)
{
  bencstate *state = (bencstate *) ctx;

  /*
   * update state
   */
  setintstate(state, value);

  /*
   * update state for next round
   */
  state->prevtype='i';
  state->prevint=value;

  //printf("Parsed int to: %ld\n", value);
	return 0;
}

/*
 * Pars bencoded string atributes
 */
static int tbl_string(void *ctx, char *value, size_t length)
{
  char *string;
  
  if(length > 300) {
    return 0;
  }

  /*
   * set state
   */
  bencstate *state = (bencstate *) ctx;
  state->prevtype='s';
  strncpy(state->prevstring, value, DATALEN);

  /*
   * terminate string
   */
  string = calloc(1, length+1);
  strncpy(string, value, length);

  free(string);
	return 0;
}

/*
 * Parse bencoded dictionary keys
 */
static int tbl_dictkey(void *ctx, char *value, size_t length)
{
  char *string;
  bencstate *state = (bencstate *) ctx;

  /*
   * terminate string
   */
  string = calloc(1, length+1);
  strncpy(string, value, length);

  /*
   * set state
   */
  state->prevtype='k';
  strncpy(state->prevkey, string, DATALEN);

  //printf("Parsed dictkey: %s\n", string);

  free(string);
	return 0;
}

/*
 * Callback structure used by tbl
 */
static tbl_callbacks_t callbacks = {
	tbl_integer,
	tbl_string,
	NULL,
	NULL,
	NULL,
	tbl_dictkey,
	NULL
};

/*
 * Allocate state struct
 */
static bencstate *allocbencstate(void){
  bencstate *state=NULL;

  state = calloc(1, sizeof(bencstate));
  state->props = calloc(1, sizeof(torprops));

  return state;
}

/*
 * Do parsing and handling here
 * Return a struct containing the fields we need.
 * Do not forget to free the stuct after use.
 */
static int parsetorrent(MemoryStruct *buffer, torprops **props)
{
	tbl_error_t err;
  bencstate *state;

  /*
   * allocate state object
   */
  state = allocbencstate();

  /*
   * parse bencoded data
   */
	err = tbl_parse(&callbacks, state, buffer->memory, buffer->memory+buffer->size);
  if(err != TBL_E_NONE){
    writelog(LOG_ERROR, "Error while parsing bencoded data.");
    return -1;
  }

  /*
   * Get the interesting part lose the rest
   */
  *props = state->props;
  free(state);

  return 0;
}

/*
 * Free torrentprop struct.
 */
void freetorprops(torprops *props)
{
  free(props);
}

/*
 * Provide the url to the torrent,
 * returns a struct containing some of the props of the torrent.
 * free struct afterwards
 */
int gettorrentinfo(char *url, torprops **props)
{
  MemoryStruct *buffer=NULL;
  int           rc=0;
  int           retval=0;
  char         *torurl=NULL;

  /*
   * Download to buffer
   */
  rc = findtorrent(url, &torurl, &buffer, RECURSE);
  if(rc != 1){
    writelog(LOG_ERROR, "could not find torrent '%s' %s:%d", url, __FILE__, __LINE__);
    *props=NULL;
    retval=-1;
  }
  // Do nothing for now with the found url.
  free(torurl);

  /*
   * Parse torrent
   */
  if(retval == 0) {
    rc = parsetorrent(buffer, props);
    if(rc != 0){
      writelog(LOG_ERROR, "could not parse torrent '%s' %s:%d", url, __FILE__, __LINE__);
      *props=NULL;
      retval=-1;
    }
  }

  /*
   * free buffer
   */
  freedownload(buffer);
  free(buffer);

  return retval;
}
