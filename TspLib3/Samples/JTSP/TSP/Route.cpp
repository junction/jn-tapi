/***************************************************************************
//
// ROUTE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineRedirect processing
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
** Procedure:  CJTLine::OnRedirectOrBlindTransfer
**
** Arguments: 'pReq' - Request object representing this REDIRECT/BLINDXFER event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineRedirect and TSPI_lineBlindTransfer
**               processing for this service provider.
**
*****************************************************************************/
bool CJTLine::OnRedirectOrBlindTransfer(CTSPIRequest* pReq, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	RTDialInfo* pRequest = dynamic_cast<RTDialInfo*>(pReq);
	DIALINFO* pAddress = (pRequest->GetCount() > 0) ? pRequest->GetDialableNumber(0) : NULL;
	CTSPICallAppearance* pCall = pReq->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Send the blind transfer request for this call to the switch. If this
		// line is a route point, then it interpreted by the switch as a redirect
		// rather than a real blind transfer.
		GetDeviceInfo()->DRV_BlindTransfer(this, pCall, pAddress->strNumber);
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
			peCommand->GetCommand() == CPECommand::BlindTransfer && pidError != NULL)
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

}// CJTLine::OnRedirectOrBlindTransfer
