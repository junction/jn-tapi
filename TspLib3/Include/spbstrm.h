/******************************************************************************/
//                                                                        
// SBSTRM.H - TAPI Server binary stream support
//                                             
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// The SPLIB classes provide a basis for developing MS-TAPI complient     
// Service Providers.  They provide basic handling for all of the TSPI    
// APIs and a C-based handler which routes all requests through a set of C++     
// classes.                                                                 
//              
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                           
/******************************************************************************/

#ifndef _SPLIBSTREAM_INC_
#define _SPLIBSTREAM_INC_

#pragma warning(push,3)
#pragma warning(disable : 4018)
#include <string>
#include <vector>
#ifndef __ATLCONV_H__
#include <atlconv.h>
#endif
#pragma warning(pop)

/******************************************************************************/
//
// SCHEMA_ERROR
//
// Internal exception generated when a iostream fails to read persistant
// data from the registry or other stream source.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class schema_error : public std::exception 
{
public:
	explicit schema_error(const char* ps = "") : std::exception(ps) {/* */}
};

#ifdef _DEBUG
#define SCHEMA_EXCEPT(s) throw (schema_error(s));
#else
#define SCHEMA_EXCEPT(s) throw (schema_error());
#endif

/******************************************************************************/
//
// TStream
//
// Base class for the binary input stream used to retrieve persistant object
// data from some location. This location could be the registry, a file, an OLE
// object, etc.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class TStream
{
// Destructor
public:
	virtual ~TStream() {/* */}

// Overridable operations required in the derived classes
public:
	virtual bool open() { return true; };
	virtual void close() {/* */};
	virtual bool read(void* pBuff, unsigned int size) = 0;
	virtual bool write(const void* pBuff, unsigned int size) = 0;
	// This is called with a zero size to determine if backup is supported
	// by the given stream. If not, then no version schema is allowed in the
	// stream.
	virtual bool backup(int /*size*/) { return false; }
};

/******************************************************************************/
//
// TVersionInfo
//
// Stores and manages schema versions within a object stream source
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class TVersionInfo
{
// Class data
protected:
	int m_nVerRead;
public:
	TVersionInfo(TStream& ar);
	TVersionInfo(TStream& ar, int nVer);
	int GetVersion() const { return m_nVerRead; }
};

