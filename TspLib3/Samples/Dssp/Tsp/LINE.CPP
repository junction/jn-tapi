/******************************************************************************/
//                                                                        
// LINE.CPP - Digital Switch Service Provider Sample
//                                                                        
// This file contains the line device code for the service provider.
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
/******************************************************************************/

#include "stdafx.h"
#include "dssp.h"
#pragma warning(disable:4201)
#include <mmsystem.h>
#pragma warning(default:4201)

/*----------------------------------------------------------------------------
	GLOBAL DATA
-----------------------------------------------------------------------------*/

// Available completion messages
LPCTSTR g_ComplMsgs[] = {
	{ _T("Completion Message #1") },
	{ _T("Completion Message #2") },
	{ _T("Completion Message #3") },
	{ _T("Completion Message #4") },
	NULL 
};  

// Array to translate Call states
DWORD g_CallStates[] = {
    LINECALLSTATE_UNKNOWN,
    LINECALLSTATE_CONNECTED,
    LINECALLSTATE_BUSY,
    LINECALLSTATE_DISCONNECTED,
    LINECALLSTATE_DIALTONE,
    LINECALLSTATE_PROCEEDING,
    LINECALLSTATE_IDLE,
    LINECALLSTATE_OFFERING,
    LINECALLSTATE_ONHOLD,
    LINECALLSTATE_CONFERENCED
};    

/*-------------------------------------------------------------------------------*/
// TSPI Request map
/*-------------------------------------------------------------------------------*/
BEGIN_TSPI_REQUEST(CDSLine)
	ON_AUTO_TSPI_REQUEST(REQUEST_ACCEPT)
	ON_AUTO_TSPI_REQUEST(REQUEST_SETTERMINAL)
	ON_AUTO_TSPI_REQUEST(REQUEST_SECURECALL)
	ON_AUTO_TSPI_REQUEST(REQUEST_GENERATETONE)
	ON_AUTO_TSPI_REQUEST(REQUEST_SENDUSERINFO)
    ON_AUTO_TSPI_REQUEST(REQUEST_RELEASEUSERINFO)
	ON_AUTO_TSPI_REQUEST(REQUEST_UNCOMPLETECALL)
	ON_AUTO_TSPI_REQUEST(REQUEST_SETCALLPARAMS)
	ON_AUTO_TSPI_REQUEST(REQUEST_SETCALLDATA)
	ON_TSPI_REQUEST_ANSWER(processAnswer)
	ON_TSPI_REQUEST_MAKECALL(processMakeCall)
	ON_TSPI_REQUEST_DIAL(processDial)
	ON_TSPI_REQUEST_PARK(processPark)
	ON_TSPI_REQUEST_PICKUP(processPickup)
	ON_TSPI_REQUEST_UNPARK(processUnpark)
	ON_TSPI_REQUEST_HOLD(processHold)
	ON_TSPI_REQUEST_UNHOLD(processUnhold)
	ON_TSPI_REQUEST_SWAPHOLD(processSwapHold)
	ON_TSPI_REQUEST_BLINDXFER(processBlindXfer)
	ON_TSPI_REQUEST_SETUPXFER(processSetupXfer)
	ON_TSPI_REQUEST_COMPLETEXFER(processCompleteXfer)
	ON_TSPI_REQUEST_DROPCALL(processDropCall)
	ON_TSPI_REQUEST_GENERATEDIGITS(processGenDigits)
	ON_TSPI_REQUEST_FORWARD(processForward)
	ON_TSPI_REQUEST_COMPLETECALL(processCompleteCall)
	ON_TSPI_REQUEST_REDIRECT(processRedirect)
	ON_TSPI_REQUEST(REQUEST_SETUPCONF, processSetupConf)
	ON_TSPI_REQUEST(REQUEST_PREPAREADDCONF, processSetupConf)
	ON_TSPI_REQUEST_ADDCONF(processAddConf)
	ON_TSPI_REQUEST_REMOVEFROMCONF(processRemoveConf)
END_TSPI_REQUEST()

/*****************************************************************************
** Procedure:  CDSLine::CDSLine
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Constructor for the line object
**
*****************************************************************************/
CDSLine::CDSLine()
{
}// CDSLine::CDSLine

/*****************************************************************************
** Procedure:  CDSLine::~CDSLine
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for the line object
**
*****************************************************************************/
CDSLine::~CDSLine()
{
}// CDSLine::~CDSLine

