/************************************************************************/
//                                                                        
// STDAFX.H - Pre-compiled header support for TSPLIB project
//                                                                        
// Copyright (C) 1994-2005 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//         
// Include the necessary files for building the library.
//                                                                
/************************************************************************/

#define WINVER 0x0500				// Windows 2000 or better
#define _SECURE_SCL 0
#define _WIN32_WINNT 0x0500
#define STRICT                      // Now required for TSP++
#include <tchar.h>					// Unicode support
#include <windows.h>				// Windows includes
#include <splib.h>                  // Our class library
#include <numeric>					// STL numeric algorithms

// VS.NET 2005 adds new safe string functions which are missing in
// previous versions of the SDK.
#if _MSC_VER < 1400
#define _tcsupr_s(b,s) _tcsupr(b)
#define _itot_s(n,b,r) _itot(n,b,r)
#define _ltoa_s(n,b,s,r) _ltoa(n,b,r)
#endif

using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::endl;
