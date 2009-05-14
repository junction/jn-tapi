/******************************************************************************/
//                                                                        
// MISC.CPP - Source file for the misc. class functions
//                                             
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//
// This file implements all the request object constructors and
// the various support classes required in the library which are
// considered internal to the spLIB++ product.
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::CTSPIBaseObject
//
// Destructor for the base TSP object
//
CTSPIBaseObject::~CTSPIBaseObject()
{
}// CTSPIBaseObject::~CTSPIBaseObject

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::DestroyObject
//
// This is called to destroy the object when all reference counts are zero.
//
void CTSPIBaseObject::DestroyObject()
{
	_TSP_ASSERTE(*((LPDWORD)this) != 0x00000000);	// Unintialized ptr
	_TSP_ASSERTE(*((LPDWORD)this) != 0xdddddddd);	// Already free'd ptr
	_TSP_ASSERTE(m_lRefCount < 0);
	delete this;

}// CTSPIBaseObject::DestroyObject

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::Dump
//
// Debug Dump function
//
TString CTSPIBaseObject::Dump() const
{
	return _T("");

}// CTSPIBaseObject::Dump
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEVICECLASSINFO::DEVICECLASSINFO
//
// Constructor for the device class information structure
//
DEVICECLASSINFO::DEVICECLASSINFO (LPCTSTR pszName, DWORD dwFormat, LPVOID lpData, DWORD dwDataSize, HANDLE htHandle) :
		strName(pszName), dwStringFormat(dwFormat), hHandle(htHandle), lpvData(NULL), dwSize(0)
{
	if (dwDataSize > 0)
	{
		USES_CONVERSION;

		// If the format is to be stored in ASCII then do any necessary conversion
		// from Unicode or store directly from the given string.
		if (dwFormat == STRINGFORMAT_ASCII)
		{
			LPSTR lpsData = T2A(static_cast<LPTSTR>(lpData));
			dwSize = lstrlenA(lpsData)+1;
			lpvData.reset(new BYTE[dwSize]);
			lstrcpyA(reinterpret_cast<LPSTR>(lpvData.get()), lpsData);
		}

		// Or.. if the format is in Unicode, then do any necessary conversion from
		// ASCII into the stored format.
		else if (dwFormat == STRINGFORMAT_UNICODE)
		{
			LPWSTR lpsData = T2W(static_cast<LPTSTR>(lpData));
			dwSize = (lstrlenW(lpsData)+1) * sizeof(wchar_t);
			lpvData.reset(new BYTE[dwSize]);
			lstrcpyW(reinterpret_cast<LPWSTR>(lpvData.get()), lpsData);
		}
		
		// Otherwise in binary form -- assume passed length is in bytes.
		else
		{
			dwSize = dwDataSize;
			lpvData.reset(new BYTE[dwSize]);
			MoveMemory(lpvData.get(), lpData, dwSize);
		}
	}

}// DEVICECLASSINFO::DEVICECLASSINFO

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEVICECLASSINFO::~DEVICECLASSINFO
//
// Destructor for the device class information structure
//
DEVICECLASSINFO::~DEVICECLASSINFO()
{
}// DEVICECLASSINFO::~DEVICECLASSINFO

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOCATIONINFO::Reload
//
// Loads location information from TAPIs location database.
//
bool LOCATIONINFO::Reload()
{
	USES_CONVERSION;

	// Reset our current strings
	Reset();

	// Load the TAPI32 dll and find the entrypoints we need. This will throw an
	// exception on failure.
	LLHelper TapiDLL;

	// Get the translation caps currently setup from our dialing location.
	std::auto_ptr<LINETRANSLATECAPS> lpCaps(TapiDLL.tapiGetTranslateCaps());
	if (lpCaps.get() == NULL)
		return false;

	// If the current location is zero, then reset to what TAPI says.
	if (dwCurrentLocation == 0)
		dwCurrentLocation = lpCaps->dwCurrentLocationID;

	// Walk through all the entry structures looking for our current location id.
	LINELOCATIONENTRY* lpEntry = reinterpret_cast<LPLINELOCATIONENTRY>(reinterpret_cast<LPBYTE>(lpCaps.get()) + lpCaps->dwLocationListOffset);
	for (unsigned int i = 0; i < lpCaps->dwNumLocations; i++)
	{
		// If this isn't the area we are looking for, ignore it.
		if (lpEntry->dwPermanentLocationID != dwCurrentLocation)
		{
			++lpEntry; 
			continue;
		}

		// Copy the country code
		char chBuff[10];
		_ltoa_s(lpEntry->dwCountryCode, chBuff, 10, 10);
		strCountryCode = A2T(chBuff);

		// Copy the area code
		if (lpEntry->dwCityCodeSize > 0)
			strAreaCode = A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpCaps.get()) + lpEntry->dwCityCodeOffset));

		// Copy the long-distance access code
		if (lpEntry->dwLongDistanceAccessCodeSize > 0)
			strLongDistanceAccess = GetSP()->GetDialableNumber(A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpCaps.get()) + lpEntry->dwLongDistanceAccessCodeOffset)));

		// Copy the local access code
		if (lpEntry->dwLocalAccessCodeSize > 0)
			strLocalAccess = GetSP()->GetDialableNumber(A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpCaps.get()) + lpEntry->dwLocalAccessCodeOffset)));

		// Copy the call-waiting sequence.
		if (lpEntry->dwCancelCallWaitingSize > 0)
			strCallWaiting = GetSP()->GetDialableNumber(A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpCaps.get()) + lpEntry->dwCancelCallWaitingOffset)), _T("#*"));

		// Now using the country id, retrieve the international dialing rules
		// for this country. This includes the prefix which must be dialed
		// in order to make an international call.
		DWORD dwCountryID = lpEntry->dwCountryID;
		std::auto_ptr<LINECOUNTRYLIST> lpList(TapiDLL.tapiGetCountry(dwCountryID));
		if (lpList.get() == NULL)
		{
			Reset();
			return false;
		}

		LPLINECOUNTRYENTRY lpEntry = reinterpret_cast<LPLINECOUNTRYENTRY>(reinterpret_cast<LPBYTE>(lpList.get()) + lpList->dwCountryListOffset);
		for (i = 0; i < lpList->dwNumCountries; i++)
		{
			// Ignore all countries which are not the default.
			if (lpEntry->dwCountryID != dwCountryID)
			{
				++lpEntry;
				continue;
			}

			if (lpEntry->dwInternationalRuleSize > 0)
			{
				strIntlCode = A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpList.get()) + lpEntry->dwInternationalRuleOffset));
				TString::iterator it = std::find_if(strIntlCode.begin(), strIntlCode.end(), 
					std::not1(std::ptr_fun(&tsplib::is_tdigit)));
				if (it != strIntlCode.end())
					strIntlCode.resize(it-strIntlCode.begin());
			}

			if (lpEntry->dwLongDistanceRuleSize > 0)
			{
				strNationalCode = A2T(reinterpret_cast<LPSTR>(reinterpret_cast<LPBYTE>(lpList.get()) + lpEntry->dwLongDistanceRuleOffset));
				TString::iterator it = std::find_if(strNationalCode.begin(), strNationalCode.end(), 
					std::not1(std::ptr_fun(&tsplib::is_tdigit)));
				if (it != strNationalCode.end())
					strNationalCode.resize(it-strNationalCode.begin());
			}
			break;
		}
		break;
	}

	_TSP_DTRACE(TRC_DUMP, _T("LOCATIONINFO structure loaded:\n"));
	_TSP_DTRACE(TRC_DUMP, _T(" CurrentLocation = %ld\n")
		                  _T(" CountryCode = (%s)\n")
						  _T(" AreaCode = (%s)\n")
						  _T(" NationalCode = (%s)\n")
						  _T(" InternationalCode = (%s)\n")
						  _T(" LongDistance Access Code = (%s)\n")
						  _T(" Local Access Code = (%s)\n")
						  _T(" Disable Call Waiting = (%s)\n"),
			dwCurrentLocation, strCountryCode.c_str(),
			strAreaCode.c_str(), strNationalCode.c_str(), strIntlCode.c_str(),
			strLongDistanceAccess.c_str(), strLocalAccess.c_str(), strCallWaiting.c_str());

	// Mark that we have loaded the information.
	m_fLoaded = true;

	return true;

}// LOCATIONINFO::Reload

