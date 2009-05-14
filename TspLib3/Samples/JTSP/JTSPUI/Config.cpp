/***************************************************************************
//
// CONFIG.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Main configuration panels
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
#include "config.h"
#include "properties.h"
#include "ddx.h"

using namespace tsplibui;

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*------------------------------------------------------------------------------
// LISTVIEW COLUMN STRUCTURE
// This structure defines the column information which is setup for a listview 
// control on any of our dialogs
--------------------------------------------------------------------------------*/
typedef struct
{
	UINT uidName;		// Resource for name
	int iWidth;			// Size of the column in characters

} LCCOLUMNS;

/*-------------------------------------------------------------------------------*/
// GLOBAL DATA
/*-------------------------------------------------------------------------------*/
LPCTSTR g_pszAddress0 = _T("Address 0");

LCCOLUMNS g_lcLines[] = {
	{ IDS_LINEID,	6  },	// Line ID
	{ IDS_TYPE,		10 },	// Line Type
	{ IDS_NAME,		20 },	// Line Name
	{ 0, 0 },				// End
};

LCCOLUMNS g_lcGroups[] = {
	{ IDS_GROUPID,	9  },	// Group ID
	{ IDS_NAME,		20 },	// Group Name
	{ 0, 0 },				// End
};

LCCOLUMNS g_lcActivity[] = {
	{ IDS_ACTID,	9  },	// Activity ID
	{ IDS_NAME,		20 },	// Activity Name
	{ 0, 0 },				// End
};

/*****************************************************************************
** Procedure:  SetupAddress
**
** Arguments:  'pLine' - Line to set address up for
**
** Returns:    void
**
** Description: Adds a single address to the line
**
*****************************************************************************/
inline void SetupAddress(CTSPUILineConnection* pLine)
{
	// Setup our dialparameters for the PBX.
	LINEDIALPARAMS DialParams;
	DialParams.dwDialPause = 50;
	DialParams.dwDialSpeed = 50;
	DialParams.dwDigitDuration = 50;
	DialParams.dwWaitForDialtone = 0;

	TCHAR chBuff[20];
	wsprintf(chBuff, _T("%04d"), pLine->GetPermanentDeviceID());

	switch (pLine->GetLineType())
	{
		case CTSPUILineConnection::Station:
			pLine->EnableAgentSupport(true);
			pLine->CreateAddress(chBuff, g_pszAddress0, true, true, 
					LINEMEDIAMODE_INTERACTIVEVOICE, LINEBEARERMODE_VOICE,
					0, 0, &DialParams, 1, 3, 3, 3, 1);
			break;
		case CTSPUILineConnection::VRU:
			pLine->CreateAddress(chBuff, g_pszAddress0, true, false, 
					LINEMEDIAMODE_AUTOMATEDVOICE, LINEBEARERMODE_VOICE,
					0, 0, &DialParams, 1, 3, 3, 3, 1);
			break;
		case CTSPUILineConnection::Trunk:
			pLine->CreateAddress(chBuff, g_pszAddress0, true, false, 
					LINEMEDIAMODE_INTERACTIVEVOICE | LINEMEDIAMODE_AUTOMATEDVOICE, 
					LINEBEARERMODE_VOICE,
					0, 0, &DialParams, 1, 1, 1, 0, 0);
			break;
		case CTSPUILineConnection::RoutePoint:
		case CTSPUILineConnection::Queue:
			pLine->CreateAddress(chBuff, g_pszAddress0, true, false, 
					LINEMEDIAMODE_INTERACTIVEVOICE | LINEMEDIAMODE_AUTOMATEDVOICE, 
					LINEBEARERMODE_VOICE,
					0, 0, &DialParams, 0, 65535, 65535, 0, 0);
			break;
		case CTSPUILineConnection::PredictiveDialer:
			pLine->CreateAddress(chBuff, g_pszAddress0, false, true,
					LINEMEDIAMODE_INTERACTIVEVOICE | LINEMEDIAMODE_AUTOMATEDVOICE, 
					LINEBEARERMODE_VOICE,
					0, 0, &DialParams, 1, 0, 0, 0, 0);
			break;
		default: ASSERT(FALSE);
	}

}// SetupAddress

/*****************************************************************************
** Procedure:  _LVLineSort
**
** Arguments:  'lParam1' - First parameter
**             'lParam2' - Second parameter
**             'lParam3' - Item data passed to SortItem
**
** Returns:    void
**
** Description: Sort routine for the line listview
**
*****************************************************************************/
int CALLBACK _LVLineSort(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CTSPUILineConnection* pLine1 = reinterpret_cast<CTSPUILineConnection*>(lParam1);
	CTSPUILineConnection* pLine2 = reinterpret_cast<CTSPUILineConnection*>(lParam2);
	if (pLine1->GetPermanentDeviceID() < pLine2->GetPermanentDeviceID())
		return -1;
	else if (pLine1->GetPermanentDeviceID() > pLine2->GetPermanentDeviceID())
		return 1;
	return 0;

}// _LVLineSort

