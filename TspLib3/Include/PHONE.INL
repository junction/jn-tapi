/******************************************************************************/
//                                                                        
// PHONE.INL - TAPI Service Provider C++ Library header                     
//      
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// The SPLIB classes provide a basis for developing MS-TAPI complient     
// Service Providers.  They provide basic handling for all of the TSPI    
// APIs and a C-based handler which routes all requests through a set of C++     
// classes.                                                                 
//              
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// INLINE FUNCTIONS
//                                                           
/******************************************************************************/

#ifndef _SPPHONE_INL_INC_
#define _SPPHONE_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPIPhoneConnection
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPhoneHandle
//
// Return the TAPI phone handle associated with this object
//
TSP_INLINE HTAPIPHONE CTSPIPhoneConnection::GetPhoneHandle() const
{                                       
    return m_htPhone;

}// CTSPIPhoneConnection::GetPhoneHandle

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetButtonCount
//
// Returns the number of button/lamp pairs in our array
//
TSP_INLINE unsigned int CTSPIPhoneConnection::GetButtonCount() const
{                      
    return m_PhoneCaps.dwNumButtonLamps;

}// CTSPIPhoneConnection::GetButtonCount

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetButtonInfo
//
// Return the button object for the specified button.
//
TSP_INLINE const CPhoneButtonInfo* CTSPIPhoneConnection::GetButtonInfo(unsigned int i) const
{   
	CEnterCode sLock(this);  // Synch access to object
	return (i < m_arrButtonInfo.size()) ? m_arrButtonInfo[i] : NULL;

}// CTSPIPhoneConnection::GetButtonInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetWButtonInfo
//
// Return the button object for the specified button (writable)
//
TSP_INLINE CPhoneButtonInfo* CTSPIPhoneConnection::GetWButtonInfo(unsigned int i)
{   
	CEnterCode sLock(this);  // Synch access to object
	return (i < m_arrButtonInfo.size()) ? m_arrButtonInfo[i] : NULL;

}// CTSPIPhoneConnection::GetWButtonInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPhoneCaps
//
// Return a pointer to the phone capabilities
//
TSP_INLINE LPPHONECAPS CTSPIPhoneConnection::GetPhoneCaps()
{
    return &m_PhoneCaps;
    
}// CTSPIPhoneConnection::GetPhoneCaps

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPhoneStatus
//
// Return a pointer to the phone status
//
TSP_INLINE LPPHONESTATUS CTSPIPhoneConnection::GetPhoneStatus()
{                                       
    return &m_PhoneStatus;

}// CTSPIPhoneConnection::GetPhoneStatus

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPhoneCaps
//
// Return a pointer to the phone capabilities
//
TSP_INLINE const PHONECAPS* CTSPIPhoneConnection::GetPhoneCaps() const
{
    return &m_PhoneCaps;
    
}// CTSPIPhoneConnection::GetPhoneCaps

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPhoneStatus
//
// Return a pointer to the phone status
//
TSP_INLINE const PHONESTATUS* CTSPIPhoneConnection::GetPhoneStatus() const
{                                       
    return &m_PhoneStatus;

}// CTSPIPhoneConnection::GetPhoneStatus

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetRing
//
// Return the ring value for the phone device.
//
TSP_INLINE LONG CTSPIPhoneConnection::GetRing (LPDWORD lpdwRingMode, LPDWORD lpdwVolume) 
{                                
    *lpdwRingMode = m_PhoneStatus.dwRingMode;
    *lpdwVolume = m_PhoneStatus.dwRingVolume;
    return false;

}// CTSPIPhoneConnection::GetRing

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetDisplayBuffer
//
// Return the current display buffer as a string.  Each row is
// spaced out to the full size for our display.
//
TSP_INLINE TString CTSPIPhoneConnection::GetDisplayBuffer() const
{                                         
	CEnterCode sLock(this);  // Synch access to object
    return m_Display.GetTextBuffer();

}// CTSPIPhoneConnection::GetDisplayBuffer

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetCursorPos
//
// Return the cursor position of the display.
//
TSP_INLINE const POINT& CTSPIPhoneConnection::GetCursorPos() const
{
	CEnterCode sLock(this);  // Synch access to object
    return m_Display.GetCursorPosition();

}// CTSPIPhoneConnection::GetCursorPos

