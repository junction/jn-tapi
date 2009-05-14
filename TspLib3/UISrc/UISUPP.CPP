/******************************************************************************/
//                                                                        
// UISUPP.CPP - User-interface support functions
//                                                                        
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the methods which can be called fromt eh
// user-interface functions (UIDLL) of the service provider.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

#include "stdafx.h"
#include <ctype.h>

/*-----------------------------------------------------------------------------*/
// GLOBALS and CONSTANTS
/*-----------------------------------------------------------------------------*/
static LPCTSTR gszTelephonyKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");
static LPCTSTR gszDevice = _T("Device%ld");
static LPCTSTR gszUsesLib = _T("UsesTSPLib3");
static LPCTSTR gszTapiDLL = _T("TAPI32.DLL");
static LPCTSTR gszTotalDevices = _T("DeviceCount");

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::CServiceProviderUI
//
// Constructor for the service provider
//
CServiceProviderUI::CServiceProviderUI(LPCTSTR pszProviderInfo) :
	m_pszProviderInfo(pszProviderInfo), 
	m_lpfnCallback(0), m_dwObjectID(0), m_dwObjectType(0)
{
	m_pObjects[0] = RUNTIME_CLASS(CTSPUIDevice);
	m_pObjects[1] = RUNTIME_CLASS(CTSPUILineConnection);
	m_pObjects[2] = RUNTIME_CLASS(CTSPUIAddressInfo);
	m_pObjects[3] = RUNTIME_CLASS(CTSPUIPhoneConnection);

}// CServiceProviderUI::CServiceProviderUI

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::~CServiceProviderUI
//
// Destructor for the service provider
//
CServiceProviderUI::~CServiceProviderUI()
{
	ResetConfiguration();

}// CServiceProviderUI::~CServiceProviderUI

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::_SetIntCallback
//
// This is called on each TSPI_xxx entrypoint to set the callback
// function pointer.
//
void CServiceProviderUI::_SetIntCallback(DWORD dwType, DWORD dwObjectID, TUISPIDLLCALLBACK pfnCallback)
{
	m_dwObjectType = dwType;
	m_dwObjectID = dwObjectID;
	m_lpfnCallback = pfnCallback;

}// CServiceProviderUI::_SetIntCallback

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::SetRuntimeObjects
//
// Set the runtime objects used to create all our various serializabe
// pieces.
//
void CServiceProviderUI::SetRuntimeObjects(CRuntimeClass* pDeviceObject, 
										 CRuntimeClass* pLineObject, 
										 CRuntimeClass* pAddrObject, 
										 CRuntimeClass* pPhoneObject)
{
	if (pDeviceObject)	m_pObjects[0] = pDeviceObject;
	if (pLineObject)	m_pObjects[1] = pLineObject;
	if (pAddrObject)	m_pObjects[2] = pAddrObject;
	if (pPhoneObject)	m_pObjects[3] = pPhoneObject;

}// CServiceProviderUI::SetRuntimeObjects

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::providerConfig
//
// This method is invoked when the user selects our ServiceProvider
// icon in the control panel.  It should invoke the configuration dialog
// which must be provided by the derived class.
//
// This method should be overriden by the derived class to supply
// a dialog.
//
LONG CServiceProviderUI::providerConfig(
DWORD /*dwPermanentProviderID*/,    // Provider ID (unique across providers)
CWnd* /*pwndOwner*/)                // Owner window to supply and UI for.
{
    return FALSE;
    
}// CServiceProviderUI::providerConfig

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::providerInstall
//
// This method is invoked when the TSP is to be installed via the
// TAPI install code.  It should insure that all the correct files
// are there, and write out the initial .INI settings.
//
// This method should be overriden by the derived class.
//
LONG CServiceProviderUI::providerInstall(
DWORD dwPermanentProviderID,		// Provider ID (unique across providers)
CWnd* /*pwndOwner*/)                // Owner window to supply and UI for.
{
	// Open the TELEPHONY key
    HKEY hKeyTelephony;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hKeyTelephony)
		!= ERROR_SUCCESS)
		return LINEERR_OPERATIONFAILED;

	// Add a section to the telephony section which points to our
	// full provider section.  This is used by the DbgSetLevel program
	// supplied by JulMar to locate the provider section properly.
	CString strKey;
	strKey.Format(gszDevice, dwPermanentProviderID);
	LONG lResult = RegSetValueEx(hKeyTelephony, strKey, 0, REG_SZ, 
			(LPBYTE)m_pszProviderInfo, 
			(lstrlen(m_pszProviderInfo)+1) * sizeof(TCHAR));

	// Close our key
	RegCloseKey(hKeyTelephony);

	// If the write failed, then we are not an administrator account and cannot
	// add the provider information.
	if (lResult != ERROR_SUCCESS)
		return LINEERR_OPERATIONFAILED;

	// Add an entry marking this provider as "TSP++" compatible for other
	// programs which search for it.
	WriteProfileDWord(0, gszUsesLib, 1);

	// Bump the device count if it exists; this will be overwritten by the
	// stream code if registry serialization is being used.
	WriteProfileDWord(0, gszTotalDevices, ReadProfileDWord(0, gszTotalDevices, 0L)+1);

    return FALSE;
    
}// CServiceProviderUI::providerInstall

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::providerRemove
//
// This method is invoked when the TSP is being removed from the
// system.  It should remove all its files and .INI settings.
//
// This method should be overriden by the derived class.
//
LONG CServiceProviderUI::providerRemove(
DWORD dwPermanentProviderID,        // Provider ID (unique across providers)
CWnd* /*pwndOwner*/)                // Owner window to supply and UI for.
{                                   
	// Open the TELEPHONY key
    HKEY hKeyTelephony;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hKeyTelephony)
		!= ERROR_SUCCESS)
		return LINEERR_OPERATIONFAILED;

	// Delete our pointer to our section from TAPI
	CString strKey;
	strKey.Format(gszDevice, dwPermanentProviderID);
	LONG lResult = RegDeleteValue (hKeyTelephony, strKey);

	// Get the current device count.  This was written by the providerInstall
	// and also by the stream serialization each time it is used.
	DWORD dwCount = ReadProfileDWord(0, gszTotalDevices, 0L);
	if (dwCount > 1)
	{
		// Decrement the device count
		WriteProfileDWord(0, gszTotalDevices, dwCount-1);

		// Condense down the DeviceXX entries in the registry; these are
		// used by the stream to identify the device sections when loading.
		TCHAR chBuff[50]; DWORD dwIndex = 0;
		for (DWORD dwCurrIndex = 0; dwCurrIndex < dwCount; ++dwCurrIndex, ++dwIndex)
		{
			wsprintf(chBuff, gszDevice, dwCurrIndex);
			DWORD dwDeviceID = ReadProfileDWord(0, chBuff, 0);
			if (dwIndex != dwCurrIndex)
			{
				wsprintf(chBuff, gszDevice, dwIndex);
				WriteProfileDWord(0, chBuff, dwDeviceID);
			}
			else if (dwDeviceID == dwPermanentProviderID)
				--dwIndex;
		}

		// Delete the final entry which is no longer in use.
		wsprintf(chBuff, gszDevice, dwCount-1);
		DeleteProfile(0, chBuff);

		// Delete the device specific information used by streams
		DeleteProfile(0, strKey);
	}
	else
	{
		// Delete our section from the profile - last device.
		IntRegDeleteKey(hKeyTelephony, m_pszProviderInfo);
	}

    RegCloseKey (hKeyTelephony);
    return (lResult == ERROR_SUCCESS) ? S_OK : LINEERR_OPERATIONFAILED;

}// CServiceProviderUI::providerRemove

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::providerGenericDialog
//
// This method is called for a UI dialog DLL
//
LONG CServiceProviderUI::providerGenericDialog (
HTAPIDIALOGINSTANCE /*htDlgInst*/,          // Dialog instance
LPVOID /*lpParams*/,                        // Parameter block
DWORD /*dwSize*/,                           // Size of above
HANDLE /*hEvent*/)                          // Event when INIT complete
{
    return LINEERR_OPERATIONUNAVAIL;

}// CServiceProviderUI::providerGenericDialog

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::providerGenericDialogData
//
// This method is called for the UI DLL when the provider 
// sends information.
//
LONG CServiceProviderUI::providerGenericDialogData (
HTAPIDIALOGINSTANCE /*htDlgInst*/,          // Dialog instance
LPVOID /*lpParams*/,                        // Parameter block
DWORD /*dwSize*/)                           // Size of above
{
    return LINEERR_OPERATIONUNAVAIL;

}// CServiceProviderUI::providerGenericDialogData

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::lineConfigDialog
//
// This function causes the service provider of the line device to
// display a modal dialog as a child of the specified owner.  The dialog
// should allow the user to configure parameters related to the line device.
//
LONG CServiceProviderUI::lineConfigDialog(
DWORD /*dwDeviceID*/,					// Device ID
CWnd* /*pwndOwner*/,                    // Application window owner
CString& /*strDeviceClass*/)            // Specific subscreen to display (device)
{
	return LINEERR_OPERATIONUNAVAIL;
	
}// CServiceProviderUI::lineConfigDialog

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::lineConfigDialogEdit
//
// This function causes the service provider to display a modal dialog as 
// a child window of the specified parent.  The dialog should allow the 
// user to configure parameters related to the line device.  The parameters
// for the configuration are passed in and passed back once the dialog is
// dismissed.
//
LONG CServiceProviderUI::lineConfigDialogEdit(
DWORD /*dwDeviceID*/,					// Device ID
CWnd* /*pwndOwner*/,                    // Application window owner
CString& /*strDeviceClass*/,            // Specific subscreen to display (device)
LPVOID const /*lpDeviceConfigIn*/,      // Returned from lineGetDevConfig
DWORD /*dwSize*/,                       // Size of above
LPVARSTRING /*lpDeviceConfigOut*/)      // Device configuration being returned.
{                    
	return LINEERR_OPERATIONUNAVAIL;
    
}// CServiceProviderUI::lineConfigDialogEdit

