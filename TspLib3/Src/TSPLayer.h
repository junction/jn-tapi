/******************************************************************************/
//                                                                        
// TSPLAYER.H - Layering function definitions
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This module defines the tsplib_xxx function which are the real entrypoint
// for each TSPI_xxx invocation.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _SPLIB_TSPLAYER_INC_
#define _SPLIB_TSPLAYER_INC_

/*---------------------------------------------------------------------------*/
// TSPI functions that are hooked
/*---------------------------------------------------------------------------*/
LONG TSPIAPI tsplib_lineAccept(DRV_REQUESTID, HDRVCALL, LPCSTR, DWORD);
LONG TSPIAPI tsplib_lineAddToConference(DRV_REQUESTID, HDRVCALL, HDRVCALL);
LONG TSPIAPI tsplib_lineAnswer(DRV_REQUESTID, HDRVCALL, LPCSTR, DWORD);
LONG TSPIAPI tsplib_lineBlindTransfer(DRV_REQUESTID, HDRVCALL, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_lineClose(HDRVLINE);
LONG TSPIAPI tsplib_lineCloseCall(HDRVCALL);
LONG TSPIAPI tsplib_lineCompleteCall(DRV_REQUESTID, HDRVCALL, LPDWORD, DWORD, DWORD);
LONG TSPIAPI tsplib_lineCompleteTransfer(DRV_REQUESTID, HDRVCALL, HDRVCALL, HTAPICALL, LPHDRVCALL, DWORD);
LONG TSPIAPI tsplib_lineConditionalMediaDetection(HDRVLINE, DWORD, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineDevSpecific(DRV_REQUESTID, HDRVLINE, DWORD, HDRVCALL, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineDevSpecificFeature(DRV_REQUESTID, HDRVLINE, DWORD, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineDial(DRV_REQUESTID, HDRVCALL, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_lineDrop(DRV_REQUESTID, HDRVCALL, LPCSTR, DWORD);
LONG TSPIAPI tsplib_lineForward(DRV_REQUESTID, HDRVLINE, DWORD, DWORD, LPLINEFORWARDLIST const, DWORD, HTAPICALL, LPHDRVCALL, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineGatherDigits(HDRVCALL, DWORD, DWORD, LPWSTR, DWORD, LPCWSTR, DWORD, DWORD);
LONG TSPIAPI tsplib_lineGenerateDigits(HDRVCALL, DWORD, DWORD, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_lineGenerateTone(HDRVCALL, DWORD, DWORD, DWORD, DWORD, LPLINEGENERATETONE const);
LONG TSPIAPI tsplib_lineGetAddressCaps(DWORD, DWORD, DWORD, DWORD, LPLINEADDRESSCAPS);
LONG TSPIAPI tsplib_lineGetAddressID(HDRVLINE, LPDWORD, DWORD, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_lineGetAddressStatus(HDRVLINE, DWORD, LPLINEADDRESSSTATUS);
LONG TSPIAPI tsplib_lineGetCallAddressID(HDRVCALL, LPDWORD);
LONG TSPIAPI tsplib_lineGetCallIDs(HDRVCALL, LPDWORD, LPDWORD, LPDWORD);
LONG TSPIAPI tsplib_lineGetCallInfo(HDRVCALL, LPLINECALLINFO);
LONG TSPIAPI tsplib_lineGetCallStatus(HDRVCALL, LPLINECALLSTATUS);
LONG TSPIAPI tsplib_lineGetDevCaps(DWORD, DWORD, DWORD, LPLINEDEVCAPS);
LONG TSPIAPI tsplib_lineGetDevConfig(DWORD, LPVARSTRING, LPCWSTR);
LONG TSPIAPI tsplib_lineGetExtensionID(DWORD, DWORD, LPLINEEXTENSIONID);
LONG TSPIAPI tsplib_lineGetIcon(DWORD, LPCWSTR, LPHICON);
LONG TSPIAPI tsplib_lineGetID(HDRVLINE, DWORD, HDRVCALL, DWORD, LPVARSTRING, LPCWSTR, HANDLE);
LONG TSPIAPI tsplib_lineGetLineDevStatus(HDRVLINE, LPLINEDEVSTATUS);
LONG TSPIAPI tsplib_lineGetNumAddressIDs(HDRVLINE, LPDWORD);
LONG TSPIAPI tsplib_lineHold(DRV_REQUESTID, HDRVCALL);
LONG TSPIAPI tsplib_lineMakeCall(DRV_REQUESTID, HDRVLINE, HTAPICALL, LPHDRVCALL, LPCWSTR, DWORD, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineMonitorDigits(HDRVCALL, DWORD);
LONG TSPIAPI tsplib_lineMonitorMedia(HDRVCALL, DWORD);
LONG TSPIAPI tsplib_lineMonitorTones(HDRVCALL, DWORD, LPLINEMONITORTONE const, DWORD);
LONG TSPIAPI tsplib_lineNegotiateExtVersion(DWORD, DWORD, DWORD, DWORD, LPDWORD);
LONG TSPIAPI tsplib_lineNegotiateTSPIVersion(DWORD, DWORD, DWORD, LPDWORD);
LONG TSPIAPI tsplib_lineOpen(DWORD, HTAPILINE, LPHDRVLINE, DWORD, LINEEVENT);
LONG TSPIAPI tsplib_linePark(DRV_REQUESTID, HDRVCALL, DWORD, LPCWSTR, LPVARSTRING);
LONG TSPIAPI tsplib_linePickup(DRV_REQUESTID, HDRVLINE, DWORD, HTAPICALL, LPHDRVCALL, LPCWSTR, LPCWSTR);
LONG TSPIAPI tsplib_linePrepareAddToConference(DRV_REQUESTID, HDRVCALL, HTAPICALL, LPHDRVCALL, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineRedirect(DRV_REQUESTID, HDRVCALL, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_lineReleaseUserUserInfo(DRV_REQUESTID, HDRVCALL);
LONG TSPIAPI tsplib_lineRemoveFromConference(DRV_REQUESTID, HDRVCALL);
LONG TSPIAPI tsplib_lineSecureCall(DRV_REQUESTID, HDRVCALL);
LONG TSPIAPI tsplib_lineSelectExtVersion(HDRVLINE, DWORD);
LONG TSPIAPI tsplib_lineSendUserUserInfo(DRV_REQUESTID, HDRVCALL, LPCSTR, DWORD);
LONG TSPIAPI tsplib_lineSetAppSpecific(HDRVCALL, DWORD);
LONG TSPIAPI tsplib_lineSetCallData(DRV_REQUESTID, HDRVCALL, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineSetCallParams(DRV_REQUESTID, HDRVCALL, DWORD, DWORD, DWORD, LPLINEDIALPARAMS const);
LONG TSPIAPI tsplib_lineSetCallQualityOfService(DRV_REQUESTID, HDRVCALL, LPVOID, DWORD, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineSetCallTreatment(DRV_REQUESTID, HDRVCALL, DWORD);
LONG TSPIAPI tsplib_lineSetCurrentLocation(DWORD);
LONG TSPIAPI tsplib_lineSetDefaultMediaDetection(HDRVLINE,DWORD);
LONG TSPIAPI tsplib_lineSetDevConfig(DWORD, LPVOID const, DWORD, LPCWSTR);
LONG TSPIAPI tsplib_lineSetLineDevStatus(DRV_REQUESTID, HDRVLINE, DWORD, DWORD);
LONG TSPIAPI tsplib_lineSetMediaControl(HDRVLINE, DWORD, HDRVCALL, DWORD, LPLINEMEDIACONTROLDIGIT const, DWORD, LPLINEMEDIACONTROLMEDIA const, DWORD, LPLINEMEDIACONTROLTONE const, DWORD, LPLINEMEDIACONTROLCALLSTATE const, DWORD);
LONG TSPIAPI tsplib_lineSetMediaMode(HDRVCALL,DWORD);
LONG TSPIAPI tsplib_lineSetQualityOfService (DRV_REQUESTID dwRequestID, HDRVCALL hdCall, LPVOID lpSendingFlowSpec, DWORD dwSendingFlowSpecSize, LPVOID lpReceivingFlowSpec, DWORD dwReceivingFlowSpecSize);
LONG TSPIAPI tsplib_lineSetStatusMessages(HDRVLINE, DWORD, DWORD);
LONG TSPIAPI tsplib_lineSetTerminal(DRV_REQUESTID, HDRVLINE, DWORD, HDRVCALL, DWORD, DWORD, DWORD, DWORD);
LONG TSPIAPI tsplib_lineSetupConference(DRV_REQUESTID, HDRVCALL, HDRVLINE, HTAPICALL, LPHDRVCALL, HTAPICALL, LPHDRVCALL, DWORD, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineSetupTransfer(DRV_REQUESTID, HDRVCALL, HTAPICALL, LPHDRVCALL, LPLINECALLPARAMS const);
LONG TSPIAPI tsplib_lineSwapHold(DRV_REQUESTID, HDRVCALL, HDRVCALL);
LONG TSPIAPI tsplib_lineUncompleteCall(DRV_REQUESTID, HDRVLINE, DWORD);
LONG TSPIAPI tsplib_lineUnhold(DRV_REQUESTID, HDRVCALL);
LONG TSPIAPI tsplib_lineUnpark(DRV_REQUESTID, HDRVLINE, DWORD, HTAPICALL, LPHDRVCALL, LPCWSTR);
LONG TSPIAPI tsplib_phoneClose(HDRVPHONE);
LONG TSPIAPI tsplib_phoneDevSpecific(DRV_REQUESTID, HDRVPHONE, LPVOID, DWORD);
LONG TSPIAPI tsplib_phoneGetButtonInfo(HDRVPHONE, DWORD, LPPHONEBUTTONINFO);
LONG TSPIAPI tsplib_phoneGetData(HDRVPHONE, DWORD, LPVOID, DWORD);
LONG TSPIAPI tsplib_phoneGetDevCaps(DWORD, DWORD, DWORD, LPPHONECAPS);
LONG TSPIAPI tsplib_phoneGetDisplay(HDRVPHONE, LPVARSTRING);
LONG TSPIAPI tsplib_phoneGetExtensionID(DWORD, DWORD, LPPHONEEXTENSIONID);
LONG TSPIAPI tsplib_phoneGetGain(HDRVPHONE, DWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneGetHookSwitch(HDRVPHONE, LPDWORD);
LONG TSPIAPI tsplib_phoneGetIcon(DWORD, LPCWSTR, LPHICON);
LONG TSPIAPI tsplib_phoneGetID(HDRVPHONE, LPVARSTRING, LPCWSTR, HANDLE);
LONG TSPIAPI tsplib_phoneGetLamp(HDRVPHONE, DWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneGetRing(HDRVPHONE, LPDWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneGetStatus(HDRVPHONE, LPPHONESTATUS);
LONG TSPIAPI tsplib_phoneGetVolume(HDRVPHONE, DWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneNegotiateExtVersion(DWORD, DWORD, DWORD, DWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneNegotiateTSPIVersion(DWORD, DWORD, DWORD, LPDWORD);
LONG TSPIAPI tsplib_phoneOpen(DWORD, HTAPIPHONE, LPHDRVPHONE, DWORD, PHONEEVENT);
LONG TSPIAPI tsplib_phoneSelectExtVersion(HDRVPHONE, DWORD);
LONG TSPIAPI tsplib_phoneSetButtonInfo(DRV_REQUESTID, HDRVPHONE, DWORD, LPPHONEBUTTONINFO const);
LONG TSPIAPI tsplib_phoneSetData(DRV_REQUESTID, HDRVPHONE, DWORD, LPVOID const, DWORD);
LONG TSPIAPI tsplib_phoneSetDisplay(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD, LPCWSTR, DWORD);
LONG TSPIAPI tsplib_phoneSetGain(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD);
LONG TSPIAPI tsplib_phoneSetHookSwitch(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD);
LONG TSPIAPI tsplib_phoneSetLamp(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD);
LONG TSPIAPI tsplib_phoneSetRing(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD);
LONG TSPIAPI tsplib_phoneSetStatusMessages(HDRVPHONE, DWORD, DWORD, DWORD);
LONG TSPIAPI tsplib_phoneSetVolume(DRV_REQUESTID, HDRVPHONE, DWORD, DWORD);
LONG TSPIAPI tsplib_providerCreateLineDevice(DWORD_PTR, DWORD);
LONG TSPIAPI tsplib_providerCreatePhoneDevice(DWORD_PTR, DWORD);
LONG TSPIAPI tsplib_providerEnumDevices(DWORD, LPDWORD, LPDWORD, HPROVIDER, LINEEVENT, PHONEEVENT);
LONG TSPIAPI tsplib_providerFreeDialogInstance(HDRVDIALOGINSTANCE);
LONG TSPIAPI tsplib_providerGenericDialogData(DWORD_PTR, DWORD, LPVOID, DWORD);
LONG TSPIAPI tsplib_providerInit(DWORD, DWORD, DWORD, DWORD, DWORD_PTR, DWORD_PTR, ASYNC_COMPLETION, LPDWORD);
LONG TSPIAPI tsplib_providerInstall(HWND, DWORD);
LONG TSPIAPI tsplib_providerRemove(HWND, DWORD);
LONG TSPIAPI tsplib_providerShutdown(DWORD, DWORD);
LONG TSPIAPI tsplib_providerUIIdentify(LPWSTR);
LONG TSPIAPI tsplib_lineSetAgentGroup(DRV_REQUESTID, DWORD, DWORD, LPLINEAGENTGROUPLIST const);
LONG TSPIAPI tsplib_lineSetAgentState(DRV_REQUESTID, DWORD, DWORD, DWORD, DWORD);
LONG TSPIAPI tsplib_lineSetAgentActivity(DRV_REQUESTID, DWORD, DWORD, DWORD);
LONG TSPIAPI tsplib_lineGetAgentStatus(DWORD, DWORD, LPLINEAGENTSTATUS);
LONG TSPIAPI tsplib_lineGetAgentCaps(DWORD, DWORD, LPLINEAGENTCAPS);
LONG TSPIAPI tsplib_lineGetAgentActivityList(DWORD, DWORD, LPLINEAGENTACTIVITYLIST);
LONG TSPIAPI tsplib_lineGetAgentGroupList(DWORD, DWORD, LPLINEAGENTGROUPLIST);
LONG TSPIAPI tsplib_lineAgentSpecific(DRV_REQUESTID, DWORD, DWORD, DWORD, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineCreateAgent(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENT lphAgent, LPCWSTR pszMachineName, LPCWSTR pszUserName, LPCWSTR pszAgentID, LPCWSTR pszAgentPIN);
LONG TSPIAPI tsplib_lineSetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwMeasurementPeriod);
LONG TSPIAPI tsplib_lineGetAgentInfo(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo);
LONG TSPIAPI tsplib_lineCreateAgentSession(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCWSTR pszAgentPIN, const GUID* pGUID, DWORD dwWorkingAddressID);
LONG TSPIAPI tsplib_lineGetAgentSessionList(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList);
LONG TSPIAPI tsplib_lineSetAgentSessionState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState);
LONG TSPIAPI tsplib_lineGetAgentSessionInfo(DWORD dwDeviceID, HAGENTSESSION hAgentSession, LPLINEAGENTSESSIONINFO lpSessionInfo);
LONG TSPIAPI tsplib_lineGetQueueList(DWORD dwDeviceID, const GUID* pGroupID, LPLINEQUEUELIST lpQueueList);
LONG TSPIAPI tsplib_lineSetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwQueueID, DWORD dwMeasurementPeriod);
LONG TSPIAPI tsplib_lineGetQueueInfo(DWORD dwDeviceID, DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo);
LONG TSPIAPI tsplib_lineGetGroupList(DWORD dwDeviceID, LPLINEAGENTGROUPLIST lpGroupList);
LONG TSPIAPI tsplib_lineSetAgentStateEx(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwState, DWORD dwNextState);
LONG TSPIAPI tsplib_lineMSPIdentify(DWORD, GUID*);
LONG TSPIAPI tsplib_lineCreateMSPInstance(HDRVLINE, DWORD, HTAPIMSPLINE, LPHDRVMSPLINE);
LONG TSPIAPI tsplib_lineCloseMSPInstance(HDRVMSPLINE);
LONG TSPIAPI tsplib_lineReceiveMSPData(HDRVLINE, HDRVCALL, HDRVMSPLINE, LPVOID, DWORD);
LONG TSPIAPI tsplib_lineGetCallHubTracking(HDRVLINE, LPLINECALLHUBTRACKINGINFO);
LONG TSPIAPI tsplib_lineSetCallHubTracking(HDRVLINE, LPLINECALLHUBTRACKINGINFO);

#endif // _SPLIB_TSPLAYER_INC_
