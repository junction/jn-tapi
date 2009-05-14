/***************************************************************************
//
// CONFERENCE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineSetupConfernce/TSPI_lineCompleteConference processing
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

/*****************************************************************************
** Procedure:  CJTLine::OnSetupConference
**
** Arguments: 'pReq' - Request object representing this SetupConf event
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineSetupConference processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSetupConference(RTSetupConference* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPIConferenceCall* pConfCall = pRequest->GetConferenceCall();
	CTSPICallAppearance* pCall = pRequest->GetOriginalCall();
	CTSPICallAppearance* pConsult = pRequest->GetConsultationCall();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// If the call we are going to transfer is not currently on hold then
		// place it on hold so it may be transferred by the switch
		if ((pCall->GetCallState() & 
				(LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | LINECALLSTATE_ONHOLDPENDCONF)) == 0)
			GetDeviceInfo()->DRV_HoldCall(this, pCall);

		// Otherwise complete the request and set the consultation call into the 
		// DIALTONE state (as a non-bandwidth call on the switch)
		else
		{
			// Transition the existing call to the CONFERENCED state
			// and the conference call into the onHold pending conference state.
			pConfCall->SetCallState(LINECALLSTATE_ONHOLDPENDCONF);
			pCall->SetCallState(LINECALLSTATE_CONFERENCED);

			// Mark the call as a "fake" call generated for TAPI to
			// dial upon but not taking up bandwidth on the switch yet.
			pConsult->MarkReal(false);
			pConsult->SetCallState(LINECALLSTATE_DIALTONE, 
					LINEDIALTONEMODE_INTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);
			
			// Complete the request - the application should not use the lineDial()
			// TAPI command to dial on this consultation call and then call 
			// lineCompleteTransfer() to finish this transfer.
			CompleteRequest(pRequest, 0);
		}
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our MAKECALL, then manage it.
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::HoldCall)
		{
			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, pidError->GetError());
			return true;
		}

		// If we received a callstate message for our active call and it is
		// transitioning to the onHold state then make it CONFERENCED
		// Return TRUE to skip the unsolicited handler.
		else if (pBlock->GetEventType() == CEventBlock::CallStateChange)
		{
			const CPECallState* pCS = dynamic_cast<const CPECallState*>(pBlock->GetElement(CPBXElement::CallState));
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
			if (pidCall != NULL && pidCall->GetCallID() == pCall->GetCallID() &&
				pCS->GetCallState() == CPECallState::Holding)
			{
				// Transition the existing call to the CONFERENCED state
				// and the conference call into the onHold pending conference state.
				pConfCall->SetCallState(LINECALLSTATE_ONHOLDPENDCONF);
				pCall->SetCallState(LINECALLSTATE_CONFERENCED);
			
				// Mark the call as a "fake" call generated for TAPI to
				// dial upon but not taking up bandwidth on the switch yet.
				pConsult->MarkReal(false);
				pConsult->SetCallState(LINECALLSTATE_DIALTONE, 
						LINEDIALTONEMODE_INTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);
				return true;
			}
		}
	}

	// Check to see if our request has exceeded the limit for processing.  If 
	// so, tell TAPI that the request failed and delete the request.
	if (pRequest->GetState() == STATE_WAITING &&
		(pRequest->GetStateTime()+REQUEST_TIMEOUT) < GetTickCount())
		CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);

	return false;

}// CJTLine::OnSetupConf

/*****************************************************************************
** Procedure:  CJTLine::OnPrepareAddToConference
**
** Arguments: 'pReq' - Request object representing this PrepareConf event
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_linePrepareAddToConference processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnPrepareAddToConference(RTPrepareAddToConference* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPIConferenceCall* pConfCall = pRequest->GetConferenceCall();
	CTSPICallAppearance* pConsult = pRequest->GetConsultationCall();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Set the last call in our conference table to be the marker
		// call. We use the request itemdata for this. We will watch for 
		// this call to change to the onHold state in the below handler
		// and then complete the request when that happens.
		pRequest->SetItemDataPtr(pConfCall->GetConferenceCall(pConfCall->GetConferenceCount()-1));

		// Send a HOLD command for all parties in the conference. This will
		// officially break the conference up so don't let the conference call
		// get destroyed by the unsolicited event handler.
		for (unsigned int i = 0; i < pConfCall->GetConferenceCount(); i++)
		{
			CTSPICallAppearance* pCall = pConfCall->GetConferenceCall(i);
			if (pCall != pConsult)
				GetDeviceInfo()->DRV_HoldCall(this, pCall);
		}
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our MAKECALL, then manage it.
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::HoldCall)
		{
			// Complete the request with the appropriate error code.
			if (pidError->GetError() != 0)
				TranslateErrorCode(pRequest, pidError->GetError());
			return true;
		}

		// If we received a callstate message for one of our conference calls
		// then set the new callstate to CONFERENCED rather than onHold.
		// Return TRUE to skip the unsolicited handler.
		else if (pBlock->GetEventType() == CEventBlock::CallStateChange)
		{
			const CPECallState* pCS = dynamic_cast<const CPECallState*>(pBlock->GetElement(CPBXElement::CallState));
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));

			// If this is our "marker" call (see above comment) then finish this
			// request and transition the whole conference to onhold pending.
			CTSPICallAppearance* pCall = reinterpret_cast<CTSPICallAppearance*>(pRequest->GetItemDataPtr());
			if (pidCall && pCall && 
				pCall->GetCallID() == pidCall->GetCallID() &&
				pCS->GetCallState() == CPECallState::Holding)
			{
				// Transition the existing call to the CONFERENCED state
				// and the conference call into the onHold pending conference state.
				pConfCall->SetCallState(LINECALLSTATE_ONHOLDPENDCONF);

				// Mark the call as a "fake" call generated for TAPI to
				// dial upon but not taking up bandwidth on the switch yet.
				pConsult->MarkReal(false);
				pConsult->SetCallState(LINECALLSTATE_DIALTONE, 
						LINEDIALTONEMODE_INTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);
					
				// Complete the request - the application should not use the lineDial()
				// TAPI command to dial on this consultation call and then call 
				// lineCompleteTransfer() to finish this transfer.
				CompleteRequest(pRequest, 0);
			}

			// Check to see if this call is in the conference array. If so, ignore
			// the callstate change.
			for (unsigned int i = 0; i < pConfCall->GetConferenceCount(); i++)
			{
				CTSPICallAppearance* pCall = pConfCall->GetConferenceCall(i);
				if (pCall && pidCall && pCall->GetCallID() == pidCall->GetCallID() &&
					pCS->GetCallState() == CPECallState::Holding)
					return true;
			}
		}
	}

	// Check to see if our request has exceeded the limit for processing.  If 
	// so, tell TAPI that the request failed and delete the request.
	if (pRequest->GetState() == STATE_WAITING &&
		(pRequest->GetStateTime()+REQUEST_TIMEOUT) < GetTickCount())
		CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);

	return false;

}// CJTLine::OnSetupConf

/*****************************************************************************
** Procedure:  CJTLine::OnAddToConference
**
** Arguments: 'pReq' - Request object representing this AddToConference event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineAddToConference processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnAddToConference(RTAddToConference* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPIConferenceCall* pConfCall = pRequest->GetConferenceCall();
	CTSPICallAppearance* pConsult = pRequest->GetConsultationCall();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Retrieve the holding calls which are in our conference array. Each
		// call in the conference array should be currently onHOLD in the PBX.
		// Send a retrieve command for each one.
		for (unsigned int i = 0; i < pConfCall->GetConferenceCount(); i++)
		{
			CTSPICallAppearance* pCall = pConfCall->GetConferenceCall(i);
			if (pCall != pConsult)
				GetDeviceInfo()->DRV_RetrieveCall(this, pCall);
		}

		// Note: we will complete the call with the first response from DRV_RetrieveCall.
		// If the first call fails the retrieve then the conference will fail, otherwise
		// it will succeed.
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RetrieveCall, then manage it.
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::RetrieveCall)
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

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;

}// CJTLine::OnAddToConference

/*****************************************************************************
** Procedure:  CJTLine::OnRemoveFromConf
**
** Arguments: 'pReq' - Request object representing this RemoveFromConf event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineRemoveFromConference 
**               processing for this service provider.
**
*****************************************************************************/
bool CJTLine::OnRemoveFromConference(RTRemoveFromConference* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallToRemove();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the ignore state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// If the call in question is a "fake" call generated simply to
		// collect enough digits using lineDial, then fail the request since
		// the call doesn't really exist and cannot be part of the conference.
		if (!pCall->IsRealCall())
		{
			CompleteRequest(pRequest, LINEERR_INVALCALLHANDLE);
			return false;
		}

		// Otherwise issue a RELEASE CALL event for the call id.
		GetDeviceInfo()->DRV_DropCall(this, pCall);
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
			peCommand->GetCommand() == CPECommand::ReleaseCall && pidError != NULL)
		{
			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, pidError->GetError());
			return true;
		}
	}

	// Check to see if our request has exceeded the limit for processing.
	// If so, tell TAPI that the request failed and delete the request.
	if (pRequest->GetState() == STATE_WAITING && 
		(pRequest->GetStateTime()+REQUEST_TIMEOUT) < GetTickCount())
		CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;

}// CJTLine::OnRemoveFromConference

