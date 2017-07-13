#pragma once

#include <Windows.h>

BOOL ProgAndArgLineFromCmdLine(const TCHAR *cmdline, TCHAR **prog,
                               TCHAR **argline);

BOOL GetCurrentDir_ThreadUnsafe(TCHAR **currentdir);

BOOL GetProgPath(TCHAR **progpath);
