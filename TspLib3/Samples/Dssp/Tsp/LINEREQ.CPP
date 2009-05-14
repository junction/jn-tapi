/*****************************************************************************/
//
// LINEREQ.CPP - Digital Switch Service Provider Sample
//                                                                        
// This file contains the line request processing functions which process
// each of the TAPI requests from the library.
// 
// Copyright (C) 1997-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This service provider talks to a simulated digital switch emulator.  It
// is designed as a sample of how to implement a service provider using
// the TAPI C++ SP class library.  
//
// This source code is only intended as a supplement to the
// documentation provided with the TAPI C++ SP class library.
//
/*****************************************************************************/

#include "stdafx.h"
#include "dssp.h"

/*----------------------------------------------------------------------------
	GLOBALS
-----------------------------------------------------------------------------*/

// Dialtone states to convert from Emulator to TAPI.
const DWORD g_DialToneStates[] = {
    LINEDIALTONEMODE_INTERNAL,
    LINEDIALTONEMODE_EXTERNAL
};  

// Busy states
const DWORD g_BusyStates[] = {
    LINEBUSYMODE_STATION,
    LINEBUSYMODE_TRUNK
};  

// Call state translation table.
extern DWORD g_CallStates[];
extern LPCTSTR g_ComplMsgs[];

/*****************************************************************************
** Procedure:  CDSLine::processAnswer
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Answer an incoming call.
**
*****************************************************************************/
bool CDSLine::processAnswer(RTAnswer* pReq, LPCVOID lpBuff)
{                             
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPICallAppearance* pCall = pReq->GetCallInfo();

    switch (pReq->GetState())
    {
        // Step 1:
        // Ask the switch emulator to answer the call - this would be to press
        // the button associated with the offering address.
        case STATE_INITIAL:
            pReq->SetState(STATE_WAITFORCONNECT);
            GetDeviceInfo()->DRV_AnswerCall(pReq->GetAddressInfo()->GetAddressID());
            break;
        
        // Step 2:
        // Address should indicate a connected end-party.
        case STATE_WAITFORCONNECT:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = reinterpret_cast<const LPEMADDRESSCHANGE>(pevBuff->lpBuff);
                if (lpAddrChange->wAddressID == pReq->GetAddressInfo()->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_CONNECT)
                    {   
                        CompleteRequest(pReq, 0);
                        pCall->SetCallState(LINECALLSTATE_CONNECTED);
						return true;
                    }                        
                    else CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
                }
            }
            break;                
    }
    
    // If we failed, then complete the request with an error - note the
	// emulator doesn't tell us WHICH request failed, we have to rely on the
	// knowledge that the error had to do with our address and therefore was
	// probably our request.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR &&
		(pevBuff->dwError == EMERROR_INVALADDRESSID ||
		 pevBuff->dwAddress == pReq->GetAddressInfo()->GetAddressID()))
	{
		CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
							  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
						LINEERR_OPERATIONFAILED);
		return true;
	}

	// Let the request flow through the unsolicited handler
    return false;

}// CDSLine::processAnswer

/*****************************************************************************
** Procedure:  CDSLine::processSetupXfer
**
** Arguments:  'pReq' - Request object we are working with
**             'pevBuff->dwResponse' - Current response from emulator
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Setup a transfer request (lineSetupTransfer)
**
*****************************************************************************/
bool CDSLine::processSetupXfer (RTSetupTransfer* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    switch (pReq->GetState())
    {                      
        // Step 1:
        // Send a transfer request with no address information.  This will
        // place the current call onHold, and the address will transition to
        // the dialtone state.
        case STATE_INITIAL:   
            pReq->SetState(STATE_CHECKFORDIALTONE);
            if (!GetDeviceInfo()->DRV_Transfer(pReq->GetAddressInfo()->GetAddressID(), ""))
                CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
            break;
                  
        // Step 2:
        // Wait for the switch to signal that we are in the proper state.
        case STATE_CHECKFORDIALTONE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = reinterpret_cast<const LPEMADDRESSCHANGE>(pevBuff->lpBuff);
                if (lpAddrChange->wAddressID == pReq->GetAddressInfo()->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_ONHOLD &&
                        lpAddrChange->wStateInfo == HOLDTYPE_TRANSFER)
                    {
                        pReq->GetCallToTransfer()->SetCallState(LINECALLSTATE_ONHOLDPENDTRANSFER);
                    }
                    else if (lpAddrChange->wNewState == ADDRESSSTATE_DIALTONE)
                    {   
                        // Request must complete before consultation callstate may 
						// be changed as soon as it is completed, the request struture 
						// is deleted, so save off the call appearance of the 
						// consultation call.
                        _TSP_ASSERTE (lpAddrChange->wStateInfo == DIALTONETYPE_INTERNAL);
                        CTSPICallAppearance* pCall = pReq->GetConsultationCall();
                        CompleteRequest(pReq, 0);

                        // Send the initial callstate change for this call - we must
                        // supply a media mode.
                        pCall->SetCallState(LINECALLSTATE_DIALTONE, LINEDIALTONEMODE_INTERNAL, 
                                        LINEMEDIAMODE_INTERACTIVEVOICE);
                    }
                    else 
						CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
					return true;
                }                     
            }
            break;
    }

    // If we failed, then complete the request with an error - note the
	// emulator doesn't tell us WHICH request failed, we have to rely on the
	// knowledge that the error had to do with our address and therefore was
	// probably our request.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR &&
		(pevBuff->dwError == EMERROR_INVALADDRESSID ||
		 pevBuff->dwAddress == pReq->GetAddressInfo()->GetAddressID()))
	{
		CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
							  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
						LINEERR_OPERATIONFAILED);
		return true;
	}

	// Let the request flow through the unsolicited handler
    return false;

}// CDSLine::processSetupXfer

