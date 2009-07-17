// OnSipInstall.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "tapi.h"
#include "windowsx.h"
#include "logger.h"
#include <string>
#include "Utils.h"

#define TSPNAME "OnSIP.tsp"

#undef DLLEXPORT
#define DLLEXPORT extern "C" 
//#define DLLEXPORT EXTERN_C STDAPICALLTYPE

#ifdef _MANAGED
#pragma managed(push, off)
#endif

long _isProviderInstalled(DWORD* providerId);
std::string _getTSPPath();
BOOL _uninstall();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	Logger::SetWin32Level( Logger::LEVEL_DEBUG );
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// Returns > 0 if installed
// Returns 0 if not installed
// Returns < 0 if error
DLLEXPORT 
BOOL __stdcall IsProviderInstalled()
{
	DWORD pid=0;
	long ret = _isProviderInstalled(&pid);
	Logger::log_debug("OnSipInstallDll::IsProviderInstalled ret=%lx pid=%ld", ret, pid );
	return ret > 0;
}

// Install the TSP provider path specified
DLLEXPORT 
BOOL __stdcall InstallProvider()
{
	Logger::log_debug("OnSipInstallDll::Installprovider");

	// Uninstall first if it is installed
	_uninstall();

	// Do the install
	DWORD dwid=0;
	std::string path = _getTSPPath();
	long ret = lineAddProvider( path.c_str(), NULL, &dwid );
	Logger::log_debug("OnSipInstallDll::Installprovider path=%s ret=%lx dwid=%ld", path.c_str(), ret, dwid );
	return ret == 0;
}

// UnInstall the TSP provider path specified
DLLEXPORT 
BOOL __stdcall UnInstallProvider()
{
	BOOL ret = _uninstall();
	Logger::log_debug("OnSipInstallDll::UnInstallProvider ret=%d", ret );
	return ret;
}

// UnInstall the TSP provider path specified
// Exact same method as UnInstallProvider, we just
// need a separate export to be called from Inno Setup
// due to Inno Setup limitation.
DLLEXPORT 
BOOL __stdcall UnInstallProvider2()
{
	BOOL ret = _uninstall();
	Logger::log_debug("OnSipInstallDll::UnInstallProvider2 ret=%d", ret );
	return ret;
}

//*********************************************************
//*********************************************************

BOOL _uninstall()
{
	// See if it is installed
	DWORD pid=0;
	long ret = _isProviderInstalled(&pid);
	Logger::log_debug("OnSipInstallDll::_uninstall ret=%ld dwid=%ld", ret, pid );

	// If error
	if ( ret < 0 )
		return FALSE;
	// If installed, then remove
	if ( ret > 0  )
	{
		ret = lineRemoveProvider( pid, NULL );
		Logger::log_debug("OnSipInstallDll::_uninstall lineRemoveProvider ret=%d", ret );
	}

	// Return < 0 if error, 0 if success
	return ret == 0;
}

// Returns the full path for the TSP (in the windows\system32 directory)
std::string _getTSPPath()
{
	// Get the windows path
	char windowsPath[MAX_PATH];
	GetWindowsDirectory( windowsPath, MAX_PATH );
	// Add the system32 and TSP name
	std::string path = Strings::stringFormat("%s\\%s\\%s", windowsPath, "system32", TSPNAME );
	Logger::log_debug("OnSipInstallDll::_getTSPPath <%s>", path.c_str() );
	return path;
}

// Checks to see if the TSP provider is installed or not.
// Returns < 0 if any type of error.
// If returns 0, then not installed.
// If > 0, then it is installed and providerId is the permanent id
long _isProviderInstalled(DWORD* providerId)
{
	LPLINEPROVIDERLIST pProviderList = NULL;
	LONG lResult=0;
	*providerId = -1;

	DWORD dwReqSize = sizeof(LINEPROVIDERLIST)*10;
	while (TRUE)
	{
		pProviderList = (LPLINEPROVIDERLIST) GlobalAllocPtr(GHND, dwReqSize);
		if (pProviderList == NULL)
		{
			Logger::log_error("OnSipInstallDll::IsProviderInstalled no memory");
			lResult = LINEERR_NOMEM;
			break;
		}

		pProviderList->dwTotalSize = dwReqSize;
		LONG ret = lineGetProviderList(0x00020000, pProviderList);
		if ( ret != 0 )
		{
			Logger::log_error("OnSipInstallDll::IsProviderInstalled lineGetProviderList error %lx",ret);
			lResult = LINEERR_OPERATIONFAILED;
			break;
		}

		// Go through the list of retrieved providers and see if we are included
		// in this list - TAPI will not add us to the registry until we return 
		// success to this function, so we should not currently be here.
		if (pProviderList->dwNeededSize <= pProviderList->dwTotalSize)
		{
			LPLINEPROVIDERENTRY pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
				pProviderList->dwProviderListOffset);
			for (DWORD i = 0; i < pProviderList->dwNumProviders; i++)
			{
				// Get the name of this provider.
				LPCSTR pszProvider = (LPCSTR) pProviderList+pProviderEntry->dwProviderFilenameOffset;

				// Make sure we are pointing at the TSP module, and not any path
				if (strrchr(pszProvider, '\\') != NULL)
					pszProvider = strrchr(pszProvider, '\\') + sizeof(char);

				Logger::log_debug("OnSipInstallDll::IsProviderInstalled ndx=%d provider=%s", i, pszProvider );

				// See if found our TSP
				if ( _stricmp( pszProvider, "onsip.tsp" ) == 0 )
				{
					*providerId = pProviderEntry->dwPermanentProviderID;
					break;
				}
				pProviderEntry++;
			}
			break;
		}
		else
		{
			dwReqSize = pProviderList->dwNeededSize;
			GlobalFreePtr ((LPSTR)pProviderList);
			pProviderList = NULL;
		}
	}

	if ( pProviderList )
		GlobalFreePtr ((LPSTR)pProviderList);

	Logger::log_debug("OnSipInstallDll::IsProviderInstalled lResult=%lx provid=%lx", lResult, *providerId );
	// If error
	if ( lResult < 0 )
		return lResult;
	// If success, then return > 0 if found
	return ( *providerId == (DWORD) -1 ) ? 0 : 1;
}


