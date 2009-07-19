#include "Stdafx.h"
#include "regio.h"

//static
DWORD RegIO::ReadDword(HKEY hKey,const tstring& path,const tstring& key,DWORD dwDefault)
{
	HKEY hNewKey = NULL;
	if ( ::RegOpenKeyEx(hKey, path.c_str(), 0L, KEY_READ, &hNewKey) != ERROR_SUCCESS )
		return dwDefault;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwRet=0;
	LONG ret = ::RegQueryValueEx(hNewKey,key.c_str(), 0, &dwType, (LPBYTE)&dwRet, &dwSize);
	CloseHandle(hNewKey);
	return (ret == ERROR_SUCCESS) ? dwRet : dwDefault;
}
