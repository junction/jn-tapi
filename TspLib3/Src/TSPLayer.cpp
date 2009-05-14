/******************************************************************************/
//                                                                        
// TSPLAYER.CPP - tsplib_xxx function layer
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
#include "debug.h"
#include <ctype.h>

/******************************************************************************/
//
// PUBLIC UTILITY FUNCTIONS
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// IsDeletedObject
//
// Simple utility function to determine if the passed object is deleted.
//
inline bool IsDeletedObject(CTSPILineConnection* pObject)
{
	// Check to see if we can access the memory at this location.
	return (pObject == NULL || IsBadWritePtr(pObject, sizeof(CTSPILineConnection)) || 
		    !pObject->IsLineDevice() || pObject->HasBeenDeleted());

}// IsDeletedObject

///////////////////////////////////////////////////////////////////////////
// IsDeletedObject
//
// Simple utility function to determine if the passed object is deleted.
//
inline bool IsDeletedObject(CTSPIPhoneConnection* pObject)
{
	return (pObject == NULL || IsBadWritePtr(pObject, sizeof(CTSPIPhoneConnection)) || 
		    !pObject->IsPhoneDevice() || pObject->HasBeenDeleted());

}// IsDeletedObject

///////////////////////////////////////////////////////////////////////////
// IsDeletedObject
//
// Simple utility function to determine if the passed object is deleted.
//
inline bool IsDeletedObject(CTSPICallAppearance* pObject)
{
	return (pObject == NULL || IsBadWritePtr(pObject, sizeof(CTSPICallAppearance)) || 
		    pObject->HasBeenDeleted());

}// IsDeletedObject

///////////////////////////////////////////////////////////////////////////
// IsDeletedObject
//
// Simple utility function to determine if the passed object is deleted.
//
inline bool IsDeletedObject(CTSPIAddressInfo* pObject)
{
	// Check to see if we can access the memory at this location.
	return (pObject == NULL || IsBadWritePtr(pObject, sizeof(CTSPIAddressInfo)));

}// IsDeletedObject

///////////////////////////////////////////////////////////////////////////
// IsDeletedObject
//
// Simple utility function to determine if the passed object is deleted.
//
inline bool IsDeletedObject(CMSPDriver* pObject)
{
	// Check to see if we can access the memory at this location.
	return (pObject == NULL || IsBadWritePtr(pObject, sizeof(CMSPDriver)));

}// IsDeletedObject

///////////////////////////////////////////////////////////////////////////
// CopyVarString
//
// Copy a string (UNICODE or not) into a variable string buffer
//
void CopyVarString (LPVARSTRING lpVarString, LPCTSTR lpszBuff)
{
	if (lpVarString->dwStringFormat != STRINGFORMAT_UNICODE &&
		lpVarString->dwStringFormat != STRINGFORMAT_ASCII)
	{
#ifdef _UNICODE
		lpVarString->dwStringFormat = STRINGFORMAT_UNICODE;
#else
		lpVarString->dwStringFormat = STRINGFORMAT_ASCII;
#endif
	}

	lpVarString->dwNeededSize = sizeof(VARSTRING);
	lpVarString->dwUsedSize = sizeof(VARSTRING);

	USES_CONVERSION;
	BOOL fSuccess = FALSE;
	if (lpVarString->dwStringFormat == STRINGFORMAT_UNICODE)
	{
		fSuccess = AddDataBlock(lpVarString, lpVarString->dwStringOffset, lpVarString->dwStringSize, T2W(const_cast<TCHAR*>(lpszBuff)));
	}
	else // dwStringFormat = STRINGFORMAT_ASCII
	{
		fSuccess = AddDataBlock(lpVarString, lpVarString->dwStringOffset, lpVarString->dwStringSize, T2A(const_cast<TCHAR*>(lpszBuff)));
	}

	if (fSuccess)
		lpVarString->dwNeededSize += lpVarString->dwStringSize;
	else
	{
#ifdef _UNICODE
		lpVarString->dwNeededSize += (lstrlenW(lpszBuff)+1) * sizeof(wchar_t);
#else
		lpVarString->dwNeededSize += MultiByteToWideChar(CP_ACP, 0, lpszBuff, -1, NULL, 0) * sizeof(WCHAR);
#endif
	}

}// CopyVarString

///////////////////////////////////////////////////////////////////////////
// CopyVarString
//
// Copy a non-typed buffer into a variable string buffer
//
void CopyVarString (LPVARSTRING lpVarString, LPVOID lpBuff, DWORD dwSize)
{                
	lpVarString->dwNeededSize = sizeof(VARSTRING) + dwSize;
	lpVarString->dwStringFormat = STRINGFORMAT_BINARY;
	if (lpVarString->dwTotalSize >= lpVarString->dwNeededSize) {
		lpVarString->dwStringSize = dwSize;
		lpVarString->dwStringOffset = sizeof(VARSTRING);
		MoveMemory(reinterpret_cast<LPBYTE>(lpVarString)+sizeof(VARSTRING), lpBuff, dwSize);
	}   		

}// CopyVarString

///////////////////////////////////////////////////////////////////////////
// AddDataBlock
//
// Public function to add a string to a VARSTRING type buffer.
//
bool AddDataBlock(LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize, LPCSTR lpszBuff)
{
	bool bResult = false;

	// Convert the ANSI to UNICODE
	USES_CONVERSION;
	LPWSTR lpszUniBuff = A2W(const_cast<LPSTR>(lpszBuff));
	int iSize = lstrlenW(lpszUniBuff);
	if (iSize > 0) 
	{
		++iSize;	// Add NULL character at end of buffer
		bResult = AddDataBlock(lpVB, dwOffset, dwSize, lpszUniBuff, iSize*sizeof(wchar_t));
	}
	return bResult;

}// AddDataBlock

///////////////////////////////////////////////////////////////////////////
// AddDataBlock
//
// Public function to adjust a variable length structure field for 
// the library.  This may be called multiple times to add the same 
// information into a structure.  The offset will only be adjusted if
// the information in the record is zero.
//
bool AddDataBlock (LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize, LPCVOID lpBuffer, DWORD dwBufferSize)
{
	// If the buffer size is zero, ignore this.
	if (dwBufferSize == 0)
		return false;
	
	// Cast our block to something which has the first three fields 
	// in the same order as all our variable structures defined by TAPI.
	LPVARSTRING lpVarBuffer = reinterpret_cast<LPVARSTRING>(lpVB);
	DWORD dwUsedSize = lpVarBuffer->dwUsedSize;
	lpVarBuffer->dwNeededSize += dwBufferSize;

#ifndef _X86_
	// Make sure the used size field is DWORD-aligned.  This is required
	// for most non-Intel machines. Note this is ALWAYS done regardless of the neededupdate flag
	// since the caller isn't aware of this detail.
	while ((dwUsedSize % 4) != 0) {
		dwUsedSize++;
		lpVarBuffer->dwNeededSize++;
	}
#endif

	// If we don't have enough room for the buffer here then exit.
	if (lpVarBuffer->dwTotalSize < dwUsedSize+dwBufferSize)
		return false;

	// We have enough room, save the new used size.
	lpVarBuffer->dwUsedSize = dwUsedSize;

	// Fill in the buffer.
	MoveMemory(reinterpret_cast<LPBYTE>(lpVarBuffer)+lpVarBuffer->dwUsedSize, lpBuffer, dwBufferSize);

	// Adjust the offset and size.  We only adjust the offset if it is zero
	// since we may be APPENDING data to an existing block.
	if (dwOffset == 0)
		dwOffset = lpVarBuffer->dwUsedSize;
	dwSize += dwBufferSize;
	lpVarBuffer->dwUsedSize += dwBufferSize;

	// Added the block successfully.
	return true;
	
}// AddDataBlock

///////////////////////////////////////////////////////////////////////////////
// CopyCallParams
//
// This function copies the LINECALLPARAMS buffer and performs any required
// conversion on the buffers.
//
LINECALLPARAMS* CopyCallParams(LPLINECALLPARAMS const lpcpIn)
{
	if (lpcpIn == NULL) 
		return NULL;

	// Allocate the output structure and copy all the data over
	LINECALLPARAMS* lpcpOut = reinterpret_cast<LINECALLPARAMS*>(AllocMem(lpcpIn->dwTotalSize));
	CopyMemory(lpcpOut, lpcpIn, lpcpIn->dwTotalSize);

#ifndef _UNICODE
	// Reset sizes to zero -- some apps, (notably TB20.EXE) pass in a 2-character 
	// non-NULL terminated string in some instances. The value is not valid and should
	// be ignored. These fields are all documented to be NULL terminated.
	lpcpOut->dwOrigAddressSize = 0;
	lpcpOut->dwDisplayableAddressSize = 0;
	lpcpOut->dwCalledPartySize = 0;
	lpcpOut->dwCommentSize = 0;
	lpcpOut->dwTargetAddressSize = 0;
	lpcpOut->dwDeviceClassSize = 0;
	lpcpOut->dwCallingPartyIDSize = 0;

	// For ANSI builds, convert all the strings	from Unicode to ANSI.
	// We assume here that Ansi strings will always be smaller than the Unicode
	// equivalants.
	if (lpcpIn->dwOrigAddressSize > 2) {
		lpcpOut->dwOrigAddressSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwOrigAddressOffset),
			lpcpIn->dwOrigAddressSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwOrigAddressOffset),
			lpcpIn->dwOrigAddressSize, NULL, NULL);
	}

	if (lpcpIn->dwDisplayableAddressSize > 2) {
		lpcpOut->dwDisplayableAddressSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwDisplayableAddressOffset),
			lpcpIn->dwDisplayableAddressSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwDisplayableAddressOffset),
			lpcpIn->dwDisplayableAddressSize, NULL, NULL);
	}
	
	if (lpcpIn->dwCalledPartySize > 2) {
		lpcpOut->dwCalledPartySize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwCalledPartyOffset),
			lpcpIn->dwCalledPartySize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwCalledPartyOffset),
			lpcpIn->dwCalledPartySize, NULL, NULL);
	}
	
	if (lpcpIn->dwCommentSize > 2) {
		lpcpOut->dwCommentSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwCommentOffset),
			lpcpIn->dwCommentSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwCommentOffset),
			lpcpIn->dwCommentSize, NULL, NULL);
	}
	
	if (lpcpIn->dwTargetAddressSize > 2) {
		lpcpOut->dwTargetAddressSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwTargetAddressOffset),
			lpcpIn->dwTargetAddressSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwTargetAddressOffset),
			lpcpIn->dwTargetAddressSize, NULL, NULL);
	}

	if (lpcpIn->dwDeviceClassSize > 2) {
		lpcpOut->dwDeviceClassSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwDeviceClassOffset),
			lpcpIn->dwDeviceClassSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwDeviceClassOffset),
			lpcpIn->dwDeviceClassSize, NULL, NULL);
	}
	
	if (lpcpIn->dwCallingPartyIDSize > 2) {
		lpcpOut->dwCallingPartyIDSize = WideCharToMultiByte(CP_ACP, 0, 
			reinterpret_cast<LPCWSTR>((LPBYTE)lpcpIn+lpcpIn->dwCallingPartyIDOffset),
			lpcpIn->dwCallingPartyIDSize/sizeof(wchar_t), 
			reinterpret_cast<LPSTR>((LPBYTE)lpcpOut+lpcpIn->dwCallingPartyIDOffset),
			lpcpIn->dwCallingPartyIDSize, NULL, NULL);
	}
#endif

	return lpcpOut;

}// CopyCallParams

///////////////////////////////////////////////////////////////////////////
// SetTraceLevel
//
// Sets the current tracing level
//
extern "C" _declspec(dllexport) void __stdcall SetTraceLevel(int iLevel)
{
	CDebugMgr::Instance().SetTraceLevel(iLevel);

}// SetTraceLevel

///////////////////////////////////////////////////////////////////////////
// GetTraceLevel
//
// Returns the current trace level from the registry.
//
extern "C" _declspec(dllexport) int __stdcall GetTraceLevel()
{
	return CDebugMgr::Instance().GetTraceLevel();

}// GetTraceLevel

///////////////////////////////////////////////////////////////////////////
// TraceOutW
//
// Exported trace function for debug hook dll
//
extern "C" _declspec(dllexport) void __stdcall TraceOutW(LPCWSTR pszBuffer)
{
	USES_CONVERSION;
	CDebugMgr::Instance().TraceOut(W2T(const_cast<LPWSTR>(pszBuffer)));

}// TraceOutW