///////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetAssociatedLine
//
// Return the associated line device with this phone.
//
TSP_INLINE CTSPILineConnection* CTSPIPhoneConnection::GetAssociatedLine() const
{
	DEVICECLASSINFO* pDevInfo = GetDeviceClass(_T("tapi/line"));
	if (pDevInfo != NULL)
	{
		DWORD dwLineDeviceID = *(reinterpret_cast<LPDWORD>(pDevInfo->lpvData.get()));
		return GetDeviceInfo()->FindLineConnectionByDeviceID(dwLineDeviceID);
	}
	return NULL;

}// CTSPIPhoneConnection::GetAssociatedLine

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddDisplayChar
//
// Add a single character to our display at the specified position.
//
TSP_INLINE void CTSPIPhoneConnection::AddDisplayChar (TCHAR cChar)
{                                       
	CEnterCode sLock(this);  // Synch access to object
    m_Display.AddCharacter (cChar);
    OnPhoneStatusChange (PHONESTATE_DISPLAY);

}// CTSPIPhoneConnection::AddDisplayChar

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetDisplayChar
//
// Set a character at a specified position in the buffer to a vlue.
//
TSP_INLINE void CTSPIPhoneConnection::SetDisplayChar (int iColumn, int iRow, TCHAR cChar)
{                                       
	CEnterCode sLock(this);  // Synch access to object
    m_Display.SetCharacterAtPosition (iColumn, iRow, cChar);
    OnPhoneStatusChange (PHONESTATE_DISPLAY);

}// CTSPIPhoneConnection::SetDisplayChar

////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::ForceClose
//
// This method may be used by the service provider to forcibly close the
// phone object.
//
TSP_INLINE void CTSPIPhoneConnection::ForceClose()
{
	Send_TAPI_Event (PHONE_CLOSE);

}// CTSPIPhoneConnection::ForceClose

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::ResetDisplay
//
// Reset the display (cursor, buffer)
//
TSP_INLINE void CTSPIPhoneConnection::ResetDisplay()
{                                       
	CEnterCode sLock(this);  // Synch access to object
    m_Display.Reset();
    OnPhoneStatusChange (PHONESTATE_DISPLAY);

}// CTSPIPhoneConnection::ResetDisplay()

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetDisplayCursorPos
//
// Set the current cursor position for the display.
//
TSP_INLINE void CTSPIPhoneConnection::SetDisplayCursorPos (int iColumn, int iRow)
{                                            
	CEnterCode sLock(this);  // Synch access to object
    m_Display.SetCursorPosition (iColumn, iRow);
    OnPhoneStatusChange (PHONESTATE_DISPLAY);

}// CTSPIPhoneConnection::SetDisplayCursorPos

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::ClearDisplayLine
//
// Clear a single display line
//
TSP_INLINE void CTSPIPhoneConnection::ClearDisplayLine (int iRow)
{                                         
	CEnterCode sLock(this);  // Synch access to object
    m_Display.ClearRow (iRow);
    OnPhoneStatusChange (PHONESTATE_DISPLAY);

}// CTSPIPhoneConnection::ClearDisplayLine

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddDisplayString 
//
// Add a string to the display.
//
TSP_INLINE void CTSPIPhoneConnection::AddDisplayString (LPCTSTR lpszText)
{                                          
	CEnterCode sLock(this);  // Synch access to object
    m_Display.AddString (lpszText);
    OnPhoneStatusChange (PHONESTATE_DISPLAY);
    
}// CTSPIPhoneConnection::AddDisplayString 

