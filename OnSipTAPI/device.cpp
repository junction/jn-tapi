/***************************************************************************
//
// DEVICE.CPP
//
// TAPI Service provider for TSP++ version 3.0
// Handles raw device events and the physical connection to the hardware.
//
// Copyright (C) 2009 Junction Networks
// All rights reserved
//
// Generated by the TSPWizard � 2009 JulMar Technology, Inc.
// 
/***************************************************************************/

/*----------------------------------------------------------------------------
	INCLUDE FILES
-----------------------------------------------------------------------------*/
#include "stdafx.h"
#include <process.h>
#include "OnSip.h"
#include <spbstrm.h>
#include "logger.h"
#include "OnSipInitStateMachine.h"

	// Hide constructor so class cannot be created directly
COnSipEvent::COnSipEvent(OnSipEventType evtType)
{ 
	Logger::log_debug( _T("COnSipEvent::COnSipEvent evtType=%d this=%p"), evtType, this );
	m_evtType = evtType; 
}

//virtual 
COnSipEvent::~COnSipEvent() 
{ 
	Logger::log_debug( _T("COnSipEvent::~COnSipEvent evtType=%d this=%p"), m_evtType, this );
}


//static
const COnSip_ConnectEvent* COnSipEvent::getConnectEvent(const COnSipEvent* pEvent)
{
	if ( pEvent == NULL || pEvent->m_evtType != CONNECT_EVENT )
		return NULL;
	return reinterpret_cast<const COnSip_ConnectEvent*>(pEvent);
}

//static
const COnSip_CallEvent* COnSipEvent::getCallEvent(const COnSipEvent* pEvent)
{
	if ( pEvent == NULL || pEvent->m_evtType != CALL_EVENT )
		return NULL;
	return reinterpret_cast<const COnSip_CallEvent*>(pEvent);
}

/*****************************************************************************
** Procedure:  MainConnThread
**
** Arguments:  'pDevice'	- Device object
**
** Returns:    void
**
** Description:  Main connection thread
**
*****************************************************************************/
//static 
unsigned __stdcall COnSipDevice::MainConnThread(void* pParam)
{
	reinterpret_cast<COnSipDevice*>(pParam)->ConnectionThread();
	_endthreadex(0);
	return 0;

}// ConnThread

/*****************************************************************************
** Procedure:  COnSipDevice::COnSipDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for the device object
**
*****************************************************************************/

COnSipDevice::COnSipDevice() : m_hConnThread(0)
{
	// Create the stop event.
	m_hevtStop = CreateEvent (NULL, true, false, NULL);
	InitializeCriticalSection(&m_csData);

	// Create the event for the OpenDevice
	m_hOpenDevice = CreateEvent (NULL, true, false, NULL);

	// TODO: 
	Logger::SetWin32Level( Logger::LEVEL_DEBUG );

}// COnSipDevice::COnSipDevice

/*****************************************************************************
** Procedure:  COnSipDevice::~COnSipDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for the device object
**
*****************************************************************************/
COnSipDevice::~COnSipDevice()
{
	// Close all our synchronization object handles
	CloseHandle(m_hevtStop);
	CloseHandle(m_hOpenDevice);
	DeleteCriticalSection(&m_csData);
}// COnSipDevice::~COnSipDevice


/*****************************************************************************
** Procedure:  CJTDevice::OnConnect
**
** Arguments:  true/false
**
** Returns:    void
**
** Description:  This function is called when a connection is made to
**               the PBX and when it is dropped.  It marks the lines as
**               connected/disconnected.
**
*****************************************************************************/
inline void COnSipDevice::OnConnect(bool bConnect)
{
	Logger::log_debug( _T("COnSipDevice::OnConnect %d"), bConnect );
	
	// Pass on device status to TSP library,
	// it will do CONNECT/DISCONNECT and INSERVICE/OUTOFSERVICE notifications.
	SetDeviceStatus(bConnect);
}

// Force the TAPI line to be closed.
// Signals this back to TAPI and TAPI applications.
void COnSipDevice::ForceClose()
{
	CTSPILineConnection* pLine = GetLineConnectionInfo(0);
	Logger::log_error( _T("COnSipDevice::ForceClose pLine=%p"), pLine );
	if (pLine != NULL)
		pLine->ForceClose();
}

