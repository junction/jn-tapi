/***************************************************************************
//
// UNSOLICITED.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Unsolicited event processing - the main work horse of the
// service provider
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
** Procedure:  CJTLine::UnsolicitedEvent
**
** Arguments:  'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function is called when no request processed a given
**               response from the device and it was directed at this line.
**
*****************************************************************************/
bool CJTLine::UnsolicitedEvent(LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);

#ifdef _DEBUG
	// Output a dump of the received block with all it's elements.
	_TSP_DTRACE(_T("EventHandler: %s"), pBlock->Dump().c_str());
#endif

	// See if there is a callid attached to this request. If so, pull it out and
	// attempt to locate the TSP++ call object which is associated with the call.
	CTSPICallAppearance* pCall = NULL;
	const CPECallID* pidCall = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::CallID));
	if (pidCall != NULL)
	{
		// Find the call associated with the callid
		pCall = FindCallByCallID(pidCall->GetCallID());

		// If we have no call object and this is NOT a CP/CD event then ignore it.
		// Our PBX switch has no mechanism for querying information on an existing call.
		// A production-level TSP might make some educated guesses here and possibly
		// create a new call to reflect this one.
		if (pCall == NULL && 
				(pBlock->GetEventType() != CEventBlock::CallDetected &&
				 pBlock->GetEventType() != CEventBlock::CallPlaced))
		{
			_TSP_DTRACE(_T("Ignoring event with unknown callid 0x%lx\n"), pidCall->GetCallID());
			return false;
		}
	}

	// Process the line event we just received.
	switch (pBlock->GetEventType())
	{
		// Agent state changed
		case CEventBlock::AgentStateChanged:
		{
			const CPEAgentState* peState = dynamic_cast<const CPEAgentState*>(pBlock->GetElement(CPBXElement::AgentState));
			OnAgentStateChange(peState->GetAgentState());
		}
		break;

		// Agent group changed
		case CEventBlock::AgentGroupChanged:
		{
			const CPEAgentID* peAgent = dynamic_cast<const CPEAgentID*>(pBlock->GetElement(CPBXElement::AgentID));
			const CPEAgentGroup* pePGroup = dynamic_cast<const CPEAgentGroup*>(pBlock->GetElement(CPBXElement::PrimaryAgentGroup));
			const CPEAgentGroup* peSGroup = dynamic_cast<const CPEAgentGroup*>(pBlock->GetElement(CPBXElement::SecondaryAgentGroup));
			OnAgentGroupChange(peAgent->GetAgentID().c_str(), pePGroup->GetAgentGroup(), peSGroup->GetAgentGroup());
		}
		break;

		// Call Detected or placed (via PlaceCall or PredictivePlaceCall) on the line.
		case CEventBlock::CallDetected:
		case CEventBlock::CallPlaced:
		{
			const CPECallInfo* peCaller = dynamic_cast<const CPECallInfo*>(pBlock->GetElement(CPBXElement::DNIS));
			const CPECallInfo* peCalled = dynamic_cast<const CPECallInfo*>(pBlock->GetElement(CPBXElement::ANI));
			pCall = OnNewCallDetected((pBlock->GetEventType() == CEventBlock::CallPlaced),
										pCall, pidCall->GetCallID(), peCaller, peCalled);
		}
		break;

		// Call released - callid is no longer valid.
		case CEventBlock::CallReleased:
			if (pCall != NULL)
			{
				pCall->SetCallState(LINECALLSTATE_IDLE);
				// If our current call count is now zero, then force the TSP to
				// re-evaluate our current agent state and features - this is
				// because we cannot rely on the PBX simulator to send the "ASC"
				// event after all calls are deallocated (vs. before). When a call
				// is transferred off a line, the "ASC" event precedes the "CR" event
				// and causes our agent features to be incorrectly reported.
				if (GetAddress(0)->GetAddressStatus()->dwNumActiveCalls == 0)
					OnAgentStateChange(-1);
			}
			break;

		// Call state has changed
		case CEventBlock::CallStateChange:
		{
			const CPECallState* pCS = dynamic_cast<const CPECallState*>(pBlock->GetElement(CPBXElement::CallState));
			OnCallStateChange(pCall, pCS);
		}
		break;

		// DTMF Digit detected on the call
		case CEventBlock::DigitDetected:
		{
			const CPEDigit* pDigit = dynamic_cast<const CPEDigit*>(pBlock->GetElement(CPBXElement::Digit));
			// Pass the digit through to the call - simulator up/down press for duration.
			pCall->OnDigit(LINEDIGITMODE_DTMF, pDigit->GetDigit());
			pCall->OnDigit(LINEDIGITMODE_DTMFEND, pDigit->GetDigit());
		}
		break;

		// Call queued to ACD (transfer event to ACD queue)
		case CEventBlock::CallQueued:
		{
			const CPEExtension* pTarget = dynamic_cast<const CPEExtension*>(pBlock->GetElement(CPBXElement::Queue));
			OnTransferEvent(pCall, 0, pTarget->GetExtension(), NULL, NULL);
		}
		break;

		// Predictive Dialer media detected
		case CEventBlock::CallMediaDetected:
		{
			const CPEAnswerType* pidAnswer = dynamic_cast<const CPEAnswerType*>(pBlock->GetElement(CPBXElement::AnswerType));
			OnPDialerMediaDetected(pCall, pidAnswer->GetAnswerType());
		}
		break;

		// Call Conference created
		case CEventBlock::CallConference:
		{
			const CPECallID* pidCall1 = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::ConfCallID1));
			const CPECallID* pidCall2 = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::ConfCallID2));
			OnConferenceEvent(pCall, pidCall1->GetCallID(), pidCall2->GetCallID());
		}
		break;

		// Call Transferred
		case CEventBlock::CallTransfer:
		{
			const CPECallID* pidOldCallId = dynamic_cast<const CPECallID*>(pBlock->GetElement(CPBXElement::OldCallID));
			const CPEExtension* pTarget = dynamic_cast<const CPEExtension*>(pBlock->GetElement(CPBXElement::TargetExtension));
			const CPECallInfo* peCaller = dynamic_cast<const CPECallInfo*>(pBlock->GetElement(CPBXElement::DNIS));
			const CPECallInfo* peCalled = dynamic_cast<const CPECallInfo*>(pBlock->GetElement(CPBXElement::ANI));
			OnTransferEvent(pCall, pidOldCallId->GetCallID(), pTarget->GetExtension(), peCaller, peCalled);
		}
		break;

		// Unknown or unhandled
		default:
			break;
	}

	// It doesn't really matter what we return to this function since we are letting
	// the library handle the request management directly. The caller doesn't look
	// at the return code for unsolicited handlers. If we were managing the
	// ReceiveData() ourselves then this could be used to continue looking for
	// an owner for a data event.
	return true;

}// CJTLine::UnsolicitedEvent

