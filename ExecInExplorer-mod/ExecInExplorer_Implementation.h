#pragma once

#include <Windows.h>

HRESULT ShellExecInExplorerProcess(PCWSTR pszFile, PCWSTR pszArgs = L"", PCWSTR pszDir = L"");
