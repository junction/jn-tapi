/***************************************************************************
//
// JTSPUI.H
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
// 15/01/1999   MCS@Julmar	Updated for v3.0b
// 
/***************************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __JTSPUI_INC__
#define __JTSPUI_INC__

#include "resource.h"       // main symbols

// Define a macro to get access to the first (and only) device in our
// configuration so we don't have to store off a pointer or type all this
// stuff over and over.
#define MyDevice() GetUISP()->GetDeviceByIndex(0)

/**************************************************************************
** CJTSPDevice
**
** This object is an override for the device object. It stores off 
** TCP/IP information into the registry stream.
**
***************************************************************************/
class CJTSPDevice : public CTSPUIDevice
{
// Class data
protected:
	CString m_strIPAddress;
	UINT m_nPort;

// Constructor
protected:
	CJTSPDevice() : CTSPUIDevice() { m_strIPAddress = _T("127.0.0.1"); m_nPort = 4020; }
	DECLARE_DYNCREATE(CJTSPDevice)

// Access methods
public:
	CString& IPAddress()	{ return m_strIPAddress; }
	UINT& IPPort()			{ return m_nPort; }

// Serialization support
protected:
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

/**************************************************************************
** CJTSPPhone
**
** This object is an override for the phone object to add additional
** data to the phone.
**
***************************************************************************/
class CJTSPPhone : public CTSPUIPhoneConnection
{
// Constructor which adds display, buttons, etc.
public:
	CJTSPPhone(DWORD dwPhoneID, LPCTSTR pszName);
};

/**************************************************************************
** CJTspUIApp
**
** This object is our connection to the service provider
**
***************************************************************************/
class CJTspUIApp : public CServiceProvider
{
// Class data
protected:
	DWORD m_dwProviderID;

// Constructor
public:
	CJTspUIApp();

// Publics
public:
	DWORD GetProviderID() const { return m_dwProviderID; }

// TSPI overrides.
public:
	// This method is invoked when the user selects our ServiceProvider
	// icon in the control panel.  It should invoke the configuration dialog
	// which must be provided by the derived class.
	virtual LONG providerConfig(DWORD dwPPID, CWnd* pwndOwner);

	// This method is invoked when the TSP is to be installed via the
	// TAPI install code.  It should insure that all the correct files
	// are there, and write out the initial registry settings.
	virtual LONG providerInstall(DWORD dwPermanentProviderID, CWnd* pwndOwner);

   // This method is called to display the line configuration dialog
   // when the user requests it through either the TAPI api or the control
   // panel applet.
   virtual LONG lineConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& strDeviceClass);

	// This method invokes the parameter configuration dialog for the
	// phone device.
	virtual LONG phoneConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& strDevClass);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTspUIApp)
	//}}AFX_VIRTUAL
};

/*****************************************************************************
** Procedure:  xtoi
**
** Arguments: 'pszValue' - ASCII Value to convert
**
** Returns:   'iValue' - Returned numeric value
**
** Description: This function converts a null terminated ascii string for a 
**              hex number to its integer value.  Should just be the number 
**              with no preceeding "0x" or trailing "H".  Garbage will result 
**              if there are non-hex digits in the string.  Hex digits are: 
**              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, a, b, c, d, e, f. 
**              Non-hex digits will be treated like '0'. 
**
*****************************************************************************/
inline unsigned int xtoi(LPCTSTR pszValue)
{ 
	register TCHAR ch; 
	register unsigned int nValue = 0; 

	while ((ch = *pszValue++) != 0) 
	{ 
		if (_istdigit(ch)) 
			ch -= _T('0'); 
		else if (ch >= _T('A') && ch <= _T('F')) 
			ch += (TCHAR)(10 - _T('A')); 
		else if (ch >= _T('a') && ch <= _T('f')) 
			ch += (TCHAR)(10 - _T('a')); 
		else 
			ch = (TCHAR)0; 

		nValue = (16 * nValue + ch); 
	} 
	return nValue; 

}// xtoi

#endif // __JTSPUI_INC__


