/***************************************************************************
//
// MAKECALL.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineMakeCall and TSPI_lineDial management
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
** Procedure:  CJTLine::OnMakeCall
**
** Arguments: 'pReq' - Request object representing this MAKECALL event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineMakeCall processing
**               for this service provider. 
**
*****************************************************************************/
bool CJTLine::OnMakeCall(RTMakeCall* pRequest, LPCVOID lpBuff)
{
	// If this is a predictive dialer, then handle it differently
	if (GetLineType() == CTSPILineConnection::PredictiveDialer)
		return OnPredictiveMakeCall(pRequest, lpBuff);

	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();
	DIALINFO* pAddress = (pRequest->GetCount() > 0) ? pRequest->GetDialableNumber(0) : NULL;

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the ignore state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		// If we have no dialing information (i.e. NULL specified in lineMakeCall)
		// or the number given is incomplete (i.e. it is terminated by a semicolon
		// as per the TAPI specification) then mark the call as a "fake" call until 
		// we collect enough digits to actually place the call.
		if (pAddress == NULL || pAddress->strNumber.empty() || pAddress->fIsPartialAddress)
		{
			// Mark the call as a "fake" call generated for TAPI to
			// dial upon but not taking up bandwidth on the switch yet.
			pCall->MarkReal(false);

			// Append the information from our dial string into our buffer which 
			// we will eventually dial.
			TString& strDigits = pCall->GetPartiallyDialedDigits();
			strDigits = (pAddress != NULL) ? pAddress->strNumber : _T("");

			// If this is a fake call waiting for lineDial requests then go ahead
			// and delete the request since we won't be working with it anymore at
			// this level (we wait for subsequent commands from the application).
			CompleteRequest(pRequest, 0);	// Deletes the request packet

			// If we have some initial dialing information with this request (non-NULL)
			// dialing string passed to lineMakeCall)
			if (pAddress != NULL && pAddress->strNumber.size() >= 1)
			{
				// If the number starts with '9' then transition to external dialtone.
				if (pAddress->strNumber[0] == _T('9'))
					pCall->SetCallState(LINECALLSTATE_DIALTONE, 
						LINEDIALTONEMODE_EXTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);

				// Finally, if we have more than one digit in the string then 
				// transition to the DIALING state.
				if (pAddress->strNumber.size() > 1)
					pCall->SetCallState(LINECALLSTATE_DIALING);
			}
		}

		// Otherwise we have a number to dial - send it to the PBX
		else
		{
			// Transition to the WAITING state
			pRequest->SetState(STATE_WAITING);

			// Send the dial string to the PBX.
			GetDeviceInfo()->DRV_MakeCall(this, pAddress->strNumber, pRequest->GetCountryCode());
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
		if (pBlock->GetEventType() == CEventBlock::CommandResponse)
		{
			// If there was an error, then complete the TAPI request with the
			// appropriate error code.
			if (peCommand->GetCommand() == CPECommand::PlaceCall && pidError != NULL)
			{
				// Complete the request
				TranslateErrorCode(pRequest, pidError->GetError());
				if (pidError->GetError() != 0)
					pCall->SetCallState(LINECALLSTATE_IDLE);
				return true;
			}
		}

		// If this is a "CALL PLACED" event then record the callid into our newly
		// placed call so the unsolicited handler won't create a new call.  We will
		// let the unsolicited handler do all the other work.
		else if (pBlock->GetEventType() == CEventBlock::CallPlaced)
		{
			// Set the call id
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
			pCall->SetCallID(pidCall->GetCallID());

			// This should be the first event we have seen regarding the call's 
			// successful placement on the switch.

			// Send TAPI a response but don't delete the request object from the
			// line queue. This is required for apps to see the following
			// state changes.
			CompleteRequest(pRequest, 0, true, false);

			// If we have a name as part of the dialing string (ISDN format) then
			// set that as the "CALLED NAME" to show how to access the name/subaddress
			// information.
			if (!pAddress->strName.empty())
				pCall->SetCalledIDInformation(LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS, 
							pAddress->strNumber.c_str(), pAddress->strName.c_str());

			// Start in internal dialtone state
			pCall->SetCallState(LINECALLSTATE_DIALTONE, 
				LINEDIALTONEMODE_INTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);

			// If the number starts with '9' then transition to external dialtone.
			if (pAddress->strNumber[0] == _T('9'))
				pCall->SetCallState(LINECALLSTATE_DIALTONE, 
					LINEDIALTONEMODE_EXTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);

			// Leave it in the DIALING state.
			pCall->SetCallState(LINECALLSTATE_DIALING);
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

}// CJTLine::OnMakeCall

/*****************************************************************************
** Procedure:  CJTLine::OnPredictiveMakeCall
**
** Arguments: 'pReq' - Request object representing this MAKECALL event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineMakeCall processing
**               for this service provider when the target is a predictive dialer
**               line.
**
*****************************************************************************/
bool CJTLine::OnPredictiveMakeCall(RTMakeCall* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();
	DIALINFO* pAddress = (pRequest->GetCount() > 0) ? pRequest->GetDialableNumber(0) : NULL;
	LPLINECALLPARAMS pCallParams = pRequest->GetCallParameters();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the ignore state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		// If we have no dialing information (i.e. NULL specified in lineMakeCall)
		// or the number given is incomplete (i.e. it is terminated by a semicolon
		// as per the TAPI specification) then fail this request - we don't allow
		// partial dialing on the predictive dialer lines.
		if (pAddress == NULL || pAddress->strNumber.empty() || 
			pAddress->fIsPartialAddress || pCallParams == NULL)
			CompleteRequest(pRequest, LINEERR_INVALPARAM);
		else 
		{
			// If the no-answer timeout is invalid then adjust it.
			int iTransferTimeout = max(pCallParams->dwNoAnswerTimeout, 60);

			// Get the target to transfer the call to.
			TString strTargetNumber = (pCallParams->dwTargetAddressSize > 0 &&
									   pCallParams->dwTargetAddressOffset > 0) ?
				   ConvertWideToAnsi((LPCWSTR)((LPBYTE)pCallParams + pCallParams->dwTargetAddressOffset)) :
					_T("");
			
			// Get the transfer state
			DWORD dwTransferState = (pCallParams->dwPredictiveAutoTransferStates & 
					GetAddress(0)->GetAddressCaps()->dwPredictiveAutoTransferStates);
			if (dwTransferState != pCallParams->dwPredictiveAutoTransferStates)
				CompleteRequest(pRequest, LINEERR_INVALPARAM);
			else
			{
				// Transition to the WAITING state
				pRequest->SetState(STATE_WAITING);

				// Send the dial string to the PBX.
				GetDeviceInfo()->DRV_MakePredictiveCall(this, pAddress->strNumber, pRequest->GetCountryCode(), iTransferTimeout, strTargetNumber);
			}
		}
	}

	// If we are in the waiting stage (2) then see if we received an event from the
	// switch (vs. an interval timer) and if that event was an ACK/NAK in response
	// to the command we issued.
	else if (pRequest->GetState() == STATE_WAITING && pBlock != NULL)
	{
		// If this is a command response for our MAKEPREDICTIVECALL, then manage it.
		const CPEErrorCode* pidError = dynamic_cast<const CPEErrorCode*>(pBlock->GetElement(CPBXElement::ErrorCode));
		const CPECommand* peCommand = dynamic_cast<const CPECommand*>(pBlock->GetElement(CPBXElement::Command));
		if (pBlock->GetEventType() == CEventBlock::CommandResponse &&
			peCommand->GetCommand() == CPECommand::PlacePredictiveCall && pidError != NULL)
		{
			// Complete the request
			TranslateErrorCode(pRequest, pidError->GetError());
			if (pidError->GetError() != 0)
				pCall->SetCallState(LINECALLSTATE_IDLE);
			return true;
		}

		// If this is a "CALL PLACED" event then record the callid into our newly
		// placed call so the unsolicited handler won't create a new call.  We will
		// let the unsolicited handler do all the other work.
		else if (pBlock->GetEventType() == CEventBlock::CallPlaced)
		{
			// Set the callid for this call
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
			pCall->SetCallID(pidCall->GetCallID());

			// This should be the first event we see about the success of placing the
			// call on the switch.

			// Send TAPI a response but don't delete the request object from the
			// line queue. This is required for apps to see the following
			// state changes.
			CompleteRequest(pRequest, 0, true, false);

			// Start in external dialtone state
			pCall->SetCallState(LINECALLSTATE_DIALTONE, 
				LINEDIALTONEMODE_EXTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);

			// Transition to DIALING
			pCall->SetCallState(LINECALLSTATE_DIALING);
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

}// CJTLine::OnPredictiveMakeCall

/*****************************************************************************
** Procedure:  CJTLine::OnDial
**
** Arguments: 'pReq' - Request object representing this DIAL event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineDial processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnDial(RTDial* pRequest, LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();
	DIALINFO* pAddress = (pRequest->GetCount() > 0) ? pRequest->GetDialableNumber(0) : NULL;
	
	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the ignore state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		// If the call has already been dialed, the fail this event.
		// The switch doesn't allow us to continue to dial on an open 
		// connection.
		if (pCall->IsRealCall())
			CompleteRequest(pRequest, LINEERR_OPERATIONUNAVAIL);
		else
		{
			// Append the information from our dial string into our
			// buffer which we will eventually dial.
			TString& strDigits = pCall->GetPartiallyDialedDigits();
			strDigits += pAddress->strNumber;

			// If we are currently in the dialtone state and the first digit
			// is a '9' then transition to the external dialtone state
			if (pCall->GetCallState() == LINECALLSTATE_DIALTONE &&
				strDigits.size() > 1 && strDigits[0] == _T('9'))
				pCall->SetCallState(LINECALLSTATE_DIALTONE, 
					LINEDIALTONEMODE_EXTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);

			// Transition to dialing once we have "dialed" one digit.
			if (strDigits.size() > 1)
				pCall->SetCallState(LINECALLSTATE_DIALING);

			// If the number is partial (i.e. we are not done dialing)
			// then simply complete the request here.
			if (pAddress->fIsPartialAddress)
				CompleteRequest(pRequest, 0);

			// Otherwise we have a number to dial - send it to the PBX
			else
			{
				// Transition to the WAITING state
				pRequest->SetState(STATE_WAITING);

				// Send the dial string to the PBX.
				GetDeviceInfo()->DRV_MakeCall(this, strDigits, 0);
			}
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
			peCommand->GetCommand() == CPECommand::PlaceCall && pidError != NULL)
		{
			// Complete the request with the appropriate error code.
			TranslateErrorCode(pRequest, pidError->GetError());

			// Force the call to idle on an error.
			if (pidError->GetError() != 0)
				pCall->SetCallState(LINECALLSTATE_IDLE);

			return true;
		}

		// If this is a "CALL PLACED" event then record the callid into our newly
		// placed call so the unsolicited handler won't create a new call.  We will
		// let the unsolicited handler do all the other work.
		else if (pBlock->GetEventType() == CEventBlock::CallPlaced)
		{
			// Set the call-id for this call
			const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
			pCall->SetCallID(pidCall->GetCallID());

			// Mark it as a real call as it is now officially on the PBX.
			pCall->MarkReal(true);
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

}// CJTLine::OnDial

/*****************************************************************************
** Procedure:  CJTLine::OnNewCallDetected
**
** Arguments: 'dwCallID' - CallID passed
**            'peCaller' - Caller information (DNIS)
**            'peCalled' - Called information (ANI)
**
** Returns:    void
**
** Description:  This function manages the call detected notification.
**
*****************************************************************************/
CTSPICallAppearance* CJTLine::OnNewCallDetected(bool fPlaced,
							CTSPICallAppearance* pCall, DWORD dwCallID, 
							const CPECallInfo* peCaller, const CPECallInfo* peCalled)
{
	// If we have not yet created a call then do so. We may already have a call
	// object created if we are placing the call from the TSP.
	if (pCall == NULL)
	{
		// Create the call appearance and set the call-id in case a subsequent request
		// on another line is searching for this call.
		pCall = GetAddress(0)->CreateCallAppearance();
		pCall->SetCallID(dwCallID);

		// Set the callstate to offering or dialing for this new call.
		if (fPlaced)
			pCall->SetCallState(LINECALLSTATE_DIALING, 0, LINEMEDIAMODE_INTERACTIVEVOICE);
		else
			pCall->SetCallState(LINECALLSTATE_OFFERING, LINEOFFERINGMODE_INACTIVE, LINEMEDIAMODE_INTERACTIVEVOICE);
	}				

	// See if this call has a trunk associated with it. The trunk may be on either
	// side of a call so check both sides.
	CTSPICallAppearance* pCall_Other = pCall->GetShadowCall();
	if (pCall_Other != NULL)
	{
		// Get the line owner for the other side of the call.
		if (pCall_Other->GetLineOwner()->GetLineType() == CTSPILineConnection::Trunk)
			pCall->SetTrunkID(pCall_Other->GetLineOwner()->GetPermanentDeviceID());
		else if (GetLineType() == CTSPILineConnection::Trunk)
			pCall_Other->SetTrunkID(GetPermanentDeviceID());
	}

	// If this line is a trunk then mark the trunk id.
	else if (GetLineType() == CTSPILineConnection::Trunk && pCall_Other != NULL)
		pCall->SetTrunkID(pCall_Other->GetLineOwner()->GetPermanentDeviceID());

	// Set the caller-id information with the call.
	if (peCaller != NULL)
	{
		DWORD dwFlags = LINECALLPARTYID_ADDRESS;
		if (!peCaller->GetName().empty())
			dwFlags |= LINECALLPARTYID_NAME;

		// If the number is an outside number, then add the prefix for outside lines
		// to the caller-number.  The PBX doesn't return this information and if we pass
		// this into the ConvertDialableToCanonical function, it might take the first digit
		// as our outside line for area codes which start with '9'.
		TString strNumber = peCaller->GetNumber();
		if (strNumber.length() > 4)
			strNumber = _T("9") + strNumber;
		pCall->SetCallerIDInformation(dwFlags, strNumber.c_str(), peCaller->GetName().c_str());
	}

	// Set the called-id information for this call
	if (peCalled != NULL && 
		 (pCall->GetCallInfo()->dwCalledIDFlags & LINECALLPARTYID_NAME) == 0)
	{
		DWORD dwFlags = LINECALLPARTYID_ADDRESS;
		if (!peCalled->GetName().empty())
			dwFlags |= LINECALLPARTYID_NAME;

		// If the number is an outside number, then add the prefix for outside lines
		// to the called-number.  The PBX doesn't return this information and if we pass
		// this into the ConvertDialableToCanonical function, it might take the first digit
		// as our outside line for area codes which start with '9'.
		TString strNumber = peCalled->GetNumber();
		if (strNumber.length() > 4)
			strNumber = _T("9") + strNumber;
		pCall->SetCalledIDInformation(dwFlags, strNumber.c_str(), peCalled->GetName().c_str());
	}

	// If we have no reason code for this call then assign it now.
	LPLINECALLINFO lpci = pCall->GetCallInfo();
	if (lpci->dwReason == LINECALLREASON_UNKNOWN)
	{
		if (GetLineType() == CTSPILineConnection::Station || 
			GetLineType() == CTSPILineConnection::VRU ||
			GetLineType() == CTSPILineConnection::RoutePoint)
			pCall->SetCallReason(LINECALLREASON_DIRECT);
		else if (GetLineType() == CTSPILineConnection::Queue)
			pCall->SetCallReason(LINECALLREASON_ROUTEREQUEST);
	}

	// Set sample QOS, Charging information and Low/Hi level compatibility
	// information. Note - this is simply a sample of how to do these tasks -
	// the information presented in these structures does not conform to the
	// ISDN Q.931 standards.
	pCall->SetQualityOfService("Sending FlowSpec", 17, "Recv Flowspec", 14);
	pCall->SetChargingInformation("Charging Information", 21);
	pCall->SetLowLevelCompatibilityInformation("LowLevel Compatibility Info", 28);
	pCall->SetHiLevelCompatibilityInformation("HiLevel Compatibility Info", 27);

	// Set the call origin for this call.
	// If this line is the originator of the call then mark as an outbound call.
	if (fPlaced || (DWORD)_ttoi(peCaller->GetNumber().c_str()) == GetPermanentDeviceID())
		pCall->SetCallOrigin(LINECALLORIGIN_OUTBOUND);

	// Otherwise mark as an incoming call.
	else 
	{
		if (pCall->GetCallInfo()->dwTrunk != 0xffffffff)
			pCall->SetCallOrigin(LINECALLORIGIN_EXTERNAL | LINECALLORIGIN_INBOUND);
		else
			pCall->SetCallOrigin(LINECALLORIGIN_INTERNAL | LINECALLORIGIN_INBOUND);
	}
	return pCall;

}// CJTLine::OnNewCallDetected

/*****************************************************************************
** Procedure:  CJTLine::OnCallStateChange
**
** Arguments: 'pCall' - Call that is changing state
**            'pCS' - Event from PBX
**
** Returns:    void
**
** Description:  This function handles a call state change event from the PBX.
**
*****************************************************************************/
void CJTLine::OnCallStateChange(CTSPICallAppearance* pCall, const CPECallState* pCS)
{
	_TSP_ASSERTE(pCS != NULL);
	_TSP_ASSERTE(pCall != NULL);

	// Change the call state based on the returned data
	switch (pCS->GetCallState())
	{
		case CPECallState::Dialing:		
			pCall->SetCallState(LINECALLSTATE_PROCEEDING); 
			break;
		case CPECallState::Ringing:		
			if (pCall->IsOutgoingCall())
				pCall->SetCallState(LINECALLSTATE_RINGBACK); 
			else 
				pCall->SetCallState(LINECALLSTATE_OFFERING);
			break;
		case CPECallState::Alerting:	
			if (!pCall->IsOutgoingCall())
				pCall->SetCallState(LINECALLSTATE_ACCEPTED); 
			break;
		case CPECallState::Connected:
			pCall->SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVE);
			break;
		case CPECallState::Busy:
			pCall->SetCallState(LINECALLSTATE_BUSY);
			break;
		case CPECallState::Disconnected:
			pCall->SetCallState(LINECALLSTATE_DISCONNECTED);
			break;
		case CPECallState::Holding:
			pCall->SetCallState(LINECALLSTATE_ONHOLD);
			break;
	}

	// If the call is part of a conference on this line and is now not CONNECTED
	// then pull it out of the conference.
	if (pCall->GetConferenceOwner() != NULL && 
		pCall->GetCallState() != LINECALLSTATE_CONNECTED)
	{
		CTSPIConferenceCall* pConfCall = pCall->GetConferenceOwner();
		pConfCall->RemoveConferenceCall(pCall, true);
	}

}// CJTLine::OnCallStateChange
