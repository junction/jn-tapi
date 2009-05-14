/******************************************************************************/
//                                                                        
// PHONE.CPP - Digital Switch Service Provider Sample
//                                                                        
// This file contains the phone device code for the service provider.
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
/******************************************************************************/

#include "stdafx.h"
#include "dssp.h"
#pragma warning(disable:4201)
#include <mmsystem.h>
#pragma warning(default:4201)

/*----------------------------------------------------------------------------
	GLOBALS
-----------------------------------------------------------------------------*/

// Translate hookswitch states from our switch to TAPI
const DWORD g_hsStates[] = { 
    PHONEHOOKSWITCHMODE_ONHOOK,
    PHONEHOOKSWITCHMODE_MICSPEAKER,
    PHONEHOOKSWITCHMODE_MIC,
    PHONEHOOKSWITCHMODE_SPEAKER
};    

// Keypad digits
const char g_szKeypad[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#' };

// Switch lamp states to TAPI translation
const DWORD g_LampStates[] = {
    PHONELAMPMODE_DUMMY,
    PHONELAMPMODE_OFF,
    PHONELAMPMODE_STEADY,
    PHONELAMPMODE_WINK,
    PHONELAMPMODE_FLASH
};    

// Button states
const DWORD g_ButtonStates[] = {
    PHONEBUTTONSTATE_UP,
    PHONEBUTTONSTATE_DOWN
};    

// This array translates the button functions into TAPI mode/functions with
// a text face for the phone button.
const struct
{
    DWORD dwMode;
    DWORD dwFunction;
    LPCTSTR pszText;
    
} g_ButtonFunctions[] = {
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_NONE,		_T("") },
    { PHONEBUTTONMODE_CALL,    PHONEBUTTONFUNCTION_CALLAPP,     _T("Call") },
    { PHONEBUTTONMODE_DISPLAY, PHONEBUTTONFUNCTION_CALLID,		_T("Display") },
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_HOLD,		_T("Hold") },
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_DROP,		_T("Release") },
    { PHONEBUTTONMODE_LOCAL,   PHONEBUTTONFUNCTION_VOLUMEUP,	_T("Vol>>") },
    { PHONEBUTTONMODE_LOCAL,   PHONEBUTTONFUNCTION_VOLUMEDOWN,	_T("<<Vol") },
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_TRANSFER,	_T("Transfer") },
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_FORWARD,		_T("Forward") },
    { PHONEBUTTONMODE_DUMMY,   PHONEBUTTONFUNCTION_MSGINDICATOR,_T("Msg Waiting") },
    { PHONEBUTTONMODE_KEYPAD,  PHONEBUTTONFUNCTION_NONE,		_T("") },
    { PHONEBUTTONMODE_FEATURE, PHONEBUTTONFUNCTION_CONFERENCE,	_T("Conference") }
};    

/*-------------------------------------------------------------------------------*/
// TSPI Request map
/*-------------------------------------------------------------------------------*/
BEGIN_TSPI_REQUEST(CDSPhone)
	ON_TSPI_REQUEST_SETHOOKSWITCHGAIN(processSetGain)
	ON_TSPI_REQUEST_SETHOOKSWITCH(processSetHook)
	ON_TSPI_REQUEST_SETHOOKSWITCHVOL(processSetVol)
	ON_TSPI_REQUEST_SETRING(processSetRing)
END_TSPI_REQUEST()

/*****************************************************************************
** Procedure:  CDSPhone::CDSPhone
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Constructor for the phone object
**
*****************************************************************************/
CDSPhone::CDSPhone()
{
}// CDSPhone::CDSPhone

/*****************************************************************************
** Procedure:  CDSPhone::~CDSPhone
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for the phone object
**
*****************************************************************************/
CDSPhone::~CDSPhone()
{
}// CDSPhone::~CDSPhone

