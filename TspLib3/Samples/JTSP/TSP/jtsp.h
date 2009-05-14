/***************************************************************************
//
// JTSP.H
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
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

#ifndef _JTSP_INC_
#define _JTSP_INC_

/*----------------------------------------------------------------------------
	INCLUDE FILES
-----------------------------------------------------------------------------*/
#include "interface.h"
#include "eventtypes.h"
#include "poolmgr.h"
#include "deque"
#include "resource.h"

/*----------------------------------------------------------------------------
	PRE-DECLARATIONS
-----------------------------------------------------------------------------*/
class CJTLine;
class CJTPhone;
class CJTDevice;

/*----------------------------------------------------------------------------
	CONSTANTS
-----------------------------------------------------------------------------*/
#define STATE_WAITING		(STATE_INITIAL + 1)
#define STATE_WAITING2		(STATE_INITIAL + 2)
#define REQUEST_TIMEOUT		(10000)				// Timeout for requests (mSec)
#define MAXCALLDATA_SIZE	(4096)				// 4k calldata block

/*----------------------------------------------------------------------------
	THREAD POOL MANAGER WORKER FUNCTIONS

  Note: we are not required to provide these - a template function within
  the poolmgr.h will generate these functions if we didn't have them here,
  but this allows us to customize how events are destroyed -and- how the 
  worker threads know when to run an event based on the synch key.
-----------------------------------------------------------------------------*/

/*****************************************************************************
** Procedure:  TPM_DelEvent
**
** Arguments:  CEventBlock* to delete
**
** Returns:    void
**
** Description:  This destroys single event blocks
**
*****************************************************************************/
inline void TPM_DelEvent(CEventBlock* pBlock)
{
	// Decrement the reference count on the block - it will
	// auto destruct when it hits zero.
	pBlock->Decrement();

}// TPM_DelEvent

/*****************************************************************************
** Procedure:  TPM_CanRunEvent
**
** Arguments:  'pEvent' - CEventBlock* to run
**             'dwKey'  - Key
**			   'fLocked' - Whether the synch key is locked
**
** Returns:    true/false whether to run this request right now.
**
** Description: This is used to determine whether to run an event through
**              the line/phone object based on the event itself and whether
**              there is an existing worker thread running an event on the
**              given line/phone object.
**
*****************************************************************************/
inline bool TPM_CanRunEvent(CEventBlock* pBlock, DWORD /*dwKey*/, bool fIsLocked)
{
	// Don't allow events which require a call-id since we may not have
	// handled the detected/placed yet. Allow agent status and phone
	// status to flow through.
	//
	// Implementation Note:
	// You have to be very carefull about event flow since you never know
	// what will affect what in handling - TEST TEST TEST!
	int iEventType = pBlock->GetEventType();
	if (fIsLocked == false ||
	   (iEventType == CEventBlock::AgentStateChanged ||
		iEventType == CEventBlock::AgentGroupChanged ||
		iEventType == CEventBlock::DisplayChanged ||
		iEventType == CEventBlock::VolumeChanged ||
		iEventType == CEventBlock::GainChanged ||
		iEventType == CEventBlock::HookswitchChanged ||
		iEventType == CEventBlock::LampChanged))
		return true;

	// Otherwise let it go through.
	return false;

}// TPM_CanRunEvent

/**************************************************************************
** CJTProvider
**
** Main provider object which manages the lifetime of the TSP
**
***************************************************************************/
class CJTProvider : public CServiceProvider
{
// Constructor
public:
    CJTProvider();
};

