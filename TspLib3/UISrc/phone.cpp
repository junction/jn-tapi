/******************************************************************************/
//                                                                        
// PHONE.CPP - User-interface PHONECONNECTION support
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
/******************************************************************************/

#include "stdafx.h"
#include <spbstrm.h>
using namespace tsplibui;

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::~CTSPUIPhoneConnection
//
// Destructor for the CTSPUIPhoneConnection class
//
CTSPUIPhoneConnection::~CTSPUIPhoneConnection()
{
	SetAssociatedLine(0xffffffff);

	// Delete all our buttons
	int i;
	for (i = 0; i < m_arrButtons.GetSize(); i++)
		delete m_arrButtons[i];

	// And our hookswitch devices
	for (i = 0; i < m_arrHookswitch.GetSize(); i++)
		 delete m_arrHookswitch[i];

}// CTSPUIPhoneConnection::~CTSPUIPhoneConnection

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::operator=
//
// Assignment operator
//
const CTSPUIPhoneConnection& CTSPUIPhoneConnection::operator=(const CTSPUIPhoneConnection& lc)
{
	if (&lc != this)
	{
		ASSERT(m_pDevice == lc.m_pDevice);

		m_dwDeviceID = lc.m_dwDeviceID;
		m_strName = lc.m_strName;
		m_dwLineID = lc.m_dwLineID;
		m_sizDisplay.cx = lc.m_sizDisplay.cx;
		m_sizDisplay.cy = lc.m_sizDisplay.cy;
		m_chDisplay = lc.m_chDisplay;

		// Delete all our buttons
		int i;
		for (i = 0; i < m_arrButtons.GetSize(); i++)
			delete dynamic_cast<CPhoneButtonInfo*>(m_arrButtons[i]);
		m_arrButtons.RemoveAll();

		// Copy the button information
		for (i = 0; i < lc.m_arrButtons.GetSize(); i++)
		{
			CPhoneButtonInfo* pButton = dynamic_cast<CPhoneButtonInfo*>(lc.m_arrButtons[i]);
			AddButton(pButton->m_dwButtonFunction, pButton->m_dwButtonMode, pButton->m_dwLampModes, pButton->m_strButtonDescription);
		}

		// Delete all our hookswitch information
		for (i = 0; i < m_arrHookswitch.GetSize(); i++)
			 delete dynamic_cast<CPhoneHSInfo*>(m_arrHookswitch[i]);
		m_arrHookswitch.RemoveAll();

		// Copy it from our other object
		for (i = 0; i < lc.m_arrHookswitch.GetSize(); i++)
		{
			CPhoneHSInfo* pHS = dynamic_cast<CPhoneHSInfo*>(lc.m_arrHookswitch[i]);
			AddHookSwitchDevice(pHS->m_dwDevice, pHS->m_dwModes, pHS->m_dwVolume, pHS->m_dwGain, pHS->m_dwSetModes, pHS->m_dwMonModes, pHS->m_fSupportsVolChange, pHS->m_fSupportsGainChange);
		}

		// Copy over all the upload/download buffers
		m_arrUploadBuffers.RemoveAll();
		m_arrUploadBuffers.Append(lc.m_arrUploadBuffers);
		m_arrDownloadBuffers.RemoveAll();
		m_arrDownloadBuffers.Append(lc.m_arrDownloadBuffers);
	}

	return *this;

}// CTSPUIPhoneConnection::operator=

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::read
//
// Reads the object from a stream
//
TStream& CTSPUIPhoneConnection::read(TStream& istm)
{
	unsigned int chDisplay = 0;
	istm >> m_dwDeviceID >> m_dwLineID >> m_strName >>
			m_sizDisplay.cx >> m_sizDisplay.cy >> chDisplay;
	m_chDisplay = static_cast<char>(chDisplay & 0xff);

	// Read in the buttons.
	unsigned int i, iCount = 0;
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		CPhoneButtonInfo* pButton = new CPhoneButtonInfo;
		istm >> *pButton;
		m_arrButtons.Add(pButton);
	}

	// Read in the hookswitch devices
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		CPhoneHSInfo* pHS = new CPhoneHSInfo;
		istm >> *pHS;
		m_arrHookswitch.Add(pHS);
	}

	// Read the download buffers
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwBuffer;
		istm >> dwBuffer;
		m_arrDownloadBuffers.Add(dwBuffer);
	}

	// Read the upload buffers
	istm >> iCount;
	for (i = 0; i < iCount; i++)
	{
		DWORD dwBuffer;
		istm >> dwBuffer;
		m_arrUploadBuffers.Add(dwBuffer);
	}

	return istm;

}// CTSPUIPhoneConnection::read

