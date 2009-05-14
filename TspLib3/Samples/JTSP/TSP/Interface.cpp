/*******************************************************************/
//
// INTERFACE.CPP
//
// WinSock interface definitions.
//
// Copyright (C) 1998 JulMar Technology, Inc.
// All rights reserved
//
// TSP++ Version 3.00 PBX/ACD Emulator Projects
//
// Modification History
// ----------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Taken from JPSTATION project
//
/*******************************************************************/

/*---------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------*/
#include "stdafx.h"
#include "Interface.h"

/*---------------------------------------------------------------*/
// DEBUG INFORMATION
/*---------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
** Procedure:  CConnection::CConnection
**
** Arguments: void
**
** Returns:    void
**
** Description:  Constructor for the connection object
**
*****************************************************************************/
CConnection::CConnection() : m_hSocket(INVALID_SOCKET)
{
}// CConnection::CConnection

/*****************************************************************************
** Procedure:  CConnection::Close
**
** Arguments: void
**
** Returns:    void
**
** Description:  Destructor for the connection object
**
*****************************************************************************/
CConnection::~CConnection()
{
	// Close the socket
	Close();

}// CConnection::~CConnection

/*****************************************************************************
** Procedure:  CConnection::Close
**
** Arguments: void
**
** Returns:   void
**
** Description:  Receieve data from the socket
**
*****************************************************************************/
void CConnection::Close()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		// Set the don't linger bit.
		BOOL fNoLinger = true;
		setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (LPCSTR)&fNoLinger, sizeof(BOOL));
		shutdown(m_hSocket, SD_BOTH);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

}// CConnection::Close

/*****************************************************************************
** Procedure:  CConnection::Connect
**
** Arguments: 'pszHostAddress' - Address to connect to
**            'nPort' - Port to use
**
** Returns:   bool result
**
** Description:  Connects to a socket destination
**
*****************************************************************************/
bool CConnection::Connect(LPCTSTR pszHostAddress, int nPort)
{
	// Create the socket if necessary
	if (m_hSocket == INVALID_SOCKET)
		m_hSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Determine if the address is in dotted notation
	SOCKADDR_IN sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((u_short)nPort);

#ifdef _UNICODE
	char chBuff[50];
	_TSP_VERIFY(WideCharToMultiByte(CP_ACP, 0, pszHostAddress, -1, chBuff, sizeof(chBuff), NULL, NULL) != 0);
	sockAddr.sin_addr.s_addr = inet_addr(chBuff);
#else
	sockAddr.sin_addr.s_addr = inet_addr(pszHostAddress);
#endif

	// If the address is not dotted notation, then do a DNS 
	// lookup of it.
	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
#ifdef _UNICODE
		lphost = gethostbyname(chBuff);
#else
		lphost = gethostbyname(pszHostAddress);
#endif
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
			WSASetLastError(WSAEINVAL); 
			return false;
		}
	}

	// Connect the socket
	if (connect(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR)
		return true;

	_TSP_DTRACEX(TRC_USERDEFINED, _T("Socket connect failed, rc=0x%lx\n"), WSAGetLastError());
	Close();
	return false;

}// CConnection::Connect

/*****************************************************************************
** Procedure:  CConnection::Send
**
** Arguments: 'pszBuff' - Buffer to send
**            'nBufSize' - Buffer size
**
** Returns:   bool result
**
** Description:  Send buffer through the socket
**
*****************************************************************************/
bool CConnection::Send(LPCSTR pszBuff, int nBufSize)
{
	if (!IsValid())
		return false;
	if (nBufSize == 0)
		nBufSize = lstrlenA(pszBuff);
	return (send(m_hSocket, pszBuff, nBufSize, 0) != SOCKET_ERROR);

}// CConnection::Send

/*****************************************************************************
** Procedure:  CConnection::Receive
**
** Arguments: 'pszBuff' - Buffer to recieve into
**            'nBufSize' - Buffer size
**
** Returns:   count of bytes read
**
** Description:  Receieve data from the socket
**
*****************************************************************************/
int CConnection::Receive(LPSTR pszBuff, int nBufSize)
{
	if (!IsValid())
		return SOCKET_ERROR;
	return recv(m_hSocket, pszBuff, nBufSize, 0); 

}// CConnection::Recieve

/*****************************************************************************
** Procedure:  CConnection::HasData
**
** Arguments: void
**
** Returns:   true/false whether the socket has data
**
** Description:  Determine wether there is data on the socket
**
*****************************************************************************/
#pragma warning(disable:4127)
bool CConnection::HasData()
{
	if (!IsValid())
		return false;

	timeval timeout = { 0, 0 };

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_hSocket, &fds);

	// Get the status of the socket
	int nStatus = select(0, &fds, NULL, NULL, &timeout);
	if (nStatus == SOCKET_ERROR)
		return FALSE;
	return (!(nStatus == 0));

}// CConnection::HasData
#pragma warning(default:4127)

/*****************************************************************************
** Procedure:  CConnection::WaitForData
**
** Arguments: 'strData' - Returning string buffer
**
** Returns:    true/false if data was received
**
** Description: This function waits for data on the connection.
**
*****************************************************************************/
bool CConnection::WaitForData(TString& strData)
{
	if (!IsValid())
		return false;

	USES_CONVERSION;
	std::string strBuffer;

	// Block on the read event
	char ch;
	for ( ;; )
	{
		if (Receive(&ch, 1) == 1)
		{
			if (ch == _T('\r'))
			{
				strData = A2T(const_cast<char*>(strBuffer.c_str()));
				if (!strData.empty())
					return true;
			}
			else if (isprint(ch))
				strBuffer += ch;
		}
		else
		{
			Close();
			break;
		}
	}
	return false;

}// CConnection::WaitForData

/*****************************************************************************
** Procedure:  CConnection::SendEvent
**
** Arguments: 'pszEvent' - Event wsprintf string
**            '...' - Arguments
**
** Returns:    void
**
** Description:  Sends a block of data to one or more connections
**
*****************************************************************************/
bool _cdecl CConnection::SendEvent(LPCTSTR pszEvent, ...)
{
	TCHAR chBuff[255];	// All sent events should be < 255

	// Formate the string
	va_list args;
	va_start(args, pszEvent);
	int nBufSize = wvsprintf(chBuff, pszEvent, args);
	va_end(args);
	lstrcat(chBuff, _T("\r\n"));

	_TSP_DTRACEX(TRC_DUMP, _T("Snd: %s"), chBuff);

#ifdef _UNICODE
	TString strEvent = chBuff;
	nBufSize = WideCharToMultiByte(CP_ACP, 0, strEvent.c_str(), -1, (LPSTR)chBuff, sizeof(chBuff), NULL, NULL);
	_TSP_ASSERTE(nBufSize > 0);
#endif

	// Pass it through the connection manager
	if (nBufSize > 0)
	{	
		if (!Send((LPCSTR)chBuff, 0))
			_TSP_DTRACEX(TRC_WARNINGS, _T("SendEvent(%d) failed, rc=0x%lx\n"), nBufSize, WSAGetLastError());
	}
	return true;

}// CConnection::SendEvent

