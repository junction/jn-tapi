/******************************************************************************/
//                                                                        
// DEBUG.H - Debug support class
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _SPLIB_DEBUG_INC_
#define _SPLIB_DEBUG_INC_

/*----------------------------------------------------------------------------
    INCLUDE FILES
-----------------------------------------------------------------------------*/
#include <memory>

/**************************************************************************
** CDebugLoggerBase
**
** This object is used to log debug output
**
***************************************************************************/
class CDebugLoggerBase
{
// Class data
private:
	mutable DWORD m_dwTraceLevel;	// Current trace level

// Constructor
public:
	CDebugLoggerBase() : m_dwTraceLevel(0xffffffff) {/* */}
	virtual ~CDebugLoggerBase() {/* */}

/*
	static void traceit(const TCHAR* format, ...)
	{
		va_list v;
		va_start(v,format);
		// Get required length of characters, add 1 for NULL
		int len = _vsctprintf(format,v) + 1;
		// Allocate the string buffer
		TCHAR* str = new TCHAR[len];
		_vstprintf_s(str,len,format,v);
		va_end(v);
		// Convert to string
		std::string ret(str);
		ret += "\r\n";
		// Free memory and return formatted tstring
		delete[] str;
		OutputDebugString(ret.c_str());
	}
*/

// Abstract methods
public:
	virtual void TraceOut(LPCTSTR pszBuffer) const = 0;
	void BuildTraceBuffer(DWORD dwTraceLevel, LPCTSTR pszFormat, va_list args);
	void BuildTraceBuffer(DWORD dwTraceLevel, UINT uiFormat, va_list args);
	void SetTraceLevel(DWORD dwLevel) {	m_dwTraceLevel = dwLevel; }
	DWORD GetTraceLevel() const	{
#ifdef _DEBUG
		const DWORD dw = TRC_FULL & ~(TRC_LOCKS);
#else
		const DWORD dw = 0;
#endif
		return (m_dwTraceLevel != 0xffffffff) ? 
			m_dwTraceLevel : 
			GetSP()->ReadProfileDWord(0, _T("DebugLevel"), dw);
	}
};

/**************************************************************************
** CDebugBasic
**
** Simple object which just spits out to ODS.  This is the primary
** debug object.
**
***************************************************************************/
class CDebugBasic : public CDebugLoggerBase
{
// Overridden methods
public:
	virtual void TraceOut(LPCTSTR pszBuffer) const
	{
		// Spit it out to the debug facility
//		OutputDebugString(pszBuffer);
		// Then log it through the TSP class
		GetSP()->TraceOut(TString(pszBuffer));
	}
};

/**************************************************************************
** CDebugMgr
**
** This object manages the debug output for the service provider.
**
***************************************************************************/
class CDebugMgr
{
// Class data
private:
	static CDebugLoggerBase* g_dbgType;
	HMODULE m_hTraceModule;

// Constructor
public:
	CDebugMgr();
	~CDebugMgr();

// Access methods
public:
	static void SetupLogger(CDebugLoggerBase* pBase);
	static CDebugLoggerBase& Instance();
	static void Delete();
	static int __cdecl ReportHook(int iReportType, char* pszMessage, int* iRetVal);
};

#endif // _SPLIB_DEBUG_INC_
