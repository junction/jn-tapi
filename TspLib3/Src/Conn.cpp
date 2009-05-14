/******************************************************************************/
//                                                                        
// CONN.CPP - Source file for the TSPIConnection class object.
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code for the base connection object upon    
// which the phone and line connections are derived.                      
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
// CTSPIConnection::CTSPIConnection
//
// Constructor
//
CTSPIConnection::CTSPIConnection() : CTSPIBaseObject(),
    m_pDevice(0), m_dwDeviceID(0xffffffff), m_strName(_T("")), 
	m_dwFlags(0), m_pExtVerInfo(0), m_dwNegotiatedVersion(0)
{
}// CTSPIConnection::CTSPIConnection

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::~CTSPIConnection
//
// Destructor
//
CTSPIConnection::~CTSPIConnection()
{
	delete m_pExtVerInfo;

}// CTSPIConnection::~CTSPIConnection

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::Init
//
// Initialize the connection.  Each CTSPIConnection is owned by a
// specific device and has a positional identifier which is set by
// TAPI.
//
void CTSPIConnection::Init(CTSPIDevice* pDevice, DWORD dwDeviceId)
{
    m_pDevice = pDevice;
	m_dwDeviceID = dwDeviceId;
    GetSP()->MapConnectionToID(dwDeviceId, this);

	// Now walk the requst map and add all the requests to our map
	// We do this for performance so we don't scan the map when requests
	// are actually being processed.  The structures built by the macros
	// should be stored in a discardable page (INIT_DATA) which would get
	// paged out after initialization is complete.
	const tsplib_REQMAP* pRequests = GetRequestList();
	TRequestMap* pMap = GetRequestMap();
	_TSP_ASSERTE(pMap != NULL);

	if (pMap != NULL && pMap->empty() && pRequests != NULL)
	{
		while (pRequests->nRequest != 0)
		{
			(*pMap)[pRequests->nRequest] = (tsplib_REQPROC) pRequests->fpReq;
			++pRequests;
		}
	}

}// CTSPIConnection::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::NegotiateVersion
//
// This negotiates the TSP version for this line or phone device
// Override this to allow for different devices to support different
// versions of TAPI functions.
//
LONG CTSPIConnection::NegotiateVersion(DWORD dwLoVersion, DWORD dwHiVersion, LPDWORD lpdwTSPIVersion)
{
    // Do a SERVICE PROVIDER negotiation.
    *lpdwTSPIVersion = GetSP()->GetSupportedVersion();
    if (dwLoVersion > *lpdwTSPIVersion) // The app is too new for us
        return LINEERR_INCOMPATIBLEAPIVERSION;

    // If the version supported is LESS than what we support, then drop to the 
	// version it allows.  The library can handle down to TAPI 1.3.
    if (dwHiVersion < *lpdwTSPIVersion)
    {
        if (dwHiVersion < TAPIVER_13)
            return LINEERR_INCOMPATIBLEAPIVERSION;
        *lpdwTSPIVersion = dwHiVersion;
    }

	// Save off the negotiated version
	m_dwNegotiatedVersion = *lpdwTSPIVersion;

    // Everything looked Ok.
    return 0;

}// CTSPIConnection::NegotiateVersion

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::NegotiateExtVersion
//
// This negotiates the extended version for this line or phone device
//
LONG CTSPIConnection::NegotiateExtVersion(DWORD dwTSPIVersion, DWORD dwLoVersion, 
									DWORD dwHiVersion, LPDWORD lpdwExtVersion)
{
	// Set it to zero
	*lpdwExtVersion = 0;

    // Ok, device Id looks ok, do the version negotiation.  If the
	// line isn't open yet, use the supported TSP version.
	DWORD dwNegotiatedVersion = (m_dwNegotiatedVersion==0) ? 
		GetSP()->GetSupportedVersion() : m_dwNegotiatedVersion;
    if (dwTSPIVersion != dwNegotiatedVersion)
        return LINEERR_INCOMPATIBLEAPIVERSION;

	// If we have extension information then attempt to negotiate to it.
	if (m_pExtVerInfo == NULL)
	    return LINEERR_OPERATIONUNAVAIL;

	// If we already have a selected extension then restrict the available
	// version reporting to that specific version.
	if (m_pExtVerInfo->dwSelectedExtVersion != 0)
	{
		if (dwLoVersion > m_pExtVerInfo->dwSelectedExtVersion ||
			dwHiVersion < m_pExtVerInfo->dwSelectedExtVersion)
			return LINEERR_INCOMPATIBLEEXTVERSION;
		*lpdwExtVersion = m_pExtVerInfo->dwSelectedExtVersion;
	}
	
	// Otherwise allow the range supported.
	else
	{
		// If the lowest version the app can negotiate to isn't supported
		// by us then refuse to negotiate.  The app is too new.
		if (dwLoVersion > m_pExtVerInfo->dwMaxExtVersion)
			return LINEERR_INCOMPATIBLEAPIVERSION;

		// If the highest version supported by the app is less than our
		// lowest supported version then fail - the app is too old.
		if (dwHiVersion < m_pExtVerInfo->dwMinExtVersion)
			return LINEERR_INCOMPATIBLEAPIVERSION;

		// The lo/hi version is within our range.
		*lpdwExtVersion = min(dwHiVersion, max(m_pExtVerInfo->dwMaxExtVersion, dwLoVersion));
	}

	return 0;

}// CTSPIConnection::NegotiateExtVersion

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetExtensionID
//
// This function returns the extension ID that the service provider
// supports for the indicated line/phone device.
//
LONG CTSPIConnection::GetExtensionID(DWORD /*dwTSPIVersion*/, LPEXTENSIONID lpExtensionID)
{
	// If we have extension information available, then provide it to the user.
	if (m_pExtVerInfo == NULL)
	{
		ZeroMemory(lpExtensionID, sizeof(EXTENSIONID));
	}
	else
	{
		CopyMemory(lpExtensionID, &m_pExtVerInfo->ExtensionID, sizeof(EXTENSIONID));
	}

    return 0;

}// CTSPIConnection::GetExtensionID