/*****************************************************************************
** Procedure:  CDSLine::Init
**
** Arguments:  'pDev'			-	Device object this line belongs to
**             'dwLineDeviceID'	-	Unique line identifier within the TSP
**             'dwPos'			-	Index position of line within device array
**             'dwItemData'		-   Used when line was dynamically created (P&P).
**
** Returns:    void
**
** Description:  This function is called by the device owner to initialize
**               the line object.
**
*****************************************************************************/
void CDSLine::Init (CTSPIDevice* pDev, DWORD dwLineDeviceID, DWORD dwPos, DWORD /*dwItemData*/)
{
	// Let the base class initialize first.
	CTSPILineConnection::Init (pDev, dwLineDeviceID, dwPos);

    // Set the name associated with this line.  This is optional, it gives
    // the user a displayable name which is associated with the line.  Most
	// applications use this name in their UI.
    SetName (_T("DSSP Line #1"));

    // Grab the version information from the emulator.
    EMVERSIONINFO VerInfo;
    GetDeviceInfo()->DRV_GetVersionInfo (&VerInfo);

    // Set the connection (line) info
	USES_CONVERSION;
	pDev->SetSwitchInfo(A2T(VerInfo.szSwitchInfo));

    // Add our terminal devices.  For our demo provider, we support a handset
    // only.  Mark that all elements are directed to this terminal.
    LINETERMCAPS tCaps = { LINETERMDEV_PHONE,
		(LINETERMMODE_BUTTONS | LINETERMMODE_LAMPS | LINETERMMODE_DISPLAY | LINETERMMODE_RINGER | LINETERMMODE_HOOKSWITCH | LINETERMMODE_MEDIABIDIRECT),
		LINETERMSHARING_SHAREDEXCL };
    AddTerminal(_T("Handset"), tCaps, tCaps.dwTermModes);

    // Now adjust the line device capabilities.  We don't support any of the
    // line device capability flags, and don't need dialing parameters since the
    // switch doesn't allow them to be adjusted.
    LPLINEDEVCAPS lpCaps = GetLineDevCaps();
    lpCaps->dwAnswerMode = LINEANSWERMODE_DROP;    
    lpCaps->dwMonitorDigitModes = (LINEDIGITMODE_PULSE | LINEDIGITMODE_DTMF | LINEDIGITMODE_DTMFEND);
    lpCaps->dwGenerateDigitModes = LINEDIGITMODE_DTMF;
    lpCaps->dwGenerateToneModes = LINETONEMODE_CUSTOM | LINETONEMODE_RINGBACK | LINETONEMODE_BUSY | LINETONEMODE_BEEP | LINETONEMODE_BILLING;
    lpCaps->dwGenerateToneMaxNumFreq = 3;                                                                                                    
    lpCaps->dwMonitorToneMaxNumFreq = 3;
    lpCaps->dwMonitorToneMaxNumEntries = 5;
    lpCaps->dwGatherDigitsMinTimeout = 500;		// 250 is the minimum for the TSP++ library timer thread
    lpCaps->dwGatherDigitsMaxTimeout = 32000;   
    lpCaps->dwDevCapFlags |= (LINEDEVCAPFLAGS_CROSSADDRCONF | LINEDEVCAPFLAGS_CLOSEDROP);

    // Setup the USER->USER information sizes.  We don't actually do anything with
    // the user information, but we allow the function calls to proceed as if they
    // DID do something.
    lpCaps->dwUUIAcceptSize = lpCaps->dwUUIAnswerSize = lpCaps->dwUUIMakeCallSize =\
    lpCaps->dwUUIDropSize = lpCaps->dwUUISendUserUserInfoSize = lpCaps->dwUUICallInfoSize = 1024;

    // Grab the address settings from the emulator.
    EMSETTINGS Settings;
    if (!GetDeviceInfo()->DRV_GetSwitchSettings (&Settings))
		return;

    // Go through and add each address and setup the address capabilities
    for (WORD i = 0; i < Settings.wAddressCount; i++)
    {   
		// Add the address based on the information given by the emulator.
        EMADDRESSINFO AddressInfo;
        AddressInfo.wAddressID = i;
        if (!GetDeviceInfo()->DRV_GetAddressInfo (&AddressInfo))
			continue;

        CreateAddress (A2T(AddressInfo.szAddress),	// Dialable address (phone#)
					   A2T(Settings.szOwner),		// Address name (used for identification)
						true,						// Allow incoming calls
						true,						// Allow outgoing calls
						MEDIAMODE_ALL,				// Avalabile media modes
						LINEBEARERMODE_VOICE,		// Single bearer mode for this address
                        0,							// Minimum data rate on address
						0,							// Maximum data rate on address
						NULL,						// Dialing parameters (LINEDIALPARAMS)
						1,							// Max number of active calls on address
						1,							// Max number of OnHold calls on address
						1,							// Max number of OnHoldPending calls on address
						8,							// Max number of calls in a conference on address
						3);							// Max number of calls transferred into a conference on address

        // Add the completion messages valid for this address.
        CTSPIAddressInfo* pAddr = GetAddress(i);
		if (pAddr != NULL)
		{
			for (int x = 0; g_ComplMsgs[x] != NULL; x++)
				pAddr->AddCompletionMessage (g_ComplMsgs[x]);
        
			// Adjust the ADDRESSCAPS for this address
			LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
			lpAddrCaps->dwCallCompletionModes = (LINECALLCOMPLMODE_CAMPON | 
							LINECALLCOMPLMODE_CALLBACK | LINECALLCOMPLMODE_INTRUDE | 
							LINECALLCOMPLMODE_MESSAGE);

			lpAddrCaps->dwMaxCallDataSize = 4096;
			lpAddrCaps->dwTransferModes = LINETRANSFERMODE_TRANSFER | LINETRANSFERMODE_CONFERENCE;
			lpAddrCaps->dwForwardModes = LINEFORWARDMODE_UNCOND;       
			lpAddrCaps->dwParkModes = LINEPARKMODE_DIRECTED | LINEPARKMODE_NONDIRECTED;
			lpAddrCaps->dwRemoveFromConfCaps = LINEREMOVEFROMCONF_LAST;
			lpAddrCaps->dwRemoveFromConfState = LINECALLSTATE_IDLE;
			lpAddrCaps->dwAddrCapFlags |= (LINEADDRCAPFLAGS_PARTIALDIAL | 
					LINEADDRCAPFLAGS_CONFDROP | LINEADDRCAPFLAGS_CONFERENCEMAKE |
					LINEADDRCAPFLAGS_FWDSTATUSVALID | LINEADDRCAPFLAGS_TRANSFERMAKE | 
					LINEADDRCAPFLAGS_TRANSFERHELD | LINEADDRCAPFLAGS_CONFERENCEHELD);
		}
    }

	// Add the WAV devices which will be our handset I/O
	if (waveInGetNumDevs() > 0)
		AddDeviceClass(_T("wave/in"), (DWORD)0);
	if (waveOutGetNumDevs() > 0)
		AddDeviceClass(_T("wave/out"), (DWORD)0);

    // Cancel any forwarding in effect - we cannot DETECT where it is
    // forwarded to, so we will initialize it to something we know and therefore
    // be able to correctly report forwarding status information
    GetDeviceInfo()->DRV_Forward(0xffffffff, NULL);

}// CDSLine::Init

