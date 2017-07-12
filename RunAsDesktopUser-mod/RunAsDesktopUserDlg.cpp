
// RunAsDesktopUserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RunAsDesktopUser.h"
#include "RunAsDesktopUserDlg.h"
#include "RunAsDesktopUser_Utils.h"
#include "RunAsDesktopUser_Implementation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRunAsDesktopUserDlg dialog




CRunAsDesktopUserDlg::CRunAsDesktopUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRunAsDesktopUserDlg::IDD, pParent)
	, m_sApp(_T("C:\\Program Files\\Internet Explorer\\iexplore.exe"))
	, m_sCmdLine(_T(""))
	, m_sCurrDir(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRunAsDesktopUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_App, m_sApp);
	DDX_Text(pDX, IDC_CmdLine, m_sCmdLine);
	DDX_Text(pDX, IDC_CurrDir, m_sCurrDir);
}

BEGIN_MESSAGE_MAP(CRunAsDesktopUserDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_Run, &CRunAsDesktopUserDlg::OnBnClickedRun)
END_MESSAGE_MAP()


// CRunAsDesktopUserDlg message handlers

BOOL CRunAsDesktopUserDlg::OnInitDialog()
{
	CoInitialize(NULL);
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SHAutoComplete(::GetDlgItem(m_hWnd, IDC_App), SHACF_FILESYS_ONLY);
	SHAutoComplete(::GetDlgItem(m_hWnd, IDC_CurrDir), SHACF_FILESYS_DIRS);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRunAsDesktopUserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRunAsDesktopUserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRunAsDesktopUserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRunAsDesktopUserDlg::OnBnClickedRun()
{
	// This demo is all about starting an app as the desktop user from an elevated app.  
	// If this app isn't running elevated, there's no point in doing anything.
	if (!IsUserAnAdmin())
	{
		MessageBoxW(L"N/A:  This demo app is not running elevated.\r\nSo just start the target process directly (left as an exercise to the developer).", NULL, MB_ICONWARNING);
		return;
	}

	// Get the text entered into the dialog
	UpdateData();

	// Build the sCmdLine argument and the other args needed for CreateProcessWithTokenW.
	wstringstream sCmdLine, sErrorInfo;
	sCmdLine << L"\"" << (LPCWSTR)m_sApp << L"\" " << (LPCWSTR)m_sCmdLine;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	SecureZeroMemory(&si, sizeof(si));
	SecureZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	//TODO:  casting sCmdLine.str().c_str() to a non-const is a little sloppy.  You can do better.
	if (RunAsDesktopUser(
		m_sApp, 
		(LPWSTR)sCmdLine.str().c_str(),
		(m_sCurrDir.GetLength() > 0 ? (LPCWSTR)m_sCurrDir : NULL),
		si,
		pi,
		sErrorInfo))
	{
		// Make sure to close HANDLEs return in the PROCESS_INFORMATION.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		// Failed - report what failed.
		MessageBoxW(sErrorInfo.str().c_str(), NULL, MB_ICONSTOP);
	}
}
