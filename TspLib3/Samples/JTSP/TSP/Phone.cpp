/***************************************************************************
//
// PHONE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Phone management functions
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

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "jtsp.h"

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-------------------------------------------------------------------------------*/
// TSPI Request map
/*-------------------------------------------------------------------------------*/
BEGIN_TSPI_REQUEST(CJTPhone)
	ON_TSPI_REQUEST_SETHOOKSWITCHGAIN(OnSetGain)
	ON_TSPI_REQUEST_SETHOOKSWITCH(OnSetHookswitch)
	ON_TSPI_REQUEST_SETHOOKSWITCHVOL(OnSetVolume)
END_TSPI_REQUEST()

/*****************************************************************************
** Procedure:  CJTPhone::read
**
** Arguments:  'istm'			- Input stream
**
** Returns:    pointer to istm
**
** Description:  This function is called to serialize data in from the
**               registry.  The phone object has already been completely
**               initialized by the TSP++ library
**
*****************************************************************************/
TStream& CJTPhone::read(TStream& istm)
{
	// Load our default display which is used while we have no agent
	// logged into this station.
	SetDisplay(_T("  NOT LOGGED ON"));

	// Return the base class loading.
	return CTSPIPhoneConnection::read(istm);

}// CJTPhone::read

/*****************************************************************************
** Procedure:  CJTPhone::OpenDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description: This method is called when the phone is opened by TAPI.
**              We want to check and see if the associated line is currently CONNECTED
**              to the TL server.
**
*****************************************************************************/
bool CJTPhone::OpenDevice()
{
	// If the associated line is not connected then fail the open request.
	// The connected flag on the line is used to determine whether we are connected
	// to the simulator.
	if ((GetAssociatedLine()->GetLineDevStatus()->dwDevStatusFlags & LINEDEVSTATUSFLAGS_CONNECTED) == 0)
		return false;
	return CTSPIPhoneConnection::OpenDevice();

}// CJTPhone::OpenDevice

/*****************************************************************************
** Procedure:  CJTPhone::OnTimer
**
** Arguments:  void
**
** Returns:    void
**
** Description: This method is called periodically by the interval timer
**
*****************************************************************************/
void CJTPhone::OnTimer()
{
	// Poll the active request for timeout
	ReceiveData();

}// CJTPhone::OnTimer

/*****************************************************************************
** Procedure:  CJTPhone::OnSetGain
**
** Arguments: 'pReq' - Request object representing this phone request
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_phoneSetGain processing
**               for this service provider. 
**
*****************************************************************************/
bool CJTPhone::OnSetGain(RTSetGain* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Send the command to the switch
		GetDeviceInfo()->DRV_SetGain(this, pRequest->GetGain());
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our SETGAIN, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::SetGain && pidError != NULL)
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
		CompleteRequest(pRequest, PHONEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;
	
}// CJTPhone::OnSetGain

/*****************************************************************************
** Procedure:  CJTPhone::OnSetHookswitch
**
** Arguments: 'pReq' - Request object representing this phone request
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_phoneSetHookSwitch processing
**               for this service provider. 
**
*****************************************************************************/
bool CJTPhone::OnSetHookswitch(RTSetHookswitch* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Validate the state passed
		if (pRequest->GetHookswitchState() == PHONEHOOKSWITCHMODE_ONHOOK ||
			pRequest->GetHookswitchState() == PHONEHOOKSWITCHMODE_MIC)
			CompleteRequest(pRequest, PHONEERR_INVALHOOKSWITCHMODE);
		// Send the command to the switch
		else
			GetDeviceInfo()->DRV_SetHookswitch(this, (pRequest->GetHookswitchState() == PHONEHOOKSWITCHMODE_MICSPEAKER) ? 1 : 0);
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our SETGAIN, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::SetHookSwitch && pidError != NULL)
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
		CompleteRequest(pRequest, PHONEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;
	
}// CJTPhone::OnSetHookswitch

/*****************************************************************************
** Procedure:  CJTPhone::OnSetVolume
**
** Arguments: 'pReq' - Request object representing this phone request
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_phoneSetVolume processing
**               for this service provider. 
**
*****************************************************************************/
bool CJTPhone::OnSetVolume(RTSetVolume* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Send the command to the switch
		GetDeviceInfo()->DRV_SetVolume(this, pRequest->GetVolume());
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our SETGAIN, then manage it.
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::SetVolume && pidError != NULL)
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
		CompleteRequest(pRequest, PHONEERR_OPERATIONFAILED);

	// Let the request fall through to the unsolicited handler where we
	// set all the options concerning the newly found call.
	return false;
	
}// CJTPhone::OnSetVolume

/*****************************************************************************
** Procedure:  CJTPhone::TranslateErrorCode
**
** Arguments: 'pRequest' - Request object representing this phone request
**            'dwError' - Error code
**
** Returns:    void
**
** Description:  This function completes the request with an appropriate
**               TAPI error code based on the PBX error received.
**
*****************************************************************************/
void CJTPhone::TranslateErrorCode(CTSPIRequest* pRequest, DWORD dwError)
{
	switch (dwError)
	{
		case CPEErrorCode::None:				dwError = 0; break;
		case CPEErrorCode::InvalidDevice:		dwError = PHONEERR_BADDEVICEID; break;
		case CPEErrorCode::BadCommand:			dwError = PHONEERR_INVALPHONESTATE;	break;
		case CPEErrorCode::InvalidParameter:	dwError = PHONEERR_INVALPARAM; break;
		default: dwError = PHONEERR_OPERATIONFAILED; break;
	}
	CompleteRequest(pRequest, dwError);

}// CJTPhone::TranslateErrorCode