/////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::SelectExtVersion
//
// This function selects the indicated Extension version for the 
// indicated line or phone device.  Subsequent requests operate according to that 
// Extension version.
//
LONG CTSPIConnection::SelectExtVersion(DWORD dwExtVersion) 
{
	// Save off the new version for structure manipulation later.
	if (m_pExtVerInfo == NULL)
		return LINEERR_OPERATIONUNAVAIL;

	// Validate the extension version information.
	if (dwExtVersion != 0 && 
		dwExtVersion < m_pExtVerInfo->dwMinExtVersion ||
		dwExtVersion > m_pExtVerInfo->dwMaxExtVersion)
		return LINEERR_INCOMPATIBLEEXTVERSION;

	// Save of the selected extension level
	m_pExtVerInfo->dwSelectedExtVersion = dwExtVersion;

	return 0;
    
}// CTSPIConnection::SelectExtVersion

/////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::SetExtVersion
//
// This function sets up the extension information reported by this device.
//
void CTSPIConnection::SetExtVersionInfo(DWORD dwMinVersion, DWORD dwMaxVersion, 
										DWORD dwExtensionID0,DWORD dwExtensionID1, 
										DWORD dwExtensionID2, DWORD dwExtensionID3)
{
	if (m_pExtVerInfo == NULL)
		m_pExtVerInfo = new TExtVersionInfo;

	m_pExtVerInfo->dwMinExtVersion = min(dwMinVersion, dwMaxVersion);
	m_pExtVerInfo->dwSelectedExtVersion = 0;
	m_pExtVerInfo->dwMaxExtVersion = max(dwMaxVersion, dwMinVersion);
	m_pExtVerInfo->ExtensionID.dwExtensionID0 = dwExtensionID0;
	m_pExtVerInfo->ExtensionID.dwExtensionID1 = dwExtensionID1;
	m_pExtVerInfo->ExtensionID.dwExtensionID2 = dwExtensionID2;
	m_pExtVerInfo->ExtensionID.dwExtensionID3 = dwExtensionID3;

}// CTSPIConnection::SetExtVersion

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetIcon
//
// This function retrieves a service device-specific icon for display
// in user-interface dialogs.
//
LONG CTSPIConnection::GetIcon (const TString& /*strDevClass*/, LPHICON /*lphIcon*/)
{
    // Return not available, TAPI will supply a default icon.
    return 0;
    
}// CTSPIConnection::GetIcon

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddAsynchRequest
//
// This method inserts a new asynchronous request into our request
// list, and if the connection has no pending requests, sends it to the 
// device.
//
int CTSPIConnection::AddAsynchRequest(CTSPIRequest* pRequest)
{
	// If the request is NULL, then return an error.
	if (pRequest == NULL)
		return 0;

	// Determine the position of the request
	// Lock the line/phone object and arrays
	CEnterCode sLock(this);

	// If this is a DROP request, then move it to the front of the queue, otherwise
	// it gets placed at the end.
	int iPos = 0;
	if (pRequest->GetCommand() == REQUEST_DROPCALL)
	{
		// If there are requests, insert this drop command after the
		// last "running" request so we wait for it to complete.
		// This stops us from killing a request while some other worker thread
		// is processing it.
		for (TRequestList::iterator pPos = m_lstAsynchRequests.begin();
			 pPos != m_lstAsynchRequests.end(); ++pPos)
		{
			if ((*pPos)->GetState() == STATE_NOTPROCESSED)
				break;
			iPos++;
		}
	}
	else
		iPos = -1;	// End of the list

	sLock.Unlock();

	// Give the line or phone or device a chance to re-shuffle existing requests.
	if (OnNewRequest (pRequest, &iPos) == false)
	{
#ifdef _DEBUG
		_TSP_DTRACEX(TRC_REQUESTS, _T("%s: Request canceled by OnNewRequest %s"), m_strName.c_str(), pRequest->Dump().c_str());
#endif
		pRequest->DecRef();
		return 0;
	}

	// Insert the request into our list.  If the worker function returns
	// a true response then automatically begin this request on the current
	// thread.  Otherwise simply let it be queued.
	if (AddAsynchRequest(pRequest, iPos))
	{
		// Ship it to the ReceiveData.  We will simply use the thread we are
		// on since we are guaranteed that it is TAPISRV.EXE.
		// Unless overridden, this will end up in ProcessData.
		ReceiveData();
	}

    return 1;
   
}// CTSPIConnection::AddAsynchRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddAsynchRequest
//
// Add the listed asynch request to the list and return whether to
// immediately start the request.  This function is overridable for
// providers which need to examine the request be starting it.
//
bool CTSPIConnection::AddAsynchRequest (CTSPIRequest* pReq, int iPos) /*throw()*/
{
	CEnterCode Key(this);  // Synch access to object

	// Validate the position.
	int iTotal = m_lstAsynchRequests.size();
	if (iPos > iTotal)
		iPos = -1;

    // If the request is to be added to the FRONT.
	if (iPos == 0)
    {
		// Add to front, always START this request.
		try
		{
			m_lstAsynchRequests.push_front(pReq);
		}
		catch(...)
		{
			return false;
		}
		return true;
    }

	// Otherwise, add to the end and start the request only if we don't 
	// have other active requests already running.  This is so we don't 
	// interrupt some response from the device which is associated with a
	// particular running request by invoking a new command.
	else if (iPos == -1)
	{
		try
		{
			m_lstAsynchRequests.push_back(pReq);
		}
		catch(...)
		{
			return false;
		}
		return (++iTotal == 1);
	}

	// Or somewhere between, don't ever start this request.
	else
	{ 
		TRequestList::iterator pPos;

		// Locate the proper position.  Walk the list until we find
		// the proper element.
		for (pPos = m_lstAsynchRequests.begin();
			 pPos != m_lstAsynchRequests.end() && iPos; ++pPos, --iPos)
			;

		try
		{
			if (pPos == m_lstAsynchRequests.end())
				m_lstAsynchRequests.push_back(pReq);
			else
				m_lstAsynchRequests.insert(pPos, pReq);
		}
		catch(...)
		{
			return false;
		}
	}
	return false;

}// CTSPIConnection::AddAsynchRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::CompleteRequest
//
// Complete a specific request and information TAPI through the 
// asynchronous completion callback that the request is finished.
// This function optionally removes the request (deletes it).
//
void CTSPIConnection::CompleteRequest(CTSPIRequest* pReq, LONG lResult, bool fTellTapi, bool fRemoveRequest)
{
	_TSP_ASSERTE(pReq != NULL);
	_TSP_ASSERTE(*((LPDWORD)pReq) != 0xdddddddd);	// Object has been deleted!

	CEnterCode sReqLock(pReq); // Lock the request
	bool fIsCurrentRequest = (GetCurrentRequest() == pReq);
	bool fSendResponse = (!pReq->HaveSentResponse() && fTellTapi);

	// Check to see if the request was deleted or completed previously.
	if (pReq->GetState() == STATE_COMPLETED)
		return;

    // Remove the request from our list.
    if (fRemoveRequest)
	{
		// Since we are removing the request, mark it as COMPLETED and then
		// remove it from our array of requests.
		pReq->SetState(STATE_COMPLETED);
		RemoveRequest(pReq);
	}

	// Unlock the request here so that blocked threads will be released 
	// in case we are going to delete this request.
	sReqLock.Unlock();

    // Unblock any threads waiting on this to complete.  This will cause our
    // thread to relinquish its time-slice in order to let the other threads
    // come alive and return our result code.
    // This will also cause the 'OnRequestComplete' method of the connection, 
    // address, and call to be invoked.
    pReq->Complete (lResult, fTellTapi);

	// Tell TAPI the result of the operation.  We do this AFTER we have removed the
	// request since TAPISRV.EXE tends to reuse handles quickly.
    if (fSendResponse)
	{
		_TSP_ASSERTE(GetDeviceInfo() != NULL);
        GetDeviceInfo()->OnAsynchRequestComplete(lResult, pReq);
	}

    // Delete the request now if asked to.
    if (fRemoveRequest)
    {
		// Deallocate the request
        pReq->DecRef();             

		// If we still have requests waiting to run on this line, start the
		// next available request.
        if (GetRequestCount() > 0 && fIsCurrentRequest)
			ReceiveData();
    }
             
}// CTSPIConnection::CompleteRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::CompleteCurrentRequest
//
// This method informs TAPI of the completion of an asynchronous
// task and optionally removes and starts the next task in the
// list.
//
bool CTSPIConnection::CompleteCurrentRequest(LONG lResult, bool fTellTapi, bool fRemoveRequest)
{
    // If the request list is not empty, then grab the head request and complete it.
    CTSPIRequest* pReq = GetCurrentRequest(TRUE);
    if (pReq != NULL)
    {
        // Unblock any threads waiting on this request to complete.
        CompleteRequest (pReq, lResult, fTellTapi, fRemoveRequest);
		pReq->DecRef();
        return true;
    }
    return false;

}// CTSPIConnection::CompleteCurrentRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::RemovePendingRequests
//
// Removes any pending requests for the specified connection.
//
void CTSPIConnection::RemovePendingRequests(CTSPICallAppearance* pCall, int iReqType, LONG lErrorCode, bool fOnlyIfReqNotStarted, const CTSPIRequest* pcurrRequest)
{
	// Don't allow other threads to mess with the request list 
	// while we are deleting requests.
	CEnterCode sLock(this);

	// Walk through all the requests in the list matching them up to the connection object
	// and the optional call appearance and request type.  We need to make multiple passes
	// since we are removing requests as we go - so we start over from the top each time.
	for (;;)
	{
		// Walk through the list and find the first request which matches the
		// requested criteria.
		CTSPIRequest* pReq = NULL;
		for (TRequestList::iterator posCurr = m_lstAsynchRequests.begin(); 
			 posCurr != m_lstAsynchRequests.end(); ++posCurr)
		{
			if ((*posCurr) != pcurrRequest &&
				IsMatchingRequest ((*posCurr), pCall, iReqType, fOnlyIfReqNotStarted))
			{
				pReq = (*posCurr);
				break;
			}
		}

		// If we found no requests in the list, then exit.
		if (pReq == NULL)
			break;

		// Otherwise, delete this request.
#ifdef _DEBUG
		_TSP_DTRACE(_T("%s: Removing pending request %s"), m_strName.c_str(), pReq->Dump().c_str());
#endif
            
		// See if this request has started.  We assume that
		// the state will be changed.
		if (pReq->GetState() != STATE_INITIAL)
		{
			// The request has at least started, tell the service 
			// provider that we are CANCELING the request.  It
			// can do anything it needs to do on this call with the request.
			OnCancelRequest (pReq);
		}
    
		// Remove the request.
		RemoveRequest(pReq);

		// Tell TAPI that the request is canceled.
		if (!pReq->HaveSentResponse())
			GetDeviceInfo()->OnAsynchRequestComplete(lErrorCode, pReq);
		
		// Unblock anyone waiting on this request to finish.
		pReq->Complete (lErrorCode, true);
       
		// Delete the request
		pReq->DecRef();
    }
      
}// CTSPIConnection::RemovePendingRequests

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::IsMatchingRequest
//
// Determine if the request matches a set of criteria.  This is called
// to delete the request.  DO NOT CALL THIS FOR ANY OTHER PURPOSE -
// SPECIAL CHECKS ARE DONE FOR EMBEDDED CALL APPEARANCES IN REQUEST
// PARAMETERS!
//
bool CTSPIConnection::IsMatchingRequest (CTSPIRequest* pReq, CTSPICallAppearance* pCall, int nRequest, bool fOnlyIfReqNotStarted)
{   
    // First do the simple matching.
    if ((pCall == NULL || (pCall && pReq->GetCallInfo() == pCall)) &&
        (nRequest == REQUEST_ALL || nRequest == pReq->GetCommand()) &&
		(fOnlyIfReqNotStarted == false || pReq->GetState() == STATE_NOTPROCESSED))
        return true;        

	// If the request has already been started, and we are only interested in
	// non-started events, then ignore the request.
	if (fOnlyIfReqNotStarted && pReq->GetState() != STATE_NOTPROCESSED)
		return false;

    // Now see if this call is the target of a command in a parameter
    // block.  The only two requests where this happens are transfer
    // and conference events.  In both these cases, the events need
    // to be removed or modified to reflect that the call is no longer
    // available.
    if (pCall != NULL)
    {
        if (nRequest == REQUEST_ALL || nRequest == pReq->GetCommand())
        {
			switch (pReq->GetCommand())
			{
				// If the SETUPXFER hasn't completed, then the consultant call
				// is not yet valid.  This call will be transitioned to IDLE
				// by the dropping of the REAL call, and the request would be deleted
				// from the above test since the REQUEST block is always inserted
				// on the REAL call appearance.  If this is the consultant call
				// that is wanting to delete the request, then we need to 
				// simply set the consultant call to NULL in the setupXfer block,
				// it will automatically be detached from the call appearance by
				// the CTSPICallAppearance::Drop method.            
				case REQUEST_SETUPXFER:
				{   
					RTSetupTransfer* pTrans = dynamic_cast<RTSetupTransfer*>(pReq);
					if (pTrans->GetConsultationCall() == pCall)
						pTrans->m_pConsult = NULL;
					break;
				}
            
				// Otherwise, if this is a CompleteTransfer request, then
				// this call could be either the consultant call or the
				// created conference call.  The consultant call SHOULD be
				// valid and connected to the REAL call appearance.  The
				// conference would not been attached - it is simply an
				// object at this point.
				//
				// If this is the consultant call requesting deletion, then
				// transition the conference call to idle (if available) and
				// return this block for deletion.
				//
				// Else if this is the conference call requesting deletion, 
				// then simply remove the conference pointer in the request
				// block and allow the call to proceed (as a straight transfer
				// to the consultant call).
				case REQUEST_COMPLETEXFER:
				{
					RTCompleteTransfer* pTransfer = dynamic_cast<RTCompleteTransfer*>(pReq);
					if (pTransfer->GetConsultationCall() == pCall)
					{
						if (pTransfer->GetConferenceCall() != NULL &&
							pTransfer->GetTransferMode() == LINETRANSFERMODE_CONFERENCE)
							pTransfer->GetConferenceCall()->SetCallState(LINECALLSTATE_IDLE);
						return true;
					}
					else if (pTransfer->GetConferenceCall() == pCall)
						return true;
					break;
				}
            
				// If this is a SetupConference command, then the request is 
				// always inserted on the CONFERENCE call handle, so we need to
				// check for the consultation and REAL call appearances.  If
				// this is the real call asking for the deletion, then the conference
				// call is considered invalid (since it hasn't been completed).
				case REQUEST_SETUPCONF:
				{
					RTSetupConference* pConf = dynamic_cast<RTSetupConference*>(pReq);
					// If it is the REAL call being dropped, then the
					// entire request becomes invalid.  Idle the conference
					// and consultant call and delete this request.
					if (pConf->GetOriginalCall() == pCall)
					{
						pConf->GetConferenceCall()->SetCallState(LINECALLSTATE_IDLE);
						pConf->GetConsultationCall()->SetCallState(LINECALLSTATE_IDLE);
						return true;   
					}
					// If it is the consultant call, then simply remove it
					// from the call structure, but allow the conference to
					// still be setup - the request can be invalidated later 
					// by the service provider (when it processes this) if that
					// type of conference is not allowed.
					else if (pConf->GetConsultationCall() == pCall)
					{
						pConf->GetConferenceCall()->DetachCall();
						pCall->DetachCall();
						pConf->m_pConsult = NULL;
					}
					break;
				}
            
				// Else if this is an ADD TO CONFERENCE request, then we 
				// only have a consultant call to add to the conference.  If
				// it gets dropped, then delete the request
				case REQUEST_ADDCONF:
				{
					RTAddToConference* pConf = dynamic_cast<RTAddToConference*>(pReq);
					if (pConf->GetConsultationCall() == pCall)
						return true;
					break;
				}        

				// Default handler
				default:
					break;
			}
        }            
    }            
    
    // Request didn't match.
    return false;
            
}// CTSPIConnection::IsMatchingRequest

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::FindRequest
//
// Locate a request packet based on a call appearance and command type.
//
CTSPIRequest* CTSPIConnection::FindRequest(CTSPICallAppearance* pCall, int nReqType) const
{
    // Walk through all the requests and see if any match our connection, call, request criteria.
	CEnterCode sLock(this);  // Synch access to object
	for (TRequestList::const_iterator posCurr = m_lstAsynchRequests.begin(); 
		 posCurr != m_lstAsynchRequests.end(); ++posCurr)
    {
        // If this request doesn't match what we are searching for, then skip it.
        if ((pCall && (*posCurr)->GetCallInfo() != pCall) ||
            (nReqType != REQUEST_ALL && nReqType != (*posCurr)->GetCommand()))
            continue; 
        return (*posCurr);
    }
    return NULL;

}// CTSPIConnection::FindRequest

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetCurrentRequest
//
// Return the head request in our asynch request list.  Override
// this function if you want to skip certain requests.
//
CTSPIRequest* CTSPIConnection::GetCurrentRequest(BOOL fAddRef) const
{
	CEnterCode sLock(this);  // Synch access to object
	if (m_lstAsynchRequests.empty() == true)
		return NULL;

	CTSPIRequest* pRequest = m_lstAsynchRequests.front();
	if (fAddRef) pRequest->AddRef();
	return pRequest;

}// CTSPIConnection::GetCurrentRequest

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetRequest
//
// Get a specific request based on a position.
//
CTSPIRequest* CTSPIConnection::GetRequest(unsigned int iPos, BOOL fAddRef) const
{
	CEnterCode sLock(this);  // Synch access to object
    if (m_lstAsynchRequests.size() > iPos)
    {
		for (TRequestList::const_iterator posCurr = m_lstAsynchRequests.begin(); 
			 posCurr != m_lstAsynchRequests.end(); ++posCurr)
		{
            if (iPos-- == 0)
			{
				CTSPIRequest* pRequest = (*posCurr);
				if (pRequest->GetState() == STATE_NOTPROCESSED)
					pRequest->SetState(STATE_INITIAL);
				if (fAddRef) pRequest->AddRef();
				return pRequest;
			}
		}
    }
    return NULL;

}// CTSPIConnection::GetRequest

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::WaitForAllRequests
//
// Wait for pending requests to complete on the specifid line/call
// of the specifid type.
//
void CTSPIConnection::WaitForAllRequests(CTSPICallAppearance* pCall, int nRequest)
{           
	CEnterCode Key(this, false);  // Synch access to object
    for(;;)
    {
		_TSP_VERIFY(Key.Lock() == true);
		TRequestList::iterator posCurr;
  		for (posCurr = m_lstAsynchRequests.begin(); 
			 posCurr != m_lstAsynchRequests.end(); ++posCurr)
		{
			CTSPIRequest* pRequest = (*posCurr);

            if ((pCall == NULL || pRequest->GetCallInfo() == pCall) &&
                (nRequest == REQUEST_ALL || pRequest->GetCommand() == nRequest))
            {
				pRequest->AddRef();
				Key.Unlock();
                pRequest->WaitForCompletion(INFINITE);
				pRequest->DecRef();
                break;
            }
        }                
        if (posCurr == m_lstAsynchRequests.end()) 
			break;          
    }

}// CTSPIConnection::WaitForAllRequests

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddDeviceClass
//
// Add a STRING data object to our device class list
//
int CTSPIConnection::AddDeviceClass (LPCTSTR pszClass, LPCTSTR lpszBuff, DWORD dwType)
{
	if (dwType == -1L)
	{
		CTSPILineConnection* pLine = dynamic_cast<CTSPILineConnection*>(this);
		if (pLine != NULL)
			dwType = pLine->m_LineCaps.dwStringFormat;
		else
		{
			CTSPIPhoneConnection* pPhone = dynamic_cast<CTSPIPhoneConnection*>(this);
			if (pPhone != NULL)
				dwType = pPhone->m_PhoneCaps.dwStringFormat;
		}
	}
	return AddDeviceClass (pszClass, dwType, const_cast<LPVOID>(reinterpret_cast<LPCVOID>(lpszBuff)), (lstrlen(lpszBuff)+1) * sizeof(TCHAR));

}// CTSPIConnection::AddDeviceClass

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::OnNewRequest
//
// A new request is being added to our connection object.  The derived
// provider may override this function or CTSPIDevice::OnNewRequest or
// the CServiceProvider::OnNewRequest function to catch these and perform 
// some function BEFORE the request has officially been added.
//
// If false is returned, the request will be canceled.
//
bool CTSPIConnection::OnNewRequest (CTSPIRequest* pReq, int* piPos)
{
	return GetDeviceInfo()->OnNewRequest (this, pReq, piPos);

}// CTSPIConnection::OnNewRequest

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::OpenDevice
//
// This method is called when lineOpen or phoneOpen is called.
//
bool CTSPIConnection::OpenDevice()
{
    // Default behavior is to pass onto the device class. This allows for 
	// a single device object to control access for ALL connections (i.e.
	// a modem style device).
    return GetDeviceInfo()->OpenDevice (this);

}// CTSPIConnection::Open

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::CloseDevice
//
// This method is called when the last line/phone connection object is
// closed.  It will not be called if multiple lines/phones are open on this
// device.
//
bool CTSPIConnection::CloseDevice ()
{
    // Default behavior is to pass onto the device class. This allows for 
	// a single device object to control access for ALL connections (i.e.
	// a modem style device).
    return GetDeviceInfo()->CloseDevice (this);

}// CTSPIConnection::Close

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::OnTimer
//
// This is invoked by our periodic timer called from the device manager
// object (m_pDevice).
//
void CTSPIConnection::OnTimer()
{                               
	/* Do nothing */

}// CTSPIConnection::OnTimer

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::OnCancelRequest
//
// This is called when a request is canceled on this line/phone.
//
void CTSPIConnection::OnCancelRequest (CTSPIRequest* pReq)
{
	GetDeviceInfo()->OnCancelRequest(pReq);

}// CTSPIConnection::OnCancelRequest

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::ReceiveData
//
// Data has been received by the device.  This method looks in our created
// TSPIREQ map and forwards the request to the appropriate function handler.
//
// We assume (since most devices work this way) that we only handle a 
// single device request at a time (i.e. we don't issue commands for
// another TAPI requests on the same line before waiting for completion
// of the first.  If your TSP design wants to do this you should replace
// the implementation of this function in your device object.
//
bool CTSPIConnection::ReceiveData(LPCVOID lpBuff)
{
	// Get the current request from the handler list.  Note that if you
	// wish to alter the logic associated with the current request (i.e.
	// some sort of priority) then you can override the GetCurrentRequest()
	// function used here to retrieve the top request.
	CTSPIRequest* pRequest = GetCurrentRequest(TRUE);
	if (pRequest != NULL)
	{
		// Set the state as initial if this request has not been processed before.
		// WARNING: This step is important and must be done in any derived code !!!
		if (pRequest->GetState() == STATE_NOTPROCESSED)
			pRequest->SetState(STATE_INITIAL);

		// Dispatch the request.
		if (DispatchRequest(pRequest, lpBuff))
		{
			pRequest->DecRef();
			return true;
		}
		pRequest->DecRef();
	}

	// No handler managed the request, forward the event to the unsolicited handler
	return (lpBuff != NULL) ? UnsolicitedEvent(lpBuff) : false;

}// CTSPIConnection::ReceiveData

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::DispatchRequest
//
// This dispatches the requests to the proper "OnXXX" handler in the
// derived TSP code.
//
bool CTSPIConnection::DispatchRequest(CTSPIRequest* pRequest, LPCVOID lpBuff)
{
	// Get the request map
	TRequestMap* pMap = GetRequestMap();
	_TSP_ASSERTE(pMap != NULL);

	// Look in our request map and see if we have a handler for
	// this TAPI request type (REQUEST_xxx).  Note we don't need to lock
	// any semaphore here because the request map is fixed -- it should never
	// change in the lifetime of the provider.
	TRequestMap::iterator theIterator = pMap->find(pRequest->GetCommand());
	if (theIterator != pMap->end())
	{
		// Pass the request onto our handler.  If it returns TRUE, then
		// skip the unsolicited handler.
		tsplib_REQPROC func = (*theIterator).second;
		if (func != NULL)
		{
			if ((this->*func)(pRequest, lpBuff))
				return true;
		}

		// Otherwise we had an entry in our map but the function pointer
		// is NULL.  This is used for auto functions (functions which are
		// exported and handled by the library).  Simply complete the request.
		else CompleteRequest(pRequest, 0);
	}
	
	// If we do not have a handler for this request then it is exported from
	// the .DEF file but no ON_TSPI_REQUEST handler is in place, give a warning
	// and finish the request with operation failed.
	else 
	{
#ifdef _DEBUG
		_TSP_DTRACE(_T("%s: Missing ON_TSPI_REQUEST handler for %s"), m_strName.c_str(), pRequest->Dump().c_str());
#endif
		if (IsLineDevice())
		{
			CompleteRequest(pRequest, LINEERR_OPERATIONFAILED);
		}
		else
		{
			_TSP_ASSERTE(IsPhoneDevice());
			CompleteRequest(pRequest, PHONEERR_OPERATIONFAILED);
		}
	}

	return false;

}// CTSPIConnection::DispatchRequest

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::UnsolicitedEvent
//
// Data has been received by the device but there is not a pending
// request which is available to process the data
//
bool CTSPIConnection::UnsolicitedEvent (LPCVOID /*lpBuff*/)
{
	return false;

}// CTSPIConnection::UnsolicitedEvent

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetID
//
// This method returns device information about the device and its
// associated resource handles.
//
LONG CTSPIConnection::GetID(const TString& strDevClass, LPVARSTRING lpDeviceID, HANDLE hTargetProcess)
{   
	DEVICECLASSINFO* pDeviceClass = GetDeviceClass(strDevClass.c_str());
	if (pDeviceClass != NULL)
		return GetSP()->CopyDeviceClass (pDeviceClass, lpDeviceID, hTargetProcess);

	if (IsLineDevice())
	{
		return LINEERR_INVALDEVICECLASS;
	}
	else
	{
		_TSP_ASSERTE(IsPhoneDevice());
		return PHONEERR_INVALDEVICECLASS;
	}
    
}// CTSPIConnection::GetID

