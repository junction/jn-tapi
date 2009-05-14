/***************************************************************************
//
// PROPERTIES.CPP
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

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "JTspUI.h"
#include "properties.h"

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-------------------------------------------------------------------------------*/
// CSingleSheet MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CSingleSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CSingleSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CSingleSheet::CSingleSheet
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the sheet manager
**
*****************************************************************************/
CSingleSheet::CSingleSheet(CWnd* pwndParent) :
	CPropertySheet(IDS_PROPCAPTION, pwndParent, 0)
{
	//{{AFX_DATA_INIT(CSingleSheet)
	//}}AFX_DATA_INIT

}// CSingleSheet::CSingleSheet

/*****************************************************************************
** Procedure:  CSingleSheet::DoDataExchange
**
** Arguments: 'pDX' - Dialog data exchange pointer
**
** Returns:    void
**
** Description: Called by the framework to exchange and validate dialog 
**              data.  This connects windows controls up to class elements
**              in the C++ object.
**
*****************************************************************************/
void CSingleSheet::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSingleSheet)
	//}}AFX_DATA_MAP
}// CSingleSheet::DoDataExchange

/*-------------------------------------------------------------------------------*/
// CActivityPropPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CActivityPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CActivityPropPage)
	ON_EN_CHANGE(IDC_ACTIVITYID, OnChange)
	ON_EN_CHANGE(IDC_ACTIVITY_NAME, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CActivityPropPage::CActivityPropPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the activities property page
**
*****************************************************************************/
CActivityPropPage::CActivityPropPage() : CPropertyPage(CActivityPropPage::IDD)
{
	//{{AFX_DATA_INIT(CActivityPropPage)
	m_strName = _T("");
	m_nActivityID = 0;
	m_fExisting = false;
	//}}AFX_DATA_INIT

}// CActivityPropPage::CActivityPropPage

/*****************************************************************************
** Procedure:  CActivityPropPage::DoDataExchange
**
** Arguments: 'pDX' - Dialog data exchange pointer
**
** Returns:    void
**
** Description: Called by the framework to exchange and validate dialog 
**              data.  This connects windows controls up to class elements
**              in the C++ object.
**
*****************************************************************************/
void CActivityPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActivityPropPage)
	DDX_Control(pDX, IDC_ACTIVITYID, m_edtActivityID);
	DDX_Control(pDX, IDC_SPIN1, m_ctlSpinner);
	DDX_Text(pDX, IDC_ACTIVITY_NAME, m_strName);
	DDX_Text(pDX, IDC_ACTIVITYID, m_nActivityID);
	//}}AFX_DATA_MAP

}// CActivityPropPage::DoDataExchange

/*****************************************************************************
** Procedure:  CGeneralPage::OnInitDialog
**
** Arguments: void
**
** Returns:    TRUE if Windows should manage focus
**
** Description: Called during dialog creation
**
*****************************************************************************/
BOOL CActivityPropPage::OnInitDialog() 
{
	// Connect up our dialog controls
	CPropertyPage::OnInitDialog();

	// Mark whether we have existing data
	if (m_nActivityID > 0)
	{
		m_fExisting = true;
		m_edtActivityID.EnableWindow(FALSE);
		m_ctlSpinner.EnableWindow(FALSE);
	}
	else
	{
		m_edtActivityID.EnableWindow(TRUE);
		m_ctlSpinner.EnableWindow(TRUE);
	}

	// Set our limits
	m_edtActivityID.SetLimitText(4);
	m_ctlSpinner.SetRange(1, 9999);
	reinterpret_cast<CEdit*>(GetDlgItem(IDC_ACTIVITY_NAME))->SetLimitText(20);

	return TRUE;

}// CActivityPropPage::OnInitDialog

/*****************************************************************************
** Procedure:  CActivityPropPage::OnChange
**
** Arguments: void
**
** Returns:    void
**
** Description: This is called when the data in our controls changes
**
*****************************************************************************/
void CActivityPropPage::OnChange() 
{
	if (m_edtActivityID.GetSafeHwnd() != NULL)
	{
		if (UpdateData(TRUE))
		{
			if (m_nActivityID > 0 && m_strName.IsEmpty() == FALSE)
				SetModified(TRUE);
		}
	}

}// CActivityPropPage::OnChange