/*****************************************************************************
** Procedure:  _LVGroupSort
**
** Arguments:  'lParam1' - First parameter
**             'lParam2' - Second parameter
**             'lParam3' - Item data passed to SortItem
**
** Returns:    void
**
** Description: Sort routine for the group listview
**
*****************************************************************************/
int CALLBACK _LVGroupSort(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	TAgentGroup* pGroup1 = reinterpret_cast<TAgentGroup*>(lParam1);
	TAgentGroup* pGroup2 = reinterpret_cast<TAgentGroup*>(lParam2);
	if (pGroup1->GroupID.dwGroupID1 < pGroup2->GroupID.dwGroupID1)
		return -1;
	else if (pGroup1->GroupID.dwGroupID1 > pGroup2->GroupID.dwGroupID1)
		return 1;
	return 0;

}// _LVGroupSort

/*****************************************************************************
** Procedure:  _LVActivitySort
**
** Arguments:  'lParam1' - First parameter
**             'lParam2' - Second parameter
**             'lParam3' - Item data passed to SortItem
**
** Returns:    void
**
** Description: Sort routine for the activity listview
**
*****************************************************************************/
int CALLBACK _LVActivitySort(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	TAgentActivity* pActivity1 = reinterpret_cast<TAgentActivity*>(lParam1);
	TAgentActivity* pActivity2 = reinterpret_cast<TAgentActivity*>(lParam2);
	if (pActivity1->dwID < pActivity2->dwID)
		return -1;
	else if (pActivity1->dwID > pActivity2->dwID)
		return 1;
	return 0;

}// _LVActivitySort

/*-------------------------------------------------------------------------------*/
// CConfigSheet MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CConfigSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CConfigSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CConfigSheet::CConfigSheet
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the sheet manager
**
*****************************************************************************/
CConfigSheet::CConfigSheet(CWnd* pwndParent) : CPropertySheet(IDS_CAPTION, pwndParent, 0)
{
	//{{AFX_DATA_INIT(CConfigSheet)
	//}}AFX_DATA_INIT

	// Add each of the property pages
	AddPage(&m_pageGeneral);
	AddPage(&m_pageLines);
	AddPage(&m_pageGroups);
	AddPage(&m_pageActivity);

}// CConfigSheet::CConfigSheet

