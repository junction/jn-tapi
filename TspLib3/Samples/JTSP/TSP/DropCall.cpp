/***************************************************************************
//
// DROPCALL.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineDropCall processing
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
** Procedure:  CJTLine::OnDropCall
**
** Arguments: 'pReq' - Request object representing this DROPCALL event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineDrop processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnDropCall(RTDropCall* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// If the call in question is a "fake" call generated simply to
		// collect enough digits using lineDial, then just IDLE it and don't 
		// send anything to the PBX simulator since the call doesn't really exist.
		if (!pCall->IsRealCall())
		{
			_TSP_DTRACE(_T("Idle'ing phantom call appearance\n"));
			CompleteRequest(pRequest, 0);
			pCall->SetCallState(LINECALLSTATE_IDLE);
		}

		// Real call on the PBX...
		else
		{
			// Drop the call
			GetDeviceInfo()->DRV_DropCall(this, pCall);
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
			peCommand->GetCommand() == CPECommand::ReleaseCall)
		{
			DWORD dwError = (pidError) ? pidError->GetError() : 0;

			// If the call is currently DISCONNECTED and we got an error from 
			// the switch, then IDLE the call as the PBX has already deleted it.
			if (dwError == CPEErrorCode::InvalidCallID &&
				pCall->GetCallState() == LINECALLSTATE_DISCONNECTED)
			{
				// This case is OK..
				pCall->SetCallState(LINECALLSTATE_IDLE);
				dwError = 0;	
			}

			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, dwError);
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

}// CJTLine::OnDropCall

