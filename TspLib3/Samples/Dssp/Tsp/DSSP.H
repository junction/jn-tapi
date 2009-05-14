/*****************************************************************************/
//
// DSSP.H - Digital Switch Service Provider Sample
//                                                                        
// This file contains the main service provider override class.
// All driver related calls are run through this class.
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

#ifndef _DSSP_INC_
#define _DSSP_INC_

#include "emintf.h"

// Define the states supported by all the state machines
#define STATE_CHECKFORDIALTONE (STATE_INITIAL+1)
#define STATE_DIALING          (STATE_INITIAL+2)
#define STATE_WAITFORONLINE    (STATE_INITIAL+3)
#define STATE_WAITFORCONNECT   (STATE_INITIAL+4)
#define STATE_WAITFOROFFLINE   (STATE_INITIAL+5)
#define STATE_CHECKFORHOLD     (STATE_INITIAL+6)
#define STATE_CHECKLAMP        (STATE_INITIAL+7)
#define STATE_STARTDROP        (STATE_INITIAL+8)
#define STATE_WAITFORCONF      (STATE_INITIAL+9)
#define STATE_WAITFORCOMPLETE  (STATE_INITIAL+10)

/**************************************************************************
** EVENTBUFF
**
** Event received from emulator
**
***************************************************************************/
typedef struct 
{
	DWORD dwResponse;
	DWORD dwAddress;
	DWORD dwError;
	LPVOID lpBuff;
	DWORD dwSize;
} EVENTBUFF;

/**************************************************************************
** CDSDevice
**
** Device object which manages a connection to the emulator.
**
***************************************************************************/
class CDSDevice : public CTSPIDevice
{
// Class data
protected:
	CWinThread* volatile m_pThreadInput; // Thread which processes asynch input from the emulator.
	bool volatile m_fConnected;			 // TRUE if we are connected to the emulator

// Constructor/Destructor
public:
	CDSDevice();
	virtual ~CDSDevice();

// Callable functions
public:
	bool SendThreadRequest (UINT uCommand, LPVOID pData, DWORD dwSize);
	bool SendThreadRequest (UINT uCommand, DWORD dwData);
	bool WaitForResponse(DWORD dwRequest, LPVOID lpSend, DWORD dwSendSize, LPVOID lpBuff, DWORD dwSize);

// Device specific functions
public:
	bool ConnectServer();
	bool DRV_Connect();
    bool DRV_GetSwitchSettings (LPEMSETTINGS lpSettings);
    bool DRV_GetAddressInfo (LPEMADDRESSINFO lpAddressInfo);
    bool DRV_PrepareCall (DWORD dwAddressID);
    bool DRV_DropCall (DWORD dwAddressID);
    bool DRV_Dial (DWORD dwAddressID, DIALINFO* pDialInfo, DWORD dwCountryCode);
    bool DRV_AnswerCall(DWORD dwAddressID);
    bool DRV_GetVersionInfo (LPEMVERSIONINFO lpVerInfo);
    bool DRV_HoldCall (DWORD dwAddressID);
    bool DRV_Flash (DWORD dwAddressID);
    bool DRV_UnholdCall (DWORD dwAddressID);
    bool DRV_Transfer (DWORD dwAddressID, LPCSTR lpszAddr, DWORD dwAddressToID=-1L, bool fConf=FALSE);
    bool DRV_Forward (DWORD dwAddressID, LPCSTR lpszAddr);
    bool DRV_SetGain (DWORD dwHSDevice, DWORD dwGain);
    bool DRV_SetVolume (DWORD dwHSDevice, DWORD dwGain);
    bool DRV_SetHookswitch (DWORD dwHSDevice, DWORD dwGain);
    bool DRV_Conference(DWORD dwAddressID, DWORD dwAddressID2, WORD wCommand);
    bool DRV_Redirect (DWORD dwAddressID, LPCSTR lpszAddr, DWORD dwCountryCode);
    bool DRV_SetRing (DWORD dwRingMode);
    bool DRV_CompleteCall (DWORD dwAddressID, DWORD dwComplType, LPCSTR pszMsg);
    bool DRV_Park (DWORD dwAddressID, LPCSTR pszAddr);
    bool DRV_Unpark (DWORD dwAddressID, LPCSTR pszAddr);

// Overriden functions from CTSPIDevice
public:
	virtual bool OpenDevice (CTSPIConnection* pConn);
protected:
	friend class CSocketMgr;
    virtual bool Init(DWORD dwProviderId, DWORD dwBaseLine, DWORD dwBasePhone, DWORD dwLines, DWORD dwPhones, HPROVIDER hProvider, ASYNC_COMPLETION lpfnCompletion);
};