////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetPermanentPhoneID
//
// Sets the permanent phone identifier for this phone.  This should
// be unique across the whole provider for any phone in the system.
//
TSP_INLINE void CTSPIPhoneConnection::SetPermanentPhoneID(DWORD dwPhoneID)
{
	GetDeviceInfo()->MapConnID(m_PhoneCaps.dwPermanentPhoneID, dwPhoneID, this);
    m_PhoneCaps.dwPermanentPhoneID = dwPhoneID;

}// CTSPILineConnection::SetPermanentLineID

////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetRequestMap
//
// Retrieves the private request map for phone objects. This may
// be overridden to provide phone-by-phone request management.
//
TSP_INLINE TRequestMap* CTSPIPhoneConnection::GetRequestMap() const
{
	return &CTSPIPhoneConnection::g_mapRequests;

}// CTSPIPhoneConnection::GetRequestMap

/******************************************************************************/
//
// CTSPIDisplay
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::GetCursorPosition
//
// Returns the maximum number of columns on display
//
TSP_INLINE const POINT& CPhoneDisplay::GetCursorPosition() const
{
    return m_ptCursor;

}// CPhoneDisplay::GetCursorPosition

///////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::GetDisplaySize
//
// Returns the size of our display.
//
TSP_INLINE const SIZE& CPhoneDisplay::GetDisplaySize() const
{
    return m_sizDisplay;

}// CPhoneDisplay::GetDisplaySize

///////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::GetLFChar
//
// Returns the CR/LF character.
//
TSP_INLINE TCHAR CPhoneDisplay::GetLFChar() const
{
	return m_cLF;

}// CPhoneDisplay::GetLFChar

////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::GetTextBuffer
//
// Return the contents of the screen buffer
//
TSP_INLINE const LPTSTR CPhoneDisplay::GetTextBuffer() const
{
    return m_lpsDisplay.get();

}// CPhoneDisplay::GetTextBuffer

////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::AddString
//
// Add a string to the display.
//
TSP_INLINE void CPhoneDisplay::AddString(LPCTSTR pszText)
{
    while (*pszText)
        AddCharacter (*pszText++);

}// CPhoneDisplay::AddString

