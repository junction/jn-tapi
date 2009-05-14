/***************************************************************************
//
// LINE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Line management functions
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
BEGIN_TSPI_REQUEST(CJTLine)
	ON_AUTO_TSPI_REQUEST(REQUEST_ACCEPT)
	ON_AUTO_TSPI_REQUEST(REQUEST_SECURECALL)
	ON_AUTO_TSPI_REQUEST(REQUEST_RELEASEUSERINFO)
	ON_AUTO_TSPI_REQUEST(REQUEST_SETCALLDATA)
	ON_AUTO_TSPI_REQUEST(REQUEST_SETQOS)
	ON_TSPI_REQUEST(REQUEST_REDIRECT, OnRedirectOrBlindTransfer)
	ON_TSPI_REQUEST(REQUEST_BLINDXFER, OnRedirectOrBlindTransfer)
	ON_TSPI_REQUEST_ADDCONF(OnAddToConference)
	ON_TSPI_REQUEST_ANSWER(OnAnswer)
	ON_TSPI_REQUEST_MAKECALL(OnMakeCall)
	ON_TSPI_REQUEST_DIAL(OnDial)
	ON_TSPI_REQUEST_DROPCALL(OnDropCall)
	ON_TSPI_REQUEST_HOLD(OnHoldCall)
	ON_TSPI_REQUEST_UNHOLD(OnRetrieveCall)
	ON_TSPI_REQUEST_SWAPHOLD(OnSwapHold)
	ON_TSPI_REQUEST_SETUPXFER(OnSetupTransfer)
	ON_TSPI_REQUEST_COMPLETEXFER(OnCompleteTransfer)
	ON_TSPI_REQUEST_SETUPCONF(OnSetupConference)
	ON_TSPI_REQUEST_PREPAREADDCONF(OnPrepareAddToConference)
	ON_TSPI_REQUEST_REMOVEFROMCONF(OnRemoveFromConference)
	ON_TSPI_REQUEST_SENDUSERINFO(OnSendUUI)
	ON_TSPI_REQUEST_SETAGENTGROUP(OnSetAgentGroup)
	ON_TSPI_REQUEST_SETAGENTSTATE(OnSetAgentState)
	ON_TSPI_REQUEST_SETAGENTACTIVITY(OnSetAgentActivity)
	ON_TSPI_REQUEST_GENERATEDIGITS(OnGenerateDigits)
END_TSPI_REQUEST()

/*****************************************************************************
** Procedure:  CJTLine::read
**
** Arguments:  'istm' - Input stream
**
** Returns:    pointer to istm
**
** Description:  This function is called to serialize data in from the
**               registry.  The line object has already been completely
**               initialized by the TSP++ library
**
*****************************************************************************/
TStream& CJTLine::read(TStream& istm)
{
    // Adjust the line device capabilities.  We don't support any of the
    // line device capability flags, and don't need dialing parameters since the
    // switch doesn't allow them to be adjusted.
    LPLINEDEVCAPS lpCaps = GetLineDevCaps();
    lpCaps->dwDevCapFlags = 0;
	lpCaps->dwUUICallInfoSize = 1024;
	lpCaps->dwLineStates &= ~(LINEDEVSTATE_RINGING | LINEDEVSTATE_MSGWAITON | LINEDEVSTATE_MSGWAITOFF | 
				LINEDEVSTATE_NUMCOMPLETIONS | LINEDEVSTATE_TERMINALS | LINEDEVSTATE_ROAMMODE | 
				LINEDEVSTATE_BATTERY | LINEDEVSTATE_SIGNAL | LINEDEVSTATE_LOCK | LINEDEVSTATE_COMPLCANCEL |
				LINEDEVSTATE_MAINTENANCE);

	// Adjust our address information
	CTSPIAddressInfo* pAddress = GetAddress(0);
	_TSP_ASSERTE (pAddress != NULL);

	LINEADDRESSCAPS* lpACaps = pAddress->GetAddressCaps();
	lpACaps->dwMaxCallDataSize = MAXCALLDATA_SIZE;
	lpACaps->dwCallerIDFlags = 
	lpACaps->dwConnectedIDFlags = 
	lpACaps->dwRedirectionIDFlags =
	lpACaps->dwRedirectingIDFlags =
	lpACaps->dwCalledIDFlags =
	lpACaps->dwCalledIDFlags & ~LINECALLPARTYID_PARTIAL;
	lpACaps->dwCallStates &= ~(LINECALLSTATE_SPECIALINFO | LINECALLSTATE_RINGBACK);
	lpACaps->dwDialToneModes &= ~LINEDIALTONEMODE_SPECIAL;
	lpACaps->dwDisconnectModes &= ~LINEDISCONNECTMODE_REJECT;

	// Adjust the various line properties based on the type of line this is.
	if (GetLineType() == Station)
		InitializeStation();
	else if (GetLineType() == RoutePoint)
		InitializeRoutePoint();
	else if (GetLineType() == Queue)
		InitializeQueue();
	else if (GetLineType() == PredictiveDialer)
		InitializeDialer();
	else if (GetLineType() == VRU)
		InitializeVRU();

	// We didn't read any extra information from the stream.
	return CTSPILineConnection::read(istm);

}// CJTLine::read