///////////////////////////////////////////////////////////////////////////
// tsplib_TspTrace
//
// Internal trace function which is used to output information from
// the class library.
//
void _cdecl tsplib_TspTrace(int iTraceLevel, UINT uidFormat, ...)
{
	va_list args;
	va_start(args, uidFormat);
	CDebugMgr::Instance().BuildTraceBuffer(iTraceLevel, uidFormat, args);
	va_end(args);

}// tsplib_TspTrace

///////////////////////////////////////////////////////////////////////////
// tsplib_TspTrace2
//
// Internal trace function which is used to output information from
// the class library.
//
void _cdecl tsplib_TspTrace2(UINT uidFormat, ...)
{
	va_list args;
	va_start(args, uidFormat);
	CDebugMgr::Instance().BuildTraceBuffer(TRC_MIN, uidFormat, args);
	va_end(args);

}// tsplib_TspTrace2

///////////////////////////////////////////////////////////////////////////
// tsplib_TspTrace
//
// Internal trace function which is used to output information from
// the class library.
//
void _cdecl tsplib_TspTrace(int iTraceLevel, LPCTSTR pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);
	CDebugMgr::Instance().BuildTraceBuffer(iTraceLevel, pszFormat, args);
	va_end(args);

}// tsplib_TspTrace

///////////////////////////////////////////////////////////////////////////
// tsplib_TspTrace2
//
// Internal trace function which is used to output information from
// the class library.
//
void _cdecl tsplib_TspTrace2(LPCTSTR pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);
	CDebugMgr::Instance().BuildTraceBuffer(TRC_MIN, pszFormat, args);
	va_end(args);

}// tsplib_TspTrace2

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// tsplib_AssertFailedLine
//
// Replacement for the ASSERT functionality of MFC.  Under NT,
// the TAPI context is actually a separate subsystem run in its own
// screen group, and therefore cannot do any kind of Win32 UI.
//
void tsplib_AssertFailedLine(LPCSTR lpszFileName, int nLine, LPCSTR lpszExpression)
{
	// Save off the last OS error
	DWORD dwError = GetLastError();

	// Get the module filename.
	char chModname[_MAX_PATH];
	if (!GetModuleFileNameA(GetSP()->GetResourceInstance(), chModname, sizeof(chModname)))
		lstrcpyA(chModname, "TSP++");

	// Dump it to the trace log.
	USES_CONVERSION;
	_TSP_DTRACE(_T("Assert(%s) failed, %s at %d\r\n"), (lpszExpression) ? 
		A2T(const_cast<LPSTR>(lpszExpression)) : _T("FALSE"), 
		A2T(const_cast<LPSTR>(lpszFileName)), nLine);

	// Now show a message box to the user so they _know_ something bad 
	// happened .. just in case they don't notice the trace.
	tsplib::inauto_ptr<char> pszDiagBuff(new char[4096]);

	// See if the error code can be translated by the system.
    LPSTR pszFmtMsg = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, 
                   reinterpret_cast<LPSTR>(&pszFmtMsg), 0, NULL);
    LPSTR pszLastErr = (pszFmtMsg) ? pszFmtMsg : "Unknown";

    // Build the message.
    wsprintfA(pszDiagBuff.get(),
              "Debug Assertion Failed!\n\n"
              "Program : %s\n"
			  "File : %s\n"
			  "Line : %d\n"
              "Expression : %s\n"
              "Last Error (0x%08X) : %s\n",
              chModname, lpszFileName, nLine, lpszExpression, dwError, pszLastErr);

    // Get rid of the allocated memory from FormatMessage.
    if (pszFmtMsg)
        LocalFree(reinterpret_cast<LPVOID>(pszFmtMsg));
	
	// Show the message box
	switch(MessageBoxA(NULL, pszDiagBuff.get(), "ASSERTION FAILURE", 
		MB_SERVICE_NOTIFICATION | MB_SETFOREGROUND | MB_ABORTRETRYIGNORE | MB_ICONSTOP))
	{
		case IDABORT: ExitProcess(1); break;	// Stop process
		case IDRETRY: DebugBreak(); break;		// Break into debugger
		case IDIGNORE: 
		default:	  break;					// Ignore
	}

	// You should stop here if retry is pressed.
	return;

}// tsplib_AssertFailedLine
#endif

