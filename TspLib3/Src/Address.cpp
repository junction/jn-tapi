/******************************************************************************/
//                                                                        
// ADDRESS.CPP - Source code for the CTSPIAddressInfo object          
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the source to manage the address objects which are 
// held by the CTSPILineConnection objects.
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
#include <ctype.h>
#include <spbstrm.h>

///////////////////////////////////////////////////////////////////////////
// distr_concat
//
// STL function which concats strings together from DIALINFO structures
//
TString distr_concat(TString strCurr, DIALINFO* pDialInfo)
{
	if (pDialInfo && !pDialInfo->strNumber.empty())
	{
		if (!strCurr.empty()) strCurr += _T("\r\n");
		strCurr += pDialInfo->strNumber;
	}
	return strCurr;
}

///////////////////////////////////////////////////////////////////////////
// distr_size
//
// STL function which sums string lengths from DIALINFO structures
//
TString::size_type distr_size(TString::size_type nCurr, DIALINFO* pDialInfo)
{
	if (pDialInfo && !pDialInfo->strNumber.empty())
	{
		if (nCurr > 0) ++nCurr; // Add in a NULL which will sit between numbers.
		nCurr += (pDialInfo->strNumber.length()+1) * sizeof(wchar_t);
	}
	return nCurr;
}

///////////////////////////////////////////////////////////////////////////
// fac_srch
//
// Private searching functor which looks for attached calls.
//
struct fac_srch : public std::binary_function<CTSPICallAppearance*,CTSPICallAppearance*,bool>
{
	result_type operator()(first_argument_type pCall, second_argument_type psCall) const {
		return (!pCall->HasBeenDeleted() && pCall->GetAttachedCall() == psCall);
	}
};

///////////////////////////////////////////////////////////////////////////
// fsc_srch
//
// Private searching functor which looks for call state(s)
//
struct fsc_srch : public std::binary_function<CTSPICallAppearance*,DWORD,bool>
{
	result_type operator()(first_argument_type pCall, second_argument_type dwState) const {
		return (!pCall->HasBeenDeleted() && (pCall->GetCallState() & dwState) != 0);
	}
};

///////////////////////////////////////////////////////////////////////////
// fci_srch
//
// Private searching functor which looks for call id
//
struct fci_srch : public std::binary_function<CTSPICallAppearance*,DWORD,bool>
{
	result_type operator()(first_argument_type pCall, second_argument_type dwCallID) const {
		return (!pCall->HasBeenDeleted() && (pCall->GetCallID() == dwCallID));
	}
};