///////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::phoneConfigDialog
//
// Causes the provider of the specified phone device to display a modal 
// dialog (attached to the application's hwndOwner) that allows the user to 
// configure parameters related to the phone device specified by dwDeviceID.
//
LONG CServiceProviderUI::phoneConfigDialog(
DWORD /*dwDeviceID*/,						// Device ID for phone
CWnd* /*pwndOwner*/,						// Owner window handle
CString& /*strDevClass*/)					// Device class
{
	return PHONEERR_OPERATIONUNAVAIL;

}// CServiceProviderUI::phoneConfigDialog

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::IsProviderInstalled
//
// This queries the TAPI subsystem to see if a provider is installed.
// It returns LINEERR_NOMULTIPLEINSTANCE if it is.
//
LONG CServiceProviderUI::IsProviderInstalled(LPCTSTR pszProviderName, LPDWORD lpdwPPid) const
{
	HINSTANCE hTapiDLL = NULL;
	LPLINEPROVIDERLIST pProviderList = NULL;
	LONG lResult = 0;
	CString strThisProvider (pszProviderName);

	// See if this provider is already installed.  We do this by querying
	// TAPI for the list of installed providers.
	hTapiDLL = LoadLibrary(gszTapiDLL);
	if (hTapiDLL == NULL)
		return LINEERR_OPERATIONFAILED;

	// Locate our entrypoint for "lineGetProviderList".  This will return
	// the list of providers installed in this system.
	LONG (WINAPI *pfnGetProviderList)(DWORD, LPLINEPROVIDERLIST);
	pfnGetProviderList = (LONG (WINAPI*) (DWORD, LPLINEPROVIDERLIST)) GetProcAddress(hTapiDLL, "lineGetProviderList");
	if (pfnGetProviderList == NULL)
	{
		FreeLibrary (hTapiDLL);
		return LINEERR_OPERATIONFAILED;
	}

	// Retrieve all the providers.
	DWORD dwReqSize = sizeof(LINEPROVIDERLIST)*10;
	while (TRUE)
	{
		pProviderList = (LPLINEPROVIDERLIST) GlobalAllocPtr(GHND, dwReqSize);
		if (pProviderList == NULL)
		{
			lResult = LINEERR_NOMEM;
			break;
		}

		pProviderList->dwTotalSize = dwReqSize;
		if (pfnGetProviderList (0x00020000, pProviderList) != 0)
		{
			lResult = LINEERR_OPERATIONFAILED;
			break;
		}

		// Go through the list of retrieved providers and see if we are included
		// in this list - TAPI will not add us to the registry until we return 
		// success to this function, so we should not currently be here.
		if (pProviderList->dwNeededSize <= pProviderList->dwTotalSize)
		{
			LPLINEPROVIDERENTRY pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
					pProviderList->dwProviderListOffset);
			for (DWORD i = 0; i < pProviderList->dwNumProviders; i++)
			{
				// Get the name of this provider.
				LPCSTR pszProvider = (LPCSTR) pProviderList+pProviderEntry->dwProviderFilenameOffset;

				// Make sure we are pointing at the TSP module, and not any path
				if (strrchr(pszProvider, '\\') != NULL)
					pszProvider = strrchr(pszProvider, '\\') + sizeof(char);

				// If this is OUR provider, then error out.
				if (strThisProvider.CompareNoCase((LPTSTR)pszProvider) == 0)
				{
					*lpdwPPid = pProviderEntry->dwPermanentProviderID;
					lResult = LINEERR_NOMULTIPLEINSTANCE;
					break;
				}
				pProviderEntry++;
			}
			break;
		}
		else
		{
			dwReqSize = pProviderList->dwNeededSize;
			GlobalFreePtr ((LPSTR)pProviderList);
			pProviderList = NULL;
		}
	}

	// Cleanup and call the providerConfig function if we can install.
	if (pProviderList != NULL)
		GlobalFreePtr ((LPSTR)pProviderList);
	if (hTapiDLL)
		FreeLibrary(hTapiDLL);

	return lResult;

}// CServiceProviderUI::IsProviderInstalled

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetProviderIDFromLineDeviceID
//
// This converts a line device ID into a provider id.
//
LONG CServiceProviderUI::GetProviderIDFromLineDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid)
{
	ASSERT(m_lpfnCallback != NULL);
	DWORD arrInfo[2] = { GDD_LINEPHONETOPROVIDER, dwDeviceID };
	if ((*m_lpfnCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID, &arrInfo, sizeof(DWORD)*2) == 0 && 
		arrInfo[0] == GDD_LINEPHONETOPROVIDEROK)
	{
		*lpdwPPid = arrInfo[1];
		return 0;
	}
	return LINEERR_OPERATIONFAILED;

}// CServiceProviderUI::GetProviderIDFromLineDeviceID

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetProviderIDFromPhoneDeviceID
//
// This converts a phone device ID into a provider id.
//
LONG CServiceProviderUI::GetProviderIDFromPhoneDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid)
{
	ASSERT(m_lpfnCallback != NULL);
	DWORD arrInfo[2] = { GDD_LINEPHONETOPROVIDER, dwDeviceID };
	if ((*m_lpfnCallback)(dwDeviceID, TUISPIDLL_OBJECT_PHONEID,
			&arrInfo, sizeof(DWORD)*2) == 0 && 
			arrInfo[0] == GDD_LINEPHONETOPROVIDEROK)
	{
		*lpdwPPid = arrInfo[1];
		return 0;
	}
	return PHONEERR_OPERATIONFAILED;

}// CServiceProviderUI::GetProviderIDFromPhoneDeviceID

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetPermanentIDFromLineDeviceID
//
// This converts a line device ID into a permanent line id.
//
LONG CServiceProviderUI::GetPermanentIDFromLineDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid)
{
	DWORD arrInfo[2] = { GDD_LINEPHONETOPERMANENT, dwDeviceID };
	ASSERT (m_lpfnCallback != NULL);
	ASSERT (lpdwPPid != NULL);

	*lpdwPPid = 0;

	LONG lResult = (*m_lpfnCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID, &arrInfo, sizeof(DWORD)*2);
	if (lResult == 0 && arrInfo[0] == GDD_LINEPHONETOPERMANENTOK)
		*lpdwPPid = arrInfo[1];
	return lResult;

}// CServiceProviderUI::GetProviderIDFromLineDeviceID