/*****************************************************************************
** Procedure:  CActivityPropPage::OnKillActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is being destroyed
**
*****************************************************************************/
BOOL CActivityPropPage::OnKillActive() 
{
	// Validate our control data
	if (UpdateData(TRUE))
	{
		if (m_nActivityID == 0 || m_nActivityID > 9999)
		{
			AfxMessageBox(IDS_BADACTIVITYID);
			GotoDlgCtrl(&m_edtActivityID);
			return FALSE;
		}

		if (m_strName.IsEmpty())
		{
			AfxMessageBox(IDS_BADACTIVITYNAME);
			GotoDlgCtrl(GetDlgItem(IDC_ACTIVITY_NAME));
			return FALSE;
		}

		if (!MyDevice()->GetAgentActivityById(m_nActivityID).IsEmpty() && m_fExisting == false)
		{
			AfxMessageBox(IDS_ACTIVITYEXISTS);
			GotoDlgCtrl(&m_edtActivityID);
			return FALSE;
		}
	}
	return CPropertyPage::OnKillActive();

}// CActivityPropPage::OnKillActive

/*-------------------------------------------------------------------------------*/
// CLinePropPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CLinePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CLinePropPage)
	ON_EN_CHANGE(IDC_EXTENSION, OnChange)
	ON_EN_CHANGE(IDC_LINE_NAME, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CLinePropPage::CLinePropPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the line property page
**
*****************************************************************************/
CLinePropPage::CLinePropPage() : CPropertyPage(CLinePropPage::IDD)
{
	//{{AFX_DATA_INIT(CLinePropPage)
	m_dwExtension = 0;
	m_strName = _T("");
	m_iType = 0;
	m_fExisting = FALSE;
	//}}AFX_DATA_INIT

}// CLinePropPage::CLinePropPage

/*****************************************************************************
** Procedure:  CLinePropPage::DoDataExchange
**
** Arguments: 'pDX' - Dialog data exchange pointer
**
** Returns:    void
**
** Description: Called by the framework to exchange and validate dialog 
**              data.  This connects windows controls up to class elements
**              in the C++ object.
**
*****************************************************************************/
void CLinePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLinePropPage)
	DDX_Control(pDX, IDC_EXTENSION, m_edtExtension);
	DDX_Control(pDX, IDC_SPIN1, m_ctlSpinner);
	DDX_Text(pDX, IDC_EXTENSION, m_dwExtension);
	DDX_Text(pDX, IDC_LINE_NAME, m_strName);
	DDX_Radio(pDX, IDC_STATION, m_iType);
	//}}AFX_DATA_MAP

}// CLinePropPage::DoDataExchange

/*****************************************************************************
** Procedure:  CLinePropPage::OnInitDialog
**
** Arguments: void
**
** Returns:    TRUE if Windows should manage focus
**
** Description: Called during dialog creation
**
*****************************************************************************/
BOOL CLinePropPage::OnInitDialog() 
{
	// Connect up our dialog controls
	CPropertyPage::OnInitDialog();

	// Mark whether we have existing data
	if (m_dwExtension > 0)
	{
		m_fExisting = true;
		m_iType--;
		m_edtExtension.EnableWindow(FALSE);
		m_ctlSpinner.EnableWindow(FALSE);
		for (int i = IDC_STATION; i <= IDC_QUEUE; i++)
			GetDlgItem(i)->EnableWindow(FALSE);
	}
	else
	{
		m_edtExtension.EnableWindow(TRUE);
		m_ctlSpinner.EnableWindow(TRUE);
	}

	UpdateData(FALSE);

	// Set our limits
	m_edtExtension.SetLimitText(4);
	m_ctlSpinner.SetRange(1, 9999);
	reinterpret_cast<CEdit*>(GetDlgItem(IDC_LINE_NAME))->SetLimitText(20);

	return TRUE;

}// CLinePropPage::OnInitDialog

