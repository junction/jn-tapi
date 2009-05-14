/******************************************************************************/
//                                                                        
// REQUEST.CPP - Source file for the TSPIRequest base class               
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code to manage the asynchronous request     
// objects which are dynamically allocated for each TSP request generated 
// by TAPI.                                                               
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

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::CTSPIRequest
//
// Constructor for the CTSPIRequest object
//
CTSPIRequest::CTSPIRequest(LPCTSTR pszType) : CTSPIBaseObject(),
	m_iReqType(0), m_iReqState(STATE_NOTPROCESSED), m_dwRequestId(0xffffffff),
	m_pConnOwner(NULL), m_pAddress(NULL), m_pCall(NULL), m_fResponseSent(false),
	m_lResult(-1), m_hevtWait(NULL), m_dwStateTime(0), m_pszType(pszType)
{
}// CTSPIRequest::CTSPIRequest()

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::CTSPIRequest
//
// Copy Constructor for the CTSPIRequest object
//
CTSPIRequest::CTSPIRequest(const CTSPIRequest& src) : CTSPIBaseObject(src)
{
    m_iReqState = src.m_iReqState;
    m_iReqType = src.m_iReqType;
    m_dwRequestId = src.m_dwRequestId;
    m_pConnOwner = src.m_pConnOwner;
	m_pAddress = src.m_pAddress;
    m_pCall = src.m_pCall;
    m_fResponseSent = src.m_fResponseSent;
    m_lResult = src.m_lResult;
	m_hevtWait = NULL;
	m_pszType = src.m_pszType;

	// Increment the CALL object reference count indicating that we have 
	// requests associated with it.
	if (m_pCall != NULL)
		m_pCall->AddRef();

}// CTSPIRequest::CTSPIRequest()

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::~CTSPIRequest
//
// Destructor for the request class
//
CTSPIRequest::~CTSPIRequest()
{
#ifdef _DEBUG        
    _TSP_DTRACEX(TRC_REQUESTS, _T("Destroying %s"), Dump().c_str());
#endif

	// Make sure all threads are unblocked
	UnblockThreads();

	// Release a reference on the call object this
	// request is attached to.
	if (m_pCall != NULL)
		m_pCall->DecRef();

}// CTSPIRequest::~CTSPIRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::Init
//
// Initialize a CTSPIRequest object for a line/address/call
//
void CTSPIRequest::Init(
CTSPILineConnection* pConn, // Connection for this request
CTSPIAddressInfo* pAddr,    // Address line for this request
CTSPICallAppearance* pCall, // Call appearance
int iReqType,				// Request type (REQTYPE_xxx)
DRV_REQUESTID dwReqId)      // Asynch request id from TAPI
{
	// Assign our owners based on the input available.
	if (pCall != NULL)
	{
		m_pCall = pCall;
		m_pConnOwner = m_pCall->GetLineOwner();
		m_pAddress = m_pCall->GetAddressOwner();

		// Increment the CALL object reference count indicating that we have 
		// requests associated with it.
		m_pCall->AddRef();
	}
	else if (pAddr != NULL)
	{
		m_pAddress = pAddr;
		m_pConnOwner = m_pAddress->GetLineOwner();
	}
	else
	{
		m_pConnOwner = pConn;
		_TSP_ASSERTE(m_pConnOwner != NULL);
	}

	// Assign the TYPE of request and the asynch request id.
    m_iReqType = iReqType;
    m_dwRequestId = dwReqId;

#ifdef _DEBUG        
    _TSP_DTRACEX(TRC_REQUESTS, _T("Created %s"), Dump().c_str());
#endif

}// TSPIRequest::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::Init
//
// Initialize a CTSPIRequest object for a phone request.
//
void CTSPIRequest::Init(
CTSPIPhoneConnection* pConn,	// Connection for this request
int iReqType,					// Request type (REQTYPE_xxx)
DRV_REQUESTID dwReqId)			// Asynch request id from TAPI
{
	m_pConnOwner = pConn;
	_TSP_ASSERTE(m_pConnOwner != NULL);
    m_iReqType = iReqType;
    m_dwRequestId = dwReqId;

#ifdef _DEBUG        
    _TSP_DTRACEX(TRC_REQUESTS, _T("Created %s"), Dump().c_str());
#endif

}// TSPIRequest::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::UnblockThreads
//
// Force any waiting threads to unblock and resume execution.
//
void CTSPIRequest::UnblockThreads()
{
	// If we have an event created with this request AND
	// it is currently not signaled then signal it to unblock
	// any waiters.
	CEnterCode sLock(this);
	if (m_hevtWait != NULL)
	{
		// If we potentially have threads waiting on this object,
		// then unblock them and give up our timeslice so we are guarenteed
		// that the other threads are awoken.
		if (WaitForSingleObject(m_hevtWait, 0) == WAIT_TIMEOUT)
		{
			SetEvent(m_hevtWait);
			sLock.Unlock();
			Sleep(0);
			sLock.Lock();
		}

		// Delete the event.
		CloseHandle(m_hevtWait);
		m_hevtWait = NULL;
	}

}// CTSPIRequest::UnblockThreads

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::Complete
//
// This causes the request to be completed.  It is invoked by 
// the device when the request is finished.
//
void CTSPIRequest::Complete (LONG lResult, bool fSendTapiNotification)
{
#ifdef _DEBUG
    _TSP_DTRACEX(TRC_REQUESTS, _T("Completing [0x%lx] %s"), lResult, Dump().c_str());
#endif

    // Mark this as completed.
	if (m_fResponseSent == false)
		m_fResponseSent = fSendTapiNotification;
    m_lResult = lResult;

    // Tell the connection, address, and call that the request completed.
	if (m_pConnOwner != NULL)
		m_pConnOwner->OnRequestComplete (this, lResult);
    if (m_pAddress != NULL)
        m_pAddress->OnRequestComplete (this, lResult);
    if (m_pCall != NULL)  
        m_pCall->OnRequestComplete (this, lResult);

	// If the request failed, call our virtual "failure" function
	if (lResult != 0)
		Failed(lResult);

    // If we have waiting threads, then unblock them and give them a chance to run.
	UnblockThreads();

}// CTSPIRequest::Complete

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::WaitForCompletion
//
// This function pauses the current thread waiting for this request
// to complete.
//
LONG CTSPIRequest::WaitForCompletion(DWORD dwMsecs)
{   
	// Wait until a final result is given.
	if (m_lResult == -1L)
	{
		// Create an event if necessary.  Make sure to unlock the
		// request before we actually block the thread.
		CEnterCode sLock(this);
		if (m_hevtWait == NULL)
			m_hevtWait = CreateEvent(NULL, TRUE, FALSE, NULL);
		sLock.Unlock();

		// Wait for the event to be set by the CTSPIRequest::Complete() function.
#ifdef _DEBUG
		_TSP_DTRACEX(TRC_THREADS, _T("Pausing thread 0x%lx on request %s"), GetCurrentThreadId(), Dump().c_str());
#endif
		if (WaitForSingleObject (m_hevtWait, dwMsecs) == WAIT_TIMEOUT) 
			return -1L;
	}
	return m_lResult;

}// CTSPIRequest::WaitForCompletion

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::EnterState
//
// Thread-safe entry for processing requests.  Returns true if the
// current state of the request is "iLookForState" and then sets the
// state to "iNextState" to stop any other threads from locking packet.
//
bool CTSPIRequest::EnterState(int iLookForState, int iNextState)
{
	_TSP_ASSERTE(iLookForState != iNextState);
	_TSP_ASSERTE(m_iReqState != 0xdddddddd); // Deleted request.

	CEnterCode sLock(this, false);
	if (sLock.Lock(50))
	{
		if (m_iReqState == iLookForState)
		{
			SetState(iNextState);
			return true;
		}
	}
	return false;

}// CTSPIRequest::EnterState