/******************************************************************************/
//
// tsplib_line functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// tsplib_lineAccept
//
// This function accepts the specified offering call.  It may optionally
// send the specified User->User information to the calling party.
//
LONG TSPIAPI tsplib_lineAccept (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->Accept (dwRequestId, lpsUserUserInfo, dwSize) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineAccept

///////////////////////////////////////////////////////////////////////////
// tsplib_lineAddToConference
//
// This function adds the specified call (hdConsultCall) to the
// conference (hdConfCall).
//
LONG TSPIAPI tsplib_lineAddToConference (DRV_REQUESTID dwRequestId, HDRVCALL hdConfCall, HDRVCALL hdConsultCall)
{
	CTSPIConferenceCall* pConfCall	  = (CTSPIConferenceCall*) hdConfCall;
	CTSPICallAppearance* pConsultCall = (CTSPICallAppearance*) hdConsultCall;

	return (!IsDeletedObject(pConsultCall) && !IsDeletedObject(pConfCall)) ?
		pConfCall->AddToConference (dwRequestId, pConsultCall) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineAddToConference

///////////////////////////////////////////////////////////////////////////
// tsplib_lineAnswer
//
// This function answers the specified offering call.
//
LONG TSPIAPI tsplib_lineAnswer (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->Answer(dwRequestId, lpsUserUserInfo, dwSize) : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineAnswer

///////////////////////////////////////////////////////////////////////////
// tsplib_lineBlindTransfer
//
// This function performs a blind or single-step transfer of the
// specified call to the specified destination address.
//
LONG TSPIAPI tsplib_lineBlindTransfer (DRV_REQUESTID dwRequestId, HDRVCALL hdCall, LPCWSTR lpszDestAddr, DWORD dwCountryCode)
{
	TString strDestAddr(ConvertWideToAnsi (lpszDestAddr));
	LONG lResult = LINEERR_INVALCALLHANDLE;
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	if (!IsDeletedObject(pCall))
	{
		TDialStringArray arrDial;
		lResult = GetSP()->DialInputToArray(pCall->GetLineOwner(), pCall->GetAddressOwner(), 
						strDestAddr.c_str(), dwCountryCode, &arrDial);
		if (lResult == 0)
			lResult = pCall->BlindTransfer (dwRequestId, &arrDial, dwCountryCode);
	}
	return lResult;
   
}// tsplib_lineBlindTransfer

////////////////////////////////////////////////////////////////////////////
// tsplib_lineClose
//
// This function closes the specified open line after stopping all
// asynchronous requests on the line.
//
LONG TSPIAPI tsplib_lineClose (HDRVLINE hdLine)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->Close() : 
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineClose

////////////////////////////////////////////////////////////////////////////
// tsplib_lineCloseCall
//
// This function closes the specified call.  The HDRVCALL handle will
// no longer be valid after this call.
//
LONG TSPIAPI tsplib_lineCloseCall (HDRVCALL hdCall)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->Close() : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineCloseCall

///////////////////////////////////////////////////////////////////////////
// tsplib_lineCompleteCall
//
// This function is used to specify how a call that could not be
// connected normally should be completed instead.  The network or
// switch may not be able to complete a call because the network
// resources are busy, or the remote station is busy or doesn't answer.
//
LONG TSPIAPI tsplib_lineCompleteCall (DRV_REQUESTID dwRequestId,
         HDRVCALL hdCall, LPDWORD lpdwCompletionID, DWORD dwCompletionMode,
         DWORD dwMessageID)
{
	// Fill in with zero in case of error.
	*lpdwCompletionID = 0;

	// Route to the call.
    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    return (!IsDeletedObject(pCall)) ?
		pCall->CompleteCall(dwRequestId, lpdwCompletionID, dwCompletionMode, dwMessageID) : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineCompleteCall

///////////////////////////////////////////////////////////////////////////
// tsplib_lineCompleteTransfer
//
// This function completes the transfer of the specified call to the
// party connected in the consultation call.  If 'dwTransferMode' is
// LINETRANSFERMODE_CONFERENCE, the original call handle is changed
// to a conference call.  Otherwise, the service provider should send
// callstate messages change all the calls to idle.
//
LONG TSPIAPI tsplib_lineCompleteTransfer (DRV_REQUESTID dwRequestId,
         HDRVCALL hdCall, HDRVCALL hdConsultCall, HTAPICALL htConfCall,
         LPHDRVCALL lphdConfCall, DWORD dwTransferMode)
{
    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    CTSPICallAppearance* pConsult = (CTSPICallAppearance*) hdConsultCall;

    return (!IsDeletedObject(pCall) && !IsDeletedObject(pConsult)) ?
		pCall->GetAddressOwner()->CompleteTransfer(pCall, dwRequestId, pConsult, 
				htConfCall, lphdConfCall, dwTransferMode) : LINEERR_INVALCALLHANDLE;
   
}// tsplib_lineCompleteTransfer   

////////////////////////////////////////////////////////////////////////////
// tsplib_lineConditionalMediaDetection
//
// This function is invoked by TAPI.DLL when the application requests a
// line open using the LINEMAPPER.  This function will check the 
// requested media modes and return an acknowledgement based on whether 
// we can monitor all the requested modes.
//
LONG TSPIAPI tsplib_lineConditionalMediaDetection (HDRVLINE hdLine,
         DWORD dwMediaModes, LPLINECALLPARAMS const lpCallParams)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->ConditionalMediaDetection(dwMediaModes, lpCallParams) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineConditionalMediaDetection

///////////////////////////////////////////////////////////////////////////
// tsplib_lineDevSpecific
//
// This function is used as a general extension mechanims to allow
// service providers to provide access to features not described in
// other operations.
//
LONG TSPIAPI tsplib_lineDevSpecific (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD dwAddressId, HDRVCALL hdCall, LPVOID lpParams, DWORD dwSize)
{
    CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    CTSPIAddressInfo* pAddr = NULL;
    if (!IsDeletedObject(pLine))
        pAddr = pLine->GetAddress(dwAddressId);

    return GetSP()->lineDevSpecific(pLine, pAddr, pCall, dwRequestId, lpParams, dwSize);
   
}// tsplib_lineDevSpecific

///////////////////////////////////////////////////////////////////////////
// tsplib_lineDevSpecificFeature
//
// This function is used as an extension mechanism to enable service
// providers to provide access to features not described in other
// operations.
//
LONG TSPIAPI tsplib_lineDevSpecificFeature (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD dwFeature, LPVOID lpParams, DWORD dwSize)
{
    CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    return (!IsDeletedObject(pLine)) ?
		pLine->DevSpecificFeature (dwFeature, dwRequestId, lpParams, dwSize) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineDevSpecificFeature

///////////////////////////////////////////////////////////////////////////
// tsplib_lineDial
//
// This function dials the specified dialable number on the specified
// call device.
//
LONG TSPIAPI tsplib_lineDial (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCWSTR lpszDestAddress, DWORD dwCountryCode)
{
	LONG lResult = LINEERR_INVALCALLHANDLE;
	TString strDestAddr(ConvertWideToAnsi (lpszDestAddress));

    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    if (!IsDeletedObject(pCall)) {
		TDialStringArray arrDial;
		if (strDestAddr.empty() || 
			(lResult = GetSP()->DialInputToArray(pCall->GetLineOwner(), pCall->GetAddressOwner(), 
				strDestAddr.c_str(), dwCountryCode, &arrDial)) == 0)
			lResult = pCall->Dial (dwRequestID, &arrDial, dwCountryCode);
	}
    return lResult;

}// tsplib_lineDial

////////////////////////////////////////////////////////////////////////////
// tsplib_lineDrop
//
// This function disconnects the specified call.  The call is still
// valid and should be closed by the application following this API.
//
LONG TSPIAPI tsplib_lineDrop (DRV_REQUESTID dwRequestID, HDRVCALL hdCall, 
         LPCSTR lpsUserUserInfo, DWORD dwSize)
{
    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    return (!IsDeletedObject(pCall)) ?
		pCall->Drop (dwRequestID, lpsUserUserInfo, dwSize) : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineDrop

///////////////////////////////////////////////////////////////////////////
// tsplib_lineForward
//
// This function forwards calls destined for the specified address on
// the specified line, according to the specified forwarding instructions.
// When an origination address is forwarded, the incoming calls for that
// address are deflected to the other number by the switch.  This function
// provides a combination of forward and do-not-disturb features.  This
// function can also cancel specific forwarding currently in effect.
//
LONG TSPIAPI tsplib_lineForward (DRV_REQUESTID dwRequestId, HDRVLINE hdLine,
         DWORD bAllAddresses, DWORD dwAddressId, 
         LPLINEFORWARDLIST const lpForwardList, DWORD dwNumRingsAnswer,
         HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
         LPLINECALLPARAMS const lpCallParams)
{
    CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    CTSPIAddressInfo* pAddr = NULL;

    LONG lResult = 0;
    if (!IsDeletedObject(pLine))
    {
        if (bAllAddresses == false)
        {
            pAddr = pLine->GetAddress(dwAddressId);
            if (IsDeletedObject(pAddr))
                lResult = LINEERR_INVALADDRESSID;
        }

        if (lResult == 0)
            lResult = GetSP()->lineForward(pLine, pAddr, dwRequestId, lpForwardList,
                                     dwNumRingsAnswer, htConsultCall, lphdConsultCall,
                                     lpCallParams);
    }
    else
        lResult = LINEERR_INVALLINEHANDLE;

    return lResult;

}// tsplib_lineForward

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGatherDigits
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
LONG TSPIAPI tsplib_lineGatherDigits (HDRVCALL hdCall, DWORD dwEndToEnd,
         DWORD dwDigitModes, LPWSTR lpszDigits, DWORD dwNumDigits,
         LPCWSTR lpszTerminationDigits, DWORD dwFirstDigitTimeout,
         DWORD dwInterDigitTimeout)
{
	TString strTermDigits(ConvertWideToAnsi (lpszTerminationDigits));
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;

	LONG lResult = LINEERR_INVALCALLHANDLE;
	if (!IsDeletedObject(pCall))
	{
		// Validate the numDigits field.
		if (dwNumDigits == 0L && lpszDigits != NULL)
			lResult = LINEERR_INVALPARAM;
		else
		{
			// Allocate our TSPIDIGITGATHER structure
			TSPIDIGITGATHER* lpGather = new TSPIDIGITGATHER;
			if (lpGather == NULL)
				lResult = LINEERR_NOMEM;
			else
			{
				// Fill it in with our parameters.
				lpGather->dwEndToEndID = dwEndToEnd;
				lpGather->dwDigitModes = dwDigitModes;
				lpGather->lpBuffer = lpszDigits;
				lpGather->dwSize = dwNumDigits;
				lpGather->strTerminationDigits = strTermDigits;
				lpGather->dwFirstDigitTimeout = dwFirstDigitTimeout;
				lpGather->dwInterDigitTimeout = dwInterDigitTimeout;    
    
				// And call our call appearance.
				lResult = pCall->GatherDigits (lpGather);
				if (lResult != 0)
					delete lpGather;
			}
		}
	}

	return lResult;

}// tsplib_lineGatherDigits

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGenerateDigits
//
// This function initiates the generation of the specified digits
// using the specified signal mode.
//
LONG TSPIAPI tsplib_lineGenerateDigits (HDRVCALL hdCall, DWORD dwEndToEndID,
         DWORD dwDigitMode, LPCWSTR lpszDigits, DWORD dwDuration)
{
	TString strDigits(ConvertWideToAnsi (lpszDigits));

	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->GenerateDigits(dwEndToEndID, dwDigitMode, strDigits.c_str(), dwDuration) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineGenerateDigits

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGenerateTone
//
// This function generates the specified tone inband over the specified
// call.  Invoking this function with a zero for 'dwToneMode' aborts any
// tone generation currently in progress on the specified call.
// Invoking 'lineGenerateTone' or 'lineGenerateDigit' also aborts the
// current tone generation and initiates the generation of the newly
// specified tone or digits.
//
LONG TSPIAPI tsplib_lineGenerateTone (HDRVCALL hdCall, DWORD dwEndToEnd,
         DWORD dwToneMode, DWORD dwDuration, DWORD dwNumTones,
         LPLINEGENERATETONE const lpTones)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->GenerateTone(dwEndToEnd, dwToneMode, dwDuration, dwNumTones, lpTones) :
		LINEERR_INVALCALLHANDLE;
   
}// tsplib_lineGenerateTone

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAddressCaps
//
// This function queries the specified address on the specified
// line device to determine its telephony capabilities.
//
LONG TSPIAPI tsplib_lineGetAddressCaps (DWORD dwDeviceID, DWORD dwAddressID,
         DWORD dwTSPIVersion, DWORD dwExtVersion, LPLINEADDRESSCAPS lpAddressCaps)
{
    CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
    LONG lResult = LINEERR_BADDEVICEID;
    if (!IsDeletedObject(pLine))
    {
        CTSPIAddressInfo* pAddr = pLine->GetAddress(dwAddressID);
        if (pAddr == NULL)
            lResult = LINEERR_INVALADDRESSID;
        else
		    lResult = pAddr->GatherCapabilities(dwTSPIVersion, dwExtVersion, lpAddressCaps);
    }
    return lResult;

}// tsplib_lineGetAddressCaps

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAddressID
//
// This function returns the specified address associated with the
// specified line in a specified format.
//
LONG TSPIAPI tsplib_lineGetAddressID (HDRVLINE hdLine, LPDWORD lpdwAddressID, 
         DWORD dwAddressMode, LPCWSTR lpszAddress, DWORD dwSize)
{
	TString strAddress(ConvertWideToAnsi (lpszAddress));
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->GetAddressID (lpdwAddressID, dwAddressMode, strAddress.c_str(), dwSize) : 
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineGetAddressID

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAddressStatus
//
// This function queries the specified address for its current status.
//
LONG TSPIAPI tsplib_lineGetAddressStatus (HDRVLINE hdLine, DWORD dwAddressID,
         LPLINEADDRESSSTATUS lpAddressStatus)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    LONG lResult = LINEERR_INVALLINEHANDLE;

    if (!IsDeletedObject(pLine))
    {   
        CTSPIAddressInfo* pAddr = pLine->GetAddress(dwAddressID);
        if (IsDeletedObject(pAddr))
            lResult = LINEERR_INVALADDRESSID;
        else
            lResult = pAddr->GatherStatusInformation(lpAddressStatus);
    }
    return lResult;

}// tsplib_lineGetAddressStatus

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetCallAddressID
//
// This function retrieves the address for the specified call.
//
LONG TSPIAPI tsplib_lineGetCallAddressID (HDRVCALL hdCall, LPDWORD lpdwAddressID)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	if (IsDeletedObject(pCall))
		return LINEERR_INVALCALLHANDLE;

    *lpdwAddressID = pCall->GetAddressOwner()->GetAddressID();
	return 0;

}// tsplib_lineGetCallAddressID

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetCallIDs
//
// This function retrieves call-id information for this call. It is used
// by TAPI as a quick way to retrieve call-id information rather than
// using the full lineGetCallInfo function.
//
// This requires TAPI 2.2 or 3.0 negotiation
//
LONG TSPIAPI tsplib_lineGetCallIDs(HDRVCALL hdCall, LPDWORD lpdwAddressID, LPDWORD lpdwCallID, LPDWORD lpdwRelatedCallID)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->GetCallIDs(lpdwAddressID, lpdwCallID, lpdwRelatedCallID) : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineGetCallIDs

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetCallInfo
//
// This function retrieves the telephony information for the specified
// call handle.
//
LONG TSPIAPI tsplib_lineGetCallInfo (HDRVCALL hdCall, LPLINECALLINFO lpCallInfo)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->GatherCallInformation (lpCallInfo) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineGetCallInfo

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetCallStatus
//
// This function retrieves the status for the specified call.
//
LONG TSPIAPI tsplib_lineGetCallStatus (HDRVCALL hdCall, LPLINECALLSTATUS lpCallStatus)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
			pCall->GatherStatusInformation(lpCallStatus) :
			LINEERR_INVALCALLHANDLE;

}// tsplib_lineGetCallStatus

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetDevCaps
//
// This function retrieves the telephony device capabilties for the
// specified line.  This information is valid for all addresses on 
// the line.
//
LONG TSPIAPI tsplib_lineGetDevCaps (DWORD dwDeviceID, DWORD dwTSPIVersion, 
         DWORD dwExtVersion, LPLINEDEVCAPS lpLineDevCaps)
{
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	return (!IsDeletedObject(pLine)) ? 
		pLine->GatherCapabilities (dwTSPIVersion, dwExtVersion, lpLineDevCaps) : 
		LINEERR_BADDEVICEID;

}// tsplib_lineGetDevCaps

//////////////////////////////////////////////////////////////////////////
// tsplib_lineGetDevConfig
//
// This function returns a data structure object, the contents of which
// are specific to the line (SP) and device class, giving the current
// configuration of a device associated one-to-one with the line device.
//
LONG TSPIAPI tsplib_lineGetDevConfig (DWORD dwDeviceID, LPVARSTRING lpDeviceConfig,
         LPCWSTR lpszDeviceClass)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());
	
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	return (!IsDeletedObject(pLine)) ? 
		pLine->GetDevConfig (strDevClass, lpDeviceConfig) :
		LINEERR_BADDEVICEID;

}// tsplib_lineGetDevConfig

//////////////////////////////////////////////////////////////////////////
// tsplib_lineGetExtensionID
//
// This function returns the extension ID that the service provider
// supports for the indicated line device.
//
LONG TSPIAPI tsplib_lineGetExtensionID (DWORD dwDeviceID, DWORD dwTSPIVersion,
         LPLINEEXTENSIONID lpExtensionID)
{
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	return (!IsDeletedObject(pLine)) ?
		pLine->GetExtensionID(dwTSPIVersion, (LPEXTENSIONID) lpExtensionID) :
		LINEERR_BADDEVICEID;

}// tsplib_lineGetExtensionID

//////////////////////////////////////////////////////////////////////////
// tsplib_lineGetIcon
//
// This function retreives a service line device-specific icon for
// display to the user
//
LONG TSPIAPI tsplib_lineGetIcon (DWORD dwDeviceID, LPCWSTR lpszDeviceClass,
      LPHICON lphIcon)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());

	// Initialize to zero
	*lphIcon = NULL;                

	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	return (!IsDeletedObject(pLine)) ?
		pLine->GetIcon (strDevClass, lphIcon) :
		LINEERR_BADDEVICEID;
   
}// tsplib_lineGetIcon

//////////////////////////////////////////////////////////////////////////
// tsplib_lineGetID
//
// This function returns a device id for the specified
// device class associated with the specified line, address, or call
// handle.
//
LONG TSPIAPI tsplib_lineGetID (HDRVLINE hdLine, DWORD dwAddressID,
         HDRVCALL hdCall, DWORD dwSelect, LPVARSTRING lpVarString,
         LPCWSTR lpszDeviceClass, HANDLE hTargetProcess)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());

    CTSPILineConnection* pLine = NULL;
    CTSPICallAppearance* pCall = NULL;
    CTSPIAddressInfo* pAddr = NULL;

    // Check how to find this connection info.  Based on the
    // information passed locate the proper connection info.
    LONG lResult = 0;
    switch(dwSelect)
    {
        case LINECALLSELECT_LINE:
            pLine = (CTSPILineConnection*) hdLine;
            if (IsDeletedObject(pLine))
				lResult = LINEERR_INVALLINEHANDLE;
			else
				lResult = pLine->GetID(strDevClass, lpVarString, hTargetProcess);
            break;

        case LINECALLSELECT_ADDRESS:
            pLine = (CTSPILineConnection*) hdLine;
            if (!IsDeletedObject(pLine))
            {
                pAddr = pLine->GetAddress(dwAddressID);
                if (pAddr == NULL)
                    lResult = LINEERR_INVALADDRESSID;
				else
					lResult = pAddr->GetID(strDevClass, lpVarString, hTargetProcess);
            }       
            else
                lResult = LINEERR_INVALLINEHANDLE;
            break;

        case LINECALLSELECT_CALL:
            pCall = (CTSPICallAppearance*) hdCall;
            if (!IsDeletedObject(pCall))
				lResult = pCall->GetID(strDevClass, lpVarString, hTargetProcess);
            else
                lResult = LINEERR_INVALCALLHANDLE;
            break;

        default:
			lResult = LINEERR_INVALPARAM;
            break;
    }

    return lResult;

}// tsplib_lineGetID

