/******************************************************************************/
//                                                                        
// UIADDR.INL - TAPI Service Provider C++ Library header                     
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
// INLINE FUNCTIONS
//                                                           
/******************************************************************************/

#ifndef _UIADDR_INL_INC_
#define _UIADDR_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPUIAddressInfo class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetDialableAddress
//
// Returns the dialable number for this addresss
//
TSP_INLINE LPCTSTR CTSPUIAddressInfo::GetDialableAddress() const
{
	return m_strDN;

}// CTSPUIAddressInfo::GetDialableAddress

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::SetDialableAddress
//
// Sets the dialable number to a value
//
TSP_INLINE void CTSPUIAddressInfo::SetDialableAddress(LPCTSTR pwszAddress)
{
	m_strDN = pwszAddress;

}// CTSPUIAddressInfo::SetDialableAddress

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetName
//
// Returns the name of this address
//
TSP_INLINE LPCTSTR CTSPUIAddressInfo::GetName() const
{
	return m_strName;

}// CTSPUIAddressInfo::GetName

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::SetName
//
// Sets the name to a known value
//
TSP_INLINE void CTSPUIAddressInfo::SetName (LPCTSTR pwszName)
{
	m_strName = pwszName;

}// CTSPUIAddressInfo::SetName

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::CanAnswerCalls
//
// Returns whether or not this address may answer calls
//
TSP_INLINE bool CTSPUIAddressInfo::CanAnswerCalls() const
{
	return m_fAllowIncoming;

}// CTSPUIAddressInfo::CanAnswerCalls

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::CanMakeCalls
//
// Returns whether or not this address may place calls
//
TSP_INLINE bool CTSPUIAddressInfo::CanMakeCalls() const
{
	return m_fAllowOutgoing;

}// CTSPUIAddressInfo::CanMakeCalls

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetAvailableMediaModes
//
// Returns the media mode(s) this address supports
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetAvailableMediaModes () const
{
	return m_dwAvailMediaModes;

}// CTSPUIAddressInfo::GetAvailableMediaModes

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetBearerMode
//
// Returns the single bearer mode of this address
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetBearerMode() const
{
	return m_dwBearerMode;

}// CTSPUIAddressInfo::GetBearerMode

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMinimumDataRate
//
// Returns the minimum data rate
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMinimumDataRate() const
{
	return m_dwMinRate;

}// CTSPUIAddressInfo::GetMinimumDataRate

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaximumDataRate
//
// Returns the maximum data rate
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaximumDataRate() const
{
	return m_dwMaxRate;

}// CTSPUIAddressInfo::GetMaximumDataRate

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaxNumActiveCalls
//
// Returns the maximum number of active calls
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaxNumActiveCalls() const
{
	return m_dwMaxNumActiveCalls;

}// CTSPUIAddressInfo::GetMaxNumActiveCalls

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaxNumOnHoldCalls
//
// Returns the maximum number of onHold calls
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaxNumOnHoldCalls() const
{
	return m_dwMaxNumOnHoldCalls;

}// CTSPUIAddressInfo::GetMaxNumOnHoldCalls

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaxNumOnHoldPendCalls
//
// Returns the maximum number of onHoldPendxxx calls
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaxNumOnHoldPendCalls() const
{
	return m_dwMaxNumOnHoldPendCalls;

}// CTSPUIAddressInfo::GetMaxNumOnHoldPendCalls

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaxNumInConference
//
// Returns the maximum number of calls in a single conference
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaxNumInConference() const
{
	return m_dwMaxNumConference;

}// CTSPUIAddressInfo::GetMaxNumInConference

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetMaxNumInTransfConference
//
// Returns the maximum number transferred into a conference.
//
TSP_INLINE DWORD CTSPUIAddressInfo::GetMaxNumInTransfConference() const
{
	return m_dwMaxNumTransConf;

}// CTSPUIAddressInfo::GetMaxNumInTransfConference

///////////////////////////////////////////////////////////////////////////
// CTSPUIAddressInfo::GetDialParams
//
// Returns the dialing parameters pointer
//
TSP_INLINE LPLINEDIALPARAMS CTSPUIAddressInfo::GetDialParams()
{
	return &m_DialParams;

}// CTSPUIAddressInfo::GetDialParams

#endif // _UIADDR_INL_INC_
