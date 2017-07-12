#include <sstream>

#include "RunAsDesktopUser_Utils.h"

using namespace std;

// Definition of the function this sample is all about.
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
  __inout wstringstream &       sErrorInfo)
{
	HANDLE hShellProcess = NULL, hShellProcessToken = NULL, hPrimaryToken = NULL;
	HWND hwnd = NULL;
	DWORD dwPID = 0;
	BOOL ret;
	DWORD dwLastErr;

	// Enable SeIncreaseQuotaPrivilege in this process.  (This won't work if current process is not elevated.)
	HANDLE hProcessToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
	{
		dwLastErr = GetLastError();
		sErrorInfo << L"OpenProcessToken failed:  " << SysErrorMessageWithCode(dwLastErr);
		return false;
	}
	else
	{
		TOKEN_PRIVILEGES tkp;
		tkp.PrivilegeCount = 1;
		LookupPrivilegeValueW(NULL, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid);
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, NULL, NULL);
		dwLastErr = GetLastError();
		CloseHandle(hProcessToken);
		if (ERROR_SUCCESS != dwLastErr)
		{
			sErrorInfo << L"AdjustTokenPrivileges failed:  " << SysErrorMessageWithCode(dwLastErr);
			return false;
		}
	}

retry:
	// Get an HWND representing the desktop shell.
	// CAVEATS:  This will fail if the shell is not running (crashed or terminated), or the default shell has been
	// replaced with a custom shell.  This also won't return what you probably want if Explorer has been terminated and
	// restarted elevated.
	hwnd = GetShellWindow();
	if (NULL == hwnd)
	{
		sErrorInfo << L"No desktop shell is present";
		return false;
	}

	// Get the PID of the desktop shell process.
	GetWindowThreadProcessId(hwnd, &dwPID);
	if (0 == dwPID)
	{
		sErrorInfo << L"Unable to get PID of desktop shell.";
		return false;
	}

	// Open the desktop shell process in order to query it (get the token)
	hShellProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if (!hShellProcess)
	{
		dwLastErr = GetLastError();
		sErrorInfo << L"Can't open desktop shell process:  " << SysErrorMessageWithCode(dwLastErr);
		return false;
	}

  // Make sure window and process id are still the same (ie we've opened the right process)
  if(hwnd != GetShellWindow()) {
    CloseHandle(hShellProcess);
    goto retry;
  }
  GetWindowThreadProcessId(hwnd, &dwPID);
  if(dwPID != GetProcessId(hShellProcess)) {
    CloseHandle(hShellProcess);
    goto retry;
  }

	// From this point down, we have handles to close, so make sure to clean up.

	bool retval = false;
	// Get the process token of the desktop shell.
	ret = OpenProcessToken(hShellProcess, TOKEN_DUPLICATE, &hShellProcessToken);
	if (!ret)
	{
		dwLastErr = GetLastError();
		sErrorInfo << L"Can't get process token of desktop shell:  " << SysErrorMessageWithCode(dwLastErr);
		goto cleanup;
	}

	// Duplicate the shell's process token to get a primary token.
	// Based on experimentation, this is the minimal set of rights required for CreateProcessWithTokenW (contrary to current documentation).
	const DWORD dwTokenRights = TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID;
	ret = DuplicateTokenEx(hShellProcessToken, dwTokenRights, NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken);
	if (!ret)
	{
		dwLastErr = GetLastError();
		sErrorInfo << L"Can't get primary token:  " << SysErrorMessageWithCode(dwLastErr);
		goto cleanup;
	}

	// Start the target process with the new token.
	ret = CreateProcessWithTokenW(
		hPrimaryToken,
		0,
		szApp,
		szCmdLine,
		0,
		NULL,
		szCurrDir,
		&si,
		&pi);
	if (!ret)
	{
		dwLastErr = GetLastError();
		sErrorInfo << L"CreateProcessWithTokenW failed:  " << SysErrorMessageWithCode(dwLastErr);
		goto cleanup;
	}

	retval = true;

cleanup:
	// Clean up resources
	CloseHandle(hShellProcessToken);
	CloseHandle(hPrimaryToken);
	CloseHandle(hShellProcess);
	return retval;
}