////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetLineDevStatus
//
// This function queries the specified open line for its status.  The
// information is valid for all addresses on the line.
//
LONG TSPIAPI tsplib_lineGetLineDevStatus (HDRVLINE hdLine, LPLINEDEVSTATUS lpLineDevStatus)
{
	_TSP_ASSERTE(lpLineDevStatus->dwTotalSize >= lpLineDevStatus->dwUsedSize);
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->GatherStatus (lpLineDevStatus) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineGetLineDevStatus

////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetNumAddressIDs
//
// This function returns the number of addresses availble on a line.
//
LONG TSPIAPI tsplib_lineGetNumAddressIDs (HDRVLINE hdLine, LPDWORD lpNumAddressIDs)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	if (IsDeletedObject(pLine))
		return LINEERR_INVALLINEHANDLE;

	*lpNumAddressIDs = pLine->GetAddressCount();
	return 0;

}// tsplib_lineGetNumAddressIDs

////////////////////////////////////////////////////////////////////////////
// tsplib_lineHold
//
// This function places the specified call appearance on hold.
//
LONG TSPIAPI tsplib_lineHold (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->Hold (dwRequestID) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineHold

////////////////////////////////////////////////////////////////////////////
// tsplib_lineMakeCall
//
// This function places a call on the specified line to the specified
// address.
//
LONG TSPIAPI tsplib_lineMakeCall (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         HTAPICALL htCall, LPHDRVCALL lphdCall, LPCWSTR lpszDestAddress,
         DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams)
{
	TString strDestAddr(ConvertWideToAnsi(lpszDestAddress));

	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->MakeCall (dwRequestID, htCall, lphdCall, strDestAddr.c_str(), dwCountryCode, lpCallParams) : 
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineMakeCall

///////////////////////////////////////////////////////////////////////////
// tsplib_lineMonitorDigits
//
// This function enables and disables the unbuffered detection of digits
// received on the call.  Each time a digit of the specified digit mode(s)
// is detected, a LINE_MONITORDIGITS message is sent to the application by
// TAPI.DLL, indicating which digit was detected.
//
LONG TSPIAPI tsplib_lineMonitorDigits (HDRVCALL hdCall, DWORD dwDigitModes)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->MonitorDigits(dwDigitModes) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineMonitorDigits

///////////////////////////////////////////////////////////////////////////
// tsplib_lineMonitorMedia
//
// This function enables and disables the detection of media modes on 
// the specified call.  When a media mode is detected, a LINE_MONITORMEDIA
// message is sent to TAPI.DLL.
//
LONG TSPIAPI tsplib_lineMonitorMedia (HDRVCALL hdCall, DWORD dwMediaModes)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->MonitorMedia (dwMediaModes) :
		LINEERR_INVALCALLHANDLE;
   
}// tsplib_lineMonitorMedia

///////////////////////////////////////////////////////////////////////////
// tsplib_lineMonitorTones
// 
// This function enables and disables the detection of inband tones on
// the call.  Each time a specified tone is detected, a message is sent
// to the client application through TAPI.DLL
//
LONG TSPIAPI tsplib_lineMonitorTones (HDRVCALL hdCall, DWORD dwToneListID,
         LPLINEMONITORTONE const lpToneList, DWORD dwNumEntries)
{
	LONG lResult = LINEERR_INVALCALLHANDLE;
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	if (!IsDeletedObject(pCall))
	{
		// Check the parameter values.                         
		if (lpToneList && dwNumEntries == 0L)
			lResult = LINEERR_INVALPARAM;
		else
		{
			// Allocate a new TONEMONITOR structure.
			TSPITONEMONITOR* lpMon = new TSPITONEMONITOR;
			if (lpMon == NULL)
				lResult = LINEERR_NOMEM;
			else
			{
				// Copy additional information.
				lpMon->dwToneListID = dwToneListID;                                 
                                         
				// Copy over all the tone entries    
				for (unsigned int i = 0; i < dwNumEntries; i++)
				{
					LPLINEMONITORTONE lpTone = new LINEMONITORTONE;
					if (lpTone == NULL)
					{
						lResult = LINEERR_NOMEM;
						break;
					}          
					MoveMemory(lpTone, lpToneList, sizeof(LINEMONITORTONE));
					lpMon->arrTones.push_back(lpTone);
				}        

				// Turn on the monitor with the call - delete the tonelist
				// if it fails.
				if (lResult == 0) lResult = pCall->MonitorTones (lpMon);
				if (lResult != 0) delete lpMon;
			}
		}
	}
	return lResult;
   
}// tsplib_lineMonitorTones

///////////////////////////////////////////////////////////////////////////
// tsplib_lineNegotiateExtVersion
//
// This function returns the highest extension version number the SP is
// willing to operate under for the device given the range of possible
// extension versions.
//
LONG TSPIAPI tsplib_lineNegotiateExtVersion (DWORD dwDeviceID, DWORD dwTSPIVersion,
         DWORD dwLowVersion, DWORD dwHiVersion, LPDWORD lpdwExtVersion)
{
	// Validate the parameter
	if (lpdwExtVersion != NULL && !IsBadWritePtr(lpdwExtVersion, sizeof(DWORD)))
	{
		CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
		return (pLine == NULL) ? 
				LINEERR_BADDEVICEID :
				pLine->NegotiateExtVersion(dwTSPIVersion, dwLowVersion, dwHiVersion, lpdwExtVersion);
	}
	return LINEERR_INVALPARAM;

}// tsplib_lineNegotiateExtVersion

///////////////////////////////////////////////////////////////////////////
// tsplib_lineNegotiateTSPIVersion
//
// This function is called to negotiate line versions for the TSP
// driver.
//
LONG TSPIAPI tsplib_lineNegotiateTSPIVersion (DWORD dwDeviceID,             
         DWORD dwLowVersion, DWORD dwHighVersion, LPDWORD lpdwTSPIVersion)
{
	if (lpdwTSPIVersion != NULL && !IsBadWritePtr(lpdwTSPIVersion, sizeof(DWORD)))
		return GetSP()->lineNegotiateTSPIVersion(dwDeviceID, dwLowVersion,
							 dwHighVersion, lpdwTSPIVersion);
	return LINEERR_INVALPARAM;

}// tsplib_lineNegotiateTSPIVersion

////////////////////////////////////////////////////////////////////////////
// tsplib_lineOpen
//
// This function opens the specified line device based on the device
// id passed and returns a handle for the line.  The TAPI.DLL line
// handle must also be retained for further interaction with this
// device.
//
LONG TSPIAPI tsplib_lineOpen (DWORD dwDeviceID, HTAPILINE htLine, 
         LPHDRVLINE lphdLine, DWORD dwTSPIVersion, LINEEVENT lpfnEventProc)
{
	// Find the line object in question   
	LONG lResult = LINEERR_BADDEVICEID;
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (!IsDeletedObject(pLine))
		lResult = pLine->Open(htLine, lpfnEventProc, dwTSPIVersion);

	// If it was successful, store off the line address
	*lphdLine = (lResult == 0) ? (HDRVLINE) pLine : 0;
	return lResult;

}// tsplib_lineOpen

//////////////////////////////////////////////////////////////////////////////
// tsplib_linePark
//
// This function parks the specified call according to the specified
// park mode.
//
LONG TSPIAPI tsplib_linePark (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         DWORD dwParkMode, LPCWSTR lpszDirAddr, LPVARSTRING lpNonDirAddress)
{
	TString strDestAddr(ConvertWideToAnsi (lpszDirAddr));

    if (dwParkMode != LINEPARKMODE_DIRECTED && dwParkMode != LINEPARKMODE_NONDIRECTED)
        return LINEERR_INVALPARKMODE;

	LONG lResult;
    CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
    if (!IsDeletedObject(pCall))
    {
		// If the directed address is supplied, then create a TDialStringArray
		if (dwParkMode == LINEPARKMODE_DIRECTED)
		{
			TDialStringArray arrStrings;
			lResult = GetSP()->DialInputToArray(pCall->GetLineOwner(), pCall->GetAddressOwner(), 
				strDestAddr.c_str(), 0, &arrStrings);
			if (lResult == 0)
				lResult = pCall->Park (dwRequestID, dwParkMode, &arrStrings, lpNonDirAddress);
		}
		else
			lResult = pCall->Park (dwRequestID, dwParkMode, NULL, lpNonDirAddress);
    }
    else
        lResult = LINEERR_INVALCALLHANDLE;
    
    return lResult;

}// tsplib_linePark

///////////////////////////////////////////////////////////////////////////////
// tsplib_linePickup
//
// This function picks up a call alerting at the specified destination
// address and returns a call handle for the picked up call.  If invoked
// with a NULL for the 'lpszDestAddr' parameter, a group pickup is performed.
// If required by the device capabilities, 'lpszGroupID' specifies the
// group ID to which the alerting station belongs.
//
LONG TSPIAPI tsplib_linePickup (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HTAPICALL htCall, LPHDRVCALL lphdCall,
         LPCWSTR lpszDestAddr, LPCWSTR lpszGroupID)
{
	TString strDestAddr(ConvertWideToAnsi (lpszDestAddr));
	TString strGroupID(ConvertWideToAnsi (lpszGroupID));

    LONG lResult = LINEERR_INVALLINEHANDLE;
    CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    if (!IsDeletedObject(pLine))
    {
        CTSPIAddressInfo* pAddr = pLine->GetAddress(dwAddressID);
        if (pAddr != NULL)
		{
			TDialStringArray arrDial;
			lResult = GetSP()->DialInputToArray(pAddr->GetLineOwner(), pAddr, strDestAddr.c_str(), 0, &arrDial);
			if (lResult == 0 || strDestAddr.empty()) // allow NULL address for group pickup
				lResult = pAddr->Pickup (dwRequestID, htCall, lphdCall, &arrDial, strGroupID.c_str());
		}
        else
            lResult = LINEERR_INVALADDRESSID;
    }
    return lResult;

}// tsplib_linePickup

////////////////////////////////////////////////////////////////////////////////
// tsplib_linePrepareAddToConference
//
// This function prepares an existing conference call for the addition of
// another party.  It creates a new temporary consultation call.  The new
// consultation call can subsequently be added to the conference call.
//
LONG TSPIAPI tsplib_linePrepareAddToConference (DRV_REQUESTID dwRequestID,
         HDRVCALL hdConfCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
         LPLINECALLPARAMS const lpCallParams)
{
	CTSPIConferenceCall* pCall = (CTSPIConferenceCall*) hdConfCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->PrepareAddToConference(dwRequestID, htConsultCall, lphdConsultCall, lpCallParams) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_linePrepareAddToConference

/////////////////////////////////////////////////////////////////////////////////
// tsplib_lineRedirect
//
// This function redirects the specified offering call to the specified
// destination address.
//
LONG TSPIAPI tsplib_lineRedirect (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCWSTR lpszDestAddr, DWORD dwCountryCode)
{
	TString strDestAddr(ConvertWideToAnsi (lpszDestAddr));

	LONG lResult = LINEERR_INVALCALLHANDLE;
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	if (!IsDeletedObject(pCall))
	{
		TDialStringArray arrDial;
		lResult = GetSP()->DialInputToArray(pCall->GetLineOwner(), pCall->GetAddressOwner(), 
			strDestAddr.c_str(), dwCountryCode, &arrDial);
		if (lResult == 0)
			lResult = pCall->Redirect (dwRequestID, &arrDial, dwCountryCode);
	}
	return lResult;

}// tsplib_lineRedirect

/////////////////////////////////////////////////////////////////////////////////
// tsplib_lineReleaseUserUserInfo
//
// This function releases a block of User->User information which is stored
// in the CALLINFO record.
//
LONG TSPIAPI tsplib_lineReleaseUserUserInfo (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->ReleaseUserUserInfo (dwRequestID) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineReleaseUserUserInfo

/////////////////////////////////////////////////////////////////////////////////
// tsplib_lineRemoveFromConference
//
// This function removes the specified call from the conference call to
// which it currently belongs.  The remaining calls in the conference call
// are unaffected.
//
LONG TSPIAPI tsplib_lineRemoveFromConference (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	LONG lResult = LINEERR_INVALCALLHANDLE;
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	if (!IsDeletedObject(pCall))
	{
		// Locate the conference call - it should be stored in the related call
		// field of the CALLINFO record.  Pass the request to the conference
		// owner for processing.
		CTSPIConferenceCall* pConfCall = pCall->GetConferenceOwner();
		lResult = (!IsDeletedObject(pConfCall)) ?
			pConfCall->RemoveFromConference(dwRequestID, pCall) : 
			LINEERR_INVALPARAM;
	}
	return lResult;

}// tsplib_lineRemoveFromConference

///////////////////////////////////////////////////////////////////////////////////
// tsplib_lineSecureCall
//
// This function secures the call from any interruptions or interference
// that may affect the call's media stream.
//
LONG TSPIAPI tsplib_lineSecureCall (DRV_REQUESTID dwRequestID, HDRVCALL hdCall)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->Secure(dwRequestID) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSecureCall

///////////////////////////////////////////////////////////////////////////////
// tsplib_lineSelectExtVersion
//
// This function selects the indicated extension version for the indicated
// line device.  Subsequent requests operate according to that extension
// version.
//
LONG TSPIAPI tsplib_lineSelectExtVersion (HDRVLINE hdLine, DWORD dwExtVersion)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->SelectExtVersion(dwExtVersion) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineSelectExtVersion

//////////////////////////////////////////////////////////////////////////////
// tsplib_lineSendUserUserInfo
//
// This function sends user-to-user information to the remote party on the
// specified call.
//
LONG TSPIAPI tsplib_lineSendUserUserInfo (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         LPCSTR lpsUserUserInfo, DWORD dwSize)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SendUserUserInfo(dwRequestID, lpsUserUserInfo, dwSize) : 
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSendUserUserInfo
                                                          
//////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAppSpecific
//
// This function sets the application specific portion of the 
// LINECALLINFO structure.  This is returned by the tsplib_lineGetCallInfo
// function.
//
LONG TSPIAPI tsplib_lineSetAppSpecific (HDRVCALL hdCall, DWORD dwAppSpecific)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ? 
			pCall->SetAppSpecificData(dwAppSpecific) :
			LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetAppSpecific

/////////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetCallData
//
// This function sets CALLDATA into a calls CALLINFO record.
//
LONG TSPIAPI tsplib_lineSetCallData (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
								   LPVOID lpCallData, DWORD dwSize)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SetCallData (dwRequestID, lpCallData, dwSize) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetCallData

/////////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetCallParams
//
// This function sets certain parameters for an existing call.
//
LONG TSPIAPI tsplib_lineSetCallParams (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate, 
         LPLINEDIALPARAMS const lpDialParams)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SetCallParams (dwRequestID, dwBearerMode, dwMinRate, dwMaxRate, lpDialParams) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetCallParams

///////////////////////////////////////////////////////////////////////////
// tsplib_lineSetCallTreatment
//
// Sets the call treatment for the specified call.  If the call
// treatment can go into effect then it happens immediately,
// otherwise it is set into place the next time the call enters
// a state where the call treatment is valid.
//
LONG TSPIAPI tsplib_lineSetCallTreatment (DRV_REQUESTID dwRequestID,
					HDRVCALL hdCall, DWORD dwCallTreatment)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SetCallTreatment (dwRequestID, dwCallTreatment) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetCallTreatment

////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetCurrentLocation (Win95)
//
// This function is called by TAPI whenever the address translation location
// is changed by the user (in the Dial Helper dialog or 
// 'lineSetCurrentLocation' function.  SPs which store parameters specific
// to a location (e.g. touch-tone sequences specific to invoke a particular
// PBX function) would use the location to select the set of parameters 
// applicable to the new location.
// 
LONG TSPIAPI tsplib_lineSetCurrentLocation (DWORD dwLocation)
{
	return GetSP()->lineSetCurrentLocation(dwLocation);

}// tsplib_lineSetCurrentLocation

////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetDefaultMediaDetection
//
// This function tells us the new set of media modes to watch for on 
// this line (inbound or outbound).
//
LONG TSPIAPI tsplib_lineSetDefaultMediaDetection (HDRVLINE hdLine, DWORD dwMediaModes)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->SetDefaultMediaDetection (dwMediaModes) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineSetDefaultMediaDetection

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetDevConfig
//
// This function restores the configuration of a device associated one-to-one
// with the line device from a data structure obtained through tsplib_lineGetDevConfig.
// The contents of the data structure are specific to the service provider.
//
LONG TSPIAPI tsplib_lineSetDevConfig (DWORD dwDeviceID, LPVOID const lpDevConfig,
         DWORD dwSize, LPCWSTR lpszDeviceClass)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());

	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	return (!IsDeletedObject(pLine)) ?
		pLine->SetDevConfig (strDevClass, lpDevConfig, dwSize) :
		LINEERR_BADDEVICEID;

}// tsplib_lineSetDevConfig

////////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetMediaControl
//
// This function enables and disables control actions on the media stream
// associated with the specified line, address, or call.  Media control actions
// can be triggered by the detection of specified digits, media modes,
// custom tones, and call states.  The new specified media controls replace all
// the ones that were in effect for this line, address, or call prior to this
// request.
//
LONG TSPIAPI tsplib_lineSetMediaControl (HDRVLINE hdLine, DWORD dwAddressID, 
         HDRVCALL hdCall, DWORD dwSelect, 
         LPLINEMEDIACONTROLDIGIT const lpDigitList, DWORD dwNumDigitEntries, 
         LPLINEMEDIACONTROLMEDIA const lpMediaList, DWORD dwNumMediaEntries, 
         LPLINEMEDIACONTROLTONE const lpToneList, DWORD dwNumToneEntries, 
         LPLINEMEDIACONTROLCALLSTATE const lpCallStateList, DWORD dwNumCallStateEntries)
{
    CTSPILineConnection* pLine = NULL;
    CTSPICallAppearance* pCall = NULL;
    CTSPIAddressInfo* pAddr = NULL;

    // Check how to find this connection info.  Based on the
    // information passed locate the proper connection info.
    LONG lResult = 0;

    switch(dwSelect)
    {
        case LINECALLSELECT_LINE:
            pLine = (CTSPILineConnection*) hdLine;
            if (IsDeletedObject(pLine))
                lResult = LINEERR_INVALLINEHANDLE;
            break;

        case LINECALLSELECT_ADDRESS:
            pLine = (CTSPILineConnection*) hdLine;
            if (!IsDeletedObject(pLine))
            {
                pAddr = pLine->GetAddress(dwAddressID);
                if (pAddr == NULL)
                    lResult = LINEERR_INVALADDRESSID;
            }
            else
                lResult = LINEERR_INVALLINEHANDLE;
            break;

        case LINECALLSELECT_CALL:
            pCall = (CTSPICallAppearance*) hdCall;
            if (!IsDeletedObject(pCall))
            {
                pAddr = pCall->GetAddressOwner();
                pLine = pAddr->GetLineOwner();
            }
            else
                lResult = LINEERR_INVALCALLHANDLE;
            break;

        default:
            _TSP_ASSERT(false);
            break;
    }

    if (lResult == 0)
    {
        lResult = GetSP()->lineSetMediaControl(pLine, pAddr, pCall, 
                  lpDigitList, dwNumDigitEntries, 
                  lpMediaList, dwNumMediaEntries, 
                  lpToneList, dwNumToneEntries, 
                  lpCallStateList, dwNumCallStateEntries);
    }
    return lResult;
   
}// tsplib_lineSetMediaControl

////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetMediaMode
//
// This function changes the provided calls media in the LINECALLSTATE
// structure.
//
LONG TSPIAPI tsplib_lineSetMediaMode (HDRVCALL hdCall, DWORD dwMediaMode)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SetMediaMode (dwMediaMode) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetMediaMode

///////////////////////////////////////////////////////////////////////////
// tsplib_lineSetQualityOfService
//
// This function attempts to nogotiate a level of QOS on the call with
// the switch.  If the desired QOS is not available, then it returns an
// error and remains at the current level of QOS.
//
LONG TSPIAPI tsplib_lineSetQualityOfService (DRV_REQUESTID dwRequestID,
					HDRVCALL hdCall, LPVOID lpSendingFlowSpec,
					DWORD dwSendingFlowSpecSize, LPVOID lpReceivingFlowSpec,
					DWORD dwReceivingFlowSpecSize)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	return (!IsDeletedObject(pCall)) ?
		pCall->SetQualityOfService(dwRequestID, lpSendingFlowSpec,
			dwSendingFlowSpecSize, lpReceivingFlowSpec, dwReceivingFlowSpecSize) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetQualityOfService

///////////////////////////////////////////////////////////////////////////
// tsplib_lineSetStatusMessages
//
// This function tells us which events to notify TAPI about when
// address or status changes about the specified line.
//
LONG TSPIAPI tsplib_lineSetStatusMessages (HDRVLINE hdLine, DWORD dwLineStates,
         DWORD dwAddressStates)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->SetStatusMessages (dwLineStates, dwAddressStates) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineSetStatusMessages

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetTerminal
//
// This operation enables TAPI.DLL to specify to which terminal information
// related to a specified line, address, or call is to be routed.  This
// can be used while calls are in progress on the line, to allow events
// to be routed to different devices as required.
//
LONG TSPIAPI tsplib_lineSetTerminal (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HDRVCALL hdCall, DWORD dwSelect, 
         DWORD dwTerminalModes, DWORD dwTerminalID, DWORD bEnable)
{
    CTSPILineConnection* pLine = NULL;
    CTSPILineConnection* pOwner = NULL;
    CTSPICallAppearance* pCall = NULL;
    CTSPIAddressInfo* pAddr = NULL;

    // Check how to find this connection info.  Based on the
    // information passed locate the proper connection info.
    LONG lResult = 0;

    switch(dwSelect)
    {
        case LINECALLSELECT_LINE:
            pLine = (CTSPILineConnection*) hdLine;
            if (IsDeletedObject(pLine))
                lResult = LINEERR_INVALLINEHANDLE;
			pOwner = pLine;
            break;

        case LINECALLSELECT_ADDRESS:
            pOwner = (CTSPILineConnection*) hdLine;
            if (!IsDeletedObject(pOwner))
            {
                pAddr = pOwner->GetAddress(dwAddressID);
                if (pAddr == NULL)
                    lResult = LINEERR_INVALADDRESSID;
            }
            else
                lResult = LINEERR_INVALLINEHANDLE;
            break;

        case LINECALLSELECT_CALL:
            pCall = (CTSPICallAppearance*) hdCall;
            if (pCall == NULL)
                lResult = LINEERR_INVALCALLHANDLE;
			else
				pOwner = pCall->GetLineOwner();
            break;

        default:
            _TSP_ASSERT(false);
            break;
    }

    if (lResult == 0)
		lResult = pOwner->SetTerminal(dwRequestID, pAddr, pCall, 
			dwTerminalModes, dwTerminalID, (bEnable) ? true : false);

    return lResult;
   
}// tsplib_lineSetTerminal

////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetupConference
//
// This function sets up a conference call for the addition of a third 
// party.
//
LONG TSPIAPI tsplib_lineSetupConference (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
         HDRVLINE hdLine, HTAPICALL htConfCall, LPHDRVCALL lphdConfCall,
         HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, DWORD dwNumParties,
         LPLINECALLPARAMS const lpLineCallParams)
{
	CTSPILineConnection* pLine = NULL;
	CTSPICallAppearance* pCall = NULL;

	// If the call handle is non-NULL, look it up.
	if (hdCall)
	{
		pCall = (CTSPICallAppearance*) hdCall;
		if (!IsDeletedObject(pCall))
			pLine = pCall->GetLineOwner();
	}
	else if (hdLine)
		pLine = (CTSPILineConnection*) hdLine;

	// Pass the request to the owner line
	return (!IsDeletedObject(pLine)) ?
		pLine->SetupConference(dwRequestID, pCall, htConfCall,
				  lphdConfCall, htConsultCall, lphdConsultCall, dwNumParties,
				  lpLineCallParams) : 
		(hdCall) ? LINEERR_INVALCALLHANDLE : LINEERR_INVALLINEHANDLE;

}// tsplib_lineSetupConference

////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetupTransfer
//
// This function sets up a call for transfer to a destination address.
// A new call handle is created which represents the destination
// address.
//
LONG TSPIAPI tsplib_lineSetupTransfer (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
       HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
       LPLINECALLPARAMS const lpCallParams)
{
	// Pass it through the library
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	_TSP_ASSERTE(lphdConsultCall != NULL);
	return (!IsDeletedObject(pCall)) ?
		pCall->GetLineOwner()->SetupTransfer(dwRequestID, pCall, htConsultCall, 
		lphdConsultCall, lpCallParams) : LINEERR_INVALCALLHANDLE;

}// tsplib_lineSetupTransfer

//////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetLineDevStatus
//
// The service provider sets the device status as indicated,
// sending the appropriate LINEDEVSTATE messages to indicate the
// new status.
//
LONG TSPIAPI tsplib_lineSetLineDevStatus (DRV_REQUESTID dwRequestID,
					HDRVLINE hdLine, DWORD dwStatusToChange,
					DWORD fStatus)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->SetLineDevStatus (dwRequestID, dwStatusToChange, (fStatus != 0) ? true : false) :
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineSetLineDevStatus

//////////////////////////////////////////////////////////////////////////////
// tsplib_lineSwapHold
//
// This function swaps the specified active call with the specified
// call on hold.
//
LONG TSPIAPI tsplib_lineSwapHold (DRV_REQUESTID dwRequestID, HDRVCALL hdCall,
       HDRVCALL hdHeldCall)
{
	CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
	CTSPICallAppearance* pHeldCall = (CTSPICallAppearance*) hdHeldCall;
	return (!IsDeletedObject(pCall) && !IsDeletedObject(pHeldCall)) ?
			pCall->SwapHold (dwRequestID, pHeldCall) :
			LINEERR_INVALCALLHANDLE;

}// tsplib_lineSwapHold

////////////////////////////////////////////////////////////////////////////
// tsplib_lineUncompleteCall
//
// This function is used to cancel the specified call completion request
// on the specified line.
//
LONG TSPIAPI tsplib_lineUncompleteCall (DRV_REQUESTID dwRequestID,
         HDRVLINE hdLine, DWORD dwCompletionID)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	return (!IsDeletedObject(pLine)) ?
		pLine->UncompleteCall (dwRequestID, dwCompletionID) : 
		LINEERR_INVALLINEHANDLE;

}// tsplib_lineUncompleteCall

