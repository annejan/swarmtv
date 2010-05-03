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
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "database.h"
#include "config.h"
#include "regexp.h"
#include "filesystem.h"
#include "source.h"
#include "filter.h"
#include "logfile.h"
#include "daemonize.h"
#include "mailmsg.h"
#include "sandboxdb.h"
#include "torrentdownload.h"
#include "handleopts.h"
#include "runloop.h"
#include "testfilter.h"
#include "simplefilter.h"
#include "filesystem.h"
#include "torrentdb.h"
#include "present.h"

/*
 * optstring what options do we allow
 */
static const char *optString = "vcC:hfF:T:t:d:s:SD:rnm:p:qRe:o:O:u:g:G:Jj:P:N:kAU:Kl:i:M:La:";

/*
 * Long opts
 */
static const struct option optLong[] =
{
	/* These options set a flag. */
	//{"verbose", no_argument,       &verbose_flag, 1},
	//{"brief",   no_argument,       &verbose_flag, 0},
	/* These options don't set a flag.
		 We distinguish them by their indices. */
	{"version",     					no_argument,       0, 'v'},
	{"list-config",  					no_argument,       0, 'c'},
	{"list-filters",					no_argument,       0, 'f'},
	{"list-sources",					no_argument,       0, 'S'},
	{"print-filter",  				required_argument, 0, 'p'},
	{"set-config",  					required_argument, 0, 'C'},
	{"add-source",   					required_argument, 0, 's'},
	{"source-parser",   			required_argument, 0, 't'},
	{"add-filter",   					required_argument, 0, 'F'},
	{"duplicate-filter", 			required_argument, 0, 'T'},
	{"del-filter", 		    		required_argument, 0, 'd'},
	{"del-source",    				required_argument, 0, 'D'},
	{"run", 									no_argument, 			 0, 'r'},
	{"once", 									no_argument, 			 0, 'R'},
	{"test", 									no_argument, 			 0, 'q'},
	{"nodetach", 							no_argument, 			 0, 'n'},
	{"test-mail", 						required_argument, 0, 'm'},
	{"help", 									no_argument, 			 0, 'h'},
  {"list-simple", 				  no_argument,       0, 'J'},
	{"print-simple", 					required_argument, 0, 'P'},
	{"print-all-simple",			no_argument, 			 0, 'A'},
	{"del-simple", 						required_argument, 0, 'j'},
	{"del-all-simple", 				no_argument, 			 0, 'k'},
	{"add-simple", 						required_argument, 0, 'e'},
	{"title",									required_argument, 0, 'E'},
	{"exclude",								required_argument, 0, 'N'},
  {"category",              required_argument, 0, 'U'},
  {"source",              	required_argument, 0, 'l'},
	{"max-size", 							required_argument, 0, 'O'},
	{"min-size", 							required_argument, 0, 'o'},
	{"nodup", 							  required_argument, 0, 'u'},
	{"from-season", 				  required_argument, 0, 'g'},
	{"from-episode", 				  required_argument, 0, 'G'},
	{"reinit-database", 		  no_argument, 			 0, 'K'},
	{"id-download",           required_argument, 0, 'i'},
	{"id-del-downed",					required_argument, 0, 'M'},
	{"find",									no_argument,			 0, 'L'},
	{"find-downed",           required_argument, 0, 'a'},
	{0, 0, 0, 0}
};


/*
 * This routine prints the help-options
 */
