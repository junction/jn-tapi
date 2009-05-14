/***************************************************************************
//
// UUI.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// TSPI_lineSendUserUserInfo processing
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
** Procedure:  CJTLine::OnSendUUI
**
** Arguments: 'pReq' - Request object representing this SENDUUI event 
**            'lpBuff' - Our CEventBlock* pointer
**
** Returns:    void
**
** Description:  This function manages the TSPI_lineSendUserUserInfo processing
**               for this service provider.
**
*****************************************************************************/
bool CJTLine::OnSendUUI(RTSendUserInfo* pRequest, LPCVOID /*lpBuff*/)
{
	CTSPICallAppearance* pCall = pRequest->GetCallInfo();

	// NOTE: This function is implemented simply for testing purposes.
	// It is intended in TAPI for use with ISDN in-band signalling or for
	// user notification through some physical device. Since the simulator
	// does not support UUI, we manage it here for station->station calls
	// within TAPI. This is NOT a standard implementation since it involves
	// no hardware (i.e. don't do this at home kids).
	if (pRequest->EnterState(STATE_INITIAL, STATE_IGNORE))	// Don't re-enter
	{
		// Look up the other side of the given call
		CTSPICallAppearance* pCall_Other = pCall->GetShadowCall();
		if (pCall_Other != NULL && GetLineType() == CTSPILineConnection::Station &&
			pCall_Other->GetLineOwner()->GetLineType() == CTSPILineConnection::Station)
		{
			// Pass it through to the other call.
			pCall_Other->OnReceivedUserUserInfo(
				pRequest->GetUserUserInfo(), pRequest->GetSize());
		}
		CompleteRequest(pRequest, 0);
	}
	return false;

}// CJTLine::OnSendUUI
