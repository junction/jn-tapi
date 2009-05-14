/*******************************************************************/
//
// EVENTTYPES.CPP
//
// This defines our various CDataBlock derivatives and the
// factory.
//
// Copyright (C) 1998 JulMar Technology, Inc.
// All rights reserved
//
// TSP++ Version 3.00 PBX/ACD Emulator Projects
//
// Modification History
// ------------------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Initial revision
//
/*******************************************************************/

/*---------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------*/
#include "stdafx.h"
#include "EventTypes.h"

/*---------------------------------------------------------------*/
// DEBUG INFORMATION
/*---------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*---------------------------------------------------------------*/
// STRING to EVENT table
/*---------------------------------------------------------------*/
static struct
{
	LPCTSTR pszEvent;
	enum CEventBlock::PBXEvent evtType;
} g_StringToEvent[] = {

	{ _T("ACK"), CEventBlock::CommandResponse },
	{ _T("NAK"), CEventBlock::CommandResponse },
	{ _T("CD"),  CEventBlock::CallDetected    },
	{ _T("CP"),  CEventBlock::CallPlaced      },
	{ _T("CS"),  CEventBlock::CallStateChange },
	{ _T("CR"),  CEventBlock::CallReleased    },
	{ _T("CC"),  CEventBlock::CallConference  },
	{ _T("CT"),  CEventBlock::CallTransfer    },
	{ _T("CQ"),  CEventBlock::CallQueued      },
	{ _T("DD"),  CEventBlock::DigitDetected   },
	{ _T("CMD"), CEventBlock::CallMediaDetected },
	{ _T("PDC"), CEventBlock::DisplayChanged  },
	{ _T("PVC"), CEventBlock::VolumeChanged   },
	{ _T("PGC"), CEventBlock::GainChanged     },
	{ _T("PHC"), CEventBlock::HookswitchChanged },
	{ _T("PLC"), CEventBlock::LampChanged     },
	{ _T("ASC"), CEventBlock::AgentStateChanged },
	{ _T("AGC"), CEventBlock::AgentGroupChanged },
	{ NULL,		 CEventBlock::Unknown }
};

/*****************************************************************************
** Procedure:  CEventFactory::CEventFactory
**
** Arguments: void
**
** Returns:    void
**
** Description: Constructor for the event factory
**
*****************************************************************************/
CEventFactory::CEventFactory() : m_pHead(0)
{
	// Add all our factory event objects to the list
	new CEBResponse(&m_pHead);
	new CEBCallDetected(&m_pHead);
	new CEBCallPlaced(&m_pHead);
	new CEBCallStateChange(&m_pHead);
	new CEBCallReleased(&m_pHead);
	new CEBCallConference(&m_pHead);
	new CEBCallTransfer(&m_pHead);
	new CEBCallQueued(&m_pHead);
	new CEBDigitDetected(&m_pHead);
	new CEBCallMediaDetected(&m_pHead);
	new CEBDisplayChanged(&m_pHead);
	new CEBVolumeChanged(&m_pHead);
	new CEBGainChanged(&m_pHead);
	new CEBHookswitchChanged(&m_pHead);
	new CEBLampChanged(&m_pHead);
	new CEBAgentStateChanged(&m_pHead);
	new CEBAgentGroupChanged(&m_pHead);

}// CEventFactory::CEventFactory

/*****************************************************************************
** Procedure:  CEventFactory::~CEventFactory
**
** Arguments: void
**
** Returns:    void
**
** Description: Destructor for the event factory
**
*****************************************************************************/
CEventFactory::~CEventFactory()
{
	// Delete the factory event blocks
	CEventBlock* pBlock = m_pHead;
	while (pBlock != NULL)
	{
		CEventBlock* pNext = pBlock->m_pNext;
		delete pBlock;
		pBlock = pNext;
	}

}// CEventFactory::~CEventFactory

/*****************************************************************************
** Procedure:  CEventFactory::Create
**
** Arguments: 'strCommand' - String from PBX
**
** Returns:    Derivative CEventBlock for this PBX command
**
** Description: This function creates a new CEventBlock object
**
*****************************************************************************/
CEventBlock* CEventFactory::Create(TString& strData)
{
	// We should have at least one object in our event list
	_TSP_ASSERT(m_pHead != NULL);

	// The first element is always the command.
	TString strCommand = GetNextElem(strData);
	if (strCommand.empty())
		return NULL;

	// Uppercase the command string
	CharUpperBuff(&strCommand[0], strCommand.length());

	// Convert the command string into its numerical equivelant for
	// quick lookup.
	enum CEventBlock::PBXEvent evtType = CEventBlock::Unknown;
	for (int i = 0; g_StringToEvent[i].pszEvent != NULL; i++)
	{
		if (!strCommand.compare(g_StringToEvent[i].pszEvent))
		{
			evtType = g_StringToEvent[i].evtType;
			break;
		}
	}

	// If the event is unknown, it means we have not completely defined
	// the interface and have a problem!
	_TSP_ASSERT (evtType != CEventBlock::Unknown);

	// Return the event object which encapsulates this command
	return m_pHead->Create(evtType, strData);

}// CEventFactory::Create

