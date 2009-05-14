/******************************************************************************/
//                                                                        
// CALL.CPP - Source code for the CTSPICallAppearance object.
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code for managing the call appearance       
// objects which are controlled by the CTSPIAddressInfo object.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#ifndef _DEBUG
	// std::vector.at() generates unreachable code warnings.
	#pragma warning(disable : 4702)
#endif

///////////////////////////////////////////////////////////////////////////
// timer_event_srch
//
// Private searching functor which looks for specific timer event type
//
struct timer_event_srch : public std::binary_function<TIMEREVENT*,int,bool>
{
	result_type operator()(first_argument_type te, second_argument_type nType) const {
		return te->iEventType == nType;
	}
};

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CTSPICallAppearance
//
// Default Constructor
//
CTSPICallAppearance::CTSPICallAppearance() : CTSPIBaseObject(),
	m_pAddr(0), m_htCall(0), m_iCallType(Normal),
	m_lpMediaControl(0), m_pConsult(0), m_pConf(0),
	m_dwFlags(_IsRealCall), m_pCallHub(0), m_lDeleteTime(0)
{
    ZeroMemory(&m_CallInfo, sizeof(LINECALLINFO));
    ZeroMemory(&m_CallStatus, sizeof(LINECALLSTATUS));

}// CTSPICallAppearance::CTSPICallAppearance

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::~CTSPICallAppearance
//
// Destructor
//
CTSPICallAppearance::~CTSPICallAppearance()
{
	// Remove ourselves from the call hub.
	if (m_pCallHub != NULL)
		m_pCallHub->RemoveFromHub(this);
	m_pCallHub = NULL;

	// Delete our tone monitor list.
	DeleteToneMonitorList();
	
	// Decrement our media control usage.
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->DecUsage();

}// CTSPICallAppearance::~CTSPICallAppearance

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Init
//
// Initialize the call appearance from our address owner.
//
void CTSPICallAppearance::Init (CTSPIAddressInfo* pAddr, DWORD dwBearerMode,
                                DWORD dwRate, DWORD dwCallParamFlags, DWORD dwOrigin, 
                                DWORD dwReason, DWORD dwTrunk, DWORD dwCompletionID)
{
	_TSP_ASSERTE(m_htCall == NULL);  // Happens as second-phase init

	// Set our owner and TAPI call handle.
    m_pAddr = pAddr;

	// Fill in the LINECALLINFO structure with known values.
    m_CallInfo.dwNeededSize = sizeof(LINECALLINFO);
    m_CallInfo.dwLineDeviceID = GetLineOwner()->GetDeviceID();
    m_CallInfo.dwAddressID = GetAddressOwner()->GetAddressID();
    m_CallInfo.dwBearerMode = dwBearerMode;
    m_CallInfo.dwRate = dwRate;
    m_CallInfo.dwMediaMode = 0;     // Will be set by 1st SetCallState
    m_CallInfo.dwCallID = 0;        // This may be used by the derived service provider.
    m_CallInfo.dwRelatedCallID = 0; // This may be used by the derived service provider.
    
    // Available Call states
    m_CallInfo.dwCallStates = 
		(LINECALLSTATE_IDLE | 
		 LINECALLSTATE_CONNECTED | 
		 LINECALLSTATE_UNKNOWN |
         LINECALLSTATE_PROCEEDING | 
		 LINECALLSTATE_DISCONNECTED | 
         LINECALLSTATE_BUSY);

    if (GetAddressOwner()->CanMakeCalls() && GetSP()->CanHandleRequest(TSPI_LINEMAKECALL))
        m_CallInfo.dwCallStates |= 
			(LINECALLSTATE_DIALING | 
			 LINECALLSTATE_DIALTONE | 
			 LINECALLSTATE_RINGBACK);

    if (GetAddressOwner()->CanAnswerCalls() && GetSP()->CanHandleRequest(TSPI_LINEANSWER))
        m_CallInfo.dwCallStates |= 
			(LINECALLSTATE_OFFERING | 
			 LINECALLSTATE_ACCEPTED);

    if (GetSP()->CanHandleRequest(TSPI_LINEADDTOCONFERENCE))
        m_CallInfo.dwCallStates |= 
			(LINECALLSTATE_CONFERENCED | 
			 LINECALLSTATE_ONHOLDPENDCONF);

    if (GetSP()->CanHandleRequest(TSPI_LINECOMPLETETRANSFER))
        m_CallInfo.dwCallStates |= LINECALLSTATE_ONHOLDPENDTRANSFER;

    // Origin/reason codes
    m_CallInfo.dwOrigin = dwOrigin;
    m_CallInfo.dwReason = dwReason;
    m_CallInfo.dwTrunk  = dwTrunk;
    m_CallInfo.dwCompletionID = dwCompletionID;     
    m_CallInfo.dwCallParamFlags = dwCallParamFlags;

    // Caller id information (unknown right now).
    m_CallInfo.dwCallerIDFlags = 
    m_CallInfo.dwCalledIDFlags = 
    m_CallInfo.dwConnectedIDFlags = 
    m_CallInfo.dwRedirectingIDFlags =
    m_CallInfo.dwRedirectionIDFlags = LINECALLPARTYID_UNKNOWN;

    // Initialize the call status to unknown
    m_CallStatus.dwNeededSize = sizeof(LINECALLSTATUS);
    m_CallStatus.dwCallState = LINECALLSTATE_UNKNOWN;

	// Pull the address types from the address owner. You should change
	// these values in your overridden Init function if they are 
	// different from the address owner's version.
	m_CallInfo.dwCallerIDAddressType = 
	m_CallInfo.dwCalledIDAddressType = 
	m_CallInfo.dwConnectedIDAddressType =
	m_CallInfo.dwRedirectionIDAddressType =
	m_CallInfo.dwRedirectingIDAddressType = GetAddressOwner()->GetAddressType();

	// Grab the terminal information from our address parent.
	for (int i = 0; i < GetLineOwner()->GetTerminalCount(); i++)
		m_arrTerminals.push_back(m_pAddr->GetTerminalInformation(i));

}// CTSPICallAppearance::Init

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::DestroyObject
//
// This is called when all references against this call are zero.
//
void CTSPICallAppearance::DestroyObject()
{
	_TSP_ASSERTE(*((LPDWORD)this) != 0x00000000);	// Unintialized ptr
	_TSP_ASSERTE(*((LPDWORD)this) != 0xdddddddd);	// Already free'd ptr
	_TSP_ASSERTE(GetRefCount() < 0);
	GetSP()->DeleteCall(this);

}// CTSPICallAppearance::DestroyObject

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CopyCall
//
// API to copy the information from one call to another
//
void CTSPICallAppearance::CopyCall(CTSPICallAppearance* pCall, bool fShadowCall, bool fReplaceCallerID)
{
	// Lock the call we are copying _from_ so that it's information doesn't 
	// change underneath us.
	CEnterCode lckCopy(pCall);

	// Copy the call information (LINECALLINFO)
	m_CallInfo.dwBearerMode		= pCall->m_CallInfo.dwBearerMode;
	m_CallInfo.dwRate			= pCall->m_CallInfo.dwRate;
	m_CallInfo.dwCallParamFlags = pCall->m_CallInfo.dwCallParamFlags;
	m_CallInfo.dwTrunk			= pCall->m_CallInfo.dwTrunk;
	m_CallInfo.dwCallerIDAddressType = pCall->m_CallInfo.dwCallerIDAddressType;
	m_CallInfo.dwCalledIDAddressType = pCall->m_CallInfo.dwCalledIDAddressType;
	m_CallInfo.dwConnectedIDAddressType = pCall->m_CallInfo.dwConnectedIDAddressType;
	m_CallInfo.dwRedirectionIDAddressType = pCall->m_CallInfo.dwRedirectionIDAddressType;
	m_CallInfo.dwRedirectingIDAddressType = pCall->m_CallInfo.dwRedirectingIDAddressType;

	// Only replace callerid if the caller wants to (v3.0b)
	if (fReplaceCallerID)
	{
		m_CallInfo.dwCallerIDFlags	= pCall->m_CallInfo.dwCallerIDFlags;
		m_CallInfo.dwCalledIDFlags	= pCall->m_CallInfo.dwCalledIDFlags;
		m_CallerID = pCall->m_CallerID;
		m_CalledID = pCall->m_CalledID;
	}

	// Copy the USER-USER information
	for (TUserInfoArray::iterator theIterator = pCall->m_arrUserUserInfo.begin();
		 theIterator != pCall->m_arrUserUserInfo.end(); theIterator++)
		OnReceivedUserUserInfo((*theIterator)->GetPtr(), (*theIterator)->GetSize());

	// Copy the call data and QOS information from our source call.
	m_sdCallData = pCall->m_sdCallData;
	m_sdSendingFS = pCall->m_sdSendingFS;
	m_sdReceivingFS = pCall->m_sdReceivingFS;

	// Copy the charging information and compatibility information (v3.0b)
	m_sdChargingInfo = pCall->m_sdChargingInfo;
	m_sdLowLevelInfo = pCall->m_sdLowLevelInfo;
	m_sdHiLevelInfo = pCall->m_sdHiLevelInfo;

	// Save off information from the call before we unlock it.
	DWORD dwCallID = pCall->m_CallInfo.dwCallID;
	DWORD dwConnectedIDFlags = pCall->m_CallInfo.dwConnectedIDFlags;
	CALLIDENTIFIER ConnectedID = pCall->m_ConnectedID;
	TString strAddressOwner = pCall->GetAddressOwner()->GetDialableAddress();
	TString strAddressName = pCall->GetAddressOwner()->GetName();

	// Unlock the input call since we are about to lock ourselves.  This
	// is to prevent a deadlock against two calls racing to lock each other.
	lckCopy.Unlock();

	// If we are keeping the same call id then leave the other call
	// in this hub.  Otherwise this is NOT a shadow call but is instead 
	// REPLACING the original call.  Change the "other" call from the 
	// original to our new call.
	if (!fShadowCall)
	{
		_TSP_DTRACEX(TRC_CALLS, _T("Replacing call 0x%lx (CallID=0x%lx) with call 0x%lx\n"), pCall, m_CallInfo.dwCallID, this);

		// Copy the connected id from the original call (v3.0b)
		m_CallInfo.dwConnectedIDFlags = dwConnectedIDFlags; 
		m_ConnectedID = ConnectedID;

		// Set the previous owner if the service provider hasn't set anything yet.
		if (m_CallInfo.dwRedirectingIDFlags == LINECALLPARTYID_UNKNOWN)
			SetRedirectingIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS, 
									strAddressOwner.c_str(), strAddressName.c_str());

		// Set the redirection information into this call. This is the address
		// that this call *was* connected to before being moved.
		if (m_CallInfo.dwRedirectionIDFlags == LINECALLPARTYID_UNKNOWN)
			SetRedirectionIDInformation(LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS, 
									strAddressOwner.c_str(), strAddressName.c_str());
	}

	// Set our new call id - this will adjust our call hub pointer
	SetCallID(dwCallID);

	// If we are replacing the call then zero out it's call-id.
	if (!fShadowCall)
	{
		// Lock the hub while we are getting information -- we must do this
		// before the call object so that other threads informing the hub about
		// a call-state change don't get dead-locked.
		CEnterCode sHub(GetCallHub());

		// Remove the callid from the passed call. This will zero out the call hub
		// pointer for the copied call since there isn't a map entry for zero.
		pCall->SetCallID(0);

		// Lock _this_ call pointer while we play with the hub
		CEnterCode lckMe(this);

		// If there is only a single entry in the call hub map then set the
		// redirecting information for this call as well.
		if (GetCallHub() && GetCallHub()->GetHubCount() <= 2)
		{
			CTSPICallAppearance* pShadow = GetShadowCall();
			if (pShadow != NULL)
			{
				// Copy the redirecting information from the call being redirected.
				if (pShadow->m_CallInfo.dwRedirectingIDFlags == LINECALLPARTYID_UNKNOWN)
					pShadow->m_RedirectingID = m_RedirectingID;

				// Set the address this call *was* connected to.
				if (pShadow->m_CallInfo.dwRedirectionIDFlags == LINECALLPARTYID_UNKNOWN)
					SetRedirectionIDInformation(LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS, 
									strAddressOwner.c_str(), strAddressName.c_str());
			}
		}
	}

	// Recalculate our features since some of our data has changed. (v3.0b)
	RecalcCallFeatures();

	// Send out the notification for this call that the calldata and QOS 
	// has been changed.
	OnCallInfoChange (LINECALLINFOSTATE_QOS | LINECALLINFOSTATE_CALLDATA);

}// CTSPICallAppearance::CopyCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnRequestComplete
//
// This method is called by the request object when an outstanding
// request completes.
//
void CTSPICallAppearance::OnRequestComplete (CTSPIRequest* pReq, LONG lResult)
{   
	// If the request failed, ignore it.
	if (lResult != 0)
		return;

	// Determine what to do based on the command id - this is faster than using
	// the down-cast pointer and can be optimized by the compiler much easier.
	switch (pReq->GetCommand())
	{
		// If this is a request to set the caller parameters, then set
		// the various fields of the CALL STATUS structure and notify TAPI.
		case REQUEST_SETCALLPARAMS:
		{
			RTSetCallParams* pCallParams = dynamic_cast<RTSetCallParams*>(pReq);
			SetBearerMode(pCallParams->GetBearerMode());
			SetDialParameters(pCallParams->GetDialParams());
			break;
		} 
    
		// If this was a request to set the call treatment for unanswered calls,
		// and it completed ok, adjust our LINECALLINFO record
		case REQUEST_SETCALLTREATMENT:
			SetCallTreatment(dynamic_cast<RTSetCallTreatment*>(pReq)->GetCallTreatment());
			break;

		// If this is an ACCEPT request and we are still in the OFFERING state,
		// move over to accepted automatically
		case REQUEST_ACCEPT:
		{
			if (GetCallState() == LINECALLSTATE_OFFERING &&
				(GetAddressOwner()->GetAddressCaps()->dwCallStates & LINECALLSTATE_ACCEPTED) != 0)
				SetCallState(LINECALLSTATE_ACCEPTED);
			break;
		}

		// If this is a request to change the terminal information for the
		// call, and it completed ok, then reset our internal terminal array.
		case REQUEST_SETTERMINAL:
		{
			RTSetTerminal* pTermStruct = dynamic_cast<RTSetTerminal*>(pReq);
			if (pTermStruct->GetCall() != NULL)
			{
				SetTerminalModes (pTermStruct->GetTerminalID(), pTermStruct->GetTerminalModes(), 
							pTermStruct->Enable());
			}
			break;
		}

		// If this is a drop request, then work with the pending request list.
		case REQUEST_DROPCALL:
		{
			// Remove all pending requests for the call.
			// This can be called to close a call handle which has not been 
			// completely setup (ie: the original asynch request which created the
			// call hasn't completely finished).  In this case, we need to return
			// LINEERR_OPERATIONFAILED for each pending request.
			GetLineOwner()->RemovePendingRequests (this, REQUEST_ALL, LINEERR_OPERATIONFAILED, false, pReq);

			// If the service provider is choosing to ignore the drop request (and instead
			// is simply letting TAPI close the line/call but leaving the call active,
			// then UNMARK the drop flag so if we ever tell TAPI about the call again we
			// will have the ability to drop the call.
			RTDropCall* pDrop = dynamic_cast<RTDropCall*>(pReq);
			if (pDrop->IsIgnoringDrop())
				m_dwFlags &= ~_IsDropped;
			break;
		}

		// If this is a request to secure the call, and it completed ok, then
		// set the status bits.
		case REQUEST_SECURECALL:
			SetCallParameterFlags (m_CallInfo.dwCallParamFlags | LINECALLPARAMFLAGS_SECURE);
			break;

		// If this is a request to swap hold with another call appearance, and the
		// other call appearance (or this one) is a consultant call created for conferencing,
		// then swap the call types.  Note we don't want to do this for conference calls!
		case REQUEST_SWAPHOLD:
		{
			RTSwapHold* pSwapHold = dynamic_cast<RTSwapHold*>(pReq);
			CTSPICallAppearance* pCall = pSwapHold->GetActiveCall();
			CTSPICallAppearance* pHeldCall = pSwapHold->GetHoldingCall();
			if (pCall->GetAttachedCall() == pHeldCall &&
				pCall->GetCallType() != Conference &&
				pHeldCall->GetCallType() != Conference)
			{
				int iType = pHeldCall->GetCallType();
				pHeldCall->SetCallType(pCall->GetCallType());
				pCall->SetCallType(iType);
			}
			break;
		}
    
		// If this is a GenerateDigit request then complete it with TAPI.
		case REQUEST_GENERATEDIGITS:
		{   
			CTSPILineConnection* pLine = GetLineOwner();
			if (pLine != NULL && GetCallHandle() != NULL)
			{
				RTGenerateDigits* pInfo = dynamic_cast<RTGenerateDigits*>(pReq);
				pLine->Send_TAPI_Event(this, LINE_GENERATE, LINEGENERATETERM_DONE,
							   pInfo->GetIdentifier(), GetTickCount());
			}
			break;
		}

		// If this is a GenerateTone request then complete it with TAPI.
		case REQUEST_GENERATETONE:
		{   
			CTSPILineConnection* pLine = GetLineOwner();
			if (pLine != NULL && GetCallHandle() != NULL)
			{
				RTGenerateTone* pInfo = dynamic_cast<RTGenerateTone*>(pReq);
				pLine->Send_TAPI_Event(this, LINE_GENERATE, LINEGENERATETERM_DONE,
							   pInfo->GetIdentifier(), GetTickCount());
			}
			break;
		}

		// If this is a SetCallData request which has completed o.k.,
		// then really set the call data into the CALLINFO record.
		case REQUEST_SETCALLDATA:
		{
			RTSetCallData* pCallData = dynamic_cast<RTSetCallData*>(pReq);
			SetCallData(pCallData->GetData(), pCallData->GetSize());
			break;
		}

		// Or if this is a request to set the quality of service which
		// was successful, then update the internal call record
		// FLOWSPEC information.
		case REQUEST_SETQOS:
		{
			RTSetQualityOfService* pQOS = dynamic_cast<RTSetQualityOfService*>(pReq);
			SetQualityOfService (pQOS->GetSendingFlowSpec(), pQOS->GetSendingFlowSpecSize(),
								 pQOS->GetReceivingFlowSpec(), pQOS->GetReceivingFlowSpecSize());
			break;
		}

		// If this is a lineSetMediaControl event, then store the new MediaControl
		// information into the call.
		case REQUEST_MEDIACONTROL:
			SetMediaControl(dynamic_cast<RTSetMediaControl*>(pReq)->GetMediaControlInfo());
			break;

		// Or if this is a lineReleaseUserUserInfo, then delete any 
		// remanents in our callinfo structure.
		case REQUEST_RELEASEUSERINFO:
			ReleaseUserUserInfo();
			break;

		// Default handler
		default:
			break;
	}

}// CTSPICallAppearance::OnRequestComplete

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnCallInfoChange
//
// This method is called when any of the information in our call 
// information record has changed.  It informs TAPI through the
// asynch. callback function in our line.
//
void CTSPICallAppearance::OnCallInfoChange (DWORD dwCallInfo)
{
	GetLineOwner()->Send_TAPI_Event(this, LINE_CALLINFO, dwCallInfo);

}// CTSPICallAppearance::OnCallInfoChange

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnCallStatusChange
//
// This method is called when the call state information record 
// changes.  It informs TAPI through the asynch. callback function in
// our line owner.
//
void CTSPICallAppearance::OnCallStatusChange (DWORD dwCallState, DWORD /*dwCallInfo*/, DWORD /*dwMediaModes*/)
{
	// If we are doing media monitoring, then check our list.  Do this before
	// the control lists get deleted due to an IDLE condition.
	if (m_lpMediaControl != NULL && dwCallState != m_CallStatus.dwCallState)
	{
		CEnterCode sLock(this, FALSE);
		if (sLock.Lock(0))
		{
			for (unsigned int i = 0; i < m_lpMediaControl->arrCallStates.size(); i++)
			{
				LPLINEMEDIACONTROLCALLSTATE lpCallState = m_lpMediaControl->arrCallStates[i];
				if ((lpCallState->dwCallStates & dwCallState) == dwCallState)
					OnMediaControl (lpCallState->dwMediaControl);
			}
		}
	}

	// Mark the global call features we support right now.
	DWORD dwCallFeatures = 0L;
	CTSPIAddressInfo* pAddr = GetAddressOwner();
	LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();

	// If the parent address is being monitored, then don't bother to determine
	// call features since this is a call which is not reported to TAPI anyway.
	// Go ahead and run through on IDLE just to cleanup any memory being held by the
	// call here.
	if (lpAddrCaps->dwAddressSharing != LINEADDRESSSHARING_MONITORED ||
		dwCallState == LINECALLSTATE_IDLE)
	{
		// Get the call features which are available at most any time.    
		dwCallFeatures |= (LINECALLFEATURE_SECURECALL |
	    				   LINECALLFEATURE_SETCALLPARAMS |
	    				   LINECALLFEATURE_SETMEDIACONTROL |
	    				   LINECALLFEATURE_MONITORDIGITS |
	    				   LINECALLFEATURE_MONITORMEDIA |
						   LINECALLFEATURE_MONITORTONES |
						   LINECALLFEATURE_SETTREATMENT |
						   LINECALLFEATURE_SETQOS |
						   LINECALLFEATURE_SETCALLDATA |
						   LINECALLFEATURE_DROP);

		// Add RELEASE user information if we have some,
		if (!m_arrUserUserInfo.empty())
			dwCallFeatures |= LINECALLFEATURE_RELEASEUSERUSERINFO;
		
		// Add terminal support
	  	if (!m_arrTerminals.empty())
			dwCallFeatures |= LINECALLFEATURE_SETTERMINAL;

		// Add features available when call is active.
		if (IsConnectedCallState(dwCallState))
		{
	    	// Add dialing capabilities
	    	if (pAddr->CanMakeCalls() &&
	        	(lpAddrCaps->dwAddrCapFlags & LINEADDRCAPFLAGS_DIALED))
	         	dwCallFeatures |= LINECALLFEATURE_DIAL;
	     
			dwCallFeatures |= (LINECALLFEATURE_GATHERDIGITS |
	        					 LINECALLFEATURE_GENERATEDIGITS |
	        					 LINECALLFEATURE_GENERATETONE);
		}

		// Now look specifically at the call state and determine what
		// we can do with the call.  This logic is taken directly from the 
		// TAPI and TSPI specification.
		switch(dwCallState)
		{
			case LINECALLSTATE_UNKNOWN:
				dwCallFeatures = 0L;
				break;

			case LINECALLSTATE_IDLE:
				dwCallFeatures = 0L;
				m_CallInfo.dwMonitorDigitModes = 0L;
				m_CallInfo.dwMonitorMediaModes = 0L;
				DeleteToneMonitorList();
				delete m_lpGather.release(); // delete the pointer, reset to 0.
				if (m_lpMediaControl != NULL)
				{
					m_lpMediaControl->DecUsage();
					m_lpMediaControl = NULL;
				}

				// Remove all requests which have not started (they are still in the "INITIAL" state).
				GetLineOwner()->RemovePendingRequests(this, REQUEST_ALL, LINEERR_INVALCALLSTATE, true);
				break;

			case LINECALLSTATE_DISCONNECTED:
				dwCallFeatures = LINECALLFEATURE_DROP;
				break;

			case LINECALLSTATE_ACCEPTED:
	        	dwCallFeatures |= LINECALLFEATURE_REDIRECT;
				if (pAddr->CanAnswerCalls())
					dwCallFeatures |= LINECALLFEATURE_ANSWER;
				break;

			case LINECALLSTATE_OFFERING:
	        	dwCallFeatures |= (LINECALLFEATURE_REDIRECT | LINECALLFEATURE_ACCEPT);
				if (pAddr->CanAnswerCalls())
					dwCallFeatures |= LINECALLFEATURE_ANSWER;
				break;

			case LINECALLSTATE_CONFERENCED:
			{
				CTSPIConferenceCall* pConfCall = GetConferenceOwner();
				if (pConfCall && pConfCall->CanRemoveFromConference(this))
					dwCallFeatures |= LINECALLFEATURE_REMOVEFROMCONF;
			}
			break;

			case LINECALLSTATE_DIALTONE:
			case LINECALLSTATE_DIALING:
				if (IsRealCall() && GetLineOwner()->FindCallByState(
					LINECALLSTATE_ONHOLD | 
					LINECALLSTATE_ONHOLDPENDTRANSFER | 
					LINECALLSTATE_ONHOLDPENDCONF))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				break;

			case LINECALLSTATE_ONHOLD:
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_CONNECTED | 
					LINECALLSTATE_DIALTONE | 
					LINECALLSTATE_PROCEEDING | 
					LINECALLSTATE_DIALING | 
					LINECALLSTATE_RINGBACK |
					LINECALLSTATE_BUSY))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				if ((lpAddrCaps->dwAddrCapFlags & LINEADDRCAPFLAGS_CONFERENCEHELD) &&
					 GetLineOwner()->IsConferenceAvailable(this))
					dwCallFeatures |= LINECALLFEATURE_ADDTOCONF;
				if (GetLineOwner()->IsTransferConsultAvailable(this) &&
					(lpAddrCaps->dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERHELD))
					dwCallFeatures |= LINECALLFEATURE_COMPLETETRANSF;
				dwCallFeatures |= LINECALLFEATURE_UNHOLD;
				break;

			case LINECALLSTATE_ONHOLDPENDCONF:
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_CONNECTED | 
					LINECALLSTATE_DIALTONE | 
					LINECALLSTATE_PROCEEDING | 
					LINECALLSTATE_DIALING | 
					LINECALLSTATE_RINGBACK |
					LINECALLSTATE_BUSY))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				//dwCallFeatures |= (LINECALLFEATURE_UNHOLD | LINECALLFEATURE_COMPLETETRANSF);
				dwCallFeatures |= LINECALLFEATURE_UNHOLD;
				break;

			case LINECALLSTATE_ONHOLDPENDTRANSFER:
				if (GetLineOwner()->IsTransferConsultAvailable(this))
					dwCallFeatures |= LINECALLFEATURE_COMPLETETRANSF;
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_CONNECTED | 
					LINECALLSTATE_DIALTONE | 
					LINECALLSTATE_PROCEEDING | 
					LINECALLSTATE_DIALING | 
					LINECALLSTATE_RINGBACK |
					LINECALLSTATE_BUSY))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				dwCallFeatures |= LINECALLFEATURE_UNHOLD;
				break;

			case LINECALLSTATE_RINGBACK:
				if (GetLineOwner()->IsConferenceAvailable(this))
					dwCallFeatures |= LINECALLFEATURE_ADDTOCONF;                
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_ONHOLD | 
					LINECALLSTATE_ONHOLDPENDTRANSFER | 
					LINECALLSTATE_ONHOLDPENDCONF))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				dwCallFeatures |= LINECALLFEATURE_COMPLETECALL;
				break;

			case LINECALLSTATE_BUSY:
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_ONHOLD | 
					LINECALLSTATE_ONHOLDPENDTRANSFER | 
					LINECALLSTATE_ONHOLDPENDCONF))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				dwCallFeatures |= LINECALLFEATURE_COMPLETECALL;
				break;

			case LINECALLSTATE_PROCEEDING:
				if (GetLineOwner()->IsConferenceAvailable(this))
					dwCallFeatures |= LINECALLFEATURE_ADDTOCONF;
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_ONHOLD | 
					LINECALLSTATE_ONHOLDPENDTRANSFER | 
					LINECALLSTATE_ONHOLDPENDCONF))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				dwCallFeatures |= (
					LINECALLFEATURE_BLINDTRANSFER |
					LINECALLFEATURE_HOLD);
				break;

			case LINECALLSTATE_CONNECTED:
				if (GetCallType() != Conference)
					dwCallFeatures |= LINECALLFEATURE_SETUPCONF;
				if ((pAddr->GetAddressStatus()->dwNumOnHoldCalls <
					pAddr->GetAddressCaps()->dwMaxNumOnHoldCalls) &&
					(pAddr->GetAddressStatus()->dwNumOnHoldPendCalls <
					pAddr->GetAddressCaps()->dwMaxNumOnHoldPendingCalls))
					dwCallFeatures |= LINECALLFEATURE_SETUPTRANSFER;
				dwCallFeatures |= LINECALLFEATURE_PARK;
				if (GetLineOwner()->IsConferenceAvailable(this) &&
					GetCallType() != Conference &&
					GetConferenceOwner() == NULL)
					dwCallFeatures |= LINECALLFEATURE_ADDTOCONF;
				if (GetCallType() == Conference)
					dwCallFeatures |= LINECALLFEATURE_PREPAREADDCONF;
				if (GetLineOwner()->FindCallByState(
					LINECALLSTATE_ONHOLD | 
					LINECALLSTATE_ONHOLDPENDTRANSFER | 
					LINECALLSTATE_ONHOLDPENDCONF))
					dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
				if (GetLineOwner()->FindCallByState(LINECALLSTATE_ONHOLDPENDTRANSFER) ||
					(GetLineOwner()->FindCallByState(LINECALLSTATE_ONHOLD) && 
						(lpAddrCaps->dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERHELD)))
					dwCallFeatures |= LINECALLFEATURE_COMPLETETRANSF;
				dwCallFeatures |= (
					LINECALLFEATURE_BLINDTRANSFER |
					LINECALLFEATURE_HOLD |
					LINECALLFEATURE_SENDUSERUSER);

				// If we don't have connectd ID information, set it based on the origin.
				if (GetCallInfo()->dwConnectedIDFlags & (LINECALLPARTYID_UNKNOWN | LINECALLPARTYID_UNAVAIL) ||
					GetCallInfo()->dwConnectedIDFlags == 0)
				{
					// When the call reason is TRANSFER, we can not accurately determine
					// what the call origin really was for the transferred call. This means
					// that we cannot really determine who we are connected to - we have to
					// let the derived service provider do that for us so leave it blank on
					// a transfer. (v3.0b)
					if (GetCallInfo()->dwReason != LINECALLREASON_TRANSFER)
						SetConnectedIDInformation(
							(GetCallInfo()->dwOrigin & LINECALLORIGIN_OUTBOUND) ? 
							 GetCalledIDInformation() : GetCallerIDInformation());
				}
				// We are now connected - send the event so that apps know the connected
				// ID information is present now.
				else
					OnCallInfoChange(LINECALLINFOSTATE_CONNECTEDID);
				break;

			default:
				break;
		}

		// Pull out all the feature bits which are not supported
		// by the address owner.
		dwCallFeatures &= lpAddrCaps->dwCallFeatures;

		// If this is a consultation call created for a transfer/conference
		// event then exclude the creation of another consultation based on this one.
		if (GetCallType() == CTSPICallAppearance::Consultant)
			dwCallFeatures &= ~(LINECALLFEATURE_SETUPTRANSFER | LINECALLFEATURE_SETUPCONF);

		// If the call is currently in PASS-THROUGH mode then remove all features which
		// are related to call control
		if (GetCallInfo()->dwBearerMode == LINEBEARERMODE_PASSTHROUGH)
			dwCallFeatures &= ~(LINECALLFEATURE_ACCEPT |
						LINECALLFEATURE_ADDTOCONF |
						LINECALLFEATURE_ANSWER |
						LINECALLFEATURE_BLINDTRANSFER |
						LINECALLFEATURE_COMPLETECALL |
						LINECALLFEATURE_COMPLETETRANSF |
						LINECALLFEATURE_DIAL |
						LINECALLFEATURE_DROP |
						LINECALLFEATURE_GATHERDIGITS |
						LINECALLFEATURE_GENERATEDIGITS |
						LINECALLFEATURE_GENERATETONE |
						LINECALLFEATURE_HOLD |
						LINECALLFEATURE_MONITORDIGITS |
						LINECALLFEATURE_MONITORMEDIA |
						LINECALLFEATURE_MONITORTONES |
						LINECALLFEATURE_PARK |
						LINECALLFEATURE_PREPAREADDCONF |
						LINECALLFEATURE_REDIRECT |
						LINECALLFEATURE_REMOVEFROMCONF |
						LINECALLFEATURE_SECURECALL |
						LINECALLFEATURE_SENDUSERUSER |
						LINECALLFEATURE_SETMEDIACONTROL |
						LINECALLFEATURE_SETUPCONF |
						LINECALLFEATURE_SETUPTRANSFER |
						LINECALLFEATURE_SWAPHOLD |
						LINECALLFEATURE_UNHOLD |
						LINECALLFEATURE_SETTREATMENT |
						LINECALLFEATURE_SETQOS);

		// Allow the derived service provider to adjust this list.
		dwCallFeatures = pAddr->OnCallFeaturesChanged(this, dwCallFeatures);
		SetCallFeatures(dwCallFeatures);
	}
	
	// If we have no TAPI handle anymore, then TAPI already discarded
	// this call - go ahead and remove it from our list when it goes IDLE.
	if (dwCallState == LINECALLSTATE_IDLE && m_htCall == NULL)
		GetAddressOwner()->RemoveCallAppearance(this);

}// CTSPICallAppearance::OnCallStatusChange

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallFeatures
//
// Set the current features of the call w/o callng OnCallFeaturesChanged.
//
void CTSPICallAppearance::SetCallFeatures (DWORD dwFeatures, bool fNotify /*=TRUE*/)
{
	// Make sure the features are in the ADDRESSCAPS.
	CTSPIAddressInfo* pAddr = GetAddressOwner();
	if (dwFeatures != 0 && (pAddr->GetAddressCaps()->dwCallFeatures & dwFeatures) != dwFeatures)
	{
		_TSP_DTRACE(_T("LINEADDRESSCAPS.dwCallFeatures doesn't have 0x%lx in it.\n"), dwFeatures);
		dwFeatures &= pAddr->GetAddressCaps()->dwCallFeatures;
	}

	if (dwFeatures != m_CallStatus.dwCallFeatures)
	{
		// Check to see if our "changing" state bit is set, if so, we
		// don't need to send a "state changed" message here.
		BOOL fTellTapi = ((m_dwFlags & _ChgState) == 0);

		CEnterCode sLock(this);
		m_CallStatus.dwCallFeatures = dwFeatures;
		sLock.Unlock();

		// Now adjust our CALLFEATURE2 field based on what our current features are.
		// We can't SET any of the bits here (not enouugh info) but we can take some away.
		SetCallFeatures2(m_CallStatus.dwCallFeatures2, fNotify);

		// Tell TAPI that they changed.
		if (fTellTapi && fNotify)
			NotifyCallStatusChanged();
	}

}// CTSPICallAppearance::SetCallFeatures

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallFeatures2
//
// Set the current secondary features of the call
//
void CTSPICallAppearance::SetCallFeatures2(DWORD dwCallFeatures2, bool fNotify /*=TRUE*/)
{
	if (GetAddressOwner()->GetAddressCaps()->dwCallFeatures2)
	{
		CEnterCode sLock(this);
		DWORD dwFeatures = m_CallStatus.dwCallFeatures;
		DWORD dwCurrFeatures = m_CallStatus.dwCallFeatures2;

		CTSPIAddressInfo* pAddr = GetAddressOwner();
		if (dwCallFeatures2 != 0 && ((pAddr->GetAddressCaps()->dwCallFeatures2 & dwCallFeatures2) != dwCallFeatures2))
		{
			_TSP_DTRACE(_T("LINEADDRESSCAPS.dwCallFeatures2 doesn't have 0x%lx in it.\n"), dwCallFeatures2);
			dwCallFeatures2 &= pAddr->GetAddressCaps()->dwCallFeatures;
		}

		// Remove any non-applicable bits based on our current call features.
		if ((dwFeatures & LINECALLFEATURE_SETUPCONF) == 0)
			dwCallFeatures2 &= ~LINECALLFEATURE2_NOHOLDCONFERENCE;
		if ((dwFeatures & LINECALLFEATURE_SETUPTRANSFER ) == 0)
			dwCallFeatures2 &= ~LINECALLFEATURE2_ONESTEPTRANSFER;
		if ((dwFeatures & LINECALLFEATURE_COMPLETECALL) == 0)
			dwCallFeatures2 &= ~(LINECALLFEATURE2_COMPLCAMPON | LINECALLFEATURE2_COMPLCALLBACK | LINECALLFEATURE2_COMPLINTRUDE | LINECALLFEATURE2_COMPLMESSAGE);
		if ((dwFeatures & LINECALLFEATURE_COMPLETETRANSF) == 0)
			dwCallFeatures2 &= ~(LINECALLFEATURE2_TRANSFERNORM | LINECALLFEATURE2_TRANSFERCONF);
		if ((dwFeatures & LINECALLFEATURE_PARK) == 0)
			dwCallFeatures2 &= ~(LINECALLFEATURE2_PARKDIRECT | LINECALLFEATURE2_PARKNONDIRECT);

		// Notify TAPI if the features changed...
		if (dwCurrFeatures != dwCallFeatures2)
		{
			m_CallStatus.dwCallFeatures2 = dwCallFeatures2;
			BOOL fTellTapi = ((m_dwFlags & _ChgState) == 0);
			if (fTellTapi && fNotify)
			{
				sLock.Unlock();
				NotifyCallStatusChanged();
			}
		}
	}

}// CTSPICallAppearance::SetCallFeatures2

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::RecalcCallFeatures
//
// Force the call to recalc its features.
//
void CTSPICallAppearance::RecalcCallFeatures(DWORD dwState/*=0*/)
{
	CEnterCode sLock(this, FALSE);
	if (sLock.Lock(0))
	{
		// If the line is out-of-service or disconnected, then
		// disallow call features to be used.
		if ((GetLineOwner()->GetLineDevStatus()->dwDevStatusFlags & 
				(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) != 
				(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED))
			SetCallFeatures(0);
		else
		{
			if (!dwState)
				dwState = m_CallStatus.dwCallState;
			OnCallStatusChange(dwState, m_CallStatus.dwCallStateMode, m_CallInfo.dwMediaMode);
		}
	}

}// CTSPICallAppearance::RecalcCallFeatures

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::DeleteToneMonitorList
//
// Delete the list of tones we are monitoring for.     
//
void CTSPICallAppearance::DeleteToneMonitorList()
{
	CEnterCode sLock(this);  // Synch access to object

	// Remove all the tones
	std::for_each(m_arrMonitorTones.begin(), m_arrMonitorTones.end(), tsplib::delete_object());
    m_arrMonitorTones.clear();

	// Delete any events for tones; we simply move them all into an auto-deleting pointer array.
	if (m_arrEvents.size() > 0)  
	{
		tsplib::ptr_vector<TIMEREVENT> arrTimers; arrTimers.reserve(m_arrEvents.size());
		m_arrEvents.erase(std::remove_copy_if(m_arrEvents.begin(), m_arrEvents.end(), arrTimers.begin(),
							std::bind2nd(timer_event_srch(), TIMEREVENT::ToneDetect)), m_arrEvents.end());
	}
                        
}// CTSPICallAppearance::DeleteToneMonitorList
            
////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Close
//
// This method is called to close the call appearance.  Once this is
// completed, the call appearance will be invalid (and deleted).
// This is called during lineCloseCall.
//
LONG CTSPICallAppearance::Close()
{   
	CTSPILineConnection* pLine = GetLineOwner();

	// Wait for any drop request to complete.  The call will be removed
	// from our call list when it goes IDLE.
	pLine->WaitForAllRequests (this, REQUEST_DROPCALL);

	// Remove our HTAPICALL handle reference since TAPI is no longer interested
	// in this call.
	m_htCall = 0;

	// Deallocate the call appearance from the address if it is IDLE.
	// Otherwise, we will wait until the call DOES go idle.
	if (GetCallState() == LINECALLSTATE_IDLE)
		GetAddressOwner()->RemoveCallAppearance(this);

    // Return success.
    return false;

}// CTSPICallAppearance::Close

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::DropOnClose
//
// This is called when the line is being closed to DROP the call.
//
void CTSPICallAppearance::DropOnClose()
{
	CEnterCode sLock(this);  // Synch access to object
	if ((m_dwFlags & _IsDropped) == 0 && GetCallState() != LINECALLSTATE_IDLE)
	{
		// Mark the call as dropped so we never allow a second drop.
		m_dwFlags |= _IsDropped;

		// Unlock the call object -- the request might actually get processed here
		// so we don't want to keep it locked.
		sLock.Unlock();

		// Submit the drop request - this will actually be a "specialized"
		// drop request marked so the provider can distinguish between calls 
		// being dropped by TAPI and calls being dropped due to the line being closed.
		AddAsynchRequest(new RTDropCall(this));
	}

}// CTSPICallAppearance::DropOnClose

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Drop
//
// Drop the call appearance and transition the call to IDLE.  The
// USERUSER info is local to our SP.  This is called by lineDrop.
//
LONG CTSPICallAppearance::Drop(DRV_REQUESTID dwRequestId, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	// Verify that the function can be called right now.
	if (dwRequestId > 0 && (GetCallStatus()->dwCallFeatures & LINECALLFEATURE_DROP) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Synchronize access to this call since it is about to change.
	CEnterCode sLock(this);

	// If the DROP flag is already set, the call is probably in the process of
	// being dropped - otherwise it would be in a state reflective of a dropped
	// call (Idle).
	if (m_dwFlags & _IsDropped)
		return 0; // Now return zero because Outlook2000 has a bug and performs several lineDrops.

#ifdef STRICT_CALLSTATES
    // Make sure the call state allows this call to be dropped.
    // According to the TAPI spec, any state except IDLE can
    // be dropped.
    if (GetCallState() == LINECALLSTATE_IDLE)
		return LINEERR_INVALCALLSTATE;
#endif

	// Mark the call as dropped so we never allow a second drop.
	m_dwFlags |= _IsDropped;

	// Copy the User-User information into another buffer.
	if (lpsUserUserInfo != NULL && dwSize > 0)
	{   
		// Validate the UserUser information against our CAPS.
		CTSPILineConnection* pLine = GetLineOwner();
		if (pLine->GetLineDevCaps()->dwUUIDropSize > 0)
		{
			// Validate the UserUser information.
			if (pLine->GetLineDevCaps()->dwUUIDropSize < dwSize)
			{
				m_dwFlags &= ~_IsDropped;
				return LINEERR_USERUSERINFOTOOBIG;
			}
		}
	}

	// Unlock the call object -- the request might actually get processed here
	// so we don't want to keep it locked.
	sLock.Unlock();

    // Submit the drop request
    if (!AddAsynchRequest(new RTDropCall(this, dwRequestId, lpsUserUserInfo, dwSize))) 
	{
		m_dwFlags &= ~_IsDropped;
		return LINEERR_OPERATIONFAILED;
	}

	return dwRequestId;
    
}// CTSPICallAppearance::Drop

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Accept
//
// Accepts ownership of the specified offered call. It may 
// optionally send the specified user-to-user information to the 
// calling party.  In some environments (such as ISDN), the station 
// will not begin to ring until the call is accepted.
//
LONG CTSPICallAppearance::Accept(DRV_REQUESTID dwRequestID, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_ACCEPT) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_ACCEPT) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure the call state allows this.
    if (GetCallState() != LINECALLSTATE_OFFERING)
        return LINEERR_INVALCALLSTATE;
#endif

    // Allocate a new buffer for the UserUserInfo.
    if (lpsUserUserInfo != NULL && dwSize > 0)
    {   
        // If we don't support this type of UserUserInfo, then simply
        // ignore it - many apps send it and don't expect a TOOBIG error.
        CTSPILineConnection* pLine = GetLineOwner();
        if (pLine->GetLineDevCaps()->dwUUIAcceptSize > 0)
        {
            // Validate the UserUser information.
            if (pLine->GetLineDevCaps()->dwUUIAcceptSize < dwSize)
                return LINEERR_USERUSERINFOTOOBIG;
        }
    }

    // Everything seems ok, submit the accept request.
    if (AddAsynchRequest(new RTAccept(this, dwRequestID, lpsUserUserInfo, dwSize)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Accept

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SendUserUserInfo
//
// This function can be used to send user-to-user information at any time 
// during a connected call. If the size of the specified information to be 
// sent is larger than what may fit into a single network message (as in ISDN),
// the service provider is responsible for dividing the information into a 
// sequence of chained network messages (using "more data").
//
// The recipient of the UserUser information will receive a LINECALLINFO
// message with the 'dwUserUserInfoxxx' fields filled out on the 
// lineGetCallInfo function.
//
LONG CTSPICallAppearance::SendUserUserInfo (DRV_REQUESTID dwRequestID, LPCSTR lpszUserUserInfo, DWORD dwSize)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SENDUSERUSER) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Check the call state of this call and make sure it is connected
    // to a call.  Added new callstates (TAPI bakeoff fix).
    if ((GetCallState() & 
			(LINECALLSTATE_CONNECTED |
			 LINECALLSTATE_OFFERING |
			 LINECALLSTATE_ACCEPTED |
			 LINECALLSTATE_RINGBACK)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif

    // If the parameters are bad, don't continue.
    if (lpszUserUserInfo == NULL || dwSize == 0L)
        return LINEERR_OPERATIONFAILED;

    // Validate the UserUser information.
    CTSPILineConnection* pLine = GetLineOwner();
    if (pLine->GetLineDevCaps()->dwUUISendUserUserInfoSize < dwSize)
        return LINEERR_USERUSERINFOTOOBIG;

    // Submit the request
    if (AddAsynchRequest(new RTSendUserInfo(this, dwRequestID, lpszUserUserInfo, dwSize)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::SendUserUserInfo

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Secure
//
// Secures the call from any interruptions or interference that may 
// affect the call's media stream.
//
LONG CTSPICallAppearance::Secure (DRV_REQUESTID dwReqId)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SECURECALL) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_SECURECALL) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

	// If this call is ALREADY secure, then ignore the request.
	DWORD dwCallFlags = m_CallInfo.dwCallParamFlags;
	if (dwCallFlags & LINECALLPARAMFLAGS_SECURE)
		return 0L;

#ifdef STRICT_CALLSTATES
	// Check the call state of this call and make sure it is not idle.
	if (GetCallState() == LINECALLSTATE_IDLE)
		return LINEERR_INVALCALLSTATE;
#endif

    // Submit the request
    if (AddAsynchRequest(new RTSecureCall(this, dwReqId)))
        return static_cast<LONG>(dwReqId);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Secure

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Answer
//
// Answer the specified OFFERING call
//
LONG CTSPICallAppearance::Answer(DRV_REQUESTID dwReq, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_ANSWER) == 0 ||
		!GetAddressOwner()->CanAnswerCalls() ||
		 GetLineOwner()->FindRequest(this, REQUEST_ANSWER) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
	// Make sure the call state allows this.
	if (GetCallState() != LINECALLSTATE_OFFERING && GetCallState() != LINECALLSTATE_ACCEPTED)
		return LINEERR_INVALCALLSTATE;
#endif

    // Allocate a new buffer for the UserUserInfo.
    if (lpsUserUserInfo && dwSize > 0)
    {                                    
        CTSPILineConnection* pLine = GetLineOwner();
        if (pLine->GetLineDevCaps()->dwUUIAnswerSize > 0)
        {
            // Validate the UserUser information.
            if (pLine->GetLineDevCaps()->dwUUIAnswerSize < dwSize)
                return LINEERR_USERUSERINFOTOOBIG;
        }
    } 

    // Everything seems ok, submit the answer request.
    if (AddAsynchRequest(new RTAnswer(this, dwReq, lpsUserUserInfo, dwSize)))
        return static_cast<LONG>(dwReq);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Answer

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::BlindTransfer
//
// Transfer the call to a destination without any consultation call
// being created.  The destination address is local to our SP and was
// allocated through GlobalAlloc.
//
LONG CTSPICallAppearance::BlindTransfer(DRV_REQUESTID dwRequestId, 
                        TDialStringArray* parrDestAddr, DWORD dwCountryCode)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_BLINDTRANSFER) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_BLINDXFER) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Ok, make sure the call state allows this.  The call should be connected.
	if (GetCallState() != LINECALLSTATE_CONNECTED)
        return LINEERR_INVALCALLSTATE;
#endif
    
    // If the destination address is not present, then return a error.
    if (parrDestAddr == NULL || parrDestAddr->empty())
        return LINEERR_INVALADDRESS;
                     
    // Everything seems ok, submit the xfer request.
    if (AddAsynchRequest(new RTBlindTransfer(this, dwRequestId, parrDestAddr, dwCountryCode)))
        return static_cast<LONG>(dwRequestId);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::BlindTransfer

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CompleteCall
//
// This method is used to specify how a call that could not be
// connected normally should be completed instead.  The network or
// switch may not be able to complete a call because the network
// resources are busy, or the remote station is busy or doesn't answer.
// 
LONG CTSPICallAppearance::CompleteCall (DRV_REQUESTID dwRequestId, 
				LPDWORD lpdwCompletionID, DWORD dwCompletionMode, DWORD dwMessageID)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_COMPLETECALL) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_COMPLETECALL) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Ok, make sure the call state allows this.
    if ((GetCallState() & 
				(LINECALLSTATE_BUSY |
    			 LINECALLSTATE_RINGBACK |
    			 LINECALLSTATE_PROCEEDING)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif

    // Validate the message id and completion mode
    if ((dwCompletionMode & GetAddressOwner()->GetAddressCaps()->dwCallCompletionModes) == 0)
        return LINEERR_INVALCALLCOMPLMODE;
    if (dwCompletionMode == LINECALLCOMPLMODE_MESSAGE &&
		dwMessageID > GetAddressOwner()->GetAddressCaps()->dwNumCompletionMessages)
        return LINEERR_INVALMESSAGEID;
    
    // If the completion mode is not Message, then zero out the message id.
	if (dwCompletionMode != LINECALLCOMPLMODE_MESSAGE)
		dwMessageID = 0;

    // Store off the completion ID as the pointer to our request object.
	RTCompleteCall* pRequest = new RTCompleteCall(this, dwRequestId, dwCompletionMode, dwMessageID);
    *lpdwCompletionID = reinterpret_cast<DWORD>(pRequest);
    
    // Submit the request to the derived class
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestId);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::CompleteCall

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Dial
//
// Dial an outgoing request onto this call appearance
//
LONG CTSPICallAppearance::Dial (DRV_REQUESTID dwRequestID, 
								TDialStringArray* parrAddresses, 
								DWORD dwCountryCode)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_DIAL) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_DIAL) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure we are in the appropriate connection state.
    if (GetCallState() == LINECALLSTATE_IDLE ||
        GetCallState() == LINECALLSTATE_DISCONNECTED)
        return LINEERR_INVALCALLSTATE;
