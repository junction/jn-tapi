/******************************************************************************/
//                                                                        
// LINE.INL - TAPI Service Provider C++ Library header                     
//     
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// The SPLIB classes provide a basis for developing MS-TAPI complient     
// Service Providers.  They provide basic handling for all of the TSPI    
// APIs and a C-based handler which routes all requests through a set of C++     
// classes.                                                                 
//              
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// INLINE FUNCTIONS
//                                                           
/******************************************************************************/

#ifndef _SPLINE_INL_INC_
#define _SPLINE_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPILineConnection
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAddress
//
// Return an address based on an address ID.
//
TSP_INLINE CTSPIAddressInfo* CTSPILineConnection::GetAddress (unsigned int i) const
{
	// Note: we don't lock the object since we cannot DELETE address objects.
	// Therefore once it is there, it is there to stay.  If we ever have the ability
	// to dynamically add/remove addresses under a line we will need to LOCK the
	// line here.
	return (i < m_arrAddresses.size()) ? m_arrAddresses[i] : NULL;

}// CTSPILineConnection::GetAddress

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineType
//
// Returns the line type (Station, RoutePoint, etc.)
//
TSP_INLINE int CTSPILineConnection::GetLineType() const
{
	return m_iLineType;

}// CTSPILineConnection::GetLineType

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetLineType
//
// Sets the line type (Station, RoutePoint, etc.)
//
TSP_INLINE void CTSPILineConnection::SetLineType(int iLineType)
{
	m_iLineType = iLineType;

}// CTSPILineConnection::SetLineType

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAddressCount
//
// Return the number of addresses available on this line.
//
TSP_INLINE int CTSPILineConnection::GetAddressCount() const
{
	return m_arrAddresses.size();

}// CTSPILineConnection::GetAddressCount

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineHandle
//
// Return the line handle for this line device.
//
TSP_INLINE HTAPILINE CTSPILineConnection::GetLineHandle() const
{
    return m_htLine;

}// CTSPILineConnection::GetLineHandle

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::ForceClose
//
// This method may be used by the service provider to forcibly close the
// line object.
//
TSP_INLINE void CTSPILineConnection::ForceClose()
{
	Send_TAPI_Event (NULL, LINE_CLOSE);
	m_dwFlags &= ~IsAgentEnabled;

}// CTSPILineConnection::ForceClose

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DeleteCallsOnClose
//
// If your switch hardware supports a query feature which allows you to read existing
// call information, you probably don't need the built-in support for tracking calls on
// un-opened lines. If so, call this function which will delete existing calls off the
// line when it is closed, but not drop the calls themselves. You must then add the code
// to re-read the switch state when the line is opened.
//
TSP_INLINE void CTSPILineConnection::DeleteCallsOnClose()
{
	_TSP_ASSERTE ((GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_CLOSEDROP) == 0);
	m_dwFlags |= DelCallsOnClose;

}// CTSPILineConnection::DeleteCallsOnClose

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::EnableAgentProxy
//
// Turn on the agent proxy support
//
TSP_INLINE void CTSPILineConnection::EnableAgentProxy()
{
	m_dwFlags |= IsAgentEnabled;
	GetDeviceInfo()->EnableAgentProxy(this);

}// CTSPILineConnection::EnableAgentProxy

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineDevCaps
//
// Return the line device capabilities.  Keep in mind that the capabilities
// are not complete here - the optional fields are not filled out until
// an actual call to GetCapabilities is made, and they are not stored in
// our structure (only the callers).
//
TSP_INLINE LPLINEDEVCAPS CTSPILineConnection::GetLineDevCaps()
{
    return &m_LineCaps;

}// CTSPILineConnection::GetLineDevCaps

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineDevStatus
//
// Return the line device status.  Keep in mind that the status information
// are not complete here - the optional fields are not filled out until
// an actual call to GatherStatus is made, and they are not stored in
// our structure (only the callers).
//
TSP_INLINE LPLINEDEVSTATUS CTSPILineConnection::GetLineDevStatus()
{
    return &m_LineStatus;

}// CTSPILineConnection::GetLineDevStatus;

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineDevCaps
//
// Return the line device capabilities.  Keep in mind that the capabilities
// are not complete here - the optional fields are not filled out until
// an actual call to GetCapabilities is made, and they are not stored in
// our structure (only the callers).
//
TSP_INLINE const LINEDEVCAPS* CTSPILineConnection::GetLineDevCaps() const
{
    return &m_LineCaps;

}// CTSPILineConnection::GetLineDevCaps

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetLineDevStatus
//
// Return the line device status.  Keep in mind that the status information
// are not complete here - the optional fields are not filled out until
// an actual call to GatherStatus is made, and they are not stored in
// our structure (only the callers).
//
TSP_INLINE const LINEDEVSTATUS* CTSPILineConnection::GetLineDevStatus() const
{
    return &m_LineStatus;

}// CTSPILineConnection::GetLineDevStatus;

///////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetDeviceID
//
// This function is called in response to a LINE_CREATE message (and
// TAPIs subsequent call to providerCreateLineDevice) to reset our new
// device identifier.
//
TSP_INLINE void CTSPILineConnection::SetDeviceID(DWORD dwDeviceId)
{
	CTSPIConnection::SetDeviceID(dwDeviceId);
	if (m_dwFlags & IsAgentEnabled)
		EnableAgentProxy();
	
}// CTSPILineConnection::SetDeviceID

