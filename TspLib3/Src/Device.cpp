/******************************************************************************/
//                                                                        
// DEVICE.CPP - Source code for the TSPIDevice class                      
//                                             
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code for managing the device class which    
// encapsulates a TAPI SP device (managing lines and phones).             
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
#include <process.h>
#include <spbstrm.h>
#include <stdexcept>
#include "spacd.h"

using std::runtime_error;

/*---------------------------------------------------------------------------*/
// Agent capture points for debug entry
/*---------------------------------------------------------------------------*/
extern "C" LONG TSPIAPI TSPI_lineSetAgentGroup(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST const lpGroupList);
extern "C" LONG TSPIAPI TSPI_lineSetAgentState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentState, DWORD dwNextAgentState);
extern "C" LONG TSPIAPI TSPI_lineSetAgentActivity(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwActivityID);
extern "C" LONG TSPIAPI TSPI_lineGetAgentStatus(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTSTATUS lpStatus);
extern "C" LONG TSPIAPI TSPI_lineGetAgentCaps(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTCAPS lpCapabilities);
extern "C" LONG TSPIAPI TSPI_lineGetAgentActivityList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTACTIVITYLIST lpList);
extern "C" LONG TSPIAPI TSPI_lineGetAgentGroupList(DWORD dwDeviceID, DWORD dwAddressID, LPLINEAGENTGROUPLIST lpList);
extern "C" LONG TSPIAPI TSPI_lineAgentSpecific(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwAddressID, DWORD dwAgentExtensionID, LPVOID lpvParams, DWORD dwSize);
extern "C" LONG TSPIAPI TSPI_lineCreateAgent(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENT lphAgent, LPCWSTR pszMachineName, LPCWSTR pszUserName, LPCWSTR pszAgentID, LPCWSTR pszAgentPIN);
extern "C" LONG TSPIAPI TSPI_lineSetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwMeasurementPeriod);
extern "C" LONG TSPIAPI TSPI_lineGetAgentInfo(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo);
extern "C" LONG TSPIAPI TSPI_lineCreateAgentSession(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCWSTR pszAgentPIN, const GUID* pGUID, DWORD dwWorkingAddressID);
extern "C" LONG TSPIAPI TSPI_lineGetAgentSessionList(DWORD dwDeviceID, HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList);
extern "C" LONG TSPIAPI TSPI_lineSetAgentSessionState(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState);
extern "C" LONG TSPIAPI TSPI_lineGetAgentSessionInfo(DWORD dwDeviceID, HAGENTSESSION hAgentSession, LPLINEAGENTSESSIONINFO lpSessionInfo);
extern "C" LONG TSPIAPI TSPI_lineGetQueueList(DWORD dwDeviceID, const GUID* pGroupID, LPLINEQUEUELIST lpQueueList);
extern "C" LONG TSPIAPI TSPI_lineSetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, DWORD dwQueueID, DWORD dwMeasurementPeriod);
extern "C" LONG TSPIAPI TSPI_lineGetQueueInfo(DWORD dwDeviceID, DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo);
extern "C" LONG TSPIAPI TSPI_lineGetGroupList(DWORD dwDeviceID, LPLINEAGENTGROUPLIST lpGroupList);
extern "C" LONG TSPIAPI TSPI_lineSetAgentStateEx(DRV_REQUESTID dwRequestID, DWORD dwDeviceID, HAGENT hAgent, DWORD dwState, DWORD dwNextState);

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::CTSPIDevice
//
// Constructor
//
CTSPIDevice::CTSPIDevice() : CTSPIBaseObject(),
	m_dwProviderId(0xffffffff), m_lpfnCompletionProc(0), m_hProvider(0),
	m_hevtDeviceShutdown(0), m_dwIntervalTimeout(0), m_psmProxy(0),
	m_htIntervalTimer(0), m_htAgentProxy(0)
{
	m_hevtDeviceShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hevtDeviceDeleted = CreateEvent(NULL, TRUE, FALSE, NULL);

}// CTSPIDevice::CTSPIDevice

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::~CTSPIDevice
//
// Destructor for the device object.
// 
// -- Clears all line objects
// -- Waits on interval timer thread
// -- Waits on agent proxy thread
//
CTSPIDevice::~CTSPIDevice()
{
	// Tell any device threads to shutdown
	SetEvent(m_hevtDeviceShutdown);

	// Tell the interval timer thread to exit if it has been started.
	SetIntervalTimer(0);

	// Wait for our child threads to exit to ensure we don't delete the
	// device before all the threads are done accessing it.
	if (m_htIntervalTimer != NULL)
	{
		WaitForSingleObject(m_htIntervalTimer, INFINITE);
		CloseHandle(m_htIntervalTimer);
	}

	// Stop all agent proxy requests.
	if (m_htAgentProxy != NULL)
	{
		WaitForSingleObject(m_htAgentProxy, INFINITE);
		CloseHandle(m_htAgentProxy);
	}

	// Inform the service provider class that the device is now "deleted"
	// and that the processing of TSPI_providerShutdown can continue.
	CloseHandle(m_hevtDeviceShutdown);
	SetEvent(m_hevtDeviceDeleted);

	// CServiceProvider::providerShutdown closes the m_hevtDeviceDeleted
	// handle as it is potentially waiting on it now.

}// CTSPIDevice::~CTSPIDevice

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::Init
//
// This initializer allocates all the connection information for
// each phone/line available and sets up all the device information
// for this device id.
//
bool CTSPIDevice::Init(DWORD dwProviderId, DWORD dwBaseLineId, DWORD dwBasePhoneId, 
		DWORD dwNumLines, DWORD dwNumPhones, HPROVIDER hProvider, 
		ASYNC_COMPLETION lpfnCompletion)
{
    // Store all the information about this device.
    m_dwProviderId = dwProviderId;
    m_lpfnCompletionProc = lpfnCompletion;
	m_hProvider = hProvider;

    _TSP_DTRACE(_T("Device %lx, Base Line=%lx, Count=%lx, Base Phone=%lx, Count=%lx\n"),
            dwProviderId, dwBaseLineId, dwNumLines, dwBasePhoneId, dwNumPhones);
    _TSP_DTRACE(_T("Completion Callback function = %08lx\n"), lpfnCompletion);

	// Ask the derived provider for our registry stream object (v3.0b)
	TStream* pStream = AllocStream();

	// If we are loading from the registry, then give the derived device object
	// an opportunity to load it's information from the registry.
	if (pStream != NULL)
	{
		try
		{
			if (!pStream->open())
				SCHEMA_EXCEPT("bad stream");
			read(*pStream);
		}
		catch (std::exception& e)
		{
			UNREFERENCED_PARAMETER(e);
			_TSP_DTRACE(TRC_WARNINGS, _T("Exception [%s] caught loading provider data from stream\n"), e.what());
			delete pStream;
			return false;
		}

		// Load our line information
		unsigned int i;
		for (i = 0; i < dwNumLines; i++)
		{
			CTSPILineConnection* pLine = GetSP()->CreateLineObject();
			AddLineConnectionInfo(pLine);
			_TSP_DTRACE(_T("Adding Line #%d (id %lx) to device list\n"), i+1, i+dwBaseLineId);
			pLine->InitWithStream(this, (i + dwBaseLineId), i, *pStream);
#ifdef _DEBUG
			_TSP_DTRACEX(TRC_OBJECTS, _T("%s\n"), pLine->Dump().c_str());
#endif
		}

		// Load the phone information.
		for (i = 0; i < dwNumPhones; i++)
		{
			CTSPIPhoneConnection* pPhone = GetSP()->CreatePhoneObject();
			AddPhoneConnectionInfo(pPhone);
			pPhone->InitWithStream(this, (i+dwBasePhoneId), i, *pStream);
#ifdef _DEBUG
			_TSP_DTRACEX(TRC_OBJECTS, _T("%s\n"), pPhone->Dump().c_str());
#endif
		}
		LoadObjects(*pStream);
		pStream->close();
		delete pStream;
	}
	
	// Otherwise the derived provider chose not to use the new registry
	// object storage facility - invoke the Init() method for normal creation.
	else
	{
		// Create a line connection for each listed line device.
		unsigned int i;
		for (i = 0; i < dwNumLines; i++)
		{
			CTSPILineConnection* pConn = GetSP()->CreateLineObject();
			AddLineConnectionInfo (pConn);
			_TSP_DTRACE(_T("Adding Line #%d (id %lx) to device list\n"), i+1, i+dwBaseLineId);
			pConn->Init(this, i + dwBaseLineId, i);
#ifdef _DEBUG
			_TSP_DTRACEX(TRC_OBJECTS, _T("%s\n"), pConn->Dump().c_str());
#endif
		}

		// Do the same for the phone objects
		for (i = 0; i < dwNumPhones; i++)
		{
			CTSPIPhoneConnection* pConn = GetSP()->CreatePhoneObject();
			AddPhoneConnectionInfo(pConn);
			_TSP_DTRACE(_T("Adding Phone #%d (id %lx) to device list\n"), i+1, i+dwBasePhoneId);
			pConn->Init(this, i+dwBasePhoneId, i);
#ifdef _DEBUG
			_TSP_DTRACEX(TRC_OBJECTS, _T("%s\n"), pConn->Dump().c_str());
#endif
		}
	}

	// Now that all lines and phones are initialized, run back through the list and
	// call the virtual function to create the "tapi/terminal" entries for each line
	// and phone device.
	for (unsigned i = 0; i < dwNumLines; i++)
	{
		CTSPILineConnection* pLine = GetLineConnectionInfo(i);
		if (pLine->GetTerminalCount() > 0 && pLine->GetDeviceClass(_T("tapi/terminal")) == NULL)
			pLine->CreateTapiTerminalDeviceClassInfo();
	}

	return true;

}// CTSPIDevice::Init

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::SetDeviceStatus
//
// This function changes the device status for all the lines associated
// with the device.
//
void CTSPIDevice::SetDeviceStatus(bool fConnect)
{
	// Mark each line as connected/disconnected based on our connection
	// to the hardware.
	unsigned int count = GetLineCount();
	for (unsigned int i = 0; i < count; i++)
	{
		CTSPILineConnection* pLine = GetLineConnectionInfo(i);
		if (pLine != NULL)
		{
			pLine->DevStatusConnected(fConnect);
			pLine->DevStatusInService(fConnect);
			pLine->RecalcLineFeatures(true);
		}
	}

}// CTSPIDevice::SetDeviceStatus

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::Save
//
// This function saves the state of the device and all child objects back
// to the registry.  It can be used to make changes to the device within
// the TSP (for plug&play).
//
bool CTSPIDevice::Save()
{
	// Ask the derived provider for our registry stream object (v3.0b)
	TStream* pStream = AllocStream();

	// If we can save to the registry, then do so.
	if (pStream == NULL)
		return false;

	try
	{
		if (!pStream->open())
			SCHEMA_EXCEPT("stream error");
		write(*pStream);
	}
	catch (std::exception& e)
	{
		UNREFERENCED_PARAMETER(e);
		_TSP_DTRACE(TRC_WARNINGS, _T("Exception [%s] caught saving provider data to stream\n"), e.what());
		delete pStream;
		return false;
	}

	// Save our line information
	int nCount = static_cast<int>(m_arrayLines.size()), i;
	for (i = 0; i < nCount; i++)
		m_arrayLines[i]->SaveToStream(*pStream);

	// Save the phone information.
	nCount = static_cast<int>(m_arrayPhones.size());
	for (i = 0; i < nCount; ++i)
		m_arrayPhones[i]->SaveToStream(*pStream);
	
	// Now save the agent information and close the stream
	SaveObjects(*pStream);
	pStream->close();
	delete pStream;

	return true;

}// CTSPIDevice::Save

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::read
//
// Read an entry from the registry
//
TStream& CTSPIDevice::read(TStream& istm)
{
	// Read and validate the provider ID stored in the stream.
	DWORD dwProviderID;
	istm >> dwProviderID;
	_TSP_ASSERTE(dwProviderID == m_dwProviderId);

	return istm;

}// CTSPIDevice::read

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::write
//
// Writes the object to a stream
//
TStream& CTSPIDevice::write(TStream& ostm) const
{
	// Write our permanent provider id.
	ostm << m_dwProviderId;
	return ostm;

}// CTSPIDevice::write

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::SetIntervalTimer
//
// Sets the current interval timer timeout.  If the timer is set
// to zero the thread associated with the timer is destroyed.
//
void CTSPIDevice::SetIntervalTimer (DWORD dwTimeout)
{
	// Is it the same as before?
	if (m_dwIntervalTimeout == dwTimeout)
		return;

	// Is it now zero?  Delete any existing interval timer.
	if (dwTimeout == 0)
		m_dwIntervalTimeout = 0;

	// Otherwise reset the timeout
	else
	{
		// Set the new timeout
		m_dwIntervalTimeout = dwTimeout;

		// Create our objects if necessary
		if (m_htIntervalTimer == NULL)
		{
			UINT uiThread;
			m_htIntervalTimer = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, tsplib_IntervalTimerThread, static_cast<void*>(this), 0, &uiThread));
			_TSP_ASSERTE(m_htIntervalTimer != NULL);
		}
	}

}// CTSPIDevice::SetIntervalTimer

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OnAsynchRequestComplete
//
// An asynchronous request has completed and TAPI needs to be 
// notified.
//
void CTSPIDevice::OnAsynchRequestComplete(LONG lResult, CTSPIRequest* pRequest)
{
    _TSP_ASSERTE(pRequest != NULL);
    DRV_REQUESTID dwReqId = pRequest->GetAsynchRequestId();

	// If the request is for a PROXY (agent) command, then send the
	// response back through the proxy application.
	int iCommand = pRequest->GetCommand();
	if (iCommand >= REQUEST_SETAGENTGROUP && iCommand <= REQUEST_AGENTSPECIFIC)
	{
		ProxyResponse(dwReqId, lResult);
		return;
	}

    // Get the asynchronous request id.  If it is non-zero, inform TAPI
    // about the request being completed.  If it is zero, this is an internally
    // generated request which has no cooresponding result to TAPI (we already
    // said it completed ok).
    if (dwReqId && m_lpfnCompletionProc != NULL)
    {
		_TSP_ASSERTE(m_lpfnCompletionProc != NULL);
		(*m_lpfnCompletionProc)(dwReqId, lResult);
#ifdef _DEBUG
		_TSP_DTRACEX(TRC_REQUESTS, _T("OnAsynchRequestComplete %s"), pRequest->Dump().c_str());
#endif
    }

}// CTSPIDevice::OnAsynchRequestComplete

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::CreateLine
//
// Create a new line device object and signal TAPI to give us a new
// line device id.
//
CTSPILineConnection* CTSPIDevice::CreateLine(DWORD_PTR dwItemData /*=0*/)
{
    LINEEVENT lpfn = GetSP()->GetLineCreateProc();
    if (lpfn == NULL || GetProcAddress(GetSP()->GetResourceInstance(), "TSPI_providerCreateLineDevice") == NULL)
    {
        // If we are here, we probably don't have "TSPI_providerEnumDevices" exported, or the
        // version of TAPI isn't sufficient to handle it.
        _TSP_DTRACE(_T("WARNING: Attempted to dynamically create line without TAPI support\n"));
        _TSP_ASSERT(false);
        return NULL;
    }

    CTSPILineConnection* pConn = GetSP()->CreateLineObject();
    pConn->Init(this, reinterpret_cast<DWORD>(pConn), GetLineCount(), dwItemData);
    AddLineConnectionInfo(pConn);

    (*lpfn)(NULL, NULL, LINE_CREATE, 
			reinterpret_cast<DWORD_PTR>(GetProviderHandle()), 
			reinterpret_cast<DWORD_PTR>(pConn), 0);

    _TSP_DTRACEX(TRC_STATS, _T("Dynamically created line object <0x%lx>\n"), pConn);

    return pConn;

}// CTSPIDevice::CreateLine

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemoveLine
//
// Remove a line device from our device array.
//
void CTSPIDevice::RemoveLine (CTSPILineConnection* pLine)
{
    LINEEVENT lpfn = GetSP()->GetLineCreateProc();
    if (lpfn == NULL)
    {
        // If we are here, we probably don't have "TSPI_providerEnumDevices" exported, or the
        // version of TAPI isn't sufficient to handle it.
        _TSP_DTRACE(_T("WARNING: Attempted to dynamically remove line without TAPI support\n"));
        _TSP_ASSERT(false);
        return;
    }

	// The derived service provider will be responsible for
	// renumbering the line connection array when providerShutdown
	// is called.
	(*lpfn)(NULL, NULL, LINE_REMOVE, static_cast<DWORD_PTR>(pLine->GetDeviceID()), 0, 0);

	// Mark the line as REMOVED.  Once it is closed it will be marked
	// as DELETED and all further references to it will be returned
	// in the TSPI_xxx layer.
	pLine->m_dwFlags |= CTSPIConnection::_IsRemoved;

	// If the line is closed, then go ahead and delete it now.
	if (pLine->GetLineHandle() == NULL)
		pLine->m_dwFlags |= CTSPIConnection::_IsDeleted;

	// Remove the permanent line identifier from our map.
	CEnterCode sLock(this);
	m_mapLineConnections.erase(pLine->GetPermanentDeviceID());

}// CTSPIDevice::RemoveLine

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::CreatePhone
//
// Create a new phone device object and signal TAPI to give us a new
// phone device id.
//
CTSPIPhoneConnection* CTSPIDevice::CreatePhone(DWORD_PTR dwItemData /*=0*/)
{
    PHONEEVENT lpfn = GetSP()->GetPhoneCreateProc();
    if (lpfn == NULL || GetProcAddress(GetSP()->GetResourceInstance(), "TSPI_providerCreatePhoneDevice") == NULL)
    {
        // If we are here, we probably don't have "TSPI_providerEnumDevices" exported, or the
        // version of TAPI isn't sufficient to handle it.
        _TSP_DTRACE(_T("WARNING: Attempted to dynamically create phone without TAPI support\n"));
        _TSP_ASSERT(false);
        return NULL;
    }

    CTSPIPhoneConnection* pConn = GetSP()->CreatePhoneObject();
    pConn->Init(this, reinterpret_cast<DWORD>(pConn), GetPhoneCount(), dwItemData);
    AddPhoneConnectionInfo (pConn);

    (*lpfn)(NULL, PHONE_CREATE, 
			reinterpret_cast<DWORD_PTR>(GetProviderHandle()), 
			reinterpret_cast<DWORD_PTR>(pConn), 0L);
    _TSP_DTRACEX(TRC_STATS, _T("Dynamically created phone object <0x%lx>\n"), pConn);

    return pConn;

}// CTSPIDevice::CreatePhone

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemovePhone
//
// Remove a phone device from our provider.
//
void CTSPIDevice::RemovePhone (CTSPIPhoneConnection* pPhone)
{
    PHONEEVENT lpfn = GetSP()->GetPhoneCreateProc();
    if (lpfn == NULL)
    {
        // If we are here, we probably don't have "TSPI_providerEnumDevices" exported, or the
        // version of TAPI isn't sufficient to handle it.
        _TSP_DTRACE(_T("WARNING: Attempted to dynamically remove phone without TAPI support\n"));
        _TSP_ASSERT(false);
		return;
    }

	// The derived service provider will be responsible for
	// renumbering the phone connection array when providerShutdown
	// is called.
	(*lpfn)(NULL, PHONE_REMOVE, pPhone->GetDeviceID(), 0L, 0L);

	// Mark the phone as REMOVED.  Once it is closed it will be marked
	// as DELETED and all further references to it will be returned
	// in the TSPI_xxx layer.
	pPhone->m_dwFlags |= CTSPIConnection::_IsRemoved;

}// CTSPIDevice::RemovePhone

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RunTimer
//
// This is the loop that the device interval timer thread uses to periodically
// show activity on the line/phone devices.  It is only activated if the 
// derived service provider uses the "SetIntervalTimer" API of this device
// object and sets the timeout to a non-zero value.
//
// It then wakes up every "m_dwIntervalTimeout" msecs and calls the OnTimer()
// method of the device.
//
void CTSPIDevice::RunTimer()
{
    for (;;)
    {
        // Wait for either our object to signal (meaning our provider
        // is being shutdown), or for our timeout value to elapse.
        LONG lResult = WaitForSingleObject (m_hevtDeviceShutdown, m_dwIntervalTimeout);
        if (lResult == WAIT_OBJECT_0 || lResult == WAIT_ABANDONED || m_dwIntervalTimeout == 0)
            break;

		try
		{
			// Let the virtual function handle the timer.
			OnTimer();
		}
		catch (...)
		{
		}
    }


}// CTSPIDevice::RunTimer

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OnTimer
//
// This invokes the OnTimer() method of all the lines/phones owned by
// this device.
//
void CTSPIDevice::OnTimer()
{
	// Walk through the line and phone array and call their "OnTimer" function
	// to let them perform some piece of work in the derived provider.
	int i, iLineCount = GetLineCount();
	for (i = 0; i < iLineCount; i++) {
		CTSPILineConnection* pLine = GetLineConnectionInfo(i);
		if (pLine && !pLine->HasBeenDeleted()) pLine->OnTimer();
	}

	int iPhoneCount = GetPhoneCount();
	for (i = 0; i < iPhoneCount; i++) {
		CTSPIPhoneConnection* pPhone = GetPhoneConnectionInfo(i);
		if (pPhone && !pPhone->HasBeenDeleted()) pPhone->OnTimer();
	}

}// CTSPIDevice::OnTimer

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::ReceiveData
//
// This is a data direction function that allows the derived service 
// provider to simply associate the PERMANENT LINE or PERMANENT PHONE
// identifier with a request and when the data is recieved, pass it
// here to have it automatically directed to the line or phone associated
// with that identifier.
//
// If no connection id is matched then this function spins through all
// the lines and phones on the device and lets each see the event.  If
// any of them return "true" to indicate that the event was processed,
// this loop terminates.
//
void CTSPIDevice::ReceiveData (DWORD dwPermanentID, LPCVOID lpBuff)
{                            
	// If we have a connection identifier passed from the caller..
    if (dwPermanentID != 0)
    {
		// Lookup the line/phone and let it process the data.
		CTSPILineConnection* pLine = FindLineConnectionByPermanentID(dwPermanentID);
		if (pLine != NULL)
			pLine->ReceiveData(lpBuff);
		CTSPIPhoneConnection* pPhone = FindPhoneConnectionByPermanentID(dwPermanentID);
		if (pPhone != NULL)
			pPhone->ReceiveData(lpBuff);
    }

	// No line/phone device was specified by the caller.
	// Spin through all line and phone devices and hand them the data.
	// If the connection returns true, then stop.
	else
	{
		CEnterCode sLock(this);
		// Try the lines first
		if (std::find_if(m_arrayLines.begin(), m_arrayLines.end(), 
			std::bind2nd(std::mem_fun1(&CTSPILineConnection::ReceiveData), lpBuff)) != m_arrayLines.end())
			return;

		// Finally, hit the phone objects
		std::find_if(m_arrayPhones.begin(), m_arrayPhones.end(), 
			std::bind2nd(std::mem_fun1(&CTSPIPhoneConnection::ReceiveData), lpBuff));
	}
        
}// CTSPIDevice::ReceiveData

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GenericDialogData
//
// Dialog Data for UI constructions in TAPI 2.0
//
LONG CTSPIDevice::GenericDialogData (LPVOID /*lpParam*/, DWORD /*dwSize*/)
{
	return false;

}// CTSPIDevice::GenericDialogData

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OpenDevice
//
// This method is called when lineOpen or phoneOpen is called.
//
bool CTSPIDevice::OpenDevice (CTSPIConnection* /*pConn*/)
{
	// This always returns that the device is open since in a 3rd party
	// view, we would stay connected to the hardware at all times to keep a
	// current view of the lines we manage. In a first-party provider, this 
	// function would be used to connect to the device.
	return true;

}// CTSPIDevice::Open

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::CloseDevice
//
// This method is called when the last line/phone connection object is
// closed.  It will not be called if multiple lines/phones are open on this
// device.
//
bool CTSPIDevice::CloseDevice (CTSPIConnection* /*pConn*/)
{
	return false;

}// CTSPIDevice::CloseDevice

