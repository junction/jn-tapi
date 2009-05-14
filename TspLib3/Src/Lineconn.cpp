/******************************************************************************/
//                                                                        
// LINECONN.CPP - Source code for the CTSPILineConnection object          
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the source to manage the line objects which are 
// held by the CTSPIDevice.                                               
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
#include <process.h>
#include <spbstrm.h>

/*----------------------------------------------------------------------------
    Request map
-----------------------------------------------------------------------------*/
TRequestMap CTSPILineConnection::g_mapRequests;

///////////////////////////////////////////////////////////////////////////
// OfferCallThread
//
// Re-offers calls back to TAPI once the line is open. Workaround for
// lineOpen/SetDefaultMediaMode "issue" with TAPISRV in TAPI 2.1, 
// this works for all known versions of TAPISRV.
//
unsigned __stdcall tsplib_OfferCallThread(void* pParam)
{
	_TSP_DTRACEX(TRC_THREADS, _T("OfferCallThread(0x%lx) starting\n"), GetCurrentThreadId());

	// Re-attempt up to 10 times to tell TAPI about our calls - we want the
	// notification to happen ASAP so that calls using lineGetNewCalls see them
	// there rather than appearing out of thin air.
    for (int i = 0; i < 10; i++)
	{
		// If it is ok then exit.
		if (reinterpret_cast<CTSPILineConnection*>(pParam)->OfferCallsToTAPISrv())
			break;
		// Otherwise wait and try again.
		Sleep(250);
	}

	_TSP_DTRACEX(TRC_THREADS, _T("OfferCallThread(0x%lx) ending\n"), GetCurrentThreadId());
	_endthreadex(0);
	return 0;

}// OfferCallThread

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CTSPILineConnection
//
// Constructor for the line object
//
CTSPILineConnection::CTSPILineConnection() : CTSPIConnection(),
	m_dwLineMediaModes(0), m_dwLineStates(0), m_lpfnEventProc(NULL), 
	m_htLine(0), m_dwConnectedCallCount(0), m_iLineType(Station),
	m_guidMSP(IID_NULL)
{ 
    ZeroMemory (&m_LineCaps, sizeof(LINEDEVCAPS));
    ZeroMemory (&m_LineStatus, sizeof(LINEDEVSTATUS));

	m_dwCallHubTracking = LINECALLHUBTRACKING_ALLCALLS;

}// CTSPILineConnection::CTSPILineConnection

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::~CTSPILineConnection
//
// Destructor for the line connection object.  Remove all the address
// information structures allocated for this line.
//
CTSPILineConnection::~CTSPILineConnection()
{
	// Remove the id from the map
	if (m_dwDeviceID != 0xffffffff)
		GetSP()->RemoveConnectionFromMap(m_dwDeviceID, this);

	// Tell windows to close all UI dialogs.
	for (TUIList::iterator i = m_lstUIDialogs.begin();
	     i != m_lstUIDialogs.end(); ++i)
		SendDialogInstanceData((*i)->htDlgInstance);

}// CTSPILineConnection::~CTSPILineConnection

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::BaseInit
//
// Basic initialization required for both v2.x and v3.x line devices
//
void CTSPILineConnection::BaseInit(CTSPIDevice* /*pDevice*/, DWORD /*dwLineID*/)
{
#ifdef _UNICODE
    m_LineCaps.dwStringFormat = STRINGFORMAT_UNICODE;
#else
    m_LineCaps.dwStringFormat = STRINGFORMAT_ASCII;
#endif

    m_LineCaps.dwAddressModes = LINEADDRESSMODE_ADDRESSID;
    m_LineCaps.dwBearerModes = LINEBEARERMODE_VOICE;
    m_LineCaps.dwLineStates = 
			(LINEDEVSTATE_OTHER | 
			 LINEDEVSTATE_RINGING | 
			 LINEDEVSTATE_CONNECTED |
             LINEDEVSTATE_DISCONNECTED | 
			 LINEDEVSTATE_MSGWAITON | 
			 LINEDEVSTATE_MSGWAITOFF |
             LINEDEVSTATE_INSERVICE | 
			 LINEDEVSTATE_OUTOFSERVICE | 
			 LINEDEVSTATE_MAINTENANCE |
             LINEDEVSTATE_TERMINALS |
             LINEDEVSTATE_NUMCALLS | 
			 LINEDEVSTATE_NUMCOMPLETIONS | 
			 LINEDEVSTATE_ROAMMODE |
             LINEDEVSTATE_BATTERY | 
			 LINEDEVSTATE_SIGNAL | 
			 LINEDEVSTATE_LOCK |
			 LINEDEVSTATE_COMPLCANCEL |
			 LINEDEVSTATE_CAPSCHANGE |
			 LINEDEVSTATE_CONFIGCHANGE |
			 LINEDEVSTATE_TRANSLATECHANGE |
			 LINEDEVSTATE_REMOVED);

    // This will be adjusted by each address added as the "SetAvailableMediaModes" API is called.
    m_LineCaps.dwMediaModes = 0;
    
    // This should be kept in synch with any PHONECAPS structures.
    m_LineCaps.dwRingModes = 1;

    // Always set the number of ACTIVE calls (i.e. connected) to one - derived
    // classes may override this if they can support more than one active call
    // at a time.  Reset the value during the Initcall -AFTER- this function.
    m_LineCaps.dwMaxNumActiveCalls = 1;

    // Now fill in the line device status
    m_LineStatus.dwNumActiveCalls = 0L;       // This will be modified as callstates change
    m_LineStatus.dwNumOnHoldCalls = 0L;       // This will be modified as callstates change
    m_LineStatus.dwNumOnHoldPendCalls = 0L;   // This will be modified as callstates change
    m_LineStatus.dwNumCallCompletions = 0L;   // This is filled out by the call appearance
    m_LineStatus.dwRingMode = 0;
    m_LineStatus.dwBatteryLevel = 0xffff;
    m_LineStatus.dwSignalLevel = 0xffff;
    m_LineStatus.dwRoamMode = LINEROAMMODE_UNAVAIL;
    m_LineStatus.dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED | LINEDEVSTATUSFLAGS_INSERVICE;

    // Set the device capability flags and v1.4 line features
    if (GetSP()->CanHandleRequest(TSPI_LINEMAKECALL))
        m_LineCaps.dwLineFeatures |= LINEFEATURE_MAKECALL;

    if (GetSP()->CanHandleRequest(TSPI_LINESETMEDIACONTROL))
    {
        m_LineCaps.dwDevCapFlags |= LINEDEVCAPFLAGS_MEDIACONTROL;
        m_LineCaps.dwLineFeatures |= LINEFEATURE_SETMEDIACONTROL;
        m_LineStatus.dwLineFeatures |= LINEFEATURE_SETMEDIACONTROL;
	}        

	if (GetSP()->CanHandleRequest(TSPI_LINEGATHERDIGITS))
	{
		m_LineCaps.dwGatherDigitsMinTimeout = LIBRARY_INTERVAL;
		m_LineCaps.dwGatherDigitsMaxTimeout = 0xffffffff;
	}

	if (GetSP()->CanHandleRequest(TSPI_LINEDEVSPECIFIC))
	{
		m_LineCaps.dwLineFeatures |= LINEFEATURE_DEVSPECIFIC;
		m_LineStatus.dwLineFeatures |= LINEFEATURE_DEVSPECIFIC;
	}

	if (GetSP()->CanHandleRequest(TSPI_LINEDEVSPECIFICFEATURE))
	{
		m_LineCaps.dwLineFeatures |= LINEFEATURE_DEVSPECIFICFEAT;
		m_LineStatus.dwLineFeatures |= LINEFEATURE_DEVSPECIFICFEAT;
	}

    if (GetSP()->CanHandleRequest(TSPI_LINEFORWARD))
    {
        m_LineCaps.dwLineFeatures |= LINEFEATURE_FORWARD;
        m_LineStatus.dwLineFeatures |= LINEFEATURE_FORWARD;
    }

    if (GetSP()->CanHandleRequest(TSPI_LINESETLINEDEVSTATUS))
	{
		m_LineCaps.dwLineFeatures |= LINEFEATURE_SETDEVSTATUS;
		m_LineStatus.dwLineFeatures |= LINEFEATURE_SETDEVSTATUS;
	}

    // Add terminal support - the LINEDEVSTATUS field will be updated when
    // the terminal is actually ADDED.
    if (GetSP()->CanHandleRequest(TSPI_LINESETTERMINAL))
        m_LineCaps.dwLineFeatures |= LINEFEATURE_SETTERMINAL;

	// Address types will be adjusted as addresses are created.
	m_LineCaps.dwAddressTypes = 0;
	m_LineCaps.dwAvailableTracking = LINECALLHUBTRACKING_ALLCALLS;
	m_LineCaps.ProtocolGuid = TAPIPROTOCOL_PSTN;
	m_LineCaps.dwDevCapFlags |= LINEDEVCAPFLAGS_CALLHUB;
	if (GetSP()->CanHandleRequest(TSPI_LINEGETCALLHUBTRACKING))  // We only check for get, not set.
		m_LineCaps.dwDevCapFlags |= LINEDEVCAPFLAGS_CALLHUBTRACKING;

	// Add in the "tapi/line" device class.
	AddDeviceClass (_T("tapi/line"), GetDeviceID());
	AddDeviceClass (_T("tapi/providerid"), GetDeviceInfo()->GetProviderID());

}// CTSPILineConnection::BaseInit

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Init
//
// Initialize this line connection object
//
void CTSPILineConnection::Init(CTSPIDevice* pDevice, DWORD dwLineId, DWORD dwPos, DWORD_PTR /*dwItemData*/)
{
	// Allow the base class to initialize
    CTSPIConnection::Init(pDevice, dwLineId);

    // Set our permanent line identifier which can be used to identify
	// this object uniquely within the scope of this service provider.
	SetPermanentLineID(MAKELONG((WORD)pDevice->GetProviderID(), dwPos+1));

	// Fill in all the required structures
	BaseInit(pDevice, dwLineId);

    // Derived class should fill in the remainder of the line capabilities.  We automatically
    // will fill in: (through various Addxxx functions).
    //
    // dwNumTerminals, MinDialParams, MaxDialParams, dwMediaModes, dwBearerModes, dwNumAddresses
    // dwLineNameSize, dwLineNameOffset, dwProviderInfoOffset, dwProviderInfoSize, dwSwitchInfoSize,
    // dwSwitchInfoOffset, dwMaxRate.
    //
    // All the other values should be filled in or left zero if not supported.  To fill them in
    // simply use "GetLineDevCaps()" and fill it in or derive a object from this and override Init.

}// CTSPILineConnection::Init

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::InitWithStream
//
// Initialize the line connection with a stream
//
void CTSPILineConnection::InitWithStream(CTSPIDevice* pDevice, DWORD dwLineID, DWORD /*dwPos*/, TStream& istm)
{
	TVersionInfo vi(istm);

	// Allow the base class to initialize
    CTSPIConnection::Init(pDevice, dwLineID);

	// Initialize the basic line information
	BaseInit(pDevice, dwLineID);

	// Read the permanent line device id.
	DWORD dwPermanentLineID;
	istm >> dwPermanentLineID;
	SetPermanentLineID(dwPermanentLineID);
	
	// Read the type - we use this to call a specific INIT function
	// which may be overriden by the derived provider.
	istm >> m_iLineType;

	// Read the associated phone id (if any)
	DWORD dwPhoneID;
	istm >> dwPhoneID;
	if (dwPhoneID != 0xffffffff)
	{
		CTSPIPhoneConnection* pPhone = GetDeviceInfo()->FindPhoneConnectionByPermanentID(dwPhoneID);
		if (pPhone != NULL)
		{
			AddDeviceClass(_T("tapi/phone"), pPhone->GetDeviceID());
			pPhone->AddDeviceClass(_T("tapi/line"), GetDeviceID());
		}
	}

	// Read the name of the line
	istm >> m_strName;

	// Read each of the addresses objects
	unsigned int iAddressCount = 0;
	istm >> iAddressCount;
	for (unsigned int i = 0; i < iAddressCount; i++)
	{
		TVersionInfo viAddr(istm);

		// Read all the information about this address and initialize it using the v2.x
		// initialization process so we toggle all the appropriate line flags.
		TString strName, strDN;
		bool fAllowIncoming=false, fAllowOutgoing=false;
		DWORD dwAvailMediaModes=0, dwBearerMode=0, dwMinRate=0, dwMaxRate=0;
		DWORD dwMaxNumActiveCalls=0, dwMaxNumOnHoldCalls=0, dwMaxNumOnHoldPendCalls=0;
		DWORD dwMaxNumConference=0, dwMaxNumTransConf=0;
		LINEDIALPARAMS dp;
		
		istm >> strName 
			 >> strDN 
			 >> fAllowIncoming 
			 >> fAllowOutgoing
             >> dwAvailMediaModes 
			 >> dwBearerMode 
			 >> dwMinRate 
			 >> dwMaxRate
			 >> dwMaxNumActiveCalls 
			 >> dwMaxNumOnHoldCalls 
			 >> dwMaxNumOnHoldPendCalls
			 >> dwMaxNumConference 
			 >> dwMaxNumTransConf
			 >> dp.dwDialPause 
			 >> dp.dwDialSpeed 
			 >> dp.dwDigitDuration 
			 >> dp.dwWaitForDialtone;

		DWORD dwAddressType = 0;
		if (viAddr.GetVersion() >= 3) // TAPI 3.0 supported
			istm >> dwAddressType;

		// Create the address
		CTSPIAddressInfo* pAddr = CreateAddress(strDN.c_str(), strName.c_str(), 
			fAllowIncoming, fAllowOutgoing, dwAvailMediaModes, dwBearerMode, 
			dwMinRate, dwMaxRate, &dp, dwMaxNumActiveCalls, dwMaxNumOnHoldCalls, 
			dwMaxNumOnHoldPendCalls, dwMaxNumConference, dwMaxNumTransConf, dwAddressType);
		_TSP_ASSERTE(pAddr != NULL);

		// Now allow the object to serialize for v3.x initialization.
		pAddr->read(istm);
	}

	// Initialize the line based on the line type
	if (GetLineType() == Queue)
	{
		// Adjust the ADDRESSCAPS for this queue.
		for (int i = 0; i < GetAddressCount(); i++)
		{
			CTSPIAddressInfo* pAddr = GetAddress(i);
			_TSP_ASSERTE (pAddr != NULL);
			LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
			lpAddrCaps->dwAddrCapFlags = LINEADDRCAPFLAGS_QUEUE;
		}
	}

	else if (GetLineType() == Trunk)
	{
		// Set the line features to drop only - most trunk devices on
		// ACD/PBX systems rely on the queues/stations to work with the
		// call.  The trunk is simply siezed as a connection to the outside
		// world.  Many switch interfaces DO allow for the call to be dropped
		// however.  If your switch is different, change the features in the 
		// derived line initialization.
		GetLineDevCaps()->dwAnswerMode = LINEANSWERMODE_NONE;

		// Adjust the ADDRESSCAPS for this trunk.
		for (int i = 0; i < GetAddressCount(); i++)
		{
			CTSPIAddressInfo* pAddr = GetAddress(i);
			_TSP_ASSERTE (pAddr != NULL);
			LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
			lpAddrCaps->dwAddrCapFlags = LINEADDRCAPFLAGS_NOINTERNALCALLS;
			lpAddrCaps->dwAddressFeatures = 0;
			lpAddrCaps->dwRemoveFromConfCaps = 
			lpAddrCaps->dwRemoveFromConfState = 
			lpAddrCaps->dwTransferModes = 0;
			lpAddrCaps->dwCallFeatures &= (
				LINECALLFEATURE_DROP |					// We can drop calls
				LINECALLFEATURE_SETCALLDATA);			// We can set call data on calls
			pAddr->SetAddressFeatures(0);
		}
		SetLineFeatures(0);
	}

	else if (GetLineType() == RoutePoint)
	{
		// Adjust the ADDRESSCAPS for this queue.
		for (int i = 0; i < GetAddressCount(); i++)
		{
			CTSPIAddressInfo* pAddr = GetAddress(i);
			_TSP_ASSERTE (pAddr != NULL);
			LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
			lpAddrCaps->dwAddrCapFlags = LINEADDRCAPFLAGS_ROUTEPOINT;
		}
	}

	else if (GetLineType() == PredictiveDialer)
	{
		// Adjust the ADDRESSCAPS for this dialer.
		for (int i = 0; i < GetAddressCount(); i++)
		{
			CTSPIAddressInfo* pAddr = GetAddress(i);
			_TSP_ASSERTE (pAddr != NULL);
			LPLINEADDRESSCAPS lpAddrCaps = pAddr->GetAddressCaps();
			lpAddrCaps->dwAddrCapFlags = 
				(LINEADDRCAPFLAGS_ORIGOFFHOOK | 
				 LINEADDRCAPFLAGS_PREDICTIVEDIALER |
				 LINEADDRCAPFLAGS_DIALED);
			lpAddrCaps->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;
			lpAddrCaps->dwPredictiveAutoTransferStates = 
				(LINECALLSTATE_ACCEPTED | 
				 LINECALLSTATE_CONNECTED | 
				 LINECALLSTATE_RINGBACK |
				 LINECALLSTATE_DISCONNECTED |
				 LINECALLSTATE_PROCEEDING | 
				 LINECALLSTATE_BUSY);
			lpAddrCaps->dwMaxNoAnswerTimeout = 0xffff;
		}
	}

	// Read agent support
	bool fSupportsAgents=0;
	istm >> fSupportsAgents;

	// If this is a version 3.x stream then load the TAPI 3.0 information
	if (vi.GetVersion() >= 3)
	{
		GUID guid;
		istm >> guid;
		SetMSPGUID(guid);

		// Load the CLSID for the device caps.
		istm >> m_LineCaps.ProtocolGuid;
	}

	// Now allow the derived class to read the additional information
	// stored in the stream
	read(istm);

	// If the agent flag was set then enable the agent proxy for this line.
	// We do this after the read so that the agent features may be set correctly.
	if (fSupportsAgents) EnableAgentProxy();

}// CTSPILineConnection::InitWithStream

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SaveToStream
//
// This function stores the line object off in the given stream.
//
void CTSPILineConnection::SaveToStream(TStream& ostm)
{
	TVersionInfo vi(ostm, 3);

	// Lookup the phone id.
	DWORD dwPhoneID = 0xffffffff;
	DEVICECLASSINFO* pDevClass = GetDeviceClass(_T("tapi/phone"));
	if (pDevClass != NULL && pDevClass->dwStringFormat == STRINGFORMAT_BINARY &&
		pDevClass->dwSize == sizeof(DWORD))
		dwPhoneID = *(reinterpret_cast<LPDWORD>(pDevClass->lpvData.get()));

	ostm << m_dwDeviceID << m_iLineType << dwPhoneID << m_strName;

	// Write each of the addresses out
	int nCount = GetAddressCount();
	ostm << nCount;
	for (int i = 0; i < nCount; i++)
		GetAddress(i)->SaveToStream(ostm);

	// Write our agent support
	bool fSupportsAgents = (m_dwFlags & IsAgentEnabled) > 0;
	ostm << fSupportsAgents;

	// Save off the media information for TAPI 3.x
	ostm << m_guidMSP;
	ostm << m_LineCaps.ProtocolGuid;

	// Now allow the object to serialize itself for v3.x initialization.
	write(ostm);

}// CTSPILineConnection::SaveToStream

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::read
//
// Reads the object from a stream - should be overriden to provide
// initialization support in the v3.x INIT process.
//
TStream& CTSPILineConnection::read(TStream& istm)
{
	return istm;

}// CTSPILineConnection::read

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::write
//
// Writes the object from a stream - should be overriden to provide
// extra data in the stream.
//
TStream& CTSPILineConnection::write(TStream& istm) const
{
	return istm;

}// CTSPILineConnection::write

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindAddress
//
// Locate an address based on a dialable address.
//
CTSPIAddressInfo* CTSPILineConnection::FindAddress (LPCTSTR lpszAddress) const
{                                  
	// Note: we don't lock the object since we cannot DELETE address objects.
	// Therefore once it is there, it is there to stay.  If we ever have the ability
	// to dynamically add/remove addresses under a line we will need to LOCK the
	// line here.
    for (TAddressArray::const_iterator it = m_arrAddresses.begin(); it != m_arrAddresses.end(); ++it)
	{
        if (!lstrcmpi((*it)->GetDialableAddress(), lpszAddress))
			return (*it);
	}
    return NULL;

}// CTSPILineConnection::FindAddress

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetPermanentDeviceID
//
// Return a permanent device id for this line identifying the provider
// and line.
//
DWORD CTSPILineConnection::GetPermanentDeviceID() const
{
    return m_LineCaps.dwPermanentLineID;

}// CTSPILineConnection::GetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnPreCallStateChange
//
// A call is about to change state on an address on this line.
//
void CTSPILineConnection::OnPreCallStateChange (CTSPIAddressInfo* /*pAddr*/, CTSPICallAppearance* /*pCall*/, DWORD dwNewState, DWORD dwOldState)
{
	// If the state has not changed, ignore.
	if (dwNewState == dwOldState)
		return;

	// Make sure only this thread is updating the status.
	CEnterCode Key(this);  // Synch access to object

    // Determine if the number of active calls has changed.
    bool fWasActive = CTSPICallAppearance::IsActiveCallState(dwOldState);
    bool fIsActive  = CTSPICallAppearance::IsActiveCallState(dwNewState);

    if (fWasActive == false && fIsActive == true)
    {       
        m_LineStatus.dwNumActiveCalls++;
		m_dwFlags |= NotifyNumCalls;
    }
    else if (fWasActive == true && fIsActive == false)
    {
        if (--m_LineStatus.dwNumActiveCalls & 0x80000000)
            m_LineStatus.dwNumActiveCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
    }       

    // Determine if the HOLD status has changed.        
    if (dwNewState == LINECALLSTATE_ONHOLD)
    {
        m_LineStatus.dwNumOnHoldCalls++;
		m_dwFlags |= NotifyNumCalls;
    }
    else if (dwNewState == LINECALLSTATE_ONHOLDPENDTRANSFER || dwNewState == LINECALLSTATE_ONHOLDPENDCONF)
    {
        m_LineStatus.dwNumOnHoldPendCalls++;
		m_dwFlags |= NotifyNumCalls;
    }

    if (dwOldState == LINECALLSTATE_ONHOLD)
    {
        if (--m_LineStatus.dwNumOnHoldCalls & 0x80000000)
            m_LineStatus.dwNumOnHoldCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
    }

    else if (dwOldState == LINECALLSTATE_ONHOLDPENDTRANSFER || dwOldState == LINECALLSTATE_ONHOLDPENDCONF)
    {
        if (--m_LineStatus.dwNumOnHoldPendCalls & 0x80000000)
            m_LineStatus.dwNumOnHoldPendCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
    }

	Key.Unlock();

	_TSP_DTRACE(_T("%s: Line 0x%lx Active=%ld, OnHold=%ld,  OnHoldPend=%ld\n"), m_strName.c_str(), GetPermanentDeviceID(),
		m_LineStatus.dwNumActiveCalls, m_LineStatus.dwNumOnHoldCalls, m_LineStatus.dwNumOnHoldPendCalls);            

	RecalcLineFeatures(true);

}// CTSPILineConnection::OnPreCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnCallStateChange
//
// A call has changed state on an address on this line.  Update our
// status.
//
void CTSPILineConnection::OnCallStateChange (CTSPIAddressInfo* /*pAddr*/, CTSPICallAppearance* /*pCall*/, 
                                             DWORD /*dwNewState*/, DWORD /*dwOldState*/)
{   
    if (m_dwFlags & NotifyNumCalls)
	{
        OnLineStatusChange (LINEDEVSTATE_NUMCALLS);
		m_dwFlags &= ~NotifyNumCalls;
	}

	// Recalc our line features based on the new call states.
	RecalcLineFeatures(true);

}// CTSPILineConnection::OnCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnLineStatusChange
//
// This method is called when any of the values in our line device
// status record are changed.  It is called internally by the library
// and should also be called by the derived class if the LINDEVSTATUS
// structure is modified directly.
//
void CTSPILineConnection::OnLineStatusChange (DWORD dwState, DWORD dwP2, DWORD dwP3)
{
	if (GetLineHandle() && ((m_dwLineStates & dwState) || dwState == LINEDEVSTATE_REINIT))
        Send_TAPI_Event (NULL, LINE_LINEDEVSTATE, dwState, dwP2, dwP3);

}// CTSPILineConnection::OnLineStatusChange

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetMSPGUID
//
// Assigns the MSP COM GUID for this line device.
//
void CTSPILineConnection::SetMSPGUID(GUID& guidMSP)
{
	if (memcmp(&m_guidMSP, &guidMSP, sizeof(GUID)) != 0)
	{
		memcpy(&m_guidMSP, &guidMSP, sizeof(GUID));
		if (guidMSP != IID_NULL)
			m_LineCaps.dwDevCapFlags |= LINEDEVCAPFLAGS_MSP;
		else
			m_LineCaps.dwDevCapFlags &= ~LINEDEVCAPFLAGS_MSP;
		OnLineCapabiltiesChanged();
	}

}// CTSPILineConnection::SetMSPGUID

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Open
//
// Open the line device
//
LONG CTSPILineConnection::Open (
HTAPILINE htLine,                // TAPI opaque handle to use for line
LINEEVENT lpfnEventProc,         // Event procedure address for notifications
DWORD dwTSPIVersion)             // Version of TSPI to synchronize to
{
    // If we are already open, return allocated.
    if (GetLineHandle() != 0)
        return LINEERR_ALLOCATED;

    // Save off the event procedure for this line and the TAPI
    // opaque line handle which represents this line to the application.
    m_lpfnEventProc = lpfnEventProc;
    m_htLine = htLine;
    m_dwNegotiatedVersion = dwTSPIVersion;

    // Tell our device to perform an open for this connection.
    if (!OpenDevice())
	{
		m_lpfnEventProc = NULL;
		m_htLine = 0;
		m_dwNegotiatedVersion = 0;
		return LINEERR_RESOURCEUNAVAIL;
	}

	// Force our line and addresses to calculate their feature set - this
	// may the first time the address calculates it's feature set. (v3.0b)
	RecalcLineFeatures(true);

    return 0L;

}// CTSPILineConnection::Open

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Close
//
// Close the line connection and destroy any call appearances that
// are active.
//
// The htLine handle is invalid after this completes.
//
LONG CTSPILineConnection::Close()
{
    if (GetLineHandle() != 0)
    {
		// If the line capabilities specify that we DROP all existing calls
		// on the line when it is closed, then spin through each of the 
		// calls active on this line and issue a drop request.
		if ((GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_CLOSEDROP) ==
				LINEDEVCAPFLAGS_CLOSEDROP)
		{
			for (int iAddress = 0; iAddress < GetAddressCount(); iAddress++)
			{
				CTSPIAddressInfo* pAddress = GetAddress(iAddress);
				for (int iCalls = 0; iCalls < pAddress->GetCallCount(); iCalls++)
				{
					CTSPICallAppearance* pCall = pAddress->GetCallInfo(iCalls);
					if (pCall != NULL) pCall->DropOnClose();
				}
			}
		}

        // Make sure any pending call close operations complete.
        WaitForAllRequests (NULL, REQUEST_DROPCALL);
        
        // Remove any other pending requests for this connection.
        RemovePendingRequests();

        // Decrement our total line open count.
        OnLineStatusChange (LINEDEVSTATE_CLOSE);
        
        // Reset our event and line proc.
        m_lpfnEventProc = NULL;
        m_htLine = 0;
        m_dwLineStates = 0L;
		m_dwNegotiatedVersion = GetSP()->GetSupportedVersion();
        
		// If the line has been removed, then mark it as DELETED now
		// so we will refuse any further traffic on this line.
		if (GetFlags() & _IsRemoved)
			m_dwFlags |= _IsDeleted;

		// If the TSP indicated that it wants to delete all the calls when the line
		// is closed, then do so.
		if (m_dwFlags & DelCallsOnClose)
		{
			for (int iAddress = 0; iAddress < GetAddressCount(); iAddress++)
			{
				// Loop through all calls on the address until we have none
				// left to remove. 
				CTSPIAddressInfo* pAddress = GetAddress(iAddress);
				while (pAddress->GetCallCount() > 0)
				{
					CTSPICallAppearance* pCall = pAddress->GetCallInfo(0);
					if (pCall)
					{
						// Force the call to idle -- this will remove all calls
						// from the address object.
						if (pCall->GetCallState() != LINECALLSTATE_IDLE)
						{
							pCall->SetCallState(LINECALLSTATE_IDLE);
							// Does a remove in OnCallStatusChange(..)
						}
						else // already idle -- remove it from the address.
						{
							pAddress->RemoveCallAppearance(pCall);
						}
					}
				}
				_TSP_ASSERTE(pAddress->GetCallCount() == 0);
			}
		}

		// Now set our connected call count to zero.  We do this after closing all
		// calls so that we don't end up with a negative call count (since the IDLEing
		// of the call will cause a function call to OnConnectedCallCountChange).
		m_dwConnectedCallCount = 0;

        // Tell our device to close.
        CloseDevice();

		// Finally, force a recalc of our features.
		RecalcLineFeatures(true);

		// Success
		return 0L;
    }   
    
    return LINEERR_OPERATIONFAILED;

}// CTSPILineConnection::Close

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Send_TAPI_Event
//
// Calls an event back into the TAPI DLL.  It is assumed that the
// validity of the event has been verified before calling this 
// function.
//
void CTSPILineConnection::Send_TAPI_Event(
CTSPICallAppearance* pCall, // Call appearance to send event for
DWORD dwMsg,                // Message to send (LINExxx)
DWORD dwP1,                 // Parameter 1 (depends on above message)
DWORD dwP2,                 // Parameter 2 (depends on above message)
DWORD dwP3)                 // Parameter 3 (depends on above message)
{
    HTAPILINE htLine = GetLineHandle();
	if (dwMsg == LINE_CREATEDIALOGINSTANCE)
	{
		htLine = (HTAPILINE) GetDeviceInfo()->GetProviderHandle();
#ifdef _DEBUG
		if (htLine == NULL)
		{
			// This happens because TSPI_providerEnumDevices
			// is not exported.  It must be exported in TAPI 2.x/3.0
			_TSP_ASSERT (false);
			_TSP_DTRACE(_T("ERROR: TSPI_providerEnumDevices needs to be exported!\n"));
			return;
		}
#endif
	}
	else if (dwMsg == LINE_SENDDIALOGINSTANCEDATA)
	{
		htLine = (HTAPILINE) dwP3;
		dwP3 = 0L;
	}

    // If a call appearance was supplied, get the call handle from it.
    HTAPICALL htCall = (HTAPICALL) 0;
    if (pCall)
	{
        htCall = pCall->GetCallHandle();
		// If the call handle is NULL then this call has been deallocated by TAPI
		// and we don't need to inform about changes anymore.
		if (htCall == NULL) return;
	}
        
#ifdef _DEBUG
    static LPCTSTR g_pszMsgs[] = {
            {_T("Line_AddressState")},               // 0
            {_T("Line_CallInfo")},                   // 1
            {_T("Line_CallState")},                  // 2
            {_T("Line_Close")},                      // 3
            {_T("Line_DevSpecific")},                // 4
            {_T("Line_DevSpecificFeature")},         // 5
            {_T("Line_GatherDigits")},               // 6
            {_T("Line_Generate")},                   // 7
            {_T("Line_LineDevState")},               // 8
            {_T("Line_MonitorDigits")},              // 9
            {_T("Line_MonitorMedia")},               // 10
            {_T("Line_MonitorTone")},                // 11
            {_T("Line_Reply")},                      // 12
            {_T("Line_Request")},                    // 13
            {_T("Phone_Button")},                    // 14
            {_T("Phone_Close")},                     // 15
            {_T("Phone_DevSpecific")},               // 16
            {_T("Phone_Reply")},                     // 17
            {_T("Phone_State")},                     // 18
            {_T("Line_Create")},                     // 19
            {_T("Phone_Create")},                    // 20
			{_T("Line_AgentSpecific")},				 // 21
			{_T("Line_AgentStatus")},				 // 22
			{_T("Line_AppNewCall")},				 // 23
			{_T("Line_ProxyRequest")},				 // 24
			{_T("Line_Remove")},					 // 25
			{_T("Phone_Remove")}					 // 26
        };        
    static LPCTSTR g_pszTSPIMsgs[] = {
			{_T("Line_NewCall")},					 // 0
			{_T("Line_CallDevSpecific")},            // 1
			{_T("Line_CallDevSpecificFeature")},     // 2
			{_T("Line_CreateDialogInstance")},       // 3
			{_T("Line_SendDialogInstanceData")}      // 4
		};

	LPCTSTR pszMsg = _T("???");			
	if (dwMsg <= 26)
		pszMsg = g_pszMsgs[dwMsg];
	else if (dwMsg >= TSPI_MESSAGE_BASE && dwMsg < TSPI_MESSAGE_BASE+5)
		pszMsg = g_pszTSPIMsgs[dwMsg-TSPI_MESSAGE_BASE];

    // Send the notification to TAPI.
	_TSP_DTRACE(_T("%s: Send_TAPI_Event: <0x%lx> Line=0x%lx, Call=0x%lx, Msg=0x%lx (%s), P1=0x%lx, P2=0x%lx, P3=0x%lx\n"), m_strName.c_str(),
                this, htLine, htCall, dwMsg, pszMsg, dwP1, dwP2, dwP3);
#endif

	// If this is a PROXY request, send it up to the device.
	if (dwMsg == LINE_AGENTSPECIFIC || dwMsg == LINE_AGENTSTATUS)
		GetDeviceInfo()->ProxyNotify(this, dwMsg, dwP1, dwP2, dwP3);
	else
	{
		if (m_lpfnEventProc != NULL)
			(*m_lpfnEventProc)(htLine, htCall, dwMsg, dwP1, dwP2, dwP3);
	}

}// CTSPILineConnection::Send_TAPI_Event

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateAddress
//
// Create a new address on this line.
//
CTSPIAddressInfo* CTSPILineConnection::CreateAddress (
			LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, bool fInput, bool fOutput, 
			DWORD dwAvailMediaModes, DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate,
            LPLINEDIALPARAMS lpDial, DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
			DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, DWORD dwMaxNumTransConf, DWORD dwAddressType)
{
    CTSPIAddressInfo* pAddr = GetSP()->CreateAddressObject();
	if (pAddr == NULL)
		throw LINEERR_NOMEM;

    // Add it to our array.
	CEnterCode sLock(this);  // Synch access to object

	try
	{
		m_arrAddresses.push_back(pAddr);
	}
	catch(...)
	{
		delete pAddr;
		throw;
	}

	int iPos = (m_arrAddresses.size()-1);

    // Increment the number of addresses.
    ++m_LineCaps.dwNumAddresses;
    
    // Update our line status features if they are now different.
    if (fOutput && GetSP()->CanHandleRequest(TSPI_LINEMAKECALL))
        m_LineStatus.dwLineFeatures |= LINEFEATURE_MAKECALL;

    // Update our line device capabilities with mediamode, bearermode info.
    m_LineCaps.dwBearerModes |= dwBearerMode;
    m_LineCaps.dwMediaModes |= dwAvailMediaModes;
    
    // Update the MAXRATE information.  This field is used in two fashions:
    // If the bearermode includes DATA, then this field indicates the top bit rate
    // of the digital channel.
	//
    // Otherwise, if it doesn't include data, but has VOICE, and the mediamode includes
    // DATAMODEM, then this should be set to the highest synchronous DCE bit rate
    // supported (excluding compression).
    if (dwMaxRate > m_LineCaps.dwMaxRate)
        m_LineCaps.dwMaxRate = dwMaxRate;
        
    // If we got a dial parameters list, modify our min/max dial parameters.
    if (lpDial)
    {
        if (m_LineCaps.MinDialParams.dwDialPause > lpDial->dwDialPause)
            m_LineCaps.MinDialParams.dwDialPause = lpDial->dwDialPause;
        if (m_LineCaps.MinDialParams.dwDialSpeed > lpDial->dwDialSpeed)
            m_LineCaps.MinDialParams.dwDialSpeed = lpDial->dwDialSpeed;
        if (m_LineCaps.MinDialParams.dwDigitDuration > lpDial->dwDigitDuration)
            m_LineCaps.MinDialParams.dwDigitDuration = lpDial->dwDigitDuration;
        if (m_LineCaps.MinDialParams.dwWaitForDialtone > lpDial->dwWaitForDialtone)
            m_LineCaps.MinDialParams.dwWaitForDialtone = lpDial->dwWaitForDialtone;
        if (m_LineCaps.MaxDialParams.dwDialPause < lpDial->dwDialPause)
            m_LineCaps.MaxDialParams.dwDialPause = lpDial->dwDialPause;
        if (m_LineCaps.MaxDialParams.dwDialSpeed < lpDial->dwDialSpeed)
            m_LineCaps.MaxDialParams.dwDialSpeed = lpDial->dwDialSpeed;
        if (m_LineCaps.MaxDialParams.dwDigitDuration < lpDial->dwDigitDuration)
            m_LineCaps.MaxDialParams.dwDigitDuration = lpDial->dwDigitDuration;
        if (m_LineCaps.MaxDialParams.dwWaitForDialtone < lpDial->dwWaitForDialtone)
            m_LineCaps.MaxDialParams.dwWaitForDialtone = lpDial->dwWaitForDialtone;
    }

	sLock.Unlock();

	m_LineCaps.dwAddressTypes |= dwAddressType;

    // Init the address
    pAddr->Init (this, iPos, lpszDialableAddr, lpszAddrName, fInput, fOutput,
                 dwAvailMediaModes, dwBearerMode, dwMinRate, dwMaxRate,
                 dwMaxNumActiveCalls, dwMaxNumOnHoldCalls, dwMaxNumOnHoldPendCalls,
                 dwMaxNumConference, dwMaxNumTransConf, LINEADDRESSSHARING_PRIVATE, lpDial, dwAddressType);

	// If the name of the address was NULL, then create a NEW name for the address.
	if (lpszAddrName == NULL)
	{
		TCHAR szName[20];
		wsprintf(szName, _T("Address%ld"), m_LineCaps.dwNumAddresses);
		pAddr->SetName(szName);
	}

    return pAddr;

}// CTSPILineConnection::CreateAddress

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateMonitoredAddress
//
// This creates a monitored address (an address which has the sharing mode
// of LINEADDRESSSHARING_MONITORED.
//
CTSPIAddressInfo* CTSPILineConnection::CreateMonitoredAddress(LPCTSTR lpszDialableAddr, 
			LPCTSTR lpszAddrName, DWORD dwAvailMediaModes, 
			DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate, DWORD dwMaxNumActiveCalls, 
			DWORD dwMaxNumOnHoldCalls, DWORD dwMaxNumOnHoldPendCalls, DWORD dwAddressType)
{
    CTSPIAddressInfo* pAddr = GetSP()->CreateAddressObject();
	if (pAddr == NULL)
		throw LINEERR_NOMEM;

    // Add it to our array.
	CEnterCode sLock(this);  // Synch access to object

	try
	{
		m_arrAddresses.push_back(pAddr);
	}
	catch (...)
	{
		delete pAddr;
	}

	int iPos = (m_arrAddresses.size()-1);

    // Increment the number of addresses.
    ++m_LineCaps.dwNumAddresses;

    // Update our line device capabilities with mediamode, bearermode info.
    m_LineCaps.dwBearerModes |= dwBearerMode;
    m_LineCaps.dwMediaModes |= dwAvailMediaModes;
    
    // Update the MAXRATE information.  This field is used in two fashions:
    // If the bearermode includes DATA, then this field indicates the top bit rate
    // of the digital channel.
	//
    // Otherwise, if it doesn't include data, but has VOICE, and the mediamode includes
    // DATAMODEM, then this should be set to the highest synchronous DCE bit rate
    // supported (excluding compression).
    if (dwMaxRate > m_LineCaps.dwMaxRate)
        m_LineCaps.dwMaxRate = dwMaxRate;
        
	sLock.Unlock();

	m_LineCaps.dwAddressTypes |= dwAddressType;

    // Init the address
    pAddr->Init (this, iPos, lpszDialableAddr, lpszAddrName, false, false,
                 dwAvailMediaModes, dwBearerMode, dwMinRate, dwMaxRate,
                 dwMaxNumActiveCalls, dwMaxNumOnHoldCalls, dwMaxNumOnHoldPendCalls,
                 0,0, LINEADDRESSSHARING_MONITORED, NULL, dwAddressType);

	// If the name of the address was NULL, then create a NEW name for the address.
	if (lpszAddrName == NULL)
	{
		TCHAR szName[20];
		wsprintf(szName, _T("Address%ld"), m_LineCaps.dwNumAddresses);
		pAddr->SetName(szName);
	}

    return pAddr;

}// CTSPILineConnection::CreateMonitoredAddress

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GatherCapabilities
//
// Gather the line device capabilities for this list.
//
LONG CTSPILineConnection::GatherCapabilities (DWORD dwTSPIVersion, DWORD /*dwExtVer*/, LPLINEDEVCAPS lpLineCaps)
{   
	CEnterCode sLock(this);  // Synch access to object

	// Determine the full size required for our line capabilities
    TString strLineName = GetName();
	TString strProviderInfo = GetSP()->GetProviderInfo();
	TString strSwitchInfo = GetDeviceInfo()->GetSwitchInfo();
    
	// Get a list of the device classes to add
	TString strDeviceNames;
	for (TDeviceClassArray::iterator ii = m_arrDeviceClass.begin(); ii != m_arrDeviceClass.end(); ++ii)
	{
		DEVICECLASSINFO* pDevClass = (*ii).second;
		strDeviceNames += pDevClass->strName + _T('~');
	}

	// Add a final NULL marker
	if (!strDeviceNames.empty())
		strDeviceNames += _T('~');

	// Copy over the static structure of the specified size based on the TSPI version requested
	// from TAPI.  This is determined by the calling application's API level.  This means that
	// we can have older TAPI complient applications calling us which don't expect the extra
	// information in our LINEDEVCAPS record.
	DWORD dwReqSize = sizeof(LINEDEVCAPS);
	if (dwTSPIVersion < TAPIVER_30)
		dwReqSize -= (sizeof(GUID) + sizeof(DWORD)*2);
	if (dwTSPIVersion < TAPIVER_22)
		dwReqSize -= sizeof(GUID);
	if (dwTSPIVersion < TAPIVER_20)
		dwReqSize -= sizeof(DWORD) * 3;
	if (dwTSPIVersion < TAPIVER_14)
		dwReqSize -= sizeof(DWORD);

    // Check the available size to make sure we have enough room.   
    m_LineCaps.dwTotalSize = lpLineCaps->dwTotalSize;
	m_LineCaps.dwNeededSize = sizeof(LINEDEVCAPS);
	m_LineCaps.dwUsedSize = dwReqSize;

    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.
	_TSP_ASSERT(dwReqSize <= lpLineCaps->dwTotalSize);
    if (dwReqSize > lpLineCaps->dwTotalSize)
    {
		lpLineCaps->dwNeededSize = m_LineCaps.dwNeededSize;
        return LINEERR_STRUCTURETOOSMALL;
    }

	// Copy over only what is allowed for the negotiated version of TAPI.
    MoveMemory(lpLineCaps, &m_LineCaps, dwReqSize);
  
    // Remove the additional capabilities if we are not at the proper TSPI version.  
	if (dwTSPIVersion < TAPIVER_20)
	{
		lpLineCaps->dwBearerModes &= ~LINEBEARERMODE_RESTRICTEDDATA;
		lpLineCaps->dwLineFeatures &= ~(LINEFEATURE_SETDEVSTATUS | LINEFEATURE_FORWARDFWD | LINEFEATURE_FORWARDDND);      
	}

    if (dwTSPIVersion < TAPIVER_14)    
    {
		lpLineCaps->dwBearerModes &= ~LINEBEARERMODE_PASSTHROUGH;
        lpLineCaps->dwMediaModes &= ~LINEMEDIAMODE_VOICEVIEW;
		lpLineCaps->dwLineStates &= ~(LINEDEVSTATE_CAPSCHANGE |
				LINEDEVSTATE_CONFIGCHANGE |
				LINEDEVSTATE_TRANSLATECHANGE |
				LINEDEVSTATE_COMPLCANCEL |
				LINEDEVSTATE_REMOVED);
	}        
    
    // If we have enough room for the provider information, then add it to the end.
	if (!strProviderInfo.empty())
		AddDataBlock (lpLineCaps, lpLineCaps->dwProviderInfoOffset, lpLineCaps->dwProviderInfoSize,
					  strProviderInfo.c_str());

    // If we have enough room for the line name, then add it.
	if (!strLineName.empty())
		AddDataBlock (lpLineCaps, lpLineCaps->dwLineNameOffset, lpLineCaps->dwLineNameSize,
					  strLineName.c_str());
    
    // If we have enough room for the switch information, then add it.
	if (!strSwitchInfo.empty())
		AddDataBlock (lpLineCaps, lpLineCaps->dwSwitchInfoOffset, lpLineCaps->dwSwitchInfoSize,
					  strSwitchInfo.c_str());
	
	// Handle the line terminal capabilities.
	lpLineCaps->dwTerminalCapsSize = 0L;
	TTerminalInfoArray::iterator it;
	for (it = m_arrTerminalInfo.begin(); it != m_arrTerminalInfo.end(); ++it)
		AddDataBlock (lpLineCaps, lpLineCaps->dwTerminalCapsOffset, lpLineCaps->dwTerminalCapsSize, &(*it)->Capabilities, sizeof(LINETERMCAPS));

    // Add the terminal name information if we have the space.
	unsigned int iTermNameLen = ((lpLineCaps->dwTerminalTextEntrySize-sizeof(wchar_t)) / sizeof(wchar_t));
	lpLineCaps->dwTerminalTextSize = 0L;
	for (it = m_arrTerminalInfo.begin(); it != m_arrTerminalInfo.end(); ++it)
    {
		TString strName = (*it)->strName;
		strName.resize(iTermNameLen, _T(' '));
		AddDataBlock (lpLineCaps, lpLineCaps->dwTerminalTextOffset, lpLineCaps->dwTerminalTextSize, strName.c_str());
	}

	// If we have some lineGetID supported device classes, return the list of supported device classes.
	if (dwTSPIVersion >= TAPIVER_20)
	{
		if (AddDataBlock(lpLineCaps, lpLineCaps->dwDeviceClassesOffset, lpLineCaps->dwDeviceClassesSize, strDeviceNames.c_str()))
		{
			// Strip out the ~ and replace with nulls.
			wchar_t* pbd = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpLineCaps)+lpLineCaps->dwDeviceClassesOffset);
			std::transform(pbd, pbd+lpLineCaps->dwDeviceClassesSize,pbd, tsplib::substitue<wchar_t>(L'~',L'\0'));
		}
	}
	return 0L;

}// CTSPILineConnection::GatherCapabilities

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GatherStatus
//
// Return the status of this line connection.
//
LONG CTSPILineConnection::GatherStatus (LPLINEDEVSTATUS lpLineDevStatus)
{
	// Get the version the line opened negotiation to.
	DWORD dwTSPIVersion = GetNegotiatedVersion();
	if (dwTSPIVersion == 0)
		dwTSPIVersion = GetSP()->GetSupportedVersion();

	// Synch access to object
	CEnterCode sLock(this);

	// Run through the addresses and see what media modes are available on outgoing lines.
	m_LineStatus.dwAvailableMediaModes = 0L;
	for (int x = 0; x < GetAddressCount(); x++)
	{
		// If the call appearance can have another active call.. grab its available media modes
		CTSPIAddressInfo* pAddr = GetAddress(x);
		if (pAddr->CanMakeCalls() &&
			pAddr->GetAddressStatus()->dwNumActiveCalls < pAddr->GetAddressCaps()->dwMaxNumActiveCalls)
		{
			m_LineStatus.dwAvailableMediaModes |= pAddr->GetAddressCaps()->dwAvailableMediaModes;
		}
	}

	// Move over the pre-filled fields.
	DWORD dwRequiredSize = sizeof(LINEDEVSTATUS);
	m_LineStatus.dwTotalSize = lpLineDevStatus->dwTotalSize;
	m_LineStatus.dwNeededSize = sizeof(LINEDEVSTATUS);
	m_LineStatus.dwNumOpens = lpLineDevStatus->dwNumOpens;
	m_LineStatus.dwOpenMediaModes = lpLineDevStatus->dwOpenMediaModes;

	// Determine the required size of our TAPI structure based on the caller and the version
	// the line originally negotiated at.
	if (dwTSPIVersion < TAPIVER_20)
		dwRequiredSize -= sizeof(DWORD)*3;
	else
	{
		// Copy over the information which TAPI will supply.
		m_LineStatus.dwAppInfoSize = lpLineDevStatus->dwAppInfoSize;
		m_LineStatus.dwAppInfoOffset = lpLineDevStatus->dwAppInfoOffset;
	}

#ifdef _DEBUG
    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.  NOTE: This should never
	// happen - TAPI.DLL is supposed to verify that there is enough space in the structure
	// for the negotiated version and return an error.  We only check this in DEBUG builds
	// to insure that TAPI is doing its job.
    if (lpLineDevStatus->dwTotalSize < dwRequiredSize)
	{
		_TSP_ASSERT (false);
		lpLineDevStatus->dwNeededSize = m_LineStatus.dwNeededSize;
        return LINEERR_STRUCTURETOOSMALL;
	}
#endif
    
    // Copy our structure onto the user structure
    MoveMemory(lpLineDevStatus, &m_LineStatus, dwRequiredSize);
    lpLineDevStatus->dwUsedSize = dwRequiredSize;

    // Now fill in the additional fields.
	lpLineDevStatus->dwNumCallCompletions = m_lstCompletions.size();
	                        
    // Fill in the terminal information if we have space.
    if (!m_arrTerminalInfo.empty())
    {
        for (unsigned int i = 0; i < m_arrTerminalInfo.size(); i++)
        {
            TERMINALINFO* lpTermInfo = m_arrTerminalInfo[i];
			AddDataBlock(lpLineDevStatus, lpLineDevStatus->dwTerminalModesOffset,
					      lpLineDevStatus->dwTerminalModesSize, &lpTermInfo->dwMode, sizeof(DWORD));
        }                
    }
	return 0L;

}// CTSPILineConnection::GatherStatus

