/******************************************************************************/
//                                                                        
// SERIALIZE.CPP - Serialization (persistant) data support
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains the serialization support for the TSP-side of the
// TSP++ library.  It allows us to read/write the information which was stored into
// the registry by the v3.0 extensions of TSPUI.LIB
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <ctype.h>
#include <regstream.h>

/*-----------------------------------------------------------------------------*/
// GLOBALS and CONSTANTS
/*-----------------------------------------------------------------------------*/
static const TCHAR * const gszUseTspUI = _T("UseTspUI");
static const TCHAR * const gszTotalLines = _T("LineCount");
static const TCHAR * const gszTotalPhones = _T("PhoneCount");
static const TCHAR * const gszTotalAgentActivities = _T("AgentActivityCount");
static const TCHAR * const gszTotalAgentGroups = _T("AgentGroupCount");

///////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ReadDeviceCount
//
// Reads the count of devices from the registry
//
bool CServiceProvider::ReadDeviceCount(DWORD dwProviderID, 
									   LPDWORD lpdwNumLines, LPDWORD lpdwNumPhones,
									   LPDWORD lpdwNumAct, LPDWORD lpdwNumGroups)
{
	if (ReadProfileDWord(0, gszUseTspUI, 0) == 1)
	{
		if (lpdwNumLines != NULL)
			*lpdwNumLines = ReadProfileDWord(dwProviderID, gszTotalLines, 0);
		if (lpdwNumPhones != NULL)
			*lpdwNumPhones = ReadProfileDWord(dwProviderID, gszTotalPhones, 0);
		if (lpdwNumAct != NULL)
			*lpdwNumAct = ReadProfileDWord(dwProviderID, gszTotalAgentActivities, 0);
		if (lpdwNumGroups != NULL)
			*lpdwNumGroups = ReadProfileDWord(dwProviderID, gszTotalAgentGroups);
		return true;
	}
	return false;

}// CServiceProvider::ReadDeviceCount

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AllocStream
//
// Allocates a stream to read persistant data from
//
TStream* CTSPIDevice::AllocStream()
{
	return (GetSP()->ReadProfileDWord(0, gszUseTspUI, 0)) ?
		new tsplib::TRegstream(GetProviderID(), GetSP()->GetProviderInfo()) : NULL;

}// CTSPIDevice::AllocStream

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::LoadObjects
//
// This function loads all the objects stored in the registry section for
// this service provider.
//
void CTSPIDevice::LoadObjects(TStream& rs)
{
	// Read the agent activities
	unsigned int iCount = static_cast<unsigned int>(GetSP()->ReadProfileDWord(GetProviderID(), gszTotalAgentActivities, 0));
	if (iCount > 0)
	{
		// Read each activity into a structure
		for (unsigned int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			TAgentActivity* pAct = ReadAgentActivity(rs);
			_TSP_ASSERT (pAct != NULL);
			if (pAct != NULL)
			{
				_TSP_DTRACE(_T("Added agent activity #%d: [0x%lx] %s\n"), i+1, 
							pAct->dwID, pAct->strName.c_str());
				_TSP_ASSERT (DoesAgentActivityExist(pAct->dwID) == NULL);
				m_arrAgentActivities.push_back(pAct);
			}
		}
	}

	// Read the agent groups
	iCount = static_cast<unsigned int>(GetSP()->ReadProfileDWord(GetProviderID(), gszTotalAgentGroups, 0));
	if (iCount > 0)
	{
		// Read each group into a structure
		for (unsigned int i = 0; i < iCount; i++)
		{
			// Read our information from the registry
			TAgentGroup* pGroup = ReadAgentGroup(rs);
			_TSP_ASSERT (pGroup != NULL);
			if (pGroup != NULL)
			{
				_TSP_DTRACE(_T("Added agent group #%d: [0x%lx 0x%lx 0x%lx 0x%lx] %s\n"), i+1, 
							pGroup->GroupID.dwGroupID1, pGroup->GroupID.dwGroupID2, pGroup->GroupID.dwGroupID3, pGroup->GroupID.dwGroupID4,
							pGroup->strName.c_str());
				_TSP_ASSERT (DoesAgentGroupExist(pGroup->GroupID.dwGroupID1, pGroup->GroupID.dwGroupID2, pGroup->GroupID.dwGroupID3, pGroup->GroupID.dwGroupID4) == NULL);
				m_arrAgentGroups.push_back(pGroup);
			}
		}
	}

}// CTSPIDevice::LoadObjects

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::SaveObjects
//
// This function saves all the objects into the registry section for
// this service provider.
//
void CTSPIDevice::SaveObjects(TStream& ostm)
{
	// Write the agent activities
	int nCount = static_cast<int>(m_arrAgentActivities.size());
	GetSP()->WriteProfileDWord(GetProviderID(), gszTotalAgentActivities, nCount);
	if (nCount > 0)
	{
		for (int i = 0; i < nCount; i++)
			WriteAgentActivity(m_arrAgentActivities[i], ostm);
	}

	// Write the agent groups
	nCount = static_cast<int>(m_arrAgentGroups.size());
	GetSP()->WriteProfileDWord(GetProviderID(), gszTotalAgentGroups, nCount);
	if (nCount > 0)
	{
		for (int i = 0; i < nCount; i++)
			WriteAgentGroup(m_arrAgentGroups[i], ostm);
	}

}// CTSPIDevice::SaveObjects

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::ReadAgentActivity
//
// Stream in an agent activity.  Since the activity is a simple structure
// it doesn't have the stream associated with it.
//
TAgentActivity* CTSPIDevice::ReadAgentActivity(TStream& istm)
{
	TAgentActivity* pAct = new TAgentActivity;

	try
	{
		istm >> pAct->dwID >> pAct->strName;
	}
	catch (...)
	{
		delete pAct;
		throw;
	}
	return pAct;

}// CTSPIDevice::ReadAgentActivity

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::WriteAgentActivity
//
// Serialize out an agent activity structure into a registry stream
//
void CTSPIDevice::WriteAgentActivity(const TAgentActivity* pAct, TStream& ostm) const
{
	_TSP_ASSERT (pAct != NULL);
	if (pAct == NULL)
		return;

	try
	{
		ostm << pAct->dwID << pAct->strName;
	}
	catch (...)
	{
		_TSP_ASSERT (FALSE);
	}

}// CTSPIDevice::WriteAgentActivity

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::ReadAgentGroup
//
// Stream in an agent group.  Since the group is a simple structure
// it doesn't have the stream associated with it.
//
TAgentGroup* CTSPIDevice::ReadAgentGroup(TStream& istm)
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
		throw;
	}
	return pGroup;

}// CTSPIDevice::ReadAgentGroup

///////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::WriteAgentGroup
//
// Serialize out an agent group structure into a registry stream
//
void CTSPIDevice::WriteAgentGroup(const TAgentGroup* pGroup, TStream& ostm) const
{
	_TSP_ASSERT (pGroup != NULL);
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
		_TSP_ASSERT (FALSE);
	}

}// CTSPIDevice::WriteAgentGroup