/*****************************************************************************
** Procedure:  CJTLine::OnConferenceEvent
**
** Arguments: 'pCall' - 1st call which is in conference
**            'dwCall2' - 2nd call in conference 
**            'dwCall3' - 3rd call in conference (may be zero)
**
** Returns:    void
**
** Description:  This function handles the raw "Call Conferenced" event
**               which is sent when a conference is created
**
*****************************************************************************/
void CJTLine::OnConferenceEvent(CTSPICallAppearance* pCall, DWORD dwCallID2, DWORD dwCallID3)
{
	_TSP_ASSERTE(pCall != NULL);

	// See if we have an existing conference call owner for the first call.
	// If so we will continue to use it. This would be the case after a 
	// linePrepareAddToConference() where we put all the calls onHold but
	// left the conference constructed in the TSP++ layer.
	CTSPIConferenceCall* pConfCall = pCall->GetConferenceOwner();
	if (pConfCall == NULL)
	{
		// See if there is an existing conference on the line that this
		// call is being added to.
		pConfCall = dynamic_cast<CTSPIConferenceCall*>(FindCallByType(CTSPICallAppearance::Conference));
		if (pConfCall == NULL)
		{
			// Nope - we need to create a new conference call to hold all the
			// calls which are now conferenced according to the PBX.
			pConfCall = GetAddress(0)->CreateConferenceCall(NULL);
		}
	}

	// Add the first call to the conference
	pConfCall->AddToConference(pCall);
	pCall->SetCallState(LINECALLSTATE_CONFERENCED);

	// See if we have the second call
	if (dwCallID2 != 0)
	{
		pCall = FindCallByCallID(dwCallID2);
		if (pCall != NULL)
		{
			pConfCall->AddToConference(pCall);
			pCall->SetCallState(LINECALLSTATE_CONFERENCED);
		}
	}

	// See if we have the third call
	if (dwCallID3 != 0)
	{
		pCall = FindCallByCallID(dwCallID3);
		if (pCall != NULL)
		{
			pConfCall->AddToConference(pCall);
			pCall->SetCallState(LINECALLSTATE_CONFERENCED);
		}
	}

	// Finally, transition the conference to connected
	pConfCall->SetCallState(LINECALLSTATE_CONNECTED);

}// CJTLine::OnConferenceEvent
