/******************************************************************************/
//                                                                        
// SPDLL.CPP - Service Provider DLL shell.                                
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This module intercepts the TSPI calls and invokes the SP object        
// with the appropriate parameters.                                       
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
#include "tsplayer.h"
#include "debug.h"

#pragma warning(disable:4100)	// Unreferenced formal parameter warning

/*---------------------------------------------------------------------------*/
// CONSTANTS
/*---------------------------------------------------------------------------*/
#undef DLLEXPORT
#define DLLEXPORT extern "C" _declspec(naked)

/*---------------------------------------------------------------------------*/
// GLOBALS
/*---------------------------------------------------------------------------*/
#ifndef NO_TRACE
static CDebugMgr g_dbgMgr;
#endif

/******************************************************************************/
//
// TSPIAPI TSPI_line functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// TSPI_lineAccept
//
// This function accepts the specified offering call.  It may optionally
// send the specified User->User information to the calling party.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineAccept (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	_asm jmp tsplib_lineAccept;

}// TSPI_lineAccept

///////////////////////////////////////////////////////////////////////////
// TSPI_lineAddToConference
//
// This function adds the specified call (hdConsultCall) to the
// conference (hdConfCall).
//
DLLEXPORT
LONG TSPIAPI TSPI_lineAddToConference (DRV_REQUESTID dwRequestId, HDRVCALL hdConfCall, HDRVCALL hdConsultCall)
{
	_asm jmp tsplib_lineAddToConference;

}// TSPI_lineAddToConference

///////////////////////////////////////////////////////////////////////////
// TSPI_lineAnswer
//
// This function answers the specified offering call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineAnswer (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	_asm jmp tsplib_lineAnswer;

}// TSPI_lineAnswer

///////////////////////////////////////////////////////////////////////////
// TSPI_lineBlindTransfer
//
// This function performs a blind or single-step transfer of the
// specified call to the specified destination address.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineBlindTransfer (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCWSTR lpszDestAddr, DWORD dwCountryCode)
{
	_asm jmp tsplib_lineBlindTransfer;

}// TSPI_lineBlindTransfer

////////////////////////////////////////////////////////////////////////////
// TSPI_lineClose
//
// This function closes the specified open line after stopping all
// asynchronous requests on the line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineClose (HDRVLINE hdLine)
{
	_asm jmp tsplib_lineClose;

}// TSPI_lineClose

////////////////////////////////////////////////////////////////////////////
// TSPI_lineCloseCall
//
// This function closes the specified call.  The HDRVCALL handle will
// no longer be valid after this call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineCloseCall (HDRVCALL hdCall)
{
	_asm jmp tsplib_lineCloseCall;

}// TSPI_lineCloseCall

///////////////////////////////////////////////////////////////////////////
// TSPI_lineCloseMSPInstance
// 
// This function closes an MSP call instance. This function 
// requires TAPI 3.0 negotiation.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineCloseMSPInstance(HDRVMSPLINE hdMSPLine)
{
	_asm jmp tsplib_lineCloseMSPInstance;

}// TSPI_lineCloseMSPInstance

///////////////////////////////////////////////////////////////////////////
// TSPI_lineCompleteCall
//
// This function is used to specify how a call that could not be
// connected normally should be completed instead.  The network or
// switch may not be able to complete a call because the network
// resources are busy, or the remote station is busy or doesn't answer.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineCompleteCall (DRV_REQUESTID dwRequestId,
         HDRVCALL hdCall, LPDWORD lpdwCompletionID, DWORD dwCompletionMode,
         DWORD dwMessageID)
{
	_asm jmp tsplib_lineCompleteCall;

}// TSPI_lineCompleteCall

///////////////////////////////////////////////////////////////////////////
// TSPI_lineCompleteTransfer
//
// This function completes the transfer of the specified call to the
// party connected in the consultation call.  If 'dwTransferMode' is
// LINETRANSFERMODE_CONFERENCE, the original call handle is changed
// to a conference call.  Otherwise, the service provider should send
// callstate messages change all the calls to idle.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineCompleteTransfer (DRV_REQUESTID dwRequestId,
         HDRVCALL hdCall, HDRVCALL hdConsultCall, HTAPICALL htConfCall,
         LPHDRVCALL lphdConfCall, DWORD dwTransferMode)
{
	_asm jmp tsplib_lineCompleteTransfer;

}// TSPI_lineCompleteTransfer   

////////////////////////////////////////////////////////////////////////////
// TSPI_lineConditionalMediaDetection
//
// This function is invoked by TAPI.DLL when the application requests a
// line open using the LINEMAPPER.  This function will check the 
// requested media modes and return an acknowledgement based on whether 
// we can monitor all the requested modes.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineConditionalMediaDetection (HDRVLINE hdLine,
         DWORD dwMediaModes, LPLINECALLPARAMS const lpCallParams)
{
	_asm jmp tsplib_lineConditionalMediaDetection;

}// TSPI_lineConditionalMediaDetection

///////////////////////////////////////////////////////////////////////////
// TSPI_lineCreateMSPInstance
// 
// This function creates a media service provider instance for a specific
// line device. This function returns a TSP handle for the MSP call. It
// requires TAPI 3.0 negotiation.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineCreateMSPInstance(HDRVLINE hdLine, DWORD dwAddressID, HTAPIMSPLINE htMSPLine, LPHDRVMSPLINE lphdMSPLine)
{
	_asm jmp tsplib_lineCreateMSPInstance;

}// TSPI_lineCreateMSPInstance

///////////////////////////////////////////////////////////////////////////
// TSPI_lineDevSpecific
//
// This function is used as a general extension mechanims to allow
// service providers to provide access to features not described in
// other operations.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineDevSpecific (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD dwAddressId, HDRVCALL hdCall, LPVOID lpParams, DWORD dwSize)
{
	_asm jmp tsplib_lineDevSpecific;

}// TSPI_lineDevSpecific

///////////////////////////////////////////////////////////////////////////
// TSPI_lineDevSpecificFeature
//
// This function is used as an extension mechanism to enable service
// providers to provide access to features not described in other
// operations.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineDevSpecificFeature (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD dwFeature, LPVOID lpParams, DWORD dwSize)
{
	_asm jmp tsplib_lineDevSpecificFeature;

}// TSPI_lineDevSpecificFeature

///////////////////////////////////////////////////////////////////////////
// TSPI_lineDial
//
// This function dials the specified dialable number on the specified
// call device.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineDial (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCWSTR lpszDestAddress, DWORD dwCountryCode)
{
	_asm jmp tsplib_lineDial;

}// TSPI_lineDial

////////////////////////////////////////////////////////////////////////////
// TSPI_lineDrop
//
// This function disconnects the specified call.  The call is still
// valid and should be closed by the application following this API.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineDrop (DRV_REQUESTID dwRequestID, HDRVCALL hdCall, 
         LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	_asm jmp tsplib_lineDrop;

}// TSPI_lineDrop

