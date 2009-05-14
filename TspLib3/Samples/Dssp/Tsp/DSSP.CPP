/*****************************************************************************/
//
// DSSP.CPP - Digital Switch Service Provider Sample
//                                                                        
// This file contains the main service provider override class.
// 
// Copyright (C) 1997-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This service provider talks to a simulated digital switch emulator.  It
// is designed as a sample of how to implement a service provider using
// the TAPI C++ SP class library.  
//
// This source code is only intended as a supplement to the
// documentation provided with the TAPI C++ SP class library.
//
/*****************************************************************************/

#include "stdafx.h"
#include "dssp.h"

DECLARE_TSPI_OVERRIDE(CDSDevice);
DECLARE_TSPI_OVERRIDE(CDSLine);
DECLARE_TSPI_OVERRIDE(CDSPhone);

/*----------------------------------------------------------------------------
	MAIN TSP OBJECT
-----------------------------------------------------------------------------*/
CDSProvider theSP;

/*****************************************************************************
** Procedure:  CDSProvider::CDSProvider
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
CDSProvider::CDSProvider() : 
	CServiceProvider(_T("DSSP32.TSP"),								// Name of UI DLL
					 _T("Digital Switch Sample Service Provider"),	// Description
					 TAPI_CURRENT_VERSION)							// Version of TAPI this is compiled for
{   
	// Setup our line device and phone device object override.
	SetRuntimeObjects (
		"CDSDevice",		// Device override 
		"CDSLine",			// Line override
		NULL,				// Address override
		NULL,				// Call override
		NULL,				// Conference call override
		"CDSPhone");		// Phone override

}// CDSProvider::CDSProvider

