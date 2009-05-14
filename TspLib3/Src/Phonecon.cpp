/******************************************************************************/
//                                                                        
// PHONECONN.CPP - Source code for the CTSPIPhoneConnection object        
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code to manage the phone objects which are  
// held by the CTSPIDevice.                                               
//                                                                        
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <spbstrm.h>

/*----------------------------------------------------------------------------
    Request map
-----------------------------------------------------------------------------*/
TRequestMap CTSPIPhoneConnection::g_mapRequests;

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::CTSPIPhoneConnection
//
// Constructor for the phone connection device
//
CTSPIPhoneConnection::CTSPIPhoneConnection() : CTSPIConnection(),
	m_lpfnEventProc(NULL), m_htPhone(0), 
	m_dwPhoneStates(0), m_dwButtonModes(0), m_dwButtonStates(0)
{
    ZeroMemory(&m_PhoneCaps, sizeof(PHONECAPS));
    ZeroMemory(&m_PhoneStatus, sizeof(PHONESTATUS));

}// CTSPIPhoneConnection::CTSPIPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::~CTSPIPhoneConnection
//
// Destructor for the phone connection device
//
CTSPIPhoneConnection::~CTSPIPhoneConnection()
{
	// Remove the id from the map
	if (m_dwDeviceID != 0xffffffff)
		GetSP()->RemoveConnectionFromMap(m_dwDeviceID, this);

}// CTSPIPhoneConnection::~CTSPIPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::BaseInit
//
// Initialize the basic phone connection information
//
void CTSPIPhoneConnection::BaseInit(CTSPIDevice* /*pDevice*/, DWORD /*dwPhoneID*/)
{
#ifdef _UNICODE
    m_PhoneCaps.dwStringFormat = STRINGFORMAT_UNICODE;
#else
    m_PhoneCaps.dwStringFormat = STRINGFORMAT_ASCII;
#endif
    
    // Add all the phone capabilities since we can notify TAPI about any of
    // these changing.  With some providers, they may not change, and that is O.K.
    m_PhoneCaps.dwPhoneStates = 
			(PHONESTATE_OTHER | 
			 PHONESTATE_CONNECTED | 
             PHONESTATE_DISCONNECTED | 
			 PHONESTATE_DISPLAY | 
			 PHONESTATE_LAMP |
             PHONESTATE_RINGMODE | 
			 PHONESTATE_RINGVOLUME | 
			 PHONESTATE_HANDSETHOOKSWITCH |
             PHONESTATE_HANDSETGAIN | 
			 PHONESTATE_SPEAKERHOOKSWITCH | 
			 PHONESTATE_SPEAKERGAIN |
             PHONESTATE_SPEAKERVOLUME | 
			 PHONESTATE_HANDSETVOLUME | 
			 PHONESTATE_HEADSETHOOKSWITCH |
             PHONESTATE_HEADSETVOLUME | 
			 PHONESTATE_HEADSETGAIN | 
			 PHONESTATE_SUSPEND |
             PHONESTATE_RESUME | 
			 PHONESTATE_CAPSCHANGE);
    
    // If the device supports more than one ring mode, then change this during INIT through
    // the GetPhoneCaps() API.
    m_PhoneCaps.dwNumRingModes = 1;                                 
    
    m_PhoneStatus.dwStatusFlags = PHONESTATUSFLAGS_CONNECTED;
    m_PhoneStatus.dwRingMode = 0L;
    m_PhoneStatus.dwRingVolume = 0xffff;

	// Add in the phone features.  Others will be added as the various
	// AddXXX functions are invoked.
	m_PhoneCaps.dwPhoneFeatures = 0;
	if (GetSP()->CanHandleRequest(TSPI_PHONEGETRING))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETRING;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETRING;
	}
	if (GetSP()->CanHandleRequest(TSPI_PHONESETRING))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETRING;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETRING;
	}

	// Add in the "tapi/phone" device class.
	AddDeviceClass (_T("tapi/phone"), GetDeviceID());
	AddDeviceClass (_T("tapi/providerid"), GetDeviceInfo()->GetProviderID());

}// CTSPIPhoneConnection::BaseInit

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::Init
//
// Initialize the phone connection object
//
void CTSPIPhoneConnection::Init(CTSPIDevice* pDevice, DWORD dwPhoneId, DWORD dwPos, DWORD_PTR /*dwItemData*/)
{
	// Call the basic connection init.
    CTSPIConnection::Init(pDevice, dwPhoneId);

    // Set our permanent line identifier which can be used to identify
	// this object uniquely within the scope of this service provider.
	SetPermanentPhoneID(MAKELONG((WORD)pDevice->GetProviderID(), dwPos+1));

	// Initialize all our information
	BaseInit(pDevice, dwPhoneId);

}// CTSPIPhoneConnection::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::InitWithStream
//
// Initialize the phone connection with a stream
//
void CTSPIPhoneConnection::InitWithStream(CTSPIDevice* pDevice, DWORD dwPhoneID, DWORD /*dwPos*/, TStream& istm)
{
	// Call the basic connection init.
    CTSPIConnection::Init(pDevice, dwPhoneID);

	// Initialize the basic phone information
	BaseInit(pDevice, dwPhoneID);

	// Read the permanent device id.
	DWORD dwPermanentPhoneID;
	istm >> dwPermanentPhoneID;
	SetPermanentPhoneID(dwPermanentPhoneID);

	// Read the associated line id
	DWORD dwLineID;
	istm >> dwLineID;
	if (dwLineID != 0xffffffff)
	{
		CTSPILineConnection* pLine = GetDeviceInfo()->FindLineConnectionByPermanentID(dwLineID);
		if (pLine != NULL)
		{
			pLine->AddDeviceClass(_T("tapi/phone"), GetDeviceID());
			AddDeviceClass(_T("tapi/line"), pLine->GetDeviceID());
		}
	}

	// Read the name of the phone
	istm >> m_strName;

	// Read the display information
	int cx=0, cy=0;
	unsigned int chDisplay=0;
	istm >> cx >> cy >> chDisplay;
	if (cx > 0 && cy > 0)
		SetupDisplay(cx, cy, static_cast<char>(chDisplay & 0xff));

	// Read in the buttons.
	unsigned int i, iCount=0;
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwMode, dwFunction, dwLampModes;
		TString strDescription;
		istm >> dwMode >> dwFunction >> dwLampModes >> strDescription;
		AddButton(dwFunction, dwMode, dwLampModes, 	(dwLampModes == PHONELAMPMODE_DUMMY) ? 
			0 : PHONELAMPMODE_UNKNOWN, strDescription.c_str());
	}

	// Read in the hookswitch devices
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwDevice, dwModes, dwVolume, dwGain, dwSetModes, dwMonModes;
		bool fVolChange=false, fGainChange=false;
		istm >> dwDevice >> dwModes >> dwVolume >> dwGain >> dwSetModes >> dwMonModes;
		istm >> fVolChange >> fGainChange;
		AddHookSwitchDevice(dwDevice, dwModes, PHONEHOOKSWITCHMODE_UNKNOWN, dwVolume, dwGain, dwSetModes, dwMonModes, fVolChange, fGainChange);
	}

	// Read the download buffers
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwBuffer;
		istm >> dwBuffer;
		AddDownloadBuffer(dwBuffer);
	}

	// Read the upload buffers
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwBuffer;
		istm >> dwBuffer;
		AddUploadBuffer(dwBuffer);
	}

	// Now allow the derived class to read the additional information
	// stored in the stream
	read(istm);

}// CTSPIPhoneConnection::InitWithStream

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SaveToStream
//
// Save the object to a stream
//
void CTSPIPhoneConnection::SaveToStream(TStream& ostm)
{
	// Get any associated line ID.
	DWORD dwLineID = 0xffffffff;
	DEVICECLASSINFO* pDevClass = GetDeviceClass(_T("tapi/line"));
	if (pDevClass != NULL && pDevClass->dwStringFormat == STRINGFORMAT_BINARY &&
		pDevClass->dwSize == sizeof(DWORD))
		dwLineID = *(reinterpret_cast<LPDWORD>(pDevClass->lpvData.get()));

	ostm << m_PhoneCaps.dwPermanentPhoneID << dwLineID << m_strName;

	// Write the display information
	unsigned int ch = m_Display.GetLFChar();
	ostm << m_Display.GetDisplaySize().cx << m_Display.GetDisplaySize().cy << ch;

	// Write out the button information
	unsigned int i, nCount = static_cast<unsigned int>(m_arrButtonInfo.size());
	ostm << nCount;
	for (i = 0; i < nCount; ++i)
	{
		CPhoneButtonInfo* pButton = m_arrButtonInfo[i];
		ostm << pButton->GetButtonMode() 
			 << pButton->GetFunction() 
			 << pButton->GetAvailLampModes()
			 << TString(pButton->GetDescription());
	}

	// Write out the hookswitch information
	nCount = 0;
	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HANDSET)
		++nCount;
	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HEADSET)
		++nCount;
	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_SPEAKER)
		++nCount;
	ostm << nCount;

	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HANDSET)
	{
		bool fSupportsVolumeChange = (m_PhoneCaps.dwVolumeFlags & PHONEHOOKSWITCHDEV_HANDSET) > 0;
        bool fSupportsGainChange = (m_PhoneCaps.dwGainFlags & PHONEHOOKSWITCHDEV_HANDSET) > 0;
		ostm << static_cast<DWORD>(PHONEHOOKSWITCHDEV_HANDSET)
			 << m_PhoneCaps.dwHandsetHookSwitchModes
			 << m_PhoneStatus.dwHandsetVolume
			 << m_PhoneStatus.dwHandsetGain
			 << m_PhoneCaps.dwSettableHandsetHookSwitchModes
			 << m_PhoneCaps.dwMonitoredHandsetHookSwitchModes
			 << fSupportsVolumeChange
			 << fSupportsGainChange;
	}
	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HEADSET)
	{
		bool fSupportsVolumeChange = (m_PhoneCaps.dwVolumeFlags & PHONEHOOKSWITCHDEV_HEADSET) > 0;
        bool fSupportsGainChange = (m_PhoneCaps.dwGainFlags & PHONEHOOKSWITCHDEV_HEADSET) > 0;
		ostm << static_cast<DWORD>(PHONEHOOKSWITCHDEV_HEADSET)
			 << m_PhoneCaps.dwHeadsetHookSwitchModes
			 << m_PhoneStatus.dwHeadsetVolume
			 << m_PhoneStatus.dwHeadsetGain
			 << m_PhoneCaps.dwSettableHeadsetHookSwitchModes
			 << m_PhoneCaps.dwMonitoredHeadsetHookSwitchModes
			 << fSupportsVolumeChange
			 << fSupportsGainChange;
	}
	if (m_PhoneCaps.dwHookSwitchDevs & PHONEHOOKSWITCHDEV_SPEAKER)
	{
		bool fSupportsVolumeChange = (m_PhoneCaps.dwVolumeFlags & PHONEHOOKSWITCHDEV_SPEAKER) > 0;
        bool fSupportsGainChange = (m_PhoneCaps.dwGainFlags & PHONEHOOKSWITCHDEV_SPEAKER) > 0;
		ostm << static_cast<DWORD>(PHONEHOOKSWITCHDEV_SPEAKER)
			 << m_PhoneCaps.dwSpeakerHookSwitchModes
			 << m_PhoneStatus.dwSpeakerVolume
			 << m_PhoneStatus.dwSpeakerGain
			 << m_PhoneCaps.dwSettableSpeakerHookSwitchModes
			 << m_PhoneCaps.dwMonitoredSpeakerHookSwitchModes
			 << fSupportsVolumeChange
			 << fSupportsGainChange;
	}

	// Write the download buffers
	nCount = static_cast<unsigned int>(m_arrDownloadBuffers.size());
	for (i = 0; i < nCount; ++i)
		ostm << m_arrDownloadBuffers[i];

	// Write the upload buffers
	nCount = static_cast<unsigned int>(m_arrUploadBuffers.size());
	for (i = 0; i < nCount; ++i)
		ostm << m_arrUploadBuffers[i];

	// Now allow the derived class to write any additional information.
	write(ostm);

}// CTSPIPhoneConnection::SaveToStream

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::read
//
// Reads the object from a stream - should be overriden to provide
// initialization support in the v3.x INIT process.
//
TStream& CTSPIPhoneConnection::read(TStream& istm)
{
	return istm;

}// CTSPUIPhoneConnection::read

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::write
//
// Writes the object to a stream - should be overriden to provide
// whatever additional information needs to be stored.
//
TStream& CTSPIPhoneConnection::write(TStream& istm) const
{
	return istm;

}// CTSPIPhoneConnection::write

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::FindButtonInfo
//
// Locate a specific button by mode/function
//
const CPhoneButtonInfo* CTSPIPhoneConnection::FindButtonInfo(DWORD dwFunction, DWORD dwMode) const
{
	CEnterCode sLock(this);
	CPhoneButtonArray::const_iterator iPos = std::find_if(m_arrButtonInfo.begin(), m_arrButtonInfo.end(), CPhoneButtonInfo::btn_pred_fm(dwFunction,dwMode));
	if (iPos != m_arrButtonInfo.end())
		return (*iPos);
	return NULL;

}// CTSPIPhoneConnection::FindButtonInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::FindButtonInfo
//
// Locate a specific button by text tag
//
const CPhoneButtonInfo* CTSPIPhoneConnection::FindButtonInfo(LPCTSTR pszText) const
{
	CEnterCode sLock(this);
	CPhoneButtonArray::const_iterator iPos = std::find_if(m_arrButtonInfo.begin(), m_arrButtonInfo.end(), CPhoneButtonInfo::btn_pred_name(pszText));
	if (iPos != m_arrButtonInfo.end())
		return (*iPos);
	return NULL;

}// CTSPIPhoneConnection::

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetPermanentDeviceID
//
// Return a permanent device id for this phone identifying the provider
// and phone.
//
DWORD CTSPIPhoneConnection::GetPermanentDeviceID() const
{
    return m_PhoneCaps.dwPermanentPhoneID;

}// CTSPIPhoneConnection::GetPermanentDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetupDisplay
//
// Setup the display device for this phone.  If there is no
// display, then don't call this function.
//
void CTSPIPhoneConnection::SetupDisplay (int iColumns, int iRows, char cLineFeed)
{                                     
	CEnterCode sLock(this);  // Synch access to object
    m_Display.Init (iColumns, iRows, cLineFeed);
    m_PhoneCaps.dwDisplayNumRows = iRows;
    m_PhoneCaps.dwDisplayNumColumns = iColumns;

	// Adjust our capabilities
	if (GetSP()->CanHandleRequest(TSPI_PHONEGETDISPLAY))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETDISPLAY;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETDISPLAY;
	}
	if (GetSP()->CanHandleRequest(TSPI_PHONESETDISPLAY))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETDISPLAY;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETDISPLAY;
	}

}// CTSPIPhoneConnection::SetupDisplay

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddUploadBuffer
//
// This function adds an upload data buffer to the phone.
//
bool CTSPIPhoneConnection::AddUploadBuffer (DWORD dwSize)
{
	_TSP_ASSERTE (GetSP()->CanHandleRequest(TSPI_PHONEGETDATA));

	CEnterCode sLock(this);  // Synch access to object

	// Add it to our buffer array.
	m_arrUploadBuffers.push_back(dwSize);
    m_PhoneCaps.dwNumGetData = m_arrUploadBuffers.size();
    
    // Adjust our phone features
	m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETDATA;
	m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETDATA;
   
	return true;
    
}// CTSPIPhoneConnection::AddUploadBuffer

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddDownloadBuffer
//
// This function adds a download data buffer to the phone.
//
bool CTSPIPhoneConnection::AddDownloadBuffer (DWORD dwSize)
{
	_TSP_ASSERTE (GetSP()->CanHandleRequest(TSPI_PHONESETDATA));

	CEnterCode sLock(this);  // Synch access to object

    // Add it to our buffer array.
	m_arrDownloadBuffers.push_back(dwSize);
    m_PhoneCaps.dwNumSetData = m_arrDownloadBuffers.size();

	// Adjust our phone features
	m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETDATA;
	m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETDATA;

    return true;
    
}// CTSPIPhoneConnection::AddDownloadBuffer

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddHookSwitchDevice
//
// Add a hookswitch device to our phone object.  This should be completed
// at INIT time (providerInit).    
//
// Pass in a (-1L) for volume/gain if volume/gain changes are not supported.
//
void CTSPIPhoneConnection::AddHookSwitchDevice (DWORD dwHookSwitchDev, 
					 DWORD dwAvailModes, DWORD dwCurrMode, DWORD dwVolume, DWORD dwGain,
					 DWORD dwSettableModes, DWORD dwMonitoredModes,
					 bool fSupportsVolumeChange, bool fSupportsGainChange)
{   
	// Default our volume and gain values
    if (dwVolume == 0xffffffff)
        dwVolume = 0xffff;
    if (dwGain == 0xffffffff)
        dwGain = 0xffff;            

	// Default the set/monitored modes
	if (dwSettableModes == -1L)
		dwSettableModes = dwAvailModes;
	if (dwMonitoredModes == -1L)
		dwMonitoredModes = dwAvailModes;

	// Synch access to object
	CEnterCode sLock(this);

    if (dwHookSwitchDev == PHONEHOOKSWITCHDEV_HANDSET)
    {
        m_PhoneCaps.dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_HANDSET;
		if (GetSP()->CanHandleRequest(TSPI_PHONEGETHOOKSWITCH))
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHHANDSET;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHHANDSET;
		}		
		if (GetSP()->CanHandleRequest (TSPI_PHONESETHOOKSWITCH) && dwSettableModes)
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHHANDSET;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHHANDSET;
		}
        if (fSupportsVolumeChange)
		{
            m_PhoneCaps.dwVolumeFlags |= PHONEHOOKSWITCHDEV_HANDSET;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMEHANDSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMEHANDSET;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMEHANDSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMEHANDSET;
			}
		}

        if (fSupportsGainChange)
		{
            m_PhoneCaps.dwGainFlags |= PHONEHOOKSWITCHDEV_HANDSET;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETGAINHANDSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETGAINHANDSET;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETGAINHANDSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETGAINHANDSET;
			}
		}
            
        m_PhoneCaps.dwHandsetHookSwitchModes = dwAvailModes;
		m_PhoneCaps.dwSettableHandsetHookSwitchModes = dwSettableModes;
		m_PhoneCaps.dwMonitoredHandsetHookSwitchModes = dwMonitoredModes;

        m_PhoneStatus.dwHandsetHookSwitchMode = dwCurrMode;
        m_PhoneStatus.dwHandsetVolume = dwVolume;
        m_PhoneStatus.dwHandsetGain = dwGain;
    }
    else if (dwHookSwitchDev == PHONEHOOKSWITCHDEV_SPEAKER)
    {
        m_PhoneCaps.dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_SPEAKER;
		if (GetSP()->CanHandleRequest(TSPI_PHONEGETHOOKSWITCH))
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHSPEAKER;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHSPEAKER;
		}
		if (GetSP()->CanHandleRequest (TSPI_PHONESETHOOKSWITCH) && dwSettableModes)
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHSPEAKER;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHSPEAKER;
		}

        if (fSupportsVolumeChange)
		{
            m_PhoneCaps.dwVolumeFlags |= PHONEHOOKSWITCHDEV_SPEAKER;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMESPEAKER;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMESPEAKER;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMESPEAKER;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMESPEAKER;
			}
		}

        if (fSupportsGainChange)
		{
            m_PhoneCaps.dwGainFlags |= PHONEHOOKSWITCHDEV_SPEAKER;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETGAINSPEAKER;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETGAINSPEAKER;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETGAINSPEAKER;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETGAINSPEAKER;
			}
		}
            
        m_PhoneCaps.dwSpeakerHookSwitchModes = dwAvailModes;
        m_PhoneStatus.dwSpeakerHookSwitchMode = dwCurrMode;
		m_PhoneCaps.dwSettableSpeakerHookSwitchModes = dwSettableModes;
		m_PhoneCaps.dwMonitoredSpeakerHookSwitchModes = dwMonitoredModes;

        m_PhoneStatus.dwSpeakerVolume = dwVolume;
        m_PhoneStatus.dwSpeakerGain = dwGain;
    }
    else if (dwHookSwitchDev == PHONEHOOKSWITCHDEV_HEADSET)
    {
        m_PhoneCaps.dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_HEADSET;
		if (GetSP()->CanHandleRequest(TSPI_PHONEGETHOOKSWITCH))
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHHEADSET;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHHEADSET;
		}
		if (GetSP()->CanHandleRequest (TSPI_PHONESETHOOKSWITCH) && dwSettableModes)
		{
			m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHHEADSET;
			m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETHOOKSWITCHHEADSET;
		}

        if (fSupportsVolumeChange)
		{
            m_PhoneCaps.dwVolumeFlags |= PHONEHOOKSWITCHDEV_HEADSET;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMEHEADSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETVOLUMEHEADSET;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETVOLUME))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMEHEADSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETVOLUMEHEADSET;
			}
		}

        if (fSupportsGainChange)
		{
            m_PhoneCaps.dwGainFlags |= PHONEHOOKSWITCHDEV_HEADSET;
			if (GetSP()->CanHandleRequest(TSPI_PHONEGETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETGAINHEADSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETGAINHEADSET;
			}
			if (GetSP()->CanHandleRequest (TSPI_PHONESETGAIN))
			{
				m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETGAINHEADSET;
				m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETGAINHEADSET;
			}
		}
            
        m_PhoneCaps.dwHeadsetHookSwitchModes = dwAvailModes;
        m_PhoneStatus.dwHeadsetHookSwitchMode = dwCurrMode;
		m_PhoneCaps.dwSettableHeadsetHookSwitchModes = dwSettableModes;
		m_PhoneCaps.dwMonitoredHeadsetHookSwitchModes = dwMonitoredModes;

        m_PhoneStatus.dwHeadsetVolume = dwVolume;
        m_PhoneStatus.dwHeadsetGain = dwGain;
    }