///////////////////////////////////////////////////////////////////////////
// fch_srch
//
// Private searching functor which looks for call handle
//
struct fch_srch : public std::binary_function<CTSPICallAppearance*,HTAPICALL,bool>
{
	result_type operator()(first_argument_type pCall, second_argument_type htCall) const {
		return (!pCall->HasBeenDeleted() && (pCall->GetCallHandle() == htCall));
	}
};

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CTSPIAddressInfo
//
// This is the constructor for the address information structure.
//
CTSPIAddressInfo::CTSPIAddressInfo() : CTSPIBaseObject(),
	m_pLine(0), m_dwAddressID(0), m_dwAddressStates(0),
	m_dwConnectedCallCount(0), m_lpMediaControl(0), m_dwFlags(0)
{
    ZeroMemory (&m_AddressCaps, sizeof(LINEADDRESSCAPS));
    ZeroMemory (&m_AddressStatus, sizeof(LINEADDRESSSTATUS));
	ZeroMemory (&m_AgentCaps, sizeof(TAgentCaps));

	m_dwAddressType = 1; // LINEADDRESSTYPE_PHONENUMBER;

}// CTSPIAddressInfo::CTSPIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::~CTSPIAddressInfo
//
// Address information object destructure - delete all existing
// call appearances.
//
CTSPIAddressInfo::~CTSPIAddressInfo()
{   
    // Delete the forwarding information - this will request
    // and release the mutex.
    DeleteForwardingInfo();

	// If we have media control information, decrement it.
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->DecUsage();

	// Delete any MSP device connections
	std::for_each(m_arrMSPInstance.begin(), m_arrMSPInstance.end(), 
					MEM_FUNCV(&CMSPDriver::DecRef));
	m_arrMSPInstance.clear();

#ifdef _DEBUG
	// Dump the call-list so we can see what was left when the TSP
	// was exiting.
	int iCount = 0;
	_TSP_DTRACEX(TRC_CALLS, _T("%d Calls left on address %s [0x%lx]\r\n"), m_lstCalls.size(), m_strAddress.c_str(), this);
	for (TCallList::iterator ii = m_lstCalls.begin(); ii != m_lstCalls.end(); ++ii)
		_TSP_DTRACEX(TRC_CALLS, _T("%d: %s\r\n"), ++iCount, (*ii)->Dump().c_str());
#endif

}// CTSPIAddressInfo::~CTSPIAddressInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::Init
//
// Initialization function for dynamically created object.  This initializes
// our internal structures for this address.  It can be overriden to add new
// fields to the ADDRESSCAPS, or the lineGetAddressCaps of the CServiceProvider
// class may be overriden, or the GetAddressCaps of this class may also be
// overriden.
//
void CTSPIAddressInfo::Init (CTSPILineConnection* pLine, DWORD dwAddressID, LPCTSTR lpszAddress,
                             LPCTSTR lpszName, bool fSupportsIncomingCalls, 
                             bool fSupportsOutgoingCalls,DWORD dwAvailMediaModes, 
                             DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate,
                             DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
                             DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
                             DWORD dwMaxNumTransConf, DWORD dwAddressSharing, const LPLINEDIALPARAMS lpDialParams, 
							 DWORD dwAddressType)
{
    // Fill in the passed information
	m_pLine = pLine;
    m_dwAddressID = dwAddressID;
    m_dwBearerMode = dwBearerMode;
    m_dwMinRateAvail = dwMinRate;
    m_dwMaxRateAvail = dwMaxRate;
	m_dwCurrRate = dwMinRate;

	// Save off the dialing parameters
	if (lpDialParams)
		CopyMemory(&m_DialParams, lpDialParams, sizeof(LINEDIALPARAMS));
	else
		ZeroMemory(&m_DialParams, sizeof(LINEDIALPARAMS));

	m_dwAddressType = dwAddressType;

	if (fSupportsIncomingCalls)
		m_dwFlags |= InputAvail;
	if (fSupportsOutgoingCalls)
		m_dwFlags |= OutputAvail;

	// Move the name over.
	if (lpszName != NULL)
		m_strName = lpszName;
    
    // Move the address over - only numbers please!
	if (lpszAddress != NULL)
		m_strAddress = GetSP()->GetDialableNumber(lpszAddress);

	// Mark the available media modes for this address.
    m_AddressCaps.dwAvailableMediaModes = (dwAvailMediaModes | LINEMEDIAMODE_UNKNOWN);

	// Set the current address sharing mode.
    m_AddressCaps.dwAddressSharing = (dwAddressSharing) ? dwAddressSharing : LINEADDRESSSHARING_PRIVATE;

    // Mark the available address state notifications the class library manages.
    m_AddressCaps.dwAddressStates = 
			(LINEADDRESSSTATE_OTHER | 
			 LINEADDRESSSTATE_INUSEZERO | 
			 LINEADDRESSSTATE_INUSEONE | 
             LINEADDRESSSTATE_FORWARD | 
			 LINEADDRESSSTATE_INUSEMANY | 
			 LINEADDRESSSTATE_NUMCALLS | 
			 LINEADDRESSSTATE_CAPSCHANGE);

    // Mark the different LINE_CALLINFO messages which we can generate by our call appearances.  The only
    // field which cannot be generated by the library is DEVSPECIFIC - if this is supported by your service provider,
    // make sure to add it to this list in your derived class
	if (dwAddressSharing == LINEADDRESSSHARING_MONITORED)
	{
		m_AddressCaps.dwCalledIDFlags = m_AddressCaps.dwCallerIDFlags = LINECALLPARTYID_UNAVAIL;
	}
	else
	{
		m_AddressCaps.dwCallInfoStates = 
				(LINECALLINFOSTATE_OTHER | 
				 LINECALLINFOSTATE_BEARERMODE | 
		         LINECALLINFOSTATE_RATE |
		         LINECALLINFOSTATE_MEDIAMODE | 
				 LINECALLINFOSTATE_APPSPECIFIC |
		         LINECALLINFOSTATE_CALLID | 
				 LINECALLINFOSTATE_RELATEDCALLID | 
		         LINECALLINFOSTATE_ORIGIN | 
				 LINECALLINFOSTATE_REASON |
		         LINECALLINFOSTATE_COMPLETIONID | 
				 LINECALLINFOSTATE_TRUNK |
		         LINECALLINFOSTATE_CALLERID | 
				 LINECALLINFOSTATE_CALLEDID |
		         LINECALLINFOSTATE_CONNECTEDID | 
				 LINECALLINFOSTATE_REDIRECTIONID |
		         LINECALLINFOSTATE_REDIRECTINGID | 
				 LINECALLINFOSTATE_USERUSERINFO | 
		         LINECALLINFOSTATE_DIALPARAMS);

		// Add the states which are set by TAPI apis.
		if (GetSP()->CanHandleRequest(TSPI_LINEMONITORMEDIA))
			m_AddressCaps.dwCallInfoStates |= LINECALLINFOSTATE_MONITORMODES;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLTREATMENT))
			m_AddressCaps.dwCallInfoStates |= LINECALLINFOSTATE_TREATMENT;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLQUALITYOFSERVICE))
			m_AddressCaps.dwCallInfoStates |= LINECALLINFOSTATE_QOS;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLDATA))
			m_AddressCaps.dwCallInfoStates |= LINECALLINFOSTATE_CALLDATA;
		if (GetSP()->CanHandleRequest(TSPI_LINESETTERMINAL))
		{
			m_AddressCaps.dwAddressStates |= LINEADDRESSSTATE_TERMINALS;
			m_AddressCaps.dwCallInfoStates |= LINECALLINFOSTATE_TERMINAL;
		}

	    // Mark the supported caller id fields.
	    m_AddressCaps.dwCallerIDFlags = 
	    m_AddressCaps.dwConnectedIDFlags = 
	    m_AddressCaps.dwRedirectionIDFlags =
	    m_AddressCaps.dwRedirectingIDFlags =
	    m_AddressCaps.dwCalledIDFlags = 
					(LINECALLPARTYID_BLOCKED | 
					 LINECALLPARTYID_OUTOFAREA |
	                 LINECALLPARTYID_NAME | 
					 LINECALLPARTYID_ADDRESS | 
	                 LINECALLPARTYID_PARTIAL | 
					 LINECALLPARTYID_UNKNOWN | 
	                 LINECALLPARTYID_UNAVAIL);

	    // Mark the call states we transition through in the class library
	    m_AddressCaps.dwCallStates = 
					(LINECALLSTATE_IDLE | 
					 LINECALLSTATE_CONNECTED | 
					 LINECALLSTATE_UNKNOWN |
	                 LINECALLSTATE_PROCEEDING | 
					 LINECALLSTATE_DISCONNECTED | 
	                 LINECALLSTATE_BUSY | 
					 LINECALLSTATE_SPECIALINFO);
	    
	    // Set the tone modes and disconnect information - report them all as available even though 
	    // the service provider might not actually report some of them
	    m_AddressCaps.dwDialToneModes = 
					(LINEDIALTONEMODE_NORMAL | 
					 LINEDIALTONEMODE_SPECIAL |
	                 LINEDIALTONEMODE_INTERNAL | 
					 LINEDIALTONEMODE_EXTERNAL |
	                 LINEDIALTONEMODE_UNKNOWN | 
					 LINEDIALTONEMODE_UNAVAIL);

	    m_AddressCaps.dwBusyModes = 
					(LINEBUSYMODE_STATION | 
					 LINEBUSYMODE_TRUNK | 
					 LINEBUSYMODE_UNKNOWN |
	                 LINEBUSYMODE_UNAVAIL);

		// Report the new TAPI 2.0 mode information - again, these might not all be reported by this
		// provider, but return them anyway.
		m_AddressCaps.dwConnectedModes = 
					(LINECONNECTEDMODE_ACTIVE | 
					 LINECONNECTEDMODE_INACTIVE |
					 LINECONNECTEDMODE_ACTIVEHELD | 
					 LINECONNECTEDMODE_INACTIVEHELD | 
					 LINECONNECTEDMODE_CONFIRMED);

		m_AddressCaps.dwOfferingModes = 
					(LINEOFFERINGMODE_ACTIVE | 
					 LINEOFFERINGMODE_INACTIVE);

	    // Report special info as unavailable/unknown - if the service provider is to support
	    // any of the special tone information, then add the appropriate fields.
	    m_AddressCaps.dwSpecialInfo = (LINESPECIALINFO_UNKNOWN | LINESPECIALINFO_UNAVAIL);

	    // Report all the disconnect modes - some may not be reported by the service provider, but
	    // its still ok to list them to TAPI.
	    m_AddressCaps.dwDisconnectModes = 
					(LINEDISCONNECTMODE_NORMAL | 
					 LINEDISCONNECTMODE_UNKNOWN |
	                 LINEDISCONNECTMODE_REJECT | 
					 LINEDISCONNECTMODE_BUSY |
	                 LINEDISCONNECTMODE_NOANSWER | 
					 LINEDISCONNECTMODE_BADADDRESS |
	                 LINEDISCONNECTMODE_UNREACHABLE | 
					 LINEDISCONNECTMODE_CONGESTION |
	                 LINEDISCONNECTMODE_INCOMPATIBLE | 
					 LINEDISCONNECTMODE_UNAVAIL | 
					 LINEDISCONNECTMODE_NODIALTONE |
					 LINEDISCONNECTMODE_NUMBERCHANGED |
					 LINEDISCONNECTMODE_OUTOFORDER |
					 LINEDISCONNECTMODE_TEMPFAILURE |
					 LINEDISCONNECTMODE_QOSUNAVAIL |
					 LINEDISCONNECTMODE_BLOCKED |
					 LINEDISCONNECTMODE_DONOTDISTURB | 
				     LINEDISCONNECTMODE_CANCELLED);

	    // Set the max calls information
	    m_AddressCaps.dwMaxNumActiveCalls = dwMaxNumActiveCalls;
	    m_AddressCaps.dwMaxNumOnHoldCalls = dwMaxNumOnHoldCalls;
	    m_AddressCaps.dwMaxNumConference = dwMaxNumConference;
	    m_AddressCaps.dwMaxNumTransConf = dwMaxNumTransConf;
	    m_AddressCaps.dwMaxNumOnHoldPendingCalls = dwMaxNumOnHoldPendCalls;

	    // Set the address capability flags to a generic set.  Replace the flags here with the ones
	    // the service provider really supports.
	    m_AddressCaps.dwAddrCapFlags = 
					(LINEADDRCAPFLAGS_DIALED | 
					 LINEADDRCAPFLAGS_ORIGOFFHOOK | 
					 LINEADDRCAPFLAGS_COMPLETIONID);
	    
	    // Determine which capabilities this address supports.
	    m_AddressCaps.dwCallFeatures = LINECALLFEATURE_DROP;

	    // Add in the call features available.
	    if (CanMakeCalls() && GetSP()->CanHandleRequest(TSPI_LINEMAKECALL))
	    {
	        m_AddressCaps.dwCallStates |= (LINECALLSTATE_DIALING | LINECALLSTATE_DIALTONE | LINECALLSTATE_RINGBACK);
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_MAKECALL;
		}        

		if (GetSP()->CanHandleRequest(TSPI_LINEDIAL))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_DIAL;

	    if (CanAnswerCalls() && GetSP()->CanHandleRequest(TSPI_LINEANSWER))
	    {
	        m_AddressCaps.dwCallStates |= (LINECALLSTATE_OFFERING | LINECALLSTATE_ACCEPTED);
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_ANSWER;
		}
		        
	    if (GetSP()->CanHandleRequest(TSPI_LINESETUPCONFERENCE))
	    {
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETUPCONF;
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_SETUPCONF;
		}   
		     
	    if (GetSP()->CanHandleRequest(TSPI_LINESETUPTRANSFER))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETUPTRANSFER;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEPARK))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_PARK;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEADDTOCONFERENCE))
	    {
	        m_AddressCaps.dwCallStates |= (LINECALLSTATE_CONFERENCED | LINECALLSTATE_ONHOLDPENDCONF);
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_ADDTOCONF;
	    }
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEBLINDTRANSFER))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_BLINDTRANSFER;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEHOLD))
	    {
	        m_AddressCaps.dwCallStates |= LINECALLSTATE_ONHOLD;
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_HOLD;
		}
		        
	    if (GetSP()->CanHandleRequest(TSPI_LINESENDUSERUSERINFO))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SENDUSERUSER;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINESWAPHOLD))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SWAPHOLD;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEREDIRECT))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_REDIRECT;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEACCEPT))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_ACCEPT;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEREMOVEFROMCONFERENCE))
		{
			m_AddressCaps.dwRemoveFromConfCaps = LINEREMOVEFROMCONF_ANY;
			m_AddressCaps.dwRemoveFromConfState = LINECALLSTATE_IDLE;
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_REMOVEFROMCONF;
		}
		else
			m_AddressCaps.dwRemoveFromConfCaps = LINEREMOVEFROMCONF_NONE;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINEUNHOLD))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_UNHOLD;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINECOMPLETETRANSFER))
	    {
	        m_AddressCaps.dwCallStates |= LINECALLSTATE_ONHOLDPENDTRANSFER;
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_COMPLETETRANSF;
		}
		        
	    if (GetSP()->CanHandleRequest(TSPI_LINECOMPLETECALL))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_COMPLETECALL;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINESECURECALL))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SECURECALL;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINESETCALLPARAMS))
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETCALLPARAMS;
	        
	    if (GetSP()->CanHandleRequest(TSPI_LINESETTERMINAL))
	    {
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETTERMINAL;
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_SETTERMINAL;
		}        
	    
	    if (GetSP()->CanHandleRequest(TSPI_LINESETMEDIACONTROL))
	    {
	        m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETMEDIACONTROL;
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_SETMEDIACONTROL;
		}        
	    
	    if (GetSP()->CanHandleRequest(TSPI_LINEFORWARD))
	    {
	        m_AddressCaps.dwMaxForwardEntries = 1;
	        m_AddressCaps.dwDisconnectModes |= LINEDISCONNECTMODE_FORWARDED;
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_FORWARD; 
		}
		    
	    if (GetSP()->CanHandleRequest(TSPI_LINEPICKUP))
	    {
	        m_AddressCaps.dwDisconnectModes |= LINEDISCONNECTMODE_PICKUP;
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_PICKUP; 
		}        
	    
	    if (GetSP()->CanHandleRequest(TSPI_LINEUNCOMPLETECALL))
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_UNCOMPLETECALL;      
	    
	    if (GetSP()->CanHandleRequest(TSPI_LINEUNPARK))
	        m_AddressCaps.dwAddressFeatures |= LINEADDRFEATURE_UNPARK;

		// Add the additional call features based on whether we can support
		// the request.
		if (GetSP()->CanHandleRequest(TSPI_LINEGATHERDIGITS))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_GATHERDIGITS;
		if (GetSP()->CanHandleRequest(TSPI_LINEGENERATEDIGITS))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_GENERATEDIGITS;
		if (GetSP()->CanHandleRequest(TSPI_LINEGENERATETONE))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_GENERATETONE;
		if (GetSP()->CanHandleRequest(TSPI_LINEMONITORDIGITS))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_MONITORDIGITS;
		if (GetSP()->CanHandleRequest(TSPI_LINEMONITORMEDIA))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_MONITORMEDIA;
		if (GetSP()->CanHandleRequest(TSPI_LINEMONITORTONES))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_MONITORTONES;
		if (GetSP()->CanHandleRequest(TSPI_LINEPREPAREADDTOCONFERENCE))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_PREPAREADDCONF;
		if (GetSP()->CanHandleRequest(TSPI_LINESETMEDIACONTROL))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETMEDIACONTROL;
		if (GetSP()->CanHandleRequest(TSPI_LINERELEASEUSERUSERINFO))		
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_RELEASEUSERUSERINFO;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLTREATMENT))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETTREATMENT;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLQUALITYOFSERVICE))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETQOS;
		if (GetSP()->CanHandleRequest(TSPI_LINESETCALLDATA))
			m_AddressCaps.dwCallFeatures |= LINECALLFEATURE_SETCALLDATA;
	}

	// Grab the terminal information from our parent line.  This information
	// will then be applied to any of our calls.
	for (int i = 0; i < m_pLine->GetTerminalCount(); i++)
		m_arrTerminals.push_back(m_pLine->GetTerminalInformation(i));		
	
	// Setup the initial address status.  We assume no calls are currently on
	// the line.
	m_AddressStatus.dwTotalSize = sizeof(LINEADDRESSSTATUS);
    m_AddressStatus.dwAddressFeatures = (m_AddressCaps.dwAddressFeatures & 
    						   (LINEADDRFEATURE_SETMEDIACONTROL |
        						LINEADDRFEATURE_SETTERMINAL |
        						LINEADDRFEATURE_FORWARD | 
        						LINEADDRFEATURE_PICKUP |
        						LINEADDRFEATURE_MAKECALL));

	// Setup our agent state/capabilities
	m_AgentStatus.dwState = 
	m_AgentStatus.dwNextState = LINEAGENTSTATE_UNKNOWN;

	// If this is a monitored address then we have no calls/features.
	if (dwAddressSharing == LINEADDRESSSHARING_MONITORED)
	{
		m_AddressStatus.dwAddressFeatures = 
		m_AddressCaps.dwAddressFeatures =
		m_AddressCaps.dwCallFeatures = 
		m_AddressCaps.dwCallFeatures2 = 0;
	}

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_OBJECTS, _T("Created %s\n"), Dump().c_str());
#endif

	// Additional elements which need to be supplied by the derived provider ..
	// These fields are all pre-initialized to zero:
	//
	//	m_AddressCaps.dwTransferModes
	//	m_AddressCaps.dwParkModes
	//	m_AddressCaps.dwMaxCallCompletions
	//	m_AddressCaps.dwCallCompletionConds
	//	m_AddressCaps.dwCallCompletionModes


}// CTSPIAddressInfo::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::read
//
// Reads the object from a stream - should be overriden to provide
// initialization support in the v3.x INIT process.
//
TStream& CTSPIAddressInfo::read(TStream& istm)
{
	return istm;

}// CTSPIAddressInfo::read

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SaveToStream
//
// Writes the ADDRESSINFO into a stream for serialization
//
void CTSPIAddressInfo::SaveToStream(TStream& ostm) const
{
	TVersionInfo vi(ostm, 3);

	bool fAllowIncoming = (m_dwFlags & InputAvail) > 0;
	bool fAllowOutgoing = (m_dwFlags & OutputAvail) > 0;

	ostm << m_strName << m_strAddress << fAllowIncoming << fAllowOutgoing
		 << m_AddressCaps.dwAvailableMediaModes 
		 << m_dwBearerMode << m_dwMinRateAvail << m_dwMaxRateAvail
		 << m_AddressCaps.dwMaxNumActiveCalls << m_AddressCaps.dwMaxNumOnHoldCalls
		 << m_AddressCaps.dwMaxNumOnHoldPendingCalls
		 << m_AddressCaps.dwMaxNumConference << m_AddressCaps.dwMaxNumTransConf
		 << m_DialParams.dwDialPause << m_DialParams.dwDialSpeed 
	     << m_DialParams.dwDigitDuration << m_DialParams.dwWaitForDialtone;
	ostm << m_dwAddressType;

	// Allow the deriveed class to add it's own information
	write(ostm);

}// CTSPIAddressInfo::SaveToStream

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::write
//
// Writes the object to a stream - should be overriden to provide
// the additional information required.
//
TStream& CTSPIAddressInfo::write(TStream& istm) const
{
	return istm;

}// CTSPIAddressInfo::write

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CreateConferenceCall
//
// This method creates a call appearance which will be used as a
// conference call.
//
CTSPIConferenceCall* CTSPIAddressInfo::CreateConferenceCall(HTAPICALL hCall)
{
    CTSPIConferenceCall* pCall = NULL;

    // See if the call appearance already exists.
    if (hCall != NULL)
    {
		CTSPICallAppearance* pTestCall = FindCallByHandle(hCall);
		if (pTestCall != NULL)
		{
			if (pTestCall->GetCallType() == CTSPICallAppearance::Conference)
				return dynamic_cast<CTSPIConferenceCall*>(pCall);
			return NULL;
		}
    }

    // Create the call appearance
    if ((pCall = GetSP()->CreateConferenceCallObject()) == NULL)
        return NULL;

    // 1st phase initialization for the call
    pCall->Init(this, m_dwBearerMode, m_dwCurrRate, 0, LINECALLORIGIN_CONFERENCE, 
		LINECALLREASON_DIRECT, 0xffffffff, 0);

    // Add it to our list
	CEnterCode sLock(this);  // Synch access to object

	try
	{
		m_lstCalls.push_back(pCall);
	}
	catch(...)
	{
		delete pCall;
		throw;
	}
	
	sLock.Unlock();

    // If we don't have a call handle, then ask TAPI for one.
    if (hCall == NULL &&
		m_AddressCaps.dwAddressSharing != LINEADDRESSSHARING_MONITORED)
    {
        DWORD dwTapiCall = 0;
        GetLineOwner()->Send_TAPI_Event(NULL, LINE_NEWCALL, reinterpret_cast<DWORD>(pCall), reinterpret_cast<DWORD>(&dwTapiCall));
        if (dwTapiCall != 0)
            hCall = reinterpret_cast<HTAPICALL>(dwTapiCall);
    }

	// Add in the call handle. This was separated out to handle a timing issue where lineCloseCall is called
	// immediately after the LINE_NEWCALL Is received.  In this case, the call object wasn't yet completely
	// initialized and didn't have it's line owner or address owner set up.
	pCall->SetCallHandle(hCall);

	_TSP_DTRACEX(TRC_CALLS, _T("%s: CreateConferenceCall: SP call=0x%lx, TAPI call=0x%lx\n"), m_strAddress.c_str(), pCall, hCall);

    // Notify ourselves in case a derived class wants to know.
    OnCreateCall (pCall);

    return pCall;

}// CTSPIAddressInfo::CreateConferenceCall

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CreateCallAppearance
//
// This method creates a new call appearance.  The call appearance
// object is returned to the caller.
//
CTSPICallAppearance* CTSPIAddressInfo::CreateCallAppearance(HTAPICALL hCall,
                                DWORD dwCallParamFlags, DWORD dwOrigin, 
                                DWORD dwReason, DWORD dwTrunk, DWORD dwCompletionID)
{
    CTSPICallAppearance* pCall = NULL;

    // See if the call appearance already exists.
    if (hCall != NULL)
    {
        pCall = FindCallByHandle (hCall);
        if (pCall)
            return pCall;
    }

    // Create the call appearance
    if ((pCall = GetSP()->CreateCallObject()) == NULL)
        return NULL;

	// Do the 1st phas initialization for this call object
    pCall->Init(this, m_dwBearerMode, m_dwCurrRate, dwCallParamFlags, dwOrigin, dwReason, dwTrunk, dwCompletionID);

    // Add it to our list
	CEnterCode sLock(this);  // Synch access to object

	try
	{
		m_lstCalls.push_back(pCall);
	}
	catch(...)
	{
		delete pCall;
		throw;
	}
	
	sLock.Unlock();

    // If we don't have a call handle, then ask TAPI for one.
    if (hCall == NULL &&
		m_AddressCaps.dwAddressSharing != LINEADDRESSSHARING_MONITORED)
    {
        DWORD dwTapiCall = 0;
        GetLineOwner()->Send_TAPI_Event(NULL, LINE_NEWCALL, reinterpret_cast<DWORD>(pCall), reinterpret_cast<DWORD>(&dwTapiCall));
        if (dwTapiCall != 0)
            hCall = reinterpret_cast<HTAPICALL>(dwTapiCall);
    }

	// Add in the call handle. This was seperated out to handle a timing issue where lineCloseCall is called
	// immediately after the LINE_NEWCALL Is received.  In this case, the call object wasn't yet completely
	// initialized and didn't have it's line owner or address owner set up.
	pCall->SetCallHandle(hCall);

	_TSP_DTRACEX(TRC_CALLS, _T("%s: CreateCallAppearance: SP call=0x%lx, TAPI call=0x%lx\n"), m_strAddress.c_str(), pCall, hCall);

	// If the completion ID is non-zero, then use the appropriate reason.
	if (dwCompletionID > 0 && dwReason == LINECALLREASON_UNKNOWN)
		dwReason = LINECALLREASON_CALLCOMPLETION;
    
    // Notify ourselves in case a derived class wants to know.
    OnCreateCall (pCall);

    return pCall;

}// CTSPIAddressInfo::CreateCallAppearance

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::RemoveCallAppearance
//
// Remove a call appearance from our list and delete it.
//
void CTSPIAddressInfo::RemoveCallAppearance (CTSPICallAppearance* pCall)
{
    // If it is null or is deleted, ignore it.
    if (pCall == NULL || pCall->HasBeenDeleted())
        return;

	// So we don't try to remove it twice
	pCall->m_dwFlags |= CTSPICallAppearance::_IsDeleted;

	// If it has an attached call, notify a change.  This will cause us
	// to re-attach any other call or zero out the attachment.
	if (pCall->GetAttachedCall() != NULL)
	{
		pCall->GetAttachedCall()->
			OnRelatedCallStateChange(pCall, LINECALLSTATE_IDLE, LINECALLSTATE_UNKNOWN);
	}

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_CALLS, _T("Removing call %s from address %s\r\n"), pCall->Dump().c_str(), Dump().c_str());
#endif

    // Locate and remove the call from our call array
	CEnterCode sLock(this);  // Synch access to object
	TCallList::iterator pos = std::find(m_lstCalls.begin(), m_lstCalls.end(), pCall);
	_TSP_ASSERTE(pos != m_lstCalls.end());
	if (pos != m_lstCalls.end())
        m_lstCalls.erase(pos);