/*****************************************************************************
** Procedure:  CDSLine::processCompleteXfer
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Complete a transfer request (lineCompleteTransfer)
**
*****************************************************************************/
bool CDSLine::processCompleteXfer(RTCompleteTransfer* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddrConsult = pReq->GetConsultationCall()->GetAddressOwner();
    CTSPIAddressInfo* pAddrCall = pReq->GetCallToTransfer()->GetAddressOwner();
    bool fProcessed = false;

    switch (pReq->GetState())
    {                      
        // Step 1:
        // Send a transfer request with no address information.  This will
        // complete the transfer request already pending on the switch.
        case STATE_INITIAL:   
            if (pReq->GetTransferMode() == LINETRANSFERMODE_TRANSFER)
            {
                pReq->SetState(STATE_WAITFOROFFLINE);
                if (!GetDeviceInfo()->DRV_Transfer(pAddrCall->GetAddressID(), "", pAddrConsult->GetAddressID()))
                {
                    fProcessed = true;
                    pevBuff->dwResponse = EMRESULT_ERROR;
                }
            }
            else if (pReq->GetTransferMode() == LINETRANSFERMODE_CONFERENCE)
            {   
                pReq->SetState(STATE_WAITFORCONF);
                if (GetDeviceInfo()->DRV_Transfer(pAddrCall->GetAddressID(), "", pAddrConsult->GetAddressID(), true))
                    pReq->GetConferenceCall()->SetCallState(LINECALLSTATE_ONHOLDPENDCONF, 0, LINEMEDIAMODE_INTERACTIVEVOICE);
                else
                {
                    fProcessed = true;
                    pevBuff->dwResponse = EMRESULT_ERROR;
                }
            }                
            break;
                  
        // Step 2:
        // Wait for the switch to signal that we are in the proper state.
        case STATE_WAITFOROFFLINE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = reinterpret_cast<const LPEMADDRESSCHANGE>(pevBuff->lpBuff);
                if (lpAddrChange->wAddressID == pAddrCall->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_OFFLINE)
                    {   
                        CTSPICallAppearance* pCall = pReq->GetCallToTransfer();
                        CTSPICallAppearance* pConsult = pReq->GetConsultationCall();
                        CompleteRequest(pReq, 0);    
                        pCall->SetCallState(LINECALLSTATE_IDLE);
                        pConsult->SetCallState(LINECALLSTATE_IDLE);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;    
                    fProcessed = true;
                }                     
            }
            break;
        
        // Step 3:
        // Wait for conference notification.
        case STATE_WAITFORCONF:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddrCall->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_INCONF)
                    {   
                        CTSPICallAppearance* pCall = pReq->GetCallToTransfer();
                        CTSPICallAppearance* pConsult = pReq->GetConsultationCall();
                        CTSPICallAppearance* pConf = pReq->GetConferenceCall();
                        CompleteRequest(pReq, 0);    
                        pCall->SetCallState(LINECALLSTATE_CONFERENCED);
                        pConsult->SetCallState(LINECALLSTATE_CONFERENCED);
                        pConf->SetCallState(LINECALLSTATE_CONNECTED);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;    
                    fProcessed = true;
                }
                // Ignore the consultant call.
                else if (lpAddrChange->wAddressID == pAddrConsult->GetAddressID())
                {
                    _TSP_ASSERTE (lpAddrChange->wNewState == ADDRESSSTATE_INCONF);
                    fProcessed = true;                                      
                }
            }
            break;
            
        default:
            _TSP_ASSERT (false);
            break;
    }

    // If we failed, then complete the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
            pevBuff->dwAddress == pAddrCall->GetAddressID() ||
            pevBuff->dwAddress == pAddrConsult->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processCompleteXfer