///////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetAssociatedPhone
//
// Return the associated phone device with this line.
//
TSP_INLINE CTSPIPhoneConnection* CTSPILineConnection::GetAssociatedPhone() const
{
	DEVICECLASSINFO* pDevInfo = GetDeviceClass(_T("tapi/phone"));
	if (pDevInfo != NULL)
	{
		DWORD dwPhoneDeviceID = *(reinterpret_cast<LPDWORD>(pDevInfo->lpvData.get()));
		return GetDeviceInfo()->FindPhoneConnectionByDeviceID(dwPhoneDeviceID);
	}
	return NULL;

}// CTSPILineConnection::GetAssociatedPhone

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetTerminalCount
//
// Return the count of terminals on this line.
//
TSP_INLINE int CTSPILineConnection::GetTerminalCount() const
{
	return m_arrTerminalInfo.size();
	
}// CTSPILineConnection::GetTerminalCount

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetDefaultMediaDetection
//
// Return the current set of media modes being detected by the SP.
// This is used when a new call is created to setup the initial monitoring
// of media modes.
//
TSP_INLINE DWORD CTSPILineConnection::GetDefaultMediaDetection() const
{                                                
    return m_dwLineMediaModes;
    
}// CTSPILineConnection::GetDefaultMediaDetection

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SendDialogInstanceData
//
// Causes TAPI to call the TUISPI_providerGenericDialogData
// function of the UI DLL associated with the htDlgInstance.
//
TSP_INLINE void CTSPILineConnection::SendDialogInstanceData(HTAPIDIALOGINSTANCE htDlgInstance, 
					LPVOID lpParams, DWORD dwSize)
{
	// TODO: This function will likely require changes for Win64; pointers are not 32-bits anymore.
	Send_TAPI_Event(NULL, LINE_SENDDIALOGINSTANCEDATA, (DWORD)lpParams, dwSize, (DWORD)htDlgInstance);

}// CTSPILineConnection::SendDialogInstanceData

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::SetPermanentLineID
//
// Sets the permanent line identifier for this line.  This should
// be unique across the whole provider for any line in the system.
//
TSP_INLINE void CTSPILineConnection::SetPermanentLineID(DWORD dwLineID)
{
	GetDeviceInfo()->MapConnID(m_LineCaps.dwPermanentLineID, dwLineID, this);
    m_LineCaps.dwPermanentLineID = dwLineID;

}// CTSPILineConnection::SetPermanentLineID

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevStatusInService
//
// Sets/unsets the DeviceStatus bit related to INSERVICE.
//
TSP_INLINE void CTSPILineConnection::DevStatusInService(bool fInservice)
{
	DWORD dwFlags = GetLineDevStatus()->dwDevStatusFlags;
	if (fInservice)
		dwFlags |= LINEDEVSTATUSFLAGS_INSERVICE;
	else
		dwFlags &= ~LINEDEVSTATUSFLAGS_INSERVICE;
	SetDeviceStatusFlags(dwFlags);

}// CTSPILineConnection::DevStatusInService

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevStatusConnected
//
// Sets/unsets the DeviceStatus bit related to CONNECTED.
//
TSP_INLINE void CTSPILineConnection::DevStatusConnected(bool fConnected)
{
	DWORD dwFlags = GetLineDevStatus()->dwDevStatusFlags;
	if (fConnected)
		dwFlags |= LINEDEVSTATUSFLAGS_CONNECTED;
	else
		dwFlags &= ~LINEDEVSTATUSFLAGS_CONNECTED;
	SetDeviceStatusFlags(dwFlags);

}// CTSPILineConnection::DevStatusConnected

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevStatusLocked
//
// Sets/unsets the DeviceStatus bit related to LOCKED
//
TSP_INLINE void CTSPILineConnection::DevLocked(bool fLocked)
{
	DWORD dwFlags = GetLineDevStatus()->dwDevStatusFlags;
	if (fLocked)
		dwFlags |= LINEDEVSTATUSFLAGS_LOCKED;
	else
		dwFlags &= ~LINEDEVSTATUSFLAGS_LOCKED;
	SetDeviceStatusFlags(dwFlags);

}// CTSPILineConnection::DevLocked

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::DevMsgWaiting
//
// Sets/unsets the DeviceStatus bit related to MSGWAITING
//
TSP_INLINE void CTSPILineConnection::DevMsgWaiting(bool fMessageWaiting)
{
	DWORD dwFlags = GetLineDevStatus()->dwDevStatusFlags;
	if (fMessageWaiting)
		dwFlags |= LINEDEVSTATUSFLAGS_MSGWAIT;
	else
		dwFlags &= ~LINEDEVSTATUSFLAGS_MSGWAIT;
	SetDeviceStatusFlags(dwFlags);

}// CTSPILineConnection::DevMsgWaiting

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::GetRequestMap
//
// Retrieves the private request map for line objects. This may
// be overridden to provide line-by-line request management.
//
TSP_INLINE TRequestMap* CTSPILineConnection::GetRequestMap() const
{
	return &CTSPILineConnection::g_mapRequests;

}// CTSPILineConnection::GetRequestMap

////////////////////////////////////////////////////////////////////////////
// CTSPILineConnection::ConvertDialableToCanonical
//
// This can be overridden to change the way a specific line formats
// it's canonical phone numbers.  The default is to pass through to the
// main formatter function in CServiceProvider.
//
TSP_INLINE TString CTSPILineConnection::ConvertDialableToCanonical(LPCTSTR pszPartyID, DWORD dwCountryCode, bool fInbound)
{
	return GetSP()->ConvertDialableToCanonical(pszPartyID, dwCountryCode, fInbound);

}// CTSPILineConnection::ConvertDialableToCanonical

/******************************************************************************/
//
// RTAccept
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAccept::RTAccept
//
// TSPI_lineAccept constructor
//
TSP_INLINE RTAccept::RTAccept(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
							  LPCSTR pszUserUserInfo, DWORD dwSize) :
	RTUserUserInfo(_T("lineAccept"), pCall, REQUEST_ACCEPT, dwRequestID, pszUserUserInfo, dwSize)
{
}// RTAccept::RTAccept

/******************************************************************************/
//
// RTAddToConference
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAddToConference::RTAddToConference
//
// Constructor for the RTAddToConference object
//
TSP_INLINE RTAddToConference::RTAddToConference(CTSPIConferenceCall* pConfCall, 
								CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
	CTSPIRequest(_T("lineAddToConference")), m_pConfCall(pConfCall), m_pConsult(pCall)
{
	CTSPIRequest::Init (NULL, NULL, pConfCall, REQUEST_ADDCONF, dwRequestID);

}// RTAddToConference::RTAddToConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAddToConference::RTAddToConference
//
// Constructor for the RTAddToConference object
//
TSP_INLINE RTAddToConference::RTAddToConference(LPCTSTR pszType, CTSPIConferenceCall* pConfCall, 
								CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
	CTSPIRequest(pszType), m_pConfCall(pConfCall), m_pConsult(pCall)
{
	CTSPIRequest::Init (NULL, NULL, pConfCall, REQUEST_ADDCONF, dwRequestID);

}// RTAddToConference::RTAddToConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAddToConference::GetConferenceCall
//
// Return the CONFERENCE call which we are to add into.
//
TSP_INLINE CTSPIConferenceCall* RTAddToConference::GetConferenceCall() const
{
	return m_pConfCall;

}// RTAddToConference::GetConferenceCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAddToConference::GetConsultationCall
//
// Return the CONSTULTATION call which we are to add into the conference.
//
TSP_INLINE CTSPICallAppearance* RTAddToConference::GetConsultationCall() const
{
	return m_pConsult;

}// RTAddToConference::GetConsultationCall

/******************************************************************************/
//
// RTUserUserInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUserUserInfo::RTUserUserInfo
//
// Constructor for the basic request User-User information object
//
TSP_INLINE RTUserUserInfo::RTUserUserInfo(LPCTSTR pszType, 
			  CTSPICallAppearance* pCall, int iReqType, DRV_REQUESTID dwRequestID, 
			  LPCSTR pszUserUserInfo, DWORD dwSize) : CTSPIRequest(pszType)
{
	CTSPIRequest::Init(NULL, NULL, pCall, iReqType, dwRequestID);
	m_UserUserInfo.SetPtr(pszUserUserInfo, dwSize);

}// RTUserUserInfo::RTUserUserInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUserUserInfo::GetUserUserInfo
//
// Return the useruser info pointer
//
TSP_INLINE LPCSTR RTUserUserInfo::GetUserUserInfo() const
{
	return static_cast<LPCSTR>(m_UserUserInfo.GetPtr());

}// RTUserUserInfo::GetUserUserInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUserUserInfo::GetSize
//
// Return the useruser info pointer size
//
TSP_INLINE DWORD RTUserUserInfo::GetSize() const
{
	return m_UserUserInfo.GetSize();

}// RTUserUserInfo::GetSize

/******************************************************************************/
//
// RTDialInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDialInfo::RTDialInfo
//
// Basic request object holding DIALINFO requests
//
TSP_INLINE RTDialInfo::RTDialInfo(LPCTSTR pszType, CTSPICallAppearance* pCall, int iReqType,
		DRV_REQUESTID dwRequestID, TDialStringArray* parrDialInfo, DWORD dwCountryCode) :
	CTSPIRequest(pszType), m_arrDialInfo(*parrDialInfo), m_dwCountryCode(dwCountryCode)
{
	CTSPIRequest::Init(NULL, NULL, pCall, iReqType, dwRequestID);

}// RTDialInfo::RTDialInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDialInfo::GetCountryCode
//
// Returns the country code associated with the event.
//
TSP_INLINE DWORD RTDialInfo::GetCountryCode() const
{
	return m_dwCountryCode;

}// RTDialInfo::GetCountryCode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDialInfo::GetCount
//
// Returns the number of DIALINFO elements in the vector
//
TSP_INLINE int RTDialInfo::GetCount() const
{
	return m_arrDialInfo.size();

}// RTDialInfo::GetCount

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDialInfo::GetDialableNumber
//
// Returns a specific DIALINFO element.
//
TSP_INLINE DIALINFO* RTDialInfo::GetDialableNumber(unsigned int i) const
{	
	return (i < m_arrDialInfo.size()) ? m_arrDialInfo[i] : NULL;

}// RTDialInfo::GetDialableNumber

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDialInfo::DialArray
//
// Returns a pointer to our array.
//
TSP_INLINE TDialStringArray* RTDialInfo::DialArray()
{	
	return &m_arrDialInfo;

}// RTDialInfo::DialArray

/******************************************************************************/
//
// RTAnswer
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTAnswer::RTAnswer
//
// TSPI_lineAnswer constructor
//
TSP_INLINE RTAnswer::RTAnswer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
							  LPCSTR pszUserUserInfo, DWORD dwSize) : 
	RTUserUserInfo(_T("lineAnswer"), pCall, REQUEST_ANSWER, dwRequestID, pszUserUserInfo, dwSize)
{
}// RTAnswer::RTAnswer

/******************************************************************************/
//
// RTBlindTransfer
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTBlindTransfer::RTBlindTransfer
//
// TSPI_lineBlindTransfer constructor
//
TSP_INLINE RTBlindTransfer::RTBlindTransfer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
							TDialStringArray* parrDialInfo, DWORD dwCountryCode) :
	RTDialInfo (_T("lineBlindTransfer"), pCall, REQUEST_BLINDXFER, dwRequestID, parrDialInfo, dwCountryCode)
{
}// RTBlindTransfer::RTBlindTransfer