/*****************************************************************************
** Procedure:  CJTLine::InitializeStation
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This function is called to set the properties for a station
**
*****************************************************************************/
void CJTLine::InitializeStation()
{
	// The switch doesn't allow us to answer calls when we have active
	// calls connected to the station.
	GetLineDevCaps()->dwAnswerMode = LINEANSWERMODE_NONE;
	
	// Only allow for DTMF digit generation
	GetLineDevCaps()->dwGenerateDigitModes = LINEDIGITMODE_DTMF;
	GetLineDevCaps()->dwMonitorDigitModes = LINEDIGITMODE_DTMF | LINEDIGITMODE_DTMFEND;

	// Allow up to 1K bytes of UUI to be sent via lineSendUserUserInfo. We
	// don't allow it on DROP/ACCEPT/ANSWER/MAKECALL.
	GetLineDevCaps()->dwUUISendUserUserInfoSize = 1024;

	// Adjust the ADDRESSCAPS based on this being a station
	CTSPIAddressInfo* pAddress = GetAddress(0);
	_TSP_ASSERTE (pAddress != NULL);
	LINEADDRESSCAPS* lpACaps = pAddress->GetAddressCaps();
	lpACaps->dwAddrCapFlags = 
		LINEADDRCAPFLAGS_DIALED |			// Outgoing addresses may be dialed from here
		LINEADDRCAPFLAGS_ORIGOFFHOOK |		// Our phone goes off-hook automatically
		LINEADDRCAPFLAGS_TRANSFERMAKE |		// We can consult transfer a lineMakeCall call
		LINEADDRCAPFLAGS_TRANSFERHELD |		// We can transfer a held call
		LINEADDRCAPFLAGS_CONFERENCEMAKE;	// We can add lineMakeCall calls to a conference

	// Set our transfer features
	lpACaps->dwTransferModes = (LINETRANSFERMODE_TRANSFER | LINETRANSFERMODE_CONFERENCE);
	lpACaps->dwCallFeatures2 = (LINECALLFEATURE2_TRANSFERNORM | LINECALLFEATURE2_TRANSFERCONF);

	// Now adjust the agent capabilities
	TAgentCaps* pAgentCaps = pAddress->GetAgentCaps();
	pAgentCaps->dwFeatures = (LINEAGENTFEATURE_SETAGENTGROUP | 
		LINEAGENTFEATURE_SETAGENTSTATE | 
		LINEAGENTFEATURE_SETAGENTACTIVITY |
		LINEAGENTFEATURE_GETAGENTACTIVITYLIST |
		LINEAGENTFEATURE_GETAGENTGROUP);
	pAgentCaps->dwStates = (LINEAGENTSTATE_LOGGEDOFF | LINEAGENTSTATE_NOTREADY |
				LINEAGENTSTATE_READY | LINEAGENTSTATE_BUSYACD |
				LINEAGENTSTATE_BUSYINCOMING | LINEAGENTSTATE_BUSYOUTBOUND |
				LINEAGENTSTATE_BUSYOTHER | LINEAGENTSTATE_WORKINGAFTERCALL |
				LINEAGENTSTATE_UNKNOWN);
	pAgentCaps->dwNextStates = (LINEAGENTSTATE_NOTREADY | LINEAGENTSTATE_READY | 
				LINEAGENTSTATE_WORKINGAFTERCALL | LINEAGENTSTATE_UNKNOWN);
	pAgentCaps->dwAgentStatusMessages = (LINEAGENTSTATUS_STATE | LINEAGENTSTATUS_NEXTSTATE |
				LINEAGENTSTATUS_VALIDSTATES | LINEAGENTSTATUS_VALIDNEXTSTATES);
	pAgentCaps->dwMaxNumGroupEntries = 4;

	// Adjust the AGENT status for this address.  Further adjustments
	// will be made when we see whether an agent is logged on or not.
	TAgentStatus* pAgentStatus = pAddress->GetAgentStatus();
	pAgentStatus->dwState = LINEAGENTSTATE_UNKNOWN;
	pAgentStatus->dwNextState = LINEAGENTSTATE_UNKNOWN;

	// Initialize our current agent state to LOGGED OUT.
	pAddress->GetAgentStatus()->dwState = LINEAGENTSTATE_LOGGEDOFF;

	// Mark the line as "out of service" until we see an agent logon.
	// This will stop all activity from TAPI against the line.
	SetDeviceStatusFlags(GetLineDevStatus()->dwDevStatusFlags & ~LINEDEVSTATUSFLAGS_INSERVICE);

}// CJTLine::InitializeStation