/////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetPermanentIDFromPhoneDeviceID
//
// This converts a phone device ID into a permanent line/phone id.
//
LONG CServiceProviderUI::GetPermanentIDFromPhoneDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid)
{
	DWORD arrInfo[2] = { GDD_LINEPHONETOPERMANENT, dwDeviceID };
	ASSERT (m_lpfnCallback != NULL);
	ASSERT (lpdwPPid != NULL);

	*lpdwPPid = 0;

	LONG lResult = (*m_lpfnCallback)(dwDeviceID, TUISPIDLL_OBJECT_PHONEID, &arrInfo, sizeof(DWORD)*2);
	if (lResult == 0 && arrInfo[0] == GDD_LINEPHONETOPERMANENTOK)
		*lpdwPPid = arrInfo[1];
	return lResult;

}// CServiceProviderUI::GetProviderIDFromPhoneDeviceID

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::ReadProfileString
//
// Read a string from our profile section in the registry.  This
// function is limited to 512 characters.
//
CString CServiceProviderUI::ReadProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR lpszDefault/*=""*/)
{
	TCHAR szBuff[512];

    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
        return lpszDefault;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
        return lpszDefault;
	}

	// Open our device section
    DWORD dwDataSize = sizeof(szBuff), dwDataType;
	if (dwDeviceID > 0)
	{
		wsprintf(szBuff, gszDevice, dwDeviceID);

		HKEY hDeviceKey;
		if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return lpszDefault;
		}

		// Query the value requested.
		if (RegQueryValueEx (hDeviceKey, pszEntry, 0, &dwDataType, 
					(LPBYTE)szBuff, &dwDataSize) == ERROR_SUCCESS &&
			(dwDataType == REG_MULTI_SZ || dwDataType == REG_SZ))
			lpszDefault = szBuff;
		RegCloseKey (hDeviceKey);
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);

	}
	else
	{
		// Query the value requested.
		if (RegQueryValueEx (hProviderKey, pszEntry, 0, &dwDataType, 
					(LPBYTE)szBuff, &dwDataSize) == ERROR_SUCCESS &&
			(dwDataType == REG_MULTI_SZ || dwDataType == REG_SZ))
			lpszDefault = szBuff;
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}

    return lpszDefault;

}// CServiceProviderUI::ReadProfileString

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::ReadProfileDWord
//
// Read a DWORD from our profile section in the registry.
//
DWORD CServiceProviderUI::ReadProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwDefault/*=0*/)
{
    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return dwDefault;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return dwDefault;
	}

	// Open our device section
	DWORD dwDataSize = sizeof(DWORD), dwDataType, dwData;
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);

		HKEY hDeviceKey;
		if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return dwDefault;
		}

		// Query the value requested.
		if (RegQueryValueEx (hDeviceKey, pszEntry, 0, &dwDataType, 
					(LPBYTE)&dwData, &dwDataSize) != ERROR_SUCCESS ||
				(dwDataType != REG_DWORD) ||  dwDataSize != sizeof(DWORD))
			dwData = dwDefault;

		RegCloseKey (hDeviceKey);
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}
	else
	{
		// Query the value requested.
		if (RegQueryValueEx (hProviderKey, pszEntry, 0, &dwDataType, 
				(LPBYTE)&dwData, &dwDataSize) != ERROR_SUCCESS ||
				(dwDataType != REG_DWORD)  || dwDataSize != sizeof(DWORD))
			dwData = dwDefault;
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}

    return dwData;

}// CServiceProviderUI::ReadProfileDWord

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::WriteProfileString
//
// Write a string into our registry profile.
//
bool CServiceProviderUI::WriteProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszValue)
{
    DWORD dwDisposition;

    // Attempt to create the telephony registry section - it should really exist if our
    // driver has been loaded by TAPI.
    HKEY hKeyTelephony;
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, _T(""), REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS, NULL, &hKeyTelephony, &dwDisposition) != ERROR_SUCCESS)
        return FALSE;

    // Now create our provider section if necessary.
    HKEY hKeyProvider;
    if (RegCreateKeyEx (hKeyTelephony, m_pszProviderInfo, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                NULL, &hKeyProvider, &dwDisposition) != ERROR_SUCCESS)
	{
		RegCloseKey (hKeyTelephony);
        return FALSE;
	}

	// Create our device section
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);

		HKEY hDeviceKey;
		if (RegCreateKeyEx (hKeyProvider, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			RegCloseKey (hKeyProvider);
			RegCloseKey (hKeyTelephony);
			return FALSE;
		}

		// Store the key.
		RegSetValueEx (hDeviceKey, pszEntry, 0, REG_SZ, (LPBYTE)pszValue, (lstrlen(pszValue)+1) * sizeof(TCHAR));
		RegCloseKey (hDeviceKey);
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);

	}
	else
	{
		RegSetValueEx (hKeyProvider, pszEntry, 0, REG_SZ, (LPBYTE)pszValue, (lstrlen(pszValue)+1) * sizeof(TCHAR));
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}

	return TRUE;

}// CServiceProviderUI::WriteProfileString

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::WriteProfileDWord
//
// Write a DWORD into our registry profile.
//
bool CServiceProviderUI::WriteProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwValue)
{
    DWORD dwDisposition;

    // Attempt to create the telephony registry section - it should really exist if our
    // driver has been loaded by TAPI.
    HKEY hKeyTelephony;
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, _T(""), REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS, NULL, &hKeyTelephony, &dwDisposition) != ERROR_SUCCESS)
        return FALSE;

    // Now create our provider section if necessary.
    HKEY hKeyProvider;
    if (RegCreateKeyEx (hKeyTelephony, m_pszProviderInfo, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                        NULL, &hKeyProvider, &dwDisposition) != ERROR_SUCCESS)
	{
		RegCloseKey (hKeyTelephony);
        return FALSE;
	}

	// Create our device section
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);

		HKEY hDeviceKey;
		if (RegCreateKeyEx (hKeyProvider, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			RegCloseKey (hKeyProvider);
			RegCloseKey (hKeyTelephony);
			return FALSE;
		}

		// Store the key.
		RegSetValueEx (hDeviceKey, pszEntry, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		RegCloseKey (hDeviceKey);
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}
	else
	{
		// Store the key.
		RegSetValueEx (hKeyProvider, pszEntry, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}

	return TRUE;

}// CServiceProviderUI::WriteProfileDWord

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::DeleteProfile
//
// Deletes the registry key directory for a section.
//
bool CServiceProviderUI::DeleteProfile (DWORD dwDeviceID, LPCTSTR pszKey/*=NULL*/)
{
    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return FALSE;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return FALSE;
	}

	// Delete the device section.
	// Delete the device section or entry.
	TCHAR szBuff[20];
	wsprintf(szBuff, gszDevice, dwDeviceID);
	bool rc = false;
	if (pszKey == NULL)
	{
		rc = IntRegDeleteKey (hProviderKey, szBuff);
	}
	else
	{
		if (dwDeviceID > 0)
		{
			HKEY hDeviceKey;
			if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) == ERROR_SUCCESS)
			{
				rc = (RegDeleteKey(hProviderKey, pszKey) == ERROR_SUCCESS || RegDeleteValue(hProviderKey, pszKey) == ERROR_SUCCESS);
				RegCloseKey (hDeviceKey);
			}
		}
		else
		{
			rc = (RegDeleteKey(hProviderKey, pszKey) == ERROR_SUCCESS || RegDeleteValue(hProviderKey, pszKey) == ERROR_SUCCESS);
		}
	}

    RegCloseKey (hProviderKey);
    RegCloseKey (hTelephonyKey);

	return rc;

}// CServiceProviderUI::DeleteProfile