#endif
    
    // Set the CALLER id information (us)
	if (m_CallerID.strPartyId.empty() || m_CallerID.strPartyName.empty())
		SetCallerIDInformation (LINECALLPARTYID_ADDRESS | LINECALLPARTYID_NAME, 
			GetAddressOwner()->GetDialableAddress(), GetAddressOwner()->GetName());

	// Only set called id information if the call is still dialing.  Otherwise,
	// the digits dialed shouldn't be part of the telephone number and therefore
	// shouldn't be reported.
	if ((GetCallState() & (LINECALLSTATE_DIALTONE | LINECALLSTATE_DIALING)) != 0)
	{
		if (!parrAddresses->empty())
		{
			DIALINFO* pDialInfo = (*parrAddresses)[0];
			DWORD dwAvail = 0;
			if (!pDialInfo->strNumber.empty())
				dwAvail = LINECALLPARTYID_ADDRESS;
			if (!pDialInfo->strName.empty())
				dwAvail |= LINECALLPARTYID_NAME;

			// If we have something to store..
			if (dwAvail > 0)
			{
				// If the call state is dialing, then APPEND the caller id
				// information, otherwise, replace it.
				TString strNumber = pDialInfo->strNumber;
				if (GetCallState() == LINECALLSTATE_DIALING)
					strNumber = m_CalledID.strPartyId + strNumber;
				SetCalledIDInformation (dwAvail, strNumber.c_str(), pDialInfo->strName.c_str());
			}
		}            
	}

    // Submit a dial request
    if (AddAsynchRequest(new RTDial(this, dwRequestID, parrAddresses, dwCountryCode)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Dial

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Hold
//
// Place the call appearance on hold.
//
LONG CTSPICallAppearance::Hold (DRV_REQUESTID dwRequestID)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_HOLD) == 0 ||
		GetLineOwner()->FindRequest(this, REQUEST_HOLD) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

	// Send the request
	if (AddAsynchRequest(new RTHold(this, dwRequestID)))
        return static_cast<LONG>(dwRequestID);
	return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Hold

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Park
//
// Park the call at a specified destination address
//
LONG CTSPICallAppearance::Park (DRV_REQUESTID dwRequestID, DWORD dwParkMode,
								TDialStringArray* parrDial, LPVARSTRING lpNonDirAddress)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_PARK) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_PARK) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure we are in the appropriate connection state.
    if (GetCallState() != LINECALLSTATE_CONNECTED)
        return LINEERR_INVALCALLSTATE;
