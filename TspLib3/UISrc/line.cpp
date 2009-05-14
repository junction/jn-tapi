/******************************************************************************/
//                                                                        
// LINE.CPP - User-interface LINECONNECTION support
//                                             
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// The SPLIB classes provide a basis for developing MS-TAPI complient     
// Service Providers.  They provide basic handling for all of the TSPI    
// APIs and a C-based handler which routes all requests through a set of C++     
// classes.                                                                 
//              
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                           
/******************************************************************************/

#include "stdafx.h"
#include <ctype.h>
#include <spbstrm.h>

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::CTSPUILineConnection
//
// Constructor for the CTSPUILineConnection class
//
CTSPUILineConnection::CTSPUILineConnection() : 
	m_pDevice(0), m_dwDeviceID(0xffffffff), 
	m_iType(Station), m_dwPhoneID(0xffffffff),
	m_fSupportsAgents(false)
{
	m_guidMSP = IID_NULL;
	m_clsidProtocol = IID_NULL;

}// CTSPUILineConnection::CTSPUILineConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::CTSPUILineConnection
//
// Constructor for the CTSPUILineConnection class
//
CTSPUILineConnection::CTSPUILineConnection(DWORD dwDeviceID, int iType, LPCTSTR pszName) : 
	m_pDevice(0), m_dwDeviceID(dwDeviceID),
    m_iType(iType), m_strName(pszName), 
	m_dwPhoneID(0xffffffff), m_fSupportsAgents(false)
{
	m_guidMSP = IID_NULL;
	m_clsidProtocol = IID_NULL;

}// CTSPUILineConnection::CTSPUILineConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::~CTSPUILineConnection
//
// Destructor for the CTSPUILineConnection class
//
CTSPUILineConnection::~CTSPUILineConnection()
{
	// Unassociate the phone
	SetAssociatedPhone(0xffffffff);

	// Delete the address objects
	for (int i = 0; i < m_arrAddress.GetSize(); i++)
		delete m_arrAddress[i];

}// CTSPUILineConnection::~CTSPUILineConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::operator=
//
// Assignment operator
//
const CTSPUILineConnection& CTSPUILineConnection::operator=(const CTSPUILineConnection& lc)
{
	if (&lc != this)
	{
		ASSERT (m_pDevice == lc.m_pDevice);

		m_dwDeviceID = lc.m_dwDeviceID;
		m_strName = lc.m_strName;
		m_iType = lc.m_iType;
		m_dwPhoneID = lc.m_dwPhoneID;

		unsigned int i;
		for (i = 0; i < GetAddressCount(); i++)
			RemoveAddress(i);

		for (i = 0; i < lc.GetAddressCount(); i++)
		{
			CTSPUIAddressInfo* pAddr = lc.GetAddress(i);
			CTSPUIAddressInfo* pNewAddr = dynamic_cast<CTSPUIAddressInfo*>(GetUISP()->m_pObjects[2]->CreateObject());
			*pNewAddr = *pAddr;
			m_arrAddress.Add(pNewAddr);
		}

		m_fSupportsAgents = lc.m_fSupportsAgents;
		m_guidMSP = lc.m_guidMSP;
		m_clsidProtocol = lc.m_clsidProtocol;
	}

	return *this;

}// CTSPUILineConnection::operator=

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::read
//
// Reads the object from a stream
//
TStream& CTSPUILineConnection::read(TStream& istm)
{
	TVersionInfo vi(istm);

	istm >> m_dwDeviceID >> m_iType >> m_dwPhoneID >> m_strName;

	// Read each of the addresses
	unsigned int iCount = 0;
	istm >> iCount;
	for (unsigned int i = 0; i < iCount; i++)
	{
		CTSPUIAddressInfo* pAddr = dynamic_cast<CTSPUIAddressInfo*>(GetUISP()->m_pObjects[2]->CreateObject());
		istm >> *pAddr;
		m_arrAddress.Add(pAddr);
	}

	// Read agent support
	istm >> m_fSupportsAgents;

	// If this is a version 3.x stream then load the TAPI 3.0 information
	if (vi.GetVersion() >= 3)
	{
		istm >> m_guidMSP;
		istm >> m_clsidProtocol;
	}

	return istm;

}// CTSPUILineConnection::read

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::write
//
// Writes the object to a stream
//
TStream& CTSPUILineConnection::write(TStream& ostm) const
{
	TVersionInfo vi(ostm, 3);

	ostm << m_dwDeviceID 
		 << m_iType 
		 << m_dwPhoneID 
		 << m_strName;

	// Write each of the addresses out
	unsigned int aCount = static_cast<unsigned int>(GetAddressCount());
	ostm << aCount;
	for (unsigned int i = 0; i < aCount; i++)
	{
		CTSPUIAddressInfo* pAddr = GetAddress(i);
		ostm << *pAddr;
	}

	// Write our agent support
	ostm << m_fSupportsAgents;

	// Save the TAPI 3.0 media provider information
	ostm << m_guidMSP;
	ostm << m_clsidProtocol;

	return ostm;

}// CTSPUILineConnection::write

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::CreateAddress
//
// Create a new address on this line
//
unsigned int CTSPUILineConnection::CreateAddress(
	LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, bool fAllowIncoming, 
	bool fAllowOutgoing, DWORD dwAvailMediaModes, DWORD dwBearerMode,
	DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams,
	DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
	DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
	DWORD dwMaxNumTransConf, DWORD dwAddressType)
{
	CTSPUIAddressInfo* pAddr = dynamic_cast<CTSPUIAddressInfo*>(GetUISP()->m_pObjects[2]->CreateObject());
	ASSERT (pAddr != NULL);

	pAddr->Init(lpszDialableAddr, lpszAddrName, fAllowIncoming, fAllowOutgoing,
		dwAvailMediaModes, dwBearerMode, dwMinRate, dwMaxRate, 
		lpDialParams, dwMaxNumActiveCalls, dwMaxNumOnHoldCalls, 
		dwMaxNumOnHoldPendCalls, dwMaxNumConference, dwMaxNumTransConf, 
		dwAddressType);

	return AddAddress(pAddr);

}// CTSPUILineConnection::CreateAddress

