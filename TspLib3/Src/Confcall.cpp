/******************************************************************************/
//                                                                        
// CONFCALL.CPP - Source code for the CTSPIConferenceCall object.
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code for managing the conference call appearance       
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

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::CTSPIConferenceCall
//
// Conference call constructor
//
CTSPIConferenceCall::CTSPIConferenceCall() : CTSPICallAppearance()
{
	m_iCallType = Conference;

}// CTSPIConferenceCall::CTSPIConferenceCall

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::~CTSPIConferenceCall
//
// Destructor for the conference call
//
CTSPIConferenceCall::~CTSPIConferenceCall()
{
}// CTSPIConferenceCall::~CTSPIConferenceCall

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::PrepareAddToConference
//
// This method asks the address owner to create a new consultation
// call appearance and submits a Prepare request.
//
LONG CTSPIConferenceCall::PrepareAddToConference(DRV_REQUESTID dwRequestID, HTAPICALL htConsultCall, 
                                                 LPHDRVCALL lphdConsultCall, LPLINECALLPARAMS lpCallParams)
{   
	// Verify that the function can be called right now.
	if ((GetCallStatus()->dwCallFeatures & LINECALLFEATURE_PREPAREADDCONF) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // If we were passed line parameters, verify them and copy them into our internal
	// buffer.
    if (lpCallParams)
    {
        // Copy the call parameters into our own buffer.
	    LPLINECALLPARAMS lpMyCallParams = CopyCallParams(lpCallParams);
        if (lpMyCallParams == NULL)
            return LINEERR_NOMEM;
        
        // Process the call parameters
        LONG lResult = GetSP()->ProcessCallParameters(this, lpMyCallParams);
        if (lResult)
        {
            FreeMem(lpMyCallParams);
            return lResult;          
        }

		// Save off the new pointer
		lpCallParams = lpMyCallParams;
    }

    // Get our address owner.  All new call appearances will be created on it.
    CTSPIAddressInfo* pAddr = GetAddressOwner();

    // If this is not a conference call, fail it.
    if (GetCallType() != Conference)
	{
		FreeMem(lpCallParams);
        return LINEERR_INVALCONFCALLHANDLE;
	}
    
    // Get the total count of people in the conference right now.
    // If we have exceeded our conference count, then fail this request.
    if (pAddr->GetAddressCaps()->dwMaxNumConference <= (m_lstConference.size()+1))
	{
		FreeMem(lpCallParams);
        return LINEERR_CONFERENCEFULL;
	}
    
    // Otherwise, create a consultation call.
    CTSPICallAppearance* pCall = pAddr->CreateCallAppearance(htConsultCall);
    if (pCall == NULL)
	{
		FreeMem(lpCallParams);
        return LINEERR_CALLUNAVAIL;
	}

	// This was created as an outgoing call
	m_dwFlags |= CTSPICallAppearance::_Outgoing;

    // Attach it to the conference in case the conference ends and the derived
    // class needs to locate any consultant calls created.  Note: this implies that
    // there may be only one consultant call per conference active at any given point.
	SetConsultationCall(pCall);

	// Create our conference event.
	RTPrepareAddToConference* pRequest = new RTPrepareAddToConference(this, pCall, dwRequestID, lpCallParams);

    // And add the request                                  
    if (AddAsynchRequest(pRequest))
    {
        *lphdConsultCall = (HDRVCALL) pCall;
        return static_cast<LONG>(dwRequestID);          
    }

    // Failed to add the request.
    pAddr->RemoveCallAppearance(pCall);
	FreeMem(lpCallParams);
    return LINEERR_OPERATIONFAILED;

}// CTSPIConferenceCall::PrepareAddToConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::AddToConference
//
// Add the passed call to our conference list.
//
LONG CTSPIConferenceCall::AddToConference (DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall)
{
	// Verify that the function can be called right now.
	if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_ADDTOCONF) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // If this isn't a consultation call we created, then make sure a new 
    // call (non-consultation) can be created by the address for addition to
    // a conference.
    if (pCall->GetCallType() != Consultant)
    {
        // Make sure this call isn't another conference call!
        if (pCall->GetCallType() == Conference)
            return LINEERR_INVALCONSULTCALLHANDLE;
        
        // Verify that we can add ANY type of call since this isn't a consultant
        // call created by PrepareAddToConf. or SetupConf.
        if ((GetAddressOwner()->GetAddressCaps()->dwAddrCapFlags & 
                LINEADDRCAPFLAGS_CONFERENCEMAKE) == 0)
            return LINEERR_INVALCONSULTCALLHANDLE;
            
        // If the call being added is not on the same address, then verify that
        // cross-address conferences are supported.
        if (pCall->GetAddressOwner() != GetAddressOwner())
        {
            if ((GetLineOwner()->GetLineDevCaps()->dwDevCapFlags & 
                    LINEDEVCAPFLAGS_CROSSADDRCONF) == 0)
                return LINEERR_INVALCONSULTCALLHANDLE;
        }    
    }

#ifdef STRICT_CALLSTATES
    // Make sure the call is in the proper state to be added to our
    // conference.  
    // Added PROCEEDING and RINGBACK to the available call state (TAPI Bakeoff fix).
    if ((pCall->GetCallState() & 
				(LINECALLSTATE_CONNECTED |
				 LINECALLSTATE_ONHOLD |
				 LINECALLSTATE_PROCEEDING |
				 LINECALLSTATE_RINGBACK)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif
    
    // Make sure our conference call is ready to receive a new 
    // member - it must be in the onHoldPendConf state or the
    // onHold state if our address caps say we can do that.
    if (GetCallState() == LINECALLSTATE_ONHOLDPENDCONF ||
		 (GetCallState() == LINECALLSTATE_ONHOLD &&
         (GetAddressOwner()->GetAddressCaps()->dwAddrCapFlags & LINEADDRCAPFLAGS_CONFERENCEHELD)))
        ;
    else
        return LINEERR_INVALCALLSTATE;

    // Attach the call appearance to the conference so we can locate the
    // original conference call later.  We store it into the related call
    // field.
    pCall->SetConferenceOwner(this);

	// Create our request object
	RTAddToConference* pRequest = new RTAddToConference(this, pCall, dwRequestID);

    // Pass the request to our service provider worker code.
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestID);          
    return LINEERR_OPERATIONFAILED;

}// CTSPIConferenceCall::AddToConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::AddToConference
//
// This method forcibly adds a call to a conference.  It should only
// be used to build a conference by hand (i.e. directed by an attached 
// handset or phone).
//
void CTSPIConferenceCall::AddToConference(CTSPICallAppearance* pCall)
{   
	_TSP_ASSERTE(pCall != NULL);

	// Add it to the conference.
    if (!IsCallInConference (pCall) && pCall != this)
	{
		// Attach the call to the conference
		pCall->SetConferenceOwner(this);
		if (pCall->GetCallState() == LINECALLSTATE_CONFERENCED)
			m_lstConference.push_back (pCall);
	}

	// If this is the consultation call for the conference, change the type
	// of call and remove the attachment.
    if (GetAttachedCall() == pCall)
	{
		SetConsultationCall(NULL);
		pCall->SetCallType(Normal);
		pCall->RecalcCallFeatures();
	}

}// CTSPIConferenceCall::AddToConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::RemoveFromConference
//
// Remove the specified call from our conference list.
//
LONG CTSPIConferenceCall::RemoveFromConference(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall)
{
	// Verify that the function can be called right now.
	if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_REMOVEFROMCONF) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // Check the call state of this call and make sure it is conferenced in.
    else if (pCall->GetCallState() != LINECALLSTATE_CONFERENCED)
        return LINEERR_INVALCALLSTATE;
    
    // Verify that this call may be removed.
    else if (!CanRemoveFromConference(pCall))
        return LINEERR_OPERATIONUNAVAIL;
 
	// Create our request object
	RTRemoveFromConference* pRequest = new RTRemoveFromConference(this, pCall, dwRequestID);

    // Looks ok, add the request.    
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestID);          
    return LINEERR_OPERATIONFAILED;

}// CTSPIConferenceCall::RemoveFromConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::CanRemoveFromConference
//
// Return true/false indicating whether the call may be removed
// from the conference.
//
bool CTSPIConferenceCall::CanRemoveFromConference(CTSPICallAppearance* pCall) const
{
    // Validate that the call being removed is in the conference.
    if (!IsCallInConference (pCall))
        return false;

    // If our address doesn't allow removal of conference parties, then error out.
    DWORD dwRemoveType = GetAddressOwner()->GetAddressCaps()->dwRemoveFromConfCaps;
    if (dwRemoveType == LINEREMOVEFROMCONF_NONE)
        return false;

    // If only the last party can be removed, make sure this is the last entry in the 
    // conference list.
    else if (dwRemoveType == LINEREMOVEFROMCONF_LAST)
    {
		CEnterCode sLock(this);  // Synch access to object
		if (pCall != m_lstConference.back())
			return false;
	}
    
    // Looks like we can remove it.
    return true;
    
}// CTSPIConferenceCall::CanRemoveFromConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::RemoveConferenceCall
//
// Remove the conference call specified from our array.  Callstate
// of the removed call is not changed.  Conference is idle'd if this
// is the last call.
//
void CTSPIConferenceCall::RemoveConferenceCall(CTSPICallAppearance* pCall, bool fForceBreakdown /*=true*/)
{   
    // Locate the call appearance and remove it.
	CEnterCode sLock(this);  // Synch access to object
	TCallHubList::iterator pos = std::find(m_lstConference.begin(), m_lstConference.end(), pCall);
	if (pos != m_lstConference.end())
	{
		m_lstConference.erase(pos);
		pCall->SetConferenceOwner(NULL);
		
		// If we have no more calls in our array, then transition 
		// the conference to IDLE.
		if (m_lstConference.empty() && fForceBreakdown)
			SetCallState(LINECALLSTATE_IDLE);

		// Otherwise if we have a single party and we are supposed to breakdown
		// the conference to a two-party call, then do so now.
		else if (m_lstConference.size() == 1 && fForceBreakdown)
		{
			pCall = GetConferenceCall(0);
			RemoveConferenceCall(pCall);
			if (pCall->GetCallState() == LINECALLSTATE_CONFERENCED)
			{
				// If the remove state is not IDLE, change the call state to the given
				// state in the ADDRESSCAPS structure.
				if (GetAddressOwner()->GetAddressCaps()->dwRemoveFromConfState != LINECALLSTATE_IDLE)
					pCall->SetCallState(GetAddressOwner()->GetAddressCaps()->dwRemoveFromConfState);
				else
					pCall->SetCallState(LINECALLSTATE_CONNECTED);
			}
		}
    }               

}// CTSPIConferenceCall::RemoveConferenceCall

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::OnCallStatusChange
//
// This verifies that we have torn down the conference call
// before our conference call handle goes idle.  The TAPI dynamic link
// library maintains the conference calls in a linked list which will
// not correctly get deallocated if we don't idle all the calls in 
// the conference before the conference call.
//
// Generally, all the calls should already be changed or removed as
// a result of this.
//
void CTSPIConferenceCall::OnCallStatusChange(DWORD dwState, DWORD /*dwMode*/, DWORD /*dwMediaMode*/)
{
	_TSP_ASSERTE(dwState != LINECALLSTATE_CONFERENCED);

	// Let the default capabilities be adjusted on this call based on its new state.
	// Turn off the call-change reporting since this function will adjust the features
	// here and report it again.
	bool fTellTapi = ((m_dwFlags & _ChgState) == 0);
	m_dwFlags |= _ChgState;
	CTSPICallAppearance::OnCallStatusChange(dwState, 0, 0);
	if (fTellTapi)
		m_dwFlags &= ~_ChgState; // put it back if necessary.

	// Remove the features which should never be available in a conference call.
	DWORD dwCallFeatures = m_CallStatus.dwCallFeatures;
	dwCallFeatures &= ~(LINECALLFEATURE_ACCEPT |
			LINECALLFEATURE_ADDTOCONF |
			LINECALLFEATURE_ANSWER |
			LINECALLFEATURE_COMPLETECALL |
			LINECALLFEATURE_DIAL |
			LINECALLFEATURE_GATHERDIGITS |
			LINECALLFEATURE_GENERATEDIGITS |
			LINECALLFEATURE_GENERATETONE |
			LINECALLFEATURE_MONITORDIGITS |
			LINECALLFEATURE_MONITORMEDIA |
			LINECALLFEATURE_MONITORTONES |
			LINECALLFEATURE_PARK |
			LINECALLFEATURE_REDIRECT |
			LINECALLFEATURE_REMOVEFROMCONF |
			LINECALLFEATURE_SECURECALL |
			LINECALLFEATURE_SENDUSERUSER |
			LINECALLFEATURE_SETCALLPARAMS |
			LINECALLFEATURE_SETMEDIACONTROL |
			LINECALLFEATURE_SETTERMINAL |
			LINECALLFEATURE_SETUPCONF |
			LINECALLFEATURE_RELEASEUSERUSERINFO |
			LINECALLFEATURE_SETTREATMENT |
			LINECALLFEATURE_SETQOS);

	// Allow derived provider access to this notification. Note that if this
	// call state is IDLE, the call may have been removed from the address.
	CTSPIAddressInfo* pAddr = GetAddressOwner();
	if (pAddr != NULL)
	{
		dwCallFeatures = pAddr->OnCallFeaturesChanged(this, dwCallFeatures);
		SetCallFeatures(dwCallFeatures,false);
	}

    // The last behavior of a REQUEST_DROPxxx on a conference call should be to idle
    // the conference call - this causes the conference to disconnect from all the
    // calls.
    if (dwState == LINECALLSTATE_IDLE)
    {   
        // Remove our back-pointers from the call appearances since we are about to
        // become non-existant.
		CEnterCode sLock(this);  // Synch access to object
		std::for_each(m_lstConference.begin(), m_lstConference.end(),
			std::bind2nd(MEM_FUNV1(&CTSPICallAppearance::SetConferenceOwner), 
					static_cast<CTSPIConferenceCall*>(0)));
    }

    // Otherwise, we are making some other transition, spin through all the connected
    // conference calls and force them to update their capabilities.                 
    // This handles the situation where attached calls change state BEFORE the conference
    // does and make sure that the capabilities stay updated properly.
    else
    {                                                                 
		CEnterCode sLock(this);  // Synch access to object
		for (TCallHubList::iterator iCall =  m_lstConference.begin();
			 iCall !=  m_lstConference.end(); ++iCall)
		{
			CTSPICallAppearance* pCall = (*iCall);

			// Recalculate this calls features
			pCall->RecalcCallFeatures();

			// Ask the call hub for this callid to update all the calls which
			// are part of this conference.
			CTSPICallHub* pHub = pCall->GetCallHub();
			if (pHub != NULL)
				pHub->OnCallStateChange(pCall, pCall->GetCallState(), pCall->GetCallState());
		}
    }

    // NOTE: Depending on the switch behavior, all calls might be dropped when
    // the conference is dropped - in which case, the LINEADDRESSCAPS 
    // "LINEADDRCAPFLAGS_CONFDROP" should be specified.  In other cases, the original
    // call is transitioned back to CONNECTED - the library cannot really do much 
    // in this area, so the best way to process this is a little code in the 
    // REQUEST_DROPxxx handler which looks at the call type for a CALLTYPE_CONFERENCE
    // and does whatever necessary to all the calls before really dropping the
    // conference.  Remember that Drop() is asynch, and therefore will return *before*
    // it is actually finished - therefore use the WaitForCompletion method to wait
    // for it if you have to physically drop the connection.

}// CTSPIConferenceCall::OnCallStatusChange

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::OnRelatedCallStateChange
//
// This method is called by all calls which are part of our confernece
// as their state changes.  We use it to insert and delete calls from
// our conference.
//
void CTSPIConferenceCall::OnRelatedCallStateChange (CTSPICallAppearance* pCall, DWORD dwState, DWORD /*dwOldState*/)
{
	CEnterCode sLock(this);  // Synch access to object

    // If the call is now conferenced in, add it to our list.
    if (dwState == LINECALLSTATE_CONFERENCED)
    {    
		// Add it to the conference.
        if (!IsCallInConference(pCall))
			m_lstConference.push_back(pCall);

		// Mark the conference owner if it isn't set yet.
		pCall->SetConferenceOwner(this);

		// If this is the consultation call for the conference, change the type
		// of call and remove the attachment.
		if (GetAttachedCall() == pCall)
		{
			SetConsultationCall(NULL);
			pCall->SetCallType(Normal);
		}

		// Force it to recalculate it's call features
		pCall->RecalcCallFeatures(dwState);

    }
    // Otherwise if it has transitioned to IDLE then remove it from the conference
    else if (dwState == LINECALLSTATE_IDLE)
    {   
        // If this was a consultant call created by PrepareConf or SetupConf, then
        // it is not part of our conference, and now never will be.
        if (GetAttachedCall() == pCall)
        {   
            // Give any derived class an opportunity to do something about the call being
            // destroyed - i.e. some switches actually drop back to a dialtone, which would
            // require a new call be created.  Some switch back to a two party call when only
            // one member is still in the conference.  The default is to do nothing.
			OnConsultantCallIdle(GetConsultationCall());

			// Detach it from the conference
			SetConsultationCall(NULL);
        }
        else
        {   
            // Locate the call appearance which went IDLE and remove it from our conference
            // array.  If the total number of conferenced calls hits zero, then IDLE this
            // call.
            RemoveConferenceCall(pCall);
        }
    }

}// CTSPIConferenceCall::OnRelatedCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::OnRequestComplete
//
// This method is called by the request object when an outstanding
// request completes.
//
void CTSPIConferenceCall::OnRequestComplete (CTSPIRequest* pReq, LONG lResult)
{   
    // If this is a request to remove a call from the conference,
	// go ahead and do it if the service provider didn't already.
    if (pReq->GetCommand() == REQUEST_REMOVEFROMCONF && lResult == 0)
    {
		CTSPICallAppearance* pCall = pReq->GetCallInfo();
		if (pCall && IsCallInConference(pCall))
			RemoveConferenceCall(pCall);
	}

}// CTSPIConferenceCall::OnRequestComplete

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::IsCallInConference
//
// Determine whether the specified call appearance is in our conference
// list or not.
//
bool CTSPIConferenceCall::IsCallInConference(CTSPICallAppearance* pCall) const
{
	CEnterCode sLock(this);  // Synch access to object
	return (std::find(m_lstConference.begin(), m_lstConference.end(), pCall) != m_lstConference.end());

}// CTSPIConferenceCall::IsCallInConference

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::GetConferenceCall
//
// Return the call at the index specified
//
CTSPICallAppearance* CTSPIConferenceCall::GetConferenceCall(unsigned int i)
{   
	CEnterCode sLock(this);  // Synch access to object

	TCallHubList::iterator it, itEnd = m_lstConference.end();
	for (it = m_lstConference.begin(); it != itEnd && i > 0; ++it, --i)
			;
	return (it != itEnd) ? (*it) : NULL;

}// CTSPIConferenceCall::GetConferenceCall

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::Dump
//
// Debug "dump" of the object and it's contents.
//
TString CTSPIConferenceCall::Dump() const 
{
	TStringStream outstm;

	CEnterCode keyLock(this, FALSE);
	outstm << _T("0x") << hex << (DWORD)this;
	outstm << _T(",htCall=0x") << hex << m_htCall;
	outstm << _T(",LineOwner=0x") << hex << GetLineOwner()->GetPermanentDeviceID();
	outstm << _T(",State=") << GetCallStateName();
	outstm << _T(",RefCnt=") << setw(2) << GetRefCount();

	// Only dump the conferenced parties if we can lock the object right now.
	// If not, ignore it and continue. This particular function is called during
	// call removal and causes a deadlock if another call is changing state at 
	// this moment in the conference. (V3.043)
	if (keyLock.Lock(0))
	{
		outstm << _T(",Parties=") << setw(2) << m_lstConference.size() << endl;
		for (TCallHubList::const_iterator iCall = m_lstConference.begin();
		     iCall != m_lstConference.end(); ++iCall)
			 outstm << _T("  ") << (*iCall)->Dump() << endl;
	}
	else outstm << endl;
    return(outstm.str());

}// CTSPIConferenceCall::Dump
#endif