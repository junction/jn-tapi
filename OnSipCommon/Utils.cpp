#include "stdafx.h"

#include "Utils.h"
#include <time.h>
#include <sys\timeb.h>
#include <time.h>
#include <windows.h>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include "logger.h"

using namespace std;

// Format the current time using the specified strftime format values
//static
// Added special non-strftime flag to output milliseconds,  %t
tstring DateTimeOperations::FormatNow(const TCHAR* format)
{
	// Get current time (with milliseconds)
	struct _timeb timebuffer;
	_ftime_s(&timebuffer);

	// Convert time to 'tm' type for string formatting
	struct tm tmx;
	localtime_s(&tmx,&timebuffer.time);

	// Get # of milliseconds as string
	tstring szms = Strings::stringFormat(_T("%04d"),timebuffer.millitm);
	// Create new copy of format string to be updated with milliseconds
	tstring szFormat = format;
	szFormat = Strings::replace(szFormat,_T("%t"),szms);

	// Format the time using strftime format types
	TCHAR str[200];
	_tcsftime( str, sizeof(str)/sizeof(TCHAR), szFormat.c_str(), &tmx );

	// Return back as tstring
	return tstring(str);
}

// Return the formatted string for the specified time
//static 
tstring DateTimeOperations::getTimeString(time_t& t)
{
	TCHAR buf[256];
	_tctime_s( buf, sizeof(buf)/sizeof(TCHAR), &t);
	return tstring(buf);
}

//*************************************************************
//*************************************************************

tstring Strings::trim_right (const tstring & s, const tstring & t)
{ 
	tstring d (s); 
	tstring::size_type i (d.find_last_not_of (t));
	if (i == tstring::npos)
		return _T("");
	else
		return d.erase (d.find_last_not_of (t) + 1) ; 
}

tstring Strings::trim_left (const tstring & s, const tstring & t) 
{ 
	tstring d (s); 
	return d.erase (0, s.find_first_not_of (t)) ; 
}

tstring Strings::trim (const tstring & s, const tstring & t)
{ 
	tstring d (s); 
	return trim_left (trim_right (d, t), t) ; 
}

// returns a lower case version of the string 
tstring Strings::tolower (const tstring & s)
{
	tstring d (s);
	transform (d.begin (), d.end (), d.begin (), (int(*)(int)) ::tolower);
	return d;
}

// returns an upper case version of the string 
tstring Strings::toupper (const tstring & s)
{
	tstring d (s);
	transform (d.begin (), d.end (), d.begin (), (int(*)(int)) ::toupper);
	return d;
}

// string find-and-replace
//  source = string to have replaced text
//  target = string to be searched within source and replaced
//  replacement = new string to be replaced where target exists
// returns new modified string
tstring Strings::replace(const tstring& source, const tstring& target, const tstring& replacement)
{
	tstring str = source;
	tstring::size_type pos = 0,   // where we are now
		found;     // where the found data is

	if (target.size () > 0)   // searching for nothing will cause a loop
	{
		while ((found = str.find (target, pos)) != tstring::npos)
		{
			str.replace (found, target.size (), replacement);
			pos = found + replacement.size ();
		}
	}

	return str;
};

// Strip all non-numeric values from string
tstring Strings::stripNonNumeric(const tstring& str)
{
	const TCHAR* pstr = str.data();
	tstring strReturn;
    while ( *pstr )
    {   
        if (is_tdigit(*pstr) )
            strReturn += *pstr;
        pstr++;
    }
	return strReturn;
}
// case insensitive compare of strings
bool Strings::stringsSame(const tstring& str1,const tstring& str2)
{	return _tcsicmp(str1.c_str(), str2.c_str()) == 0; }

// Returns true if 'str' starts with 'chars'
bool Strings::startsWith(const tstring& str,const tstring& chars)
{	return str.find(chars) == 0;	}

// Returns true if 'str' ends with 'chars'
bool Strings::endsWith(const tstring& str,const tstring& chars)
{
	size_t i = str.rfind(chars);
	return (i != tstring::npos) && (i == (str.length() - chars.length()));
}

// Returns true if 'str' contains 'chars'
bool Strings::contains(const tstring& str,const tstring& chars)
{	return str.find(chars) != tstring::npos;	}

tstring Strings::stringFormat(const TCHAR* format, ...)
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
	tstring ret(str);
	// Free memory and return formatted tstring
	delete[] str;
	return ret;
}

//static
// string converts from widechar to multibyte
string Strings::convert(const wstring& src)
{
	string ret;
	ret.resize(src.size());
	size_t i;
	wcstombs_s(&i, (char *) ret.data(), ret.size()+1, src.data(), ret.size() );
	return ret;
}

//static
// string converts from mulibyte to widechar
wstring Strings::convert(const string& src)
{
	size_t i;
	wstring ret;
	ret.resize(src.size());
	mbstowcs_s(&i,(wchar_t *)ret.data(),ret.size()+1, src.data(), ret.size());
	return ret;
}

const TCHAR * toStrip = _T(" \n\r\t");

tstring Strings::lstrip(const tstring& s) {
	tstring::size_type n = s.find_first_not_of(toStrip);
	return n == tstring::npos ? tstring() : s.substr(n);
}

tstring Strings::rstrip(const tstring& s) {
	tstring::size_type n = s.find_last_not_of(toStrip);
	return n == tstring::npos ? tstring() : s.substr(0, n+1);
}

tstring Strings::strip(const tstring& s) {
	return lstrip(rstrip(s));
}

tstring Strings::repr(const tstring& _s) {
	string s = Strings::T_TO_S(_s);
	string r;
	for (unsigned i = 0; i < s.size(); i++) {
		if (s[i] == '\n')
			r += "\\n";
		else if (s[i] == '\r')
			r += "\\r";
		else if (s[i] == '\t')
			r += "\\t";
		else
			r += s[i];
	}
	return Strings::S_TO_T(r);
}

