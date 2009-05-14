/******************************************************************************/
//                                                                        
// SERIALIZE.CPP - Serialization (persistant) data support
//                                                                        
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the methods which can be called fromt eh
// user-interface functions (UIDLL) of the service provider.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

#include "stdafx.h"
#include <ctype.h>
#include <regstream.h>
using namespace tsplibui;

IMPLEMENT_DYNCREATE(_tspuiBaseObject, CObject)
IMPLEMENT_DYNCREATE(CTSPUIDevice, _tspuiBaseObject)
IMPLEMENT_DYNCREATE(CTSPUILineConnection, _tspuiBaseObject)
IMPLEMENT_DYNCREATE(CTSPUIPhoneConnection, _tspuiBaseObject)
IMPLEMENT_DYNCREATE(CTSPUIAddressInfo, _tspuiBaseObject)

/*-----------------------------------------------------------------------------*/
// GLOBALS and CONSTANTS
/*-----------------------------------------------------------------------------*/
static LPCTSTR gszTotalLines = _T("LineCount");
static LPCTSTR gszTotalPhones = _T("PhoneCount");
static LPCTSTR gszTotalAgentActivities = _T("AgentActivityCount");
static LPCTSTR gszTotalAgentGroups = _T("AgentGroupCount");
static LPCTSTR gszUseTspUI = _T("UseTspUI");
static LPCTSTR gszTotalDevices = _T("DeviceCount");
static LPCTSTR gszDevice = _T("Device%ld");

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::InitInstance
//
// Load our configuration from the registry if it is stored there.
//
BOOL CServiceProvider::InitInstance()
{
	// If the registry contains information about objects, then load the
	// information into our internal arrays.
	if (ReadProfileDWord(0, gszUseTspUI, 0) == 1)
		LoadObjects();
	return CWinApp::InitInstance();

}// CServiceProvider::InitInstance

///////////////////////////////////////////////////////////////////////////////
// TSPUIDevice::AllocStream
//
// Allocate the stream object for this provider and device
//
TStream* CTSPUIDevice::AllocStream()
{
	// If we are not using our new extensions, don't read any objects.
	if (GetUISP()->ReadProfileDWord(0, gszUseTspUI, 0) == 0)
		return NULL;
	return new tsplib::TRegstream(GetProviderID(), GetUISP()->GetProviderInfo());

}// TSPUIDevice::AllocStream

///////////////////////////////////////////////////////////////////////////////
// CServiceProvider::LoadObjects
//
// This function loads all the objects stored in the registry section for
// this service provider.
//
void CServiceProvider::LoadObjects()
{
	// Should not have any existing information loaded.
	ASSERT (m_arrDevices.GetSize() == 0);

	// Load the device objects into our internal array. Each device will load
	// the line/phone/agent information.
	unsigned int iCount = static_cast<unsigned int>(ReadProfileDWord(0, gszTotalDevices, 0));
	if (iCount > 0)
	{
		// Read each device definition from the registry and load it into
		// a CTSPUIDevice object.  These objects are a "subset" of the real objects in the TSP.
		for (unsigned int i = 0; i < iCount; i++)
		{
			// Get the permanent provider id for the given device position
			TCHAR chBuff[30];
			wsprintf(chBuff, gszDevice, i);
			DWORD dwProviderID = ReadProfileDWord(0, chBuff, i);
			if (dwProviderID != 0)
			{
				// Create the device object for this entry
				CTSPUIDevice* pDevice = dynamic_cast<CTSPUIDevice*>(m_pObjects[0]->CreateObject());
				pDevice->m_dwPermProviderID = dwProviderID;

				// Create the stream object to read the device information from.
				TStream* pStream = pDevice->AllocStream();
				if (pStream != NULL)
				{
					if (pStream->open())
					{
						*pStream >> *pDevice;
						pDevice->LoadFromStream(*pStream);
					}
					pStream->close();
					delete pStream;
				}
				m_arrDevices.Add(pDevice);
			}
		}
	}

}// CServiceProvider::LoadObjects

