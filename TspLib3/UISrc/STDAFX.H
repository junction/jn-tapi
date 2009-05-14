/************************************************************************/
//                                                                        
// STDAFX.H - Pre-compiled header support for SPLIB retail build
//                                                                        
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//         
// Include the necessary files for building the library.
//                                                                
/************************************************************************/

#define _WIN32_WINNT 0x0500			// Windows 2000
#define WINVER 0x0500				// Windows 2000
#include <tchar.h>					// Unicode support
#include <afxwin.h>                 // Include the MFC/AFX headers
#include <afxmt.h>					// Multi-threaded classes
#include <afxtempl.h>				// Template classes
#include <windowsx.h>				// For GlobalAllocPtr
#include <splibui.h>                // The user-interface portion of our class library
