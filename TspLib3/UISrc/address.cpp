/******************************************************************************/
//                                                                        
// ADDRESS.CPP - User-interface ADDRESSINFO support
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
// CTSPUIAddressInfo::CTSPUIAddressInfo
//
// Constructor for the ADDRESSINFO object
//
CTSPUIAddressInfo::CTSPUIAddressInfo() : 
	m_fAllowIncoming(true), m_fAllowOutgoing(true), 
	m_dwAvailMediaModes(0L), m_dwBearerMode(0L),
	m_dwMinRate(0L), m_dwMaxRate(0L), m_dwMaxNumActiveCalls(0L),
	m_dwMaxNumOnHoldCalls(0L), m_dwMaxNumOnHoldPendCalls(0L), 
	m_dwMaxNumConference(0L), m_dwMaxNumTransConf(0L), m_dwAddressType(LINEADDRESSTYPE_PHONENUMBER)
{
	ZeroMemory(&m_DialParams, sizeof(LINEDIALPARAMS));

}// CTSPUIAddressInfo::CTSPUIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::~CTSPUIAddressInfo
//
// Destructor
//
CTSPUIAddressInfo::~CTSPUIAddressInfo()
{
}// CTSPUIAddressInfo::~CTSPUIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::operator=
//
// Assignment operator
//
const CTSPUIAddressInfo& CTSPUIAddressInfo::operator=(const CTSPUIAddressInfo& ai)
{
	if (&ai != this)
	{
		m_strDN = ai.m_strDN;
		m_strName = ai.m_strName;
		m_fAllowIncoming = ai.m_fAllowIncoming; 
		m_fAllowOutgoing = ai.m_fAllowOutgoing;
		m_dwAvailMediaModes = ai.m_dwAvailMediaModes;
		m_dwBearerMode = ai.m_dwBearerMode;
		m_dwMinRate = ai.m_dwMinRate;
		m_dwMaxRate = ai.m_dwMaxRate;
		m_dwMaxNumActiveCalls = ai.m_dwMaxNumActiveCalls;
		m_dwMaxNumOnHoldCalls = ai.m_dwMaxNumOnHoldCalls;
		m_dwMaxNumOnHoldPendCalls = ai.m_dwMaxNumOnHoldPendCalls;
		m_dwMaxNumConference = ai.m_dwMaxNumConference;
		m_dwMaxNumTransConf = ai.m_dwMaxNumTransConf;
		m_dwAddressType = ai.m_dwAddressType;
		CopyMemory(&m_DialParams, &ai.m_DialParams, sizeof(LINEDIALPARAMS));
	}
	return *this;

}// CTSPUIAddressInfo::operator=

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::CTSPUIAddressInfo
//
// Copy constructor for the address object
//
CTSPUIAddressInfo::CTSPUIAddressInfo(const CTSPUIAddressInfo& ai) :
	m_strDN(ai.m_strDN), m_strName(ai.m_strName), m_fAllowIncoming(ai.m_fAllowIncoming), 
	m_fAllowOutgoing(ai.m_fAllowOutgoing), m_dwAvailMediaModes(ai.m_dwAvailMediaModes),
	m_dwBearerMode(ai.m_dwBearerMode), m_dwMinRate(ai.m_dwMinRate), m_dwMaxRate(ai.m_dwMaxRate),
	m_dwMaxNumActiveCalls(ai.m_dwMaxNumActiveCalls), m_dwMaxNumOnHoldCalls(ai.m_dwMaxNumOnHoldCalls),
	m_dwMaxNumOnHoldPendCalls(ai.m_dwMaxNumOnHoldPendCalls), m_dwMaxNumConference(ai.m_dwMaxNumConference),
	m_dwMaxNumTransConf(ai.m_dwMaxNumTransConf), m_dwAddressType(ai.m_dwAddressType)
{
	CopyMemory(&m_DialParams, &ai.m_DialParams, sizeof(LINEDIALPARAMS));

}// CTSPUIAddressInfo::CTSPUIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::CTSPUIAddressInfo
//
// Parameterized constructor for the address object
//
CTSPUIAddressInfo::CTSPUIAddressInfo(LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, 
	bool fAllowIncoming, bool fAllowOutgoing, DWORD dwAvailMediaModes, DWORD dwBearerMode,
	DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams, DWORD dwMaxNumActiveCalls, 
	DWORD dwMaxNumOnHoldCalls, DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
	DWORD dwMaxNumTransConf, DWORD dwAddressType) :
	m_strDN(lpszDialableAddr), m_strName(lpszAddrName),
	m_fAllowIncoming(fAllowIncoming), m_fAllowOutgoing(fAllowOutgoing), m_dwAvailMediaModes(dwAvailMediaModes), 
	m_dwBearerMode(dwBearerMode), m_dwMinRate(dwMinRate), m_dwMaxRate(dwMaxRate), m_dwMaxNumActiveCalls(dwMaxNumActiveCalls),
	m_dwMaxNumOnHoldCalls(dwMaxNumOnHoldCalls), m_dwMaxNumOnHoldPendCalls(dwMaxNumOnHoldPendCalls), 
	m_dwMaxNumConference(dwMaxNumConference), m_dwMaxNumTransConf(dwMaxNumTransConf), m_dwAddressType(dwAddressType)
{
	if (lpDialParams != NULL)
		CopyMemory(&m_DialParams, lpDialParams, sizeof(LINEDIALPARAMS));
	else
		ZeroMemory(&m_DialParams, sizeof(LINEDIALPARAMS));

}// CTSPUIAddressInfo::CTSPUIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::Init
//
// Initialization function for the address info object
//
void CTSPUIAddressInfo::Init(LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, 
	bool fAllowIncoming, bool fAllowOutgoing, DWORD dwAvailMediaModes, DWORD dwBearerMode,
	DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams, DWORD dwMaxNumActiveCalls, 
	DWORD dwMaxNumOnHoldCalls, DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
	DWORD dwMaxNumTransConf, DWORD dwAddressType)
{
	m_strDN = lpszDialableAddr;
	m_strName = lpszAddrName;
	m_fAllowIncoming = fAllowIncoming; 
	m_fAllowOutgoing = fAllowOutgoing;
	m_dwAvailMediaModes = dwAvailMediaModes;
	m_dwBearerMode = dwBearerMode;
	m_dwMinRate = dwMinRate;
	m_dwMaxRate = dwMaxRate;
	m_dwMaxNumActiveCalls = dwMaxNumActiveCalls;
	m_dwMaxNumOnHoldCalls = dwMaxNumOnHoldCalls;
	m_dwMaxNumOnHoldPendCalls = dwMaxNumOnHoldPendCalls;
	m_dwMaxNumConference = dwMaxNumConference;
	m_dwMaxNumTransConf = dwMaxNumTransConf;
	m_dwAddressType = dwAddressType;

	if (lpDialParams != NULL)
		CopyMemory(&m_DialParams, lpDialParams, sizeof(LINEDIALPARAMS));

}// CTSPUIAddressInfo::CTSPUIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::write
//
// Writes the ADDRESSINFO into a stream for serialization
//
TStream& CTSPUIAddressInfo::write(TStream& ostm) const
{
	TVersionInfo vi(ostm, 3);

	ostm << m_strName 
		 << m_strDN 
		 << m_fAllowIncoming 
		 << m_fAllowOutgoing
		 << m_dwAvailMediaModes 
		 << m_dwBearerMode 
		 << m_dwMinRate 
		 << m_dwMaxRate
		 << m_dwMaxNumActiveCalls 
		 << m_dwMaxNumOnHoldCalls 
		 << m_dwMaxNumOnHoldPendCalls
		 << m_dwMaxNumConference 
		 << m_dwMaxNumTransConf
		 << m_DialParams.dwDialPause 
		 << m_DialParams.dwDialSpeed
		 << m_DialParams.dwDigitDuration 
		 << m_DialParams.dwWaitForDialtone
         << m_dwAddressType;

	return ostm;

}// CTSPUIAddressInfo::write

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::read
//
// Reads the ADDRESSINFO from a stream for serialization
//
TStream& CTSPUIAddressInfo::read(TStream& istm)
{
	TVersionInfo vi(istm);

	istm >> m_strName 
		 >> m_strDN 
		 >> m_fAllowIncoming 
		 >> m_fAllowOutgoing
		 >> m_dwAvailMediaModes 
		 >> m_dwBearerMode 
		 >> m_dwMinRate 
		 >> m_dwMaxRate
		 >> m_dwMaxNumActiveCalls 
		 >> m_dwMaxNumOnHoldCalls 
		 >> m_dwMaxNumOnHoldPendCalls
		 >> m_dwMaxNumConference 
		 >> m_dwMaxNumTransConf
		 >> m_DialParams.dwDialPause 
		 >> m_DialParams.dwDialSpeed
		 >> m_DialParams.dwDigitDuration 
		 >> m_DialParams.dwWaitForDialtone;

	if (vi.GetVersion() >= 3)
		istm >> m_dwAddressType;

	return istm;

}// CTSPUIAddressInfo::read
