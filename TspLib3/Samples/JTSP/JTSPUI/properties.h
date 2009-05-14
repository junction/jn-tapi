/***************************************************************************
//
// PROPERTIES.H
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Single object property sheets
//
// Copyright (C) 1998 JulMar Entertainment Technology, Inc.
// All rights reserved
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// Modification History
// ------------------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Initial revision
// 
/***************************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __PROPERTIES_INC__
#define __PROPERTIES_INC__

/**************************************************************************
** CSingleSheet
**
** Property sheet which will hold one of the "properties" pages
**
***************************************************************************/
class CSingleSheet : public CPropertySheet
{
// Class data
protected:
	//{{AFX_DATA(CSingleSheet)
	//}}AFX_DATA

// Construction
public:
	CSingleSheet(CWnd* pwndParent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSingleSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CSingleSheet)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**************************************************************************
** CActivityPropPage
**
** Activity propert page
**
***************************************************************************/
class CActivityPropPage : public CPropertyPage
{
// Dialog Data
public:
	//{{AFX_DATA(CActivityPropPage)
	enum { IDD = IDD_ACTIVITY_PROP };
	CEdit m_edtActivityID;
	CSpinButtonCtrl	m_ctlSpinner;
	CString	m_strName;
	UINT m_nActivityID;
	BOOL m_fExisting;
	//}}AFX_DATA

// Construction
public:
	CActivityPropPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CActivityPropPage)
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CActivityPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**************************************************************************
** CLinePropPage
**
** Line property page
**
***************************************************************************/
class CLinePropPage : public CPropertyPage
{
// Dialog Data
public:
	//{{AFX_DATA(CLinePropPage)
	enum { IDD = IDD_LINE_PROP };
	CEdit m_edtExtension;
	CSpinButtonCtrl	m_ctlSpinner;
	DWORD m_dwExtension;
	CString	m_strName;
	BOOL m_fExisting;
	int	m_iType;
	//}}AFX_DATA

// Construction
public:
	CLinePropPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLinePropPage)
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLinePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**************************************************************************
** CGroupPropPage
**
** Agent group property page
**
***************************************************************************/
class CGroupPropPage : public CPropertyPage
{
// Dialog Data
public:
	//{{AFX_DATA(CGroupPropPage)
	enum { IDD = IDD_GROUP_PROP };
	CEdit m_edtGroupID;
	CSpinButtonCtrl	m_ctlSpinner;
	CString	m_strName;
	DWORD m_dwGroupID;
	BOOL m_fExisting;
	//}}AFX_DATA

// Construction
public:
	CGroupPropPage();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupPropPage)
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGroupPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**************************************************************************
** CPhonePropPage
**
** Phone device property page
**
***************************************************************************/
class CPhonePropPage : public CPropertyPage
{
// Dialog Data
public:
	//{{AFX_DATA(CPhonePropPage)
	enum { IDD = IDD_PHONE_PROP };
	CString	m_strName;
	DWORD m_dwExtension;
	//}}AFX_DATA

// Construction
public:
	CPhonePropPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPhonePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPhonePropPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __PROPERTIES_INC__