#ifdef _DEBUG
	{
		int iPos = 0;
		_TSP_DTRACEX(TRC_CALLMAP, _T("%d remaining calls on address 0x%lx\r\n"), m_lstCalls.size(), (DWORD) this);
		for (pos = m_lstCalls.begin(); pos != m_lstCalls.end(); ++pos)
			_TSP_DTRACEX(TRC_CALLMAP, _T("%d: %s\r\n"), ++iPos, (*pos)->Dump().c_str());
	}
#endif
	sLock.Unlock();

	// Make sure it goes IDLE for our active call counts.
	pCall->SetCallState(LINECALLSTATE_IDLE, 0, 0, false);

	// Set our callid to zero - this will remove our callhub entry
	pCall->SetCallID(0);

    // Make sure the call isn't referenced in a request packet anywhere.
    GetLineOwner()->OnCallDeleted (pCall);

    // Zero out the TAPI handle
	pCall->m_htCall = 0;

	// Decrement the reference count.  This will eventually delete the
	// call object when all requests which are associated with the call 
	// are deleted.
    pCall->DecRef();

}// CTSPIAddressInfo::RemoveCallAppearance

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::NotifyInUseZero
//
// This checks to see whether any calls exist in the non-IDLE state.
// It is called as each call is removed from the address
//
bool CTSPIAddressInfo::NotifyInUseZero()
{
	CEnterCode sLock(this);  // Synch access to object

	// If we already have reported no non-IDLE calls, don't do it again.
	if (m_AddressStatus.dwNumInUse == 0)
		return false;

	// Walk the call list looking for non-idle calls.
	if (std::find_if(m_lstCalls.begin(), m_lstCalls.end(), 
			std::bind2nd(fsc_srch(), ~LINECALLSTATE_IDLE)) != m_lstCalls.end())
		return false;

	m_AddressStatus.dwNumInUse = 0;
	OnAddressStateChange(LINEADDRESSSTATE_INUSEZERO);
	return true;

}// CTSPIAddressInfo::NotifyInUseZero

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetCallInfo
//
// Return the call information object based on a position in our
// list.
//
CTSPICallAppearance* CTSPIAddressInfo::GetCallInfo(int iPos) const
{
	CEnterCode sLock(this);  // Synch access to object
	TCallList::const_iterator pos, end = m_lstCalls.end();
	for (pos = m_lstCalls.begin(); 
		 pos != end && iPos > 0; ++pos, --iPos);
	return (pos == m_lstCalls.end()) ? NULL : (*pos);

}// CTSPIAddressInfo::GetCallInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::FindAttachedCall
//
// Returns a pointer to the call which is attached to the specified
// call.  This allows a link of attached calls.
//
// Note: the call must be on the same address/line.
//
CTSPICallAppearance* CTSPIAddressInfo::FindAttachedCall(CTSPICallAppearance* pSCall) const
{
	CEnterCode sLock(this);  // Synch access to object
	TCallList::const_iterator it = std::find_if(m_lstCalls.begin(), m_lstCalls.end(), 
					std::bind2nd(fac_srch(), pSCall));
	return (it != m_lstCalls.end()) ? (*it) : NULL;

}// CTSPIAddressInfo::FindAttachedCall

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::FindCallByState
//
// Returns a pointer to the call with the specified call state
//
CTSPICallAppearance* CTSPIAddressInfo::FindCallByState(DWORD dwState) const
{
	CEnterCode sLock(this);  // Synch access to object
	TCallList::const_iterator it = std::find_if(m_lstCalls.begin(), m_lstCalls.end(), 
					std::bind2nd(fsc_srch(), dwState));
	return (it != m_lstCalls.end()) ? (*it) : NULL;

}// CTSPIAddressInfo::FindCallByState

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::FindCallByCallID
//
// Locate a call by the dwCallID field in CALLINFO.  This function may
// be used by providers to match up a call appearance on an address to
// a device call which has been identified and placed into the dwCallID field
// of the CALLINFO record.
//
// This searches only the current address.  There is a 
// CTSPIDevice::FindCallByCallID function which goes across all lines.
//
CTSPICallAppearance* CTSPIAddressInfo::FindCallByCallID (DWORD dwCallID) const
{
	CEnterCode sLock(this);  // Synch access to object
	TCallList::const_iterator it = std::find_if(m_lstCalls.begin(), m_lstCalls.end(), 
					std::bind2nd(fci_srch(), dwCallID));
	return (it != m_lstCalls.end()) ? (*it) : NULL;

}// CTSPIAddressInfo::FindCallByCallID

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::FindCallByHandle
//
// Locate a call appearance by the handle used in TAPI
//
CTSPICallAppearance* CTSPIAddressInfo::FindCallByHandle(HTAPICALL htCall) const
{
	// If we are monitoring the addess then we don't report the calls to
	// TAPI and thus will not have a call handle.
	if (m_AddressCaps.dwAddressSharing == LINEADDRESSSHARING_MONITORED)
		return NULL;

	CEnterCode sLock(this);  // Synch access to object
	TCallList::const_iterator it = std::find_if(m_lstCalls.begin(), m_lstCalls.end(), 
					std::bind2nd(fch_srch(), htCall));
	return (it != m_lstCalls.end()) ? (*it) : NULL;

}// CTSPIAddressInfo::FindCallByHandle

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnPreCallStateChange
//
// A call appearance on this address is about to change states.
//
void CTSPIAddressInfo::OnPreCallStateChange (CTSPICallAppearance* pCall, DWORD dwNewState, DWORD dwOldState)
{
	// If the state has not changed, ignore.
	if (dwNewState == dwOldState)
		return;

	CEnterCode sLock(this);  // Synch access to object

    // Determine if the number of active calls has changed.
    bool fWasActive = CTSPICallAppearance::IsActiveCallState(dwOldState);
    bool fIsActive  = CTSPICallAppearance::IsActiveCallState(dwNewState);
	bool fWasConnected = CTSPICallAppearance::IsConnectedCallState(dwOldState);
	bool fIsConnected = CTSPICallAppearance::IsConnectedCallState(dwNewState);

	_TSP_DTRACE(_T("%s: OnPreCallStateChange Call=0x%lx Old=0x%lx, New=0x%lx, WasActive=%d, IsActive=%d, WasConnected=%d, IsConnected=%d\n"),
					m_strAddress.c_str(), pCall, dwOldState, dwNewState, fWasActive, fIsActive, fWasConnected, fIsConnected);

	// If the call is now active, but wasn't before, then our bump active call count is up by one.
	if (fWasActive == false && fIsActive == true)
	{                                   
		m_AddressStatus.dwNumActiveCalls++;
		m_dwFlags |= NotifyNumCalls;
	}

	// Or if the number of active calls has gone down
	else if (fWasActive == true && fIsActive == false)
	{
		m_AddressStatus.dwNumActiveCalls--;
		if (m_AddressStatus.dwNumActiveCalls & 0x80000000)
			m_AddressStatus.dwNumActiveCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
	}       

	// Count the active calls.  This is used to determine the bandwidth of the TSP.
	if (fWasConnected == false && fIsConnected == true)
	{
		++m_dwConnectedCallCount;
		GetLineOwner()->OnConnectedCallCountChange(this, 1);
	}
	else if (fWasConnected == true && fIsConnected == false)
	{
		if (--m_dwConnectedCallCount & 0x80000000)
			m_dwConnectedCallCount = 0;
		GetLineOwner()->OnConnectedCallCountChange(this, -1);
	}

	// Determine if the HOLD status has changed.        
	if (dwNewState == LINECALLSTATE_ONHOLD && dwOldState != dwNewState)
	{
		++m_AddressStatus.dwNumOnHoldCalls;
		m_dwFlags |= NotifyNumCalls;
	}
	else if ((dwNewState == LINECALLSTATE_ONHOLDPENDTRANSFER ||
			  dwNewState == LINECALLSTATE_ONHOLDPENDCONF) &&
			 (dwOldState != LINECALLSTATE_ONHOLDPENDTRANSFER &&
			  dwOldState != LINECALLSTATE_ONHOLDPENDCONF))
	{
		++m_AddressStatus.dwNumOnHoldPendCalls;
		m_dwFlags |= NotifyNumCalls;
	}

	if (dwOldState == LINECALLSTATE_ONHOLD && dwNewState != dwOldState)
	{
		if (--m_AddressStatus.dwNumOnHoldCalls & 0x80000000)
			m_AddressStatus.dwNumOnHoldCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
	}
	else if ((dwOldState == LINECALLSTATE_ONHOLDPENDTRANSFER ||
			  dwOldState == LINECALLSTATE_ONHOLDPENDCONF) &&
			 (dwNewState != LINECALLSTATE_ONHOLDPENDTRANSFER &&
			  dwNewState != LINECALLSTATE_ONHOLDPENDCONF))
	{
		if (--m_AddressStatus.dwNumOnHoldPendCalls & 0x80000000)
			m_AddressStatus.dwNumOnHoldPendCalls = 0L;
		m_dwFlags |= NotifyNumCalls;
	}

	// Unlock our address
	sLock.Unlock();

#ifdef _DEBUG	
	if (dwNewState != dwOldState)
		_TSP_DTRACEX(TRC_STATS, _T("%s: Address Call Counts Active=%ld, OnHold=%ld,  OnHoldPend=%ld\n"),
				m_strAddress.c_str(), m_AddressStatus.dwNumActiveCalls, m_AddressStatus.dwNumOnHoldCalls, m_AddressStatus.dwNumOnHoldPendCalls);            
#endif

    // Tell our line owner about the call changing state.
    GetLineOwner()->OnPreCallStateChange(this, pCall, dwNewState, dwOldState);

}// CTSPIAddressInfo::OnPreCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnCallStateChange
//
// A call appearance on this address has changed states.  Potentially
// send out an address state change.
//
void CTSPIAddressInfo::OnCallStateChange (CTSPICallAppearance* pCall, DWORD dwNewState, DWORD dwOldState)
{
	// The LINEADDRESSSTATE_NUMCALLS should be sent whenever any of the LINEADDRESSSTATUS
	// dwNumXXXX fields have changed. If this has happened, send the notification.
    if (m_dwFlags & NotifyNumCalls)
	{
        OnAddressStateChange (LINEADDRESSSTATE_NUMCALLS);
		m_dwFlags &= ~NotifyNumCalls;
	}

	// Recalculate our address features if this isn't a monitored address
	if (m_AddressCaps.dwAddressSharing != LINEADDRESSSHARING_MONITORED)
		RecalcAddrFeatures();

    // Tell our line owner about the call changing state.
    GetLineOwner()->OnCallStateChange(this, pCall, dwNewState, dwOldState);

	// Send the INUSEZERO notification if necessary
	NotifyInUseZero();

}// CTSPIAddressInfo::OnCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GatherCapabilities
//
// Gather all the address capabilities and store them off into an
// ADDRESSCAPS structure for TAPI.
//
LONG CTSPIAddressInfo::GatherCapabilities (
DWORD dwTSPIVersion,                   // Version of SPI expected
DWORD /*dwExtVersion*/,                // Driver specific version
LPLINEADDRESSCAPS lpAddressCaps)       // Address CAPS structure to fill in
{
	_TSP_ASSERTE (dwTSPIVersion != 0);
	CEnterCode sLock(this);  // Synch access to object

	// Grab the dialable address.  If there isn't one, or we are not to report the 
	// DN to the upper level (possibly because it is dynamic), then report a
	// static entity based on our address position.
	TString strAddr = GetDialableAddress();
	if (strAddr.empty() || (m_dwFlags & NotUseDN) == NotUseDN)
	{
		TCHAR chBuff[20];
		wsprintf(chBuff, _T("Address %ld"), m_dwAddressID);
		strAddr = chBuff;
	}

	// Get the length of the device classes we support.
	TString strDeviceNames; TDeviceClassArray::const_iterator ii;
	for (ii = m_arrDeviceClass.begin(); 
		 ii != m_arrDeviceClass.end(); ++ii)
	{
		const DEVICECLASSINFO* pDevClass = (*ii).second;
		strDeviceNames += pDevClass->strName + _T('~');
	}

	// Add the DEVICECLASS entries from the owner line object.
	for (ii = GetLineOwner()->m_arrDeviceClass.begin(); 
		 ii !=  GetLineOwner()->m_arrDeviceClass.end(); ++ii)
	{
		const DEVICECLASSINFO* pDevClass = (*ii).second;
		if (strDeviceNames.find(pDevClass->strName) == TString::npos)
			strDeviceNames += pDevClass->strName + _T('~');
	}

	// If we have device class entries, then add up the total length of 
	// all the string keys for TAPI.
	if (!strDeviceNames.empty())
		strDeviceNames += _T('~');

	// Get the preliminary sizes for all the structures.
	m_AddressCaps.dwTotalSize = lpAddressCaps->dwTotalSize;
	m_AddressCaps.dwLineDeviceID = GetLineOwner()->GetDeviceID();
	m_AddressCaps.dwNeededSize = sizeof(LINEADDRESSCAPS);

	// Make sure we have enough space.  Note that we must subtract the address
	// features which were added in TAPI 2.0 for backward compatibility.
	DWORD dwReqSize = sizeof(LINEADDRESSCAPS);
	if (dwTSPIVersion < TAPIVER_20)
		dwReqSize -= sizeof(DWORD) * 12;
	if (dwTSPIVersion < TAPIVER_14)
		dwReqSize -= sizeof(DWORD);

	// Copy over our STATIC version of the capabilities.
	MoveMemory (lpAddressCaps, &m_AddressCaps, dwReqSize);
	lpAddressCaps->dwUsedSize = dwReqSize;

	// Remove the things which are not available in previous versions depending on 
	// the negotiation level for the calling application.  Note that we can get older
	// TAPI apps calling us.
	if (dwTSPIVersion < TAPIVER_20)
	{
		lpAddressCaps->dwAddrCapFlags &= ~(LINEADDRCAPFLAGS_PREDICTIVEDIALER | 
										   LINEADDRCAPFLAGS_QUEUE | 
										   LINEADDRCAPFLAGS_ROUTEPOINT |
										   LINEADDRCAPFLAGS_HOLDMAKESNEW |
										   LINEADDRCAPFLAGS_NOINTERNALCALLS |
										   LINEADDRCAPFLAGS_NOEXTERNALCALLS |
										   LINEADDRCAPFLAGS_SETCALLINGID);
		lpAddressCaps->dwAddressFeatures &= ~(LINEADDRFEATURE_PICKUPHELD |
										   LINEADDRFEATURE_PICKUPGROUP |
										   LINEADDRFEATURE_PICKUPDIRECT |
										   LINEADDRFEATURE_PICKUPWAITING |
										   LINEADDRFEATURE_FORWARDFWD |
										   LINEADDRFEATURE_FORWARDDND);
		lpAddressCaps->dwCallFeatures &= ~(LINECALLFEATURE_SETTREATMENT |
										   LINECALLFEATURE_SETQOS |
										   LINECALLFEATURE_SETCALLDATA);
		lpAddressCaps->dwCallInfoStates &= ~(LINECALLINFOSTATE_TREATMENT |
											LINECALLINFOSTATE_QOS |
											LINECALLINFOSTATE_CALLDATA);
		lpAddressCaps->dwDisconnectModes &= ~(LINEDISCONNECTMODE_NUMBERCHANGED |
											LINEDISCONNECTMODE_OUTOFORDER |
											LINEDISCONNECTMODE_TEMPFAILURE |
											LINEDISCONNECTMODE_QOSUNAVAIL |
											LINEDISCONNECTMODE_BLOCKED |
											LINEDISCONNECTMODE_DONOTDISTURB | 
											LINEDISCONNECTMODE_CANCELLED);
	}

	// Strip out unsupporting additions added to Win95.
	if (dwTSPIVersion < TAPIVER_14)
	{
    	lpAddressCaps->dwAddressStates &= ~LINEADDRESSSTATE_CAPSCHANGE;
		lpAddressCaps->dwCallFeatures &= ~LINECALLFEATURE_RELEASEUSERUSERINFO;
    	lpAddressCaps->dwDisconnectModes &= ~LINEDISCONNECTMODE_NODIALTONE;
		lpAddressCaps->dwForwardModes &= ~(LINEFORWARDMODE_UNKNOWN | LINEFORWARDMODE_UNAVAIL);
	}

	// Add the string if we have the space.
	AddDataBlock(lpAddressCaps, lpAddressCaps->dwAddressOffset, lpAddressCaps->dwAddressSize, strAddr.c_str());

	// Add the completion messages to the structure. Note that each message is required to be 
	// identical in size so we pad it out with spaces.
	if (lpAddressCaps->dwNumCompletionMessages > 0)
	{   
		lpAddressCaps->dwCompletionMsgTextSize = 0L;
		unsigned int iTextEntryLen = ((lpAddressCaps->dwCompletionMsgTextEntrySize - sizeof(wchar_t)) / sizeof(wchar_t));
		for (TStringArray::const_iterator it = m_arrCompletionMsgs.begin(); it != m_arrCompletionMsgs.end(); ++it)
		{   
			TString strBuff = (*it);
			strBuff.resize(iTextEntryLen, _T(' '));
			AddDataBlock(lpAddressCaps, lpAddressCaps->dwCompletionMsgTextOffset, lpAddressCaps->dwCompletionMsgTextSize, strBuff.c_str());
		}
	}

	// Add the call treatment information if we have the space.
	if (dwTSPIVersion >= TAPIVER_20)
	{
		if (lpAddressCaps->dwNumCallTreatments > 0)
		{
			_TSP_ASSERTE(m_mapCallTreatment.size() == lpAddressCaps->dwNumCallTreatments);
			LINECALLTREATMENTENTRY* plctTop = new LINECALLTREATMENTENTRY[lpAddressCaps->dwNumCallTreatments];
			LINECALLTREATMENTENTRY* plctCurr = plctTop;
			if (plctTop == NULL)
				return LINEERR_NOMEM;

			// Copy the LINECALLTREATMENTENTRY array information into place.  
			// Our string names get placed directly after the array information.
			TMapDWordToString::iterator itMap;
			for (itMap = m_mapCallTreatment.begin(); 
				 itMap != m_mapCallTreatment.end(); ++itMap)
			{
				DWORD dwKey = (*itMap).first;
				TString strValue = (*itMap).second;
				plctCurr->dwCallTreatmentID = dwKey;
				plctCurr->dwCallTreatmentNameOffset = 0;
				plctCurr->dwCallTreatmentNameSize = 0;
				++plctCurr;
			}				

			// Add it to our buffer.
			if (AddDataBlock(lpAddressCaps, lpAddressCaps->dwCallTreatmentListOffset, lpAddressCaps->dwCallTreatmentListSize, plctTop, 
					lpAddressCaps->dwNumCallTreatments*sizeof(LINECALLTREATMENTENTRY)))
				plctCurr = reinterpret_cast<LINECALLTREATMENTENTRY*>
						(reinterpret_cast<LPBYTE>(lpAddressCaps) + lpAddressCaps->dwCallTreatmentListOffset);
			else
				plctCurr = NULL;

			// Delete our temp buffer.
			delete [] plctTop;

			// Copy the string array information into place.  
			for (itMap = m_mapCallTreatment.begin(); itMap != m_mapCallTreatment.end(); ++itMap)
			{
				TString strValue = (*itMap).second;
				DWORD dwSize = 0, dwOffset = 0;
				if (AddDataBlock(lpAddressCaps, dwOffset, dwSize, strValue.c_str()) && plctCurr)
				{
					plctCurr->dwCallTreatmentNameOffset = dwOffset;
					plctCurr->dwCallTreatmentNameSize = dwSize;
					++plctCurr;
				}
			}				
		}

		// If we have some lineGetID supported device classes,
		// return the list of supported device classes.
		if (!strDeviceNames.empty())
		{
			if (AddDataBlock(lpAddressCaps, lpAddressCaps->dwDeviceClassesOffset, lpAddressCaps->dwDeviceClassesSize, strDeviceNames.c_str()))
			{
				// Strip out the ~ and replace with nulls.
				wchar_t* pbd = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpAddressCaps) + lpAddressCaps->dwDeviceClassesOffset);
				std::transform(pbd, pbd+lpAddressCaps->dwDeviceClassesSize,pbd, tsplib::substitue<wchar_t>(L'~',L'\0'));
			}
		}
	}
    return 0L;

}// CTSPIAddressInfo::GatherCapabilities

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GatherStatusInformation
//
// Gather all the status information for this address.
//
LONG CTSPIAddressInfo::GatherStatusInformation (LPLINEADDRESSSTATUS lpAddressStatus)
{
	CEnterCode sLock(this);  // Synch access to object

	// Copy the basic structure over.
	m_AddressStatus.dwTotalSize = lpAddressStatus->dwTotalSize;
	m_AddressStatus.dwNeededSize = sizeof(LINEADDRESSSTATUS);
	m_AddressStatus.dwUsedSize = m_AddressStatus.dwNeededSize;
	MoveMemory(lpAddressStatus, &m_AddressStatus, m_AddressStatus.dwUsedSize);

	// If we have room for forwarding information, then include it.
	lpAddressStatus->dwForwardNumEntries = m_arrForwardInfo.size();
	if (lpAddressStatus->dwForwardNumEntries > 0)
	{
		LINEFORWARD* lpForwardTop = new LINEFORWARD[m_arrForwardInfo.size()];
		LINEFORWARD* lpForwardCurr = lpForwardTop;
		if (lpForwardTop == NULL)
			return LINEERR_NOMEM;

		// Copy over the structures to our new block which will be returned in the status structure.
		TForwardInfoArray::iterator it;
		for (it = m_arrForwardInfo.begin(); it != m_arrForwardInfo.end(); ++it, ++lpForwardCurr)
		{
			ZeroMemory(lpForwardCurr, sizeof(LINEFORWARD));
			lpForwardCurr->dwForwardMode = (*it)->dwForwardMode;
			lpForwardCurr->dwDestCountryCode = (*it)->dwDestCountry;
		}

		// Copy the forwarding entries over.
		if (AddDataBlock(lpAddressStatus, lpAddressStatus->dwForwardOffset, lpAddressStatus->dwForwardSize, 
						 lpForwardTop, sizeof(LINEFORWARD)*m_arrForwardInfo.size()))
		{
			// Now point to the block within the address object to copy names over.
			lpForwardCurr = reinterpret_cast<LINEFORWARD*>(reinterpret_cast<LPBYTE>(lpAddressStatus) + lpAddressStatus->dwForwardOffset);

			// Copy the string buffers for the forwarding information over.
			for (it = m_arrForwardInfo.begin(); it != m_arrForwardInfo.end(); ++it)
			{
				// Copy the caller information if available.  Multiple addresses
				// may be strung together in the standard dialable format.  We
				// don't include the NAME or ISDN sub address information we might
				// have pulled out of the original request - only the dialable string.
				TString strFinalBuffer = std::accumulate((*it)->arrCallerAddress.begin(), (*it)->arrCallerAddress.end(), TString(_T("")), distr_concat);

				// Add the caller information to the forwarding buffer at the end.
				if (!strFinalBuffer.empty())
					AddDataBlock(lpAddressStatus, lpForwardCurr->dwCallerAddressOffset,
							lpForwardCurr->dwCallerAddressSize, strFinalBuffer.c_str());

				// Copy the destination address information if available.
				strFinalBuffer = std::accumulate((*it)->arrDestAddress.begin(), (*it)->arrDestAddress.end(), TString(_T("")), distr_concat);

				// Add the destination address to the forwarding buffer at the end.
				if (!strFinalBuffer.empty())
					AddDataBlock(lpAddressStatus, lpForwardCurr->dwDestAddressOffset,
							lpForwardCurr->dwDestAddressSize, strFinalBuffer.c_str());
			}    
		}

		delete [] lpForwardTop;
	}

	// If we have room for terminal entries, then include them.
	if (!m_arrTerminals.empty())
	{
		for (TDWordArray::const_iterator it = m_arrTerminals.begin(); it != m_arrTerminals.end(); ++it)
		{
			DWORD dwValue = (*it);
			AddDataBlock(lpAddressStatus, lpAddressStatus->dwTerminalModesOffset, 
				lpAddressStatus->dwTerminalModesSize, &dwValue, sizeof(DWORD));
		}
	}

    return 0L;

}// CTSPIAddressInfo::GatherStatusInformation

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::Unpark
//
// This function retrieves the call parked at the specified address and
// returns a call handle to it.
//
LONG CTSPIAddressInfo::Unpark (
DRV_REQUESTID dwRequestID,          // Asynch. request id.
HTAPICALL htCall,                   // New unparked call
LPHDRVCALL lphdCall,                // Return address for unparked call
TDialStringArray* parrAddresses)    // Array of addresses to unpark from
{
	// Don't allow call-control on a monitored address
	if (m_AddressCaps.dwAddressSharing == LINEADDRESSSHARING_MONITORED)
		return LINEERR_OPERATIONUNAVAIL;

    // Verify we have an address to unpark from.
    if (parrAddresses == NULL || parrAddresses->empty())
        return LINEERR_INVALADDRESS;

    // Create a new call on the address specified.
    CTSPICallAppearance* pCall = CreateCallAppearance(htCall, 0, 
				LINECALLORIGIN_UNKNOWN, LINECALLREASON_UNPARK);
    _TSP_ASSERTE (pCall != NULL);

    // Pass the request down to the new call.
    LONG lResult = pCall->Unpark (dwRequestID, parrAddresses);
    if (lResult == static_cast<LONG>(dwRequestID) || lResult == 0)
        *lphdCall = (HDRVCALL) pCall;
    else
        RemoveCallAppearance(pCall);
    return lResult;

}// CTSPIAddressInfo::Unpark

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetStatusMessages
//
// Set the state notifications to tell TAPI about
//
void CTSPIAddressInfo::SetStatusMessages (DWORD dwStates)
{ 
    m_dwAddressStates = dwStates;

}// CTSPIAddressInfo::SetStatusMessages

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetNumRingsNoAnswer
//
// Set the number of rings before a call is considered a "no answer".
//
void CTSPIAddressInfo::SetNumRingsNoAnswer (DWORD dwNumRings)
{ 
    m_AddressStatus.dwNumRingsNoAnswer = dwNumRings;
    OnAddressStateChange (LINEADDRESSSTATE_FORWARD);

}// CTSPIAddressInfo::SetNumRingsNoAnswer

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetCurrentRate
//
// Set the current data rate
//
void CTSPIAddressInfo::SetCurrentRate (DWORD dwRate)
{                                   
    m_dwCurrRate = dwRate;
    
}// CTSPIAddressInfo::SetCurrentRate

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::GetID
//
// Manage device-level requests for information based on a device id.
//
LONG CTSPIAddressInfo::GetID (const TString& strDevClass, 
							LPVARSTRING lpDeviceID, HANDLE hTargetProcess)
{
	DEVICECLASSINFO* pDeviceClass = GetDeviceClass(strDevClass.c_str());
	if (pDeviceClass == NULL)
		return GetLineOwner()->GetID(strDevClass, lpDeviceID, hTargetProcess);
	return GetSP()->CopyDeviceClass (pDeviceClass, lpDeviceID, hTargetProcess);

}// CTSPIAddressInfo::GetID

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::Pickup
//
// This function picks up a call alerting at the specified destination
// address and returns a call handle for the picked up call.  If invoked
// with a NULL for the 'lpszDestAddr' parameter, a group pickup is performed.
// If required by the device capabilities, 'lpszGroupID' specifies the
// group ID to which the alerting station belongs.
//
LONG CTSPIAddressInfo::Pickup (
DRV_REQUESTID dwRequestID,      // Asynch request id.   
HTAPICALL htCall,               // New call handle
LPHDRVCALL lphdCall,            // Return address for call handle
TDialStringArray* parrDial,		// Dial array
LPCTSTR pszGroupID)				// GroupID for pickup
{   
	// Make sure we can perform this function now.
	if ((GetAddressStatus()->dwAddressFeatures & 
			(LINEADDRFEATURE_PICKUP|LINEADDRFEATURE_PICKUPHELD |
			 LINEADDRFEATURE_PICKUPGROUP|LINEADDRFEATURE_PICKUPDIRECT|
			 LINEADDRFEATURE_PICKUPWAITING)) == 0)
		return LINEERR_OPERATIONUNAVAIL;
	
    // If we require a group id, and one is not supplied, give an error.
    if ((m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_PICKUPGROUPID) &&
         (pszGroupID == NULL || *pszGroupID == _T('\0'))) 
		return LINEERR_INVALGROUPID;         

    // Create or locate existing call appearance on this address.
    CTSPICallAppearance* pCall = CreateCallAppearance(htCall, 0, 
				LINECALLORIGIN_UNKNOWN, LINECALLREASON_PICKUP);
    _TSP_ASSERTE (pCall != NULL);

    // Pass the request down to the new call.
    LONG lResult = pCall->Pickup (dwRequestID, parrDial, pszGroupID);
    if (lResult == static_cast<LONG>(dwRequestID) || lResult == 0)
        *lphdCall = (HDRVCALL) pCall;
    else
        RemoveCallAppearance(pCall);

    return lResult;

}// CTSPIAddressInfo::Pickup

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetTerminalModes
//
// This is the function which will be called when a lineSetTerminal is
// completed by the derived service provider class.
// This stores or removes the specified terminal from the terminal modes 
// given, and then forces it to happen for any existing calls on the 
// address.
//
void CTSPIAddressInfo::SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal)
{
	CEnterCode sLock(this);  // Synch access to object
	if (iTerminalID < m_arrTerminals.size())
	{
		DWORD dwCurrMode = m_arrTerminals[iTerminalID];
		if (fRouteToTerminal)
			dwCurrMode |= dwTerminalModes;
		else
			dwCurrMode &= ~dwTerminalModes;
		m_arrTerminals[iTerminalID] = dwCurrMode;
	}

	// Notify TAPI about our address state change
	sLock.Unlock();
	OnAddressStateChange(LINEADDRESSSTATE_TERMINALS);
	sLock.Lock();

	// Run through all our call appearances and force them to update their terminal
	// maps as well.  It is assumed that the service provider code already performed
	// the REAL transfer in H/W.
	TCallList::iterator end = m_lstCalls.end();
	for (TCallList::iterator iCall = m_lstCalls.begin(); iCall != end; ++iCall)
		(*iCall)->SetTerminalModes(iTerminalID, dwTerminalModes, fRouteToTerminal);

}// CTSPIAddressInfo::SetTerminalModes

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnTerminalCountChanged
//
// The terminal count has changed, either add or remove a terminal
// entry from our array
//
void CTSPIAddressInfo::OnTerminalCountChanged (bool fAdded, int iPos, DWORD dwMode)
{
	CEnterCode sLock(this);  // Synch access to object

	if (fAdded)
		m_arrTerminals.push_back(dwMode);
	else
		m_arrTerminals.erase(m_arrTerminals.begin() + iPos);

	// Notify TAPI about our address state change
	sLock.Unlock();
	OnAddressStateChange (LINEADDRESSSTATE_TERMINALS);
	sLock.Lock();

	// Run through all our call appearances and force them to update their terminal
	// maps as well.
	TCallList::iterator end = m_lstCalls.end();
	for (TCallList::iterator iCall = m_lstCalls.begin(); iCall != end; ++iCall)
		(*iCall)->OnTerminalCountChanged (fAdded, iPos, dwMode);

}// CTSPIAddressInfo::OnTerminalCountChanged

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetupTransfer
//
// Setup the passed call appearance for a consultation transfer.  This
// allocates a new consultation call and associates it with the passed
// call.
//
LONG CTSPIAddressInfo::SetupTransfer(
DRV_REQUESTID dwRequestID,          // Asynch. request id
CTSPICallAppearance* pCall,			// Call to transfer
HTAPICALL htConsultCall,            // Consultant call to create
LPHDRVCALL lphdConsultCall,         // Return handle for call to create  
LPLINECALLPARAMS lpCallParams)		// LINECALLPARMS for created call
{
	// Verify that the function can be called right now.
	if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETUPTRANSFER) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// If the ONESTEPTRANSFER bit is set, but it is NOT in our call features, exit out.
	if (lpCallParams && (lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_ONESTEPTRANSFER) != 0 &&
		pCall->GetCallStatus()->dwCallFeatures2 > 0 &&
		(pCall->GetCallStatus()->dwCallFeatures2 & LINECALLFEATURE2_ONESTEPTRANSFER) == 0)
		return LINEERR_OPERATIONUNAVAIL;

