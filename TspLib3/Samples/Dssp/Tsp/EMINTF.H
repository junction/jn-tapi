/*****************************************************************************/
//
// EMINTF.H - Emulator interface file for Digital Switch Sample (DSSP)
//                                                                        
// This file contains public callable interface to the digital switch 
// emulator program.
// 
// Copyright (C) 1994-1997 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
/*****************************************************************************/

#ifndef __EMINT_INC__
#define __EMINT_INC__

#ifndef RC_INVOKED
#pragma pack(1)         // Assume byte packing
#endif 

// Command codes going TO the emulator
#define EMCOMMAND_INIT           0x0001      // Initialize connection to service provider
#define EMCOMMAND_QUERYCAPS      0x0002      // Query the capabilities of the emulator
#define EMCOMMAND_GETVERSION     0x0003      // Returns version information of emulator
#define EMCOMMAND_GETADDRESSINFO 0x0004		 // Request information on an address.
#define EMCOMMAND_PREPAREADDR    0x0006      // Prepare an address for an outgoing call
#define EMCOMMAND_DIAL           0x0007      // Dial a number (LPARAM = dialstring)
#define EMCOMMAND_DROPCALL       0x0008      // Drop a call
#define EMCOMMAND_TSPEXIT        0x0009      // TSP exiting - window is now invalid.
#define EMCOMMAND_ANSWER         0x000A      // Answer an offering call.
#define EMCOMMAND_FORWARD        0x000B      // Forward to a destination address
#define EMCOMMAND_TRANSFER       0x000C      // Transfer a call to another address
#define EMCOMMAND_HOLDCALL       0x000D      // Place a call on hold
#define EMCOMMAND_UNHOLDCALL     0x000E      // Take a call off hold
#define EMCOMMAND_FLASH          0x000F      // Flash the hookswitch
#define EMCOMMAND_SETLEVEL       0x0010      // Set vol/gain of phone
#define EMCOMMAND_SETHOOKSWITCH  0x0011      // Set the hookswitch of the phone (MIC)
#define EMCOMMAND_CONFERENCE     0x0012      // Conference call
#define EMCOMMAND_REDIRECT       0x0013      // Redirect an offering call
#define EMCOMMAND_SETRINGMODE    0x0014      // Set the ring mode of the phone
#define EMCOMMAND_COMPLETECALL   0x0015      // Complete a call
#define EMCOMMAND_PARKCALL       0x0016      // Park a call
#define EMCOMMAND_UNPARKCALL     0x0017      // Unpark a call
#define EMCOMMAND_CLOSE          0x0018      // Close the connection

// Result codes from the emulator
#define EMRESULT_TIMER           0x0000      // Not used by emulator - generated by TSP     
#define EMRESULT_INIT            0x0001      // Emulator registering with us (MUST MATCH EMCOMMAND_INIT)
#define EMRESULT_APPEXIT         0x0002      // Emulator is terminating
#define EMRESULT_ADDRESSCHANGED  0x0003      // Address has new state
#define EMRESULT_RING            0x0004      // Address RING event detected
#define EMRESULT_CALLERID        0x0005      // CallerID information detected
#define EMRESULT_LAMPCHANGED     0x0006      // Lamp changed state
#define EMRESULT_HSCHANGED       0x0007      // Hookswitch changed state
#define EMRESULT_DISPLAYCHANGED  0x000B      // Display has changed
#define EMRESULT_BUTTONCHANGED   0x000C      // Button state changed               
#define EMRESULT_DIGITDETECTED   0x000D      // Detected digit from remote end
#define EMRESULT_LEVELCHANGED    0x000E      // Vol/Gain levels changed.
#define EMRESULT_REDIRECTID      0x000F      // RedirectingID information detected
#define EMRESULT_TONEDETECTED    0x0010      // Tone detected on address.
#define EMRESULT_RINGCHANGE      0x0011      // Ring style change                  
#define EMRESULT_COMPLRESULT     0x0012      // Call completion result
#define EMRESULT_RECEIVED        0x0013		 // EMCOMMAND received and being processed.
#define EMRESULT_ERROR           0x0014      // EMCOMMAND received but had errors.