///////////////////////////////////////////////////////////////////////////
// TSPI_lineForward
//
// This function forwards calls destined for the specified address on
// the specified line, according to the specified forwarding instructions.
// When an origination address is forwarded, the incoming calls for that
// address are deflected to the other number by the switch.  This function
// provides a combination of forward and do-not-disturb features.  This
// function can also cancel specific forwarding currently in effect.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineForward (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD bAllAddresses, DWORD dwAddressId, 
         LPLINEFORWARDLIST const lpForwardList, DWORD dwNumRingsAnswer,
         HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
         LPLINECALLPARAMS const lpCallParams)
{
	_asm jmp tsplib_lineForward;

}// TSPI_lineForward

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGatherDigits
//
// This function initiates the buffered gathering of digits on the 
// specified call.  TAPI.DLL specifies a buffer in which to place the digits,
// and the maximum number of digits to be collected.
//
// Digit collection may be terminated in the following ways:
//
//  (1)  The requested number of digits is collected.
//
//  (2)  One of the digits detected matches a digit in 'szTerminationDigits'
//       before the specified number of digits is collected.  The detected
//       termination digit is added to the buffer and the buffer is returned.
// 
//  (3)  One of the timeouts expires.  The 'dwFirstDigitTimeout' expires if
//       the first digit is not received in this time period.  The 
//       'dwInterDigitTimeout' expires if the second, third (and so on) digit
//       is not received within that time period, and a partial buffer is 
//       returned.
//
//  (4)  Calling this function again while digit gathering is in process.
//       The old collection session is terminated, and the contents is
//       undefined.  The mechanism for canceling without restarting this
//       event is to invoke this function with 'lpszDigits' equal to NULL.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGatherDigits (HDRVCALL hdCall, DWORD dwEndToEnd,
         DWORD dwDigitModes, LPWSTR lpszDigits, DWORD dwNumDigits,
         LPCWSTR lpszTerminationDigits, DWORD dwFirstDigitTimeout,
         DWORD dwInterDigitTimeout)
{
	_asm jmp tsplib_lineGatherDigits;

}// TSPI_lineGatherDigits

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGenerateDigits
//
// This function initiates the generation of the specified digits
// using the specified signal mode.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGenerateDigits (HDRVCALL hdCall, DWORD dwEndToEndID,
         DWORD dwDigitMode, LPCWSTR lpszDigits, DWORD dwDuration)
{
	_asm jmp tsplib_lineGenerateDigits;

}// TSPI_lineGenerateDigits

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGenerateTone
//
// This function generates the specified tone inband over the specified
// call.  Invoking this function with a zero for 'dwToneMode' aborts any
// tone generation currently in progress on the specified call.
// Invoking 'lineGenerateTone' or 'lineGenerateDigit' also aborts the
// current tone generation and initiates the generation of the newly
// specified tone or digits.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGenerateTone (HDRVCALL hdCall, DWORD dwEndToEnd,
         DWORD dwToneMode, DWORD dwDuration, DWORD dwNumTones,
         LPLINEGENERATETONE const lpTones)
{
	_asm jmp tsplib_lineGenerateTone;

}// TSPI_lineGenerateTone

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAddressCaps
//
// This function queries the specified address on the specified
// line device to determine its telephony capabilities.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetAddressCaps (DWORD dwDeviceID, DWORD dwAddressID,
         DWORD dwTSPIVersion, DWORD dwExtVersion, LPLINEADDRESSCAPS lpAddressCaps)
{
	_asm jmp tsplib_lineGetAddressCaps;

}// TSPI_lineGetAddressCaps

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAddressID
//
// This function returns the specified address associated with the
// specified line in a specified format.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetAddressID (HDRVLINE hdLine, LPDWORD lpdwAddressID, 
         DWORD dwAddressMode, LPCWSTR lpszAddress, DWORD dwSize)
{
	_asm jmp tsplib_lineGetAddressID;

}// TSPI_lineGetAddressID

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetCallIDs
//
// This function retrieves call-id information for this call. It is used
// by TAPI as a quick way to retrieve call-id information rather than
// using the full lineGetCallInfo function.
//
// TAPI 2.2 and 3.0
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetCallIDs(HDRVCALL hdCall, LPDWORD lpdwAddressID, LPDWORD lpdwCallID, LPDWORD lpdwRelatedCallID)
{
	_asm jmp tsplib_lineGetCallIDs;

}// TSPI_lineGetCallIDs

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAddressStatus
//
// This function queries the specified address for its current status.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetAddressStatus (HDRVLINE hdLine, DWORD dwAddressID,
         LPLINEADDRESSSTATUS lpAddressStatus)
{
	_asm jmp tsplib_lineGetAddressStatus;

}// TSPI_lineGetAddressStatus

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetCallAddressID
//
// This function retrieves the address for the specified call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetCallAddressID (HDRVCALL hdCall, LPDWORD lpdwAddressID)
{
	_asm jmp tsplib_lineGetCallAddressID;

}// TSPI_lineGetCallAddressID

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetCallHubTracking
//
// This function retrieves the call hub tracking support structure (TAPI 3.0)
//
DLLEXPORT 
LONG TSPIAPI TSPI_lineGetCallHubTracking(HDRVLINE hdLine, LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	_asm jmp tsplib_lineGetCallHubTracking;

}// TSPI_lineGetCallHubTracking

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetCallInfo
//
// This function retrieves the telephony information for the specified
// call handle.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetCallInfo (HDRVCALL hdCall, LPLINECALLINFO lpCallInfo)
{
	_asm jmp tsplib_lineGetCallInfo;

}// TSPI_lineGetCallInfo

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetCallStatus
//
// This function retrieves the status for the specified call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetCallStatus (HDRVCALL hdCall, LPLINECALLSTATUS lpCallStatus)
{
	_asm jmp tsplib_lineGetCallStatus;

}// TSPI_lineGetCallStatus

///////////////////////////////////////////////////////////////////////////
// TSPI_lineGetDevCaps
//
// This function retrieves the telephony device capabilties for the
// specified line.  This information is valid for all addresses on 
// the line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetDevCaps (DWORD dwDeviceID, DWORD dwTSPIVersion, 
         DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps)
{
	_asm jmp tsplib_lineGetDevCaps;

}// TSPI_lineGetDevCaps

//////////////////////////////////////////////////////////////////////////
// TSPI_lineGetDevConfig
//
// This function returns a data structure object, the contents of which
// are specific to the line (SP) and device class, giving the current
// configuration of a device associated one-to-one with the line device.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetDevConfig (DWORD dwDeviceID, LPVARSTRING lpDeviceConfig,
         LPCWSTR lpszDeviceClass)
{
	_asm jmp tsplib_lineGetDevConfig;

}// TSPI_lineGetDevConfig