// 3.044 -- we allow transfers of conferences if the call features say we can do it.
//    // If the call is unavailable because it is a conference, error it.
//    if (pCall->GetCallType() == CTSPICallAppearance::Conference)
//        return LINEERR_OPERATIONUNAVAIL;   

#ifdef STRICT_CALLSTATES
    // Ok, make sure the call state allows this.
    if (pCall->GetCallState() != LINECALLSTATE_CONNECTED)
        return LINEERR_INVALCALLSTATE;
#endif

	// A transfer event causes the call involved to move to the HOLD
	// state - make sure we haven't exceeded the total allowed for
	// holding.
	if ((GetAddressStatus()->dwNumOnHoldCalls >=
		GetAddressCaps()->dwMaxNumOnHoldCalls) ||
		(GetAddressStatus()->dwNumOnHoldPendCalls >=
		GetAddressCaps()->dwMaxNumOnHoldPendingCalls))
		return LINEERR_CALLUNAVAIL;

    // Create the consultation call and associate it with this call appearance.
    DWORD dwCallParamFlags = (lpCallParams != NULL) ? lpCallParams->dwCallParamFlags : 0;
    CTSPICallAppearance* pConsultCall = CreateCallAppearance(htConsultCall, dwCallParamFlags, 
                                         LINECALLORIGIN_OUTBOUND, LINECALLREASON_DIRECT);
    _TSP_ASSERTE(pConsultCall != NULL);

    // Attach the two calls.
	pCall->SetConsultationCall(pConsultCall);

	// Create our transfer request object
	RTSetupTransfer* pRequest = new RTSetupTransfer(pCall, dwRequestID, pConsultCall, lpCallParams);

    // Submit the request.  The worker should look at the related call field
    // and transition the consultant call to the DIALTONE state.   
    if (GetLineOwner()->AddAsynchRequest(pRequest))
    {
        *lphdConsultCall = (HDRVCALL) pConsultCall;
		return static_cast<LONG>(dwRequestID);
    }

    // It failed, kill the new call.
    RemoveCallAppearance (pConsultCall);
    return LINEERR_OPERATIONFAILED;

}// CTSPIAddressInfo::SetupTransfer

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CompleteTransfer
//
// Complete the pending transfer using the specified consultant
// call as the destination.  This can create an additional call appearance
// which then becomes a conference call.
//
LONG CTSPIAddressInfo::CompleteTransfer (
CTSPICallAppearance* pCall,         // Call appearance for this transfer
DRV_REQUESTID dwRequestId,          // Asynch, request id
CTSPICallAppearance* pConsult,      // Specifies the destination of xfer
HTAPICALL htConfCall,               // Conference call handle if needed.
LPHDRVCALL lphdConfCall,            // Return SP handle for conference
DWORD dwTransferMode)               // Transfer mode
{
	// Verify that the function can be called right now.
	if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_COMPLETETRANSF) == 0)
		return LINEERR_OPERATIONUNAVAIL;

	// You should specify *some* transfer mode.  Validate the passed
	// transfer mode request based on the types we support
	_TSP_ASSERTE(m_AddressCaps.dwTransferModes != 0);
	if (m_AddressCaps.dwTransferModes > 0 &&
		(m_AddressCaps.dwTransferModes & dwTransferMode) != dwTransferMode)
		return LINEERR_INVALTRANSFERMODE;

    CTSPIConferenceCall* pConfCall = NULL;

