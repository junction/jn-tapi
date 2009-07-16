#include "stdafx.h"
#include "Logger.h"

#ifndef FILEIO_H
#define FILEIO_H

class FileIO
{
private:
	HANDLE _hFile;
public:

	FileIO()
	{	_hFile == INVALID_HANDLE_VALUE; }

	~FileIO()
	{	Close();	}

	bool Open(const tstring& filename);

	bool IsOpen()
	{	return _hFile != INVALID_HANDLE_VALUE;	}

	void Write(const void *buffer,DWORD size);
	void Write(const tstring& buffer);
	void Close();
};

#endif