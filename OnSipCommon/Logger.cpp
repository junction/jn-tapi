#include "stdafx.h"

#include "stdarg.h"
#include "windows.h"
#include <assert.h>
#include <tchar.h>
#include "logger.h"
#include "Utils.h"

#define LOGGING_MAX_CHARS_LIMIT  2048

//#if DEBUG
int Logger::_consoleLevel  = LEVEL_DEBUG;
int Logger::_win32Level  = LEVEL_DEBUG;
//#else
//int Logger::_consoleLevel  = LEVEL_ERROR;
//int Logger::_win32Level  = LEVEL_ERROR;
//#endif

bool Logger::_checkLevel(int level)
{	return ( _consoleLevel >= level || _win32Level >= level );	}

// Set level for both console and win32
void Logger::SetLevel(LogLevel level)
{	
	SetConsoleLevel(level);
	SetWin32Level(level);
}

// Set level for console
void Logger::SetConsoleLevel(LogLevel level)
{	_consoleLevel = (int) level;	}

// Set level for Win32
void Logger::SetWin32Level(LogLevel level)
{	_win32Level = (int) level;	}

bool Logger::IsLevel(LogLevel level)
{	return _checkLevel((int)level); }

void Logger::_outputMsg(int log_level,const TCHAR *str)
{
	if ( _consoleLevel >= log_level )
		_tprintf(str);
	if ( _win32Level >= log_level )
		OutputDebugString(str);
}

void Logger::_output(const int log_level, const TCHAR * msg)
{
	const TCHAR *prefix = NULL;

	switch (log_level)
	{
	case LEVEL_ERROR: 
		prefix = _T("[ERROR] "); 
		break;
	case LEVEL_DEBUG: 
		prefix = _T("[DEBUG] "); 
		break;
	case LEVEL_TRACE: 
		prefix = _T("[TRACE] "); 
		break;
	case LEVEL_WARN:  
		prefix = _T("[WARN]  ");
		break;
	case LEVEL_APP:  
		prefix = _T("[APP]  ");
		break;
	default:          
		prefix = _T("[UNKNOWN] "); 
		break;
	}

	tstring curTime = DateTimeOperations::FormatNow(_T("%m/%d %H:%M:%S:%t"));
	_outputMsg( log_level, Strings::stringFormat(_T("%s [TID=%ld] %s %s\n"), curTime.c_str(), GetCurrentThreadId(), prefix, msg ).c_str() );
}

void Logger::_output(int level,va_list & argList, const TCHAR * szFormat)
{
	TCHAR buffer[LOGGING_MAX_CHARS_LIMIT];
	TCHAR* allocBuffer = NULL;
	unsigned allocSize = LOGGING_MAX_CHARS_LIMIT;
	TCHAR* fmtBuffer = NULL;

	while ( true )
	{
		fmtBuffer = (allocBuffer!=NULL) ? allocBuffer : buffer;

		// Do the format and output
		const int ret = _vsntprintf_s( fmtBuffer , allocSize , _TRUNCATE, szFormat, argList);
		if ( ret >= 0 || ret > 64000 )
			break;
		// Allocate bigger buffer for output
		if ( allocBuffer != NULL )
			delete[] allocBuffer;
		allocSize += 2048;
		allocBuffer = new TCHAR[allocSize];
	}

	_output(level, (const TCHAR *) fmtBuffer );

	// Free allocated memory
	if ( allocBuffer != NULL )
		delete[] allocBuffer;
}

void Logger::log_app(const TCHAR *format,...)
{
	int level = LEVEL_APP;
	if (_checkLevel(level))
	{
		va_list argList;
		va_start(argList, format);
		_output(level, argList, format);
		va_end(argList);
	}
}

void Logger::log_debug(const TCHAR *format,...)
{
	int level = LEVEL_DEBUG;
	if (_checkLevel(level))
	{
		va_list argList;
		va_start(argList, format);
		_output(level, argList, format);
		va_end(argList);
	}
}

void Logger::log_trace(const TCHAR *format,...)
{
	int level = LEVEL_TRACE;
	if (_checkLevel(level))
	{
		va_list argList;
		va_start(argList, format);
		_output(level, argList, format);
		va_end(argList);
	}
}

void Logger::log_warn(const TCHAR *format,...)
{
	int level = LEVEL_WARN;
	if (_checkLevel(level))
	{
		va_list argList;
		va_start(argList, format);
		_output(level, argList, format);
		va_end(argList);
	}
}

void Logger::log_error(const TCHAR *format,...)
{
	int level = LEVEL_ERROR;
	if (_checkLevel(level))
	{
		va_list argList;
		va_start(argList, format);
		_output(level, argList, format);
		va_end(argList);
	}
}
