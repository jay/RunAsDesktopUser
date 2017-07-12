
// RunAsDesktopUserDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CRunAsDesktopUserDlg dialog
class CRunAsDesktopUserDlg : public CDialog
{
// Construction
public:
	CRunAsDesktopUserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RUNASDESKTOPUSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CString m_sApp;
	CString m_sCmdLine;
	CString m_sCurrDir;

	afx_msg void OnBnClickedRun();
};
