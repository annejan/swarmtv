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
#include <sqlite3.h>

#include "database.h"
#include "config.h"
#include "regexp.h"
#include "source.h"
#include "filter.h"
#include "runloop.h"
#include "logfile.h"
#include "daemonize.h"
#include "mailmsg.h"
#include "sandboxdb.h"
#include "torrentdownload.h"
#include "handleopts.h"
#include "testfilter.h"
#include "simplefilter.h"

/*
 * optstring what options do we allow
 */
static const char *optString = "vcC:hfF:T:t:d:s:SD:rnm:p:qRe:o:O:u:g:G:Jj:P:";

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
	{"filter-shell",  				required_argument, 0, 'p'},
	{"set-config",  					required_argument, 0, 'C'},
	{"add-source",   					required_argument, 0, 's'},
	{"source-filter",   			required_argument, 0, 't'},
	{"add-filter",   					required_argument, 0, 'F'},
	{"duplicate-filter", 			required_argument, 0, 'T'},
	{"delete-filter", 				required_argument, 0, 'd'},
	{"delete-source", 				required_argument, 0, 'D'},
	{"run", 									no_argument, 			 0, 'r'},
	{"once", 									no_argument, 			 0, 'R'},
	{"test", 									no_argument, 			 0, 'q'},
	{"nodetach", 							no_argument, 			 0, 'n'},
	{"test-mail", 						required_argument, 0, 'm'},
	{"help", 									no_argument, 			 0, 'h'},
  {"list-simple", 				  no_argument,       0, 'J'},
	{"print-simple", 					required_argument, 0, 'P'},
	{"del-simple", 						required_argument, 0, 'j'},
	{"add-simple", 						required_argument, 0, 'e'},
	{"title",									required_argument, 0, 'E'},
	{"max-size", 							required_argument, 0, 'O'},
	{"min-size", 							required_argument, 0, 'o'},
	{"nodup", 							  required_argument, 0, 'u'},
	{"from-season", 				  required_argument, 0, 'g'},
	{"from-episode", 				  required_argument, 0, 'G'},
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
          "\nConfig settings\n"
          "list-config      -c               : List config Items and their values.\n"  
          "set-config       -C <name:value>  : Set a config value.\n"  
          "\nSource\n"
          "add-source       -s <name:value>  : Set RSS source (default filter type 'defaultrss') .\n"  
          "delete-source    -D <name>        : Delete RSS source.\n"  
          "list-sources     -S               : List RSS sources.\n"  
          "source-filter    -t <value>       : Set RSS source filter type. (use with -s)\n"  
          "\nDownload filters\n"
          "delete-filter    -d <name>        : Delete filter, name 'all' to delete all filters.\n"  
          "list-filters     -f               : List filters.\n"  
          "filter-shell     -p <name>        : Print filter in shell format.\n"  
          "\nSQL Download filters\n"
          "add-sql-filter   -F <name:url>    : Set download filter (empty default duplicate filter).\n"  
          "nodup-sql-filter -T <value>       : Set no duplicate filter. (use with -F) \n"  
          "\nSimple Download filters\n"
          "list-simple      -J               : List the simple filters\n"
          "print-simple     -P <value>       : Print simple filter\n"
          "del-simple       -j <value>       : Delete a simple download filter\n"
          "add-simple       -e <value>       : Add a simple download filter\n"
          "title            -E <value>       : Title expression\n"
          "max-size         -O <value>       : Maximal size of downloaded torrent\n"
          "min-size         -o <value>       : Minimal size of downloaded torrent\n"
          "nodup            -u <value>       : No duplicate filter type (unique, newer, link, none)\n"
          "from-season      -g <value>       : Season number to start downloading from.\n"
          "from-episode     -G <value>       : Episode number to start downloading from.\n"               
          "\n"
          "Flags to be used together.\n"  
          "-F <name:value> -T <value>        : Set download filter and diplicate check\n"  
          "-q -F <name:value> -T <value>     : Test download filter and diplicate check\n"  
          "-s <name:url> -t <value>          : set RSS source and filter\n"  
          "-r -R                             : run once then exit for use in crontab\n"  
          "\nThe long arguments need to be preceded with '--'\n"  
          "Example: rsstorrent --help\n"  
          "################\n\n");  
}

static void printversion(void)
{
	printf("RSSTorrent by Paul Honig 2009-2010\n");
	printf("Version %s \n", PROGVERSION);
}