namespace TDynamicCreate
{
// Global variables within this namespace
tsplib_TFactoryListMgr* tsplib_TFactoryListMgr::m_pHead = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CreateObject
//
// Create a new object dynamically. Search the list of dynamic
// objects and call each CreateObject() method until one of them 
// is successful in making the required object. If we get to the end of the
// list, then the object wasn't registered.
//
void* CreateObject (const char* pszClassName)    
{
	void* pObject = NULL;
	const tsplib_TFactoryListMgr* pCurrFactory = tsplib_TFactoryListMgr::m_pHead;
	for (; pCurrFactory != NULL; pCurrFactory = pCurrFactory->m_pNext)
	{
		pObject = pCurrFactory->m_pManufacturer->CreateObject(pszClassName);
		if (pObject != NULL)
			break;
	}
	return pObject;

}// CreateObject

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// _TFactoryListMgr::~_TFactoryListMgr
//
// When the factory<t> is destroyed, its "manufacturer"
// field is also destroyed, so this destructor is called.
// Remove the current factory from the factory_list.
//
tsplib_TFactoryListMgr::~tsplib_TFactoryListMgr()
{
	tsplib_TFactoryListMgr** ppNext = &tsplib_TFactoryListMgr::m_pHead;
	for (; *ppNext != NULL; ppNext = &(*ppNext)->m_pNext)
	{
		if (*ppNext == this)
		{
			*ppNext = (*ppNext)->m_pNext;
			break;
		}
	}
}// _TFactoryListMgr::~_TFactoryListMgr
}

