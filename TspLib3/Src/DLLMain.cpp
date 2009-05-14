/******************************************************************************/
//                                                                        
// DLLMAIN.CPP - DLL Main entry point
//                                                                        
// Copyright (C) 1994-2000 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This initializes the TSP library and derived provider.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// This DLLMain handler is safe for CRT and MFC usage - both or either may be
// used - it uses the undocumented pRawDllMain handler to hook the DllMain
// process.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <ctype.h>
#include "debug.h"

/*---------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*---------------------------------------------------------------------------*/
HINSTANCE CServiceProvider::g_hInstance = NULL;
CServiceProvider* CServiceProvider::g_pAppObject = NULL;
static HMODULE g_hTraceModule = NULL;

/******************************************************************************/
// DllMain
// 
// Main entrypoint for the DLL - processes all thread attach and detach
// requests.
//
/******************************************************************************/
extern "C" 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	static long g_lAttached = 0;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Don't need thread notifications.
		DisableThreadLibraryCalls(hInstance);
		if (InterlockedIncrement(&g_lAttached) == 1)
		{
			// Quick check for Unicode on Win9x or for Ansi on NT - the second
			// case works ok it's just a bit slower due to all the conversions back
			// and forth for each API.
			OSVERSIONINFOA info;
			info.dwOSVersionInfoSize = sizeof(info);
			if (GetVersionExA(&info))
			{
#ifdef _UNICODE
				if (info.dwPlatformId != VER_PLATFORM_WIN32_NT)
				{
					MessageBoxA(NULL, "Cannot run Unicode version of a TAPI service provider on Windows 9x.\nPlease install a version intended for this operating system.", "TAPI", MB_ICONSTOP|MB_OK|MB_SERVICE_NOTIFICATION);
					return FALSE;
				}
#else
#ifdef _DEBUG
				if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
					OutputDebugString(_T("Running ANSI version of TSP on Windows NT : Slight Performance loss.\nPlease install the UNICODE version on NT.\n"));
#endif
#endif
			}

			// Save off the hInstance value for us to use in GetResourceInstance().
			CServiceProvider::g_hInstance = static_cast<HINSTANCE>(hInstance);

			// Hook in the debug dll if we can find it.
			g_hTraceModule = LoadLibraryA("JTTSPTRC.DLL");
			if (g_hTraceModule != NULL)
			{
				typedef bool (*DBGPROC)(HANDLE);
				DBGPROC dbgProc = (DBGPROC)GetProcAddress(g_hTraceModule, "InitializeDebugLayer");
				if (dbgProc != NULL)
					(*dbgProc)(hInstance);
			}
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		// If this is the final detach, unload JTTspTrcDLL. We cannot unload it
		// before this point since it hooks into our code through targeted JMPs
		// and so if any APIs get called after the unload we would crash. (v3.043)
		if (InterlockedDecrement(&g_lAttached) == 0)
		{
		// This code was determined to be causing problems with heap management for 
		// whatever reason. It has been disabled until we locate the problem.  Until then,
		// the tracing DLL simply won't be unloaded until TAPISRV.EXE unloads.
		// v3.044
		// if (g_hTraceModule)
		// {
		//		FreeLibrary(g_hTraceModule);
		//	    g_hTraceModule = NULL;
		// }
		}

		// Delete the debug object; the CRT is still initialized at this point.
		CDebugMgr::Delete();
	}

	return TRUE;

}// DLLMain