static void printhelp(void) 
{
  printf( "################\n"  
          "help             -h               : Print this help.\n"  
          "nodetach         -n               : Don't detach from console.\n"  
          "once             -R               : Run once, then exit.\n"  
          "run              -r               : Run in daemon mode.\n"  
          "test             -q               : Test filter (together with -F & -T).\n"  
          "test-mail        -m <text>        : Send testmail notification.\n"  
          "version          -v               : Print version.\n"  
					"reinit-database  -K               : Reinitialize the database. (warning: data is lost)\n"
					"\nDownload manually\n"
					"id-download      -i <id>          : Download torrents by entering id (use --test to retrieve id's)\n"
					"id-del-downed    -M <id>          : Remove downloaded torrent from puplicate check table.\n"
					"find             -L               : Find torrents. (use with simple options)\n"
					"find-downed      -a <regexp>      : Find entries in the downloaded table.\n"
          "\nConfig settings\n"
          "list-config      -c               : List config Items and their values.\n"  
          "set-config       -C <name:value>  : Set a config value.\n"  
          "\nSource\n"
          "add-source       -s <name:url>    : Set RSS source. (default parser type 'defaultrss')\n"  
          "del-source       -D <name>        : Delete RSS source.\n"  
          "list-sources     -S               : List RSS sources.\n"  
          "source-parser    -t <type>        : Set RSS source parser type. (use with -s)\n"  
          "\nSQL Download filters\n"
          "del-filter       -d <name>        : Delete filter, name 'all' to delete all filters.\n"  
          "list-filters     -f               : List filters.\n"  
          "print-filter     -p <name>        : Print filter in shell format.\n"  
          "add-sql-filter   -F <name:query>  : Set SQL download filter (use with -T).\n"  
          "nodup-sql-filter -T <query>       : Set SQL duplicate filter.\n"  
          "\nSimple Download filters\n"
          "list-simple      -J               : List all simple filters\n"
          "print-simple     -P <name>        : Print simple filter\n"
          "print-all-simple -A               : Print all simple filters\n"
          "del-simple       -j <name>        : Delete a simple filter\n"
          "del-all-simple   -k               : Delete all simple filters\n"
          "add-simple       -e <name>        : Add a simple download filter\n"
          "title            -E <regexp>      : Title expression\n"
          "exclude          -N <regexp>      : Exclude expression\n"
          "category         -U <regexp>      : Category expression\n"
          "source           -l <regexp>      : Source expression\n"
          "max-size         -O <size>        : Maximal size of downloaded torrent\n"
          "min-size         -o <size>        : Minimal size of downloaded torrent\n"
          "nodup            -u <type>        : No duplicate filter type (unique, newer, link, none)\n"
          "from-season      -g <value>       : Season number to start downloading from.\n"
          "from-episode     -G <value>       : Episode number to start downloading from.\n"               
          "\n"
          "For examples see man rsstorrent(1)\n"
          "\n"
          "The long arguments need to be preceded with '--'\n"  
          "Example: rsstorrent --help\n"  
          "################\n\n");  
}

/*
 * Set config value, and handle failure
 */
static void setconfigvalue(rsstor_handle *handle, char *configval)
{
	int						rc=0;
	char 				 *name=NULL;
	char				 *value=NULL;

	/*
	 * REMOVE IN THE FUTURE
	 */

	rsstsplitnameval(configval, &name, &value);
	rc = rsstsetconfigitem(handle, name, value);
	if(rc == -1) {
		fprintf(stderr, "Value not found in config\n");
	} else {
		printf("config value: %s, set to: %s\n", name, value);
	}

	free(name);
	free(value);
}

static void optdeletefilter(rsstor_handle *handle, char *name)
{
	int rc;

	/*
	 * Name "all" deletes all filters.
	 */
	if(!strcmp(name, "all")) {
		rc = rsstdelallfilters(handle);
		if(rc == 0) {
			printf("Delete all filters succesful\n");
		} else {
			fprintf(stderr, "Deletion of all filters failed\n");
		}
	} else {
		rc = rsstdelfilter(handle, name);
		if(rc == 0) {
			printf("Delete filter: %s succesful\n", name);
		} else {
			fprintf(stderr, "Delete filter: %s failed\n",  name);
		}
	}
}

/*
 * Verify arguments.
 * When an illegal combination of arguments is used, this function generates an error.
 * On valid combination 0 is retuned, else -1 indicates error.
 */
