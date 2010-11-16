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
#include <getopt.h>

#include <swarm.h>

#include "handleopts.h"
#include "frontfuncts.h"
#include "present.h"

#define INPUTSIZE 256

/*
 * Buffer size used in present routines.
 */
#define   BUFSIZE 20

/*
 * Get input from the user in a SAFE way :)
 * Thanks to http://home.datacomm.ch/t_wolf/tw/c/getting_input.html#scanf
 * @arguments
 * buf 			Buffer to write input in (256 chars long)
 * bufsize	Size of input buffer
 * filep		File pointer to read from
 * @return
 * lenght of captured string
 */
static int get_line (char *buf, size_t bufsize)
{
	int retval=0;
	int length=0;

	/*
	 * Clear input buffer
	 */
	memset(buf, 0, bufsize);

	/*
	 * Get string
	 * And remove the '\n' at end of line when needed.
	 */
	fgets(buf, bufsize-1, stdin);

	if(buf != NULL) {
		length = strlen(buf);
		if(buf[length-1] == '\n') {
			buf[length-1] = '\0';
		}
	}

	/*
	 * Evaluate result
	 */
	retval = strlen(buf);

	/*
	 * return size of captured string
	 */
	return retval;
}

/*
 * Function to get user input
 * @arguments
 * answer 		pointer that will be allocated, should be freed afterwards (Make sure to free afterwards)
 * question		The question that has to be printed to the user
 * suggestion The suggestion for the answer, when no answer is given, suggestion is copied to answer
 * @return
 * -1 on failure
 *  0 when no answer was given
 *  1 when answer was given
 */
static int rssfaskuser(char **answer, char *question, char *suggestion)
{
	int  rc=0;
	char inputbuf[INPUTSIZE];

	/*
	 * Print question
	 */
	printf("%s\n[%s] :", question, suggestion);

	/*
	 * Get input
	 */
	rc = get_line(inputbuf, INPUTSIZE);

	/*
	 * Copy input to answer
	 * when empty copy suggestion to answer
	 */
	if(rc == 0) {
		rssfalloccopy(answer, suggestion, strlen(suggestion));
	} else {
		rssfalloccopy(answer, inputbuf, strlen(inputbuf));
	}

	return rc;
}

/*
 * Function get numerical input
 * @arguments
 * question question to be asked
 * answer input/output variable to store answer in.
 * @return
 * 0 on success, -1 on error
 */
static int rssfasknumeric(int *answer, char *question)
{
	int  rc=0;
	char inputbuf[INPUTSIZE];

	while(1==1){
		/*
		 * Print question
		 */
		printf("%s\n[%d] :", question, *answer);

		/*
		 * Get input
		 */
		rc = get_line(inputbuf, INPUTSIZE);

		/*
		 * When nothing entered, assume 0
		 */
		if(rc == 0) {
			*answer = 0;
			break;
		}

		/*
		 * Translate to int
		 */
		*answer=atoi(inputbuf);
		if(*answer != 0){
			break;
		} 

		printf("Invalid input please enter a number.\n");
	}

	return 0;
}


/*
 * This function tests yes or no answers and returns them as boolean values
 * @arguments
 * answer 		0 on 'no', 1 on 'yes', -1 on no matching answer
 * question		The question that has to be printed to the user
 * suggestion The suggestion for the answer, when no answer is given, suggestion is copied to answer
 * @return
 * 0 on matching answer
 * 1 on no matching answer
 */
static int rssfaskbool(int *answer, char *question, char *suggestion)
{
	int   rc=0;
	int   retval=0;
	int   match=0;
	char *ansbuf=NULL;
	char *locsug=NULL;

	/*
	 * Get a local lower case copy of the suggestion.
	 */
	rssfalloccopy(&locsug, suggestion, strlen(suggestion));
	rssflowercase(locsug);

	/*
	 * Get user input
	 */
	rc = rssfaskuser(&ansbuf, question, suggestion);
	rssflowercase(ansbuf);

	/*
	 * Evaluate ansbuf
	 */
	if(strcmp(ansbuf, "yes") == 0 || strcmp(ansbuf, "y") == 0){
		retval=1;
		match=1;
	}
	if(strcmp(ansbuf, "no") == 0 || strcmp(ansbuf, "n") == 0){
		retval=0;
		match=1;
	}


	/*
	 * When no ansbuf is given, eval suggestion.
	 */
	if(match == 0)
	{
		if(strcmp(locsug, "yes") == 0 || strcmp(locsug, "y") == 0){
			retval=1;
			match=1;
		}
		if(strcmp(locsug, "no") == 0 || strcmp(locsug, "n") == 0){
			retval=0;
			match=1;
		}

		/*
		 * When an ansbuf is given, and not yes or no fail.
		 */
		if(strlen(ansbuf) != 0 || match == 0)
		{
			printf("Invalid input entered.\n");
			retval=-1;
			match=0;
		}
	}

	/*
	 * store found answer
	 */
	*answer = retval;

	/*
	 * Clean up
	 */
	free(ansbuf);
	free(locsug);

	return match;
}