/**************************************************************************
** CJTDevice
**
** Device object which manages a connection to the JPBX simulator
**
***************************************************************************/
class CJTDevice : public CTSPIDevice
{
// Class Data
protected:
	HANDLE m_hevtStop;									// Stop event
	HANDLE m_hevtData;									// Data available event
	HANDLE m_hInputThread;								// Thread processing input
	HANDLE m_hConnThread;								// Thread running socket connection
	CConnection m_connPBX;								// Connection to PBX (WinSock)
	std::deque<TString> m_arrInput;						// Queue with received data
	CRITICAL_SECTION m_csData;							// CS protecting queue
	CEventFactory m_facEvents;							// Event factory for parsing events
	CThreadPoolMgr<DWORD, CEventBlock*> m_mgrThreads;	// Thread pool manager class
	TString m_strIPAddress;								// Our IP connection
	UINT m_nPort;										// The port to connect to.
	
// Constructor/Destructor
public:
	CJTDevice();
	virtual ~CJTDevice();

// Access methods
public:
	inline CConnection* GetPBX() { return &m_connPBX; }

// Controlling methods
public:
	void DRV_Answer(const CJTLine* pLine, CTSPICallAppearance* pCall);
	void DRV_DropCall(const CJTLine* pLine, CTSPICallAppearance* pCall);
	void DRV_HoldCall(const CJTLine* pLine, CTSPICallAppearance* pCall);
	void DRV_RetrieveCall(const CJTLine* pLine, CTSPICallAppearance* pCall);
	void DRV_GenerateDigit(const CJTLine* pLine, CTSPICallAppearance* pCall, TCHAR chDigit);
	void DRV_MakeCall(const CJTLine* pLine, const TString& strDigits, DWORD dwCountryCode=0);
	void DRV_MakePredictiveCall(const CJTLine* pLine, const TString& strDigits, DWORD dwCountryCode, int iTimeout, const TString& strTarget);
	void DRV_BlindTransfer(const CJTLine* pLine, CTSPICallAppearance* pCall, const TString& strAddress);
	void DRV_Transfer(const CJTLine* pLine, CTSPICallAppearance* pCall, CTSPICallAppearance* pConsult);
	void DRV_SetAgentState(const CJTLine* pLine, const TString& strAgent, TCHAR chState);
	void DRV_Logon(const CJTLine* pLine, const TString& strAgent, const TString& strPassword, DWORD dwGroup1, DWORD dwGroup2);
	void DRV_SetGain(const CJTPhone* pPhone, int iGain);
	void DRV_SetHookswitch(const CJTPhone* pPhone, int iHookswitch);
	void DRV_SetVolume(const CJTPhone* pPhone, int iGain);

// Input thread function
public:
	unsigned InputThread();
	unsigned ConnectionThread();

// Overriden functions from CTSPIDevice
protected:
    virtual bool Init(DWORD dwProviderId, DWORD dwBaseLine, DWORD dwBasePhone, DWORD dwLines, DWORD dwPhones, HPROVIDER hProvider, ASYNC_COMPLETION lpfnCompletion);
	virtual TStream& read(TStream& istm);

// Internal methods
private:
	friend class CJTLine;
	void QueuePacket(CEventBlock* pBlock);
	void OnConnect(bool fConnect);
};

/**************************************************************************
** CJTLine
**
** Line object which manages a single line on the switch.
**
***************************************************************************/
class CJTLine : public CTSPILineConnection
{
// Constructor
public:
	CJTLine() {/* */}
	inline CJTDevice* GetDeviceInfo() { return dynamic_cast<CJTDevice*>(CTSPILineConnection::GetDeviceInfo()); }
	
// Overrides from CTSPILineConnection
public:
	virtual TStream& read(TStream& istm);
protected:
	virtual DWORD OnAddressFeaturesChanged (CTSPIAddressInfo* pAddr, DWORD dwFeatures);
	virtual DWORD OnLineFeaturesChanged(DWORD dwLineFeatures);
	virtual DWORD OnCallFeaturesChanged(CTSPICallAppearance* pCall, DWORD dwCallFeatures);
	virtual bool OpenDevice();
	virtual bool UnsolicitedEvent(LPCVOID lpBuff);
	virtual void OnTimer();

	// Create the event map
	DECLARE_TSPI_REQUESTS()

