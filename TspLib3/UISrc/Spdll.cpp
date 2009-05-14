/******************************************************************************/
//                                                                        
// SPDLL.CPP - UI Service Provider DLL shell.                                
//                                                                        
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This module intercepts the TSPI calls and invokes the SP object        
// with the appropriate parameters.                                       
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

#include "stdafx.h"
#include <ctype.h>

// Include all our inline code if necessary
#ifdef _NOINLINES_
#include "uiline.inl"
#include "uiaddr.inl"
#include "uidevice.inl"
#include "uiagent.inl"
#include "uiphone.inl"
#endif

// Entrypoint definition for each TSPUI_xxx function
#define TSPUI_ENTRY(t,o,f) (GetUISP()->_SetIntCallback(t,(DWORD)o,f))

#ifndef _UNICODE
///////////////////////////////////////////////////////////////////////////
// ConvertWideToAnsi
//
// Utility function included with non-UNICODE build to convert the
// UNICODE strings to normal ANSI single-byte strings.
//
CString ConvertWideToAnsi(LPCWSTR lpszInput)
{
	CString strReturn;
	if (lpszInput != NULL)
	{
		int iSize = WideCharToMultiByte (CP_ACP, 0, lpszInput, -1, NULL, 0, NULL, NULL);
		if (iSize > 0)
		{
			LPSTR lpszBuff = strReturn.GetBuffer(iSize+1);
			if (lpszBuff != NULL)
				WideCharToMultiByte (CP_ACP, 0, lpszInput, -1, lpszBuff, iSize, NULL, NULL);
			strReturn.ReleaseBuffer();
		}
	}
	return strReturn;

}// ConvertWideToAnsi
#endif // _UNICODE

/////////////////////////////////////////////////////////////////////////////
// TUISPI_lineConfigDialog
//
// This function is called to display the line configuration dialog
// when the user requests it through either the TAPI api or the control
// panel applet.
//
extern "C"
LONG TSPIAPI TUISPI_lineConfigDialog (TUISPIDLLCALLBACK lpfnDLLCallback, DWORD dwDeviceID, HWND hwndOwner, LPCWSTR lpszDeviceClass)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_LINEID , dwDeviceID, lpfnDLLCallback);

#ifdef _UNICODE
	CString strDevClass = lpszDeviceClass;
#else
	CString strDevClass = ConvertWideToAnsi (lpszDeviceClass);
#endif

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

	strDevClass.MakeLower();
	return GetUISP()->lineConfigDialog(dwDeviceID, CWnd::FromHandle(hwndOwner), strDevClass);

}// TUISPI_lineConfigDialog

///////////////////////////////////////////////////////////////////////////
// TUISPI_lineConfigDialogEdit
//
// This function causes the provider of the specified line device to
// display a modal dialog to allow the user to configure parameters
// related to the line device.  The parameters editted are NOT the
// current device parameters, rather the set retrieved from the
// 'TSPI_lineGetDevConfig' function (lpDeviceConfigIn), and are returned
// in the lpDeviceConfigOut parameter.
//
extern "C"
LONG TSPIAPI TUISPI_lineConfigDialogEdit (TUISPIDLLCALLBACK lpfnDLLCallback,
		 DWORD dwDeviceID, HWND hwndOwner,
         LPCWSTR lpszDeviceClass, LPVOID const lpDeviceConfigIn, DWORD dwSize,
         LPVARSTRING lpDeviceConfigOut)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_LINEID , dwDeviceID, lpfnDLLCallback);

#ifdef _UNICODE
	CString strDevClass = lpszDeviceClass;
#else
	CString strDevClass = ConvertWideToAnsi (lpszDeviceClass);
#endif

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

	strDevClass.MakeLower();
    return GetUISP()->lineConfigDialogEdit(dwDeviceID, CWnd::FromHandle(hwndOwner),
                        strDevClass, lpDeviceConfigIn, dwSize, lpDeviceConfigOut);

}// TUISPI_lineConfigDialogEdit