/******************************************************************************/
//
// RTCompleteCall
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::RTCompleteCall
//
// Constructor for the completion request
//
TSP_INLINE RTCompleteCall::RTCompleteCall(CTSPICallAppearance* pCall, 
			DRV_REQUESTID dwRequestID, DWORD dwCompletionMode, DWORD dwMessageId) :
    CTSPIRequest(_T("lineCompleteCall")), 
	m_dwCompletionID(0), m_dwCompletionMode(dwCompletionMode), 
	m_dwMessageId(dwMessageId), m_dwSwitchInfo(0)
{
	// Initialize the basic request object
	CTSPIRequest::Init (NULL, NULL, pCall, REQUEST_COMPLETECALL, dwRequestID);

}// RTCompleteCall::RTCompleteCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::RTCompleteCall
//
// Copy constructor for the completion request
//
TSP_INLINE RTCompleteCall::RTCompleteCall(const RTCompleteCall& pComp) : 
		CTSPIRequest(pComp), m_dwCompletionID(reinterpret_cast<DWORD>(&pComp)),
		m_dwCompletionMode(pComp.m_dwCompletionMode), m_dwMessageId(pComp.m_dwMessageId),
		m_dwSwitchInfo(pComp.m_dwSwitchInfo), m_strSwitchInfo(pComp.m_strSwitchInfo)
{
}// RTCompleteCall::RTCompleteCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::GetCompletionID
//
// Returns the completion ID associated with this request.
//
TSP_INLINE DWORD RTCompleteCall::GetCompletionID() const
{
	return m_dwCompletionID;

}// RTCompleteCall::GetCompletionID

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::GetCompletionMode
//
// Returns the completion mode associated with this request.
//
TSP_INLINE DWORD RTCompleteCall::GetCompletionMode() const
{
	return m_dwCompletionMode;

}// RTCompleteCall::GetCompletionMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::GetMessageID
//
// Returns the completion message id associated with this request.
//
TSP_INLINE DWORD RTCompleteCall::GetMessageID() const
{
	return m_dwMessageId;

}// RTCompleteCall::GetMessageID

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::SetIdentifier
//
// Sets the DWORD and string identifer for this completion request so that
// it may be identified later by the service provider.
//
TSP_INLINE void RTCompleteCall::SetIdentifier(LPCTSTR pszSwitchInfo, DWORD dwSwitchInfo)
{
    m_dwSwitchInfo = dwSwitchInfo;
	if (pszSwitchInfo == NULL)
		m_strSwitchInfo.erase(m_strSwitchInfo.begin(), m_strSwitchInfo.end());
	else
		m_strSwitchInfo = pszSwitchInfo;

}// RTCompleteCall::SetIdentifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::SetIdentifier
//
// Sets the DWORD identifer for this completion request so that
// it may be identified later by the service provider.
//
TSP_INLINE void RTCompleteCall::SetIdentifier(DWORD dwSwitchInfo)
{
    m_dwSwitchInfo = dwSwitchInfo;
	m_strSwitchInfo.erase(m_strSwitchInfo.begin(), m_strSwitchInfo.end());

}// RTCompleteCall::SetIdentifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::GetNumericIdentifier
//
// Returns the DWORD identifier which was assigned to this request by
// the service provider.
//
TSP_INLINE DWORD RTCompleteCall::GetNumericIdentifier() const
{
    return m_dwSwitchInfo;

}// RTCompleteCall::GetNumericIdentifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteCall::GetStringIdentifier
//
// Returns the string identifier which was assigned to this request by
// the service provider.
//
TSP_INLINE LPCTSTR RTCompleteCall::GetStringIdentifier() const
{
    return m_strSwitchInfo.c_str();

}// RTCompleteCall::GetStringIdentifier