static int rssfaskreplace(char **answer, char *question)
{
	int rc=0;
	char *temp=NULL;

	/*
	 * Make sure we have a string to begin with (empty string);
	 */
	if(*answer == NULL) {
		*answer=calloc(1,sizeof(char));
	}

	/*
	 * Get the answer
	 */
	rc = rssfaskuser(&temp, question, *answer);
	if(rc < 0){
		fprintf(stderr, "Getting user input failed.\n");
		return -1;
	}

	/*
	 * When only a "." is given, set answer value to empty string
	 * This is needed to empty strings when cloning filters.
	 */
	if(strcmp(".", temp) == 0) {
		*temp='\0';
	}

	/*
	 * Free original string and replace it with the answer
	 */
	free(*answer);
	*answer=temp;

	/*
	 * Return status
	 */
	return rc;
}

/*
 * This function returns a single simple struct
 * @arguments
 * handle RSS-torrent handle
 * name simple filter name
 * simple simple filter pointer to store answer in. (NULL when name is not found)
 * @return
 * 0 on success
 * -1 on error 
 */
static int rssfgetsimple(rsstor_handle *handle, int id, simplefilter_struct **simple)
{
	int rc=0;
	simplefilter_container *container=NULL;

	/*
	 * Get container, return when not found
	 */
	rc = rsstgetsimplefilterid(handle, &container, id);
	if(rc < 0) {
		return rc;
	}

	/*
	 * When more then one filter match, this is not okay
	 */
	if(container->nr > 1){	
		fprintf(stderr, "More than 1 filter is known by this id '%d'!\n", id);
		rsstfreesimplefiltercontainer(container);
		return -1;
	}

	/*
	 * When no simple filter is found return -1
	 */
	if(container->nr == 0){
		*simple=NULL;
	} else {
		/*
		 * get pointer to the structure, free the container
		 */
		*simple=container->simplefilter;
	}
	free(container);

	return 0;
}


/*
 * Get user input and decide what to put in the simplefilter_struct
 * @arguments
 * handle rsstorrent handle
 * simple pointer pointer to simplestruct to initialize
 * @return
 * 0 on success
 * -1 on failure
 */
static int rssfasksimplefilter(rsstor_handle *handle, simplefilter_struct **simple)
{
	int		rc=0;
	char  *answer=NULL;
  int   ansint=0;

	/*
	 * Show simple filters
	 */
	rssflistallsimple(handle);

	/*
	 * Ask until we get a valid answer
	 */
	while(1 == 1) {
		/*
		 * Ask name of filter
		 */
		rc = rssfaskuser(&answer, "Please enter simple filter ID to clone.", "");
		if(rc < 0){
				fprintf(stderr, "Getting user input failed.\n");
				return -1;
		}

		/*
		 * When an empty answer is given, start with an empty filter
		 */
		if(strlen(answer) == 0) {
			printf("Start with an empty filter.\n");
			*simple = calloc(1, sizeof(simplefilter_struct));
			break;
		}

		/*
		 * Find simple struct by name
		 */
    ansint = atoi(answer);
		rc = rssfgetsimple(handle, ansint, simple);
		if(rc < 0){
			fprintf(stderr, "Error filter finding failed\n");
			break;
		}

		/*
		 * When a filter is found clone it.
		 */
		if(*simple != NULL) {
			printf("Filter '%s' will be cloned.\n", answer);
			break;
		} 

		/*
		 * No valid input, defaulting to this message
		 */
		fprintf(stderr, "Filter name not found.\n");
		fprintf(stderr, "Please enter a valid simple filter name.\n");
		fprintf(stderr, "you could also enter an empty line to start a new simple filter.\n");
		free(answer);
	}

	/*
	 * Done !
	 */
	free(answer);
	return 0;
}