#ifdef _DEBUG
    else
        // Unsupported hookswitch device!
        _TSP_ASSERT (false);
#endif
                           
}// CTSPIPhoneConnection::AddHookSwitchDevice

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::AddButton
//
// Add a new button to our button array
//
bool CTSPIPhoneConnection::AddButton(DWORD dwFunction, DWORD dwMode, 
                           DWORD dwAvailLampModes, DWORD dwLampState, LPCTSTR lpszText)
{                                  
	CEnterCode sLock(this);  // Synch access to object
    m_PhoneCaps.dwNumButtonLamps++;

	// Adjust our phone features
	if (GetSP()->CanHandleRequest(TSPI_PHONEGETBUTTONINFO))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETBUTTONINFO;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETBUTTONINFO;
	}
	if (GetSP()->CanHandleRequest(TSPI_PHONEGETLAMP))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_GETLAMP;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_GETLAMP;
	}
	if (GetSP()->CanHandleRequest(TSPI_PHONESETBUTTONINFO))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETBUTTONINFO;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETBUTTONINFO;
	}
	if (GetSP()->CanHandleRequest(TSPI_PHONESETLAMP))
	{
		m_PhoneCaps.dwPhoneFeatures |= PHONEFEATURE_SETLAMP;
		m_PhoneStatus.dwPhoneFeatures |= PHONEFEATURE_SETLAMP;
	}

    // Add the button to our array
	m_arrButtonInfo.push_back(new CPhoneButtonInfo(dwFunction, dwMode, dwAvailLampModes, dwLampState, lpszText));

	return true;

}// CTSPIPhoneConnection::AddButton

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::Send_TAPI_Event
//
// Calls an event back into the TAPI DLL
//
void CTSPIPhoneConnection::Send_TAPI_Event(DWORD dwMsg, DWORD dwP1, DWORD dwP2, DWORD dwP3)
{
    _TSP_ASSERTE(m_lpfnEventProc != NULL);                                  
    
#ifdef _DEBUG
        static LPCTSTR g_pszMsgs[] = {
                {_T("Line_AddressState")},               // 0
                {_T("Line_CallInfo")},                   // 1
                {_T("Line_CallState")},                  // 2
                {_T("Line_Close")},                      // 3
                {_T("Line_DevSpecific")},                // 4
                {_T("Line_DevSpecificFeature")},         // 5
                {_T("Line_GatherDigits")},               // 6
                {_T("Line_Generate")},                   // 7
                {_T("Line_LineDevState")},               // 8
                {_T("Line_MonitorDigits")},              // 9
                {_T("Line_MonitorMedia")},               // 10
                {_T("Line_MonitorTone")},                // 11
                {_T("Line_Reply")},                      // 12
                {_T("Line_Request")},                    // 13
                {_T("Phone_Button")},                    // 14
                {_T("Phone_Close")},                     // 15
                {_T("Phone_DevSpecific")},               // 16
                {_T("Phone_Reply")},                     // 17
                {_T("Phone_State")},                     // 18
                {_T("Line_Create")},                     // 19
                {_T("Phone_Create")},                    // 20
				{_T("Line_AgentSpecific")},				 // 21
				{_T("Line_AgentStatus")},				 // 22
				{_T("Line_AppNewCall")},				 // 23
				{_T("Line_ProxyRequest")},				 // 24
				{_T("Line_Remove")},					 // 25
				{_T("Phone_Remove")}					 // 26
            };                
    _TSP_ASSERTE(dwMsg <= 26);
    _TSP_DTRACE(_T("Send_TAPI_Event: <0x%lx> Phone=0x%lx, Msg=0x%lx (%s), P1=0x%lx, P2=0x%lx, P3=0x%lx\n"),
                    this, GetPhoneHandle(), dwMsg, g_pszMsgs[dwMsg], dwP1, dwP2, dwP3);
#endif

    (*m_lpfnEventProc)(GetPhoneHandle(), dwMsg, dwP1, dwP2, dwP3);

}// CTSPIPhoneConnection::Send_TAPI_Event

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetButtonInfo
//
// This function returns information about the specified phone 
// button.
//
LONG CTSPIPhoneConnection::GetButtonInfo (DWORD dwButtonId, LPPHONEBUTTONINFO lpButtonInfo)
{   
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETBUTTONINFO) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // Get our button.
    const CPhoneButtonInfo* pButton = GetButtonInfo(dwButtonId);
    if (pButton == NULL)
        return PHONEERR_INVALBUTTONLAMPID;
        
    // Zero out the structure
    DWORD dwTotalSize = lpButtonInfo->dwTotalSize;
    ZeroMemory(lpButtonInfo, sizeof(PHONEBUTTONINFO));

    // Get the descriptive name if available.    
    TString strName = pButton->GetDescription();
    int cbSize = 0;
    if (!strName.empty())
        cbSize = (strName.length()+1) * sizeof(wchar_t);

    // Fill in the PHONEBUTTONINFO structure.  Do NOT touch
    // the total size since it is what TAPI set.
    lpButtonInfo->dwButtonMode = pButton->GetButtonMode();
    lpButtonInfo->dwButtonFunction = pButton->GetFunction();
	lpButtonInfo->dwButtonState = pButton->GetButtonState();

    // Set the data length
    lpButtonInfo->dwTotalSize = dwTotalSize;
    lpButtonInfo->dwNeededSize = sizeof(PHONEBUTTONINFO) + cbSize;
    lpButtonInfo->dwUsedSize = sizeof(PHONEBUTTONINFO);

    // Set the text description string if available.
	AddDataBlock (lpButtonInfo, lpButtonInfo->dwButtonTextOffset,
				  lpButtonInfo->dwButtonTextSize, strName.c_str());
    return false;
    
}// CTSPIPhoneConnection::GetButtonInfo   

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GatherCapabilities
//
// This function queries a specified phone device to determine its
// telephony capabilities
//
LONG CTSPIPhoneConnection::GatherCapabilities(DWORD dwTSPIVersion, DWORD /*dwExtVersion*/, LPPHONECAPS lpPhoneCaps)
{  
	// Synch access to object
	CEnterCode sLock(this);  

	// Grab all the pieces which are offset/sizes to add to the PHONECAPS
	// structure and determine the total size of the PHONECAPS buffer required
	// for ALL blocks of information.
    TString strPhoneName = GetName();
    TString strProviderInfo = GetSP()->GetProviderInfo();
    TString strPhoneInfo = GetDeviceInfo()->GetSwitchInfo();
    int cbName=0, cbInfo=0, cbPhoneInfo=0;
    int cbButton = (m_PhoneCaps.dwNumButtonLamps * sizeof(DWORD));
    int cbUpload = (m_PhoneCaps.dwNumGetData * sizeof(DWORD));
    int cbDownload = (m_PhoneCaps.dwNumSetData * sizeof(DWORD));

    if (!strPhoneName.empty())
        cbName = (strPhoneName.length()+1) * sizeof(wchar_t);
    if (!strProviderInfo.empty())
        cbInfo = (strProviderInfo.length()+1) * sizeof(wchar_t);
    if (!strPhoneInfo.empty())
        cbPhoneInfo = (strPhoneInfo.length()+1) * sizeof(wchar_t);

	// Get the length of the device classes we support.
	TString strDeviceNames;
	int cbDeviceNameLen = 0;
	for (TDeviceClassArray::const_iterator ii = m_arrDeviceClass.begin();
		 ii != m_arrDeviceClass.end(); ++ii)
	{
		DEVICECLASSINFO* pDevClass = (*ii).second;
		strDeviceNames += pDevClass->strName + _T('~');
	}

	// Add up the total length
	if (!strDeviceNames.empty())
	{
		strDeviceNames += _T('~');
		cbDeviceNameLen = (strDeviceNames.length()+1) * sizeof(wchar_t);
	}

    // Save off the sections that TAPI provides
    m_PhoneCaps.dwTotalSize = lpPhoneCaps->dwTotalSize;
    m_PhoneCaps.dwNeededSize = sizeof(PHONECAPS) + cbName + cbInfo + (cbButton*3) + 
							cbUpload + cbDownload + cbPhoneInfo + cbDeviceNameLen;

	// Determine what the minimum size required is based on the TSPI version
	// we negotiated to.
	DWORD dwReqSize = sizeof(PHONECAPS);
	if (dwTSPIVersion < TAPIVER_22)
		dwReqSize -= sizeof(GUID);
	if (dwTSPIVersion < TAPIVER_20)
		dwReqSize -= (9*sizeof(DWORD));

#ifdef _DEBUG
    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.  NOTE: This should never
	// happen - TAPI.DLL is supposed to verify that there is enough space in the structure
	// for the negotiated version and return an error.  We only check this in DEBUG builds
	// to insure that TAPI is doing its job.
    if (lpPhoneCaps->dwTotalSize < dwReqSize)
	{
		lpPhoneCaps->dwNeededSize = m_PhoneCaps.dwNeededSize;
		_TSP_ASSERT (false);
        return PHONEERR_STRUCTURETOOSMALL;
	}
#endif
    
    // Copy the phone capabilities over from our structure
    MoveMemory (lpPhoneCaps, &m_PhoneCaps, dwReqSize);
	lpPhoneCaps->dwUsedSize = dwReqSize;
    
    // Now add the phone name if we have the room
	AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwPhoneNameOffset, 
			lpPhoneCaps->dwPhoneNameSize, strPhoneName.c_str());
    
    // Add the phone information if we have the room.
	AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwPhoneInfoOffset, 
			lpPhoneCaps->dwPhoneInfoSize, strPhoneInfo.c_str());
    
    // Add the Provider information if we have the room
	AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwProviderInfoOffset, 
			lpPhoneCaps->dwProviderInfoSize, strProviderInfo.c_str());

    // Fill in the button information - mode first
    if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbButton)
    {
        for (unsigned int i = 0; i < GetButtonCount(); i++)
        {
			DWORD dwButtonMode = GetButtonInfo(i)->GetButtonMode();
			AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwButtonModesOffset, 
					lpPhoneCaps->dwButtonModesSize, &dwButtonMode, sizeof(DWORD));
		}
	}
    
    // Now the functions
    if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbButton)
    {
        for (unsigned int i = 0; i < GetButtonCount(); i++)
        {
			DWORD dwButtonFunction = GetButtonInfo(i)->GetFunction();
			AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwButtonFunctionsOffset,
					lpPhoneCaps->dwButtonFunctionsSize, &dwButtonFunction, sizeof(DWORD));			
        }
    }
    
    // Add the lamps
    if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbButton)
    {
        for (unsigned int i = 0; i < GetButtonCount(); i++)
        {
			DWORD dwLampMode = GetButtonInfo(i)->GetAvailLampModes();
			AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwLampModesOffset,
					lpPhoneCaps->dwLampModesSize, &dwLampMode, sizeof(DWORD));			
	    }
	}

    // Add the download buffer sizes.
    if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbDownload)
    {
        for (TDWordArray::iterator it = m_arrDownloadBuffers.begin(); it != m_arrDownloadBuffers.end(); ++it)
		{
			DWORD dwBuffer = (*it);
			AddDataBlock(lpPhoneCaps, lpPhoneCaps->dwSetDataOffset,
					lpPhoneCaps->dwSetDataSize, &dwBuffer, sizeof(DWORD));
		}
	}

    // Add the upload buffer sizes.
    if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbUpload)
    {
        for (TDWordArray::iterator it = m_arrUploadBuffers.begin(); it != m_arrUploadBuffers.end(); ++it)
		{
			DWORD dwBuffer = (*it);
			AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwGetDataOffset,
				lpPhoneCaps->dwGetDataSize, &dwBuffer, sizeof(DWORD));
		}
	}
    
	// If we negotiated to TAPI 2.0, then add the additional fields.
	if (dwTSPIVersion >= TAPIVER_20)
	{
		if (cbDeviceNameLen)
		{
			if (lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwUsedSize + cbDeviceNameLen)
			{
				AddDataBlock (lpPhoneCaps, lpPhoneCaps->dwDeviceClassesOffset,
					          lpPhoneCaps->dwDeviceClassesSize, strDeviceNames.c_str());
				// Remove the '~' chars and change to NULLs.
				wchar_t* lpBuff = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(lpPhoneCaps) + lpPhoneCaps->dwDeviceClassesOffset);
				_TSP_ASSERTE(lpBuff != NULL);
				do
				{
					if (*lpBuff == L'~')
						*lpBuff = L'\0';
					lpBuff++;

				} while (*lpBuff);
			}
		}
	}

    return false;                            
   
}// CTSPIPhoneConnection::GatherCapabilities

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GatherStatus
// 
// This method gathers the PHONESTATUS values for TAPI
//
LONG CTSPIPhoneConnection::GatherStatus (LPPHONESTATUS lpPhoneStatus)
{                                                          
	CEnterCode sLock(this);  // Synch access to object

	// Get the version we negotiated this phone at
	DWORD dwTSPIVersion = GetNegotiatedVersion();
	if (dwTSPIVersion == 0)
		dwTSPIVersion = GetSP()->GetSupportedVersion();

    // Save off the values which TAPI supplies.
    m_PhoneStatus.dwTotalSize = lpPhoneStatus->dwTotalSize;
    m_PhoneStatus.dwNumOwners = lpPhoneStatus->dwNumOwners;
    m_PhoneStatus.dwNumMonitors = lpPhoneStatus->dwNumMonitors;
    m_PhoneStatus.dwOwnerNameSize = lpPhoneStatus->dwOwnerNameSize;
    m_PhoneStatus.dwOwnerNameOffset = lpPhoneStatus->dwOwnerNameOffset;

	// If this is the first call, then set the phone features to be
	// what we determined during our INIT phase.
	if (m_PhoneStatus.dwPhoneFeatures == 0)
		m_PhoneStatus.dwPhoneFeatures = m_PhoneCaps.dwPhoneFeatures;

    // Now begin filling in our side.
	DWORD dwReqSize = sizeof (PHONESTATUS);
	if (dwTSPIVersion < TAPIVER_20)
		dwReqSize -= sizeof(DWORD);

    // Determine the space for the display.
    TString strDisplay = GetDisplayBuffer();
    int cbDisplay = 0;
    if (!strDisplay.empty())
        cbDisplay = (strDisplay.length()+1) * sizeof(wchar_t);
    int cbButton = GetButtonCount() * sizeof(DWORD);

    // Fill in the required size.
    m_PhoneStatus.dwNeededSize = dwReqSize+cbDisplay+cbButton;

#ifdef _DEBUG
    // If we don't have enough space based on our negotiated version, return an error and tell
	// TAPI how much we need for the full structure to come down.  NOTE: This should never
	// happen - TAPI.DLL is supposed to verify that there is enough space in the structure
	// for the negotiated version and return an error.  We only check this in DEBUG builds
	// to insure that TAPI is doing its job.
    if (lpPhoneStatus->dwTotalSize < dwReqSize)
	{
		lpPhoneStatus->dwNeededSize = m_PhoneStatus.dwNeededSize;
		_TSP_ASSERT (false);
        return PHONEERR_STRUCTURETOOSMALL;
	}
#endif

    // Copy over the basic PHONESTATUS structure
    MoveMemory (lpPhoneStatus, &m_PhoneStatus, dwReqSize);
	lpPhoneStatus->dwUsedSize = dwReqSize;
    
    // Copy the display information.
	if (!strDisplay.empty())
		AddDataBlock (lpPhoneStatus, lpPhoneStatus->dwDisplayOffset, 
				lpPhoneStatus->dwDisplaySize, const_cast<LPTSTR>(strDisplay.c_str()), 
				(strDisplay.length()+1) * sizeof(TCHAR));

    // Copy in the lamp mode information if enough space.
    if (cbButton && lpPhoneStatus->dwTotalSize >= lpPhoneStatus->dwUsedSize+cbButton)
    {
        for (unsigned int i = 0; i < GetButtonCount(); i++)
        {
			DWORD dwLampMode = GetButtonInfo(i)->GetLampMode();
			AddDataBlock (lpPhoneStatus, lpPhoneStatus->dwLampModesOffset,
				lpPhoneStatus->dwLampModesSize, &dwLampMode, sizeof(DWORD));
		}
	}
    
    return false;
   
}// CTSPIPhoneConnection::GatherStatus

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetDisplay
//
// Retrieve the display for the phone device into a VARSTRING
// buffer.
//
LONG CTSPIPhoneConnection::GetDisplay (LPVARSTRING lpVarString)
{
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETDISPLAY) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    TString strDisplay = GetDisplayBuffer();

    lpVarString->dwStringFormat = m_PhoneCaps.dwStringFormat;
    lpVarString->dwNeededSize = sizeof(VARSTRING);
    lpVarString->dwUsedSize = sizeof(VARSTRING);
    lpVarString->dwStringSize = 0;
    lpVarString->dwStringOffset = 0;
    
    if (!strDisplay.empty())
		AddDataBlock (lpVarString, lpVarString->dwStringOffset, 
					  lpVarString->dwStringSize, const_cast<LPTSTR>(strDisplay.c_str()), 
					  (strDisplay.length()+1) * sizeof(TCHAR));
            
    return false;
    
}// CTSPIPhoneConnection::GetDisplay

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetGain
//
// Return the current gain value for one of our hookswitch devices.
//
LONG CTSPIPhoneConnection::GetGain (DWORD dwHookSwitchDevice, LPDWORD lpdwGain)
{   
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_GETGAINHANDSET|
			PHONEFEATURE_GETGAINHEADSET|PHONEFEATURE_GETGAINSPEAKER)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
    
    // If we don't support the hook switch device.                         
    if ((m_PhoneCaps.dwHookSwitchDevs & dwHookSwitchDevice) != dwHookSwitchDevice)
        return PHONEERR_INVALHOOKSWITCHDEV;
    
    // The hook switch must be only a single bit.
    switch (dwHookSwitchDevice)
    {
        case PHONEHOOKSWITCHDEV_HANDSET:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETGAINHANDSET) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwGain = m_PhoneStatus.dwHandsetGain;
            break;
        case PHONEHOOKSWITCHDEV_HEADSET:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETGAINHEADSET) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwGain = m_PhoneStatus.dwHeadsetGain;
            break;
        case PHONEHOOKSWITCHDEV_SPEAKER:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETGAINSPEAKER) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwGain = m_PhoneStatus.dwSpeakerGain;
            break;
        default:
            return PHONEERR_INVALHOOKSWITCHDEV;
    }       
    return false;

}// CTSPIPhoneConnection::GetGain

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetVolume
//
// Return the volume level for a specified hook switch device.  If
// the hookswitch device is not supported, return an error.
//
LONG CTSPIPhoneConnection::GetVolume (DWORD dwHookSwitchDevice, LPDWORD lpdwVolume)
{                                  
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_GETVOLUMEHANDSET|
			PHONEFEATURE_GETVOLUMEHEADSET|PHONEFEATURE_GETVOLUMESPEAKER)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // If we don't support the hook switch device.                         
    if ((m_PhoneCaps.dwHookSwitchDevs & dwHookSwitchDevice) != dwHookSwitchDevice)
        return PHONEERR_INVALHOOKSWITCHDEV;
    
    // The hook switch must be only a single bit.
    switch (dwHookSwitchDevice)
    {
        case PHONEHOOKSWITCHDEV_HANDSET:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETVOLUMEHANDSET) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwVolume = m_PhoneStatus.dwHandsetVolume;
            break;
        case PHONEHOOKSWITCHDEV_HEADSET:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETVOLUMEHANDSET) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwVolume = m_PhoneStatus.dwHeadsetVolume;
            break;
        case PHONEHOOKSWITCHDEV_SPEAKER:
			if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETVOLUMEHANDSET) == 0)
				return PHONEERR_OPERATIONUNAVAIL;
            *lpdwVolume = m_PhoneStatus.dwSpeakerVolume;
            break;
        default:
            return PHONEERR_INVALHOOKSWITCHDEV;
    }       
    return false;

}// CTSPIPhoneConnection::GetVolume

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetHookSwitch
//
// This method returns the current hook switch mode for all our
// hookswitch devices.  Each device which is offhook gets a bit
// set in the field representing handset/speaker/headset.
//
LONG CTSPIPhoneConnection::GetHookSwitch (LPDWORD lpdwHookSwitch)
{                                      
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_GETHOOKSWITCHHANDSET|
			PHONEFEATURE_GETHOOKSWITCHSPEAKER|PHONEFEATURE_GETHOOKSWITCHHEADSET)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
	
    *lpdwHookSwitch = 0L; // All onhook.
    if (m_PhoneStatus.dwHandsetHookSwitchMode & 
            (PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_MIC | PHONEHOOKSWITCHMODE_SPEAKER))
        *lpdwHookSwitch |= PHONEHOOKSWITCHDEV_HANDSET;
    if (m_PhoneStatus.dwHeadsetHookSwitchMode & 
            (PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_MIC | PHONEHOOKSWITCHMODE_SPEAKER))
        *lpdwHookSwitch |= PHONEHOOKSWITCHDEV_HEADSET;
    if (m_PhoneStatus.dwSpeakerHookSwitchMode & 
            (PHONEHOOKSWITCHMODE_MICSPEAKER | PHONEHOOKSWITCHMODE_MIC | PHONEHOOKSWITCHMODE_SPEAKER))
        *lpdwHookSwitch |= PHONEHOOKSWITCHDEV_SPEAKER;
    return false;            

}// CTSPIPhoneConnection::GetHookSwitch

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetLamp
//
// Return the current lamp mode of the specified lamp
//
LONG CTSPIPhoneConnection::GetLamp (DWORD dwButtonId, LPDWORD lpdwLampMode)
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETLAMP) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // Make sure the button id is valid.
    const CPhoneButtonInfo* pButton = GetButtonInfo (dwButtonId);
    if (pButton == NULL || pButton->GetLampMode() == PHONELAMPMODE_DUMMY)
        return PHONEERR_INVALBUTTONLAMPID;
        
    *lpdwLampMode = pButton->GetLampMode();    
    return false;

}// CTSPIPhoneConnection::GetLamp

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetLampMode
//
// Return the current lamp mode of the specified lamp
//
DWORD CTSPIPhoneConnection::GetLampMode (int iButtonId)
{                                
    // Make sure the button id is valid.
    const CPhoneButtonInfo* pButton = GetButtonInfo (iButtonId);
    if (pButton == NULL || pButton->GetLampMode() == PHONELAMPMODE_DUMMY)
        return PHONELAMPMODE_DUMMY;
    return pButton->GetLampMode();

}// CTSPIPhoneConnection::GetLampMode

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetButtonInfo
//
// Sets the button information for a specified button
//
LONG CTSPIPhoneConnection::SetButtonInfo (DRV_REQUESTID dwRequestID, DWORD dwButtonId,
		LPPHONEBUTTONINFO const lpPhoneInfo)
{                                      
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_SETBUTTONINFO) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // Make sure the button id is valid.
    const CPhoneButtonInfo* pButton = GetButtonInfo(dwButtonId);
    if (pButton == NULL || pButton->GetButtonMode() == PHONEBUTTONMODE_DUMMY)
        return PHONEERR_INVALBUTTONLAMPID;

	// Copy our button information structure.
	LPPHONEBUTTONINFO lpbi = NULL;
	if (lpPhoneInfo != NULL)
	{
		lpbi = reinterpret_cast<LPPHONEBUTTONINFO>(AllocMem(lpPhoneInfo->dwTotalSize));
		if (lpbi == NULL)
			return PHONEERR_NOMEM;
		MoveMemory(lpbi, lpPhoneInfo, lpPhoneInfo->dwTotalSize);
	}

	// Build a request to map this event.
	RTSetButtonInfo* pRequest = new RTSetButtonInfo(this, dwRequestID, dwButtonId, lpbi);
	if (pRequest == NULL)
	{
		FreeMem(lpbi);
		return PHONEERR_NOMEM;
	}

    // Submit the request.    
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestID);   
    return PHONEERR_OPERATIONUNAVAIL;

}// CTSPIPhoneConnection::SetButtonInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetButtonInfo
//
// Set the button information internally and notify TAPI.  This should
// be called when the worker code completes a REQUEST_SETBUTTONINFO.
//
void CTSPIPhoneConnection::SetButtonInfo (int iButtonID, DWORD dwFunction, DWORD dwMode, LPCTSTR pszName)
{
	CEnterCode sLock(this);  // Synch access to object
	CPhoneButtonInfo* pButton = GetWButtonInfo(iButtonID);
	if (pButton != NULL)
        pButton->SetButtonInfo(dwFunction, dwMode, pszName);

}// CTSPIPhoneConnection::SetButtonInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetLamp
//
// This method causes the specified lamp to be set on our phone device.
//
LONG CTSPIPhoneConnection::SetLamp (DRV_REQUESTID dwRequestID, DWORD dwButtonLampID, DWORD dwLampMode)
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_SETLAMP) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // Make sure the button id is valid.
    const CPhoneButtonInfo* pButton = GetButtonInfo(dwButtonLampID);
    if (pButton == NULL || pButton->GetLampMode() == PHONELAMPMODE_DUMMY)
        return PHONEERR_INVALBUTTONLAMPID;

	// Allocate our request for this event.
	RTSetLampInfo* pRequest = new RTSetLampInfo(this, dwRequestID, dwButtonLampID, dwLampMode);

    // Submit the request.    
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestID);   
    return PHONEERR_OPERATIONFAILED;    

}// CTSPIPhoneConnection::SetLamp

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetLampState
//
// Set the lamp state for the specified lamp id (button id).  This
// should be called when the worker code has completed a REQUEST_SETLAMP.
// 
// This notifies TAPI of the lampstate change.
//
DWORD CTSPIPhoneConnection::SetLampState (int iButtonId, DWORD dwLampState)
{                                     
	CEnterCode sLock(this);  // Synch access to object
    CPhoneButtonInfo* pButton = GetWButtonInfo (iButtonId);
    if (pButton != NULL)
    {
		_TSP_ASSERTE((pButton->GetAvailLampModes() & dwLampState) == dwLampState);
        DWORD dwCurrState = pButton->GetLampMode();
        pButton->SetLampMode (dwLampState);

		// See if this is the MSGWAIT lamp.  If so, record it in the line device capabilities.
		if (pButton->GetFunction() == PHONEBUTTONFUNCTION_MSGINDICATOR)
		{
			CTSPILineConnection* pLine = GetAssociatedLine();
			if (pLine != NULL)
			{
				DWORD dwFlags = pLine->GetLineDevStatus()->dwDevStatusFlags;
				if (dwLampState == PHONELAMPMODE_OFF)
					dwFlags &= ~LINEDEVSTATUSFLAGS_MSGWAIT;
				else
					dwFlags |= LINEDEVSTATUSFLAGS_MSGWAIT;
				pLine->SetDeviceStatusFlags(dwFlags);
			}
		}

        OnPhoneStatusChange (PHONESTATE_LAMP, static_cast<DWORD>(iButtonId));
        return dwCurrState;
    }   
    
    return PHONELAMPMODE_DUMMY;

}// CTSPIPhoneConnection::SetLampState

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetButtonState
//
// Set the current state of the button for a specified button id.  This
// should be called when the worker code detects a button going up or down.
//
DWORD CTSPIPhoneConnection::SetButtonState (int iButtonId, DWORD dwButtonState)
{
	CEnterCode sLock(this);  // Synch access to object
    CPhoneButtonInfo* pButton = GetWButtonInfo (iButtonId);
    if (pButton != NULL)
    {
        DWORD dwCurrState = pButton->GetButtonState();
        pButton->SetButtonState (dwButtonState);
        
        // Tell TAPI if it is valid for a button state message.
        if (pButton->GetButtonMode() != PHONEBUTTONMODE_DUMMY &&
            (dwButtonState == PHONEBUTTONSTATE_UP && dwCurrState == PHONEBUTTONSTATE_DOWN) ||
            (dwCurrState == PHONEBUTTONSTATE_UP && dwButtonState == PHONEBUTTONSTATE_DOWN))
            OnButtonStateChange (iButtonId, pButton->GetButtonMode(), dwButtonState);
        return dwCurrState;
    }   
    
    return PHONEBUTTONSTATE_UNAVAIL;

}// CTSPIPhoneDevice::SetButtonState
                                          
