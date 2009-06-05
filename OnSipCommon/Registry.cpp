#include "stdafx.h"

#include <windows.h>
#include <string>
#include "logger.h"
#include "Registry.h"

using namespace std;

// Read value from the registry, put result into "value"
bool Registry::ReadKeyDword(const HKEY hKey, const tstring& name, DWORD* value, DWORD dwDefault)
{
	*value = dwDefault;
	DWORD type = REG_DWORD;
	DWORD sz = sizeof(DWORD);
	const LONG ret = RegQueryValueEx(hKey, name.c_str(), NULL, &type, (LPBYTE) value, &sz);
	if (0 != ret)
	{
		Logger::log_debug( _T("ReadKeyDword %s not found, return default %ld, ret=%ld"), name.c_str(), dwDefault, ret );
		return false;
	}
	return true;
}

