/***************************************************************************
//
// JTSP.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
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

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "jtsp.h"
#include <crtdbg.h>
#include <time.h>

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-------------------------------------------------------------------------------*/
// CONSTANTS AND GLOBALS
/*-------------------------------------------------------------------------------*/
CJTProvider theSP;

/*-------------------------------------------------------------------------------*/
// RTTI overrides of TSP++ objects
/*-------------------------------------------------------------------------------*/
DECLARE_TSPI_OVERRIDE(CJTDevice);
DECLARE_TSPI_OVERRIDE(CJTLine);
DECLARE_TSPI_OVERRIDE(CJTPhone);

/*****************************************************************************
** Procedure:  CJTProvider::CJTProvider
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This is the constructor for the provider object.  It
**               is responsible for initializing the UI.DLL name and the
**               provider manufacturer names.  In additional, any overrides
**               of class objects should be performed here.
**
*****************************************************************************/
CJTProvider::CJTProvider() : 
	CServiceProvider(_T("JTSPUI.DLL"),					// Name of UI DLL
					 _T("JulMar Sample TAPI Server"),	// Description
					 TAPIVER_22)						// Hi TAPI version to negotiate to
{
	// Output a startup banner.
	_TSP_TRACE(_T("JTSP Version 3.02 (C) 1999-2004 JulMar Technology, Inc.\n"));

	// Setup our line device and phone device object override.
	SetRuntimeObjects (
		"CJTDevice",		// Device override 
		"CJTLine",			// Line override
		NULL,				// Address override
		NULL,				// Call override
		NULL,				// Conference call override
		"CJTPhone");		// Phone override

#ifdef _DEBUG
	// Set the desired level of tracing
	SetTraceLevel(TRC_FULL & ~(TRC_LOCKS));
#endif

}// CJTProvider::CJTProvider