/*****************************************************************************
** Procedure:  CDSLine::GetDevConfig
**
** Arguments:  'strDeviceClass' - Device class which is being queried.
**             'lpDeviceConfig' - VARSTRING to return the requested data to
**
** Returns:    TAPI result code
**
** Description:  This function is used by applications to query device
**               configuration from our TSP.  It is overriden to supply 
**               "dummy" information about a DATAMODEM ability so that
**               applications like HyperTerminal will talk to us.
**
*****************************************************************************/
LONG CDSLine::GetDevConfig(CString& strDeviceClass, LPVARSTRING lpDeviceConfig)
{
	if (strDeviceClass == _T("comm/datamodem"))
	{
		// We fill it in with junk.. some MODEM applications expect this
		// structure and won't talk to us unless we provide this function.
		lpDeviceConfig->dwUsedSize = lpDeviceConfig->dwNeededSize = sizeof(VARSTRING)+1;
		lpDeviceConfig->dwStringFormat = STRINGFORMAT_BINARY;
		lpDeviceConfig->dwStringOffset = sizeof(VARSTRING);
		lpDeviceConfig->dwStringSize = 1;
		return false;
	}
	return LINEERR_INVALDEVICECLASS;

}// CDSLine::GetDevConfig

/*****************************************************************************
** Procedure:  CDSLine::SetDevConfig
**
** Arguments:  'strDeviceClass' - Device class which is being set.
**             'lpDeviceConfig' - VARSTRING to set the requested data to
**             'dwSize'         - Size of the VARSTRING data
**
** Returns:    void
**
** Description:  This function is called by applications to set the
**               device configuration.  It is overriden to handle the
**               "modem" case for HyperTerminal.  It does nothing.
**
*****************************************************************************/
LONG CDSLine::SetDevConfig(CString& strDeviceClass, LPVOID const /*lpDevConfig*/, DWORD /*dwSize*/)
{
	if (strDeviceClass == _T("comm/datamodem"))
		return false;
	return LINEERR_INVALDEVICECLASS;

}// CDSLine::SetDevConfig