/*****************************************************************************
** Procedure:  CConfigSheet::DoDataExchange
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
void CConfigSheet::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigSheet)
	//}}AFX_DATA_MAP
}// CConfigSheet::DoDataExchange

/*-------------------------------------------------------------------------------*/
// CGeneralPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGeneralPage)
	ON_BN_CLICKED(IDC_AUTOCONFIG, OnAutoConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CGeneralPage::CGeneralPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the general property page
**
*****************************************************************************/
CGeneralPage::CGeneralPage() : CPropertyPage(CGeneralPage::IDD)
{
	//{{AFX_DATA_INIT(CGeneralPage)
	//}}AFX_DATA_INIT

}// CGeneralPage::CGeneralPage

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
BOOL CGeneralPage::OnInitDialog() 
{
	// Connect up our dialog controls
	CPropertyPage::OnInitDialog();
	
	// Set our limits
	m_edtPort.SetLimitText(4);
	m_ctlSpin.SetRange(0, 9999);

	return TRUE;

}// CGeneralPage::OnInitDialog

/*****************************************************************************
** Procedure:  CGeneralPage::DoDataExchange
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
void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralPage)
	DDX_Control(pDX, IDC_PORT, m_edtPort);
	DDX_Control(pDX, IDC_SPIN1, m_ctlSpin);
	DDX_Control(pDX, IDC_SERVERIPADDRESS, m_ctlServerIP);
	//}}AFX_DATA_MAP

	// We should always have a device!
	ASSERT(GetUISP()->GetDeviceCount() > 0);

	// Transfer the data directly to/from our device object.
	CJTSPDevice* pDevice = dynamic_cast<CJTSPDevice*>(MyDevice());

	DDX_Text(pDX, IDC_PORT, pDevice->IPPort());
	DDV_MinMaxInt(pDX, pDevice->IPPort(), 1, 9999);
	DDX_IPText(pDX, IDC_SERVERIPADDRESS, pDevice->IPAddress());

}// CGeneralPage::DoDataExchange

/*****************************************************************************
** Procedure:  CGeneralPage::OnAutoConfig
**
** Arguments: void
**
** Returns:    void
**
** Description: This is called to auto-configure the TSP using a data
**              file written by the PBX simulator.  Many PBX/ACD systems will
**              write a text file of the configuration through a service port
**              prodding.
**
*****************************************************************************/
void CGeneralPage::OnAutoConfig() 
{
	CTSPUIDevice* pDevice = MyDevice();

	// If we have existing information, warn the user.
	if (pDevice->GetLineCount() > 0 ||
		pDevice->GetAgentGroupCount() > 0 ||
		pDevice->GetAgentActivityCount() > 0)
	{
		if (AfxMessageBox(IDS_REMOVEALL, MB_YESNO) == IDNO)
			return;
	}

	// Prompt for a filename
	static TCHAR BASED_CODE szFilter[] = _T("Config Files (*.cfg)|*.cfg|All Files (*.*)|*.*||");
	CFileDialog fd(TRUE, _T("CFG"), _T("JPBX.CFG"), 
			OFN_ENABLESIZING | OFN_EXPLORER | OFN_PATHMUSTEXIST,
			szFilter, AfxGetMainWnd());

	// Show the dialog and if the user clicks OK, open the file and read the 
	// configuration directly.
	if (fd.DoModal() == IDOK)
	{
		CStdioFile fileCfg;
		if (fileCfg.Open(fd.GetPathName(), (CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText), NULL))
		{
			// Dump all the existing configuration.
			MyDevice()->ResetConfiguration();

			// Read each line and parse all the information from it.
			CString strLine;
			int iStep = 0;
			while (fileCfg.ReadString(strLine))
			{
				strLine.TrimRight();
				if (strLine.Left(7) == _T("GROUPS:"))
					iStep = 1;
				else if (strLine.Left(8) == _T("DEVICES:"))
					iStep = 2;
				else if (strLine.IsEmpty())
					iStep = 0;

				// If we are loading groups
				else if (iStep == 1)
				{
					int iPos = strLine.Find(_T(','));
					if (iPos <= 0)
						iStep = 0;
					else
					{
						DWORD dwGroupID;
						dwGroupID = xtoi(strLine.Left(iPos));
						if (dwGroupID > 0)
							MyDevice()->AddAgentGroup(strLine.Mid(iPos+1), dwGroupID);
					}
				}

				// Else if we are loading devices (lines)
				else if (iStep == 2)
				{
					int iPos = strLine.Find(_T(','));
					if (iPos <= 0)
						iStep = 0;
					else
					{
						CString strType = strLine.Left(iPos);
						strLine = strLine.Mid(iPos+1);
						iPos = strLine.Find(_T(','));
						if (iPos > 0)
						{
							DWORD dwLineID = _ttol(strLine.Left(iPos));
							int iType = CTSPUILineConnection::Station;
							switch (strType[0])
							{
								case 'Q': iType = CTSPUILineConnection::Queue; break;
								case 'R': iType = CTSPUILineConnection::RoutePoint; break;
								case 'P': iType = CTSPUILineConnection::PredictiveDialer; break;
								case 'T': iType = CTSPUILineConnection::Trunk; break;
							}

							CTSPUILineConnection* pLine = new CTSPUILineConnection(dwLineID, iType, strLine.Mid(iPos+1));
							MyDevice()->AddLine(pLine);

							// Based on the type, add an address.
							SetupAddress(pLine);

							// If this is a station, add a phone device to accompany it.
							if (iType == CTSPUILineConnection::Station)
							{
								MyDevice()->AddPhone(new CJTSPPhone(dwLineID, strLine.Mid(iPos+1)));
								pLine->SetAssociatedPhone(dwLineID);
							}
						}
					}
				}
			}

			// Close the file and exit.
			fileCfg.Close();

			CString strBuff;
			strBuff.LoadString(IDS_COUNT);
			strLine.Format(strBuff, 
							MyDevice()->GetLineCount(), 
							MyDevice()->GetPhoneCount(), 
							MyDevice()->GetAgentGroupCount());
			AfxMessageBox(strLine);
		}
		else
			AfxMessageBox(IDS_AUTOLOADERR);
	}

}// CGeneralPage::OnAutoConfig

/*-------------------------------------------------------------------------------*/
// CLinePage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CLinePage, CPropertyPage)
	//{{AFX_MSG_MAP(CLinePage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LINES, OnChangedLine)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LINES, OnGetDispInfoLines)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CLinePage::CLinePage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the line property page
**
*****************************************************************************/
CLinePage::CLinePage() : CPropertyPage(CLinePage::IDD)
{
	//{{AFX_DATA_INIT(CLinePage)
	//}}AFX_DATA_INIT
}// CLinePage::CLinePage

/*****************************************************************************
** Procedure:  CLinePage::OnInitDialog
**
** Arguments:  void
**
** Returns:    TRUE/FALSE whether Windows should set focus to first control
**
** Description: This is called by Windows before the dialog is shown on the
**              screen.
**
*****************************************************************************/
BOOL CLinePage::OnInitDialog() 
{
	// Connect controls
	CPropertyPage::OnInitDialog();
	
	// Get the current font properties
	CDC* pDC = GetDC();
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	ReleaseDC(pDC);

	// Load our listview columns
	for (int i = 0; g_lcLines[i].uidName != 0; i++)
	{
		CString strName;
		VERIFY(strName.LoadString(g_lcLines[i].uidName));
		m_lcLines.InsertColumn (i, strName, LVCFMT_LEFT, 
			tm.tmAveCharWidth*(g_lcLines[i].iWidth+1));
	}

	// Load all the line information from the provider data structures
	LoadLines();

	return TRUE;

}// CLinePage::OnInitDialog

/*****************************************************************************
** Procedure:  CLinePage::DoDataExchange
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
void CLinePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLinePage)
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_PROPERTIES, m_btnProperties);
	DDX_Control(pDX, IDC_LINES, m_lcLines);
	//}}AFX_DATA_MAP

}// CLinePage::DoDataExchange

/*****************************************************************************
** Procedure:  CLinePage::LoadLines
**
** Arguments: void
**
** Returns:    void
**
** Description: Loads all the line information from the service provider
**
*****************************************************************************/
void CLinePage::LoadLines()
{
	// Remove all the existing entries
	m_lcLines.DeleteAllItems();

	// Build our basic insertion structure
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	lvItem.cchTextMax = 0;

	// Now walk through the service provider data structures and load
	// each of the line data structures
	int iLines = MyDevice()->GetLineCount();
	for (int i = 0; i < iLines; i++)
	{
		CTSPUILineConnection* pLine = MyDevice()->GetLineConnectionInfo(i);
		ASSERT (pLine != NULL);
		lvItem.iItem = m_lcLines.GetItemCount();
		lvItem.lParam = reinterpret_cast<LPARAM>(pLine);
		m_lcLines.InsertItem (&lvItem);
	}

	m_lcLines.SortItems(_LVLineSort, 0);

	m_btnRemove.EnableWindow(FALSE);
	m_btnProperties.EnableWindow(FALSE);

}// CLinePage::LoadLines

/*****************************************************************************
** Procedure:  CLinePage::OnGetDispInfoLines
**
** Arguments: 'pNMHDR' - LV_DISPINFO structure
**            'pResult' - Returning result value
**
** Returns:    void
**
** Description: Called by the listview control to show data
**
*****************************************************************************/
void CLinePage::OnGetDispInfoLines(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	CTSPUILineConnection* pLine = reinterpret_cast<CTSPUILineConnection*>(pItem->lParam);

	static TCHAR szBuffer[257];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
			case 0:	// Line ID
				wsprintf(szBuffer, _T("%04d"), pLine->GetPermanentDeviceID());
				break;
			case 1: // Line Type
				switch (pLine->GetLineType())
				{
					case CTSPUILineConnection::Station:
						lstrcpy(szBuffer, _T("Station"));
						break;
					case CTSPUILineConnection::Queue:
						lstrcpy(szBuffer, _T("Queue"));
						break;
					case CTSPUILineConnection::RoutePoint:
						lstrcpy(szBuffer, _T("Route Point"));
						break;
					case CTSPUILineConnection::PredictiveDialer:
						lstrcpy(szBuffer, _T("Dialer"));
						break;
					case CTSPUILineConnection::VRU:
						lstrcpy(szBuffer, _T("IVR Unit"));
						break;
					case CTSPUILineConnection::Trunk:
						lstrcpy(szBuffer, _T("Trunk"));
						break;
				}
				break;
			case 2:	// Line Name
				lstrcpy(szBuffer, pLine->GetName());
				break;
		}
	}

	pItem->pszText = szBuffer;
	*pResult = 0;

}// CLinePage::OnGetDispInfoLines