#endif
    
    // Check whether or not our address supports the park mode requested.
    if ((dwParkMode & GetAddressOwner()->GetAddressCaps()->dwParkModes) == 0)
        return LINEERR_INVALPARKMODE;

	// Pull the first address from the array
	TString strAddress;
	if (parrDial != NULL)
	{
		if (parrDial->empty())
			return LINEERR_INVALADDRESS;
		else
		{
			DIALINFO* pDialInfo = parrDial->at(0);
			strAddress = pDialInfo->strNumber;
		}
	}

	// Create our request to map this park event.
	RTPark* pRequest = new RTPark(this, dwRequestID, dwParkMode, 
			(parrDial != NULL) ? strAddress.c_str() : NULL, lpNonDirAddress);

    // Submit the request
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Park

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Unpark
//
// Unpark the specified address onto this call
//
LONG CTSPICallAppearance::Unpark (DRV_REQUESTID dwRequestID, 
								  TDialStringArray* parrAddresses)
{
    _TSP_ASSERTE(m_CallInfo.dwReason == LINECALLREASON_UNPARK);
    
    // Submit the request
    if (AddAsynchRequest(new RTUnpark(this, dwRequestID, parrAddresses)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONUNAVAIL;

}// CTSPICallAppearance::Unpark

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Pickup
//
// This function picks up a call alerting at the specified destination
// address and returns a call handle for the picked up call.  If invoked
// with a NULL for the 'lpszDestAddr' parameter, a group pickup is performed.
// If required by the device capabilities, 'lpszGroupID' specifies the
// group ID to which the alerting station belongs.
//
LONG CTSPICallAppearance::Pickup (DRV_REQUESTID dwRequestID, TDialStringArray* parrDial, LPCTSTR pszGroupID)
{
    _TSP_ASSERTE(m_CallInfo.dwReason == LINECALLREASON_PICKUP);

    // Setup our callerid information from the first entry in the pickup
	// dialing string information.
    if (!parrDial->empty())
    {
        DWORD dwAvail = 0;
        DIALINFO* pDialInfo = parrDial->at(0);
        if (!pDialInfo->strNumber.empty())
            dwAvail = LINECALLPARTYID_ADDRESS;
        if (!pDialInfo->strName.empty())
            dwAvail |= LINECALLPARTYID_NAME;
        if (dwAvail > 0)
            SetCallerIDInformation (dwAvail, pDialInfo->strNumber.c_str(), pDialInfo->strName.c_str());
    }            

    // Submit the request
    if (AddAsynchRequest(new RTPickup(this, dwRequestID, parrDial, pszGroupID)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Pickup

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::MakeCall
//
// Make an outgoing call on this call appearance.
//
LONG CTSPICallAppearance::MakeCall (DRV_REQUESTID dwRequestID, TDialStringArray* parrDialInfo,
									DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams)   // Optional call parameters
{
#ifdef STRICT_CALLSTATES
    // If the call is in the wrong call state or not available, return an error.
    if (GetCallState() != LINECALLSTATE_UNKNOWN)
        return LINEERR_INVALCALLSTATE;                
#endif
         
    // If we cannot dial out, give an error
    if (!GetAddressOwner()->CanMakeCalls())
        return LINEERR_OPERATIONUNAVAIL;                           

	// This was created as an outgoing call
	m_dwFlags |= _Outgoing;

    // Store off some known call information pieces.
    m_CallInfo.dwOrigin = LINECALLORIGIN_OUTBOUND;
    m_CallInfo.dwReason = LINECALLREASON_DIRECT;
    m_CallInfo.dwCountryCode = dwCountryCode;
    m_CallInfo.dwCompletionID = 0L;

    // Set the bearer and media modes based on the call parameters or
    // default it to the voice settings.
    if (lpCallParams)
    {
        m_CallInfo.dwMediaMode = lpCallParams->dwMediaMode;
        m_CallInfo.dwBearerMode = lpCallParams->dwBearerMode;
        _TSP_ASSERTE((m_CallInfo.dwBearerMode & GetAddressOwner()->GetBearerMode()) == m_CallInfo.dwBearerMode || m_CallInfo.dwBearerMode == LINEBEARERMODE_PASSTHROUGH);
        m_CallInfo.dwCallParamFlags = lpCallParams->dwCallParamFlags;
        MoveMemory (&m_CallInfo.DialParams, &lpCallParams->DialParams, sizeof(LINEDIALPARAMS));

		// If the Call params has calling ID information in it, then use it.
		LPCTSTR lpszBuff = NULL;
		if (lpCallParams->dwCallingPartyIDSize > 0 &&
			lpCallParams->dwCallingPartyIDOffset > 0)
			lpszBuff = reinterpret_cast<LPCTSTR>(reinterpret_cast<LPBYTE>(lpCallParams) + lpCallParams->dwCallingPartyIDOffset);
		else
			lpszBuff = GetAddressOwner()->GetName();
		SetCallerIDInformation (LINECALLPARTYID_ADDRESS | LINECALLPARTYID_NAME, GetAddressOwner()->GetDialableAddress(), lpszBuff);
    }   
    else
    {   
        // Otherwise default to the address information.
        m_CallInfo.dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
        m_CallInfo.dwBearerMode = GetAddressOwner()->GetBearerMode();
		SetCallerIDInformation (LINECALLPARTYID_ADDRESS | LINECALLPARTYID_NAME, GetAddressOwner()->GetDialableAddress(), GetAddressOwner()->GetName());
    }

    // Mark our (the originator) phone as automatically being taken offhook
	// if the line device we are on supports it.
	if (GetAddressOwner()->GetAddressCaps()->dwAddrCapFlags & LINEADDRCAPFLAGS_ORIGOFFHOOK)
		m_CallInfo.dwCallParamFlags |= LINECALLPARAMFLAGS_ORIGOFFHOOK;

    // Setup our initial called id field to the first address within the
    // address list.  The worker code can override this later if necessary.
    if (!parrDialInfo->empty())
    {
        DIALINFO* pDialInfo = parrDialInfo->at(0);
        DWORD dwAvail = 0;
        if (!pDialInfo->strNumber.empty())
            dwAvail = LINECALLPARTYID_ADDRESS;
        if (!pDialInfo->strName.empty())
            dwAvail |= LINECALLPARTYID_NAME;
        if (dwAvail > 0)
            SetCalledIDInformation (dwAvail, pDialInfo->strNumber.c_str(), pDialInfo->strName.c_str());
    }            

    // Submit a call request
    if (AddAsynchRequest(new RTMakeCall(this, dwRequestID, parrDialInfo, dwCountryCode, lpCallParams)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::MakeCall

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Redirect
//
// This function redirects the specified offering call to the specified
// destination address.
//
LONG CTSPICallAppearance::Redirect (
DRV_REQUESTID dwRequestID,             // Asynch. request id
TDialStringArray* parrAddresses,       // Destination to direct to
DWORD dwCountryCode)                   // Country of destination
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_REDIRECT) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_REDIRECT) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure we are in the appropriate connection state.
    if (GetCallState() != LINECALLSTATE_OFFERING)
		return LINEERR_INVALCALLSTATE;
#endif

    // Store off the redirecting ID (us)
    SetRedirectingIDInformation (LINECALLPARTYID_ADDRESS | LINECALLPARTYID_NAME, GetAddressOwner()->GetDialableAddress(), GetAddressOwner()->GetName());

    // Store off the redirection ID (them)
    if (!parrAddresses->empty())
    {
        DIALINFO* pDialInfo = parrAddresses->at(0);
        DWORD dwAvail = 0;
        if (!pDialInfo->strNumber.empty())
            dwAvail = LINECALLPARTYID_ADDRESS;
        if (!pDialInfo->strName.empty())
            dwAvail |= LINECALLPARTYID_NAME;
        if (dwAvail > 0)
            SetRedirectionIDInformation (dwAvail, pDialInfo->strNumber.c_str(), pDialInfo->strName.c_str());
    }            
    else
        return LINEERR_INVALADDRESS;

    // Submit the request.
    if (AddAsynchRequest(new RTRedirect(this, dwRequestID, parrAddresses, dwCountryCode)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Redirect

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetTerminalModes
//
// This is the function which should be called when a lineSetTerminal is
// completed by the derived service provider class.  It is also invoked by
// the owning address or line if a lineSetTerminal was issued for them.
// This stores or removes the specified terminal from the terminal modes 
// given.  TAPI will be notified.
//
void CTSPICallAppearance::SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal)
{
	CEnterCode sLock(this);  // Synch access to object

    // Adjust the value in our terminal map.
    if (iTerminalID < m_arrTerminals.size())
    {
        if (fRouteToTerminal)
            m_arrTerminals[iTerminalID] |= dwTerminalModes;
        else
            m_arrTerminals[iTerminalID] &= ~dwTerminalModes;
    }

    // Notify TAPI about the change.
    OnCallInfoChange (LINECALLINFOSTATE_TERMINAL);

}// CTSPICallAppearance::SetTerminalModes

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnTerminalCountChanged
//
// The terminal count has changed, either add or remove a terminal
// entry from our array
//
void CTSPICallAppearance::OnTerminalCountChanged (bool fAdded, int iPos, DWORD dwMode)
{
	CEnterCode sLock(this);  // Synch access to object

	if (fAdded)
		m_arrTerminals.push_back(dwMode);
	else
		m_arrTerminals.erase(m_arrTerminals.begin() + iPos);

    // Notify TAPI about the change.
    OnCallInfoChange (LINECALLINFOSTATE_TERMINAL);

}// CTSPICallAppearance::OnTerminalCountChanged

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallParams
//
// Set the calling parameters for this call appearance
//
LONG CTSPICallAppearance::SetCallParams (DRV_REQUESTID dwRequestID, DWORD dwBearerMode,
										 DWORD dwMinRate, DWORD dwMaxRate, 
										 LPLINEDIALPARAMS lpDialParams)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETCALLPARAMS) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure the call state allows changes.
    if ((GetCallState() & (LINECALLSTATE_IDLE | LINECALLSTATE_DISCONNECTED)) != 0)
        return LINEERR_INVALCALLSTATE;
#endif

    if (AddAsynchRequest(new RTSetCallParams (this, dwRequestID, dwBearerMode, dwMinRate, dwMaxRate, lpDialParams)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::SetCallParams

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnDetectedNewMediaModes
//
// This method is invoked when a new media mode is detected for a call.
// This happens when the call is set to a new CALLSTATE with a new media
// mode and the LINECALLINFO record changes.  The mode can be a single mode
// (except media mode UNKNOWN), or it can be a combination which
// must include UNKNOWN.
//
// In general, the service provider code should always OR the new type
// into the existing media modes detected, as the application should 
// really be given the opprotunity to change the media mode via lineSetMediaMode.
//
void CTSPICallAppearance::OnDetectedNewMediaModes (DWORD dwModes)
{                                             
	// If the call is no longer valid, then exit.
	if (GetCallHandle() == NULL)
		return;

	CEnterCode sLock(this);  // Synch access to object
	CTSPILineConnection* pLine = GetLineOwner();
	_TSP_ASSERTE(pLine != NULL);

	// Remove the UNKNOWN media mode.
	dwModes &= ~LINEMEDIAMODE_UNKNOWN;

	// Do some validations.
    _TSP_ASSERTE((pLine->GetLineDevCaps()->dwMediaModes & dwModes) == dwModes);
    
    // If media monitoring is enabled, and we hit a mode TAPI
    // is interested in, tell it.
    if (m_CallInfo.dwMonitorMediaModes > 0 && 
        (m_CallInfo.dwMonitorMediaModes & dwModes) == dwModes)
        pLine->Send_TAPI_Event(this, LINE_MONITORMEDIA, dwModes, 0L, GetTickCount());

	// If we are doing media monitoring, then check our list.
	if (m_lpMediaControl != NULL)
	{
		// See if any older timer events which have not yet expired are no longer
		// valid due to a media mode change.
		for (TTimerEventArray::iterator iTimer = m_arrEvents.begin();
			 iTimer != m_arrEvents.end();)
		{
			TIMEREVENT* pEvent = (*iTimer);
			if ((pEvent->iEventType == TIMEREVENT::MediaControlMedia) &&
				((pEvent->dwData2 & dwModes) == 0))
			{
				iTimer = m_arrEvents.erase(iTimer);
				delete pEvent;
			}
			else ++iTimer;
		}
    
		// Now go through our media events and see if any match up here.
		for (std::vector<LINEMEDIACONTROLMEDIA*>::iterator iMedia = m_lpMediaControl->arrMedia.begin();
			 iMedia != m_lpMediaControl->arrMedia.end(); ++iMedia)
		{
			LPLINEMEDIACONTROLMEDIA lpMedia = (*iMedia);
			if (lpMedia->dwMediaModes & dwModes)
			{
				if (lpMedia->dwDuration == 0)
					OnMediaControl (lpMedia->dwMediaControl);
				else
					AddTimedEvent(TIMEREVENT::MediaControlMedia, GetTickCount()+lpMedia->dwDuration, lpMedia->dwMediaControl, lpMedia->dwMediaModes);
			}
		}
	}

}// CTSPICallAppearance::OnDetectedNewMediaModes

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetMediaMode
//
// This function is called as a direct result of the 'lineSetMediaMode'
// API.  This will set the new media mode(s) in the CALLINFO structure
// and tell TAPI it has changed.
//
LONG CTSPICallAppearance::SetMediaMode (DWORD dwMediaMode)
{
	// Check to see if it has changed.
	if (m_CallInfo.dwMediaMode != dwMediaMode)
	{
	    // Validate the media mode.  It should be one of the media modes set in our
	    // CALLINFO structure.
	    if (!GetAddressOwner()->CanSupportMediaModes (dwMediaMode))
	        return LINEERR_INVALMEDIAMODE;

	    // Adjust the media mode in the call record.  This function is designed to be
	    // simply "advisory".  The media mode is not FORCED to be this new mode.
	    m_CallInfo.dwMediaMode = dwMediaMode;
	    OnCallInfoChange (LINECALLINFOSTATE_MEDIAMODE);
	}

    return 0L;

}// CTSPICallAppearance::SetMediaMode

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallerIDInformation
//
// Determines the validity and content of the caller party ID 
// information. The caller is the originator of the call.
//
void CTSPICallAppearance::SetCallerIDInformation(DWORD dwFlags, LPCTSTR lpszPartyID, LPCTSTR lpszName, DWORD dwCountryCode, DWORD dwAddressType)
{
	CEnterCode sLock(this);  // Synch access to object
	
	// Only send if changed.
	if ((dwFlags != m_CallInfo.dwCallerIDFlags) ||
		((dwFlags & LINECALLPARTYID_NAME) && lstrcmp(m_CallerID.strPartyName.c_str(), lpszName)) ||
		((dwFlags & LINECALLPARTYID_ADDRESS) && lstrcmp(m_CallerID.strPartyId.c_str(), lpszPartyID)))
	{
		m_CallInfo.dwCallerIDFlags = dwFlags;
		m_CallerID.strPartyName = (lpszName) ? lpszName : _T("");
		m_CallerID.strPartyId = GetLineOwner()->ConvertDialableToCanonical(lpszPartyID, dwCountryCode, ((m_dwFlags & _Outgoing) == 0));
    
		if (m_CallerID.strPartyName.empty())
			m_CallInfo.dwCallerIDFlags &= ~LINECALLPARTYID_NAME;
		if (m_CallerID.strPartyId.empty())        
			m_CallInfo.dwCallerIDFlags &= ~LINECALLPARTYID_ADDRESS;
		if (dwAddressType == 0xffffffff)
			dwAddressType = GetAddressOwner()->GetAddressType();
		m_CallInfo.dwCallerIDAddressType = dwAddressType;
		OnCallInfoChange(LINECALLINFOSTATE_CALLERID);
	}

}// CTSPICallAppearance::SetCallerIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCalledIDInformation
//
// Determines the validity and content of the called-party ID 
// information. The called party corresponds to the orignally addressed party.
//
void CTSPICallAppearance::SetCalledIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID, LPCTSTR lpszName, DWORD dwCountryCode, DWORD dwAddressType)
{
	CEnterCode sLock(this);  // Synch access to object

	// Only send if changed.
	if ((dwFlags != m_CallInfo.dwCalledIDFlags) ||
		((dwFlags & LINECALLPARTYID_NAME) && lstrcmp(m_CalledID.strPartyName.c_str(), lpszName)) ||
		((dwFlags & LINECALLPARTYID_ADDRESS) && lstrcmp(m_CalledID.strPartyId.c_str(), lpszPartyID)))
	{
		m_CallInfo.dwCalledIDFlags = dwFlags;
		m_CalledID.strPartyName = (lpszName) ? lpszName : _T("");
		m_CalledID.strPartyId = GetLineOwner()->ConvertDialableToCanonical(lpszPartyID, dwCountryCode, ((m_dwFlags & _Outgoing) == 0));
		
		if (m_CalledID.strPartyName.empty())
			m_CallInfo.dwCalledIDFlags &= ~LINECALLPARTYID_NAME;
		if (m_CalledID.strPartyId.empty())        
			m_CallInfo.dwCalledIDFlags &= ~LINECALLPARTYID_ADDRESS;
		if (dwAddressType == 0xffffffff)
			dwAddressType = GetAddressOwner()->GetAddressType();
		m_CallInfo.dwCalledIDAddressType = dwAddressType;
		OnCallInfoChange(LINECALLINFOSTATE_CALLEDID);
	}

}// CTSPICallAppearance::SetCalledIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetConnectedIDInformation
//
// Determines the validity and content of the connected party ID 
// information. The connected party is the party that was actually 
// connected to. This may be different from the called-party ID if the 
// call was diverted.
//
void CTSPICallAppearance::SetConnectedIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID, LPCTSTR lpszName, DWORD dwCountryCode, DWORD dwAddressType)
{
	CEnterCode sLock(this);  // Synch access to object

	// Only send if changed.
	if ((dwFlags != m_CallInfo.dwConnectedIDFlags) ||
		((dwFlags & LINECALLPARTYID_NAME) && lstrcmp(m_ConnectedID.strPartyName.c_str(), lpszName)) ||
		((dwFlags & LINECALLPARTYID_ADDRESS) && lstrcmp(m_ConnectedID.strPartyId.c_str(), lpszPartyID)))
	{
		m_CallInfo.dwConnectedIDFlags = dwFlags;
		m_ConnectedID.strPartyName = (lpszName) ? lpszName : _T("");
		m_ConnectedID.strPartyId = GetLineOwner()->ConvertDialableToCanonical(lpszPartyID, dwCountryCode, ((m_dwFlags & _Outgoing) == 0));

		if (m_ConnectedID.strPartyName.empty())
			m_CallInfo.dwConnectedIDFlags &= ~LINECALLPARTYID_NAME;
		if (m_ConnectedID.strPartyId.empty())        
			m_CallInfo.dwConnectedIDFlags &= ~LINECALLPARTYID_ADDRESS;

		if (dwAddressType == 0xffffffff)
			dwAddressType = GetAddressOwner()->GetAddressType();
		m_CallInfo.dwConnectedIDAddressType = dwAddressType;

		// Only notify TAPI if we are really connected to the caller. Otherwise
		// wait until we do connect. (v3.0b)
		if (GetCallState() & (LINECALLSTATE_ONHOLD |
                 LINECALLSTATE_ONHOLDPENDTRANSFER |
                 LINECALLSTATE_ONHOLDPENDCONF |
                 LINECALLSTATE_CONFERENCED |
				 LINECALLSTATE_CONNECTED))
			OnCallInfoChange(LINECALLINFOSTATE_CONNECTEDID);
	}

}// CTSPICallAppearance::SetConnectedIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetRedirectionIDInformation
//
// Determines the validity and content of the redirection party ID 
// information. The redirection party identifies to the calling user 
// the number towards which diversion was invoked.
//
void CTSPICallAppearance::SetRedirectionIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID, LPCTSTR lpszName, DWORD dwCountryCode, DWORD dwAddressType)
{
	CEnterCode sLock(this);  // Synch access to object

	// Only send if changed.
	if ((dwFlags != m_CallInfo.dwRedirectionIDFlags) ||
		((dwFlags & LINECALLPARTYID_NAME) && lstrcmp(m_RedirectionID.strPartyName.c_str(), lpszName)) ||
		((dwFlags & LINECALLPARTYID_ADDRESS) && lstrcmp(m_RedirectionID.strPartyId.c_str(), lpszPartyID)))
	{
		m_CallInfo.dwRedirectionIDFlags = dwFlags;
		m_RedirectionID.strPartyName = (lpszName) ? lpszName : _T("");
		m_RedirectionID.strPartyId = GetLineOwner()->ConvertDialableToCanonical(lpszPartyID, dwCountryCode);

		if (m_RedirectionID.strPartyName.empty())
			m_CallInfo.dwRedirectionIDFlags &= ~LINECALLPARTYID_NAME;
		if (m_RedirectionID.strPartyId.empty())        
			m_CallInfo.dwRedirectionIDFlags &= ~LINECALLPARTYID_ADDRESS;

		if (dwAddressType == 0xffffffff)
			dwAddressType = GetAddressOwner()->GetAddressType();
		m_CallInfo.dwRedirectionIDAddressType = dwAddressType;

		OnCallInfoChange(LINECALLINFOSTATE_REDIRECTIONID);
	}

}// CTSPICallAppearance::SetRedirectionIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetRedirectingIDInformation
//
// Determines the validity and content of the redirecting party ID 
// information. The redirecting party identifies to the diverted-to 
// user the party from which diversion was invoked.
//
void CTSPICallAppearance::SetRedirectingIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID, LPCTSTR lpszName, DWORD dwCountryCode, DWORD dwAddressType)
{
	CEnterCode sLock(this);  // Synch access to object

	// Only send if changed.
	if ((dwFlags != m_CallInfo.dwRedirectingIDFlags) ||
		((dwFlags & LINECALLPARTYID_NAME) && lstrcmp(m_RedirectingID.strPartyName.c_str(), lpszName)) ||
		((dwFlags & LINECALLPARTYID_ADDRESS) && lstrcmp(m_RedirectingID.strPartyId.c_str(), lpszPartyID)))
	{
		m_CallInfo.dwRedirectingIDFlags = dwFlags;
		m_RedirectingID.strPartyName = (lpszName) ? lpszName : _T("");
		m_RedirectingID.strPartyId = GetLineOwner()->ConvertDialableToCanonical(lpszPartyID, dwCountryCode);

		if (m_RedirectingID.strPartyName.empty())
			m_CallInfo.dwRedirectingIDFlags &= ~LINECALLPARTYID_NAME;
		if (m_RedirectingID.strPartyId.empty())        
			m_CallInfo.dwRedirectingIDFlags &= ~LINECALLPARTYID_ADDRESS;

		if (dwAddressType == 0xffffffff)
			dwAddressType = GetAddressOwner()->GetAddressType();
		m_CallInfo.dwRedirectingIDAddressType = dwAddressType;

		OnCallInfoChange(LINECALLINFOSTATE_REDIRECTINGID);
	}

}// CTSPICallAppearance::SetRedirectingIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallState
//
// This function sets the current connection state for the device
// and notifies TAPI if asked to.
//
void CTSPICallAppearance::SetCallState(DWORD dwState, DWORD dwMode, DWORD dwMediaMode, bool fTellTapi)
{
	// Lock the call hub object -- this will give us singular access to the hub and
	// all the related calls while we determine our new call state and adjust call
	// features.  If we have no hub, this call is a NOP.
	CEnterCode sHub(GetCallHub());

	// Lock the call object for update
	CEnterCode sLock(this);

	// Get the current state of the call object
	DWORD dwCurrState = m_CallStatus.dwCallState;
	DWORD dwCurrMode = m_CallStatus.dwCallStateMode;

	// If the state isn't changing, then don't inform TAPI.
	if (dwState == dwCurrState && ((dwMode == dwCurrMode) || dwMode == 0))
		fTellTapi = false;

	// If the current call state is IDLE, and we are moving out of IDLE, report an error.
	// Once a call enters the IDLE state, it should never transition out.
	if (dwCurrState == LINECALLSTATE_IDLE && dwState != LINECALLSTATE_IDLE)
	{
#ifdef _DEBUG
		_TSP_DTRACE(_T("Attempt to set call 0x%lx (CallID 0x%lx) to %s from IDLE ignored\n"),
				this, GetCallID(), GetCallStateName(dwState));
#endif
		return;
	}

	// Mark us as in the process of changing state.
	m_dwFlags |= _ChgState;

	// If this is the first time we have set the call appearace, mark it as now being
	// "known" to TAPI.
	if ((m_dwFlags & _InitNotify) == 0)
	{
		// Mark this as being KNOWN to tapi.
		m_dwFlags |= _InitNotify;

		// If the call has a pending MAKECALL request waiting, then make sure TAPI has
		// been notified before switching the call state.  TAPI will never send call state
		// change messages through to the application until the MAKECALL request returned an
		// OK response.
		CTSPILineConnection* pLine = GetLineOwner();
		CTSPIRequest* pRequest = pLine->FindRequest(this, REQUEST_MAKECALL);
		if (pRequest != NULL && !pRequest->HaveSentResponse())
		{
			// Complete the request with a zero success response, but do NOT delete
			// the request from the queue.
			pLine->CompleteRequest(pRequest, 0, true, false);
		}
	}

	// If the media mode wasn't supplied, use the media mode which is in our
	// call information record
	if (dwMediaMode == 0L)
	{
		if (m_CallInfo.dwMediaMode == 0)
		{
			// If we only have one bit besides UNKNOWN set, then unmark unknown.  
			// Otherwise leave it there since this call could be one of many media modes.
			m_CallInfo.dwMediaMode = GetAddressOwner()->GetAddressCaps()->dwAvailableMediaModes;
			DWORD dwTest = (m_CallInfo.dwMediaMode & ~LINEMEDIAMODE_UNKNOWN);
			if (((dwTest) & (dwTest - 1)) == 0)
				m_CallInfo.dwMediaMode = dwTest;
		}
		dwMediaMode = m_CallInfo.dwMediaMode;
	}
        
	// Otherwise, set the media mode.  This should be the INITIAL media mode
	// being detected by the provider for a call, OR a new media type which is
	// detected on an existing call (such as a fax tone).
	else  
	{   
		// If we have never set the media mode (initial state)
		if (m_CallInfo.dwMediaMode == 0)
			m_CallInfo.dwMediaMode = dwMediaMode;
		OnDetectedNewMediaModes (dwMediaMode);
	}

	// If the state has changed, then tell everyone who needs to know.
	if ((dwCurrState != dwState) || (dwMode > 0 && dwCurrMode != dwMode))
	{   
		// Save off the new state/mode.  We do this BEFORE notifying the
		// address/line objects since they might change our state.
		m_CallStatus.dwCallState = dwState;
		if (dwMode > 0 || (dwCurrState != dwState))
			m_CallStatus.dwCallStateMode = dwMode;

		// Mark the time we entered this call state.
		GetSystemTime(&m_CallStatus.tStateEntryTime);

		// Unlock the call and hub to notify the address (3.043)
		sLock.Unlock();
		sHub.Unlock();

    	// Tell the address, and then the call appearance that the call
    	// status record is about to be changed. It can at this point call SetCallState
    	// here in order to affect the current state as TAPI has not yet been told.
		// This will change the call count fields stored in the LINEADDRESSSTATUS
		// structure. The notification will not be sent until after we have officially
		// changed the call state.
		m_pAddr->OnPreCallStateChange(this, dwState, dwCurrState);

		// Relock the call and hub (3.043)
		sHub.Lock();
		sLock.Lock();

		// If our call state was changed by the line or call notification
		// done above, then don't bother to go any further into this function
		// since we already came through here recursively.  (I.e. the line
		// called SetCallState() during the OnPreCallStateChange message.
		if (m_CallStatus.dwCallState != dwState)
		{
#ifdef _DEBUG
			_TSP_DTRACE(_T("Aborted PreCallStateChange 0x%lx, CallID 0x%lx Notify=%d, \"%s\" %lx to \"%s\" %lx\n"), 
				this, GetCallID(), fTellTapi, GetCallStateName(dwCurrState), dwCurrMode, GetCallStateName(dwState), dwMode);
#endif			
			m_dwFlags &= ~_ChgState;
			return;
		}

#ifdef _DEBUG
		// Output debug message to the log
		_TSP_DTRACE(_T("CallStateChange 0x%lx, CallID 0x%lx Notify=%d, \"%s\" %lx to \"%s\" %lx\n"), 
				this, GetCallID(), fTellTapi, GetCallStateName(dwCurrState), dwCurrMode, GetCallStateName(dwState), dwMode);
#endif

		// Ask any call hub to forward our call state change to the other calls
		// which share our call id.
		if (GetCallHub() != NULL)
			GetCallHub()->OnCallStateChange(this, dwState, dwCurrState);

		// Update our own internal state.
		OnCallStatusChange(dwState, dwMode, dwMediaMode);

		// If we are related to another call, then tell it about our state change.
		// This is used by the conferencing calls to associate our call with a conference
		// and automatically remove it when we drop off.
		if (GetConferenceOwner() != NULL && GetCallType() != Conference)
		{
			CTSPICallAppearance* pConfCall = GetConferenceOwner();
			pConfCall->OnRelatedCallStateChange (this, dwState, dwCurrState);
		}
    
		// If we have an attached call (consultation or otherwise) tell them about
		// our state change.
		if (GetAttachedCall() != NULL)
		{
			// Lock the attached call for update before we call to ensure that
			// we don't deadlock in that code against this thread if it is
			// updating right now.
			CTSPICallAppearance* pCall = GetAttachedCall();
			CEnterCode cLock(pCall, FALSE);
			if (cLock.Lock(0))
				pCall->OnRelatedCallStateChange (this, dwState, dwCurrState);
		}

		// Now run through all the other calls in the system and tell them to update 
		// their capabilities.  Ignore conference calls since they handle adjustment 
		// of children calls internally.  We do this because it is assumed that
		// one call's state can affect the feature set of other calls on this address.
		if (GetCallType() != Conference)
		{
			// Spin through all the other calls on this
			// address excluding calls which are part of a conference (the conference
			// owner itself will perform this same loop).
			CEnterCode sAddrLock(m_pAddr); // Unlocks at exit of if()
			for (int iCall = 0; iCall < m_pAddr->GetCallCount(); iCall++)
			{
				CTSPICallAppearance* pCall = m_pAddr->GetCallInfo(iCall);
				_TSP_ASSERTE(pCall != NULL);
				if (pCall != this && 
					pCall->GetConferenceOwner() == NULL &&
					pCall->GetCallState() != LINECALLSTATE_IDLE)
					pCall->RecalcCallFeatures();
			}
		}

		// Pass the notification to TAPI if necessary.  This is always the last
		// step performed so each object gets a chance to look at the new state before
		// we tell TAPI.
		if (fTellTapi)
			NotifyCallStatusChanged();
	}

	// TAPI has been notified now.  Let the address/line send out
	// state events regarding call counts and recalc their feature set.
	m_pAddr->OnCallStateChange(this, dwState, dwCurrState);

	// We are done changing state
	m_dwFlags &= ~_ChgState;

}// CTSPICallAppearance::SetCallState

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnRelatedCallStateChange
//
// A related call has changed state
//
void CTSPICallAppearance::OnRelatedCallStateChange(CTSPICallAppearance* pCall, DWORD dwState, DWORD /*dwOldState*/)
{
    // We should only get related call information from consultation calls.
    if (dwState == LINECALLSTATE_IDLE)
    {   
		// Call the notification handler if this is the NORMAL call.
		if (GetCallType() != Consultant)
			OnConsultantCallIdle(GetConsultationCall());

		// Detach the call
		SetConsultationCall(NULL);

		// Now run through and see if any other call was attached to this call
		// appearance if WE are not idle.  This will allow for a "chain" of
		// attached call relationships.
		if (GetCallState() != LINECALLSTATE_IDLE)
		{
			pCall = GetAddressOwner()->FindAttachedCall(this);
			if (pCall != NULL)
				AttachCall(pCall);
		}

		// If we have no call attachment and we are a consultant call created for
		// some other call (lineSetupTransfer/Conference) then mark us a NORMAL
		// call now since the original purpose of this call has changed.  Also,
		// change our call features to allow this new call to be a target of a 
		// transfer/conference.
		if (GetAttachedCall() == NULL && GetCallType() == Consultant)
		{
			SetCallType(Normal);
			RecalcCallFeatures();
		}
    }

}// CTSPICallAppearance::OnRelatedCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnConsultantCallIdle
//
// This member is called when the consultant call attached to this
// call goes IDLE.
//
// It should be overridden if the switch performs some action
// when a consultation call goes IDLE - such as restoring the original
// call.
//
void CTSPICallAppearance::OnConsultantCallIdle(CTSPICallAppearance* /*pConsultCall*/)
{
    /* Do nothing */
    
}// CTSPICallAppearance::OnConsultantCallIdle

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnShadowCallStateChange
//
// Virtual callback when the "other" side of a call changes
// state.  Default behavior is to handle the IDLE case and idle the
// other side of the call.
//
void CTSPICallAppearance::OnShadowCallStateChange(CTSPICallAppearance* pCall, 
												  DWORD dwState, DWORD /*dwCurrState*/)
{
	// Get the total number of calls in the hub.  If there are more than 2 calls in the hub
	// then we ignore the state change since we cannot tell what will affect us - i.e. dropping
	// a single leg shouldn't drop other legs.  Also ignore the notification if we are currently
	// part of a conference - allow the CTSPIConferenceCall to manage our state instead.
	DWORD dwCallCount = GetCallHub()->GetHubCount();
	if (dwCallCount > 2 || GetConferenceOwner() != NULL)
		return;

	// If the other side is going IDLE the show a disconnect on our side of the call. 
	if (dwState == LINECALLSTATE_IDLE && GetCallState() != LINECALLSTATE_IDLE)
	{
		SetCallState(LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NORMAL);
		// If this call is not being referenced by TAPI (i.e. no owner) then
		// transition it automatically to IDLE and let it disappear.
		if (GetCallHandle() == NULL)
			SetCallState(LINECALLSTATE_IDLE);
	}

	// If the other side is onHold and we are connected, then show ActiveHeld.
	else if ((dwState & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDCONF | LINECALLSTATE_ONHOLDPENDTRANSFER)) != 0 &&
			  GetCallState() == LINECALLSTATE_CONNECTED)
	{
		SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVEHELD);
	}
	
	// If they are both connected -OR- we were OFFERING.
	else if (dwState == LINECALLSTATE_CONNECTED && 
			(GetCallState() & (LINECALLSTATE_CONNECTED | 
					LINECALLSTATE_OFFERING | 
					LINECALLSTATE_RINGBACK)) != 0)
	{
		SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVE);
	}

	// If the other side is part of a conference, then use the conference
	// state to determine ours.
	else if (dwState == LINECALLSTATE_CONFERENCED && GetCallState() == LINECALLSTATE_CONNECTED)
	{
		CTSPIConferenceCall* pConf = pCall->GetConferenceOwner();
		if (pConf != NULL)
		{
			if ((pConf->GetCallState() & (LINECALLSTATE_ONHOLDPENDCONF | LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER)) != 0)
				SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVEHELD);
			else
				SetCallState(LINECALLSTATE_CONNECTED, LINECONNECTEDMODE_ACTIVE);
		}
	}

}// CTSPICallAppearance::OnShadowCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GatherCallInformation
//
// Gather the LINECALLINFO information for this call appearance
//
LONG CTSPICallAppearance::GatherCallInformation (LPLINECALLINFO lpCallInfo)    
{   
	// Determine what the negotiated version was for the line.
	DWORD dwTSPIVersion = GetLineOwner()->GetNegotiatedVersion();
	if (dwTSPIVersion == 0)
		dwTSPIVersion = GetSP()->GetSupportedVersion();

	// Synch access to object
	CEnterCode sLock(this);  

    // Save off the information that TAPI provides.
    m_CallInfo.dwTotalSize = lpCallInfo->dwTotalSize;
    m_CallInfo.hLine = lpCallInfo->hLine;
    // NOTE: Original TSPI document (dated 1993) states that TAPI.DLL fills in
    // these fields, but that doesn't appear to be the case.
	// m_CallInfo.dwCallStates = lpCallInfo->dwCallStates.
	// m_CallInfo.dwMonitorDigitModes = lpCallInfo->dwMonitorDigitModes;
	// m_CallInfo.dwMonitorMediaModes = lpCallInfo->dwMonitorMediaModes;
    m_CallInfo.dwNumOwners = lpCallInfo->dwNumOwners;
    m_CallInfo.dwNumMonitors = lpCallInfo->dwNumMonitors;
    m_CallInfo.dwAppNameSize = lpCallInfo->dwAppNameSize;
    m_CallInfo.dwAppNameOffset = lpCallInfo->dwAppNameOffset;
    m_CallInfo.dwDisplayableAddressSize = lpCallInfo->dwDisplayableAddressSize;
    m_CallInfo.dwDisplayableAddressOffset = lpCallInfo->dwDisplayableAddressOffset;
    m_CallInfo.dwCalledPartySize = lpCallInfo->dwCalledPartySize;
    m_CallInfo.dwCalledPartyOffset = lpCallInfo->dwCalledPartyOffset;
    m_CallInfo.dwCommentSize = lpCallInfo->dwCommentSize;
    m_CallInfo.dwCommentOffset = lpCallInfo->dwCommentOffset;
    
    // Now verify that we have enough space for our basic structure
	m_CallInfo.dwNeededSize = sizeof(LINECALLINFO);
	if (dwTSPIVersion < TAPIVER_20)
		m_CallInfo.dwNeededSize -= sizeof(DWORD)*7;
	if (dwTSPIVersion < TAPIVER_30)
		m_CallInfo.dwNeededSize -= sizeof(DWORD)*5;		
		//m_CallInfo.dwNeededSize -= sizeof(DWORD);		
	// Fix above by Ron on 5/8/09.  Was only substrating 1 DWORD, but if you look at
	// the CallInfo structure, there are 5 values added to the struct.
	// When creating an incoming call to TAPI Browser, it only calls with the minimal size.

#ifdef _DEBUG
    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.  NOTE: This should never
	// happen - TAPI.DLL is supposed to verify that there is enough space in the structure
	// for the negotiated version and return an error.  We only check this in DEBUG builds
	// to insure that TAPI is doing its job.
    if (lpCallInfo->dwTotalSize < m_CallInfo.dwNeededSize)
	{
		lpCallInfo->dwNeededSize = m_CallInfo.dwNeededSize;
        return LINEERR_STRUCTURETOOSMALL;
	}
#endif

    // Copy our basic structure over
    MoveMemory (lpCallInfo, &m_CallInfo, m_CallInfo.dwNeededSize);
    lpCallInfo->dwUsedSize = m_CallInfo.dwNeededSize;

	// Now handle some version manipulation
	if (dwTSPIVersion < TAPIVER_14)
	{
		if (lpCallInfo->dwOrigin == LINECALLORIGIN_INBOUND)
			lpCallInfo->dwOrigin = LINECALLORIGIN_UNAVAIL;
		if (lpCallInfo->dwBearerMode == LINEBEARERMODE_PASSTHROUGH)
			lpCallInfo->dwBearerMode = 0;
		if (lpCallInfo->dwReason & (LINECALLREASON_INTRUDE | LINECALLREASON_PARKED))
			lpCallInfo->dwReason = LINECALLREASON_UNAVAIL;
		if (lpCallInfo->dwMediaMode == LINEMEDIAMODE_VOICEVIEW)
			lpCallInfo->dwMediaMode = LINEMEDIAMODE_UNKNOWN;
	}
	
	if (dwTSPIVersion < TAPIVER_20)
	{
		if (lpCallInfo->dwBearerMode == LINEBEARERMODE_RESTRICTEDDATA)
			lpCallInfo->dwBearerMode = 0;
		if (lpCallInfo->dwReason & (LINECALLREASON_CAMPEDON | LINECALLREASON_ROUTEREQUEST))
			lpCallInfo->dwReason = LINECALLREASON_UNAVAIL;
	}

    // Fill in the caller id information.
    if (!m_CallerID.strPartyId.empty() &&
		(lpCallInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS))
		AddDataBlock (lpCallInfo, lpCallInfo->dwCallerIDOffset,
				lpCallInfo->dwCallerIDSize, m_CallerID.strPartyId.c_str());

    if (!m_CallerID.strPartyName.empty() &&
		(lpCallInfo->dwCallerIDFlags & LINECALLPARTYID_NAME))
		AddDataBlock (lpCallInfo, lpCallInfo->dwCallerIDNameOffset, 
				lpCallInfo->dwCallerIDNameSize, m_CallerID.strPartyName.c_str());
    
    if (!m_CalledID.strPartyId.empty() &&
		(lpCallInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS))
		AddDataBlock (lpCallInfo, lpCallInfo->dwCalledIDOffset,
				lpCallInfo->dwCalledIDSize, m_CalledID.strPartyId.c_str());

    if (!m_CalledID.strPartyName.empty() &&
		(lpCallInfo->dwCalledIDFlags & LINECALLPARTYID_NAME))
		AddDataBlock (lpCallInfo, lpCallInfo->dwCalledIDNameOffset, 
				lpCallInfo->dwCalledIDNameSize, m_CalledID.strPartyName.c_str());

	// Only add the connected id information if we are really connected. This allows
	// the derived provider to use the "SetConnectedIDInformation" function when it
	// determines who the connected party will be (such as in a transfer). The information
	// will only be reported when the call is actually connected. (v3.0b)
	if (GetCallState() & (LINECALLSTATE_ONHOLD | LINECALLSTATE_ONHOLDPENDTRANSFER | 
			LINECALLSTATE_ONHOLDPENDCONF | LINECALLSTATE_CONFERENCED | LINECALLSTATE_CONNECTED))
	{
		if (!m_ConnectedID.strPartyId.empty() &&    
			(lpCallInfo->dwConnectedIDFlags & LINECALLPARTYID_ADDRESS))
			AddDataBlock (lpCallInfo, lpCallInfo->dwConnectedIDOffset,
					lpCallInfo->dwConnectedIDSize, m_ConnectedID.strPartyId.c_str());
		if (!m_ConnectedID.strPartyName.empty() &&
			(lpCallInfo->dwConnectedIDFlags & LINECALLPARTYID_NAME))
			AddDataBlock (lpCallInfo, lpCallInfo->dwConnectedIDNameOffset, 
					lpCallInfo->dwConnectedIDNameSize, m_ConnectedID.strPartyName.c_str());
	}
	else
	{
		if (lpCallInfo->dwConnectedIDFlags & (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS))
			lpCallInfo->dwConnectedIDFlags = LINECALLPARTYID_UNKNOWN;
	}

    if (!m_RedirectionID.strPartyId.empty() &&
		(lpCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_ADDRESS))
		AddDataBlock (lpCallInfo, lpCallInfo->dwRedirectionIDOffset,
				lpCallInfo->dwRedirectionIDSize, m_RedirectionID.strPartyId.c_str());

    if (!m_RedirectionID.strPartyName.empty() &&
		(lpCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_NAME))
		AddDataBlock (lpCallInfo, lpCallInfo->dwRedirectionIDNameOffset, 
				lpCallInfo->dwRedirectionIDNameSize, m_RedirectionID.strPartyName.c_str());

    if (!m_RedirectingID.strPartyId.empty() &&
		(lpCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_ADDRESS))
		AddDataBlock (lpCallInfo, lpCallInfo->dwRedirectingIDOffset,
				lpCallInfo->dwRedirectingIDSize, m_RedirectingID.strPartyId.c_str());

    if (!m_RedirectingID.strPartyName.empty() &&
		(lpCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_NAME))
		AddDataBlock (lpCallInfo, lpCallInfo->dwRedirectingIDNameOffset, 
				lpCallInfo->dwRedirectingIDNameSize, m_RedirectingID.strPartyName.c_str());
    
    // If we have room for terminal entries, then include them.
    if (!m_arrTerminals.empty())
    {
        if (lpCallInfo->dwTotalSize >= lpCallInfo->dwUsedSize + (m_arrTerminals.size() * sizeof(DWORD)))
        {
			for (TDWordArray::const_iterator it = m_arrTerminals.begin(); it != m_arrTerminals.end(); ++it)
			{
				DWORD dwValue = (*it);
				AddDataBlock (lpCallInfo, lpCallInfo->dwTerminalModesOffset,
					lpCallInfo->dwTerminalModesSize, &dwValue, sizeof(DWORD));
			}
        }
    }

    // If we have room for the UserUser information, then copy it over.
    if (!m_arrUserUserInfo.empty())
	{
		SIZEDDATA* pInfo = m_arrUserUserInfo[0];
		AddDataBlock (lpCallInfo, lpCallInfo->dwUserUserInfoOffset,
				lpCallInfo->dwUserUserInfoSize, pInfo->GetPtr(), pInfo->GetSize());
	}

	// If we have room for the charging information, the copy it over (v3.0b)
	if (m_sdChargingInfo.GetSize() > 0)
		AddDataBlock(lpCallInfo, lpCallInfo->dwChargingInfoOffset,
				lpCallInfo->dwChargingInfoSize, m_sdChargingInfo.GetPtr(), m_sdChargingInfo.GetSize());

	// If we have room for the compatibility information, then copy it over (v3.0b)
	if (m_sdLowLevelInfo.GetSize() > 0)
		AddDataBlock(lpCallInfo, lpCallInfo->dwLowLevelCompOffset,
				lpCallInfo->dwLowLevelCompSize, m_sdLowLevelInfo.GetPtr(), m_sdLowLevelInfo.GetSize());

	if (m_sdHiLevelInfo.GetSize() > 0)
		AddDataBlock(lpCallInfo, lpCallInfo->dwHighLevelCompOffset,
				lpCallInfo->dwHighLevelCompSize, m_sdHiLevelInfo.GetPtr(), m_sdHiLevelInfo.GetSize());

	// Add in the TAPI 2.0 extensions
	if (dwTSPIVersion >= TAPIVER_20)
	{
	    // If we have room for the CALLDATA information, then add it into the callinfo record.
		if (m_sdCallData.GetSize() > 0)
			AddDataBlock(lpCallInfo, lpCallInfo->dwCallDataOffset, 
					lpCallInfo->dwCallDataSize, m_sdCallData.GetPtr(), m_sdCallData.GetSize());

		// If we have negotiated a QOS, and have room for it, then add
		// the FLOWSPEC data into the CALLINFO record.
		if (m_sdSendingFS.GetSize() > 0)
			AddDataBlock (lpCallInfo, lpCallInfo->dwSendingFlowspecOffset,
					lpCallInfo->dwSendingFlowspecSize, m_sdSendingFS.GetPtr(), m_sdSendingFS.GetSize());
		if (m_sdReceivingFS.GetSize() > 0)
			AddDataBlock (lpCallInfo, lpCallInfo->dwReceivingFlowspecOffset, 
					lpCallInfo->dwReceivingFlowspecSize, m_sdReceivingFS.GetPtr(), m_sdReceivingFS.GetSize());
	}

    return false;         
   
}// CTSPICallAppearance::GatherCallInformation

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GatherStatusInformation
//
// Gather all the current status information for this call
//
LONG CTSPICallAppearance::GatherStatusInformation(LPLINECALLSTATUS lpCallStatus)
{   
	// Determine what the negotiated version was for the line.
	DWORD dwTSPIVersion = GetLineOwner()->GetNegotiatedVersion();
	if (dwTSPIVersion == 0)
		dwTSPIVersion = GetSP()->GetSupportedVersion();

	// Synch access to object
	CEnterCode sLock(this);  

    // Save off what TAPI provides
    m_CallStatus.dwTotalSize = lpCallStatus->dwTotalSize;
    m_CallStatus.dwCallPrivilege = lpCallStatus->dwCallPrivilege;
    
    // Now fill in the other fields.
    m_CallStatus.dwNeededSize = sizeof(LINECALLSTATUS);
	if (dwTSPIVersion < TAPIVER_20)
		m_CallStatus.dwNeededSize -= (sizeof(DWORD) + sizeof(SYSTEMTIME));
    
#ifdef _DEBUG
    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.  NOTE: This should never
	// happen - TAPI.DLL is supposed to verify that there is enough space in the structure
	// for the negotiated version and return an error.  We only check this in DEBUG builds
	// to insure that TAPI is doing its job.
    if (lpCallStatus->dwTotalSize < m_CallStatus.dwNeededSize)
	{
		lpCallStatus->dwNeededSize = m_CallStatus.dwNeededSize;
        return LINEERR_STRUCTURETOOSMALL;
	}
#endif

    // Copy our static structure into this one.
    MoveMemory (lpCallStatus, &m_CallStatus, m_CallStatus.dwNeededSize);
    lpCallStatus->dwUsedSize = m_CallStatus.dwNeededSize;
    
    return 0L;

}// CTSPICallAppearance::GatherStatusInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Unhold
//
// Take the call appearance off hold
//
LONG CTSPICallAppearance::Unhold (DRV_REQUESTID dwRequestID)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_UNHOLD) == 0 ||
		GetLineOwner()->FindRequest(this, REQUEST_UNHOLD) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure the call is in the HOLD state.
    if ((GetCallState() & 
				(LINECALLSTATE_ONHOLD |
				 LINECALLSTATE_ONHOLDPENDTRANSFER |
				 LINECALLSTATE_ONHOLDPENDCONF)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif

    // Submit the request
    if (AddAsynchRequest(new RTUnhold(this, dwRequestID)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::Unhold

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CompleteDigitGather
//
// This function completes a digit gathering session and deletes and 
// resets our digit gather.  It is called when a termination
// digit is found, the buffer is full, or a gather is cancelled.
//
void CTSPICallAppearance::CompleteDigitGather(DWORD dwReason)
{
	CEnterCode sLock(this);  // Synch access to object
    if (m_lpGather.get() != NULL && GetCallHandle() != NULL)
    {
        _TSP_ASSERTE(m_lpGather->lpBuffer != NULL);
        GetLineOwner()->Send_TAPI_Event(this, LINE_GATHERDIGITS, dwReason, m_lpGather->dwEndToEndID, GetTickCount());
        delete m_lpGather.release();
    }

}// CTSPICallAppearance::CompleteDigitGather

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnInternalTimer
//
// This method is called by the service provider when this call
// is in the "need timer" list.
//
bool CTSPICallAppearance::OnInternalTimer()
{                           
	CEnterCode Key (this, false);
	if (Key.Lock(0))
	{
		// Get the current TICK count - it is used to timeout media events
		// on this call appearance.
		DWORD dwCurr = GetTickCount();

		// If we are collecting digits then see if our inter-digit or first-digit
		// timeout has expired.  If so, tell TAPI we have cancled the digit gathering
		// event because we haven't seen the digits quickly enough.
		if (m_lpGather.get() != NULL)
		{
			// No digits collected at all?
			if (m_lpGather->dwCount == 0L)
			{
				// If we have a timeout and it is PAST our current tick then complete
				// the request with an error.
				if (m_lpGather->dwFirstDigitTimeout > 0L &&
					m_lpGather->dwFirstDigitTimeout + m_lpGather->dwLastTime < dwCurr)
					CompleteDigitGather (LINEGATHERTERM_FIRSTTIMEOUT);
			}                
			else // Have at least one character
			{
				// If we have a timeout and it is PAST our current tick then complete
				// the request with an error.
				if (m_lpGather->dwInterDigitTimeout > 0L &&
					m_lpGather->dwInterDigitTimeout + m_lpGather->dwLastTime < dwCurr)
					CompleteDigitGather (LINEGATHERTERM_INTERTIMEOUT);
			}                 
		}
    
		// See if we have any pending TIMER events we need to check on.  These are
		// inserted as a result of a detected TONE on the call which matched a
		// monitor event.  They are marked with the last time the tone on the call
		// changed (via OnTone) and if we see the tone long enough we send a completion.
		// 
		// If the tone changes (in OnTone) on the call and we didn't see the tone
		// long enough for the monitor event it is removed in the OnTone function.
		for (TTimerEventArray::iterator iTimer = m_arrEvents.begin();
		     iTimer != m_arrEvents.end();)
		{
			TIMEREVENT* lpEvent = (*iTimer);
			if (lpEvent->dwEndTime <= GetTickCount())
			{
				switch (lpEvent->iEventType)
				{
					case TIMEREVENT::MediaControlMedia:
					case TIMEREVENT::MediaControlTone:
						OnMediaControl (lpEvent->dwData1);
						break;
					case TIMEREVENT::ToneDetect:
						OnToneMonitorDetect(lpEvent->dwData1, lpEvent->dwData2);
						break;
					default:
						_TSP_ASSERT(false);
						break;
				}

				// Remove and delete the event.
				iTimer = m_arrEvents.erase(iTimer);
				delete lpEvent;
			}
			else ++iTimer;
		}

		// Now check to see if we need any further interval timer activity.
		// If not we remove this call from the activity list which keeps our
		// timer running smoothly by not processing calls which don't need it.
		if (m_arrEvents.empty() && m_lpGather.get() == NULL)
			return true;
	}

	// Continue sending interval timers.
	return false;

}// CTSPICallAppearance::OnInternalTimer

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnTone
//
// This method is called when a tone is detected for either the address
// this call appears on, or if possible, directly by the worker code for
// this call appearance.  This should only be invoked when a CHANGE is
// detected (ie: if tone transitions to silence, then we should see the
// tone, followed by a single silence indicator, followed by the next
// tone whenever that happens).
//
// The three frequency fields are the Hz value components which comprise this
// tone.  If fewer than three tone frequencies are required, then the unused
// entries should be zero.  If all three values are zero, this indicates
// silence on the media.
//
void CTSPICallAppearance::OnTone (DWORD dwFreq1, DWORD dwFreq2, DWORD dwFreq3)
{                              
	// See if any older timer events which have not yet expired are no longer
    // valid due to this tone change.  If the proper time DID elapse, and we
    // just didn't spin around quick enough to catch it, then send the notification
    // to TAPI.
	CEnterCode sLock(this);  // Synch access to object
	for (TTimerEventArray::iterator iTimer = m_arrEvents.begin();
		 iTimer != m_arrEvents.end();)
	{
		TIMEREVENT* lpEvent = (*iTimer);
		
		// If our timeout has elapsed for this tone on the call then
		// TAPI needs to be notified that the tone was on the call long enough
		// to be noticed.
		if (lpEvent->dwEndTime <= GetTickCount())
		{
			if (lpEvent->iEventType == TIMEREVENT::MediaControlTone)
				OnMediaControl (lpEvent->dwData1);
			else if (lpEvent->iEventType == TIMEREVENT::ToneDetect)
            	OnToneMonitorDetect (lpEvent->dwData1, lpEvent->dwData2);
		}

		// Otherwise, we assume the tone has changed. We need to
		// remove this entry since it wasn't on the call long enough for the 
		// app to be notified -or- we just notified TAPI about it.
        iTimer = m_arrEvents.erase(iTimer);
        delete lpEvent;
    }
    
	// Now that we have dealt with any timed-event media control, lets walk through
	// our structures which represent the tones we are watching for.  If we see
	// a matching tone, then either notify TAPI (for no duration) or insert a new
	// TIMEREVENT so we can see how long the tone is presented.
	if (m_lpMediaControl != NULL)
	{        
        // Now go through our media events and see if any match up here.
		for (TSPIMEDIACONTROL::TToneArray::iterator it = m_lpMediaControl->arrTones.begin(); it != m_lpMediaControl->arrTones.end(); ++it)
        {
            if (GetSP()->MatchTones ((*it)->dwFrequency1, (*it)->dwFrequency2, (*it)->dwFrequency3, dwFreq1, dwFreq2, dwFreq3))
            {
				// Immediate notification?
                if ((*it)->dwDuration == 0)
                    OnMediaControl ((*it)->dwMediaControl);
				// Or timed?
                else
					AddTimedEvent(TIMEREVENT::MediaControlTone,::GetTickCount()+(*it)->dwDuration, (*it)->dwMediaControl);
            }
        }
    }
    
    // If we have any tone lists we are looking for, search them.
	for (TMonitorToneArray::iterator it = m_arrMonitorTones.begin(); it != m_arrMonitorTones.end(); ++it)
    {
        TSPITONEMONITOR* lpToneList = (*it);
        if (lpToneList)
        {
            for (unsigned int j = 0; j < lpToneList->arrTones.size(); j++)
            {
                LPLINEMONITORTONE lpTone = reinterpret_cast<LPLINEMONITORTONE>(lpToneList->arrTones[j]);
                _TSP_ASSERTE(lpTone != NULL);
                if (GetSP()->MatchTones (lpTone->dwFrequency1, lpTone->dwFrequency2, lpTone->dwFrequency3,
                                        dwFreq1, dwFreq2, dwFreq3))
                {
                    if (lpTone->dwDuration == 0)
                        OnToneMonitorDetect (lpToneList->dwToneListID, lpTone->dwAppSpecific);
                    else
						AddTimedEvent(TIMEREVENT::ToneDetect, GetTickCount() + lpTone->dwDuration, lpToneList->dwToneListID, lpTone->dwAppSpecific);
                }
            }
        }
    }

}// CTSPICallAppearance::OnTone

#ifdef _UNICODE
///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnDigit
//
// Non-UNICODE version of function.
//
void CTSPICallAppearance::OnDigit (DWORD dwType, char cDigit)
{                               
	TCHAR ch;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &cDigit, 1, &ch, sizeof(TCHAR));
	OnDigit (dwType, ch);

}// CTSPICallAppearance::OnDigit
#endif // _UNICODE

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnDigit
//
// This method should be called by the address when a digit is detected
// on the address this call appearance is attached to.  It may be called
// directly by the derived class if digit detection can be isolated to 
// a particular call (not always possible).
//
void CTSPICallAppearance::OnDigit(DWORD dwType, TCHAR cDigit)
{
	// If the call is not valid, exit.
	if (GetCallHandle() == NULL)
		return;

    // If we are monitoring for this type of digit, send a 
    // digit monitor message to TAPI.
	CEnterCode sLock(this);  // Synch access to object
    if (m_CallInfo.dwMonitorDigitModes & dwType)
    {
        CTSPILineConnection* pLine = GetLineOwner();
        pLine->Send_TAPI_Event(this, LINE_MONITORDIGITS, static_cast<DWORD>(cDigit), dwType, GetTickCount());
    }

    // Add the character to our buffer. 
    if (m_lpGather.get() != NULL)
    {   
        if (m_lpGather->dwDigitModes & dwType)
        {
#ifndef _UNICODE
			wchar_t cChar;
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &cDigit, 1, &cChar, sizeof(wchar_t));
			LPWSTR lpwStr = reinterpret_cast<LPWSTR>(m_lpGather->lpBuffer);
			*(lpwStr + m_lpGather->dwCount) = cChar;
#else
            *(m_lpGather->lpBuffer+m_lpGather->dwCount) = cDigit;
#endif
            ++m_lpGather->dwCount;
            m_lpGather->dwLastTime = GetTickCount();
        
            // Check for termination conditions.
            if (m_lpGather->dwCount == m_lpGather->dwSize)
                CompleteDigitGather (LINEGATHERTERM_BUFFERFULL);
            else if (m_lpGather->strTerminationDigits.find(cDigit) != TString::npos)
                CompleteDigitGather (LINEGATHERTERM_TERMDIGIT);
        }                
    }

    // If we are doing media monitoring, then check our list.
    if (m_lpMediaControl != NULL)
    {
        for (unsigned int i = 0; i < m_lpMediaControl->arrDigits.size(); i++)
        {
            LPLINEMEDIACONTROLDIGIT lpDigit = reinterpret_cast<LPLINEMEDIACONTROLDIGIT>(m_lpMediaControl->arrDigits[i]);
			if (lpDigit->dwDigit == static_cast<DWORD>(cDigit))
                OnMediaControl (lpDigit->dwMediaControl);
        }
    }

}// CTSPICallAppearance::OnDigit

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GatherDigits
//
// Initiates the buffered gathering of digits on the specified call. 
// The application specifies a buffer in which to place the digits and the 
// maximum number of digits to be collected.
//
LONG CTSPICallAppearance::GatherDigits(TSPIDIGITGATHER* lpGather)
{   
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_GATHERDIGITS) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // If this is a request to CANCEL digit gathering, then do so.
    if (lpGather == NULL || lpGather->lpBuffer == NULL)
    {
        delete lpGather;
        CompleteDigitGather(LINEGATHERTERM_CANCEL);
        return false;
    }

    // Verify that the parameters within the gather structure are valid.
    CTSPILineConnection* pLine = GetLineOwner();
    
    // Digit timeout values.
    if (lpGather->dwFirstDigitTimeout > 0 &&
        (lpGather->dwFirstDigitTimeout < pLine->GetLineDevCaps()->dwGatherDigitsMinTimeout ||
         lpGather->dwFirstDigitTimeout > pLine->GetLineDevCaps()->dwGatherDigitsMaxTimeout) ||
        lpGather->dwInterDigitTimeout > 0 &&
         (lpGather->dwInterDigitTimeout < pLine->GetLineDevCaps()->dwGatherDigitsMinTimeout ||
         lpGather->dwInterDigitTimeout > pLine->GetLineDevCaps()->dwGatherDigitsMaxTimeout))
        return LINEERR_INVALTIMEOUT;
    
    // If we cannot detect the type of digits requested, return an error.
    if ((lpGather->dwDigitModes & pLine->GetLineDevCaps()->dwMonitorDigitModes) != lpGather->dwDigitModes)
        return LINEERR_INVALDIGITMODE;
                                     
    // Validate the termination digits.
    if (lpGather->dwDigitModes & LINEDIGITMODE_PULSE)
    {
        if (lpGather->strTerminationDigits.find_first_not_of(_T("0123456789")) != TString::npos)
            return LINEERR_INVALDIGITS;
    }
    
    if (lpGather->dwDigitModes & LINEDIGITMODE_DTMF)
    {
        if (lpGather->strTerminationDigits.find_first_not_of(_T("0123456789ABCD*#")) != TString::npos)
            return LINEERR_INVALDIGITS;
    }

	lpGather->dwCount = 0;
	lpGather->dwLastTime = GetTickCount();

    // Everything looks ok, setup the new digit gathering.
	// Would prefer the standard STL method of m_lpGather.reset(lpGather), but
	// vc6 doesn't support it, so do it the other way:
	std::auto_ptr<TSPIDIGITGATHER> myGather(lpGather);  // non-temp for SXL.
	CEnterCode sLock(this);  // Synch access to object
    m_lpGather = myGather;
	sLock.Unlock();

	// Make sure we see timer events.
	GetSP()->AddTimedCall(this);
        
    return false;

}// CTSPICallAppearance::GatherDigits

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GenerateDigits
//
// Initiates the generation of the specified digits on the specified 
// call as inband tones using the specified signaling mode. Invoking this 
// function with a NULL value for lpszDigits aborts any digit generation 
// currently in progress.  Invoking lineGenerateDigits or lineGenerateTone 
// while digit generation is in progress aborts the current digit generation 
// or tone generation and initiates the generation of the most recently 
// specified digits or tone. 
//
LONG CTSPICallAppearance::GenerateDigits (DWORD dwEndToEndID, DWORD dwDigitMode, 
				LPCTSTR lpszDigits, DWORD dwDuration)
{                                  
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_GENERATEDIGITS) == 0)
		return LINEERR_OPERATIONUNAVAIL;
    
    CTSPILineConnection* pLine = GetLineOwner();
	TString strDigits = (lpszDigits != NULL) ? lpszDigits : _T("");

    // If we cannot detect the type of digits requested, return an error.
    if ((dwDigitMode & pLine->GetLineDevCaps()->dwGenerateDigitModes) != dwDigitMode)
        return LINEERR_INVALDIGITMODE;

    // Adjust the duration to the nearest available.
    if (dwDuration < pLine->GetLineDevCaps()->MinDialParams.dwDigitDuration)
        dwDuration = pLine->GetLineDevCaps()->MinDialParams.dwDigitDuration;
    else if (dwDuration > pLine->GetLineDevCaps()->MaxDialParams.dwDigitDuration)
        dwDuration = pLine->GetLineDevCaps()->MaxDialParams.dwDigitDuration;

	// Check the digit mode - don't check the digits themselves, the TAPI specification
	// indicates that invalid digits are to be ignored.
	if (dwDigitMode != LINEDIGITMODE_PULSE && dwDigitMode != LINEDIGITMODE_DTMF)
		return LINEERR_INVALPARAM;
    
    // Remove any existing generate digit requests.
    pLine->RemovePendingRequests(this, REQUEST_GENERATEDIGITS);
    
    // Submit the request to generate the digits.
    if (AddAsynchRequest(new RTGenerateDigits(this, dwEndToEndID, dwDigitMode, lpszDigits, dwDuration)))
        return false;
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::GenerateDigits

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GenerateTone
//
// Generates the specified inband tone over the specified call. Invoking 
// this function with a zero for dwToneMode aborts the tone generation 
// currently in progress on the specified call. Invoking lineGenerateTone or
// lineGenerateDigits while tone generation is in progress aborts the current 
// tone generation or digit generation and initiates the generation of 
// the newly specified tone or digits.
//
LONG CTSPICallAppearance::GenerateTone (DWORD dwEndToEndID, DWORD dwToneMode, DWORD dwDuration,
		DWORD dwNumTones, LPLINEGENERATETONE lpTones)
{                                     
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_GENERATETONE) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    CTSPILineConnection* pLine = GetLineOwner();

    // If we cannot detect the type of digits requested, return an error.
    if ((dwToneMode & pLine->GetLineDevCaps()->dwGenerateToneModes) != dwToneMode)
        return LINEERR_INVALTONEMODE;

    // If a custom tone is specified, verify that it is within the parameters according
    // to our line.
    if (dwNumTones > 0)
    {
        if (dwNumTones > pLine->GetLineDevCaps()->dwGenerateToneMaxNumFreq)
            return LINEERR_INVALTONE;
    }

    // If there are any custom tone blocks, copy them over in an array.
	TGenerateToneArray* parrTones = new TGenerateToneArray;
    while (dwNumTones-- > 0)
    {   
        LPLINEGENERATETONE lpMyTone = new LINEGENERATETONE;
        if (lpMyTone == NULL)
        {
            delete parrTones;
            return LINEERR_NOMEM;
        }
        
        MoveMemory (lpMyTone, lpTones, sizeof(LINEGENERATETONE));
		try
		{
			parrTones->push_back(lpMyTone);
		}
		catch(...)
		{
			delete parrTones;
			delete lpMyTone;
			return LINEERR_NOMEM;
		}
    }

    // Remove any existing generate tone requests.
    pLine->RemovePendingRequests(this, REQUEST_GENERATETONE);

    // Submit the request to the worker thread.
    if (AddAsynchRequest(new RTGenerateTone(this, dwEndToEndID, dwToneMode, dwDuration, parrTones)))
        return false;
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::GenerateTone

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::MonitorDigits
//
// Enables and disables the unbuffered detection of digits received on the 
// call. Each time a digit of the specified digit mode(s) is detected, a 
// message is sent to the application indicating which digit has been detected.
//
LONG CTSPICallAppearance::MonitorDigits (DWORD dwDigitModes)
{   
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_MONITORDIGITS) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Validate the call state.                             
    if ((GetCallState() & (LINECALLSTATE_IDLE | LINECALLSTATE_DISCONNECTED)) != 0)
        return LINEERR_INVALCALLSTATE;