static int verifyarguments(opts_struct *opts)
{
	int retval=0;

	/*
	 * source and filter 
	 */
	if(opts->source && opts->filter){
		fprintf(stderr, "Error, you can not use add filter and add source in the same command line.\n");
		retval=-1;
	}
	/*
	 * run and source
	 */
	if(opts->run && opts->source){
		fprintf(stderr, "Error, you can not use run and add source in the same command line.\n");
		retval=-1;
	}
	/*
	 * run and filter
	 */
	if(opts->run && opts->filter){
		fprintf(stderr, "Error, you can not use add filter and run in the same command line.\n");
		retval=-1;
	}
	/*
	 * run and simple
	 */
	if(opts->run && opts->simplename){
		fprintf(stderr, "Error, you can not use add simple and run in the same command line.\n");
		retval=-1;
	}
	/*
	 * sourcefilter no filter
	 */
	if(opts->sourcefilter && !(opts->source)){
		fprintf(stderr, "Error, you provided a filter type but no filter.\n");
		retval=-1;
	}
	/*
	 * doublefilter no filter
	 */
	if(opts->doublefilter && !(opts->filter)){
		fprintf(stderr, "Error, you provided a 'no double filter' but no filter.\n");
		retval=-1;
	}

	/*
	 * Simple filter and sql filter are not allowed in the same commandline
	 */
	if( opts->filter && opts->simplename ) {
		fprintf(stderr, "Error, you cannot add an sql and simple filter in one line.\n");
		retval=-1;
	}
	
	/*
	 * Simple filter arguments are not allowed without the add filter argument
	 */
	if( (!opts->simplename    &&
			 !opts->findtorid)    && 
			(opts->simpletitle    || 
			 opts->simpleexclude  || 
			 opts->simplecategory || 
			 opts->simplemaxsize  || 
			 opts->simpleminsize  ||
			 opts->simplenodup    ||
			 opts->simpleseason   ||
			 opts->simplesource   ||
			 opts->simpleepisode)) {
		fprintf(stderr, "Error, you can not use simple filter argument without adding a simple filter.\n");
		retval=-1;
	}

	/*
	 * When find is used without extra filters error
	 */
	if(opts->findtorid == 1 &&
			(!opts->simpletitle &&
			 !opts->simpleexclude &&
			 !opts->simplecategory &&
			 !opts->simplemaxsize &&
			 !opts->simpleminsize &&
			 !opts->simplenodup &&
			 !opts->simpleseason &&
			 !opts->simplesource &&
			 !opts->simpleepisode)) {
		fprintf(stderr, "Error, no filter options found, use simple options to specify filter.\n");
		retval=-1;
	}

	/*
	 * No dup is mandatory when adding a simple filter.
	 */
	if(opts->simplename && !opts->simplenodup) {
		fprintf(stderr, "Error, When entering a simple filter a noduplicate filter is mandatory.\n");
		retval=-1;
	}

	/*
	 * Nodetach and ontime should be used together with run
	 */
	if(!opts->run && (opts->nodetach || opts->onetime == once)) {
		fprintf(stderr, "Error, run options can only be used together with the run command.\n");
		retval=-1;
	}

	return retval;
}


static void reinitdb(rsstor_handle *handle)
{
	int rc=0;

	printf("Running reinitializion script on database...\n");
	rc = rsstrundbinitscript(handle);
	if(rc != 0) {
		fprintf(stderr, "Reinitializing database failed!\n");
	} else {
		printf("Reinitialization succesful!\n");
	}
}

/*
 * handle Commandline options, setting up the structure for the complex 
 * arguments.
 */