/////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindAvailableAddress
//
// Locate an address which is available for an outgoing call.
//
CTSPIAddressInfo* CTSPILineConnection::FindAvailableAddress (LPLINECALLPARAMS const lpCallParams, 
															 DWORD dwFeature) const
{
    // Walk through all our addresses and look to see if they can support
    // the type of call desired.
    for (int x = 0; x < GetAddressCount(); x++)
    {
        CTSPIAddressInfo* pAddr = GetAddress(x);
        
		// If the call appearance can have another active call.. 
		LINEADDRESSSTATUS* pAS = pAddr->GetAddressStatus();
		LINEADDRESSCAPS*   pAC = pAddr->GetAddressCaps();

		// If no feature is specified, check the MAX number of
		// calls available.
		if (dwFeature == 0 &&
			(pAS->dwNumActiveCalls >= pAC->dwMaxNumActiveCalls))
			continue;
		
		// If the feature(s) are available
		if (dwFeature > 0 && (pAS->dwAddressFeatures & dwFeature) != dwFeature)
			continue;

        // And can support the type of call required..
        if (lpCallParams && pAddr->CanSupportCall(lpCallParams) != 0)
            continue;

		// This address fits the bill.
        return pAddr;
    }
    return NULL;

}// CTSPILineConnection::FindAvailableAddress        

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CanSupportCall
//
// Return true/false whether this line can support the type of
// call specified.
//
LONG CTSPILineConnection::CanSupportCall (LPLINECALLPARAMS const lpCallParams) const
{                                    
    if ((lpCallParams->dwBearerMode & m_LineCaps.dwBearerModes) != lpCallParams->dwBearerMode)
        return LINEERR_INVALBEARERMODE;
        
    if (m_LineCaps.dwMaxRate > 0 && lpCallParams->dwMaxRate > m_LineCaps.dwMaxRate)
        return LINEERR_INVALRATE;
        
    if ((lpCallParams->dwMediaMode & m_LineCaps.dwMediaModes) != lpCallParams->dwMediaMode)
        return LINEERR_INVALMEDIAMODE;

	if (m_dwNegotiatedVersion >= TAPIVER_30 && (lpCallParams->dwAddressType > 0) &&
		(lpCallParams->dwAddressType & m_LineCaps.dwAddressTypes) == 0)
		return LINEERR_INVALADDRESSTYPE;

    // If a specific address is identified, then run it through that address to
    // insure that the other fields are ok, otherwise, check them all.
    if (lpCallParams->dwAddressMode == LINEADDRESSMODE_ADDRESSID)
    {
        CTSPIAddressInfo* pAddr = GetAddress(lpCallParams->dwAddressID);
        if (pAddr != NULL)
            return pAddr->CanSupportCall (lpCallParams);
        return LINEERR_INVALADDRESSID;
    }
    else
    {   
        // Attempt to pass it to an address with the specified dialable address.
        if (lpCallParams->dwAddressMode == LINEADDRESSMODE_DIALABLEADDR &&
            lpCallParams->dwOrigAddressSize > 0)
        {                              
            LPTSTR lpBuff = reinterpret_cast<LPTSTR>(lpCallParams) + lpCallParams->dwOrigAddressOffset;
            TString strAddress (lpBuff, lpCallParams->dwOrigAddressSize);
            CTSPIAddressInfo* pAddr = FindAddress(strAddress.c_str());
            if (pAddr != NULL)
                return pAddr->CanSupportCall (lpCallParams);
        }
        
        // Search through ALL our addresses and see if any can support this call.
        for (int x = 0; x < GetAddressCount(); x++)
        {
            CTSPIAddressInfo* pAddr = GetAddress(x);
            if (pAddr && pAddr->CanSupportCall(lpCallParams) == false)
                return false;
        }
    }
    
    return LINEERR_INVALCALLPARAMS;
            
}// CTSPILineConnection::CanSupportCall

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::MakeCall
//
// Make a new call on our line connection.  Allocate a call appearance
// from an available address on the line.
//
LONG CTSPILineConnection::MakeCall (
DRV_REQUESTID dwRequestID,             // Asynchronous request ID
HTAPICALL htCall,                      // TAPI opaque call handle
LPHDRVCALL lphdCall,                   // return addr for TSPI call handle
LPCTSTR lpszDestAddr,                  // Address to call
DWORD dwCountryCode,                   // Country code (SP specific)
LPLINECALLPARAMS const lpCallParamsIn) // Optional call parameters
{   
	// Verify that the derived provider says MAKECALL is valid currently.
	if ((m_LineStatus.dwLineFeatures & LINEFEATURE_MAKECALL) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	CTSPIAddressInfo* pAddr = NULL;
    LONG lResult = 0L;

    // Validate the call parameters if they exist.
	LPLINECALLPARAMS lpCallParams = NULL;
    if (lpCallParamsIn)
    {                                    
		// Validate the User to user information
        if (lpCallParamsIn->dwUserUserInfoOffset > 0 &&
            GetLineDevCaps()->dwUUIMakeCallSize > 0 &&
            GetLineDevCaps()->dwUUIMakeCallSize < lpCallParamsIn->dwUserUserInfoSize)
            return LINEERR_USERUSERINFOTOOBIG;

        // Copy the call parameters into our own buffer so we keep a pointer
		// to it during the asynchronous processing of this call.
        lpCallParams = CopyCallParams(lpCallParamsIn);
        if (lpCallParams == NULL)
            return LINEERR_NOMEM;

		// Validate them against our line configuration
        lResult = GetSP()->ProcessCallParameters(this, lpCallParams);
        if (lResult)
        {
            FreeMem(lpCallParams);
            return lResult;
        }
    }
                  
	// Split the destination address out into multiple DIALINFO structures into a
	// heap-based dial string array since the request will maintain it's own copy.
	TDialStringArray arrDialInfo;

    // Verify the destination address if it exists.    
    if (lpszDestAddr && *lpszDestAddr != _T('\0'))
    {
        lResult = GetSP()->CheckDialableNumber(this, NULL, lpszDestAddr, &arrDialInfo, dwCountryCode);
        if (lResult)
        {
			// Changes by Ron Langham.  No longer return error if number is not valid.
			// This prevents from being able to dial SIP addresses.
			// If invalid, just create DIALINFO passing the values exactly as passed

			// Now store the information into a DIALINFO structure.
			DIALINFO* pDialInfo = new DIALINFO ( false, lpszDestAddr, _T(""), _T("") );
			arrDialInfo.push_back(pDialInfo);
//            FreeMem(lpCallParams);
//            return lResult;
        }
    }
    
	// Lock the line connection until we add the request to the list.
	// This is done specifically for the MAKECALL since it creates new call appearances
	// and there is a window of opportunity if multiple threads are attempting to make 
	// calls simultaneously where both will be submitted as asychronous requests.  To
	// stop this, we lock the object for updates until it is submitted.
	CEnterCode sLock (this, true);

	// If we still have the bandwidth for one more call, but have a pending MAKECALL
	// request in our queue..
	if (m_LineCaps.dwMaxNumActiveCalls-1 == m_dwConnectedCallCount && 
			 FindRequest(NULL, REQUEST_MAKECALL) != NULL)
	{
        FreeMem(lpCallParams);
		return LINEERR_RESOURCEUNAVAIL;
	}

    // Create a call appearance on a known address.
    CTSPICallAppearance* pCall = NULL;

    // If the user passes a specific call appearance in the 
    // call parameters, use it.
    if (lpCallParams)
    {
        // If they specified a specific address ID, then find the address on this
        // line and create the call appearance on the address.
        if (lpCallParams->dwAddressMode == LINEADDRESSMODE_ADDRESSID)
        {
            pAddr = GetAddress(lpCallParams->dwAddressID);
            if (pAddr == NULL)
            {
				_TSP_DTRACE(_T("%s: lineMakeCall: invalid address id <%ld>\n"), m_strName.c_str(), lpCallParams->dwAddressID);
				FreeMem(lpCallParams);
                return LINEERR_INVALADDRESSID;
            }

            // If the address currently doesn't support placing a 
			// call, then fail this attempt.
            if ((pAddr->GetAddressStatus()->dwAddressFeatures & LINEADDRFEATURE_MAKECALL) == 0)
			{
				FreeMem(lpCallParams);
				return LINEERR_CALLUNAVAIL;
			}
            
            // Create the call appearance on this address.
            pCall = pAddr->CreateCallAppearance(htCall);
        }

        // Otherwise, if they specified a dialable address, then walk through all
        // our addresses and find the matching address.
        else if (lpCallParams->dwAddressMode == LINEADDRESSMODE_DIALABLEADDR)
        {                       
            if (lpCallParams->dwOrigAddressSize > 0 && 
                lpCallParams->dwOrigAddressOffset > 0)
            {
                LPCTSTR lpszAddress = reinterpret_cast<LPCTSTR>(lpCallParams) + lpCallParams->dwOrigAddressSize;
                for (int x = 0; x < GetAddressCount(); x++)
                {
                    pAddr = GetAddress(x);
                    if (lstrcmp(pAddr->GetDialableAddress(), lpszAddress) == 0)
                    {
            			// If the address currently doesn't support placing a 
						// call, then fail this attempt.
            			if ((pAddr->GetAddressStatus()->dwAddressFeatures & LINEADDRFEATURE_MAKECALL) == 0)
						{
							FreeMem(lpCallParams);
							return LINEERR_CALLUNAVAIL;
						}

						// Otherwise create the call on this address.
                        pCall = pAddr->CreateCallAppearance(htCall);
                        break;
                    }
                }

				if (pCall == NULL)
				{
					_TSP_DTRACE(_T("%s: lineMakeCall: address <%s> does not exist\n"), m_strName.c_str(), lpszAddress);
					FreeMem(lpCallParams);
					return LINEERR_INVALADDRESS;
				}
            }
        }
    }

    // If they did not specify which call appearance to use by address, then locate one
    // which matches the specifications they desire from our service provider.
    if (pCall == NULL)
    {
        pAddr = FindAvailableAddress (lpCallParams, LINEADDRFEATURE_MAKECALL);
        if (pAddr != NULL)
            pCall = pAddr->CreateCallAppearance(htCall);
    }
   
    // If there are no more call appearances, exit.
    if (pCall == NULL)
    {
		_TSP_DTRACE(_T("%s: lineMakeCall: no address available for outgoing call\n"), m_strName.c_str());
		FreeMem(lpCallParams);
        return LINEERR_CALLUNAVAIL;
    }

    // Return the call appearance handle.
    *lphdCall = (HDRVCALL) pCall;

    // Otherwise, tell the call appearance to make a call.
    lResult = pCall->MakeCall (dwRequestID, &arrDialInfo, dwCountryCode, lpCallParams);
	if (lResult != static_cast<LONG>(dwRequestID))
	{
		// Delete the call appearance.
		pCall->GetAddressOwner()->RemoveCallAppearance(pCall);
		*lphdCall = NULL;
		FreeMem(lpCallParams);
	}
	return lResult;

}// CTSPILineConnection::MakeCall

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAddressID
//
// This method returns the address ID associated with this line
// in the specified format.
//
LONG CTSPILineConnection::GetAddressID(
LPDWORD lpdwAddressId,                 // DWORD for return address ID
DWORD dwAddressMode,                   // Address mode in lpszAddress
LPCTSTR lpszAddress,                   // Address of the specified line
DWORD dwSize)                          // Size of the above string/buffer
{
    // We don't support anything but the dialable address
    if (dwAddressMode != LINEADDRESSMODE_DIALABLEADDR)
        return LINEERR_INVALADDRESSMODE;

    // Make sure the size field is filled out ok.
    if (dwSize == 0)
        dwSize = lstrlen(lpszAddress);

    TString strAddress(lpszAddress, dwSize);

    // Walk through all the addresses on this line and see if the
    // address passed matches up.
    CTSPIAddressInfo* pFinal = NULL;
    for (int i = 0; i < GetAddressCount(); i++)
    {
        CTSPIAddressInfo* pAddr = GetAddress(i);
        if (pAddr && !lstrcmp(strAddress.c_str(), pAddr->GetDialableAddress()))
        {
            pFinal = pAddr;
            break;
        }
    }

    // Never found it? return error.
    if (pFinal == NULL)
        return LINEERR_INVALADDRESS;

    // Otherwise set the returned address id to the call appearance
    // address id.
    *lpdwAddressId = pFinal->GetAddressID();
    
    return false;

}// CTSPILineConnection::GetAddressID

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetTerminalInformation
//
// Return the current terminal modes for the specified terminal
// identifier.
//
DWORD CTSPILineConnection::GetTerminalInformation (unsigned int iTerminalID) const
{                                              
	CEnterCode sLock(this);  // Synch access to object
	if (iTerminalID < m_arrTerminalInfo.size())
	{
		TERMINALINFO* lpTermInfo = m_arrTerminalInfo[iTerminalID];
		if (lpTermInfo)
			return lpTermInfo->dwMode;
	}       
	return 0L;
	
}// CTSPILineConnection::GetTerminalInformation

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::AddTerminal
//
// Add a new terminal to the line connection.
//
int CTSPILineConnection::AddTerminal (LPCTSTR lpszName, LINETERMCAPS& Caps, DWORD dwModes)
{
    TERMINALINFO* lpTermInfo = new TERMINALINFO;
    lpTermInfo->strName = lpszName;
    MoveMemory (&lpTermInfo->Capabilities, &Caps, sizeof(LINETERMCAPS));
    lpTermInfo->dwMode = dwModes;

    // Add it to our array.
	CEnterCode sLock(this);  // Synch access to object

	try
	{
		m_arrTerminalInfo.push_back(lpTermInfo);
	}
	catch (...)
	{
		delete lpTermInfo;
		throw;
	}

    int iPos = m_arrTerminalInfo.size()-1;

    // Set the new terminal count
    m_LineCaps.dwNumTerminals = m_arrTerminalInfo.size();
    m_LineCaps.dwTerminalCapsSize = (m_LineCaps.dwNumTerminals * sizeof(LINETERMCAPS));

	// Set our new text entry size for the terminal if it exceeds the total size of
	// the biggest terminal name already in place.
	DWORD dwTextLen = ((lpTermInfo->strName.length()+1) * sizeof(wchar_t));
	if (m_LineCaps.dwTerminalTextEntrySize < dwTextLen)
		m_LineCaps.dwTerminalTextEntrySize = dwTextLen;
	m_LineCaps.dwTerminalTextSize = (m_LineCaps.dwTerminalTextEntrySize * m_LineCaps.dwNumTerminals);

	sLock.Unlock();

    // Tell all our address about the new terminal count
    for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
        m_arrAddresses[i]->OnTerminalCountChanged(true, iPos, dwModes);

	// Change the line features
    SetLineFeatures(OnLineFeaturesChanged(m_LineStatus.dwLineFeatures | LINEFEATURE_SETTERMINAL));

    // Tell TAPI our terminal information has changed.
    OnLineStatusChange (LINEDEVSTATE_TERMINALS);
    return iPos;

}// CTSPILineConnection::AddTerminal

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::RemoveTerminal
//
// Remove a terminal from our terminal list.  This will cause our
// terminal numbers to be re-ordered!
//
void CTSPILineConnection::RemoveTerminal (unsigned int iTerminalId)
{
	CEnterCode sLock(this);  // Synch access to object
    if (iTerminalId < m_arrTerminalInfo.size())
    {
        TERMINALINFO* lpInfo = m_arrTerminalInfo[iTerminalId];
        _TSP_ASSERTE(lpInfo != NULL);

        // Remove it and delete the object
        m_arrTerminalInfo.erase(m_arrTerminalInfo.begin() + iTerminalId);
        delete lpInfo;

        // Set the new terminal count
        m_LineCaps.dwNumTerminals = GetTerminalCount();
		m_LineCaps.dwTerminalCapsSize = (m_LineCaps.dwNumTerminals * sizeof(LINETERMCAPS));
		m_LineCaps.dwTerminalTextSize = m_LineCaps.dwTerminalTextEntrySize * m_LineCaps.dwNumTerminals;

		sLock.Unlock();

        // Tell all our address about the new terminal count
        for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
            m_arrAddresses[i]->OnTerminalCountChanged(false, iTerminalId, 0L);

		if (m_LineCaps.dwNumTerminals == 0)
			SetLineFeatures(OnLineFeaturesChanged(m_LineStatus.dwLineFeatures & ~LINEFEATURE_SETTERMINAL));

        // Tell TAPI our terminal information has changed.
        OnLineStatusChange (LINEDEVSTATE_TERMINALS);
    }

}// CTSPILineConnection::RemoveTerminal

///////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetTerminal
//
// This operation enables TAPI.DLL to specify to which terminal information
// related to a specified line, address, or call is to be routed.  This
// can be used while calls are in progress on the line, to allow events
// to be routed to different devices as required.
//
LONG CTSPILineConnection::SetTerminal(DRV_REQUESTID dwRequestID, 
		CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall,
		DWORD dwTerminalModes, DWORD dwTerminalID, bool bEnable)
{
    // Make sure we handle this request tyupe
    if ((GetLineDevStatus()->dwLineFeatures & LINEFEATURE_SETTERMINAL) == 0)
        return LINEERR_OPERATIONUNAVAIL;

	// Allocate a terminal request.
	RTSetTerminal* pRequest = new RTSetTerminal(this, pAddr, pCall, dwRequestID,
										dwTerminalModes, dwTerminalID, bEnable);

	// Add it into the request list associated with the line.
	if (AddAsynchRequest(pRequest))
		return static_cast<LONG>(dwRequestID);
	return LINEERR_OPERATIONFAILED;

}// CTSPILineConnection::SetTerminal

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetTerminalModes
//
// This is the function which should be called when a lineSetTerminal is
// completed by the derived service provider class.
// This stores or removes the specified terminal from the terminal modes 
// given, and then forces it to happen for any existing calls on the 
// line.
//
void CTSPILineConnection::SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal)
{
	CEnterCode sLock(this);  // Synch access to object
    if (iTerminalID < m_arrTerminalInfo.size())
    {
        TERMINALINFO* lpInfo = m_arrTerminalInfo[iTerminalID];
        _TSP_ASSERTE(lpInfo != NULL);

        // Either add the bits or mask them off based on what we are told
        // by TAPI.
        if (fRouteToTerminal)
            lpInfo->dwMode |= dwTerminalModes;
        else
            lpInfo->dwMode &= ~dwTerminalModes;

        // Notify TAPI about our device state changes
        OnLineStatusChange (LINEDEVSTATE_TERMINALS);
        
        // Force all the addresses to update the terminal list
        for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
            m_arrAddresses[i]->SetTerminalModes (iTerminalID, dwTerminalModes, fRouteToTerminal);
    }

}// CTSPILineConnection::SetTerminalModes

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetLineFeatures
//
// Used by the user to sets the current line features
//
void CTSPILineConnection::SetLineFeatures (DWORD dwFeatures)
{   
	// Make sure the capabilities structure reflects this ability.
	if (dwFeatures && (m_LineCaps.dwLineFeatures & dwFeatures) == 0)
	{
		_TSP_DTRACE(_T("%s: LINEDEVCAPS.dwLineFeatures missing 0x%lx bit\n"), m_strName.c_str(), dwFeatures);
		m_LineCaps.dwLineFeatures |= dwFeatures;	
		OnLineCapabiltiesChanged();
	}

	// Update it only if it has changed.
	if (m_LineStatus.dwLineFeatures != dwFeatures)
	{
		m_LineStatus.dwLineFeatures = dwFeatures;
		OnLineStatusChange (LINEDEVSTATE_OTHER);
	}

	_TSP_DTRACE(_T("%s: Adjusting line features with 0x%lx, New Caps=0x%lx, New Status=0x%lx\n"), m_strName.c_str(), dwFeatures, m_LineCaps.dwLineFeatures, m_LineStatus.dwLineFeatures);

}// CTSPILineConnection::SetLineFeatures

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnLineCapabilitiesChanged
//
// This function is called when the line capabilties change in the lifetime
// of the provider.
//
void CTSPILineConnection::OnLineCapabiltiesChanged()
{
	// Verify that we haven't REMOVED capabilities from the line
	// features.  If so, remove them from the status as well.
	m_LineStatus.dwLineFeatures &= m_LineCaps.dwLineFeatures;
	OnLineStatusChange (LINEDEVSTATE_CAPSCHANGE);

}// CTSPILineConnection::OnLineCapabiltiesChanged

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnLineFeaturesChanged
//
// Hook to allow a derived line to adjust the line features.
//
DWORD CTSPILineConnection::OnLineFeaturesChanged(DWORD dwLineFeatures)
{
	return dwLineFeatures;

}// CTSPILineConnection::OnLineFeaturesChanged

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetDeviceStatusFlags
//
// Sets the device status flags in the LINEDEVSTATUS structure
//
void CTSPILineConnection::SetDeviceStatusFlags (DWORD dwStatus)
{   
    DWORD dwOldStatus = m_LineStatus.dwDevStatusFlags;
	DWORD dwNotify = 0;
    m_LineStatus.dwDevStatusFlags = dwStatus;         
	
    // Send TAPI the appropriate notifications.
    if ((dwOldStatus & LINEDEVSTATUSFLAGS_CONNECTED) &&
        (dwStatus & LINEDEVSTATUSFLAGS_CONNECTED) == 0)
        dwNotify |= LINEDEVSTATE_DISCONNECTED;
    else if ((dwOldStatus & LINEDEVSTATUSFLAGS_CONNECTED) == 0 &&    
        (dwStatus & LINEDEVSTATUSFLAGS_CONNECTED))
        dwNotify |= LINEDEVSTATE_CONNECTED;
                                         
    if ((dwOldStatus & LINEDEVSTATUSFLAGS_MSGWAIT) &&
        (dwStatus & LINEDEVSTATUSFLAGS_MSGWAIT) == 0)
        dwNotify |= LINEDEVSTATE_MSGWAITOFF;
    else if ((dwOldStatus & LINEDEVSTATUSFLAGS_MSGWAIT) == 0 &&    
        (dwStatus & LINEDEVSTATUSFLAGS_MSGWAIT))
        dwNotify |= LINEDEVSTATE_MSGWAITON;

    if ((dwOldStatus & LINEDEVSTATUSFLAGS_INSERVICE) &&
        (dwStatus & LINEDEVSTATUSFLAGS_INSERVICE) == 0)
        dwNotify |= LINEDEVSTATE_OUTOFSERVICE;
    else if ((dwOldStatus & LINEDEVSTATUSFLAGS_INSERVICE) == 0 &&    
        (dwStatus & LINEDEVSTATUSFLAGS_INSERVICE))
        dwNotify |= LINEDEVSTATE_INSERVICE;

    if ((dwOldStatus & LINEDEVSTATUSFLAGS_LOCKED) &&
        (dwStatus & LINEDEVSTATUSFLAGS_LOCKED) == 0)
        dwNotify |= LINEDEVSTATE_LOCK;
    else if ((dwOldStatus & LINEDEVSTATUSFLAGS_LOCKED) == 0 &&    
        (dwStatus & LINEDEVSTATUSFLAGS_LOCKED))
        dwNotify |= LINEDEVSTATE_LOCK;

	// Force the line to recalc its feature set.
	RecalcLineFeatures(true);

	// Inform TAPI about the changes in our device status.
	OnLineStatusChange (dwNotify);

}// CTSPILineConnection::SetDeviceStatusFlags

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Forward
//
// Forward a specific or all addresses on this line to a destination
// address.
//
LONG CTSPILineConnection::Forward(
DRV_REQUESTID dwRequestId,				// Asynchronous request id
CTSPIAddressInfo* pAddr,				// Address to forward to (NULL=all)
TForwardInfoArray* parrForwardInfo,		// Array of TSPIFORWARDINFO elements
DWORD dwNumRingsNoAnswer,				// Number of rings before "no answer"
HTAPICALL htConsultCall,				// New TAPI call handle if necessary
LPHDRVCALL lphdConsultCall,				// Our return call handle if needed
LPLINECALLPARAMS lpCallParams)			// Used if creating a new call
{
    // Pass it directly onto the address specified, or onto all our addresses.
    if (pAddr != NULL)
		return pAddr->Forward (dwRequestId, parrForwardInfo, dwNumRingsNoAnswer,
							   htConsultCall, lphdConsultCall, lpCallParams);

	// Verify that a FORWARD is available on this line right now.
	if ((m_LineStatus.dwLineFeatures & 
			(LINEFEATURE_FORWARD|LINEFEATURE_FORWARDFWD|LINEFEATURE_FORWARDDND)) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// Otherwise we are forwarding *all* addresses on this line.
	// Synch access to object
	CEnterCode sLock(this);  
    
    // Run through all the addresses and see if they can forward given the 
    // forwarding instructions.  This function should NOT insert a request!
	int iCount = m_arrAddresses.size();
    for (int i = 0; i < iCount; i++)
    {
        LONG lResult = m_arrAddresses[i]->CanForward(parrForwardInfo, &dwNumRingsNoAnswer, iCount);
		if (lResult != 0) return lResult;
    }

	// Unlock the object
	sLock.Unlock();

	// Create our request object to map this forwarding request.
	RTForward* pRequest = new RTForward(this, NULL, dwRequestId, parrForwardInfo, dwNumRingsNoAnswer);

    // Push the request onto the list.
    if (AddAsynchRequest(pRequest))
		return static_cast<LONG>(dwRequestId);
    return LINEERR_OPERATIONFAILED;

}// CTSPILineConnection::Forward

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetStatusMessages
//
// This operation enables the TAPI DLL to specify which notification 
// messages the Service Provider should generate for events related to 
// status changes for the specified line or any of its addresses.
// 
LONG CTSPILineConnection::SetStatusMessages(DWORD dwLineStates, DWORD dwAddressStates)
{
	// Set our new states based on what our LINEDEVCAPS says we can support.
    m_dwLineStates = (dwLineStates & m_LineCaps.dwLineStates);
    
    // Tell all the addresses which states to send.
    for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
        m_arrAddresses[i]->SetStatusMessages (dwAddressStates);

    return false;
    
}// CTSPILineConnection::SetStatusMessages

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::ConditionalMediaDetection
//
// This method is invoked by TAPI.DLL when the application requests a
// line open using the LINEMAPPER.  This method will check the 
// requested media modes and return an acknowledgement based on whether 
// we can monitor all the requested modes.
// 
LONG CTSPILineConnection::ConditionalMediaDetection(DWORD dwMediaModes, LPLINECALLPARAMS const lpCallParams)
{   
    // We MUST have call parameters (TAPI should always pass these).
    if (lpCallParams == NULL)
        return LINEERR_INVALCALLPARAMS;
    
    // Copy the call params into our own private buffer so we may alter them.
    LPLINECALLPARAMS lpMyCallParams = CopyCallParams(lpCallParams);
    if (lpMyCallParams == NULL)
        return LINEERR_NOMEM;
    
    // Allow searching for ANY address.
    if (lpMyCallParams->dwAddressMode == LINEADDRESSMODE_ADDRESSID)
        lpMyCallParams->dwAddressMode = 0;        
    lpMyCallParams->dwMediaMode = dwMediaModes;
        
    // Verify the call parameters for the line/address given.
    LONG lResult = GetSP()->ProcessCallParameters(this, lpMyCallParams);
    FreeMem(lpMyCallParams);
    return lResult;
    
}// CTSPILineConnection::ConditionalMediaDetection

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::ValidateMediaControlList
//
// This method is called by the lineSetMediaControl to validate that
// the media control parameters are ok for this line device.
//
LONG CTSPILineConnection::ValidateMediaControlList(TSPIMEDIACONTROL* lpMediaControl) const
{
    // If media control is not available, then exit.
    if ((m_LineCaps.dwDevCapFlags & LINEDEVCAPFLAGS_MEDIACONTROL) == 0)                                
        return LINEERR_OPERATIONUNAVAIL;
                                
    // Validate the media control elements.
    if (lpMediaControl->arrDigits.size() > m_LineCaps.dwMedCtlDigitMaxListSize)
        return LINEERR_INVALDIGITLIST;

	unsigned int i;
    for (i = 0; i< lpMediaControl->arrDigits.size(); i++)
    {
        LPLINEMEDIACONTROLDIGIT lpDigit = reinterpret_cast<LPLINEMEDIACONTROLDIGIT>(lpMediaControl->arrDigits[i]);
        if ((lpDigit->dwDigitModes & m_LineCaps.dwMonitorDigitModes) != lpDigit->dwDigitModes)
            return LINEERR_INVALDIGITLIST;

        char cDigit = LOBYTE(LOWORD(lpDigit->dwDigit));        
        if (lpDigit->dwDigitModes & (LINEDIGITMODE_DTMF | LINEDIGITMODE_DTMFEND))
        {
            if (strchr ("0123456789ABCD*#", cDigit) == NULL)
                return LINEERR_INVALDIGITLIST;
        }
        else if (lpDigit->dwDigitModes & LINEDIGITMODE_PULSE)
        {
            if (strchr ("0123456789", cDigit) == NULL)
                return LINEERR_INVALDIGITLIST;
        }
    }

    if (lpMediaControl->arrMedia.size() > m_LineCaps.dwMedCtlMediaMaxListSize)
        return LINEERR_INVALMEDIALIST;
        
    for (i = 0; i <lpMediaControl->arrMedia.size(); i++)
    {
        LPLINEMEDIACONTROLMEDIA lpMedia = reinterpret_cast<LPLINEMEDIACONTROLMEDIA>(lpMediaControl->arrMedia[i]);
        if ((lpMedia->dwMediaModes & m_LineCaps.dwMediaModes) != lpMedia->dwMediaModes)
            return LINEERR_INVALMEDIALIST;
    }
    
    if (lpMediaControl->arrTones.size() > m_LineCaps.dwMedCtlToneMaxListSize)
        return LINEERR_INVALTONELIST;
        
    for (i = 0; i < lpMediaControl->arrTones.size(); i++)
    {
        LPLINEMEDIACONTROLTONE lpTone = reinterpret_cast<LPLINEMEDIACONTROLTONE>(lpMediaControl->arrTones[i]);
        unsigned int iFreqCount = 0;
        if (lpTone->dwFrequency1 > 0)
            ++iFreqCount;
        if (lpTone->dwFrequency2 > 0)
            ++iFreqCount;
        if (lpTone->dwFrequency3 > 0)
            ++iFreqCount;
        if (iFreqCount > m_LineCaps.dwMonitorToneMaxNumFreq)
            return LINEERR_INVALTONELIST;
    }
    
    if (lpMediaControl->arrCallStates.size() > m_LineCaps.dwMedCtlCallStateMaxListSize)
        return LINEERR_INVALCALLSTATELIST;

    // Alls ok.
    return false;
    
}// CTSPILineConnection::ValidateMediaControlList

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetMediaControl
//
// This function enables and disables control actions on the media stream 
// associated with this line and all addresses/calls present here. Media control 
// actions can be triggered by the detection of specified digits, media modes, 
// custom tones, and call states.  The new specified media controls replace 
// all the ones that were in effect for this line, address, or call prior 
// to this request.
//
void CTSPILineConnection::SetMediaControl (TSPIMEDIACONTROL* lpMediaControl)
{   
    // We don't need to store this at the LINE level - 
	// since addresses are static, we can simply pass it through them.
    for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
        m_arrAddresses[i]->SetMediaControl(lpMediaControl);
    
}// CTSPILineConnection::SetMediaControl

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnRingDetected
//
// This method should be called by the service provider worker code
// when an incoming ring is detected on a line.
//
void CTSPILineConnection::OnRingDetected (DWORD dwRingMode, bool fFirstRing)
{                                                                      
    // If our ring count array is empty, then grab the total number of
    // ring modes supported and add an entry for each.
    if (m_arrRingCounts.size() < m_LineCaps.dwRingModes)
    {
        for (unsigned int i = 0; i < m_LineCaps.dwRingModes; i++)
            m_arrRingCounts.push_back(0);
    }

	// Ring mode must be 1 to LineDevCaps.dwRingModes per TAPI spec. (3.043)
	_TSP_ASSERTE(dwRingMode > 0 && dwRingMode <= m_arrRingCounts.size());

    // Grab the current ring count.
    UINT uiRingCount = (fFirstRing) ? 1 : m_arrRingCounts[dwRingMode-1]+1;
    m_arrRingCounts[dwRingMode-1] = uiRingCount;

    // Notify TAPI about the ring.
    OnLineStatusChange (LINEDEVSTATE_RINGING, dwRingMode, static_cast<DWORD>(uiRingCount));

}// CTSPILineConnection::OnRingDetected

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnRequestComplete
//
// This virtual method is called when an outstanding request on this
// line device has completed.  The return code indicates the success
// or failure of the request.  Note that the request will filter to
// the address and caller where available.
//
void CTSPILineConnection::OnRequestComplete(CTSPIRequest* pReq, LONG lResult)
{                                     
    // If the request failed, ignore it.
	if (lResult != 0)
		return;

	// Otherwise see if we need to process it.
    switch (pReq->GetCommand())
	{
		// On a set terminal request, if it is successful, then go ahead and set the
		// terminal identifiers up inside our class.  This information can then be
		// retrieved by TAPI through the GetAddressStatus/Caps methods.
		case REQUEST_SETTERMINAL:
		{
			RTSetTerminal* pTermStruct = dynamic_cast<RTSetTerminal*>(pReq);
			if (pTermStruct->GetLine() != NULL)
				SetTerminalModes (pTermStruct->GetTerminalID(), 
							pTermStruct->GetTerminalModes(), 
							pTermStruct->Enable());
			break;
		} 

		// If this is a forwarding request, and there is no address information (ie: forward ALL
		// addresses on the line), then route the request complete to each address so it will 
		// store the forwarding information.
		case REQUEST_FORWARD:
		{   
			if (pReq->GetAddressInfo() == NULL)
			{   
				CEnterCode Key (this);
				for (unsigned int i = 0; i < m_arrAddresses.size(); i++)
					m_arrAddresses[i]->OnRequestComplete (pReq, lResult);
			}            
			break;
		}
    
		// If this is a COMPLETION request then, if it was successful, mark the completion
		// request and data filled in by the service provider.
		case REQUEST_COMPLETECALL:
		{   
			// Copy the request over to a new "storable" request.
			RTCompleteCall* pRequest = dynamic_cast<RTCompleteCall*>(pReq);
			RTCompleteCall* pWait = new RTCompleteCall(*pRequest);

			// Add this new waiting request to our completion list.
			CEnterCode Key (this);
			try
			{
				m_lstCompletions.push_back(pWait);
			}
			catch(...)
			{
				delete pWait;
				throw;
			}
			Key.Unlock();

			// Note that the number of completions has changed.
			OnLineStatusChange (LINEDEVSTATE_NUMCOMPLETIONS);
			break;
		}
    
		// If this is a request to change the line device status bits and it completed
		// successfully then manage it.
		case REQUEST_SETDEVSTATUS:
		{
			RTSetLineDevStatus* pRequest = dynamic_cast<RTSetLineDevStatus*>(pReq);
			DWORD dwFlags = GetLineDevStatus()->dwDevStatusFlags;
			if (pRequest->TurnOnBits())
				dwFlags |= pRequest->GetStatusBitsToChange();
			else
				dwFlags &= ~pRequest->GetStatusBitsToChange();
			SetDeviceStatusFlags(dwFlags);
			break;
		}

		// If this is an UNCOMPLETE call request which completed successfully, then
		// remove the request from the list.
		case REQUEST_UNCOMPLETECALL:
		{
			RTCompleteCall* pComplete = dynamic_cast<RTUncompleteCall*>(pReq)->GetRTCompleteCall();
			RemoveCallCompletionRequest(pComplete->GetCompletionID(), false);
			break;
		}

		// If this is a lineSetMediaControl event, then store the new MediaControl
		// information in the line (and all of it's addresses).
		case REQUEST_MEDIACONTROL:
		{
			RTSetMediaControl* pMC = dynamic_cast<RTSetMediaControl*>(pReq);
			if (pMC->GetAddress() == NULL && pMC->GetCall() == NULL)
				SetMediaControl(pMC->GetMediaControlInfo());
			break;
		}

		// Default
		default:
			break;
	}

}// CTSPILineConnection::OnRequestComplete 

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallCompletionRequest
//
// Return the RTCompleteCall ptr associated with a completion ID.
//
RTCompleteCall* CTSPILineConnection::FindCallCompletionRequest (DWORD dwSwitchInfo, LPCTSTR pszSwitchInfo) const
{       
	CEnterCode sLock(this);  // Synch access to object
    for (TCompletionList::const_iterator pPos = m_lstCompletions.begin(); 
		 pPos != m_lstCompletions.end(); ++pPos)
    {
        RTCompleteCall* pRequest = (*pPos);
        if (pRequest->GetNumericIdentifier() == dwSwitchInfo &&
            (pszSwitchInfo == NULL || 
			 !lstrcmp(pRequest->GetStringIdentifier(), pszSwitchInfo)))
            return pRequest;
    } 
    return NULL;

}// CTSPILineConnection::FindCallCompletionRequest

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallCompletionRequest
//
// Locate a call completion request for a specific call appearance.
// Note that this will only locate call completions for CAMPed requests.
//
RTCompleteCall* CTSPILineConnection::FindCallCompletionRequest(CTSPICallAppearance* pCall) const
{
	CEnterCode sLock(this);  // Synch access to object
    for (TCompletionList::const_iterator pPos = m_lstCompletions.begin(); 
		 pPos != m_lstCompletions.end(); ++pPos)
    {
        RTCompleteCall* pRequest = (*pPos);
		if (pRequest->GetCompletionMode() == LINECALLCOMPLMODE_CAMPON &&
			pRequest->GetCallInfo() == pCall)
        	return pRequest;
    } 
    return NULL;

}// CTSPILineConnection::FindCallCompletionRequest

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallCompletionRequest
//
// Locate a call completion request based on a completion ID passed
// back to TAPI.
//
RTCompleteCall* CTSPILineConnection::FindCallCompletionRequest(DWORD dwCompletionID) const
{
	CEnterCode sLock(this);  // Synch access to object
    for (TCompletionList::const_iterator pPos = m_lstCompletions.begin(); 
		 pPos != m_lstCompletions.end(); ++pPos)
    {
        RTCompleteCall* pRequest = (*pPos);
		if (pRequest->GetCompletionID() == dwCompletionID)
			return pRequest;
    } 
    return NULL;

}// CTSPILineConnection::FindCallCompletionRequest

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::RemoveCallCompletionRequest
//
// Remove an existing call completion request from our array.  This
// is called by the "OnRequestComplete"
//
void CTSPILineConnection::RemoveCallCompletionRequest(DWORD dwCompletionID, bool fNotifyTAPI)
{                                
	CEnterCode sLock(this);  // Synch access to object

    // Locate the completion ID and delete it from our list.                   
	TCompletionList::iterator pPos;
    for (pPos = m_lstCompletions.begin(); 
		 pPos != m_lstCompletions.end(); ++pPos)
    {                                 
        RTCompleteCall* pRequest = (*pPos);
		if (pRequest->GetCompletionID() == dwCompletionID)
        {
            m_lstCompletions.remove((*pPos));
			pRequest->DecRef();
            break;
        }
    } 

	// If we didn't find any completion request, then ignore.    
    if (pPos == m_lstCompletions.end())
    	return;
    
    // If we are to notify TAPI, then do so.  This should only happen 
    // when the completion is canceled by the derived class (i.e. the hardware
    // canceled the request.
    if (fNotifyTAPI && GetNegotiatedVersion() >= TAPIVER_14)
        OnLineStatusChange (LINEDEVSTATE_COMPLCANCEL, dwCompletionID);
    OnLineStatusChange (LINEDEVSTATE_NUMCOMPLETIONS);
    
}// CTSPILineConnection::RemoveCallCompletionRequest

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::UncompleteCall
//
// Cancel a call completion request from TAPI.
//
LONG CTSPILineConnection::UncompleteCall (DRV_REQUESTID dwRequestID, DWORD dwCompletionID)
{
    // Make sure the completion ID is valid.  
    RTCompleteCall* pCC = FindCallCompletionRequest(dwCompletionID);
    if (pCC == NULL)
        return LINEERR_INVALCOMPLETIONID;

	// Build a new request from the original
	RTUncompleteCall* pRequest = new RTUncompleteCall(this, dwRequestID, pCC);

    // Submit the request to the worker code to uncode the actual line device.
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;
        
}// CTSPILineConnection::UncompleteCall

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnCallDeleted
//
// A call is being deleted from some address our line owns.
//
void CTSPILineConnection::OnCallDeleted(CTSPICallAppearance* /*pCall*/)
{
	/* Do nothing */

}// CTSPILineConnection::OnCallDeleted

//////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetupConference
//
// Sets up a conference call for the addition of the third party. 
//
LONG CTSPILineConnection::SetupConference(
DRV_REQUESTID dwRequestID,       // Asynch. req. id
CTSPICallAppearance* pCall,      // First party of the conference call   
HTAPICALL htConfCall,            // New conference call TAPI handle
LPHDRVCALL lphdConfCall,         // Returning call handle
HTAPICALL htConsultCall,         // Call to conference in
LPHDRVCALL lphdConsultCall,      // Returning call handle
DWORD dwNumParties,              // Number of parties expected.
LPLINECALLPARAMS const lpCallParamsIn) // Line parameters
{
    // We need an address to create the conference call onto.  If we got
    // a valid call handle to start the call relationship with, then use
    // it as the starting address.
    CTSPIAddressInfo* pAddr = (pCall != NULL) ?
		pCall->GetAddressOwner() : (lpCallParamsIn != NULL) ?
			FindAvailableAddress(lpCallParamsIn) : GetAddress(0);

    // If we were passed line parameters, verify them
	LPLINECALLPARAMS lpCallParams = NULL;
    if (lpCallParamsIn)
    {
        // Copy the call parameters into our own buffer so we keep a pointer
		// to it during the asynchronous processing of this call.
        lpCallParams = CopyCallParams(lpCallParamsIn);
        if (lpCallParams == NULL)
            return LINEERR_NOMEM;
        
        // Process the call parameters
        LONG lResult = GetSP()->ProcessCallParameters(this, lpCallParams);
        if (lResult)
        {
            FreeMem(lpCallParams);
            return lResult;          
        }
    }

    // Let the address do the work of setting up the conference.
    LONG lResult = pAddr->SetupConference(dwRequestID, pCall, htConfCall, lphdConfCall, 
					htConsultCall, lphdConsultCall, dwNumParties, lpCallParams);
	if (lResult != static_cast<LONG>(dwRequestID))
		FreeMem(lpCallParams);
	return lResult;

}// CTSPILineConnection::SetupConference

//////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetupTransfer
//
// This function sets up a call for transfer to a destination address.
// A new call handle is created which represents the destination
// address.
//
LONG CTSPILineConnection::SetupTransfer(
DRV_REQUESTID dwRequestID,          // Asynch. request id
CTSPICallAppearance *pCall,         // Call appearance to transfer
HTAPICALL htConsultCall,            // Consultant call to create
LPHDRVCALL lphdConsultCall,         // Return handle for call to create   
LPLINECALLPARAMS const lpCallParamsIn)   // Calling parameters
{
    // If there are call parameters, then force the derived class to
    // deal with them since there are device specific flags in the
    // set.
	LPLINECALLPARAMS lpCallParams = NULL;
    if (lpCallParamsIn)
    {
        // Copy the call parameters into our own buffer so we keep a pointer
		// to it during the asynchronous processing of this call.
        lpCallParams = CopyCallParams(lpCallParamsIn);
        if (lpCallParams == NULL)
            return LINEERR_NOMEM;
        
        // Process the call parameters
        LONG lResult = GetSP()->ProcessCallParameters(pCall, lpCallParams);
        if (lResult)
        {
            FreeMem(lpCallParams);
            return lResult;          
        }
    }

    // Determine which address to use based on our CALLPARAMS.  If they indicate
    // a different address, then this is a cross-address transfer.
    CTSPIAddressInfo* pAddr = pCall->GetAddressOwner();
    if (lpCallParams)
    {
        if (lpCallParams->dwAddressMode == LINEADDRESSMODE_ADDRESSID)
        {
            pAddr = GetAddress(lpCallParams->dwAddressID);
            if (pAddr == NULL)
            {
				FreeMem(lpCallParams);
                return LINEERR_INVALCALLPARAMS;
            }
        }            
    }
    
    // Now tell the address to setup the transfer.
    LONG lResult = pAddr->SetupTransfer (dwRequestID, pCall, htConsultCall, lphdConsultCall, lpCallParams);
	if (lResult != static_cast<LONG>(dwRequestID))
		FreeMem(lpCallParams);
	return lResult;

}// CTSPILineConnection::SetupTransfer

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::IsConferenceAvailable
//
// This determines whether there is a conference call active on the
// same line/address as the call passed.
//
bool CTSPILineConnection::IsConferenceAvailable(CTSPICallAppearance* pCall)
{   
    // Check on the same address.                                        
    CTSPIAddressInfo* pAddr = pCall->GetAddressOwner();
	CEnterCode KeyAddr (pAddr);  // Synch access to object

	int iCall;
    for (iCall = 0; iCall < pAddr->GetCallCount(); iCall++)
    {
        CTSPICallAppearance* pThisCall = pAddr->GetCallInfo(iCall);
        if (pThisCall != NULL && pThisCall != pCall && 
			pThisCall->GetCallType() == CTSPICallAppearance::Conference &&
			(pThisCall->GetCallState() & (LINECALLSTATE_IDLE | LINECALLSTATE_DISCONNECTED)) == 0)
            return true;
    }
    KeyAddr.Unlock();

    // None found there, check on the same line if cross-address conferencing is
    // available.
    if (m_LineCaps.dwDevCapFlags & LINEDEVCAPFLAGS_CROSSADDRCONF)
    {
		CEnterCode sLock(this);  // Synch access to object
        for (int iAddr = 0; iAddr < GetAddressCount(); iAddr++)
        {
            CTSPIAddressInfo* pThisAddr = GetAddress(iAddr);
            if (pThisAddr != pAddr)
            {
				CEnterCode KeyAddr (pThisAddr);
                for (iCall = 0; iCall < pThisAddr->GetCallCount(); iCall++)
                {
                    CTSPICallAppearance* pThisCall = pThisAddr->GetCallInfo(iCall);
                    if (pThisCall != NULL && 
						pThisCall->GetCallType() == CTSPICallAppearance::Conference &&
						(pThisCall->GetCallState() & (LINECALLSTATE_IDLE | LINECALLSTATE_DISCONNECTED)) == 0)
                        return true;
                }
            }                                       
        }
    }
    
    // No conference found.
    return false;
    
}// CTSPILineConnection::IsConferenceAvailable

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::IsTransferConsultAvailable
//
// This determines whether there is a consultant call or some other
// call which we could transfer to right now.
//
bool CTSPILineConnection::IsTransferConsultAvailable(CTSPICallAppearance* pCall)
{   
    // See if we have an attached consultation call which can be used for
    // transfer.  This would have been created through the SetupTransfer API.
    CTSPICallAppearance* pThisCall = pCall->GetAttachedCall();
    if (pThisCall != NULL)
    {
		return ((pThisCall->GetCallState() & (LINECALLSTATE_CONNECTED |
						LINECALLSTATE_RINGBACK |
						LINECALLSTATE_BUSY |
						LINECALLSTATE_PROCEEDING)) != 0);
    }               
    
    // If the address supports creation of a new consultation call, then search 
    // our address for calls in the appropriate state.  This may or may not have been
    // created with the SetupTransfer API.
    CTSPIAddressInfo* pAddr = pCall->GetAddressOwner();
    if ((pAddr->GetAddressCaps()->dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERMAKE) == 0)
        return false;

	// Walk through all the addresses checking for calls on addresses which 
	// support creation of TRANSFER consultation calls and are in the proper
	// state.
    for (int iAddr = 0; iAddr < GetAddressCount(); iAddr++)
	{
		CTSPIAddressInfo* pAddr = GetAddress(iAddr);
		if ((pAddr->GetAddressCaps()->dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERMAKE) != 0)
		{
			// Synch access to object - don't allow system lockup just for this.
			CEnterCode KeyAddr(pAddr, FALSE);
			if (KeyAddr.Lock(100))
			{
				for (int iCall = 0; iCall < pAddr->GetCallCount(); iCall++)
				{
					CTSPICallAppearance* pThisCall = pAddr->GetCallInfo(iCall);
					if (pThisCall != pCall && 
						((pThisCall->GetCallState() & 
								(LINECALLSTATE_CONNECTED | 
								 LINECALLSTATE_RINGBACK |
								 LINECALLSTATE_BUSY |
								 LINECALLSTATE_PROCEEDING)) != 0))
						return true;
				}            
				KeyAddr.Unlock();
			}
		}
	}

    return false;
        
}// CTSPILineConnection::IsTransferConsultAvailable

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallByState
//
// Run through all our addresses and look for a call in the specified
// state.
//
CTSPICallAppearance* CTSPILineConnection::FindCallByState(DWORD dwState) const
{                
    for (int j = 0; j < GetAddressCount(); j++)
    {
        CTSPIAddressInfo* pAddr = GetAddress(j);
		CEnterCode AddrLock(pAddr);
        for (int i = 0; i < pAddr->GetCallCount(); i++)
        {
            CTSPICallAppearance* pThisCall = pAddr->GetCallInfo(i);
            if (pThisCall != NULL && 
				(dwState & pThisCall->GetCallState()))
                return pThisCall;
        }            
    }       
    return NULL;

}// CTSPILineConnection::FindCallByState

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallByType
//
// Run through all our addresses and look for the first call of the
// specified call type
//
CTSPICallAppearance* CTSPILineConnection::FindCallByType(int iType) const
{
    for (int j = 0; j < GetAddressCount(); j++)
    {
        CTSPIAddressInfo* pAddr = GetAddress(j);
		CEnterCode AddrLock(pAddr);
        for (int i = 0; i < pAddr->GetCallCount(); i++)
        {
            CTSPICallAppearance* pCall = pAddr->GetCallInfo(i);
            if (pCall != NULL && pCall->GetCallType() == iType &&
				pCall->GetCallState() != LINECALLSTATE_IDLE)
                return pCall;
        }            
		AddrLock.Unlock();
    }       
    return NULL;

}// CTSPILineConnection::FindCallByType

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FindCallByCallID
//
// Run through all our addresses and look for the first call with the
// specified call id.
//
CTSPICallAppearance* CTSPILineConnection::FindCallByCallID(DWORD dwCallID) const
{
    for (int j = 0; j < GetAddressCount(); j++)
    {
        CTSPIAddressInfo* pAddr = GetAddress(j);
		CEnterCode AddrLock(pAddr);
        for (int i = 0; i < pAddr->GetCallCount(); i++)
        {
            CTSPICallAppearance* pCall = pAddr->GetCallInfo(i);
            if (pCall != NULL && pCall->GetCallID() == dwCallID)
                return pCall;
        }            
    }       
    return NULL;

}// CTSPILineConnection::FindCallByCallID

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateUIDialog
//
// Create a new dialog on a user thread.
//
HTAPIDIALOGINSTANCE CTSPILineConnection::CreateUIDialog (
DRV_REQUESTID dwRequestID,				// Request ID to work with
LPVOID lpItemData,						// Item data passed back with Generic
LPVOID lpParams,						// Parameter block
DWORD dwSize,							// Size of param block
LPCTSTR lpszUIDLLName/*=NULL*/)			// Different UI dll?
{
	// If no UI dll was passed, use our own DLL as the UI dll.
	TCHAR szPath[_MAX_PATH+1];
	if (lpszUIDLLName == NULL || *lpszUIDLLName == _T('\0'))
	{
		GetSystemDirectory (szPath, _MAX_PATH);
		if (szPath[lstrlen(szPath)-1] != _T('\\'))
			lstrcat(szPath, _T("\\"));
		lstrcat(szPath, GetSP()->GetUIManager());
		lpszUIDLLName = szPath;
	}

	// Allocate a new UI structure
	LINEUIDIALOG* pLineDlg = new LINEUIDIALOG;

	// Ask TAPI for a UI dialog event.
	USES_CONVERSION;
	TUISPICREATEDIALOGINSTANCEPARAMS Params;
	Params.dwRequestID = dwRequestID;
	Params.hdDlgInst = (HDRVDIALOGINSTANCE) pLineDlg;
	Params.htDlgInst = NULL;
	Params.lpszUIDLLName = T2W(const_cast<LPTSTR>(lpszUIDLLName));
	Params.lpParams = lpParams;
	Params.dwSize = dwSize;

	Send_TAPI_Event(NULL, LINE_CREATEDIALOGINSTANCE, reinterpret_cast<DWORD>(&Params), 0L, 0L);

	if (Params.htDlgInst == NULL)
	{
		_TSP_DTRACE(_T("%s: Failed to create UI dialog for request ID 0x%lx\n"), m_strName.c_str(), dwRequestID);
		delete pLineDlg;
	}
	else
	{
		_TSP_DTRACE(_T("%s: New UI dialog created TAPI=0x%lx, SP=0x%lx\n"), m_strName.c_str(), Params.htDlgInst, Params.hdDlgInst);
		pLineDlg->pLineOwner = this;
		pLineDlg->dwRequestID = dwRequestID;
		pLineDlg->htDlgInstance = Params.htDlgInst;
		pLineDlg->lpvItemData = lpItemData;

		CEnterCode sLock(this);  // Synch access to object
		try
		{
			m_lstUIDialogs.push_back(pLineDlg);
		}
		catch(...)
		{
			delete pLineDlg;
			throw;
		}
	}

	return Params.htDlgInst;

}// CTSPILineConnection::CreateUIDialog

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetUIDialogItem
//
// Retrieve a UI dialog instance item data handle for a given TAPI
// dialog instance
//
LPVOID CTSPILineConnection::GetUIDialogItem(HTAPIDIALOGINSTANCE htDlgInst) const
{
	CEnterCode sLock(this);  // Synch access to object
	for (TUIList::const_iterator iPos = m_lstUIDialogs.begin(); 
		 iPos != m_lstUIDialogs.end(); iPos++)
	{
		if ((*iPos)->htDlgInstance == htDlgInst)
			return (*iPos)->lpvItemData;
	}
	return NULL;

}// CTSPILineConnection::GetUIDialog

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::FreeDialogInstance
//
// Destroy a user-interface dialog for this line connection.  In general,
// don't call this function from user-level code.
//
LONG CTSPILineConnection::FreeDialogInstance(HTAPIDIALOGINSTANCE htDlgInst)
{
	CEnterCode sLock(this);  // Synch access to object
	for (TUIList::iterator pos = m_lstUIDialogs.begin();
		pos != m_lstUIDialogs.end(); ++pos)
	{
		if ((*pos)->htDlgInstance == htDlgInst)
		{
			// Remove it from our array.
			LINEUIDIALOG* pLineDlg = (*pos);
			m_lstUIDialogs.erase(pos);

			// Given the line a chance to know that the thread on the client
			// was destroyed.
			OnUIDialogClosed(pLineDlg->htDlgInstance, pLineDlg->lpvItemData);
			delete pLineDlg;
			break;
		}
	}
	return false;

}// CTSPILineConnection::FreeDialogInstance

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnUIDialogClosed
//
// Called when the UI dialog is being free'd by TAPI. Can be overridden
// to delete the item data, cancel a pending request, etc.
//
void CTSPILineConnection::OnUIDialogClosed(HTAPIDIALOGINSTANCE /*htDlgInst*/, LPVOID /*lpItemData*/)
{
}// CTSPILineConnection::OnUIDialogClosed

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetLineDevStatus
//
// Change the line device status according to what TAPI
// has requested.  Derived provider should use the "SetDeviceStatusFlags"
// to actually set the flags.
//
LONG CTSPILineConnection::SetLineDevStatus (DRV_REQUESTID dwRequestID,
						DWORD dwStatusToChange, bool fSet)
{
	// Verify that we can execute this request right now.
    if ((GetLineDevStatus()->dwLineFeatures & LINEFEATURE_SETDEVSTATUS) == 0)
        return LINEERR_OPERATIONUNAVAIL;

	// If this isn't one of the supported status bits, error out.
	if ((dwStatusToChange & m_LineCaps.dwSettableDevStatus) == 0)
		return LINEERR_INVALPARAM;

	// Create the request object to mirror this event.
	RTSetLineDevStatus* pRequest = new RTSetLineDevStatus(this, dwRequestID, dwStatusToChange, fSet);

	// Submit the request and let the derived provider set the appropriate bits
	// using the "SetDeviceStatusFlags" when it is finished.
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestID);
    return LINEERR_OPERATIONFAILED;
 
}// CTSPILineConnection::SetLineDevStatus

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetRingMode
//
// Set the ring mode for this line.
//
void CTSPILineConnection::SetRingMode (DWORD dwRingMode)
{
	if (m_LineStatus.dwRingMode != dwRingMode)
	{
		m_LineStatus.dwRingMode = dwRingMode;
		OnLineStatusChange (LINEDEVSTATE_OTHER);
	}

}// CTSPILineConnection::SetRingMode

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetBatteryLevel
//
// Set the battery level for this line connection
//
void CTSPILineConnection::SetBatteryLevel (DWORD dwBattery)
{
    if (dwBattery > 0xffff)
        dwBattery = 0xffff;
	if (m_LineStatus.dwBatteryLevel != dwBattery)
	{
		m_LineStatus.dwBatteryLevel = dwBattery;
		OnLineStatusChange (LINEDEVSTATE_BATTERY);
	}

}// CTSPILineConnection::SetBatteryLevel

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetSignalLevel
//
// Set the signal level for the line connection
//
void CTSPILineConnection::SetSignalLevel (DWORD dwSignal)
{
    if (dwSignal > 0xffff)
        dwSignal = 0xffff;
	if (m_LineStatus.dwSignalLevel != dwSignal)
	{
		m_LineStatus.dwSignalLevel = dwSignal;
		OnLineStatusChange (LINEDEVSTATE_SIGNAL);
	}

}// CTSPILineConnection::SetSignalLevel

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetRoamMode
//
// Set the roaming mode for the line
//
void CTSPILineConnection::SetRoamMode (DWORD dwRoamMode)
{
	if (m_LineStatus.dwRoamMode != dwRoamMode)
	{
		m_LineStatus.dwRoamMode = dwRoamMode;
		OnLineStatusChange (LINEDEVSTATE_ROAMMODE);
	}

}// CTSPILineConnection::SetRoamMode

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetDefaultMediaDetection
//
// This sets our current set of media mode detection being done
// on this line.  The new modes should be tested for on any offering calls.
//
LONG CTSPILineConnection::SetDefaultMediaDetection(DWORD dwMediaModes)
{                                                
    // Validate the media modes 
    if ((dwMediaModes & m_LineCaps.dwMediaModes) != dwMediaModes)
        return LINEERR_INVALMEDIAMODE;

	// Set our new media mode into place.
    m_dwLineMediaModes = dwMediaModes; 

	// If the media modes have changed, then start a thread which will
	// wait a minute (for the line to be completely open) and then re-offer
	// all the calls to TAPI which are still in our call list.
	if (dwMediaModes > 0)
	{
		// Attempt to notify TAPI about all the calls now. If this fails, spawn
		// a thread and retry until it is successful.
		if (OfferCallsToTAPISrv() == false)
		{
			UINT uiThread;
			_beginthreadex(NULL, 0, tsplib_OfferCallThread, 
							static_cast<void*>(this), 0, &uiThread);
		}
	}

    return false;

}// CTSPILineConnection::SetDefaultMediaDetection

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OfferCallsToTAPISrv
//
// This offers existing call objects back to TAPI once the line has been
// opened by an owner.
//
// The optimal place for this would actually be in the SetDefaultMediaDetection 
// code above. Unfortunately, TAPI 2.1 changed the way TAPISRV opens lines and
// now TAPISRV is not yet done opening the line (the line handle hasn't been 
// moved into it's "open" structure), and so the LINE_NEWCALL fails because 
// TAPI's htLine handle isn't recognized by TAPISRV - even though it sent it to us 
// on a previous function call (TSPI_lineOpen). This was reported as a bug, 
// but according to Microsoft this is the way it will be from now on so... 
// we spawn a thread to get of TAPI's worker thread and wait for a minute.
//
bool CTSPILineConnection::OfferCallsToTAPISrv()
{
	bool fAllOK = true;

	// Now walk the call list and see if we have any calls on this
	// line which are in the specified media mode BUT don't have TAPI
	// call handles.  This happens when TAPI closes the call but later
	// gets a client which owns the line again.
	for (int iAddress = 0; iAddress < GetAddressCount(); iAddress++)
	{
		CTSPIAddressInfo* pAddr = GetAddress(iAddress);
		if (pAddr->GetAddressCaps()->dwAddressSharing == LINEADDRESSSHARING_MONITORED)
			continue;

		CEnterCode sAddrLock(pAddr);
		for (int iCall = 0; iCall < pAddr->GetCallCount(); iCall++)
		{
			// Offer the call if we still have a valid object, we 
			// don't have a TAPI call handle for it, and the media mode
			// of the call is something that TAPI cares about (otherwise it
			// will turn around and drop it so don't bother to waste the CPU).
			CTSPICallAppearance* pCall = pAddr->GetCallInfo(iCall);
			if (pCall != NULL && pCall->GetCallHandle() == NULL &&
				(pCall->GetCallInfo()->dwMediaMode & m_dwLineMediaModes))
			{
				if (!pCall->CreateCallHandle())
					fAllOK = false;
			}
		}
	}

	return fAllOK;

}// CTSPILineConnection::OfferCallsToTAPISrv

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetDevConfig
//
// Return the device configuration for this line and device class.
//
LONG CTSPILineConnection::GetDevConfig(const TString& /*strDeviceClass*/, LPVARSTRING /*lpDeviceConfig*/)
{   
    // Derived class needs to supply the data structures for this based on what may be configured.                                 
    return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetDevConfig

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetDevConfig
//
// Sets the device configuration for this line and device class.
//
LONG CTSPILineConnection::SetDevConfig(const TString& /*strDeviceClass*/, 
                                       LPVOID const /*lpDevConfig*/, DWORD /*dwSize*/)
{   
    // Derived class needs to supply the data structures for this based on
    // what may be configured.                                 
    return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::SetDevConfig

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevSpecific
//
// Invoke a device-specific feature on this line device.
//
LONG CTSPILineConnection::DevSpecific(CTSPIAddressInfo* /*pAddr*/, 
			CTSPICallAppearance* /*pCall*/, DRV_REQUESTID /*dwRequestID*/, 
			LPVOID /*lpParam*/, DWORD /*dwSize*/)
{
	if ((m_LineStatus.dwLineFeatures & LINEFEATURE_DEVSPECIFIC) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // Derived class must manage device-specific features.
    return LINEERR_OPERATIONFAILED;

}// CTSPILineConnection::DevSpecific

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevSpecificFeature
//
// Invoke a device-specific feature on this line device.
//
LONG CTSPILineConnection::DevSpecificFeature(DWORD /*dwFeature*/, DRV_REQUESTID /*dwRequestId*/,
                                             LPVOID /*lpParams*/, DWORD /*dwSize*/)
{                                          
	if ((m_LineStatus.dwLineFeatures & LINEFEATURE_DEVSPECIFICFEAT) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // Derived class must manage device-specific features.
    return LINEERR_OPERATIONFAILED;
    
}// CTSPILineConnection::DevSpecificFeature

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GenericDialogData
//
// This method is called when a dialog which sent in a LINE
// device ID called our UI callback.
//
LONG CTSPILineConnection::GenericDialogData(LPVOID /*lpvItemData*/, LPVOID /*lpParam*/, DWORD /*dwSize*/)
{
	return FALSE;

}// CTSPILineConnection::GenericDialogData

//////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnCallFeaturesChanged
//
// This method gets called whenever a call changes its currently
// available features in the CALLINFO structure.
//
DWORD CTSPILineConnection::OnCallFeaturesChanged (CTSPICallAppearance* /*pCall*/, DWORD dwFeatures)
{ 
	return dwFeatures;

}// CTSPILineConnection::OnCallFeaturesChanged

//////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnAddressFeaturesChanged
//
// This method gets called whenever an address changes its currently
// available features in the ADDRESSSTATUS structure.
//
DWORD CTSPILineConnection::OnAddressFeaturesChanged (CTSPIAddressInfo* /*pAddr*/, DWORD dwFeatures)
{ 
	return dwFeatures;

}// CTSPILineConnection::OnAddressFeaturesChanged

//////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnConnectedCallCountChange
//
// This method gets called whenever any address on this line changes
// the total count of connected calls.  This impacts the bandwidth of the
// service provider (i.e. whether MAKECALL and such may be called).
//
void CTSPILineConnection::OnConnectedCallCountChange(CTSPIAddressInfo* /*pInfo*/, int iDelta)
{
	m_dwConnectedCallCount += iDelta;

	_TSP_DTRACE(_T("%s: OnConnectedCallCountChange: Delta=%d, New Count=%ld\n"), m_strName.c_str(), iDelta, m_dwConnectedCallCount);

	// Now adjust our LINE features based on the total counts.
	RecalcLineFeatures();

}// CTSPILineConnection::OnConnectedCallCountChange

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnMediaConfigChanged
//
// This method may be used by the service provider to notify TAPI that
// the media configuration has changed.
//
void CTSPILineConnection::OnMediaConfigChanged()
{
	// Tell TAPI the configuration has changed.
	Send_TAPI_Event (NULL, LINE_LINEDEVSTATE, LINEDEVSTATE_CONFIGCHANGE);

}// CTSPILineConnection::OnMediaConfigChanged

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::OnMediaControl
//
// This method is called when a media control event was activated
// due to a media monitoring event being caught on this call.
//
void CTSPILineConnection::OnMediaControl (CTSPICallAppearance* /*pCall*/, DWORD /*dwMediaControl*/)
{                                      
	// User must override this or the CTSPICallAppearance::OnMediaControl
	// and perform action on the media event.
	_TSP_ASSERT (false);

}// CTSPILineConnection::OnMediaControl

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SupportsAgents
//
// true/false whether this line has an address which supports agent
// logon/logoff or some other feature.
//
bool CTSPILineConnection::SupportsAgents() const
{
	int nAddrCount = GetAddressCount();
	for (int iAddress = 0; iAddress < nAddrCount; iAddress++)
	{
		if (GetAddress(iAddress)->GetAgentCaps()->dwFeatures != 0)
			return true;
	}
	return false;

}// CTSPILineConnection::SupportsAgents

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::RecalcLineFeatures
//
// This is called when the line device status changes to recalc our
// feature set based on the status and address/call information
//
void CTSPILineConnection::RecalcLineFeatures(bool fRecalcAllAddresses)
{
	DWORD dwFeatures = m_LineStatus.dwLineFeatures;
    DWORD dwStatus = m_LineStatus.dwDevStatusFlags;
	bool isDisabled = ((dwStatus & 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) != 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED));

	// If we are NOT in-service now, we have NO line features.
	if (isDisabled)
		dwFeatures = 0;
	else
	{
		dwFeatures = m_LineCaps.dwLineFeatures & 
			(LINEFEATURE_DEVSPECIFIC |
			 LINEFEATURE_DEVSPECIFICFEAT |
			 LINEFEATURE_FORWARD |
			 LINEFEATURE_MAKECALL |
			 LINEFEATURE_SETMEDIACONTROL |
			 LINEFEATURE_SETDEVSTATUS |
			 LINEFEATURE_FORWARDFWD |
			 LINEFEATURE_FORWARDDND);

		// If we have the bandwidth for a call... 
		dwFeatures &= ~LINEFEATURE_MAKECALL;
		if (m_dwConnectedCallCount < m_LineCaps.dwMaxNumActiveCalls)
		{
			int acount = GetAddressCount();
			for (int i = 0; i < acount; i++)
			{
				if (GetAddress(i)->GetAddressStatus()->
					dwAddressFeatures & LINEADDRFEATURE_MAKECALL)
				{
					dwFeatures |= LINEFEATURE_MAKECALL;
					break;
				}
			}
		}

		// If we have terminals, allow SETTERMINAL.
		if (GetTerminalCount() > 0)
			dwFeatures |= (m_LineCaps.dwLineFeatures & LINEFEATURE_SETTERMINAL);
	}

	// Set the new features; this might not be accurate since we
	// have not recalculated the address features which may remove
	// the MAKECALL bit.
	DWORD dwFeaturesNew = OnLineFeaturesChanged(dwFeatures & m_LineCaps.dwLineFeatures);
	_TSP_DTRACE(_T("%s: Calculating new line features - 0x%lx, Final=0x%lx, ConnectedCalls=%d, Max=%d\n"), m_strName.c_str(), dwFeatures, dwFeaturesNew, m_dwConnectedCallCount, m_LineCaps.dwMaxNumActiveCalls);
	SetLineFeatures(dwFeaturesNew);

	// Recalculate address features if necessary
	bool needRecalcLine = false;
	if (fRecalcAllAddresses)
	{
		// Walk through each address and recalculate the available
		// features for that address; also track whether the feature
		// set actually changed, if so we will need to adjust our 
		// line features as appropriate.
		int acount = GetAddressCount();
		for (int i = 0; i < acount; i++)
		{
			CTSPIAddressInfo* pAddr = GetAddress(i);
			DWORD dwCurrFeatures = pAddr->GetAddressStatus()->dwAddressFeatures;
			pAddr->RecalcAddrFeatures();
			if (dwCurrFeatures != pAddr->GetAddressStatus()->dwAddressFeatures)
				needRecalcLine = true;
		}
	}

	// If the address features changed, then we need to recalculate
	// the new line features; however we do not want to go back
	// through the address calculation a second time.
	if (needRecalcLine && !isDisabled)
		RecalcLineFeatures(false);

}// CTSPILineConnection::RecalcLineFeatures

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateTapiTerminalDeviceClassInfo
//
// This is called directly after the Init() function to create the
// "tapi/terminal" device class information - override this if the 
// assumptions used for phone to terminal matchups are invalid for the
// device in question.
//
bool CTSPILineConnection::CreateTapiTerminalDeviceClassInfo()
{
	// The default implementation requires that a phone was associated
	// to this line during the initialization process.
	CTSPIPhoneConnection* pPhone = GetAssociatedPhone();
	if (pPhone == NULL)
		return false;

	// We assume that any terminal marked as "phone" on the line are associated 
	// with the given phone device.
	TDWordArray arrTermInfo;
	for (int iTerminalID = 0; iTerminalID < GetTerminalCount(); iTerminalID++)
	{
		TERMINALINFO* lpTermInfo = m_arrTerminalInfo[iTerminalID];
		if (lpTermInfo->Capabilities.dwTermDev == LINETERMDEV_PHONE)
			arrTermInfo.push_back(pPhone->GetDeviceID());
		else
			arrTermInfo.push_back(0xffffffff);
	}
	AddDeviceClass(_T("tapi/terminal"), STRINGFORMAT_BINARY, &arrTermInfo[0], 
					GetTerminalCount() * sizeof(DWORD));

	return true;

}// CTSPILineConnection::CreateTapiTerminalDeviceClassInfo

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateAgent
//
// This creates a new agent for this line device. 
//
LONG CTSPILineConnection::CreateAgent(DRV_REQUESTID dwRequestID, LPHAGENT lphAgent, LPCTSTR pszMachineName, LPCTSTR pszUserName, LPCTSTR pszAgentID, LPCTSTR pszAgentPIN)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(lphAgent);
	UNREFERENCED_PARAMETER(pszMachineName);
	UNREFERENCED_PARAMETER(pszUserName);
	UNREFERENCED_PARAMETER(pszAgentID);
	UNREFERENCED_PARAMETER(pszAgentPIN);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::CreateAgent

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetAgentMeasurementPeriod
//
// This method changes the agent measurement period for collecting
// statistics for thie given agent.
//
LONG CTSPILineConnection::SetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, HAGENT hAgent, DWORD dwMeasurementPeriod)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(hAgent);
	UNREFERENCED_PARAMETER(dwMeasurementPeriod);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::SetAgentMeasurementPeriod

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAgentInfo
//
// This function returns information about the given agent handle.
//
LONG CTSPILineConnection::GetAgentInfo(HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo)
{
	UNREFERENCED_PARAMETER(hAgent);
	UNREFERENCED_PARAMETER(lpAgentInfo);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetAgentInfo

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::CreateAgentSession
//
// This function starts a new session for an agent. (agent login)
//
LONG CTSPILineConnection::CreateAgentSession(DRV_REQUESTID dwRequestID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCTSTR pszAgentPIN, const GUID& guid, DWORD dwWorkingAddressID)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(lphSession);
	UNREFERENCED_PARAMETER(hAgent);
	UNREFERENCED_PARAMETER(pszAgentPIN);
	UNREFERENCED_PARAMETER(guid);
	UNREFERENCED_PARAMETER(dwWorkingAddressID);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::CreateAgentSession

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAgentSessionList
//
// This returns a list of the sessions an agent is associated with.
//
LONG CTSPILineConnection::GetAgentSessionList(HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList)
{
	UNREFERENCED_PARAMETER(hAgent);
	UNREFERENCED_PARAMETER(lpSessionList);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetAgentSessionList

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetAgentSessionState
//
// This changes the agent session state.
//
LONG CTSPILineConnection::SetAgentSessionState(DRV_REQUESTID dwRequestID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(hSession);
	UNREFERENCED_PARAMETER(dwAgentSessionState);
	UNREFERENCED_PARAMETER(dwNextAgentSessionState);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::SetAgentSessionState

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAgentSessionInfo
//
// This returns information on a specific agent session
//
LONG CTSPILineConnection::GetAgentSessionInfo(HAGENTSESSION hSession, LPLINEAGENTSESSIONINFO lpSessionInfo)
{
	UNREFERENCED_PARAMETER(hSession);
	UNREFERENCED_PARAMETER(lpSessionInfo);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetAgentSessionInfo

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetQueueList
//
// This returns information about an agent queue
//
LONG CTSPILineConnection::GetQueueList(const GUID& GroupID, LPLINEQUEUELIST lpQueueList)
{
	UNREFERENCED_PARAMETER(GroupID);
	UNREFERENCED_PARAMETER(lpQueueList);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetQueueList

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetQueueMeasurementPeriod
//
// This changes the statistic gathering period for queues.
//
LONG CTSPILineConnection::SetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwQueueID, DWORD dwMeasurementPeriod)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(dwQueueID);
	UNREFERENCED_PARAMETER(dwMeasurementPeriod);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::SetQueueMeasurementPeriod

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetQueueInfo
//
// This retrieves information about the given queue.
//
LONG CTSPILineConnection::GetQueueInfo(DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo)
{
	UNREFERENCED_PARAMETER(dwQueueID);
	UNREFERENCED_PARAMETER(lpQueueInfo);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetQueueInfo

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetGroupList
//
// This retrieves the list of agent groups.
//
LONG CTSPILineConnection::GetGroupList(LPLINEAGENTGROUPLIST lpGroupList)
{
	UNREFERENCED_PARAMETER(lpGroupList);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::GetGroupList

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetAgentStateEx
//
// This changes the agent state
//
LONG CTSPILineConnection::SetAgentStateEx(DRV_REQUESTID dwRequestID, HAGENT hAgent, DWORD dwState, DWORD dwNextState)
{
	UNREFERENCED_PARAMETER(dwRequestID);
	UNREFERENCED_PARAMETER(hAgent);
	UNREFERENCED_PARAMETER(dwState);
	UNREFERENCED_PARAMETER(dwNextState);

	return LINEERR_OPERATIONUNAVAIL;

}// CTSPILineConnection::SetAgentStateEx

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::MSPIdentify
//
// This function determines the associated MSP CLSID for each line
// device. This function requires TAPI 3.0 negotiation.
//
LONG CTSPILineConnection::MSPIdentify(GUID* pGUID)
{
	memcpy(pGUID, &m_guidMSP, sizeof(GUID));
	return (m_guidMSP == IID_NULL) ? LINEERR_OPERATIONUNAVAIL : 0;

}// CTSPILineConnection::MSPIdentify

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::ReceiveMSPData
//
// This function receives data sent by a media service provider (MSP).
// It requires TAPI 3.0 negotiation.
//
LONG CTSPILineConnection::ReceiveMSPData(CMSPDriver* /*pMSP*/, CTSPICallAppearance* /*pCall*/, LPVOID /*lpData*/, DWORD /*dwSize*/)
{
	// Must be overridden by derived class
	return LINEERR_OPERATIONFAILED;

}// CTSPILineConnection::ReceiveMSPData

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetCallHubTracking
//
// This function fills in the status of call hub tracking provided by the
// service proider. It requires TAPI 3.0 negotiation.
//
LONG CTSPILineConnection::GetCallHubTracking(LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	// Mark that provider-level tracking is supported.
	lpTrackingInfo->dwNeededSize = sizeof(LINECALLHUBTRACKINGINFO);
	lpTrackingInfo->dwUsedSize = sizeof(LINECALLHUBTRACKINGINFO);
	lpTrackingInfo->dwAvailableTracking = LINECALLHUBTRACKING_ALLCALLS;
	lpTrackingInfo->dwCurrentTracking = m_dwCallHubTracking;
	return 0;

}// CTSPILineConnection::GetCallHubTracking

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetCallHubTracking
//
// This function fills in the status of call hub tracking provided by the
// service proider. It requires TAPI 3.0 negotiation.
//
LONG CTSPILineConnection::SetCallHubTracking(LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	// Set the new call hub tracking support; the TSP library actually ignores
	// this at the moment as TAPISRV doesn't appear to expect anything different as
	// of Win2K build 2195.
	m_dwCallHubTracking = lpTrackingInfo->dwCurrentTracking;
	return 0;

}// CTSPILineConnection::SetCallHubTracking

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::Dump
//
// Debug "dump" of the object and it's contents.
//
TString CTSPILineConnection::Dump() const 
{
	TStringStream outstm;
	outstm << _T("0x") << hex << (DWORD)this;
    outstm << _T(",LineID=0x") << hex << m_LineCaps.dwPermanentLineID;
	outstm << _T(",DeviceID=0x") << hex << m_dwDeviceID;
	outstm << _T(",htLine=0x") << hex << m_htLine;
    return(outstm.str());

}// CTSPILineConnection::Dump
#endif