/******************************************************************************/
//
// RTCompleteTransfer
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::RTCompleteTransfer
//
// Constructor for the complete transfer request
//
TSP_INLINE RTCompleteTransfer::RTCompleteTransfer(CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, CTSPICallAppearance* pConsultCall, 
		CTSPIConferenceCall* pConfCall, DWORD dwTransferMode) : 
    CTSPIRequest(_T("lineCompleteTransfer")), 
	m_pConsult(pConsultCall), m_pConfCall(pConfCall), m_dwTransferMode(dwTransferMode)
{
	// Initialize the basic request object
	CTSPIRequest::Init (NULL, NULL, pCall, REQUEST_COMPLETEXFER, dwRequestID);
    m_pConsult = pConsultCall;
    m_pConfCall = pConfCall;
    m_dwTransferMode = dwTransferMode;

}// RTCompleteTransfer::RTCompleteTransfer

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::GetTransferMode
//
// Returns the type of transfer completion to use.
//
TSP_INLINE DWORD RTCompleteTransfer::GetTransferMode() const
{
	return m_dwTransferMode;

}// RTCompleteTransfer::GetTransferMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::GetConsultationCall
//
// Returns the consultation call which is the "destination" of the transfer.
//
TSP_INLINE CTSPICallAppearance* RTCompleteTransfer::GetConsultationCall() const
{
	return m_pConsult;

}// RTCompleteTransfer::GetConsultationCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::GetCallToTransfer
//
// Return the call which is being transferred.
//
TSP_INLINE CTSPICallAppearance* RTCompleteTransfer::GetCallToTransfer() const
{
	return GetCallInfo();

}// RTCompleteTransfer::GetCallToTransfer

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTCompleteTransfer::GetConferenceCall
//
// Returns the conference call which is being created by this transfer
//
TSP_INLINE CTSPIConferenceCall* RTCompleteTransfer::GetConferenceCall() const
{
	return m_pConfCall;

}// RTCompleteTransfer::GetConferenceCall

/******************************************************************************/
//
// RTDial
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDial::RTDial
//
// TSPI_lineDial constructor
//
TSP_INLINE RTDial::RTDial(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
						  TDialStringArray* parrDialInfo, DWORD dwCountryCode) :
	RTDialInfo (_T("lineDial"), pCall, REQUEST_DIAL, dwRequestID, parrDialInfo, dwCountryCode)
{
}// RTDial::RTDial

/******************************************************************************/
//
// RTDropCall
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDropCall::RTDropCall
//
// TSPI_lineDrop constructor
//
TSP_INLINE RTDropCall::RTDropCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
								  LPCSTR pszUserUserInfo, DWORD dwSize) :
	m_fDropOnClose(false), 
	RTUserUserInfo (_T("lineDropCall"), pCall, REQUEST_DROPCALL, dwRequestID, pszUserUserInfo, dwSize)
{
}// RTDropCall::RTDropCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDropCall::RTDropCall
//
// TSPI_lineDrop constructor when used for DROPONCLOSE
//
TSP_INLINE RTDropCall::RTDropCall(CTSPICallAppearance* pCall) :
	m_fDropOnClose(true), RTUserUserInfo (_T("lineDropCall"), pCall, REQUEST_DROPCALL, 0, NULL, 0)
{
}// RTDropCall::RTDropCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDropCall::IsImplicitDropFromLineClose
//
// Returns true/false whether the call is being dropped due to a TSPI_lineClose
// or TSPI_lineCloseCall on active call.
//
TSP_INLINE bool RTDropCall::IsImplicitDropFromLineClose() const
{
	return m_fDropOnClose;

}// RTDropCall::IsImplicitDropFromLineClose

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDropCall::IgnoringDrop
//
// Allows the service provider to ignore the drop and "unmarks" the drop flag from
// the call.
//
TSP_INLINE void RTDropCall::IgnoringDrop(bool fIsIgnoring)
{
	m_fIgnoreDrop = fIsIgnoring;

}// RTDropCall::IgnoringDrop

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTDropCall::IsIgnoringDrop
//
// Allows the service provider to ignore the drop and "unmarks" the drop flag from
// the call.
//
TSP_INLINE bool RTDropCall::IsIgnoringDrop() const
{
	return m_fIgnoreDrop;

}// RTDropCall::IsIgnoringDrop

/******************************************************************************/
//
// RTForward
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::RTForward
//
// Constructor for the lineForward request object
//
TSP_INLINE RTForward::RTForward(CTSPILineConnection* pLine, 
				CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestID, 
				TForwardInfoArray* parrForwardInfo, DWORD dwNumRings, 
				CTSPICallAppearance* pConsultCall, LPLINECALLPARAMS pCallParams) :
    CTSPIRequest(_T("lineForward")), 
	m_parrForwardInfo(parrForwardInfo), m_dwNumRingsNoAnswer(dwNumRings),
    m_pConsult(pConsultCall), m_lpCallParams(pCallParams)
{
	// Initialize the base object
	CTSPIRequest::Init (pLine, pAddr, NULL, REQUEST_FORWARD, dwRequestID);

}// RTForward::RTForward

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetNoAnswerRingCount
//
// Returns the number of rings before the call is forwarded.
//
TSP_INLINE DWORD RTForward::GetNoAnswerRingCount() const
{
	return m_dwNumRingsNoAnswer;

}// RTForward::GetNoAnswerRingCount

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetConsultationCall
//
// Returns the consultation call used for the forwarding.
//
TSP_INLINE CTSPICallAppearance* RTForward::GetConsultationCall() const
{
	return m_pConsult;

}// RTForward::GetConsultationCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetCallParameters
//
// Returns the LINECALLPARAMS which are associated with the new consultation call.
//
TSP_INLINE LPLINECALLPARAMS RTForward::GetCallParameters()
{
	return m_lpCallParams;

}// RTForward::GetCallParameters

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetForwardingAddressCount
//
// Returns the number of TSPIFORWARDINFO entries in this request
//
TSP_INLINE unsigned int RTForward::GetForwardingAddressCount() const
{
	return (m_parrForwardInfo != NULL) ? m_parrForwardInfo->size() : 0;

}// RTForward::GetForwardingAddressCount

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetForwardingInfo
//
// Returns a specific TSPIFORWARDINFO from the request
//
TSP_INLINE TSPIFORWARDINFO* RTForward::GetForwardingInfo(unsigned int i)
{
	return (m_parrForwardInfo != NULL && i < m_parrForwardInfo->size()) 
		? m_parrForwardInfo->at(i) : NULL;

}// RTForward::GetForwardingInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTForward::GetForwardingArray
//
// Returns a pointer to the entire array for modification.
//
TSP_INLINE TForwardInfoArray* RTForward::GetForwardingArray()
{
	return m_parrForwardInfo;

}// RTForward::GetForwardingArray

/******************************************************************************/
//
// RTMakeCall
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTMakeCall::RTMakeCall
//
// Constructor for the MakeCall event
//
TSP_INLINE RTMakeCall::RTMakeCall(CTSPICallAppearance* pCall, 
					DRV_REQUESTID dwRequestID, TDialStringArray* parrDialInfo, 
					DWORD dwCountryCode, LPLINECALLPARAMS pCallParams) :
	RTDialInfo (_T("lineMakeCall"), pCall, REQUEST_MAKECALL, dwRequestID, parrDialInfo, dwCountryCode)
{
    m_lpCallParams = pCallParams;

}// RTMakeCall::RTMakeCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTMakeCall::GetCallParameters
//
// Returns the LINECALLPARAMS which are associated with the call.
//
TSP_INLINE LPLINECALLPARAMS RTMakeCall::GetCallParameters()
{
	return m_lpCallParams;

}// RTMakeCall::GetCallParameters