///////////////////////////////////////////////////////////////////////////////
// CServiceProvider::SaveObjects
//
// Save our objects into the registry
//
void CServiceProvider::SaveObjects()
{
	// If we have no devices then remove the stream key so the TSP
	// will not load any information from the registry entries.
	if (GetDeviceCount() == 0)
	{
		DeleteProfile(0, gszUseTspUI);
		return;
	}

	// If we have some objects to save, write out that we are using
	// our new UI extensions.
	WriteProfileDWord(0, gszUseTspUI, 1);

	// Write out the devices
	WriteProfileDWord(0, gszTotalDevices, static_cast<DWORD>(GetDeviceCount()));
	for (unsigned int i = 0; i < GetDeviceCount(); i++)
	{
		// Get the object to store
		CTSPUIDevice* pDevice = GetDeviceByIndex(i);
		ASSERT (pDevice != NULL);

		TStream* pStream = pDevice->AllocStream();
		if (pStream != NULL)
		{
			if (pStream->open())
			{
				*pStream << *pDevice;
				pDevice->SaveIntoStream(*pStream);

				// Write our key so we can find it later.
				TCHAR chBuff[30];
				wsprintf(chBuff, gszDevice, i);
				WriteProfileDWord(0, chBuff, pDevice->GetProviderID());
			}
			pStream->close();
			delete pStream;
		}
	}

}// CServiceProvider::SaveObjects

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::LoadFromStream
//
// Sub-loader function which loads all the line, phone and agent 
// information for a device. This was divided from the CTSPUIDevice::read
// function so that stream positioning doesn't affect return streams
// during a read event.
//
void CTSPUIDevice::LoadFromStream(TStream& istm)
{
	// Start with the line objects.  Read the total number of lines/ stored in the registry.
	int iCount = static_cast<int>(GetUISP()->ReadProfileDWord(m_dwPermProviderID, gszTotalLines, 0));
	if (iCount > 0)
	{
		// Read each line definition from the registry and load it into
		// a CTSPUILineConnectionLineConnection object.  These objects are a "subset" of
		// the real objects in the TSP.
		for (int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			CTSPUILineConnection* pLine = dynamic_cast<CTSPUILineConnection*>(GetUISP()->m_pObjects[1]->CreateObject());
			istm >> *pLine;

			// Add it to our array
			m_arrLines.Add(pLine);
		}
	}

	// Read the total number of phones stored in the registry.
	iCount = static_cast<int>(GetUISP()->ReadProfileDWord(m_dwPermProviderID, gszTotalPhones, 0));
	if (iCount > 0)
	{
		// Read each phone definition from the registry and load it into
		// a CTSPUIPhoneConnectionLineConnection object.  These objects are a "subset" of
		// the real objects in the TSP.
		for (int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			CTSPUIPhoneConnection* pPhone = dynamic_cast<CTSPUIPhoneConnection*>(GetUISP()->m_pObjects[3]->CreateObject());
			istm >> *pPhone;

			// Add it to our array
			m_arrPhones.Add(pPhone);
		}
	}

	// Read the agent activities
	iCount = static_cast<int>(GetUISP()->ReadProfileDWord(m_dwPermProviderID, gszTotalAgentActivities, 0));
	if (iCount > 0)
	{
		// Read each activity into a structure
		for (int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			TAgentActivity* pAct = ReadAgentActivity(istm);
			ASSERT (pAct != NULL);
			if (pAct != NULL)
				m_arrActivity.Add(pAct);
		}
	}

	// Read the agent groups
	iCount = static_cast<int>(GetUISP()->ReadProfileDWord(m_dwPermProviderID, gszTotalAgentGroups, 0));
	if (iCount > 0)
	{
		// Read each activity into a structure
		for (int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			TAgentGroup* pGroup = ReadAgentGroup(istm);
			ASSERT (pGroup != NULL);
			if (pGroup != NULL)
				m_arrGroups.Add(pGroup);
		}
	}

}// CTSPUIDevice::LoadFromStream

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::SaveIntoStream
//
// Sub-saver function which stores all the line, phone and agent 
// information for a device. This was divided from the CTSPUIDevice::write
// function so that stream positioning doesn't affect return streams
// during a read event.
//
void CTSPUIDevice::SaveIntoStream(TStream& ostm)
{
	// Write the counts.. this will create our registry section for this
	// device if it doesn't exist.
	GetUISP()->WriteProfileDWord(m_dwPermProviderID, gszTotalLines, static_cast<DWORD>(GetLineCount()));
	GetUISP()->WriteProfileDWord(m_dwPermProviderID, gszTotalPhones, static_cast<DWORD>(GetPhoneCount()));
	GetUISP()->WriteProfileDWord(m_dwPermProviderID, gszTotalAgentActivities, static_cast<DWORD>(GetAgentActivityCount()));
	GetUISP()->WriteProfileDWord(m_dwPermProviderID, gszTotalAgentGroups, static_cast<DWORD>(GetAgentGroupCount()));

	// Start with the line objects.
	unsigned int i;
	for (i = 0; i < GetLineCount(); i++)
		ostm << *GetLineConnectionInfo(i);

	// Next store the phone objects
	for (i = 0; i < GetPhoneCount(); i++)
		ostm << *GetPhoneConnectionInfo(i);

	// Next store the agent activities
	for (i = 0; i < GetAgentActivityCount(); i++)
		WriteAgentActivity(GetAgentActivity(i), ostm);

	// Next store the agent groups
	for (i = 0; i < GetAgentGroupCount(); i++)
		WriteAgentGroup(GetAgentGroup(i), ostm);

}// CTSPUIDevice::SaveIntoStream

///////////////////////////////////////////////////////////////////////////////
// operator<< for _tspuiBaseObject
//
// Allows all our derived data objects to be serialized out.
//
TStream& operator<<(TStream& ostm, const _tspuiBaseObject& bobj)
{
	try
	{
		return( bobj.write( ostm ) );
	}
#ifdef _DEBUG
	catch (std::exception& e)
	{
		TRACE(_T("Caught exception [%s] writing information to stream\n"), e.what());
#else
	catch (...)
	{
#endif
		return ostm;
	}
}

///////////////////////////////////////////////////////////////////////////////
// operator>> for _tspuiBaseObject
//
// Allows all our derived data objects to be serialized out.
//
TStream& operator>>(TStream& istm, _tspuiBaseObject& bobj)
{
	try
	{
		return( bobj.read( istm ) );
	}
#ifdef _DEBUG
	catch (std::exception& e)
	{
		TRACE(_T("Caught exception [%s] reading information from stream\n"), e.what());
#else
	catch (...)
	{
#endif
		return istm;
	}
}