///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetGain
//
// This method is called by the CServiceProvider class in response to
// a phoneSetGain function.  It calls the worker thread to do the
// H/W setting.
//
LONG CTSPIPhoneConnection::SetGain (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwGain)
{   
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_SETGAINHANDSET|
			PHONEFEATURE_SETGAINSPEAKER|PHONEFEATURE_SETGAINHEADSET)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
	
    // If we don't support the hook switch device.                         
    if ((m_PhoneCaps.dwHookSwitchDevs & dwHookSwitchDev) != dwHookSwitchDev)
        return PHONEERR_INVALHOOKSWITCHDEV;

	// Build our request object
	RTSetGain* pRequest = new RTSetGain(this, dwRequestId, dwHookSwitchDev, dwGain);

    // Submit the request.
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestId);   
    return PHONEERR_OPERATIONFAILED;    

}// CTSPIPhoneConnection::SetGain

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetVolume
//
// This method is called by the CServiceProvider class in response to
// a phoneSetVolume function.  It calls the worker thread to do the
// H/W setting.
//
LONG CTSPIPhoneConnection::SetVolume (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwVolume)
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_SETVOLUMEHANDSET|
			PHONEFEATURE_SETVOLUMESPEAKER|PHONEFEATURE_SETVOLUMEHEADSET)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // If we don't support the hook switch device.                         
    if ((m_PhoneCaps.dwHookSwitchDevs & dwHookSwitchDev) != dwHookSwitchDev)
        return PHONEERR_INVALHOOKSWITCHDEV;

	// Build our request object
	RTSetVolume* pRequest = new RTSetVolume(this, dwRequestId, dwHookSwitchDev, dwVolume);

    // Submit the request.
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestId);   
    return PHONEERR_OPERATIONFAILED;    

}// CTSPIPhoneConnection::SetVolume

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetHookSwitch
//
// This method is called by the CServiceProvider class in response to
// a phoneSetHookSwitch function.  It calls the worker thread to do the
// H/W setting.
//
LONG CTSPIPhoneConnection::SetHookSwitch (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwHookSwitchMode)
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & (PHONEFEATURE_SETHOOKSWITCHHANDSET|
			PHONEFEATURE_SETHOOKSWITCHSPEAKER|PHONEFEATURE_SETHOOKSWITCHHEADSET)) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // If we don't support one of the hook switch devices.                         
    if ((m_PhoneCaps.dwHookSwitchDevs & dwHookSwitchDev) != dwHookSwitchDev)
        return PHONEERR_INVALHOOKSWITCHDEV;

	// Build our request object
	RTSetHookswitch* pRequest = new RTSetHookswitch(this, dwRequestId, dwHookSwitchDev, dwHookSwitchMode);

    // Submit the request.
    if (AddAsynchRequest(pRequest))
        return static_cast<LONG>(dwRequestId);   
    return PHONEERR_OPERATIONFAILED;    

}// CTSPIPhoneConnection::SetHookSwitch

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetRing
//
// This method is called in response to a phoneSetRing request.  It
// passes control to the worker thread.
//
LONG CTSPIPhoneConnection::SetRing (DRV_REQUESTID dwRequestID, DWORD dwRingMode, DWORD dwVolume)
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_SETRING) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
	
    // Verify the ring mode.
    if (dwRingMode >= m_PhoneCaps.dwNumRingModes)
        return PHONEERR_INVALRINGMODE;

	// Allocate our request.
	RTSetRing* pRequest = new RTSetRing(this, dwRequestID, dwRingMode, dwVolume);

    // Submit the request
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestID);   
    return PHONEERR_OPERATIONFAILED;

}// CTSPIPhoneConnection::SetRing

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetDisplay
//
// This method is called in response to a phoneSetDisplay command.  It
// submits an asynch request to the worker code.
//
LONG CTSPIPhoneConnection::SetDisplay (DRV_REQUESTID dwRequestID, DWORD dwRow, DWORD dwCol,
			LPCTSTR lpszDisplay, DWORD dwSize)
{                                   
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_SETDISPLAY) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
	
    // Verify the row/col field.
    if (dwRow > m_PhoneCaps.dwDisplayNumRows || 
		dwCol > m_PhoneCaps.dwDisplayNumColumns)
        return PHONEERR_INVALPARAM;

	// Allocate our request.
	RTSetDisplay* pRequest = new RTSetDisplay(this, dwRequestID, dwRow, dwCol, 
		const_cast<LPTSTR>(lpszDisplay), dwSize);

    // Submit the request
    if (AddAsynchRequest (pRequest))
        return static_cast<LONG>(dwRequestID);   
    return PHONEERR_OPERATIONFAILED;

}// CTSPIPhoneConnection::SetDisplay

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetData
//
// Downloads the information in the specified buffer to the phone buffer.
//
LONG CTSPIPhoneConnection::SetData (DRV_REQUESTID dwRequestID, DWORD dwDataID, 
									LPCVOID lpData, DWORD dwSize) 
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_SETDATA) == 0)
		return PHONEERR_OPERATIONUNAVAIL;

    // Verify the buffer id.
    if (dwDataID > m_PhoneCaps.dwNumSetData)
        return PHONEERR_INVALDATAID;
    
	// Build our request.
	RTSetPhoneData* pRequest = new RTSetPhoneData(this, dwRequestID, dwDataID, 
		const_cast<LPVOID>(lpData), dwSize);

    // Submit the request
    if (AddAsynchRequest (pRequest))    
        return static_cast<LONG>(dwRequestID);   
    return PHONEERR_OPERATIONFAILED;

}// CTSPIPhoneConnection::SetData

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GetData
//
// Return the data from an uploadable buffer on a phone.
//
LONG CTSPIPhoneConnection::GetData (DWORD dwDataID, LPVOID lpData, DWORD dwSize) 
{                                
	// Validate the request
	if ((GetPhoneStatus()->dwPhoneFeatures & PHONEFEATURE_GETDATA) == 0)
		return PHONEERR_OPERATIONUNAVAIL;
	
    // Verify the buffer id.
    if (dwDataID > m_PhoneCaps.dwNumGetData)
        return PHONEERR_INVALDATAID;

	// Build a request to retrieve the phone data.
	RTGetPhoneData* pRequest = new RTGetPhoneData(this, dwDataID, lpData, dwSize);

    // Pass the request to the worker code.  This request is handled specially in that
    // it is NOT asynchronous.  Therefore, WE must delete the phone data on an error
    // since when we delete the request it will be deleted as well.
    if (AddAsynchRequest (pRequest))
	{
		// Request has been queued.  Wait for it to complete.
		return WaitForRequest(pRequest);
	}
    return PHONEERR_OPERATIONFAILED;

}// CTSPIPhoneConnection::GetData

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetStatusMessages
//
// Enables an application to monitor the specified phone device 
// for selected status events.
//
LONG CTSPIPhoneConnection::SetStatusMessages(DWORD dwPhoneStates, DWORD dwButtonModes, DWORD dwButtonStates)
{   
    // Validate the button modes/states.  If dwButtonModes is zero, then ignore both
    // fields.
    if (dwButtonModes != 0)
    {
        // Otherwise, button states CANNOT be zero
        if (dwButtonStates == 0)
            return PHONEERR_INVALBUTTONSTATE;
        
        // Set button modes/states
    	m_dwButtonModes = dwButtonModes;
    	m_dwButtonStates = dwButtonStates;  
    }
    
    // Set the phone states to monitor for.
    m_dwPhoneStates = dwPhoneStates;
    return false;

}// CTSPIPhoneConnection::SetStatusMessages

