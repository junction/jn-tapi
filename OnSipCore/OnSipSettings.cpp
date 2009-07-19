#include "stdafx.h"
#include "OnSipSettings.h"
#include "RegIO.h"
#include "Logger.h"

// Class wrapper around registry settings that are not
// dependent on the telephony provider location, etc.
// In consistent place, HKEY_LOCAL_MACHINE\Software\OnSIP

LPCTSTR regPath = _T("Software\\OnSIP");

// spLib.h
// Our repeat of the definitions in spLib.h (since we can't access from here)
// Only used to set the default values
int TRC_NONE	= 0x00000000;	// No Tracing
int TRC_MIN		= 0x00000001;	// Minimum tracing, TRACE macro, DTRACE macro
int TRC_API		= 0x00000002;	// TAPI api traces (parameters only)
int TRC_STRUCT	= 0x00000004;	// All structures to/from TAPI
int TRC_DUMP	= 0x00000008;	// Offset/Size pointers within structures to/from TAPI
int TRC_STATS   = 0x00000010;	// Statistics (event notification)
int TRC_OBJECTS = 0x00000020;	// Basic telephony object creation/destruction (addr/line/phone)
int TRC_THREADS	= 0x00000040;	// Thread creation/destruction
int TRC_REQUESTS = 0x00000080;	// Request creation/destruction
int TRC_CALLS	 = 0x00000100;	// Call creation/destruction
int TRC_CALLMAP	 = 0x00000200;	// Call id map
int TRC_WARNINGS	= 0x00000400;	// Warnings/Errors
int TRC_WORKERTHRD  = 0x00000800;	// Worker thread execution
int TRC_LOCKS		= 0x00001000;	// Full lock/unlock notifications
int TRC_CRITSEC     = 0x00002000;	// Win32 Critical section create/destroy
int TRC_AGENTPROXY  = 0x00004000;	// Agent proxy support
int TRC_USERDEFINED = 0x0FF00000;	// Derived TSP traces
int TRC_FULL		= 0x0FFFFFFF;	// All of the above

#ifdef _DEBUG
int DEFAULT_TSP_DEBUGLEVEL = (TRC_MIN | TRC_API | TRC_STATS | TRC_STRUCT | TRC_OBJECTS | TRC_CALLS | TRC_WARNINGS | TRC_THREADS );
int DEFAULT_DEBUGLEVEL = Logger::LEVEL_DEBUG;
#else
int DEFAULT_TSP_DEBUGLEVEL = TRC_WARNINGS;
int DEFAULT_DEBUGLEVEL = Logger::LEVEL_ERROR;
#endif

// Return the TSP Debug Level to control the Julmar TSP Layer debug
//static 
DWORD OnSipSettings::GetTSPDebugLevel()
{
	return RegIO::ReadDword(HKEY_LOCAL_MACHINE,regPath,"TSPDebugLevel",DEFAULT_TSP_DEBUGLEVEL);
}

// Return the general Debug Level to control the XMPP layer (non-Julmar)
// debug settings
//static 
Logger::LogLevel OnSipSettings::GetDebugLevel()
{
	return (Logger::LogLevel) RegIO::ReadDword(HKEY_LOCAL_MACHINE,regPath,"DebugLevel", DEFAULT_DEBUGLEVEL);
}

