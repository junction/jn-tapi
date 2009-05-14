/*****************************************************************************/
//
// INIT.CPP - Digital Switch Service Provider Sample
//                                                                        
// This file contains the initialization code for the service provider.
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

/*****************************************************************************
** Procedure:  CDSProvider::providerEnumDevices
**
** Arguments:  'dwProviderId'	-	Passed provider ID from TAPI
**             'lpNumLines'		-	Returning count of lines
**             'lpNumPhones'	-	Returning count of phones
**			   'hProvider'		-	Handle to the provider from TAPI
**			   'lpfnLineC..'	-	Line creation PROC for LINE_CREATE
**			   'lpfnPhoneC..'	-	Phone creation PROC for PHONE_CREATE	
**
** Returns:    TAPI result code
**
** Description:  This is called after the constructor to determine how many
**				 lines and phones the provider is managing.  This is called
**				 BEFORE providerInit.
**
**				 It is a required function under TAPI 2.x
**
*****************************************************************************/
LONG CDSProvider::providerEnumDevices(DWORD dwProviderId, LPDWORD lpNumLines,
                                 LPDWORD lpNumPhones, HPROVIDER hProvider,
                                 LINEEVENT lpfnLineCreateProc, 
                                 PHONEEVENT lpfnPhoneCreateProc)
{   
	// Pass it onto the base class and let the library know
	// about the provider handle.  This is required for 
	// dynamic UI dialog support.
	LONG lResult = CServiceProvider::providerEnumDevices(dwProviderId,
								lpNumLines, lpNumPhones, hProvider,
								lpfnLineCreateProc, lpfnPhoneCreateProc);

	// Enforce the line/phone device counts.
    *lpNumLines	 = 1;
    *lpNumPhones = 1;

    return lResult;

}// CDSProvider::providerEnumDevices