////////////////////////////////////////////////////////////////////////////
// tsplib_lineUnhold
//
// This function retrieves the specified held call
//
LONG TSPIAPI tsplib_lineUnhold (DRV_REQUESTID dwRequestId, HDRVCALL hdCall)
{
   CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
   return (!IsDeletedObject(pCall)) ?
		pCall->Unhold (dwRequestId) :
		LINEERR_INVALCALLHANDLE;

}// tsplib_lineUnhold

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineUnpark
//
// This function retrieves the call parked at the specified
// address and returns a call handle for it.
//
LONG TSPIAPI tsplib_lineUnpark (DRV_REQUESTID dwRequestID, HDRVLINE hdLine,
         DWORD dwAddressID, HTAPICALL htCall, LPHDRVCALL lphdCall, 
         LPCWSTR lpszDestAddr)
{
	TString strDestAddr(ConvertWideToAnsi (lpszDestAddr));

    LONG lResult = LINEERR_INVALLINEHANDLE;
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    if (!IsDeletedObject(pLine))
    {
        CTSPIAddressInfo* pAddr = pLine->GetAddress(dwAddressID);
        if (!IsDeletedObject(pAddr))
		{
			TDialStringArray arrDial;
			lResult = GetSP()->DialInputToArray(pAddr->GetLineOwner(), pAddr, strDestAddr.c_str(), 0, &arrDial);
			if (lResult == 0)
			    lResult = pAddr->Unpark (dwRequestID, htCall, lphdCall, &arrDial);
		}
		else
            lResult = LINEERR_INVALADDRESSID;
    }
    return lResult;

}// tsplib_lineUnpark

