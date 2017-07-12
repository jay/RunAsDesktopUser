#include <Windows.h>
#include <Psapi.h>
#include <shlobj.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "RunAsDesktopUser_Implementation.h"

using namespace std;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR cmdline, int)
{
  cmdline = _wcsdup(cmdline);
  if(!*cmdline) {
    MessageBoxA(NULL,
      "RunAsDesktopUser <cmdline>\n"
      "If this program is not run as admin it has no effect.\n",
      "Usage", 0);
    return -1;
  }

  STARTUPINFOW si = {};
  PROCESS_INFORMATION pi = {};
  wstringstream errinfo;

  if(IsUserAnAdmin()) {
    /* This uses CreateProcessWithTokenW which is not available in XP */
    if(!RunAsDesktopUser(NULL, cmdline, NULL, si, pi, errinfo))
      return -1;
  }
  else
    return -1;

  return 0;
}
