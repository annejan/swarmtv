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
#include <unistd.h>
#include <string.h>
#include <pcre.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <libxml/parser.h>
#include <signal.h>

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
#include "testfilter.h"
#include "torrentdownload.h"

/*
 * Program version
 */
#define PROGVERSION "0.4"

/*
 * Default timeout when no -i is provided
 */
#define  DEFAULTSEC 900

/*
 * Number of arguments retrieved from regexp.
 */
#define OVECCOUNT 3

/*
 * optstring what options do we allow
 */
static const char *optString = "vcC:hfF:T:t:d:s:SD:rnm:p:qR";

/*
 * Bit of a hack.. but needed for cleanup
 */
static sqlite3 *cleandb;

/*
 * options for after the getopt loop.
 */
typedef struct {
  int loopsec;
  int run;
  int nodetach;
  int testfilt;
  int onetime;
} opts_struct;

/*
 * This routine prints the help-options
 */
void printhelp(void) 
{
  printf( "################\n"
          "-v               : Print version.\n"
          "-c               : List config Items and there values.\n"
          "-f               : List filters.\n"
          "-S               : List RSS sources.\n"
          "-p <name>        : Print filter in shell format.\n"
          "-C <name:value>  : Set a config value.\n"
          "-s <name:value>  : Set RSS source (default filter type 'eztv') .\n"
          "-t <value>       : Set RSS source filter type. (use with -s)\n"
          "-F <name:url>    : Set download filter (empty default duplicate filter).\n"
          "-T <value>       : Set duplicate filter. (use with -F) \n"
          "-d <name>        : Delete filter, name 'all' to delete all filters.\n"
          "-D <name>        : Delete RSS source.\n"
          "-r               : Run in daemon mode.\n"
          "-R               : Run once, then exit.\n"
          "-q               : Test filter (together with -F & -T).\n"
          "-n               : Don't detach from console.\n"
          "-m <text>        : Send testmail notification.\n"
          "-h               : Print this help.\n"
          "\n"
          "Flags to be used together.\n"
          "-F <name:value> -T <value>     : Set download filter and diplicate check\n"
          "-q -F <name:value> -T <value>  : Test download filter and diplicate check\n"
          "-s <name:url> -t <value>       : set RSS source and filter\n"
          "-r -R                          : run once then exit for use in crontab\n"
          "################\n\n");
}


/*
 * Clean up 
 */
void cleanup(){
  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();

  /* 
   * we're done with libcurl, so clean it up
   */ 
  curl_global_cleanup();

  /*
   * Close the sqlite database.
   * No way to get dbpointer here
   */
  sqlite3_close(cleandb);

  /*
   * Close logfile.
   */
  closelog();
}


/*
 * Handle standard signals
 */
void Signal_Handler(int sig) /* signal handler function */
{
  switch(sig){
    case SIGHUP:
    case SIGPIPE:
      /* Do nothing */
      break;    
    case SIGINT:
    case SIGTERM:
      /* finalize the server */
      printf("Cleaning up\n");
      cleanup();
      exit(0);
      break;    
  } 
}


/*
 * Temporary main function.
 */