///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::Failed
//
// This is a virtual function called when a request fails.  It gives the
// request an opportunity to perform cleanup on failure.
//
void CTSPIRequest::Failed(LONG /*lResult*/)
{
	/* Do nothing */

}// CTSPIRequest::Failed

///////////////////////////////////////////////////////////////////////////
// RTForward::Failed
//
// Delete our call appearance when a forward request fails.
//
void RTForward::Failed(LONG /*lResult*/)
{
	// Delete the call if the forwarding request failed and we never
	// notified TAPI about call-state changes.
	CTSPICallAppearance* pConsult = GetConsultationCall();
	if (pConsult != NULL &&
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
		pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);

}// RTForward::Failed

///////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::Failed
//
// Delete our call appearance when the request fails.
//
void RTSetupTransfer::Failed(LONG /*lResult*/)
{
	// Delete the call if the transfer request failed and we never
	// notified TAPI about call-state changes.
	CTSPICallAppearance* pConsult = GetConsultationCall();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);               

}// RTSetupTransfer::Failed

///////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::Failed
//
// Delete our call appearance when the request fails.
//
void RTCompleteTransfer::Failed(LONG /*lResult*/)
{
	CTSPIConferenceCall* pConf = GetConferenceCall();
    if (pConf != NULL &&
		(pConf->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
   		pConf->GetAddressOwner()->RemoveCallAppearance(pConf);

	// Reset the related conference call of the other two calls.
	if (GetCallInfo() != NULL)
		GetCallInfo()->SetConferenceOwner(NULL);
	if (GetConsultationCall() != NULL)
		GetConsultationCall()->SetConferenceOwner(NULL);

}// RTCompleteTransfer::Failed

///////////////////////////////////////////////////////////////////////////
// RTPrepareAddToConference::Failed
//
// Delete our call appearance when the request fails.
//
void RTPrepareAddToConference::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetConsultationCall();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);               

}// RTPrepareAddToConference::Failed