/*****************************************************************************
** Procedure:  CLinePage::OnAdd
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to add a new line into the system
**
*****************************************************************************/
void CLinePage::OnAdd() 
{
	CSingleSheet cSheet(GetParent());
	CLinePropPage linePage;
	cSheet.AddPage(&linePage);

	if (cSheet.DoModal() == IDOK)
	{
		// Create the new line.
		CTSPUILineConnection* pLine = new CTSPUILineConnection(
			linePage.m_dwExtension, linePage.m_iType, linePage.m_strName);

		// Add the line to the device list
		MyDevice()->AddLine(pLine);
		SetupAddress(pLine);
		LoadLines();

		// If this is a station, add a phone device
		if (linePage.m_iType == CTSPUILineConnection::Station)
		{
			MyDevice()->AddPhone(new CJTSPPhone(linePage.m_dwExtension, linePage.m_strName));
			pLine->SetAssociatedPhone(linePage.m_dwExtension);
		}
	}

}// CLinePage::OnAdd

/*****************************************************************************
** Procedure:  CLinePage::OnProperties
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to show the properties for a line
**
*****************************************************************************/
void CLinePage::OnProperties() 
{
	CTSPUILineConnection* pLine = NULL;
	for (int i = 0; i < m_lcLines.GetItemCount(); i++)
	{
		if (m_lcLines.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pLine = reinterpret_cast<CTSPUILineConnection*>(m_lcLines.GetItemData(i));
			break;
		}
	}

	// If we didn't find a line, exit
	if (pLine == NULL)
		return;

	CSingleSheet cSheet(GetParent());
	CLinePropPage linePage;
	cSheet.AddPage(&linePage);

	linePage.m_dwExtension = pLine->GetPermanentDeviceID();
	linePage.m_strName = pLine->GetName();
	linePage.m_iType = pLine->GetLineType();

	if (cSheet.DoModal() == IDOK)
	{
		pLine->SetName(linePage.m_strName);
		m_lcLines.RedrawItems(i,i);
	}

}// CLinePage::OnProperties