// Address states 
#define ADDRESSSTATE_UNKNOWN       0x0      // Unknown
#define ADDRESSSTATE_CONNECT       0x1      // Address is connected to another call
#define ADDRESSSTATE_BUSY          0x2      // Destination party is BUSY
#define ADDRESSSTATE_DISCONNECT    0x3      // Address disconnected from remote party
#define ADDRESSSTATE_DIALTONE      0x4      // Address is at a dialtone.
#define ADDRESSSTATE_ONLINE        0x5      // Address is online - waiting for digits/connect
#define ADDRESSSTATE_OFFLINE       0x6      // Online address now idle (final state)
#define ADDRESSSTATE_OFFERING      0x7      // Call is being offered on address.
#define ADDRESSSTATE_ONHOLD        0x8      // Call is onhold
#define ADDRESSSTATE_INCONF        0x9      // Call is conferenced
#define ADDRESSSTATE_OFFERINGT	   0xA      // Call is being offered from external trunk

// Dialtone types (mode for ADDRESSSTATE_DIALTONE)
#define DIALTONETYPE_INTERNAL      0x0      // Internal "buzz" dialtone
#define DIALTONETYPE_EXTERNAL      0x1      // Normal Bell dialtone
        
// Busy types (mode for ADDRESSSTATE_BUSY)
#define BUSYTYPE_NORMAL            0x0      // Normal busy indicator
#define BUSYTYPE_SWITCH            0x1      // Switch is busy (no trunk available)        

// Hold types
#define HOLDTYPE_NORMAL            0x0      // Normal hold pattern
#define HOLDTYPE_TRANSFER          0x1      // Hold for pending transfer
#define HOLDTYPE_CONFERENCE        0x2      // Hold for conference attach

// Call completion types
#define CALLCOMPLTYPE_CAMP         0x1      // Camp on call (Nop)
#define CALLCOMPLTYPE_CALLBACK     0x2      // Call back after 5 secs.
#define CALLCOMPLTYPE_INTRUDE      0x3      // Force call to CONNECT
#define CALLCOMPLTYPE_MESSAGE      0x4      // Nop (display msg)
        
// Hookswitch devices supported in emulator
#define HSDEVICE_HANDSET           0x1      // Handset hookswitch

// Ringer styles
#define RINGER_STYLE1              0x0
#define RINGER_STYLE2              0x1
#define RINGER_STYLE3              0x2
#define RINGER_NONE                0x3      // No ring

// Level types
#define LEVELTYPE_MIC              0x1      // Microphone
#define LEVELTYPE_SPEAKER          0x2      // Speaker

// Hookswitch states
#define HSSTATE_ONHOOK             0x0      // Hookswitch device is onhook
#define HSSTATE_OFFHOOKMICSPEAKER  0x1      // Hookswitch device has speaker/mic active
#define HSSTATE_OFFHOOKMIC         0x2      // Hookswitch device has mic active
#define HSSTATE_OFFHOOKSPEAKER     0x3      // Hookswitch device has speaker active

// Lamp states
#define LAMPSTATE_NONE             0x0      // No lamp for this button
#define LAMPSTATE_OFF              0x1      // Lamp is dark
#define LAMPSTATE_STEADY           0x2      // Lamp is on
#define LAMPSTATE_BLINKING         0x3      // Lamp is blinking
#define LAMPSTATE_FLASHING         0x4      // Lamp is flashing
    
// Button states                                                      
#define BUTTONSTATE_UP             0x0      // Button is in the UP state
#define BUTTONSTATE_DOWN           0x1      // Button is in the DOWN state

