#define _WIN32_WINNT 0x0400			// Windows NT
#define _USRDLL                     // TSPs are DLL files

#include <tchar.h>					// Unicode support
#include <afxwin.h>                 // Include the MFC/AFX headers
#include <splib.h>                  // Our class library
#include <splibui.h>				// UI library
#include <windowsx.h>               // Some useful MS macros
#include <afxmt.h>					// Multi-threading headers
#include <afxpriv.h>				// Conversion macros
#include <afxsock.h>				// Socket interface
#include <afxtempl.h>				// MFC template support