/*
 * Get user input and decide what to put in the simplefilter_struct
 * @arguments
 * handle rsstorrent handle
 * srcreg pointer pointer input/output var src regexp
 * @return
 * 0 on success
 * -1 on failure
 */
static int rssfasksource(rsstor_handle *handle, char **srcreg)
{
	int rc=0;

	/*
	 * Print available sources
	 */
	rssfprintsources(handle);

	/*
	 * Get user input
	 */
	rc = rssfaskreplace(srcreg, "Please enter source regexp.");
	if(rc < 0){
		fprintf(stderr, "Getting user input failed.\n");
		return -1;
	}

	return 0;
}


/*
 * Get nodup simple filter
 * @arguments
 * handle rsstorrent handle
 * nodup nodup input/output nodup filter string
 * @return
 * 0 on success
 * -1 on failure
 */
static int rssfasknodup(char **nodup)
{
	int rc=0;

	/*
	 * Print all nodup filters we have
	 */
	printf("The nodup filters that are :\n"
			   "'none' - Do not filter on anything.\n"
				 "'link' - Only look at the URL of the torrent.\n"
				 "'unique' - Look at season, episode, title regexp.\n"
				 "'newer' - Look at same as unique, but make sure only newer episodes are download.\n");

	/*
	 * Loop until we have a valid answer
	 */
	while (1 == 1) {
		/*
		 * Ask user input
		 */
		rc = rssfaskreplace(nodup, "Please enter nodup filter name, or empty to use suggestion.");
		if(rc < 0){
			fprintf(stderr, "Getting user input failed.\n");
			return -1;
		}

		/*
		 * Compare to the known nodup filters
		 */
		if( strcmp(*nodup, "none") == 0 ||
				strcmp(*nodup, "link") == 0 ||
				strcmp(*nodup, "unique") == 0 ||
				strcmp(*nodup, "newer") == 0)
		{
			printf("nodup '%s' selected.\n", *nodup);
			break;
		}
	}

	/*
	 * Done
	 */
	return 0;
}


/*
 * Ask the user for the sizes
 * @Arguments
 * question the question to display
 * size input/output double
 * @Return
 * 0 on success
 * -1 on error
 */
static int rssfasksize(char *question, double *size)
{
	int 		rc=0;
	double 	temp=0.0;
	char 		sizestring[BUFSIZE];
	char    *answer=NULL;

	/*
	 * Convert double to size-string.
	 */
	rssfsizetohuman(*size, sizestring);

	/*
	 * Loop until we have valid input
	 */
	while(1 == 1){
		/*
		 * Get input
		 */
		rc = rssfaskuser(&answer, question, sizestring);
		if(rc < 0){
			fprintf(stderr, "Getting user input failed.\n");
			return -1;
		}

		/*
		 * If empty set value 0 and break
		 */
		if(strlen(answer) == 0) {
			free(answer);
			break;
		}

		/*
		 * decode the input
		 */
		rc = rssfhumantosize(answer, &temp); 
		free(answer);

		/*
		 * When the input is valid break
		 */
		if(rc == 0) {
			/*
			 * Save size
			 */
			*size = temp;
			break;
		}
	}

	/*
	 * All gone well
	 */
	return 0;
}


/*
 * Accept filter yes/no
 * @arguments
 * simple pointer to simple filter stuct
 * int answer output value for output
 * @return
 * 0 on success, -1 on error
 */
static int rssfacceptsimple(simplefilter_struct *simple, int *answer, char *execname)
{
	int rc=0;

	/*
	 * Print current simple-filter
	 */
	printsimplestruct(execname, simple);

	/*
	 * Ask for conformation
	 */
	rc = rssfaskbool(answer, "Is this filter correct ?", "No");

	/*
	 * Done
	 */
	return 0;
}


/*
 * Get season/episode
 * @arguments
 * @return
 * 0 on success, -1 on error
 */