/*****************************************************************************
** Procedure:  CLinePropPage::OnChange
**
** Arguments: void
**
** Returns:    void
**
** Description: This is called when the data in our controls changes
**
*****************************************************************************/
void CLinePropPage::OnChange() 
{
	if (m_edtExtension.GetSafeHwnd() != NULL)
	{
		if (UpdateData(TRUE))
		{
			if (m_dwExtension > 0 && m_strName.IsEmpty() == FALSE)
				SetModified(TRUE);
		}
	}

}// CLinePropPage::OnChange

/*****************************************************************************
** Procedure:  CLinePropPage::OnKillActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is being destroyed
**
*****************************************************************************/
BOOL CLinePropPage::OnKillActive() 
{
	CTSPUIDevice* pDevice = MyDevice();

	// Validate our control data
	if (UpdateData(TRUE))
	{
		int iType = m_iType+1;

		// Validate the extension.
		if (iType == CTSPUILineConnection::Trunk || 
			iType == CTSPUILineConnection::PredictiveDialer)
		{
			if (m_dwExtension == 0 || m_dwExtension > 999)
			{
				AfxMessageBox(IDS_BADTRUNKDIALER);
				GotoDlgCtrl(&m_edtExtension);
				return FALSE;
			}
		}
		else if (iType == CTSPUILineConnection::Queue)
		{
			if (m_dwExtension < 8999)
			{
				AfxMessageBox(IDS_BADQUEUE);
				GotoDlgCtrl(&m_edtExtension);
				return FALSE;
			}
		}
		else // Station, VRU, Route Point
		{
			if (m_dwExtension < 1000 || m_dwExtension > 8999)
			{
				AfxMessageBox(IDS_BADSTATION);
				GotoDlgCtrl(&m_edtExtension);
				return FALSE;
			}
		}

		// Make sure it doesn't exist.
		if (!m_fExisting && pDevice->FindLineConnectionByPermanentID(m_dwExtension) != NULL)
		{
			AfxMessageBox(IDS_LINEEXISTS);
			GotoDlgCtrl(&m_edtExtension);
			return FALSE;
		}

		if (m_strName.IsEmpty())
		{
			AfxMessageBox(IDS_BADLINENAME);
			GotoDlgCtrl(GetDlgItem(IDC_LINE_NAME));
			return FALSE;
		}
	}

	// Everythings ok, adjust the type
	m_iType++;
	return CPropertyPage::OnKillActive();

}// CLinePropPage::OnKillActive

/*-------------------------------------------------------------------------------*/
// CGroupPropPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CGroupPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGroupPropPage)
	ON_EN_CHANGE(IDC_GROUPID, OnChange)
	ON_EN_CHANGE(IDC_GROUP_NAME, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CLinePropPage::CLinePropPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the group property page
**
*****************************************************************************/
CGroupPropPage::CGroupPropPage() : CPropertyPage(CGroupPropPage::IDD)
{
	//{{AFX_DATA_INIT(CGroupPropPage)
	m_strName = _T("");
	m_dwGroupID = 0;
	m_fExisting = FALSE;
	//}}AFX_DATA_INIT

}// CGroupPropPage::CGroupPropPage

/*****************************************************************************
** Procedure:  CGroupPropPage::DoDataExchange
**
** Arguments: 'pDX' - Dialog data exchange pointer
**
** Returns:    void
**
** Description: Called by the framework to exchange and validate dialog 
**              data.  This connects windows controls up to class elements
**              in the C++ object.
**
*****************************************************************************/
void CGroupPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupPropPage)
	DDX_Control(pDX, IDC_GROUPID, m_edtGroupID);
	DDX_Control(pDX, IDC_SPIN1, m_ctlSpinner);
	DDX_Text(pDX, IDC_GROUP_NAME, m_strName);
	DDX_Text(pDX, IDC_GROUPID, m_dwGroupID);
	//}}AFX_DATA_MAP

}// CGroupPropPage::DoDataExchange