//////////////////////////////////////////////////////////////////////////
// TSPI_lineGetExtensionID
//
// This function returns the extension ID that the service provider
// supports for the indicated line device.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetExtensionID (DWORD dwDeviceID, DWORD dwTSPIVersion,
         LPLINEEXTENSIONID lpExtensionID)
{
	_asm jmp tsplib_lineGetExtensionID;

}// TSPI_lineGetExtensionID

//////////////////////////////////////////////////////////////////////////
// TSPI_lineGetIcon
//
// This function retreives a service line device-specific icon for
// display to the user
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetIcon (DWORD dwDeviceID, LPCWSTR lpszDeviceClass,
      LPHICON lphIcon)
{
	_asm jmp tsplib_lineGetIcon;

}// TSPI_lineGetIcon

//////////////////////////////////////////////////////////////////////////
// TSPI_lineGetID
//
// This function returns a device id for the specified
// device class associated with the specified line, address, or call
// handle.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetID (HDRVLINE hdLine, DWORD dwAddressID,
         HDRVCALL hdCall, DWORD dwSelect, LPVARSTRING lpVarString,
         LPCWSTR lpszDeviceClass, HANDLE hTargetProcess)
{
	_asm jmp tsplib_lineGetID;

}// TSPI_lineGetID

////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetLineDevStatus
//
// This function queries the specified open line for its status.  The
// information is valid for all addresses on the line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetLineDevStatus (HDRVLINE hdLine, LPLINEDEVSTATUS lpLineDevStatus)
{
	_asm jmp tsplib_lineGetLineDevStatus;

}// TSPI_lineGetLineDevStatus

////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetNumAddressIDs
//
// This function returns the number of addresses availble on a line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineGetNumAddressIDs (HDRVLINE hdLine, LPDWORD lpNumAddressIDs)
{
	_asm jmp tsplib_lineGetNumAddressIDs;

}// TSPI_lineGetNumAddressIDs

////////////////////////////////////////////////////////////////////////////
// TSPI_lineHold
//
// This function places the specified call appearance on hold.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineHold (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	_asm jmp tsplib_lineHold;

}// TSPI_lineHold

////////////////////////////////////////////////////////////////////////////
// TSPI_lineMakeCall
//
// This function places a call on the specified line to the specified
// address.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineMakeCall (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         HTAPICALL htCall, LPHDRVCALL lphdCall, LPCWSTR lpszDestAddress,
         DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams)
{
	_asm jmp tsplib_lineMakeCall;

}// TSPI_lineMakeCall

///////////////////////////////////////////////////////////////////////////
// TSPI_lineMonitorDigits
//
// This function enables and disables the unbuffered detection of digits
// received on the call.  Each time a digit of the specified digit mode(s)
// is detected, a LINE_MONITORDIGITS message is sent to the application by
// TAPI.DLL, indicating which digit was detected.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineMonitorDigits (HDRVCALL hdCall, DWORD dwDigitModes)
{
	_asm jmp tsplib_lineMonitorDigits;

}// TSPI_lineMonitorDigits

///////////////////////////////////////////////////////////////////////////
// TSPI_lineMonitorMedia
//
// This function enables and disables the detection of media modes on 
// the specified call.  When a media mode is detected, a LINE_MONITORMEDIA
// message is sent to TAPI.DLL.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineMonitorMedia (HDRVCALL hdCall, DWORD dwMediaModes)
{
	_asm jmp tsplib_lineMonitorMedia;

}// TSPI_lineMonitorMedia

///////////////////////////////////////////////////////////////////////////
// TSPI_lineMonitorTones
// 
// This function enables and disables the detection of inband tones on
// the call.  Each time a specified tone is detected, a message is sent
// to the client application through TAPI.DLL
//
DLLEXPORT
LONG TSPIAPI TSPI_lineMonitorTones (HDRVCALL hdCall, DWORD dwToneListID,
         LPLINEMONITORTONE const lpToneList, DWORD dwNumEntries)
{
	_asm jmp tsplib_lineMonitorTones;

}// TSPI_lineMonitorTones

///////////////////////////////////////////////////////////////////////////
// TSPI_lineMSPIdentify
// 
// This function determines the associated MSP CLSID for each line
// device. This function requires TAPI 3.0 negotiation.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineMSPIdentify(DWORD dwDeviceID, GUID* pCLSID)
{
	_asm jmp tsplib_lineMSPIdentify;
}

///////////////////////////////////////////////////////////////////////////
// TSPI_lineNegotiateExtVersion
//
// This function returns the highest extension version number the SP is
// willing to operate under for the device given the range of possible
// extension versions.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineNegotiateExtVersion (DWORD dwDeviceID, DWORD dwTSPIVersion,
         DWORD dwLowVersion, DWORD dwHiVersion, LPDWORD lpdwExtVersion)
{
	_asm jmp tsplib_lineNegotiateExtVersion;

}// TSPI_lineNegotiateExtVersion

///////////////////////////////////////////////////////////////////////////
// TSPI_lineNegotiateTSPIVersion
//
// This function is called to negotiate line versions for the TSP
// driver.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineNegotiateTSPIVersion (DWORD dwDeviceID,             
         DWORD dwLowVersion, DWORD dwHighVersion, LPDWORD lpdwTSPIVersion)
{
	_asm jmp tsplib_lineNegotiateTSPIVersion;

}// TSPI_lineNegotiateTSPIVersion

////////////////////////////////////////////////////////////////////////////
// TSPI_lineOpen
//
// This function opens the specified line device based on the device
// id passed and returns a handle for the line.  The TAPI.DLL line
// handle must also be retained for further interaction with this
// device.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineOpen (DWORD dwDeviceID, HTAPILINE htLine, 
         LPHDRVLINE lphdLine, DWORD dwTSPIVersion, LINEEVENT lpfnEventProc)
{
	_asm jmp tsplib_lineOpen;

}// TSPI_lineOpen

//////////////////////////////////////////////////////////////////////////////
// TSPI_linePark
//
// This function parks the specified call according to the specified
// park mode.
//
DLLEXPORT
LONG TSPIAPI TSPI_linePark (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         DWORD dwParkMode, LPCWSTR lpszDirAddr, LPVARSTRING lpNonDirAddress)
{
	_asm jmp tsplib_linePark;

}// TSPI_linePark

///////////////////////////////////////////////////////////////////////////////
// TSPI_linePickup
//
// This function picks up a call alerting at the specified destination
// address and returns a call handle for the picked up call.  If invoked
// with a NULL for the 'lpszDestAddr' parameter, a group pickup is performed.
// If required by the device capabilities, 'lpszGroupID' specifies the
// group ID to which the alerting station belongs.
//
DLLEXPORT
LONG TSPIAPI TSPI_linePickup (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HTAPICALL htCall, LPHDRVCALL lphdCall,
         LPCWSTR lpszDestAddr, LPCWSTR lpszGroupID)
{
	_asm jmp tsplib_linePickup;

}// TSPI_linePickup