/*****************************************************************************
** Procedure:  CDSLine::processForward
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Process a forwarding request (lineForward)
**
*****************************************************************************/
bool CDSLine::processForward(RTForward* pReq, LPCVOID lpBuff)
{
	USES_CONVERSION;
    bool fProcessed = false;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    LPCSTR lpszDest = NULL;
    
    // See if there are any forwarding entries - in our sample we only support
    // one forward request.  If multiple were supported, the 'arrForwardInfo' would
    // have more than one entry in it.
    if (pReq->GetForwardingAddressCount() > 0)
    {
        TSPIFORWARDINFO* pInfo = pReq->GetForwardingInfo(0);
        _TSP_ASSERTE (pInfo != NULL);                                          
        if (pInfo->arrDestAddress.size() > 0)
        {
            DIALINFO* pDialInfo = pInfo->arrDestAddress[0];
            lpszDest = T2CA(pDialInfo->strNumber.c_str());
        }
    }
                                   
    // TODO: Implement "no answer" ring count
    // TODO: Implement management of Call Params in lpForwData->lpCallParams.                                   
                                   
    switch (pReq->GetState())
    {
        // Step 1:
        // Tell the switch to forward or un-forward our calls.
        case STATE_INITIAL:
            pReq->SetState (STATE_CHECKLAMP);
            if (!GetDeviceInfo()->DRV_Forward(0xffffffff, lpszDest))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }                
            break;
            
        // Step 2:
        // Watch for the FORWARD lamp to move to the blinking state
        case STATE_CHECKLAMP:
            if (pevBuff->dwResponse == EMRESULT_LAMPCHANGED)
            {                           
                LPEMLAMPCHANGE lpLamp = (LPEMLAMPCHANGE) pevBuff->lpBuff;
                if ((lpszDest == NULL && lpLamp->wLampState == LAMPSTATE_OFF) ||
                    (lpszDest != NULL && lpLamp->wLampState == LAMPSTATE_BLINKING))
                    CompleteRequest(pReq, 0);
            }
            break;        
            
        default:
            _TSP_ASSERT (false);
            break;
    }

    // If we failed, then complete the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == 0xffff)
        {
            CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;
    
}// CDSLine::processForward

/*****************************************************************************
** Procedure:  CDSLine::processSetupConf
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Setup a conference event (linePrepareConference)
**
*****************************************************************************/
bool CDSLine::processSetupConf(CTSPIRequest* pReq, LPCVOID lpBuff)
{
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    bool fProcessed = false;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
	CTSPIConferenceCall* pConfCall = NULL;
	CTSPICallAppearance* pConsCall = NULL;
	CTSPICallAppearance* pCall = NULL;
	if (pReq->GetCommand() == REQUEST_SETUPCONF)
	{
		RTSetupConference* pConf = (RTSetupConference*) pReq;
		pConfCall = pConf->GetConferenceCall();
		pConsCall = pConf->GetConsultationCall();
		pCall= pConf->GetOriginalCall();
	}	
	else
	{
		RTPrepareAddToConference* pConf = (RTPrepareAddToConference*) pReq;
		pConfCall = pConf->GetConferenceCall();
		pConsCall = pConf->GetConsultationCall();
	}

    switch (pReq->GetState())
    {
        // Step 1:   
        // Send the switch a conference command ADD - we should see a request for
        // onHold, followed by a dialtone.
        case STATE_INITIAL:
            pReq->SetState(STATE_CHECKFORHOLD);
			if (!GetDeviceInfo()->DRV_Conference (pAddr->GetAddressID(), 0xffff, CONFCOMMAND_ADD))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
            
        // Step 2:
        // Watch for our address changes
        case STATE_CHECKFORHOLD:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_ONHOLD &&
                        lpAddrChange->wStateInfo == HOLDTYPE_CONFERENCE)
                    {                                                
                        // Send back response of OK since we see we are moving
                        // to the conference.
                        CompleteRequest(pReq, 0, true, false);
                        if (pCall != NULL)
                            pCall->SetCallState(LINECALLSTATE_CONFERENCED);
                        pConfCall->SetCallState(LINECALLSTATE_ONHOLDPENDCONF, 0, LINEMEDIAMODE_INTERACTIVEVOICE);
                    }
                    else if (lpAddrChange->wNewState == ADDRESSSTATE_DIALTONE)
                    {                                                             
                        _TSP_ASSERTE (lpAddrChange->wStateInfo == DIALTONETYPE_INTERNAL);
                        // Supply media mode for initial callstate - we only support
                        // interactive voice for conferenced calls.
                        pConsCall->SetCallState(LINECALLSTATE_DIALTONE, LINEDIALTONEMODE_INTERNAL, 
                                            LINEMEDIAMODE_INTERACTIVEVOICE);
                        CompleteRequest(pReq, 0);
                    } 
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;
                }                     
            }
            break;

        default:
            _TSP_ASSERT(false);
            break;
    }

    // If we failed, then complete the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == 0xffff ||
            pevBuff->dwAddress == pAddr->GetAddressID())
        {   
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processSetupConf

/*****************************************************************************
** Procedure:  CDSLine::processAddConf
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Add a new party to a conference (lineAddConference)
**
*****************************************************************************/
bool CDSLine::processAddConf(RTAddToConference* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Tell the switch to complete the conference and tie the two
        // calls together.
        case STATE_INITIAL:
            pReq->SetState(STATE_CHECKFORHOLD);
            if (!GetDeviceInfo()->DRV_Conference (pReq->GetConferenceCall()->GetAddressOwner()->GetAddressID(),
                                pReq->GetConsultationCall()->GetAddressOwner()->GetAddressID(),
                                CONFCOMMAND_ADD))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
            
        // Step 2:
        // Watch for our address changes
        case STATE_CHECKFORHOLD:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pReq->GetConferenceCall()->GetAddressOwner()->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_INCONF)
                    {
                        pReq->GetConsultationCall()->SetCallState(LINECALLSTATE_CONFERENCED);
                        pReq->GetConferenceCall()->SetCallState(LINECALLSTATE_CONNECTED);
                        CompleteRequest(pReq, 0);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;
                }                     
                else if (lpAddrChange->wAddressID == pReq->GetConsultationCall()->GetAddressOwner()->GetAddressID())
                {                     
                    _TSP_ASSERTE (lpAddrChange->wNewState == ADDRESSSTATE_INCONF);
                    fProcessed = true;
                }
            }
            break;
            
        default:
            _TSP_ASSERT(false);
            break;
    }

    // If we failed, then complete the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == 0xffff ||
            pevBuff->dwAddress == pAddr->GetAddressID() ||
            pevBuff->dwAddress == pReq->GetConsultationCall()->GetAddressOwner()->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processAddConf

