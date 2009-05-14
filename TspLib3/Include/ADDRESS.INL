/******************************************************************************/
//                                                                        
// ADDRESS.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _SPADDR_INL_INC_
#define _SPADDR_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPIAddressInfo class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddAsynchRequest
//
// This method adds a request to a particular connection.  It
// will add the request to the device list this connection belongs
// to.
//
TSP_INLINE int CTSPIAddressInfo::AddAsynchRequest(CTSPIRequest* pReq)
{
    return GetLineOwner()->AddAsynchRequest(pReq);

}// CTSPIAddressInfo::AddAsynchRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressID
//
// Return the address identifier for this address.  This is always a
// numeric number from 0-numAddr on the line owner.
//
TSP_INLINE DWORD CTSPIAddressInfo::GetAddressID() const
{
    return m_dwAddressID;

}// CTSPIAddressInfo::GetAddressID

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CanSupportMediaModes
//
// Return whether this address can support the requested media modes.
// The parameter passed is a collection of LINEMEDIAMODE_xxxx constants.
//
TSP_INLINE bool CTSPIAddressInfo::CanSupportMediaModes (DWORD dwMediaModes) const
{
	return (GetAvailableMediaModes() == 0) ? true :
			((GetAvailableMediaModes() & (dwMediaModes & ~LINEMEDIAMODE_UNKNOWN)) != 0) ?
				true : false;

}// CTSPIAddressInfo::CanSupportMediaModes

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetLineOwner
//
// Return the line owner for this address
//
TSP_INLINE CTSPILineConnection* CTSPIAddressInfo::GetLineOwner() const
{
    return m_pLine;

}// CTSPIAddressInfo::GetLineOwner

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCallCount
//
// Return the total number of allocated CTSPICallAppearance object in
// the call array.  Note that some of the objects may be idle.
//
TSP_INLINE int CTSPIAddressInfo::GetCallCount() const
{
	CEnterCode sLock(this);  // Synch access to object
	return m_lstCalls.size();
	
}// CTSPIAddressInfo::GetCallCount

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetDialableAddress
//
// Return the dialable address (phone#) for this address
//
TSP_INLINE LPCTSTR CTSPIAddressInfo::GetDialableAddress() const
{ 
	if (m_strAddress.empty() == false)
		return m_strAddress.c_str();
	return _T("");

}// CTSPIAddressInfo::GetDialableAddress

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetName
//
// Return the name given to us when the address was created.
//
TSP_INLINE LPCTSTR CTSPIAddressInfo::GetName() const
{ 
	if (m_strName.empty() == false)
		return m_strName.c_str();
	return _T("");

}// CTSPIAddressInfo::GetName

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetName
//
// Changes the name of the address object internally.
//
TSP_INLINE void CTSPIAddressInfo::SetName (LPCTSTR pwszName)
{
	m_strName = pwszName;
	OnAddressCapabiltiesChanged();

}// CTSPIAddressInfo::SetName

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetDialableAddress
//
// Changes the address of the address object internally.
//
TSP_INLINE void CTSPIAddressInfo::SetDialableAddress (LPCTSTR pwszAddr, bool fShowInCaps/*=true*/)
{
	if (pwszAddr != NULL)
		m_strAddress = GetSP()->GetDialableNumber(pwszAddr);
	else
	{
		m_strAddress.erase();
		m_strAddress = _T("");
	}

	if (fShowInCaps == false)
		m_dwFlags |= NotUseDN;
	else
	{
		m_dwFlags &= ~NotUseDN;
		OnAddressCapabiltiesChanged();
	}

}// CTSPIAddressInfo::SetDialableAddress

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressCaps
//
// Return our "partial" address caps structure.
//
TSP_INLINE LPLINEADDRESSCAPS CTSPIAddressInfo::GetAddressCaps()
{ 
    return &m_AddressCaps;

}// CTSPIAddressInfo::GetAddressCaps

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressStatus
//
// Return our "partial" address status structure.
//
TSP_INLINE LPLINEADDRESSSTATUS CTSPIAddressInfo::GetAddressStatus()
{ 
    return &m_AddressStatus;

}// CTSPIAddressInfo::GetAddressStatus

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressCaps
//
// Return our "partial" address caps structure.
//
TSP_INLINE const LINEADDRESSCAPS* CTSPIAddressInfo::GetAddressCaps() const
{ 
    return &m_AddressCaps;

}// CTSPIAddressInfo::GetAddressCaps

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressStatus
//
// Return our "partial" address status structure.
//
TSP_INLINE const LINEADDRESSSTATUS* CTSPIAddressInfo::GetAddressStatus() const
{ 
    return &m_AddressStatus;

}// CTSPIAddressInfo::GetAddressStatus

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetDialingParameters
//
// Return the dialing parameters for this address (if any).
//
TSP_INLINE const LINEDIALPARAMS* CTSPIAddressInfo::GetDialingParameters() const
{
	return &m_DialParams; 

}// CTSPIAddressInfo::GetDialingParameters

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCompletionMessageCount
//
// Return the total number of completion messages available.
//
TSP_INLINE unsigned int CTSPIAddressInfo::GetCompletionMessageCount() const
{          
    return m_AddressCaps.dwNumCompletionMessages;
    
}// CTSPIAddressInfo::GetCompletionMessageCount

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCompletionMessage
//
// Return a particular completion message.
//
TSP_INLINE TString CTSPIAddressInfo::GetCompletionMessage (unsigned int i) const
{
	CEnterCode sLock(this);  // Synch access to object
    return (i < m_arrCompletionMsgs.size()) ? m_arrCompletionMsgs[i] : NULL;
                                          
}// CTSPIAddressInfo::GetCompletionMessage

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAvailableMediaModes
//
// Return the available media modes for this address.
//
TSP_INLINE DWORD CTSPIAddressInfo::GetAvailableMediaModes () const
{
    return m_AddressCaps.dwAvailableMediaModes;

}// CTSPIAddressInfo::GetAvailableMediaModes

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CanMakeCalls
//
// Return whether this address can make calls EVER
//
TSP_INLINE bool CTSPIAddressInfo::CanMakeCalls() const
{                                 
    return ((m_dwFlags & OutputAvail) == OutputAvail);
    
}// CTSPIAddressInfo::CanMakeCalls

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CanAnswerCalls
//
// Return whether this address can answer calls EVER
//
TSP_INLINE bool CTSPIAddressInfo::CanAnswerCalls() const
{                                 
    return ((m_dwFlags & InputAvail) == InputAvail);
    
}// CTSPIAddressInfo::CanAnswerCalls

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCurrentRate
//
// Get the current data rate
//
TSP_INLINE DWORD CTSPIAddressInfo::GetCurrentRate() const
{                                   
    return m_dwCurrRate;
    
}// CTSPIAddressInfo::GetCurrentRate

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCurrentBearerMode
//
// Get the current bearer mode for this address.  This cooresponds to
// the Quality of Service (QOS) for the address.
//
TSP_INLINE DWORD CTSPIAddressInfo::GetBearerMode() const
{   
    return m_dwBearerMode;
    
}// CTSPIAddressInfo::GetCurrentBearerMode

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetTerminalInformation
//
// Return the terminal information for the specified terminal.
//
TSP_INLINE DWORD CTSPIAddressInfo::GetTerminalInformation (unsigned int i) const
{                                           
	CEnterCode sLock(this);  // Synch access to object
	return (i < m_arrTerminals.size()) ? m_arrTerminals[i] : 0;

}// CTSPIAddressInfo::GetTerminalInformation

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCallTreatmentName
//
// Return the name for the specified call treatment value.
//
TSP_INLINE TString CTSPIAddressInfo::GetCallTreatmentName (DWORD dwCallTreatment) const
{
	TMapDWordToString::const_iterator pPos = m_mapCallTreatment.find(dwCallTreatment);
	if (pPos != m_mapCallTreatment.end())
		return (*pPos).second;
	return _T("");

}// CTSPIAddressInfo::GetCallTreatmentName

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPIAddressInfo::AddDeviceClass (LPCTSTR pszClass, DWORD dwData)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, &dwData, sizeof(DWORD));

}// CTSPIAddressInfo::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPIAddressInfo::AddDeviceClass (LPCTSTR pszClass, LPCTSTR lpszBuff, DWORD dwType)
{
	if (dwType == -1L)
		dwType = m_pLine->m_LineCaps.dwStringFormat;
	return AddDeviceClass (pszClass, dwType, const_cast<LPTSTR>(lpszBuff), (lstrlen(lpszBuff)+1) * sizeof(TCHAR));

}// CTSPIAddressInfo::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPIAddressInfo::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, const_cast<LPTSTR>(lpszBuff), (lstrlen(lpszBuff)+1) * sizeof(TCHAR), hHandle);

}// CTSPIAddressInfo::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPIAddressInfo::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, lpBuff, dwSize, hHandle);

}// CTSPIAddressInfo::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPIAddressInfo::AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle)
{
	CEnterCode sLock(this);
	return GetSP()->AddDeviceClassInfo (m_arrDeviceClass, pszClass, dwFormat, lpBuff, dwSize, hHandle);

}// CTSPIAddressInfo::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::RemoveDeviceClass
//
// Remove a device class list object.
//
TSP_INLINE bool CTSPIAddressInfo::RemoveDeviceClass (LPCTSTR pszClass)
{
	CEnterCode sLock(this);
	return GetSP()->RemoveDeviceClassInfo (m_arrDeviceClass, pszClass);	

}// CTSPIAddressInfo::RemoveDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetDeviceClass
//
// Return the device class information for a specified name.
//
TSP_INLINE DEVICECLASSINFO* CTSPIAddressInfo::GetDeviceClass(LPCTSTR pszClass)
{
	CEnterCode sLock(this);
	return GetSP()->FindDeviceClassInfo (m_arrDeviceClass, pszClass);

}// CTSPIAddressInfo::GetDeviceClass

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentCaps
//
// Return our "partial" address caps structure.
//
TSP_INLINE TAgentCaps* CTSPIAddressInfo::GetAgentCaps()
{ 
    return &m_AgentCaps;

}// CTSPIAddressInfo::GetAgentCaps

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentStatus
//
// Return our "partial" address status structure.
//
TSP_INLINE TAgentStatus* CTSPIAddressInfo::GetAgentStatus()
{ 
    return &m_AgentStatus;

}// CTSPIAddressInfo::GetAgentStatus

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentCaps
//
// Return our "partial" address caps structure.
//
const TSP_INLINE TAgentCaps* CTSPIAddressInfo::GetAgentCaps() const
{ 
    return &m_AgentCaps;

}// CTSPIAddressInfo::GetAgentCaps

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentStatus
//
// Return our "partial" address status structure.
//
const TSP_INLINE TAgentStatus* CTSPIAddressInfo::GetAgentStatus() const
{ 
    return &m_AgentStatus;

}// CTSPIAddressInfo::GetAgentStatus

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCurrentAgentState
//
// Returns the current agent state
//
TSP_INLINE DWORD CTSPIAddressInfo::GetCurrentAgentState() const
{
	return m_AgentStatus.dwState;

}// CTSPIAddressInfo::GetCurrentAgentState

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCurrentAgentGroupCount
//
// Returns the current count of groups associated with this agent
//
TSP_INLINE unsigned int CTSPIAddressInfo::GetCurrentAgentGroupCount() const
{
	return m_AgentStatus.arrGroups.size();

}// CTSPIAddressInfo::GetCurrentAgentGroupCount

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCurrentAgentGroup
//
// Returns a specific agent group by index
//
TSP_INLINE const TAgentGroup* CTSPIAddressInfo::GetCurrentAgentGroup(unsigned int i) const
{
	return (i < m_AgentStatus.arrGroups.size()) ? m_AgentStatus.arrGroups[i] : NULL;

}// CTSPIAddressInfo::GetCurrentAgentGroup

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAddressType
//
// Retrieves the address type assigned to this address object
//
TSP_INLINE DWORD CTSPIAddressInfo::GetAddressType() const
{
	return m_dwAddressType;

}// CTSPIAddressInfo::GetAddressType

#endif // _SPADDR_INL_INC_