/*****************************************************************************
** Procedure:  CJTLine::InitializeVRU
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This function is called to set the properties for a vru
**
*****************************************************************************/
void CJTLine::InitializeVRU()
{
	// We don't support answering calls on the VRU - it is done
	// automatically by the PBX.
	GetLineDevCaps()->dwAnswerMode = LINEANSWERMODE_NONE;

	// Do not allow digit generation
	GetLineDevCaps()->dwGenerateDigitModes = 0;
	GetLineDevCaps()->dwMonitorDigitModes = LINEDIGITMODE_DTMF | LINEDIGITMODE_DTMFEND;

	// Adjust the ADDRESSCAPS for this address.  We need to 
	// zero the address features (not supported on this line) and the
	// conference features.  They were set because the functions are exported
	// from the TSP but they are not used for VRU stations
    CTSPIAddressInfo* pAddr = GetAddress(0);
	LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
	lpAddrCaps->dwAddressFeatures = 0;
	lpAddrCaps->dwRemoveFromConfCaps = 
	lpAddrCaps->dwRemoveFromConfState = 
	lpAddrCaps->dwTransferModes = 0;
	lpAddrCaps->dwCallFeatures = (
		LINECALLFEATURE_DROP |					// We can drop calls
		LINECALLFEATURE_REDIRECT |				// We can redirect calls ringing here 
		LINECALLFEATURE_RELEASEUSERUSERINFO |	// We can release useruser information
		LINECALLFEATURE_SETCALLDATA);			// We can set call data on calls

	// Turn off all the line and address features since we cannot do anything
	// listed in the LINEFEATURE_xxx constants.
	pAddr->SetAddressFeatures(0);
	SetLineFeatures(0);

}// CJTLine::InitializeVRU

/*****************************************************************************
** Procedure:  CJTLine::InitalizeRoutePoint
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Method which does all the route-point specific initialization
**
*****************************************************************************/
void CJTLine::InitializeRoutePoint()
{
	// We don't support answering calls on this line
	GetLineDevCaps()->dwAnswerMode = LINEANSWERMODE_NONE;

	// Do not allow digit generation
	GetLineDevCaps()->dwGenerateDigitModes = 0;
	GetLineDevCaps()->dwMonitorDigitModes = 0;

	// Adjust the ADDRESSCAPS for this address.  We need to 
	// zero the address features (not supported on this line) and the
	// conference features.  They were set because the functions are exported
	// from the TSP but they are not used for route points.
    CTSPIAddressInfo* pAddr = GetAddress(0);
	LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
	lpAddrCaps->dwAddressFeatures = 0;
	lpAddrCaps->dwRemoveFromConfCaps = 
	lpAddrCaps->dwRemoveFromConfState = 
	lpAddrCaps->dwTransferModes = 0;
	lpAddrCaps->dwCallFeatures = (
		LINECALLFEATURE_DROP |					// We can drop calls
		LINECALLFEATURE_REDIRECT |				// We can redirect calls ringing here 
		LINECALLFEATURE_RELEASEUSERUSERINFO |	// We can release useruser information
		LINECALLFEATURE_SETCALLDATA);			// We can set call data on calls

	// Turn off all the line and address features since we cannot do anything
	// listed in the LINEFEATURE_xxx constants.
	pAddr->SetAddressFeatures(0);
	SetLineFeatures(0);

}// CJTLine::InitializeRoutePoint

