/******************************************************************************/
//                                                                        
// UILINE.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _UILINE_INL_INC_
#define _UILINE_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPUILineConnection class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SupportsAgents
//
// Returns TRUE if this line supports agents
// 
TSP_INLINE bool CTSPUILineConnection::SupportsAgents() const
{
	return m_fSupportsAgents;

}// CTSPUILineConnection::SupportsAgents

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::EnableAgentSupport
//
// Turns the agent support flag on and off.
//
TSP_INLINE void CTSPUILineConnection::EnableAgentSupport(bool fAgentSupport)
{
	m_fSupportsAgents = fAgentSupport;

}// CTSPUILineConnection::EnableAgentSupport

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetMSPGUID
//
// Sets the GUID of the media service provider for this line.
//
TSP_INLINE void CTSPUILineConnection::SetMSPGUID(const GUID& guid)
{
	m_guidMSP = guid;

}// CTSPUILineConnection::SetMSPGUID

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetProtocolCLSID
//
// Sets the CLSID of the line for TAPI 3.0
//
TSP_INLINE void CTSPUILineConnection::SetProtocolCLSID(const GUID& clsid)
{
	m_clsidProtocol = clsid;

}// CTSPUILineConnection::SetProtocolCLSID

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetAssociatedPhone
//
// Sets the phone associated to this line object
//
TSP_INLINE void CTSPUILineConnection::SetAssociatedPhone(DWORD dwPhoneID)
{
	if (dwPhoneID != m_dwPhoneID)
	{
		CTSPUIPhoneConnection* pPhone = GetAssociatedPhone();
		m_dwPhoneID = dwPhoneID;

		if (pPhone != NULL)
			pPhone->SetAssociatedLine(0xffffffff);

		pPhone = GetAssociatedPhone();
		if (pPhone != NULL)
			pPhone->SetAssociatedLine(m_dwDeviceID);
		else
			m_dwPhoneID = 0xffffffff;
	}

}// CTSPUILineConnection::SetAssociatedPhone

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetAssociatedPhone
//
// Gets the phone associated to this line object
//
TSP_INLINE CTSPUIPhoneConnection* CTSPUILineConnection::GetAssociatedPhone() const
{
	if (m_dwPhoneID != 0xffffffff && GetDeviceInfo() != NULL)
		return GetDeviceInfo()->FindPhoneConnectionByPermanentID(m_dwPhoneID);
	return NULL;

}// CTSPUILineConnection::GetAssociatedPhone

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetLineType
//
// Returns the line type for this line
//
TSP_INLINE int CTSPUILineConnection::GetLineType() const
{
	return m_iType;

}// CTSPUILineConnection::GetLineType

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetAddressCount
//
// Returns the count of addresses in our line array
//
TSP_INLINE unsigned int CTSPUILineConnection::GetAddressCount() const
{
	return m_arrAddress.GetSize();

}// CTSPUILineConnection::GetAddressCount

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetAddress
//
// Return a specific address object
//
TSP_INLINE CTSPUIAddressInfo* CTSPUILineConnection::GetAddress (unsigned int iAddressID) const
{
	return (iAddressID < GetAddressCount()) ?
		m_arrAddress[iAddressID] : NULL;

}// CTSPUILineConnection::GetAddress

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetAddress
//
// Return a specific address object
//
TSP_INLINE CTSPUIAddressInfo* CTSPUILineConnection::GetAddress (LPCTSTR lpszDialableAddr) const
{
	for (unsigned int i = 0; i < GetAddressCount(); i++)
	{
		CTSPUIAddressInfo* pAddr = GetAddress(i);
		if (!lstrcmpi(pAddr->GetDialableAddress(), lpszDialableAddr))
			return pAddr;
	}
	return NULL;

}// CTSPUILineConnection::GetAddress

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetPermanentDeviceID
//
// Returns the device id for this line
//
TSP_INLINE DWORD CTSPUILineConnection::GetPermanentDeviceID() const
{
	return m_dwDeviceID;

}// CTSPUILineConnection::GetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetName
//
// Returns the name for this line
//
TSP_INLINE LPCTSTR CTSPUILineConnection::GetName() const
{
	return m_strName;

}// CTSPUILineConnection::GetName

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetLineType
//
// Returns the line type for this line
//
TSP_INLINE void CTSPUILineConnection::SetLineType(int iType)
{
	ASSERT (iType >= Station && iType <= Other);
	m_iType = iType;

}// CTSPUILineConnection::SetLineType

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetPermanentDeviceID
//
// Returns the device id for this line
//
TSP_INLINE void CTSPUILineConnection::SetPermanentDeviceID(DWORD dwDeviceID)
{
	m_dwDeviceID = dwDeviceID;

}// CTSPUILineConnection::SetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::SetName
//
// Returns the name for this line
//
TSP_INLINE void CTSPUILineConnection::SetName(LPCTSTR pszName)
{
	m_strName = pszName;

}// CTSPUILineConnection::SetName

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::GetDeviceInfo
//
// Return the device information object
//
TSP_INLINE CTSPUIDevice* CTSPUILineConnection::GetDeviceInfo() const
{
	return m_pDevice;

}// CTSPUILineConnection::GetDeviceInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::AddAddress
//
// Add a single address object to our line
//
TSP_INLINE unsigned int CTSPUILineConnection::AddAddress(CTSPUIAddressInfo* pAddr)
{
	ASSERT(pAddr != NULL);
	return m_arrAddress.Add(pAddr);

}// CTSPUILineConnection::AddAddress

///////////////////////////////////////////////////////////////////////////
// CTSPUILineConnection::RemoveAddress
//
// Remove an address on this line
//
TSP_INLINE void CTSPUILineConnection::RemoveAddress(unsigned int iAddressID)
{
	if (iAddressID < GetAddressCount())
		m_arrAddress.RemoveAt(iAddressID);

}// CTSPUILineConnection::RemoveAddress

#endif // _UILINE_INL_INC_