/******************************************************************************/
//
// CPhoneButtonInfo
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::CPhoneButtonInfo
//
// Default constructor (protected)
//
TSP_INLINE CPhoneButtonInfo::CPhoneButtonInfo() : 
	m_dwButtonFunction(PHONEBUTTONFUNCTION_UNKNOWN), 
    m_dwButtonMode(PHONEBUTTONMODE_DUMMY),
    m_dwLampMode(PHONELAMPMODE_UNKNOWN),
    m_dwButtonState(PHONEBUTTONSTATE_UNKNOWN),
    m_dwAvailLampModes(PHONELAMPMODE_DUMMY)
{
}// CPhoneButtonInfo::CPhoneButtonInfo

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::CPhoneButtonInfo
//
// Parameter constructor
//
TSP_INLINE CPhoneButtonInfo::CPhoneButtonInfo(DWORD dwFunction, DWORD dwMode, 
			DWORD dwAvailLamp, DWORD dwLamp, LPCTSTR lpszDesc) : 
    m_dwButtonFunction(dwFunction), m_dwButtonMode(dwMode), 
	m_dwLampMode(dwLamp), m_dwAvailLampModes(dwAvailLamp),
    m_strButtonDescription(lpszDesc)
{
}// CPhoneButtonInfo::CPhoneButtonInfo

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::operator=
//
// This is an assignment operator from a TAPI PHONEBUTTONINFO structure
//
TSP_INLINE CPhoneButtonInfo& CPhoneButtonInfo::operator=(LPPHONEBUTTONINFO lpPhoneInfo)
{
	m_dwButtonMode = lpPhoneInfo->dwButtonMode;
	m_dwButtonFunction = lpPhoneInfo->dwButtonFunction;
    m_dwButtonState = lpPhoneInfo->dwButtonState;
	m_dwLampMode = 0L;
    m_dwAvailLampModes = 0;

	if (lpPhoneInfo->dwButtonTextSize > 0)   
		m_strButtonDescription = reinterpret_cast<LPCTSTR>(lpPhoneInfo) + lpPhoneInfo->dwButtonTextOffset;
	else
		m_strButtonDescription.erase();
	return *this;

}// CPhoneButtonInfo::operator=

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::SetButtonInfo
//
// Set the button information when it changes.
//
TSP_INLINE void CPhoneButtonInfo::SetButtonInfo (DWORD dwFunction, DWORD dwMode, LPCTSTR lpszDesc)
{
    m_dwButtonFunction = dwFunction;
    m_dwButtonMode = dwMode;
    m_strButtonDescription = lpszDesc;

}// CPhoneButtonInfo::SetButtonInfo

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::SetButtonState
//
// Set the pressed state of the button.
//
TSP_INLINE void CPhoneButtonInfo::SetButtonState (DWORD dwButtonState)
{                                   
    m_dwButtonState = dwButtonState;

}// CPhoneButtonInfo::SetButtonState

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetButtonState
//                                 
// Return the current button state
//
TSP_INLINE DWORD CPhoneButtonInfo::GetButtonState() const
{                                   
    return m_dwButtonState;

}// CPhoneButtonInfo::GetButtonState

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetDescription
//
// Return the description of the button.
//
TSP_INLINE LPCTSTR CPhoneButtonInfo::GetDescription() const
{
    return m_strButtonDescription.c_str();
    
}// CPhoneButtonInfo::GetDescription

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetFunction
//
// Return the button function
//
TSP_INLINE DWORD CPhoneButtonInfo::GetFunction() const
{
    return m_dwButtonFunction;
    
}// CPhoneButtonInfo::GetFunction

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetButtonMode
//
// Return the button mode
//
TSP_INLINE DWORD CPhoneButtonInfo::GetButtonMode() const
{
    return m_dwButtonMode;
    
}// CPhoneButtonInfo::GetButtonMode

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetLampMode
//
// Return the lamp mode (state)
//
TSP_INLINE DWORD CPhoneButtonInfo::GetLampMode() const
{
    return m_dwLampMode;
    
}// CPhoneButtonInfo::GetLampMode

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::GetAvailLampModes
//
// Return the available lamp modes for this button.
//
TSP_INLINE DWORD CPhoneButtonInfo::GetAvailLampModes() const
{                                      
    return m_dwAvailLampModes;
    
}// CPhoneButtonInfo::GetAvailLampModes

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::SetLampMode
//
// Set the current lamp state
//
TSP_INLINE void CPhoneButtonInfo::SetLampMode(DWORD dwLampMode)
{   
    _TSP_ASSERTE (m_dwAvailLampModes & dwLampMode);
    m_dwLampMode = dwLampMode;
    
}// CPhoneButtonInfo::SetLampMode

