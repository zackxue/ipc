// JaAssistDlg.h : header file
//

#if !defined(AFX_JAASSISTDLG_H__2746F4A7_BE85_4CAE_9FE1_41616779894B__INCLUDED_)
#define AFX_JAASSISTDLG_H__2746F4A7_BE85_4CAE_9FE1_41616779894B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>

#include "../jastsession.h"

/////////////////////////////////////////////////////////////////////////////
// CJaAssistDlg dialog

class CJaAssistDlg : public CDialog
{
// jastclient interfaces
public:
	int m_seldev;
public:
	//
	FILE* file_new(const char *name);
	int file_write(FILE *f,char *buf,int size);
	unsigned long file_size(const char *path);
	//
	int JASTC_init();
	int JASTC_start(char *ip,int port);
	int JASTC_stop(JastSession_t *s);
	//
	int DeviceList_update();
	int Output_owner_change(int index);
// Construction
public:
	CJaAssistDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CJaAssistDlg)
	enum { IDD = IDD_JAASSIST_DIALOG };
	CEdit	m_editoutput;
	CListCtrl	m_devicesList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJaAssistDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CJaAssistDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonDiscovery();
	afx_msg void OnButtonConnect();
	afx_msg void OnButtonDisconnect();
	afx_msg void OnDblclkListDevice(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JAASSISTDLG_H__2746F4A7_BE85_4CAE_9FE1_41616779894B__INCLUDED_)