////////////////////////////////////////////////////////////////////////////////
// TSPI_linePrepareAddToConference
//
// This function prepares an existing conference call for the addition of
// another party.  It creates a new temporary consultation call.  The new
// consultation call can subsequently be added to the conference call.
//
DLLEXPORT
LONG TSPIAPI TSPI_linePrepareAddToConference (DRV_REQUESTID dwRequestID,
         HDRVCALL hdConfCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
         LPLINECALLPARAMS const lpCallParams)
{
	_asm jmp tsplib_linePrepareAddToConference;

}// TSPI_linePrepareAddToConference

///////////////////////////////////////////////////////////////////////////
// TSPI_lineReceiveMSPData
// 
// This function receives data sent by a media service provider (MSP).
// It requires TAPI 3.0 negotiation.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineReceiveMSPData(HDRVLINE hdLine, HDRVCALL hdCall, HDRVMSPLINE hdMSPLine, LPVOID pBuffer, DWORD dwSize)
{
	_asm jmp tsplib_lineReceiveMSPData;
	
}// TSPI_lineReceiveMSPData

/////////////////////////////////////////////////////////////////////////////////
// TSPI_lineRedirect
//
// This function redirects the specified offering call to the specified
// destination address.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineRedirect (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCWSTR lpszDestAddr, DWORD dwCountryCode)
{
	_asm jmp tsplib_lineRedirect;

}// TSPI_lineRedirect

/////////////////////////////////////////////////////////////////////////////////
// TSPI_lineReleaseUserUserInfo
//
// This function releases a block of User->User information which is stored
// in the CALLINFO record.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineReleaseUserUserInfo (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	_asm jmp tsplib_lineReleaseUserUserInfo;

}// TSPI_lineReleaseUserUserInfo

/////////////////////////////////////////////////////////////////////////////////
// TSPI_lineRemoveFromConference
//
// This function removes the specified call from the conference call to
// which it currently belongs.  The remaining calls in the conference call
// are unaffected.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineRemoveFromConference (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	_asm jmp tsplib_lineRemoveFromConference;

}// TSPI_lineRemoveFromConference

///////////////////////////////////////////////////////////////////////////////////
// TSPI_lineSecureCall
//
// This function secures the call from any interruptions or interference
// that may affect the call's media stream.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSecureCall (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	_asm jmp tsplib_lineSecureCall;

}// TSPI_lineSecureCall

///////////////////////////////////////////////////////////////////////////////
// TSPI_lineSelectExtVersion
//
// This function selects the indicated extension version for the indicated
// line device.  Subsequent requests operate according to that extension
// version.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSelectExtVersion (HDRVLINE hdLine, DWORD dwExtVersion)
{
	_asm jmp tsplib_lineSelectExtVersion;

}// TSPI_lineSelectExtVersion

//////////////////////////////////////////////////////////////////////////////
// TSPI_lineSendUserUserInfo
//
// This function sends user-to-user information to the remote party on the
// specified call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSendUserUserInfo (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	_asm jmp tsplib_lineSendUserUserInfo;

}// TSPI_lineSendUserUserInfo
                                                          
//////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAppSpecific
//
// This function sets the application specific portion of the 
// LINECALLINFO structure.  This is returned by the TSPI_lineGetCallInfo
// function.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetAppSpecific (HDRVCALL hdCall, DWORD dwAppSpecific)
{
	_asm jmp tsplib_lineSetAppSpecific;

}// TSPI_lineSetAppSpecific

/////////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCallData
//
// This function sets CALLDATA into a calls CALLINFO record.
//
// Added for v2.0
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetCallData (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
								   LPVOID lpCallData, DWORD dwSize)
{
	_asm jmp tsplib_lineSetCallData;

}// TSPI_lineSetCallData

///////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCallHubTracking
//
// This function sets the call hub tracking support structure (TAPI 3.0)
//
DLLEXPORT 
LONG TSPIAPI TSPI_lineSetCallHubTracking(HDRVLINE hdLine, LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	_asm jmp tsplib_lineSetCallHubTracking;

}// TSPI_lineGetCallHubTracking

/////////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCallParams
//
// This function sets certain parameters for an existing call.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetCallParams (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate, 
         LPLINEDIALPARAMS const lpDialParams)
{
	_asm jmp tsplib_lineSetCallParams;

}// TSPI_lineSetCallParams

///////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCallTreatment
//
// Sets the call treatment for the specified call.  If the call
// treatment can go into effect then it happens immediately,
// otherwise it is set into place the next time the call enters
// a state where the call treatment is valid.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetCallTreatment (DRV_REQUESTID dwRequestID,
					HDRVCALL hdCall, DWORD dwCallTreatment)
{
	_asm jmp tsplib_lineSetCallTreatment;

}// TSPI_lineSetCallTreatment

////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCurrentLocation
//
// This function is called by TAPI whenever the address translation location
// is changed by the user (in the Dial Helper dialog or 
// 'lineSetCurrentLocation' function.  SPs which store parameters specific
// to a location (e.g. touch-tone sequences specific to invoke a particular
// PBX function) would use the location to select the set of parameters 
// applicable to the new location.
// 
DLLEXPORT
LONG TSPIAPI TSPI_lineSetCurrentLocation (DWORD dwLocation)
{
	_asm jmp tsplib_lineSetCurrentLocation;

}// TSPI_lineSetCurrentLocation

////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetDefaultMediaDetection
//
// This function tells us the new set of media modes to watch for on 
// this line (inbound or outbound).
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetDefaultMediaDetection (HDRVLINE hdLine, DWORD dwMediaModes)
{
	_asm jmp tsplib_lineSetDefaultMediaDetection;

}// TSPI_lineSetDefaultMediaDetection

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetDevConfig
//
// This function restores the configuration of a device associated one-to-one
// with the line device from a data structure obtained through TSPI_lineGetDevConfig.
// The contents of the data structure are specific to the service provider.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetDevConfig (DWORD dwDeviceID, LPVOID const lpDevConfig,
         DWORD dwSize, LPCWSTR lpszDeviceClass)
{
	_asm jmp tsplib_lineSetDevConfig;

}// TSPI_lineSetDevConfig