///////////////////////////////////////////////////////////////////////////
// RTSetupConference::Failed
//
// Delete our call appearance when the request fails.
//
void RTSetupConference::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetConsultationCall();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);

	// Unattach our original call if present.
	pConsult = GetOriginalCall();
	if (pConsult != NULL)
		pConsult->SetConferenceOwner(NULL);

	// And idle the conference.
	CTSPIConferenceCall* pConf = GetConferenceCall();
    if (pConf != NULL &&
		(pConf->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
   		pConf->GetAddressOwner()->RemoveCallAppearance(pConf);
	else
		pConf->SetCallState(LINECALLSTATE_IDLE);

}// RTSetupConference::Failed

///////////////////////////////////////////////////////////////////////////
// RTMakeCall::Failed
//
// Delete our call appearance when the request fails.
//
void RTMakeCall::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetCallInfo();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);               

}// RTMakeCall::Failed

///////////////////////////////////////////////////////////////////////////
// RTPickup::Failed
//
// Delete our call appearance when the request fails.
//
void RTPickup::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetCallInfo();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);               

}// RTPickup::Failed

///////////////////////////////////////////////////////////////////////////
// RTUnpark::Failed
//
// Delete our call appearance when the request fails.
//
void RTUnpark::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetCallInfo();
    if (pConsult != NULL && 
		(pConsult->GetCallFlags() & CTSPICallAppearance::_InitNotify) == 0)
       	pConsult->GetAddressOwner()->RemoveCallAppearance(pConsult);               

}// RTUnpark::Failed

///////////////////////////////////////////////////////////////////////////
// RTDropCall::Failed
//
// Unmark the drop flag when the request fails.
//
void RTDropCall::Failed(LONG /*lResult*/)
{
	CTSPICallAppearance* pConsult = GetCallInfo();
	CEnterCode sLock (pConsult);
	pConsult->m_dwFlags &= ~CTSPICallAppearance::_IsDropped;

}// RTDropCall::Failed

///////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::Failed
//
// Cancel the generation request when it fails.
//
void RTGenerateDigits::Failed(LONG /*lResult*/)
{
	GetLineOwner()->Send_TAPI_Event(GetCallInfo(), 
		LINE_GENERATE, LINEGENERATETERM_CANCEL,
        GetIdentifier(), GetTickCount());

}// RTGenerateDigits::Failed