	// TSPI handlers
	bool OnAnswer(RTAnswer* pRequest, LPCVOID lpBuff);
	bool OnDropCall(RTDropCall* pReq, LPCVOID lpBuff);
	bool OnMakeCall(RTMakeCall* pReq, LPCVOID lpBuff);
	bool OnHoldCall(RTHold* pReq, LPCVOID lpBuff);
	bool OnRetrieveCall(RTUnhold* pReq, LPCVOID lpBuff);
	bool OnSwapHold(RTSwapHold* pReq, LPCVOID lpBuff);
	bool OnDial(RTDial* pReq, LPCVOID lpBuff);
	bool OnPredictiveMakeCall(RTMakeCall* pReq, LPCVOID lpBuff);
	bool OnRedirectOrBlindTransfer(CTSPIRequest* pReq, LPCVOID lpBuff);
	bool OnGenerateDigits(RTGenerateDigits* pReq, LPCVOID lpBuff);
	bool OnSetupTransfer(RTSetupTransfer* pReq, LPCVOID lpBuff);
	bool OnCompleteTransfer(RTCompleteTransfer* pReq, LPCVOID lpBuff);
	bool OnSetupConference(RTSetupConference* pReq, LPCVOID lpBuff);
	bool OnPrepareAddToConference(RTPrepareAddToConference* pReq, LPCVOID lpBuff);
	bool OnAddToConference(RTAddToConference* pReq, LPCVOID lpBuff);
	bool OnRemoveFromConference(RTRemoveFromConference* pReq, LPCVOID lpBuff);
	bool OnSetAgentGroup (RTSetAgentGroup* pReq, LPCVOID lpBuff);
	bool OnSetAgentState (RTSetAgentState* pReq, LPCVOID lpBuff);
	bool OnSetAgentActivity (RTSetAgentActivity* pReq, LPCVOID lpBuff);
	bool OnSendUUI(RTSendUserInfo* pReq, LPCVOID lpBuff);

// Internal methods
private:
	void InitializeStation();
	void InitializeVRU();
	void InitializeQueue();
	void InitializeRoutePoint();
	void InitializeDialer();
	void TranslateErrorCode(CTSPIRequest* pRequest, DWORD dwError);

	// Other event notifications
	void OnAgentStateChange(int iState);
	void OnAgentGroupChange(LPCTSTR pszAgentID, DWORD dwPrimary, DWORD dwSecondary);
	void OnCallStateChange(CTSPICallAppearance* pCall, const CPECallState* pCS);
	void OnPDialerMediaDetected(CTSPICallAppearance* pCall, int iAType);
	void OnTransferEvent(CTSPICallAppearance* pCall, DWORD dwOldCallID, DWORD dwTarget, const CPECallInfo* peCaller, const CPECallInfo* peCalled);
	void OnConferenceEvent(CTSPICallAppearance* pCall, DWORD dwCallID2, DWORD dwCallID3);
	CTSPICallAppearance* OnNewCallDetected(bool fPlaced, CTSPICallAppearance* pCall, DWORD dwCallID, const CPECallInfo* peCaller, const CPECallInfo* peCalled);
};

/**************************************************************************
** CJTPhone
**
** Phone object which manages a single phone on the switch.
**
***************************************************************************/
class CJTPhone : public CTSPIPhoneConnection
{
// Overrides from CTSPIPhoneConnection
public:
	CJTPhone() {/* */}
	inline CJTDevice* GetDeviceInfo() { return reinterpret_cast<CJTDevice*>(CTSPIPhoneConnection::GetDeviceInfo()); }
	virtual TStream& read(TStream& istm);
protected:
	virtual bool OpenDevice();
	virtual bool UnsolicitedEvent(LPCVOID lpBuff);
	virtual void OnTimer();

	// Create the event map
	DECLARE_TSPI_REQUESTS()

	// TSPI requests
	bool OnSetGain(RTSetGain* pReq, LPCVOID lpBuff);
	bool OnSetHookswitch(RTSetHookswitch* pReq, LPCVOID lpBuff);
	bool OnSetVolume(RTSetVolume* pReq, LPCVOID lpBuff);

// Internal methods
private:
	void TranslateErrorCode(CTSPIRequest* pRequest, DWORD dwError);
};

/*****************************************************************************
** Procedure:  CJTDevice::OnConnect
**
** Arguments:  true/false
**
** Returns:    void
**
** Description:  This function is called when a connection is made to
**               the PBX and when it is dropped.  It marks the lines as
**               connected/disconnected.
**
*****************************************************************************/
inline void CJTDevice::OnConnect(bool fConnect)
{
	// Mark each line as connected/disconnected based on our connection
	// to the PBX switch.  Note that stations use the INSERVICE bit to determine
	// if the station is ready to be used (i.e. an agent is logged on).
	for (unsigned int i = 0; i < GetLineCount(); i++)
		GetLineConnectionInfo(i)->DevStatusConnected(fConnect);

}// CJTDevice::OnConnect