////////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetMediaControl
//
// This function enables and disables control actions on the media stream
// associated with the specified line, address, or call.  Media control actions
// can be triggered by the detection of specified digits, media modes,
// custom tones, and call states.  The new specified media controls replace all
// the ones that were in effect for this line, address, or call prior to this
// request.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetMediaControl (HDRVLINE hdLine, DWORD dwAddressID, 
         HDRVCALL hdCall, DWORD dwSelect, 
         LPLINEMEDIACONTROLDIGIT const lpDigitList, DWORD dwNumDigitEntries, 
         LPLINEMEDIACONTROLMEDIA const lpMediaList, DWORD dwNumMediaEntries, 
         LPLINEMEDIACONTROLTONE const lpToneList, DWORD dwNumToneEntries, 
         LPLINEMEDIACONTROLCALLSTATE const lpCallStateList, DWORD dwNumCallStateEntries)
{
	_asm jmp tsplib_lineSetMediaControl;

}// TSPI_lineSetMediaControl

////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetMediaMode
//
// This function changes the provided calls media in the LINECALLSTATE
// structure.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetMediaMode (HDRVCALL hdCall, DWORD dwMediaMode)
{
	_asm jmp tsplib_lineSetMediaMode;

}// TSPI_lineSetMediaMode

///////////////////////////////////////////////////////////////////////////
// TSPI_lineSetCallQualityOfService
//
// This function attempts to nogotiate a level of QOS on the call with
// the switch.  If the desired QOS is not available, then it returns an
// error and remains at the current level of QOS.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetCallQualityOfService (DRV_REQUESTID dwRequestID,
					HDRVCALL hdCall, LPVOID lpSendingFlowSpec,
					DWORD dwSendingFlowSpecSize, LPVOID lpReceivingFlowSpec,
					DWORD dwReceivingFlowSpecSize)
{
	_asm jmp tsplib_lineSetQualityOfService;

}// TSPI_lineSetCallQualityOfService

///////////////////////////////////////////////////////////////////////////
// TSPI_lineSetStatusMessages
//
// This function tells us which events to notify TAPI about when
// address or status changes about the specified line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetStatusMessages (HDRVLINE hdLine, DWORD dwLineStates,
         DWORD dwAddressStates)
{
	_asm jmp tsplib_lineSetStatusMessages;

}// TSPI_lineSetStatusMessages

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetTerminal
//
// This operation enables TAPI.DLL to specify to which terminal information
// related to a specified line, address, or call is to be routed.  This
// can be used while calls are in progress on the line, to allow events
// to be routed to different devices as required.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetTerminal (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HDRVCALL hdCall, DWORD dwSelect, 
         DWORD dwTerminalModes, DWORD dwTerminalID, DWORD bEnable)
{
	_asm jmp tsplib_lineSetTerminal;

}// TSPI_lineSetTerminal

////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetupConference
//
// This function sets up a conference call for the addition of a third 
// party.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetupConference (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         HDRVLINE hdLine, HTAPICALL htConfCall, LPHDRVCALL lphdConfCall,
         HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, DWORD dwNumParties,
         LPLINECALLPARAMS const lpLineCallParams)
{
	_asm jmp tsplib_lineSetupConference;

}// TSPI_lineSetupConference

////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetupTransfer
//
// This function sets up a call for transfer to a destination address.
// A new call handle is created which represents the destination
// address.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetupTransfer (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
       HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
       LPLINECALLPARAMS const lpCallParams)
{
	_asm jmp tsplib_lineSetupTransfer;

}// TSPI_lineSetupTransfer

//////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetLineDevStatus
//
// The service provider sets the device status as indicated,
// sending the appropriate LINEDEVSTATE messages to indicate the
// new status.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSetLineDevStatus (DRV_REQUESTID dwRequestID,
					HDRVLINE hdLine, DWORD dwStatusToChange,
					DWORD fStatus)
{
	_asm jmp tsplib_lineSetLineDevStatus;

}// TSPI_lineSetLineDevStatus

//////////////////////////////////////////////////////////////////////////////
// TSPI_lineSwapHold
//
// This function swaps the specified active call with the specified
// call on hold.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineSwapHold (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
       HDRVCALL hdHeldCall)
{
	_asm jmp tsplib_lineSwapHold;

}// TSPI_lineSwapHold

////////////////////////////////////////////////////////////////////////////
// TSPI_lineUncompleteCall
//
// This function is used to cancel the specified call completion request
// on the specified line.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineUncompleteCall (DRV_REQUESTID dwRequestID,
         HDRVLINE hdLine, DWORD dwCompletionID)
{
	_asm jmp tsplib_lineUncompleteCall;

}// TSPI_lineUncompleteCall

////////////////////////////////////////////////////////////////////////////
// TSPI_lineUnhold
//
// This function retrieves the specified held call
//
DLLEXPORT
LONG TSPIAPI TSPI_lineUnhold (DRV_REQUESTID dwRequestId, HDRVCALL hdCall)
{
	_asm jmp tsplib_lineUnhold;

}// TSPI_lineUnhold

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineUnpark
//
// This function retrieves the call parked at the specified
// address and returns a call handle for it.
//
DLLEXPORT
LONG TSPIAPI TSPI_lineUnpark (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HTAPICALL htCall, LPHDRVCALL lphdCall, 
         LPCWSTR lpszDestAddr)
{
	_asm jmp tsplib_lineUnpark;

}// TSPI_lineUnpark

/******************************************************************************/
//
// TSPI_phone functions
//
/******************************************************************************/

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneClose
//
// This function closes the specified open phone device after completing
// or aborting all outstanding asynchronous requests on the device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneClose (HDRVPHONE hdPhone)
{
	_asm jmp tsplib_phoneClose;

}// TSPI_phoneClose

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneDevSpecific
//
// This function is used as a general extension mechanism to enable
// a TAPI implementation to provide features not generally available
// to the specification.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneDevSpecific (DRV_REQUESTID dwRequestID, HDRVPHONE hdPhone,
               LPVOID lpParams, DWORD dwSize)
{
	_asm jmp tsplib_phoneDevSpecific;

}// TSPI_phoneDevSpecific

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetButtonInfo
//
// This function returns information about the specified phone 
// button.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetButtonInfo (HDRVPHONE hdPhone, DWORD dwButtonId,
               LPPHONEBUTTONINFO lpButtonInfo)
{
	_asm jmp tsplib_phoneGetButtonInfo;

}// TSPI_phoneGetButtonInfo