///////////////////////////////////////////////////////////////////////////
// TUISPI_phoneConfigDialog
//
// This function invokes the parameter configuration dialog for the
// phone device.
//
extern "C"
LONG TSPIAPI TUISPI_phoneConfigDialog (TUISPIDLLCALLBACK lpfnDLLCallback,
			DWORD dwDeviceId, HWND hwndOwner, LPCWSTR lpszDeviceClass)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_PHONEID, dwDeviceId, lpfnDLLCallback);

#ifdef _UNICODE
	CString strDevClass = lpszDeviceClass;
#else
	CString strDevClass = ConvertWideToAnsi (lpszDeviceClass);
#endif

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

    return GetUISP()->phoneConfigDialog(dwDeviceId, CWnd::FromHandle(hwndOwner), strDevClass);

}// TUISPI_phoneConfigDialog

////////////////////////////////////////////////////////////////////////////
// TUISPI_providerConfig
//
// This function is invoked from the control panel and allows the user
// to configure our service provider.
//        
extern "C"
LONG TSPIAPI TUISPI_providerConfig (TUISPIDLLCALLBACK lpfnDLLCallback,
		HWND hwndOwner, DWORD dwPermanentProviderID)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_PROVIDERID, dwPermanentProviderID, lpfnDLLCallback);

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

	return GetUISP()->providerConfig(dwPermanentProviderID, CWnd::FromHandle(hwndOwner));
   
}// TUISPI_providerConfig

////////////////////////////////////////////////////////////////////////////
// TUISPI_providerGenericDialog
//
// This function is called when the service provider requests
// a spontaneous dialog not known by TAPI itself.
//
extern "C"
LONG TSPIAPI TUISPI_providerGenericDialog (TUISPIDLLCALLBACK lpfnDLLCallback,
		HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize,
		HANDLE hEvent)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_DIALOGINSTANCE, htDlgInst, lpfnDLLCallback);
	return GetUISP()->providerGenericDialog (htDlgInst, lpParams, dwSize, hEvent);

}// TUISPI_providerGenericDialog

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerGenericDialogData
//
// This function delivers to the UI DLL data that was
// sent from the service provider via the LINE_SENDDIALOGINSTANCEDATA
// message.  The contents of the memory block pointed to be 
// lpParams is defined by the service provider and UI DLL.  
//
extern "C"
LONG TSPIAPI TUISPI_providerGenericDialogData (
		HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize)
{
	return GetUISP()->providerGenericDialogData (htDlgInst, lpParams, dwSize);

}// TSPI_providerGenericDialogData

////////////////////////////////////////////////////////////////////////////
// TUISPI_providerInstall
//
// This function is invoked to install the service provider onto
// the system.
//        
extern "C"
LONG TSPIAPI TUISPI_providerInstall(TUISPIDLLCALLBACK lpfnDLLCallback,
			HWND hwndOwner, DWORD dwPermanentProviderID)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_PROVIDERID, dwPermanentProviderID, lpfnDLLCallback);

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

	return GetUISP()->providerInstall(dwPermanentProviderID, CWnd::FromHandle(hwndOwner));

}// TUISPI_providerInstall

////////////////////////////////////////////////////////////////////////////
// TUISPI_providerRemove
//
// This function removes the service provider
//
extern "C"
LONG TSPIAPI TUISPI_providerRemove(TUISPIDLLCALLBACK lpfnDLLCallback,
			HWND hwndOwner, DWORD dwPermanentProviderID)
{
	TSPUI_ENTRY(TUISPIDLL_OBJECT_PROVIDERID, dwPermanentProviderID, lpfnDLLCallback);

	// If the parent window is the desktop, then set it to NULL
	// so we don't inadvertantly disable the desktop window (which causes
	// big trouble even under WinNT).  NULL has the same basic effect.
	if (hwndOwner == ::GetDesktopWindow())
		hwndOwner = NULL;

	return GetUISP()->providerRemove(dwPermanentProviderID, CWnd::FromHandle(hwndOwner));

}// TUISPI_providerRemove