// Button functions (i.e. what the button/lamp is assigned to)
#define BUTTONFUNCTION_NONE        0x00     // Unassigned button
#define BUTTONFUNCTION_CALL        0x01     // Button is a call appearance (i.e. has an address)
#define BUTTONFUNCTION_DISPLAY     0x02     // Button is a DISPLAY key
#define BUTTONFUNCTION_HOLD        0x03     // Button is a HOLD key
#define BUTTONFUNCTION_RELEASE     0x04     // Button is a RELEASE key
#define BUTTONFUNCTION_VOLUP       0x05     // Button is a VOL-UP key
#define BUTTONFUNCTION_VOLDN       0x06     // Button is a VOL-DN key
#define BUTTONFUNCTION_TRANSFER    0x07     // Button is a TRANSFER key
#define BUTTONFUNCTION_FORWARD     0x08     // Button is a FORWARD key
#define BUTTONFUNCTION_MSGWAITING  0x09     // Message Waiting Lamp
#define BUTTONFUNCTION_DIGIT       0x0A     // Digit (0-9, *, #)
#define BUTTONFUNCTION_CONFERENCE  0x0B     // Button is a CONFERENCE key
    
// Conference requests
#define CONFCOMMAND_ADD            0x1      // Add a new call to the conference
#define CONFCOMMAND_REMOVE         0x2      // Remove a call from the conference
#define CONFCOMMAND_DESTROY        0x3      // Destroy a conference
    
// Media modes for addresses.  These match the constants setup by TAPI for convenience.
#define MEDIAMODE_UNKNOWN           0x00000002
#define MEDIAMODE_INTERACTIVEVOICE  0x00000004
#define MEDIAMODE_AUTOMATEDVOICE    0x00000008
#define MEDIAMODE_DATAMODEM         0x00000010
#define MEDIAMODE_G3FAX             0x00000020
#define MEDIAMODE_TDD               0x00000040
#define MEDIAMODE_G4FAX             0x00000080
#define MEDIAMODE_DIGITALDATA       0x00000100
#define MEDIAMODE_TELETEX           0x00000200
#define MEDIAMODE_VIDEOTEX          0x00000400
#define MEDIAMODE_TELEX             0x00000800
#define MEDIAMODE_MIXED             0x00001000
#define MEDIAMODE_ADSI              0x00002000
#define MEDIAMODE_VOICEVIEW         0x00004000
#define MEDIAMODE_ALL               0x00007ffe
    
// Emulator values
#define DISPLAY_COLS                40      // Columns on the phone display
#define DISPLAY_ROWS                 2      // Rows on the phone display
#define BUTTON_COUNT                33      // Total buttons on emulator
#define SWITCHINFO_SIZE             40      // Switch info size
#define OWNER_SIZE                  50      // Owner name
#define ADDRESS_SIZE                50      // Address owner name
#define COMPLMESSAGE_SIZE           50      // Call completion message size

// Error codes passed in lParam for EMRESULT_ERROR
#define EMERROR_INVALADDRESSID      0x0001  // Address id passed is invalid.
#define EMERROR_INVALADDRESSSTATE   0x0002  // Function failed because address not in correct state
#define EMERROR_INVALADPARAM        0x0003  // Invalid parameter passed on function request
#define EMERROR_INVALIDFUNCTION     0x0004  // Invalid function
#define EMERROR_NOPARKINFO          0x0005  // No park information available

// Define simple macro for errors                                      
#define MAKEERR(wAddress, wError) MAKELONG(wAddress,wError)                                      
#define GETERROR(dw)    HIWORD(dw)
#define GETADDRID(dw)   LOWORD(dw)

// Port for the socket interface
#define SOCKET_PORT 972

/*******************************************************************************/
// 
// EMVERSIONINFO
//
// This structure is passed with a EMRESULT_VERSIONINFO.
//
/*******************************************************************************/
typedef struct _tagEMVERSIONINFO
{
    DWORD dwVersion;                        // Version of the emulator
    char  szSwitchInfo[SWITCHINFO_SIZE+1];  // NULL-terminated SwitchInfo buffer

} EMVERSIONINFO, FAR* LPEMVERSIONINFO;

/*******************************************************************************/
// 
// EMADDRESSCHANGE
//
// This structure is passed with a EMRESULT_ADDRESSCHANGED
//
/*******************************************************************************/
typedef struct _tagEMADDRESSCHANGE
{                                        
    WORD wAddressID;                        // Address index (0-numAddress-1)
    WORD wNewState;                         // New state of address (ADDRESSSTATE_xxx)
    DWORD wStateInfo;                       // Additional information about state (xxxMODE_xxx)
    DWORD dwMediaModes;                     // Potential media mode(s) (0 if not changed).

} EMADDRESSCHANGE, FAR* LPEMADDRESSCHANGE;