/******************************************************************************/
//
// RTPark
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPark::RTPark
//
// Constructor for the linePark event
//
TSP_INLINE RTPark::RTPark(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
		DWORD dwParkMode, LPCTSTR pszAddress, LPVARSTRING lpNonDirAddress) :
		CTSPIRequest(_T("linePark")), 
		m_dwParkMode(dwParkMode), m_lpNonDirAddress(lpNonDirAddress), 
		m_strDirectedAddress((pszAddress) ? pszAddress : _T(""))
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_PARK, dwRequestID);

}// RTPark::RTPark

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPark::GetDialableNumber
//
// Returns the number to park the call to.
//
TSP_INLINE LPCTSTR RTPark::GetDialableNumber() const
{
	if (!m_strDirectedAddress.empty())
		return m_strDirectedAddress.c_str();
	return NULL;

}// RTPark::GetDialableNumber

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPark::GetParkMode
//
// Returns the parking mode to use.
//
TSP_INLINE DWORD RTPark::GetParkMode() const
{
	return m_dwParkMode;

}// RTPark::GetParkMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPark::SetParkedAddress
//
// Fills in the VARSTRING with a string containing the parked address.
// false is returned if there is not enough space in the VARSTRING.
//
TSP_INLINE bool RTPark::SetParkedAddress(LPCTSTR pszAddress)
{
	if (m_lpNonDirAddress != NULL)
	{
		CopyVarString(m_lpNonDirAddress, pszAddress);
		return (m_lpNonDirAddress->dwNeededSize > m_lpNonDirAddress->dwTotalSize);
	}
	return false;

}// RTPark::SetParkedAddress

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPark::ParkedAddress
//
// Returns a pointer to the LPVARSTRING for the parked address
//
TSP_INLINE LPVARSTRING RTPark::ParkedAddress()
{
	return m_lpNonDirAddress;

}// RTPark::ParkedAddress

/******************************************************************************/
//
// RTPickup
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPickup::RTPickup
//
// Constructor for the linePickup event
//
TSP_INLINE RTPickup::RTPickup(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
							  TDialStringArray* parrDialInfo, LPCTSTR pszGroupID) :
	RTDialInfo (_T("linePickup"), pCall, REQUEST_PICKUP, dwRequestID, parrDialInfo),
	m_strGroupID((pszGroupID) ? pszGroupID : _T(""))
{
}// RTPickup::RTPickup

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPickup::GetGroupID
//
// Returns the group identifier for this pickup request
//
TSP_INLINE LPCTSTR RTPickup::GetGroupID() const
{
	if (!m_strGroupID.empty())
		return m_strGroupID.c_str();
	return NULL;

}// RTPickup::GetGroupID

/******************************************************************************/
//
// RTRedirect
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTRedirect::RTRedirect
//
// Constructor for the lineRedirect event
//
TSP_INLINE RTRedirect::RTRedirect(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
						TDialStringArray* parrDialInfo, DWORD dwCountryCode) :
	RTDialInfo (_T("lineRedirect"), pCall, REQUEST_REDIRECT, dwRequestID, parrDialInfo, dwCountryCode)
{
}// RTRedirect::RTRedirect

/******************************************************************************/
//
// RTRemoveFromConference
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTRemoveFromConference::RTRemoveFromConference
//
// Constructor for the lineRemoveFromConference event
//
TSP_INLINE RTRemoveFromConference::RTRemoveFromConference(CTSPIConferenceCall* pConfCall,
						CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
	    CTSPIRequest(_T("lineRemoveFromConference")), 
		m_pConfCall(pConfCall)
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_REMOVEFROMCONF, dwRequestID);

}// RTRemoveFromConference::RTRemoveFromConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTRemoveFromConference::GetConferenceCall
//
// Return the CONFERENCE call which we are removing from.
//
TSP_INLINE CTSPIConferenceCall* RTRemoveFromConference::GetConferenceCall() const
{
	return m_pConfCall;

}// RTRemoveFromConference::GetConferenceCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTRemoveFromConference::GetCallToRemove
//
// Return the call we are removing from our conference.
//
TSP_INLINE 	CTSPICallAppearance* RTRemoveFromConference::GetCallToRemove() const
{
	return GetCallInfo();

}// RTRemoveFromConference::GetCallToRemove

/******************************************************************************/
//
// RTSendUserInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSendUserInfo::RTSendUserInfo
//
// TSPI_lineSendUserUserInfo constructor
//
TSP_INLINE RTSendUserInfo::RTSendUserInfo(CTSPICallAppearance* pCall, 
			DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize) :
	RTUserUserInfo (_T("lineSendUserInfo"), pCall, REQUEST_SENDUSERINFO, dwRequestID, pszUserUserInfo, dwSize)
{
}// RTSendUserInfo::RTSendUserInfo

/******************************************************************************/
//
// RTSetCallParams
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::RTSetCallParams
//
// Request to adjust the call parameters of an existing call.
//
TSP_INLINE RTSetCallParams::RTSetCallParams(CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, DWORD dwBearerMode, 
		DWORD dwMinRate, DWORD dwMaxRate, 
		LPLINEDIALPARAMS pdp) :
    CTSPIRequest(_T("lineSetCallParams")), 
	m_dwBearerMode(dwBearerMode), m_dwMinRate(dwMinRate), m_dwMaxRate(dwMaxRate)
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SETCALLPARAMS, dwRequestID);
	if (pdp != NULL)
		MoveMemory(&m_DialParams, pdp, sizeof(LINEDIALPARAMS));
	else
		MoveMemory(&m_DialParams, &pCall->GetCallInfo()->DialParams, sizeof(LINEDIALPARAMS));

}// RTSetCallParams::RTSetCallParams

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::GetBearerMode
//
// Returns the bearer mode associated with this request.
//
TSP_INLINE DWORD RTSetCallParams::GetBearerMode() const
{
	return m_dwBearerMode;

}// RTSetCallParams::GetBearerMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::GetMinDataRate
//
// Returns the minimum data rate.
//
TSP_INLINE DWORD RTSetCallParams::GetMinDataRate() const
{
	return m_dwMinRate;

}// RTSetCallParams::GetMinDataRate

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::GetMaxDataRate
//
// Returns the minimum data rate.
//
TSP_INLINE DWORD RTSetCallParams::GetMaxDataRate() const
{
	return m_dwMaxRate;

}// RTSetCallParams::GetMaxDataRate

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::SetBearerMode
//
// Sets the bearer mode in the request object.
//
TSP_INLINE void RTSetCallParams::SetBearerMode(DWORD dwBearerMode)
{
	m_dwBearerMode = dwBearerMode;

}// RTSetCallParams::SetBearerMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::SetDataRate
//
// Sets the min/max data rate in the structure
//
TSP_INLINE void RTSetCallParams::SetDataRate(DWORD dwMinRate, DWORD dwMaxRate)
{
	m_dwMinRate = dwMinRate;
	m_dwMaxRate = dwMaxRate;

}// RTSetCallParams::SetDataRate

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallParams::GetDialParams
//
// Returns the LINEDIALPARAMS structure.
//
TSP_INLINE LPLINEDIALPARAMS RTSetCallParams::GetDialParams()
{
	return &m_DialParams;

}// RTSetCallParams::GetDialParams