/*****************************************************************************
** Procedure:  CJTPhone::UnsolicitedEvent
**
** Arguments:  'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function is called when no request processed a given
**               response from the device and it was directed at this phone.
**
*****************************************************************************/
bool CJTPhone::UnsolicitedEvent(LPCVOID lpBuff)
{
	// Cast our pointer back to an event block
	const CEventBlock* pBlock = static_cast<const CEventBlock*>(lpBuff);

	// Manage the phone event we have received.
	switch (pBlock->GetEventType())
	{
		// Display changed
		case CEventBlock::DisplayChanged:
		{
			// Get the new display string and set it into our device.
			const CPEDisplay* pDisplay = dynamic_cast<const CPEDisplay*>(pBlock->GetElement(CPBXElement::Display));
			if (pDisplay != NULL)
				SetDisplay(pDisplay->GetDisplay().c_str());
		}
		break;

		// Volume changed
		case CEventBlock::VolumeChanged:
		{
			const CPEVolume* pVolume = dynamic_cast<const CPEVolume*>(pBlock->GetElement(CPBXElement::Volume));
			if (pVolume != NULL)
				SetVolume(PHONEHOOKSWITCHDEV_SPEAKER, pVolume->GetVolume());
		}
		break;

		// Gain changed
		case CEventBlock::GainChanged:
		{
			const CPEGain* pGain = dynamic_cast<const CPEGain*>(pBlock->GetElement(CPBXElement::Gain));
			if (pGain != NULL)
				SetGain(PHONEHOOKSWITCHDEV_SPEAKER, pGain->GetGain());
		}
		break;

		// Hookswitch changed
		case CEventBlock::HookswitchChanged:
		{
			const CPEHookswitch* pHS = dynamic_cast<const CPEHookswitch*>(pBlock->GetElement(CPBXElement::Hookswitch));
			if (pHS != NULL)
			{
				SetHookSwitch(PHONEHOOKSWITCHDEV_HEADSET, 
					(pHS->GetHookswitchState() == CPEHookswitch::OnHook)  ? 
					PHONEHOOKSWITCHMODE_ONHOOK : 
					(pHS->GetHookswitchState() == CPEHookswitch::OffHook) ? 
					PHONEHOOKSWITCHMODE_MICSPEAKER : PHONEHOOKSWITCHMODE_SPEAKER);
			}
		}
		break;

		// Lamp changed
		case CEventBlock::LampChanged:
		{
			const CPELampInfo* pLamp = dynamic_cast<const CPELampInfo*>(pBlock->GetElement(CPBXElement::Lamp));
			if (pLamp != NULL)
			{
				_TSP_ASSERTE(pLamp->GetLampID() == 0 || pLamp->GetLampID() == 1);
				// Note we could search for the button we want, but since we know the
				// button layout of the phone we will just use it directly.
				// If this were a production TSP it would probably keep a mapping of
				// button id to lamps from the PBX and use that to lookup the proper
				// button.
				int iButtonID = (pLamp->GetLampID() == 0) ? 21 : 22;
				SetLampState(iButtonID, (pLamp->IsLit()) ? PHONELAMPMODE_STEADY : PHONELAMPMODE_OFF);
			}
		}
		break;

		// Unknown or unhandled
		default:
			break;
	}

	// It doesn't really matter what we return to this function since we are letting
	// the library handle the request management directly. The caller doesn't look
	// at the return code for unsolicited handlers. If we were managing the
	// ReceiveData() ourselves then this could be used to continue looking for
	// an owner for a data event.
	return true;

}// CJTPhone::UnsolicitedEvent
