/******************************************************************************/
//                                                                        
// AGENT.CPP - Source code for the agent support
//                                                                        
// Copyright (C) 2001-2002 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the source to manage the line objects which are 
// held by the CTSPIDevice.                                               
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

// Correct a mistype in the TAPI.H file shipped with VC++
#ifndef LINEAGENTFEATURE_GETAGENTGROUPLIST
#define LINEAGENTFEATURE_GETAGENTGROUPLIST (LINEAGENTFEATURE_GETAGENTGROUP)
#endif

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnAgentCapabiltiesChanged
//
// The capabilities for this agent have changed, notify TAPI
//
void CTSPIAddressInfo::OnAgentCapabiltiesChanged()
{
	GetLineOwner()->Send_TAPI_Event(NULL, LINE_AGENTSTATUS, 
			GetAddressID(), LINEAGENTSTATUS_CAPSCHANGE);

}// CTSPIAddressInfo::OnAgentCapabiltiesChanged

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnAgentStatusChanged
//
// The status for this agent have changed, notify TAPI
//
void CTSPIAddressInfo::OnAgentStatusChanged(DWORD dwState, DWORD dwParam)
{
	GetLineOwner()->Send_TAPI_Event(NULL, LINE_AGENTSTATUS, 
			GetAddressID(), dwState, dwParam);

}// CTSPIAddressInfo::OnAgentCapabiltiesChanged

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentState
//
// Set the current agent state for this address object.
//
void CTSPIAddressInfo::SetAgentState(DWORD dwState, DWORD dwNextState)
{
	DWORD dwMsg = 0;
	TAgentStatus* pAS = GetAgentStatus();

	if (pAS->dwState != dwState)
	{
		pAS->dwState = dwState;
		dwMsg |= LINEAGENTSTATUS_STATE;
	}
	if (dwNextState != 0 &&	pAS->dwNextState != dwNextState)
	{
		pAS->dwNextState = dwNextState;
		dwMsg |= LINEAGENTSTATUS_NEXTSTATE;
	}

	// Now based on the state of the agent, determine the agent features.
	DWORD dwFeatures = GetAgentCaps()->dwFeatures;
	if (dwState == LINEAGENTSTATE_LOGGEDOFF)
	{
		// If we support lineSetAgentGroup, then force that to be the method
		// used for login - this is the standard usage by most TAPI applications
		// so it should be the way the service provider functions.
		if (GetAgentCaps()->dwFeatures & LINEAGENTFEATURE_SETAGENTGROUP)
			dwFeatures &= ~(LINEAGENTFEATURE_SETAGENTACTIVITY |
							LINEAGENTFEATURE_SETAGENTSTATE);

		// Otherwise if we don't support lineSetAgentGroup, the allow the
		// lineSetAgentState function to be used for logon or setting the
		// initial state.
		else
			dwFeatures &= ~LINEAGENTFEATURE_SETAGENTACTIVITY;
	}

	// Otherwise, if we are logged on but don't have any groups loaded into our
	// group list then don't allow lineSetAgentGroup.
	else if (dwState != LINEAGENTSTATE_UNKNOWN)
	{
		if (GetLineOwner()->GetDeviceInfo()->GetAgentGroupCount() == 0)
			dwFeatures &= ~LINEAGENTFEATURE_SETAGENTGROUP;
	}

	SetAgentFeatures(dwFeatures);

	// Send a notification concerning the agent state changing.
	if (dwMsg > 0)
		OnAgentStatusChanged(dwMsg, (dwMsg & LINEAGENTSTATUS_STATE) ? dwState : 0);

	// Force the address features to change.
	RecalcAddrFeatures();

	// Force the line features to be reflected correctly based on
	// the current agent state.  In many ACD systems, the agent state
	// will cause the call/address/line features to change.
	GetLineOwner()->RecalcLineFeatures();

}// CTSPIAddressInfo::SetAgentState

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentFeatures
//
// Sets the currently available agent features.
//
void CTSPIAddressInfo::SetAgentFeatures(DWORD dwFeatures)
{
	dwFeatures &= GetAgentCaps()->dwFeatures;
	if (dwFeatures != GetAgentStatus()->dwAgentFeatures)
		GetAgentStatus()->dwAgentFeatures = dwFeatures;

}// CTSPIAddressInfo::SetAgentFeatures

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentActivity
//
// Set the current agent activity for this address object.
//
void CTSPIAddressInfo::SetAgentActivity(DWORD dwActivity)
{
	if (GetAgentStatus()->dwActivityID != dwActivity)
	{
		GetAgentStatus()->dwActivityID = dwActivity;
		OnAgentStatusChanged(LINEAGENTSTATUS_ACTIVITY);
	}

}// CTSPIAddressInfo::SetAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentGroup
//
// Sets the current agent groups this address is currently logged into.
//
void CTSPIAddressInfo::SetAgentGroup(TAgentGroupArray* parrGroups)
{
	// Remove any existing agent information
	RemoveAllAgentGroups();
	if (parrGroups != NULL)
	{
		CEnterCode sLock(this);	// Unlocks at exit of if()
		// Add the current set of groups.
		for (TAgentGroupArray::const_iterator iPos = parrGroups->begin(); 
			iPos != parrGroups->end(); iPos++)
			m_AgentStatus.arrGroups.push_back((*iPos));
		parrGroups->clear();
	}

	// Notify TAPI that the groups have changed.
	OnAgentStatusChanged(LINEAGENTSTATUS_GROUP);

}// CTSPIAddressInfo::SetAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetValidAgentStates
//
// This changes the set of valid states within the STATUS structure.
//
void CTSPIAddressInfo::SetValidAgentStates(DWORD dwStates)
{
	if (GetAgentStatus()->dwValidStates != dwStates)
	{
		GetAgentStatus()->dwValidStates = dwStates;
		OnAgentStatusChanged(LINEAGENTSTATUS_VALIDSTATES);
	}

}// CTSPIAddressInfo::SetValidAgentStates

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetValidNextAgentStates
//
// This changes the set of valid states within the STATUS structure.
//
void CTSPIAddressInfo::SetValidNextAgentStates(DWORD dwStates)
{
	if (GetAgentStatus()->dwValidNextStates != dwStates)
	{
		GetAgentStatus()->dwValidNextStates = dwStates;
		OnAgentStatusChanged(LINEAGENTSTATUS_VALIDNEXTSTATES);
	}

}// CTSPIAddressInfo::SetValidNextAgentStates

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentGroup
//
// This function manages the lineSetAgentGroup API which sets the agent
// groups into which the agent is logged into on this address.
//
LONG CTSPIAddressInfo::SetAgentGroup(DRV_REQUESTID dwRequestID, 
									 LPLINEAGENTGROUPLIST lpGroupList)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_SETAGENTGROUP) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// If there are groups, move them into our array.
	TAgentGroupArray* parrGroups = NULL;
	if (lpGroupList && lpGroupList->dwNumEntries > 0)
	{
		// If the count is bigger than we expect, give an error.
		if (lpGroupList->dwNumEntries > GetAgentCaps()->dwMaxNumGroupEntries)
			return LINEERR_INVALAGENTGROUP;

		// Otherwise move each of the groups into our array.  We don't
		// validate the groups here - the derived provider may perform 
		// group validation using the CTSPIDevice::DoesAgentGroupExist
		// function if desired.  It is not done here so that the GROUPENTRY
		// field can specify agent id or password.
		parrGroups = new TAgentGroupArray;
		DWORD dwTotal = lpGroupList->dwNumEntries;
		LPLINEAGENTGROUPENTRY lpge = reinterpret_cast<LPLINEAGENTGROUPENTRY>(reinterpret_cast<LPBYTE>(lpGroupList) + lpGroupList->dwListOffset);
		while (dwTotal-- > 0)
		{
			TAgentGroup* pGroup = new TAgentGroup;
			pGroup->GroupID.dwGroupID1 = lpge->GroupID.dwGroupID1;
			pGroup->GroupID.dwGroupID2 = lpge->GroupID.dwGroupID2;
			pGroup->GroupID.dwGroupID3 = lpge->GroupID.dwGroupID3;
			pGroup->GroupID.dwGroupID4 = lpge->GroupID.dwGroupID4;
			if (lpge->dwNameOffset > 0 && lpge->dwNameSize > 0)
			{
				// Special note: TAPISRV ignores the name entry in the group fields
				// and doesn't seem to do any translation of the information. This
				// means that if the service provider is UNICODE and the application
				// was MultiByte, then the string passed to us is MultiByte even though
				// we are really UNICODE.  Check for an ANSI string here and do the
				// appropriate conversions.
				if (lstrlenA(reinterpret_cast<char*>(reinterpret_cast<LPBYTE>(lpGroupList)+lpge->dwNameOffset)) == (int)(lpge->dwNameSize-1))
				{
					// Found an ANSI string - convert it to UNICODE
					USES_CONVERSION;
					pGroup->strName = A2T(reinterpret_cast<char*>(reinterpret_cast<LPBYTE>(lpGroupList)+lpge->dwNameOffset));
				}

				// It is UNICODE. Convert it to MultiByte if necessary
				else
					pGroup->strName = ConvertWideToAnsi(reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpGroupList)+lpge->dwNameOffset));
			}

			try
			{
				parrGroups->push_back(pGroup);
			}
			catch (...)
			{
				delete parrGroups;
				return LINEERR_NOMEM;
			}
			lpge++;
		}
	}

	// Now add the request to the provider list.
    if (!GetLineOwner()->AddAsynchRequest(new RTSetAgentGroup(this, dwRequestID, parrGroups)))
		return LINEERR_OPERATIONFAILED;
	return static_cast<LONG>(dwRequestID);

}// CTSPIAddressInfo::SetAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentState
//
// This function manages the lineSetAgentState API which sets the current
// agent state associated with a particular address.
//
LONG CTSPIAddressInfo::SetAgentState(DRV_REQUESTID dwRequestID, 
									 DWORD dwState, DWORD dwNextState)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_SETAGENTSTATE) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// If the state or next state is not allowed right now, fail the request.
	if ((dwState != 0 &&
		(((dwState) & (dwState-1)) ||
		 (dwState & GetAgentCaps()->dwStates) == 0)) ||
	    (dwNextState != 0 &&
		 (((dwNextState) & (dwNextState-1)) ||
		 (dwNextState & GetAgentCaps()->dwNextStates) == 0)))
		 return LINEERR_INVALAGENTSTATE;

	// Now add the request to the provider list.
    if (!GetLineOwner()->AddAsynchRequest(new RTSetAgentState(this, dwRequestID, dwState, dwNextState)))
		return LINEERR_OPERATIONFAILED;
	return static_cast<LONG>(dwRequestID);

}// CTSPIAddressInfo::SetAgentState

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAgentActivity
//
// This function manages the lineSetAgentActivity API which sets the
// agent activity code associated with this address.
//
LONG CTSPIAddressInfo::SetAgentActivity(DRV_REQUESTID dwRequestID, DWORD dwActivity)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_SETAGENTACTIVITY) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Verify the activity code.
	if (!GetLineOwner()->GetDeviceInfo()->DoesAgentActivityExist(dwActivity))
		return LINEERR_INVALAGENTACTIVITY;

	// Now add the request to the provider list.
    if (!GetLineOwner()->AddAsynchRequest(new RTSetAgentActivity(this, dwRequestID, dwActivity)))
		return LINEERR_OPERATIONFAILED;
	return static_cast<LONG>(dwRequestID);

}// CTSPIAddressInfo::SetAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AgentSpecific
//
// This function manages the lineAgentSpecific which allows the application
// to access proprietary handler-specific functions associated with this 
// address.
//
LONG CTSPIAddressInfo::AgentSpecific(DRV_REQUESTID dwRequestID, 
				DWORD dwAgentExtensionIDIndex, LPVOID lpvBuff, DWORD dwSize)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_AGENTSPECIFIC) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Verify that the extension ID exists.
	if (dwAgentExtensionIDIndex > 
		GetLineOwner()->GetDeviceInfo()->GetAgentSpecificExtensionCount())
		return LINEERR_INVALPARAM;

	// Build a request object.
	RTAgentSpecific* pRequest = new RTAgentSpecific(this, dwRequestID, 
			dwAgentExtensionIDIndex, lpvBuff, dwSize);

	// Now add the request to the provider list.
    if (!GetLineOwner()->AddAsynchRequest(pRequest))
		return LINEERR_OPERATIONFAILED;
	return static_cast<LONG>(dwRequestID);

}// CTSPIAddressInfo::AgentSpecific

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GatherAgentCapabilities
//
// This function manages the lineGetAgentCaps function which allows an
// application to query information associated with agent capabilities on
// this address.
//
LONG CTSPIAddressInfo::GatherAgentCapabilities(LPLINEAGENTCAPS lpAgentCaps)
{
	// Fill in the basics from our Agent capabilities structure
	lpAgentCaps->dwCapsVersion = GetLineOwner()->GetNegotiatedVersion();
	lpAgentCaps->dwFeatures = GetAgentCaps()->dwFeatures;
	lpAgentCaps->dwStates = GetAgentCaps()->dwStates;
	lpAgentCaps->dwNextStates = GetAgentCaps()->dwNextStates;
	lpAgentCaps->dwMaxNumGroupEntries = GetAgentCaps()->dwMaxNumGroupEntries;
	lpAgentCaps->dwAgentStatusMessages = GetAgentCaps()->dwAgentStatusMessages;
	lpAgentCaps->dwNeededSize = sizeof(LINEAGENTCAPS);

	// Remove the GUID at the end if necessary.
	if (lpAgentCaps->dwCapsVersion < TAPIVER_22)
		lpAgentCaps->dwNeededSize -= sizeof(GUID);

	// Save off the used size.
	lpAgentCaps->dwUsedSize = lpAgentCaps->dwNeededSize;

	// Add the handler name.  If there isn't enough room, the "dwNeededSize" will
	// still be incremented properly.
	TString strProviderInfo = GetSP()->GetProviderInfo();
	if (!strProviderInfo.empty())
	{
		AddDataBlock(lpAgentCaps, lpAgentCaps->dwAgentHandlerInfoOffset, 
					  lpAgentCaps->dwAgentHandlerInfoSize,
					  strProviderInfo.c_str());
	}

	// Now add each of the extension ids to the list.
	lpAgentCaps->dwNumAgentExtensionIDs = GetLineOwner()->GetDeviceInfo()->GetAgentSpecificExtensionCount();
	lpAgentCaps->dwAgentExtensionIDListSize = 0;
	lpAgentCaps->dwAgentExtensionIDListOffset = 0;

	for (unsigned int i = 0; i < GetLineOwner()->GetDeviceInfo()->GetAgentSpecificExtensionCount(); i++)
	{
		const TAgentSpecificEntry* pEntry = GetLineOwner()->GetDeviceInfo()->GetAgentSpecificExtensionID(i);
		AddDataBlock (lpAgentCaps, lpAgentCaps->dwAgentExtensionIDListOffset,
				lpAgentCaps->dwAgentExtensionIDListSize, 
				static_cast<LPCVOID>(pEntry), sizeof(TAgentSpecificEntry));
	}
	return 0L;

}// CTSPIAddressInfo::GatherAgentCapabilities

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GatherAgentStatus
//
// This function manages the lineGetAgentStatus function which allows
// an application to query information associated with the current
// agent status on this address.
//
LONG CTSPIAddressInfo::GatherAgentStatus(LPLINEAGENTSTATUS lpAgentStatus)
{
	// Set the known members from our own status structure
	lpAgentStatus->dwState = GetAgentStatus()->dwState;
	lpAgentStatus->dwNextState = GetAgentStatus()->dwNextState;
	lpAgentStatus->dwActivityID = GetAgentStatus()->dwActivityID;
	lpAgentStatus->dwAgentFeatures = GetAgentStatus()->dwAgentFeatures;
	lpAgentStatus->dwValidStates = GetAgentStatus()->dwValidStates;
	lpAgentStatus->dwValidNextStates = GetAgentStatus()->dwValidNextStates;
	lpAgentStatus->dwUsedSize = sizeof(LINEAGENTSTATUS);
	lpAgentStatus->dwNeededSize = sizeof(LINEAGENTSTATUS);

	// Add in the current activity ID text string.
	TString strActivity = GetLineOwner()->GetDeviceInfo()->GetAgentActivityById(lpAgentStatus->dwActivityID);
	if (!strActivity.empty())
	{
		AddDataBlock (lpAgentStatus, lpAgentStatus->dwActivityOffset,
					  lpAgentStatus->dwActivitySize, strActivity.c_str());
	}

	// Now add the currently logged in "groups".
	lpAgentStatus->dwNumEntries = GetAgentStatus()->arrGroups.size();
	lpAgentStatus->dwNeededSize += lpAgentStatus->dwNumEntries*sizeof(LINEAGENTGROUPENTRY);
	lpAgentStatus->dwGroupListSize = 0;
	lpAgentStatus->dwGroupListOffset = 0;

	// Add any existing group entries.  If there isn't enough room, we will simply
	// count the required space for the names.
	if (lpAgentStatus->dwNumEntries > 0)
	{
		LPBYTE lpBuff = NULL;
		if (lpAgentStatus->dwUsedSize < lpAgentStatus->dwNeededSize &&
			lpAgentStatus->dwTotalSize > lpAgentStatus->dwNeededSize)
		{
			lpAgentStatus->dwGroupListOffset = lpAgentStatus->dwUsedSize;
			lpAgentStatus->dwUsedSize += lpAgentStatus->dwNumEntries*sizeof(LINEAGENTGROUPENTRY);
			lpAgentStatus->dwGroupListSize += lpAgentStatus->dwNumEntries*sizeof(LINEAGENTGROUPENTRY);
			lpBuff = (reinterpret_cast<LPBYTE>(lpAgentStatus)+lpAgentStatus->dwGroupListOffset);
		}

		// Walk through all our GROUPENTRY structures and add each one to the 
		// list if we have enough space.
		for (TAgentGroupArray::iterator iPos = GetAgentStatus()->arrGroups.begin();
			 iPos != GetAgentStatus()->arrGroups.end(); iPos++)
		{
			LINEAGENTGROUPENTRY ge;
			TAgentGroup* pGroup = (*iPos);

			ge.GroupID.dwGroupID1 = pGroup->GroupID.dwGroupID1;
			ge.GroupID.dwGroupID2 = pGroup->GroupID.dwGroupID2;
			ge.GroupID.dwGroupID3 = pGroup->GroupID.dwGroupID3;
			ge.GroupID.dwGroupID4 = pGroup->GroupID.dwGroupID4;
			ge.dwNameSize = ge.dwNameOffset = 0;

			// Add the name of the group to the END of the structure.
			if (pGroup->strName.length() > 0)
			{
				if (AddDataBlock(lpAgentStatus, ge.dwNameOffset, 
						ge.dwNameSize, pGroup->strName.c_str()))
					lpAgentStatus->dwGroupListSize += ge.dwNameSize;
			}
								
			// Copy the structure into place.
			if (lpBuff != NULL)
			{
				MoveMemory(lpBuff, &ge, sizeof(LINEAGENTGROUPENTRY));
				lpBuff += sizeof(LINEAGENTGROUPENTRY);
			}
		}
	}

	return 0L;

}// CTSPIAddressInfo::GatherAgentStatus

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentActivityList
//
// This function manages the lineGetAgentActivityList which allows the
// application to retrieve a list of activities available from this
// provider.
//
LONG CTSPIAddressInfo::GetAgentActivityList(LPLINEAGENTACTIVITYLIST lpActivityList)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_GETAGENTACTIVITYLIST) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Set the known fields.
	lpActivityList->dwNumEntries = GetLineOwner()->GetDeviceInfo()->GetAgentActivityCount();
	lpActivityList->dwUsedSize = sizeof(LINEAGENTACTIVITYLIST);
	lpActivityList->dwNeededSize = sizeof(LINEAGENTACTIVITYLIST);
	lpActivityList->dwListSize = 0;

	if (lpActivityList->dwNumEntries == 0)
		lpActivityList->dwListOffset = 0;
	else
	{
		lpActivityList->dwListOffset = lpActivityList->dwUsedSize;
		lpActivityList->dwNeededSize += (lpActivityList->dwNumEntries * sizeof(LINEAGENTACTIVITYENTRY));

		// If we have enough room for all the entries, then add it to the USED size.
		LPBYTE lpBuff = NULL;
		if (lpActivityList->dwNeededSize <= lpActivityList->dwTotalSize)
		{
			lpActivityList->dwUsedSize += (lpActivityList->dwNumEntries * sizeof(LINEAGENTACTIVITYENTRY));
			lpActivityList->dwListSize += (lpActivityList->dwNumEntries * sizeof(LINEAGENTACTIVITYENTRY));
			lpBuff = (reinterpret_cast<LPBYTE>(lpActivityList)+lpActivityList->dwListOffset);
		}

		// Go through our entries and add each one.
		for (unsigned int i = 0; i < lpActivityList->dwNumEntries; i++)
		{
			const TAgentActivity* pActivity = GetLineOwner()->GetDeviceInfo()->GetAgentActivity(i);
			LINEAGENTACTIVITYENTRY ae;
			ae.dwID = pActivity->dwID;
			ae.dwNameOffset = ae.dwNameSize = 0;

			// Add the name if available.
			if (pActivity->strName.length() > 0)
			{
				AddDataBlock(lpActivityList, ae.dwNameOffset, ae.dwNameSize, pActivity->strName.c_str());
				lpActivityList->dwListSize += ae.dwNameSize;
			}

			// Copy the structure into place if we had enough room to do so.
			if (lpBuff != NULL)
			{
				MoveMemory (lpBuff, &ae, sizeof(LINEAGENTACTIVITYENTRY));
				lpBuff += sizeof(LINEAGENTACTIVITYENTRY);
			}
		}
	}

	return 0L;

}// CTSPIAddressInfo::GetAgentActivityList

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetAgentGroupList
//
// This function manages the lineGetAgentGroupList which allows the
// application to retrieve information concerning groups that agents
// may log into.
//
LONG CTSPIAddressInfo::GetAgentGroupList(LPLINEAGENTGROUPLIST lpGroupList)
{
	// If this feature isn't currently valid then exit.
	if ((GetAgentStatus()->dwAgentFeatures & LINEAGENTFEATURE_GETAGENTGROUPLIST) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Set the known fields.
	lpGroupList->dwNumEntries = GetLineOwner()->GetDeviceInfo()->GetAgentGroupCount();
	lpGroupList->dwUsedSize = sizeof(LINEAGENTGROUPLIST);
	lpGroupList->dwNeededSize = sizeof(LINEAGENTGROUPLIST);
	lpGroupList->dwListSize = 0;

	if (lpGroupList->dwNumEntries == 0)
		lpGroupList->dwListOffset = 0;
	else
	{
		lpGroupList->dwListOffset = lpGroupList->dwUsedSize;
		lpGroupList->dwNeededSize += (lpGroupList->dwNumEntries * sizeof(LINEAGENTGROUPENTRY));

		// If we have enough room for all the entries, then add it to the USED size.
		LPBYTE lpBuff = NULL;
		if (lpGroupList->dwNeededSize <= lpGroupList->dwTotalSize)
		{
			lpGroupList->dwUsedSize += (lpGroupList->dwNumEntries * sizeof(LINEAGENTGROUPENTRY));
			lpGroupList->dwListSize += (lpGroupList->dwNumEntries * sizeof(LINEAGENTGROUPENTRY));
			lpBuff = (reinterpret_cast<LPBYTE>(lpGroupList)+lpGroupList->dwListOffset);
		}

		// Go through our entries and add each one if we can.
		for (unsigned int i = 0; i < lpGroupList->dwNumEntries; i++)
		{
			const TAgentGroup* pGroup = GetLineOwner()->GetDeviceInfo()->GetAgentGroup(i);
			LINEAGENTGROUPENTRY ge;

			ge.GroupID.dwGroupID1 = pGroup->GroupID.dwGroupID1;
			ge.GroupID.dwGroupID2 = pGroup->GroupID.dwGroupID2;
			ge.GroupID.dwGroupID3 = pGroup->GroupID.dwGroupID3;
			ge.GroupID.dwGroupID4 = pGroup->GroupID.dwGroupID4;
			ge.dwNameSize = ge.dwNameOffset = 0;

			// Add the name if available.
			if (pGroup->strName.length() > 0)
			{
				AddDataBlock(lpGroupList, ge.dwNameOffset, ge.dwNameSize, pGroup->strName.c_str());
				lpGroupList->dwListSize += ge.dwNameSize;
			}

			// Copy the structure into place.
			if (lpBuff != NULL)
			{
				MoveMemory (lpBuff, &ge, sizeof(LINEAGENTGROUPENTRY));
				lpBuff += sizeof(LINEAGENTGROUPENTRY);
			}
		}
	}

	return 0L;

}// CTSPIAddressInfo::GetAgentGroupList

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::RemoveAllAgentGroups
//
// Remove us from all agent groups
//
void CTSPIAddressInfo::RemoveAllAgentGroups()
{
	CEnterCode sLock(this);

	// Erase the previous groups.
	std::for_each(m_AgentStatus.arrGroups.begin(),
		m_AgentStatus.arrGroups.end(), tsplib::delete_object());
	m_AgentStatus.arrGroups.clear();

}// CTSPIAddressInfo::RemoveAllAgentGroups

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddAgentGroup
//
// Add a single group to the current array
//
void CTSPIAddressInfo::AddAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4, LPCTSTR pszGroupName)
{
	TAgentGroup* pNewGroup = new TAgentGroup;
	const TAgentGroup* pGroup = NULL;

	// Locate the agent group in the device array.
	CTSPIDevice* pDevice = GetLineOwner()->GetDeviceInfo();
	for (unsigned int i = 0; i < pDevice->GetAgentGroupCount(); i++)
	{
		const TAgentGroup* pTestGroup = pDevice->GetAgentGroup(i);
		if (pTestGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pTestGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pTestGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pTestGroup->GroupID.dwGroupID4 == dwGroupID4)
		{
			pGroup = pTestGroup;
			break;
		}
	}

	// If we didn't find it add it anyway but spit out a developer warning.
	if (pGroup == NULL)
	{
#ifdef _DEBUG
		// Only give warning on non-zero agent groups since many TSPs will use this
		// for special purposes - using only the name.
		if ((dwGroupID1 + dwGroupID2 + dwGroupID3 + dwGroupID4) > 0)
			_TSP_DTRACEX(TRC_WARNINGS, _T("Warning: AddCurrentAgentGroup could not locate groupid (0x%lx 0x%lx 0x%lx 0x%lx) ")\
									_T("you may be missing some agent groups which are defined on the ACD.\n"),
									dwGroupID1, dwGroupID2, dwGroupID3, dwGroupID4);
#endif
		pNewGroup->GroupID.dwGroupID1 = dwGroupID1;
		pNewGroup->GroupID.dwGroupID2 = dwGroupID2;
		pNewGroup->GroupID.dwGroupID3 = dwGroupID3;
		pNewGroup->GroupID.dwGroupID4 = dwGroupID4;
		pNewGroup->strName = (pszGroupName != NULL) ? pszGroupName : _T("");
	}
	else
	{
		CopyMemory(&pNewGroup->GroupID, &pGroup->GroupID, sizeof(pNewGroup->GroupID));
		pNewGroup->strName = pGroup->strName;
	}

	// Now add it to the address list
	CEnterCode sLock(this);
	try
	{
		m_AgentStatus.arrGroups.push_back(pNewGroup);
	}
	catch (...)
	{
		delete pNewGroup;
		throw;
	}

	sLock.Unlock();

}// CTSPIAddressInfo::AddCurrentAgentGroup