#endif

    CTSPILineConnection* pLine = GetLineOwner();

    // If we cannot detect the type of digits requested, return an error.
    if ((dwDigitModes & pLine->GetLineDevCaps()->dwMonitorDigitModes) != dwDigitModes)
        return LINEERR_INVALDIGITMODE;
    
    // Assign the digit modes detected.
    SetDigitMonitor (dwDigitModes);
    return false;

}// CTSPICallAppearance::MonitorDigits

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::MonitorMedia
//
// Enables and disables the detection of media modes on the specified call. 
// When a media mode is detected, a message is sent to the application.
//
LONG CTSPICallAppearance::MonitorMedia (DWORD dwMediaModes)
{                                    
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_MONITORMEDIA) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Validate the call state.                             
    if (GetCallState() == LINECALLSTATE_IDLE)
        return LINEERR_INVALCALLSTATE;
#endif

    CTSPILineConnection* pLine = GetLineOwner();

    // If we cannot detect the type of media requested, return an error.
    if ((dwMediaModes & pLine->GetLineDevCaps()->dwMediaModes) != dwMediaModes)
        return LINEERR_INVALMEDIAMODE;
    
    // Assign the digit modes detected.
    SetMediaMonitor(dwMediaModes);
    return false;

}// CTSPICallAppearance::MonitorMedia

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::MonitorTones
//
// Enables and disables the detection of inband tones on the call. Each 
// time a specified tone is detected, a message is sent to the application. 
//
LONG CTSPICallAppearance::MonitorTones (TSPITONEMONITOR* lpMon)
{   
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_MONITORTONES) == 0)
		return LINEERR_OPERATIONUNAVAIL;
    
