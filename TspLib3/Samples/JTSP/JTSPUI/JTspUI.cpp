/***************************************************************************
//
// JTSPUI.CPP
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
// 09/04/1998	MCS@Julmar	Generated from TSPWizard.exe
// 
/***************************************************************************/

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "JTspUI.h"
#include "config.h"
#include "properties.h"
#include <spbstrm.h>

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-------------------------------------------------------------------------------*/
// RTTI Support
/*-------------------------------------------------------------------------------*/
IMPLEMENT_DYNCREATE(CJTSPDevice, CTSPUIDevice)

/*-------------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*-------------------------------------------------------------------------------*/
CJTspUIApp theApp;

/*-------------------------------------------------------------------------------*/
// STRUCTURES
/*-------------------------------------------------------------------------------*/
struct
{
	DWORD dwFunction;
	DWORD dwMode;
	LPCTSTR pszText;
} g_arrButtons[] = {

	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("0") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("1") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("2") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("3") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("4") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("5") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("6") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("7") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("8") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("9") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("*") },
	{ PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_KEYPAD, _T("#") },
    { PHONEBUTTONFUNCTION_CALLAPP, PHONEBUTTONMODE_CALL, _T("Line #1") },
    { PHONEBUTTONFUNCTION_CALLAPP, PHONEBUTTONMODE_CALL, _T("Line #2") },
    { PHONEBUTTONFUNCTION_CALLAPP, PHONEBUTTONMODE_CALL, _T("Line #3") },
    { PHONEBUTTONFUNCTION_HOLD,	   PHONEBUTTONMODE_FEATURE,	_T("Hold") },
    { PHONEBUTTONFUNCTION_DROP,	   PHONEBUTTONMODE_FEATURE,	_T("Release") },
    { PHONEBUTTONFUNCTION_TRANSFER, PHONEBUTTONMODE_FEATURE, _T("Transfer") },
    { PHONEBUTTONFUNCTION_CONFERENCE, PHONEBUTTONMODE_FEATURE, _T("Conference") },
    { PHONEBUTTONFUNCTION_VOLUMEUP, PHONEBUTTONMODE_LOCAL,	_T("Vol>>") },
    { PHONEBUTTONFUNCTION_VOLUMEDOWN, PHONEBUTTONMODE_LOCAL, _T("<<Vol") }
};

#define TOTAL_BUTTONS (sizeof(g_arrButtons)/sizeof(g_arrButtons[0]))

/*****************************************************************************
** Procedure:  CJTspUIApp::CJTspUIApp
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Constructor for the UI application dll
**
*****************************************************************************/
CJTspUIApp::CJTspUIApp() : 
	CServiceProvider(_T("JulMar Sample TAPI Server"))
{
	// Note: you must assign the overriden objects to the library 
	// so they are serialized properly when reloaded if you add additional
	// data to the iostream (read/write overriden).
	SetRuntimeObjects(RUNTIME_CLASS(CJTSPDevice), NULL, NULL, NULL);

}// CJTspUIApp::CJTspUIApp

/*****************************************************************************
** Procedure:  CJTspUIApp::providerInstall
**
** Arguments:  'dwPermanentProviderID' - Provider ID
**             'pwndOwner' - Owner window handle
**
** Returns:    TAPI 2.x result code
**
** Description:  This method is invoked when the TSP is to be installed via 
**               the TAPI install code.  It should insure that all the correct 
**               files are there, and write out the initial .INI settings.
**
*****************************************************************************/
LONG CJTspUIApp::providerInstall(DWORD dwPermanentProviderID, CWnd* pwndOwner)
{
	// Check to see if this provider is already installed. If so we will
	// NOT install it a second time.
	DWORD dwMyPPid = 0;
	LONG lResult = IsProviderInstalled(AfxGetAppName(), &dwMyPPid);
	if (lResult == 0) // Returns LINEERR_NOMULTIPLEINSTANCE if in TAPI already.
	{
		// Create our device object. This will use our CJTSPDevice because
		// we added it above in SetRuntimeObjects.
		AddDevice(dwPermanentProviderID);

		// Pass it through the TSP++ library to add registry keys and such.
		// If that is successful, then run the configuration for this device.
		lResult = CServiceProvider::providerInstall(dwPermanentProviderID, pwndOwner);
		if (lResult == 0)
		{
			// Create our configuration sheet
			CConfigSheet cSheet(pwndOwner);
			if (cSheet.DoModal() == IDOK)
			{
				// Save off all our registry data.  Store off our line information first.
				// This is a built-in function of the TSP++ UI library
				SaveObjects();
			}
			
			// Otherwise cancel the installation.
			else
				lResult = LINEERR_OPERATIONFAILED;
		}
	}
	return lResult;

}// CJTspUIApp::providerInstall

/*****************************************************************************
** Procedure:  CJTspUIApp::lineConfigDialog
**
** Arguments:  'dwDeviceID' - Line Device ID
**             'pwndOwner' - Owner window handle
**             'strDeviceClass' - Device class we are working with
**
** Returns:    TAPI 2.x result code
**
** Description:  This method is called to display the line configuration dialog
**				 when the user requests it through either the TAPI api or the 
**               control panel applet.
**
*****************************************************************************/
LONG CJTspUIApp::lineConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& /*strDeviceClass*/) 
{
	// Convert the device id into a permanent device id to ensure that we
	// have the correct line device.  Normally the dwDeviceID is simply an
	// index into our line array but if there is more than one provider installed
	// the base might not be zero.
	DWORD dwpLID;
	if (GetUISP()->GetPermanentIDFromLineDeviceID(dwDeviceID, &dwpLID) == 0)
	{
		CTSPUILineConnection* pLine = MyDevice()->FindLineConnectionByPermanentID(dwpLID);
		if (pLine != NULL)
		{
			CSingleSheet cSheet(pwndOwner);
			CLinePropPage linePage;
			cSheet.AddPage(&linePage);

			linePage.m_dwExtension = dwpLID;
			linePage.m_strName = pLine->GetName();
			linePage.m_iType = pLine->GetLineType();

			if (cSheet.DoModal() == IDOK)
				pLine->SetName(linePage.m_strName);

			return 0;
		}
	}

	return LINEERR_OPERATIONFAILED;

}// CJTspUIApp::lineConfigDialog