/*******************************************************************************/
// 
// EMLEVELCHANGE
//
// This structure is passed back with a EMRESULT_LEVELCHANGED
//
/*******************************************************************************/
typedef struct _tagEMLEVELCHANGE
{
    WORD wLevelType;                        // Type of level change (LEVELTYPE_xxx)
    WORD wLevel;                            // New value (0-ffff)

} EMLEVELCHANGE, FAR* LPEMLEVELCHANGE;

/*******************************************************************************/
// 
// EMDIGITBUFF
//
// This structure is passed back with a EMRESULT_DIGITDETECTED
//
/*******************************************************************************/
typedef struct _tagEMDIGITBUFF
{
    WORD wAddressID;                        // Address index (0-numAddress-1)
    WORD fDTMF;                             // TRUE if it is a tone (DTMF) frequency.
    char cDigit;                            // Digit detected (0=silence, '0'-'9','A'-'D','#*').
    
} EMDIGITBUFF, FAR* LPEMDIGITBUFF;

/*******************************************************************************/
// 
// EMTONEBUFF
//
// This structure is passed back with a EMRESULT_TONEDETECTED
//
/*******************************************************************************/
typedef struct _tagEMTONEBUFF
{
    WORD wAddressID;                        // Address index (0-numAddress-1)
    DWORD dwFreq[3];                        // Frequency of tone generated
    
} EMTONEBUFF, FAR* LPEMTONEBUFF;

/*******************************************************************************/
// 
// EMCALLERID
//
// This structure is passed with a EMRESULT_CALLERID.
//
/*******************************************************************************/
typedef struct _tagEMCALLERID
{
    WORD wAddressID;                        // Address identifier caller id is on
    char szAddressInfo[ADDRESS_SIZE+1];     // Address information
    char szName[OWNER_SIZE+1];              // Name information
    
} EMCALLERID, FAR* LPEMCALLERID;

/*******************************************************************************/
// 
// EMLAMPCHANGE
//
// This structure is passed with a EMRESULT_LAMPCHANGED.
//
/*******************************************************************************/
typedef struct _tagLAMPCHANGE
{
    WORD wButtonLampID;                     // Button Lamp identifier (setup by emulator)
    WORD wLampState;                        // New lamp state (LAMPSTATE_xxx)

} EMLAMPCHANGE, FAR* LPEMLAMPCHANGE;

/*******************************************************************************/
// 
// EMBUTTONCHANGE
//
// This structure is passed with a EMRESULT_BUTTONCHANGED.
//
/*******************************************************************************/
typedef struct _tagBUTTONCHANGE
{
    WORD wButtonLampID;                     // Button Lamp identifier (setup by emulator)
    WORD wButtonState;                      // New button state (BUTTONSTATE_xxx)

} EMBUTTONCHANGE, FAR* LPEMBUTTONCHANGE;

/*******************************************************************************/
// 
// EMHOOKSWITCHCHANGE
//
// This structure is passed with a EMRESULT_HOOKSWITCHCHANGED
//
/*******************************************************************************/
typedef struct _tagEMHOOKSWITCHCHANGE
{
    WORD wHookswitchID;                     // Hookswitch ID (HSDEVICE_xxxx)
    WORD wHookswitchState;                  // Hookswitch state (HSSTATE_xxx)

} EMHOOKSWITCHCHANGE, FAR *LPEMHOOKSWITCHCHANGE;

/*******************************************************************************/
// 
// EMDISPLAY
//
// This structure is passed with a EMRESULT_DISPLAYCHANGED
//
/*******************************************************************************/
typedef struct _tagEMDISPLAY
{
    char szDisplay [DISPLAY_COLS*DISPLAY_ROWS+1];

} EMDISPLAY, FAR* LPEMDISPLAY;

