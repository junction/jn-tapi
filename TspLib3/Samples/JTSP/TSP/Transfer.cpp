/***************************************************************************
//
// TRANSFER.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineTransfer/BlindTransfer processing
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
** Procedure:  CJTLine::OnSetupTransfer
**
** Arguments: 'pReq' - Request object representing this TRANSFER event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineSetupTransfer processing
**               for this service provider.  The Transfer Call event 
**               will transfer a call.
**
*****************************************************************************/
bool CJTLine::OnSetupTransfer(RTSetupTransfer* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallToTransfer();
	CTSPICallAppearance* pConsult = pRequest->GetConsultationCall();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// If the call we are going to transfer is not currently on hold then
		// place it on hold so it may be transferred by the switch
		if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER)) == 0)
			GetDeviceInfo()->DRV_HoldCall(this, pCall);

		// Or if our call is ALREADY onHold, then simply setup the consultation
		// call in the DIALTONE state.
		else
		{
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
		// transitioning to the onHold state then make it onHoldPendTransfer.
		// Return TRUE to skip the unsolicited handler.
		else if (pBlock->GetEventType() == CEventBlock::CallStateChange)
		{
			const CPECallState* pCS = dynamic_cast<const CPECallState*>(pBlock->GetElement(CPBXElement::CallState));
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
			if (pidCall != NULL && pidCall->GetCallID() == pCall->GetCallID() &&
				pCS->GetCallState() == CPECallState::Holding)
			{
				// Transition the existing call to the onHOLD pending transfer status.
				pCall->SetCallState(LINECALLSTATE_ONHOLDPENDTRANSFER);
			
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

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;

}// CJTLine::OnSetupTransfer

/*****************************************************************************
** Procedure:  CJTLine::OnCompleteTransfer
**
** Arguments: 'pReq' - Request object representing this TRANSFER event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineCompleteTransfer processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnCompleteTransfer(RTCompleteTransfer* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallToTransfer();
	CTSPICallAppearance* pConsult = pRequest->GetConsultationCall();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the ignore state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// If we are to transfer into a conference then simply issue a
		// RetrieveCall request against the held call. This will implicitly
		// create a conference.
		if (pRequest->GetTransferMode() == LINETRANSFERMODE_CONFERENCE)
			GetDeviceInfo()->DRV_RetrieveCall(this, pCall);

		// Otherwise complete the transfer of the held party to the consultation
		// call party.
		else
			GetDeviceInfo()->DRV_Transfer(this, pCall, pConsult);
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
			(peCommand->GetCommand() == CPECommand::TransferCall ||
			 peCommand->GetCommand() == CPECommand::RetrieveCall))
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

	return false;

}// CJTLine::OnCompleteTransfer

/*****************************************************************************
** Procedure:  CJTLine::OnTransferEvent
**
** Arguments: 'pCall' - Call being transferred - it belongs to this line.
**            'dwOldCallID' - Old call id if this is a merge of two calls (may be 0)
**            'dwTarget' - Target if this is a blind transfer (may be 0)
**            'peCaller' - Caller information (DNIS) may be NULL
**            'peCalled' - Called information (ANI) may be NULL
**
** Returns:    void
**
** Description:  This function manages the raw "Call Transferred" event from
**               the PBX for both solicited and unsolicited transfers of
**               the blind and consultation types. It also processes the 
**               "Call Queued" event for ACD queues.
**
*****************************************************************************/
void CJTLine::OnTransferEvent(CTSPICallAppearance* pCall, DWORD dwOldCallID, DWORD dwTarget,
							const CPECallInfo* peCaller, const CPECallInfo* peCalled)
{
	_TSP_ASSERTE(dwTarget != 0);

	// If there is a new callid then this is a merge of two calls on our line. Locate
	// the other call on this line device and transfer the call information from
	// this existing call to that new call. This call is about to go away.
	if (dwOldCallID != 0)
	{
		// Get the calls involved.
		CTSPICallAppearance* pCall_S1 = pCall->GetShadowCall();
		CTSPICallAppearance* pCall2   = FindCallByCallID(dwOldCallID);
		CTSPICallAppearance* pCall_S2 = pCall2->GetShadowCall();

		// Here is our map of lines/calls for consultation transfer
		//
		// (S1)							(pCall)  
		//   T1 -------------------------- A1
		//            (CALLID1)             | (pCall2)
		//                                  |
		//                                  |
		//                                  | (CALLID2)
		//                                  |
		//                                  |
		//                                  |
		//                                  |
		//                                  |
		//                                 A2 (pCall_S2)
		//
		//
		// When we are finished, we want it to look like:
		//
		// (S1)       (CALLID1)           (pCall_S2)
		//   T1 --------------------------- A2
		// 
		// pCall and pCall2 are both IDLE'd.

		// If we are missing any of the call information then stop.
		// Somehow we dropped events and are out of synch with the switch.
		// Wait for all the CR to filter through.
		if (pCall_S1 == NULL || pCall2 == NULL || pCall_S2 == NULL)
			return;

		// Now copy the call information from the original call to the
		// new call. This will also replace the callid and connect to the
		// other side of the existing call (if any). Since we are not
		// marking this as a shadow call, the redirecting information will
		// be set from the current call's information.
		pCall_S2->CopyCall(pCall, false, false);

		// Set the reason code as "transferred" or "routed" based on the 
		// line type which is causing this event.
		pCall_S2->SetCallReason((GetLineType() == CTSPILineConnection::Queue) ?
			LINECALLREASON_ROUTEREQUEST : LINECALLREASON_TRANSFER);

		// Dump the callerid off the original call which is getting dissolved
		// from this merge.
		pCall->SetCallID(0);
		pCall->SetCallState(LINECALLSTATE_IDLE);

		// pCall2 will be destroyed by a subseqent CR event from the PBX.
	}

	// Otherwise this is a blind transfer event and we should have a target line
	// device id. Create the call (if necessary) on the line and move the existing
	// call information to the new call.
	else 
	{
		// If we don't own the call then blow off the event - we see it twice since
		// we are a global monitor of the PBX.
		if (pCall->GetLineOwner() != this)
			return;

		// Locate the new line owner
		CTSPILineConnection* pLine = GetDeviceInfo()->FindLineConnectionByPermanentID(dwTarget);
		_TSP_ASSERTE(pLine != NULL);

		// Now move the call from one line to another. This creates the call on the
		// destination line if it doesn't already exist. Wait up to 1 second for the
		// unsolicited handler to see the "Call Detected" event and create a call object.
		pCall = pLine->GetAddress(0)->MoveCall(pCall, 
			(pLine->GetLineType() == CTSPILineConnection::Queue) ? 
			LINECALLREASON_ROUTEREQUEST : LINECALLREASON_TRANSFER,
			LINECALLSTATE_OFFERING, LINEOFFERINGMODE_INACTIVE, 1000);

		// Change the origin of the call - when it was created by the unsolicited handler,
		// it may have been incorrectly marked as an external inbound call if the
		// other side is a trunk. Blind transfers should always come from another internal
		// line.
		pCall->SetCallOrigin(LINECALLORIGIN_INTERNAL | LINECALLORIGIN_INBOUND);
	}

	// Now see if we got callerid and calledid information. If so, place it into the call.
	// Set the caller-id information with the call.
	if (peCaller != NULL)
	{
		DWORD dwFlags = LINECALLPARTYID_ADDRESS;
		if (!peCaller->GetName().empty())
			dwFlags |= LINECALLPARTYID_NAME;

		TString strNumber = peCaller->GetNumber();
		if (strNumber.length() > 4)
			strNumber = _T("9") + strNumber;
		pCall->SetCallerIDInformation(dwFlags, strNumber.c_str(), peCaller->GetName().c_str());
	}

	// Set the called-id information for this call
	if (peCalled != NULL)
	{
		DWORD dwFlags = LINECALLPARTYID_ADDRESS;
		if (!peCalled->GetName().empty())
			dwFlags |= LINECALLPARTYID_NAME;

		TString strNumber = peCalled->GetNumber();
		if (strNumber.length() > 4)
			strNumber = _T("9") + strNumber;
		pCall->SetCalledIDInformation(dwFlags, strNumber.c_str(), peCalled->GetName().c_str());
	}

}// CJTLine::OnTransferEvent
