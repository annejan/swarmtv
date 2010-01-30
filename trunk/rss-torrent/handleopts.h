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

/*
 * Program version
 */
#define PROGVERSION "0.6b"

/*
 * Default timeout when no -i is provided
 */
#define  DEFAULTSEC 3600

/*
 * options for after the getopt loop.
 */
typedef struct {
  int run;							// run 
  int nodetach;					// run in forground 
  int testfilt;					// Only test the filter
  int onetime;					// run once then exit
	char *sourcefilter; 	// source filter 
	char *source;					// source
	char *filter;					// sql downloadfilter
	char *doublefilter;		// sql no duplicate filter
	char *simplename;			// Simple filter name
	char *simpletitle;		// Simple title regexp
	char *simplemaxsize;	// Simple max size
	char *simpleminsize;	// Simple minimal size
	char *simplenodup;	// Simple no double filter type
	char *simpleseason;		// From what season to download
	char *simpleepisode;	// From episode
  char *simpledel;      // Name of simple filter to delete
} opts_struct;

void handleopts(sqlite3 *db, int argc, char *argv[]);

