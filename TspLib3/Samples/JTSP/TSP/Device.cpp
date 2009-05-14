/***************************************************************************
//
// DEVICE.CPP
//
// JulMar Sample TAPI Service provider for TSP++ version 3.00
// Handles raw PBX device events and connection to PBX
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
#include <process.h>
#include "jtsp.h"
#include <spbstrm.h>

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-------------------------------------------------------------------------------*/
// CONSTANTS
/*-------------------------------------------------------------------------------*/
LPCTSTR g_pszIPAddress = _T("IPAddress");
LPCTSTR g_pszIPPort = _T("IPPort");

/*****************************************************************************
** Procedure:  MainInputThread
**
** Arguments:  'pDevice'	- Device object
**
** Returns:    void
**
** Description:  Main input thread
**
*****************************************************************************/
static unsigned __stdcall MainInputThread(void* pParam)
{
	reinterpret_cast<CJTDevice*>(pParam)->InputThread();
	_endthreadex(0);
	return 0;

}// InputThread

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
static unsigned __stdcall MainConnThread(void* pParam)
{
	reinterpret_cast<CJTDevice*>(pParam)->ConnectionThread();
	_endthreadex(0);
	return 0;

}// ConnThread

/*****************************************************************************
** Procedure:  CJTDevice::CJTDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description: Constructor for our device object
**
*****************************************************************************/
CJTDevice::CJTDevice()
{
	// Create the stop event.
	m_hevtStop = CreateEvent (NULL, true, false, NULL);
	m_hevtData = CreateEvent (NULL, false, false, NULL);
	InitializeCriticalSection(&m_csData);

}// CJTDevice::CJTDevice

/*****************************************************************************
** Procedure:  CJTDevice::~CJTDevice
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for the device object
**
*****************************************************************************/
CJTDevice::~CJTDevice()
{
	_TSP_TRACE(IDS_DEVSTOP, GetProviderID());

	// Close our socket connection
	m_connPBX.Close();

	// Stop the input thread.
	SetEvent (m_hevtStop);

	// Wait for the two threads to terminate
	HANDLE arrThreads[] = { m_hInputThread, m_hConnThread };
	WaitForMultipleObjects(2, arrThreads, TRUE, 5000);
	CloseHandle(m_hInputThread);
	CloseHandle(m_hConnThread);

	// Shutdown Winsock
	while (WSACleanup() == WSAEINPROGRESS)
		Sleep(100);

	// Close all our handles
	CloseHandle(m_hevtStop);
	CloseHandle(m_hevtData);
	DeleteCriticalSection(&m_csData);

	// Delete any left over events/data
	m_arrInput.clear();

}// CJTDevice::~CJTDevice

/*****************************************************************************
** Procedure:  CJTDevice::read
**
** Arguments:  'istm' - Input iostream
**
** Returns:    iostream reference
**
** Description: This is called to read information in from the registry.
**
*****************************************************************************/
TStream& CJTDevice::read(TStream& istm)
{
	// Read our port and IP address
	istm >> m_strIPAddress >> m_nPort;

	// Always call the base class
	return CTSPIDevice::read(istm);

}// CJTDevice::read

/*****************************************************************************
** Procedure:  CJTDevice::Init
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
**              It is overriden to create threads and other init-time work
**              that might fail (and therefore shouldn't be done in the 
**              constructor).
**
*****************************************************************************/
bool CJTDevice::Init(DWORD dwProviderId, DWORD dwBaseLine, DWORD dwBasePhone, 
					 DWORD dwLines, DWORD dwPhones, HPROVIDER hProvider, 
					 ASYNC_COMPLETION lpfnCompletion)
{
	// Output a device startup banner
	_TSP_TRACE(IDS_DEVSTART, dwProviderId, dwLines);

	// Initialize WinSock
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (nResult != 0)
	{
		_TSP_TRACE(IDS_NOWINSOCK, WSAGetLastError());
		return false;
	}

	// Add the switch information so others can identify this provider.
	SetSwitchInfo(_T("JulMar Sample Client/Server TAPI Service Provider"));

	// Turn on the interval timer for this device for once every second
	SetIntervalTimer(1000);

	// Pass through to the base class and let it initialize the line and phone 
	// devices.  After this call, each of the objects will be available.
	// In addition, the providerID information will be filled in.
	if (CTSPIDevice::Init (dwProviderId, dwBaseLine, dwBasePhone, dwLines, dwPhones, hProvider, lpfnCompletion))
	{
		// Create our connection thread which manages the connection to the PBX and
		// a thread to manage the events being queued by the connector thread.
		UINT uiThread;
		m_hConnThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, MainConnThread, static_cast<void*>(this), 0, &uiThread));
		m_hInputThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, MainInputThread, static_cast<void*>(this), 0, &uiThread));

		// If either failed to create the output an error and tell TAPI to stop
		// loading us (fatal initialization error).
		if (m_hConnThread == NULL || m_hInputThread == NULL)
		{
			_TSP_TRACE(IDS_NOTHREAD, GetLastError());
			return false;
		}

		// Tell TAPI to continue loading our provider.
		return true;
	}
	return false;

}// CJTDevice::Init

