#ifndef REGISTRY_H
#define REGISTRY_H

class Registry
{
public:
	static bool ReadKeyDword(const HKEY hKey, const tstring& name, DWORD* value, DWORD dwDefault=0 );
};

#endif