/*****************************************************************************
** Procedure:  CDSLine::UnsolicitedEvent
**
** Arguments: 'lpBuff'    - Data buffer (structure) specific to code
**
** Returns:    void
**
** Description:  This function processes any responses from the device which
**               are not matched to a pending line request.
**
*****************************************************************************/
bool CDSLine::UnsolicitedEvent(LPCVOID lpBuff)
{                    
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*) lpBuff;

    static DWORD dwCompletionID = 0L;
    switch (pevBuff->dwResponse)
    {   
        // An address has changed states on the device - see if it is an
        // offering call.  If so, create a new call on the address in question.
        case EMRESULT_ADDRESSCHANGED:                      
        {   
            LPEMADDRESSCHANGE lpChange = (LPEMADDRESSCHANGE) pevBuff->lpBuff;
            if (lpChange->wNewState == ADDRESSSTATE_OFFERING || lpChange->wNewState == ADDRESSSTATE_OFFERINGT)
            {
                HandleNewCall ((DWORD)lpChange->wAddressID, lpChange->dwMediaModes, dwCompletionID,
					           (lpChange->wNewState == ADDRESSSTATE_OFFERINGT));
                dwCompletionID = 0L;
            }   
            else
                UpdateCallState ((DWORD)lpChange->wAddressID, (int)lpChange->wNewState, (int)lpChange->wStateInfo, lpChange->dwMediaModes);
        }
        break;

        // An offering call is ringing.
        case EMRESULT_RING:
            OnRingDetected(1);
            break;

        // A tone (specific frequency) was detected on the device.
        case EMRESULT_TONEDETECTED:
        {
			LPEMTONEBUFF lpTone = (LPEMTONEBUFF) pevBuff->lpBuff;
            CTSPIAddressInfo* pAddr = GetAddress(lpTone->wAddressID);
            _TSP_ASSERTE (pAddr != NULL);
                
            // Call must be connected or proceeding/dialing
            CTSPICallAppearance* pCall = pAddr->FindCallByState(LINECALLSTATE_CONNECTED);
            if (pCall == NULL)
                pCall = pAddr->FindCallByState(LINECALLSTATE_PROCEEDING);
            if (pCall == NULL)
                pCall = pAddr->FindCallByState(LINECALLSTATE_DIALING);
            
            // The emulator sends the frequency as three valid entries which can then be checked together.
            if (pCall != NULL)
                pCall->OnTone (lpTone->dwFreq[0], lpTone->dwFreq[1], lpTone->dwFreq[2]);
        }
        break;
			
        // A call completion request has completed
        case EMRESULT_COMPLRESULT:
        {     
			LPEMCOMPLETECALL emComplete = (LPEMCOMPLETECALL) pevBuff->lpBuff;

            // Locate the completion request if available.
            DWORD dwReqID = emComplete->wCompletionType;
            RTCompleteCall* pComplete = FindCallCompletionRequest(dwReqID, NULL);
            if (pComplete != NULL)
            {                   
                if (pComplete->GetCompletionMode() == LINECALLCOMPLMODE_CAMPON)
                {
                    // The call for this address is about to go connected..
                    _TSP_ASSERTE (pComplete->GetCallInfo() != NULL);
                    pComplete->GetCallInfo()->GetCallInfo()->dwCompletionID = pComplete->GetCompletionID();
                    pComplete->GetCallInfo()->SetCallReason (LINECALLREASON_CALLCOMPLETION);
                }
                else if (pComplete->GetCompletionMode() == LINECALLCOMPLMODE_CALLBACK)
                    dwCompletionID = pComplete->GetCompletionID();
                RemoveCallCompletionRequest(pComplete->GetCompletionID());
            }                   
            else
                dwCompletionID = 0L;
        }
        break;
       
        // A digit was detected from the remote side - supply it to the
        // connected call appearance on the address specified for digit
        // monitoring/gathering support.
        case EMRESULT_DIGITDETECTED:
        {               
            static char cLastChar;
            LPEMDIGITBUFF lpDigit = (LPEMDIGITBUFF) pevBuff->lpBuff;
            CTSPIAddressInfo* pAddr = GetAddress(lpDigit->wAddressID);
            _TSP_ASSERTE (pAddr != NULL);
            CTSPICallAppearance* pCall = pAddr->FindCallByState(LINECALLSTATE_CONNECTED);
            if (pCall != NULL)
            {   
                DWORD dwType = LINEDIGITMODE_PULSE;
                if (lpDigit->fDTMF)
                {   
                    // The emulator switch sends a ZERO digit to indicate
                    // silence on the line after a DTMF tone detection.  We
                    // interpret this to mean that the digit is UP and fake
                    // a DTMF END tone.
                    if (lpDigit->cDigit != (char)0)
                    {
                        dwType = LINEDIGITMODE_DTMF;
                        cLastChar = lpDigit->cDigit;
                    }
                    else
                        dwType = LINEDIGITMODE_DTMFEND;                            
                }
                else
                    cLastChar = lpDigit->cDigit;
                pCall->OnDigit (dwType, cLastChar);
            }
        }
        break;
        
        // An offering call has callerID information.
        case EMRESULT_CALLERID:                      
        {
            LPEMCALLERID lpCallerInfo = (LPEMCALLERID) pevBuff->lpBuff;
            CTSPIAddressInfo* pAddr = GetAddress ((DWORD)lpCallerInfo->wAddressID);
            CTSPICallAppearance* pCall = pAddr->FindCallByState(LINECALLSTATE_OFFERING);
            if (pCall != NULL)
                pCall->SetCallerIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS,
                                    A2T(lpCallerInfo->szAddressInfo), A2T(lpCallerInfo->szName));                                    
        }
        break;
        
        // An offering call was redirected from another address
        case EMRESULT_REDIRECTID:
        {
            LPEMCALLERID lpCallerInfo = (LPEMCALLERID) pevBuff->lpBuff;
            CTSPIAddressInfo* pAddr = GetAddress ((DWORD)lpCallerInfo->wAddressID);
            CTSPICallAppearance* pCall = pAddr->FindCallByState(LINECALLSTATE_OFFERING);
            if (pCall != NULL)
            {
                // Set the call reason to REDIRECT
                pCall->SetCallReason(LINECALLREASON_REDIRECT);
                
                // Move the redirecting information into the CALLED information
                // and REDIRECTING information.  Move the original CALLED 
                // information into the REDIRECTED information.  Since the
                // emulator cannot redirect itself, we are guarenteed that this
                // redirection occurred from the redirecting ID.
                pCall->SetRedirectionIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS,
                                    pAddr->GetDialableAddress(), pAddr->GetName());
                pCall->SetRedirectingIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS,
                                    A2T(lpCallerInfo->szAddressInfo), A2T(lpCallerInfo->szName));
                pCall->SetCalledIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS,
                                    A2T(lpCallerInfo->szAddressInfo), A2T(lpCallerInfo->szName));
            }                                            
        }
        break;
    }
	
	// Let it flow to the phone object..
	return false;

}// CDSLine::UnsolicitedEvent