/*****************************************************************************
** Procedure:  CJTLine::InitializeQueue
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Method which does all the ACD queue specific initialization
**
*****************************************************************************/
void CJTLine::InitializeQueue()
{
	// We don't support answering calls on this line
	GetLineDevCaps()->dwAnswerMode = LINEANSWERMODE_NONE;

	// Do not allow digit generation
	GetLineDevCaps()->dwGenerateDigitModes = 0;
	GetLineDevCaps()->dwMonitorDigitModes = 0;

	// Adjust the ADDRESSCAPS for this address.  We need to 
	// zero the address and call features (not supported on this line) and the
	// conference features.  They were set because the functions are exported
	// from the TSP but they are not used for queues.
    CTSPIAddressInfo* pAddr = GetAddress(0);
	LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
	lpAddrCaps->dwCallFeatures = 0;
	lpAddrCaps->dwAddressFeatures = 0;
	
	// Turn off all the line and address features since we cannot do anything
	// listed in the LINEFEATURE_xxx constants.
	SetLineFeatures(0);
	pAddr->SetAddressFeatures(0);

}// CJTLine::InitializeQueue

/*****************************************************************************
** Procedure:  CJTLine::InitializeDialer
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Method which does all the predictive dialer specific initialization
**
*****************************************************************************/
void CJTLine::InitializeDialer()
{
	// Set the features for this line.
    LPLINEDEVCAPS lpCaps = GetLineDevCaps();
	lpCaps->dwAnswerMode = LINEANSWERMODE_NONE;
	lpCaps->dwLineFeatures = LINEFEATURE_MAKECALL;

	// Do not allow digit generation
	GetLineDevCaps()->dwGenerateDigitModes = 0;
	GetLineDevCaps()->dwMonitorDigitModes = 0;

	// Adjust the ADDRESSCAPS for the dialer.
    CTSPIAddressInfo* pAddr = GetAddress(0);
	LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
	lpAddrCaps->dwCallFeatures = LINECALLFEATURE_DROP;
	lpAddrCaps->dwPredictiveAutoTransferStates = LINECALLSTATE_CONNECTED;
	lpAddrCaps->dwMaxNoAnswerTimeout = 60;

	// Recalculate our line features - we only support lineMakeCall on the
	// predictive dialer line
	RecalcLineFeatures();
	pAddr->SetAddressFeatures(LINEADDRFEATURE_MAKECALL);

}// CJTLine::InitializeDialer

/*****************************************************************************
** Procedure:  CJTLine::OnAddressFeaturesChanged
**
** Arguments:  'pAddr' - Address features are changing on
**             'dwFeatures' - New features for address
**
** Returns:    New features for address
**
** Description: This method overrides the TSP++ library default behavior 
**              concerning active calls and placing new calls.  By default,
**              TSP++ will remove the MAKECALL bit if there is an active call
**              on the address.  In this PBX, you can place another call
**              even when you are on an existing call.  The existing call
**              is transitioned to onHOLD by the switch.
**
*****************************************************************************/
DWORD CJTLine::OnAddressFeaturesChanged (CTSPIAddressInfo* pAddr, DWORD dwFeatures)
{
	// If this is not a station then let the default behavior take place
	// in the TSP library.
	if (GetLineType() == Station)
	{
		LPLINEDEVSTATUS pStat = GetLineDevStatus();
		LPLINEADDRESSCAPS pACaps = GetAddress(0)->GetAddressCaps();

		// Assume we CANNOT place a new call
		dwFeatures &= ~LINEADDRFEATURE_MAKECALL;

		// The following must be true in order to place a new outgoing call
		// at a station on the PBX:
		//
		// 1. Line must be INSERVICE and CONNECTED (i.e. agent logged on)
		// 2. Must have bandwidth for new outgoing call
		// 3. Agent must NOT be in the READY state.
		//
		if ((pStat->dwDevStatusFlags & 
				(LINEDEVSTATUSFLAGS_CONNECTED | LINEDEVSTATUSFLAGS_INSERVICE)) ==
				(LINEDEVSTATUSFLAGS_CONNECTED | LINEDEVSTATUSFLAGS_INSERVICE) &&
				(pStat->dwNumActiveCalls + pStat->dwNumOnHoldCalls + 
				pStat->dwNumOnHoldPendCalls) < pACaps->dwMaxNumOnHoldCalls &&
				GetAddress(0)->GetAgentStatus()->dwState != LINEAGENTSTATE_READY)
			dwFeatures |= LINEADDRFEATURE_MAKECALL;
	}
	return CTSPILineConnection::OnAddressFeaturesChanged(pAddr, dwFeatures);

}// CJTLine::OnAddressFeaturesChanged