/*****************************************************************************
** Procedure:  CLinePage::OnRemove
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to remove a line from the configuration
**
*****************************************************************************/
void CLinePage::OnRemove() 
{
	CTSPUILineConnection* pLine = NULL;
	for (int i = 0; i < m_lcLines.GetItemCount(); i++)
	{
		if (m_lcLines.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pLine = reinterpret_cast<CTSPUILineConnection*>(m_lcLines.GetItemData(i));
			break;
		}
	}

	// If we didn't find a line, exit
	if (pLine == NULL)
		return;

	// Make sure the user wants to remove the line from the configuration.
	if (AfxMessageBox(IDS_REMOVELINE, MB_YESNO) == IDYES)
	{
		// Get the associated phone (if any)
		CTSPUIPhoneConnection* pPhone = pLine->GetAssociatedPhone();
		if (pPhone != NULL)
		{
			MyDevice()->RemovePhone(pPhone);
			delete pPhone;
		}

		// Delete the line
		MyDevice()->RemoveLine(pLine);
		delete pLine;
		m_lcLines.DeleteItem(i);
	}

}// CLinePage::OnRemove

/*****************************************************************************
** Procedure:  CLinePage::OnChangedLine
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework when the list control is changing a line
**
*****************************************************************************/
void CLinePage::OnChangedLine(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if ((pNMListView->uChanged & LVIF_STATE) == LVIF_STATE)
	{
		if ((pNMListView->uNewState & LVIS_FOCUSED) == LVIS_FOCUSED)
		{
			m_btnRemove.EnableWindow(TRUE);
			m_btnProperties.EnableWindow(TRUE);
		}
		else
		{
			m_btnRemove.EnableWindow(FALSE);
			m_btnProperties.EnableWindow(FALSE);
		}
	}
	
	*pResult = 0;

}// CLinePage::OnChangedLine

/*****************************************************************************
** Procedure:  CLinePage::OnSetActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is made active
**
*****************************************************************************/
BOOL CLinePage::OnSetActive() 
{
	LoadLines();
	return CPropertyPage::OnSetActive();

}// CLinePage::OnSetActive

/*-------------------------------------------------------------------------------*/
// CGroupPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CGroupPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGroupPage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_GROUPS, OnGetDispInfoGroup)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GROUPS, OnChangedGroup)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CGroupPage::CGroupPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the group property page
**
*****************************************************************************/
CGroupPage::CGroupPage() : CPropertyPage(CGroupPage::IDD)
{
	//{{AFX_DATA_INIT(CGroupPage)
	//}}AFX_DATA_INIT

}// CGroupPage::CGroupPage

/*****************************************************************************
** Procedure:  CGroupPage::DoDataExchange
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
void CGroupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupPage)
	DDX_Control(pDX, IDC_GROUPS, m_lcGroups);
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_PROPERTIES, m_btnProperties);
	//}}AFX_DATA_MAP

}// CGroupPage::DoDataExchange

/*****************************************************************************
** Procedure:  CGroupPage::OnInitDialog
**
** Arguments:  void
**
** Returns:    TRUE/FALSE whether Windows should set focus to first control
**
** Description: This is called by Windows before the dialog is shown on the
**              screen.
**
*****************************************************************************/
BOOL CGroupPage::OnInitDialog() 
{
	// Connect controls
	CPropertyPage::OnInitDialog();
	
	// Get the current font properties
	CDC* pDC = GetDC();
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	ReleaseDC(pDC);

	// Load our listview columns
	for (int i = 0; g_lcGroups[i].uidName != 0; i++)
	{
		CString strName;
		VERIFY(strName.LoadString(g_lcGroups[i].uidName));
		m_lcGroups.InsertColumn (i, strName, LVCFMT_LEFT, 
			tm.tmAveCharWidth*(g_lcGroups[i].iWidth+1));
	}

	// Load all the line information from the provider data structures
	LoadGroups();

	return TRUE;

}// CGroupPage::OnInitDialog