/******************************************************************************/
//
// tsplib_phone functions
//
/******************************************************************************/

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneClose
//
// This function closes the specified open phone device after completing
// or aborting all outstanding asynchronous requests on the device.
//
LONG TSPIAPI tsplib_phoneClose (HDRVPHONE hdPhone)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->Close() : PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneClose

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneDevSpecific
//
// This function is used as a general extension mechanism to enable
// a TAPI implementation to provide features not generally available
// to the specification.
//
LONG TSPIAPI tsplib_phoneDevSpecific (DRV_REQUESTID dwRequestID, HDRVPHONE hdPhone,
               LPVOID lpParams, DWORD dwSize)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->DevSpecific(dwRequestID, lpParams, dwSize) :
		PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneDevSpecific

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetButtonInfo
//
// This function returns information about the specified phone 
// button.
//
LONG TSPIAPI tsplib_phoneGetButtonInfo (HDRVPHONE hdPhone, DWORD dwButtonId,
               LPPHONEBUTTONINFO lpButtonInfo)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetButtonInfo (dwButtonId, lpButtonInfo) :
		PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneGetButtonInfo

////////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetData
//
// This function uploads the information from the specified location
// in the open phone device to the specified buffer.
//
LONG TSPIAPI tsplib_phoneGetData (HDRVPHONE hdPhone, DWORD dwDataId,
               LPVOID lpData, DWORD dwSize)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetData (dwDataId, lpData, dwSize) :
		PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneGetData

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetDevCaps
//
// This function queries a specified phone device to determine its
// telephony capabilities
//
LONG TSPIAPI tsplib_phoneGetDevCaps (DWORD dwDeviceId, DWORD dwTSPIVersion,
               DWORD dwExtVersion, LPPHONECAPS lpPhoneCaps)
{
	CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceId);
	return (pPhone != NULL && !IsDeletedObject(pPhone)) ?
		pPhone->GatherCapabilities(dwTSPIVersion, dwExtVersion, lpPhoneCaps) :
		PHONEERR_BADDEVICEID;
   
}// tsplib_phoneGetDevCaps

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetDisplay
//
// This function returns the current contents of the specified phone
// display.
//
LONG TSPIAPI tsplib_phoneGetDisplay (HDRVPHONE hdPhone, LPVARSTRING lpString)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetDisplay (lpString) :
		PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneGetDisplay

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetExtensionID
//
// This function retrieves the extension ID that the service provider
// supports for the indicated device.
//
LONG TSPIAPI tsplib_phoneGetExtensionID (DWORD dwDeviceId, DWORD dwTSPIVersion,
               LPPHONEEXTENSIONID lpExtensionId)
{
	CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceId);
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetExtensionID(dwTSPIVersion, (LPEXTENSIONID) lpExtensionId) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneGetExtensionID

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetGain
//
// This function returns the gain setting of the microphone of the
// specified phone's hookswitch device.
//
LONG TSPIAPI tsplib_phoneGetGain (HDRVPHONE hdPhone, DWORD dwHookSwitchDev, LPDWORD lpdwGain)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetGain (dwHookSwitchDev, lpdwGain) : 
		PHONEERR_BADDEVICEID;

}// tsplib_phoneGetGain

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetHookSwitch
//
// This function retrieves the current hook switch setting of the
// specified open phone device
//
LONG TSPIAPI tsplib_phoneGetHookSwitch (HDRVPHONE hdPhone, LPDWORD lpdwHookSwitchDevs)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetHookSwitch (lpdwHookSwitchDevs) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneGetHookSwitch

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetIcon
//
// This function retrieves a specific icon for display from an
// application.  This icon will represent the phone device.
//
LONG TSPIAPI tsplib_phoneGetIcon (DWORD dwDeviceId, LPCWSTR lpszDeviceClass, LPHICON lphIcon)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());

	// Null out parameter
    *lphIcon = NULL;                

	CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceId);
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetIcon (strDevClass, lphIcon) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneGetIcon

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetID
//
// This function retrieves the device id of the specified open phone
// handle (or some other media handle if available).
//
LONG TSPIAPI tsplib_phoneGetID (HDRVPHONE hdPhone, LPVARSTRING lpDeviceId, LPCWSTR lpszDeviceClass, HANDLE hTargetProcess)
{
	TString strDevClass(ConvertWideToAnsi (lpszDeviceClass));
	if (!strDevClass.empty())
		CharLowerBuff(const_cast<LPTSTR>(strDevClass.c_str()), strDevClass.length());

	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetID(strDevClass, lpDeviceId, hTargetProcess) : PHONEERR_BADDEVICEID;

}// tsplib_phoneGetID

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetLamp
//
// This function returns the current lamp mode of the specified
// lamp.
//
LONG TSPIAPI tsplib_phoneGetLamp (HDRVPHONE hdPhone, DWORD dwButtonLampId, LPDWORD lpdwLampMode)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->GetLamp (dwButtonLampId, lpdwLampMode) :
			PHONEERR_BADDEVICEID;
   
}// tsplib_phoneGetLamp

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetRing
//
// This function enables an application to query the specified open
// phone device as to its current ring mode.
//
LONG TSPIAPI tsplib_phoneGetRing (HDRVPHONE hdPhone, LPDWORD lpdwRingMode, LPDWORD lpdwVolume)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GetRing (lpdwRingMode, lpdwVolume) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneGetRing

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetStatus
//
// This function queries the specified open phone device for its
// overall status.
//
LONG TSPIAPI tsplib_phoneGetStatus (HDRVPHONE hdPhone, LPPHONESTATUS lpPhoneStatus)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->GatherStatus (lpPhoneStatus) : PHONEERR_BADDEVICEID;

}// tsplib_phoneGetStatus

////////////////////////////////////////////////////////////////////////////
// tsplib_phoneGetVolume
//
// This function returns the volume setting of the phone device.
//
LONG TSPIAPI tsplib_phoneGetVolume (HDRVPHONE hdPhone, DWORD dwHookSwitchDev, LPDWORD lpdwVolume)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->GetVolume (dwHookSwitchDev, lpdwVolume) :
			PHONEERR_BADDEVICEID;

}// tsplib_phoneGetVolume

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneNegotiateTSPIVersion
//
// This function returns the highest SP version number the
// service provider is willing to operate under for this device,
// given the range of possible values.
//
LONG TSPIAPI tsplib_phoneNegotiateTSPIVersion (DWORD dwDeviceID,
               DWORD dwLowVersion, DWORD dwHighVersion,
               LPDWORD lpdwVersion)
{
	if (lpdwVersion != NULL && !IsBadWritePtr(lpdwVersion, sizeof(DWORD)))
	{
		// Let the phone object negotiate it's version
		CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceID);
		return (!IsDeletedObject(pPhone)) ?
			pPhone->NegotiateVersion(dwLowVersion, dwHighVersion, lpdwVersion) :
			PHONEERR_BADDEVICEID;
	}
	return PHONEERR_INVALPARAM;

}// tsplib_phoneNegotiateTSPIVersion   

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneNegotiateExtVersion
//
// This function returns the highest extension version number the
// service provider is willing to operate under for this device,
// given the range of possible extension values.
//
LONG TSPIAPI tsplib_phoneNegotiateExtVersion (DWORD dwDeviceID,
               DWORD dwTSPIVersion, DWORD dwLowVersion, DWORD dwHighVersion,
               LPDWORD lpdwExtVersion)
{
	// Validate the parameter
	if (lpdwExtVersion != NULL && !IsBadWritePtr(lpdwExtVersion, sizeof(DWORD)))
	{
		CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceID);
		return (!IsDeletedObject(pPhone)) ?
			pPhone->NegotiateExtVersion(dwTSPIVersion, dwLowVersion, dwHighVersion, lpdwExtVersion) :
			PHONEERR_BADDEVICEID;
	}
	return PHONEERR_INVALPARAM;

}// tsplib_phoneNegotiateExtVersion   