#ifdef STRICT_CALLSTATES
    // Verify the call state of the consultant call.  Note that the call may not
    // be the same as the one we created during SetupTransfer.  It is possible
    // to drop/deallocate the original call and release a held call in order to
    // transfer to another line.
	DWORD dwCallState = pConsult->GetCallState();
    if ((dwCallState & (LINECALLSTATE_CONNECTED |
				LINECALLSTATE_RINGBACK |
				LINECALLSTATE_BUSY |
				LINECALLSTATE_PROCEEDING)) == 0)
        return LINEERR_INVALCALLSTATE;
#endif
                                
    // Make sure it is the call we created if we don't support the creation of
    // a new call.
	if ((m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERMAKE) == 0 &&
		 (pConsult->GetCallType() != CTSPICallAppearance::Conference &&
		  pConsult->GetAttachedCall() != pCall))
		return LINEERR_INVALCONSULTCALLHANDLE;
                                
	// If the transfer mode is a normal directed transfer..
	if (dwTransferMode == LINETRANSFERMODE_TRANSFER)
	{
		if ((pCall->GetCallState() == LINECALLSTATE_ONHOLD &&
				m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_TRANSFERHELD) ||
			pCall->GetCallState() == LINECALLSTATE_ONHOLDPENDTRANSFER)
			;
		else
			return LINEERR_INVALCALLSTATE;
	}

    // If the transfer mode is conference, then create a conference call
    // to return.  This indicates to transition the call into a three-way
    // conference call.
	else // if (dwTransferMode == LINETRANSFERMODE_CONFERENCE)
	{
        if (lphdConfCall == NULL)
            return LINEERR_INVALPOINTER;

        pConfCall = CreateConferenceCall(htConfCall);
        _TSP_ASSERTE(pConfCall != NULL);
        
        // Attach both calls to the conference call.  They should both transition
        // to CONFERENCED and be automatically added to our conference.
        pCall->SetConferenceOwner(pConfCall);
        pConsult->SetConferenceOwner(pConfCall);
    }

	// Create our request object.
	RTCompleteTransfer* pRequest = new RTCompleteTransfer(pCall, dwRequestId, pConsult,
						pConfCall, dwTransferMode);

    // Submit the request.
    if (GetLineOwner()->AddAsynchRequest(pRequest))
    {   
        if (lphdConfCall != NULL && pConfCall)
            *lphdConfCall = (HDRVCALL)pConfCall;
		return static_cast<LONG>(dwRequestId);
    }

    // Remove it if we failed.
    if (pConfCall)
        RemoveCallAppearance (pConfCall);
    return LINEERR_OPERATIONFAILED;

}// CTSPIAddressInfo::CompleteTransfer

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetupConference
//
// This function creates a new conference either from an existing 
// call or a new call.
//
LONG CTSPIAddressInfo::SetupConference (                                       
DRV_REQUESTID dwRequestID,          // Asynch Request id.
CTSPICallAppearance* pCall,			// Initial call for conference
HTAPICALL htConfCall,               // New conference call TAPI handle
LPHDRVCALL lphdConfCall,            // Returning call handle
HTAPICALL htConsultCall,            // New consultation call TAPI handle
LPHDRVCALL lphdConsultCall,         // Returning call handle
DWORD dwNumParties,					// Count of initial parties
LPLINECALLPARAMS lpCallParams)		// Line parameters
{
	// If we have an initial call, then it must be connected.
	if (pCall != NULL)
	{   
		// Verify that the function can be called right now.
		if ((pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETUPCONF) == 0)
			return LINEERR_OPERATIONUNAVAIL;

		// If the NOHOLDCONFERENCE bit is set, but it is NOT in our call features, exit out.
		if (lpCallParams && (lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_NOHOLDCONFERENCE) != 0 &&
			pCall->GetCallStatus()->dwCallFeatures2 > 0 &&
			(pCall->GetCallStatus()->dwCallFeatures2 & LINECALLFEATURE2_NOHOLDCONFERENCE) == 0)
			return LINEERR_OPERATIONUNAVAIL;

		// If we are not supposed to have a call appearance during setup, 
		// then fail this call.
		if (m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_SETUPCONFNULL)
			return LINEERR_INVALCALLHANDLE;

#ifdef STRICT_CALLSTATES
		if (pCall->GetCallState() != LINECALLSTATE_CONNECTED)
			return LINEERR_INVALCALLSTATE;
#endif
	}   
	
	// No explicit starting call appearance.
	else 
	{
		// Make sure we can perform this function now.
		if ((GetAddressStatus()->dwAddressFeatures & LINEADDRFEATURE_SETUPCONF) == 0)
			return LINEERR_OPERATIONUNAVAIL;

		// We don't have a call appearance to start the conference on, verify
		// that the capabilities don't require one.
		if ((m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_SETUPCONFNULL) == 0)
			return LINEERR_INVALCALLHANDLE;
	}

	// Verify the count we are getting against our maximum allowed in a conference.
	if (dwNumParties > m_AddressCaps.dwMaxNumConference)
		return LINEERR_INVALPARAM;

    // Create the conference call we are going to master this with.
    CTSPIConferenceCall* pConfCall = CreateConferenceCall (htConfCall);
    _TSP_ASSERTE(pConfCall != NULL);

    // Create the consultation call which will be associated with the
    // initial call -OR- be a new call altogether on the address.
    CTSPICallAppearance* pConsultCall = CreateCallAppearance(htConsultCall, 0,
                   LINECALLORIGIN_OUTBOUND, LINECALLREASON_DIRECT);
    _TSP_ASSERTE(pConsultCall != NULL);
    
    // Attach the consultation call to the conference.
	// It will be formally attached to the conference once the AddToConf is called.
	pConfCall->SetConsultationCall(pConsultCall);

    // Attach the original call to the conference call.  With this attachment,
    // when the call enters the "CONFERENCED" state, the conference call will
    // automatically add it to the conferencing array.
    if (pCall != NULL)
        pCall->SetConferenceOwner(pConfCall);

	// Allocate our request for this setup.
	RTSetupConference* pRequest = new RTSetupConference(pConfCall, pCall, 
		pConsultCall, dwRequestID, dwNumParties, lpCallParams);

    // Submit the request via the conference call.
    if (GetLineOwner()->AddAsynchRequest(pRequest))
    {
        *lphdConfCall = (HDRVCALL) pConfCall;
        *lphdConsultCall = (HDRVCALL) pConsultCall;
		return static_cast<LONG>(dwRequestID);
    }

	// Breakdown our initial conference setup.
	pCall->SetConferenceOwner(NULL);
	RemoveCallAppearance(pConfCall);
	RemoveCallAppearance(pConsultCall);
    return LINEERR_OPERATIONFAILED;

}// CTSPIAddressInfo::SetupConference

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddCompletionMessage
//
// Add a new completion message to the address information.
// TAPI is NOT notified.  This should be done at INIT time when each
// of the addresses are being added.
//
void CTSPIAddressInfo::AddCompletionMessage (LPCTSTR pszBuff)
{   
	CEnterCode sLock(this);  // Synch access to object

	m_arrCompletionMsgs.push_back(pszBuff); // can throw

	unsigned int cbLen = (lstrlen(pszBuff)+1) * sizeof(wchar_t);
	if (m_AddressCaps.dwCompletionMsgTextEntrySize < cbLen)
		m_AddressCaps.dwCompletionMsgTextEntrySize = cbLen;
    
	m_AddressCaps.dwNumCompletionMessages++;                   
	m_AddressCaps.dwCompletionMsgTextSize = (m_AddressCaps.dwCompletionMsgTextEntrySize * m_AddressCaps.dwNumCompletionMessages);

}// CTSPIAddressInfo::AddCompletionMessage