////////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OnCancelRequest
//
// Cancel a request which has already been started (its state is not
// STATE_INITIAL).  The request is about to be deleted and send an
// error notification back to TAPI.  Generally this happens when the
// call or line is closed.  The service provider code should determine
// what to do based on the state of this request and its command.
//
void CTSPIDevice::OnCancelRequest (CTSPIRequest* /*pRequest*/)
{
    /* Do nothing */

}// CTSPIDevice::CancelRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::OnNewRequest
//
// A new request is being added to our connection object.  The derived
// provider may override this function or CTSPIConnection::OnNewRequest or
// the CServiceProvider::OnNewRequest function to catch these and perform 
// some function BEFORE the request has officially been added.
//
// If false is returned, the request will be canceled.
//
bool CTSPIDevice::OnNewRequest (CTSPIConnection* /*pConn*/, CTSPIRequest* /*pReq*/, int* /*piPos*/)
{
    // Return true to continue adding the request or false to cancel the request.
    return true;

}// CTSPIDevice::OnNewRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::FindCallHub
//
// This function may be used by the service provider to locate a call hub
// with calls using the CALLID field of the LINECALLINFO structure.
//
// If no call hub has been created for the given call id, this function
// returns NULL.
//
const CTSPICallHub* CTSPIDevice::FindCallHub(DWORD dwCallID) const
{
	CEnterCode sLock(this);
	if (!m_mapCalls.empty())
	{
		TCallMap::const_iterator theIterator = m_mapCalls.find(dwCallID);
		if (theIterator != m_mapCalls.end())
			return (*theIterator).second;
	}
	return NULL;

}// CTSPIDevice::FindCallHub

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetCallHubFromMap
//
// This function is called to create or locate an existing call hub
// for a given callid.
//
CTSPICallHub* CTSPIDevice::GetCallHubFromMap(DWORD dwCallID)
{
	// Don't support zero call id - always NULL.
	if (dwCallID == 0)
		return NULL;

	// Otherwise search for the call hub
	CEnterCode sLock(this);
	if (!m_mapCalls.empty())
	{
		TCallMap::const_iterator theIterator = m_mapCalls.find(dwCallID);
		if (theIterator != m_mapCalls.end())
			return (*theIterator).second;
	}

	// Doesn't yet exist, create it.
	CTSPICallHub* pHub = new CTSPICallHub(dwCallID);
	m_mapCalls.insert(std::make_pair(dwCallID, pHub));

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_CALLMAP, _T("Created callhub %s"), pHub->Dump().c_str());
#endif

	return pHub;

}// CTSPIDevice::GetCallHubFromMap

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemoveCallHub
//
// This is called when the final call is removed from a call hub
// object - it deletes the call hub and removes it from the map.
//
void CTSPIDevice::RemoveCallHub(DWORD dwCallID, CTSPICallHub* pHub)
{
	CEnterCode sLock(this);

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_CALLMAP, _T("Deleting callhub %s"), pHub->Dump().c_str());
#endif

	// Find the hub in question and delete it.
	if (!m_mapCalls.empty())
	{
		TCallMap::iterator theIterator = m_mapCalls.find(dwCallID);
		_TSP_ASSERTE(theIterator != m_mapCalls.end());
		if (theIterator != m_mapCalls.end())
			m_mapCalls.erase(theIterator);
	}

	_TSP_ASSERTE(FindCallHub(dwCallID) == NULL);

	// We are done with this object -- when the reference count hits
	// zero it will be deleted from memory.
	pHub->DecRef();

}// CTSPIDevice::RemoveCallHub

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::EnableAgentProxy
//
// Turn on the agent proxy support for a specific line.
//
void CTSPIDevice::EnableAgentProxy(CTSPILineConnection* pLine, DWORD dwAgentFeatures)
{
	// Create an OPEN packet
	DWORD dwSize = sizeof(ACDREQUEST) + sizeof(ACDOPEN);
	ACDREQUEST* pRequest = reinterpret_cast<ACDREQUEST*>(AllocMem(dwSize));
	pRequest->dwSize = dwSize;
	pRequest->dwType = ACDTYPE_OPEN;
	pRequest->dwProviderID = GetProviderID();
	pRequest->dwLineDeviceID = pLine->GetDeviceID();
	ACDOPEN* pInit = reinterpret_cast<ACDOPEN*>(pRequest->vPart);
	pInit->dwAgentFeatures = dwAgentFeatures;

	// Walk through our address objects on the line and see what agent capabilities
	// are supported.
	for (int iAddress = 0; iAddress < pLine->GetAddressCount(); iAddress++)
	{
		CTSPIAddressInfo* pAddress = pLine->GetAddress(iAddress);
		if (pAddress != NULL)
		{
			// Add in any calculated agent features.
			DWORD dwFeatures = pAddress->GetAgentCaps()->dwFeatures;

			// If we were not passed any features, try to figure them out.
			if (dwAgentFeatures == 0)
			{
				if (dwFeatures & LINEAGENTFEATURE_SETAGENTGROUP)
					pInit->dwAgentFeatures |= AGENTFEATURE_SETAGENTGROUP;
				if (dwFeatures & LINEAGENTFEATURE_SETAGENTSTATE)
					pInit->dwAgentFeatures |= AGENTFEATURE_SETAGENTSTATE;
				if (dwFeatures & LINEAGENTFEATURE_SETAGENTACTIVITY)
					pInit->dwAgentFeatures |= AGENTFEATURE_SETAGENTACTIVITY;
				if (dwFeatures & LINEAGENTFEATURE_AGENTSPECIFIC)
					pInit->dwAgentFeatures |= AGENTFEATURE_AGENTSPECIFIC;
				if (dwFeatures & LINEAGENTFEATURE_GETAGENTACTIVITYLIST)
					pInit->dwAgentFeatures |= AGENTFEATURE_GETAGENTACTIVITYLIST;
				if (dwFeatures & LINEAGENTFEATURE_GETAGENTGROUP)
					pInit->dwAgentFeatures |= AGENTFEATURE_GETAGENTGROUPLIST;
			}
				
			// Setup the status messages we can send based on the features
			// that the provider is expecting to support on this address.
			pAddress->GetAgentCaps()->dwAgentStatusMessages |= 
				(LINEAGENTSTATUS_CAPSCHANGE | 
				LINEAGENTSTATUS_VALIDSTATES | 
				LINEAGENTSTATUS_VALIDNEXTSTATES);
			if (dwFeatures & LINEAGENTFEATURE_SETAGENTGROUP)
				pAddress->GetAgentCaps()->dwAgentStatusMessages |= LINEAGENTSTATUS_GROUP;
			if (dwFeatures & LINEAGENTFEATURE_SETAGENTSTATE)
				pAddress->GetAgentCaps()->dwAgentStatusMessages |= (LINEAGENTSTATUS_STATE | LINEAGENTSTATUS_NEXTSTATE);
			if (dwFeatures & LINEAGENTFEATURE_SETAGENTACTIVITY)
				pAddress->GetAgentCaps()->dwAgentStatusMessages |= LINEAGENTSTATUS_ACTIVITY;
			if (dwFeatures & LINEAGENTFEATURE_GETAGENTACTIVITYLIST)
				pAddress->GetAgentCaps()->dwAgentStatusMessages |= LINEAGENTSTATUS_ACTIVITYLIST;
			if (dwFeatures & LINEAGENTFEATURE_GETAGENTGROUP)
				pAddress->GetAgentCaps()->dwAgentStatusMessages |= LINEAGENTSTATUS_GROUPLIST;

			// Determine our current agent feature set if the derived provider didn't set it already.
			if (pAddress->GetAgentStatus()->dwAgentFeatures == 0)
			{
				DWORD dwState = pAddress->GetAgentStatus()->dwState;
				if (dwState == LINEAGENTSTATE_LOGGEDOFF)
				{
					// If we support lineSetAgentGroup, then force that to be the method
					// used for login - this is the standard usage by most TAPI applications
					// so it should be the way the service provider functions.
					if (pAddress->GetAgentCaps()->dwFeatures & LINEAGENTFEATURE_SETAGENTGROUP)
						dwFeatures &= ~(LINEAGENTFEATURE_SETAGENTACTIVITY |
										LINEAGENTFEATURE_SETAGENTSTATE);

					// Otherwise if we don't support lineSetAgentGroup, the allow the
					// lineSetAgentState function to be used for logon or setting the
					// initial state.
					else
						dwFeatures &= ~LINEAGENTFEATURE_SETAGENTACTIVITY;
				}

				// Otherwise, if we are logged on but don't have any groups loaded into our
				// group list then don't allow lineSetAgentGroup. We want to allow it 
				// normally in case the provider wants to use lineSetAgentGroup to perform
				// a logon w/o real groups.
				else if (dwState != LINEAGENTSTATE_UNKNOWN)
				{
					if (GetAgentGroupCount() == 0)
						dwFeatures &= ~LINEAGENTFEATURE_SETAGENTGROUP;
				}

				// Set the current available features
				pAddress->SetAgentFeatures(dwFeatures);
			}
		}
	}

	// If the line doesn't support ANY agent activity, then ignore this request.
	// Otherwise, send the OPEN request to the proxy.
	if (pInit->dwAgentFeatures != 0)
	{
		// Make sure we include the default ones.
		pInit->dwAgentFeatures |= (AGENTFEATURE_GETAGENTCAPS | AGENTFEATURE_GETAGENTSTATUS);

		_TSP_DTRACEX(TRC_AGENTPROXY, _T("Enabling JTProxy for line %ld\n"), pLine->GetDeviceID());

		// Ensure that we have the shared memory allocated and an agent thread running.
		if (m_psmProxy == NULL)
		{
			m_psmProxy = new CSharedMemMgr;
			if (m_psmProxy->Init(ACDPROXY_OUTPUT, MAX_ACD_MAX_SIZE, ACDPROXY_INPUT, MAX_ACD_MAX_SIZE))
			{
				UINT uiThread;
				m_htAgentProxy = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, tsplib_AgentProxyThread, static_cast<void*>(this), 0, &uiThread));
				_TSP_ASSERTE(m_htAgentProxy != NULL);
			}
			else
			{
				_TSP_DTRACEX(TRC_AGENTPROXY, _T("Unable to connect to the JTProxy application. Agent services have been disabled.\n"));
				delete m_psmProxy;
				m_psmProxy = NULL;
				FreeMem(pRequest);
				return;
			}
		}
		m_psmProxy->Write(pRequest);
	}

	// Free the block
	FreeMem(pRequest);

}// CTSPIDevice::EnableAgentProxy

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AddAgentActivity
//
// This function adds a new activity code to the agent capabilities.
//
int CTSPIDevice::AddAgentActivity (DWORD dwID, LPCTSTR pszName)
{
	// Verify that this activity isn't already in our list.
	if (DoesAgentActivityExist(dwID) != NULL)
		return -1;

	// Create a new agent activity structure.
	TAgentActivity* pActivity = new TAgentActivity;
	pActivity->dwID = dwID;
	pActivity->strName = pszName;

	CEnterCode sLock(this);
	m_arrAgentActivities.push_back(pActivity);
	return m_arrAgentActivities.size()-1;

}// CTSPIDevice::AddAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemoveAgentActivity
//
// This function removes an agent activity item from our array.
//
void CTSPIDevice::RemoveAgentActivity(DWORD dwID)
{
	CEnterCode sLock(this);
	bool fFound = false;
	for (TAgentActivityArray::iterator iPos = m_arrAgentActivities.begin();
		 iPos != m_arrAgentActivities.end(); iPos++)
	{
		TAgentActivity* pActivity = (*iPos);
		if (pActivity->dwID == dwID)
		{
			m_arrAgentActivities.erase(iPos);
			delete pActivity;
			fFound = true;
			break;
		}
	}

	sLock.Unlock();

	// Notify TAPI that the activities have changed for each line on this
	// device that supports agent capabilities
	if (fFound == true)
	{
		for (unsigned int i = 0; i < GetLineCount(); i++)
		{
			CTSPILineConnection* pLine = GetLineConnectionInfo(i);
			if (pLine->SupportsAgents())
			{
				for (int j = 0; j < pLine->GetAddressCount(); j++)
					pLine->GetAddress(j)->OnAgentCapabiltiesChanged();
			}
		}
	}

}// CTSPIDevice::RemoveAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::DoesAgentActivityExist
//
// This function returns true/false whether the given activity
// is valid within our list.
//
bool CTSPIDevice::DoesAgentActivityExist(DWORD dwActivity) const
{
	CEnterCode sLock(this);
	for (TAgentActivityArray::const_iterator iPos = m_arrAgentActivities.begin();
		 iPos != m_arrAgentActivities.end(); iPos++)
	{
		if ((*iPos)->dwID == dwActivity)
			return true;
	}
	return false;

}// CTSPIDevice::DoesAgentActivityExist

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentActivityById
//
// This function returns the activity structure associated with the ID.
//
TString CTSPIDevice::GetAgentActivityById(DWORD dwID) const
{
	CEnterCode sLock(this);
	for (TAgentActivityArray::const_iterator iPos = m_arrAgentActivities.begin();
		 iPos != m_arrAgentActivities.end(); iPos++)
	{
		if ((*iPos)->dwID == dwID)
			return (*iPos)->strName;
	}
	return TString(_T(""));

}// CTSPIDevice::GetAgentActivityById

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AddAgentGroup
//
// Adds a new agent group to our list.
//
int CTSPIDevice::AddAgentGroup(LPCTSTR pszName, DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	// Verify that this group isn't already in our list.
	if (DoesAgentGroupExist(dwGroupID1, dwGroupID2, dwGroupID3, dwGroupID4))
		return -1;

	// Create a new agent activity structure.
	TAgentGroup* pGroup = new TAgentGroup;
	pGroup->strName = pszName;
	pGroup->GroupID.dwGroupID1 = dwGroupID1;
	pGroup->GroupID.dwGroupID2 = dwGroupID2;
	pGroup->GroupID.dwGroupID3 = dwGroupID3;
	pGroup->GroupID.dwGroupID4 = dwGroupID4;
	
	// Now add it to our array
	CEnterCode sLock(this);
	m_arrAgentGroups.push_back(pGroup);
	return m_arrAgentGroups.size()-1;

}// CTSPIDevice::AddAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemoveAgentGroup
//
// Removes the specified agent group.
//
void CTSPIDevice::RemoveAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	// Locate the group in our array
	CEnterCode sLock(this);
	bool fFound = false;
	for (TAgentGroupArray::iterator iPos = m_arrAgentGroups.begin();
		 iPos != m_arrAgentGroups.end(); iPos++)
	{
		TAgentGroup* pGroup = (*iPos);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
		{
			m_arrAgentGroups.erase(iPos);
			delete pGroup;
			fFound = true;
			break;
		}
	}

	sLock.Unlock();

	// Notify TAPI that the activities have changed for each line on this
	// device that supports agent capabilities
	if (fFound == true)
	{
		for (unsigned int i = 0; i < GetLineCount(); i++)
		{
			CTSPILineConnection* pLine = GetLineConnectionInfo(i);
			if (pLine->SupportsAgents())
			{
				for (int j = 0; j < pLine->GetAddressCount(); j++)
					pLine->GetAddress(j)->OnAgentCapabiltiesChanged();
			}
		}
	}

}// CTSPIDevice::RemoveAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::DoesAgentGroupExist
//
// This function returns true/false whether the given group
// is valid within our list.
//
bool CTSPIDevice::DoesAgentGroupExist(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4) const
{
	CEnterCode sLock(this);
	for (TAgentGroupArray::const_iterator iPos = m_arrAgentGroups.begin();
		 iPos != m_arrAgentGroups.end(); iPos++)
	{
		TAgentGroup* pGroup = (*iPos);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
			return true;
	}
	return false;

}// CTSPIDevice::DoesAgentGroupExist

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::GetAgentGroupById
//
// Returns the agent group by the identifier
//
TString CTSPIDevice::GetAgentGroupById(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4) const
{
	// Locate the group in our array
	CEnterCode sLock(this);
	for (TAgentGroupArray::const_iterator iPos = m_arrAgentGroups.begin();
		 iPos != m_arrAgentGroups.end(); iPos++)
	{
		TAgentGroup* pGroup = (*iPos);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
			return pGroup->strName;
	}
	return TString(_T(""));

}// CTSPIDevice::GetAgentGroupById

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AddAgentSpecificExtension
//
// Adds a new agent specific handler extension.
//
int CTSPIDevice::AddAgentSpecificExtension(DWORD dwID1, DWORD dwID2, DWORD dwID3, DWORD dwID4)
{
	// Verify that this group isn't already in our list.
	if (DoesAgentSpecificExtensionExist(dwID1, dwID2, dwID3, dwID4) != NULL)
		return -1;

	// Create a new agent activity structure.
	TAgentSpecificEntry* pEntry = new TAgentSpecificEntry;
	pEntry->dwID1 = dwID1;
	pEntry->dwID2 = dwID1;
	pEntry->dwID3 = dwID1;
	pEntry->dwID4 = dwID1;
	
	// Now add it to our array
	CEnterCode sLock(this);
	m_arrAgentExtensions.push_back(pEntry);
	return m_arrAgentExtensions.size()-1;

}// CTSPIDevice::AddAgentSpecificExtension

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::RemoveAgentSpecificExtension
//
// Removes the specified agent extension
//
void CTSPIDevice::RemoveAgentSpecificExtension(DWORD dwID1, DWORD dwID2, DWORD dwID3, DWORD dwID4)
{
	// Locate the group in our array
	CEnterCode sLock(this);
	bool fFound = false;
	for (TAgentExtensionArray::iterator iPos = m_arrAgentExtensions.begin();
		 iPos != m_arrAgentExtensions.end(); iPos++)
	{
		TAgentSpecificEntry* pEntry = (*iPos);
		if (pEntry->dwID1 == dwID1 &&
			pEntry->dwID2 == dwID2 &&
			pEntry->dwID3 == dwID3 &&
			pEntry->dwID4 == dwID4)
		{
			m_arrAgentExtensions.erase(iPos);
			delete pEntry;
			fFound = true;
			break;
		}
	}

	sLock.Unlock();

	// Notify TAPI that the activities have changed for each line on this
	// device that supports agent capabilities
	if (fFound == true)
	{
		for (unsigned int i = 0; i < GetLineCount(); i++)
		{
			CTSPILineConnection* pLine = GetLineConnectionInfo(i);
			if (pLine->SupportsAgents())
			{
				for (int j = 0; j < pLine->GetAddressCount(); j++)
					pLine->GetAddress(j)->OnAgentCapabiltiesChanged();
			}
		}
	}

}// CTSPIDevice::RemoveSpecificExtension

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::DoesAgentSpecificExtension
//
// Locate an extension by the id.
//
bool CTSPIDevice::DoesAgentSpecificExtensionExist(DWORD dwID1, DWORD dwID2, DWORD dwID3, DWORD dwID4) const
{
	// Locate the group in our array
	CEnterCode sLock(this);
	for (TAgentExtensionArray::const_iterator iPos = m_arrAgentExtensions.begin();
		 iPos != m_arrAgentExtensions.end(); iPos++)
	{
		TAgentSpecificEntry* pEntry = (*iPos);
		if (pEntry->dwID1 == dwID1 && pEntry->dwID2 == dwID2 &&
			pEntry->dwID3 == dwID3 && pEntry->dwID4 == dwID4)
			return true;
	}
	return false;

}// CTSPIDevice::DoesAgentSpecificExtensionExist

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::AgentProxy
//
// This function is run by a secondary thread which pulls requests
// from our agent proxy application and sends them through the service
// provider.
//
void CTSPIDevice::AgentProxy()
{
    for (;;)
    {
        // Wait for either our object to signal (meaning our provider
        // is being shutdown), or for the proxy buffer to have information.
		ACDREQUEST* pRequest;
		if (m_psmProxy == NULL || 
			!m_psmProxy->ReadSpecific(GetProviderID(), &pRequest, m_hevtDeviceShutdown))
			break;

		// Validate that we got something.
		if (pRequest == NULL)
			continue;

		// See if the proxy application just OPENED itself.
		// If so, we need to tell it about each of the lines which 
		// are expecting agent handling.
		if (pRequest->dwType == ACDTYPE_OPEN)
		{
			_TSP_DTRACEX(TRC_AGENTPROXY, _T("Received PROXY_open\n"));
			for (unsigned int iLine = 0; iLine < GetLineCount(); iLine++)
			{
				CTSPILineConnection* pLine = GetLineConnectionInfo(iLine);
				if (pLine != NULL && !pLine->HasBeenDeleted() && pLine->SupportsAgents())
					pLine->EnableAgentProxy();
			}
		}

		// Or if this is a request to run, run it.
		else if (pRequest->dwType == ACDTYPE_PROXYREQUEST)
		{
			LPLINEPROXYREQUEST pProxy = reinterpret_cast<LPLINEPROXYREQUEST>(pRequest->vPart);
			switch (pProxy->dwRequestType)
			{
				// lineSetAgentGroup
				case LINEPROXYREQUEST_SETAGENTGROUP:
					TSPI_lineSetAgentGroup(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID, 
						pProxy->SetAgentGroup.dwAddressID,
						&pProxy->SetAgentGroup.GroupList);
					break;

				// lineSetAgentState
				case LINEPROXYREQUEST_SETAGENTSTATE:
					TSPI_lineSetAgentState(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID, 
						pProxy->SetAgentState.dwAddressID,
						pProxy->SetAgentState.dwAgentState,
						pProxy->SetAgentState.dwNextAgentState);
					break;

				// lineSetAgentActivity
				case LINEPROXYREQUEST_SETAGENTACTIVITY:
					TSPI_lineSetAgentActivity(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID, 
						pProxy->SetAgentActivity.dwAddressID,
						pProxy->SetAgentActivity.dwActivityID);
					break;

				// lineGetAgentStatus
				case LINEPROXYREQUEST_GETAGENTSTATUS:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentStatus(
							pRequest->dwLineDeviceID, 
							pProxy->GetAgentStatus.dwAddressID,
							&pProxy->GetAgentStatus.AgentStatus));
					break;

				// lineGetAgentCaps
				case LINEPROXYREQUEST_GETAGENTCAPS:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentCaps(
							pRequest->dwLineDeviceID, 
							pProxy->GetAgentCaps.dwAddressID,
							&pProxy->GetAgentCaps.AgentCaps));
					break;

				// lineGetAgentActivityList
				case LINEPROXYREQUEST_GETAGENTACTIVITYLIST:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentActivityList(
							pRequest->dwLineDeviceID, 
							pProxy->GetAgentActivityList.dwAddressID,
							&pProxy->GetAgentActivityList.ActivityList));
					break;

				// lineGetAgentGroupList
				case LINEPROXYREQUEST_GETAGENTGROUPLIST:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentGroupList(
							pRequest->dwLineDeviceID, 
							pProxy->GetAgentGroupList.dwAddressID,
							&pProxy->GetAgentGroupList.GroupList));
					break;

				// lineAgentSpecific
				case LINEPROXYREQUEST_AGENTSPECIFIC:
					TSPI_lineAgentSpecific(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID, 
						pProxy->AgentSpecific.dwAddressID,
						pProxy->AgentSpecific.dwAgentExtensionIDIndex,
						&pProxy->AgentSpecific.Params[0],
						pProxy->AgentSpecific.dwSize);
					break;

				case LINEPROXYREQUEST_CREATEAGENT:
					TSPI_lineCreateAgent(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						&pProxy->CreateAgent.hAgent,
						pRequest->chMachine,
						pRequest->chUserName,
						GetWStringFromVARBlock(pProxy, pProxy->CreateAgent.dwAgentIDSize, pProxy->CreateAgent.dwAgentIDOffset),
						GetWStringFromVARBlock(pProxy, pProxy->CreateAgent.dwAgentPINSize, pProxy->CreateAgent.dwAgentPINOffset));
					break;
				case LINEPROXYREQUEST_SETAGENTMEASUREMENTPERIOD:
					TSPI_lineSetAgentMeasurementPeriod(
						reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						pProxy->SetAgentMeasurementPeriod.hAgent,
						pProxy->SetAgentMeasurementPeriod.dwMeasurementPeriod);
					break;
				case LINEPROXYREQUEST_GETAGENTINFO:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentInfo(
							pRequest->dwLineDeviceID,
							pProxy->GetAgentInfo.hAgent, 
							&pProxy->GetAgentInfo.AgentInfo));
					break;
				case LINEPROXYREQUEST_CREATEAGENTSESSION:
					TSPI_lineCreateAgentSession(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						&pProxy->CreateAgentSession.hAgentSession,
						pProxy->CreateAgentSession.hAgent,
						GetWStringFromVARBlock(pProxy, pProxy->CreateAgentSession.dwAgentPINSize, pProxy->CreateAgentSession.dwAgentPINOffset),
						&pProxy->CreateAgentSession.GroupID,
						pProxy->CreateAgentSession.dwWorkingAddressID);
					break;
				case LINEPROXYREQUEST_GETAGENTSESSIONLIST:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentSessionList(pRequest->dwLineDeviceID,
							pProxy->GetAgentSessionList.hAgent,
							&pProxy->GetAgentSessionList.SessionList));
					break;
				case LINEPROXYREQUEST_SETAGENTSESSIONSTATE:
					TSPI_lineSetAgentSessionState(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						pProxy->SetAgentSessionState.hAgentSession,
						pProxy->SetAgentSessionState.dwAgentSessionState,
						pProxy->SetAgentSessionState.dwNextAgentSessionState);
					break;
				case LINEPROXYREQUEST_GETAGENTSESSIONINFO:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetAgentSessionInfo(
							pRequest->dwLineDeviceID,
							pProxy->GetAgentSessionInfo.hAgentSession,
							&pProxy->GetAgentSessionInfo.SessionInfo));
					break;
				case LINEPROXYREQUEST_GETQUEUELIST:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetQueueList(pRequest->dwLineDeviceID,
							&pProxy->GetQueueList.GroupID,
							&pProxy->GetQueueList.QueueList));
					break;
				case LINEPROXYREQUEST_SETQUEUEMEASUREMENTPERIOD:
					TSPI_lineSetQueueMeasurementPeriod(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						pProxy->SetQueueMeasurementPeriod.dwQueueID,
						pProxy->SetQueueMeasurementPeriod.dwMeasurementPeriod);
					break;
				case LINEPROXYREQUEST_GETQUEUEINFO:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetQueueInfo(
							pRequest->dwLineDeviceID,
							pProxy->GetQueueInfo.dwQueueID,
							&pProxy->GetQueueInfo.QueueInfo));
					break;
				case LINEPROXYREQUEST_GETGROUPLIST:
					ProxyResponse(reinterpret_cast<DRV_REQUESTID>(pRequest),
						TSPI_lineGetGroupList(pRequest->dwLineDeviceID,
							&pProxy->GetGroupList.GroupList));
					break;
				case LINEPROXYREQUEST_SETAGENTSTATEEX:
					TSPI_lineSetAgentStateEx(reinterpret_cast<DRV_REQUESTID>(pRequest),
						pRequest->dwLineDeviceID,
						pProxy->SetAgentStateEx.hAgent,
						pProxy->SetAgentStateEx.dwAgentState,
						pProxy->SetAgentStateEx.dwNextAgentState);
					break;
				default:
					_TSP_DTRACEX(TRC_WARNINGS, _T("PROBLEM: Unknown agent proxy request type: 0x%lx from %s\\%s"), pRequest->dwType, pRequest->chMachine, pRequest->chUserName);
					break;
			}
		}
    }

	// Delete the shared memory object for the proxy
	delete m_psmProxy;
	m_psmProxy = NULL;

}// CTSPIDevice::AgentProxy

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::ProxyResponse
//
// Returns a response to the proxy application concerning an event.
//
void CTSPIDevice::ProxyResponse(DRV_REQUESTID dwRequestID, LONG lResult)
{
	// Convert the DRV_REQUESTID back into our pointer and validate
	// that it may be passed back to the proxy application.
	ACDREQUEST* pRequest = reinterpret_cast<ACDREQUEST*>(dwRequestID);
	if (pRequest == NULL || IsBadReadPtr(pRequest, sizeof(ACDREQUEST)) || pRequest->dwType != ACDTYPE_PROXYREQUEST)
	{
		_TSP_DTRACE(_T("ProxyResponse: bad ACDREQUEST 0x%lx, type=%ld, (possibly completed request twice?)\n"), pRequest, pRequest->dwType);
		return;
	}

	// If there is no proxy installed/running or if the requestid and the 
	// result are the same, then it is ok ignore it.
	if (m_psmProxy != NULL && (dwRequestID > 0 && dwRequestID != static_cast<DRV_REQUESTID>(lResult)))
	{
		// Fill in our result.
		pRequest->lResult = lResult;
		pRequest->dwType = ACDTYPE_PROXYRESPONSE;
		pRequest->dwProviderID = GetProviderID();

		// Throw it back to the agent app and delete the memory block.
		m_psmProxy->Write(pRequest);
	}

	// Delete the request received from the proxy
	delete [] (BYTE*) pRequest;

}// CTSPIDevice::ProxyResponse

