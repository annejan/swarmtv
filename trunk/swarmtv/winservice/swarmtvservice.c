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
 *  Windows port Anne Jan Brouwer 2011
 */

#include <windows.h>
#include <stdio.h>
#include <swarmtv.h>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void  ServiceMain(int argc, char** argv);
void  ControlHandler(DWORD request);
rsstor_handle *InitService();

void main()
{
  SERVICE_TABLE_ENTRY ServiceTable[2];
  ServiceTable[0].lpServiceName = "SwarmTv";
  ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

  ServiceTable[1].lpServiceName = NULL;
  ServiceTable[1].lpServiceProc = NULL;
  // Start the control dispatcher thread for our service
  StartServiceCtrlDispatcher(ServiceTable);
}


void ServiceMain(int argc, char** argv)
{
  rsstor_handle *handle=NULL;
  int rc=0;
  int timewait=0;
  int before=0;
  int after=0;
  int timeleft=0;

  ServiceStatus.dwServiceType        = SERVICE_WIN32;
  ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
  ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  ServiceStatus.dwWin32ExitCode      = 0;
  ServiceStatus.dwServiceSpecificExitCode = 0;
  ServiceStatus.dwCheckPoint         = 0;
  ServiceStatus.dwWaitHint           = 0;

  hStatus = RegisterServiceCtrlHandler(
      "SwarmTv",
      (LPHANDLER_FUNCTION)ControlHandler);
  if (hStatus == (SERVICE_STATUS_HANDLE)0)
  {
    // Registering Control Handler failed
    return;
  }

  // Initialize Service
  handle = InitService();
  if (handle==NULL)
  {
    // Initialization failed
    ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    ServiceStatus.dwWin32ExitCode      = -1;
    SetServiceStatus(hStatus, &ServiceStatus);
    return;
  }

  // We report the running status to SCM.
  ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  SetServiceStatus (hStatus, &ServiceStatus);

  while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
  {
    rc = rsstconfiggetint(handle, CONF_REFRESH, &timewait);

    //                if (result)
    //                {
    //                        ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    //                        ServiceStatus.dwWin32ExitCode      = -1;
    //                        SetServiceStatus(hStatus, &ServiceStatus);
    //                        return;
    //                }

    before = time(NULL);

    /*
     * Call the routine to do the work
     */
    rc = runcycle(handle);

    after = time(NULL);

    /*
     * Calculate sleep time left.
     */
    after = time(NULL);
    timeleft = timewait - (after - before);
    if(timeleft < 0) {
      timeleft = timewait;
    }

    Sleep(timeleft*1000);
  }
  return;
}

// Service initialization
rsstor_handle *InitService()
{
  rsstor_handle *handle=NULL;
  handle = initrsstor();
  return(handle);
}

// Control handler function
void ControlHandler(DWORD request)
{
  switch(request)
  {
  case SERVICE_CONTROL_STOP:
    //WriteToLog("Monitoring stopped.");

    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
    SetServiceStatus (hStatus, &ServiceStatus);
    return;

  case SERVICE_CONTROL_SHUTDOWN:
    //WriteToLog("Monitoring stopped.");

    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
    SetServiceStatus (hStatus, &ServiceStatus);
    return;

  default:
    break;
  }

  // Report current status
  SetServiceStatus (hStatus,  &ServiceStatus);

  return;
}