static int rsstaskseasonepisode(simplefilter_struct *simple)
{
	int   ansval=0;
	int   foundseason=0;
	int   foundepisode=0;
	int   loop=0;
	int 	rc=0;
	
	/*
	 * Ask if auto season/episode should be applied
	 * If not ask separately
	 */
	while(loop == 0) {
		loop = rssfaskbool(&ansval, "Would you like to automatically determine the last season/episode for this filter? (yes/no)", "Yes");
		if(ansval == 1){
			/*
			 * Fill season/episode automatically
			 */
			rc = rsstgetnewestepisode(simple, &foundseason, &foundepisode);
			simple->fromseason = foundseason;
			simple->fromepisode = foundepisode;
			printf("found Last season %d Last episode %d\n", foundseason, foundepisode);
		} else {
			/*
			 * Ask for last season/episode numbers.
			 */
			rc = rssfasknumeric(&(simple->fromseason), "Enter season number: ");
			rc = rssfasknumeric(&(simple->fromepisode), "Enter episode number: ");
		}
		//printf("answer: '%d' '%d' '%s'\n", loop, ansval, ansval ? "yes" : "no");
	}

	return 0;
}


/*
 * Get season/episode
 * @arguments
 * @return
 * 0 on success, -1 on error
 */
static int rssfgetsimplename(simplefilter_struct *simple)
{
	int rc=0;

	while(1==1){
		/*
		 * ask name
		 */
		rc = rssfaskreplace(&(simple->name), "Please enter simple filter name.");
		if(rc < 0) {
			return -1;
		}

		/*
		 * Make sure the name is not empty
		 */
		if(strlen(simple->name) != 0) {
			break;
		}

		/*
		 * Print directions.
		 */
		fprintf(stderr, "Simple filter Name can not be empty.\n");
	}

	return 0;
}


/*
 * Wizard to make adding simple filters even simpler
 * @arguments
 * @return
 */
void rssfsimplewizard(rsstor_handle *handle)	
{
	int   ansval=0;
	int 	rc=0;
	simplefilter_struct *simple=NULL;

	/*
	 * Print Header
	 */
	printf("### Simple Filter Wizard ###\n"
			"Please enter an empty line to keep suggested value.\n"
			"Enter an single dot '.' in a line to clear the suggestion value and enter an empty string.\n");


	/*
	 * Ask the user to clone an existing filter or start from scratch.
	 */
	rc = rssfasksimplefilter(handle, &simple);
	if(rc == -1) {
		fprintf(stderr, "Getting simple filter failed!\n");
		return;
	}

	/*
	 * Get filter name
	 * When empty and an other filter is used as base, change that filter.
	 */
	rc = rssfgetsimplename(simple);
	//printf("The answer given: (%d) '%s'\n", strlen(simple->name), simple->name);

	/*
	 * Get title regexp (regexp that should match with title)
	 */
	rc = rssfaskreplace(&(simple->title), "Please enter content title regexp.");
	//printf("The answer given: (%d) '%s'\n", strlen(simple->title), simple->title);

	/*
	 * Get exclude regexp (regexp that should not match with title)
	 */
	rc = rssfaskreplace(&(simple->exclude), "Please enter regexp to exclude from matches.");
	//printf("The answer given: (%d) '%s'\n", strlen(simple->exclude), simple->exclude);

	/*
	 * Category regexp
	 */
	rc = rssfaskreplace(&(simple->category), "Please enter RSS category.");
	//printf("The answer given: (%d) '%s'\n", strlen(simple->category), simple->category);

	/*
	 * Source regexp
	 */
	rc = rssfasksource(handle, &(simple->source));
	//printf("The answer given: (%d) '%s'\n", strlen(simple->source), simple->source);

	/*
	 * Get nodup filter
	 */
	rc = rssfasknodup(&(simple->nodup));

	/*
	 * Get max size
	 */
	rc = rssfasksize("Maximal size of torrent content: ", &(simple->maxsize));

	/*
	 * Get min size
	 */
	rc = rssfasksize("Minimal size of torrent content: ", &(simple->minsize));
	
	/*
	 * Get season/episode
	 */
	rc = rsstaskseasonepisode(simple);

	/*
	 * Okay/discard simple filter Or test filter
	 */
	rc = rssfacceptsimple(simple, &ansval, "");
	if(ansval == 0) {
		printf("Filter not accepted.\n");
	} else {
		/*
		 * Store the simple filter
		 */
		rc = rsstaddsimplefilter(handle, simple);
		printf("Filter inserted.\n");
	}

	/*
	 * Clean up.
	 */
	rsstfreesimplefilter(simple);
	free(simple);
}