// insertion operations
inline TStream& operator<<(TStream& stm, long l) {
	if(!stm.write(&l, sizeof(long)))
	{
		SCHEMA_EXCEPT("operator<<(long)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, float f) {
	if(!stm.write(&f, sizeof(float)))
	{
		SCHEMA_EXCEPT("operator<<(float)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, double d) {
	if(!stm.write(&d, sizeof(double)))
	{
		SCHEMA_EXCEPT("operator<<(double)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, bool f) {
	unsigned char c = (f == true) ? 
		static_cast<unsigned char>(0x1) : 
		static_cast<unsigned char>(0x0);
	if(!stm.write(&c, sizeof(unsigned char)))
	{
		SCHEMA_EXCEPT("operator<<(bool)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, int i) {
	long l = static_cast<long>(i);
	if(!stm.write(&l, sizeof(long)))
	{
		SCHEMA_EXCEPT("operator<<(int)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, short w) {
	if(!stm.write(&w, sizeof(short)))
	{
		SCHEMA_EXCEPT("operator<<(short)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, char ch) {
	if(!stm.write(&ch, sizeof(char)))
	{
		SCHEMA_EXCEPT("operator<<(char)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, unsigned char by) {
	if(!stm.write(&by, sizeof(unsigned char)))
	{
		SCHEMA_EXCEPT("operator<<(unsigned char)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, unsigned short w) {
	if(!stm.write(&w, sizeof(unsigned short)))
	{
		SCHEMA_EXCEPT("operator<<(unsigned short)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, unsigned int w) {
	unsigned long l = static_cast<unsigned long>(w);
	if(!stm.write(&l, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator<<(unsigned int)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, unsigned long dw) {
	if(!stm.write(&dw, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator<<(unsigned long)");
	}
	return stm;
}

#ifdef __AFX_H__
inline TStream& operator<<(TStream& stm, const CString& str) {
	USES_CONVERSION;
	LPWSTR pwstrBuff = T2W(const_cast<LPTSTR>(static_cast<LPCTSTR>(str)));
	unsigned long len = (lstrlenW(pwstrBuff)+1) * sizeof(wchar_t);
	if (!stm.write(&len, sizeof(unsigned long)) ||
		!stm.write(pwstrBuff, len))
	{
		SCHEMA_EXCEPT("operator<<(CString)");
	}
	return stm;
}
#endif

inline TStream& operator<<(TStream& stm, const std::string& str) {
	USES_CONVERSION;
	LPWSTR pwstrBuff = A2W(const_cast<LPSTR>(str.c_str()));
	unsigned long len = (lstrlenW(pwstrBuff)+1) * sizeof(wchar_t);
	if (!stm.write(&len, sizeof(unsigned long)) ||
		!stm.write(pwstrBuff, len))
	{
		SCHEMA_EXCEPT("operator<<(std::string)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, const std::wstring& str) {
	unsigned long len = static_cast<unsigned long>(str.length()+1) * sizeof(wchar_t);
	if (!stm.write(&len, sizeof(unsigned long)) ||
		!stm.write(str.c_str(), len))
	{
		SCHEMA_EXCEPT("operator<<(wstring)");
	}
	return stm;
}

inline TStream& operator<<(TStream& stm, const GUID& guid) {
	if (!stm.write(&guid, sizeof(GUID)))
	{
		SCHEMA_EXCEPT("operator<<(GUID)");
	}
	return stm;
}

// extraction operations
inline TStream& operator>>(TStream& stm, long& l) {
	if(!stm.read(&l, sizeof(long)))
	{
		SCHEMA_EXCEPT("operator>>(long)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, float& f) {
	if(!stm.read(&f, sizeof(float)))
	{
		SCHEMA_EXCEPT("operator>>(float)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, double& d) {
	if(!stm.read(&d, sizeof(double)))
	{
		SCHEMA_EXCEPT("operator>>(double)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, bool& f) {
	unsigned char i;
	if(!stm.read(&i, sizeof(unsigned char)))
	{
		SCHEMA_EXCEPT("operator>>(bool)");
	}
	f = (i == static_cast<unsigned char>(0)) ? false : true;
	return stm;
}
inline TStream& operator>>(TStream& stm, int& i) {
	long l;
	if(!stm.read(&l, sizeof(long)))
	{
		SCHEMA_EXCEPT("operator>>(int)");
	}

	i = static_cast<int>(l);
	return stm;
}
inline TStream& operator>>(TStream& stm, short& w) {
	if(!stm.read(&w, sizeof(short)))
	{
		SCHEMA_EXCEPT("operator>>(short)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, char& ch) {
	if(!stm.read(&ch, sizeof(char)))
	{
		SCHEMA_EXCEPT("operator>>(char)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, unsigned char& by) {
	if(!stm.read(&by, sizeof(unsigned char)))
	{
		SCHEMA_EXCEPT("operator>>(unsigned char)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, unsigned short& w) {
	if(!stm.read(&w, sizeof(unsigned short)))
	{
		SCHEMA_EXCEPT("operator>>(unsigned short)");
	}
	return stm;
}
inline TStream& operator>>(TStream& stm, unsigned int& w) {
	unsigned long l;
	if(!stm.read(&l, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator>>(unsigned int)");
	}

	w = static_cast<unsigned int>(l);
	return stm;
}
inline TStream& operator>>(TStream& stm, unsigned long& dw) {
	if(!stm.read(&dw, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator>>(unsigned long)");
	}
	return stm;
}

#ifdef __AFX_H__
inline TStream& operator>>(TStream& stm, CString& str) {
	unsigned long len=0;
	if (!stm.read(&len, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator>>(CString)");
	}

	if (len > 0)
	{
		std::vector<BYTE> ch(len);
		if (!stm.read(&ch[0], len))
		{
			SCHEMA_EXCEPT("operator>>(CString)");
		}

		USES_CONVERSION;
		str = W2T(reinterpret_cast<LPWSTR>(&ch[0]));
	}
	else 
		str.Empty();
	return stm;
}
#endif

inline TStream& operator>>(TStream& stm, std::string& str) {
	unsigned long len=0;
	if (!stm.read(&len, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator>>(string)");
	}

	if (len > 0)
	{
		std::vector<BYTE> ch(len);
		if (!stm.read(&ch[0], len))
		{
			SCHEMA_EXCEPT("operator>>(string)");
		}

		USES_CONVERSION;
		str = W2A(reinterpret_cast<LPWSTR>(&ch[0]));
	}
	else
		str = "";

	return stm;
}

inline TStream& operator>>(TStream& stm, std::wstring& str) {
	unsigned long len=0;
	if (!stm.read(&len, sizeof(unsigned long)))
	{
		SCHEMA_EXCEPT("operator>>(string)");
	}

	if (len > 0)
	{
		std::vector<BYTE> ch(len);
		if (!stm.read(&ch[0], len))
		{
			SCHEMA_EXCEPT("operator>>(string)");
		}
		str = reinterpret_cast<LPWSTR>(&ch[0]);
	}
	else
		str = L"";
	
	return stm;
}

inline TStream& operator>>(TStream& stm, GUID& guid) {
	if (!stm.read(&guid, sizeof(GUID)))
	{
		SCHEMA_EXCEPT("operator>>(GUID)");
	}
	return stm;
}

///////////////////////////////////////////////////////////////////////////////
// TVersionInfo::TVersionInfo
//
// Version support within a stream archive
//
inline TVersionInfo::TVersionInfo(TStream& ar, int nVer)
{
	const DWORD VER_MARK = 0x52455600;
#ifdef SPLIB_CURRENT_VERSION
	_TSP_ASSERT(nVer <= 0x000000ff);
#else
#ifdef ASSERT
	ASSERT(nVer <= 0x000000ff);
#endif
#endif
	m_nVerRead = -1;

	// If the stream doesn't support backups then exit.
	if (!ar.backup(0))
		return;

	// Otherwise, store the version marker
	DWORD dwID = (VER_MARK | (DWORD)nVer);	
	ar << dwID;

}// TVersionInfo::TVersionInfo

///////////////////////////////////////////////////////////////////////////////
// TVersionInfo::TVersionInfo
//
// Version support within a stream archive
//
inline TVersionInfo::TVersionInfo(TStream& ar)
{
	const DWORD VER_MARK = 0x52455600;
	m_nVerRead = -1;

	// If the stream doesn't support backups then exit.
	if (!ar.backup(0))
		return;

	// Read the version marker and backup if it doesn't exist.
	DWORD dwID; ar >> dwID;
	if ((dwID & 0xffffff00) == VER_MARK)
		m_nVerRead = (int) (dwID & 0x000000ff);
	else
		ar.backup(sizeof(DWORD));

}// TVersionInfo::TVersionInfo

#endif // _SPLIBSTREAM_INC_
