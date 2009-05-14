/***************************************************************************
//
// GENERATE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineGenerateDigits processing
//
// Copyright (C) 1998 JulMar Entertainment Technology, Inc.
// All rights reserved
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// Modification History
// ------------------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Generated from TSPWizard.exe
// 
/***************************************************************************/

/*----------------------------------------------------------------------------
	INCLUDE FILES
-----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "jtsp.h"

/*----------------------------------------------------------------------------
	DEBUG SUPPORT
-----------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
** Procedure:  CJTLine::OnGenerateDigits
**
** Arguments: 'pReq' - Request object representing this GENERATEDIGITS event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineSendUserUserInfo processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnGenerateDigits(RTGenerateDigits* pRequest, LPCVOID /*lpBuff*/)
{
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// If we are in the initial state (i.e. this request has not been processed
	// before by any other thread). Then move the packet to the waiting state so 
	// other threads will not interfere with other events or timers.  This is 
	// guarenteed to be thread-safe and atomic.
	if (pRequest->EnterState(STATE_INITIAL, STATE_WAITING))
	{
		// Spit all the digits out to the PBX. Wait for the proper duration
		// between digits
		for (unsigned int i = 0; i < pRequest->GetDigits().length(); i++)
		{
			// If it is a pause character then wait for 50 msec. This is
			// up to the provider to decide how long to wait.
			TCHAR chChar = pRequest->GetDigits().at(i);
			if (chChar == _T(',')) 
				Sleep(GetLineDevCaps()->DefaultDialParams.dwDialPause);

			// Otherwise send the character and then wait for the inter-character
			// duration. TSP++ verifies the duration and limits it to the set
			// LINEDEVCAPS.MinMaxDialParams values.
			else
			{
				GetDeviceInfo()->DRV_GenerateDigit(this, pCall, chChar);
				Sleep(pRequest->GetDuration());
			}
		}

		// Complete the request successfully.
		CompleteRequest(pRequest, 0);
	}
	return false;

}// CJTLine::OnGenerateDigits