/******************************************************************************/
//
// RTSetButtonInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::RTSetButtonInfo
//
// Constructor for the phoneSetButtonInfo request
//
TSP_INLINE RTSetButtonInfo::RTSetButtonInfo(CTSPIPhoneConnection* pPhone, 
					DRV_REQUESTID dwRequestID, DWORD dwButtonLampID, LPPHONEBUTTONINFO lpbi) :
    CTSPIRequest(_T("phoneSetButtonInfo"))
{
	CTSPIRequest::Init(pPhone, REQUEST_SETBUTTONINFO, dwRequestID);
	m_dwButtonID = dwButtonLampID;
	m_lpbi = lpbi;

}// RTSetButtonInfo::RTSetButtonInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetButtonLampID
//
// Return the button/lamp id in question
//
TSP_INLINE DWORD RTSetButtonInfo::GetButtonLampID() const
{
	return m_dwButtonID;

}// RTSetButtonInfo::GetButtonLampID

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetButtonFunction
//
// Return the button function from our PHONEBUTTONINFO structure
//
TSP_INLINE DWORD RTSetButtonInfo::GetButtonFunction() const
{
	if (m_lpbi != NULL)
		return m_lpbi->dwButtonFunction;
	return PHONEBUTTONFUNCTION_UNKNOWN;

}// RTSetButtonInfo::GetButtonFunction

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetButtonMode
//
// Return the button mode from our PHONEBUTTONINFO structure
//
TSP_INLINE DWORD RTSetButtonInfo::GetButtonMode() const
{
	if (m_lpbi != NULL)
		return m_lpbi->dwButtonMode;
	return PHONEBUTTONMODE_DUMMY;

}// RTSetButtonInfo::GetButtonMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetButtonText
//
// Return the button text from our PHONEBUTTONINFO structure
//
TSP_INLINE LPCTSTR RTSetButtonInfo::GetButtonText() const
{
	if (m_lpbi != NULL && m_lpbi->dwButtonTextSize > 0)
		return reinterpret_cast<LPCTSTR>(reinterpret_cast<LPBYTE>(m_lpbi) + m_lpbi->dwButtonTextOffset);
	return NULL;

}// RTSetButtonInfo::GetButtonText

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetDevSpecificInfo
//
// Return the device specific block from our PHONEBUTTONINFO structure
//
TSP_INLINE LPVOID RTSetButtonInfo::GetDevSpecificInfo()
{
	if (m_lpbi != NULL && m_lpbi->dwDevSpecificSize > 0)
		return reinterpret_cast<LPVOID>(reinterpret_cast<LPBYTE>(m_lpbi) + m_lpbi->dwDevSpecificOffset);
	return NULL;

}// RTSetButtonInfo::GetDevSpecificInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetButtonInfo::GetButtonInfo
//
// Return the device specific block from our PHONEBUTTONINFO structure
//
TSP_INLINE LPPHONEBUTTONINFO RTSetButtonInfo::GetButtonInfo()
{
	return m_lpbi;

}// RTSetButtonInfo::GetButtonInfo

/******************************************************************************/
//
// RTSetLampInfo
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLampInfo::RTSetLampInfo
//
// Constructor for the phoneSetButtonInfo request
//
TSP_INLINE RTSetLampInfo::RTSetLampInfo(CTSPIPhoneConnection* pPhone, 
			DRV_REQUESTID dwRequestID, DWORD dwButtonLampID, DWORD dwMode) :
    CTSPIRequest(_T("phoneSetLampInfo"))
{
	CTSPIRequest::Init(pPhone, REQUEST_SETLAMP, dwRequestID);
	m_dwLampID = dwButtonLampID;
	m_dwMode = dwMode;

}// RTSetLampInfo::RTSetLampInfo

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLampInfo::GetButtonLampID
//
// Return the button/lamp id in question
//
TSP_INLINE DWORD RTSetLampInfo::GetButtonLampID() const
{
	return m_dwLampID;

}// RTSetLampInfo::GetButtonLampID

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetLampInfo::GetLampMode
//
// Return the lamp mode
//
TSP_INLINE DWORD RTSetLampInfo::GetLampMode() const
{
	return m_dwMode;

}// RTSetLampInfo::GetLampMode

/******************************************************************************/
//
// RTSetRing
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetRing::RTSetRing
//
// Constructor for the phoneSetButtonInfo request
//
TSP_INLINE RTSetRing::RTSetRing(CTSPIPhoneConnection* pPhone, 
			DRV_REQUESTID dwRequestID, DWORD dwRingMode, DWORD dwVolume) :
    CTSPIRequest(_T("phoneSetRing"))
{
	CTSPIRequest::Init(pPhone, REQUEST_SETRING, dwRequestID);
	m_dwRingMode = dwRingMode;
	m_dwVolume = dwVolume;

}// RTSetRing::RTSetRing

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetRing::GetRingMode
//
// Return the ringer mode.
//
TSP_INLINE DWORD RTSetRing::GetRingMode() const
{
	return m_dwRingMode;

}// RTSetRing::GetRingMode

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetRing::GetVolume
//
// Return the ringer volume
//
TSP_INLINE DWORD RTSetRing::GetVolume() const
{
	return m_dwVolume;

}// RTSetRing::GetVolume