///////////////////////////////////////////////////////////////////////////
// RTGenerateTone::Failed
//
// Cancel the generation request when it fails.
//
void RTGenerateTone::Failed(LONG /*lResult*/)
{
	GetLineOwner()->Send_TAPI_Event(GetCallInfo(), LINE_GENERATE, LINEGENERATETERM_CANCEL, GetIdentifier(), GetTickCount());

}// RTGenerateDigits::Failed

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::~RTForward
//
// Destructor for the lineForward request object
//
RTForward::~RTForward()
{
	if (m_parrForwardInfo != NULL)
		std::for_each(m_parrForwardInfo->begin(), m_parrForwardInfo->end(), MEM_FUNV(&TSPIFORWARDINFO::DecUsage));
    FreeMem(m_lpCallParams);

}// RTForward::~RTForward

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTMakeCall::~RTMakeCall
//
// Destructor for the MakeCall event
//
RTMakeCall::~RTMakeCall()
{
	FreeMem(m_lpCallParams);

}// RTMakeCall::~RTMakeCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::~RTSetupConference
//
// Destructor for the RTSetupConference object
//
RTSetupConference::~RTSetupConference()
{
    FreeMem(m_lpCallParams);

}// RTSetupConference::~RTSetupConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::~RTSetupTransfer
//
// Destructor for the RTSetupTransfer object
//
RTSetupTransfer::~RTSetupTransfer()
{
    FreeMem(m_lpCallParams);

}// RTSetupTransfer::~RTSetupTransfer

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::~RTSetMediaControl
// 
// Request to manage the lineSetMediaControl event
//
RTSetMediaControl::~RTSetMediaControl()
{
	// Decrement the reference count on the media control structure
	if (m_pMediaControl != NULL)
		m_pMediaControl->DecUsage();

}// RTSetMediaControl::~RTSetMediaControl

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPrepareAddToConference::~RTPrepareAddToConference
//
// Constructor for the RTPrepareAddToConference object
//
RTPrepareAddToConference::~RTPrepareAddToConference()
{
    FreeMem(m_lpCallParams);

}// RTPrepareAddToConference::~RTPrepareAddToConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::~RTGenerateTone
//
// Destructor - delete tone list.
//
RTGenerateTone::~RTGenerateTone()
{
	delete m_parrTones;

}// RTGenerateTone::~RTGenerateTone

///////////////////////////////////////////////////////////////////////////
// RTSetAgentGroup::~RTSetAgentGroup
//
// Destructor for the lineSetAgentGroup request.
//
RTSetAgentGroup::~RTSetAgentGroup()
{
	delete m_parrGroups;

}// RTSetAgentGroup::~RTSetAgentGroup

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::~RTSetButtonInfo
//
// Destructor for the phoneSetButtonInfo request
//
RTSetButtonInfo::~RTSetButtonInfo()
{
	FreeMem(m_lpbi);

}// RTSetButtonInfo::~RTSetButtonInfo

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPIRequest::Dump
//
// Debug method to dump out the TSPI Request block
//
TString CTSPIRequest::Dump() const
{
	TStringStream outstm;
	outstm << _T("0x") << hex << reinterpret_cast<DWORD>(this) << _T(" ");

	LPCTSTR pszName = GetRequestName();
	if (pszName == NULL)
		pszName = _T("(Unknown Type)");

	outstm << _T("0x") << hex << m_iReqType << _T(" ") << pszName << endl;
	outstm << _T("  DRV_REQUESTID=0x") << hex << m_dwRequestId;
	outstm << _T(",RefCnt=") << GetRefCount();
	outstm << _T(",State=");
	
	switch (m_iReqState)
	{
		case STATE_INITIAL:			outstm << _T("Initial"); break;
		case STATE_IGNORE:			outstm << _T("Processing"); break;
		case STATE_NOTPROCESSED:	outstm << _T("Not Started"); break;
		case STATE_COMPLETED:		outstm << _T("Completed"); break;
		default:					outstm << _T("0x") << hex << m_iReqState; break;
	}

	outstm << _T(",ResponseSent=") << (m_fResponseSent) ? _T("Y") : _T("N");
	if (m_pCall != NULL) outstm << endl << _T("  Call:") << m_pCall->Dump();
	outstm << endl << _T("  Owner:") << m_pConnOwner->Dump() << endl;
	return (outstm.str());

}// CTSPIRequest::Dump

#endif // _DEBUG

