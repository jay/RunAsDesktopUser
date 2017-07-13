/* LICENSE: MIT License
Copyright (C) 2017 Jay Satiro <raysatiro@yahoo.com>
https://github.com/jay/RunAsDesktopUser/blob/master/LICENSE
*/

#include <Windows.h>
#include <shlobj.h>

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "ExecInExplorer_Util.h"

#include "RunAsDesktopUser_Implementation.h"
#include "RunAsDesktopUser_Utils.h"

using namespace std;

#define OPTSTR_CURDIR    L"--use-current-directory"
#define OPTSTR_FALLBACK  L"--fallback-current-user"
#define OPTSTR_WAITEXIT  L"--wait-for-exit-code"

void ShowUsageAndDie(void)
{
  MessageBoxW(NULL,
    L"Usage: RunAsDesktopUser "
    L"[" OPTSTR_CURDIR L"[=dir]] [" OPTSTR_FALLBACK L"] [" OPTSTR_WAITEXIT L"] <prog> [[arg]...]\n"
    L"Execute a command in the context of the desktop user.\n"
    L"\n"
    L"RunAsDesktopUser is useful if you are running as administrator and you want to execute a "
    L"program in the context of the desktop user (eg it will de-elevate the program if the desktop "
    L"user's shell isn't running as admin).\n"
    L"\n"
    L"Use option " OPTSTR_FALLBACK L" to fall back to executing in the context of the current user "
    L"running this program if that user is not running the program with administrator privileges. "
    L"If you do not use this option and are not running this program as an administrator then it's "
    L"likely this program will be unable to obtain the required permissions and will have no "
    L"effect.\n"
    L"\n"
    L"Use option " OPTSTR_CURDIR L" to use this program's current directory or optionally 'dir'. "
    L"Due to the command line being parsed as raw data in order to preserve it for CreateProcess, "
    L"this option is parsed raw (escapes are not acknowledged), must not contain quotes but can be "
    L"quoted and if quoted ends immediately at the next quote no matter what. For example:\n"
    L"\n"
    L"Yes:  RunAsDesktopUser \"" OPTSTR_CURDIR L"=C:\\foo bar\\baz\" notepad foo\n"
    L"No:   RunAsDesktopUser " OPTSTR_CURDIR L"=\"C:\\foo bar\\baz\" notepad foo\n"
    L"No:   RunAsDesktopUser \"" OPTSTR_CURDIR L"=\\\"C:\\foo bar\\baz\\\"\" notepad foo\n"
    L"\n"
    L"If the specified directory is invalid or inaccessible the behavior is undocumented. "
    L"Empirical testing shows process creation will fail. If you do not use option " OPTSTR_CURDIR
    L" then the behavior varies; it is documented that both CreateProcessWithTokenW (admin) and "
    L"CreateProcessW (nonadmin) will use RunAsDesktopUser's current directory, however empirical "
    L"testing shows the former may use the system directory (typically C:\\Windows\\System32).\n"
    L"\n"
    L"Use option " OPTSTR_WAITEXIT L" to wait for the exit code of the 'prog' process, and have "
    L"this program return that exit code. If you do not use this option then the exit code is 0 if "
    L"prog was started. Whether this option is used or not if an error occurs in RunAsDesktopUser "
    L"the exit code is -1. There is no other interaction.\n"
    L"\n"
    L"https://github.com/jay/RunAsDesktopUser\n", L"RunAsDesktopUser", 0);
  exit(-1);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR cmdline, int)
{
  WCHAR *dir, *prog, *argline;
  bool fallback_current_user, wait_for_exit_code;

  cmdline = _wcsdup(cmdline);
  if(!*cmdline)
    ShowUsageAndDie();

  if(!ProgAndArgLineFromCmdLine(cmdline, &prog, &argline))
    return -1;

  if(!wcscmp(prog, L"--help") && !*argline)
    ShowUsageAndDie();

  dir = NULL;
  fallback_current_user = false;
  wait_for_exit_code = false;

  for(;;) {
    if(!wcscmp(prog, OPTSTR_FALLBACK))
      fallback_current_user = true;
    else if(!wcscmp(prog, OPTSTR_WAITEXIT))
      wait_for_exit_code = true;
    else if(!wcscmp(prog, OPTSTR_CURDIR) ||
            !wcsncmp(prog, OPTSTR_CURDIR L"=", wcslen(OPTSTR_CURDIR) + 1)) {
      WCHAR *p = prog + wcslen(OPTSTR_CURDIR);
      if(*p == '=' && *++p) {
        if(wcschr(p, '"'))
          return -1;

        dir = _wcsdup(p);
        if(!dir)
          return -1;
      }
      else {
        if(!GetCurrentDir_ThreadUnsafe(&dir))
          return -1;
      }
    }
    else
      break;

    free(prog);
    free(cmdline);
    cmdline = argline;

    if(!ProgAndArgLineFromCmdLine(cmdline, &prog, &argline))
      return -1;
  }

  STARTUPINFOW si = {};
  PROCESS_INFORMATION pi = {};
  wstringstream errinfo;

  if(IsUserAnAdmin()) {
    /* This uses CreateProcessWithTokenW which is not available in XP */
    if(!RunAsDesktopUser(NULL, cmdline, dir, si, pi, errinfo))
      return -1;
  }
  else if(fallback_current_user) {
    if(!CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, 0,
                       NULL, dir, &si, &pi))
      return -1;
  }
  else
    return -1;

  DWORD exitcode = 0;

  if(wait_for_exit_code) {
    if(WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
      return -1;

    if(!GetExitCodeProcess(pi.hProcess, &exitcode))
      return -1;
  }

  free(dir);
  free(prog);
  free(argline);
  free(cmdline);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return (int)exitcode;
}