/*****************************************************************************
** Procedure:  CDSLine::processRemoveConf
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Remove a party from an existing conference call
**
*****************************************************************************/
bool CDSLine::processRemoveConf(RTRemoveFromConference* pReq, LPCVOID /*lpBuff*/)
{
	if (pReq->GetState() == STATE_INITIAL)
	{
		pReq->SetState(STATE_WAITFORCOMPLETE);
		CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();

		// We assume the remove works.  Call state changes will be reflected automatically.
		if (GetDeviceInfo()->DRV_Conference(pAddr->GetAddressID(), (DWORD)-1L, CONFCOMMAND_REMOVE) != 0)
		{
			// Complete the request BEFORE idle'ing the call or TAPI will return an
			// error indicating that the remove failed.
			CTSPICallAppearance* pCall = pReq->GetCallToRemove();
			CompleteRequest(pReq, 0);
			pCall->SetCallState(LINECALLSTATE_IDLE);
		}
		else            
			CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
	}
    
	// Let it continue to process the emulator response since we didn't.
    return false;

}// CDSLine::processRemoveConf

/*****************************************************************************
** Procedure:  CDSLine::processBlindXfer
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Process an automated (blind) transfer of a call (lineBlindTransfer)
**
*****************************************************************************/
bool CDSLine::processBlindXfer(RTBlindTransfer* pReq, LPCVOID lpBuff)
{
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    DIALINFO* pDialInfo = pReq->GetDialableNumber(0);
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Ask the switch to perform the transfer - since all the data is
        // here to perform the transfer, we can simply wait for the appearance
        // to go idle.
        case STATE_INITIAL:
            pReq->SetState(STATE_WAITFOROFFLINE);
            if (!GetDeviceInfo()->DRV_Transfer(pAddr->GetAddressID(), T2CA(pDialInfo->strNumber.c_str())))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
            
        // Step 2:
        // The call should go idle.
        case STATE_WAITFOROFFLINE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_OFFLINE)
                    {   
                        pCall->SetCallState(LINECALLSTATE_IDLE);
                        CompleteRequest(pReq, 0);
                    }
                    else if (lpAddrChange->wNewState == ADDRESSSTATE_ONHOLD &&
                             lpAddrChange->wStateInfo == HOLDTYPE_TRANSFER)
                    {
                        pCall->SetCallState(LINECALLSTATE_ONHOLDPENDTRANSFER);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;    
                    fProcessed = true;
                }                     
            }
            break;
            
        default:
            _TSP_ASSERTE (false);
            break;
    }

    // If we failed, then error the request.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processBlindXfer