/******************************************************************************/
//
// RTSetTerminal
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::RTSetTerminal
//
// lineSetTerminal constructor
//
TSP_INLINE RTSetTerminal::RTSetTerminal(CTSPILineConnection* pLine, 
		CTSPIAddressInfo* pAddress, CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, DWORD dwTerminalModes, DWORD dwTerminalID, bool fEnable) :
    CTSPIRequest(_T("lineSetTerminal"))
{
	CTSPIRequest::Init(pLine, pAddress, pCall, REQUEST_SETTERMINAL, dwRequestID);
    m_pLine = pLine;
    m_pAddress = pAddress;
    m_pCall = pCall;
    m_dwTerminalModes = dwTerminalModes;
    m_dwTerminalID = dwTerminalID;
    m_fEnable = fEnable;

}// RTSetTerminal::RTSetTerminal

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::GetLine
//
// Returns the line (may be null) for the request
//
TSP_INLINE CTSPILineConnection* RTSetTerminal::GetLine() const
{
	return m_pLine;

}// RTSetTerminal::GetLine

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::GetAddress
//
// Returns the address (may be null) for the request
//
TSP_INLINE CTSPIAddressInfo* RTSetTerminal::GetAddress() const
{
	return m_pAddress;

}// RTSetTerminal::GetAddress

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::GetCall
//
// Returns the call (may be null) for the request
//
TSP_INLINE CTSPICallAppearance* RTSetTerminal::GetCall() const
{
	return m_pCall;

}// RTSetTerminal::GetCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::GetTerminalModes
//
// Returns the terminal modes being established
//
TSP_INLINE DWORD RTSetTerminal::GetTerminalModes() const
{
	return m_dwTerminalModes;

}// RTSetTerminal::GetTerminalmodes

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::GetTerminalID
//
// Returns the terminal ID being established
//
TSP_INLINE DWORD RTSetTerminal::GetTerminalID() const
{
	return m_dwTerminalID;

}// RTSetTerminal::GetTerminalID

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetTerminal::Enable
//
// Returns bool representing whether to enable/disable terminal
//
TSP_INLINE bool RTSetTerminal::Enable() const
{
	return m_fEnable;

}// RTSetTerminal::Enable

/******************************************************************************/
//
// RTSetupConference
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::RTSetupConference
//
// Constructor for the lineSetupConference event
//
TSP_INLINE RTSetupConference::RTSetupConference(CTSPIConferenceCall* pConfCall,
		CTSPICallAppearance* pOrigCall, CTSPICallAppearance* pConsult, 
		DRV_REQUESTID dwRequestID, DWORD dwNumParties, LPLINECALLPARAMS lpCallParams) :
    CTSPIRequest(_T("lineSetupConference"))
{
	CTSPIRequest::Init(NULL, NULL, pConfCall, REQUEST_SETUPCONF, dwRequestID);
    m_pConfCall = pConfCall;
    m_pConsult = pConsult;
	m_pOrigParty = pOrigCall;
    m_dwPartyCount = dwNumParties;
    m_lpCallParams = lpCallParams;

}// RTSetupConference::RTSetupConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::GetConferenceCall
//
// Return the CONFERENCE call we are playing with
//
TSP_INLINE CTSPIConferenceCall* RTSetupConference::GetConferenceCall() const
{
	return m_pConfCall;

}// RTSetupConference::GetConferenceCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::GetConsultationCall
//
// Return the CONSTULTATION call which we are to add into the conference.
//
TSP_INLINE CTSPICallAppearance* RTSetupConference::GetConsultationCall() const
{
	return m_pConsult;

}// RTSetupConference::GetConsultationCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::GetOriginalCall
//
// Return the call which started the conference (may be NULL)
//
TSP_INLINE CTSPICallAppearance* RTSetupConference::GetOriginalCall() const
{
	return m_pOrigParty;

}// RTSetupConference::GetOriginalCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::GetInitialPartyCount
//
// Returns the number of initial parties expected in this conference.
//
TSP_INLINE DWORD RTSetupConference::GetInitialPartyCount() const
{
	return m_dwPartyCount;

}// RTSetupConference::GetInitialPartyCount

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupConference::GetCallParameters
//
// Returns the LINECALLPARAMS which are associated with the call.
//
TSP_INLINE LPLINECALLPARAMS RTSetupConference::GetCallParameters()
{
	return m_lpCallParams;

}// RTSetupConference::GetCallParameters

/******************************************************************************/
//
// RTSetupTransfer
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::RTSetupTransfer
//
// Constructor for the lineSetupTransfer event
//
TSP_INLINE RTSetupTransfer::RTSetupTransfer(CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, CTSPICallAppearance* pConsult, 
		LPLINECALLPARAMS lpCallParams) :
    CTSPIRequest(_T("lineSetupTransfer")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SETUPXFER, dwRequestID);
    m_pConsult = pConsult;
    m_lpCallParams = lpCallParams;

}// RTSetupTransfer::RTSetupTransfer

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::GetConsultationCall
//
// Return the CONSTULTATION call which we are transferring to.
//
TSP_INLINE CTSPICallAppearance* RTSetupTransfer::GetConsultationCall() const
{
	return m_pConsult;

}// RTSetupTransfer::GetConsultationCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::GetCallToTransfer
//
// Return the call which is being transferred.
//
TSP_INLINE CTSPICallAppearance* RTSetupTransfer::GetCallToTransfer() const
{
	return GetCallInfo();

}// RTSetupTransfer::GetCallToTransfer

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetupTransfer::GetCallParameters
//
// Returns the LINECALLPARAMS which are associated with the call.
//
TSP_INLINE LPLINECALLPARAMS RTSetupTransfer::GetCallParameters()
{
	return m_lpCallParams;

}// RTSetupTransfer::GetCallParameters

/******************************************************************************/
//
// RTSwapHold
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSwapHold::RTSwapHold
//
// Request to swap a holding call with an active call
//
TSP_INLINE RTSwapHold::RTSwapHold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID,
								  CTSPICallAppearance* pConsult) :
    CTSPIRequest(_T("lineSwapHold")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SWAPHOLD, dwRequestID);
    m_pConsult = pConsult;

}// RTSwapHold::RTSwapHold

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSwapHold::GetHoldingCall
//
// Return the CONSTULTATION call which we are swapping with
//
TSP_INLINE CTSPICallAppearance* RTSwapHold::GetHoldingCall() const
{
	return m_pConsult;

}// RTSwapHold::GetHoldingCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSwapHold::GetActiveCall
//
// Return the call which is currently connected
//
TSP_INLINE CTSPICallAppearance* RTSwapHold::GetActiveCall() const
{
	return GetCallInfo();

}// RTSwapHold::GetActiveCall