/**************************************************************************
** CDSLine
**
** Line object which manages a single line on the switch.
**
***************************************************************************/
class CDSLine : public CTSPILineConnection
{
// Constructor/Destructor
public:
    CDSLine();
	virtual ~CDSLine();

// Overrides from CTSPILineConnection
public:
	CDSDevice* GetDeviceInfo() { return (CDSDevice*) CTSPIConnection::GetDeviceInfo(); }
	virtual bool UnsolicitedEvent(LPCVOID lpBuff);

protected:
	virtual void Init(CTSPIDevice* pDev, DWORD dwLineDeviceID, DWORD dwPos, DWORD dwItemData);
    virtual LONG GetDevConfig(CString& strDeviceClass, LPVARSTRING lpDeviceConfig);
    virtual LONG SetDevConfig(CString& strDeviceClass, LPVOID const lpDevConfig, DWORD dwSize);
	virtual DWORD OnCallFeaturesChanged (CTSPICallAppearance* pCall, DWORD dwFeatures);
	virtual DWORD OnAddressFeaturesChanged (CTSPIAddressInfo* pAddr, DWORD dwFeatures);
	virtual DWORD OnLineFeaturesChanged(DWORD dwFeatures);

	// Create the event map
	DECLARE_TSPI_REQUESTS()

// Internal functions
private:
	void UpdateCallState (DWORD dwAddressID, int iNewState, int iStateInfo, DWORD dwMediaModes);
	void HandleNewCall (DWORD dwAddressID, DWORD dwMediaModes, DWORD dwCompletionID, bool fExternal);
	bool HandleDialEvent(RTDialInfo* pReq, EVENTBUFF* pevBuff);

// TAPI function handlers
protected:
    bool processAnswer(RTAnswer* pReq, LPCVOID lpBuff);
    bool processBlindXfer(RTBlindTransfer* pReq, LPCVOID lpBuff);
    bool processCompleteCall(RTCompleteCall* pReq, LPCVOID lpBuff);
    bool processMakeCall(RTMakeCall* pReq, LPCVOID lpBuff);
    bool processDial(RTDial* pReq, LPCVOID lpBuff);
    bool processDropCall(RTDropCall* pReq, LPCVOID lpBuff);
    bool processGenDigits(RTGenerateDigits* pReq, LPCVOID lpBuff);
    bool processForward(RTForward* pReq, LPCVOID lpBuff);
    bool processHold(RTHold* pReq, LPCVOID lpBuff);
    bool processUnhold(RTUnhold* pReq, LPCVOID lpBuff);
    bool processSwapHold(RTSwapHold* pReq, LPCVOID lpBuff);
    bool processSetupXfer(RTSetupTransfer* pReq, LPCVOID lpBuff);
    bool processCompleteXfer(RTCompleteTransfer* pReq, LPCVOID lpBuff);
    bool processSetupConf(CTSPIRequest* pReq, LPCVOID lpBuff);
    bool processAddConf(RTAddToConference* pReq, LPCVOID lpBuff);
    bool processRemoveConf(RTRemoveFromConference* pReq, LPCVOID lpBuff);
    bool processRedirect(RTRedirect* pReq, LPCVOID lpBuff);
    bool processPark(RTPark* pReq, LPCVOID lpBuff);
    bool processUnpark(RTUnpark* pReq, LPCVOID lpBuff);
    bool processPickup(RTPickup* pReq, LPCVOID lpBuff);
};