/*****************************************************************************
** Procedure:  CJTDevice::ConnectionThread
**
** Arguments:  void
**
** Returns:    Thread return code
**
** Description:  This thread manages the socket connection to the PBX.
**               It creates and receives the strings from the socket and
**               queues them into a string array for the input thread to
**               parcel out to worker threads.  This is done in two stages to
**               ensure that we don't overrun the socket buffer by spending
**               to much time on this thread.
**
**               If we knew that the device would only send a minimal amount
**               of data, or that it was easy to decode the data stream and
**               assign a line owner, we could combine the input/conn thread
**               together.
**
*****************************************************************************/
unsigned CJTDevice::ConnectionThread()
{
	// Loop around trying to connect and then receiving data
	bool fConnected = false;
	while (WaitForSingleObject(m_hevtStop, 0) == WAIT_TIMEOUT)
	{
		// If we are not currently connected to the PBX then perform a connect.
		if (fConnected == false)
		{
			// Attempt to connect to the PBX using Winsock
			_TSP_DTRACE(_T("Connecting to the PBX using \"%s\" port %d\n"), m_strIPAddress.c_str(), m_nPort);
			if (!m_connPBX.Connect(m_strIPAddress.c_str(), m_nPort))
			{
				// Failed, sleep for 30 seconds and try again.
				if (WaitForSingleObject(m_hevtStop, 30000) != WAIT_TIMEOUT)
					break;
			}

			// Connected to PBX - Send a "LOGON" request to the simulator using 
			// the extension zero so we receive all events system-wide.
			else 
			{
				// Make sure we aren't shutting down if so, ignore the connect
				if (WaitForSingleObject(m_hevtStop, 0) != WAIT_TIMEOUT)
					break;

				_TSP_DTRACE(_T("Connected to PBX - Sending LOGON command\n"));
				fConnected = true;
				m_connPBX.SendEvent(PBXCMD_LOGON, 0);
			}
		}

		// We are connected, receive the next data block
		else
		{
			TString strData;
			if (!m_connPBX.WaitForData(strData))
			{
				_TSP_DTRACE(_T("Socket error: Connection to PBX lost!\n"));
				fConnected = false;
				OnConnect(false);
			}
			else
			{
				// Output our recieved buffer
				_TSP_DTRACEX(TRC_DUMP, _T("Rcv: %s\n"), strData.c_str());

				// If we received a logon response, then mark us as
				// connected and send the initialization events (VER/QAS)
				// to get the PBX to send back status information.
				if (!lstrcmpi(_T("ACK;LO"), strData.substr(0,6).c_str()))
				{
					// We are now connected
					OnConnect(true);

					// Query the agent states and version
					m_connPBX.SendEvent(PBXCMD_VERSION);
					m_connPBX.SendEvent(PBXCMD_QAS);
					continue;
				}

				// Received a data string, drop into our array for processing and
				// spin around to get the next event from the PBX
				EnterCriticalSection(&m_csData);
				try
				{
					m_arrInput.push_back(strData);
				}
				catch (...)
				{
				}

				// Now set the event to signal the input thread that there is 
				// work pending in our array
				LeaveCriticalSection(&m_csData);
				SetEvent(m_hevtData);
			}
		}
	}

	return 0;

}// CJTDevice::ConnectionThread

/*****************************************************************************
** Procedure:  CJTDevice::InputThread
**
** Arguments:  void
**
** Returns:    Thread return code
**
** Description:  Main input thread
**
*****************************************************************************/
unsigned CJTDevice::InputThread()
{
	HANDLE arrHandles[] = { m_hevtData, m_hevtStop };
	for ( ;; )
	{
		// If either of our events go bad or the stop event is signaled then exit.
		if (WaitForMultipleObjects(2, arrHandles, FALSE, INFINITE) != WAIT_OBJECT_0)
			break;

		// Otherwise, grab the waiting string
		TString strData;
		EnterCriticalSection(&m_csData);
		try
		{
			if (m_arrInput.size() > 0)
			{
				strData = m_arrInput[0];
				m_arrInput.pop_front();
			}
		}
		catch (...)
		{
		}

		// If there is more data waiting, allow this thread to fall back through
		if (m_arrInput.size() != 0)
			SetEvent(m_hevtData);

		LeaveCriticalSection(&m_csData);

		// Now process the string we received.  Build a data block from the data
		// and then queue it.
		if (strData.empty() == false)
			QueuePacket(m_facEvents.Create(strData));
	}

	return 0;

}// CJTDevice::InputThread