/*****************************************************************************
** Procedure:  CJTLine::OnLineFeaturesChanged
**
** Arguments:  'dwFeatures' - New features for line
**
** Returns:    New features for address
**
** Description: This method overrides the TSP++ library default behavior 
**              concerning calls placed on an address with active calls
**              already present.  The default behavior is to not allow
**              this situation but in our switch we will automatically 
**              transition to HOLD so we need to allow it.
**
*****************************************************************************/
DWORD CJTLine::OnLineFeaturesChanged (DWORD dwFeatures)
{
	// If this is not a station then let the default behavior take place
	// in the TSP library.
	if (GetLineType() == Station)
	{
		LPLINEDEVSTATUS pStat = GetLineDevStatus();
		LPLINEADDRESSCAPS pACaps = GetAddress(0)->GetAddressCaps();

		// Assume we CANNOT place a new call
		dwFeatures &= ~LINEFEATURE_MAKECALL;

		// The following must be true in order to place a new outgoing call
		// at a station on the simulated PBX:
		//
		// 1. Line must be INSERVICE and CONNECTED (i.e. agent logged on)
		// 2. Must have bandwidth for new outgoing call
		// 3. Agent must NOT be in the READY state.
		//
		if (((pStat->dwDevStatusFlags & 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) == 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) &&
			 ((pStat->dwNumActiveCalls + pStat->dwNumOnHoldCalls + 
			   pStat->dwNumOnHoldPendCalls) < pACaps->dwMaxNumOnHoldCalls) &&
			(GetAddress(0)->GetAgentStatus()->dwState != LINEAGENTSTATE_READY))
			dwFeatures |= LINEFEATURE_MAKECALL;
	}
	return CTSPILineConnection::OnLineFeaturesChanged(dwFeatures);

}// CJTLine::OnLineFeaturesChanged