///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::ProxyNotify
//
// Sends an asynch notification to the proxy application
//
void CTSPIDevice::ProxyNotify(CTSPILineConnection* pLine, DWORD dwMsg, DWORD dwParam1, 
							  DWORD dwParam2, DWORD dwParam3)
{
	// If there is no proxy installed/running then exit.
	if (m_psmProxy == NULL)
		return;

	// Otherwise send it.
	DWORD dwSize = sizeof(ACDREQUEST) + sizeof(ACDNOTIFY);
	ACDREQUEST* pRequest = reinterpret_cast<ACDREQUEST*>(AllocMem(dwSize));
	pRequest->dwSize = dwSize;
	pRequest->dwType = ACDTYPE_NOTIFY;
	pRequest->dwProviderID = GetProviderID();
	pRequest->dwLineDeviceID = pLine->GetDeviceID();
	
	ACDNOTIFY* pNotify = reinterpret_cast<ACDNOTIFY*>(pRequest->vPart);
	pNotify->dwMsg = dwMsg;
	pNotify->dwParam1 = dwParam1;
	pNotify->dwParam2 = dwParam2;
	pNotify->dwParam3 = dwParam3;

	// Send the notify request to the proxy.
	m_psmProxy->Write(pRequest);

	// Free the block
	FreeMem(pRequest);

}// CTSPIDevice::ProxyNotify