/*****************************************************************************
** Procedure:  COnSipDevice::read
**
** Arguments:  'istm' - Input iostream
**
** Returns:    iostream reference
**
** Description: This is called to read information in from the registry.
**
*****************************************************************************/
TStream& COnSipDevice::read(TStream& istm)
{
	// Always call the base class!
	CTSPIDevice::read(istm);

	// TODO: Add any additional information which was stored by the 
	// user-interface component of the TSP.

	return istm;

}// COnSipDevice::read

/*****************************************************************************
** Procedure:  COnSipDevice::Init
**
** Arguments:  'dwProviderId'		-	TAPI provider ID
**             'dwBaseLine'			-	Starting line index from TAPI system
**             'dwBasePhone'		-	Starting phone index from TAPI system
**             'dwLines'			-	Number of lines owned by this device
**             'dwPhones'			-	Number of phones owned by this device
**             'hProvider'			-	Opaque Provider handle from TAPI
**			   'lpfnCompletion'		-	Completion PROC for any ASYNCH requests.
**
** Returns:    true/false whether TAPI should continue loading the device.
**
** Description: This function is called during providerInit to initialize
**              each device identified by TAPI (group of lines/phones).
**              It is overridden to create threads and other init-time work
**              that might fail (and therefore shouldn't be done in the 
**              constructor).
**
*****************************************************************************/
bool COnSipDevice::Init(DWORD dwProviderId, DWORD dwBaseLine, DWORD dwBasePhone, 
					 DWORD dwLines, DWORD dwPhones, HPROVIDER hProvider, 
					 ASYNC_COMPLETION lpfnCompletion)
{
	// Add the switch information so others can identify this provider.
	SetSwitchInfo(SWITCH_NAME);

	// Turn on the interval timer for this device for once every second
	SetIntervalTimer(1000);

	// Pass through to the base class and let it initialize the line and phone 
	// devices.  After this call, each of the objects will be available.
	// In addition, the providerID information will be filled in.
	if (CTSPIDevice::Init (dwProviderId, dwBaseLine, dwBasePhone, dwLines, dwPhones, hProvider, lpfnCompletion))
	{
		// Tell TAPI to continue loading our provider.
		return true;
	}
	return false;

}// COnSipDevice::Init

bool COnSipDevice::DRV_DropCall(const COnSipLine * pLine, CTSPICallAppearance* pCall)
{
	Logger::log_debug( _T("COnSipDevice::DRV_DropCall pLine=%p pCall=%p pCall.CallId=%ld"), pLine, pCall, pCall->GetCallID() );

	CriticalSectionScope css(&m_cs);
	if ( m_OnSipTapi.get() == NULL )
	{
		Logger::log_error( _T("COnSipDevice::DRV_DropCall callId=%ld notavail"), pCall->GetCallID() );
		return false;
	}
	m_OnSipTapi->DropCall(pCall->GetCallID());
	return true;
}

long COnSipDevice::DRV_MakeCall(const COnSipLine* /*pLine*/, const TString& strDigits, DWORD dwCountryCode)
{
	Logger::log_debug( _T("COnSipDevice::DRV_MakeCall digits=%s dwCC=%ld"), strDigits.c_str(), dwCountryCode );

	CriticalSectionScope css(&m_cs);
	if ( m_OnSipTapi.get() == NULL )
	{
		Logger::log_error( _T("COnSipDevice::DRV_MakeCall digits=%s OnSipTapi notavail"), strDigits.c_str() );
		return -1;
	}
	return m_OnSipTapi->MakeCall( strDigits.c_str() );
}

