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
 * Default timeout when no -i is provided
 */
#define  DEFAULTSEC 1800

/*
 * options for after the getopt loop.
 */
typedef struct {
  int run;							// run 
  int nodetach;					// run in foreground 
  int testfilt;					// Only test the filter
	int findtorid;				// Find torrent id
	int autoseasep;       // Auto fill out season/episode
	int simplewizard;			// Prevents wizard from being started multiple times 
  LOOPMODE onetime;			// run once then exit
	char *sourcefilter; 	// source filter 
	char *source;					// source
	char *filter;					// SQL download filter
	char *doublefilter;		// SQL no duplicate filter
	char *simplename;			// Simple filter name
	char *simpletitle;		// Simple title regexp
	char *simpleexclude;	// Simple exclude regexp
  char *simplecategory; // Simple category regexp
	char *simplesource;   // Simple source regexp
	char *simplemaxsize;	// Simple max size
	char *simpleminsize;	// Simple minimal size
	char *simplenodup;	  // Simple no double filter type
	char *simpleseason;		// From what season to download
	char *simpleepisode;	// From episode
  char *simpledel;      // Name of simple filter to delete
} opts_struct;

/*
 * Handles the arguments, and Calls the subroutines when needed.
 * @arguments
 * argc from main
 * argv from main
 */
void rssthandleopts(rsstor_handle *handle, int argc, char *argv[]);