/*****************************************************************************
** Procedure:  CJTspUIApp::phoneConfigDialog
**
** Arguments:  'dwDeviceID' - Phone Device ID
**             'pwndOwner' - Owner window handle
**             'strDeviceClass' - Device class we are working with
**
** Returns:    TAPI 2.x result code
**
** Description:  This method is called to display the phone configuration dialog
**				 when the user requests it through either the TAPI api or the 
**               control panel applet.
**
*****************************************************************************/
LONG CJTspUIApp::phoneConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& /*strDeviceClass*/) 
{
	// Convert the device id into a permanent device id to ensure that we
	// have the correct phone device.  Normally the dwDeviceID is simply an
	// index into our phone array but if there is more than one provider installed
	// the base might not be zero.
	DWORD dwpPID;
	if (GetUISP()->GetPermanentIDFromPhoneDeviceID(dwDeviceID, &dwpPID) == 0)
	{
		CTSPUIPhoneConnection* pPhone = MyDevice()->FindPhoneConnectionByPermanentID(dwpPID);
		if (pPhone != NULL)
		{
			CSingleSheet cSheet(pwndOwner);
			CPhonePropPage phonePage;
			cSheet.AddPage(&phonePage);

			phonePage.m_dwExtension = dwpPID;
			phonePage.m_strName = pPhone->GetName();

			cSheet.DoModal();
			return 0;
		}
	}

	return PHONEERR_OPERATIONFAILED;

}// CJTspUIApp::phoneConfigDialog

/*****************************************************************************
** Procedure:  CJTspUIApp::providerConfig
**
** Arguments:  'dwPPID' - Provider ID
**             'pwndOwner' - Owner window handle
**
** Returns:    TAPI 2.x result code
**
** Description:  This method is invoked when the user selects our ServiceProvider
**				 icon in the control panel.  It should invoke the configuration 
**               dialog which must be provided by the derived class.
**
*****************************************************************************/
LONG CJTspUIApp::providerConfig(DWORD /*dwProviderID*/, CWnd* pwndOwner)
{
	// Create our configuration sheet
	CConfigSheet cSheet(pwndOwner);
	if (cSheet.DoModal() == IDOK)
	{
		// Save off all our registry data.  Store off our line information first.
		// This is a built-in function of the TSP++ UI library
		SaveObjects();
	}
	return 0;

}// CJTspUIApp::providerConfig

/*****************************************************************************
** Procedure:  CJTSPPhone::CJTSPPhone
**
** Arguments:  'dwPhoneID' - Phone identifier
**             'pszName' - Name of phone unit
**
** Returns:    void
**
** Description:  Constructor for the phone device
**
*****************************************************************************/
CJTSPPhone::CJTSPPhone(DWORD dwPhoneID, LPCTSTR pszName) : 
	CTSPUIPhoneConnection(dwPhoneID, pszName)
{
	// Setup the display
	SetupDisplay(40, 2);
	
	// Add the hookswitch device - a single headset with the ability to adjust the
	// volume/gain and mute the microphone.
	AddHookSwitchDevice(PHONEHOOKSWITCHDEV_HEADSET, 
		(PHONEHOOKSWITCHMODE_ONHOOK | PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_SPEAKER),
		0x1000, 0x1000, (PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_SPEAKER),
		(PHONEHOOKSWITCHMODE_ONHOOK | PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_SPEAKER),
		true, true);

	// Add our digit and function buttons to the system
	for (int i = 0; i < TOTAL_BUTTONS; i++)
	{
		AddButton(g_arrButtons[i].dwFunction, g_arrButtons[i].dwMode, 
				PHONELAMPMODE_DUMMY, g_arrButtons[i].pszText);
	}

	// Add the lamp buttons to the device.  The emulator has two lamps
	AddButton(PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_DUMMY, 
		(PHONELAMPMODE_OFF | PHONELAMPMODE_STEADY),
		_T("READY"));
	AddButton(PHONEBUTTONFUNCTION_NONE, PHONEBUTTONMODE_DUMMY, 
		(PHONELAMPMODE_OFF | PHONELAMPMODE_STEADY),
		_T("CALL WORK"));

}// CJTSPPhone::CJTSPPhone

/*****************************************************************************
** Procedure:  CJTSPDevice::read
**
** Arguments:  'istm' - Input iostream
**
** Returns:    iostream reference
**
** Description: This is called to read information in from the registry.
**
*****************************************************************************/
TStream& CJTSPDevice::read(TStream& istm )
{
	// Read our port and IP address
	istm >> m_strIPAddress >> m_nPort;

	// Always call the base class
	return CTSPUIDevice::read(istm);

}// CJTSPDevice::read

/*****************************************************************************
** Procedure:  CJTSPDevice::write
**
** Arguments:  'ostm' - Input iostream
**
** Returns:    iostream reference
**
** Description: This is called to read information in from the registry.
**
*****************************************************************************/
TStream& CJTSPDevice::write(TStream& ostm) const
{
	// Write our port and IP address
	ostm << m_strIPAddress << m_nPort;

	// Always call the base class
	return CTSPUIDevice::write(ostm);

}// CJTSPDevice::write