////////////////////////////////////////////////////////////////////////////
// tsplib_phoneOpen
//
// This function opens the phone device whose device ID is given,
// returning the service provider's opaque handle for the device and
// retaining the TAPI opaque handle.
//
LONG TSPIAPI tsplib_phoneOpen (DWORD dwDeviceId, HTAPIPHONE htPhone,
               LPHDRVPHONE lphdPhone, DWORD dwTSPIVersion, PHONEEVENT lpfnEventProc)
{
	CTSPIPhoneConnection* pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwDeviceId);
	LONG lResult = PHONEERR_BADDEVICEID;
	if (!IsDeletedObject(pPhone))
	{
		lResult = pPhone->Open(htPhone, lpfnEventProc, dwTSPIVersion);
		if (lResult == 0)
			*lphdPhone = (HDRVPHONE) pPhone;
	}
	return lResult;

}// tsplib_phoneOpen

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneSelectExtVersion
//
// This function selects the indicated extension version for the
// indicated phone device.  Subsequent requests operate according to
// that extension version.
//
LONG TSPIAPI tsplib_phoneSelectExtVersion (HDRVPHONE hdPhone, DWORD dwExtVersion)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->SelectExtVersion(dwExtVersion) :
			PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneSelectExtVersion

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetButtonInfo
//
// This function sets information about the specified button on the
// phone device.
//
LONG TSPIAPI tsplib_phoneSetButtonInfo (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, DWORD dwButtonId,
               LPPHONEBUTTONINFO const lpButtonInfo)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->SetButtonInfo (dwRequestId, dwButtonId, lpButtonInfo) :
			PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneSetButtonInfo

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetData
//
// This function downloads the information in the specified buffer
// to the opened phone device at the selected data id.
//
LONG TSPIAPI tsplib_phoneSetData (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwDataId, LPVOID const lpData, DWORD dwSize)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->SetData (dwRequestId, dwDataId, lpData, dwSize) :
			PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneSetData

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetDisplay
//
// This function causes the specified string to be displayed on the
// phone device.
//
LONG TSPIAPI tsplib_phoneSetDisplay (DRV_REQUESTID dwRequestID, 
         HDRVPHONE hdPhone, DWORD dwRow, DWORD dwCol, LPCWSTR lpszDisplay,
         DWORD dwSize)   
{
	TString strDisplay(ConvertWideToAnsi (lpszDisplay));
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->SetDisplay (dwRequestID, dwRow, dwCol, strDisplay.c_str(), dwSize) :
		PHONEERR_INVALPHONEHANDLE;

}// tsplib_phoneSetDisplay

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetGain
//
// This function sets the gain of the microphone of the specified hook
// switch device.
//
LONG TSPIAPI tsplib_phoneSetGain (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
								DWORD dwHookSwitchDev, DWORD dwGain)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->SetGain (dwRequestId, dwHookSwitchDev, dwGain) :
			PHONEERR_BADDEVICEID;

}// tsplib_phoneSetGain

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetHookSwitch
//
// This function sets the hook state of the specified open phone's
// hookswitch device to the specified mode.  Only the hookswitch
// state of the hookswitch devices listed is affected.
//
LONG TSPIAPI tsplib_phoneSetHookSwitch (DRV_REQUESTID dwRequestId, 
      HDRVPHONE hdPhone, DWORD dwHookSwitchDevs, DWORD dwHookSwitchMode)
{
   CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
   return (!IsDeletedObject(pPhone)) ?
			pPhone->SetHookSwitch (dwRequestId, dwHookSwitchDevs, dwHookSwitchMode) :
			PHONEERR_BADDEVICEID;

}// tsplib_phoneSetHookSwitch

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetLamp
//
// This function causes the specified lamp to be set on the phone
// device to the specified mode.
//  
LONG TSPIAPI tsplib_phoneSetLamp (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwButtonLampId, DWORD dwLampMode)
{
   CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
   return (!IsDeletedObject(pPhone)) ?
		pPhone->SetLamp (dwRequestId, dwButtonLampId, dwLampMode) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneSetLamp

///////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetRing
//
// This function rings the specified open phone device using the
// specified ring mode and volume.
//
LONG TSPIAPI tsplib_phoneSetRing (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwRingMode, DWORD dwVolume)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
		pPhone->SetRing (dwRequestId, dwRingMode, dwVolume) :
		PHONEERR_BADDEVICEID;
   
}// tsplib_phoneSetRing

//////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetStatusMessages
//
// This function causes the service provider to filter status messages
// which are not currently of interest to any application.
//
LONG TSPIAPI tsplib_phoneSetStatusMessages (HDRVPHONE hdPhone, DWORD dwPhoneStates,
            DWORD dwButtonModes, DWORD dwButtonStates)
{
	CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
	return (!IsDeletedObject(pPhone)) ?
			pPhone->SetStatusMessages(dwPhoneStates, dwButtonModes, dwButtonStates) :
			PHONEERR_BADDEVICEID;

}// tsplib_phoneSetStatusMessages

/////////////////////////////////////////////////////////////////////////
// tsplib_phoneSetVolume
//
// This function either sets the volume of the speaker or the 
// specified hookswitch device on the phone
//
LONG TSPIAPI tsplib_phoneSetVolume (DRV_REQUESTID dwRequestId, HDRVPHONE hdPhone, 
               DWORD dwHookSwitchDev, DWORD dwVolume)
{
   CTSPIPhoneConnection* pPhone = (CTSPIPhoneConnection*) hdPhone;
   return (!IsDeletedObject(pPhone)) ?
	    pPhone->SetVolume (dwRequestId, dwHookSwitchDev, dwVolume) :
		PHONEERR_BADDEVICEID;

}// tsplib_phoneSetVolume

/******************************************************************************/
//
// tsplib_provider functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// tsplib_providerInit
//
// This function is called when TAPI.DLL wants to initialize
// our service provider.
//
LONG TSPIAPI tsplib_providerInit (DWORD dwTSPIVersion,
         DWORD dwPermanentProviderID, DWORD dwLineDeviceIDBase,
         DWORD dwPhoneDeviceIDBase, DWORD_PTR dwNumLines, DWORD_PTR dwNumPhones,
         ASYNC_COMPLETION lpfnCompletionProc, LPDWORD lpdwTSPIOptions)
{
	return GetSP()->providerInit(dwTSPIVersion, dwPermanentProviderID,
                        dwLineDeviceIDBase, dwPhoneDeviceIDBase, dwNumLines,
                        dwNumPhones, lpfnCompletionProc, lpdwTSPIOptions);

}// tsplib_providerInit

///////////////////////////////////////////////////////////////////////////
// tsplib_providerShutdown
//
// This function is called when the TAPI.DLL is shutting down our
// service provider.
//
LONG TSPIAPI tsplib_providerShutdown (DWORD dwTSPIVersion, DWORD dwProviderID)
{
	// Locate the proper device.
	CTSPIDevice* pDevice = GetSP()->GetDevice(dwProviderID);
	return GetSP()->providerShutdown(dwTSPIVersion, pDevice);

}// tsplib_providerShutdown

////////////////////////////////////////////////////////////////////////////
// tsplib_providerEnumDevices (Win95)
//
// This function is called before the tsplib_providerInit to determine
// the number of line and phone devices supported by the service provider.
// If the function is not available, then TAPI will read the information
// out of the TELEPHON.INI file per TAPI 1.0.
//
LONG TSPIAPI tsplib_providerEnumDevices (DWORD dwPermanentProviderID, LPDWORD lpdwNumLines,
         LPDWORD lpdwNumPhones, HPROVIDER hProvider, LINEEVENT lpfnLineCreateProc,
         PHONEEVENT lpfnPhoneCreateProc)
{
	return GetSP()->providerEnumDevices(dwPermanentProviderID, lpdwNumLines,
			lpdwNumPhones, hProvider, lpfnLineCreateProc, lpfnPhoneCreateProc);

}// tsplib_providerEnumDevices

/////////////////////////////////////////////////////////////////////////////
// tsplib_providerCreateLineDevice  (Win95)
//
// This function is called by TAPI in response to the receipt of a 
// LINE_CREATE message from the service provider which allows the dynamic
// creation of a new line device.  The passed deviceId identifies this
// line from TAPIs perspective.
//
LONG TSPIAPI tsplib_providerCreateLineDevice (DWORD_PTR dwTempID, DWORD dwDeviceID)
{
	return GetSP()->providerCreateLineDevice(dwTempID, dwDeviceID);

}// tsplib_providerCreateLineDevice

/////////////////////////////////////////////////////////////////////////////
// tsplib_providerCreatePhoneDevice (Win95)
//
// This function is called by TAPI in response to the receipt of a
// PHONE_CREATE message from the service provider which allows the dynamic
// creation of a new phone device.  The passed deviceId identifies this
// phone from TAPIs perspective.
//
LONG TSPIAPI tsplib_providerCreatePhoneDevice (DWORD dwTempID, DWORD dwDeviceID)
{
	return GetSP()->providerCreatePhoneDevice(dwTempID, dwDeviceID);

}// tsplib_providerCreatePhoneDevice

/////////////////////////////////////////////////////////////////////////////
// tsplib_providerFreeDialogInstance (Tapi 2.0)
//
// This function is called to inform the service provider that
// the dialog associated with the "hdDlgInstance" has exited.
// After this function is called, the service provider should no
// longer send data to the dialog using the LINE_SENDDIALOGINSTANCEDATA
// message.
//
LONG TSPIAPI tsplib_providerFreeDialogInstance (HDRVDIALOGINSTANCE hdDlgInstance)
{
	return GetSP()->providerFreeDialogInstance (hdDlgInstance);

}// tsplib_providerFreeDialogInstance

/////////////////////////////////////////////////////////////////////////////
// tsplib_providerGenericDialogData (Tapi 2.0)
//
// This function delivers to the service provider data that was
// sent from the UI DLL running in an application context via
// the TSISPIDLLCALLBACK function.  The contents of the memory
// block pointed to be lpParams is defined by the service provider
// and UI DLL.  The service provider can modify the contents of the
// memory block; when this function returns, TAPI will copy the
// new modified data back to the original UI DLL parameter block.
//
LONG TSPIAPI tsplib_providerGenericDialogData (DWORD_PTR dwObjectID, DWORD dwObjectType, LPVOID lpParams, DWORD dwSize)
{
	CTSPIDevice* pDevice = NULL;
	CTSPILineConnection* pLine = NULL;
	CTSPIPhoneConnection* pPhone = NULL;
	HDRVDIALOGINSTANCE hdDlgInst = NULL;

	LONG lResult = 0L;

	if (dwObjectType == TUISPIDLL_OBJECT_LINEID)
	{
		pLine = GetSP()->GetConnInfoFromLineDeviceID(dwObjectID);
		if (!IsDeletedObject(pLine))
		{
			// Internal mechanism to convert line/phone ids into 
			if (dwSize == sizeof(DWORD)*2 && *(reinterpret_cast<LPDWORD>(lpParams)) == GDD_LINEPHONETOPROVIDER)
			{
				LPDWORD pdwInfo = reinterpret_cast<LPDWORD>(lpParams);
				*pdwInfo++ = GDD_LINEPHONETOPROVIDEROK;
				*pdwInfo = pLine->GetDeviceInfo()->GetProviderID();
				return 0;
			}
			// or permanent line/phone id
			else if (dwSize == sizeof(DWORD)*2 && *(reinterpret_cast<LPDWORD>(lpParams)) == GDD_LINEPHONETOPERMANENT)
			{
				LPDWORD pdwInfo = reinterpret_cast<LPDWORD>(lpParams);
				*pdwInfo++ = GDD_LINEPHONETOPERMANENTOK;
				*pdwInfo = pLine->GetPermanentDeviceID();
				return 0;
			}
		}
		else lResult = LINEERR_BADDEVICEID;
	}

	else if (dwObjectType == TUISPIDLL_OBJECT_PHONEID)
	{
		pPhone = GetSP()->GetConnInfoFromPhoneDeviceID(dwObjectID);
		if (!IsDeletedObject(pPhone))
		{
			// Internal mechanism to convert line/phone ids into 
			if (dwSize == sizeof(DWORD)*2 && *(reinterpret_cast<LPDWORD>(lpParams)) == GDD_LINEPHONETOPROVIDER)
			{
				LPDWORD pdwInfo = reinterpret_cast<LPDWORD>(lpParams);
				*pdwInfo++ = GDD_LINEPHONETOPROVIDEROK;
				*pdwInfo = pPhone->GetDeviceInfo()->GetProviderID();
				return 0;
			}
			// or permanent line/phone id
			else if (dwSize == sizeof(DWORD)*2 && *(reinterpret_cast<LPDWORD>(lpParams)) == GDD_LINEPHONETOPERMANENT)
			{
				LPDWORD pdwInfo = reinterpret_cast<LPDWORD>(lpParams);
				*pdwInfo++ = GDD_LINEPHONETOPERMANENTOK;
				*pdwInfo = pPhone->GetPermanentDeviceID();
				return 0;
			}
		} else 	lResult = PHONEERR_BADDEVICEID;
	}

	else if (dwObjectType == TUISPIDLL_OBJECT_PROVIDERID)
	{
		pDevice = GetSP()->GetDevice(dwObjectID);
		if (pDevice == NULL)
			lResult = TAPIERR_INVALDEVICEID;
	}

	else if (dwObjectType == TUISPIDLL_OBJECT_DIALOGINSTANCE)
		hdDlgInst = (HDRVDIALOGINSTANCE) dwObjectID;

	else
		lResult = LINEERR_INVALPARAM;

	if (lResult == 0)
		lResult = GetSP()->providerGenericDialogData (pDevice, pLine, 
					pPhone, hdDlgInst,lpParams, dwSize);
	
	return lResult;

}// tsplib_providerGenericDialogData