////////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::RenameProfile
//
// Moves a profile from one area to another.
//
bool CServiceProviderUI::RenameProfile (DWORD dwOldDevice, DWORD dwNewDevice)
{
	// Ignore requests for the same profile.
	if (dwOldDevice == dwNewDevice)
		return TRUE;

	// First make sure the NEW device doesn't exist already
	if (dwNewDevice > 0 && !DeleteProfile(dwNewDevice))
		return FALSE;

    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return FALSE;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return FALSE;
	}

	// Open our device section.
	HKEY hOldDeviceKey;
	TCHAR szBuff[20];
	if (dwOldDevice > 0)
	{
		wsprintf(szBuff, gszDevice, dwOldDevice);
		if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hOldDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return FALSE;
		}
	}
	else
		hOldDeviceKey = hProviderKey;

	// Create the new section
	HKEY hNewDeviceKey;
	if (dwNewDevice > 0)
	{
		DWORD dwDisposition;
		wsprintf(szBuff, gszDevice, dwNewDevice);
		if (RegCreateKeyEx (hProviderKey, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hNewDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			if (dwOldDevice > 0)
				RegCloseKey(hOldDeviceKey);
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return FALSE;
		}
	}
	else
		hNewDeviceKey = hProviderKey;

	// Get the max size of the name and values.
	DWORD dwNameSize, dwValueSize, dwType;
	if (RegQueryInfoKey (hOldDeviceKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		                 &dwNameSize, &dwValueSize, NULL, NULL) != ERROR_SUCCESS)
	{
		dwNameSize = 1024;
		dwValueSize = 4096;
	}
	else
	{
		dwNameSize++;
		dwValueSize++;
	}

	// Alloc blocks to hold the information.
	LPTSTR pszName = (LPTSTR) GlobalAllocPtr (GHND, dwNameSize * sizeof(TCHAR));
	LPBYTE pszValue = (LPBYTE) GlobalAllocPtr (GHND, dwValueSize * sizeof(TCHAR));

	// Enumerate through all the values within the old key, and move them to the new section.
	DWORD dwIndex = 0;
	while (TRUE)
	{
		// Enumerate through the items.
		DWORD dwNewNameSize = dwNameSize, dwNewValueSize = dwValueSize;
		if (RegEnumValue (hOldDeviceKey, dwIndex++, pszName, &dwNewNameSize, NULL,
									   &dwType, pszValue, &dwNewValueSize) != ERROR_SUCCESS)
			break;

		// Delete the value.
		RegDeleteValue (hOldDeviceKey, pszName);

		// Create the key in our new subkey.
		RegSetValueEx (hNewDeviceKey, pszName, 0, dwType, (LPBYTE)pszValue, dwNewValueSize);
	}

	// We're done with the memory.
	GlobalFreePtr (pszName);
	GlobalFreePtr (pszValue);

	// Close all the used keys.
	if (dwNewDevice > 0)
		RegCloseKey(hNewDeviceKey);
	if (dwOldDevice > 0)
		RegCloseKey(hOldDeviceKey);
	RegCloseKey (hProviderKey);
	RegCloseKey (hTelephonyKey);

	// Now delete the original section
	if (dwOldDevice > 0)
		DeleteProfile(dwOldDevice);

	return TRUE;

}// CServiceProviderUI::RenameProfile

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::IntRegDeleteKey
//
// Internal function used to delete a section of the registry under Windows NT
// where we must delete each branch seperately.
//
bool CServiceProviderUI::IntRegDeleteKey (HKEY hKeyTelephony, LPCTSTR pszMainDir)
{
	// Attempt to delete the key directly. Under Win95, this will also delete
	// any branches under it.
    if (RegDeleteKey(hKeyTelephony, pszMainDir) != ERROR_SUCCESS)
	{
		// Open the top-level key.
		HKEY hKey;
		DWORD dwRC = RegOpenKeyEx(hKeyTelephony, pszMainDir, 0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey);
		if (dwRC == ERROR_SUCCESS)
		{
			DWORD dwReqSize = _MAX_PATH;
			LPTSTR pszName = new TCHAR[_MAX_PATH];
			if (pszName == NULL)
			{
				RegCloseKey(hKey);
				dwRC = ERROR_NOT_ENOUGH_MEMORY;
			}

			while (dwRC == ERROR_SUCCESS)
			{
				dwReqSize = _MAX_PATH;
				dwRC = RegEnumKeyEx(hKey, 0, pszName, &dwReqSize, NULL, NULL, NULL, NULL);
				if (dwRC == ERROR_NO_MORE_ITEMS)
				{
				   dwRC = RegDeleteKey(hKeyTelephony, pszMainDir);
				   break;
				}
				else if (dwRC == ERROR_SUCCESS)
				   IntRegDeleteKey(hKey, pszName);
			}

			RegCloseKey(hKey);
			delete [] pszName;
		}
	}
	return true;

}// CServiceProviderUI::IntRegDeleteKey

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::InvokeTSP
//
// This is a function which jumps through the pointer provided by
// the TSP to send information
//
LONG CServiceProviderUI::InvokeTSP(LPVOID lpParams, DWORD dwSize) 
{
	ASSERT (m_lpfnCallback != NULL);
	if (m_lpfnCallback != NULL)
		return (*m_lpfnCallback)(m_dwObjectID, m_dwObjectType, lpParams, dwSize);
	return LINEERR_UNINITIALIZED;

}// CServiceProviderUI::InvokeTSP

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::InvokeTSP
//
// This is a function which jumps through the pointer provided by
// the TSP to send information
//
LONG CServiceProviderUI::InvokeTSP(HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize) 
{
	ASSERT (m_lpfnCallback != NULL);
	if (m_lpfnCallback != NULL)
		return (*m_lpfnCallback)((DWORD)htDlgInst, TUISPIDLL_OBJECT_DIALOGINSTANCE, lpParams, dwSize);
	return LINEERR_UNINITIALIZED;

}// CServiceProviderUI::InvokeTSP

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::ResetConfiguration
//
// This dumps all our configuration data and resets all the arrays
// as if we had nothing loaded.
//
void CServiceProviderUI::ResetConfiguration()
{
	// Delete all our objects
	while (GetDeviceCount() > 0)
	{
		CTSPUIDevice* pDevice = GetDeviceByIndex(0);
		m_arrDevices.RemoveAt(0);
		delete pDevice;
	}

}// CServiceProviderUI::ResetConfiguration

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::FindUIDialogFromInstance
//
// This looks in our UI map and locates the CWnd* associated with a 
// TAPI dialog instance handle.
//
CWnd* CServiceProviderUI::FindUIDialogFromInstance(HTAPIDIALOGINSTANCE htDlgInst)
{
	CWnd* pwnd = NULL;
	if (m_mapUIDlgs.Lookup(htDlgInst, pwnd))
		return pwnd;
	return NULL;

}// CServiceProviderUI::FindUIDialogFromInstance

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::AddUIDialog
//
// This adds the user-interface dialog to the service provider list.
//
void CServiceProviderUI::AddUIDialog(HTAPIDIALOGINSTANCE htDlgInst, CWnd* pwnd)
{
	m_mapUIDlgs[htDlgInst] = pwnd;

}// CServiceProviderUI::AddUIDialog

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::RemoveUIDialog
//
// This removes the user-interface dialog from the service provider list.
//
void CServiceProviderUI::RemoveUIDialog(CWnd* pwnd)
{
	for (POSITION pos = m_mapUIDlgs.GetStartPosition(); pos != NULL;)
	{
		HTAPIDIALOGINSTANCE htDlgInst;
		CWnd* pwndTest;

		m_mapUIDlgs.GetNextAssoc(pos, htDlgInst, pwndTest);
		if (pwnd == pwndTest)
		{
			m_mapUIDlgs.RemoveKey(htDlgInst);
			break;
		}
	}

}// CServiceProviderUI::RemoveUIDialog