/*****************************************************************************
** Procedure:  CDSLine::processMakeCall
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Handle the mechanics of taking the switch off hook and 
**              dialing another party (lineMakeCall)
**
*****************************************************************************/
bool CDSLine::processMakeCall(RTMakeCall* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    bool fProcessed = false;
	LPLINECALLPARAMS lpcp = pReq->GetCallParameters();

    switch (pReq->GetState())
    {
        // Step 1:
        // Attach the call appearance to an address on our device using the
        // address ID information.  Take the address off hook in preparation for
        // dialing.  We inserted the addresses in the order that the switch gave them
        // to us, so the address ID should match what the switch says.
        case STATE_INITIAL:
            pReq->SetState(STATE_CHECKFORDIALTONE);

			// Dump out LINECALLPARAMS data
			if (lpcp)
				_TSP_DTRACE(
					_T("LINECALLPARAMS:\n")
					_T(" OrigAddress : %s\n")
					_T(" DisplayableAddress : %s\n")
					_T(" CalledParty : %s\n")
					_T(" Comment : %s\n")
					_T(" TargetAddress : %s\n")
					_T(" DeviceClass : %s\n")
					_T(" CallingPartyID : %s\n"),
					(lpcp->dwOrigAddressSize) ? (LPBYTE)lpcp+lpcp->dwOrigAddressOffset : NULL,
					(lpcp->dwDisplayableAddressSize) ? (LPBYTE)lpcp+lpcp->dwDisplayableAddressOffset : NULL,
					(lpcp->dwCalledPartySize) ? (LPBYTE)lpcp+lpcp->dwCalledPartyOffset : NULL,
					(lpcp->dwCommentSize) ? (LPBYTE)lpcp+lpcp->dwCommentOffset : NULL,
					(lpcp->dwTargetAddressSize) ? (LPBYTE)lpcp+lpcp->dwTargetAddressOffset : NULL,
					(lpcp->dwDeviceClassSize) ? (LPBYTE)lpcp+lpcp->dwDeviceClassOffset : NULL,
					(lpcp->dwCallingPartyIDSize) ? (LPBYTE)lpcp+lpcp->dwCallingPartyIDOffset : NULL);

            if (!GetDeviceInfo()->DRV_PrepareCall (pAddr->GetAddressID()))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
        
        // State 2:
        // Transition to the "DIALTONE" state when we get a notification that
        // the phone is in DIALTONE.
        case STATE_CHECKFORDIALTONE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                // Give TAPI a "good" result back indicating that the switch
                // has begun allocating trunk lines and such - it will notify us
                // when DIALTONE has been received.
                //
                // Do not delete the request since more work needs to be accomplished.
                CompleteRequest(pReq, 0, true, false);

				// Determine which address changed.
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    // Transition to a dialtone, and move to a DIAL state.
                    if (lpAddrChange->wNewState == ADDRESSSTATE_DIALTONE)
                    {
						// Set the new callstate
                        pCall->SetCallState(LINECALLSTATE_DIALTONE, g_DialToneStates[lpAddrChange->wStateInfo],
                                            LINEMEDIAMODE_INTERACTIVEVOICE);

						// Set a call-id for the call - the emulator doesn't use
						// call-ids so we fabricate one here.
						pCall->SetCallID(GetTickCount());
                        
                        // If we have addresse(s) to dial, then move to the
                        // dialing state, otherwise complete the request here and
                        // leave the call in the "dialtone" state.
                        if (pReq->GetCount() == 0)
                            CompleteRequest(pReq, 0);
                        else
                        {
                            pReq->SetState(STATE_DIALING);
                            HandleDialEvent(pReq, pevBuff);
                        }    
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;                        
                }
            }
            break;                
        
        // Step 3-xx
        // Process the dial request.  Send off to a master handler for
        // dialing which will field requests from a MAKECALL and DIAL event.
        case STATE_DIALING:
        case STATE_WAITFORONLINE:
        case STATE_WAITFORCONNECT:
            return HandleDialEvent(pReq, pevBuff);
        
        // Should never get here.    
        default:
            _TSP_ASSERT(false);
            break;
    }
    
    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
            pCall->SetCallState(LINECALLSTATE_IDLE);
            CompleteRequest(pReq, LINEERR_CALLUNAVAIL);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processMakeCall

/*****************************************************************************
** Procedure:  CDSLine::processGenDigits
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Generate a series of tones or pulses on the call. (lineGenerateDigits)
**
*****************************************************************************/
bool CDSLine::processGenDigits(RTGenerateDigits* pReq, LPCVOID /*lpBuff*/)
{
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();

	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
	    DIALINFO ds;
	    ds.strNumber = pReq->GetDigits();
	    if (GetDeviceInfo()->DRV_Dial(pAddr->GetAddressID(), &ds, 0))
	        CompleteRequest(pReq, 0);
	    else
	        CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
	}
    return false;        
        
}// CDSLine::processGenDigits

/*****************************************************************************
** Procedure:  CDSLine::processHold
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Transition a call to the OnHold status (lineHold)
**
*****************************************************************************/
bool CDSLine::processHold(RTHold* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Ask the switch to place the call on hold
        case STATE_INITIAL:                        
            pReq->SetState (STATE_CHECKFORHOLD);
            if (!GetDeviceInfo()->DRV_HoldCall(pAddr->GetAddressID()))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
 
        // State 2:
        // Transition to the "ONHOLD" state when we get a notification that
        // the phone is in a holding pattern.
        case STATE_CHECKFORHOLD:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_ONHOLD)
                    {
                        pCall->SetCallState(LINECALLSTATE_ONHOLD);
                        CompleteRequest(pReq, 0);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;                        
                }
            }
            break;  
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then kill the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processHold