/*****************************************************************************
** Procedure:  CGroupPage::LoadGroups
**
** Arguments: void
**
** Returns:    void
**
** Description: Loads all the group information from the service provider
**
*****************************************************************************/
void CGroupPage::LoadGroups()
{
	// Remove all the existing entries
	m_lcGroups.DeleteAllItems();

	// Build our basic insertion structure
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	lvItem.cchTextMax = 0;

	// Now walk through the service provider data structures and load
	// each of the line data structures
	int iGroups = MyDevice()->GetAgentGroupCount();
	for (int i = 0; i < iGroups; i++)
	{
		TAgentGroup* pGroup = MyDevice()->GetAgentGroup(i);
		ASSERT (pGroup != NULL);
		lvItem.iItem = m_lcGroups.GetItemCount();
		lvItem.lParam = reinterpret_cast<LPARAM>(pGroup);
		m_lcGroups.InsertItem (&lvItem);
	}

	m_lcGroups.SortItems(_LVGroupSort, 0);

	m_btnRemove.EnableWindow(FALSE);
	m_btnProperties.EnableWindow(FALSE);

}// CLinePage::LoadLines

/*****************************************************************************
** Procedure:  CGroupPage::OnAdd
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to add a new group into the system
**
*****************************************************************************/
void CGroupPage::OnAdd() 
{
	CSingleSheet cSheet(GetParent());
	CGroupPropPage grpPage;
	cSheet.AddPage(&grpPage);

	if (cSheet.DoModal() == IDOK)
	{
		if (GetUISP()->GetDevice(0)->AddAgentGroup(grpPage.m_strName, grpPage.m_dwGroupID) >= 0)
			LoadGroups();
	}

}// CGroupPage::OnAdd

/*****************************************************************************
** Procedure:  CGroupPage::OnGetDispInfoGroup
**
** Arguments: 'pNMHDR' - LV_DISPINFO structure
**            'pResult' - Returning result value
**
** Returns:    void
**
** Description: Called by the listview control to show data
**
*****************************************************************************/
void CGroupPage::OnGetDispInfoGroup(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	TAgentGroup* pGroup = reinterpret_cast<TAgentGroup*>(pItem->lParam);

	static TCHAR szBuffer[257];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
			case 0:	// Group ID
				wsprintf(szBuffer, _T("%08X"), pGroup->GroupID.dwGroupID1);
				break;
			case 1:	// Group Name
				lstrcpy(szBuffer, pGroup->strName);
				break;
		}
	}

	pItem->pszText = szBuffer;
	*pResult = 0;

}// CGroupPage::OnGetDispInfoGroup

/*****************************************************************************
** Procedure:  CGroupPage::OnChangedGroup
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework when the list control is changing a group
**
*****************************************************************************/
void CGroupPage::OnChangedGroup(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if ((pNMListView->uChanged & LVIF_STATE) == LVIF_STATE)
	{
		if ((pNMListView->uNewState & LVIS_FOCUSED) == LVIS_FOCUSED)
		{
			m_btnRemove.EnableWindow(TRUE);
			m_btnProperties.EnableWindow(TRUE);
		}
		else
		{
			m_btnRemove.EnableWindow(FALSE);
			m_btnProperties.EnableWindow(FALSE);
		}
	}
	*pResult = 0;

}// CGroupPage::OnChangedGroup

/*****************************************************************************
** Procedure:  CGroupPage::OnProperties
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to show the properties for a group
**
*****************************************************************************/
void CGroupPage::OnProperties() 
{
	TAgentGroup* pGroup = NULL;
	for (int i = 0; i < m_lcGroups.GetItemCount(); i++)
	{
		if (m_lcGroups.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pGroup = reinterpret_cast<TAgentGroup*>(m_lcGroups.GetItemData(i));
			break;
		}
	}

	// If we didn't find a group, exit
	if (pGroup == NULL)
		return;

	CSingleSheet cSheet(GetParent());
	CGroupPropPage grpPage;
	cSheet.AddPage(&grpPage);

	grpPage.m_dwGroupID = pGroup->GroupID.dwGroupID1;
	grpPage.m_strName = pGroup->strName;

	if (cSheet.DoModal() == IDOK)
	{
		pGroup->strName = grpPage.m_strName;
		m_lcGroups.RedrawItems(i,i);
	}

}// CGroupPage::OnProperties

