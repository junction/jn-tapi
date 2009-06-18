#ifndef UTILS_H
#define UTILS_H

#include <tchar.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
using namespace std;

typedef basic_string<TCHAR>	tstring;
typedef vector<tstring> tstring_vector;
typedef map<const tstring,tstring> mapOfStringToString;

#define SPACES _T(" \t\r\n")

class DateTimeOperations
{
public:
	// Format the current time using the specified strftime format values
	static tstring FormatNow(const TCHAR* format);

	// Return the formatted string for the specified time
	static tstring getTimeString(time_t& t);

	// Convert time_t to system time in struct tm
	static struct tm GetUTCTime(time_t& t);

	static tstring getUTCTimeString(time_t& t);

};

// Class that can be used to determine the
// amount of time that has elapsed in msecs
class TimeElapsed
{
private:
	clock_t _start;

public:
	TimeElapsed() 
	{ Reset(); }

	void Reset() 
	{ _start = clock(); }

	// Return # msecs elapsed since construct or since last Reset()
	long Msecs()
	{	return  (long) ((clock() - _start) / (double)CLK_TCK * 1000); }
};

#define MSECS_IN_SEC	1000
#define SECS_IN_MIN		60
#define MINS_IN_HOUR	60
#define MSECS_IN_MIN	(MSECS_IN_SEC * SECS_IN_MIN)
#define MSECS_IN_HOUR	(MSECS_IN_MIN * MINS_IN_HOUR)

// Class to manage a period of timeout period.
// Specify the # msecs til timeout.  The timer
// can then be reset and then be polled using IsExpired().
class TimeOut
{
private:
	TimeElapsed _t;
	long _msecs;
public:
	TimeOut() 
	{  _msecs = 0; }

	TimeOut(long msecs) 
	{  _msecs = msecs; }

	void SetMsecs(long msecs)
	{ _msecs = msecs; Reset(); }

	void Reset() 
	{ _t.Reset(); }

	long Msecs()
	{	return _msecs; }

	bool IsExpired()
	{	return _t.Msecs() >= _msecs; }
};

class Strings
{
private:
	static tstring _xorWithChar(const tstring& strInput,TCHAR charXor);
	static tstring _xor(const tstring& strInput,const tstring& stringXor);

public:
	static tstring stringFormat(const TCHAR* format, ...);

	// string converts from mulibyte to widechar, and back
	static string convert(const wstring& src);
	static wstring convert(const string& src);

	// Return a copy of s, stripping whitespace from the left.
	static tstring lstrip(const tstring& s);
	// Return a copy of s, stripping whitespace from the right.
	static tstring rstrip(const tstring& s);
	// Return a copy of s, stripping whitespace from either side.
	static tstring strip(const tstring& s);
	// Return a copy of s with special characters escaped.
	static tstring repr(const tstring& s);

	static tstring trim_right (const tstring & s, const tstring & t = SPACES);
	static tstring trim_left (const tstring & s, const tstring & t = SPACES);
	static tstring trim (const tstring & s, const tstring & t = SPACES);
	// returns a lower case version of the string 
	static tstring tolower (const tstring & s);
	// returns an upper case version of the string 
	static tstring toupper (const tstring & s);
	// string find-and-replace
	// string find-and-replace
	//  source = string to have replaced text
	//  target = string to be searched within source and replaced
	//  replacement = new string to be replaced where target exists
	// returns new modified string
	static tstring replace(const tstring& source, const tstring& target, const tstring& replacement);

	// Simple decrypt on 'instr' using the key
	static string decryptString(const string& strInput,const string& decryptKey);
	// Do a simple encrypt on strInput with each character in the encryptKey
	static string encryptString(const string& strInput,const string& encryptKey);
	// Returns a reverse copy of 'str'
	static tstring reverseString(const tstring& strInput);

	static tstring stripNonNumeric(const tstring& str);

	static inline bool is_tdigit(TCHAR ch) 
		{return ((ch >= _T('0') && ch <= _T('9')) ? true : false);}

	// Returns true if 'str' starts with 'chars'
	static bool startsWith(const tstring& str,const tstring& chars);
	// Returns true if 'str' ends with 'chars'
	static bool endsWith(const tstring& str,const tstring& chars);
	// Returns true if 'str' contains 'chars'
	static bool contains(const tstring& str,const tstring& chars);

	// case insensitive compare of strings
	static bool stringsSame(const tstring& str1,const tstring& str2);
	
	// Conversion routines between tstring and wstring/string
#ifdef _UNICODE
	static wstring T_TO_W(const tstring& str)	{ return str; }
	static string T_TO_S(const tstring& str)	{ return Strings::convert(str); }
	static tstring W_TO_T(const wstring& str)	{ return str; }
	static tstring S_TO_T(const string& str)	{ return Strings::convert(str); }
#else
	static wstring T_TO_W(const tstring& str)	{ return Strings::convert(str); }
	static string T_TO_S(const tstring& str)	{ return str; }
	static tstring W_TO_T(const wstring& str)	{ return Strings::convert(str); }
	static tstring S_TO_T(const string& str)	{ return str; }
#endif

	// Conversion routine for raw char* to TCHAR*. Always makes a copy. Caller owns the returned pointer.
	static TCHAR* rS_TO_rT(const char* str);
};

// UniqueID class without thread-safe
// on retrieving unique IDs.
class UniqueId
{
private:
	long _id;

public:
	UniqueId() { _id=0; }

	long getNextId()
	{
		return ++_id;
	}
};


#endif