////////////////////////////////////////////////////////////////////////////
// CTSPIAddress::CanForward
//
// This function is called to verify that this address can forward
// given the specified forwarding information.  All addresses being
// forwarded in a group will be given a chance to check the forwarding
// request before the "Forward" function is actually invoked to insert
// the asynch. request.
//
LONG CTSPIAddressInfo::CanForward(TForwardInfoArray* parrForwardInfo, 
								  LPDWORD pdwNumRings, int iCount)
{
	_TSP_ASSERTE(pdwNumRings != NULL);

	// If we need to establish a consultation call, and this is an
	// "all forward" request, then fail it since each address will require
	// its own consultation call.  The only exception is if this is the only address.
	if (iCount > 1 && m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_FWDCONSULT) 
		return LINEERR_RESOURCEUNAVAIL;

	// Do some checks on the forwarding list.
	if (parrForwardInfo != NULL)
	{
		// If the forwarding list exceeds what our derived class expects, fail it.
		if (parrForwardInfo->size() > static_cast<int>(m_AddressCaps.dwMaxForwardEntries))
			return LINEERR_INVALPARAM;

		// Run through the forwarding list and verify that we support the different
		// forwarding modes being asked for.
		unsigned int iFwdCount = 0;
		for (unsigned int i = 0; i < parrForwardInfo->size(); i++)
		{
			TSPIFORWARDINFO* lpForward = (*parrForwardInfo)[i];
			if ((lpForward->dwForwardMode & m_AddressCaps.dwForwardModes) == 0)
				return LINEERR_INVALPARAM;
			iFwdCount += lpForward->arrCallerAddress.size();
		}                
                              
		// If the specific entries exceeds our list, fail it.                                  
		if (iFwdCount > static_cast<int>(m_AddressCaps.dwMaxSpecificEntries))
			return LINEERR_INVALPARAM;                       
	}

	// Adjust the ring count if it falls outside our range.        
	if ((m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_FWDNUMRINGS) == 0)
		*pdwNumRings = 0;
	else if (*pdwNumRings < m_AddressCaps.dwMinFwdNumRings)
		*pdwNumRings = m_AddressCaps.dwMinFwdNumRings;
	else if (*pdwNumRings > m_AddressCaps.dwMaxFwdNumRings)
		*pdwNumRings = m_AddressCaps.dwMaxFwdNumRings;
    
    // Everything looks ok.
    return 0L;        

}// CTSPIAddress::CanForward

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::Forward
//
// Forward this address to another address.  This can also UNFORWARD.
// This list of forwarding instructions REPLACES any existing forwarding
// information.
//
LONG CTSPIAddressInfo::Forward (
DRV_REQUESTID dwRequestId,				// Asynchronous request id
TForwardInfoArray* parrForwardInfo,		// Array of TSPIFORWARDINFO elements
DWORD dwNumRingsNoAnswer,				// Number of rings before "no answer"
HTAPICALL htConsultCall,				// New TAPI call handle if necessary
LPHDRVCALL lphdConsultCall,				// Our return call handle if needed
LPLINECALLPARAMS lpCallParams)			// Used if creating a new call
{
	// Make sure we can perform this function now.
	if ((GetAddressStatus()->dwAddressFeatures & 
			(LINEADDRFEATURE_FORWARD|LINEADDRFEATURE_FORWARDFWD|
			 LINEADDRFEATURE_FORWARDDND)) == 0)
		return LINEERR_OPERATIONUNAVAIL;

    // If we need to establish a consultation call for forwarding, then do so.
    CTSPICallAppearance* pCall = NULL;
    if (m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_FWDCONSULT) 
    {                                                              
        _TSP_ASSERTE(*lphdConsultCall == NULL);
        
        // If call capabilities were supplied, then use them to create the
        // consultation call.
        DWORD dwCallParamFlags = 0;
        if (lpCallParams != NULL)
            dwCallParamFlags = lpCallParams->dwCallParamFlags;
        pCall = CreateCallAppearance(htConsultCall, dwCallParamFlags, 
                                     LINECALLORIGIN_OUTBOUND, LINECALLREASON_DIRECT);
        pCall->SetCallType (CTSPICallAppearance::Consultant);                           
        *lphdConsultCall = (HDRVCALL) pCall;
    }

	// Create our request object to map this forwarding request.
	RTForward* pRequest = new RTForward(NULL, this, dwRequestId, parrForwardInfo, 
		dwNumRingsNoAnswer, pCall, lpCallParams);

    // Insert the request into our request list.
    if (GetLineOwner()->AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestId);

    // If it fails, remove the call appearance and delete the request.
    *lphdConsultCall = NULL;
    if (pCall != NULL)
        RemoveCallAppearance (pCall);
    return LINEERR_OPERATIONFAILED;

}// CTSPIAddressInfo::Forward

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CanSupportCall
//
// Return zero or an error if the call cannot be supported on this
// address.
//
LONG CTSPIAddressInfo::CanSupportCall (LPLINECALLPARAMS const lpCallParams) const
{   
	// Don't allow calls to be generated via TAPI on this address if it
	// is being monitored -- only calls created directly by the TSP can exist
	// on this type of address.
	if (m_AddressCaps.dwAddressSharing == LINEADDRESSSHARING_MONITORED)
		return LINEERR_CALLUNAVAIL;

    if (!CanSupportMediaModes(lpCallParams->dwMediaMode))
        return LINEERR_INVALMEDIAMODE;
    
    if ((lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_SECURE) &&
            (m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_SECURE) == 0)
        return LINEERR_INVALCALLPARAMS;
            
    if ((lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_BLOCKID) &&
            (m_AddressCaps.dwAddrCapFlags & (LINEADDRCAPFLAGS_BLOCKIDOVERRIDE | LINEADDRCAPFLAGS_BLOCKIDDEFAULT)) == 0)
        return LINEERR_INVALCALLPARAMS;
            
    if ((lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_ORIGOFFHOOK) &&
            (m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_ORIGOFFHOOK) == 0)
        return LINEERR_INVALCALLPARAMS;
            
    if ((lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_DESTOFFHOOK) &&
            (m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_DESTOFFHOOK) == 0)
        return LINEERR_INVALCALLPARAMS;

	// Check the predictive dialer information.
	if (lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_PREDICTIVEDIAL)
	{
		// Address must be a predictive dialer.
		if ((m_AddressCaps.dwAddrCapFlags & LINEADDRCAPFLAGS_PREDICTIVEDIALER) == 0)
			return LINEERR_INVALCALLPARAMS;

		// Check the auto-transfer states.
		if (lpCallParams->dwPredictiveAutoTransferStates > 0 &&
			(lpCallParams->dwPredictiveAutoTransferStates & 
			 m_AddressCaps.dwPredictiveAutoTransferStates) == 0)
			return LINEERR_INVALCALLPARAMS;

		// Adjust the max no answer timeout instead of failing the call
		// outright.
		if (lpCallParams->dwNoAnswerTimeout > m_AddressCaps.dwMaxNoAnswerTimeout)
		{
			// If timeouts aren't supported go ahead and fail it.
			if (m_AddressCaps.dwMaxNoAnswerTimeout == 0)
				return LINEERR_INVALCALLPARAMS;
			lpCallParams->dwNoAnswerTimeout = m_AddressCaps.dwMaxNoAnswerTimeout;
		}
	}

	if (lpCallParams->dwCallDataSize > m_AddressCaps.dwMaxCallDataSize &&
		m_AddressCaps.dwMaxCallDataSize > 0)
		return LINEERR_INVALCALLPARAMS;;

	// If we have no bandwidth for this call then fail.
	if (m_AddressStatus.dwNumActiveCalls >= m_AddressCaps.dwMaxNumActiveCalls)
		return LINEERR_CALLUNAVAIL;	// Should this be RESOURCEUNAVAIL?

	// Looks ok
    return 0;

}// CTSPIAddressInfo::CanSupportCall

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetMediaControl
//
// This function enables and disables control actions on the media stream 
// associated with this address and all calls present here. Media control 
// actions can be triggered by the detection of specified digits, media modes, 
// custom tones, and call states.  The new specified media controls replace 
// all the ones that were in effect for this line, address, or call prior 
// to this request.
//
void CTSPIAddressInfo::SetMediaControl (TSPIMEDIACONTROL* lpMediaControl)
{   
	// If this is the CURRENT media control object, ignore this
	// set.
	if (lpMediaControl == m_lpMediaControl)
		return;

	// Remove our reference to the PREVIOUS media control structure (if any)
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->DecUsage();

	// Assign our new reference to this media control and increment
	// it's usage counter.
    m_lpMediaControl = lpMediaControl;    
	if (m_lpMediaControl != NULL)
		m_lpMediaControl->IncUsage();

	// Go through all calls and tell them about this new media monitoring.
	CEnterCode sLock(this);  // Synch access to object
	std::for_each(m_lstCalls.begin(), m_lstCalls.end(),
		std::bind2nd(MEM_FUNV1(&CTSPICallAppearance::SetMediaControl),lpMediaControl));

}// CTSPIAddressInfo::SetMediaControl

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::DeleteForwardingInfo
//
// Delete the information in our forwarding array.  This array
// holds the forwarding information reported to TAPI.
//
void CTSPIAddressInfo::DeleteForwardingInfo()
{   
	CEnterCode sLock(this);  // Synch access to object
	std::for_each(m_arrForwardInfo.begin(), m_arrForwardInfo.end(), MEM_FUNV(&TSPIFORWARDINFO::DecUsage));
	m_arrForwardInfo.clear();

}// CTSPIAddressInfo::DeleteForwardingInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnRequestComplete
//
// This virtual method is called when an outstanding request on this
// line address has completed.  The return code indicates the success
// or failure of the request.  Note that the request will filter to
// the call appearance.
//
void CTSPIAddressInfo::OnRequestComplete(CTSPIRequest* pReq, LONG lResult)
{                                         
	// If the request failed, ignore it.
	if (lResult != 0)
		return;

	// Determine what to do based on the command id - this is faster than using
	// the down-cast pointer and can be optimized by the compiler much easier.
	switch (pReq->GetCommand())
	{
		// On a set terminal request, if it is successful, then go ahead and set the
		// terminal identifiers up inside our class.  This information can then be
		// retrieved by TAPI through the GetAddressStatus/Caps methods.
		case REQUEST_SETTERMINAL:
		{
			RTSetTerminal* pTermStruct = dynamic_cast<RTSetTerminal*>(pReq);
			if (pTermStruct->GetAddress() != NULL)
				SetTerminalModes (pTermStruct->GetTerminalID(), 
							pTermStruct->GetTerminalModes(), 
							pTermStruct->Enable());
			break;
		} 
    
		// On a CompleteXfer request, detach the consultation call
		case REQUEST_COMPLETEXFER:
		{
			RTCompleteTransfer* pComTrans = dynamic_cast<RTCompleteTransfer*>(pReq);
			if (pComTrans->GetConsultationCall() != NULL)
				pComTrans->GetConsultationCall()->SetConsultationCall(NULL);
			if (pComTrans->GetCallToTransfer() != NULL)
				pComTrans->GetCallToTransfer()->SetConsultationCall(NULL);
			break;
		}

		// On a FORWARD request, note the forwarding information internally so we
		// may return it to TAPI if requested.
		case REQUEST_FORWARD:
		{
			RTForward* pForward = dynamic_cast<RTForward*>(pReq);

			// Move the FORWARDINFO pointers over to our array
			DeleteForwardingInfo();
			CEnterCode sLock(this);  // Synch access to object
			for (unsigned int i = 0; i < pForward->GetForwardingAddressCount(); i++)
			{
				TSPIFORWARDINFO* pInfo = pForward->GetForwardingInfo(i);
				pInfo->IncUsage();
				try
				{
					m_arrForwardInfo.push_back(pInfo);
				}
				catch(...)
				{
					pInfo->DecUsage();
					throw;
				}
			}                                        

			m_AddressStatus.dwNumRingsNoAnswer = pForward->GetNoAnswerRingCount();
			sLock.Unlock();
			OnAddressStateChange (LINEADDRESSSTATE_FORWARD);
			break;
		}                                 

		// If this is a lineSetMediaControl event, then store the new MediaControl
		// information in the address (and all of it's calls)
		case REQUEST_MEDIACONTROL:
		{
			RTSetMediaControl* pMC = dynamic_cast<RTSetMediaControl*>(pReq);
			if (pMC->GetCall() == NULL)
				SetMediaControl(pMC->GetMediaControlInfo());
			break;
		}

		// If this is a lineSetAgentGroup request and it was successful, load
		// the new group information.
		case REQUEST_SETAGENTGROUP:
			SetAgentGroup(dynamic_cast<RTSetAgentGroup*>(pReq)->GetGroupArray());
			break;

		// .. or a lineSetAgentState, adjust the current agent state/next state.
		case REQUEST_SETAGENTSTATE:
		{
			RTSetAgentState* pState = dynamic_cast<RTSetAgentState*>(pReq);
			SetAgentState(pState->GetAgentState(), pState->GetNextAgentState());
			break;
		}

		// .. or a lineSetAgentActivity, store it into our agent status.
		case REQUEST_SETAGENTACTIVITY:
			SetAgentActivity(dynamic_cast<RTSetAgentActivity*>(pReq)->GetActivity());
			break;

		// Default handler
		default:
			break;
	}

}// CTSPIAddressInfo::OnRequestComplete

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::AddForwardEntry
//
// This method allows the direct addition of forwarding information
// on this address.  This should only be used if the service provider
// can detect that the address is already forwarded on initialization.
//
// To delete the forwarding information, pass a zero in for forward mode.
//
int CTSPIAddressInfo::AddForwardEntry(DWORD dwForwardMode, LPCTSTR pszCaller, LPCTSTR pszDestination, DWORD dwDestCountry)
{   
    int iPos = -1;
    if (dwForwardMode == 0)
        DeleteForwardingInfo();
    else
    {
        TSPIFORWARDINFO* pForward = new TSPIFORWARDINFO;
        if (pForward != NULL)
        {
            pForward->dwForwardMode = dwForwardMode;
            pForward->dwDestCountry = dwDestCountry;
            
            if (pszCaller != NULL && *pszCaller != _T('\0'))
                GetSP()->CheckDialableNumber(GetLineOwner(), this, pszCaller, &pForward->arrCallerAddress, dwDestCountry);
            
            if (pszDestination != NULL && *pszDestination != _T('\0'))
                GetSP()->CheckDialableNumber(GetLineOwner(), this, pszDestination, &pForward->arrDestAddress, dwDestCountry);

			// Calculate the size used for this forwarding information object so
			// we don't have to do it everytime TAPI requests our configuration.
			// This size is what is needed in TAPI terms for the forwarding information.
			pForward->dwTotalSize = sizeof(LINEFORWARD);

			// Count the size of the caller strings.
			pForward->dwTotalSize += std::accumulate(pForward->arrCallerAddress.begin(), pForward->arrCallerAddress.end(), 0, distr_size);
			ALIGN_VALUE(pForward->dwTotalSize);

			// Do the same for the destination strings
			pForward->dwTotalSize += std::accumulate(pForward->arrDestAddress.begin(), pForward->arrDestAddress.end(), 0, distr_size);
			ALIGN_VALUE(pForward->dwTotalSize);

			// Add the forwarding information to our array.
			CEnterCode sLock(this);  // Synch access to object
			try
			{
				m_arrForwardInfo.push_back(pForward);
				iPos = m_arrForwardInfo.size()-1;
			}
			catch(...)
			{
				delete pForward;
				throw;
			}
			sLock.Unlock();
            OnAddressStateChange(LINEADDRESSSTATE_FORWARD);
        }            
    }           
    
    return iPos;

}// CTSPIAddressInfo::AddForwardEntry

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo:AddCallTreatment
//
// Add a new call treatment entry into our address.  This will be
// returned in the ADDRESSCAPS for this address.
//
void CTSPIAddressInfo::AddCallTreatment (DWORD dwCallTreatment, LPCTSTR pszName)
{
	// Add or alter the existing call treatment.
	CEnterCode sLock(this);  // Synch access to object
	m_mapCallTreatment[dwCallTreatment] = pszName;
	m_AddressCaps.dwNumCallTreatments = m_mapCallTreatment.size();
	m_AddressCaps.dwCallTreatmentListSize = sizeof(LINECALLTREATMENTENTRY) * m_AddressCaps.dwNumCallTreatments;
	
}// CTSPIAddressInfo::AddCallTreatment

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::RemoveCallTreatment
//
// Remove a call treatment entry from our address.
//
void CTSPIAddressInfo::RemoveCallTreatment(DWORD dwCallTreatment)
{
	// Locate and remove the specified call treatment name.
	CEnterCode sLock(this);  // Synch access to object
	if (m_mapCallTreatment.erase(dwCallTreatment))
	{
		m_AddressCaps.dwNumCallTreatments = m_mapCallTreatment.size();
		m_AddressCaps.dwCallTreatmentListSize = sizeof(LINECALLTREATMENTENTRY) * m_AddressCaps.dwNumCallTreatments;
		// Tell TAPI our address capabilities have changed.
		OnAddressCapabiltiesChanged();
	}

}// CTSPIAddressInfo::RemoveCallTreatment

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CreateMSPInstance
//
// This function creates a media service provider instance for a specific
// line device. This function returns a TSP handle for the MSP call. It
// requires TAPI 3.0 negotiation.
//
LONG CTSPIAddressInfo::CreateMSPInstance(HTAPIMSPLINE htMSPLine, LPHDRVMSPLINE lphdMSPLine)
{
#ifdef _DEBUG
	// Verify that the handle is not in-use. This is more a validation check against the
	// beta TAPI code.
	for (TMSPArray::iterator i = m_arrMSPInstance.begin(); i != m_arrMSPInstance.end(); ++i)
		_TSP_ASSERTE ((*i)->GetTapiHandle() != htMSPLine);
#endif

	// Allocate a new CMSPDriver object and attach the returned object as the
	// opaque device handle.
	CMSPDriver* pDriver = new CMSPDriver(this, htMSPLine);
	if (pDriver == NULL)
		return LINEERR_NOMEM;

	// Insert it into our array
	try
	{
		m_arrMSPInstance.push_back(pDriver);
	}
	catch(...)
	{
		pDriver->DecRef();
		return LINEERR_NOMEM;
	}

	// Return the driver as the opaque handle
	*lphdMSPLine = (HDRVMSPLINE) pDriver;
	return 0;

}// CTSPIAddressInfo::CreateMSPInstance

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::CloseMSPInstance
//
// This function closes an MSP call instance. This function 
// requires TAPI 3.0 negotiation.
//
LONG CTSPIAddressInfo::CloseMSPInstance(CMSPDriver* pMSP)
{
	TMSPArray::iterator i = std::find(m_arrMSPInstance.begin(), m_arrMSPInstance.end(), pMSP);
	if (i != m_arrMSPInstance.end())
	{
		m_arrMSPInstance.erase(i);
		pMSP->DecRef();
		return 0;
	}
	return LINEERR_OPERATIONFAILED;

}// CTSPIAddressInfo::CloseMSPInstance

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnAddressStateChange
//
// Send a TAPI event for this address.
//
void CTSPIAddressInfo::OnAddressStateChange (DWORD dwAddressState)
{
    if ((m_dwAddressStates & dwAddressState) == dwAddressState)
        GetLineOwner()->Send_TAPI_Event (NULL, LINE_ADDRESSSTATE, GetAddressID(), dwAddressState);

}// CTSPIAddressInfo::Send_TAPI_Event

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnAddressCapabiltiesChanged
//
// The address capabilities have changed (ADDRESSCAPS), tell TAPI 
// about it.
//
void CTSPIAddressInfo::OnAddressCapabiltiesChanged()
{
	OnAddressStateChange (LINEADDRESSSTATE_CAPSCHANGE);

}// CTSPIAddressInfo::OnAddressCapabiltiesChanged

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnCreateCall
//
// This method gets called whenever a new call appearance is created
// on this address.  It is responsible for incrementing the
// dwNumInUse flag and sending out INUSEZERO/ONE/MANY messages
//
void CTSPIAddressInfo::OnCreateCall (CTSPICallAppearance* /*pCall*/)
{ 
	CEnterCode sLock(this);
	if (m_AddressStatus.dwNumInUse == 0)
	{
		m_AddressStatus.dwNumInUse = 1;
		sLock.Unlock();
		OnAddressStateChange(LINEADDRESSSTATE_INUSEONE);
	}

}// CTSPIAddressInfo::OnCreateCall

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnCallFeaturesChanged
//
// This method gets called whenever a call changes its currently
// available features in the CALLINFO structure.
//
DWORD CTSPIAddressInfo::OnCallFeaturesChanged (CTSPICallAppearance* pCall, DWORD dwFeatures)
{ 
	// Let the line adjust its counts based on the changing call.
	return GetLineOwner()->OnCallFeaturesChanged(pCall, dwFeatures);

}// CTSPIAddressInfo::OnCallFeaturesChanged

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::SetAddressFeatures
//
// This method sets the current features available on the address
// It does NOT invoke the OnAddressFeaturesChanged.
//
void CTSPIAddressInfo::SetAddressFeatures(DWORD dwFeatures)
{
	// Make sure the capabilities structure reflects this ability.
	if (dwFeatures != 0 && (m_AddressCaps.dwAddressFeatures & dwFeatures) == 0)
	{
		// If you get this, then you need to update the object dwAddressFeatures
		// in the Init method of the address or in the line while adding it.
		_TSP_DTRACE(_T("%s: LINEADDRCAPS.dwAddressFeatures missing 0x%lx bit\n"), m_strAddress.c_str(), dwFeatures);
		m_AddressCaps.dwAddressFeatures |= dwFeatures;	
		OnAddressCapabiltiesChanged();
	}

	// Update it only if it has changed.
	if (m_AddressStatus.dwAddressFeatures != dwFeatures)
	{
		m_AddressStatus.dwAddressFeatures = dwFeatures;
		OnAddressStateChange (LINEADDRESSSTATE_OTHER);
	}
	
}// CTSPIAddressInfo::SetAddressFeatures

