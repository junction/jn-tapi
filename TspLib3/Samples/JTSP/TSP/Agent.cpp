/***************************************************************************
//
// AGENT.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Agent management (from proxy) and event handling
//
// Copyright (C) 1998 JulMar Entertainment Technology, Inc.
// All rights reserved
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// Modification History
// ------------------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Generated from TSPWizard.exe
// 
/***************************************************************************/

/*----------------------------------------------------------------------------
	INCLUDE FILES
-----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "jtsp.h"

/*----------------------------------------------------------------------------
	DEBUG SUPPORT
-----------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*----------------------------------------------------------------------------
	GLOBAL VARIABLES
-----------------------------------------------------------------------------*/
// Map between TAPI agent states and JPBX simulator
static DWORD g_dwAgentStates[] = {
	LINEAGENTSTATE_UNKNOWN,			// 0x0000 Unknown
	LINEAGENTSTATE_LOGGEDOFF,		// 0x0001 Signed-out
	LINEAGENTSTATE_READY,			// 0x0002 Ready
	LINEAGENTSTATE_NOTREADY,		// 0x0003 Not Ready
	LINEAGENTSTATE_WORKINGAFTERCALL,// 0x0004 In Call Work
	LINEAGENTSTATE_BUSYACD,			// 0x0005 Handling ACD call
	LINEAGENTSTATE_BUSYOTHER,		// 0x0006 Handling outbound call
};

// Guess map for next state based on current state
static DWORD g_dwNextAgentStates[] = {
	LINEAGENTSTATE_UNKNOWN,			// 0x0000 Unknown
	LINEAGENTSTATE_LOGGEDOFF,		// 0x0001 Signed-out
	LINEAGENTSTATE_BUSYACD,			// 0x0002 Ready
	LINEAGENTSTATE_READY,			// 0x0003 Not Ready
	LINEAGENTSTATE_READY,			// 0x0004 In Call Work
	LINEAGENTSTATE_NOTREADY,		// 0x0005 Handling ACD call
	LINEAGENTSTATE_NOTREADY,		// 0x0006 Handling outbound call
};

/*****************************************************************************
** Procedure:  CJTLine::OnAgentStateChange
**
** Arguments: 'iState' - New agent state (CPEAgentState::enum)
**
** Returns:    void
**
** Description:  This function changes our agent state to reflect the
**               current reported state of the station
**
*****************************************************************************/
void CJTLine::OnAgentStateChange(int iState)
{
	CTSPIAddressInfo* pAddr = GetAddress(0);
	DWORD dwState, dwNextState;

	if (iState == -1)
	{
		dwState = pAddr->GetAgentStatus()->dwState;
		dwNextState = pAddr->GetAgentStatus()->dwNextState;
	}
	else
	{
		// Determine the TAPI state of the agent based on our two maps
		dwState = g_dwAgentStates[iState];
		dwNextState = g_dwNextAgentStates[iState];
	}

	// Set the new agent state into place.
	pAddr->SetAgentState(dwState, dwNextState);
	if (dwState == LINEAGENTSTATE_LOGGEDOFF)
	{
		// Mark the line as out-of-service since we cannot use it when 
		// there is not an agent logged on.
		DevStatusInService(false);
		
		// Remove agent states and group information.
		pAddr->SetValidAgentStates(0);
		pAddr->SetValidNextAgentStates(0);
		pAddr->SetAgentGroup(NULL);
	}
	else
	{
		// Mark the line as in-service since we now have an agent logged on
		DevStatusInService(true);

		// Remove the current state we are in NOW
		dwState = (LINEAGENTSTATE_NOTREADY |
				   LINEAGENTSTATE_READY | 
				   LINEAGENTSTATE_WORKINGAFTERCALL | 
				   LINEAGENTSTATE_LOGGEDOFF) & ~dwState;
		dwNextState = (LINEAGENTSTATE_NOTREADY |
				   LINEAGENTSTATE_READY | 
				   LINEAGENTSTATE_WORKINGAFTERCALL | 
				   LINEAGENTSTATE_LOGGEDOFF) & ~dwNextState;

		// If we have active calls, don't allow state to change.
		if (pAddr->GetAddressStatus()->dwNumActiveCalls > 0)
			dwState = 0;

		// If we are READY, don't allow logout.
		if (pAddr->GetAgentStatus()->dwState != LINEAGENTSTATE_NOTREADY)
			dwState &= ~LINEAGENTSTATE_LOGGEDOFF;

		// Mark the valid CURRENT/NEXT states.
		pAddr->SetValidAgentStates(dwState);
		pAddr->SetValidNextAgentStates(dwNextState);
	}

}// CJTLine::OnAgentStateChange