/*****************************************************************************
** Procedure:  CGroupPage::OnRemove
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to remove a group from the configuration
**
*****************************************************************************/
void CGroupPage::OnRemove() 
{
	TAgentGroup* pGroup = NULL;
	for (int i = 0; i < m_lcGroups.GetItemCount(); i++)
	{
		if (m_lcGroups.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pGroup = reinterpret_cast<TAgentGroup*>(m_lcGroups.GetItemData(i));
			break;
		}
	}

	// If we didn't find a group, exit
	if (pGroup == NULL)
		return;

	// Make sure the user wants to remove the group from the configuration.
	if (AfxMessageBox(IDS_REMOVEGROUP, MB_YESNO) == IDYES)
	{
		MyDevice()->RemoveAgentGroup(pGroup->GroupID.dwGroupID1);
		delete pGroup;
		m_lcGroups.DeleteItem(i);
	}

}// CGroupPage::OnRemove

/*****************************************************************************
** Procedure:  CGroupPage::OnSetActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is made active
**
*****************************************************************************/
BOOL CGroupPage::OnSetActive() 
{
	LoadGroups();
	return CPropertyPage::OnSetActive();

}// CGroupPage::OnSetActive

/*-------------------------------------------------------------------------------*/
// CActivityPage MFC Message map
/*-------------------------------------------------------------------------------*/
BEGIN_MESSAGE_MAP(CActivityPage, CPropertyPage)
	//{{AFX_MSG_MAP(CActivityPage)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_ACTIVITY, OnGetDispInfoActivity)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ACTIVITY, OnChangedActivity)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************************
** Procedure:  CActivityPage::CActivityPage
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the Activity property page
**
*****************************************************************************/
CActivityPage::CActivityPage() : CPropertyPage(CActivityPage::IDD)
{
	//{{AFX_DATA_INIT(CActivityPage)
	//}}AFX_DATA_INIT

}// CActivityPage::CActivityPage

/*****************************************************************************
** Procedure:  CActivityPage::DoDataExchange
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
void CActivityPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActivityPage)
	DDX_Control(pDX, IDC_ACTIVITY, m_lcActivity);
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_PROPERTIES, m_btnProperties);
	//}}AFX_DATA_MAP

}// CActivityPage::DoDataExchange

/*****************************************************************************
** Procedure:  CActivityPage::OnInitDialog
**
** Arguments:  void
**
** Returns:    TRUE/FALSE whether Windows should set focus to first control
**
** Description: This is called by Windows before the dialog is shown on the
**              screen.
**
*****************************************************************************/
BOOL CActivityPage::OnInitDialog() 
{
	// Connect controls
	CPropertyPage::OnInitDialog();
	
	// Get the current font properties
	CDC* pDC = GetDC();
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	ReleaseDC(pDC);

	// Load our listview columns
	for (int i = 0; g_lcActivity[i].uidName != 0; i++)
	{
		CString strName;
		VERIFY(strName.LoadString(g_lcActivity[i].uidName));
		m_lcActivity.InsertColumn (i, strName, LVCFMT_LEFT, 
			tm.tmAveCharWidth*(g_lcActivity[i].iWidth+1));
	}

	// Load all the line information from the provider data structures
	LoadActivity();

	return TRUE;

}// CActivityPage::OnInitDialog

/*****************************************************************************
** Procedure:  CActivityPage::LoadActivity
**
** Arguments: void
**
** Returns:    void
**
** Description: Loads all the Activity information from the service provider
**
*****************************************************************************/
void CActivityPage::LoadActivity()
{
	// Remove all the existing entries
	m_lcActivity.DeleteAllItems();

	// Build our basic insertion structure
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	lvItem.cchTextMax = 0;

	// Now walk through the service provider data structures and load
	// each of the line data structures
	int iActivity = MyDevice()->GetAgentActivityCount();
	for (int i = 0; i < iActivity; i++)
	{
		TAgentActivity* pActivity = MyDevice()->GetAgentActivity(i);
		ASSERT (pActivity != NULL);
		lvItem.iItem = m_lcActivity.GetItemCount();
		lvItem.lParam = reinterpret_cast<LPARAM>(pActivity);
		m_lcActivity.InsertItem (&lvItem);
	}

	m_lcActivity.SortItems(_LVActivitySort, 0);

	m_btnRemove.EnableWindow(FALSE);
	m_btnProperties.EnableWindow(FALSE);

}// CLinePage::LoadActivity