///////////////////////////////////////////////////////////////////////////
// tsplib_IntervalTimerThread
//
// Runs the interval timer for this device
//
unsigned __stdcall tsplib_IntervalTimerThread(void* pParam)
{
	_TSP_DTRACEX(TRC_THREADS, _T("IntervalTimerThread(0x%lx) starting\n"), GetCurrentThreadId());
    reinterpret_cast<CTSPIDevice*>(pParam)->RunTimer();
	_TSP_DTRACEX(TRC_THREADS, _T("IntervalTimerThread(0x%lx) ending\n"), GetCurrentThreadId());
	_endthreadex(0);
	return 0;

}// tsplib_IntervalTimerThread

///////////////////////////////////////////////////////////////////////////
// tsplib_AgentProxyThread
//
// Runs the agent proxy delivery thread for this device
//
unsigned __stdcall tsplib_AgentProxyThread(void* pParam)
{
	_TSP_DTRACEX(TRC_THREADS, _T("AgentProxyThread(0x%lx) starting\n"), GetCurrentThreadId());
    reinterpret_cast<CTSPIDevice*>(pParam)->AgentProxy();
	_TSP_DTRACEX(TRC_THREADS, _T("AgentProxyThread(0x%lx) ending\n"), GetCurrentThreadId());
	_endthreadex(0);
	return 0;

}// tsplib_AgentProxyThread

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPIDevice::Dump
//
// Debug "dump" of the object and it's contents.
//
TString CTSPIDevice::Dump() const 
{
	TStringStream outstm;

	CEnterCode Lock(this);
	outstm << _T("0x") << hex << (DWORD)this;
    outstm << _T(",ProviderID=0x") << hex << m_dwProviderId;
    outstm << _T(" [") << m_strDevInfo << _T("] ");
	outstm << _T("AgentProxy is");
	if (m_psmProxy == NULL)
		outstm << _T(" not");
	outstm << _T(" running");
    return(outstm.str());

}// CTSPIDevice::Dump
#endif