/*****************************************************************************
** Procedure:  CDSLine::UpdateCallState
**
** Arguments:  'dwAddressID' - Address the call has changed on.
**             'iNewState' - New state of the call reported by the emulator
**             'iStateInfo' - Secondary information of the call state
**             'dwMediaMode' - New media mode of the call
**
** Returns:    void
**
** Description: Update an existing call's call state for an address.  
**				Emulator has changed the state of a call.
**
*****************************************************************************/
void CDSLine::UpdateCallState (DWORD dwAddressID, int iNewState, int iStateInfo, DWORD /*dwMediaModes*/)
{
    CTSPIAddressInfo* pAddr = GetAddress(dwAddressID);
    CTSPICallAppearance* pCall = NULL;

    // Locate the call appearance this is referring to.  It will either be a 
    // direct call appearance, or a consultation call appearance.
    if (pAddr->GetAddressStatus()->dwNumInUse == 1)
        pCall = pAddr->GetCallInfo(0);
    else
    {   
        // Look for a call in a particular state - the following is the order
        // we search in.  The emulator switch always works with the active call, i.e.
        // the connected call will always become offline *before* the onHold call
        // moves back to connected.
        static DWORD dwStateOrder[] = {
			LINECALLSTATE_CONNECTED, 
			LINECALLSTATE_PROCEEDING,
			LINECALLSTATE_ONHOLD, 
			LINECALLSTATE_ONHOLDPENDTRANSFER,
			LINECALLSTATE_ONHOLDPENDCONF, 
			LINECALLSTATE_CONFERENCED,
			LINECALLSTATE_BUSY,
			LINECALLSTATE_DIALTONE,
            (DWORD)-1L
        };
        
        for (int i = 0; dwStateOrder[i] != (DWORD)-1L; i++)
        {
            pCall = pAddr->FindCallByState(dwStateOrder[i]);
            if (pCall)
                break;
        }
    }        

	// If we never found a call, create one for TAPI.  The user
	// is interacting with the phone device directly via the emulator.
	if (pCall == NULL && iNewState != ADDRESSSTATE_OFFLINE)
	{
		// Create a call appearance on the address in question.  This function will create
		// the necessary class library call structures and notify TAPI about a new call.
		CTSPIAddressInfo* pAddress = GetAddress (dwAddressID);
		_TSP_ASSERTE (pAddress != NULL);
		pCall = pAddress->CreateCallAppearance (
											NULL,						// Existing TAPI hCall to assicate with (NULL=create).
											0,							// LINECALLINFO Call Parameter flags
											LINECALLORIGIN_OUTBOUND,	// Call origin	
											LINECALLREASON_DIRECT);		// Call reason
		// Now transition this call to the proper state based on what
		// the emulator says it should be.
		pCall->SetCallID(GetTickCount());
		pCall->SetCallState(g_CallStates[iNewState], 0, LINEMEDIAMODE_INTERACTIVEVOICE);
	}

	// Otherwise, transition the call to the appropriate state.
    else if (pCall && pCall->GetCallState() != LINECALLSTATE_IDLE)
    {   
        if (iNewState == ADDRESSSTATE_OFFLINE)
            pCall->SetCallState (LINECALLSTATE_IDLE);
        else if (iNewState == ADDRESSSTATE_ONHOLD)
        {
            if (pCall->GetCallState() != LINECALLSTATE_ONHOLD &&
                pCall->GetCallState() != LINECALLSTATE_ONHOLDPENDTRANSFER &&
                pCall->GetCallState() != LINECALLSTATE_ONHOLDPENDCONF)
            {
                switch (iStateInfo)
                {
                    case HOLDTYPE_NORMAL:
                        pCall->SetCallState(LINECALLSTATE_ONHOLD);
                        break;
                    case HOLDTYPE_TRANSFER:
                        pCall->SetCallState(LINECALLSTATE_ONHOLDPENDTRANSFER);
                        break;
                    case HOLDTYPE_CONFERENCE:
                        pCall->SetCallState(LINECALLSTATE_ONHOLDPENDCONF);
                        break;                                            
                    default:
                        _TSP_ASSERT(false);
                        break;
                }
            }
        }
        else if (iNewState == ADDRESSSTATE_INCONF)
        {
            _TSP_ASSERTE (pCall->GetCallState() == LINECALLSTATE_ONHOLDPENDCONF);
            pCall->SetCallState (LINECALLSTATE_CONNECTED);
        }                                                    
        else if (iNewState == ADDRESSSTATE_CONNECT)
        {   
            // If this call isn't transitioning from a HOLDing pattern, then
            // mark the connection ID (we assume that it is transitioning from
            // DIAL or PROCEED state).
            if (pCall->GetCallState() != LINECALLSTATE_ONHOLD &&
                pCall->GetCallState() != LINECALLSTATE_ONHOLDPENDTRANSFER &&
                pCall->GetCallState() != LINECALLSTATE_ONHOLDPENDCONF)
            {
                if (pCall->GetCallInfo()->dwReason != LINECALLREASON_DIRECT)                
                {
                    pCall->SetConnectedIDInformation (LINECALLPARTYID_NAME | LINECALLPARTYID_ADDRESS,
                                pAddr->GetDialableAddress(), pAddr->GetName());
                }
            }                               
        
            // If a conference call just reverted to two-party call due to
            // either a remove conference or an unhold, then idle the conference
            // call itself and transition the final party into the two party call.
            if (pCall->GetCallState() == LINECALLSTATE_CONNECTED && 
                pCall->GetCallType() == CTSPICallAppearance::Consultant)
            {
                CTSPIConferenceCall* pConfCall = (CTSPIConferenceCall*) pCall;
                _TSP_ASSERTE (pConfCall->GetConferenceCount() == 1);
                pCall = pConfCall->GetConferenceCall(0);
                pConfCall->SetCallState(LINECALLSTATE_IDLE);
                pCall->SetCallState(LINECALLSTATE_CONNECTED);
            }    
            
            // If this was a PROCEEDING call which just connected, transition to
            // the RINGBACK state for a second to see "ring events".
            else if (pCall->GetCallState() == LINECALLSTATE_PROCEEDING)
            {
                pCall->SetCallState(LINECALLSTATE_RINGBACK);
                pCall->SetCallState(LINECALLSTATE_CONNECTED);
            }
            
            // Otherwise simply transition the call
            else if (pCall->GetCallState() != LINECALLSTATE_CONNECTED)
                pCall->SetCallState(LINECALLSTATE_CONNECTED);
        }
        
        else if (iNewState == ADDRESSSTATE_DIALTONE ||
                 iNewState == ADDRESSSTATE_ONLINE ||
                 iNewState == ADDRESSSTATE_BUSY)
        {
            if (pCall->GetCallState() != g_CallStates[iNewState])
                pCall->SetCallState(g_CallStates[iNewState]);
        }
    }        
    
}// CDSLine::UpdateCallState

