/***************************************************************************
//
// HOLD.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineHold/Unhold/SwapHold processing
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
** Procedure:  CJTLine::OnHoldCall
**
** Arguments: 'pReq' - Request object representing this HOLD event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineHold processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnHoldCall(RTHold* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
		GetDeviceInfo()->DRV_HoldCall(this, pCall);

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RELEASECALL, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::HoldCall && pidError != NULL)
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

}// CJTLine::OnHoldCall

/*****************************************************************************
** Procedure:  CJTLine::OnRetrieveCall
**
** Arguments: 'pReq' - Request object representing this UNHOLD event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineUnhold processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnRetrieveCall(RTUnhold* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
		GetDeviceInfo()->DRV_RetrieveCall(this, pCall);

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RELEASECALL, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::RetrieveCall && pidError != NULL)
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

}// CJTLine::OnRetrieveCall

/*****************************************************************************
** Procedure:  CJTLine::OnSwapHold
**
** Arguments: 'pReq' - Request object representing this SWAPHOLD event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineSwapHold processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSwapHold(RTSwapHold* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pActiveCall = pRequest->GetActiveCall();
	CTSPICallAppearance* pHeldCall = pRequest->GetHoldingCall();

	// If the holding call is a conference call then we are currently building
	// a conference call and want to swap over to the other party in the conference.
	// Use the first call in the conference itself.
	if (pHeldCall->GetCallType() == CTSPICallAppearance::Conference)
		pHeldCall = dynamic_cast<CTSPIConferenceCall*>(pHeldCall)->GetConferenceCall(0);

	_TSP_ASSERTE(pHeldCall != NULL);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
		GetDeviceInfo()->DRV_HoldCall(this, pActiveCall);

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));

		// Watch for ACK/NAK responses to either a HOLD or RETRIEVE call command
		if (pBlock->GetEventType() == CEventBlock::CommandResponse && pidError != NULL &&
			(peCommand->GetCommand() == CPECommand::HoldCall ||	peCommand->GetCommand() == CPECommand::RetrieveCall))
		{
			// If this is an ACK for our HOLDCALL request then retrieve the other call
			// so that it is now active.
			if (peCommand->GetCommand() == CPECommand::HoldCall && pidError->GetError() == 0)
			{
				// If our held call has a conference owner, then swap it for this new call.
				DWORD dwCallState = pHeldCall->GetCallState();
				if (pHeldCall->GetConferenceOwner())
				{
					// Get the conference owner
					CTSPIConferenceCall* pConfCall = pHeldCall->GetConferenceOwner();

					// Remove the only party from the conference - don't
					// let the conference break down.
					pConfCall->RemoveConferenceCall(pHeldCall, false);

					// Add our new onHold party to the conference. It is still connected 
					// so it is not yet added to the conference array, only setup with 
					// a conference owner.
					pConfCall->AddToConference(pActiveCall);

					// Detach the consultation call since it is now attached 
					// through the conference.
					pActiveCall->DetachCall();
					pConfCall->DetachCall();

					// Change the state of the original call within the
					// conference (which should be CONFERENCED) to
					// onHold so we transition it correctly.
					pHeldCall->SetCallState(LINECALLSTATE_ONHOLD);
				}

				// Change the state of the now holding call to be the state of the 
				// original onHold call itself.  We want to preserve the 
				// transfer/conference state.
				pActiveCall->SetCallState(dwCallState);

				// Retrieve the held call
				GetDeviceInfo()->DRV_RetrieveCall(this, pHeldCall);
			}

			// If this is an ACK response to our RetrieveCall command then complete
			// the request and finish connecting up the calls
			else if (peCommand->GetCommand() == CPECommand::RetrieveCall &&	pidError->GetError() == 0)
			{
				// Re-attach the newly connected call as our consultation call for the conference 
				// so that we will properly monitor it for IDLE.
				if (pActiveCall->GetConferenceOwner())
				{
					pActiveCall->GetConferenceOwner()->AttachCall(pHeldCall);
					pHeldCall->AttachCall(pActiveCall->GetConferenceOwner());
					pHeldCall->SetCallType(CTSPICallAppearance::Consultant);
				}
				CompleteRequest(pRequest, 0);
			}

			// Complete the request with the appropriate error code.
			else
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

}// CJTLine::OnSwapHold
