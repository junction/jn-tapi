/*******************************************************************/
//
// DDX.H
//
// Additional Dialog Data Exchange technology
//
// Copyright (C) 1998 JulMar Technology, Inc.
// All rights reserved
//
// TSP++ Version 3.00 PBX/ACD Emulator Projects
// Internal Source Code - Do Not Release
//
// Modification History
// --------------------
// 1998/09/05 MCS@JulMar	Initial revision
//
/*******************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __JPBX_DDX_INC__
#define __JPBX_DDX_INC__

/*---------------------------------------------------------------*/
// PUBLIC FUNCTIONS
/*---------------------------------------------------------------*/
void AFXAPI DDX_LBStringArray(CDataExchange* pDX, int nIDC, CStringArray& array);
void AFXAPI DDX_CBStringArray(CDataExchange* pDX, int nIDC, CStringArray& array);
void AFXAPI DDX_IPText(CDataExchange* pDX, int nIDC, CString& strAddress);

#endif // __JPBX_DDX_INC__