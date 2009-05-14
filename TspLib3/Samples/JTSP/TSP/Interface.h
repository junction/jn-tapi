/*******************************************************************/
//
// INTERFACE.H
//
// WinSock interface definitions
//
// Copyright (C) 1998 JulMar Technology, Inc.
// All rights reserved
//
// TSP++ Version 3.00 PBX/ACD Emulator Projects
//
// Modification History
// --------------------
// 1998/09/05 MCS@JulMar	Initial revision
//
/*******************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __JPBX_INTF_INC__
#define __JPBX_INTF_INC__

/*----------------------------------------------------------------------------
	COMMANDS
-----------------------------------------------------------------------------*/
// LOGON - LO;EXTENSION
#define PBXCMD_LOGON _T("LO;%d")
// AGENT LOGON - ALO;EXTENSION;AGENTID;PASSWORD;GROUP1;GROUP2
#define PBXCMD_ALO _T("ALO;%d;%s;%s;%X;%X")
// AGENT STATE - CAS;EXTENSION;AGENTID;STATE
#define PBXCMD_CAS _T("CAS;%d;%s;%c")
// HOLD CALL - HC;EXTENSION;CALLID
#define PBXCMD_HC  _T("HC;%d;%x")
// RETRIEVE CALL - RC;EXTENSION;CALLID
#define PBXCMD_RTC  _T("RTC;%d;%x")
// GENERATE TONE/DIGIT - GTD;EXTENSION;CALLID;DIGIT
#define PBXCMD_GTD _T("GTD;%d;%x;%c")
// PLACE CALL - PC;EXTENSION;NUMBER
#define PBXCMD_PC  _T("PC;%d;%s")
// RELEASE CALL - RC;EXTENSION;CALLID
#define PBXCMD_RC  _T("RC;%d;%x")
// ANSWER CALL -  AN;EXTENSION;CALLID
#define PBXCMD_AN  _T("AN;%d;%x")
// TRANSFER CALL - TC;EXTENSION;HCALLID;TCALLID
#define PBXCMD_TC  _T("TC;%d;%x;%x")
// BLIND TRANSFER and REDIRECT call - BTC;EXTENSION;CALLID;EXTENSION
#define PBXCMD_BTC _T("BTC;%d;%x;%s")
// PLACE PREDICTIVE CALL - PPC;EXTENSION;NUMBER;TIMEOUT(SEC);TRANSFERTO
#define PBXCMD_PPC _T("PPC;%d;%s;%d;%s")
// QUERY AGENT STATES
#define PBXCMD_QAS _T("QAS;")
// QUERY VERSION OF PBX
#define PBXCMD_VERSION _T("VER;")
// SET GAIN - SPG;EXTENSION;GAIN
#define PBXCMD_SPG _T("SPG;%d;%d")
// SET HOOKSWITCH - SPH;EXTENSION;0/1
#define PBXCMD_SPH _T("SPH;%d;%d")
// SET VOLUME - SPV;EXTENSION;VOLUME
#define PBXCMD_SPV _T("SPV;%d;%d")

/**************************************************************************
** CConnection
**
** This object represents a single connection to the PBX
**
***************************************************************************/
class CConnection
{
// Class data
protected:
	SOCKET m_hSocket;			// Socket this connection is working with
	HANDLE m_hThread;			// Thread running input from socke
	HANDLE m_hevtStop;			// Stop evnt

// Constructor
public:
	CConnection();
	~CConnection();

// Socket Methods
public:
	bool Connect(LPCTSTR pszHostAddress, int nPort);
	bool Send(LPCSTR pszBuf, int nBuf);
	void Close();
	int  Receive(LPSTR pszBuf, int nBuf);
	bool IsValid() const;
	bool HasData();
	bool _cdecl SendEvent(LPCTSTR pszEvent, ...);
	bool WaitForData(TString& strData);
};

/*****************************************************************************
** Procedure:  CConnection::IsValid
**
** Arguments: void
**
** Returns:   void
**
** Description:  TRUE/FALSE whether socket is valid
**
*****************************************************************************/
inline bool CConnection::IsValid() const
{
	return (m_hSocket != INVALID_SOCKET);

}// CConnection::IsValid

#endif // __JPBX_INTF_INC__