// This is called by the connection object to open the device.
//virtual 
bool COnSipDevice::OpenDevice (CTSPIConnection* pConn)
{
	// Cast the connection to a line
	CTSPILineConnection* pLine = dynamic_cast<CTSPILineConnection*>(pConn);

	Logger::log_debug( _T("COnSipDevice::OpenDevice enter pConn=%p pLine=%p"), pConn, pLine );
	if ( m_hConnThread != NULL )
	{
		// TODO: Verify OpenDevice only called once system-wide
		Logger::log_error( _T("COnSipDevice::OpenDevice connectionThread already created") );
		_ASSERT(false);
		return false;
	}

	// Reset the OpenDevice event, this will be reset once the ConnectionThread
	// has gone through the initialization of trying to open the XMPP device
	ResetEvent(m_hOpenDevice);
	// Clear Stop event used to signal stop in the connection thread
	ResetEvent(m_hevtStop);

	// Create our connection thread which manages the connection to the hardware.
	UINT uiThread;
	m_hConnThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, MainConnThread, static_cast<void*>(this), 0, &uiThread));

	// If the thread failed to create, output an error and tell TAPI to stop
	// loading us (fatal initialization error).
	if (m_hConnThread == NULL)
	{
		Logger::log_error( _T("COnSipDevice::OpenDevice failed to create connection thread") );
		return false;
	}

	Logger::log_debug( _T("COnSipDevice::OpenDevice wait Init..") );

	// Wait for the ConnectionThread to get started and go through the 
	// init code of the XMPP
	HANDLE handles[] = { m_hOpenDevice, m_hConnThread };
	DWORD dwWait = WaitForMultipleObjects( 2, handles, FALSE, 30000 );

	Logger::log_debug( _T("COnSipDevice::OpenDevice wait Init done.. dwWait=%ld"), dwWait );

	// If OpenDevice event, then it initialized ok!
	if ( dwWait == WAIT_OBJECT_0 )
	{
		Logger::log_debug( _T("COnSipDevice::OpenDevice init success") );
		return true;
	}

	// If connection thread, then it was an error
	if ( dwWait == WAIT_OBJECT_1 )
	{
		Logger::log_error( _T("COnSipDevice::OpenDevice init ERROR") );
		m_hConnThread = NULL;
		return false;
	}

	// Must be a timeout, still not initialized, assume error
	_ASSERT( dwWait == WAIT_TIMEOUT );
	Logger::log_error( _T("COnSipDevice::OpenDevice timeout wait for init") );
	_killThreads();
	return false;
}

// Kill the threads
void COnSipDevice::_killThreads()
{
	Logger::log_debug(_T("COnSipDevice::_killThreads enter.. %x"),m_hConnThread);

	if ( m_hConnThread != NULL )
	{
		// Stop the input thread and wait for it to terminate.
		SetEvent (m_hevtStop);
		WaitForSingleObject(m_hConnThread, 10000);
		CloseHandle(m_hConnThread);
		m_hConnThread = NULL;
	}

	Logger::log_debug(_T("COnSipDevice::_killThreads exit.."));
}

// This is called by the connection object to close the device.
//virtual 
bool COnSipDevice::CloseDevice (CTSPIConnection* /*pConn*/)
{
	Logger::log_debug( _T("COnSipDevice::CloseDevice enter") );

	// Kill connection thread
	_killThreads();

	Logger::log_debug( _T("COnSipDevice::CloseDevice exit success") );
	return true;
}