/*****************************************************************************
** Procedure:  CDSPhone::Init
**
** Arguments:  'pDev'				-	Device object this phone belongs to
**             'dwPhoneDeviceID'	-	Unique phone identifier within the TSP
**             'dwPos'				-	Index position of phone within device array
**             'dwItemData'		-   Used when line was dynamically created (P&P).
**
** Returns:    void
**
** Description:  This function is called by the device owner to initialize
**               the phone object.
**
*****************************************************************************/
VOID CDSPhone::Init (CTSPIDevice* pDev, DWORD dwPhoneDeviceID, DWORD dwPos, DWORD /*dwItemData*/)
{
	// Let the base class initialize first.
	CTSPIPhoneConnection::Init (pDev, dwPhoneDeviceID, dwPos);

    // Grab the settings from the emulator.
    EMSETTINGS Settings;
    if (!GetDeviceInfo()->DRV_GetSwitchSettings (&Settings))
		return;
    
    // Add the handset device to our hookswitch list.
    AddHookSwitchDevice(PHONEHOOKSWITCHDEV_HANDSET,				// Hookswitch device
						(PHONEHOOKSWITCHMODE_ONHOOK |			// Modes available to hookswitch
						 PHONEHOOKSWITCHMODE_MIC | 
						 PHONEHOOKSWITCHMODE_SPEAKER |
						 PHONEHOOKSWITCHMODE_MICSPEAKER | 
						 PHONEHOOKSWITCHMODE_UNKNOWN),
						 g_hsStates[Settings.wHandsetHookswitch],	// States supported by hookswitch
						 Settings.wVolHandset,						// Current Volume level of hookswitch (0-0xffff)
						 Settings.wGainHandset,						// Current Gain level of hookswitch (0-0xffff)
						 0xffffffff, 0xffffffff, true, true);		// Monitored/Set modes
                
    // Setup the display buffer for the phone.  It uses a standard line feed so don't
    // change the default line break character.
    SetupDisplay(DISPLAY_COLS, DISPLAY_ROWS);

    // Add all the buttons to our phone.
    for (int i = 0; i < BUTTON_COUNT; i++)
    {   
		// If the button is one of our STANDARD buttons (0-9,A-D,#,*), then it is a keypad
		// button.
		if (i < TOTAL_STD_BUTTONS)
        {
            AddButton (PHONEBUTTONFUNCTION_NONE,		// Button function
					   PHONEBUTTONMODE_KEYPAD,			// Button mode
                       PHONELAMPMODE_DUMMY,				// Available Lamp states (Dummy = None)
					   PHONELAMPMODE_DUMMY,				// Current Lamp state (Dummy = None)
					   CString(g_szKeypad[i],1));		// Text name of button
        }                               

		// Otherwise the button is a "soft" button, the mode and function are determined
		// by how the user sets up the emulator.  The emulator reports the button functions
		// through the EMSETTINGS structure.
        else
        {   
			// Determine the available lamp states based on the reported mode of the
			// button from the emulator.
            DWORD dwAvailLampStates = 0;
            if (Settings.wButtonModes[i] == BUTTONFUNCTION_CALL)
                dwAvailLampStates = (PHONELAMPMODE_OFF | PHONELAMPMODE_STEADY | PHONELAMPMODE_WINK | PHONELAMPMODE_FLASH);
            else if (Settings.wLampStates[i] == LAMPSTATE_NONE)
                dwAvailLampStates = PHONELAMPMODE_DUMMY;
            else
                dwAvailLampStates = (PHONELAMPMODE_OFF | PHONELAMPMODE_STEADY | PHONELAMPMODE_WINK);

			// Add the button
            AddButton (g_ButtonFunctions[Settings.wButtonModes[i]].dwFunction,
                       g_ButtonFunctions[Settings.wButtonModes[i]].dwMode,  
                       dwAvailLampStates,
                       g_LampStates[Settings.wLampStates[i]],
                       g_ButtonFunctions[Settings.wButtonModes[i]].pszText);                            
        }                               
    }

    // Setup the initial state of the display.
	USES_CONVERSION;
    SetDisplay (A2T(Settings.szDisplay));

    // Setup the ringer modes
    LPPHONECAPS pPhoneCaps = GetPhoneCaps();
    LPPHONESTATUS pPhoneStatus = GetPhoneStatus();
    pPhoneCaps->dwNumRingModes = 4;
    pPhoneStatus->dwRingMode = (DWORD) Settings.wRingMode;

	// Add the WAV devices which will be our handset I/O
	if (waveInGetNumDevs() > 0)
		AddDeviceClass(_T("wave/in"), (DWORD)0);
	if (waveOutGetNumDevs() > 0)
		AddDeviceClass(_T("wave/out"), (DWORD)0);

}// CDSPhone::Init

/*****************************************************************************
** Procedure:  CDSPhone::processSetGain
**
** Arguments:  'pReq' - Request being handled
**             'lpBuff' - Data buffer from emulator
**
** Returns:    bool
**
** Description:  This function processes the phoneSetGain API.
**
*****************************************************************************/
bool CDSPhone::processSetGain(RTSetGain* pReq, LPCVOID /*lpBuff*/)
{
	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		GetDeviceInfo()->DRV_SetGain(pReq->GetHookswitchDevice(), pReq->GetGain());
		CompleteRequest(pReq, 0);
	}	
	return false;
}

