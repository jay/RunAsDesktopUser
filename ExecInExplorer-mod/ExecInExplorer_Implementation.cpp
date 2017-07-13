// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Copyright (C) 2017 Jay Satiro <raysatiro@yahoo.com>
//
// ShellExecInExplorerProcess() has been modified to take optional args and dir.
//
// Unmodified project at:
// https://github.com/Microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/winui/shell/appplatform/ExecInExplorer
//
// MIT License:
// https://github.com/jay/ExecInExplorer/blob/master/LICENSE

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <malloc.h>

#pragma comment(lib, "shlwapi.lib")

// use the shell view for the desktop using the shell windows automation to find the
// desktop web browser and then grabs its view
//
// returns:
//      IShellView, IFolderView and related interfaces

HRESULT GetShellViewForDesktop(REFIID riid, void **ppv)
{
    *ppv = NULL;

    IShellWindows *psw;
    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&psw));
    if (SUCCEEDED(hr))
    {
        HWND hwnd;
        IDispatch* pdisp;
        VARIANT vEmpty = {}; // VT_EMPTY
        if (S_OK == psw->FindWindowSW(&vEmpty, &vEmpty, SWC_DESKTOP, (long*)&hwnd, SWFO_NEEDDISPATCH, &pdisp))
        {
            IShellBrowser *psb;
            hr = IUnknown_QueryService(pdisp, SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
            if (SUCCEEDED(hr))
            {
                IShellView *psv;
                hr = psb->QueryActiveShellView(&psv);
                if (SUCCEEDED(hr))
                {
                    hr = psv->QueryInterface(riid, ppv);
                    psv->Release();
                }
                psb->Release();
            }
            pdisp->Release();
        }
        else
        {
            hr = E_FAIL;
        }
        psw->Release();
    }
    return hr;
}

// From a shell view object gets its automation interface and from that gets the shell
// application object that implements IShellDispatch2 and related interfaces.

HRESULT GetShellDispatchFromView(IShellView *psv, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IDispatch *pdispBackground;
    HRESULT hr = psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&pdispBackground));
    if (SUCCEEDED(hr))
    {
        IShellFolderViewDual *psfvd;
        hr = pdispBackground->QueryInterface(IID_PPV_ARGS(&psfvd));
        if (SUCCEEDED(hr))
        {
            IDispatch *pdisp;
            hr = psfvd->get_Application(&pdisp);
            if (SUCCEEDED(hr))
            {
                hr = pdisp->QueryInterface(riid, ppv);
                pdisp->Release();
            }
            psfvd->Release();
        }
        pdispBackground->Release();
    }
    return hr;
}

HRESULT ShellExecInExplorerProcess(PCWSTR pszFile, PCWSTR pszArgs, PCWSTR pszDir)
{
    IShellView *psv;
    HRESULT hr = GetShellViewForDesktop(IID_PPV_ARGS(&psv));
    if (SUCCEEDED(hr))
    {
        IShellDispatch2 *psd;
        hr = GetShellDispatchFromView(psv, IID_PPV_ARGS(&psd));
        if (SUCCEEDED(hr))
        {
            BSTR bstrFile = SysAllocString(pszFile);
            hr = bstrFile ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                VARIANT vtArgs = {};
                if (pszArgs && *pszArgs)
                {
                    vtArgs.vt = VT_BSTR;
                    vtArgs.bstrVal = SysAllocString(pszArgs);
                    hr = vtArgs.bstrVal ? S_OK : E_OUTOFMEMORY;
                }
                if (SUCCEEDED(hr))
                {
                    VARIANT vtDir = {};
                    if (pszDir && *pszDir)
                    {
                        vtDir.vt = VT_BSTR;
                        vtDir.bstrVal = SysAllocString(pszDir);
                        hr = vtDir.bstrVal ? S_OK : E_OUTOFMEMORY;
                    }
                    if (SUCCEEDED(hr))
                    {
                        /* Note using an app path directly here like L"notepad++.exe" may appear to
                           work but it's misleading. That first param requires a BSTR not wchar_t *.
                           The underlying type is the same but the data is stored differently.
                           Due to a quirk in the way explorer receives the arguments it may appear
                           to work arbitrarily (or execute something different entirely).
                           Wrong:
                           hr = psd->ShellExecuteW(L"notepad++.exe", vtArgs, vtDir, vtEmpty, vtEmpty);

                           To see all the app paths refer to:
                           HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
                           AppPaths are used by ShellExecute if the command isn't found in the path.

                           Note also for first param that bstrFile it can be a full path to the exe.

                           Note the third parameter can be the directory of bstrFile if it's just a
                           filename. However it can also be the working directory. If the directory
                           is not found or inaccessible then it will crop it to the root.
                           eg curdir of C:\Windows\doesnotexist explorer changes it to curdir C:\
                           */
                        VARIANT vtEmpty = {}; // VT_EMPTY
                        hr = psd->ShellExecuteW(bstrFile, vtArgs, vtDir, vtEmpty, vtEmpty);
                        VariantClear(&vtDir);
                    }
                    VariantClear(&vtArgs);
                }
                SysFreeString(bstrFile);
            }
            psd->Release();
        }
        psv->Release();
    }
    return hr;
}