/*******************************************************************************/
// 
// EMSETTINGS
//
// This structure is passed with a EMRESULT_SETTINGS
//
/*******************************************************************************/
typedef struct _tagEMSETTINGS
{
    WORD wAddressCount;                     // Number of addresses in emulator
    WORD wHandsetHookswitch;                // Hookswitch state for the handset. (HSSTATE_xxx)
    WORD wVolHandset;                       // Volume for the handset (0-0xffff)
    WORD wGainHandset;                      // Gain for the handset (0-0xffff)
    WORD wRingMode;                         // Current ringer mode
    WORD wLampStates[BUTTON_COUNT];         // All the lamp states (LAMPSTATE_xxx)
    WORD wButtonModes[BUTTON_COUNT];        // Setting for the button
    char szDisplay [DISPLAY_COLS*DISPLAY_ROWS+1]; // Initial display state
    char szOwner[OWNER_SIZE+1];             // Owner size (user name)

} EMSETTINGS, FAR* LPEMSETTINGS;

/*******************************************************************************/
// 
// EMADDRESSINFO
//
// This structure is passed with a EMRESULT_ADDRESSINFO.
//
/*******************************************************************************/
typedef struct _tagEMADDRESSINFO
{
    WORD wAddressID;                        // Address index (0-numAddresses-1)
    char szAddress[ADDRESS_SIZE+1];         // Dialable phone number for this address.
    WORD wAddressState;                     // Current state of the address
    WORD wStateInfo;                        // Extra state information
    DWORD dwMediaMode;                      // Media mode for the address

} EMADDRESSINFO, FAR* LPEMADDRESSINFO;

/*******************************************************************************/
//
// EMFORWARDINFO
//
// This is sent with a forwarding request from the service provider
//
/*******************************************************************************/
typedef struct _tagEMFORWARDINFO
{
    WORD wAddressID;                        // Address to forward (-1 = ALL)
    char szAddress[ADDRESS_SIZE+1];         // Address to forward TO (NULL=cancel)

} EMFORWARDINFO, FAR* LPEMFORWARDINFO;    

/*******************************************************************************/
// 
// EMCONFERENCEINFO
//
// This is sent with conferencing requests from the service provider.
//
/*******************************************************************************/
typedef struct _tagEMCONFERENCEINFO
{
    WORD wAddressID;                        // Address to work conference off of.
    WORD wConfAddress;                      // Active/Connect/Online call to conference into (-1=none)
    WORD wCommand;                          // Command (ADD, REMOVE)
    
} EMCONFERENCEINFO, FAR* LPEMCONFERENCEINFO;

/*******************************************************************************/
// 
// EMPARKINFO
//
// Call parking information
//
/*******************************************************************************/
typedef struct _tagEMPARKINFO
{                               
    WORD wAddressID;                        // Address to perform park/unpark
    char szAddress[ADDRESS_SIZE+1];         // Address to park/unpark from.
    
} EMPARKINFO, FAR* LPEMPARKINFO;

/*******************************************************************************/
// 
// EMCOMPLETECALL
//
// Call completion information structure passed with COMMAND_COMPLETECALL
//
/*******************************************************************************/
typedef struct _tagEMCOMPLETECALL
{
    WORD wAddressID;                        // Address to work conference off of.
    WORD wCompletionType;                   // Completion Type (CALLCOMPLTYPE_xxx)
    char szMessage[COMPLMESSAGE_SIZE+1];    // Message for CALLCOMPLTYPE_MESSAGE.
    
} EMCOMPLETECALL, FAR* LPEMCOMPLETECALL;

/*******************************************************************************/
//
// EMTRANSFERINFO
//
// This is sent with a transfer request from the service provider
//
/*******************************************************************************/
typedef struct _tagEMTRANSFERINFO
{
    WORD wAddressID;                        // Address to transfer                       
    WORD wTransferAddress;                  // Active/Connect/Online call to transfer to (-1=none)
    WORD fMoveToConference;                 // Transition to a conference state
    char szAddress[ADDRESS_SIZE+1];         // Address to transfer TO (NULL=consultation)
    
} EMTRANSFERINFO, FAR* LPEMTRANSFERINFO;    

#ifndef RC_INVOKED
#pragma pack()          // Revert to original packing
#endif 

#endif // __EMINT_INC__
