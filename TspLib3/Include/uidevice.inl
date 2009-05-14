/******************************************************************************/
//                                                                        
// UIDEVICE.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _UIDEVIC_INL_INC_
#define _UIDEVICE_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetProviderInfo
//
// Returns the name of the registry key for storing information
//
TSP_INLINE LPCTSTR CServiceProviderUI::GetProviderInfo() const
{
	return m_pszProviderInfo;

}// CServiceProviderUI::GetProviderInfo

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetDeviceCount
//
// Returns the count of devices
//
TSP_INLINE unsigned int CServiceProviderUI::GetDeviceCount() const
{
	return m_arrDevices.GetSize();

}// CServiceProviderUI::GetDeviceCount

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetDeviceByIndex
//
// Returns a device based on the index
//
TSP_INLINE CTSPUIDevice* CServiceProviderUI::GetDeviceByIndex(unsigned int iPos) const
{
	return (iPos < GetDeviceCount()) ? 
		m_arrDevices[iPos] : NULL;

}// CServiceProviderUI::GetDeviceByIndex

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::GetDevice
//
// Returns a device based on the provider id
//
TSP_INLINE CTSPUIDevice* CServiceProviderUI::GetDevice(DWORD dwProviderID) const
{
	for (unsigned int i = 0; i < GetDeviceCount(); i++)
	{
		CTSPUIDevice* pDevice = m_arrDevices[i];
		if (pDevice->GetProviderID() == dwProviderID)
			return pDevice;
	}
	return NULL;

}// CServiceProviderUI::GetDevice

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::AddDevice
//
// Adds a new device to the provider
//
TSP_INLINE unsigned int CServiceProviderUI::AddDevice(DWORD dwPermProviderID)
{
	CTSPUIDevice* pDevice = reinterpret_cast<CTSPUIDevice*>(m_pObjects[0]->CreateObject());
	pDevice->m_dwPermProviderID = dwPermProviderID;
	return AddDevice(pDevice);

}// CServiceProviderUI::AddDevice

///////////////////////////////////////////////////////////////////////////
// CServiceProviderUI::AddDevice
//
// Adds a new device to the provider
//
TSP_INLINE unsigned int CServiceProviderUI::AddDevice(CTSPUIDevice* pDevice)
{
	return m_arrDevices.Add(pDevice);

}// CServiceProviderUI::AddDevice

/******************************************************************************/
//
// CTSPUIDevice class functions
//
/****************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::CTSPUIDevice
//
// Constructor for the device object
//
TSP_INLINE CTSPUIDevice::CTSPUIDevice() : m_dwPermProviderID(0)
{
}// CTSPUIDevice::CTSPUIDevice

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::CTSPUIDevice
//
// Constructor for the device object
//
TSP_INLINE CTSPUIDevice::CTSPUIDevice(DWORD dwPermProvID) : m_dwPermProviderID(dwPermProvID)
{
}// CTSPUIDevice::CTSPUIDevice

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetProviderID
//
// Returns the permanent provider ID for this device
//
TSP_INLINE DWORD CTSPUIDevice::GetProviderID() const
{
	return m_dwPermProviderID;

}// CTSPUIDevice::GetProviderID

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetLineCount
//
// Returns the number of lines stored in our internal array
//
TSP_INLINE unsigned int CTSPUIDevice::GetLineCount() const
{
	return m_arrLines.GetSize();

}// CTSPUIDevice::GetLineCount

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetLineConnectionInfo
//
// Returns a specific line object by index
//
TSP_INLINE CTSPUILineConnection* CTSPUIDevice::GetLineConnectionInfo(unsigned int nIndex) const
{
	return (nIndex < GetLineCount()) ? 
		m_arrLines[nIndex] : NULL;

}// CTSPUIDevice::GetLineConnectionInfo

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::AddLine
//
// Add a line to the service provider
//
TSP_INLINE unsigned int CTSPUIDevice::AddLine(CTSPUILineConnection* pLine)
{
	pLine->m_pDevice = this;
	return m_arrLines.Add(pLine);

}// CTSPUIDevice::AddLine

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemoveLine
//
// Remove a line from the service provider
//
TSP_INLINE void CTSPUIDevice::RemoveLine(unsigned int iLine)
{
	if (iLine < GetLineCount())
		m_arrLines.RemoveAt(iLine);

}// CTSPUIDevice::RemoveLine

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemoveLine
//
// Remove a line from the service provider
//
TSP_INLINE void CTSPUIDevice::RemoveLine(CTSPUILineConnection* pLine)
{
	for (unsigned int i = 0; i < GetLineCount(); i++)
	{
		if (GetLineConnectionInfo(i) == pLine)
		{
			RemoveLine(i);
			break;
		}
	}

}// CTSPUIDevice::RemoveLine

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::FindLineConnectionByPermanentID
//
// Returns a specific line object by line id
//
TSP_INLINE CTSPUILineConnection* CTSPUIDevice::FindLineConnectionByPermanentID(DWORD dwPermID)
{
	for (unsigned int i = 0; i < GetLineCount(); i++)
	{
		CTSPUILineConnection* pLine = GetLineConnectionInfo(i);
		if (pLine->GetPermanentDeviceID() == dwPermID)
			return pLine;
	}
	return NULL;

}// CTSPUIDevice::FindLineConnectionByPermanentID

#endif // _UIDEVICE_INL_INC_