#ifdef STRICT_CALLSTATES
    // Validate the call state.                             
    if (GetCallState() == LINECALLSTATE_IDLE)
        return LINEERR_INVALCALLSTATE;
#endif
    
    // Validate the tone monitor list.
    CTSPILineConnection* pLine = GetLineOwner();
                                        
    // If it is disabled in the line device capabilities.                                        
    if (pLine->GetLineDevCaps()->dwMonitorToneMaxNumFreq == 0)
        return LINEERR_OPERATIONUNAVAIL;

	CEnterCode sLock(this);  // Synch access to object

    // If this entry already exists, locate it.
    unsigned int iPos = 0xffffffff, i;
    for (i = 0; i < m_arrMonitorTones.size(); i++)
    {
        TSPITONEMONITOR* lpMyMon = m_arrMonitorTones[i];
        if (lpMyMon->dwToneListID == lpMon->dwToneListID)
        {
            iPos = i;
            break;
        }  
    }          
    
    // If this is a request to turn off tone monitoring for this tone list ID,
    // then remove the tone from our list.
    if (lpMon->arrTones.empty())
    {   
    	if (iPos != 0xffffffff)
    	{
        	TSPITONEMONITOR* lpMyMon = m_arrMonitorTones[iPos];
       		m_arrMonitorTones.erase(m_arrMonitorTones.begin() + iPos);
        	delete lpMyMon;
        	delete lpMon;
        	return false;
		}
		return LINEERR_INVALTONELIST;        	
    }

    // Otherwise, validate the tone list.    
    if (lpMon->arrTones.size() > pLine->GetLineDevCaps()->dwMonitorToneMaxNumEntries)
        return LINEERR_INVALTONE;
        
    // Verify the count of tones in each of the tone lists.
    for (i = 0; i < lpMon->arrTones.size(); i++)
    {
        LPLINEMONITORTONE lpTone = reinterpret_cast<LPLINEMONITORTONE>(lpMon->arrTones[i]);
        int iFreqCount = 0;
        if (lpTone->dwFrequency1 > 0)
            iFreqCount++;
        if (lpTone->dwFrequency2 > 0)
            iFreqCount++;
        if (lpTone->dwFrequency3 > 0)
            iFreqCount++;
        if (iFreqCount > static_cast<int>(pLine->GetLineDevCaps()->dwMonitorToneMaxNumFreq))
            return LINEERR_INVALTONE;
    }       
    
    // Looks ok, insert it into our list of detectable tones.
    // If it already existed, remove it first - this will replace the entry.
    if (iPos >= 0)
    {
        TSPITONEMONITOR* lpMyMon = m_arrMonitorTones[iPos];
        m_arrMonitorTones.erase(m_arrMonitorTones.begin() + iPos);
        delete lpMyMon;
    }

	try
	{
		m_arrMonitorTones.push_back(lpMon);
	}
	catch(...)
	{
		delete lpMon;
		return LINEERR_NOMEM;
	}

    return 0;
        
}// CTSPICallAppearance::MonitorTones

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SwapHold
//
// Swap this active call with another call on some type of
// hold
//
LONG CTSPICallAppearance::SwapHold(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SWAPHOLD) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_SWAPHOLD) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
    // Make sure the held call is really onHold.
    if ((pCall->GetCallState() & 
				(LINECALLSTATE_ONHOLDPENDTRANSFER |
				 LINECALLSTATE_ONHOLDPENDCONF |
				 LINECALLSTATE_ONHOLD)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif

    // Everything seems ok, submit the accept request.
    if (AddAsynchRequest(new RTSwapHold(this, dwRequestID, pCall)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;
    
}// CTSPICallAppearance::SwapHold

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::ReleaseUserUserInfo
//
// Submit a request to release user-user information.
//
LONG CTSPICallAppearance::ReleaseUserUserInfo(DRV_REQUESTID dwRequestID)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_RELEASEUSERUSERINFO) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Pass the request through to the service provider in the 
	// request list in case it stores the USERUSER info somewhere
	// in the hardware.
    if (AddAsynchRequest(new RTReleaseUserInfo(this, dwRequestID)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;
	
}// CTSPICallAppearance::ReleaseUserUserInfo

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::ReleaseUserUserInfo
//
// Release a block of USER-USER information in our CALLINFO record.
//
void CTSPICallAppearance::ReleaseUserUserInfo()
{
	// Delete the first entry in our UUI array.
	CEnterCode sLock(this);

	if (!m_arrUserUserInfo.empty())
	{
		// Delete the first entry.
		m_arrUserUserInfo.erase(m_arrUserUserInfo.begin());

		// Send out a notification about the change in user information.
		if (!m_arrUserUserInfo.empty())
			OnCallInfoChange (LINECALLINFOSTATE_USERUSERINFO);

		// Or remove the UUI flag if we have no more.
		else
			m_CallStatus.dwCallFeatures	&= ~LINECALLFEATURE_RELEASEUSERUSERINFO;
	}

}// CTSPICallAppearance::ReleaseUserUserInfo

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnReceivedUserUserInfo
//
// Received a block of user-user information from the network.
// Store it inside our call appearance.
//    
void CTSPICallAppearance::OnReceivedUserUserInfo (const void* lpBuff, DWORD dwSize)
{                                              
	if (lpBuff != NULL && dwSize > 0)
	{
		SIZEDDATA* pInfo = new SIZEDDATA(lpBuff, dwSize);

		// Add it to our array.
		CEnterCode sLock(this);  // Synch access to object
		m_arrUserUserInfo.push_back(pInfo);

    	if (GetAddressOwner()->GetAddressCaps()->dwCallFeatures & LINECALLFEATURE_RELEASEUSERUSERINFO)
			m_CallStatus.dwCallFeatures	|= LINECALLFEATURE_RELEASEUSERUSERINFO;

		// Send out a notification about the change in user information.
		OnCallInfoChange (LINECALLINFOSTATE_USERUSERINFO);
	}                      

}// CTSPICallAppearance::OnReceivedUserUserInfo

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallData
//
// Sets calldata into our CALLINFO record.  TAPI is notified of the
// change, and depending on the implementation, the calldata should
// be propagated to all systems which have a copy of this data.
//
LONG CTSPICallAppearance::SetCallData (DRV_REQUESTID dwRequestID, 
									   LPVOID lpCallData, DWORD dwSize)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETCALLDATA) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
	// Don't allow on IDLE calls.
	if (GetCallState() == LINECALLSTATE_IDLE)
		return LINEERR_INVALCALLSTATE;
#endif

	// If the call data is too big, error it out.
	if (dwSize > GetAddressOwner()->GetAddressCaps()->dwMaxCallDataSize)
		return LINEERR_INVALPARAM;

	// Submit the request.  The AsynchReply will copy the data if successfull.
    if (AddAsynchRequest(new RTSetCallData(this, dwRequestID, lpCallData, dwSize)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}//	CTSPICallAppearance::SetCallData

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallID
//
// In some telephony environments, the switch or service provider may 
// assign a unique identifier to each call. This allows the call to be 
// tracked across transfers, forwards, or other events.  The field is
// not used in the base class and is available for derived service
// providers to use.
//
void CTSPICallAppearance::SetCallID (DWORD dwCallID)
{
	CEnterCode sLock(this);

	// If our callid is changing...
	if (dwCallID != m_CallInfo.dwCallID)
	{
		_TSP_DTRACE(_T("Setting callid for Call 0x%lx, old=0x%lx, new=0x%lx\n"),
				this, m_CallInfo.dwCallID, dwCallID);

		// Remove it from the current callhub map
		CTSPICallHub* pOldHub = GetCallHub();
		CTSPICallHub* pNewHub = GetLineOwner()->GetDeviceInfo()->GetCallHubFromMap(dwCallID);

		// Set our call id and set our new hub pointer
		m_CallInfo.dwCallID = dwCallID;
		m_pCallHub = pNewHub;

		// Unlock this call object now so aren't holding two locked
		// objects (hub and call).
		sLock.Unlock();

		// Add ourselves to the new hub
		if (pNewHub != NULL) pNewHub->AddToHub(this);

		// Remove ourselves from the old hub
		if (pOldHub != NULL) pOldHub->RemoveFromHub(this);

		// Notify TAPI that our callid has changed
		OnCallInfoChange(LINECALLINFOSTATE_CALLID);
	}

}// CTSPICallAppearance::SetCallID

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetRelatedCallID
//
// Telephony environments that use the call ID often may find it 
// necessary to relate one call to another. The dwRelatedCallID field 
// may be used by the service provider for this purpose.  The field
// is not used in the base class and is available for derived service 
// providers to use.
//
void CTSPICallAppearance::SetRelatedCallID (DWORD dwCallID)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwRelatedCallID != dwCallID)
	{
		m_CallInfo.dwRelatedCallID = dwCallID;
		OnCallInfoChange(LINECALLINFOSTATE_RELATEDCALLID);
	}

}// CTSPICallAppearance::SetRelatedCallID

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetMediaControl
//
// This function enables and disables control actions on the media stream 
// associated with the specified line, address, or call. Media control 
// actions can be triggered by the detection of specified digits, media modes, 
// custom tones, and call states.  The new specified media controls replace 
// all the ones that were in effect for this line, address, or call prior 
// to this request.
//
void CTSPICallAppearance::SetMediaControl(TSPIMEDIACONTROL* lpMediaControl)
{   
	CEnterCode sLock(this);  // Synch access to object

	// Ignore this if it is the same as we have seen already.
	if (m_lpMediaControl == lpMediaControl)
		return;

	// If we already have media control information, then decrement it.
	// This will auto-delete it if we are the last one using this data.
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->DecUsage();  

	// Assign the new media control and increment it's usage count.
    m_lpMediaControl = lpMediaControl;    
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->IncUsage();
        
}// CTSPICallAppearance::SetMediaControl

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnToneMonitorDetect
//
// A monitored tone has been detected, inform TAPI.
//
void CTSPICallAppearance::OnToneMonitorDetect (DWORD dwToneListID, DWORD dwAppSpecific)
{                                           
	// If the call isn't valid anymore, exit.
	CEnterCode sLock(this);
	if (GetCallHandle() == NULL)
		return;

    CTSPILineConnection* pLine = GetLineOwner();
    if (pLine != NULL)
        pLine->Send_TAPI_Event(this, LINE_MONITORTONE, dwAppSpecific, dwToneListID, GetTickCount());

}// CTSPICallAppearance::OnToneMonitorDetect

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::OnMediaControl
//
// This method is called when a media control event was activated
// due to a media monitoring event being caught on this call.
//
void CTSPICallAppearance::OnMediaControl (DWORD dwMediaControl)
{                                      
	// Default behavior is to send it up to the line owner object
	// and let it be processed there.  This function may be overriden
	// to process at the call level.
	GetLineOwner()->OnMediaControl(this, dwMediaControl);

}// CTSPICallAppearance::OnMediaControl

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetID
//
// Manage device-level requests for information based on a device id.
//
LONG CTSPICallAppearance::GetID (const TString& strDevClass, 
								LPVARSTRING lpDeviceID, HANDLE hTargetProcess)
{
	DEVICECLASSINFO* pDeviceClass = GetDeviceClass(strDevClass.c_str());
	if (pDeviceClass == NULL)
		return GetAddressOwner()->GetID(strDevClass, lpDeviceID, hTargetProcess);
	return GetSP()->CopyDeviceClass (pDeviceClass, lpDeviceID, hTargetProcess);
    
}// CTSPICallAppearance::GetID

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallTreatment
//
// The specified call treatment value is stored off in the
// LINECALLINFO record and TAPI is notified of the change.  If
// the call is currently in a state where the call treatment is
// relevent, then it goes into effect immediately.  Otherwise,
// the treatment will take effect the next time the call enters a
// relevent state.
//
void CTSPICallAppearance::SetCallTreatment(DWORD dwCallTreatment)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwCallTreatment != dwCallTreatment)
	{
		m_CallInfo.dwCallTreatment = dwCallTreatment;
		OnCallInfoChange (LINECALLINFOSTATE_TREATMENT);
	}

}// CTSPICallAppearance::SetCallTreatment

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallParameterFlags
//
// Specifies a collection of call-related parameters when the call is 
// outbound. These are same call parameters specified in lineMakeCall, of 
// type LINECALLPARAMFLAGS_.  Note that whenever you call this function
// to adjust a flag setting, retrieve the setting first and add your
// flags since they are REPLACED with this call.
//
void CTSPICallAppearance::SetCallParameterFlags (DWORD dwFlags)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwCallParamFlags != dwFlags)
	{
		m_CallInfo.dwCallParamFlags = dwFlags;
		OnCallInfoChange(LINECALLINFOSTATE_OTHER);
	}

}// CTSPICallAppearance::SetCallParameterFlags

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetDigitMonitor
//
// Specifies the various digit modes for which monitoring is 
// currently enabled, of type LINEDIGITMODE_.
//
void CTSPICallAppearance::SetDigitMonitor(DWORD dwDigitModes)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwMonitorDigitModes != dwDigitModes)
	{
		m_CallInfo.dwMonitorDigitModes = dwDigitModes;
		OnCallInfoChange(LINECALLINFOSTATE_MONITORMODES);
	}

}// CTSPICallAppearance::SetDigitMonitor

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetMediaMonitor
//
// Specifies the various media modes for which monitoring is currently 
// enabled, of type LINEMEDIAMODE_.
//
void CTSPICallAppearance::SetMediaMonitor(DWORD dwModes)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwMonitorMediaModes != dwModes)
	{
		m_CallInfo.dwMonitorMediaModes = dwModes;
		OnCallInfoChange(LINECALLINFOSTATE_MONITORMODES);
	}

}// CTSPICallAppearance::SetMediaMonitor

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetDialParameters
//
// Specifies the dialing parameters currently in effect on the call, of 
// type LINEDIALPARAMS. Unless these parameters are set by either 
// lineMakeCall or lineSetCallParams, their values will be the same as the 
// defaults used in the LINEDEVCAPS.
//
void CTSPICallAppearance::SetDialParameters (LPLINEDIALPARAMS pdp)
{
	CEnterCode sLock(this);  // Synch access to object
	if (memcmp(&m_CallInfo.DialParams, pdp, sizeof(LINEDIALPARAMS)) != 0)
	{
		MoveMemory (&m_CallInfo.DialParams, pdp, sizeof(LINEDIALPARAMS));
		OnCallInfoChange(LINECALLINFOSTATE_DIALPARAMS);
	}

}// CTSPICallAppearance::SetDialParameters

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallOrigin
//
// This function sets the call origin for the call appearance and
// sends a TAPI event indicating that the origin has changed.
//
void CTSPICallAppearance::SetCallOrigin(DWORD dwOrigin)
{
	CEnterCode sLock(this);  // Synch access to object
	if (m_CallInfo.dwOrigin != dwOrigin)
	{
		m_CallInfo.dwOrigin = dwOrigin; 
		OnCallInfoChange(LINECALLINFOSTATE_ORIGIN);
	}

}// CTSPICallAppearance::SetCallOrigin

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallReason
//
// Specifies the reason why the call occurred. This field uses the 
// LINECALLREASON_ constants.
//
void CTSPICallAppearance::SetCallReason(DWORD dwReason)
{
	CEnterCode sLock(this);  // Synch access to object
	if (m_CallInfo.dwReason != dwReason)
	{
		m_CallInfo.dwReason = dwReason; 
		OnCallInfoChange(LINECALLINFOSTATE_REASON);
	}

}// CTSPICallAppearance::SetCallReason

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetDestinationCountry
//
// The country code of the destination party. Zero if unknown.
//
void CTSPICallAppearance::SetDestinationCountry (DWORD dwCountryCode)
{
	CEnterCode sLock(this);  // Synch access to object
	if (m_CallInfo.dwCountryCode != dwCountryCode)
	{
		m_CallInfo.dwCountryCode = dwCountryCode;
		OnCallInfoChange(LINECALLINFOSTATE_OTHER);
	}

}// CTSPICallAppearance::SetDestinationCountry

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetTrunkID
//
// The number of the trunk over which the call is routed. This field 
// is used for both inbound and outgoing calls. It should be set to 
// 0xFFFFFFFF if it is unknown.
//
void CTSPICallAppearance::SetTrunkID (DWORD dwTrunkID)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwTrunk != dwTrunkID)
	{
		m_CallInfo.dwTrunk = dwTrunkID;
		OnCallInfoChange(LINECALLINFOSTATE_TRUNK);
	}

}// CTSPICallAppearance::SetTrunkID

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetQualityOfService
//
// Re-negotiates the quality of service (QOS) on the call with the
// switch.  If the desired QOS is not available, then the function
// fails but the call continues with the previous QOS.
//
LONG CTSPICallAppearance::SetQualityOfService (DRV_REQUESTID dwRequestID, 
	LPVOID lpSendingFlowSpec, DWORD dwSendingFlowSpecSize,
	LPVOID lpReceivingFlowSpec, DWORD dwReceivingFlowSpecSize)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETQOS) == 0)
		return LINEERR_OPERATIONUNAVAIL;