////////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetData
//
// This function uploads the information from the specified location
// in the open phone device to the specified buffer.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetData (HDRVPHONE hdPhone, DWORD dwDataId,
               LPVOID lpData, DWORD dwSize)
{
	_asm jmp tsplib_phoneGetData;

}// TSPI_phoneGetData

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetDevCaps
//
// This function queries a specified phone device to determine its
// telephony capabilities
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetDevCaps (DWORD dwDeviceId, DWORD dwTSPIVersion,
               DWORD dwExtVersion, LPPHONECAPS lpPhoneCaps)
{
	_asm jmp tsplib_phoneGetDevCaps;

}// TSPI_phoneGetDevCaps

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetDisplay
//
// This function returns the current contents of the specified phone
// display.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetDisplay (HDRVPHONE hdPhone, LPVARSTRING lpString)
{
	_asm jmp tsplib_phoneGetDisplay;

}// TSPI_phoneGetDisplay

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetExtensionID
//
// This function retrieves the extension ID that the service provider
// supports for the indicated device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetExtensionID (DWORD dwDeviceId, DWORD dwTSPIVersion,
               LPPHONEEXTENSIONID lpExtensionId)
{
	_asm jmp tsplib_phoneGetExtensionID;

}// TSPI_phoneGetExtensionID

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetGain
//
// This function returns the gain setting of the microphone of the
// specified phone's hookswitch device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetGain (HDRVPHONE hdPhone, DWORD dwHookSwitchDev,
               LPDWORD lpdwGain)
{
	_asm jmp tsplib_phoneGetGain;

}// TSPI_phoneGetGain

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetHookSwitch
//
// This function retrieves the current hook switch setting of the
// specified open phone device
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetHookSwitch (HDRVPHONE hdPhone, LPDWORD lpdwHookSwitchDevs)
{
	_asm jmp tsplib_phoneGetHookSwitch;

}// TSPI_phoneGetHookSwitch

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetIcon
//
// This function retrieves a specific icon for display from an
// application.  This icon will represent the phone device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetIcon (DWORD dwDeviceId, LPCWSTR lpszDeviceClass, 
               LPHICON lphIcon)
{
	_asm jmp tsplib_phoneGetIcon;

}// TSPI_phoneGetIcon

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetID
//
// This function retrieves the device id of the specified open phone
// handle (or some other media handle if available).
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetID (HDRVPHONE hdPhone, LPVARSTRING lpDeviceId, 
               LPCWSTR lpszDeviceClass, HANDLE hTargetProcess)
{
	_asm jmp tsplib_phoneGetID;

}// TSPI_phoneGetID

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetLamp
//
// This function returns the current lamp mode of the specified
// lamp.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetLamp (HDRVPHONE hdPhone, DWORD dwButtonLampId,
               LPDWORD lpdwLampMode)
{
	_asm jmp tsplib_phoneGetLamp;

}// TSPI_phoneGetLamp

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetRing
//
// This function enables an application to query the specified open
// phone device as to its current ring mode.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetRing (HDRVPHONE hdPhone, LPDWORD lpdwRingMode,
               LPDWORD lpdwVolume)
{
	_asm jmp tsplib_phoneGetRing;

}// TSPI_phoneGetRing

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetStatus
//
// This function queries the specified open phone device for its
// overall status.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetStatus (HDRVPHONE hdPhone, LPPHONESTATUS lpPhoneStatus)
{
	_asm jmp tsplib_phoneGetStatus;

}// TSPI_phoneGetStatus

////////////////////////////////////////////////////////////////////////////
// TSPI_phoneGetVolume
//
// This function returns the volume setting of the phone device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneGetVolume (HDRVPHONE hdPhone, DWORD dwHookSwitchDev,
               LPDWORD lpdwVolume)
{
	_asm jmp tsplib_phoneGetVolume;

}// TSPI_phoneGetVolume

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneNegotiateTSPIVersion
//
// This function returns the highest SP version number the
// service provider is willing to operate under for this device,
// given the range of possible values.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneNegotiateTSPIVersion (DWORD dwDeviceID,
               DWORD dwLowVersion, DWORD dwHighVersion,
               LPDWORD lpdwVersion)
{
	_asm jmp tsplib_phoneNegotiateTSPIVersion;

}// TSPI_phoneNegotiateTSPIVersion   

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneNegotiateExtVersion
//
// This function returns the highest extension version number the
// service provider is willing to operate under for this device,
// given the range of possible extension values.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneNegotiateExtVersion (DWORD dwDeviceID,
               DWORD dwTSPIVersion, DWORD dwLowVersion, DWORD dwHighVersion,
               LPDWORD lpdwExtVersion)
{
	_asm jmp tsplib_phoneNegotiateExtVersion;

}// TSPI_phoneNegotiateExtVersion   

////////////////////////////////////////////////////////////////////////////
// TSPI_phoneOpen
//
// This function opens the phone device whose device ID is given,
// returning the service provider's opaque handle for the device and
// retaining the TAPI opaque handle.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneOpen (DWORD dwDeviceId, HTAPIPHONE htPhone,
               LPHDRVPHONE lphdPhone, DWORD dwTSPIVersion, PHONEEVENT lpfnEventProc)
{
	_asm jmp tsplib_phoneOpen;

}// TSPI_phoneOpen

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneSelectExtVersion
//
// This function selects the indicated extension version for the
// indicated phone device.  Subsequent requests operate according to
// that extension version.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSelectExtVersion (HDRVPHONE hdPhone, DWORD dwExtVersion)
{
	_asm jmp tsplib_phoneSelectExtVersion;

}// TSPI_phoneSelectExtVersion

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetButtonInfo
//
// This function sets information about the specified button on the
// phone device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetButtonInfo (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, DWORD dwButtonId,
               LPPHONEBUTTONINFO const lpButtonInfo)
{
	_asm jmp tsplib_phoneSetButtonInfo;

}// TSPI_phoneSetButtonInfo

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetData
//
// This function downloads the information in the specified buffer
// to the opened phone device at the selected data id.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetData (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwDataId, LPVOID const lpData, DWORD dwSize)
{
	_asm jmp tsplib_phoneSetData;

}// TSPI_phoneSetData

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetDisplay
//
// This function causes the specified string to be displayed on the
// phone device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetDisplay (DRV_REQUESTID dwRequestID, 
         HDRVPHONE hdPhone, DWORD dwRow, DWORD dwCol, LPCWSTR lpszDisplay,
         DWORD dwSize)   
{
	_asm jmp tsplib_phoneSetDisplay;

}// TSPI_phoneSetDisplay

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetGain
//
// This function sets the gain of the microphone of the specified hook
// switch device.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetGain (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
								DWORD dwHookSwitchDev, DWORD dwGain)
{
	_asm jmp tsplib_phoneSetGain;

}// TSPI_phoneSetGain

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetHookSwitch
//
// This function sets the hook state of the specified open phone's
// hookswitch device to the specified mode.  Only the hookswitch
// state of the hookswitch devices listed is affected.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetHookSwitch (DRV_REQUESTID dwRequestId, 
      HDRVPHONE hdPhone, DWORD dwHookSwitchDevs, DWORD dwHookSwitchMode)
{
	_asm jmp tsplib_phoneSetHookSwitch;

}// TSPI_phoneSetHookSwitch

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetLamp
//
// This function causes the specified lamp to be set on the phone
// device to the specified mode.
//  
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetLamp (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwButtonLampId, DWORD dwLampMode)
{
	_asm jmp tsplib_phoneSetLamp;

}// TSPI_phoneSetLamp

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetRing
//
// This function rings the specified open phone device using the
// specified ring mode and volume.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetRing (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwRingMode, DWORD dwVolume)
{
	_asm jmp tsplib_phoneSetRing;

}// TSPI_phoneSetRing