/******************************************************************************/
//
// RTSetDisplay
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetDisplay::RTSetDisplay
//
// Constructor for the phoneSetDisplay request
//
TSP_INLINE RTSetDisplay::RTSetDisplay(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, 
				unsigned int Row, unsigned int Column, TCHAR* pszDisplay, DWORD dwSize) : 
    CTSPIRequest(_T("phoneSetDisplay")), m_iColumn(Column), m_iRow(Row)
{
	CTSPIRequest::Init(pPhone, REQUEST_SETDISPLAY, dwRequestID);
	m_Display.SetPtr(pszDisplay, dwSize);

}// RTSetDisplay::RTSetDisplay

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetDisplay::GetColumn
//
// Return the column to set data at
//
TSP_INLINE unsigned int RTSetDisplay::GetColumn() const
{
	return m_iColumn;

}// RTSetDisplay::GetColumn

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetDisplay::GetRow
//
// Return the row to set data at
//
TSP_INLINE unsigned int RTSetDisplay::GetRow() const
{
	return m_iRow;

}// RTSetDisplay::GetRow

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetDisplay::GetBufferSize
//
// Return the size of the display buffer
//
TSP_INLINE DWORD RTSetDisplay::GetBufferSize() const
{
	return m_Display.GetSize();

}// RTSetDisplay::GetBufferSize

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetDisplay::GetBufferPtr
//
// Return the ptr to the display buffer to set
//
TSP_INLINE LPCTSTR RTSetDisplay::GetBufferPtr() const
{
	return static_cast<LPCTSTR>(m_Display.GetPtr());

}// RTSetDisplay::GetBufferPtr

/******************************************************************************/
//
// RTSetGain
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetGain::RTSetGain
//
// Constructor for the phoneSetGain request
//
TSP_INLINE RTSetGain::RTSetGain(CTSPIPhoneConnection* pPhone, 
				   DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwGain) : 
    CTSPIRequest(_T("phoneSetGain")),
	m_dwDevice(dwHookswitch), m_dwGain(dwGain)
{
	CTSPIRequest::Init(pPhone, REQUEST_SETHOOKSWITCHGAIN, dwRequestID);

}// RTSetGain::RTSetGain

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetGain::GetHookswitchDevice
//
// Return the hookswitch device this request refers to.
//
TSP_INLINE DWORD RTSetGain::GetHookswitchDevice() const
{
	return m_dwDevice;

}// RTSetDisplay::GetHookswitchDevice

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetGain::GetGain
//
// Return the gain
//
TSP_INLINE DWORD RTSetGain::GetGain() const
{
	return m_dwGain;

}// RTSetGain::GetGain

/******************************************************************************/
//
// RTSetVolume
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetVolume::RTSetVolume
//
// Constructor for the phoneSetGain request
//
TSP_INLINE RTSetVolume::RTSetVolume(CTSPIPhoneConnection* pPhone, 
				   DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwVolume) : 
    CTSPIRequest(_T("phoneSetVolume")),
	m_dwDevice(dwHookswitch), m_dwVolume(dwVolume)
{
	CTSPIRequest::Init(pPhone, REQUEST_SETHOOKSWITCHVOL, dwRequestID);

}// RTSetVolume::RTSetVolume

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetVolume::GetHookswitchDevice
//
// Return the hookswitch device this request refers to.
//
TSP_INLINE DWORD RTSetVolume::GetHookswitchDevice() const
{
	return m_dwDevice;

}// RTSetDisplay::GetHookswitchDevice

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetVolume::GetVolume
//
// Return the volume 
//
TSP_INLINE DWORD RTSetVolume::GetVolume() const
{
	return m_dwVolume;

}// RTSetVolume::GetVolume