/**************************************************************************
** CDSPhone
**
** Phone object which manages a single phone on the switch.
**
***************************************************************************/
class CDSPhone : public CTSPIPhoneConnection
{
// Constructor/Destructor
public:
    CDSPhone();
	virtual ~CDSPhone();

// CTSPIPhoneConnection Overrides
public:
	CDSDevice* GetDeviceInfo() { return (CDSDevice*) CTSPIConnection::GetDeviceInfo(); }
	virtual bool UnsolicitedEvent(LPCVOID lpBuff);
protected:
	virtual void Init(CTSPIDevice* pDev, DWORD dwPhoneDeviceID, DWORD dwPos, DWORD dwItemData);

	// Create the event map
	DECLARE_TSPI_REQUESTS()

// TAPI function handlers
protected:
    bool processSetGain(RTSetGain* pReq, LPCVOID lpBuff);
    bool processSetVol(RTSetVolume* pReq, LPCVOID lpBuff);
    bool processSetHook(RTSetHookswitch* pReq, LPCVOID lpBuff);
	bool processSetRing(RTSetRing* pReq, LPCVOID lpBuff);
};

/**************************************************************************
** CDSProvider
**
** Main provider object which manages the lifetime of the TSP
**
***************************************************************************/
class CDSProvider : public CServiceProvider
{
// Constructor
public:
    CDSProvider();

// TSPI Methods
public:
    virtual LONG providerEnumDevices(DWORD dwProviderId, LPDWORD lpNumLines, LPDWORD lpNumPhones, HPROVIDER hProvider, LINEEVENT lpfnLineCreateProc, PHONEEVENT lpfnPhoneCreateProc);
};

////////////////////////////////////////////////////////////////////////////
// Inline functions for CDSDevice

inline bool CDSDevice::DRV_GetVersionInfo (LPEMVERSIONINFO lpVerInfo)
{
	return WaitForResponse(EMCOMMAND_GETVERSION, NULL, 0, lpVerInfo, sizeof(EMVERSIONINFO));

}// CDSDevice::DRV_GetVersionInfo

inline bool CDSDevice::DRV_GetSwitchSettings (LPEMSETTINGS lpSettings)
{
	return WaitForResponse(EMCOMMAND_QUERYCAPS, NULL, 0, lpSettings, sizeof(EMSETTINGS));

}// CDSDevice::DRV_GetSwitchSettings

inline bool CDSDevice::DRV_GetAddressInfo (LPEMADDRESSINFO lpAddressInfo)
{
	WORD wAddr = lpAddressInfo->wAddressID;
	return WaitForResponse(EMCOMMAND_GETADDRESSINFO, &wAddr, sizeof(WORD), lpAddressInfo, sizeof(EMADDRESSINFO));
    
}// CDSDevice::DRV_GetAddressInfo

inline bool CDSDevice::DRV_PrepareCall (DWORD dwAddressID)
{
    return SendThreadRequest (EMCOMMAND_PREPAREADDR, dwAddressID);

}// CDSDevice::DRV_PrepareCall

inline bool CDSDevice::DRV_DropCall (DWORD dwAddressID)
{
    return SendThreadRequest (EMCOMMAND_DROPCALL, dwAddressID);

}// CDSDevice::DRV_DropCall

inline bool CDSDevice::DRV_AnswerCall(DWORD dwAddressID)
{                              
    return SendThreadRequest (EMCOMMAND_ANSWER, dwAddressID);

}// CDSDevice::DRV_AnswerCall

inline bool CDSDevice::DRV_HoldCall (DWORD dwAddressID)
{                            
    return SendThreadRequest (EMCOMMAND_HOLDCALL, dwAddressID);
    
}// CDSDevice::DRV_HoldCall 

inline bool CDSDevice::DRV_UnholdCall (DWORD dwAddressID)
{                            
    return SendThreadRequest (EMCOMMAND_UNHOLDCALL, dwAddressID);
    
}// CDSDevice::DRV_HoldCall

inline bool CDSDevice::DRV_Flash (DWORD dwAddressID)
{                         
    return SendThreadRequest (EMCOMMAND_FLASH, dwAddressID);

}// CDSDevice::DRV_Flash

inline bool CDSDevice::DRV_SetRing (DWORD dwRingMode)
{                           
    return SendThreadRequest (EMCOMMAND_SETRINGMODE, dwRingMode);
    
}// CDSDevice::DRV_SetRing


#endif // _DSSP_INC_
