/******************************************************************************/
//                                                                        
// DEVICE.CPP - User-interface DEVICE support
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
using namespace tsplibui;

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::~CTSPUIDevice
//
// Destructor for the device object
//
CTSPUIDevice::~CTSPUIDevice()
{
	ResetConfiguration();

}// CTSPUIDevice::~CTSPUIDevice

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::ResetConfiguration
//
// Remove all the line/phone configuration
//
void CTSPUIDevice::ResetConfiguration()
{
	// Delete all our objects
	while (GetLineCount() > 0)
	{
		CTSPUILineConnection* pLine = GetLineConnectionInfo(0);
		m_arrLines.RemoveAt(0);
		delete pLine;
	}

	while (GetPhoneCount() > 0)
	{
		CTSPUIPhoneConnection* pPhone = GetPhoneConnectionInfo(0);
		m_arrPhones.RemoveAt(0);
		delete pPhone;
	}

	int i;
	for (i = 0; i < m_arrGroups.GetSize(); i++)
		delete static_cast<TAgentGroup*>(m_arrGroups[i]);
	m_arrGroups.RemoveAll();

	for (i = 0; i < m_arrActivity.GetSize(); i++)
		delete static_cast<TAgentActivity*>(m_arrActivity[i]);
	m_arrActivity.RemoveAll();

}// CTSPUIDevice::ResetConfiguration

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::read
//
// Reads the object from a stream
//
TStream& CTSPUIDevice::read(TStream& istm)
{
	// Should not have any existing information loaded.
	ASSERT (m_arrLines.GetSize() == 0);
	ASSERT (m_arrPhones.GetSize() == 0);

	// Read our permanent provider ID from the open stream. This is the only
	// piece of information we store in the stream for the device.
	DWORD dwProviderID;
	istm >> dwProviderID;
	ASSERT(dwProviderID == m_dwPermProviderID);

	return istm;

}// CTSPUIDevice::read

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::write
//
// Writes the object to a stream
//
TStream& CTSPUIDevice::write(TStream& ostm) const
{
	// Write our permanent provider id.
	ostm << m_dwPermProviderID;

	return ostm;

}// CTSPUIDevice::write

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::ReadAgentActivity
//
// Stream in an agent activity.  Since the activity is a simple structure
// it doesn't have the stream associated with it.
//
TAgentActivity* CTSPUIDevice::ReadAgentActivity(TStream& istm)
{
	TAgentActivity* pAct = new TAgentActivity;

	try
	{
		istm >> pAct->dwID >> pAct->strName;
	}
	catch (...)
	{
		delete pAct;
		pAct = NULL;
	}
	return pAct;

}// CTSPUIDevice::ReadAgentActivity

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::WriteAgentActivity
//
// Serialize out an agent activity structure into a registry stream
//
void CTSPUIDevice::WriteAgentActivity(const TAgentActivity* pAct, TStream& ostm) const
{
	ASSERT (pAct != NULL);
	if (pAct == NULL)
		return;

	try
	{
		ostm << pAct->dwID << pAct->strName;
	}
	catch (...)
	{
		ASSERT (FALSE);
	}

}// CTSPUIDevice::WriteAgentActivity

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::ReadAgentGroup
//
// Stream in an agent group.  Since the group is a simple structure
// it doesn't have the stream associated with it.
//
TAgentGroup* CTSPUIDevice::ReadAgentGroup(TStream& istm)
{
	TAgentGroup* pGroup = new TAgentGroup;

	try
	{
		istm >> pGroup->GroupID.dwGroupID1 >>
				pGroup->GroupID.dwGroupID2 >>
				pGroup->GroupID.dwGroupID3 >>
				pGroup->GroupID.dwGroupID4 >>
				pGroup->strName;
	}
	catch (...)
	{
		delete pGroup;
		pGroup = NULL;
	}
	return pGroup;

}// CTSPUIDevice::ReadAgentGroup

///////////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::WriteAgentGroup
//
// Serialize out an agent group structure into a registry stream
//
void CTSPUIDevice::WriteAgentGroup(const TAgentGroup* pGroup, TStream& ostm) const
{
	ASSERT (pGroup != NULL);
	if (pGroup == NULL)
		return;

	try
	{
		ostm << pGroup->GroupID.dwGroupID1 <<
				pGroup->GroupID.dwGroupID2 <<
				pGroup->GroupID.dwGroupID3 <<
				pGroup->GroupID.dwGroupID4 <<
				pGroup->strName;
	}
	catch (...)
	{
		ASSERT (FALSE);
	}

}// CTSPUIDevice::WriteAgentGroup

