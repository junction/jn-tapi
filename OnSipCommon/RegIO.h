#ifndef REGIO_H
#define REGIO_H

class RegIO
{
public:

	static DWORD ReadDword(HKEY hKey,const tstring& path,const tstring& key,DWORD dwDefault=0);
};

#endif