/*****************************************************************************
** Procedure:  CJTDevice::QueuePacket
**
** Arguments:  'pevtBlock' - EventBlock to locate an owner for
**
** Returns:    void
**
** Description: This is one of the main functions in the TSP.  It is
**              responsible for examining an incoming event packet and 
**              determining which line or phone device to reflect the packet
**              to and then queueing it for that device.
**
*****************************************************************************/
void CJTDevice::QueuePacket(CEventBlock* pBlock)
{	
	bool fDispatched = false;

#ifdef _DEBUG
	// Output a dump of the received block with all it's elements.
	_TSP_DTRACE(pBlock->Dump().c_str());
#endif

	// Determine which line(s) and phone(s) to reflect this message to
	// by examining the contents of the event block.  First look for an
	// extension.  This is the most common method of identification for this
	// particular device (JPBX).
	const CPEExtension* pExtension = dynamic_cast<const CPEExtension*>
			(pBlock->GetElement(CPBXElement::Extension));
	if (pExtension != NULL)
	{
		// Now determine if this is a phone or line request based on the type
		CTSPIConnection* pConn = NULL;
		if (pBlock->GetEventType() == CEventBlock::CommandResponse)
		{
			const CPECommand* pCommand = dynamic_cast<const CPECommand*>
				(pBlock->GetElement(CPBXElement::Command));
			if (pCommand != NULL)
			{
				if (pCommand->GetCommand() >= CPECommand::SetGain)
					pConn = FindPhoneConnectionByPermanentID(pExtension->GetExtension());
				else
					pConn = FindLineConnectionByPermanentID(pExtension->GetExtension());
			}
		}
		else if (pBlock->GetEventType() >= CEventBlock::DisplayChanged)
			pConn = FindPhoneConnectionByPermanentID(pExtension->GetExtension());
		else
			pConn = FindLineConnectionByPermanentID(pExtension->GetExtension());

		// If we found a connection for this request then add it to the 
		// appropriate queue for dispatch.
		if (pConn != NULL)
			fDispatched = m_mgrThreads.Add(pConn, pConn->GetPermanentDeviceID(), pBlock);
	}

	// The second mechanism of looking up a request is by call-id. Many of the events
	// coming in from the switch have a call-id to indicate which call the event is
	// happening on. Specifically, the "RELEASE CALL" event has only a call-id.
	if (!fDispatched)
	{
		const CPECallID* pCallID = dynamic_cast<const CPECallID*>
				(pBlock->GetElement(CPBXElement::CallID));
		if (pCallID != NULL)
		{
			// Get the call hub for the given callid. The call hub maps call-ids
			// to multiple call appearance objects in the TSP library at the 
			// device level. Each call has a pointer to it's respective call hub
			// object which allows it to "see" other sides of the same call.
			const CTSPICallHub* pHub = FindCallHub(pCallID->GetCallID());
			if (pHub != NULL)
			{
				// If we have no calls in the hub then we cannot dispatch this event.
				if (pHub->GetHubCount() > 0)
				{
					// Now determine which call in the hub to use. Walk through
					// each call and look at the line owner and make a semi-intelligent
					// prediction as to which line should receive the event.
					//
					// If this is a "digit dialed" event then forward to all calls.
					// The PBX doesn't tell us WHICH side of the call got the digit
					// so we send to both and let the app figure it out.
					if (pBlock->GetEventType() == CEventBlock::DigitDetected)
					{
						// Addref the block up front On a MP system, the request
						// can actually finish before we cycle around to add it a second
						// time to another call. This ensures that it stays valid at least
						// until the end of the loop.
						pBlock->AddRef();
						for (unsigned int i = 0; i < pHub->GetHubCount(); i++)
						{
							if (i > 0) pBlock->AddRef();
							CTSPICallAppearance* pCall = pHub->GetCall(i);
							fDispatched = m_mgrThreads.Add(pCall->GetLineOwner(),
								pCall->GetLineOwner()->GetPermanentDeviceID(), pBlock);
						}

						// Now decrement the refcount on the request since we are done
						// adding it to different lines.
						pBlock->Decrement();
					}

					// Otherwise look for a non-trunk line to pass the request to.
					// Calls always stay on trunks so any release is typically for
					// a station or VRU - plus this synchronizes quick transfer/release
					// events to a single line key in the thread pool.
					else
					{
						for (unsigned int i = 0; i < pHub->GetHubCount(); i++)
						{
							CTSPICallAppearance* pCall = pHub->GetCall(i);
							if (pCall->GetLineOwner()->GetLineType() != CTSPILineConnection::Trunk)
							{
								fDispatched = m_mgrThreads.Add(pCall->GetLineOwner(),
									pCall->GetLineOwner()->GetPermanentDeviceID(), pBlock);
								break;
							}
						}

						// If we never dispatched, send it to the first call in the hub.
						if (!fDispatched)
						{
							CTSPICallAppearance* pCall = pHub->GetCall(0);
							fDispatched = m_mgrThreads.Add(pCall->GetLineOwner(),
									pCall->GetLineOwner()->GetPermanentDeviceID(), pBlock);
						}
					}
				}
			}
		}
	}

	// Delete it if we could not find an owner - sometimes we get events for which
	// we never saw preceding events (such as call creations) because the TSP wasn't
	// running - this is the most common cause of unowned events.
	if (!fDispatched)
		TPM_DelEvent(pBlock);

}// CJTDevice::QueuePacket




