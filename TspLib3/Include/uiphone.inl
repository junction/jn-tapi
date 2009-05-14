/******************************************************************************/
//                                                                        
// UIPHONE.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _UIPHONE_INL_INC_
#define _UIPHONE_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPUIDevice class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetPhoneCount
//
// Returns the number of phones stored in our internal array
//
TSP_INLINE unsigned int CTSPUIDevice::GetPhoneCount() const
{
	return m_arrPhones.GetSize();

}// CTSPUIDevice::GetPhoneCount

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetPhoneConnection
//
// Returns a specific phone object by index
//
TSP_INLINE CTSPUIPhoneConnection* CTSPUIDevice::GetPhoneConnectionInfo(unsigned int nIndex) const
{
	return (nIndex < GetPhoneCount()) ? 
		m_arrPhones[nIndex] : NULL;

}// CTSPUIDevice::GetPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::AddPhone
//
// Add a Phone to the service provider
//
TSP_INLINE unsigned int CTSPUIDevice::AddPhone(CTSPUIPhoneConnection* pPhone)
{
	pPhone->m_pDevice = this;
	return m_arrPhones.Add(pPhone);

}// CTSPUIDevice::AddPhone

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemovePhone
//
// Remove a Phone from the service provider
//
TSP_INLINE void CTSPUIDevice::RemovePhone(unsigned int iPhone)
{
	if (iPhone < GetPhoneCount())
		m_arrPhones.RemoveAt(iPhone);

}// CTSPUIDevice::RemovePhone

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemovePhone
//
// Remove a Phone from the service provider
//
TSP_INLINE void CTSPUIDevice::RemovePhone(CTSPUIPhoneConnection* pPhone)
{
	for (unsigned int i = 0; i < GetPhoneCount(); i++)
	{
		if (GetPhoneConnectionInfo(i) == pPhone)
		{
			RemovePhone(i);
			break;
		}
	}

}// CTSPUIDevice::RemovePhone

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::FindPhoneConnectionByPermanentID
//
// Returns a specific line object by line id
//
TSP_INLINE CTSPUIPhoneConnection* CTSPUIDevice::FindPhoneConnectionByPermanentID(DWORD dwPermID)
{
	for (unsigned int i = 0; i < GetPhoneCount(); i++)
	{
		CTSPUIPhoneConnection* pPhone = GetPhoneConnectionInfo(i);
		if (pPhone->GetPermanentDeviceID() == dwPermID)
			return pPhone;
	}
	return NULL;

}// CTSPUIDevice::FindPhoneConnectionByPermanentID

