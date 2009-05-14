/******************************************************************************/
//                                                                        
// CALL.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _SPCALL_INL_INC_
#define _SPCALL_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPICallAppearance
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallType
//
// Return a unique call type which can identify conference and
// consultant calls
//
TSP_INLINE int CTSPICallAppearance::GetCallType() const
{
    return m_iCallType;

}// CTSPICallAppearance::GetCallType

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallType
//
// Set the call type for this call.
//
TSP_INLINE void CTSPICallAppearance::SetCallType(int iCallType)
{
	_TSP_DTRACE(_T("Changing call 0x%lx (CallID 0x%lx) from type %s to %s\n"),
		this, GetCallID(), (m_iCallType == Normal) ? _T("Normal") :
		  (m_iCallType == Consultant) ? _T("Consultation") : _T("Conference"),
		(iCallType == Normal) ? _T("Normal") :
		  (iCallType == Consultant) ? _T("Consultation") : _T("Conference"));

	// We shouldn't ever switch a conference around since that is
	// the wrong type of object!  They aren't the same!
	_TSP_ASSERTE(m_iCallType != Conference && iCallType != Conference);

	CEnterCode sLock(this);
    m_iCallType = iCallType;

}// CTSPICallAppearance::SetCallType

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallHub
//
// Retrieve the call hub which contains pointers to other calls which
// share our call id (shadow calls).
//
TSP_INLINE CTSPICallHub* CTSPICallAppearance::GetCallHub() const
{
	return m_pCallHub;

}// CTSPICallAppearance::GetCallHub

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetShadowCall
//
// Retrieve the call shadow for this call baed on the information in our
// attached call hub.
//
TSP_INLINE CTSPICallAppearance* CTSPICallAppearance::GetShadowCall() const
{
	CEnterCode sHub(m_pCallHub);
	CEnterCode sLock(this);
	return (m_pCallHub != NULL) ? m_pCallHub->GetShadowCall(this) : NULL;

}// CTSPICallAppearance::GetCallShadowCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddAsynchRequest
//
// This method adds a request to a particular connection.  It
// will add the request to the device list this connection belongs
// to.
//
TSP_INLINE int CTSPICallAppearance::AddAsynchRequest(CTSPIRequest* pReq)
{
    return GetLineOwner()->AddAsynchRequest(pReq);

}// CTSPICallAppearance::AddAsynchRequest

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetLineOwner
//
// This returns the line connection information for this call
// appearance.
//
TSP_INLINE CTSPILineConnection* CTSPICallAppearance::GetLineOwner() const
{
    return GetAddressOwner()->GetLineOwner();

}// CTSPICallAppearance::GetLineOwner

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetAddressOwner
//
// Return the address information for this call appearance.
//
TSP_INLINE CTSPIAddressInfo* CTSPICallAppearance::GetAddressOwner() const
{
    return m_pAddr;

}// CTSPICallAppearance::GetAddressOwner

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallID
//
// Return the callid associated with this call
//
TSP_INLINE DWORD CTSPICallAppearance::GetCallID() const
{
	return GetCallInfo()->dwCallID;

}// CTSPICallAppearance::GetCallID

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallHandle
//
// Second-phase initialization for the call object
//
TSP_INLINE void CTSPICallAppearance::SetCallHandle(HTAPICALL htCall)
{
	_TSP_ASSERTE(m_htCall == NULL);	// Only happens once!
    m_htCall = htCall;

}// CTSPICallAppearance::SetCallHandle

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallHandle
//
// Return the TAPI call handle for this call appearance.
//
TSP_INLINE HTAPICALL CTSPICallAppearance::GetCallHandle() const
{
    return m_htCall;

}// CTSPICallAppearance::GetCallHandle

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallState
//
// Return the call state of the call appearance from our LINECALLSTATE
// structure.
//
TSP_INLINE DWORD CTSPICallAppearance::GetCallState() const
{ 
    return m_CallStatus.dwCallState;

}// CTSPICallAppearance::GetCallState

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallInfo
//
// Return a pointer to the LINECALLINFO record
//
TSP_INLINE LPLINECALLINFO CTSPICallAppearance::GetCallInfo()
{                                     
    return &m_CallInfo;
    
}// CTSPICallAppearance::GetCallInfo

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallStatus
//                                   
// Return a pointer to the LINECALLSTATUS record
//
TSP_INLINE LPLINECALLSTATUS CTSPICallAppearance::GetCallStatus()
{   
    return &m_CallStatus;
    
}// CTSPICallAppearance::GetCallStatus

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallInfo
//
// Return a pointer to the LINECALLINFO record
//
TSP_INLINE const LINECALLINFO* CTSPICallAppearance::GetCallInfo() const
{                                     
    return &m_CallInfo;
    
}// CTSPICallAppearance::GetCallInfo

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallStatus
//                                   
// Return a pointer to the LINECALLSTATUS record
//
TSP_INLINE const LINECALLSTATUS* CTSPICallAppearance::GetCallStatus() const
{   
    return &m_CallStatus;
    
}// CTSPICallAppearance::GetCallStatus

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetConsultationCall
//                                   
// Sets the consultation call for this call
//
TSP_INLINE void CTSPICallAppearance::SetConsultationCall(CTSPICallAppearance* pConsultCall)
{
	if (pConsultCall != NULL)
	{
	    pConsultCall->SetCallType(CTSPICallAppearance::Consultant);
		AttachCall(pConsultCall);
	    pConsultCall->AttachCall(this);
	}
	else
	{
		if (m_pConsult != NULL)
			m_pConsult->DetachCall();
		DetachCall();
	}

}// CTSPICallAppearance::SetConsultationCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetConsultationCall
//
// Return the attached call appearance
//
TSP_INLINE CTSPICallAppearance* CTSPICallAppearance::GetConsultationCall() const
{
	CEnterCode sLock(this);
    return m_pConsult;
    
}// CTSPICallAppearance::GetConsultationCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AttachCall
//
// Attach a call to this call appearance
//
TSP_INLINE void CTSPICallAppearance::AttachCall (CTSPICallAppearance* pCall)
{
	_TSP_ASSERTE(pCall != NULL);

	if (pCall == NULL)
		DetachCall();
	else
	{
		_TSP_DTRACE(_T("Attaching call 0x%lx (CallID 0x%lx) to call 0x%lx (CallID 0x%lx)\n"),
					this, GetCallID(), pCall, pCall->GetCallID());

		m_pConsult = pCall;
		SetRelatedCallID(pCall->GetCallID());
	}
    
}// CTSPICallAppearance::AttachCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetConferenceOwner
//
// Set the attached conference owner
//
TSP_INLINE void CTSPICallAppearance::SetConferenceOwner(CTSPIConferenceCall* pConf)
{
	_TSP_DTRACE(_T("Setting conference owner for call 0x%lx (CallID 0x%lx) to call 0x%lx\n"),
			this, GetCallID(), pConf);

	CEnterCode sLock(this);
	m_pConf = pConf;

}// CTSPICallAppearance::SetConferenceOwner

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetAttachedCall
//
// Return the attached call appearance
//
TSP_INLINE CTSPICallAppearance* CTSPICallAppearance::GetAttachedCall() const
{
	CEnterCode sLock(this);
    return m_pConsult;
    
}// CTSPICallAppearance::GetAttachedCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetConferenceOwner
//
// Return the attached conference call appearance
//
TSP_INLINE CTSPIConferenceCall* CTSPICallAppearance::GetConferenceOwner() const
{
	CEnterCode sLock(this);
    return m_pConf;
    
}// CTSPICallAppearance::GetConferenceOwner

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::DetachCall
//
// Detach a call from this call appearance
//
TSP_INLINE void CTSPICallAppearance::DetachCall()
{       
	CEnterCode sLock(this);
	if (m_pConsult != NULL)
	{
		_TSP_DTRACE(_T("Detaching call 0x%lx (CallID 0x%lx) from call 0x%lx (CallID 0x%lx)\n"),
				this, GetCallID(), m_pConsult, (m_pConsult != NULL) ? m_pConsult->GetCallID() : 0);
		m_pConsult = NULL;
	}
	SetRelatedCallID(0);

}// CTSPICallAppearance::DetachCall

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPICallAppearance::AddDeviceClass (LPCTSTR pszClass, DWORD dwData)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, &dwData, sizeof(DWORD));

}// CTSPICallAppearance::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddDeviceClass
//
// Add a STRING data object to our device class list
//
TSP_INLINE int CTSPICallAppearance::AddDeviceClass (LPCTSTR pszClass, LPCTSTR lpszBuff, DWORD dwType)
{
	if (dwType == -1L)
		dwType = m_pAddr->m_pLine->m_LineCaps.dwStringFormat;
	return AddDeviceClass(pszClass, dwType, const_cast<LPTSTR>(lpszBuff), (lstrlen(lpszBuff)+1) * sizeof(TCHAR));

}// CTSPICallAppearance::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPICallAppearance::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, const_cast<LPTSTR>(lpszBuff), (lstrlen(lpszBuff)+1) * sizeof(TCHAR), hHandle);

}// CTSPICallAppearance::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPICallAppearance::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, lpBuff, dwSize, hHandle);

}// CTSPICallAppearance::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPICallAppearance::AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle)
{
	CEnterCode sLock(this);
	return GetSP()->AddDeviceClassInfo (m_arrDeviceClass, pszClass, dwFormat, lpBuff, dwSize, hHandle);

}// CTSPICallAppearance::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::RemoveDeviceClass
//
// Remove a device class list object.
//
TSP_INLINE bool CTSPICallAppearance::RemoveDeviceClass (LPCTSTR pszClass)
{
	CEnterCode sLock(this);
	return GetSP()->RemoveDeviceClassInfo (m_arrDeviceClass, pszClass);	

}// CTSPICallAppearance::RemoveDeviceClass

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallFlags
//
// Returns the call flags
//
TSP_INLINE DWORD& CTSPICallAppearance::GetCallFlags()
{
	CEnterCode sLock(this);
	return m_dwFlags;

}// CTSPICallAppearance::GetCallFlags