/*****************************************************************************
** Procedure:  CDSPhone::processSetVol
**
** Arguments:  'pReq' - Request being handled
**             'lpBuff' - Data buffer from emulator
**
** Returns:    bool
**
** Description:  This function processes the phoneSetVolume API.
**
*****************************************************************************/
bool CDSPhone::processSetVol(RTSetVolume* pReq, LPCVOID /*lpBuff*/)
{
	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		GetDeviceInfo()->DRV_SetVolume(pReq->GetHookswitchDevice(), pReq->GetVolume());
		CompleteRequest(pReq, 0);
	}
	return false;
}

/*****************************************************************************
** Procedure:  CDSPhone::processSetHook
**
** Arguments:  'pReq' - Request being handled
**             'lpBuff' - Data buffer from emulator
**
** Returns:    bool
**
** Description:  This function processes the phoneSetHookswitch API.
**
*****************************************************************************/
bool CDSPhone::processSetHook(RTSetHookswitch* pReq, LPCVOID /*lpBuff*/)
{
	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		GetDeviceInfo()->DRV_SetHookswitch(pReq->GetHookswitchDevice(), pReq->GetHookswitchState());
		CompleteRequest(pReq, 0);
	}
	return false;
}

/*****************************************************************************
** Procedure:  CDSPhone::processSetRing
**
** Arguments:  'pReq' - Request being handled
**             'lpBuff' - Data buffer from emulator
**
** Returns:    bool
**
** Description:  This function processes the phoneSetRing API.
**
*****************************************************************************/
bool CDSPhone::processSetRing(RTSetRing* pReq, LPCVOID /*lpBuff*/)
{
	if (pReq->EnterState(STATE_INITIAL, STATE_IGNORE))
	{
		GetDeviceInfo()->DRV_SetRing(pReq->GetRingMode());
		CompleteRequest(pReq, 0);
	}
	return false;
}

/*****************************************************************************
** Procedure:  CDSPhone::UnsolicitedEvent
**
** Arguments: 'lpBuff'    - Data buffer (structure) specific to code
**
** Returns:    void
**
** Description:  This function processes any responses from the device which
**               are not matched to a pending line request.
**
*****************************************************************************/
bool CDSPhone::UnsolicitedEvent(LPCVOID lpBuff)
{             
	USES_CONVERSION;
	EVENTBUFF* pevBuff = (EVENTBUFF*) lpBuff;

    switch (pevBuff->dwResponse)
    {
        // A lamp has changed states.
        case EMRESULT_LAMPCHANGED:        
        {
            LPEMLAMPCHANGE lpChange = (LPEMLAMPCHANGE) pevBuff->lpBuff;
            SetLampState ((int)lpChange->wButtonLampID, g_LampStates[lpChange->wLampState]);
        }                                        
        break;
        
        // A hookswitch device has changed states.
        case EMRESULT_HSCHANGED:
        {
            LPEMHOOKSWITCHCHANGE lpChange = (LPEMHOOKSWITCHCHANGE) pevBuff->lpBuff;
            _TSP_ASSERTE (lpChange->wHookswitchID == HSDEVICE_HANDSET);
            SetHookSwitch (PHONEHOOKSWITCHDEV_HANDSET, g_hsStates[lpChange->wHookswitchState]);
        }
        break;

        // A button has changed
        case EMRESULT_BUTTONCHANGED:
        {
            LPEMBUTTONCHANGE lpChange = (LPEMBUTTONCHANGE) pevBuff->lpBuff;
            SetButtonState (lpChange->wButtonLampID, g_ButtonStates[lpChange->wButtonState]);
        }                    
        break;
        
        // Ringer mode changed
        case EMRESULT_RINGCHANGE:
            SetRingMode (*((LPDWORD)pevBuff->lpBuff));
            break;
        
        // Volume/Gain of the handset changed
        case EMRESULT_LEVELCHANGED:
        {
            LPEMLEVELCHANGE lpChange = (LPEMLEVELCHANGE) pevBuff->lpBuff;
            if (lpChange->wLevelType == LEVELTYPE_MIC)
                SetGain (PHONEHOOKSWITCHDEV_HANDSET, lpChange->wLevel);
            else if (lpChange->wLevelType == LEVELTYPE_SPEAKER)
                SetVolume (PHONEHOOKSWITCHDEV_HANDSET, lpChange->wLevel);
        }
        break;                        
        
        // The display has changed.
        case EMRESULT_DISPLAYCHANGED:
        {
            LPEMDISPLAY lpChange = (LPEMDISPLAY) pevBuff->lpBuff;
            SetDisplay (A2T(lpChange->szDisplay));
        }        
        break;
    }

	return true;

}// CDSPhone::UnsolicitedEvent