static void parsearguments(rsstor_handle *handle, int argc, char *argv[], opts_struct *opts)
{
	int						rc=0;
	char        	opt=0;
	int 					stopop=0;
	int 					optindex=0;
	int						torid=0;

  /*
   * Handle commandline options
   */
	opt = getopt_long (argc, argv, optString, optLong, &optindex);
  while( opt != -1 && stopop == 0) {
    switch( opt ) {
      case 'v':
				rsstprintversion();
        stopop = 1; // no more
        break;
      case 'c':
        rsstprintconfigitems(handle);
        break;
      case 'C': // Set config value
				setconfigvalue(handle, optarg);
        break;
      case 'f': // list filters
        rsstprintfilters(handle, argv[0]);
        stopop = 1; // no more
        break;
      case 'F': // set download filter
				rsstalloccopy(&(opts->filter), optarg, strlen(optarg));
        break;
      case 'p': // print filter en shell format
        rsstprintshellfilter(handle, argv[0], optarg);
				break;
      case 't': // set source filter type
        if( opts->sourcefilter != NULL) {
          fprintf(stderr, "Warning: ignoring second doublefilter parameter.\n");
          break;
        }
				rsstalloccopy(&(opts->sourcefilter), optarg, strlen(optarg));
				break;
      case 'q': // set filtertest flag
        opts->testfilt=1;
        break;
      case 'T': // set duplicate filter
        if( opts->doublefilter != NULL) {
          fprintf(stderr, "Warning: ignoring second sourcefilter parameter.\n");
          break;
        }
				rsstalloccopy(&(opts->doublefilter), optarg, strlen(optarg));
        break;
      case 'd': // delete filter
				optdeletefilter(handle, optarg);
        stopop = 1; // no more
        break;
      case 's': // set rss source
        if(opts->source!=NULL) {
          fprintf(stderr, "Warning: ignoring second source parameter.\n");
          break;
        }
				rsstalloccopy(&(opts->source), optarg, strlen(optarg));
        break;
      case 'S': // List available sources
        rsstprintsources(handle);
        stopop = 1; // no more
        break;
      case 'D': // delete rss source
        rc = rsstdelsource(handle, optarg);
        break;
      case 'r': // run as daemon 
        opts->run = 1;
        break;
      case 'R': // run once
        opts->onetime = once;
        break;
      case 'n': // no detach
        opts->nodetach = 1;
        break;
      case 'm': // Send test mail notification
        rssttestmail(handle, optarg);
        stopop = 1; // no more
        break;
      case 'J': // List simple filter
        rsstlistsimple(handle);
        stopop = 1; // no more
        break;
      case 'P': // Print A simple filter in shell format
        rsstprintsimple(handle, optarg);
        stopop =1; // no more
        break;
      case 'A': // Print A simple filter in shell format
        rsstprintallsimple(handle);
        stopop =1; // no more
        break;
      case 'j': // Del simple filter
        rc = rsstdelsimple(handle, optarg);
        if(rc == 0) {
          printf("Deletion of simple filters '%s' Successful.\n", optarg);
        }
        stopop =1; // no more
        break;
      case 'k': // Del all simple filters
        rc = rsstdelallsimple(handle);
        if(rc == 0) {
          printf("Deletion of all simple filters Successful.\n");
        }
        stopop =1; // no more
        break;
      case 'e': // Add simple filter
        if( opts->simplename != NULL) {
          fprintf(stderr, "Warning: ignoring second simple filter addition.\n");
          break;
        }
				rsstalloccopy(&(opts->simplename), optarg, strlen(optarg));
        break;
      case 'N': // Add simple exclude regexp
        if( opts->simpleexclude != NULL) {
          fprintf(stderr, "Warning: ignoring second exclude addition.\n");
          break;
        }
				rsstalloccopy(&(opts->simpleexclude), optarg, strlen(optarg));
        break;
      case 'U': // Add simple category regexp
        if( opts->simplecategory != NULL) {
          fprintf(stderr, "Warning: ignoring second category addition.\n");
          break;
        }
        rsstalloccopy(&(opts->simplecategory), optarg, strlen(optarg));
        break;
      case 'l': // Add simple category regexp
        if( opts->simplesource != NULL) {
          fprintf(stderr, "Warning: ignoring second source addition.\n");
          break;
        }
        rsstalloccopy(&(opts->simplesource), optarg, strlen(optarg));
        break;
      case 'E': // Add title-filter argument
        if( opts->simpletitle != NULL) {
          fprintf(stderr, "Warning: ignoring second simple filter addition.\n");
          break;
        }
        rsstalloccopy(&(opts->simpletitle), optarg, strlen(optarg));
        break;
      case 'O': // Add 'max size' argument
        if( opts->simplemaxsize != NULL) {
          fprintf(stderr, "Warning: ignoring second 'max size' argument.\n");
          break;
        }
        rsstalloccopy(&(opts->simplemaxsize), optarg, strlen(optarg));
        break;
      case 'o': // Add 'min size' argument
        if( opts->simpleminsize != NULL) {
          fprintf(stderr, "Warning: ignoring second 'min size' argument.\n");
          break;
        }
				rsstalloccopy(&(opts->simpleminsize), optarg, strlen(optarg));
        break;
      case 'u': // Add 'min size' argument
        if( opts->simplenodup != NULL) {
          fprintf(stderr, "Warning: ignoring second nodup argument.\n");
          break;
        }
				rsstalloccopy(&(opts->simplenodup), optarg, strlen(optarg));
        break;
      case 'g': // Add 'min size' argument
        if( opts->simpleseason != NULL) {
          fprintf(stderr, "Warning: ignoring second 'from season' argument.\n");
          break;
        }
				rsstalloccopy(&(opts->simpleseason), optarg, strlen(optarg));
        break;
      case 'G': // Add 'min size' argument
        if( opts->simpleepisode != NULL) {
          fprintf(stderr, "Warning: ignoring second 'from episode' argument.\n");
          break;
        }
				rsstalloccopy(&(opts->simpleepisode), optarg, strlen(optarg));
        break;
      case 'K': // Reinitialize the database
				reinitdb(handle);
        stopop =1; // no more
        break;
			case 'i': // Download by id
				rsstdownloadbyidstr(handle, optarg);
        stopop =1; // no more
				break;
			case 'M': // Delete a entry from the downed table
				if(strlen(optarg) > 0) {
					torid = atoi(optarg);
					rc = rsstdeldownloaded(handle, torid);
					if(rc == ROWS_CHANGED){
						printf("Deletion of %s succesfull.\n", optarg);
					} else {
						printf("Deletion of %s failed.\n", optarg);
					}
				}
				stopop =1; // no more
				break;
			case 'L': // find torrents, use simple filters to describe conditions
				opts->findtorid = 1;
				break;
			case 'a': // find downloaded by name
				rsstfinddowned(handle, optarg);
        stopop =1; // no more
				break;
      case 'h':   /* fall-through is intentional */
      case '?':
        /*
         * Print all available config items to the screen.
         * format varname : value
         * All from database
         */
        printhelp();
        stopop = 1; // no more
        break;
      default:
        /* You won't actually get here. */
        break;
    }

		opt = getopt_long (argc, argv, optString, optLong, &optindex);
  }
}