/*****************************************************************************
** Procedure:  CJTLine::OnAgentGroupChange
**
** Arguments: 'dwPrimary' - Primary agent group
**            'dwSecondary' - Secondary agent group
**
** Returns:    void
**
** Description:  This function changes our agent groups to reflect the
**               current reported state of the station
**
*****************************************************************************/
void CJTLine::OnAgentGroupChange(LPCTSTR pszAgentID, DWORD dwPrimary, DWORD dwSecondary)
{
	// Remove any existing agent information from the address
	CTSPIAddressInfo* pAddr = GetAddress(0);
	pAddr->RemoveAllAgentGroups();

	// Add the two agent groups if they exist.
	pAddr->AddAgentGroup(0, 0, 0, 0, pszAgentID);	// AgentID
	pAddr->AddAgentGroup(0, 0, 0, 0, NULL);			// Password
	pAddr->AddAgentGroup(dwPrimary);				// Primary group
	pAddr->AddAgentGroup(dwSecondary);				// Secondary group

	// Notify TAPI that the agent groups have changed. This isn't done
	// automatically by TSP++ for the AddCurrentAgentGroup() since it may
	// be called many times and we don't want to send more than one event.
	pAddr->OnAgentStatusChanged(LINEAGENTSTATUS_GROUP);

}// CJTLine::OnAgentGroupChange

