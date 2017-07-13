/* LICENSE: MIT License
Copyright (C) 2017 Jay Satiro <raysatiro@yahoo.com>
https://github.com/jay/ExecInExplorer/blob/master/LICENSE
*/

#include <Windows.h>
#include <tchar.h>

#include <vector>

#include "ExecInExplorer_Implementation.h"
#include "ExecInExplorer_Util.h"

using namespace std;

#define ISSPACE(c) ((c) == ' ' || (c) == '\t')

#define OPTSTR_CURDIR  L"--use-current-directory"

void ShowUsageAndDie(void)
{
  MessageBoxW(NULL,
    L"Usage: ExecInExplorer [" OPTSTR_CURDIR L"[=dir]] <prog> [[arg]...]\n"
    L"Execute a command asynchronously in the context of explorer.\n"
    L"\n"
    L"ExecInExplorer is useful if you are running as administrator and you want to execute a "
    L"program in the context of the user's explorer (eg it will de-elevate the program if "
    L"explorer isn't running as admin).\n"
    L"\n"
    L"Use option " OPTSTR_CURDIR L" to tell explorer to use this program's current directory or "
    L"optionally 'dir'. Due to the command line being parsed as raw data in order to preserve it "
    L"for explorer, this option is parsed raw (escapes are not acknowledged), must not contain "
    L"quotes but can be quoted and if quoted ends immediately at the next quote no matter what. "
    L"For example:\n"
    L"\n"
    L"Yes:  ExecInExplorer \"" OPTSTR_CURDIR L"=C:\\foo bar\\baz\" notepad foo\n"
    L"No:   ExecInExplorer " OPTSTR_CURDIR L"=\"C:\\foo bar\\baz\" notepad foo\n"
    L"No:   ExecInExplorer \"" OPTSTR_CURDIR L"=\\\"C:\\foo bar\\baz\\\"\" notepad foo\n"
    L"\n"
    L"If the specified directory is invalid or inaccessible the behavior is undocumented. "
    L"Empirical testing shows if the directory does not exist then explorer will use the root "
    L"directory of the drive if the drive exists or the system root (typically C:\\Windows) if it "
    L"doesn't. If you do not use option " OPTSTR_CURDIR L" then explorer will use its current "
    L"directory (typically C:\\Windows\\System32).\n"
    L"\n"
    L"The exit code returned by ExecInExplorer is 0 if the command line to be executed was "
    L"*dispatched* successfully or otherwise an HRESULT indicating why it wasn't. There is no "
    L"other interaction.\n"
    L"\n"
    L"https://github.com/jay/ExecInExplorer\n", L"ExecInExplorer", 0);
  exit(-1);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, const PWSTR cmdline, int)
{
  HRESULT hr;
  WCHAR *dir, *prog, *argline;

  if(!*cmdline)
    ShowUsageAndDie();

  if(!ProgAndArgLineFromCmdLine(cmdline, &prog, &argline))
    return -1;

  if(!wcscmp(prog, L"--help") && !*argline)
    ShowUsageAndDie();

  dir = NULL;

  if(!wcsncmp(prog, OPTSTR_CURDIR, wcslen(OPTSTR_CURDIR))) {
    WCHAR *p = prog + wcslen(OPTSTR_CURDIR);

    if(!*p || *p == '=') {
      WCHAR *newcmdline;

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

      p = NULL;
      free(prog);
      newcmdline = argline;

      if(!ProgAndArgLineFromCmdLine(newcmdline, &prog, &argline))
        return -1;

      free(newcmdline);
    }
  }

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if(SUCCEEDED(hr)) {
    hr = ShellExecInExplorerProcess(prog, argline, dir);
    CoUninitialize();
  }

  free(dir);
  free(prog);
  free(argline);
  return SUCCEEDED(hr) ? 0 : (int)hr;
}
