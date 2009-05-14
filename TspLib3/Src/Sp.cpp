/******************************************************************************/
//                                                                        
// SP.CPP - Service Provider Base source code                             
//                                                                        
// Copyright (C) 1994-2005 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the general methods for the "CServiceProvider" class    
// which is the main CWinApp derived class in the SPLIB C++ library.      
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include <ctype.h>
#include <process.h>
#include <stdexcept>
#include "debug.h"

/*---------------------------------------------------------------------------*/
// RTTI FACTORY DEFINITIONS
/*---------------------------------------------------------------------------*/
DECLARE_TSPI_OVERRIDE(CTSPIDevice);
DECLARE_TSPI_OVERRIDE(CTSPILineConnection);
DECLARE_TSPI_OVERRIDE(CTSPIPhoneConnection);
DECLARE_TSPI_OVERRIDE(CTSPICallAppearance);
DECLARE_TSPI_OVERRIDE(CTSPIConferenceCall);
DECLARE_TSPI_OVERRIDE(CTSPIAddressInfo);

// This is ignored by the loader, so doesn't impact load time or image size.
#pragma comment (user, "TSP++ 3.052 (C) 1994-2005 JulMar Technology, Inc.")

/*-------------------------------------------------------------------------------*/
// GLOBALS and CONSTANTS
/*-------------------------------------------------------------------------------*/
static const TCHAR * const gszTelephonyKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");
static const TCHAR * const gszDevice = _T("Device%ld");
CServiceProvider* CServiceProvider::g_pAppObject = NULL;

#pragma data_seg ("init_data")
// These strings should be placed into a swappable segment since they 
// are only used during INIT to determine the capabilities of the provider.
// TAPI 2.x and above no longer exports by ordinal.
static const char * const gszEntryPoints[] = {
    "TSPI_lineAccept",							//	0
    "TSPI_lineAddToConference",					//	1
    "TSPI_lineAnswer",							//	2
    "TSPI_lineBlindTransfer",					//	3
    "TSPI_lineCompleteCall",					//	4
    "TSPI_lineCompleteTransfer",				//	5	
	"TSPI_lineConditionalMediaDetection",		//	6 
	"TSPI_lineDevSpecific",						//  7 
	"TSPI_lineDevSpecificFeature",				//  8
    "TSPI_lineDial",							//  9
    "TSPI_lineForward",							// 10
	"TSPI_lineGatherDigits",					// 11
	"TSPI_lineGenerateDigits",					// 12
	"TSPI_lineGenerateTone",					// 13
	"TSPI_lineGetDevConfig",					// 14
	"TSPI_lineGetExtensionID",					// 15
	"TSPI_lineGetIcon",							// 16
	"TSPI_lineGetID",							// 17
	"TSPI_lineGetLineDevStatus",				// 18
    "TSPI_lineHold",							// 19	
    "TSPI_lineMakeCall",						// 20	
	"TSPI_lineMonitorDigits",					// 21
	"TSPI_lineMonitorMedia",					// 22
	"TSPI_lineMonitorTones",					// 23
	"TSPI_lineNegotiateExtVersion",				// 24
    "TSPI_linePark",							// 25
    "TSPI_linePickup",							// 26
	"TSPI_linePrepareAddToConference",		    // 27
    "TSPI_lineRedirect",						// 28
	"TSPI_lineReleaseUserUserInfo",				// 29
    "TSPI_lineRemoveFromConference",			// 30
    "TSPI_lineSecureCall",						// 31
	"TSPI_lineSelectExtVersion",				// 32
    "TSPI_lineSendUserUserInfo",				// 33
    "TSPI_lineSetCallData",						// 34
    "TSPI_lineSetCallParams",					// 35	
    "TSPI_lineSetCallQualityOfService",			// 36
    "TSPI_lineSetCallTreatment",				// 37
	"TSPI_lineSetDevConfig",					// 38
    "TSPI_lineSetLineDevStatus",				// 39
	"TSPI_lineSetMediaControl",					// 40
    "TSPI_lineSetTerminal",						// 41	
    "TSPI_lineSetupConference",					// 42
    "TSPI_lineSetupTransfer",					// 43
    "TSPI_lineSwapHold",						// 44	
    "TSPI_lineUncompleteCall",					// 45
    "TSPI_lineUnhold",							// 46
    "TSPI_lineUnpark",							// 47
	"TSPI_phoneDevSpecific",					// 48
	"TSPI_phoneGetButtonInfo",					// 49
	"TSPI_phoneGetData",						// 50
	"TSPI_phoneGetDisplay",						// 51
	"TSPI_phoneGetExtensionID",					// 52
	"TSPI_phoneGetGain",						// 53
	"TSPI_phoneGetHookSwitch",					// 54
	"TSPI_phoneGetIcon",						// 55
	"TSPI_phoneGetID",							// 56
	"TSPI_phoneGetLamp",						// 57
	"TSPI_phoneGetRing",						// 58
	"TSPI_phoneGetVolume",						// 59
	"TSPI_phoneNegotiateExtVersion",			// 60
	"TSPI_phoneSelectExtVersion",				// 61
	"TSPI_phoneSetButtonInfo",					// 62
	"TSPI_phoneSetData",						// 63
	"TSPI_phoneSetDisplay",						// 64
	"TSPI_phoneSetGain",						// 65
	"TSPI_phoneSetHookSwitch",					// 66
	"TSPI_phoneSetLamp",						// 67
	"TSPI_phoneSetRing",						// 68
	"TSPI_phoneSetVolume",						// 69
	"TSPI_providerCreateLineDevice",			// 70
	"TSPI_providerCreatePhoneDevice",			// 71
	"TSPI_lineGetCallHubTracking",              // 72
};
#pragma data_seg ()

///////////////////////////////////////////////////////////////////////////
// Pull in the inline functions here as full code if we are not using
// inline functions.
#ifdef _NOINLINES_
#include "splib.inl"
#include "address.inl"
#include "call.inl"
#include "callhub.inl"
#include "device.inl"
#include "line.inl"
#include "phone.inl"
#include "request.inl"
#include "agent.inl"
#endif

