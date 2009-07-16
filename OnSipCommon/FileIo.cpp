#include "stdafx.h"
#include "Logger.h"
#include "fileio.h"

bool FileIO::Open(const tstring& filename)
{
	Logger::log_debug("FileIO::Open %s", filename.c_str());
	Close();
	_hFile = ::CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN, 
		NULL);
	if(_hFile == INVALID_HANDLE_VALUE)
	{
		Logger::log_error("FileIO::Open %s err=%d", filename.c_str(), GetLastError() );
		return false;
	}
	return true;
}

void FileIO::Write(const void *buffer,DWORD size)
{
	if ( _hFile == INVALID_HANDLE_VALUE )
	{
		Logger::log_error("FileIO::Write file not open");
		return;
	}
	DWORD dwWrite=0;
	BOOL ret = ::WriteFile( _hFile, buffer, size, &dwWrite, NULL  );
	if ( !ret || dwWrite != size )
		Logger::log_error("FileIO::Write error ret=%d size=%ld write=%ld", ret, size, dwWrite );
}

void FileIO::Write(const tstring& buffer)
{
	Write( buffer.c_str(), buffer.size() );
}

void FileIO::Close()
{
	if ( _hFile != INVALID_HANDLE_VALUE )
		::CloseHandle(_hFile);
	_hFile = INVALID_HANDLE_VALUE;
}