/*****************************************************************************
** Procedure:  CDSLine::HandleNewCall
**
** Arguments:  'dwAddressID' - Address the call has changed on.
**             'dwMediaModes' - New media mode(s) of the call
**             'dwCompletionID' - Completion ID if this was parked call.
**
** Returns:    void
**
** Description: Process a newly detected call on the line.
**
*****************************************************************************/
void CDSLine::HandleNewCall (DWORD dwAddressID, DWORD dwMediaModes, DWORD dwCompletionID, bool fExternal)
{   
    // Default the media mode.
    if (dwMediaModes == 0)
        dwMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE;

    // Create a call appearance on the address in question.  This function will create
	// the necessary class library call structures and notify TAPI about a new call.
    CTSPIAddressInfo* pAddress = GetAddress(dwAddressID);
    _TSP_ASSERTE (pAddress != NULL);

	DWORD dwOrigin = (fExternal) ? 
		(LINECALLORIGIN_INBOUND | LINECALLORIGIN_EXTERNAL) : 
		(LINECALLORIGIN_INBOUND | LINECALLORIGIN_INTERNAL);
	DWORD dwCallReason = (dwCompletionID != 0) ? 
		LINECALLREASON_CALLCOMPLETION : 
		LINECALLREASON_DIRECT;

    CTSPICallAppearance* pCall = pAddress->CreateCallAppearance (
										NULL,			// Existing TAPI hCall to assicate with (NULL=create).
										0,				// LINECALLINFO Call Parameter flags
										dwOrigin,		// Call origin
										dwCallReason,	// Call reason
										(fExternal) ? (dwAddressID+1) : 0xffffffff,
										dwCompletionID);

	// Set a call-id into the call - we don't use the callid in this provider
	// so it is just a dummy value.
	pCall->SetCallID(GetTickCount());

    // Reset the ring counter for this call.
    OnRingDetected(1, true);

    // Report an offering call to TAPI.
    pCall->SetCallState(LINECALLSTATE_OFFERING, 0, dwMediaModes);

}// CDSLine::HandleNewCall