/*****************************************************************************
** Procedure:  CDSLine::processUnhold
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Transition a call which is currently holding on the switch
**              back to the previous state (normally Connected).
**
*****************************************************************************/
bool CDSLine::processUnhold(RTUnhold* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    CTSPICallAppearance* pConsult = pCall->GetConsultationCall();
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Ask the switch to place the call on hold
        case STATE_INITIAL:                        
            pReq->SetState (STATE_CHECKFORHOLD);
            if (!GetDeviceInfo()->DRV_UnholdCall(pAddr->GetAddressID()))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
 
        // State 2:
        // Transition to the "ONHOLD" state when we get a notification that
        // the phone is in a holding pattern.
        case STATE_CHECKFORHOLD:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState != ADDRESSSTATE_ONHOLD)
                    {                         
                        switch (lpAddrChange->wNewState)
                        {
                            case ADDRESSSTATE_DIALTONE:
                                pCall->SetCallState(LINECALLSTATE_DIALTONE, LINEDIALTONEMODE_UNKNOWN);
                                CompleteRequest(pReq, 0);
                                break;
                            
                            case ADDRESSSTATE_ONLINE:
                                pCall->SetCallState(LINECALLSTATE_PROCEEDING);
                                CompleteRequest(pReq, 0);
                                break;
                                
                            case ADDRESSSTATE_CONNECT:
                                pCall->SetCallState(LINECALLSTATE_CONNECTED);
                                CompleteRequest(pReq, 0);
                                break;

                            case ADDRESSSTATE_DISCONNECT:
                                if (pConsult != NULL)
                                    pConsult->SetCallState(LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NORMAL);
                                else                                    
                                    pCall->SetCallState(LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NORMAL);
                                break;
                                
                            case ADDRESSSTATE_OFFLINE:
                                if (pConsult != NULL)
                                    pConsult->SetCallState(LINECALLSTATE_IDLE);
                                else
                                {
                                    pCall->SetCallState(LINECALLSTATE_IDLE);
                                    CompleteRequest(pReq, 0);
                                }                                   
                                break;
                            
                            case ADDRESSSTATE_INCONF:
                                _TSP_ASSERTE (pCall->GetCallType() == CTSPICallAppearance::Conference);
                                pCall->SetCallState(LINECALLSTATE_CONNECTED);
                                CompleteRequest(pReq, 0);
                                break;
                                
                            default:
                                pevBuff->dwResponse = EMRESULT_ERROR;
                                break;
                        }                                                            
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;                        
                }
            }
            break;  
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then kill the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processUnhold

/*****************************************************************************
** Procedure:  CDSLine::processSwapHold
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Swap the current call with the call waiting on hold (lineSwapHold).
**
*****************************************************************************/
bool CDSLine::processSwapHold(RTSwapHold* pReq, LPCVOID lpBuff)
{
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetActiveCall();
    CTSPICallAppearance* pHeldCall = pReq->GetHoldingCall();
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // If the two calls are on different addresses, then place the 
        // active call into the holding pattern.  Otherwise, if they are
        // on the same address (consultation) then issue a "flash" command
        // to the switch.
        case STATE_INITIAL:                        
            if (pCall->GetAddressOwner() == pHeldCall->GetAddressOwner())
            {
                pReq->SetState(STATE_WAITFORONLINE);        
                if (!GetDeviceInfo()->DRV_Flash(pAddr->GetAddressID()))
                {
                    fProcessed = true;
                    pevBuff->dwResponse = EMRESULT_ERROR;
                }     
            }
            else
            { 
                pReq->SetState (STATE_CHECKFORHOLD);
                if (!GetDeviceInfo()->DRV_HoldCall (pAddr->GetAddressID()))
                {
                    fProcessed = true;
                    pevBuff->dwResponse = EMRESULT_ERROR;
                }     
            }                
            break;
 
        // State 2:
        // Watch for the ACTIVE call to move to the hold state, and
        // when it does, re-activate the original held call (take it offhold).
        case STATE_CHECKFORHOLD:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_ONHOLD)
                    {                                                  
                        if (GetDeviceInfo()->DRV_UnholdCall(pHeldCall->GetAddressOwner()->GetAddressID()))
                        {   
                            pReq->SetState(STATE_WAITFORCONNECT);
                            pCall->SetCallState(LINECALLSTATE_ONHOLD);
                        }
                        else
                            pevBuff->dwResponse = EMRESULT_ERROR;
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;                        
                }
            }
            break;  
            
        // State 3:
        // Watch for the held call to move out of the held state.  It might
        // not move to the connected state - it could be at a dialtone, or
        // ringing, etc.
        case STATE_WAITFORCONNECT:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pHeldCall->GetAddressOwner()->GetAddressID())
                {   
                    if (lpAddrChange->wNewState != ADDRESSSTATE_ONHOLD)
                    {
                        pHeldCall->SetCallState(g_CallStates[lpAddrChange->wNewState]);
                        CompleteRequest(pReq, 0);
                    }
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;                        
                }
            }
            break;  
        
        // State 2:2
        // Watch for the swap signal on the same address.  We simply will watch
        // for a non-onHold state.  This is used in the case of a swap on two
        // call appearances on a single address (i.e. consultation call)
        case STATE_WAITFORONLINE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState != ADDRESSSTATE_ONHOLD)
                    {   
                        // Make sure to switch with the correct type of hold - 
                        // otherwise completeXfer and completeConf won't work
                        // with this call anymore.
                        DWORD dwHoldType = pHeldCall->GetCallState();
                        _TSP_ASSERTE (dwHoldType == LINECALLSTATE_ONHOLD ||
                                dwHoldType == LINECALLSTATE_ONHOLDPENDCONF ||
                                dwHoldType == LINECALLSTATE_ONHOLDPENDTRANSFER);  
                        pCall->SetCallState(dwHoldType);
                        pHeldCall->SetCallState(g_CallStates[lpAddrChange->wNewState]);
                        CompleteRequest(pReq, 0);
                    }
                    fProcessed = true;                        
                }
            }
            break;  
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then kill the request
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
            CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processSwapHold

/*****************************************************************************
** Procedure:  CDSLine::processDial
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Dial digits on a call (lineDial).
**
*****************************************************************************/
bool CDSLine::processDial(RTDial* pReq, LPCVOID lpBuff)
{
    return HandleDialEvent(pReq, (EVENTBUFF*)lpBuff);

}// CDSLine::processDial

