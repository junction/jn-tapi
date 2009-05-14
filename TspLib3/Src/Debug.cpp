/******************************************************************************/
//                                                                        
// DEBUG.CPP - Debug support class
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This module includes the debug support for the TSP library
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "debug.h"
#include <crtdbg.h>
#include <process.h>

/*---------------------------------------------------------------------------*/
// CONSTANTS AND GLOBALS
/*---------------------------------------------------------------------------*/
CDebugLoggerBase* CDebugMgr::g_dbgType = NULL;

///////////////////////////////////////////////////////////////////////////
// CDebugLoggerBase::BuildTraceBuffer
//
// This builds a trace buffer and outputs it through the service provider.
//
void CDebugLoggerBase::BuildTraceBuffer(DWORD dwTraceLevel, LPCTSTR pszFormat, va_list args)
{
	const UINT MAX_BUFFER_SIZE = 4096;

	// If we aren't in this trace level, exit.
	if ((dwTraceLevel & GetTraceLevel()) == 0)
		return;

	// Put the contents into a buffer
	tsplib::inauto_ptr<TCHAR> pszBuff(new TCHAR[MAX_BUFFER_SIZE]);
	_TSP_VERIFY(wvsprintf(pszBuff.get(), pszFormat, args) < MAX_BUFFER_SIZE);
	TraceOut(pszBuff.get());

}// CDebugLoggerbase::BuildTraceBuffer

///////////////////////////////////////////////////////////////////////////
// CDebugLoggerBase::BuildTraceBuffer
//
// This builds a trace buffer and outputs it through the service provider.
//
void CDebugLoggerBase::BuildTraceBuffer(DWORD dwTraceLevel, UINT uiFormat, va_list args)
{
	const UINT MAX_BUFFER_SIZE = 512;

	// If we aren't in this trace level, exit.
	if ((dwTraceLevel & GetTraceLevel()) == 0)
		return;

	// Load our formatted string
	tsplib::inauto_ptr<TCHAR> pszBuff(new TCHAR[MAX_BUFFER_SIZE]);
	_TSP_VERIFY(LoadString(GetSP()->GetResourceInstance(), uiFormat, pszBuff.get(), MAX_BUFFER_SIZE) < MAX_BUFFER_SIZE);
	// Pass through to the formal version
	BuildTraceBuffer(dwTraceLevel, pszBuff.get(), args);

}// CDebugLoggerbase::BuildTraceBuffer

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::CDebugMgr
//
// This initializes our debug manager.
//
CDebugMgr::CDebugMgr() : m_hTraceModule(NULL)
{
	// Hook in the debug dll if we can find it.
	m_hTraceModule = LoadLibraryA("JTTSPTRC.DLL");
	if (m_hTraceModule != NULL)
	{
		typedef bool (*DBGPROC)(HANDLE);
		DBGPROC dbgProc = (DBGPROC)GetProcAddress(m_hTraceModule, "InitializeDebugLayer");
		if (dbgProc != NULL)
			(*dbgProc)(GetSP()->GetResourceInstance());
	}

}// CDebugMgr::CDebugMgr

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::~CDebugMgr
//
// This stops the debugger
//
CDebugMgr::~CDebugMgr()
{
	// Delete any debug logger object
	CDebugMgr::Delete();

}// CDebugMgr::~CDebugMgr

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::SetupLogger
//
// This sets the logger object
//
void CDebugMgr::SetupLogger(CDebugLoggerBase* pBase)
{
	CDebugLoggerBase* pCurrent = 
		reinterpret_cast<CDebugLoggerBase*>(
			InterlockedExchange(
				reinterpret_cast<long*>(&g_dbgType), 
				reinterpret_cast<long>(pBase)));
	delete pCurrent;

}// CDebugMgr::SetupLogger

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::Delete
//
// Delete the debug manager pointer
//
void CDebugMgr::Delete()
{
	CDebugLoggerBase* pCurrent = 
		reinterpret_cast<CDebugLoggerBase*>(
			InterlockedExchange(
				reinterpret_cast<long*>(&g_dbgType), 
				NULL));
	delete pCurrent;

}// CDebugMgr::Delete

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::Instance
//
// Returns the singleton debug manager object
//
CDebugLoggerBase& CDebugMgr::Instance()
{
	static CDebugBasic nop_;
	CDebugLoggerBase* pCurrent = g_dbgType;
	return (pCurrent == NULL) ? dynamic_cast<CDebugLoggerBase&>(nop_) : *pCurrent;

}// CDebugMgr::Instance

///////////////////////////////////////////////////////////////////////////
// CDebugMgr::ReportHook
//
// This hooks the MSVC CRT reporting facility and redirects it through
// the debug object.
//
int __cdecl CDebugMgr::ReportHook(int iReportType, char* pszMessage, int* iRetVal)
{
	// Trace out the data
	USES_CONVERSION;
	CDebugMgr::Instance().TraceOut(A2T(pszMessage));

	// If it is an ASSERT box, then route it through our internal ASSERT so it
	// can be shown on non-interactive desktops.
	if (iReportType == _CRT_ASSERT)
	{
		switch(MessageBoxA(NULL, pszMessage, "ASSERTION FAILURE", MB_SERVICE_NOTIFICATION | MB_SETFOREGROUND | MB_ABORTRETRYIGNORE | MB_ICONSTOP))
		{
			case IDABORT: ExitProcess(1); break;	// Stop process
			case IDRETRY: *iRetVal = 1; break;		// Start Debugger
			case IDIGNORE: 
			default:	  *iRetVal = 0; break;		// Ignore
		}
		return TRUE;
	}

	*iRetVal = 0;
	return TRUE; // Do not pass further down the reporting chain -- stops here.

}// CDebugLog::ReportHook