/////////////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::Open
//
// This method opens the phone device, returning the service provider's 
// opaque handle for the device and retaining the TAPI opaque handle.
// CTSPIPhoneConnection::Open 
//
LONG CTSPIPhoneConnection::Open (
HTAPIPHONE htPhone,                 // TAPI opaque phone handle
PHONEEVENT lpfnEventProc,           // PHONEEVENT callback   
DWORD dwTSPIVersion)                // Version expected
{   
    // We should not have an existing handle.
    if (GetPhoneHandle() != NULL)
		return PHONEERR_ALLOCATED;

    // Save off the event procedure for this phone and the TAPI
    // opaque phone handle which represents this device to the application.
    m_htPhone = htPhone;
    m_lpfnEventProc = lpfnEventProc;
    m_dwNegotiatedVersion = dwTSPIVersion;

    _TSP_DTRACE(_T("Opening phone 0x%lX, TAPI handle=0x%lX, SP handle=0x%lX\n"), GetDeviceID(), m_htPhone, this);
    
    // Tell our device to perform an open for this connection.
    if (!OpenDevice())
	{
		m_htPhone = 0;
		m_lpfnEventProc = NULL;
		m_dwNegotiatedVersion = 0;
		return LINEERR_RESOURCEUNAVAIL;
	}

    return false;
    
}// CTSPIPhoneConnection::Open

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::Close
//
// Close the phone connection object and reset the phone handle
//
LONG CTSPIPhoneConnection::Close()
{       
    if (GetPhoneHandle())
    {
        _TSP_DTRACE(_T("Closing phone 0x%lX, TAPI handle=0x%lX, SP handle=0x%lX\n"), GetDeviceID(), GetPhoneHandle(), this);

        // Kill any pending requests.
        RemovePendingRequests();
        
        // Reset the event procedure and phone handle.
        m_lpfnEventProc = NULL;
        m_htPhone = 0;
        m_dwPhoneStates = 0L;
        m_dwButtonModes = 0L;
        m_dwButtonStates = 0L;
		m_dwNegotiatedVersion = GetSP()->GetSupportedVersion();
        
		// If the phone has been removed, then mark it as DELETED now
		// so we will refuse any further traffic on this line.
		if (GetFlags() & _IsRemoved)
			m_dwFlags |= _IsDeleted;

        // Close the device
        CloseDevice();
        return false;
    }
    return PHONEERR_OPERATIONFAILED;

}// CTSPIPhoneConnection::Close

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetStatusFlags
//
// This method sets the "dwStatusFlags" field of the PHONESTATUS
// record.  TAPI is notified of the appropriate things.
// 
// The previous status flags are returned.
//
DWORD CTSPIPhoneConnection::SetStatusFlags (DWORD dwStatus)
{                                       
    DWORD dwOldStatus = m_PhoneStatus.dwStatusFlags;
    m_PhoneStatus.dwStatusFlags = dwStatus;

    // Send TAPI notifications
    if (((dwStatus & PHONESTATUSFLAGS_CONNECTED) != 0) &&
         (dwOldStatus & PHONESTATUSFLAGS_CONNECTED) == 0)
        OnPhoneStatusChange (PHONESTATE_CONNECTED);
    else if (((dwStatus & PHONESTATUSFLAGS_CONNECTED) == 0) &&        
        (dwOldStatus & PHONESTATUSFLAGS_CONNECTED) != 0)
        OnPhoneStatusChange (PHONESTATE_DISCONNECTED);
        
    if (((dwStatus & PHONESTATUSFLAGS_SUSPENDED) != 0) &&
        (dwOldStatus & PHONESTATUSFLAGS_SUSPENDED) == 0)
        OnPhoneStatusChange (PHONESTATE_SUSPEND);
    else if (((dwStatus & PHONESTATUSFLAGS_SUSPENDED) == 0) &&
        (dwOldStatus & PHONESTATUSFLAGS_SUSPENDED) != 0)
        OnPhoneStatusChange (PHONESTATE_RESUME);

    return dwOldStatus;

}// CTSPIPhoneConnection::SetStatusFlags

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetHookSwitch
//
// Change the hookswitch state of a hookswitch device(s).
//
void CTSPIPhoneConnection::SetHookSwitch (DWORD dwHookSwitchDev, DWORD dwMode)
{                                      
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HANDSET)
    {
        m_PhoneStatus.dwHandsetHookSwitchMode = dwMode;
        OnPhoneStatusChange (PHONESTATE_HANDSETHOOKSWITCH, dwMode);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_SPEAKER)
    {   
        m_PhoneStatus.dwSpeakerHookSwitchMode = dwMode;
        OnPhoneStatusChange (PHONESTATE_SPEAKERHOOKSWITCH, dwMode);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HEADSET)
    {                                                      
        m_PhoneStatus.dwHeadsetHookSwitchMode = dwMode;
        OnPhoneStatusChange (PHONESTATE_HEADSETHOOKSWITCH, dwMode);
    }

}// CTSPIPhoneConnection::SetHookSwitch

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetVolume
//
// Set the volume of a hookswitch device(s)
//
void CTSPIPhoneConnection::SetVolume (DWORD dwHookSwitchDev, DWORD dwVolume)
{                                      
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HANDSET)
    {
        m_PhoneStatus.dwHandsetVolume = dwVolume;
        OnPhoneStatusChange (PHONESTATE_HANDSETVOLUME);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_SPEAKER)
    {   
        m_PhoneStatus.dwSpeakerVolume = dwVolume;
        OnPhoneStatusChange (PHONESTATE_SPEAKERVOLUME);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HEADSET)
    {                                                      
        m_PhoneStatus.dwHeadsetVolume = dwVolume;
        OnPhoneStatusChange (PHONESTATE_HEADSETVOLUME);
    }

}// CTSPIPhoneConnection::SetVolume

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetGain
//
// Set the gain of a hookswitch device(s)
//
void CTSPIPhoneConnection::SetGain (DWORD dwHookSwitchDev, DWORD dwGain)
{                                      
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HANDSET)
    {
        m_PhoneStatus.dwHandsetGain = dwGain;
        OnPhoneStatusChange (PHONESTATE_HANDSETGAIN);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_SPEAKER)
    {   
        m_PhoneStatus.dwSpeakerGain = dwGain;
        OnPhoneStatusChange (PHONESTATE_SPEAKERGAIN);
    }
    
    if (dwHookSwitchDev & PHONEHOOKSWITCHDEV_HEADSET)
    {                                                      
        m_PhoneStatus.dwHeadsetGain = dwGain;
        OnPhoneStatusChange (PHONESTATE_HEADSETGAIN);
    }

}// CTSPIPhoneConnection::SetGain

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::OnRequestComplete
//
// This virtual method is called when an outstanding request on this
// phone device has completed.  The return code indicates the success
// or failure of the request.  
//
void CTSPIPhoneConnection::OnRequestComplete(CTSPIRequest* pReq, LONG lResult)
{              
	// Ignore the response if the request failed.
    if (lResult != 0)
		return;

	switch (pReq->GetCommand())
	{
		// If this is a SETBUTTON request, then set the information back into our object.
		case REQUEST_SETBUTTONINFO:
		{
			RTSetButtonInfo* pInfo = dynamic_cast<RTSetButtonInfo*>(pReq);
			SetButtonInfo(pInfo->GetButtonLampID(), pInfo->GetButtonFunction(), pInfo->GetButtonMode(), pInfo->GetButtonText());
			break;
		}                                              
        
		// If this is a SETLAMP request, then set the information to our PHONESTATUS record.
		case REQUEST_SETLAMP:
		{
			RTSetLampInfo* pInfo = dynamic_cast<RTSetLampInfo*>(pReq);
			SetLampState (pInfo->GetButtonLampID(), pInfo->GetLampMode());
			break;
		}
    
		// If this is a SETRING request, then set the new ring pattern and volume
		// into our status record.
		case REQUEST_SETRING:
		{
			RTSetRing* pInfo = dynamic_cast<RTSetRing*>(pReq);
			SetRingMode (pInfo->GetRingMode());
			SetRingVolume (pInfo->GetVolume());
			break;
		}

		// If this is a HOOKSWITCH request, then set the state of the hookswitch
		// into our status record.
		case REQUEST_SETHOOKSWITCH:
		{
			RTSetHookswitch* pInfo = dynamic_cast<RTSetHookswitch*>(pReq);
			SetHookSwitch (pInfo->GetHookswitchDevice(), pInfo->GetState());
			break;
		}
    
		// If this is a hookswitch GAIN request, then set the gain value into
		// our status record.
		case REQUEST_SETHOOKSWITCHGAIN:
		{
			RTSetGain* pInfo = dynamic_cast<RTSetGain*>(pReq);
			SetGain (pInfo->GetHookswitchDevice(), pInfo->GetGain());
			break;
		}
    
		// If this is a hookswitch VOLUME request, then set the new volume into
		// our status record.
		case REQUEST_SETHOOKSWITCHVOL:
		{
			RTSetVolume* pInfo = dynamic_cast<RTSetVolume*>(pReq);
			SetVolume (pInfo->GetHookswitchDevice(), pInfo->GetVolume());
			break;
		}

		// Default action
		default:
			break;
	}

}// CTSPIPhoneConnection::OnRequestComplete

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetDisplay
//
// Set the display to a known string.
//
void CTSPIPhoneConnection::SetDisplay (LPCTSTR pszBuff)
{
	CEnterCode sLock(this);  // Synch access to object
    for (int iRow = 0; iRow < m_Display.GetDisplaySize().cy; iRow++)
	{
        for (int iCol = 0 ; iCol < m_Display.GetDisplaySize().cx; iCol++)
		{
			if (pszBuff == NULL || *pszBuff == '\0')
				m_Display.SetCharacterAtPosition (iCol, iRow, _T(' '));
			else
				m_Display.SetCharacterAtPosition (iCol, iRow, *pszBuff++);
		}
	}
    OnPhoneStatusChange (PHONESTATE_DISPLAY);
            
}// CTSPIPhoneConnection::SetDisplay

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::OnPhoneStatusChange
//
// This is called when anything in our PHONESTATUS record changes.
//
void CTSPIPhoneConnection::OnPhoneStatusChange(DWORD dwState, DWORD dwParam)
{                                                                         
    if ((m_dwPhoneStates & dwState) || dwState == PHONESTATE_REINIT)
        Send_TAPI_Event (PHONE_STATE, dwState, dwParam);

}// CTSPIPhoneConnection::OnPhoneStatusChange

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::OnButtonStateChange
//
// This method is invoked when a button changes state (UP/DOWN)
//
void CTSPIPhoneConnection::OnButtonStateChange (DWORD dwButtonID, DWORD dwMode, DWORD dwState)
{                                            
    if ((m_dwButtonModes & dwMode) && (m_dwButtonStates & dwState))
        Send_TAPI_Event (PHONE_BUTTON, dwButtonID, dwMode, dwState);

}// CTSPIPhoneConnection::OnButtonStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetRingMode
//
// Set the ring mode in the PHONESTATUS record and notify TAPI.  This
// should only be called by the worker code when the ring mode really
// changes on the device.
//
void CTSPIPhoneConnection::SetRingMode (DWORD dwRingMode)
{   
    // The ringmode should have already been verified.
    m_PhoneStatus.dwRingMode = dwRingMode;
    OnPhoneStatusChange (PHONESTATE_RINGMODE, dwRingMode);

}// CTSPIPhoneConnection::SetRingMode

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetRingVolume
//
// Set the ring volume in the PHONESTATUS record and notify TAPI.  This
// should only be called by the worker code when the ring volume really
// changes on the device.
//
void CTSPIPhoneConnection::SetRingVolume (DWORD dwRingVolume)
{                                      
    // The ring volume should have already been verified.
    m_PhoneStatus.dwRingVolume = dwRingVolume;
    OnPhoneStatusChange (PHONESTATE_RINGVOLUME);

}// CTSPIPhoneConnection::SetRingVolume

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::DevSpecific
//
// Invoke a device-specific feature on this phone device.
//
LONG CTSPIPhoneConnection::DevSpecific(DRV_REQUESTID /*dwRequestId*/, LPVOID /*lpParams*/, DWORD /*dwSize*/)
{                                          
    // Derived class must manage device-specific features.
    return LINEERR_OPERATIONUNAVAIL;
    
}// CTSPIPhoneConnection::DevSpecific