/*****************************************************************************
** Procedure:  COnSipDevice::ConnectionThread
**
** Arguments:  void
**
** Returns:    Thread return code
**
** Description:  This thread manages the communication connection to the hardware.
**
*****************************************************************************/
unsigned COnSipDevice::ConnectionThread()
{
	Logger::log_debug(_T("COnSipDevice::ConnectionThread enter for dwProviderID=%ld"), m_dwProviderId );

	// Create the OnSipTapi object
	{
		Logger::log_debug(_T("COnSipDevice::ConnectionThread create OnSipTapi"));
		CriticalSectionScope css(&m_cs);
		m_OnSipTapi.reset( new OnSipTapi() );
	}

	// Read the Logon info from the registry
	tstring userName = GetSP()->ReadProfileString(m_dwProviderId, REG_USERNAME, _T("") );
	tstring password = Strings::decryptString( GetSP()->ReadProfileString(m_dwProviderId, REG_PASSWORD, _T("") ), KEY_VALUE );
	tstring domain =  GetSP()->ReadProfileString(m_dwProviderId, REG_DOMAIN, _T("") );

	Logger::log_debug(_T("COnSipDevice::ConnectionThread userName=%s pwd=%d domain=%s"), userName.c_str(), password.length(), domain.c_str() );

	LoginInfo loginInfo(userName,password,domain);

	// connect and do intialization
	// Keep track of the current init state stype
	OnSipInitStatesType::InitStatesType initStateType = OnSipInitStatesType::NOTSET;
	if ( !m_OnSipTapi->InitOnSipTapi(loginInfo,m_hevtStop,&initStateType) )
	{
		Logger::log_error(_T("COnSipDevice::ConnectionThread init error") );
		m_OnSipTapi->Disconnect(false);
		CriticalSectionScope css(&m_cs);
		m_OnSipTapi.reset( NULL );
		return false;
	}

	// Signal back to main thread that we are initialized
	SetEvent( m_hOpenDevice );

	Logger::log_debug(_T("COnSipDevice::ConnectionThread loop..."));

	// List of events that we get from OnSipXmpp engine
	list<OnSipTapiCall> lstOnSipCallEvents;
	list<OnSipInitState> lstOnSipInitEvents;
	list<COnSip_CallEvent *> lstCallEvents;
	list<COnSip_ConnectEvent *> lstConnectEvents;

	// Loop around trying to connect and then receiving data
	bool fConnected = true;
	while (WaitForSingleObject(m_hevtStop, 0) == WAIT_TIMEOUT )
	{
		// Keep trying to connect
		while (!fConnected)
		{
			Logger::log_trace(_T("COnSipDevice::ConnectionThread tryConnect"));
			if ( !m_OnSipTapi->InitOnSipTapi(loginInfo,m_hevtStop,&initStateType) )
			{
				// If fatal type of initialization (e.g. authorize),
				// then force the line close, no need to keep trying
				if ( initStateType == OnSipInitStatesType::FATAL )
				{
					Logger::log_error(_T("COnSipDevice::ConnectionThread tryConnect fatal error"));
					// Force our line to close, this will then result in thread shutting down
					ForceClose();
				}

				// Failed, sleep for 30 seconds and try again.
				if (WaitForSingleObject(m_hevtStop, 30000) != WAIT_TIMEOUT)
					break;
			}
			else
			{
				Logger::log_debug(_T("COnSipDevice::ConnectionThread connected"));
				fConnected = true;
				OnConnect(true);
			}
		}

		if ( fConnected )
		{
			if ( !m_OnSipTapi->Poll() )
			{
				Logger::log_error( _T("COnSipDevice::ConnectionThread poll error") );
				fConnected = false;
				OnConnect(false);
				// Wait a little bit just to check to see if we are disconnecting because of shutdown.
				if (WaitForSingleObject(m_hevtStop, 10000) != WAIT_TIMEOUT)
					break;
				Logger::log_error( _T("COnSipDevice::ConnectionThread poll error waitdone") );
				continue;
			}

			bool bGotHit=false;

			// See if we have any new call events
			if ( m_OnSipTapi->GetCallEvents(lstOnSipCallEvents) )
			{
				Logger::log_debug( _T("COnSipevice::ConnectionThread haveCallEvents=%d"), lstOnSipCallEvents.size() );
				lstCallEvents = _getEvents(lstOnSipCallEvents);
				bGotHit=true;
			}

			// See if we have any new init state events
			if ( m_OnSipTapi->GetInitEvents(lstOnSipInitEvents) )
			{
				Logger::log_debug( _T("COnSipevice::ConnectionThread haveInitEvents=%d"), lstOnSipInitEvents.size() );
				lstConnectEvents = _getEvents(lstOnSipInitEvents);
				bGotHit=true;
			}

			// If we have events, process them
			if ( bGotHit )
			{
				Logger::log_debug( _T("COnSipevice::ConnectionThread haveEvents callevts=%d initevts=%d"), lstCallEvents.size(), lstConnectEvents.size() );

				// Process any call events
				list<COnSip_CallEvent *>::iterator callIter = lstCallEvents.begin();
				while ( callIter != lstCallEvents.end() )
				{
					_processEvent( *callIter );
					callIter++;
				}
				lstCallEvents.clear();

				// Process any init events
				list<COnSip_ConnectEvent *>::iterator initIter = lstConnectEvents.begin();
				while ( initIter != lstConnectEvents.end() )
				{
					_processEvent( *initIter );
					initIter++;
				}
				lstConnectEvents.clear();

				// Update the Init State Machine current state type
				OnSipInitStates::InitStates curState = m_OnSipTapi->GetInitStateMachineState();
				OnSipInitStatesType::InitStatesType curStateType  = OnSipInitStatesType::GetInitStatesType( curState  );

				// If the InitStateMachine type has changed!!
				if ( curStateType != initStateType )
				{
					Logger::log_debug( _T("COnSipevice::ConnectionThread initStateChange prev=%s cur=%s state=%s"),
						OnSipInitStatesType::InitStatesTypeToString(initStateType), OnSipInitStatesType::InitStatesTypeToString(curStateType),
						OnSipInitStates::InitStatesToString( curState ) );
					initStateType = curStateType;
					// Report OnConnect state
					OnConnect( initStateType == OnSipInitStatesType::OK );
					// If disconnected, clear our connected flag
					if ( initStateType == OnSipInitStatesType::DISCONNECTED)
					{
						Logger::log_debug( _T("COnSipevice::ConnectionThread initStateChange disconnected") );
						m_OnSipTapi->Disconnect(false);
						fConnected = false;
					}
				}
			}
		}
	}

	Logger::log_debug(_T("COnSipDevice::ConnectionThread exiting..."));

	m_OnSipTapi->Disconnect(fConnected);

	// Delete the OnSipTapi object
	{
		Logger::log_debug(_T("COnSipDevice::ConnectionThread del OnSipTapi"));
		CriticalSectionScope css(&m_cs);
		m_OnSipTapi.reset( NULL );
	}

	Logger::log_debug(_T("COnSipDevice::ConnectionThread exit"));
	return 0;

}// COnSipDevice::ConnectionThread

