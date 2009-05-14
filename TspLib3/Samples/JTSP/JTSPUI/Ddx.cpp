/*******************************************************************/
//
// DDX.CPP
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

/*---------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------*/
#include "stdafx.h"
#include "ddx.h"

/*---------------------------------------------------------------*/
// DEBUG INFORMATION
/*---------------------------------------------------------------*/
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/*****************************************************************************
** Procedure:  DDX_LBStringArray
**
** Arguments: 'pDX' - DDX pointer
**            'nIDC' - Control id of the listbox
**            'array' - String array
**
** Returns:    void
**
** Description:  Synchronizes the data between a listbox and CStringArray
**
*****************************************************************************/
void AFXAPI DDX_LBStringArray(CDataExchange* pDX, int nIDC, CStringArray& array)
{
   HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
   if (hWndCtrl == NULL)
      return;

	if (pDX->m_bSaveAndValidate)
	{
		array.RemoveAll();
		int nCount = static_cast<int>(::SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L));

		if (nCount > 0)
		{
			int nCurrLen = 100;
			TCHAR* pszItem = new TCHAR[nCurrLen];
			for (int x = 0; x < nCount; x++)
			{
				int nLen = static_cast<int>(::SendMessage(hWndCtrl, LB_GETTEXTLEN, x, 0L));
				ASSERT(nLen > 0);

				if (nLen > nCurrLen)
				{
					delete [] pszItem;
					pszItem = new TCHAR[nLen+1];
					nCurrLen = nLen+1;
				}

				::SendMessage(hWndCtrl, LB_GETTEXT, x, (LPARAM)(LPSTR)pszItem);
				array.Add(pszItem);
			}
			delete [] pszItem;
		}
	}
	else
	{
		int nCurSel = static_cast<int>(::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L));
		int nTopIndex = static_cast<int>(::SendMessage(hWndCtrl, LB_GETTOPINDEX, 0, 0L));

		::SendMessage(hWndCtrl, WM_SETREDRAW, 0, 0);
		::SendMessage(hWndCtrl, LB_RESETCONTENT, 0, 0L);

		for (int x = 0; x < array.GetSize(); x++)
		{
			CString str = array.GetAt(x);
			if (!str.IsEmpty())
				::SendMessage(hWndCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		}

		if (nCurSel >= 0)
			::SendMessage(hWndCtrl, LB_SETCURSEL, nCurSel, 0);
		if (nTopIndex > 0)
			::SendMessage(hWndCtrl, LB_SETTOPINDEX, nTopIndex, 0);

		::SendMessage(hWndCtrl, WM_SETREDRAW, 1, 0);
		::InvalidateRect(hWndCtrl, NULL, TRUE);
		::UpdateWindow(hWndCtrl);
   }

}// DDX_LBStringArray

/*****************************************************************************
** Procedure:  DDX_CBStringArray
**
** Arguments: 'pDX' - DDX pointer
**            'nIDC' - Control id of the combobox
**            'array' - String array
**
** Returns:    void
**
** Description:  Synchronizes the data between a combobox and CStringArray
**
*****************************************************************************/
void AFXAPI DDX_CBStringArray(CDataExchange* pDX, int nIDC, CStringArray& array)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (hWndCtrl == NULL)
		return;

	if (pDX->m_bSaveAndValidate)
	{
		array.RemoveAll();
		int nCount = static_cast<int>(::SendMessage(hWndCtrl, CB_GETCOUNT, 0, 0L));

		if (nCount > 0)
		{
			int nCurrLen = 100;
			TCHAR* pszItem = new TCHAR[nCurrLen];
			for (int x = 0; x < nCount; x++)
			{
				int nLen = static_cast<int>(::SendMessage(hWndCtrl, CB_GETLBTEXTLEN, x, 0L));
				ASSERT(nLen > 0);

				if (nLen > nCurrLen)
				{
					delete [] pszItem;
					pszItem = new TCHAR[nLen+1];
					nCurrLen = nLen+1;
				}

				::SendMessage(hWndCtrl, CB_GETLBTEXT, x, (LPARAM)(LPSTR)pszItem);
				array.Add(pszItem);
			}
			delete [] pszItem;
		}
	}
	else
	{
		int nCurSel = static_cast<int>(::SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L));

		::SendMessage(hWndCtrl, WM_SETREDRAW, 0, 0);
		::SendMessage(hWndCtrl, CB_RESETCONTENT, 0, 0L);

		for (int x = 0; x < array.GetSize(); x++)
		{
			CString str = array.GetAt(x);
			if (!str.IsEmpty())
				::SendMessage(hWndCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		}

		if (nCurSel >= 0)
			::SendMessage(hWndCtrl, CB_SETCURSEL, nCurSel, 0L);

		::SendMessage(hWndCtrl, WM_SETREDRAW, 1, 0);
		::InvalidateRect(hWndCtrl, NULL, TRUE);
		::UpdateWindow(hWndCtrl);
	}

}// DDX_CBStringArray

/*****************************************************************************
** Procedure:  DDX_IPText
**
** Arguments: 'pDX' - DDX pointer
**            'nIDC' - Control id of the combobox
**            'strAddress' - CString to put data into
**
** Returns:    void
**
** Description:  Synchronizes the data between a CString and IPControl
**
*****************************************************************************/
void AFXAPI DDX_IPText(CDataExchange* pDX, int nIDC, CString& strAddress)
{
   HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
   if (hWndCtrl == NULL)
      return;

	if (pDX->m_bSaveAndValidate)
	{
		strAddress.Empty();
		if (::SendMessage(hWndCtrl, IPM_ISBLANK, 0, 0) == 0)
		{
			DWORD dwValue;
			if (::SendMessage(hWndCtrl, IPM_GETADDRESS, 0, reinterpret_cast<LPARAM>(&dwValue)) == 4)
				strAddress.Format(_T("%d.%d.%d.%d"), 
					FIRST_IPADDRESS(dwValue), SECOND_IPADDRESS(dwValue), 
					THIRD_IPADDRESS(dwValue), FOURTH_IPADDRESS(dwValue));
		}
	}
	else
	{
		if (strAddress.IsEmpty())
			::SendMessage(hWndCtrl, IPM_CLEARADDRESS, 0, 0);
		else
		{
			int b1,b2,b3,b4;
			_stscanf(strAddress,_T("%d.%d.%d.%d"), &b1, &b2, &b3, &b4);
			::SendMessage(hWndCtrl, IPM_SETADDRESS, 0, MAKEIPADDRESS(b1, b2, b3, b4));
		}
	}

}// DDX_IPText