static void setconfigvalue(sqlite3 *db, char *configval)
{
	int		rc;
	char 	*name;
	char	*value;

	splitnameval(configval, &name, &value);
	rc = setconfigitem(db, name, value);
	if(rc == -1) {
		fprintf(stderr, "Value not found in config\n");
	} else {
		printf("config value: %s, set to: %s\n", name, value);
	}

	free(name);
	free(value);
}

static void optdeletefilter(sqlite3 *db, char *name)
{
	int rc;

	/*
	 * Name "all" deletes all filters.
	 */
	if(!strcmp(name, "all")) {
		rc = delallfilters(db);
		if(rc == 0) {
			printf("Delete all filters succesfull\n");
		} else {
			fprintf(stderr, "Deletion of all filters failed\n");
		}
	} else {
		rc = delfilter(db, name);
		if(rc == 0) {
			printf("Delete filter: %s succesfull\n", name);
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
	 * sourcefilter no filter
	 */
	if(opts->sourcefilter && !(opts->filter)){
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
	if( !opts->simplename && 
			(opts->simpletitle || 
			 opts->simplemaxsize || 
			 opts->simpleminsize ||
			 opts->simplenodup ||
			 opts->simpleseason ||
			 opts->simpleepisode)) {
		fprintf(stderr, "Error, you can not use simple filter argument without adding a simple filter.\n");
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
	if(!opts->run && (opts->nodetach || opts->onetime)) {
		fprintf(stderr, "Error, run options can only be used together with the run command.\n");
		retval=-1;
	}

	return retval;
}

/*
 * handle Commandline options, setting up the structure for the complex 
 * arguments.
 */
static void parsearguments(sqlite3 *db, int argc, char *argv[], opts_struct *opts)
{
	int					rc=0;
	char        opt=0;
	int 				stopop=0;
	int 				optindex=0;
	char       	*name=NULL;

  /*
   * Handle commandline options
   */
	opt = getopt_long (argc, argv, optString, optLong, &optindex);
  while( opt != -1 && stopop == 0) {
    switch( opt ) {
      case 'v':
				printversion();
        stopop = 1; // no more
        break;
      case 'c':
        printconfigitems(db);
        break;
      case 'C': // Set config value
				setconfigvalue(db, optarg);
        break;
      case 'f': // list filters
        printfilters(db);
        stopop = 1; // no more
        break;
      case 'F': // set download filter
				alloccopy(&(opts->filter), optarg, strlen(optarg));
        break;
      case 'p': // print filter en shell format
        printshellfilter(db, argv[0], optarg);
				break;
      case 't': // set source filter type
        if( opts->sourcefilter != NULL) {
          fprintf(stderr, "Warning: ignoring second doublefilter parameter.\n");
          break;
        }
				alloccopy(&(opts->sourcefilter), optarg, strlen(optarg));
				break;
      case 'q': // set filtertest flag
        opts->testfilt=1;
        break;
      case 'T': // set duplicate filter
        if( opts->doublefilter != NULL) {
          fprintf(stderr, "Warning: ignoring second sourcefilter parameter.\n");
          break;
        }
				alloccopy(&(opts->doublefilter), optarg, strlen(optarg));
        break;
      case 'd': // delete filter
				optdeletefilter(db, name);
        break;
      case 's': // set rss source
        if(opts->source!=NULL) {
          fprintf(stderr, "Warning: ignoring second source parameter.\n");
          break;
        }
				alloccopy(&(opts->source), optarg, strlen(optarg));
        break;
      case 'S': // List available sources
        printsources(db);
        stopop = 1; // no more
        break;
      case 'D': // delete rss source
        rc = delsource(db, optarg);
        if(rc == 0) {
          printf("Delete source: %s succesfull\n", optarg);
        } 
        break;
      case 'r': // run as daemon 
        opts->run = 1;
        break;
      case 'R': // run as daemon 
        opts->onetime = 1;
        break;
      case 'n': // no detach
        opts->nodetach = 1;
        break;
      case 'm': // Send test mail notification
        sendrssmail(db, optarg, optarg);
        stopop = 1; // no more
        break;
      case 'J': // List simple filter
        listsimple(db);
        stopop = 1; // no more
        break;
      case 'P': // Print A simple filter in shell format
        printsimple(db, optarg);
        stopop =1; // no more
        break;
      case 'j': // Del simple filter
        rc = delsimple(db, optarg);
        if(rc == 0) {
          printf("Deletion of simple filters '%s' Successfull.\n", optarg);
        }
        stopop =1; // no more
        break;
      case 'e': // Add simple filter
        if( opts->simplename != NULL) {
          fprintf(stderr, "Warning: ignoring second simple filter addition.\n");
          break;
        }
				alloccopy(&(opts->simplename), optarg, strlen(optarg));
        break;
      case 'E': // Add title-filter argument
        if( opts->simpletitle != NULL) {
          fprintf(stderr, "Warning: ignoring second simple filter addition.\n");
          break;
        }
				alloccopy(&(opts->simpletitle), optarg, strlen(optarg));
        break;
      case 'O': // Add 'max size' argument
        if( opts->simplemaxsize != NULL) {
          fprintf(stderr, "Warning: ignoring second 'max size' argument.\n");
          break;
        }
				alloccopy(&(opts->simplemaxsize), optarg, strlen(optarg));
        break;
      case 'o': // Add 'min size' argument
        if( opts->simpleminsize != NULL) {
          fprintf(stderr, "Warning: ignoring second 'min size' argument.\n");
          break;
        }
				alloccopy(&(opts->simpleminsize), optarg, strlen(optarg));
        break;
      case 'u': // Add 'min size' argument
        if( opts->simplenodup != NULL) {
          fprintf(stderr, "Warning: ignoring second nodup argument.\n");
          break;
        }
				alloccopy(&(opts->simplenodup), optarg, strlen(optarg));
        break;
      case 'g': // Add 'min size' argument
        if( opts->simpleseason != NULL) {
          fprintf(stderr, "Warning: ignoring second 'from season' argument.\n");
          break;
        }
				alloccopy(&(opts->simpleseason), optarg, strlen(optarg));
        break;
      case 'G': // Add 'min size' argument
        if( opts->simpleepisode != NULL) {
          fprintf(stderr, "Warning: ignoring second 'from episode' argument.\n");
          break;
        }
				alloccopy(&(opts->simpleepisode), optarg, strlen(optarg));
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
void handlemultiple(sqlite3 *db, opts_struct *opts)
{
	int		rc;
	char *name=NULL;
	char *value=NULL;

	/*
	 * When source is set add it here.
	 * Handle here because a type could be set
	 */
	if(opts->source != NULL) {
		splitnameval(opts->source, &name, &value);
		addsource(db, name, value, opts->sourcefilter);
	}

	/*
	 * When a filter is set, evaluate it here
	 */
	if(opts->filter != NULL) {
		splitnameval(opts->filter, &name, &value);
		/*
		 * Add the filter 
		 */
		if(opts->testfilt == 0) { 
			rc = addfilter(db, name, value, opts->doublefilter);
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
      rc =  dofiltertest(value, opts->doublefilter);
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
   * When add simple filter is set
   * Call routine here.
   */
	if(opts->simplename != NULL) {
		/*
		 * Add the simple filter 
		 */
		if(opts->testfilt == 0) { 
			rc = addsimplefilter(db, opts);
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
			rc = dosimpletest(opts);
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

	/*
	 * NULL the whole struct
	 */ 
	memset(opts, 0, sizeof(opts_struct));
}

/*
 * Choose runmode.
 */
static void runmode(sqlite3 *db, opts_struct *opts)
{
	int 		rc=0;
	char    *lockpath=NULL;

	/*
	 * Test if torrent directory is writable
	 */
	rc = testtorrentdir(db);
	if(rc != 0) {
		writelog(LOG_ERROR, "Torrent directory not usable exiting.");
		fprintf(stderr, "Torrent directory is not usable, please look in log to find out why!\n");
	} else {
		/*
		 * Fork to background 
		 * when nodetach is 0
		 * when not running once
		 */
		if(opts->nodetach == 0 && opts->onetime == 0) {
			printf("Forking to background.\n");
			daemonize();
		} else {
			printf("No forking, running on shell.\n");
		}

		/*
		 * Check and lock lockfile
		 */
		configgetproperty(db, CONF_LOCKFILE, &lockpath);
		lockfile(lockpath);
		free(lockpath);

		/*
		 * Call main loop here.
		 */
		runloop(db, opts->onetime);
	}
}

/*
 * Handles the arguments, and Calls the subroutines when needed.
 */
void handleopts(sqlite3 *db, int argc, char *argv[])
{
	int 				rc=0;
	opts_struct opts; 

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
	parsearguments(db, argc, argv, &opts);

	/*
	 * Test the argumentcombinations.
	 * only execute handling routines when all is okay.
	 */
	rc = verifyarguments(&opts);
	if(rc == 0) {
		/*
		 * Handle commands, that consist of more then one arguments.
		 */
		handlemultiple(db, &opts);

		/*
		 * When then run option is provided call the main loop
		 */
		if(opts.run == 1) {
			runmode(db, &opts);
		}
	}

	/*
	 * Free allocated strings
	 */
	freeopts(&opts);
}