/////////////////////////////////////////////////////////////////////////////
// tsplib_providerUIIdentify (Tapi 2.0)
//
// This function returns the name of the UI DLL for this 
// service provider.
//
LONG TSPIAPI tsplib_providerUIIdentify (LPWSTR lpszUIDLLName)
{
	return GetSP()->providerUIIdentify(lpszUIDLLName);

}// tsplib_providerUIIdentify

/******************************************************************************/
//
// AGENT SUPPORT FUNCTION (for tracing)
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentGroup
//
// Called by the agent proxy to set the agent group for a particular line
// and address.
//
LONG TSPIAPI tsplib_lineSetAgentGroup(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST const lpGroupList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->SetAgentGroup(dwRequestID, lpGroupList);

}// tsplib_lineSetAgentGroup

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentState
//
// Called by the agent proxy to set the current agent state
//
LONG TSPIAPI tsplib_lineSetAgentState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentState, DWORD dwNextAgentState)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->SetAgentState(dwRequestID, dwAgentState, dwNextAgentState);

}// tsplib_lineSetAgentGroup

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentActivity
//
// Called by the agent proxy to set the current agent activity
//
LONG TSPIAPI tsplib_lineSetAgentActivity(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwActivityID)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->SetAgentActivity(dwRequestID, dwActivityID);

}// tsplib_lineSetAgentActivity

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentStatus
//
// Called by the agent proxy to query the current agent state
//
LONG TSPIAPI tsplib_lineGetAgentStatus(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTSTATUS lpStatus)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->GatherAgentStatus(lpStatus);

}// tsplib_lineGetAgentStatus

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentCaps
//
// Called by the agent proxy to query the capabilities of the address
//
LONG TSPIAPI tsplib_lineGetAgentCaps(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTCAPS lpCapabilities)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->GatherAgentCapabilities(lpCapabilities);

}// tsplib_lineGetAgentCaps

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentActivityList
//
// Called by the agent proxy to get the list of available agent activities
//
LONG TSPIAPI tsplib_lineGetAgentActivityList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTACTIVITYLIST lpList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->GetAgentActivityList(lpList);

}// tsplib_lineGetAgentActivityList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentGroupList
//
// Called by the agent proxy to query the available groups
//
LONG TSPIAPI tsplib_lineGetAgentGroupList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST lpList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->GetAgentGroupList(lpList);

}// tsplib_lineGetAgentGroupList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineAgentSpecific
//
// Called by the agent proxy to manage device-specific agent requests
//
LONG TSPIAPI tsplib_lineAgentSpecific(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentExtensionID, LPVOID lpvParams, DWORD dwSize)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	CTSPIAddressInfo* pAddress = pLine->GetAddress(dwAddressID);
	if (pAddress == NULL)
		return LINEERR_INVALADDRESSID;

	return pAddress->AgentSpecific(dwRequestID, dwAgentExtensionID, lpvParams, dwSize);

}// tsplib_lineGetAgentGroupList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineCreateAgent
//
// This function creates a new agent. 
//
LONG TSPIAPI tsplib_lineCreateAgent(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENT lphAgent, LPCWSTR pszMachineName, LPCWSTR pszUserName, LPCWSTR pszAgentID, LPCWSTR pszAgentPIN)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	USES_CONVERSION; 
	return pLine->CreateAgent(dwRequestID, lphAgent, W2CT(pszMachineName), W2CT(pszUserName), W2CT(pszAgentID), W2CT(pszAgentPIN));

}// tsplib_lineCreateAgent

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentMeasurementPeriod
//
// This function sets the measurement period associated with a particular agent.
//
LONG TSPIAPI tsplib_lineSetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwMeasurementPeriod)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->SetAgentMeasurementPeriod(dwRequestID, hAgent, dwMeasurementPeriod);

}// tsplib_lineSetAgentMeasurementPeriod

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentInfo
//
// This function retrieves the agent information for a specific agent.
//
LONG TSPIAPI tsplib_lineGetAgentInfo(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetAgentInfo(hAgent, lpAgentInfo);

}// tsplib_lineGetAgentInfo

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineCreateAgentSession
//
// This function creates a new agent session.
//
LONG TSPIAPI tsplib_lineCreateAgentSession(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCWSTR pszAgentPIN, const GUID* pGUID, DWORD dwWorkingAddressID)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	USES_CONVERSION;
	return pLine->CreateAgentSession(dwRequestID, lphSession, hAgent, W2CT(pszAgentPIN), *pGUID, dwWorkingAddressID);

}// tsplib_lineCreateAgentSession

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentSessionList
//
// This function retrieves the current agent session list.
//
LONG TSPIAPI tsplib_lineGetAgentSessionList(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetAgentSessionList(hAgent, lpSessionList);

}// tsplib_lineGetAgentSessionList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentSessionState
//
// This function sets the agent session state for a specific session.
//
LONG TSPIAPI tsplib_lineSetAgentSessionState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->SetAgentSessionState(dwRequestID, hSession, dwAgentSessionState, dwNextAgentSessionState);

}// tsplib_lineSetAgentSessionState

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetAgentSessionInfo
//
// This function retrieves the session information for a specific agent session
//
LONG TSPIAPI tsplib_lineGetAgentSessionInfo(DWORD dwDeviceID, HAGENTSESSION hAgentSession, LPLINEAGENTSESSIONINFO lpSessionInfo)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetAgentSessionInfo(hAgentSession, lpSessionInfo);

}// tsplib_lineGetAgentSessionInfo

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetQueueList
//
// This function retrieves the agent queue list
//
LONG TSPIAPI tsplib_lineGetQueueList(DWORD dwDeviceID, const GUID* pGroupID, LPLINEQUEUELIST lpQueueList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetQueueList(*pGroupID, lpQueueList);

}// tsplib_lineGetQueueList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetQueueMeasurementPeriod
//
// This function sets the measurement period for an agent queue
//
LONG TSPIAPI tsplib_lineSetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwQueueID, DWORD dwMeasurementPeriod)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->SetQueueMeasurementPeriod(dwRequestID, dwQueueID, dwMeasurementPeriod);

}// tsplib_lineSetQueueMeasurementPeriod

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetQueueInfo
//
// This function retrieves the queue information for a specific queue.
//
LONG TSPIAPI tsplib_lineGetQueueInfo(DWORD dwDeviceID, DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetQueueInfo(dwQueueID, lpQueueInfo);

}// tsplib_lineGetQueueInfo

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineGetGroupList
//
// This function retrieves the agent group list
//
LONG TSPIAPI tsplib_lineGetGroupList(DWORD dwDeviceID, LPLINEAGENTGROUPLIST lpGroupList)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->GetGroupList(lpGroupList);

}// tsplib_lineGetGroupList

/////////////////////////////////////////////////////////////////////////////
// tsplib_lineSetAgentStateEx
//
// This function changes an agents state.
//
LONG TSPIAPI tsplib_lineSetAgentStateEx(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwState, DWORD dwNextState)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL)
		return LINEERR_BADDEVICEID;

	// Pass off to the line implementation
	return pLine->SetAgentStateEx(dwRequestID, hAgent, dwState, dwNextState);

}// tsplib_lineSetAgentStateEx

/******************************************************************************/
//
// TAPI 3.0 (Windows 2000) SUPPORT
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// tsplib_lineMSPIdentify
// 
// This function determines the associated MSP CLSID for each line
// device. This function requires TAPI 3.0 negotiation.
//
LONG TSPIAPI tsplib_lineMSPIdentify(DWORD dwDeviceID, GUID* pGUID)
{
	// Find the line from the device id.
	CTSPILineConnection* pLine = GetSP()->GetConnInfoFromLineDeviceID(dwDeviceID);
	if (pLine == NULL || IsDeletedObject(pLine))
		return LINEERR_BADDEVICEID;
	return pLine->MSPIdentify(pGUID);

}// tsplib_lineMSPIdentify

///////////////////////////////////////////////////////////////////////////
// tsplib_lineCreateMSPInstance
// 
// This function creates a media service provider instance for a specific
// line device. This function returns a TSP handle for the MSP call. It
// requires TAPI 3.0 negotiation.
//
LONG TSPIAPI tsplib_lineCreateMSPInstance(HDRVLINE hdLine, DWORD dwAddressID, HTAPIMSPLINE htMSPLine, LPHDRVMSPLINE lphdMSPLine)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
    CTSPIAddressInfo* pAddr = NULL;
    if (!IsDeletedObject(pLine))
	{
        pAddr = pLine->GetAddress(dwAddressID);
		return (!IsDeletedObject(pAddr)) ?
			pAddr->CreateMSPInstance(htMSPLine, lphdMSPLine) : 
			LINEERR_INVALADDRESSID;
	}
	return LINEERR_INVALLINEHANDLE;

}// tsplib_lineCreateMSPInstance

///////////////////////////////////////////////////////////////////////////
// tsplib_lineCloseMSPInstance
// 
// This function closes an MSP call instance. This function 
// requires TAPI 3.0 negotiation.
//
LONG TSPIAPI tsplib_lineCloseMSPInstance(HDRVMSPLINE hdMSP)
{
	// TODO: Determine proper error return code - it is assumed that MS will 
	// eventually add error codes to TAPI.H for MSP devices??
	CMSPDriver* pMSP = (CMSPDriver*) hdMSP;
	return (!IsDeletedObject(pMSP)) ?
		pMSP->GetAddressOwner()->CloseMSPInstance(pMSP) : 
		LINEERR_OPERATIONFAILED;

}// tsplib_lineCloseMSPInstance

///////////////////////////////////////////////////////////////////////////
// tsplib_lineReceiveMSPData
// 
// This function receives data sent by a media service provider (MSP).
// It requires TAPI 3.0 negotiation.
//
LONG TSPIAPI tsplib_lineReceiveMSPData(HDRVLINE hdLine, HDRVCALL hdCall, HDRVMSPLINE hdMSP, LPVOID lpData, DWORD dwSize)
{
	LONG lResult = LINEERR_OPERATIONFAILED;

	CMSPDriver* pMSP = (CMSPDriver*) hdMSP;
	if (!IsDeletedObject(pMSP))
	{
		CTSPICallAppearance* pCall = (CTSPICallAppearance*) hdCall;
		if (pCall && !IsDeletedObject(pCall))
		{
			lResult = pCall->ReceiveMSPData(pMSP, lpData, dwSize);
			if (lResult != LINEERR_OPERATIONUNAVAIL)
				return lResult;
		}

		CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
		if (pLine && !IsDeletedObject(pLine))
			return pLine->ReceiveMSPData(pMSP, pCall, lpData, dwSize);
	}

	return lResult;

}// tsplib_lineReceiveMSPData

///////////////////////////////////////////////////////////////////////////
// tsplib_lineGetCallHubTracking
//
// This function retrieves the call hub tracking support structure (TAPI 3.0)
//
LONG TSPIAPI tsplib_lineGetCallHubTracking(HDRVLINE hdLine, LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	if (pLine == NULL || IsDeletedObject(pLine))
		return LINEERR_BADDEVICEID;
	return pLine->GetCallHubTracking(lpTrackingInfo);

}// tsplib_lineGetCallHubTracking

///////////////////////////////////////////////////////////////////////////
// tsplib_lineSetCallHubTracking
//
// This function sets the call hub tracking support structure (TAPI 3.0)
//
LONG TSPIAPI tsplib_lineSetCallHubTracking(HDRVLINE hdLine, LPLINECALLHUBTRACKINGINFO lpTrackingInfo)
{
	CTSPILineConnection* pLine = (CTSPILineConnection*) hdLine;
	if (pLine == NULL || IsDeletedObject(pLine))
		return LINEERR_BADDEVICEID;
	return pLine->SetCallHubTracking(lpTrackingInfo);

}// tsplib_lineSetCallHubTracking