///////////////////////////////////////////////////////////////////////////
// LibraryTimerThread
//
// Runs the internal timer for the library.
//
unsigned __stdcall tsplib_LibraryTimerThread(void* pParam)
{
	_TSP_DTRACEX(TRC_THREADS, _T("LibraryTimerThread(0x%lx) starting\n"), GetCurrentThreadId());
    reinterpret_cast<CServiceProvider*>(pParam)->IntervalTimer();
	_TSP_DTRACEX(TRC_THREADS, _T("LibraryTimerThread(0x%lx) ending\n"), GetCurrentThreadId());
	_endthreadex(0);
	return 0;

}// LibraryTimerThread

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CServiceProvider
//
// Constructor for our main application shell (CServiceProvider)
//
CServiceProvider::CServiceProvider(LPCTSTR pszAppName, LPCTSTR pszProviderInfo, DWORD dwVersion) :
	m_hProvider(0), m_dwTAPIVersionFound(0), m_pszUIName(pszAppName), m_hInstance(NULL),
	m_pszProviderInfo(pszProviderInfo), m_htLibraryTimer(0), m_pcurrLocation(0)
{
#ifdef _DEBUG
	// Change the default C-Runtime error reporting to go to
	// the debug output facility.  Otherwise, it might attempt to
	// output to the GUI which results in a deadlock in Windows NT TAPISRV
	// unless it is set to interactive desktop.
	_CrtSetReportHook(CDebugMgr::ReportHook);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Initialize our global 'this' instance.
	g_pAppObject = this;

	// Initialize our critical section for protecting TSP data
	InitializeCriticalSection(&m_csProvider);

	// Initialize the shutdown event
	m_hevtShutdown = CreateEvent(NULL, TRUE, 0, NULL);

    // Setup the default runtime objects.  These may be overriden by
    // calling the member "SetRuntimeObjects"   
	m_pObjects[0] = "CTSPILineConnection";
	m_pObjects[1] = "CTSPIPhoneConnection";
	m_pObjects[2] = "CTSPIDevice";
	m_pObjects[3] = "CTSPICallAppearance";
	m_pObjects[4] = "CTSPIAddressInfo";
	m_pObjects[5] = "CTSPIConferenceCall";

    // Version must be at least 2.0 according to TAPI documentation.
    if (dwVersion < TAPIVER_20)
        dwVersion = TAPIVER_20;
    m_dwTapiVerSupported = dwVersion;

	try
	{
		// Allocate a new location information block
		m_pcurrLocation = new LOCATIONINFO;
	}
	catch(...)
	{
		delete m_pcurrLocation;
		throw;
	}

    // Determine what provider abilities are exported from this module.
    DetermineProviderCapabilities();

}// CServiceProvider::CServiceProvider

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::~CServiceProvider
//
// Delete internal data
//
CServiceProvider::~CServiceProvider()
{
    // If the service provider is still running, then providerShutdown
    // never got called.  This can happen in TAPI if the providerInit
    // function failed (generally by the derived provider) and we still
    // initialized our device.
    while (GetDeviceCount() > 0)
    {
        CTSPIDevice* pDevice = GetDeviceByIndex(0);
#ifdef _DEBUG
        _TSP_ASSERTE(pDevice != NULL);
        _TSP_DTRACEX(TRC_WARNINGS, _T("Forcing shutdown of abandoned device %s\n"), pDevice->Dump().c_str());
#endif
        providerShutdown(GetSystemVersion(), pDevice);
    }

	// Delete our line connection map so it doesn't show up incorrectly as a memory leak
	// Both of these should be empty already (v3.01)
	m_mapLineConn.clear();
	m_mapPhoneConn.clear();

	// Delete the country/location information.
	delete m_pcurrLocation;

	// Delete our critical section and close our shutdown event
	DeleteCriticalSection(&m_csProvider);
	CloseHandle(m_hevtShutdown);

#ifdef _DEBUG
	// Reset the report hook otherwise TAPISRV can crash if it
	// attempts to write to cout.
	_CrtSetReportHook(NULL);
#endif

}// CServiceProvider::~CServiceProvider

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::IsRunningUnderTapiSrv
//
// This function can be called to determine if the module is loaded
// under TAPISRV or under a UI thread.  This allows the service provider
// constructor to do specific tasks based on UI or TSP activation.
//
bool CServiceProvider::IsRunningUnderTapiSrv()
{
	static int fIsRunning = -1;
	if (fIsRunning == -1)
	{
		TCHAR chModule[_MAX_PATH], *pchModule;
		::GetModuleFileName(NULL, chModule, _MAX_PATH);
		_tcsupr_s(chModule, _MAX_PATH);
		if ((pchModule = _tcsrchr(chModule, _T('\\'))) != NULL)
		{
			// Added SVCHOST.EXE for W2K/XP support
			++pchModule;
			fIsRunning = (!lstrcmpi(pchModule, _T("TAPISRV.EXE")) || !lstrcmpi(pchModule, _T("SVCHOST.EXE"))) ? 1 : 0;
		}
	}
	return (fIsRunning == 1) ? true : false;

}// CServiceProvider::IsRunningUnderTapiSrv

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::SetRuntimeObjects
//
// This method replaces some or all of the runtime objects used by
// the service provider class.  This allows the derived class to 
// override the functionallity of ANY of the used base classes, but
// still keep most of the creation/initialization hidden away.
//
void CServiceProvider::SetRuntimeObjects(LPCSTR pDevObj,
				LPCSTR pLineObj, LPCSTR pAddrObj,
				LPCSTR pCallObj, LPCSTR pConfCallObj, LPCSTR pPhoneObj)
{
    if (pLineObj)		m_pObjects[0] = pLineObj;
    if (pPhoneObj)		m_pObjects[1] = pPhoneObj;
    if (pDevObj)		m_pObjects[2] = pDevObj;
    if (pCallObj)		m_pObjects[3] = pCallObj;
    if (pAddrObj)		m_pObjects[4] = pAddrObj;
    if (pConfCallObj)	m_pObjects[5] = pConfCallObj;

}// CServiceProvider::SetRuntimeObjects

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::RemoveConnectionFromMap
//
// Remove a line/phone connection from the map
//
void CServiceProvider::RemoveConnectionFromMap(DWORD dwDeviceId, CTSPIConnection* pConn)
{
	if (dynamic_cast<CTSPILineConnection*>(pConn))
	{
		m_mapLineConn.erase(dwDeviceId);
	}
	else
	{
		_TSP_ASSERTE(dynamic_cast<CTSPIPhoneConnection*>(pConn) != NULL);
		m_mapPhoneConn.erase(dwDeviceId);
	}

}// CServiceProvider::MapConnectionToID

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::MapConnectionToID
//
// Access function for the global connection map
//
void CServiceProvider::MapConnectionToID(DWORD dwDeviceId, CTSPIConnection* pConn)
{
	if (CTSPILineConnection* pLine = dynamic_cast<CTSPILineConnection*>(pConn))
	{
		_TSP_ASSERTE(m_mapLineConn.find(dwDeviceId) == m_mapLineConn.end());
		m_mapLineConn[dwDeviceId] = pLine;
	}
	else
	{
		_TSP_ASSERTE(m_mapPhoneConn.find(dwDeviceId) == m_mapPhoneConn.end());
		CTSPIPhoneConnection* pPhone = dynamic_cast<CTSPIPhoneConnection*>(pConn);
		_TSP_ASSERTE(pPhone != NULL);
		m_mapPhoneConn[dwDeviceId] = pPhone;
	}

}// CServiceProvider::MapConnectionToID

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::IntervalTimer
//
// Walk through all the devices we are running and let our library timer
// run through it.
//
void CServiceProvider::IntervalTimer()
{
	CEnterCriticalSection csLock(&m_csProvider);
	csLock.Unlock();

    for (;;)
    {
        // Wait for either our object to signal (meaning our provider
        // is being shutdown), or for our timeout value to elapse.
        LONG lResult = WaitForSingleObject (m_hevtShutdown, LIBRARY_INTERVAL);
        if (lResult == WAIT_OBJECT_0 || lResult == WAIT_ABANDONED)
            break;

		// First cleanup any call appearances which have been deleted.
		// They are kept around for DELETECALL_STALE mSecs in case
		// TAPISRV sends us a deleted call handle for a secondary lineDrop
		// when the object was already gone.
		csLock.Lock();

		// Determine how quickly we are to delete our calls.  This is based on how
		// many calls are actually in our deleted list.  We keep the calls around because
		// according to MS, in the client/server version of TAPISRV, the call handle can
		// come back to us AFTER it has been deallocated.
		DWORD lCurrTime = GetTickCount();
		DWORD lDeleteTime = (m_lstDeletedCalls.size() > 100) ? DELETECALL_STALE/2 : 
							(m_lstDeletedCalls.size() > 50) ? DELETECALL_STALE/3 : 
							DELETECALL_STALE;

		// Spin through the loop deleting calls
		TCallList::iterator pos;
		for (pos = m_lstDeletedCalls.begin(); 
				pos != m_lstDeletedCalls.end();)
		{
			CTSPICallAppearance* pCall = (*pos);
			if ((lCurrTime > pCall->m_lDeleteTime + lDeleteTime) ||
				(lCurrTime > ~(pCall->m_lDeleteTime + lDeleteTime)))
			{
				_TSP_DTRACEX(TRC_CALLS, _T("Deleting call 0x%lx, CallID=0x%lx [Count=%ld]\n"), 
							pCall, pCall->GetCallID(), m_lstDeletedCalls.size());
				pos = m_lstDeletedCalls.erase(pos);
				delete pCall;
			}
			else break;  // early exit, assume that list is FIFO.
		}
		csLock.Unlock();

		// Next, walk through any calls which need timeout values.
		csLock.Lock();
		for (pos = m_lstTimedCalls.begin(); pos != m_lstTimedCalls.end();)
		{
			if ((*pos)->OnInternalTimer())
				pos = m_lstTimedCalls.erase(pos);
			else ++pos;
		}
		csLock.Unlock();
    }

	// Delete any calls left over in our array
	csLock.Lock();
	_TSP_DTRACEX(TRC_CALLS, _T("Emptying deleted call list [Count=%ld]\n"), m_lstDeletedCalls.size());
	std::for_each(m_lstDeletedCalls.begin(), m_lstDeletedCalls.end(), tsplib::delete_object());
	m_lstDeletedCalls.clear();

}// CServiceProvider::IntervalTimer

///////////////////////////////////////////////////////////////////////////
// CTSPIServiceProvider::ConvertDialableToCanonical
//
// This function is used to convert a dialable number into
// a standard TAPI canonical number.
//
// This is the format +x (xxx) xxx-xxxx
//                               
TString CServiceProvider::ConvertDialableToCanonical(LPCTSTR pszNumber, DWORD /*dwCountryCode*/, bool fInbound/*=false*/)
{   
	// If the number is NULL, then return a blank string so it can be assigned to
	// a std::basic_string collection.
	if (pszNumber == NULL)
		return _T("");

	// If the number is already in canonical format then use that data.
	if (*pszNumber == _T('+'))
		return pszNumber;

	// If we have not yet loaded our country information, do so now.
	if (m_pcurrLocation != NULL)
	{
		// We need to load the location data to parse the number out.
		if (!m_pcurrLocation->IsLoaded())
		{
			// If it fails to load, then just return the number in it's raw form.
			if (!m_pcurrLocation->Reload())
			{
				_TSP_DTRACEX(TRC_WARNINGS, _T("Failed to load LOCATIONINFO from TAPI; cannot format canonical numbers."));
				return pszNumber;
			}
		}

		// If the number of digits we were passed is less than required to build a 
		// canonical formatted number then simply pass the same digits back. This
		// is specifically for in-switch calls
		TString strInput(GetDialableNumber(pszNumber, (m_pcurrLocation->strCallWaiting.empty()) ? 
							NULL : m_pcurrLocation->strCallWaiting.c_str()));
		if (strInput.length() < 7 || m_pcurrLocation == NULL)
			return pszNumber;

		TString strCountryCode, strAreaCode, strNumber, strOutput;

		// If this is an outbound call, then check for prefix digits.
		if (!fInbound)
		{
			// Strip out any prefix digits required to get an outside line from the dialing string.
			// This should be the first element in the string.
			if (!m_pcurrLocation->strLocalAccess.empty() &&
				strInput.substr(0, m_pcurrLocation->strLocalAccess.length()) ==
				m_pcurrLocation->strLocalAccess)
				strInput = strInput.substr(m_pcurrLocation->strLocalAccess.length());
			else if (!m_pcurrLocation->strLongDistanceAccess.empty() &&
					strInput.substr(0, m_pcurrLocation->strLongDistanceAccess.length()) ==
					m_pcurrLocation->strLongDistanceAccess)
				strInput = strInput.substr(m_pcurrLocation->strLongDistanceAccess.length());

			// Strip out call waiting information from the dialing string
			if (!m_pcurrLocation->strCallWaiting.empty() &&
				strInput.substr(0, m_pcurrLocation->strCallWaiting.length()) ==
				m_pcurrLocation->strCallWaiting)
				strInput = strInput.substr(m_pcurrLocation->strCallWaiting.length());
		}

		// See if this is an international call based on the international digits.
		if (!m_pcurrLocation->strIntlCode.empty() &&
			strInput.substr(0, m_pcurrLocation->strIntlCode.length()) == 
			m_pcurrLocation->strIntlCode)
		{
			// Remove the international sequence.
			strInput = strInput.substr(m_pcurrLocation->strIntlCode.length());

			// Get the country code we are dialing.
			strCountryCode = DetermineCountryCode(strInput.substr(0,3));
			strInput = strInput.substr(strCountryCode.length());

			// Determine the area code for the number.
			strAreaCode = DetermineAreaCode(strCountryCode, strInput);
		}

		// International call coming inbound without international digits in caller id.
		else if (fInbound && lstrlen(pszNumber) > 10)
		{
			// Get the country code we are dialing.
			strCountryCode = DetermineCountryCode(strInput.substr(0,3));
			strInput = strInput.substr(strCountryCode.length());

			// Determine the area code for the number.
			strAreaCode = DetermineAreaCode(strCountryCode, strInput);
		}

		// Non-international call -- parse out the long-distance code (if included) and
		// the area code information from the call.
		else
		{
			// If we have a national code for our country, then strip that information out
			// of the string if it is present.  This value is the sequence of digits which allow
			// the user to place a non-local call.
			//
			// Thanks to Martin Roth @ Ascom for this piece of code.
			if (!m_pcurrLocation->strNationalCode.empty() &&
						  strInput.substr(0, m_pcurrLocation->strNationalCode.length()) ==
						  m_pcurrLocation->strNationalCode)
			{
				// Strip out the national code
				strInput = strInput.substr(m_pcurrLocation->strNationalCode.length());
			}

			// If the country code is included in the number, then strip it out.
			// This happens in some dialing areas with caller-id.
			else if (!m_pcurrLocation->strCountryCode.empty() &&
				strInput.length() > 10 && 
				strInput.substr(0, m_pcurrLocation->strCountryCode.length()) ==
				m_pcurrLocation->strCountryCode)
			{
				// Strip out the coutry code
				strInput = strInput.substr(m_pcurrLocation->strCountryCode.length());
			}

			// Set the country code
			strCountryCode = m_pcurrLocation->strCountryCode;

			// Determine the area code for the number.
			strAreaCode = DetermineAreaCode(strCountryCode, strInput);
		}

		// Now determine what the number is based on the remaining digits in the
		// dialable string.
		TString::size_type nPos = strInput.find_first_not_of(_T("0123456789"));
		if (nPos == TString::npos)
			nPos = strInput.length();
		strNumber = strInput.substr(0,nPos);

		// First character in canonical format is "+" followed by country code.
		if (!strCountryCode.empty())
		{
			strOutput = _T("+");
			strOutput += strCountryCode;
			strOutput += _T(" ");
		}

		// Create our phone number
		if (!strAreaCode.empty())
			strOutput += _T("(") + strAreaCode + _T(") ");

		// Append the numerical number to the output.
		strOutput += strNumber;
	    return strOutput;    
	}
	return pszNumber;
    
}// CServiceProvider::ConvertDialableToCanonical