/******************************************************************************/
//
// RTSetHookswitch
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetHookswitch::RTSetHookswitch
//
// Constructor for the phoneSetGain request
//
TSP_INLINE RTSetHookswitch::RTSetHookswitch(CTSPIPhoneConnection* pPhone, 
				   DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwState) : 
    CTSPIRequest(_T("phoneSetHookswitch")),
	m_dwDevice(dwHookswitch), m_dwState(dwState)
{
	CTSPIRequest::Init(pPhone, REQUEST_SETHOOKSWITCH, dwRequestID);

}// RTSetHookswitch::RTSetHookswitch

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetHookswitch::GetHookswitchDevice
//
// Return the hookswitch device this request refers to.
//
TSP_INLINE DWORD RTSetHookswitch::GetHookswitchDevice() const
{
	return m_dwDevice;

}// RTSetDisplay::GetHookswitchDevice

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetHookswitch::GetHookswitchState
//
// Return the new state of the hookswitch device
//
TSP_INLINE DWORD RTSetHookswitch::GetHookswitchState() const
{
	return m_dwState;

}// RTSetHookswitch::GetHookswitchState

/******************************************************************************/
//
// RTSetPhoneData
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetPhoneData::RTSetPhoneData
//
// Constructor for the phoneSetData request
//
TSP_INLINE RTSetPhoneData::RTSetPhoneData(CTSPIPhoneConnection* pPhone, 
				DRV_REQUESTID dwRequestID, DWORD dwIndex, LPVOID lpBuff, DWORD dwSize) : 
	CTSPIRequest(_T("phoneSetPhoneData")), m_dwIndex(dwIndex)
{
	CTSPIRequest::Init(pPhone, REQUEST_SETPHONEDATA, dwRequestID);
	m_lpData.SetPtr(lpBuff, dwSize);

}// RTSetPhoneData::RTSetPhoneData

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetPhoneData::GetDownloadIndex
//
// Return the download index to use with the phone request
//
TSP_INLINE DWORD RTSetPhoneData::GetDownloadIndex() const
{
	return m_dwIndex;

}// RTSetDisplay::GetDownloadIndex

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetPhoneData::GetSize
//
// Return the size of the download buffer
//
TSP_INLINE DWORD RTSetPhoneData::GetSize() const
{
	return m_lpData.GetSize();

}// RTSetPhoneData::GetSize

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTSetPhoneData::GetBuffer
//
// Return the buffer to set into the download buffer on the phone
//
TSP_INLINE LPCVOID RTSetPhoneData::GetBuffer() const
{
	return m_lpData.GetPtr();

}// RTSetPhoneData::GetBuffer

/******************************************************************************/
//
// RTGetPhoneData
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGetPhoneData::RTGetPhoneData
//
// Constructor for the phoneSetData request
//
TSP_INLINE RTGetPhoneData::RTGetPhoneData(CTSPIPhoneConnection* pPhone, 
				   DWORD dwIndex, LPVOID lpBuff, DWORD dwSize) : 
    CTSPIRequest(_T("phoneGetPhoneData")),
	m_dwIndex(dwIndex), m_lpBuff(lpBuff), m_dwSize(dwSize)
{
	CTSPIRequest::Init(pPhone, REQUEST_GETPHONEDATA, 0);

}// RTGetPhoneData::RTGetPhoneData

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGetPhoneData::GetUploadIndex
//
// Return the upload index to use with the phone request
//
TSP_INLINE DWORD RTGetPhoneData::GetUploadIndex() const
{
	return m_dwIndex;

}// RTSetDisplay::GetUploadIndex

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGetPhoneData::GetSize
//
// Return the size of the download buffer
//
TSP_INLINE DWORD RTGetPhoneData::GetSize() const
{
	return m_dwSize;

}// RTGetPhoneData::GetSize

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RTGetPhoneData::GetBuffer
//
// Return the buffer to set into the download buffer on the phone
//
TSP_INLINE LPVOID RTGetPhoneData::GetBuffer()
{
	return m_lpBuff;

}// RTGetPhoneData::GetBuffer

#endif // _SPPHONE_INL_INC_
