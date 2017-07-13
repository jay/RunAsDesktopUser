/* LICENSE: MIT License
Copyright (C) 2017 Jay Satiro <raysatiro@yahoo.com>
https://github.com/jay/ExecInExplorer/blob/master/LICENSE
*/

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include "ExecInExplorer_Util.h"

#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>

#pragma comment(lib, "psapi.lib")

/* MAX_EXTENDED_PATH includes the NUL the same way MAX_PATH does.
   https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx#maxpath */
#define MAX_EXTENDED_PATH (32767 + 1)

#define ISSPACE(c) ((c) == ' ' || (c) == '\t')

/*
Separate the first argument from remaining arguments in a command line.

The first argument is parsed as the program argument not as a normal argument,
which means if the argument starts with a quote then it ends at the next quote,
if any, no matter what (ie escaped quotes aren't acknowledged) and those quotes
and outer whitespace are removed. If it doesn't start with a quote then it ends
at the following whitespace, if any, no matter what (ie escaped whitespace
isn't acknowledged). For example:

cmdline:   foo   bar baz
prog:     foo
argline:  bar baz

cmdline:   "C:\foo bar\baz"  " wibble wobble"  wubble 
prog:     C:\foo bar\baz
argline:  " wibble wobble"  wubble 

On success nonzero is returned and:
*prog points to a copy of the first argument.
*argline points to a copy of the remaining arguments.
Caller free()s memory.
*/
BOOL ProgAndArgLineFromCmdLine(const TCHAR *cmdline, TCHAR **prog,
                               TCHAR **argline)
{
  const TCHAR *p;
  size_t prog_len;

  *prog = NULL;
  *argline = NULL;

  for(p = cmdline; *p && ISSPACE(*p); ++p);

  if(*p == '"') {
    cmdline = ++p;
    for(; *p && *p != '"'; ++p);
  }
  else {
    cmdline = p;
    for(; *p && !ISSPACE(*p); ++p);
  }

  prog_len = p - cmdline;

  *prog = (TCHAR *)malloc((prog_len + 1) * sizeof(TCHAR));
  if(!*prog) {
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  _tcsncpy(*prog, cmdline, prog_len);
  (*prog)[prog_len] = 0;

  if(*p == '"')
    ++p;

  for(; *p && ISSPACE(*p); ++p);

  *argline = _tcsdup(p);
  if(!*argline) {
    free(*prog);
    *prog = NULL;
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  return TRUE;
}

/*
Get the current directory.
Note this calls GetCurrentDirectory API which is not thread-safe.

On success nonzero is returned and:
*currentdir points to a copy of the current directory.
Caller free()s memory.
*/
BOOL GetCurrentDir_ThreadUnsafe(TCHAR **currentdir)
{
  DWORD c, res;

  *currentdir = NULL;

  c = GetCurrentDirectory(0, NULL);

  if(!c || c >= MAX_EXTENDED_PATH) {
    DWORD gle = c ? ERROR_BAD_LENGTH : GetLastError();
    /**/
    SetLastError(gle);
    return FALSE;
  }

  *currentdir = (TCHAR *)malloc((size_t)c * sizeof(TCHAR));
  if(!*currentdir) {
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  res = GetCurrentDirectory(c, *currentdir);

  if(!res || res >= c) {
    DWORD gle = res ? ERROR_BAD_LENGTH : GetLastError();
    free(*currentdir);
    *currentdir = NULL;
    SetLastError(gle);
    return FALSE;
  }

  return TRUE;
}

/*
Get the full path of this program.

This is more reliable than the global _tpgmptr which may not be the full path.

On success nonzero is returned and:
*progpath points to a copy of the full path of this program.
Caller free()s memory.
*/
BOOL GetProgPath(TCHAR **progpath)
{
  TCHAR *buf;
  DWORD res;

  *progpath = NULL;

  buf = (TCHAR *)malloc(MAX_EXTENDED_PATH * sizeof(TCHAR));

  res = GetModuleFileNameEx(GetCurrentProcess(), NULL, buf, MAX_EXTENDED_PATH);

  /* If the character length written is MAX_EXTENDED_PATH - 1 the path may be
     truncated, so error. In practice this shouldn't happen. */
  if(!res || res >= MAX_EXTENDED_PATH - 1) {
    DWORD gle = res ? ERROR_BAD_LENGTH : GetLastError();
    free(buf);
    SetLastError(gle);
    return FALSE;
  }

  *progpath = _tcsdup(buf);

  if(!*progpath) {
    free(buf);
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  free(buf);
  return TRUE;
}