///////////////////////////////////////////////////////////////////////////
// DetermineCountryCode
//
// This function attempts to determine the digits which are part of
// the country code. If the given country is not known then it simply
// returns the passed input.
//
TString CServiceProvider::DetermineCountryCode(const TString& strInput) const
{
	// List of valid known country codes in size order
	static unsigned short Countries9[] = {
		995,994,993,977,976,975,974,973,972,971,968,967,966,965,964,963,962,961,960,98,95,94,93,92,91,90,0 };
	static unsigned short Countries8[] = {
		886,880,874,873,872,871,870,856,855,853,852,850,800,86,84,82,81,0 };
	static unsigned short Countries7[] = {
		7,0 };
	static unsigned short Countries6[] = {
		692,691,690,689,688,687,686,685,684,683,682,681,680,679,678,677,676,675,674,673,672,670,66,65,64,63,62,61,60,0 };
	static unsigned short Countries5[] = {
		599,598,597,596,595,594,593,592,591,590,509,508,507,506,505,504,503,502,501,500,58,57,56,55,54,53,52,51,0 };
	static unsigned short Countries4[] = {
		421,420,49,48,47,46,45,44,43,41,40,0 };
	static unsigned short Countries3[] = {
		389,387,386,385,381,380,378,377,376,375,374,373,372,371,370,359,358,357,356,355,354,353,352,351,350,39,39,36,34,33,32,31,30,0 };
	static unsigned short Countries2[] = {
		299,298,297,291,290,269,268,267,266,265,264,263,262,261,260,258,257,256,255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,218,216,213,212,27,20,0 };
	static unsigned short Countries1[] = {
		1,0 };

	unsigned short* pSearch = NULL;
	switch (strInput[0])
	{
		case _T('9'):	pSearch = Countries9; break;
		case _T('8'):	pSearch = Countries8; break;
		case _T('7'):	pSearch = Countries7; break;
		case _T('6'):	pSearch = Countries6; break;
		case _T('5'):	pSearch = Countries5; break;
		case _T('4'):	pSearch = Countries4; break;
		case _T('3'):	pSearch = Countries3; break;
		case _T('2'):	pSearch = Countries2; break;
		case _T('1'):	pSearch = Countries1; break;
	}

	// Search the sub-list based on the first character
	if (pSearch != NULL)
	{
		TCHAR chBuff[5];
		while (*pSearch != 0)
		{
			_itot_s(*pSearch,chBuff,10);
			if (!_tcsncmp(strInput.c_str(), chBuff, lstrlen(chBuff)))
				return chBuff;
			++pSearch;
		}
	}
	return strInput;

}// DetermineCountryCode

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::DetermineAreaCode
//
// This function is used to locate the area code information within
// the given dialing string. It is coded specifically for the U.S. and
// requires override for many countries. In some cases, the area code
// cannot be fully determined (in Europe for instance where it is a 
// variable-number of digits).  Override this to do what you need if 
// it is not sufficient for your locale.
//
// If someone can come up with a way to always determine the area code
// regardless of country, we would love to add the function to the 
// library and would certainly credit the author :-)
//
TString CServiceProvider::DetermineAreaCode(const TString& strCountryCode, TString& strInput)
{
	TString strAreaCode = _T("");

	// If the country code doesn't match our current location, then don't try
	// to determine an area code.  Since the international rules are not fully
	// described within TAPI, the bulk of area-code translation would be upon
	// the service provider.  The TSP should override this functon for other countries.
	if (strCountryCode != m_pcurrLocation->strCountryCode)
	{
		const int nUSAreaCodeLength = 3;
		const int nUSFullNumberLength = 10;

		// Go ahead and implement a simple parser for US numbers -- since 
		// the area code rules are consistant in that country.
		if (strCountryCode == _T("1"))
		{
			if (strInput.length() >= nUSFullNumberLength)
			{
				strAreaCode = strInput.substr(0,nUSAreaCodeLength);
				strInput = strInput.substr(nUSAreaCodeLength);
			}
		}
	}

	// See if we have area code information embedded in the number. We try
	// to determine this by using the NA standard of a 7-digit number with
	// some area code length.  If we have enough digits left in the string, then
	// pull the first 'n' digits off as the area code.  Otherwise, we will assume
	// that the area code is not present and that this is within the default
	// area code for our location. (V3.04)
	else // same country
	{
		strAreaCode = m_pcurrLocation->strAreaCode;
		if (strInput.length() >= 7 + m_pcurrLocation->strAreaCode.length())
		{
			// We will assume in this generic function that the area code is the
			// same length as the current location's area code. Since this routine is
			// is only executed for non-international calls, this should be an ok assumption
			// for many, if not most, countries.
			if (strInput.substr(0, m_pcurrLocation->strAreaCode.length()) != m_pcurrLocation->strAreaCode)
				strAreaCode = strInput.substr(0,m_pcurrLocation->strAreaCode.length());

			// Remove the area code from the input string. This should _not_ done if we didn't
			// have enough digits to include it..
			strInput = strInput.substr(strAreaCode.length());
		}
	}
	return strAreaCode;

}// CServiceProvider::DetermineAreaCode

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetDialableNumber
//
// This method will go through a dialable string and return the 
// portion that is considered the "number" (ie: digits only) which 
// can be used to represent an address.
//
TString CServiceProvider::GetDialableNumber(LPCTSTR pszNumber, LPCTSTR pszAllow) const
{                 
    TString strReturn;
    if (pszNumber != NULL)
    {
        while (*pszNumber)
        {   
            if (tsplib::is_tdigit(*pszNumber) || (pszAllow && _tcschr(pszAllow, *pszNumber)))
                strReturn += *pszNumber;
            pszNumber++;
        }
    }                
    return strReturn;

}// CServiceProvider::GetDialableNumber

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CheckDialableNumber
//
// Check the provided string to determine if our service provider
// supports the type of number given.
//
// This function should be overriden to provide additional checks on the
// dialable number (ie: billing info, etc.)
//
LONG CServiceProvider::CheckDialableNumber(           
CTSPILineConnection* pLine,		// Line for this check
CTSPIAddressInfo* pAddr,		// Address for this check
LPCTSTR lpszDigits,				// Original input
TDialStringArray* parrEntries,  // Entry array for broken number information
DWORD  /*dwCountry*/,			// Country code for this number
LPCTSTR lpszValidChars)			// Valid characters for the number
{
	// If the digits passed is NULL, error it.
	if (lpszDigits == NULL)
		return LINEERR_INVALPOINTER;

    LPCTSTR szCRLF = _T("\n");
	if (lpszValidChars == NULL)
		lpszValidChars = _T("0123456789ABCD*#!WPT@$+,");

    // Move the buffer to another string so we can modify it as we go.
    TString strNumber(lpszDigits);

    // If the prompt is in the string, this is an error and the
    // application has sent us a bad dial string.
    if (strNumber.find(_T('?')) != TString::npos)
        return LINEERR_DIALPROMPT;
    
    // If more than one address is listed, and we don't support multiplexing
    // on this line device, then error it.
    if (strNumber.find(szCRLF) != TString::npos)
    {
        if (pLine != NULL &&
            (pLine->GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_MULTIPLEADDR) == 0)
        {
            _TSP_DTRACE(_T("Multiple addresses listed in dialable address, ignoring all but first.\n"));
            strNumber = strNumber.substr (0, strNumber.find(szCRLF));
        }
    }                
    
    // Final result code
    LONG lResult = 0;
    
    // Now go through the string breaking each up into a seperate dial string (if
    // more than one is there).
    while (!strNumber.empty())
    {   
        TString strBuff, strSubAddress, strName;
		TString::size_type iPos = strNumber.find(szCRLF);
        if (iPos != TString::npos)          
        {
            strBuff = strNumber.substr(0, iPos);
            strNumber = strNumber.substr(iPos+2);
        }
        else
        {
            strBuff = strNumber; 
            strNumber.erase();
        }
        
        // Break the number up into its component parts.  Check to see if an
        // ISDN subaddress is present.
        iPos = strBuff.find(_T('|'));
        if (iPos != TString::npos)
        {   
            strSubAddress = strBuff.substr(iPos+1);
			TString::size_type iEndPos = strSubAddress.find_first_of(_T("+|^"), 0);
            if (iEndPos != TString::npos)
                strSubAddress.resize(iEndPos);
        }
        
        // Now grab the NAME if present in the string.
        iPos = strBuff.find(_T('^'));
        if (iPos != TString::npos)
            strName = strBuff.substr(iPos+1);

        // Strip off all the ISDN/Name info
        iPos = strBuff.find_first_of(_T("|^"));
        if (iPos != TString::npos)
            strBuff.resize(iPos);
        
        // Uppercase the string and strip all the blanks off
		// the end of the string.
		CharUpperBuff(&strBuff[0], strBuff.length());
		iPos = strBuff.find_last_not_of(_T(' '));
		if (iPos != TString::npos)
			strBuff.resize(iPos+1);

        // Check to see if partial dialing is allowed.
        bool fPartialDial = false;
        if (*(strBuff.rbegin()) == _T(';'))
        {
            fPartialDial = true;
            strBuff.resize(strBuff.length()-1);
        }
        
        // Remove anything which we don't understand.  Typically, the app
        // will do this for us, but just in case, remove any dashes, parens,
        // etc. from a phonebook entry.
        TString strNewBuff, strValidChars(lpszValidChars);
        for (iPos = 0; iPos < strBuff.length(); iPos++)
        {
            if (strValidChars.find(strBuff[iPos]) != TString::npos)
                strNewBuff += strBuff[iPos];
        }
        strBuff = strNewBuff;
        
        // Check the address capabilities against specific entries in our
        // dial string.
        if (fPartialDial && pAddr &&
            (pAddr->GetAddressCaps()->dwAddrCapFlags & LINEADDRCAPFLAGS_PARTIALDIAL) == 0)
        {
            lResult = LINEERR_INVALPOINTER;
            break;
        }
        
        if (strBuff.find(_T('$')) != TString::npos && pLine &&
            (pLine->GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_DIALBILLING) == 0)
        {
            lResult = LINEERR_DIALBILLING;
            break;
        }
       
        if (strBuff.find(_T('@')) != TString::npos && pLine &&
            (pLine->GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_DIALQUIET) == 0)
        {
            lResult = LINEERR_DIALQUIET;
            break;
        }
        
        if (strBuff.find(_T('W')) != TString::npos && pLine &&
            (pLine->GetLineDevCaps()->dwDevCapFlags & LINEDEVCAPFLAGS_DIALDIALTONE) == 0)
        {
            lResult = LINEERR_DIALDIALTONE;
            break;
        }
     
        // Now store the information into a DIALINFO structure.
        DIALINFO* pDialInfo = new DIALINFO (fPartialDial, strBuff, strName, strSubAddress);
		try
		{
			parrEntries->push_back(pDialInfo);
		}
		catch (...)
		{
			delete pDialInfo;
			lResult = LINEERR_NOMEM;
			break;
		}
    }        

    // If it failed somewhere, then spin through and delete ALL the
    // dialinfo requests already broken out.
    if (lResult != 0)
    {
        for (TDialStringArray::iterator i = parrEntries->begin();
			 i != parrEntries->end(); ++i)
            delete (*i);  // delete the DIALINFO pointer
		parrEntries->erase(parrEntries->begin(), parrEntries->end());
    }
    return lResult;

}// CServiceProvider::CheckDialableNumber

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetDevice
//
// Returns the device information structure for the specified
// permanent provider id.
//
CTSPIDevice* CServiceProvider::GetDevice(DWORD dwPPid) const
{
    int iCount = GetDeviceCount();
    for (int i = 0; i < iCount; i++)
    {
        CTSPIDevice* pDevice = GetDeviceByIndex(i);
        if (pDevice->GetProviderID() == dwPPid)
            return pDevice;
    }
    return NULL;
   
}// CServiceProvider::GetDevice

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ProcessCallParameters
//
// Check the parameters in a LINECALLPARAMS structure and verify that
// the fields are capable of being handled on the line/address/call.
//
// It is guarenteed that the line will be valid, the address and call can
// be NULL, but if a call is present, an address will always be present.
//  
// This function changed significantly in v1.21.  In previous releases
// it was required that this function was supplied by the derived class.
// It now passes control down to the specific objects in question which
// can perform most of the validation based on how the derived class sets up
// the LINECAPS structures.  It still may be overriden, but it is no
// longer necessary.
// 
LONG CServiceProvider::ProcessCallParameters(CTSPILineConnection* pLine, LPLINECALLPARAMS lpCallParams)
{                                                     
    // Fill in the default values if not supplied.
    if (lpCallParams == NULL)
        return LINEERR_INVALCALLPARAMS;
    
    // Set the defaults up if not supplied.  TAPI should be filling these
    // out, but the documentation is a bit vague as to who is really responsible
    // so we will just make sure that our values are ALWAYS valid.    
    if (lpCallParams->dwBearerMode == 0)
        lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
    if (lpCallParams->dwMaxRate == 0)
        lpCallParams->dwMaxRate = pLine->GetLineDevCaps()->dwMaxRate;
    if (lpCallParams->dwMediaMode == 0)
        lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
    if (lpCallParams->dwAddressMode == 0)
        lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
    
    // Make sure the DIAL parameters are all filled in.    
    if (lpCallParams->DialParams.dwDialPause == 0)
        lpCallParams->DialParams.dwDialPause = pLine->GetLineDevCaps()->DefaultDialParams.dwDialPause;
    if (lpCallParams->DialParams.dwDialSpeed == 0)
        lpCallParams->DialParams.dwDialSpeed = pLine->GetLineDevCaps()->DefaultDialParams.dwDialSpeed;
    if (lpCallParams->DialParams.dwDigitDuration == 0)
        lpCallParams->DialParams.dwDigitDuration = pLine->GetLineDevCaps()->DefaultDialParams.dwDigitDuration;
    if (lpCallParams->DialParams.dwWaitForDialtone == 0)
        lpCallParams->DialParams.dwWaitForDialtone = pLine->GetLineDevCaps()->DefaultDialParams.dwWaitForDialtone;
    
    // Pass it to the line object - it will determine if the address specified
    // or any address can support the call.
    return pLine->CanSupportCall (lpCallParams);

}// CServiceProvider::ProcessCallParameters

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::DetermineProviderCapabilities
//
// Determine what the service provider can and cannot do based on 
// what is exported from the TSP.  These are later used to provide
// the basic "CanHandleRequest" function.
//
void CServiceProvider::DetermineProviderCapabilities()
{           
	USES_CONVERSION;
	HINSTANCE hInst = GetResourceInstance();
    for (int i = 0; i <= TSPI_ENDOFLIST; i++)
    {
        if (GetProcAddress(hInst, gszEntryPoints[i]))
		{
			_TSP_DTRACEX(TRC_MIN, _T("Supports %s\r\n"), A2T(const_cast<LPSTR>(gszEntryPoints[i])));
            m_arrProviderCaps.set(i, true);
		}
    }

}// CServiceProvider::DetermineProviderCapabilities

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::CanHandleRequest
//
// This function is used to dynamically determine what the capabilities
// of our service provider really is.
//
bool CServiceProvider::CanHandleRequest(DWORD dwAPIFunction) const
{   
	_TSP_ASSERT(dwAPIFunction <= TSPI_ENDOFLIST);
	return (dwAPIFunction <= TSPI_ENDOFLIST) ?
		(m_arrProviderCaps[dwAPIFunction] == true) : false;

}// CServiceProvider::CanHandleRequest

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::MatchTones
//
// This function matches a series of tone frequencies against each other
// and determines if they are equal.  It is provided to allow for "fuzzy"
// searches rather than exact frequency matches.
//
bool CServiceProvider::MatchTones (
DWORD dwSFreq1,             // Search frequency 1 (what we are looking for)
DWORD dwSFreq2,             // Search frequency 2 (what we are looking for)
DWORD dwSFreq3,             // Search frequency 3 (what we are looking for)
DWORD dwTFreq1,             // Target frequency 1 (what we found)
DWORD dwTFreq2,             // Target frequency 2 (what we found)
DWORD dwTFreq3)             // Target frequency 3 (what we found)
{                               
    // The default is to to direct matching (exact) against any of the three frequency 
    // components.  If you require a filter or some "fuzzy" testing of the tones, then
    // override this function.
    return ((dwSFreq1 == dwTFreq1 || dwSFreq1 == dwTFreq2 || dwSFreq1 == dwTFreq3) &&
			(dwSFreq2 == dwTFreq1 || dwSFreq2 == dwTFreq2 || dwSFreq2 == dwTFreq3) &&
			(dwSFreq3 == dwTFreq1 || dwSFreq3 == dwTFreq2 || dwSFreq3 == dwTFreq3)) ?
			true : false;
   
}// CServiceProvider::MatchTones

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerInit
//
// This method is called when the service provider is first initialized.
// It supplies the base line/phone ids for us and our permanent provider
// id which has been assigned by TAPI.  It will be called right after
// the INITIALIZE_NEGOTIATION.
//
LONG CServiceProvider::providerInit(
DWORD dwTSPVersion,              // Version required for TAPI.DLL
DWORD dwProviderId,              // Our permanent provider Id.
DWORD dwLineBase,                // Our device line base id.
DWORD dwPhoneBase,               // Our device phone base id.
DWORD_PTR dwNumLines,            // Number of lines TAPI expects us to run
DWORD_PTR dwNumPhones,           // Number of phones TAPI expects us to run
ASYNC_COMPLETION lpfnCallback,   // Asynchronous completion callback.
LPDWORD lpdwTSPIOptions)         // TSPI options
{
    // Make sure we don't already have this provider in our
    // device array.
    if (GetDevice(dwProviderId) != NULL)
        return TAPIERR_DEVICEINUSE;

    // The library is fully re-entrant, the derived service provider
    // should set the re-entrancy flag if it is not re-entrant.
    *lpdwTSPIOptions = 0L;

    // Allocate a device information object for this id and add it to
    // our device map.  This device object maintains the connection lists
    // and line information.
    CTSPIDevice* pDevice = CreateDeviceObject();
    if (!pDevice->Init(dwProviderId, dwLineBase, dwPhoneBase, dwNumLines, dwNumPhones, m_hProvider, lpfnCallback))
	{
		// Failed, delete the device object
		pDevice->DecRef();
		return LINEERR_NODEVICE;
	}

	// Reset the shutdown event.
    ResetEvent(m_hevtShutdown);

    // Add the device to the list.
	CEnterCriticalSection csLock(&m_csProvider);
    m_arrDevices.push_back(pDevice); // can throw
	csLock.Unlock();

	// If we now have a single device, then record the version of TAPI found and
	// start our internal timer thread. Note - also check to see if the timer is
	// NULL just in case we went through a shutdown/init too quickly to physically
	// unload from memory. (v3.01)
	if (m_arrDevices.size() == 1 || m_htLibraryTimer == NULL)
    {
        m_dwTAPIVersionFound = dwTSPVersion;
		// Start our library interval thread which performs cleanup on calls and
		// media/tone/digit monitoring timeouts.
		UINT uiThread;
		m_htLibraryTimer = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, tsplib_LibraryTimerThread, static_cast<void*>(this), 0, &uiThread));
		_TSP_ASSERTE(m_htLibraryTimer != NULL);
    }
    return false;

}// CServiceProvider::providerInit   

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerShutdown
//
// This method is called to shutdown our service provider.  It will
// be called directly before the unload of our driver.  This should
// only be called when ALL line/phone devices supported by this service provider
// are to be shutdown.
//
LONG CServiceProvider::providerShutdown(DWORD /*dwTSPVersion*/, CTSPIDevice* pShutdownDevice)
{   
    // Locate the device in our array and remove it.
	CEnterCriticalSection csLock(&m_csProvider);
	TDeviceArray::iterator pos = std::find(m_arrDevices.begin(), m_arrDevices.end(), pShutdownDevice);
	if (pos != m_arrDevices.end())
		m_arrDevices.erase(pos);
#ifdef _DEBUG
	else
        _TSP_DTRACE(_T("ERR: Could not locate Device 0x%lx in provider list -- multiple calls to TSPI_providerShutdown?\n"), pShutdownDevice->GetProviderID());
#endif

	// Determine whether this was the final device that this provider was handling -- 
	// this will almost _always_ be true.
	bool fFinalExit = m_arrDevices.empty();
	csLock.Unlock();

	// If this is the final provider in the list then tell all global threads
	// that the DLL is about to be unloaded from memory.
	if (fFinalExit)		
	{
		_TSP_DTRACE(_T("Performing final exit for the provider\n"));
		SetEvent(m_hevtShutdown);
		if (m_htLibraryTimer != NULL)
		{
			// Wait for the timer thread so we don't unload prematurely.
			WaitForSingleObject(m_htLibraryTimer, INFINITE);
			CloseHandle(m_htLibraryTimer);
			m_htLibraryTimer = NULL;
		}
	}

	// Cache a copy of the device "gone" event.  Since the object might have
	// no outstanding locks, the following DecRef could delete it here.
	// Or, the delete might occur on some other thread because the object
	// was being accessed right now.
	HANDLE hEvent = pShutdownDevice->m_hevtDeviceDeleted;

    // Decrement our reference count on the device. When it hits zero,
	// it will be deleted from memory.  This may or may not happen now.
    pShutdownDevice->DecRef();

	// Wait for the device to get physically deleted.  In most cases, this
	// will not ever block.  However, if the provider was actively servicing
	// requests, then the device may not get deleted immediately (i.e. the 
	// ref count on the device > 1).  This ensures that we wait for all threads
	// currently running within the provider on this device.
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);

	// Now do final cleanup necessary for shutdown of the entire provider after
	// almost all threads and requests are complete.
	if (fFinalExit)
	{
		// Delete the function request pointers from our map -- the destructor for the
		// global object will do this too, but then it is reported as a memory leak by
		// the CRT (even though it really is free'd, just after the diagnostic report).
		CTSPILineConnection::g_mapRequests.clear();
		CTSPIPhoneConnection::g_mapRequests.clear();
	}

	// Return success
    return 0;

}// CServiceProvider::providerShutdown

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerCreateLineDevice
//
// This function is called by TAPI in response to a LINE_CREATE message
// being sent from us.  This allows TAPI to assign the line device id.
//
// This function is specific to TAPI version 1.4
//
LONG CServiceProvider::providerCreateLineDevice(
DWORD_PTR dwTempId,                 // Specifies the line device ID used in our LINE_CREATE
DWORD dwDeviceId)                   // Specifies TAPIs new line device id.
{                    
    // Locate the line device 
    CTSPILineConnection* pLine = GetConnInfoFromLineDeviceID(static_cast<DWORD>(dwTempId));
    if (pLine)
    {
        // Assign the NEW device id.
        pLine->SetDeviceID(dwDeviceId);
        return false;
    }

    // Couldn't find the device id in our table?
    return LINEERR_BADDEVICEID;

}// CServiceProvider::providerCreateLineDevice

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerCreatePhoneDevice
//
// This function is called by TAPI in response to a PHONE_CREATE message
// being sent from us.  This allows TAPI to assign the phone device id.
//
// This function is specific to TAPI version 1.4
//
LONG CServiceProvider::providerCreatePhoneDevice(
DWORD_PTR dwTempId,                 // Specifies the phone device ID used in our PHONE_CREATE
DWORD dwDeviceId)                   // Specifies TAPIs new phone device id.
{
    // Locate the phone device 
    CTSPIPhoneConnection* pPhone = GetConnInfoFromPhoneDeviceID(static_cast<DWORD>(dwTempId));
    if (pPhone)
    {
        // Assign the NEW device id.
        pPhone->SetDeviceID(dwDeviceId);
        return false;
    }

    // Couldn't find the device id in our table?
    return PHONEERR_BADDEVICEID;

}// CServiceProvider::providerCreatePhoneDevice

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerFreeDialogInstance
//
// Informs the provider about a dialog instance terminating.
//
LONG CServiceProvider::providerFreeDialogInstance (HDRVDIALOGINSTANCE hdDlgInstance)
{
    // Cast the handle back to our LINEUIDIALOG structure.
    LINEUIDIALOG* pLineDialog = (LINEUIDIALOG*)hdDlgInstance;
    _TSP_ASSERTE(pLineDialog != NULL);

    // Ask the line connection to free the specific dialog instance.
    CTSPILineConnection* pLine = pLineDialog->pLineOwner;
    return pLine->FreeDialogInstance(pLineDialog->htDlgInstance);

}// CServiceProvider::providerFreeDialogInstance

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerGenericDialogData 
//
// This is called when the UI DLL sends data back to our 
// provider.
//
LONG CServiceProvider::providerGenericDialogData (
CTSPIDevice* pDevice, 
CTSPILineConnection* pLine, 
CTSPIPhoneConnection* pPhone, 
HDRVDIALOGINSTANCE hdDlgInstance, 
LPVOID lpBuff, 
DWORD dwSize)
{
    LINEUIDIALOG* pLineDialog = (LINEUIDIALOG*)hdDlgInstance;

    // Notify the appropriate object
    if (pPhone)
        return pPhone->GenericDialogData (lpBuff, dwSize);
    else if (pLine)
        return pLine->GenericDialogData ((pLineDialog != NULL) ? pLineDialog->lpvItemData : NULL, lpBuff, dwSize);
    else if (pDevice)
        return pDevice->GenericDialogData (lpBuff, dwSize);

    // Only one left is the dialog instance.
    _TSP_ASSERTE(pLineDialog != NULL);
    return pLineDialog->pLineOwner->GenericDialogData(pLineDialog->lpvItemData, lpBuff, dwSize);
    
}// CServiceProvider::providerGenericDialogData 

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerEnumDevices
//
// TAPI calls this function before providerInit to determine the number
// of line and phone devices supported by the service provider.  This
// allows the service provider to read a configuration OTHER than
// the TELEPHON.INI to gather line/phone counts.  This is especially
// important in devices which support Plug&Play.
//
// THIS IS A MANDATORY FUNCTION IN TAPI 2.0
//
LONG CServiceProvider::providerEnumDevices(
DWORD dwProviderId,					// Our Provider ID
LPDWORD lpNumLines,                 // Number of lines (return)
LPDWORD lpNumPhones,                // Number of phones (return)    
HPROVIDER hProvider,                // TAPIs HANDLE to our service provider
LINEEVENT lpfnLineCreateProc,       // LINEEVENT for dynamic line creation
PHONEEVENT lpfnPhoneCreateProc)     // PHONEEVENT for dynamic phone creation
{   
    // Store off the line/phone event procedures.
    m_lpfnLineCreateProc = lpfnLineCreateProc;
    m_lpfnPhoneCreateProc = lpfnPhoneCreateProc;
    m_hProvider = hProvider;

    // Override to return a correct count for lines/phones.
    *lpNumLines = 0L;
    *lpNumPhones = 0L;

	// See if we are using the new v3.0 extensions to serialize our
	// devices to the registry.  If so, the derived service provider does
	// NOT need to override this function.
	ReadDeviceCount(dwProviderId, lpNumLines, lpNumPhones);

    return false;

}// CServiceProvider::providerEnumDevices

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::providerUIIdentify 
//
// Return the name passed in the constructor for this TSP.
//
LONG CServiceProvider::providerUIIdentify (LPWSTR lpszUIDLLName)
{
#ifdef _UNICODE
    lstrcpy(lpszUIDLLName, GetUIManager());
#else
    MultiByteToWideChar (CP_ACP, 0, GetUIManager(), -1, lpszUIDLLName, _MAX_PATH);
#endif
    return false;

}// CServiceProvider::providerUIIdentify 

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ReadProfileString
//
// Read a string from our profile section in the registry.  This
// function is limited to 512 characters.
//
TString CServiceProvider::ReadProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR lpszDefault/*=""*/)
{
	TCHAR szBuff[512];

    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
        return lpszDefault;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
        return lpszDefault;
	}

	// Open our device section
    DWORD dwDataSize = sizeof(szBuff), dwDataType;
	if (dwDeviceID > 0)
	{
		wsprintf(szBuff, gszDevice, dwDeviceID);
		HKEY hDeviceKey;
		if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return lpszDefault;
		}

		// Query the value requested.
		if (RegQueryValueEx(hDeviceKey, pszEntry, 0, &dwDataType, 
					reinterpret_cast<LPBYTE>(szBuff), &dwDataSize) == ERROR_SUCCESS &&
			(dwDataType == REG_MULTI_SZ || dwDataType == REG_SZ))
		{
			lpszDefault = szBuff;
		}
		RegCloseKey (hDeviceKey);
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);

	}
	else
	{
		// Query the value requested.
		if (RegQueryValueEx (hProviderKey, pszEntry, 0, &dwDataType, 
					reinterpret_cast<LPBYTE>(szBuff), &dwDataSize) == ERROR_SUCCESS &&
			(dwDataType == REG_MULTI_SZ || dwDataType == REG_SZ))
			lpszDefault = szBuff;
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}

    return lpszDefault;

}// CServiceProvider::ReadProfileString

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ReadProfileDWord
//
// Read a DWORD from our profile section in the registry.
//
DWORD CServiceProvider::ReadProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwDefault/*=0*/)
{
    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return dwDefault;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx(hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return dwDefault;
	}

	// Open our device section
	DWORD dwDataSize = sizeof(DWORD), dwDataType, dwData;
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);
		HKEY hDeviceKey;
		if (RegOpenKeyEx(hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return dwDefault;
		}

		// Query the value requested.
		if (RegQueryValueEx(hDeviceKey, pszEntry, 0, &dwDataType, 
				reinterpret_cast<LPBYTE>(&dwData), &dwDataSize) != ERROR_SUCCESS ||
				(dwDataType != REG_DWORD) || (dwDataSize != sizeof(DWORD)))
			dwData = dwDefault;

		RegCloseKey (hDeviceKey);
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}
	else
	{
		// Query the value requested.
		if (RegQueryValueEx(hProviderKey, pszEntry, 0, &dwDataType, 
				reinterpret_cast<LPBYTE>(&dwData), &dwDataSize) != ERROR_SUCCESS ||
				(dwDataType != REG_DWORD) || (dwDataSize != sizeof(DWORD)))
			dwData = dwDefault;
		RegCloseKey (hProviderKey);
		RegCloseKey (hTelephonyKey);
	}

    return dwData;

}// CServiceProvider::ReadProfileDWord

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::WriteProfileString
//
// Write a string into our registry profile.
//
bool CServiceProvider::WriteProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszValue)
{
    DWORD dwDisposition;

    // Attempt to create the telephony registry section - it should really exist if our
    // driver has been loaded by TAPI.
    HKEY hKeyTelephony;
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, _T(""), REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS, NULL, &hKeyTelephony, &dwDisposition) != ERROR_SUCCESS)
        return false;

    // Now create our provider section if necessary.
    HKEY hKeyProvider;
    if (RegCreateKeyEx (hKeyTelephony, m_pszProviderInfo, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                NULL, &hKeyProvider, &dwDisposition) != ERROR_SUCCESS)
	{
		RegCloseKey (hKeyTelephony);
        return false;
	}

	// Create our device section
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);
		HKEY hDeviceKey;
		if (RegCreateKeyEx (hKeyProvider, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			RegCloseKey (hKeyProvider);
			RegCloseKey (hKeyTelephony);
			return false;
		}

		// Store the key.
		RegSetValueEx (hDeviceKey, pszEntry, 0, REG_SZ, reinterpret_cast<CONST BYTE *>(pszValue), (lstrlen(pszValue)+1) * sizeof(TCHAR));
		RegCloseKey (hDeviceKey);
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);

	}
	else
	{
		RegSetValueEx (hKeyProvider, pszEntry, 0, REG_SZ, reinterpret_cast<CONST BYTE *>(pszValue), (lstrlen(pszValue)+1) * sizeof(TCHAR));
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}

	return true;

}// CServiceProvider::WriteProfileString

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::WriteProfileDWord
//
// Write a DWORD into our registry profile.
//
bool CServiceProvider::WriteProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwValue)
{
    DWORD dwDisposition;

    // Attempt to create the telephony registry section - it should really exist if our
    // driver has been loaded by TAPI.
    HKEY hKeyTelephony;
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, _T(""), REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS, NULL, &hKeyTelephony, &dwDisposition) != ERROR_SUCCESS)
        return false;

    // Now create our provider section if necessary.
    HKEY hKeyProvider;
    if (RegCreateKeyEx (hKeyTelephony, m_pszProviderInfo, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                        NULL, &hKeyProvider, &dwDisposition) != ERROR_SUCCESS)
	{
		RegCloseKey (hKeyTelephony);
        return false;
	}

	// Create our device section
	if (dwDeviceID > 0)
	{
		TCHAR szBuff[20];
		wsprintf(szBuff, gszDevice, dwDeviceID);
		HKEY hDeviceKey;
		if (RegCreateKeyEx (hKeyProvider, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			RegCloseKey (hKeyProvider);
			RegCloseKey (hKeyTelephony);
			return false;
		}

		// Store the key.
		RegSetValueEx (hDeviceKey, pszEntry, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&dwValue), sizeof(DWORD));
		RegCloseKey (hDeviceKey);
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}
	else
	{
		// Store the key.
		RegSetValueEx (hKeyProvider, pszEntry, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&dwValue), sizeof(DWORD));
		RegCloseKey (hKeyProvider);
		RegCloseKey (hKeyTelephony);
	}

	return true;

}// CServiceProvider::WriteProfileDWord

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::DeleteProfile
//
// Deletes the registry key directory for a section.
//
bool CServiceProvider::DeleteProfile (DWORD dwDeviceID, LPCTSTR pszKey/*=NULL*/)
{
    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return false;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return false;
	}

	// Delete the device section or entry.
	TCHAR szBuff[20];
	wsprintf(szBuff, gszDevice, dwDeviceID);
	bool rc = false;
	if (pszKey == NULL)
	{
		rc = IntRegDeleteKey (hProviderKey, szBuff);
	}
	else
	{
		if (dwDeviceID > 0)
		{
			HKEY hDeviceKey;
			if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hDeviceKey) == ERROR_SUCCESS)
			{
				rc = (RegDeleteKey (hDeviceKey, pszKey) == ERROR_SUCCESS);
				RegCloseKey (hDeviceKey);
			}
		}
		else
			rc = (RegDeleteKey (hProviderKey, pszKey) == ERROR_SUCCESS);
	}

    RegCloseKey (hProviderKey);
	RegDeleteKey(hTelephonyKey, m_pszProviderInfo);
    RegCloseKey (hTelephonyKey);

	return rc;

}// CServiceProvider::DeleteProfile

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::RenameProfile
//
// Moves a profile from one area to another.
//
bool CServiceProvider::RenameProfile (DWORD dwOldDevice, DWORD dwNewDevice)
{
	// Ignore requests for the same profile.
	if (dwOldDevice == dwNewDevice)
		return true;

	// First make sure the NEW device doesn't exist already
	if (dwNewDevice > 0 && !DeleteProfile(dwNewDevice))
		return false;

    // Open the master registry key.
    HKEY hTelephonyKey;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszTelephonyKey, 0, KEY_ALL_ACCESS, &hTelephonyKey) != ERROR_SUCCESS)
		return false;

    // Open our Provider section
    HKEY hProviderKey;
    if (RegOpenKeyEx (hTelephonyKey, m_pszProviderInfo, 0, KEY_ALL_ACCESS, &hProviderKey) != ERROR_SUCCESS)
	{
		RegCloseKey (hTelephonyKey);
		return false;
	}

	// Open our device section.
	HKEY hOldDeviceKey;
	TCHAR szBuff[20];
	if (dwOldDevice > 0)
	{
		wsprintf(szBuff, gszDevice, dwOldDevice);
		if (RegOpenKeyEx (hProviderKey, szBuff, 0, KEY_ALL_ACCESS, &hOldDeviceKey) != ERROR_SUCCESS)
		{
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return false;
		}
	}
	else
		hOldDeviceKey = hProviderKey;

	// Create the new section
	HKEY hNewDeviceKey;
	if (dwNewDevice > 0)
	{
		DWORD dwDisposition;
		wsprintf(szBuff, gszDevice, dwNewDevice);
		if (RegCreateKeyEx (hProviderKey, szBuff, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hNewDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		{
			if (dwOldDevice > 0)
				RegCloseKey(hOldDeviceKey);
			RegCloseKey (hProviderKey);
			RegCloseKey (hTelephonyKey);
			return false;
		}
	}
	else
		hNewDeviceKey = hProviderKey;

	// Get the max size of the name and values.
	DWORD dwNameSize, dwValueSize, dwType;
	if (RegQueryInfoKey (hOldDeviceKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		                 &dwNameSize, &dwValueSize, NULL, NULL) != ERROR_SUCCESS)
	{
		dwNameSize = 1024;
		dwValueSize = 4096;
	}
	else
	{
		++dwNameSize;
		++dwValueSize;
	}

	// Alloc blocks to hold the information.
	LPTSTR pszName = new TCHAR[dwNameSize];
	LPBYTE pszValue = new BYTE[dwValueSize * sizeof(TCHAR)];

	// Enumerate through all the values within the old key, and move them to the new section.
	DWORD dwIndex = 0;
	for ( ;; )
	{
		// Enumerate through the items.
		DWORD dwNewNameSize = dwNameSize, dwNewValueSize = dwValueSize;
		if (RegEnumValue (hOldDeviceKey, dwIndex++, pszName, &dwNewNameSize, NULL,
									   &dwType, pszValue, &dwNewValueSize) != ERROR_SUCCESS)
			break;

		// Delete the value.
		RegDeleteValue (hOldDeviceKey, pszName);

		// Create the key in our new subkey.
		RegSetValueEx (hNewDeviceKey, pszName, 0, dwType, pszValue, dwNewValueSize);
	}

	// We're done with the memory.
	delete [] pszName;
	delete [] pszValue;

	// Close all the used keys.
	if (dwNewDevice > 0)
		RegCloseKey(hNewDeviceKey);
	if (dwOldDevice > 0)
		RegCloseKey(hOldDeviceKey);
	RegCloseKey (hProviderKey);
	RegCloseKey (hTelephonyKey);

	// Now delete the original section
	if (dwOldDevice > 0)
		DeleteProfile(dwOldDevice);

	return true;

}// CServiceProvider::RenameProfile

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::AddDeviceClassInfo
//
// Add a device class info structure to an array
//
int CServiceProvider::AddDeviceClassInfo (TDeviceClassArray& arrElem, 
				LPCTSTR pszName, DWORD dwType, LPVOID lpBuff, 
				DWORD dwSize, HANDLE hHandle)
{
	// Remove the entry if it exists.
	RemoveDeviceClassInfo(arrElem, pszName);
    DEVICECLASSINFO* pDevClass = new DEVICECLASSINFO(pszName, dwType, lpBuff, dwSize, hHandle);

	CEnterCriticalSection csLock(&m_csProvider);
	arrElem.insert(TDeviceClassArray::value_type(pszName,pDevClass));
	return 1;

}// CServiceProvider::AddDeviceClassInfo

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::RemoveDeviceClassInfo
//
// Remove a device class structure from our array
//
bool CServiceProvider::RemoveDeviceClassInfo(TDeviceClassArray& arrElem, LPCTSTR pszName)
{
	// Remove the given entry from our device class map.
	CEnterCriticalSection csLock(&m_csProvider);
    TDeviceClassArray::iterator i = arrElem.find(pszName);
	bool fFound = (i != arrElem.end());
	if (fFound)
	{
		DEVICECLASSINFO* pDevClass = (*i).second;
		arrElem.erase(i);
        delete pDevClass;
    }
    return fFound;

}// CServiceProvider::RemoveDeviceClassInfo

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::FindDeviceClassInfo
//
// Locate and return a device class structure from our array
//
DEVICECLASSINFO* CServiceProvider::FindDeviceClassInfo (TDeviceClassArray& arrElem, LPCTSTR pszName)
{
	CEnterCriticalSection csLock(&m_csProvider);
    TDeviceClassArray::iterator ii = arrElem.find(pszName);
	return (ii != arrElem.end()) ? (*ii).second : NULL;

}// CServiceProvider::FindDeviceClassInfo

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::CopyDeviceClass
//
// Copy a DEVICECLASSINFO structure into a VARSTRING
//
LONG CServiceProvider::CopyDeviceClass (DEVICECLASSINFO* pDeviceClass, 
                                        LPVARSTRING lpDeviceID, HANDLE hTargetProcess)
{
    // Copy the basic information into the VARSTRING.
    lpDeviceID->dwUsedSize = sizeof(VARSTRING);
    lpDeviceID->dwNeededSize = sizeof(VARSTRING) + pDeviceClass->dwSize;
    lpDeviceID->dwStringOffset = 0;
    lpDeviceID->dwStringSize = 0;

    if (pDeviceClass->hHandle != INVALID_HANDLE_VALUE)
        lpDeviceID->dwNeededSize += sizeof(DWORD);
    lpDeviceID->dwStringFormat = pDeviceClass->dwStringFormat;

    // If we have enough space, copy the handle and data into our buffer.
    if (lpDeviceID->dwTotalSize >= lpDeviceID->dwNeededSize)
    {
        // Copy the handle first.  It must be duplicated for the process which
        // needs it.
        if (pDeviceClass->hHandle != INVALID_HANDLE_VALUE)
        {
            HANDLE hTargetHandle;
            if (DuplicateHandle(GetCurrentProcess(), pDeviceClass->hHandle, 
                        hTargetProcess, &hTargetHandle, 0, false,
                        DUPLICATE_SAME_ACCESS))
            {
                AddDataBlock(lpDeviceID, lpDeviceID->dwStringOffset, lpDeviceID->dwStringSize,
                              &hTargetHandle, sizeof(HANDLE));
            }
        }

        // Now the buffer if present.
        if (pDeviceClass->dwSize > 0)
            AddDataBlock(lpDeviceID, lpDeviceID->dwStringOffset, lpDeviceID->dwStringSize,
                          pDeviceClass->lpvData.get(), pDeviceClass->dwSize);
    }
    return false;

}// CServiceProvider::CopyDeviceClass

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::IntRegDeleteKey
//
// Internal function used to delete a section of the registry under Windows NT
// where we must delete each branch seperately.
//
bool CServiceProvider::IntRegDeleteKey (HKEY hKeyTelephony, LPCTSTR pszMainDir)
{
	// Attempt to delete the key directly. Under Win95, this will also delete
	// any branches under it.
    if (RegDeleteKey(hKeyTelephony, pszMainDir) != ERROR_SUCCESS)
	{
		// Open the top-level key.
		HKEY hKey;
		DWORD dwRC = RegOpenKeyEx(hKeyTelephony, pszMainDir, 0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey);
		if (dwRC == ERROR_SUCCESS)
		{
			DWORD dwReqSize = _MAX_PATH;
			LPTSTR pszName = new TCHAR[_MAX_PATH];
			if (pszName == NULL)
			{
				RegCloseKey(hKey);
				dwRC = ERROR_NOT_ENOUGH_MEMORY;
			}

			while (dwRC == ERROR_SUCCESS)
			{
				dwReqSize = _MAX_PATH;
				dwRC = RegEnumKeyEx(hKey, 0, pszName, &dwReqSize, NULL, NULL, NULL, NULL);
				if (dwRC == ERROR_NO_MORE_ITEMS)
				{
				   dwRC = RegDeleteKey(hKeyTelephony, pszMainDir);
				   break;
				}
				else if (dwRC == ERROR_SUCCESS)
				   IntRegDeleteKey(hKey, pszName);
			}

			RegCloseKey(hKey);
			delete [] pszName;
		}
	}
	return true;

}// CServiceProvider::IntRegDeleteKey

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::TraceOut
//
// This method is called during DEBUG builds to output the TRACE
// facility.
//
void CServiceProvider::TraceOut(const TString& /*strOut*/)
{
	/* Do nothing */
	
}// CServiceProvider::TraceOut

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::DeleteCall
//
// This method is called to delete a call appearance.
//
void CServiceProvider::DeleteCall(CTSPICallAppearance* pCall)
{
	// Output some trace information.  The call SHOULD be deleted
	// at this point - presumably by a lineCloseCall.
	_TSP_DTRACEX(TRC_CALLS, _T("Marking call <0x%lx> as deleted\n"), pCall);
	_TSP_ASSERTE(pCall->HasBeenDeleted());

	// Get uninterrupted access to the deleted array
	CEnterCriticalSection csLock(&m_csProvider);

	// Set the timeout to delete the call.
	pCall->m_lDeleteTime = GetTickCount();
	m_lstDeletedCalls.push_back(pCall); // can throw
 
}// CServiceProvider::DeleteCall

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::AddTimedCall
//
// This method adds a call to our "need timer" list.
//
void CServiceProvider::AddTimedCall(CTSPICallAppearance* pCall)
{
	CEnterCriticalSection csLock(&m_csProvider);
	TCallList::iterator pos = std::find(m_lstTimedCalls.begin(), m_lstTimedCalls.end(), pCall);
	if (pos == m_lstTimedCalls.end())
		m_lstTimedCalls.push_back(pCall);

}// CServiceProvider::AddTimedCall

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::RemoveTimedCall
//
// This method removes a call from our "need timer" list.
//
void CServiceProvider::RemoveTimedCall(CTSPICallAppearance* pCall)
{
	CEnterCriticalSection csLock(&m_csProvider);
	TCallList::iterator pos = std::find(m_lstTimedCalls.begin(), m_lstTimedCalls.end(), pCall);
	if (pos != m_lstTimedCalls.end())
		m_lstTimedCalls.erase(pos);

}// CServiceProvider::RemoveTimedCall
                     
////////////////////////////////////////////////////////////////////////////
// CServiceProvider::DialInputToArray
//
// This converts a dialable number input from TAPI into a TDialStringArray
// object which is manipulated in the class library
//
LONG CServiceProvider::DialInputToArray(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, 
										LPCTSTR lpszDestAddr, DWORD dwCountryCode, TDialStringArray* pArray)
{
    // If the destination address is not there, then error out.
    if (lpszDestAddr == NULL || *lpszDestAddr == _T('\0'))
        return LINEERR_INVALADDRESS;

    // Verify the destination address and move it into another buffer.
    return CheckDialableNumber(pLine, pAddr, lpszDestAddr, pArray, dwCountryCode);

}// CServiceProvider::DialInputToArray

/////////////////////////////////////////////////////////////////////////
// CServiceProvider::lineNegotiateTSPIVersion
//
// This method is called when TAPI wishes to negotiate available
// versions with us for any line device installed.  The low and high
// version numbers passed are ones which the installed TAPI.DLL supports,
// and we are expected to return the highest value which we can support
// so TAPI knows what type of calls to make to us.
// 
// This method may be called more than once during the life of our
// SP.
//
LONG CServiceProvider::lineNegotiateTSPIVersion(
DWORD dwDeviceId,             // Valid line device -or- INITIALIZE_NEGOTIATION
DWORD dwLowVersion,           // Lowest TAPI version known
DWORD dwHiVersion,            // Highest TAPI version known
LPDWORD lpdwTSPVersion)       // Return version
{
	// Validate the parameter
	if (lpdwTSPVersion == NULL || IsBadWritePtr(lpdwTSPVersion, sizeof(DWORD)))
		return LINEERR_INVALPARAM;

    // If this is a specific line initialize request, then locate
    // it and make sure it belongs to us.
    if (dwDeviceId != INITIALIZE_NEGOTIATION)
    {   
    	CTSPILineConnection* pLine = GetConnInfoFromLineDeviceID(dwDeviceId);
    	if (pLine == NULL)
            return LINEERR_BADDEVICEID;
		return pLine->NegotiateVersion(dwLowVersion, dwHiVersion, lpdwTSPVersion);
    }
	    
    // Do a SERVICE PROVIDER negotiation.
    *lpdwTSPVersion = GetSupportedVersion();
    if (dwLowVersion > *lpdwTSPVersion) // The app is too new for us
        return LINEERR_INCOMPATIBLEAPIVERSION;

    // If the version supported is LESS than what we support,
    // then drop to the version it allows.  The library can handle
    // down to TAPI 1.3.
    if (dwHiVersion < *lpdwTSPVersion)
    {
        if (dwHiVersion < TAPIVER_13)
            return LINEERR_INCOMPATIBLEAPIVERSION;
        *lpdwTSPVersion = dwHiVersion;
    }

    // Everything looked Ok.
    return 0;

}// CServiceProvider::lineNegotiateTSPIVersion

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::lineForward
//
// This function forwards calls destined for the specified address on
// the specified line, according to the specified forwarding instructions.
// When an origination address is forwarded, the incoming calls for that
// address are deflected to the other number by the switch.  This function
// provides a combination of forward and do-not-disturb features.  This
// function can also cancel specific forwarding currently in effect.
//
LONG CServiceProvider::lineForward(
CTSPILineConnection* pLine,            // Line Connection (for All)
CTSPIAddressInfo* pAddr,               // Address to forward to (NULL=all)
DRV_REQUESTID dwRequestId,             // Asynchronous request id
LPLINEFORWARDLIST const lpForwardList, // Forwarding instructions
DWORD dwNumRingsAnswer,                // Rings before "no answer"
HTAPICALL htConsultCall,               // New TAPI call handle if necessary
LPHDRVCALL lphdConsultCall,            // Our return call handle if needed
LPLINECALLPARAMS const lpCallParamsIn) // Used if creating a new call
{
	// Initialize the input parameters.
    *lphdConsultCall = NULL;

    // Verify the call parameters if present.  We only use them if a single
	// address is forwarded (since the FWDCONSULT is only present on the 
	// address capabilities).  If all lines are being forwarded, ignore the
	// structure.
	LPLINECALLPARAMS lpCallParams = NULL;
    if (pAddr && lpCallParamsIn)
    {
        // Copy the call parameters into our own buffer so we keep a pointer
		// to it during the asynchronous processing of this call.
		lpCallParams = CopyCallParams(lpCallParamsIn);
        if (lpCallParams == NULL)
            return LINEERR_NOMEM;

        // Process the call parameters
        LONG lResult = ProcessCallParameters(pLine, lpCallParams);
        if (lResult)
        {
            FreeMem(lpCallParams);
            return lResult;          
        }
    }

	// Allocate an array to hold the forwarding information.
	TForwardInfoArray* parrForwardInfo = new TForwardInfoArray;
	LONG lResult;

    // Copy and validate the forwarding information.
    if (lpForwardList && lpForwardList->dwNumEntries > 0)
    {   
        // Validate the forward list passed.
        if (lpForwardList->dwTotalSize < sizeof (LINEFORWARDLIST))
        {
			delete parrForwardInfo;
            FreeMem(lpCallParams);
            return LINEERR_STRUCTURETOOSMALL;
        }
    
        // Allocate seperate line forward requests which we stick into a ptr
        // array.
        for (unsigned int iCount = 0; iCount < lpForwardList->dwNumEntries; iCount++)
        {   
            TSPIFORWARDINFO* lpForward = new TSPIFORWARDINFO;
            if (lpForward == NULL)
            {
				delete parrForwardInfo;
				FreeMem(lpCallParams);
                return LINEERR_NOMEM;
            }
            
            // Insert it into our array
			lpForward->IncUsage();
            parrForwardInfo->push_back(lpForward);
            
            // Get the passed forwarding information
            LPLINEFORWARD lpLineForward = &lpForwardList->ForwardList[iCount];

            // Copy it over - validate each of the numbers (if available)
            lpForward->dwForwardMode = lpLineForward->dwForwardMode;
            lpForward->dwDestCountry = lpLineForward->dwDestCountryCode;
            
            // First the caller address - it will always be offset from the OWNING structure
            // which in this case is the forward list.
            if (lpLineForward->dwCallerAddressSize > 0 && lpLineForward->dwCallerAddressOffset > 0)
            {
                wchar_t* lpszBuff = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpForwardList) + lpLineForward->dwDestAddressOffset);
#ifndef UNICODE
				TString strAddress = ConvertWideToAnsi(lpszBuff);
#else
				TString strAddress = lpszBuff;
#endif
                lResult = CheckDialableNumber (pLine, pAddr, strAddress.c_str(), &lpForward->arrCallerAddress, 0);
                if (lResult != 0)
                {   
					delete parrForwardInfo;
					FreeMem(lpCallParams);
                    return lResult;
                }
            }
            
            // Now the destination address
            if (lpLineForward->dwDestAddressSize > 0 && lpLineForward->dwDestAddressOffset > 0)
            {
                wchar_t* lpszBuff = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpForwardList) + lpLineForward->dwDestAddressOffset);
#ifndef UNICODE
				TString strAddress = ConvertWideToAnsi(lpszBuff);
#else
				TString strAddress = lpszBuff;
#endif
                lResult = CheckDialableNumber (pLine, pAddr, strAddress.c_str(), &lpForward->arrDestAddress, 0);
                if (lResult != 0)
                {   
					delete parrForwardInfo;
					FreeMem(lpCallParams);
                    return lResult;
                }
            }

			// Calculate the size used for this forwarding information object so
			// we don't have to do it everytime TAPI requests our configuration.
			// This size is what is needed in TAPI terms for the forwarding information.
			lpForward->dwTotalSize = sizeof(LINEFORWARD);
			unsigned int j;
            for (j = 0; j < lpForward->arrCallerAddress.size(); j++)
            {
                DIALINFO* pDialInfo = lpForward->arrCallerAddress[j];
                if (!pDialInfo->strNumber.empty())
                {
					lpForward->dwTotalSize += pDialInfo->strNumber.length() + sizeof(wchar_t);
					if (j > 0)
						lpForward->dwTotalSize += sizeof(wchar_t);
					while ((lpForward->dwTotalSize%4) != 0)
						++lpForward->dwTotalSize;
                }
            }

            for (j = 0; j < lpForward->arrDestAddress.size(); j++)
            {
                DIALINFO* pDialInfo = lpForward->arrDestAddress[j];
                if (!pDialInfo->strNumber.empty())
                {
					lpForward->dwTotalSize += pDialInfo->strNumber.length() + sizeof(wchar_t);
					if (j > 0)
						lpForward->dwTotalSize += sizeof(wchar_t);
					while ((lpForward->dwTotalSize%4) != 0)
						++lpForward->dwTotalSize;
                }
            }                
        }            
    }

    // Now tell the line to forward the address (or all of its addresses)
    lResult = pLine->Forward (dwRequestId, pAddr, parrForwardInfo, dwNumRingsAnswer, 
							htConsultCall, lphdConsultCall, lpCallParams);

	// If the request failed, deallocate our memory.
	if (lResult != static_cast<LONG>(dwRequestId) && lResult != 0)
	{
		*lphdConsultCall = NULL;
		delete parrForwardInfo;
		FreeMem(lpCallParams);
	}
	return lResult;

}// CServiceProvider::lineForward

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::lineSetCurrentLocation
//
// This function is called by TAPI whenever the address translation location
// is changed by the user (in the Dial Helper dialog or 
// 'lineSetCurrentLocation' function.  SPs which store parameters specific
// to a location (e.g. touch-tone sequences specific to invoke a particular
// PBX function) would use the location to select the set of parameters 
// applicable to the new location.
// 
LONG CServiceProvider::lineSetCurrentLocation(DWORD dwLocation)
{                
	if (m_pcurrLocation == NULL)
		return LINEERR_OPERATIONFAILED;

	// Only reload if the location is really changing!
	if (dwLocation != m_pcurrLocation->dwCurrentLocation)
	{
		m_pcurrLocation->dwCurrentLocation = dwLocation;
		m_pcurrLocation->Reload();
	}
    return 0L;
    
}// CServiceProvider::lineSetCurrentLocation

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::lineSetMediaControl
//
// This function enables and disables control actions on the media stream 
// associated with the specified line, address, or call. Media control 
// actions can be triggered by the detection of specified digits, media modes, 
// custom tones, and call states.  The new specified media controls replace 
// all the ones that were in effect for this line, address, or call prior 
// to this request.
//
LONG CServiceProvider::lineSetMediaControl(
CTSPILineConnection *pLine,                         // Line connection
CTSPIAddressInfo* pAddr,                            // Address (may be NULL)
CTSPICallAppearance* pCall,                         // Call (may be NULL)
LPLINEMEDIACONTROLDIGIT const lpDigitListIn,        // Digits to trigger actions
DWORD dwNumDigitEntries,                            // Count of digits
LPLINEMEDIACONTROLMEDIA const lpMediaListIn,        // Media modes to be monitored    
DWORD dwNumMediaEntries,                            // Count of media modes
LPLINEMEDIACONTROLTONE const lpToneListIn,          // Tones to be monitored    
DWORD dwNumToneEntries,                             // Count of tone
LPLINEMEDIACONTROLCALLSTATE const lpCallStateListIn, // Callstates to be monitored
DWORD dwNumCallStateEntries)                        // Count of call states
{   
	// Verify that we can execute this request right now.
    if ((pCall && (pCall->GetCallStatus()->dwCallFeatures & LINECALLFEATURE_SETMEDIACONTROL) == 0) ||
		(pAddr && (pAddr->GetAddressStatus()->dwAddressFeatures & LINEADDRFEATURE_SETMEDIACONTROL) == 0) ||
		(pLine->GetLineDevStatus()->dwLineFeatures & LINEFEATURE_SETMEDIACONTROL) == 0)
        return LINEERR_OPERATIONUNAVAIL;

    // Allocate a media control structure
    TSPIMEDIACONTROL* lpMediaControl = new TSPIMEDIACONTROL;
    if (lpMediaControl == NULL)
        return LINEERR_NOMEM;
    
    LPLINEMEDIACONTROLDIGIT lpDigitList = reinterpret_cast<LPLINEMEDIACONTROLDIGIT>(lpDigitListIn);
    LPLINEMEDIACONTROLMEDIA lpMediaList = reinterpret_cast<LPLINEMEDIACONTROLMEDIA>(lpMediaListIn);
    LPLINEMEDIACONTROLTONE lpToneList = reinterpret_cast<LPLINEMEDIACONTROLTONE>(lpToneListIn);
    LPLINEMEDIACONTROLCALLSTATE lpCallStateList = reinterpret_cast<LPLINEMEDIACONTROLCALLSTATE>(lpCallStateListIn);
        
    // Run through each array and copy over the values.
	unsigned int i;
    for (i = 0; i < dwNumDigitEntries; i++)
    {
        LPLINEMEDIACONTROLDIGIT lpDigit = new LINEMEDIACONTROLDIGIT;
        if (lpDigit == NULL)
        {
            delete lpMediaControl;
            return LINEERR_NOMEM;
        }                                                                
        
        MoveMemory (lpDigit, lpDigitList, sizeof(LINEMEDIACONTROLDIGIT));
        lpMediaControl->arrDigits.push_back(lpDigit);
        lpDigitList++;
    }    

    for (i = 0; i < dwNumMediaEntries; i++)
    {
        LPLINEMEDIACONTROLMEDIA lpMedia = new LINEMEDIACONTROLMEDIA;
        if (lpMedia == NULL)
        {
            delete lpMediaControl;
            return LINEERR_NOMEM;
        }                        
        
        MoveMemory (lpMedia, lpMediaList, sizeof (LINEMEDIACONTROLMEDIA));
        lpMediaControl->arrMedia.push_back(lpMedia);
        lpMediaList++;
    }
    
    for (i = 0; i < dwNumToneEntries; i++)
    {
        LPLINEMEDIACONTROLTONE lpTone = new LINEMEDIACONTROLTONE;
        if (lpTone == NULL)
        {
            delete lpMediaControl;
            return LINEERR_NOMEM;
        }
        
        MoveMemory (lpTone, lpToneList, sizeof(LINEMEDIACONTROLTONE));
        lpMediaControl->arrTones.push_back(lpTone);
        lpToneList++;
    }        
    
    for (i = 0; i < dwNumCallStateEntries; i++)
    {
        LPLINEMEDIACONTROLCALLSTATE lpCallState = new LINEMEDIACONTROLCALLSTATE;        
        if (lpCallState == NULL)
        {
            delete lpMediaControl;
            return LINEERR_NOMEM;
        }                        
        
        MoveMemory (lpCallState, lpCallStateList, sizeof(LINEMEDIACONTROLCALLSTATE));
        lpMediaControl->arrCallStates.push_back(lpCallState);
        lpCallStateList++;
    }
    
    // Validate the parameters in the media control list
    LONG lResult = pLine->ValidateMediaControlList (lpMediaControl);
    if (lResult != 0)
    {
        delete lpMediaControl;
        return lResult;
    }

	// Now create a request object to manage this SetMediaControl event
	// and pass it through our line.
	RTSetMediaControl* pRequest = new RTSetMediaControl(pLine, pAddr, pCall, lpMediaControl);
	if (pCall != NULL)
		lResult = pCall->AddAsynchRequest(pRequest);
	else if (pAddr != NULL)
		lResult = pAddr->AddAsynchRequest(pRequest);
	else
		lResult = pLine->AddAsynchRequest(pRequest);

	// If the add was successful, wait on the request.
	if (lResult == 0)
		lResult = pRequest->WaitForCompletion(INFINITE);
	return lResult;
   
}// CServiceProvider::lineSetMediaControl

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::lineDevSpecific
//
// Device specific extensions to the service provider.  This is handled
// here so that a common override can catch all device-specific requests
// no matter what object they are targeted for.
//
LONG CServiceProvider::lineDevSpecific(
CTSPILineConnection* pLine,                 // Line connection
CTSPIAddressInfo* pAddr,                    // Address on line    
CTSPICallAppearance* pCall,                 // Call appearance on address
DRV_REQUESTID dwRequestId,                  // Asynch. request id.
LPVOID lpParams,                            // Parameters (device specific)
DWORD dwSize)                               // Size of parameters
{
	LONG lResult = LINEERR_OPERATIONUNAVAIL;
	if (pCall != NULL)
		lResult = pCall->DevSpecific(dwRequestId, lpParams, dwSize);
	if (lResult == LINEERR_OPERATIONUNAVAIL)
	{
		if (pAddr != NULL)
			lResult = pAddr->DevSpecific(pCall, dwRequestId, lpParams, dwSize);
		if (lResult == LINEERR_OPERATIONUNAVAIL)
		{
			if (pLine != NULL)
				lResult = pLine->DevSpecific(pAddr, pCall, dwRequestId, lpParams, dwSize);
		}
	}

	return lResult;
    
}// CServiceProvider::lineDevSpecific
