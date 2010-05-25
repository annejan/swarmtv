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
 *  http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "filesystem.h"

#include "sqlite3.h"
#include "types.h"
#include "config.h"

/*
 * Fork process to daemon.
 */
void rssfdaemonize(char *path)
{
  pid_t pid, sid;

  /* already a daemon */
  if ( getppid() == 1 ) return;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* At this point we are executing as the child process */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  /*
	 * Redirect input files to /dev/null.
	 * Redirect all else to given path.
	 */
  freopen( "/dev/null", "r", stdin);
  freopen( path, "w", stdout);
  freopen( path, "w", stderr);
}

/*
 * check Lockfile 
 */
static void rssflockfile(rsstor_handle *handle, const char *lockpath)
{
  int  lfp;
  char str[10];
  char *fullpath=NULL;

  rsstcompletepath(lockpath, &fullpath);

  lfp=open(fullpath, O_RDWR|O_CREAT, 0640);

  free(fullpath);

  if (lfp<0){
    fprintf(stderr, "Can not open lockfile\n");
    exit(1); /* can not open */
  }
  if (lockf(lfp,F_TLOCK,0)<0){ 
    fprintf(stderr, "Can not lock lockfile\n");
    exit(0); /* can not lock */
  }
  /* only first instance continues */

  snprintf(str, 9,"%d\n",getpid());
  write(lfp,str,strlen(str)); /* record pid to lockfile */

	/* Store pointer in handle */
	handle->lockfile = lfp;
}

/*
 * Lock the rsstorrent lockfile
 * This routine gets the path of the lockfile from the config settings.
 * @Arguments
 * handle RSS-torrent handle
 */
void rssflock(rsstor_handle *handle)
{
	sqlite3 *db=NULL;
	char    *lockpath=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

	/*
	 * Get the path the lockfile is in
	 */
	rsstconfiggetproperty(handle, CONF_LOCKFILE, &lockpath);

	/*
	 * Lock the file
	 */
	rssflockfile(handle, lockpath);

	/*
	 * Free the path
	 */
	free(lockpath);
}

/*
 * Free lockfile
 * This routine frees up the lockfile and leaves the database for other instances of RSS-torrent
 * @Arguments 
 * handle RSS-torrent handle
 * @Return
 * 0 on success otherwise -1
 */
int rssfunlock(rsstor_handle *handle)
{
  int  lfp;

	lfp = handle->lockfile;

	/*
	 * Valid pointer ?
	 */
  if (lfp<=0){
    fprintf(stderr, "no lockfile opened\n");
    return -1;
  }

	/* Release filelock */
  if (lockf(lfp,F_ULOCK,0)<0){ 
    fprintf(stderr, "Can not unlock lockfile\n");
    exit(0); /* can not lock */
  }

	/* Close the lockfile */
	close(lfp);

	/* Reset lockfile pointer */
	handle->lockfile = 0;

	return 0;
}
