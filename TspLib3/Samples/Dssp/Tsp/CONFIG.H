/*****************************************************************************/
//
// CONFIG.H - Digital Switch Service Provider Sample
//                                                                        
// This file contains the configuration support code.
// 
// Copyright (C) 1997 Mark C. Smith
// Copyright (C) 1997 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This service provider talks to a simulated digital switch emulator.  It
// is designed as a sample of how to implement a service provider using
// the TAPI C++ SP class library.  
//
// This source code is only intended as a supplement to the
// documentation provided with the TAPI C++ SP class library.
//
/*****************************************************************************/

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog

class CConfigDlg : public CDialog
{
// Dialog Data
private:
	//{{AFX_DATA(CConfigDlg)
	enum { IDD = IDD_CONFIGURE };
	CString	m_strIPAddress;
	DWORD m_dwProviderID;
	int		m_iTimeout;
	//}}AFX_DATA

// Construction
public:
	CConfigDlg(CWnd* pParent, DWORD dwPPID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeIpaddress();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