///////////////////////////////////////////////////////////////////////////
// CTSPUIPhoneConnection::write
//
// Writes the object to a stream
//
TStream& CTSPUIPhoneConnection::write(TStream& ostm) const
{
	unsigned int chDisplay = static_cast<unsigned int>(m_chDisplay);
	ostm << m_dwDeviceID << m_dwLineID << m_strName <<
			m_sizDisplay.cx << m_sizDisplay.cy << chDisplay;

	// Write out the buttons.
	ostm << static_cast<unsigned int>(m_arrButtons.GetSize());
	int i;
	for (i = 0; i < m_arrButtons.GetSize(); i++)
		ostm << *dynamic_cast<CPhoneButtonInfo*>(m_arrButtons[i]);

	// Write out the hookswitch devices.
	ostm << static_cast<unsigned int>(m_arrHookswitch.GetSize());
	for (i = 0; i < m_arrHookswitch.GetSize(); i++)
		ostm << *dynamic_cast<CPhoneHSInfo*>(m_arrHookswitch[i]);

	// Write the download buffers
	ostm << static_cast<unsigned int>(m_arrDownloadBuffers.GetSize());
	for (i = 0; i < m_arrDownloadBuffers.GetSize(); i++)
		ostm << m_arrDownloadBuffers[i];

	// Write the upload buffers
	ostm << static_cast<unsigned int>(m_arrUploadBuffers.GetSize());
	for (i = 0; i < m_arrUploadBuffers.GetSize(); i++)
		ostm << m_arrUploadBuffers[i];

	return ostm;

}// CTSPUIPhoneConnection::write

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::read
//
// Read the object from a stream
//
TStream& CPhoneButtonInfo::read(TStream& istm)
{
	istm >> m_dwButtonMode >> m_dwButtonFunction >>
			m_dwLampModes >> m_strButtonDescription;

	return istm;

}// CPhoneButtonInfo::read

///////////////////////////////////////////////////////////////////////////
// CPhoneButtonInfo::write
//
// Writes the object to a stream
//
TStream& CPhoneButtonInfo::write(TStream& ostm) const
{
	ostm << m_dwButtonMode << m_dwButtonFunction << 
			m_dwLampModes << m_strButtonDescription;

	return ostm;

}// CPhoneButtonInfo::write

///////////////////////////////////////////////////////////////////////////
// CPhoneHSInfo::read
//
// Read the object from a stream
//
TStream& CPhoneHSInfo::read(TStream& istm)
{
	istm >> m_dwDevice >> m_dwModes >> m_dwVolume >> 
			m_dwGain >>	m_dwSetModes >> m_dwMonModes >> 
			m_fSupportsVolChange >> m_fSupportsGainChange;

	return istm;

}// CPhoneHSInfo::read

///////////////////////////////////////////////////////////////////////////
// CPhoneHSInfo::write
//
// Writes the object to a stream
//
TStream& CPhoneHSInfo::write(TStream& ostm) const
{
	ostm << m_dwDevice << m_dwModes << m_dwVolume << 
			m_dwGain <<	m_dwSetModes << m_dwMonModes << 
			m_fSupportsVolChange << m_fSupportsGainChange;

	return ostm;

}// CPhoneHSInfo::write