/*
 * Handle commands that consist of more then one arguments
 */
void handlemultiple(rsstor_handle *handle, opts_struct *opts)
{
	int						rc=0;
	char 				 *name=NULL;
	char 				 *value=NULL;

	/*
	 * When source is set add it here.
	 * Handle here because a type could be set
	 */
	if(opts->source != NULL) {
		rsstsplitnameval(opts->source, &name, &value);
		rsstaddsource(handle, name, value, opts->sourcefilter);
	}

	/*
	 * When a filter is set, evaluate it here
	 */
	if(opts->filter != NULL) {
		rsstsplitnameval(opts->filter, &name, &value);
		/*
		 * Add the filter 
		 */
		if(opts->testfilt == 0) { 
			rc = rsstaddfilter(handle, name, value, opts->doublefilter);
			if(rc == 0) {
				printf("new filter : %s\n"
						"filter : %s\n"
						"nodouble filter : %s\n"
						"Was added succesfully\n",
						name, value, opts->doublefilter);
      }
    } 

		/*
		 * Test the filter
		 */
    if(opts->testfilt != 0) { 
      rc =  rsstdofiltertest(value, opts->doublefilter);
      if(rc != 0) {
        printf("new filter : %s\n"
            "filter : %s\n"
            "nodouble filter : %s\n"
            "Test failed\n",
            name, value, opts->doublefilter);
      }
    }
  }

	/*
	 * Find Torrent ids
	 */
	if(opts->findtorid != 0){
		rc = rsstfindtorrentids(opts);
	}

  /*
   * When add simple filter is set
   * Call routine here.
   */
	if(opts->simplename != NULL) {
		/*
		 * Add the simple filter 
		 */
		if(opts->testfilt == 0 && opts->findtorid == 0) { 
			rc = rsstaddsimplefilter(handle, opts);
			if(rc != 0){
				fprintf(stderr, "Adding filter failed.\n");
			} else {
				printf("Filter '%s' added succesfully.\n", 
						opts->simplename);
			}
		}

		/*
		 * Test simple filter
		 */
		if(opts->testfilt != 0) { 
			rc = rsstdosimpletest(opts);
			if(rc != 0) {
				printf("new filter : '%s' Test failed\n",
						opts->simplename);
			}
		}
	}

	/*
	 * Cleanup
	 */
	free(name);
	free(value);
}