/*****************************************************************************
** Procedure:  CJTLine::OnCallFeaturesChanged
**
** Arguments:  'pCall' - Call that changed
**             'dwCallFeatures' - new feature list
**
** Returns:    true/false success indicator
**
** Description: This method is called whenever the call features have changed due to
**              state changes.  The address/line have also been adjusted by the time
**              this function is called.
**
*****************************************************************************/
DWORD CJTLine::OnCallFeaturesChanged(CTSPICallAppearance* pCall, DWORD dwCallFeatures)
{                                 
	// If we have some call features..
	if (dwCallFeatures > 0)
	{
		// If the call is a "fake" call which hasn't really been dialed on the
		// switch, then allow the lineDial feature to be reflected.  Otherwise
		// remove it.
		if (pCall->IsRealCall())
			dwCallFeatures &= ~LINECALLFEATURE_DIAL;

		// If the call is NOT on a trunk, then disallow any blind transfer events.
		// The PBX simulator will not allow an unsupervised transfer on a non-outside call.
		if (pCall->GetCallInfo()->dwTrunk == 0xffffffff)
			dwCallFeatures &= ~LINECALLFEATURE_BLINDTRANSFER;

		// If we have connected calls then don't allow the ANSWER bit to be
		// presented to applications. We could allow this if the PBX allowed us to 
		// send an answer command at any time.
		// We could also have moved this test to the Answer logic and moved any
		// calls to onHOLD automatically.
		if (FindCallByState(LINECALLSTATE_CONNECTED | 
				LINECALLSTATE_DIALTONE |
				LINECALLSTATE_DIALING |
				LINECALLSTATE_RINGBACK |
				LINECALLSTATE_BUSY |
				LINECALLSTATE_PROCEEDING |
				LINECALLSTATE_DISCONNECTED) != NULL)
			dwCallFeatures &= ~LINECALLFEATURE_ANSWER;

		// If the call has the COMPLETETRANSFER bit set then we have more than one
		// call and one is connected. Check to make sure that the destination (the
		// connected call) is an internal call to another station. The switch doesn't
		// allow merges of two trunk calls.
		if ((dwCallFeatures & LINECALLFEATURE_COMPLETETRANSF) != 0)
		{
			CTSPICallAppearance* pCall2 = FindCallByState(LINECALLSTATE_CONNECTED);
			if (pCall2 == NULL || pCall2->GetCallInfo()->dwTrunk != 0xffffffff)
				dwCallFeatures &= ~LINECALLFEATURE_COMPLETETRANSF;
		}

		// If the call is a CONFERENCE owner then see if it has been fully created.
		if (pCall->GetCallType() == CTSPICallAppearance::Conference)
		{
			// Do not allow drops of conferences - you must drop the parties.
			dwCallFeatures &= ~LINECALLFEATURE_DROP;

			// If it hasn't actually been created yet, then allow
			// the conference to be swapped in-place.
			if (pCall->GetCallState() == LINECALLSTATE_ONHOLDPENDCONF)
				dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
		}

		// Normal call
		else 
		{
			// Allow for both normal transfer and transfer-to-conference.
			if (dwCallFeatures & LINECALLFEATURE_COMPLETETRANSF)
				pCall->SetCallFeatures2(LINECALLFEATURE2_TRANSFERNORM | LINECALLFEATURE2_TRANSFERCONF);

			// If the call is now part of a conference, allow it to be HELD.
			if (pCall->GetCallState() == LINECALLSTATE_CONFERENCED)
				dwCallFeatures |= LINECALLFEATURE_HOLD;
		}
	}
	return CTSPILineConnection::OnCallFeaturesChanged(pCall, dwCallFeatures);

}// CJTLine::OnCallFeaturesChanged

/*****************************************************************************
** Procedure:  CJTLine::OpenDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description: This method is called when the line is opened by TAPI.
**              We want to check and see if the line is currently CONNECTED
**              to the PBX server.
**
*****************************************************************************/
bool CJTLine::OpenDevice()
{
	// If the line is not CONNECTED, fail the open.  This indicates that we are
	// not currently connected to transaction link so we shouldn't allow the line to open.
	if ((GetLineDevStatus()->dwDevStatusFlags & LINEDEVSTATUSFLAGS_CONNECTED) == 0)
		return false;
	return CTSPILineConnection::OpenDevice();

}// CJTLine::OpenDevice

/*****************************************************************************
** Procedure:  CJTLine::OnTimer
**
** Arguments:  void
**
** Returns:    void
**
** Description: This method is called periodically by the interval timer
**
*****************************************************************************/
void CJTLine::OnTimer()
{
	// Poll the active request for timeout
	ReceiveData();

}// CJTLine::OnTimer

/*****************************************************************************
** Procedure:  CJTLine::TranslateErrorCode
**
** Arguments:  'pRequest' - Request object
**             'iError'	  - Error code received from PBX (may be zero)
**
** Returns:    void
**
** Description: This method completes the request with an appropriate 
**              TAPI error code.
**
*****************************************************************************/
void CJTLine::TranslateErrorCode(CTSPIRequest* pRequest, DWORD dwError)
{
	static DWORD arrErrors[] = {
		0, LINEERR_BADDEVICEID, LINEERR_INVALDIGITS, LINEERR_INVALPARAM,
		LINEERR_INVALCALLHANDLE, LINEERR_RESOURCEUNAVAIL, LINEERR_OPERATIONFAILED,
		LINEERR_OPERATIONUNAVAIL, LINEERR_OPERATIONUNAVAIL, LINEERR_INVALPASSWORD,   
		LINEERR_INVALAGENTID, LINEERR_INVALAGENTGROUP,  LINEERR_INVALAGENTSTATE, 
		LINEERR_OPERATIONFAILED
	};

	// Do a simple translation. In a real production TSP this would take into account
	// the request and do a better mapping to TAPI error codes. 
	_TSP_ASSERTE(dwError >= CPEErrorCode::None && dwError <= CPEErrorCode::Failed);
	CompleteRequest(pRequest, arrErrors[dwError]);

}// CJTLine::TranslateErrorCode
