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
#include <time.h>

#include <swarm.h>

#include "handleopts.h"
#include "frontfuncts.h"

/*
 * Program version
 */
#define PROGVERSION "0.8"

/*
 * Buffer size used in present routines.
 */
#define   BUFSIZE 20

/*
 * Default filter name and nodup filter that will be added in findtorrent routine
 */
#define  FINDNAME 	"filter"
#define  FINDNODUP	"none"

/*
 * Print version 
 */
void rssfprintversion(void)
{
	printf("RSS-Torrent by Paul Honig 2009-2010\n");
	printf("Version %s \n", PROGVERSION);
}

/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 * @Arguments
 * handle RSS-torrent handle
 */
void rssfprintconfigitems(rsstor_handle *handle) 
{
	int 							rc=0;
	int								count=0;
	config_container *container=NULL;

  /*
   * header
   */
  printf("#############\n");
  printf("Available config items.\n");
  printf("#############\n");
	
	/*
	 * Get config values.
	 */
	rc = rsstgetallconfig(handle, &container);
	if(rc != 0){
		fprintf(stderr, "Retrieving of config failed !\n");
		exit(1);
	}	

	/*
	 * Print config values.
	 */
	for(count=0; count < container->nr; count++) {
		printf("%-25s : %-25s : %s\n", 
				container->config[count].name,
				container->config[count].value,
				container->config[count].description);
	}

	/*
	 * Free the results
	 */
	rc = rsstfreeconfigcontainer(container);
	if(rc != 0) {
		fprintf(stderr, "Freeing of the container failed !\n");
		exit(1);
	}

  /*
   * Footer
   */
  printf("\n#############\n");
}