/******************************************************************************/
//
// CEnterCode
//
/******************************************************************************/

#undef CEnterCode	// Don't put any classes after these definitions!!
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CEnterCode::CEnterCode
//
// Synchronization class
//
CEnterCode::CEnterCode (const CTSPIBaseObject* pObj, bool fAutoLock) :
	m_lLockCount(0), m_pObject(pObj)
{
	// If we were passed a valid object to lock...
	if (m_pObject != NULL)
	{
		// Increment the reference count for the object so it cannot be destroyed
		// until this object is destroyed.
		m_pObject->AddRef();

		// If we are to "auto-lock" the object ...
		if (fAutoLock)
		{
			if (!Lock(INFINITE))
			{
				_TSP_DTRACE(_T("WARNING: Infinite Lock failed Thread 0x%lx, Object 0x%lx\n"), 
					GetCurrentThreadId(), m_pObject);
			}
		}
	}

}// CEnterCode::CEnterCode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CEnterCode::~CEnterCode
//
// Synchronization class
//
CEnterCode::~CEnterCode()
{
	if (m_pObject != NULL)
	{
		// Unlock the critsec for each time we locked it.
		while (m_lLockCount > 0)
			Unlock();

		// Decrement the object's usage count so it may self-destruct now that
		// we are done using it (if it's refcount hits zero).
		m_pObject->DecRef();
	}

}// CEnterCode::~CEnterCode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CEnterCode::Lock
//
// Enter and lock the object for update
//
bool CEnterCode::Lock (DWORD dwMsecs)
{
	if (m_pObject != NULL)
	{
		bool fLock = m_pObject->GetSyncObject()->Lock(dwMsecs);
		if (fLock) ++m_lLockCount;
		return fLock;
	}
	return false;

}// CEnterCode::Lock

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CEnterCode::Unlock
//
// Exit and unlock the object
//
void CEnterCode::Unlock()
{
	if (m_pObject && m_lLockCount > 0)
	{
		--m_lLockCount;
		m_pObject->GetSyncObject()->Unlock();
	}

}// CEnterCode::Unlock