///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::GenericDialogData
//
// This method is called when a dialog which sent in a PHONE
// device ID called our UI callback.
//
LONG CTSPIPhoneConnection::GenericDialogData (LPVOID /*lpParam*/, DWORD /*dwSize*/)
{
	return false;

}// CTSPIPhoneConnection::GenericDialogData

////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::SetPhoneFeatures
//
// Used by the user to sets the current phone features
//
void CTSPIPhoneConnection::SetPhoneFeatures (DWORD dwFeatures)
{   
	CEnterCode sLock (this);

	// Make sure the capabilities structure reflects this ability.
	if ((m_PhoneCaps.dwPhoneFeatures & dwFeatures) == 0)
	{
		_TSP_DTRACE(_T("PHONECAPS.dwPhoneFeatures missing 0x%lx bit\n"), dwFeatures);
		m_PhoneCaps.dwPhoneFeatures |= dwFeatures;	
		OnPhoneCapabiltiesChanged();
	}

	// Update it only if it has changed.
	if (m_PhoneStatus.dwPhoneFeatures != dwFeatures)
	{
		m_PhoneStatus.dwPhoneFeatures = dwFeatures;
		OnPhoneStatusChange (PHONESTATE_OTHER);
	}

}// CTSPIPhoneConnection::SetPhoneFeatures