//////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::OnAddressFeaturesChanged
//
// This method gets called whenever our address changes its features.
// It is only called when the LIBRARY changes the features.
//
DWORD CTSPIAddressInfo::OnAddressFeaturesChanged (DWORD dwFeatures)
{ 
	return GetLineOwner()->OnAddressFeaturesChanged(this, dwFeatures);

}// CTSPIAddressInfo::OnAddressFeaturesChanged

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::DevSpecific
//
// Invoke a device-specific feature on this line device's address.
//
LONG CTSPIAddressInfo::DevSpecific(CTSPICallAppearance* /*pCall*/, 
			DRV_REQUESTID /*dwRequestID*/, LPVOID /*lpParam*/, DWORD /*dwSize*/)
{
    // Derived class must manage device-specific features.
    return LINEERR_OPERATIONUNAVAIL;

}// CTSPIAddressInfo::DevSpecific

///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::RecalcAddrFeatures
//
// Recalculate the address features associated with this address
//
void CTSPIAddressInfo::RecalcAddrFeatures()
{
	CTSPILineConnection* pLine = GetLineOwner();

	// Adjust our address features based on the call counts we now have.
	DWORD dwAddressFeatures = m_AddressCaps.dwAddressFeatures;
	if ((pLine->GetLineDevStatus()->dwDevStatusFlags & 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED)) != 
			(LINEDEVSTATUSFLAGS_INSERVICE | LINEDEVSTATUSFLAGS_CONNECTED))
		dwAddressFeatures = 0;
	else if (dwAddressFeatures > 0)
	{
		// If we have no more terminals, then remove the TERMINAL bit.
		if (pLine->GetTerminalCount() == 0)
			dwAddressFeatures &= ~LINEADDRFEATURE_SETTERMINAL;

		// If there are active calls, and we support conferencing, then show SetupConf as a feature
		if (m_dwConnectedCallCount == 0)
			dwAddressFeatures &= ~LINEADDRFEATURE_SETUPCONF;
		else
			dwAddressFeatures &= ~(LINEADDRFEATURE_FORWARD | 
				LINEADDRFEATURE_FORWARDFWD | LINEADDRFEATURE_FORWARDDND);

		// Determine if any calls are waiting (camped on) pending call completions.
		if (dwAddressFeatures & LINEADDRFEATURE_UNCOMPLETECALL)
		{
    		dwAddressFeatures &= ~LINEADDRFEATURE_UNCOMPLETECALL;
			// Walk through all the calls on the address.
			CEnterCode sAddr(this, false);
			CEnterCode sLine(pLine, false);
			if (sAddr.Lock(0) && sLine.Lock(0))
			{
				TCallList::iterator end = m_lstCalls.end();
				for (TCallList::iterator it = m_lstCalls.begin(); it != end; ++it)
    			{
    				if (pLine->FindCallCompletionRequest(*it)) {
        				dwAddressFeatures |= LINEADDRFEATURE_UNCOMPLETECALL;
    					break;
    				}
    			}
			}
		}

		// If we have don't the bandwidth for another active call, then remove all
		// the features which create a new call appearance.
		if (m_AddressStatus.dwNumActiveCalls >= m_AddressCaps.dwMaxNumActiveCalls)
   			dwAddressFeatures &= ~(LINEADDRFEATURE_MAKECALL | 
						LINEADDRFEATURE_PICKUP | 
						LINEADDRFEATURE_UNPARK | 
						LINEADDRFEATURE_PICKUPHELD | 
						LINEADDRFEATURE_PICKUPGROUP | 
						LINEADDRFEATURE_PICKUPDIRECT | 
						LINEADDRFEATURE_PICKUPWAITING);
	}

	// If our new address features are zero, recalc our call features.
	else
	{
		CEnterCode sLock(this);	// Unlocks at exit of if()
		TCallList::iterator end = m_lstCalls.end();
		for (TCallList::iterator ii = m_lstCalls.begin(); ii != end; ++ii)
		{
			// Get the call appearance from the list and lock it.
			CTSPICallAppearance* pCall = (*ii);
			CEnterCode sCall(pCall, FALSE);
			// Update the call features if we can get a locked handle
			// to the call (i.e. it isn't being updated by some other thread).
			if (sCall.Lock(0) &&
				pCall->GetCallState() != LINECALLSTATE_IDLE)
				pCall->RecalcCallFeatures();
		}
	}

	// Set our address features
	DWORD dwNewFeatures = OnAddressFeaturesChanged(dwAddressFeatures & m_AddressCaps.dwAddressFeatures);
	_TSP_DTRACE(_T("%s: Calculating new address features - 0x%lx, Final=0x%lx, ConnectedCalls=%d, Max=%d\n"), m_strAddress.c_str(), dwAddressFeatures, dwNewFeatures, m_AddressStatus.dwNumActiveCalls, m_AddressCaps.dwMaxNumActiveCalls);
	m_AddressStatus.dwAddressFeatures = dwNewFeatures;

}// CTSPIAddressInfo::RecalcAddrFeatures