//////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetStatusMessages
//
// This function causes the service provider to filter status messages
// which are not currently of interest to any application.
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetStatusMessages (HDRVPHONE hdPhone, DWORD dwPhoneStates,
            DWORD dwButtonModes, DWORD dwButtonStates)
{
	_asm jmp tsplib_phoneSetStatusMessages;

}// TSPI_phoneSetStatusMessages

/////////////////////////////////////////////////////////////////////////
// TSPI_phoneSetVolume
//
// This function either sets the volume of the speaker or the 
// specified hookswitch device on the phone
//
DLLEXPORT
LONG TSPIAPI TSPI_phoneSetVolume (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwHookSwitchDev, DWORD dwVolume)
{
	_asm jmp tsplib_phoneSetVolume;

}// TSPI_phoneSetVolume

/******************************************************************************/
//
// TSPI_provider functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// TSPI_providerInit
//
// This function is called when TAPI.DLL wants to initialize
// our service provider.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerInit(DWORD dwTSPIVersion,
         DWORD dwPermanentProviderID, DWORD dwLineDeviceIDBase,
         DWORD dwPhoneDeviceIDBase, DWORD_PTR dwNumLines, DWORD_PTR dwNumPhones,
         ASYNC_COMPLETION lpfnCompletionProc, LPDWORD lpdwTSPIOptions)
{
	_asm jmp tsplib_providerInit;

}// TSPI_providerInit

///////////////////////////////////////////////////////////////////////////
// TSPI_providerShutdown
//
// This function is called when the TAPI.DLL is shutting down our
// service provider.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerShutdown (DWORD dwTSPIVersion, DWORD dwProviderID)
{
	_asm jmp tsplib_providerShutdown;

}// TSPI_providerShutdown

////////////////////////////////////////////////////////////////////////////
// TSPI_providerEnumDevices (Win95)
//
// This function is called before the TSPI_providerInit to determine
// the number of line and phone devices supported by the service provider.
// If the function is not available, then TAPI will read the information
// out of the TELEPHON.INI file per TAPI 1.0.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerEnumDevices (DWORD dwPermanentProviderID, LPDWORD lpdwNumLines,
         LPDWORD lpdwNumPhones, HPROVIDER hProvider, LINEEVENT lpfnLineCreateProc,
         PHONEEVENT lpfnPhoneCreateProc)
{
	_asm jmp tsplib_providerEnumDevices;

}// TSPI_providerEnumDevices

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerCreateLineDevice  (Win95)
//
// This function is called by TAPI in response to the receipt of a 
// LINE_CREATE message from the service provider which allows the dynamic
// creation of a new line device.  The passed deviceId identifies this
// line from TAPIs perspective.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerCreateLineDevice (DWORD_PTR dwTempID, DWORD dwDeviceID)
{
	_asm jmp tsplib_providerCreateLineDevice;

}// TSPI_providerCreateLineDevice

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerCreatePhoneDevice (Win95)
//
// This function is called by TAPI in response to the receipt of a
// PHONE_CREATE message from the service provider which allows the dynamic
// creation of a new phone device.  The passed deviceId identifies this
// phone from TAPIs perspective.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerCreatePhoneDevice (DWORD dwTempID, DWORD dwDeviceID)
{
	_asm jmp tsplib_providerCreatePhoneDevice;

}// TSPI_providerCreatePhoneDevice

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerFreeDialogInstance (Tapi 2.0)
//
// This function is called to inform the service provider that
// the dialog associated with the "hdDlgInstance" has exited.
// After this function is called, the service provider should no
// longer send data to the dialog using the LINE_SENDDIALOGINSTANCEDATA
// message.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerFreeDialogInstance (HDRVDIALOGINSTANCE hdDlgInstance)
{
	_asm jmp tsplib_providerFreeDialogInstance;

}// TSPI_providerFreeDialogInstance

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerGenericDialogData (Tapi 2.0)
//
// This function delivers to the service provider data that was
// sent from the UI DLL running in an application context via
// the TSISPIDLLCALLBACK function.  The contents of the memory
// block pointed to be lpParams is defined by the service provider
// and UI DLL.  The service provider can modify the contents of the
// memory block; when this function returns, TAPI will copy the
// new modified data back to the original UI DLL parameter block.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerGenericDialogData(DWORD_PTR dwObjectID, DWORD dwObjectType, LPVOID lpParams, DWORD dwSize)
{
	_asm jmp tsplib_providerGenericDialogData;

}// TSPI_providerGenericDialogData

/////////////////////////////////////////////////////////////////////////////
// TSPI_providerUIIdentify (Tapi 2.0)
//
// This function returns the name of the UI DLL for this 
// service provider.
//
DLLEXPORT
LONG TSPIAPI TSPI_providerUIIdentify (LPWSTR lpszUIDLLName)
{
	_asm jmp tsplib_providerUIIdentify;

}// TSPI_providerUIIdentify

