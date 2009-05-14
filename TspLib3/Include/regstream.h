/******************************************************************************/
//                                                                        
// REGSTREAM.H - Registry stream class
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// INTERNAL DATA STRUCTURE HEADER
//
/******************************************************************************/

#ifndef __REGSTREAM_INC__
#define __REGSTREAM_INC__

#include <spbstrm.h> // Base class support

namespace tsplib {
///////////////////////////////////////////////////////////////////////////////
// TRegstream
//
// Registry stream object - derivative of the TStream object, stores 
// it's information to/from the registry. This organizes the stream as a 
// series of binary bytes.
//
class TRegstream : public TStream
{
// Class data
protected:
	bool m_fDirty;
	TString m_strDeviceKey;
	LPBYTE m_pBuffer, m_pCurr_R, m_pCurr_W, m_pLast;
	DWORD m_dwProviderID;

// Constructor
public:
	// custom constructor
	TRegstream(DWORD dwProviderID, LPCTSTR pszDevKey) : 
		m_pBuffer(0), m_pCurr_R(0), m_pCurr_W(0), m_pLast(0), 
		m_strDeviceKey(pszDevKey), m_fDirty(false), 
		m_dwProviderID(dwProviderID) {
	}
	virtual ~TRegstream() {	
		close();
		delete m_pBuffer; 
	}
	virtual bool open() { 
		DWORD dwSize = _regQuerySize();
		if (dwSize > 0)
		{
			realloc(dwSize);
			_regQueryValue(m_pBuffer, &dwSize);
		}
		return true;
	}
	virtual void close() {
		if ((m_pCurr_W > m_pBuffer) && m_fDirty)
		{
			_regSetValue(m_pBuffer, (m_pCurr_W-m_pBuffer));
			m_fDirty = false;
		}
	}
	virtual bool read(void* pBuff, unsigned int size) {
		if ((m_pCurr_R + size) >= m_pLast)
			return false;
		ZeroMemory(pBuff, size);
		CopyMemory(pBuff, m_pCurr_R, size);
		m_pCurr_R += size;
		return true;
	}
	virtual bool write(const void* pBuff, unsigned int size) {
		if ((m_pCurr_W + size) >= m_pLast)
			realloc((m_pCurr_W+size) - m_pLast);
		m_fDirty = true;
		CopyMemory(m_pCurr_W, pBuff, size);
		m_pCurr_W += size;
		return true;
	}
	virtual bool backup(int size) {
		if (m_pCurr_R - size >= m_pBuffer)
			m_pCurr_R -= size;
		return true;
	}
	void realloc(unsigned int size) {
		unsigned int csize = (m_pLast - m_pBuffer);
		unsigned int asize = (((csize+size+1024)>>10)<<10);
		BYTE* lpByte = new BYTE[asize];
		if (lpByte == NULL)
			SCHEMA_EXCEPT("out of memory");
		ZeroMemory(lpByte, asize);
		CopyMemory(lpByte, m_pBuffer, csize);

		unsigned int rsize = (m_pCurr_R - m_pBuffer);
		unsigned int wsize = (m_pCurr_W - m_pBuffer);
		m_pCurr_R = (lpByte + rsize);
		m_pCurr_W = (lpByte + wsize);
		m_pLast = (lpByte + asize);
		delete [] m_pBuffer;
		m_pBuffer = lpByte;
	}

	TString KeyName(unsigned int i) const {
		LPCTSTR gszTelephonyKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");
		LPTSTR lpBuff = new TCHAR[20+m_strDeviceKey.length()+1];
		TString strKey = gszTelephonyKey;
		wsprintf(lpBuff, _T("\\%s\\Device%ld\\Data%d"), m_strDeviceKey.c_str(), m_dwProviderID, i);
		strKey += lpBuff;
		delete [] lpBuff;
		return strKey;
	}

// Non-exposed members and unavailable constructor/operators
private:
	TRegstream( const TRegstream& );
	TRegstream& operator=( const TRegstream& );

