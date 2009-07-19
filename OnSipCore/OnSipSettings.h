#ifndef ONSIPSETTINGS_H
#define ONSIPSETTINGS_H

#include "Logger.h"

// Class wrapper around registry settings that are not
// dependent on the telephony provider location, etc.
// In consistent place, HKEY_LOCAL_MACHINE\Software\OnSIP
class OnSipSettings
{
public:
	// Return the TSP Debug Level to control the Julmar TSP Layer debug
	// Flag settings are...
	//TRC_NONE		= 0x00000000,	// No Tracing
	//TRC_MIN		= 0x00000001,	// Minimum tracing, TRACE macro, DTRACE macro
	//TRC_API		= 0x00000002,	// TAPI api traces (parameters only)
	//TRC_STRUCT	= 0x00000004,	// All structures to/from TAPI
	//TRC_DUMP		= 0x00000008,	// Offset/Size pointers within structures to/from TAPI
	//TRC_STATS      = 0x00000010,	// Statistics (event notification)
	//TRC_OBJECTS    = 0x00000020,	// Basic telephony object creation/destruction (addr/line/phone)
	//TRC_THREADS	= 0x00000040,	// Thread creation/destruction
	//TRC_REQUESTS	= 0x00000080,	// Request creation/destruction
	//TRC_CALLS		= 0x00000100,	// Call creation/destruction
	//TRC_CALLMAP	= 0x00000200,	// Call id map
	//TRC_WARNINGS	= 0x00000400,	// Warnings/Errors
	//TRC_WORKERTHRD  = 0x00000800,	// Worker thread execution
	//TRC_LOCKS		= 0x00001000,	// Full lock/unlock notifications
	//TRC_CRITSEC     = 0x00002000,	// Win32 Critical section create/destroy
	//TRC_AGENTPROXY  = 0x00004000,	// Agent proxy support
	//TRC_USERDEFINED = 0x0FF00000,	// Derived TSP traces
	//TRC_FULL		= 0x0FFFFFFF,	// All of the above
	static DWORD GetTSPDebugLevel();

	// Return the general Debug Level to control the XMPP layer (non-Julmar)
	// debug settings
	static Logger::LogLevel  GetDebugLevel();
};

#endif
