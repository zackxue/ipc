// JaAssist.h : main header file for the JAASSIST application
//

#if !defined(AFX_JAASSIST_H__A1B479A7_E6A7_4B41_B50B_3E8CD6F7D507__INCLUDED_)
#define AFX_JAASSIST_H__A1B479A7_E6A7_4B41_B50B_3E8CD6F7D507__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJaAssistApp:
// See JaAssist.cpp for the implementation of this class
//

class CJaAssistApp : public CWinApp
{
public:
	CJaAssistApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJaAssistApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJaAssistApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JAASSIST_H__A1B479A7_E6A7_4B41_B50B_3E8CD6F7D507__INCLUDED_)