/*****************************************************************************
** Procedure:  CDSLine::OnCallFeaturesChanged
**
** Arguments:  'pCall' - Call that changed
**             'dwFeatures' - new feature list
**
** Returns:    Modified feature list
**
** Description: This method is called whenever the call features have changed 
**              due to state changes.
**
*****************************************************************************/
DWORD CDSLine::OnCallFeaturesChanged (CTSPICallAppearance* pCall, DWORD dwFeatures)
{
	// We don't allow conferences to be held
	if (pCall->GetCallType() == CTSPICallAppearance::Conference)
		dwFeatures &= ~LINECALLFEATURE_HOLD;
	// We don't allow individual calls in a conference to be dropped.
	else if (pCall->GetConferenceOwner() != NULL)
		dwFeatures &= ~LINECALLFEATURE_DROP;
	return dwFeatures;

}// CDSLine::OnCallFeaturesChanged

/*****************************************************************************
** Procedure:  CDSLine::OnAddressFeaturesChanged
**
** Arguments:  'pAddr' - Address that changed
**             'dwFeatures' - new feature list
**
** Returns:    Modified feature list
**
** Description: This method is called whenever the address features have changed due to
**              state changes.
**
*****************************************************************************/
DWORD CDSLine::OnAddressFeaturesChanged (CTSPIAddressInfo* pAddr, DWORD dwFeatures)
{
	// Don't allow pickup/makecall/unpark if any activity on the address - even
	// if the call is onHOLD.  We only allow consultation calls to be created via
	// transfer/conference events on an active address.  This is an emulator design
	// restriction.  The TSP++ library automatically adds these features when the active
	// call goes onHOLD.
	if (pAddr->GetAddressStatus()->dwNumOnHoldCalls + 
		pAddr->GetAddressStatus()->dwNumOnHoldPendCalls + 
		pAddr->GetAddressStatus()->dwNumActiveCalls +
		GetLineDevStatus()->dwNumActiveCalls)
		dwFeatures &= ~(LINEADDRFEATURE_MAKECALL | LINEADDRFEATURE_PICKUP | 
						LINEADDRFEATURE_UNPARK | LINEADDRFEATURE_FORWARD);
	return dwFeatures;

}// CDSLine::OnAddressFeaturesChanged