/*****************************************************************************
** Procedure:  CActivityPage::OnAdd
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to add a new Activity into the system
**
*****************************************************************************/
void CActivityPage::OnAdd() 
{
	CSingleSheet cSheet(GetParent());
	CActivityPropPage actPage;
	cSheet.AddPage(&actPage);

	if (cSheet.DoModal() == IDOK)
	{
		if (MyDevice()->AddAgentActivity(static_cast<DWORD>
				(actPage.m_nActivityID), actPage.m_strName) >= 0)
			LoadActivity();
	}

}// CActivityPage::OnAdd

/*****************************************************************************
** Procedure:  CActivityPage::OnGetDispInfoActivity
**
** Arguments: 'pNMHDR' - LV_DISPINFO structure
**            'pResult' - Returning result value
**
** Returns:    void
**
** Description: Called by the listview control to show data
**
*****************************************************************************/
void CActivityPage::OnGetDispInfoActivity(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	TAgentActivity* pActivity = reinterpret_cast<TAgentActivity*>(pItem->lParam);

	static TCHAR szBuffer[257];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
			case 0:	// Activity ID
				wsprintf(szBuffer, _T("%04d"), pActivity->dwID);
				break;
			case 1:	// Activity Name
				lstrcpy(szBuffer, pActivity->strName);
				break;
		}
	}

	pItem->pszText = szBuffer;
	*pResult = 0;

}// CActivityPage::OnGetDispInfoActivity

/*****************************************************************************
** Procedure:  CActivityPage::OnChangedActivity
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework when the list control is changing a Activity
**
*****************************************************************************/
void CActivityPage::OnChangedActivity(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if ((pNMListView->uChanged & LVIF_STATE) == LVIF_STATE)
	{
		if ((pNMListView->uNewState & LVIS_FOCUSED) == LVIS_FOCUSED)
		{
			m_btnRemove.EnableWindow(TRUE);
			m_btnProperties.EnableWindow(TRUE);
		}
		else
		{
			m_btnRemove.EnableWindow(FALSE);
			m_btnProperties.EnableWindow(FALSE);
		}
	}
	*pResult = 0;

}// CActivityPage::OnChangedActivity

/*****************************************************************************
** Procedure:  CActivityPage::OnProperties
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to show the properties for a Activity
**
*****************************************************************************/
void CActivityPage::OnProperties() 
{
	TAgentActivity* pActivity = NULL;
	for (int i = 0; i < m_lcActivity.GetItemCount(); i++)
	{
		if (m_lcActivity.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pActivity = reinterpret_cast<TAgentActivity*>(m_lcActivity.GetItemData(i));
			break;
		}
	}

	// If we didn't find a Activity, exit
	if (pActivity == NULL)
		return;

	CSingleSheet cSheet(GetParent());
	CActivityPropPage actPage;

	actPage.m_nActivityID = static_cast<UINT>(pActivity->dwID);
	actPage.m_strName = pActivity->strName;
	cSheet.AddPage(&actPage);

	if (cSheet.DoModal() == IDOK)
	{
		pActivity->strName = actPage.m_strName;
		m_lcActivity.RedrawItems(i,i);
	}

}// CActivityPage::OnProperties

/*****************************************************************************
** Procedure:  CActivityPage::OnRemove
**
** Arguments: void
**
** Returns:    void
**
** Description: Called by the framework to remove a Activity from the configuration
**
*****************************************************************************/
void CActivityPage::OnRemove() 
{
	TAgentActivity* pActivity = NULL;
	for (int i = 0; i < m_lcActivity.GetItemCount(); i++)
	{
		if (m_lcActivity.GetItemState(i, LVIS_FOCUSED) & LVIS_FOCUSED)
		{
			pActivity = reinterpret_cast<TAgentActivity*>(m_lcActivity.GetItemData(i));
			break;
		}
	}

	// If we didn't find a Activity, exit
	if (pActivity == NULL)
		return;

	// Make sure the user wants to remove the Activity from the configuration.
	if (AfxMessageBox(IDS_REMOVEACTIVITY, MB_YESNO) == IDYES)
	{
		MyDevice()->RemoveAgentActivity(pActivity->dwID);
		delete pActivity;
		m_lcActivity.DeleteItem(i);
	}

}// CActivityPage::OnRemove

/*****************************************************************************
** Procedure:  CActivityPage::OnSetActive
**
** Arguments: void
**
** Returns:    TRUE/FALSE if page switch is ok
**
** Description: Called by the framework when this page is made active
**
*****************************************************************************/
BOOL CActivityPage::OnSetActive() 
{
	LoadActivity();
	return CPropertyPage::OnSetActive();

}// CActivityPage::OnSetActive


