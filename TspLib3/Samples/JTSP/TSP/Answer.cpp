/***************************************************************************
//
// ANSWER.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineAnswer processing
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
** Procedure:  CJTLine::OnAnswer
**
** Arguments: 'pReq' - Request object representing this ANSWER event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineAnswer processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnAnswer(RTAnswer* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
		GetDeviceInfo()->DRV_Answer(this, pCall);

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our RELEASECALL, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::AnswerCall && pidError != NULL)
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

}// CJTLine::OnAnswer

/*****************************************************************************
** Procedure:  CJTLine::OnPDialerMediaDetected
**
** Arguments: 'pCall' - Call the predictive dialer was working with
**            'iAType' - Answer type detected
**
** Returns:    void
**
** Description: This function handles the predictive dialer media detection
**              event which we receive when the dialer has completed a call. 
**
*****************************************************************************/
void CJTLine::OnPDialerMediaDetected(CTSPICallAppearance* pCall, int iAType)
{
	// Based on the type, set the media mode for the call
	DWORD dwMediaMode = 0;
	switch (iAType)
	{
		// Busy tone detected
		case CPEAnswerType::Busy:
			pCall->SetCallState(LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_BUSY);
			return;

		// Call timed out
		case CPEAnswerType::NoAnswer:
			pCall->SetCallState(LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NOANSWER);
			return;

		// Got a person!
		case CPEAnswerType::AnswerVoice:
			dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
			break;

		// Got a machine..
		case CPEAnswerType::AnswerMachine:
			dwMediaMode = LINEMEDIAMODE_AUTOMATEDVOICE;
			break;

		// Don't know what we got - return unknown.
		case CPEAnswerType::AnswerUnknown:
			dwMediaMode = (LINEMEDIAMODE_AUTOMATEDVOICE | LINEMEDIAMODE_INTERACTIVEVOICE | LINEMEDIAMODE_UNKNOWN);
			break;
	}

	// Set the media mode for the call.
	pCall->SetMediaMode(dwMediaMode);

	// Mark the call as connected!
	pCall->SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVE, dwMediaMode);

}// CJTLine::OnPDialerMediaDetected