#ifdef STRICT_CALLSTATES
	// Don't allow on IDLE calls.
	if (GetCallState() == LINECALLSTATE_IDLE)
		return LINEERR_INVALCALLSTATE;
#endif

	// Submit the request.  The AsynchReply will copy the data if successfull.
    if (AddAsynchRequest(new RTSetQualityOfService(this, dwRequestID,
							lpSendingFlowSpec, dwSendingFlowSpecSize, 
							lpReceivingFlowSpec, dwReceivingFlowSpecSize)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::SetQualityOfService

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetQualityOfService
//
// Re-negotiates the quality of service (QOS) on the call with the
// switch.  If the desired QOS is not available, then the function
// fails but the call continues with the previous QOS.
//
void CTSPICallAppearance::SetQualityOfService(
				LPCVOID lpSendingFlowSpec, DWORD dwSendingFlowSpecSize,
				LPCVOID lpReceivingFlowSpec, DWORD dwReceivingFlowSpecSize)
{
	CEnterCode sLock(this);  // Synch access to object

	// Set the new data in place
	m_sdSendingFS.SetPtr(lpSendingFlowSpec, dwSendingFlowSpecSize);
	m_sdReceivingFS.SetPtr(lpReceivingFlowSpec, dwReceivingFlowSpecSize);

	// Tell TAPI of the change.
	OnCallInfoChange (LINECALLINFOSTATE_QOS);

}// CTSPICallAppearance::SetQualityOfService

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallTreatment
//
// The specified call treatment value is stored off in the
// LINECALLINFO record and TAPI is notified of the change.  If
// the call is currently in a state where the call treatment is
// relevent, then it goes into effect immediately.  Otherwise,
// the treatment will take effect the next time the call enters a
// relevent state.
//
LONG CTSPICallAppearance::SetCallTreatment(DRV_REQUESTID dwRequestID, DWORD dwCallTreatment)
{
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETTREATMENT) == 0 ||
		 GetLineOwner()->FindRequest(this, REQUEST_SETCALLTREATMENT) != NULL)
		return LINEERR_OPERATIONUNAVAIL;

	// Verify that the call treatment is valid.
	if (GetAddressOwner()->GetCallTreatmentName(dwCallTreatment) == _T(""))
		return LINEERR_INVALPARAM;
	
	// Submit a request for the derived provider to handle.
	if (AddAsynchRequest(new RTSetCallTreatment(this, dwRequestID, dwCallTreatment)))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;

}// CTSPICallAppearance::SetCallTreatment

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::DevSpecific
//
// Called routine for lineDevSpecific processing associated with
// this call.
//
LONG CTSPICallAppearance::DevSpecific(DRV_REQUESTID, LPVOID, DWORD)
{
	return LINEERR_OPERATIONUNAVAIL;

}// CTSPICallAppearance::DevSpecific

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CreateConsultationCall
//
// Creates a new consultation call for this call. It can be located from
// the existing object using the "GetConsultationCall" method.
//
CTSPICallAppearance* CTSPICallAppearance::CreateConsultationCall(HTAPICALL htCall, DWORD dwCallParamFlags)
{
    CTSPICallAppearance* pCall = NULL;

    // See if the call appearance already exists.
    if (htCall != NULL)
    {
        pCall = GetAddressOwner()->FindCallByHandle (htCall);
        if (pCall)
			return pCall;
    }

    // Create the call appearance
    if ((pCall = GetSP()->CreateCallObject()) == NULL)
        return NULL;

    // Init the call appearance (1st phase)
    pCall->Init(GetAddressOwner(), GetCallInfo()->dwBearerMode, GetCallInfo()->dwRate, dwCallParamFlags, 
		LINECALLORIGIN_OUTBOUND, LINECALLREASON_DIRECT, 0xffffffff,	0);

    // Add it to the address list list.
	CTSPIAddressInfo* pAddress = GetAddressOwner();
	CEnterCode sLock(pAddress);  // Synch access to object
	try
	{
		pAddress->m_lstCalls.push_back(pCall);
	}
	catch (...)
	{
		delete pCall;
		throw;
	}
	sLock.Unlock();

    // If we don't have a call handle, then ask TAPI for one.
    if (htCall == NULL && 
		GetAddressOwner()->GetAddressCaps()->dwAddressSharing != LINEADDRESSSHARING_MONITORED)
    {
        DWORD dwTapiCall = 0;
        GetLineOwner()->Send_TAPI_Event(NULL, LINE_NEWCALL, reinterpret_cast<DWORD>(pCall), reinterpret_cast<DWORD>(&dwTapiCall));
        if (dwTapiCall != 0)
            htCall = reinterpret_cast<HTAPICALL>(dwTapiCall);
    }

	// Now set the call handle in place
	pCall->SetCallHandle(htCall);

    _TSP_DTRACEX(TRC_CALLS, _T("CreateConsultationCall: SP call=0x%lx, TAPI call=0x%lx\n"), pCall, htCall);

	// Now set the consultation status up with this call.
	SetConsultationCall(pCall);

    // Notify the line/address in case a derived class wants to know.
    pAddress->OnCreateCall(pCall);

    return pCall;

}// CTSPICallAppearance::CreateConsultationCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::CreateCallHandle
//
// This function is used to re-associate a call handle with an existing
// call appearance.  It causes TAPI to recognize the call's existance.
//
bool CTSPICallAppearance::CreateCallHandle()
{
	// Validate the call.
	if (GetCallHandle() != NULL || GetCallState() == LINECALLSTATE_IDLE)
		return true;

	// Get a call handle for this call.
    DWORD dwTapiCall = 0;
    GetLineOwner()->Send_TAPI_Event(NULL, LINE_NEWCALL, reinterpret_cast<DWORD>(this), reinterpret_cast<DWORD>(&dwTapiCall));
    if (dwTapiCall != 0)
	{
		// Assign the new call handle and update TAPI on it's state
		m_htCall = reinterpret_cast<HTAPICALL>(dwTapiCall);
		m_CallStatus.dwCallFeatures = 0;
		RecalcCallFeatures();
	}
	else
	{
		_TSP_DTRACE(_T("(LINE_NEWCALL failed) TAPI failed to generate a call handle for 0x%lx\n"), this);
		return false;
	}

	return true;

}// CTSPICallAppearance::CreateCallHandle

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::NotifyCallStatusChanged
//
// Function to tell TAPI that something in our callstatus structure
// has changed.
//  
void CTSPICallAppearance::NotifyCallStatusChanged()
{
	// No call handle?  TAPI doesn't care..
	if (GetCallHandle() == NULL)
		return;

	CTSPILineConnection* pLine = GetLineOwner();
	DWORD dwVersion = pLine->GetNegotiatedVersion();
	if (dwVersion == 0)
		dwVersion = GetSP()->GetSupportedVersion();

	// Determine what the parameters are for this callstate.
	DWORD dwP2 = 0;
	switch (m_CallStatus.dwCallState)
	{   
		case LINECALLSTATE_CONNECTED:
			dwP2 = m_CallStatus.dwCallStateMode;
			if (dwVersion < TAPIVER_14)
				dwP2 = 0;
			else if (dwVersion < TAPIVER_20 &&
				(dwP2 & (LINECONNECTEDMODE_ACTIVEHELD |
					LINECONNECTEDMODE_INACTIVEHELD |
					LINECONNECTEDMODE_CONFIRMED)))
				dwP2 = LINECONNECTEDMODE_ACTIVE;
			break;

		case LINECALLSTATE_DISCONNECTED:
			dwP2 = m_CallStatus.dwCallStateMode;
			if ((dwVersion < TAPIVER_14) &&
				dwP2 == LINEDISCONNECTMODE_NODIALTONE)
				dwP2 = LINEDISCONNECTMODE_UNKNOWN;
			else if (dwVersion < TAPIVER_20 &&
				(dwP2 & (LINEDISCONNECTMODE_NUMBERCHANGED |
					LINEDISCONNECTMODE_OUTOFORDER |
					LINEDISCONNECTMODE_TEMPFAILURE |
					LINEDISCONNECTMODE_QOSUNAVAIL |
					LINEDISCONNECTMODE_BLOCKED |
					LINEDISCONNECTMODE_DONOTDISTURB |
					LINEDISCONNECTMODE_CANCELLED)))
				dwP2 = LINEDISCONNECTMODE_UNKNOWN;
			break;

		case LINECALLSTATE_BUSY:
		case LINECALLSTATE_DIALTONE:
		case LINECALLSTATE_SPECIALINFO:
		case LINECALLSTATE_OFFERING:
			dwP2 = m_CallStatus.dwCallStateMode;
			break;

		// TAPI 1.4 extension
		case LINECALLSTATE_CONFERENCED:               
			// If the call was CREATED by us (i.e. a new call) which
			// TAPI may not know the conference owner of, then send the
			// conference owner in dwP2.
			{CTSPIConferenceCall* pConf = GetConferenceOwner();
			_TSP_ASSERTE(pConf != NULL && pConf->GetCallType() == Conference);
			dwP2 = reinterpret_cast<DWORD>(pConf->GetCallHandle());
			break;}
        
		default:
			break;
	}

	pLine->Send_TAPI_Event(this, LINE_CALLSTATE, m_CallStatus.dwCallState, dwP2, m_CallInfo.dwMediaMode);

}// CTSPICallAppearance::NotifyCallStatusChanged

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallIDs
//
// This function retrieves call-id information for this call. It is used
// by TAPI as a quick way to retrieve call-id information rather than
// using the full lineGetCallInfo function.
//
// It requires TAPI 2.2 or 3.0 negotiation.
//
LONG CTSPICallAppearance::GetCallIDs(LPDWORD lpdwAddressID, LPDWORD lpdwCallID, LPDWORD lpdwRelatedCallID)
{
	CEnterCode sLock(this);
	*lpdwAddressID = GetAddressOwner()->GetAddressID();
	*lpdwCallID = m_CallInfo.dwCallID;
	*lpdwRelatedCallID = m_CallInfo.dwRelatedCallID;
	return 0;

}// CTSPICallAppearance::GetCallIDs

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::ReceiveMSPData
//
// This function receives data sent by a media service provider (MSP).
// It requires TAPI 3.0 negotiation.
//
LONG CTSPICallAppearance::ReceiveMSPData(CMSPDriver* /*pMSP*/, LPVOID /*lpData*/, DWORD /*dwSize*/)
{
	// This causes the TSPI_lineReceiveMSPData to be routed to the line
	return LINEERR_OPERATIONUNAVAIL;

}// CTSPICallAppearance::ReceiveMSPData

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallStateName
//
// Returns a string indicating the current state of the connection
//
LPCTSTR CTSPICallAppearance::GetCallStateName (DWORD dwState) const
{
    switch((dwState) ? dwState : GetCallState())
    {
        case LINECALLSTATE_IDLE:		return _T("Idle");
        case LINECALLSTATE_OFFERING:	return _T("Offering");
        case LINECALLSTATE_ACCEPTED:    return _T("Accepted");
        case LINECALLSTATE_DIALTONE:    return _T("Dialtone");
        case LINECALLSTATE_DIALING:     return _T("Dialing");
        case LINECALLSTATE_RINGBACK:    return _T("Ringback");
        case LINECALLSTATE_BUSY:        return _T("Busy");
        case LINECALLSTATE_SPECIALINFO: return _T("SpecialInfo");
        case LINECALLSTATE_CONNECTED:   return _T("Connected");
        case LINECALLSTATE_PROCEEDING:  return _T("Proceeding");
        case LINECALLSTATE_ONHOLD:      return _T("OnHold");
        case LINECALLSTATE_CONFERENCED: return _T("Conferenced");
        case LINECALLSTATE_ONHOLDPENDCONF:     return _T("HoldPendConference");
        case LINECALLSTATE_ONHOLDPENDTRANSFER: return _T("HoldPendTransfer");
        case LINECALLSTATE_DISCONNECTED:       return _T("Disconnected");
    }
    return _T("Unknown");

}// CTSPICallAppearance::GetCallStateName

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::Dump
//
// Debug "dump" of the object and it's contents.
//
TString CTSPICallAppearance::Dump() const 
{
	TStringStream outstm;

	outstm << _T("0x") << hex << (DWORD)this;
	outstm << _T(",CallID=0x") << hex << m_CallInfo.dwCallID;
	outstm << _T(",LineOwner=0x") << hex << GetLineOwner()->GetPermanentDeviceID();
	outstm << _T(",htCall=0x") << hex << m_htCall;
	outstm << _T(",State=") << GetCallStateName();
	outstm << _T(",RefCnt=") << GetRefCount();
	if (m_pConsult) outstm << _T(",htConsult=0x") << hex << m_pConsult->m_htCall;
	if (m_pConf)	outstm << _T(",htConference=0x") << hex << m_pConf->m_htCall;
    return(outstm.str());

}// CTSPICallAppearance::Dump

#endif // _DEBUG