/*****************************************************************************
** Procedure:  CGroupPropPage::OnInitDialog
**
** Arguments: void
**
** Returns:    TRUE if Windows should manage focus
**
** Description: Called during dialog creation
**
*****************************************************************************/
BOOL CGroupPropPage::OnInitDialog() 
{
	// Connect up our dialog controls
	CPropertyPage::OnInitDialog();

	// Mark whether we have existing data
	if (m_dwGroupID > 0)
	{
		m_fExisting = true;
		m_edtGroupID.EnableWindow(FALSE);
		m_ctlSpinner.EnableWindow(FALSE);
	}
	else
	{
		// Find a suitable group id to assign to this.
		for (m_dwGroupID = 10000; m_dwGroupID < 0xfffffff; m_dwGroupID++)
		{
			if (MyDevice()->GetAgentGroupById(m_dwGroupID).IsEmpty())
				break;
		}
		m_edtGroupID.EnableWindow(TRUE);
		m_ctlSpinner.EnableWindow(TRUE);
		UpdateData(FALSE);
	}

	// Set our limits
	m_edtGroupID.SetLimitText(8);
	m_ctlSpinner.SetRange32(1, 0xffffffff);
	reinterpret_cast<CEdit*>(GetDlgItem(IDC_GROUP_NAME))->SetLimitText(20);

	return TRUE;

}// CGroupPropPage::OnInitDialog

/*****************************************************************************
** Procedure:  CGroupPropPage::OnChange
**
** Arguments: void
**
** Returns:    void
**
** Description: This is called when the data in our controls changes
**
*****************************************************************************/
void CGroupPropPage::OnChange() 
{
	if (m_edtGroupID.GetSafeHwnd() != NULL)
	{
		if (UpdateData(TRUE))
		{
			if (m_dwGroupID > 0 && m_strName.IsEmpty() == FALSE)
				SetModified(TRUE);
		}
	}

}// CGroupPropPage::OnChange

/*****************************************************************************
** Procedure:  CGroupPropPage::OnKillActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is being destroyed
**
*****************************************************************************/
BOOL CGroupPropPage::OnKillActive() 
{
	// Validate our control data
	if (UpdateData(TRUE))
	{
		// Validate the groupid.
		if (m_dwGroupID == 0)
		{
			AfxMessageBox(IDS_BADGROUPID);
			GotoDlgCtrl(&m_edtGroupID);
			return FALSE;
		}

		// Make sure it doesn't exist.
		if (!m_fExisting && !MyDevice()->GetAgentGroupById(m_dwGroupID).IsEmpty())
		{
			AfxMessageBox(IDS_GROUPEXISTS);
			GotoDlgCtrl(&m_edtGroupID);
			return FALSE;
		}

		if (m_strName.IsEmpty())
		{
			AfxMessageBox(IDS_BADGROUPNAME);
			GotoDlgCtrl(GetDlgItem(IDC_LINE_NAME));
			return FALSE;
		}
	}
	return CPropertyPage::OnKillActive();

}// CGroupPropPage::OnKillActive

/*-------------------------------------------------------------------------------*/
// CPhonePropPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CPhonePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPhonePropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CPhonePropPage::CPhonePropPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the phone property page
**
*****************************************************************************/
CPhonePropPage::CPhonePropPage() : CPropertyPage(CPhonePropPage::IDD)
{
	//{{AFX_DATA_INIT(CPhonePropPage)
	m_strName = _T("");
	m_dwExtension = 0;
	//}}AFX_DATA_INIT

}// CPhonePropPage::CPhonePropPage

/*****************************************************************************
** Procedure:  CPhonePropPage::DoDataExchange
**
** Arguments: 'pDX' - Dialog data exchange pointer
**
** Returns:    void
**
** Description: Called by the framework to exchange and validate dialog 
**              data.  This connects windows controls up to class elements
**              in the C++ object.
**
*****************************************************************************/
void CPhonePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhonePropPage)
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_EXTENSION, m_dwExtension);
	//}}AFX_DATA_MAP

}// CPhonePropPage::DoDataExchange

/*****************************************************************************
** Procedure:  CPhonePropPage::OnInitDialog
**
** Arguments: void
**
** Returns:    TRUE if Windows should manage focus
**
** Description: Called during dialog creation
**
*****************************************************************************/
BOOL CPhonePropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CancelToClose();
	return TRUE;

}// CPhonePropPage::OnInitDialog