////////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::MoveCall
//
// This "moves" a call from one line to another.
//
CTSPICallAppearance* CTSPIAddressInfo::MoveCall(CTSPICallAppearance* pCall, DWORD dwReason, 
												DWORD dwState, DWORD dwMode, DWORD dwWaitTime)
{
	_TSP_ASSERTE(pCall != NULL);
	CTSPILineConnection* pDestLine = GetLineOwner();
	_TSP_ASSERTE(pCall->GetAddressOwner() != this);

	_TSP_DTRACE(_T("%s: Moving call <0x%lx> %ld from 0x%lx to 0x%lx\n"), 
		m_strAddress.c_str(), pCall, pCall->GetCallID(), pCall->GetLineOwner(), pDestLine);

	// If the reason code is zero, get it from the call.
	if (dwReason == 0)
		dwReason = pCall->GetCallInfo()->dwReason;

	// Spin for a second waiting for the call to be created - this is important
	// for switches that send back to back events
	if (dwWaitTime)
	{
		DWORD dwTicks = GetTickCount();
		do
		{
			if (pDestLine->FindCallByCallID(pCall->GetCallID()) != NULL)
				break;

		} while ((dwTicks + dwWaitTime) > GetTickCount());
	}

	// See if the said callid is already on the destination line - it may have already
	// been created by a previous event from the switch (some PBX systems will send
	// a "Call Created" event followed by a "Call Transferred" especially for
	// consultation transfers which don't create a new callid on the PBX.
	LINECALLINFO* pCallInfo = pCall->GetCallInfo();
	CTSPICallAppearance* pNewCall = pDestLine->FindCallByCallID(pCall->GetCallID());
	if (pNewCall == NULL)
	{
		// Create a new call on our line.
		pNewCall = CreateCallAppearance(NULL, 0, pCallInfo->dwOrigin, 
							dwReason, pCallInfo->dwTrunk);
	}
	else
	{
		pNewCall->SetCallReason(dwReason);
		pNewCall->SetTrunkID(pCallInfo->dwTrunk);
	}

	// Transfer the CALLID and caller-id information from the old call.
	// This causes the callid in pCall to be set to zero.
	pNewCall->CopyCall(pCall, false);

	// Set the new state for the call. Check the existing state in case the
	// call was not created by this function (and was found existing based on call-id).
	if (pNewCall->GetCallState() == LINECALLSTATE_UNKNOWN &&
		(dwState == 0 || dwState == LINECALLSTATE_CONNECTED))
	{
		// TAPI applications are notorious for not properly handling calls
		// when the "normal" series of callstates are not seen in sequence.
		// A good example of this is Symantec's ACT! program - it completely
		// ignores calls that don't start in DIALTONE or OFFERING states.

		// Show the call as OFFERING, then whatever state so apps see a new call.
		if (pCallInfo->dwOrigin != LINECALLORIGIN_OUTBOUND)
			pNewCall->SetCallState(LINECALLSTATE_OFFERING, 
					LINEOFFERINGMODE_INACTIVE, LINEMEDIAMODE_INTERACTIVEVOICE);
		else
		{
			// Show a normal dialing progression - many TAPI unfortunately fail
			// if they don't see what they consider "normal".
			pNewCall->SetCallState(LINECALLSTATE_DIALTONE, 
					LINEDIALTONEMODE_INTERNAL, LINEMEDIAMODE_INTERACTIVEVOICE);
			if (dwState == LINECALLSTATE_CONNECTED)
			{
				pNewCall->SetCallState(LINECALLSTATE_DIALING);
				pNewCall->SetCallState(LINECALLSTATE_PROCEEDING);
			}
		}

		// Default the state to CONNECTED
		if (dwState == 0)
			dwState = LINECALLSTATE_CONNECTED;
		pNewCall->SetCallState(dwState, dwMode);
	}

	// Mark the CURRENT call (which has transferred) as IDLE.
	// Don't show a disconnect since the call simply transferred (didn't really
	// disconnect).
	pCall->SetCallState(LINECALLSTATE_IDLE);

	return pNewCall;

}// CTSPIAddressInfo::MoveCall

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPIAddressInfo::Dump
//
// Debug method to dump out the Address object
//
TString CTSPIAddressInfo::Dump() const
{
	TStringStream outstm;

	CEnterCode keyLock(this, FALSE);
	outstm << _T("0x") << hex << reinterpret_cast<DWORD>(this);
    outstm << _T(",AddrID=0x") << hex << m_dwAddressID;
	if (m_AddressCaps.dwAddressSharing == LINEADDRESSSHARING_MONITORED)
		outstm << _T(",(Monitor)");		
	if (keyLock.Lock(0))
	{
		outstm << _T(",LineOwner=0x") << hex << GetLineOwner()->GetPermanentDeviceID();
		outstm << _T(" [") << m_strAddress << _T(",") << m_strName << _T("] ") << m_strAddress;
	}
    return(outstm.str());

}// CTSPIAddressInfo::Dump
#endif // _DEBUG