/******************************************************************************/
//
// AGENT SUPPORT FUNCTIONS called by DEVICE.CPP when proxy requests are
// received
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentGroup
//
// Called by the agent proxy to set the agent group for a particular line
// and address.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentGroup(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST const lpGroupList)
{
	_asm jmp tsplib_lineSetAgentGroup;

}// TSPI_lineSetAgentGroup

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentState
//
// Called by the agent proxy to set the current agent state
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentState, DWORD dwNextAgentState)
{
	_asm jmp tsplib_lineSetAgentState;

}// TSPI_lineSetAgentState

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentActivity
//
// Called by the agent proxy to set the current agent activity
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentActivity(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwActivityID)
{
	_asm jmp tsplib_lineSetAgentActivity;

}// TSPI_lineSetAgentActivity

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentStatus
//
// Called by the agent proxy to query the current agent state
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentStatus(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTSTATUS lpStatus)
{
	_asm jmp tsplib_lineGetAgentStatus;

}// TSPI_lineGetAgentStatus

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentCaps
//
// Called by the agent proxy to query the capabilities of the address
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentCaps(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTCAPS lpCapabilities)
{
	_asm jmp tsplib_lineGetAgentCaps;

}// TSPI_lineGetAgentCaps

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentActivityList
//
// Called by the agent proxy to get the list of available agent activities
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentActivityList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTACTIVITYLIST lpList)
{
	_asm jmp tsplib_lineGetAgentActivityList;

}// TSPI_lineGetAgentActivityList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentGroupList
//
// Called by the agent proxy to query the available groups
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentGroupList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST lpList)
{
	_asm jmp tsplib_lineGetAgentGroupList;

}// TSPI_lineGetAgentGroupList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineAgentSpecific
//
// Called by the agent proxy to manage device-specific agent requests
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineAgentSpecific(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentExtensionID, LPVOID lpvParams, DWORD dwSize)
{
	_asm jmp tsplib_lineAgentSpecific;

}// TSPI_lineGetAgentGroupList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineCreateAgent
//
// This function creates a new agent. 
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineCreateAgent(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENT lphAgent, LPCWSTR pszMachineName, LPCWSTR pszUserName, LPCWSTR pszAgentID, LPCWSTR pszAgentPIN)
{
	_asm jmp tsplib_lineCreateAgent;

}// TSPI_lineCreateAgent

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentMeasurementPeriod
//
// This function sets the measurement period associated with a particular agent.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwMeasurementPeriod)
{
	_asm jmp tsplib_lineSetAgentMeasurementPeriod;

}// TSPI_lineSetAgentMeasurementPeriod

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentInfo
//
// This function retrieves the agent information for a specific agent.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentInfo(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo)
{
	_asm jmp tsplib_lineGetAgentInfo;

}// TSPI_lineGetAgentInfo

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineCreateAgentSession
//
// This function creates a new agent session.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineCreateAgentSession(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCWSTR pszAgentPIN, const GUID* pGUID, DWORD dwWorkingAddressID)
{
	_asm jmp tsplib_lineCreateAgentSession;

}// TSPI_lineCreateAgentSession

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentSessionList
//
// This function retrieves the current agent session list.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentSessionList(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList)
{
	_asm jmp tsplib_lineGetAgentSessionList;

}// TSPI_lineGetAgentSessionList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentSessionState
//
// This function sets the agent session state for a specific session.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentSessionState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState)
{
	_asm jmp tsplib_lineSetAgentSessionState;

}// TSPI_lineSetAgentSessionState

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetAgentSessionInfo
//
// This function retrieves the session information for a specific agent session
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetAgentSessionInfo(DWORD dwDeviceID, HAGENTSESSION hAgentSession, LPLINEAGENTSESSIONINFO lpSessionInfo)
{
	_asm jmp tsplib_lineGetAgentSessionInfo;

}// TSPI_lineGetAgentSessionInfo

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetQueueList
//
// This function retrieves the agent queue list
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetQueueList(DWORD dwDeviceID, const GUID* pGroupID, LPLINEQUEUELIST lpQueueList)
{
	_asm jmp tsplib_lineGetQueueList;

}// TSPI_lineGetQueueList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetQueueMeasurementPeriod
//
// This function sets the measurement period for an agent queue
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwQueueID, DWORD dwMeasurementPeriod)
{
	_asm jmp tsplib_lineSetQueueMeasurementPeriod;

}// TSPI_lineSetQueueMeasurementPeriod

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetQueueInfo
//
// This function retrieves the queue information for a specific queue.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetQueueInfo(DWORD dwDeviceID, DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo)
{
	_asm jmp tsplib_lineGetQueueInfo;

}// TSPI_lineGetQueueInfo

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineGetGroupList
//
// This function retrieves the agent group list
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineGetGroupList(DWORD dwDeviceID, LPLINEAGENTGROUPLIST lpGroupList)
{
	_asm jmp tsplib_lineGetGroupList;

}// TSPI_lineGetGroupList

/////////////////////////////////////////////////////////////////////////////
// TSPI_lineSetAgentStateEx
//
// This function changes an agents state.
//
DLLEXPORT _declspec(dllexport) LONG TSPIAPI 
TSPI_lineSetAgentStateEx(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwState, DWORD dwNextState)
{
	_asm jmp tsplib_lineSetAgentStateEx;

}// TSPI_lineSetAgentStateEx

/******************************************************************************/
//
// OBSOLETE FUNCTIONS - Required only for export
//
/******************************************************************************/

////////////////////////////////////////////////////////////////////////////
// TSPI_lineConfigDialog
//
// This function is called to display the line configuration dialog
// when the user requests it through either the TAPI api or the control
// panel applet.
//
extern "C"
LONG TSPIAPI TSPI_lineConfigDialog (DWORD /*dwDeviceID*/, HWND /*hwndOwner*/, LPCSTR /*lpszDeviceClass*/)
{
	return LINEERR_OPERATIONUNAVAIL;

}// TSPI_lineConfigDialog

///////////////////////////////////////////////////////////////////////////
// TSPI_lineConfigDialogEdit (Win95)
//
// This function causes the provider of the specified line device to
// display a modal dialog to allow the user to configure parameters
// related to the line device.  The parameters editted are NOT the
// current device parameters, rather the set retrieved from the
// 'TSPI_lineGetDevConfig' function (lpDeviceConfigIn), and are returned
// in the lpDeviceConfigOut parameter.
//
extern "C"
LONG TSPIAPI TSPI_lineConfigDialogEdit (DWORD /*dwDeviceID*/, HWND /*hwndOwner*/,
		LPCSTR /*lpszDeviceClass*/, LPVOID const /*lpDeviceConfigIn*/, DWORD /*dwSize*/,
		LPVARSTRING /*lpDeviceConfigOut*/)
{
	return LINEERR_OPERATIONUNAVAIL;

}// TSPI_lineConfigDialogEdit

///////////////////////////////////////////////////////////////////////////
// TSPI_phoneConfigDialog
//
// This function invokes the parameter configuration dialog for the
// phone device.
//
extern "C"
LONG TSPIAPI TSPI_phoneConfigDialog (DWORD /*dwDeviceId*/, HWND /*hwndOwner*/, 
									 LPCSTR /*lpszDeviceClass*/)
{
	return PHONEERR_OPERATIONUNAVAIL;

}// TSPI_phoneConfigDialog

////////////////////////////////////////////////////////////////////////////
// TSPI_providerConfig
//
// This function is invoked from the control panel and allows the user
// to configure our service provider.
//        
extern "C"
LONG TSPIAPI TSPI_providerConfig (HWND, DWORD)
{
	return LINEERR_OPERATIONUNAVAIL;
   
}// TSPI_providerConfig

////////////////////////////////////////////////////////////////////////////
// TSPI_providerInstall
//
// This function is invoked to install the service provider onto
// the system.
//        
extern "C"
LONG TSPIAPI TSPI_providerInstall (HWND, DWORD)
{
	return LINEERR_OPERATIONUNAVAIL;

}// TSPI_providerInstall

////////////////////////////////////////////////////////////////////////////
// TSPI_providerRemove
//
// This function removes the service provider
//
extern "C"
LONG TSPIAPI TSPI_providerRemove (HWND, DWORD)
{
	return LINEERR_OPERATIONUNAVAIL;

}// TSPI_providerRemove