/*****************************************************************************
** Procedure:  CJTDevice::DRV_Answer
**
** Arguments: 'pLine' - Line owner
**            'pCall' - Call to answer
**
** Returns:    void
**
** Description:  This function sends an answer request to the switch
**
*****************************************************************************/
inline void CJTDevice::DRV_Answer(const CJTLine* pLine, CTSPICallAppearance* pCall)
{
	GetPBX()->SendEvent(PBXCMD_AN, pLine->GetPermanentDeviceID(), pCall->GetCallID());

}// CJTDevice::DRV_Answer

/*****************************************************************************
** Procedure:  CJTDevice::DRV_DropCall
**
** Arguments: 'pLine' - Line owner
**            'pCall' - Call to drop
**
** Returns:    void
**
** Description:  This function sends a drop request to the switch
**
*****************************************************************************/
inline void CJTDevice::DRV_DropCall(const CJTLine* pLine, CTSPICallAppearance* pCall)
{
	GetPBX()->SendEvent(PBXCMD_RC, pLine->GetPermanentDeviceID(), pCall->GetCallID());

}// CJTDevice::DRV_DropCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_GenerateDigit
**
** Arguments: 'pLine' - Line owner
**            'pCall' - Call to drop
**            'chChar' - Digit to generate
**
** Returns:    void
**
** Description:  This function sends a GenerateDigit command
**
*****************************************************************************/
inline void CJTDevice::DRV_GenerateDigit(const CJTLine* pLine, CTSPICallAppearance* pCall, TCHAR chChar)
{
	GetPBX()->SendEvent(PBXCMD_GTD, pLine->GetPermanentDeviceID(), pCall->GetCallID(), chChar);

}// CJTDevice::DRV_DropCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_MakeCall
**
** Arguments: 'pLine' - Line Owner
**            'strExtension' - Number to dial
**            'dwCountryCode' - Ignored.
**
** Returns:    void
**
** Description:  This function sends a PlaceCall request to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_MakeCall(const CJTLine* pLine, const TString& strNumber, DWORD /*dwCountryCode*/)
{
	GetPBX()->SendEvent(PBXCMD_PC, pLine->GetPermanentDeviceID(), strNumber.c_str());

}// CJTDevice::DRV_PlaceCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_HoldCall
**
** Arguments: 'pLine' - Line owner
**            'pCall' - Call to place on hold
**
** Returns:    void
**
** Description:  This function sends a hold request to the switch
**
*****************************************************************************/
inline void CJTDevice::DRV_HoldCall(const CJTLine* pLine, CTSPICallAppearance* pCall)
{
	GetPBX()->SendEvent(PBXCMD_HC, pLine->GetPermanentDeviceID(), pCall->GetCallID());

}// CJTDevice::DRV_HoldCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_RetrieveCall
**
** Arguments: 'pLine' - Line owner
**            'pCall' - Call to retrieve
**
** Returns:    void
**
** Description:  This function sends a retrieve call request to the switch
**
*****************************************************************************/
inline void CJTDevice::DRV_RetrieveCall(const CJTLine* pLine, CTSPICallAppearance* pCall)
{
	GetPBX()->SendEvent(PBXCMD_RTC, pLine->GetPermanentDeviceID(), pCall->GetCallID());

}// CJTDevice::DRV_RetrieveCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_MakePredictiveCall
**
** Arguments: 'pLine' - Drop to drop call from
**            'strExtension' - Number to dial
**            'dwCountryCode' - Ignored.
**            'iTimeout' - Timeout for call
**            'strTransferTo' - Extension to transfer to when connected
**
** Returns:    void
**
** Description:  This function sends a PlacePredictiveCall request to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_MakePredictiveCall(const CJTLine* pLine, const TString& strNumber, DWORD /*dwCountryCode*/,
											  int iTimeout, const TString& strTarget)
{
	GetPBX()->SendEvent(PBXCMD_PPC, pLine->GetPermanentDeviceID(), strNumber.c_str(),
						iTimeout, strTarget.c_str());

}// CJTDevice::DRV_PlacePredictiveCall