// Processes the event in TAPI, passes it on to the TAPI thread pools
bool COnSipDevice::_processEvent( COnSipEvent* pEvent )
{
	// Check NULL, shouldn't occur
	_ASSERT( pEvent != NULL );
	if ( pEvent == NULL )
		return false;

	// Determine the owner of the event
	CTSPIConnection* pConnOwner = LocateOwnerFromEvent(pEvent);

	// Add it to the pool manager
	bool fDispatched = (pConnOwner != NULL &&  m_mgrThreads.Add(pConnOwner, pConnOwner->GetPermanentDeviceID(), pEvent));

	Logger::log_debug( _T("COnSipDevice::_processEvent evtType=%d fDispatched=%d pEvent=%p pConnOwner=%p "), pEvent->getType(), fDispatched, pEvent, pConnOwner );

	// Delete it if we could not find an owner. This could happen if the TSP
	// never saw required preceding events (such as call creations) because it 
	// wasn't running at the time or an event loss occurred due to link failure.
	if (!fDispatched)
		TPM_DelEvent(pEvent);

	return fDispatched;
}

// Convert list of OnSipTapiCall events to the TAPI Call Events
list<COnSip_CallEvent *> COnSipDevice::_getEvents( list< OnSipTapiCall >& lstCallEvents )
{
	list<COnSip_CallEvent *> lstEvents;

	list<OnSipTapiCall>::iterator iter = lstCallEvents.begin();
	while ( iter != lstCallEvents.end() )
	{
		lstEvents.push_back( new COnSip_CallEvent( (*iter) ) );
		iter++;
	}
	return lstEvents;
}

// Convert list of OnSipInitState events to TAPI Connect events
list<COnSip_ConnectEvent *> COnSipDevice::_getEvents( list< OnSipInitState >& lstInitEvents )
{
	list<COnSip_ConnectEvent *> lstEvents;

	list<OnSipInitState>::iterator iter = lstInitEvents.begin();
	while ( iter != lstInitEvents.end() )
	{
		lstEvents.push_back( new COnSip_ConnectEvent( (*iter) ) );
		iter++;
	}
	return lstEvents;
}


/*****************************************************************************
** Procedure:  COnSipDevice::LocateOwnerFromEvent
**
** Arguments:  'pEvent' - Event received from the hardware
**
** Returns:    CTSPIConnection which "owns" the event.
**
** Description: This function examines the received event and determines
**              which connection object the event belongs to.
**
*****************************************************************************/
CTSPIConnection* COnSipDevice::LocateOwnerFromEvent(COnSipEvent* pEvent)
{
	// Get Call Event if it is that type
	const COnSip_CallEvent* pCallEvent = COnSipEvent::getCallEvent(pEvent);

	// If Call Event, get the unique CallId
	long callID = ( pCallEvent == NULL ) ? 0 : pCallEvent->CallId();

	// We only have 1 potential owner, the LINE device
	CTSPIConnection* pConnOwner = FindLineConnectionByPermanentID(LINE_ID_UNIQUE);
	Logger::log_debug( _T("COnSipDevice::LocateOwnerFromEvent pConnOwner=%p, pCallEvent=%p, callID=%ld"), pConnOwner, pCallEvent, callID );
	if ( pConnOwner != NULL )
		return pConnOwner;
	Logger::log_error( _T("COnSipDevice::LocateOwnerFromEvent no LINE found") );
	return NULL;
}// COnSipDevice::LocateOwnerFromEvent
