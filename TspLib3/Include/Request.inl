/******************************************************************************/
//                                                                        
// REQUEST.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _SPREQ_INL_INC_
#define _SPREQ_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPIRequest
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetCommand
//
// Return the command request for this packet
//
TSP_INLINE int CTSPIRequest::GetCommand() const
{
    return m_iReqType;

}// CTSPIRequest::GetCommand

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetAsynchRequestId
//
// Return the TAPI asynchronous request id associated with this
// command.
//
TSP_INLINE DRV_REQUESTID CTSPIRequest::GetAsynchRequestId() const 
{
    return m_dwRequestId;

}// CTSPIRequest::GetAsynchRequestId

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetRequestName
//
// Returns the request name for this request.
//
TSP_INLINE LPCTSTR CTSPIRequest::GetRequestName() const 
{ 
	return m_pszType;

}// CTSPIRequest::GetRequestName()

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::SetState
//
// Set the current state of this request.
//
TSP_INLINE void CTSPIRequest::SetState(int iState) 
{ 
	m_dwStateTime = GetTickCount();
    m_iReqState = iState;

}// CTSPIRequest::SetState

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetState
//
// Get the state of this request
//
TSP_INLINE int CTSPIRequest::GetState() const 
{
    return m_iReqState;

}// CTSPIRequest::GetState

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetStateTime
//
// Get the time this request entered the current state
//
TSP_INLINE DWORD CTSPIRequest::GetStateTime() const 
{
    return m_dwStateTime;

}// CTSPIRequest::GetStateTime

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::HaveSentResponse
//
// Returns bool indicating whether a response has been sent to TAPI
// about this request.  If this has no request id, then always return
// as if we sent a response.
//
TSP_INLINE bool CTSPIRequest::HaveSentResponse() const 
{ 
    return (GetAsynchRequestId() > 0) ? m_fResponseSent : true;

}// CTSPIRequest::HaveSentResponse

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetConnectionInfo
//
// Return the CTSPIConnection pointer this packet relates to
//
TSP_INLINE CTSPIConnection* CTSPIRequest::GetConnectionInfo() const
{ 
    return m_pConnOwner;

}// CTSPIRequest::GetConnectionInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetLineOwner
//
// Return the CTSPILineConnection pointer this packet relates to
//
TSP_INLINE CTSPILineConnection* CTSPIRequest::GetLineOwner() const
{ 
	// According to ARM, should not throw exception since casting
	// pointer.  Removed try/catch blocks (v3.043)
	return dynamic_cast<CTSPILineConnection*>(m_pConnOwner);

}// CTSPIRequest::GetConnectionInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetPhoneOwner
//
// Return the CTSPIPhoneConnection pointer this packet relates to
//
TSP_INLINE CTSPIPhoneConnection* CTSPIRequest::GetPhoneOwner() const
{ 
	// According to ARM, should not throw exception since casting
	// pointer.  Removed try/catch blocks (v3.043)
	return dynamic_cast<CTSPIPhoneConnection*>(m_pConnOwner);

}// CTSPIRequest::GetPhoneOwner

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetAddressInfo
//
// Return the address information for a request packet.
//
TSP_INLINE CTSPIAddressInfo* CTSPIRequest::GetAddressInfo() const
{                               
    return m_pAddress;
    
}// CTSPIRequest::GetAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::GetCallInfo
//
// Return the CTSPICallAppearance associated with this request.
//
TSP_INLINE CTSPICallAppearance* CTSPIRequest::GetCallInfo() const
{ 
    return m_pCall;

}// CTSPIRequest::GetCallInfo

#endif // _SPREQ_INL_INC_