/******************************************************************************/
//
// CTSPUIPhoneConnection class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::CTSPUIPhoneConnection
//
// Constructor for the CTSPUIPhoneConnection class
//
TSP_INLINE CTSPUIPhoneConnection::CTSPUIPhoneConnection() : m_pDevice(0),
	m_dwDeviceID(0xffffffff), m_dwLineID(0xffffffff)
{
	m_sizDisplay.cx = m_sizDisplay.cy = 0;
	
}// CTSPUIPhoneConnection::CTSPUIPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::CTSPUIPhoneConnection
//
// Constructor for the CTSPUIPhoneConnection class
//
TSP_INLINE CTSPUIPhoneConnection::CTSPUIPhoneConnection(DWORD dwDeviceID, LPCTSTR pszName) : 
	m_pDevice(0), m_dwDeviceID(dwDeviceID), m_strName(pszName), m_dwLineID(0xffffffff)
{
	m_sizDisplay.cx = m_sizDisplay.cy = 0;

}// CTSPUIPhoneConnection::CTSPUIPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::GetDeviceInfo
//
// Return the device information object
//
TSP_INLINE CTSPUIDevice* CTSPUIPhoneConnection::GetDeviceInfo() const
{
	return m_pDevice;

}// CTSPUIPhoneConnection::GetDeviceInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::SetAssociatedLine
//
// Sets the line associated to this phone object
//
TSP_INLINE void CTSPUIPhoneConnection::SetAssociatedLine(DWORD dwLineID)
{
	if (m_dwLineID != dwLineID)
	{
		CTSPUILineConnection* pLine = GetAssociatedLine();
		m_dwLineID = dwLineID;

		if (pLine != NULL)
			pLine->SetAssociatedPhone(0xffffffff);

		pLine = GetAssociatedLine();
		if (pLine != NULL)
			pLine->SetAssociatedPhone(m_dwDeviceID);
		else
			m_dwLineID = 0xffffffff;
	}

}// CTSPUIPhoneConnection::SetAssociatedLine

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::GetAssociatedLine
//
// Gets the line associated to this phone object
//
TSP_INLINE CTSPUILineConnection* CTSPUIPhoneConnection::GetAssociatedLine() const
{
	if (m_dwLineID != 0xffffffff && GetDeviceInfo() != NULL)
		return GetDeviceInfo()->FindLineConnectionByPermanentID(m_dwLineID);
	return NULL;

}// CTSPUIPhoneConnection::GetAssociatedLine

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::GetPermanentDeviceID
//
// Returns the device id for this line
//
TSP_INLINE DWORD CTSPUIPhoneConnection::GetPermanentDeviceID() const
{
	return m_dwDeviceID;

}// CTSPUIPhoneConnection::GetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::GetName
//
// Returns the name for this line
//
TSP_INLINE LPCTSTR CTSPUIPhoneConnection::GetName() const
{
	return m_strName;

}// CTSPUIPhoneConnection::GetName

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::SetPermanentDeviceID
//
// Returns the device id for this line
//
TSP_INLINE void CTSPUIPhoneConnection::SetPermanentDeviceID(DWORD dwDeviceID)
{
	m_dwDeviceID = dwDeviceID;

}// CTSPUIPhoneConnection::SetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::SetName
//
// Returns the name for this line
//
TSP_INLINE void CTSPUIPhoneConnection::SetName(LPCTSTR pszName)
{
	m_strName = pszName;

}// CTSPUIPhoneConnection::SetName

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::AddUploadBuffer
//
// Adds a new upload buffer to the phone
//
TSP_INLINE int CTSPUIPhoneConnection::AddUploadBuffer (DWORD dwSizeOfBuffer)
{
	return m_arrUploadBuffers.Add(dwSizeOfBuffer);

}// CTSPUIPhoneConnection::AddUploadBuffer

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::AddDownloadBuffer
//
// Adds a new download buffer to the phone
//
TSP_INLINE int CTSPUIPhoneConnection::AddDownloadBuffer (DWORD dwSizeOfBuffer)
{
	return m_arrDownloadBuffers.Add(dwSizeOfBuffer);

}// CTSPUIPhoneConnection::AddDownloadBuffer

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::SetupDisplay
//
// Setup the display for the phone
//
TSP_INLINE void CTSPUIPhoneConnection::SetupDisplay (int iColumns, int iRows, char cChar)
{
	m_sizDisplay.cx = iColumns;
	m_sizDisplay.cy = iRows;
	m_chDisplay = cChar;

}// CTSPUIPhoneConnection::SetupDisplay

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::AddButton
//
// Add a button to the phone unit.
//
TSP_INLINE int CTSPUIPhoneConnection::AddButton (DWORD dwFunction, DWORD dwMode, DWORD dwAvailLampModes, LPCTSTR lpszText)
{
	return m_arrButtons.Add(new tsplibui::CPhoneButtonInfo(dwFunction, dwMode, dwAvailLampModes, lpszText));

}// CTSPUIPhoneConnection::AddButton

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::AddHookSwitchDevice
//
// Add a hook switch device to the unit
//
TSP_INLINE int CTSPUIPhoneConnection::AddHookSwitchDevice(DWORD dwHookSwitchDev, DWORD dwAvailModes, DWORD dwVolume, DWORD dwGain, DWORD dwSettableModes, DWORD dwMonitoredModes, bool fSupportsVolChange, bool fSupportsGainChange)
{
	return m_arrHookswitch.Add(new tsplibui::CPhoneHSInfo(dwHookSwitchDev, dwAvailModes, dwVolume, dwGain, dwSettableModes, dwMonitoredModes, fSupportsVolChange, fSupportsGainChange));

}// CTSPUIPhoneConnection::AddHookSwitchDevice

#endif // _UIPHONE_INL_INC_