////////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::OnPhoneCapabilitiesChanged
//
// This function is called when the phone capabilties change in the lifetime
// of the provider.
//
void CTSPIPhoneConnection::OnPhoneCapabiltiesChanged()
{
	OnPhoneStatusChange (PHONESTATE_CAPSCHANGE);

}// CTSPIPhoneConnection::OnPhoneCapabiltiesChanged

/////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::FindButton
//
// Locate a button index based on some criteria passed in about the
// function, type, and how many.
//
int CTSPIPhoneConnection::FindButton (DWORD dwButtonFunction, DWORD dwButtonMode, int iCount)
{
    int iFound = 0, i = 0;
    for (CPhoneButtonArray::iterator iButton = m_arrButtonInfo.begin();
	     iButton != m_arrButtonInfo.end(); ++iButton, ++i)
    {
        if ((*iButton) && (*iButton)->GetFunction() == dwButtonFunction &&
            (dwButtonMode == PHONEBUTTONMODE_DUMMY || (*iButton)->GetButtonMode() == dwButtonMode))
        {
            if (++iFound == iCount)
                return i;
        }
    }
    return -1;
   
}// CTSPIPhoneConnection::FindButton

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPIPhoneConnection::Dump
//
// Debug "dump" of the object and it's contents.
//
TString CTSPIPhoneConnection::Dump() const 
{
	TStringStream outstm;
	outstm << _T("0x") << hex << (DWORD)this;
    outstm << _T(",PhoneID=0x") << hex << m_PhoneCaps.dwPermanentPhoneID;
	outstm << _T(",DeviceID=0x") << hex << m_dwDeviceID;
	outstm << _T(",htPhone=0x") << hex << m_htPhone;
    return(outstm.str());

}// CTSPIPhoneConnection::Dump
#endif