/*****************************************************************************
** Procedure:  CJTLine::OnSetAgentGroup
**
** Arguments: 'pReq' - Request object representing this SetAgentGroup event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the lineSetAgentGroup processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSetAgentGroup(RTSetAgentGroup* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPIAddressInfo* pAddress = GetAddress(0);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Collect the parameters. The agent-id is in the first
		// group entry position, the password is in the second and any groups
		// the agent wants access to are in subsequent entries.
		//
		// Note this is a deviation from the TAPI specification since it has no
		// facility for providing a userid/password for logon which our switch
		// requires. This coded "standard" has been used as a workaround in many
		// commercial TSPs currently in use.
		TString strName = (pRequest->GetCount() > 0) ? pRequest->GetGroup(0)->strName : _T("");
		TString strPassword = (pRequest->GetCount() > 1) ? pRequest->GetGroup(1)->strName : _T("");

		// Gather the group entries
		TDWordArray arrGroups;
		for (unsigned int i = 2; i < pRequest->GetCount(); i++)
			arrGroups.push_back(pRequest->GetGroup(i)->GroupID.dwGroupID1);

		// If this is a logoff request then handle it seperately from the
		// other requests since it is an agent state change to the switch.
		if (pRequest->GetCount() == 0)
		{
			// Fail if there is no agent logged on at this address.
			if (pAddress->GetCurrentAgentGroupCount() == 0)
			{
				CompleteRequest(pRequest, LINEERR_INVALAGENTGROUP);
				return false;
			}
			GetDeviceInfo()->DRV_SetAgentState(this, strName, _T('S'));
		}
		else
		{
			// Validate the parameters - if there is no agent id then fail this
			// request since we cannot log on or change our agent groups without it.
			if (strName.empty())
			{
				const TAgentGroup* pGroup = pAddress->GetCurrentAgentGroup(0);
				if (pGroup != NULL)
					strName = pGroup->strName;
				else
					CompleteRequest(pRequest, LINEERR_INVALAGENTID);
			}
			// Or if too many agent groups are passed (max is 2)
			else if (arrGroups.size() > 2)
				CompleteRequest(pRequest, LINEERR_INVALPARAM);

			// Perform the logon if we haven't failed with an error yet
			if (!pRequest->HaveSentResponse())
			{
				DWORD dwGroup1 = (arrGroups.size() > 0) ? arrGroups[0] : 0;
				DWORD dwGroup2 = (arrGroups.size() > 1) ? arrGroups[1] : 0;

				if ((dwGroup1 > 0 && !GetDeviceInfo()->DoesAgentGroupExist(dwGroup1)) ||
					(dwGroup2 > 0 && !GetDeviceInfo()->DoesAgentGroupExist(dwGroup2)))
					CompleteRequest(pRequest, LINEERR_INVALAGENTGROUP);
				else
					GetDeviceInfo()->DRV_Logon(this, strName, strPassword, dwGroup1, dwGroup2);
			}
		}
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RELEASECALL, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			(peCommand->GetCommand() == CPECommand::AgentLogon ||
			peCommand->GetCommand() == CPECommand::AgentState) && pidError != NULL)
		{
			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, pidError->GetError());
			return true;
		}
	}

	// Check to see if our request has exceeded the limit for processing.  If 
	// so, tell TAPI that the request failed and delete the request.
	if (pRequest->GetState() == STATE_WAITING && 
		(pRequest->GetStateTime()+REQUEST_TIMEOUT) < GetTickCount())
		CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler.
	return false;

}// CJTLine::OnSetAgentGroup

/*****************************************************************************
** Procedure:  CJTLine::OnSetAgentState
**
** Arguments: 'pReq' - Request object representing this SetAgentState event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the lineSetAgentState processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSetAgentState(RTSetAgentState* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPIAddressInfo* pAddress = GetAddress(0);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Fail if there is not an agent logged on at this address.
		if (pAddress->GetCurrentAgentGroupCount() == 0)
		{
			CompleteRequest(pRequest, LINEERR_INVALAGENTID);
			return true;
		}

		// Determine whether we are setting the current state or the next
		// state based on the current status of the address in question. If we
		// have active calls running on the line then we cannot adjust the current
		// state - it is done by the ACD.
		DWORD dwState = (pAddress->GetAddressStatus()->dwNumActiveCalls > 0) ?
				pRequest->GetNextAgentState() : pRequest->GetAgentState();

		// Convert the state to the proper ACD status character
		TCHAR chState = _T('U');
		switch (dwState)
		{
			case LINEAGENTSTATE_READY:				chState = _T('R'); break;
			case LINEAGENTSTATE_WORKINGAFTERCALL:	chState = _T('W'); break;
			case LINEAGENTSTATE_NOTREADY:			chState = _T('N'); break;
			case LINEAGENTSTATE_LOGGEDOFF:			chState = _T('S'); break;
		}

		// If we have a valid state then send it to the switch.
		if (chState != _T('U'))
		{
			const TAgentGroup* pGroup = pAddress->GetCurrentAgentGroup(0);
			_TSP_ASSERTE(pGroup != NULL);
			GetDeviceInfo()->DRV_SetAgentState(this, pGroup->strName, chState);
		}
		else
			CompleteRequest(pRequest, LINEERR_INVALAGENTSTATE);
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RELEASECALL, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::AgentState && pidError != NULL)
		{
			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, pidError->GetError());
			return true;
		}
	}

	// Check to see if our request has exceeded the limit for processing.  If 
	// so, tell TAPI that the request failed and delete the request.
	if (pRequest->GetState() == STATE_WAITING && 
		(pRequest->GetStateTime()+REQUEST_TIMEOUT) < GetTickCount())
		CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler.
	return false;

}// CJTLine::OnSetAgentState

/*****************************************************************************
** Procedure:  CJTLine::OnSetAgentActivity
**
** Arguments: 'pReq' - Request object representing this SetAgentActivity event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the lineSetAgentActivity processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSetAgentActivity(RTSetAgentActivity* pRequest, LPCVOID /*lpBuff*/)
{
	// Validate the activity itself and either reject it or allow it
	// to be set into the agent properties. The provider manages the agent
	// activities itself since the ACD doesn't support the concept.
	TString strActivity = GetDeviceInfo()->GetAgentActivityById(pRequest->GetActivity());
	if (strActivity.empty())
		CompleteRequest(pRequest, LINEERR_INVALAGENTACTIVITY);
	else
		CompleteRequest(pRequest, 0);

	// Let the request fall through to the unsolicited handler.
	return false;

}// CJTLine::OnSetAgentActivity