/******************************************************************************/
//
// RTUncompleteCall
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUncompleteCall::RTUncompleteCall
//
// Request to uncomplete a pending complete call request
//
TSP_INLINE RTUncompleteCall::RTUncompleteCall(CTSPILineConnection* pLine, DRV_REQUESTID dwRequestID,
											  RTCompleteCall* pReq) :
    CTSPIRequest(_T("lineUncompleteCall"))
{
	CTSPIRequest::Init(pLine, NULL, NULL, REQUEST_UNCOMPLETECALL, dwRequestID);
    m_pRequest = pReq;

}// RTUncompleteCall::RTUncompleteCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUncompleteCall::GetRTCompleteCall
//
// Return the original complete call request
//
TSP_INLINE RTCompleteCall* RTUncompleteCall::GetRTCompleteCall() const
{
	return m_pRequest;

}// RTUncompleteCall::GetRTCompleteCall

/******************************************************************************/
//
// RTHold
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTHold::RTHold
//
// Request to hold a call.
//
TSP_INLINE RTHold::RTHold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
    CTSPIRequest(_T("lineHold")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_HOLD, dwRequestID);

}// RTHold::RTHold

/******************************************************************************/
//
// RTUnhold
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUnhold::RTUnhold
//
// Request to unhold a call.
//
TSP_INLINE RTUnhold::RTUnhold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
    CTSPIRequest(_T("lineUnhold"))
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_UNHOLD, dwRequestID);

}// RTUnhold::RTUnhold

/******************************************************************************/
//
// RTSecureCall
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSecureCall::RTSecureCall
//
// Request to secure a call.
//
TSP_INLINE RTSecureCall::RTSecureCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
    CTSPIRequest(_T("lineSecureCall")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SECURECALL, dwRequestID);

}// RTSecureCall::RTSecureCall

/******************************************************************************/
//
// RTSetCallTreatment
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallTreatment::RTSetCallTreatment
//
// Request to set the current treatment for a call
//
TSP_INLINE RTSetCallTreatment::RTSetCallTreatment(CTSPICallAppearance* pCall, 
				DRV_REQUESTID dwRequestID, DWORD dwTreatment) :
    CTSPIRequest(_T("lineSetCallTreatment")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SETCALLTREATMENT, dwRequestID);
	m_dwCallTreatment = dwTreatment;

}// RTSetCallTreatment::RTSetCallTreatment

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallTreatment::GetCallTreatment
//
// Return the asking treatment for the call
//
TSP_INLINE DWORD RTSetCallTreatment::GetCallTreatment() const
{
	return m_dwCallTreatment;

}// RTSetCallTreatment::GetCallTreatment

/******************************************************************************/
//
// RTUnpark
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTUnpark::RTUnpark
//
// Request to set the unpark a call at some specified address
//
TSP_INLINE RTUnpark::RTUnpark(CTSPICallAppearance* pCall, 
					  DRV_REQUESTID dwRequestID, TDialStringArray* parrDialInfo) :
	RTDialInfo (_T("lineUnpark"), pCall, REQUEST_UNPARK, dwRequestID, parrDialInfo)
{
}// RTUnpark::RTUnpark

/******************************************************************************/
//
// RTSetMediaControl
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::RTSetMediaControl
// 
// Request to manage the lineSetMediaControl event
//
TSP_INLINE RTSetMediaControl::RTSetMediaControl(CTSPILineConnection* pLine, 
		CTSPIAddressInfo* pAddress, CTSPICallAppearance* pCall, 
		TSPIMEDIACONTROL* pMediaControl) :
    CTSPIRequest(_T("lineSetMediaControl"))
{
	CTSPIRequest::Init(pLine, pAddress, pCall, REQUEST_MEDIACONTROL, 0);
    m_pLine = pLine;
    m_pAddress = pAddress;
    m_pCall = pCall;
	m_pMediaControl = pMediaControl;

	// Increment the reference count on the media control structure
	if (m_pMediaControl != NULL)
		m_pMediaControl->IncUsage();

}// RTSetMediaControl::RTSetMediaControl

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::GetLine
//
// Returns the line (may be null) for the request
//
TSP_INLINE CTSPILineConnection* RTSetMediaControl::GetLine() const
{
	return m_pLine;

}// RTSetMediaControl::GetLine

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::GetAddress
//
// Returns the address (may be null) for the request
//
TSP_INLINE CTSPIAddressInfo* RTSetMediaControl::GetAddress() const
{
	return m_pAddress;

}// RTSetMediaControl::GetAddress

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::GetCall
//
// Returns the call (may be null) for the request
//
TSP_INLINE CTSPICallAppearance* RTSetMediaControl::GetCall() const
{
	return m_pCall;

}// RTSetMediaControl::GetCall

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetMediaControl::GetMediaControlInfo
//
// Returns the media control information structure
//
TSP_INLINE TSPIMEDIACONTROL* RTSetMediaControl::GetMediaControlInfo()
{
	return m_pMediaControl;

}// RTSetMediaControl::GetMediaControlInfo

/******************************************************************************/
//
// RTPrepareAddToConference
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPrepareAddToConference::RTPrepareAddToConference
//
// Constructor for the RTPrepareAddToConference object
//
TSP_INLINE RTPrepareAddToConference::RTPrepareAddToConference(
					CTSPIConferenceCall* pConfCall, CTSPICallAppearance* pCall, 
					DRV_REQUESTID dwRequestID, LPLINECALLPARAMS pCallParams) :
	RTAddToConference(_T("linePrepareAddToConference"), pConfCall, pCall, dwRequestID)
{
	// Reset the packet type
	m_iReqType = REQUEST_PREPAREADDCONF;	
	m_lpCallParams = pCallParams;

}// RTPrepareAddToConference::RTPrepareAddToConference

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTPrepareAddToConference::GetCallParameters
//
// Returns the LINECALLPARAMS which are associated with the new consultation call.
//
TSP_INLINE LPLINECALLPARAMS RTPrepareAddToConference::GetCallParameters()
{
	return m_lpCallParams;

}// RTPrepareAddToConference::GetCallParameters

/******************************************************************************/
//
// RTReleaseUserInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTReleaseUserInfo::RTReleaseUserInfo
//
// Request to release information in the CALLINFO record.
//
TSP_INLINE RTReleaseUserInfo::RTReleaseUserInfo(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID) :
    CTSPIRequest(_T("lineReleaseUserInfo")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_RELEASEUSERINFO, dwRequestID);

}// RTReleaseUserInfo::RTReleaseUserInfo

/******************************************************************************/
//
// RTGenerateDigits
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::RTGenerateDigits
//
// Request to generate a set of INBOUND tones.
//
TSP_INLINE RTGenerateDigits::RTGenerateDigits(CTSPICallAppearance* pCall, 
	  DWORD dwEndToEndID, DWORD dwDigitMode, LPCTSTR lpszDigits, DWORD dwDuration) :
		CTSPIRequest(_T("lineGenerateDigits")), 
		m_dwEndToEndID(dwEndToEndID), m_dwDigitMode(dwDigitMode),
		m_strDigits((lpszDigits) ? lpszDigits : _T("")), m_dwDuration(dwDuration)
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_GENERATEDIGITS, 0);

}// RTGenerateDigits::RTGenerateDigits

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::GetIdentifier
//
// Return the unique end-to-end identifier for this request.
//
TSP_INLINE DWORD RTGenerateDigits::GetIdentifier() const
{
	return m_dwEndToEndID;

}// RTGenerateDigits::GetIdentifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::GetDigitMode
//
// Return the LINEDIGITMODE_xxx for this request
//
TSP_INLINE DWORD RTGenerateDigits::GetDigitMode() const
{
	return m_dwDigitMode;

}// RTGenerateDigits::GetDigitMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::GetDigits
//
// Return a reference to the string of digits to generate.
// The string may be modified during the request.
//
TSP_INLINE TString& RTGenerateDigits::GetDigits()
{
	return m_strDigits;

}// RTGenerateDigits::GetDigits

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateDigits::GetDuration
//
// Return the duration of each tone.
//
TSP_INLINE DWORD RTGenerateDigits::GetDuration() const
{
	return m_dwDuration;

}// RTGenerateDigits::GetDuration