/*
 * List all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssflistfilters(rsstor_handle *handle)
{
	filter_container *container=NULL;
	int								count=0;
	int								rc=0;


	/*
	 * Get container with SQL filters.
	 */
	rc = rsstgetallfilter(handle, &container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Get all filters failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Loop through the SQLs.
	 */
	for(count=0; count < container->nr; count++)
	{
		/*
		 * Print the SQL filters.
		 * int   id;       // Id of the filter
		 * char *name;     // Name of the filter
		 * char *filter;   // SQL of the filter
		 * char *nodup;    // SQL of the avoiding duplicates filter
		 */
		printf ( "Filter name:'%s'\n", 
				container->filter[count].name); 
	}

	/*
	 * Free container
	 */
	rc = rsstfreefiltercontainer(container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Freeing filter container failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * All done
	 */
	return;
}


/*
 * Print all available config items to the screen.
 * format varname : value
 * All from database
 */
void rssfprintfilters(rsstor_handle *handle, char *appname) 
{
	filter_container *container=NULL;
	int								count=0;
	int								rc=0;


	/*
	 * Get container with SQL filters.
	 */
	rc = rsstgetallfilter(handle, &container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Get all filters failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Loop through the SQLs.
	 */
	for(count=0; count < container->nr; count++)
	{
		/*
		 * Print the SQL filters.
		 * int   id;       // Id of the filter
		 * char *name;     // Name of the filter
		 * char *filter;   // SQL of the filter
		 * char *nodup;    // SQL of the avoiding duplicates filter
		 */
		printf ( "# %s \\\n -F \'%s:%s\' \\\n -T \'%s\'\n", 
				appname,
				container->filter[count].name,  
				container->filter[count].filter, 
				container->filter[count].nodup);
	}

	/*
	 * Free container
	 */
	rc = rsstfreefiltercontainer(container);
	if(rc != 0) {
		rsstwritelog(LOG_ERROR, "Freeing filter container failed! %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * All done
	 */
	return;
}


/*
 * Print filter in a way it could be modified and reentered
 * @Arguments 
 * handle RSS-torrent handle
 * appname		The name of the executable
 * filtername	The name of the filter to print. 	
 */
int rssfprintshellfilter(rsstor_handle *handle, char *appname, char *filtername)
{
  int           		rc=0;
	filter_container 	*container=NULL;
	int								retval=0;

	/*
	 * Retrieve container
	 */
	rc = rsstgetfilterbyname(handle, filtername, &container);
	if(rc != 0){
		retval=-1;
	}

	if(retval == 0) {
		/*
		 * Print the filter
		 */
		printf ( "# %s \\\n -F \'%s:%s\' \\\n -T \'%s\'\n", 
				appname,
				container->filter[0].name,  
				container->filter[0].filter, 
				container->filter[0].nodup);

		/*
		 * Free container
		 */
		rc = rsstfreefiltercontainer(container);
		if(rc != 0) {
			rsstwritelog(LOG_ERROR, "Freeing filter container failed! %s:%d", __FILE__, __LINE__);
			retval = -1;
		}
	}

	/*
	 * All done
	 */
	return retval;
}

/*
 * Print all available source items to the screen.
 * format varname : value
 * All from database
 */
void rssfprintsources(rsstor_handle *handle) 
{
	int rc=0;
	int count=0;
	source_container *container=NULL;

	/*
	 * header
	 */
	printf("#############\n");
  printf("Sources\n");
  printf("Name : Parser : Meta type : URL\n");
  printf("#############\n");

	/*
	 * Get source values.
	 */
	rc = rsstgetallsources(handle, &container);
	if(rc != 0){
		fprintf(stderr, "Retrieving of source failed !\n");
		exit(1);
	}	

	/*
	 * Print source values.
	 */
	for(count=0; count < container->nr; count++) {
		printf("%-25s : %-15s : %-15s : %s\n", 
				container->source[count].name,
				container->source[count].parser,
        container->source[count].metatype,
				container->source[count].url);
	}

	/*
	 * Free the results
	 */
	rc = rsstfreesourcecontainer(container);
	if(rc != 0) {
		fprintf(stderr, "Freeing of the container failed !\n");
		exit(1);
	}

  /*
   * Footer
   */
  printf("\n#############\n");
}

/*
 * Print a simple filter struct in shell format
 * @Arguments
 *
 * @return
 */
void printsimplestruct(char *execname, simplefilter_struct *simple)
{
  unsigned char 			maxsizestring[BUFSIZE+1];
  unsigned char 			minsizestring[BUFSIZE+1];

  /*
   * Print the components that are set
   */
  printf("# %s --add-simple='%s' --nodup='%s' ", execname, simple->name, simple->nodup); 

  if(strlen((char*)simple->title) > 0) {
    printf("--title='%s' ", simple->title);
  }

  /*
   * Exclude
   */
  if(strlen((char*)simple->exclude) != 0){
    printf("--exclude='%s' ", simple->exclude);
  }

  /*
   * Category
   */
  if(strlen((char*)simple->category) != 0){
    printf("--category='%s' ", simple->category);
  }

  /*
   * Category
   */
  if(strlen((char*)simple->source) != 0){
    printf("--source='%s' ", simple->source);
  }

  /*
   * Max size
   */
  if(simple->maxsize != 0){
    rssfsizetohuman(simple->maxsize, (char*) maxsizestring);

    printf("--max-size='%s' ", maxsizestring);
  }

  /*
   * Min size
   */
  if(simple->minsize != 0){
    rssfsizetohuman(simple->minsize, (char*) minsizestring);

    printf("--min-size='%s' ", minsizestring);
  }

  /*
   * From season
   */
  if(simple->fromseason != 0) {
    printf("--from-season='%d' ", simple->fromseason);
  }

  /*
   * From episode
   */
  if(simple->fromepisode != 0) {
    printf("--from-episode='%d' ", simple->fromepisode);
  }

  /*
   * Close print row
   */
  putchar('\n');
}

/*
 * Print filter in shell format
 * Prints the names of the simple filters + a header.
 */
void rssfprintsimple(rsstor_handle *handle, char *filtername, char *execname)
{
  int           			rc=0;
	simplefilter_container *simplefilter=NULL;

	/*
	 * Get simple container limit of -1 gets all records.
	 */
	rc = rsstgetsimplefiltername(handle, &simplefilter, filtername);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstgetsimplefilter failed %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Print result
	 */
	printsimplestruct(execname, simplefilter->simplefilter);

	/*
	 * Free container
	 */
	rc = rsstfreesimplefiltercontainer(simplefilter);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstfreesimplefiltercontainer failed %s:%d", __FILE__, __LINE__);
		return;
	}
}


/*
 * Print all simple filters in shell format.
 */
void rssfprintallsimple(rsstor_handle *handle, char *execname)
{
  int           					rc=0;
	int											count=0;
	simplefilter_container *simplefilter=NULL;

	/*
	 * Get simple container limit of -1 gets all records.
	 */
	rc = rsstgetallsimplefilter(handle, &simplefilter, -1, 0);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstgetsimplefilter failed %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Print result
	 */
	for(count=0; count < simplefilter->nr; count++) {
		printsimplestruct(execname, (simplefilter->simplefilter)+count);
	}

	/*
	 * Free container
	 */
	rc = rsstfreesimplefiltercontainer(simplefilter);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstfreesimplefiltercontainer failed %s:%d", __FILE__, __LINE__);
		return;
	}
}


/*
 * Print all simple filters in shell format.
 * @Arguments
 * handle RSS-torrent handle
 */
void rssflistallsimple(rsstor_handle *handle)
{
  int           					rc=0;
	int											count=0;
	simplefilter_container *simplefilter=NULL;

	/*
	 * Get simple container limit of -1 gets all records.
	 */
	rc = rsstgetallsimplefilter(handle, &simplefilter, -1, 0);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstgetsimplefilter failed %s:%d", __FILE__, __LINE__);
		return;
	}

	/*
	 * Print result
	 */
	for(count=0; count < simplefilter->nr; count++) {
		//printf("(%d) %s\n", count, simplefilter->simplefilter[count].name);
		printf("(%d) %s\n", simplefilter->simplefilter[count].id, simplefilter->simplefilter[count].name);
	}

	/*
	 * Free container
	 */
	rc = rsstfreesimplefiltercontainer(simplefilter);
	if(rc != 0){
    rsstwritelog(LOG_ERROR, "rsstfreesimplefiltercontainer failed %s:%d", __FILE__, __LINE__);
		return;
	}
}


/*
 * Do filter test
 * show first 10 matches
 * Takes opts_struct * as argument.
 * return 0 on success, return -1 on failure.
 */
int rssffindtorrentids(opts_struct *opts)
{
  int rc=0;
	int retval=0;
	int count=0;
	char humansize[20];
	newtorrents_container *newtorrents=NULL;
	simplefilter_struct filter;

	/*
	 * Clear filter struct
	 */
	memset(&filter, 0, sizeof(simplefilter_struct));

	/*
	 * Make sure the mandatory fields are inserted.
	 */
	if(opts->simplename == NULL) {
		rssfalloccopy(&(opts->simplename), FINDNAME, strlen(FINDNAME));
	}
	if(opts->simplenodup == NULL) {
		rssfalloccopy(&(opts->simplenodup), FINDNODUP, strlen(FINDNODUP));
	}

	/*
	 * Create filter struct
	 */
	rc = rssfoptstosimple(opts, &filter);

	/*
	 * Add bogus name and nodup to filter
	 */
	if(filter.name == NULL) {
		rssfalloccopy(&(filter.name), FINDNAME, strlen(FINDNAME));
	}
	if(filter.nodup == NULL) {
		rssfalloccopy(&(filter.nodup), FINDNODUP, strlen(FINDNODUP));
	}

	rc = rsstfindnewtorrents(&filter, &newtorrents, 100, 0);
  if(rc != 0){
    retval=-1;
  }

	if(retval == 0){
		/*
		 * Print results
		 */
		for(count=0; count < newtorrents->nr; count++){
			rssfsizetohuman(newtorrents->newtorrent[count].size, humansize);
			printf("id: %d, name: %s , size: %s\n", 
					newtorrents->newtorrent[count].id,
					newtorrents->newtorrent[count].title,
					humansize);
			printf("URL: %s\n\n", 
					newtorrents->newtorrent[count].link);
		}
	}
	
	rsstfreesimplefilter(&filter);
	rc = rsstfreenewtorrentscontainer(newtorrents);
	free(newtorrents);
  if(rc != 0){
    retval=-1;
  }

	return retval;
}


/*
 * Print the last downloaded content per filter
 * @Arguments
 * handle RSS-torrent handle
 */
void rsstprintlastdowned(rsstor_handle *handle)
{
  int rc=0;
  int count=0;
  lastdowned_container container;
  lastdowned_struct *lastdownloaded;


  printf("Printing all filters, and the last content they downloaded.\n");

  /*
   * Get the last downloaded data
   */
  rc = rsstgetlastdownloaded(handle, &container);
  if(rc != 0) {
    fprintf(stderr, "Could not retrieve last downloaded data.\n");
    return;
  }

  /*
   * Print the restuls
   */
  printf("Number of records retreived '%d'\n", container.nr);
  while(count < container.nr) {
    lastdownloaded=&(container.lastdownloaded[count]);

    printf("Filter name: '%s', Filter type: '%s', Title: '%s', Season: '%d', Episode: '%d'\n", 
        lastdownloaded->filtername,
        lastdownloaded->filtertype,
        lastdownloaded->downloaded->title,
        lastdownloaded->downloaded->season,
        lastdownloaded->downloaded->episode);
    count++;
  }

  /*
   * Free the container
   */
  rsstfreelastdownedcontainer(&container);
}

/*
 * Print the program statistics
 * @Arguments
 * handle swarmtv handle
 */
void printstats(rsstor_handle *handle)
{
  int           rc=0;
  stats_struct  stats;
  char          dbsizestr[20];

  /*
   * Get statistics
   */
  rc = rsstgetstats(handle, &stats);
  if(rc != 0) {
    fprintf(stderr, "Getting program statistics failed ! %s:%d\n", __FILE__, __LINE__);
    return;
  }

  /*
   * Create human readable database size 
   */
  rssfsizetohuman(stats.dbsize, dbsizestr);

  /*
   * Format and print statistics
   */
  printf(
      "Statistics for SwarmTv %s\n"
      "================================\n"
      "Database version         : %d\n"
      "Number of sources        : %d\n"
      "Number of simple filters : %d\n"
      "Number of SQL filters    : %d\n"
      "Number of meta files     : %d\n"
      "Number of downloaded     : %d\n"
      "Size of the database     : %s\n",
      stats.version, stats.database, stats.sources,
      stats.simples, stats.sqls, stats.metafile,
      stats.downloaded, dbsizestr
      );
}