/*****************************************************************************
** Procedure:  CDSLine::processDropCall
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Drop a call off the switch (lineDrop).
**
*****************************************************************************/
bool CDSLine::processDropCall(RTDropCall* pReq, LPCVOID /*lpBuff*/)
{
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    bool fProcessed = false;
    
    // If this is a conference call, then don't send the drop request to the
    // switch - it is a "fake" call appearance.
    if (pCall->GetCallType() == CTSPICallAppearance::Conference)
	{
		if (pReq->EnterState(STATE_INITIAL, STATE_WAITFORCOMPLETE))
		{
			CTSPIConferenceCall* pCall = (CTSPIConferenceCall*) pReq->GetCallInfo();
			pReq->SetState(STATE_WAITFORCOMPLETE);

			// Idle any attached call to the conference.  This would be a consultation
			// call created for adding a new member but not yet in conferenced state.
			CTSPICallAppearance* pThisCall = pCall->GetConsultationCall();
			if (pThisCall != NULL)
				pThisCall->SetCallState(LINECALLSTATE_IDLE);
    
			// Idle all the calls in this conference.  As each call is IDLE'd,
			// it is automatically removed from the conference array.
			while (pCall->GetConferenceCount() > 0)
			{   
				CTSPICallAppearance* pThisCall = pCall->GetConferenceCall(0);
				pThisCall->SetCallState(LINECALLSTATE_IDLE);
			}

			// Complete the request
			CompleteRequest(pReq, 0);

			// Issue the DROP request to the switch
			GetDeviceInfo()->DRV_Conference(pAddr->GetAddressID(), (DWORD)-1L, CONFCOMMAND_DESTROY);
		}
		return false;    
	}
	else    
    {   
		if (pReq->EnterState(STATE_INITIAL, STATE_WAITFORCOMPLETE))
		{
			GetDeviceInfo()->DRV_DropCall (pAddr->GetAddressID());
			CompleteRequest(pReq, 0);
			pCall->SetCallState(LINECALLSTATE_IDLE);
		}
	}
	
    return fProcessed;

}// CDSLine::processDropCall

/*****************************************************************************
** Procedure:  CDSLine::processRedirect
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description:  Redirect a call to another station.
**
*****************************************************************************/
bool CDSLine::processRedirect(RTRedirect* pReq, LPCVOID lpBuff)
{                             
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
	DIALINFO* pDialInfo = (pReq->GetCount() > 0) ? pReq->GetDialableNumber(0) : NULL;
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Ask the switch emulator to answer the call - this would be to press
        // the button associated with the offering address.
        case STATE_INITIAL:
            pReq->SetState (STATE_WAITFOROFFLINE);
            if (!GetDeviceInfo()->DRV_Redirect(pAddr->GetAddressID(), T2CA(pDialInfo->strNumber.c_str()), pReq->GetCountryCode()))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;
        
        // Step 2:
        // Address should indicate an offline condition.
        case STATE_WAITFOROFFLINE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_OFFLINE)
                    {
                        CompleteRequest(pReq, 0);
                        pCall->SetCallState(LINECALLSTATE_IDLE);
                    }                        
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;             
                }
            }
            break;                
        
        default:
            _TSP_ASSERT (false);
            break;
    }
                    
    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processRedirect

