/***************************************************************************
//
// CONFIG.H
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// User-Interface main entrypoint
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

#ifndef __CONFIG_INC__
#define __CONFIG_INC__

/**************************************************************************
** CGeneralPage
**
** General page (main sheet shown in configuration)
**
***************************************************************************/
class CGeneralPage : public CPropertyPage
{
// Dialog data
protected:
	//{{AFX_DATA(CGeneralPage)
	enum { IDD = IDD_GENERAL_PAGE };
	CEdit	m_edtPort;
	CSpinButtonCtrl	m_ctlSpin;
	CIPAddressCtrl	m_ctlServerIP;
	//}}AFX_DATA

// Construction
public:
	CGeneralPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGeneralPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGeneralPage)
	afx_msg void OnAutoConfig();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**************************************************************************
** CLinePage
**
** Line display page
**
***************************************************************************/
class CLinePage : public CPropertyPage
{
// Dialog Data
protected:
	//{{AFX_DATA(CLinePage)
	enum { IDD = IDD_LINE_PAGE };
	CButton	m_btnRemove;
	CButton	m_btnProperties;
	CListCtrl m_lcLines;
	//}}AFX_DATA

// Construction
public:
	CLinePage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLinePage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLinePage)
	afx_msg void OnAdd();
	afx_msg void OnProperties();
	afx_msg void OnRemove();
	afx_msg void OnChangedLine(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnGetDispInfoLines(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Internal functions
private:
	void LoadLines();
};

/**************************************************************************
** CGroupPage
**
** Agent Group property page
**
***************************************************************************/
class CGroupPage : public CPropertyPage
{
// Dialog Data
protected:
	//{{AFX_DATA(CGroupPage)
	enum { IDD = IDD_GROUP_PAGE };
	CButton	m_btnRemove;
	CButton	m_btnProperties;
	CListCtrl m_lcGroups;
	//}}AFX_DATA

// Construction
public:
	CGroupPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGroupPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGroupPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnGetDispInfoGroup(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedGroup(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProperties();
	afx_msg void OnRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Internal functions
private:
	void LoadGroups();
};

/**************************************************************************
** CActivityPage
**
** Agent Activity property page
**
***************************************************************************/
class CActivityPage : public CPropertyPage
{
// Dialog Data
	//{{AFX_DATA(CActivityPage)
	enum { IDD = IDD_ACTIVITY_PAGE };
	CButton	m_btnRemove;
	CButton	m_btnProperties;
	CListCtrl m_lcActivity;
	//}}AFX_DATA

// Construction
public:
	CActivityPage();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CActivityPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CActivityPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnGetDispInfoActivity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedActivity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProperties();
	afx_msg void OnRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Internal functions
private:
	void LoadActivity();
};

/**************************************************************************
** CConfigSheet
**
** Configuration sheet (main property sheet)
**
***************************************************************************/
class CConfigSheet : public CPropertySheet
{
// Class data
protected:
	//{{AFX_DATA(CConfigSheet)
	CGeneralPage m_pageGeneral;
	CLinePage m_pageLines;
	CGroupPage m_pageGroups;
	CActivityPage m_pageActivity;
	//}}AFX_DATA

// Construction
public:
	CConfigSheet(CWnd* pwndParent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CConfigSheet)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __CONFIG_INC__