int main(int argc, char **argv){
  sqlite3    *db;
  int         rc;
  char        opt;
  int         stopop = 0;
  char       *name;
  char       *value;
  char       *source=NULL;
  char       *filtertype=NULL;
  char       *doublefilter=NULL;
  char       *filter=NULL;
  char       *lockpath=NULL;

  signal(SIGHUP,Signal_Handler); /* hangup signal */
  signal(SIGTERM,Signal_Handler); /* software termination signal from kill */
  signal(SIGINT,Signal_Handler); /* software termination signal CTRL+C */
  signal(SIGPIPE,Signal_Handler); /* software termination signal CTRL+C */

  /*
   * Initialize the database
   */
  rc = initdatabase( DBFILE, &db);  
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "Initializing db : \'%s\' failed", argv[1]);
    exit(1);
  }
  cleandb=db;

  /*
   * Initialize lib curl
   */
  curl_global_init(CURL_GLOBAL_ALL);

  /*
   * open logfile
   */
  rc = initlogdb(db);
  if(rc != 0) {
    fprintf(stderr, "Can't open logfile!\n");
    exit(1);
  }
  writelog(LOG_NORMAL, "Start rss-torrent");

  /*
   * init opts struct
   */
  opts_struct opts; 
  memset(&opts, 0, sizeof(opts_struct));
  opts.loopsec=DEFAULTSEC;  // set timeout to default 

  /*
   * When no arguments are given.
   */
  if(argc == 1) {
    printhelp();
  }

  /*
   * Handle commandline options
   */
  opt = getopt( argc, argv, optString );
  while( opt != -1 && stopop == 0) {
    switch( opt ) {
      case 'v':
        printf("RSSTorrent by Paul Honig 2009-2010\n");
        printf("Version %s \n", PROGVERSION);
        break;
      case 'c':
        printconfigitems(db);
        break;
      case 'C': // Set config value
        splitnameval(optarg, &name, &value);
        rc = setconfigitem(db, name, value);
        if(rc == -1) {
          fprintf(stderr, "Value not found in config\n");
        } else {
          printf("config value: %s, set to: %s\n", name, value);
        }

        free(name);
        free(value);
        break;
      case 'f': // list filters
        printfilters(db);

        stopop = 1; // no more
        break;
      case 'F': // set download filter
        filter=calloc(1, strlen(optarg)+1);
        strcpy(filter, optarg);
        break;
      case 'p': // print filter en shell format
        printshellfilter(db, argv[0], optarg);
      case 't': // set source filter type
        if( filtertype != NULL) {
          fprintf(stderr, "Warning: ignoring second doublefilter parameter.\n");
          break;
        }
        filtertype=calloc(1, strlen(optarg)+1);
        strcpy(filtertype, optarg);
      case 'q': // set filtertest flag
        opts.testfilt=1;
        break;
      case 'T': // set duplicate filter
        if( doublefilter != NULL) {
          fprintf(stderr, "Warning: ignoring second filtertype parameter.\n");
          break;
        }
        doublefilter=calloc(1, strlen(optarg)+1);
        strcpy(doublefilter, optarg);

        break;
      case 'd': // delete filter
        if(!strcmp(optarg, "all")) {
          rc = delallfilters(db);
          if(rc == 0) {
            printf("Delete all filters succesfull\n");
          } else {
            fprintf(stderr, "Deletion of all filters failed\n");
          }
        } else {
          rc = delfilter(db, optarg);
          if(rc == 0) {
            printf("Delete filter: %s succesfull\n", optarg);
          } else {
            fprintf(stderr, "Delete filter: %s failed\n", optarg);
          }
        }
        break;
      case 's': // set rss source
        if(source!=NULL) {
          fprintf(stderr, "Warning: ignoring second source parameter.\n");
          break;
        }
        source=calloc(1, strlen(optarg)+1);
        strcpy(source, optarg);
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
        opts.run = 1;
        break;
      case 'R': // run as daemon 
        opts.onetime = 1;
        break;
      case 'n': // no detach
        opts.nodetach = 1;
        break;
      case 'm': // Send test mail notification
        sendrssmail(db, optarg, optarg);
        break;
      case 'h':   /* fall-through is intentional */
      case '?':
        /*
         * Print all available config items to the screen.
         * format varname : value
         * All from database
         */
        printhelp();
        stopop = 1; // do nothing after printing help
        break;
      default:
        /* You won't actually get here. */
        break;
    }

    opt = getopt( argc, argv, optString );
  }

  /*
   * When source is set add it here.
   * Handle here because a type could be set
   */
  if(source != NULL) {
    splitnameval(source, &name, &value);
    addsource(db, name, value, filtertype);
  }

  if(filter != NULL) {
    splitnameval(filter, &name, &value);
    if(opts.testfilt == 0) { // add filter
      rc = addfilter(db, name, value, doublefilter);
      if(rc == 0) {
        printf("new filter : %s\n"
            "filter : %s\n"
            "nodouble filter : %s\n"
            "Was added succesfully\n",
            name, value, doublefilter);
      }

    } 
    else { // test the filters
      rc =  dofiltertest(value, doublefilter);
      if(rc != 0) {
        printf("new filter : %s\n"
            "filter : %s\n"
            "nodouble filter : %s\n"
            "Test failed\n",
            name, value, doublefilter);
      }
    }

    free(filter);
    free(name);
    free(value);
  }

  /*
   * When then run option is provided call the main loop
   */
  if(opts.run == 1) {
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
      if(opts.nodetach == 0 && opts.onetime == 0) {
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
      runloop(db, opts.onetime);
    }
  }


  /*
   * Free allocated strings
   */
  free(source);
  free(filtertype);
  free(doublefilter);

  /*
   * Cleanup the rest of the libaries
   */
  cleanup();

  return 0;
}