/*****************************************************************************
** Procedure:  CDSLine::processCompleteCall
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Wait for a call completion on an existing call. (lineCompleteCall)
**
*****************************************************************************/
bool CDSLine::processCompleteCall(RTCompleteCall* pReq, LPCVOID lpBuff)
{
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // Tell the switch to complete the call.
        case STATE_INITIAL:       
            {                    
                pReq->SetState (STATE_WAITFORCOMPLETE);
                CString strMessage = _T("");
                if (pReq->GetCompletionMode() == LINECALLCOMPLMODE_MESSAGE)
                    strMessage = g_ComplMsgs[pReq->GetMessageID()];
                if (!GetDeviceInfo()->DRV_CompleteCall (pAddr->GetAddressID(), 
                        pReq->GetCompletionMode(), T2CA(strMessage)))
                {
                    fProcessed = true;
                    pevBuff->dwResponse = EMRESULT_ERROR;
                }
            }
            break;

        // Step 2:
        // Switch sends back OK response for call completion.
        case STATE_WAITFORCOMPLETE:
            if (pevBuff->dwResponse == EMRESULT_COMPLRESULT)
            {
				LPEMCOMPLETECALL emComplete = (LPEMCOMPLETECALL) pevBuff->lpBuff;
                if (emComplete->wAddressID == pAddr->GetAddressID())
                {   
                    // Save off data which gets deleted when CompleteRequest() occurs.                            
                    DWORD dwCompletionMode = pReq->GetCompletionMode();
                    DWORD dwCompletionID = pReq->GetCompletionID();
                    
                    // Save off the switch identifier and complete the request.
                    pReq->SetIdentifier((DWORD) emComplete->wCompletionType);
                    CompleteRequest(pReq, 0);
                    
                    // If this is a message/intrude completion request, then
                    // remove the completion request from the list.  We will still
                    // get notified from the switch, but this is easier in hooking
                    // the call up.
                    if (dwCompletionMode == LINECALLCOMPLMODE_MESSAGE)
                        RemoveCallCompletionRequest (dwCompletionID);
                    else if (dwCompletionMode == LINECALLCOMPLMODE_INTRUDE)
                    {
                        pCall->GetCallInfo()->dwCompletionID = dwCompletionID;
                        pCall->SetCallReason(LINECALLREASON_CALLCOMPLETION);
                        RemoveCallCompletionRequest (dwCompletionID);
                    }
                    fProcessed = true;
                }
            }
            break;
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processCompleteCall

/*****************************************************************************
** Procedure:  CDSLine::processPark
**
** Arguments:  'pReq' - Request object we are working with
**             'pevBuff->dwResponse' - Current response from emulator
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Park a call on the switch
**
*****************************************************************************/
bool CDSLine::processPark (RTPark* pReq, LPCVOID lpBuff)
{                   
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    const TCHAR * pszNonDirAddr = _T("5551212");
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    CString strNum = (pReq->GetParkMode() == LINEPARKMODE_DIRECTED) ? pReq->GetDialableNumber() : pszNonDirAddr;
    bool fProcessed = false;

    switch (pReq->GetState())
    {
        // Step 1:
        // If this is a NON-directed park, then fill it in with a bogus address
        // which we park everything into.
        case STATE_INITIAL:
            if (pReq->GetParkMode() == LINEPARKMODE_NONDIRECTED)
                pReq->SetParkedAddress(pszNonDirAddr);
            pReq->SetState (STATE_WAITFORCOMPLETE);
            if (!GetDeviceInfo()->DRV_Park(pAddr->GetAddressID(), T2CA(strNum)))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;

        // Step 2:
        // Switch idles call
        case STATE_WAITFORCOMPLETE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_OFFLINE)
                    {
                        CompleteRequest(pReq, 0);
                        pCall->SetCallState(LINECALLSTATE_IDLE);
                    }                        
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;             
                }
            }
            break;                
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || 
			pevBuff->dwError == EMERROR_INVALADDRESSID ||
			pevBuff->dwAddress == pAddr->GetAddressID())
        {
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSLine::processPark

/*****************************************************************************
** Procedure:  CDSLine::processUnpark
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Unpark a call from the switch (lineUnpark)
**
*****************************************************************************/
bool CDSLine::processUnpark (RTUnpark* pReq, LPCVOID lpBuff)
{                             
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*)lpBuff;
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    bool fProcessed = false;
	DIALINFO* pDialInfo = (pReq->GetCount() > 0) ? pReq->GetDialableNumber(0) : NULL;
                              
    switch (pReq->GetState())
    {
        // Step 1:
        // Attempt to unpark a call at the specified address.  The emulator
        // will return an error if no call is parked there.
        case STATE_INITIAL:
            pReq->SetState (STATE_WAITFORCOMPLETE);
            if (!GetDeviceInfo()->DRV_Unpark(pAddr->GetAddressID(), T2CA(pDialInfo->strNumber.c_str())))
            {
                fProcessed = true;
                pevBuff->dwResponse = EMRESULT_ERROR;
            }
            break;

        // Step 2:
        // Switch idles call
        case STATE_WAITFORCOMPLETE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == pAddr->GetAddressID())
                {   
                    if (lpAddrChange->wNewState == ADDRESSSTATE_CONNECT)
                    {
                        CompleteRequest(pReq, 0);
                        pCall->SetCallReason (LINECALLREASON_UNPARK);
                        pCall->SetCallState(LINECALLSTATE_CONNECTED, 0, LINEMEDIAMODE_INTERACTIVEVOICE);
                    }                        
                    else
                        pevBuff->dwResponse = EMRESULT_ERROR;
                    fProcessed = true;             
                }
            }
            break;                
            
        default:
            _TSP_ASSERT (false);
            break;
    }
    
    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {       
		if (pevBuff->dwError == EMERROR_NOPARKINFO)
			CompleteRequest(pReq, LINEERR_OPERATIONUNAVAIL);
        else
			CompleteRequest(pReq, (pevBuff->dwError == EMERROR_INVALADDRESSSTATE) ? LINEERR_INVALCALLSTATE : 
								  (pevBuff->dwError == EMERROR_INVALADPARAM) ? LINEERR_INVALPARAM : 
							LINEERR_OPERATIONFAILED);
        fProcessed = true;
    }
    return fProcessed;
                              
}// CDSLine::processUnpark

/*****************************************************************************
** Procedure:  CDSLine::processPickup
**
** Arguments:  'pReq' - Request object we are working with
**             'lpBuff' - Current data structure from emulator
**
** Returns:    true/false success code
**
** Description: Pickup a call from a remote station. (linePickup)
**
*****************************************************************************/
bool CDSLine::processPickup (RTPickup* pReq, LPCVOID /*lpBuff*/)
{                             
	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
	    // TODO: Implement pickup in emulator.
	    CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
	}
    return false;
    
}// CDSLine::processPickup