/*
 * Frees all allocated data from the opts struct.
 * The struct itself is not freed, you have to do that yourself.
 */
static void freeopts(opts_struct *opts)
{
	/*
	 * Free all strings.
	 */
	free(opts->filter);
	free(opts->sourcefilter);
	free(opts->doublefilter);
	free(opts->source);
	free(opts->simplename);
	free(opts->simpletitle);
	free(opts->simplemaxsize);
	free(opts->simpleminsize);	
	free(opts->simplenodup);	
	free(opts->simpleseason);		
	free(opts->simpleepisode);	
	free(opts->simplesource);	

	/*
	 * NULL the whole struct
	 */ 
	memset(opts, 0, sizeof(opts_struct));
}

/*
 * Choose runmode.
 */
static void runmode(rsstor_handle *handle, opts_struct *opts)
{
	int 		rc=0;
	char		*logpath=NULL;
	char		*logfullpath=NULL;

	/*
	 * Test if torrent directory is writable
	 */
	rc = rssttesttorrentdir(handle);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Torrent directory not usable exiting.");
		fprintf(stderr, "Torrent directory is not usable, please look in log to find out why!\n");
	} else {
		/*
		 * Fork to background 
		 * when nodetach is 0
		 * when not running once
		 */
		if(opts->nodetach == 0 && opts->onetime == (LOOPMODE) loop) {
			printf("Forking to background.\n");
			rc = rsstconfiggetproperty(handle, CONF_LOGFILE, &logpath);
			if(rc == 0) {
				rsstcompletepath(logpath, &logfullpath);
				rsstdaemonize(logpath);
			} else {
				rsstdaemonize("/dev/null");
			}
		} else {
			printf("No forking, running on shell.\n");
		}

		/*
		 * Check and lock lockfile
		 */
		rsstlock(handle);

		/*
		 * Free paths
		 */
		free(logpath);
		free(logfullpath);

		/*
		 * Call main loop here.
		 */
		rsstrunloop(handle, opts->onetime);
	}
}

/*
 * Handles the arguments, and Calls the subroutines when needed.
 */
void rssthandleopts(rsstor_handle *handle, int argc, char *argv[])
{
	int 					rc=0;
	opts_struct 	opts; 

	/*
	 * init opts struct
	 */
	memset(&opts, 0, sizeof(opts_struct));

	/*
	 * When no arguments are given.
	 */
	if(argc == 1) {
		printhelp();
	}

	/*
	 * Parse all options, calling simple actions directly.
	 * Fill the opts truct to handle complex arguments later.
	 */
	parsearguments(handle, argc, argv, &opts);

	/*
	 * Test the argumentcombinations.
	 * only execute handling routines when all is okay.
	 */
	rc = verifyarguments(&opts);
	if(rc == 0) {
		/*
		 * Handle commands, that consist of more then one arguments.
		 */
		handlemultiple(handle, &opts);

		/*
		 * When then run option is provided call the main loop
		 */
		if(opts.run == 1) {
			runmode(handle, &opts);
		}
	}

	/*
	 * Free allocated strings
	 */
	freeopts(&opts);
}