	// Internal registry functions - always write to BINARY!
	DWORD _regQuerySize() const;
	bool _regQueryValue(LPBYTE lpBuffer, LPDWORD lpdwSize) const;
	bool _regSetValue(LPBYTE lpBuffer, DWORD dwSize) const;
	bool _regOpenCreateKey(HKEY* lpkey, TString* pstrKey, bool fCreate=true) const;
};

///////////////////////////////////////////////////////////////////////////////
// TRegstream::_regQuerySize
//
// Returns the size of a key in the registry
//
inline DWORD TRegstream::_regQuerySize() const
{
#ifdef _SPLIBUI_INC_
	DWORD dwCount = GetUISP()->ReadProfileDWord(m_dwProviderID, _T("Count"));
#else
	DWORD dwCount = GetSP()->ReadProfileDWord(m_dwProviderID, _T("Count"));
#endif
	return (dwCount * 1024);

}// TRegstream::_regQuerySize

///////////////////////////////////////////////////////////////////////////////
// TRegstream::_regQueryValue
//
// Returns the value of a key
//
inline bool TRegstream::_regQueryValue(LPBYTE pszBuff, LPDWORD lpdwSize) const
{
#ifdef _SPLIBUI_INC_
	DWORD dwCount = GetUISP()->ReadProfileDWord(m_dwProviderID, _T("Count"));
#else
	DWORD dwCount = GetSP()->ReadProfileDWord(m_dwProviderID, _T("Count"));
#endif
	DWORD dwSize = 0;
	LPBYTE lpCurr = pszBuff;
	for (unsigned int i = 0; i < dwCount; i++)
	{
		TString strKey = KeyName(i);
		HKEY hKey = NULL;
		if (_regOpenCreateKey(&hKey, &strKey, false))
		{
			DWORD dwDataType;
			dwSize = 1024;
			if (::RegQueryValueEx(hKey, strKey.c_str(), 0, &dwDataType, lpCurr, &dwSize) != ERROR_SUCCESS)
			{
				*lpdwSize = 0;
				CloseHandle(hKey);
				break;
			}

			*lpdwSize += dwSize;
			lpCurr += dwSize;
			CloseHandle(hKey);
		}
	}

	return (*lpdwSize != 0);

}// TRegstream::_regQueryValue

///////////////////////////////////////////////////////////////////////////////
// TRegstream::_regSetValue
//
// Write the value of a key in the registry
//
inline bool TRegstream::_regSetValue(LPBYTE pszBuff, DWORD dwSize) const
{
	LPBYTE lpCurr = pszBuff; unsigned int i;
	for (i = 0; dwSize > 0; i++)
	{
		TString strKey = KeyName(i);
		HKEY hKey = NULL;
		if (_regOpenCreateKey(&hKey, &strKey, false))
		{
			DWORD dwCurrSize = (dwSize > 1024) ? 1024 : dwSize;
			if (::RegSetValueEx (hKey, strKey.c_str(), 0, REG_BINARY, reinterpret_cast<const BYTE*>(lpCurr), dwCurrSize) != ERROR_SUCCESS)
			{
				CloseHandle(hKey);
				break;
			}

			lpCurr += dwCurrSize;
			dwSize -= dwCurrSize;
			CloseHandle(hKey);
		}
	}
#ifdef _SPLIBUI_INC_
	GetUISP()->WriteProfileDWord(m_dwProviderID, _T("Count"), i);
#else
	GetSP()->WriteProfileDWord(m_dwProviderID, _T("Count"), i);
#endif

	return (dwSize == 0);

}// TRegstream::_regSetValue

///////////////////////////////////////////////////////////////////////////////
// TRegstream::_regOpenCreateKey
//
// Opens/Creates the key heirarchy in the registry.
//
inline bool TRegstream::_regOpenCreateKey(HKEY* lphKey, TString* lpsKey, bool fCreate) const
{
	// Walk through the tree opening each key
	HKEY hKey = HKEY_LOCAL_MACHINE;
	TString strTree = *lpsKey, strCurrent;

	TString::size_type iPos = strTree.find('\\');
	while (iPos != TString::npos)
	{
		strCurrent = strTree.substr(0,iPos);
		strTree = strTree.substr(iPos+1);
		
		HKEY hNewKey;
		DWORD dwDisposition;
		if ((::RegOpenKeyEx(hKey, strCurrent.c_str(), 0L, KEY_ALL_ACCESS, &hNewKey) != ERROR_SUCCESS && !fCreate) ||
			(fCreate && ::RegCreateKeyEx(hKey, strCurrent.c_str(), 0L, _T(""), 
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwDisposition) != ERROR_SUCCESS))
		{
			if (hKey != HKEY_LOCAL_MACHINE)
				CloseHandle(hKey);
			return false;
		}

		if (hKey != HKEY_LOCAL_MACHINE)
			CloseHandle(hKey);
		hKey = hNewKey;
		iPos = strTree.find('\\');
	}

	// Return our results.
	*lphKey = hKey;
	*lpsKey = strTree;
	return true;

}// TRegstream::_regOpenCreateKey

}// namespace tsplib

#endif // __REGSTREAM_INC__