/*****************************************************************************
** Procedure:  CJTDevice::DRV_BlindTransfer
**
** Arguments: 'pLine' - Line device owner
**            'pCall' - Call to transfer
**            'strAddress' - Address to transfer to
**
** Returns:    void
**
** Description:  This function sends a Blind Transfer/Redirect command to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_BlindTransfer(const CJTLine* pLine, CTSPICallAppearance* pCall, const TString& strAddress)
{
	GetPBX()->SendEvent(PBXCMD_BTC, pLine->GetPermanentDeviceID(), pCall->GetCallID(), strAddress.c_str());

}// CJTDevice::DRV_BlindTransfer

/*****************************************************************************
** Procedure:  CJTDevice::DRV_Transfer
**
** Arguments: 'pLine' - Line device owner
**            'pCall' - Call to transfer
**            'pConsult' - Call to transfer to
**
** Returns:    void
**
** Description:  This function sends a Transfer command to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_Transfer(const CJTLine* pLine, CTSPICallAppearance* pCall, CTSPICallAppearance* pConsult)
{
	GetPBX()->SendEvent(PBXCMD_TC, pLine->GetPermanentDeviceID(), pCall->GetCallID(), pConsult->GetCallID());

}// CJTDevice::DRV_Transfer

/*****************************************************************************
** Procedure:  CJTDevice::DRV_SetAgentState
**
** Arguments: 'pLine' - Line requesting change
**            'strAgent' - Agent on this line
**            'chState' - New state
**
** Returns:    void
**
** Description:  This function sends an AgentStateChange request to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_SetAgentState(const CJTLine* pLine, const TString& strAgent, TCHAR chState)
{
	GetPBX()->SendEvent(PBXCMD_CAS, pLine->GetPermanentDeviceID(), strAgent.c_str(), chState);

}// CJTDevice::DRV_SetAgentState

/*****************************************************************************
** Procedure:  CJTDevice::DRV_SetLogon
**
** Arguments: 'pLine' - Line requesting change
**            'strAgent' - Agent on this line
**            'strPassword' - Password
**            'dwGroup1' - Group 1
**            'dwGroup2' - Group 2
**
** Returns:    void
**
** Description:  This function sends an AgentLogon request to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_Logon(const CJTLine* pLine, const TString& strAgent, const TString& strPassword, DWORD dwGroup1, DWORD dwGroup2)
{
	GetPBX()->SendEvent(PBXCMD_ALO, pLine->GetPermanentDeviceID(), strAgent.c_str(), strPassword.c_str(), dwGroup1, dwGroup2);

}// CJTDevice::DRV_Logon

/*****************************************************************************
** Procedure:  CJTDevice::DRV_SetGain
**
** Arguments: 'pPhone' - Phone device to set gain for
**            'iGain' - New gain level
**
** Returns:    void
**
** Description:  This function sends a PhoneSetGain command to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_SetGain(const CJTPhone* pPhone, int iGain)
{
	GetPBX()->SendEvent(PBXCMD_SPG, pPhone->GetPermanentDeviceID(), iGain);

}// CJTDevice::DRV_SetGain

/*****************************************************************************
** Procedure:  CJTDevice::DRV_SetHookswitch
**
** Arguments: 'pPhone' - Phone device to set hookswitch for
**            'iGain' - New hookswitch state (0/1)
**
** Returns:    void
**
** Description:  This function sends a PhoneSetHookswitch command to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_SetHookswitch(const CJTPhone* pPhone, int iHS)
{
	GetPBX()->SendEvent(PBXCMD_SPH, pPhone->GetPermanentDeviceID(), iHS);

}// CJTDevice::DRV_SetHookswitch

/*****************************************************************************
** Procedure:  CJTDevice::DRV_SetVolume
**
** Arguments: 'pPhone' - Phone
**            'iVolume' - New volume level
**
** Returns:    void
**
** Description:  This function sends a Change Volume command to the PBX.
**
*****************************************************************************/
inline void CJTDevice::DRV_SetVolume(const CJTPhone* pPhone, int iVolume)
{
	GetPBX()->SendEvent(PBXCMD_SPV, pPhone->GetPermanentDeviceID(), iVolume);

}// CJTDevice::DRV_SetVolume

#endif // _JTSP_INC_
