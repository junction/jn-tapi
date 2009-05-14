/******************************************************************************/
//                                                                        
// DEVICE.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _SPDEV_INL_INC_
#define _SPDEV_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPIDevice
//
/******************************************************************************/

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetProviderHandle
//
// Returns the handle supplied to TSPI_providerEnumDevices.
//
TSP_INLINE HPROVIDER CTSPIDevice::GetProviderHandle() const
{
	return m_hProvider;
	
}// CTSPIDevice::GetProviderHandle

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AssociateLineWithPhone
//
// Associate a line/phone together the "tapi" way.
//
TSP_INLINE void CTSPIDevice::AssociateLineWithPhone(unsigned int iLine, unsigned int iPhone)
{
	CTSPILineConnection* pLine = GetLineConnectionInfo(iLine);
	CTSPIPhoneConnection* pPhone = GetPhoneConnectionInfo(iPhone);
	if (pLine && pPhone)
	{
		pLine->AddDeviceClass(_T("tapi/phone"), pPhone->GetDeviceID());
		pPhone->AddDeviceClass(_T("tapi/line"), pLine->GetDeviceID());
		pLine->CreateTapiTerminalDeviceClassInfo();
	}

}// CTSPIDevice::AssociateLineWithPhone

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetLineCount
//
// Return the count of lines available on this device.
//
TSP_INLINE unsigned int CTSPIDevice::GetLineCount() const
{
	CEnterCode sLock(this);  // Synch access to object
    return m_arrayLines.size();

}// CTSPIDevice::GetLineCount

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetPhoneCount
//
// Return the count of phones available on this device.
//
TSP_INLINE unsigned int CTSPIDevice::GetPhoneCount() const
{
	CEnterCode sLock(this);  // Synch access to object
    return m_arrayPhones.size();

}// CTSPIDevice::GetPhoneCount

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AddLineConnectionInfo
//
// Add a new line connection object to our device list.
//
TSP_INLINE void CTSPIDevice::AddLineConnectionInfo(CTSPILineConnection* pConn)
{
	CEnterCode sLock(this);  // Synch access to object
	m_arrayLines.push_back(pConn);

}// CTSPIDevice::AddLineConnectionInfo

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AddPhoneConnectionInfo
//
// Add a new line connection object to our device list.
//
TSP_INLINE void CTSPIDevice::AddPhoneConnectionInfo(CTSPIPhoneConnection* pConn)
{
	CEnterCode sLock(this);  // Synch access to object
    m_arrayPhones.push_back(pConn);

}// CTSPIDevice::AddPhoneConnectionInfo

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::MapConnID
//
// This is a map from the permanent line/phone identifiers for each of 
// our line/phone objects to the actual object itself.  It allows the
// derived provider to quickly locate a line or phone based on whatever
// criteria they desire.
//
TSP_INLINE void CTSPIDevice::MapConnID(DWORD dwOldID, DWORD dwNewID, CTSPILineConnection* pObject)
{
	CEnterCode sLock(this);

	// If an old identifier was given, remove it.
	if (dwOldID != 0)
		m_mapLineConnections.erase(dwOldID);

	// Add the new one.  Ignore it if it is the object itself.
	if (dwNewID != 0 && dwNewID != reinterpret_cast<DWORD>(pObject))
		m_mapLineConnections[dwNewID] = pObject;

}// CTSPIDevice::MapConnID

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::MapConnID
//
// This is a map from the permanent line/phone identifiers for each of 
// our line/phone objects to the actual object itself.  It allows the
// derived provider to quickly locate a line or phone based on whatever
// criteria they desire.
//
TSP_INLINE void CTSPIDevice::MapConnID(DWORD dwOldID, DWORD dwNewID, CTSPIPhoneConnection* pObject)
{
	CEnterCode sLock(this);

	// If an old identifier was given, remove it.
	if (dwOldID != 0)
		m_mapPhoneConnections.erase(dwOldID);

	// Add the new one.  Ignore it if it is the object itself.
	if (dwNewID != 0 && dwNewID != reinterpret_cast<DWORD>(pObject))
		m_mapPhoneConnections[dwNewID] = pObject;

}// CTSPIDevice::MapConnID

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::FindLineConnectionByPermanentID
//
// Locate a connection object by it's permanent line/phone identifier.
//
TSP_INLINE CTSPILineConnection* CTSPIDevice::FindLineConnectionByPermanentID(DWORD dwConnID) const
{
	TConnectionMap::const_iterator theIterator = m_mapLineConnections.find(dwConnID);
	if (theIterator != m_mapLineConnections.end())
		return dynamic_cast<CTSPILineConnection*>((*theIterator).second);
	return NULL;

}// CTSPIDevice::FindConnectionByPermanentID

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::FindPhoneConnectionByPermanentID
//
// Locate a connection object by it's permanent line/phone identifier.
//
TSP_INLINE CTSPIPhoneConnection* CTSPIDevice::FindPhoneConnectionByPermanentID(DWORD dwConnID) const
{
	TConnectionMap::const_iterator theIterator = m_mapPhoneConnections.find(dwConnID);
	if (theIterator != m_mapPhoneConnections.end())
		return dynamic_cast<CTSPIPhoneConnection*>((*theIterator).second);
	return NULL;

}// CTSPIDevice::FindPhoneConnectionByPermanentID

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OnAsynchRequestComplete
//
// This function allows you to explicitely complete a request by requestID
// vs. using a request object.  Note that if a request object is associated with
// the request it will NOT be updated! This is best used for lineDevSpecific completions.
//
TSP_INLINE void CTSPIDevice::OnAsynchRequestComplete(DRV_REQUESTID drvRequestID, LONG lResult)
{
	_TSP_ASSERTE(m_lpfnCompletionProc != NULL);
	(*m_lpfnCompletionProc)(drvRequestID, lResult);
#ifdef _DEBUG
	_TSP_DTRACEX(TRC_REQUESTS, _T("OnAsynchRequestComplete: explicit completion of requestID 0x%lx, rc=0x%lx"), drvRequestID, lResult);
#endif
	
}// CTSPIDevice::OnAsynchRequestComplete

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetProviderId
//
// The provider id is assigned by TAPISRV.EXE when a service provider
// is installed through TSPI_providerInstall.  It generally starts
// with zero (the first one) and gets incremented as each new TSP is
// installed.
//
TSP_INLINE DWORD CTSPIDevice::GetProviderID() const
{
    return m_dwProviderId;

}// CTSPIDevice::GetProviderId

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetLineConnectionInfo
//
// Return a line connection object based on a position
//
TSP_INLINE CTSPILineConnection* CTSPIDevice::GetLineConnectionInfo(unsigned int nIndex) const
{
	CEnterCode sLock(this);  // Synch access to object
	return (nIndex < m_arrayLines.size()) ? m_arrayLines[nIndex] : NULL;

}// CTSPIDevice::GetLineConnectionInfo

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetPhoneConnectionInfo
//
// Return a line connection object based on a position.
//
TSP_INLINE CTSPIPhoneConnection* CTSPIDevice::GetPhoneConnectionInfo(unsigned int nIndex) const
{
	CEnterCode sLock(this);  // Synch access to object
	return (nIndex < m_arrayPhones.size()) ? m_arrayPhones[nIndex] : NULL;

}// CTSPIDevice::GetPhoneConnectionInfo

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::FindLineConnectionByDeviceID
//
// Method to locate a line object from line device ID
//
// Note: the line object must be owned by ths device.
//
TSP_INLINE CTSPILineConnection* CTSPIDevice::FindLineConnectionByDeviceID(DWORD dwDeviceId) const
{
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceId);
	if (pLine && pLine->GetDeviceInfo() == this)
		return pLine;
	return NULL;

}// CTSPIDevice::FindLineConnectionByDeviceID

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::FindPhoneConnectionByDeviceID
//
// Method to locate a phone object from phone device ID
//
// Note: the phone object must be owned by ths device.
//
TSP_INLINE CTSPIPhoneConnection* CTSPIDevice::FindPhoneConnectionByDeviceID(DWORD dwDeviceId) const
{
	CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceId);
	if (pPhone && pPhone->GetDeviceInfo() == this)
		return pPhone;
	return NULL;

}// CTSPIDevice::FindPhoneConnectionByDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentActivityCount
//
// This function returns the number of agent activities present.
//
TSP_INLINE unsigned int CTSPIDevice::GetAgentActivityCount() const
{
	CEnterCode sLock(this);
	return m_arrAgentActivities.size();

}// CTSPIDevice::GetAgentActivityCount

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentGroupCount
//
// Returns the count of groups in our array
//
TSP_INLINE unsigned int CTSPIDevice::GetAgentGroupCount() const
{
	CEnterCode sLock(this);
	return m_arrAgentGroups.size();

}// CTSPIDevice::GetAgentGroupCount

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentActivity
//
// This function returns a located agent activity structure
//
TSP_INLINE const TAgentActivity* CTSPIDevice::GetAgentActivity(unsigned int i) const
{
	CEnterCode sLock(this);
	return (i < m_arrAgentActivities.size()) ? m_arrAgentActivities[i] : NULL;

}// CTSPIDevice::GetAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentGroup
//
// Returns the agent group by position.
//
TSP_INLINE const TAgentGroup* CTSPIDevice::GetAgentGroup(unsigned int i) const
{
	CEnterCode sLock(this);
	return (i < m_arrAgentGroups.size()) ? m_arrAgentGroups[i] : NULL;

}// CTSPIDevice::GetAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentSpecificExtensionCount
//
// Returns the count of added-handler extensions in our array
//
TSP_INLINE unsigned int CTSPIDevice::GetAgentSpecificExtensionCount() const
{
	CEnterCode sLock(this);
	return m_arrAgentExtensions.size();

}// CTSPIDevice::GetAgentSpecificExtensionCount

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentSpecificExtensionID
//
// Returns the agent extension by position.
//
TSP_INLINE const TAgentSpecificEntry* CTSPIDevice::GetAgentSpecificExtensionID(unsigned int i) const
{
	CEnterCode sLock(this);
	return (i < m_arrAgentExtensions.size()) ? m_arrAgentExtensions[i] : NULL;

}// CTSPIDevice::GetAgentSpecificExtensionID

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetSwitchInfo
//
// Return the switch/phone information for this connection
//
TSP_INLINE LPCTSTR CTSPIDevice::GetSwitchInfo() const
{                               
    return m_strDevInfo.c_str();

}// CTSPIDevice::GetSwitchInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::SetSwitchInfo
//
// Set the switch/phone information for this connection - should
// be called during providerInit.
//
TSP_INLINE void CTSPIDevice::SetSwitchInfo(LPCTSTR pszInfo)
{                               
    m_strDevInfo = pszInfo;

}// CTSPIDevice::SetSwitchInfo

#endif // _SPDEV_INL_INC_
