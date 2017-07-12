#pragma once

#include <sstream>

// Declaration of the function this sample is all about.
// The szApp, szCmdLine, szCurrDir, si, and pi parameters are passed directly to CreateProcessWithTokenW.
// sErrorInfo returns text describing any error that occurs.
// Returns "true" on success, "false" on any error.
// It is up to the caller to close the HANDLEs returned in the PROCESS_INFORMATION structure.
bool RunAsDesktopUser(
  __in    const wchar_t *       szApp,
  __in    wchar_t *             szCmdLine,
  __in    const wchar_t *       szCurrDir,
  __in    STARTUPINFOW &        si,
  __inout PROCESS_INFORMATION & pi,
  __inout std::wstringstream &       sErrorInfo);

