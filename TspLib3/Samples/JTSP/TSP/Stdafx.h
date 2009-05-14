// STDAFX.H - Pre-compiled header support for JulMar TSP

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WINSOCKAPI_				// Don't include Winsock.h

#include <tchar.h>					// Unicode support
#include <windows.h>				// Windows includes
#include <winsock2.h>				// Winsock 2
#include <windowsx.h>               // Some useful MS macros
#include <splib.h>                  // TSP++ Class library

using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::endl;