/*****************************************************************************
** Procedure:  CDSLine::OnLineFeaturesChanged
**
** Arguments:  'dwFeatures' - new feature list
**
** Returns:    Modified feature list
**
** Description: This method is called whenever the line features have changed due to
**              state changes.
**
*****************************************************************************/
DWORD CDSLine::OnLineFeaturesChanged(DWORD dwFeatures)
{
	// Don't allow pickup/makecall/unpark if there are any active calls.
	if (GetLineDevStatus()->dwNumActiveCalls > 0)
		dwFeatures &= ~(LINEFEATURE_MAKECALL | LINEFEATURE_FORWARD);

	// Force the address to recalc its features in case we changed them.
	for (int i = 0; i < (int) GetAddressCount(); i++)
		GetAddress(i)->RecalcAddrFeatures();

	return dwFeatures;

}// CDSLine::OnLineFeaturesChanged

/*****************************************************************************
** Procedure:  CDSLine::HandleDialEvent
**
** Arguments:  'pReq' - Request this dial event was generated from.
**             'pevBuff' - Data structure (response based)
**
** Returns:    true/false success indicator
**
** Description: Master function to process a dial request.  This is called from
**				both the lineMakeCall events and the lineDial events.
**
*****************************************************************************/
bool CDSLine::HandleDialEvent(RTDialInfo* pReq, EVENTBUFF* pevBuff)
{                                   
    CTSPIAddressInfo* pAddr = pReq->GetAddressInfo();
    CTSPICallAppearance* pCall = pReq->GetCallInfo();
    DIALINFO* pDialInfo = pReq->GetDialableNumber(0);
    bool fProcessed = false, fPartialAddr = pDialInfo->fIsPartialAddress;
	bool fAlreadyDialing = (pCall->GetCallState() == LINECALLSTATE_DIALING || pCall->GetCallState() == LINECALLSTATE_PROCEEDING);
    
    switch (pReq->GetState())
    {   
        // State 1:
        // Send a dial string to the emulator device.  We only send the 
        // first address given in the object array.  Country code is
        // ignored.
        case STATE_INITIAL:           
        case STATE_DIALING:
            pReq->SetState (STATE_WAITFORONLINE);
            if (GetDeviceInfo()->DRV_Dial(pAddr->GetAddressID(), pDialInfo, pReq->GetCountryCode()))
			{
				// If the call was already in the DIALING state, then we won't see a 
				// address online change.
				if (fAlreadyDialing)
				{
					pCall->SetCallState((fPartialAddr) ? LINECALLSTATE_DIALING : LINECALLSTATE_PROCEEDING);
                    CompleteRequest (pReq, 0);
				}
			}
            fProcessed = true;
            break;

        // State 2:
        // Look for the address moving to a "ONLINE" state.  This indicates
        // that we are not connected to a call, and not in dialtone, but
        // transitioning somehow between them.
        case STATE_WAITFORONLINE:
            if (pevBuff->dwResponse == EMRESULT_ADDRESSCHANGED)
            {
                const LPEMADDRESSCHANGE lpAddrChange = (const LPEMADDRESSCHANGE) pevBuff->lpBuff;
                if (lpAddrChange->wAddressID == (WORD) pAddr->GetAddressID())
                {   
                    fProcessed = true;                        
                    if (lpAddrChange->wNewState == ADDRESSSTATE_ONLINE)
                    {
						// If the end of the string is a ';', then more digits
						// are to follow, leave the call in the DIALING state.
						if (fPartialAddr)
							pCall->SetCallState(LINECALLSTATE_DIALING);

                        // Move the the "proceeding" state on this call.
						else
							pCall->SetCallState(LINECALLSTATE_PROCEEDING);

                        // Go ahead and complete the request.  If the call changes
                        // to something other than "ONLINE", then the Update function
                        // will change the callstate.
                        CompleteRequest (pReq, 0);
                    }
                    else if (lpAddrChange->wNewState == ADDRESSSTATE_DIALTONE)
					{
						// Move to the DIALTONE state.
						pCall->SetCallState(LINECALLSTATE_DIALTONE, (lpAddrChange->wStateInfo == DIALTONETYPE_INTERNAL) ?
									LINEDIALTONEMODE_INTERNAL : LINEDIALTONEMODE_EXTERNAL);
						if (lpAddrChange->wStateInfo == DIALTONETYPE_EXTERNAL)
						{
							// Assign the TRUNK id based on the address + 1.
							pCall->SetTrunkID(lpAddrChange->wAddressID + 1);
						}
					}
                }
            }
            break;                
        
        default:
            _TSP_ASSERT(false);
            break;
    }

    // If we failed, then idle the call appearance.
    if (pevBuff && pevBuff->dwResponse == EMRESULT_ERROR)
    {                               
        if (fProcessed || pevBuff->dwAddress == pAddr->GetAddressID())
        {
            CompleteRequest(pReq, LINEERR_OPERATIONFAILED);
            fProcessed = true;
        }           
    }
    return fProcessed;

}// CDSProvider::HandleDialEvent