////////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetDeviceClass
//
// Return the device class information for a specified name.
//
TSP_INLINE DEVICECLASSINFO* CTSPICallAppearance::GetDeviceClass(LPCTSTR pszClass)
{
	CEnterCode sLock(this);
	return GetSP()->FindDeviceClassInfo (m_arrDeviceClass, pszClass);

}// CTSPICallAppearance::GetDeviceClass

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::HasBeenDeleted
//
// Return true/false whether this object has been deleted
//
TSP_INLINE bool CTSPICallAppearance::HasBeenDeleted() const
{
	return ((m_dwFlags & _IsDeleted) == _IsDeleted);

}// CTSPICallAppearance::HasBeenDeleted

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCallerIDInformation
//
// Return the call information
//
TSP_INLINE const CALLIDENTIFIER& CTSPICallAppearance::GetCallerIDInformation() const
{
	CEnterCode sLock(this);
	return m_CallerID;

}// CTSPICallAppearance::GetCallerIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetCalledIDInformation
//
// Return the call information
//
TSP_INLINE const CALLIDENTIFIER& CTSPICallAppearance::GetCalledIDInformation() const
{
	CEnterCode sLock(this);
	return m_CalledID;

}// CTSPICallAppearance::GetCalledIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetConnectedIDInformation
//
// Return the call information
//
TSP_INLINE const CALLIDENTIFIER& CTSPICallAppearance::GetConnectedIDInformation() const
{
	CEnterCode sLock(this);
	return m_ConnectedID;

}// CTSPICallAppearance::GetConnectedIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetRedirectingIDInformation
//
// Return the call information
//
TSP_INLINE const CALLIDENTIFIER& CTSPICallAppearance::GetRedirectingIDInformation() const
{
	CEnterCode sLock(this);
	return m_RedirectingID;

}// CTSPICallAppearance::GetRedirectingIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetRedirectionIDInformation
//
// Return the call information
//
TSP_INLINE const CALLIDENTIFIER& CTSPICallAppearance::GetRedirectionIDInformation() const
{
	CEnterCode sLock(this);
	return m_RedirectionID;

}// CTSPICallAppearance::GetRedirectionIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallerIDInformation
//
// Set the call information
//
TSP_INLINE void CTSPICallAppearance::SetCallerIDInformation(const CALLIDENTIFIER& ci)
{
	DWORD dwFlags = 0;

	if (!ci.strPartyId.empty())
		dwFlags |= LINECALLPARTYID_ADDRESS;
	if (!ci.strPartyName.empty())
		dwFlags |= LINECALLPARTYID_NAME;
	if (dwFlags == 0)
		dwFlags = LINECALLPARTYID_UNKNOWN;

	SetCallerIDInformation(dwFlags, ci.strPartyId.c_str(), ci.strPartyName.c_str());

}// CTSPICallAppearance::SetCallerIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCalledIDInformation
//
// Set the call information
//
TSP_INLINE void CTSPICallAppearance::SetCalledIDInformation(const CALLIDENTIFIER& ci)
{
	DWORD dwFlags = 0;

	if (!ci.strPartyId.empty())
		dwFlags |= LINECALLPARTYID_ADDRESS;
	if (!ci.strPartyName.empty())
		dwFlags |= LINECALLPARTYID_NAME;
	if (dwFlags == 0)
		dwFlags = LINECALLPARTYID_UNKNOWN;

	SetCalledIDInformation(dwFlags, ci.strPartyId.c_str(), ci.strPartyName.c_str());

}// CTSPICallAppearance::SetCalledIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetConnectedIDInformation
//
// Set the call information
//
TSP_INLINE void CTSPICallAppearance::SetConnectedIDInformation(const CALLIDENTIFIER& ci)
{
	DWORD dwFlags = 0;

	if (!ci.strPartyId.empty())
		dwFlags |= LINECALLPARTYID_ADDRESS;
	if (!ci.strPartyName.empty())
		dwFlags |= LINECALLPARTYID_NAME;
	if (dwFlags == 0)
		dwFlags = LINECALLPARTYID_UNKNOWN;

	SetConnectedIDInformation(dwFlags, ci.strPartyId.c_str(), ci.strPartyName.c_str());

}// CTSPICallAppearance::SetConnectedIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetRedirectionIDInformation
//
// Return the call information
//
TSP_INLINE void CTSPICallAppearance::SetRedirectionIDInformation(const CALLIDENTIFIER& ci)
{
	DWORD dwFlags = 0;

	if (!ci.strPartyId.empty())
		dwFlags |= LINECALLPARTYID_ADDRESS;
	if (!ci.strPartyName.empty())
		dwFlags |= LINECALLPARTYID_NAME;
	if (dwFlags == 0)
		dwFlags = LINECALLPARTYID_UNKNOWN;

	SetRedirectionIDInformation(dwFlags, ci.strPartyId.c_str(), ci.strPartyName.c_str());

}// CTSPICallAppearance::SetRedirectionIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetRedirectingIDInformation
//
// Set the call information
//
TSP_INLINE void CTSPICallAppearance::SetRedirectingIDInformation(const CALLIDENTIFIER& ci)
{
	DWORD dwFlags = 0;

	if (!ci.strPartyId.empty())
		dwFlags |= LINECALLPARTYID_ADDRESS;
	if (!ci.strPartyName.empty())
		dwFlags |= LINECALLPARTYID_NAME;
	if (dwFlags == 0)
		dwFlags = LINECALLPARTYID_UNKNOWN;

	SetRedirectingIDInformation(dwFlags, ci.strPartyId.c_str(), ci.strPartyName.c_str());

}// CTSPICallAppearance::SetRedirectingIDInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetCallData
//
// Sets calldata into our CALLINFO record.  TAPI is notified of the
// change, and depending on the implementation, the calldata should
// be propagated to all systems which have a copy of this data.
//
TSP_INLINE void CTSPICallAppearance::SetCallData (LPCVOID lpvCallData, DWORD dwSize)
{
	// Set the call data
	CEnterCode sLock(this);  // Synch access to object
	m_sdCallData.SetPtr(lpvCallData, dwSize);
	sLock.Unlock();

	// Notify TAPI of the change.
	OnCallInfoChange (LINECALLINFOSTATE_CALLDATA);

}// CTSPICallAppearance::SetCallData

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetChargingInformation
//
// Sets charging information into our LINECALLINFO record.
//
TSP_INLINE void CTSPICallAppearance::SetChargingInformation(LPCVOID lpvData, DWORD dwSize)
{
	// Set the charging data
	CEnterCode sLock(this);  // Synch access to object
	m_sdChargingInfo.SetPtr(lpvData, dwSize);
	sLock.Unlock();

	// Notify TAPI of the change.
	OnCallInfoChange(LINECALLINFOSTATE_CHARGINGINFO);

}// CTSPICallAppearance::SetChargingInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetChargingInformation
//
// Return the charging information
//
TSP_INLINE const SIZEDDATA CTSPICallAppearance::GetChargingInformation() const
{
	CEnterCode sLock(this);
	return m_sdChargingInfo;

}// CTSPICallAppearance::GetChargingInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetLowLevelCompatibilityInformation
//
// Sets low-level compatibility information into our LINECALLINFO record.
//
TSP_INLINE void CTSPICallAppearance::SetLowLevelCompatibilityInformation(LPCVOID lpvData, DWORD dwSize)
{
	// Set the low-level data
	CEnterCode sLock(this);  // Synch access to object
	m_sdLowLevelInfo.SetPtr(lpvData, dwSize);
	sLock.Unlock();

	// Notify TAPI of the change.
	OnCallInfoChange(LINECALLINFOSTATE_LOWLEVELCOMP);

}// CTSPICallAppearance::SetLowLevelCompatibilityInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetLowLevelCompatibilityInformation
//
// Return the low-level compatibility information
//
TSP_INLINE const SIZEDDATA CTSPICallAppearance::GetLowLevelCompatibilityInformation() const
{
	CEnterCode sLock(this);
	return m_sdLowLevelInfo;

}// CTSPICallAppearance::GetLowLevelCompatibilityInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetHiLevelCompatibilityInformation
//
// Sets high-level compatibility information into our LINECALLINFO record.
//
TSP_INLINE void CTSPICallAppearance::SetHiLevelCompatibilityInformation(LPCVOID lpvData, DWORD dwSize)
{
	// Set the low-level data
	CEnterCode sLock(this);  // Synch access to object
	m_sdHiLevelInfo.SetPtr(lpvData, dwSize);
	sLock.Unlock();

	// Notify TAPI of the change.
	OnCallInfoChange(LINECALLINFOSTATE_HIGHLEVELCOMP);

}// CTSPICallAppearance::SetHiLevelCompatibilityInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetHiLevelCompatibilityInformation
//
// Return the high-level compatibility information
//
TSP_INLINE const SIZEDDATA CTSPICallAppearance::GetHiLevelCompatibilityInformation() const
{
	CEnterCode sLock(this);
	return m_sdHiLevelInfo;

}// CTSPICallAppearance::GetHiLevelCompatibilityInformation

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetSendingFlowSpec
//
// Return the sending flow spec information
//
TSP_INLINE const SIZEDDATA CTSPICallAppearance::GetSendingFlowSpec() const
{
	CEnterCode sLock(this);
	return m_sdSendingFS;

}// CTSPICallAppearance::GetSendingFlowSpec

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetReceivingFlowSpec
//
// Return the receiving flow spec information
//
TSP_INLINE const SIZEDDATA CTSPICallAppearance::GetReceivingFlowSpec() const
{
	CEnterCode sLock(this);
	return m_sdReceivingFS;

}// CTSPICallAppearance::GetReceivingFlowSpec

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetBearerMode
//
// This function sets the current bearer mode for this call.
//
TSP_INLINE void CTSPICallAppearance::SetBearerMode(DWORD dwBearerMode)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwBearerMode != dwBearerMode)
	{
		m_CallInfo.dwBearerMode = dwBearerMode;
		OnCallInfoChange (LINECALLINFOSTATE_BEARERMODE);
	}

}// CTSPICallAppearance::SetBearerMode

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetDataRate
//
// This field determines the rate in bits per second for the call
// appearance.  It is determined by the media type and physical line.
//
TSP_INLINE void CTSPICallAppearance::SetDataRate(DWORD dwRateBps)
{
	CEnterCode sLock(this);
	if (m_CallInfo.dwRate != dwRateBps)
	{
		m_CallInfo.dwRate = dwRateBps;
		OnCallInfoChange (LINECALLINFOSTATE_RATE);
	}

}// CTSPICallAppearance::SetDataRate

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::SetAppSpecificData
//
// Set the application specific data for the call appearance.  This
// will be visible ACROSS applications.
//
TSP_INLINE LONG CTSPICallAppearance::SetAppSpecificData(DWORD dwAppSpecific)
{
	CEnterCode sLock(this);
    m_CallInfo.dwAppSpecific = dwAppSpecific; 
    OnCallInfoChange(LINECALLINFOSTATE_APPSPECIFIC);
	
	return 0;

}// CTSPICallAppearance::SetAppSpecificData

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::IsOutgoingCall
//
// Returns whether this call was initially created as an outgoing
// call.  Note this can be wrong if the call is moved to another
// line
// 
TSP_INLINE bool CTSPICallAppearance::IsOutgoingCall() const
{
	CEnterCode sLock(this);
	return ((m_dwFlags & _Outgoing) == _Outgoing);

}// CTSPICallAppearance::IsOutgoingCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::IsRealCall
//
// Returns whether this call is associated with a real call on the 
// telephony hardware.
// 
TSP_INLINE bool CTSPICallAppearance::IsRealCall() const
{
	CEnterCode sLock(this);
	return ((m_dwFlags & _IsRealCall) == _IsRealCall);

}// CTSPICallAppearance::IsRealCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::MarkReal
//
// Allows the "real" call bit to be altered.
// 
TSP_INLINE void CTSPICallAppearance::MarkReal(bool fIsReal)
{
	CEnterCode sLock(this);
	if (fIsReal)
		m_dwFlags |= _IsRealCall;
	else
		m_dwFlags &= ~_IsRealCall;

}// CTSPICallAppearance::MarkReal

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::AddTimedEvent
//
// This method is used to add a new timed event (TIMEREVENT) to 
// the timed event list and marks this call as a timer-required call.
// 
TSP_INLINE void CTSPICallAppearance::AddTimedEvent(int nType, DWORD dwDuration, DWORD dwData1, DWORD dwData2)
{
	TIMEREVENT* lpTimer = new TIMEREVENT(nType, dwDuration, dwData1, dwData2);
	if (lpTimer != NULL)
	{
		m_arrEvents.push_back(lpTimer);
		GetSP()->AddTimedCall(this);
	}
}

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::GetPartiallyDialedDigits
//
// Returns the string which can be used to store the partially dialed
// digits for this call.
// 
TSP_INLINE TString& CTSPICallAppearance::GetPartiallyDialedDigits()
{
	return m_strDialedDigits;

}// CTSPICallAppearance::GetPartiallyDialedDigits

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::IsActiveCallState
//
// Function to return whether the supplied call state is 
// considered ACTIVE to TAPI.
//  
// STATIC FUNCTION
// 
TSP_INLINE bool CTSPICallAppearance::IsActiveCallState(DWORD dwState)
{
    return ((dwState & 
				(LINECALLSTATE_IDLE |
				 LINECALLSTATE_UNKNOWN |
                 LINECALLSTATE_ONHOLD |
                 LINECALLSTATE_ONHOLDPENDTRANSFER |
                 LINECALLSTATE_ONHOLDPENDCONF |
                 LINECALLSTATE_CONFERENCED |
			     LINECALLSTATE_DISCONNECTED)) == 0);

}// CTSPICallAppearance::IsActiveCallState

///////////////////////////////////////////////////////////////////////////
// CTSPICallAppearance::IsConnectedCallState
//
// Function to return whether the supplied call state is 
// CONNECTED to a destination party or channel.
//  
// STATIC FUNCTION
// 
TSP_INLINE bool CTSPICallAppearance::IsConnectedCallState(DWORD dwState)
{
	return ((dwState & (LINECALLSTATE_DIALTONE |
				LINECALLSTATE_DIALING |
				LINECALLSTATE_RINGBACK |
				LINECALLSTATE_BUSY |
				LINECALLSTATE_CONNECTED |
				LINECALLSTATE_PROCEEDING)) != 0);

}// CTSPICallAppearance::IsConnectedCallState

/******************************************************************************/
//
// CTSPIConferenceCall
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPIConferenceCall::GetConferenceCount
//
// Return the count of call appearances in our conference.
//
TSP_INLINE unsigned int CTSPIConferenceCall::GetConferenceCount() const
{                                          
	CEnterCode sLock(this);
    return m_lstConference.size();

}// CTSPIConferenceCall::GetConferenceCount

#endif // _SPCALL_INL_INC_