/******************************************************************************/
//
// RTGenerateTone
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::RTGenerateTone
//
// Request to generate a set of INBOUND tones.
//
TSP_INLINE RTGenerateTone::RTGenerateTone(CTSPICallAppearance* pCall, 
		DWORD dwEndToEndID, DWORD dwToneMode, DWORD dwDuration, TGenerateToneArray* arrTones) :
    CTSPIRequest(_T("lineGenerateTone")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_GENERATETONE, 0);
	m_dwEndToEndID = dwEndToEndID;
	m_dwToneMode = dwToneMode;
	m_dwDuration = dwDuration;
	m_parrTones = arrTones;

}// RTGenerateTone::RTGenerateTone

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetIdentifier
//
// Return the unique end-to-end identifier for this request.
//
TSP_INLINE DWORD RTGenerateTone::GetIdentifier() const
{
	return m_dwEndToEndID;

}// RTGenerateTone::GetIdentifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetToneMode
//
// Return the LINETONEMODE_xxx for this request
//
TSP_INLINE DWORD RTGenerateTone::GetToneMode() const
{
	return m_dwToneMode;

}// RTGenerateTone::GetToneMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetDuration
//
// Return the duration of each tone.
//
TSP_INLINE DWORD RTGenerateTone::GetDuration() const
{
	return m_dwDuration;

}// RTGenerateTone::GetDuration

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetToneCount
//
// Return the count of custom tones associated with this request
//
TSP_INLINE unsigned int RTGenerateTone::GetToneCount() const
{
	return (m_parrTones != NULL) ? m_parrTones->size() : 0;

}// RTGenerateTone::GetToneCount

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetTone
//
// Return the specific tone at the specified index.
//
TSP_INLINE LPLINEGENERATETONE RTGenerateTone::GetTone(unsigned int i)
{
	return (m_parrTones != NULL && i < m_parrTones->size()) ? m_parrTones->at(i) : NULL;

}// RTGenerateTone::GetTone

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGenerateTone::GetToneArray
//
// Return the tone array for modification
//
TSP_INLINE TGenerateToneArray* RTGenerateTone::GetToneArray()
{
	return m_parrTones;

}// RTGenerateTone::GetToneArray

/******************************************************************************/
//
// RTSetCallData
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallData::RTSetCallData
//
// Request to set call data into a call object.
//
TSP_INLINE RTSetCallData::RTSetCallData(CTSPICallAppearance* pCall, 
						DRV_REQUESTID dwRequestID, LPVOID lpCallData, DWORD dwSize) :
    CTSPIRequest(_T("lineSetCallData"))
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SETCALLDATA, dwRequestID);
	m_CallData.SetPtr(lpCallData, dwSize);

}// RTSetCallData::RTSetCallData

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallData::GetSize
//
// Returns the size of the data block.
//
TSP_INLINE DWORD RTSetCallData::GetSize() const
{
	return m_CallData.GetSize();

}// RTSetCallData::GetSize

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetCallData::GetData
//
// Returns the pointer to the data
//
TSP_INLINE LPCVOID RTSetCallData::GetData() const
{
	return m_CallData.GetPtr();

}// RTSetCallData::GetData

/******************************************************************************/
//
// RTSetQualityOfService
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetQualityOfService::RTSetQualityOfService
//
// Request to set the quality of service
//
TSP_INLINE RTSetQualityOfService::RTSetQualityOfService(CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, LPVOID lpvSFS, DWORD dwSFSSize, 
		LPVOID lpvRFS, DWORD dwRFSSize) :
    CTSPIRequest(_T("lineSetQualityOfService")) 
{
	CTSPIRequest::Init(NULL, NULL, pCall, REQUEST_SETQOS, dwRequestID);
	m_SendingFlowSpec.SetPtr(lpvSFS, dwSFSSize);
	m_RecvFlowSpec.SetPtr(lpvRFS, dwRFSSize);

}// RTSetQualityOfService::RTSetQualityOfService

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetQualityOfService::GetSendingFlowSpec
//
// Return information from object
//
TSP_INLINE const FLOWSPEC*  RTSetQualityOfService::GetSendingFlowSpec() const
{
	return static_cast<const FLOWSPEC* >(m_SendingFlowSpec.GetPtr());

}// RTSetQualityOfService::GetSendingFlowSpec

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetQualityOfService::GetReceivingFlowSpec
//
// Return information from object
//
TSP_INLINE const FLOWSPEC*  RTSetQualityOfService::GetReceivingFlowSpec() const
{
	return static_cast<const FLOWSPEC* >(m_RecvFlowSpec.GetPtr());

}// RTSetQualityOfService::GetReceivingFlowSpec

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetQualityOfService::GetSendingFlowSpecSize
//
// Return information from object
//
TSP_INLINE DWORD RTSetQualityOfService::GetSendingFlowSpecSize() const
{
	return m_SendingFlowSpec.GetSize();

}// RTSetQualityOfService::

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetQualityOfService::GetReceivingFlowSpecSize
//
// Return information from object
//
TSP_INLINE DWORD RTSetQualityOfService::GetReceivingFlowSpecSize() const
{
	return m_RecvFlowSpec.GetSize();

}// RTSetQualityOfService::GetReceivingFlowSpecSize

/******************************************************************************/
//
// RTSetLineDevStatus
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLineDevStatus::RTSetLineDevStatus
//
// Request to set the current line status bits
//
TSP_INLINE RTSetLineDevStatus::RTSetLineDevStatus(CTSPILineConnection* pLine, 
			DRV_REQUESTID dwRequestID, DWORD dwStatusBits, bool fSet) :
    CTSPIRequest(_T("lineSetLineDevStatus")) 
{
	CTSPIRequest::Init(pLine, NULL, NULL, REQUEST_SETDEVSTATUS, dwRequestID);
	m_dwLineStatus = dwStatusBits;
	m_fSetBits = fSet;

}// RTSetQualityOfService::RTSetQualityOfService

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLineDevStatus::GetStatusBitsToChange
//
// Returns the status bits to adjust
//
TSP_INLINE DWORD RTSetLineDevStatus::GetStatusBitsToChange() const
{
	return m_dwLineStatus;

}// RTSetLineDevStatus::GetStatusBitsToChange

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLineDevStatus::TurnOnBits
//
// Returns the new state of the bits in question (true=on)
//
TSP_INLINE bool RTSetLineDevStatus::TurnOnBits() const
{
	return m_fSetBits;

}// RTSetLineDevStatus::TurnOnBits

#endif // _SPLINE_INL_INC_
