/******************************************************************************/
//                                                                        
// POOLMGR.H - TSP++ Pool manager template
//
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _SPLIB_POOLMGR_INC_
#define _SPLIB_POOLMGR_INC_

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include <process.h>

/*-------------------------------------------------------------------------------*/
// LIMITS
/*-------------------------------------------------------------------------------*/
#ifndef PM_THREADS_PER_PROCESSOR
#define PM_THREADS_PER_PROCESSOR	20		// Max # of threads per processor
#endif

/**************************************************************************
** CIntThreadMgr
**
** This object manages a set of threads which are used to process
** requests.
**
***************************************************************************/
class CIntThreadMgr
{
// Class data
protected:
	enum {
		CreationThreshold  = 2,			// Number of queued requests before we spawn
		MinimumThreadCount = 4			// Minimum thread count for manager
	};

	HANDLE m_evtRequest;				// Event which threads BLOCK on
	CRITICAL_SECTION m_csReqLock;		// Critical section lock
	int m_iWorkerThreads;				// Count of running worker threads
	int m_iMaxThreadCount;				// Maximum count of running threads
	LONG m_lRunningThreads;				// Count of running threads
	std::vector<HANDLE> m_arrThreads;	// Running thread handle list

// Constructor/Destructor
public:
	CIntThreadMgr(int iMaxCount);		// Default does 1 per 1M RAM detected
	virtual ~CIntThreadMgr() { /* */ }

// Access methods
public:
	LONG WaitForAllWorkersToStop(DWORD dwTimeout);

// Internal methods
protected:
	friend unsigned __stdcall tsplib_PoolThread(void* pParam);
	virtual void Runner() = 0;
	virtual bool CheckForThreadCreate(int nQueued) const = 0;
	void SpawnWorkerThread();
};

/**************************************************************************
** TPM_DelEvent
**
** This template function is used to delete an event. It is the default
** function created for the thread pool manager. If you have special needs
** related to deleting an event object (i.e. it was not allocated with new,
** it is reference counted, etc.) then simply declare your own
** TPM_DelRequest with the proper parameters before including this file.
**
** For example:
**
** void TPM_DelEvent(CEvent* pEvent) { pEvent->Release(); }
** void TPM_DelEvent(TString str) {  Do nothing  }
**
***************************************************************************/
template <class _REQ> inline void
TPM_DelEvent(_REQ request)
{
	delete request;

}// TPM_DelEvent

/**************************************************************************
** TPM_CanRunEvent
**
** This template function is used to check whether the given event can be
** run at this moment in time. It is the default function created for the 
** thread pool manager. If you have special needs related to running
** events - you can run certain events with others running on the same key,
** then simply declare your own TPM_CanRunEvent with the proper parameters 
** before instantiating the CThreadPoolMgr template.
**
** For example:
**
** bool TPM_CanRunEvent(CEvent* pEvent, CMyKey* pKey, bool fSynchKeyIsLocked)
**
***************************************************************************/
template <class _REQ, class _KEY> inline bool
TPM_CanRunEvent(_REQ, _KEY, bool fSynchKeyIsLocked)
{
	// This will allow events to run as long as the synch key is not
	// currently held by another thread.  If there are requests which
	// MAY run simultaneously, then this function would return TRUE for
	// those events.
	return (fSynchKeyIsLocked == false);

}// TPM_CanRunEvent

/**************************************************************************
** TPoolRequest
**
** This object is a template for a pool manager request object.
**
***************************************************************************/
template <class _KEY, class _REQ>
struct TPoolRequest
{ 
	_KEY key; 
	_REQ req; 
	CTSPIConnection* pConn; 
#ifdef _DEBUG
	DWORD qTime;
#endif
};

/**************************************************************************
** CThreadPoolMgr
**
** This object is a template for a pool request queue with thread
** manager.  
**
***************************************************************************/
template <class _KEY, class _REQ>
class CThreadPoolMgr : public CIntThreadMgr
{
// Class data
protected:
	typedef TPoolRequest<_KEY,_REQ> PMREQ;
	std::list<PMREQ*> m_lstRequests;	// Queue of requests
	std::map<_KEY, int> m_mapRunning;	// Map of running keys

// Constructor
public:
	CThreadPoolMgr(int iMaxCount=0);
	virtual ~CThreadPoolMgr();

// Access methods
public:
	bool Add(CTSPIConnection* pConn, _KEY key, _REQ request);

// Internal methods
protected:
	virtual void Runner();
	virtual bool CheckForThreadCreate(int nQueued) const;
	PMREQ* GetNextAvailableRequest();
};

/*****************************************************************************
** Procedure:  CIntThreadMgr::CIntThreadMgr
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Constructor for the pool manager
**
*****************************************************************************/
inline CIntThreadMgr::CIntThreadMgr(int iMaxThreadCount) : 
	m_evtRequest(NULL), m_iWorkerThreads(0), m_lRunningThreads(0)
{
	// Setup our internal variables
	InitializeCriticalSection(&m_csReqLock);
	m_evtRequest = CreateEvent(NULL, false, false, NULL);

	// Get the processor count on this machine
	if (iMaxThreadCount == 0)
	{
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&ms);

		// Get the total amount of RAM available on this machine.
		// We will allow 1 thread per 1M of memory on the machine.  This
		// will in effect be our MAX number of threads.
		m_iMaxThreadCount = static_cast<int>((ms.dwTotalPhys / (1024L*1024L)));

		// Now limit that based on the number of processors on this server.
		// We allow up to 16 threads per processor -- this can be overridden
		// by pre-defining THREADS_PER_PROC.
		SYSTEM_INFO si;
		ZeroMemory(&si, sizeof(SYSTEM_INFO));
		GetSystemInfo(&si);
		if (si.dwNumberOfProcessors > 0)
			m_iMaxThreadCount = min((int)(si.dwNumberOfProcessors*PM_THREADS_PER_PROCESSOR), m_iMaxThreadCount);

		// Pre-allocate our vector
		m_arrThreads.reserve(m_iMaxThreadCount);
	}
	else if (iMaxThreadCount < MinimumThreadCount)
		iMaxThreadCount = MinimumThreadCount;

}// CIntThreadMgr::CIntThreadMgr

/*****************************************************************************
** Procedure:  CIntThreadMgr::SpawnWorkerThread
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This creates a new worker thread
**
*****************************************************************************/
inline void CIntThreadMgr::SpawnWorkerThread()
{
	UINT uiThread;
	HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, tsplib_PoolThread, reinterpret_cast<void*>(this), 0, &uiThread));
	if (hThread == NULL)
		_TSP_DTRACEX(TRC_THREADS, _T("CThreadPoolMgr: failed to create worker thread, 0x%lx\n"), GetLastError());
	else
	{
		m_arrThreads.push_back(hThread);

		_TSP_DTRACEX(TRC_THREADS, _T("CThreadPoolMgr: New worker thread created, total=%d (%ld running), max=%d\n"), 
					m_iWorkerThreads, m_lRunningThreads, m_iMaxThreadCount);
		++m_iWorkerThreads;
	}

}// CIntThreadMgr::SpawnWorkerThread

/*****************************************************************************
** Procedure:  CIntThreadMgr::WaitForAllWorkersToStop
**
** Arguments:  'dwTimeout' - Amount of time to wait for threads to exit
**
** Returns:    void
**
** Description:  This pauses the thread and waits for our array of worker
**               threads to shut down.  Do _not_ call this function on a 
**               worker thread!
**
******************************************************************************/
inline LONG CIntThreadMgr::WaitForAllWorkersToStop(DWORD dwTimeout)
{
	// If no threads running, then exit.
	if (m_arrThreads.empty())
		return 0;

	// Wait for the determined time
	int nCount = m_arrThreads.size();
	_TSP_DTRACEX(TRC_THREADS, _T("CThreadPoolMgr: waiting for %d worker threads to shutdown.\r\n"), nCount);
	LONG lResult = WaitForMultipleObjects(nCount, &m_arrThreads[0], TRUE, dwTimeout);
	if (lResult == WAIT_TIMEOUT)
		_TSP_DTRACEX(TRC_THREADS, _T("CThreadPoolMgr: timed out waiting for worker threads to shutdown.\r\n"));

	// Close all the thread handles.
	std::vector<HANDLE>::iterator itEnd = m_arrThreads.end();
	for (std::vector<HANDLE>::iterator it = m_arrThreads.begin(); it != itEnd; ++it)
		CloseHandle(*it);

	m_arrThreads.clear();
	return lResult;

}// CIntThreadMgr::WaitForAllWorkersToStop

/*****************************************************************************
** Procedure:  CThreadPoolMgr::CThreadPoolMgr
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Constructor for the pool manager
**
*****************************************************************************/
template <class _KEY, class _REQ>
inline CThreadPoolMgr<_KEY,_REQ>::CThreadPoolMgr(int iMaxThreadCount) :
	CIntThreadMgr(iMaxThreadCount)
{
}// CThreadPoolMgr::CThreadPoolMgr

/*****************************************************************************
** Procedure:  CIntThreadMgr::~CIntThreadMgr
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for the pool manager
**
*****************************************************************************/
template <class _KEY, class _REQ>
inline CThreadPoolMgr<_KEY,_REQ>::~CThreadPoolMgr()
{
	// Wait for all our worker threads to shutdown.
	WaitForAllWorkersToStop(10000);

	// Delete the request event and CS.
	CloseHandle(m_evtRequest);
	DeleteCriticalSection(&m_csReqLock);

	// Delete all the requests in our list
	for (std::list<PMREQ*>::iterator i = m_lstRequests.begin(); 
		i != m_lstRequests.end(); ++i) {
		TPM_DelEvent((*i)->req);	// User defined delete
		delete (*i);				// Request wrapper
	}
	m_lstRequests.clear();
	
}// CIntThreadMgr::~CIntThreadMgr

/*****************************************************************************
** Procedure:  CThreadPoolMgr::Runner
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This function is run by multiple worker threads which 
**               manage the incoming data from the device
**
*****************************************************************************/
template <class _KEY, class _REQ>
inline void CThreadPoolMgr<_KEY,_REQ>::Runner()
{
	// Work until we are told to exit.
	for ( ;; )
	{
		// Find a request which is available to work on
		PMREQ* pmReq = GetNextAvailableRequest();
		if (pmReq == NULL)
			break;

		_TSP_DTRACEX(TRC_WORKERTHRD, _T("CThreadPoolMgr running block 0x%lx, Conn=0x%lx, Key=0x%lx\n"),
							(DWORD) pmReq->req, pmReq->pConn->GetPermanentDeviceID(), (DWORD) pmReq->key);

		// Increment the running threads
		InterlockedIncrement(&m_lRunningThreads);

		// Process the request.
		try
		{
			_TSP_ASSERTE(pmReq->pConn != NULL);
			pmReq->pConn->ReceiveData(reinterpret_cast<LPCVOID>(pmReq->req));
			TPM_DelEvent(pmReq->req);
		}
		catch (std::exception& x) 
		{
#ifdef _DEBUG
			_TSP_DTRACEX(TRC_WARNINGS, _T("CThreadPoolMgr::Runner exception occurred: %s\n"), x.what());
#else
			UNREFERENCED_PARAMETER(x);
#endif
		}

		// Remove this entry from the map.  This will allow another thread to
		// pick a request with the same key.
		EnterCriticalSection(&m_csReqLock);
		if (!m_mapRunning.empty())
		{
			std::map<_KEY,int>::iterator iPos = m_mapRunning.find(pmReq->key);
			if (iPos != m_mapRunning.end())
			{
				// Decrement the "in" count for our threads
				if ((*iPos).second == 1)
					m_mapRunning.erase(iPos);
				else
					m_mapRunning[pmReq->key] = (*iPos).second-1;
			}
		}
		LeaveCriticalSection(&m_csReqLock);

		// Decrement the running threads
		InterlockedDecrement(&m_lRunningThreads);

		// Delete the request.  Note this does NOT delete the data!
		// That should have been done above by the deleting functional
		delete pmReq;
	}

}// CThreadPoolMgr::Runner

/*****************************************************************************
** Procedure:  CThreadPoolMgr::GetNextAvailableRequest
**
** Arguments:  void
**
** Returns:    PMREQ* pointer which has key/request
**
** Description:  This function runs through our queued list and finds a
**               request which a thread may work on.
**
*****************************************************************************/
template <class _KEY, class _REQ>
inline TPoolRequest<_KEY,_REQ>* CThreadPoolMgr<_KEY,_REQ>::GetNextAvailableRequest()
{
	// If the STOP event is signalled, exit here.
	if (WaitForSingleObject(GetSP()->GetShutdownEvent(), 0) == WAIT_OBJECT_0)
		return NULL;

	// Check to see if we have any requests in the list right now.  If so,
	// set the event so this thread will drop through to get one of them.
	// This is done because our event is a single-thread release event which
	// means that only one thread is released even if we receive multiple
	// events from the PBX.  This will ensure that all events are processed 
	// quickly.
	EnterCriticalSection(&m_csReqLock);
	if (!m_lstRequests.empty())
		SetEvent(m_evtRequest);
	LeaveCriticalSection(&m_csReqLock);

	// Loop until we have a request or we exit due to system shutdown.
	PMREQ* pmReq = NULL;
	HANDLE arrWait[2] = { GetSP()->GetShutdownEvent(), m_evtRequest };
	do
	{
		// Force the thread to wait on the event.
		LONG lResult = WaitForMultipleObjects(2, arrWait, false, INFINITE);
		if (lResult == WAIT_OBJECT_0 || lResult == WAIT_ABANDONED_0 || lResult == (WAIT_ABANDONED_0+1))
			return NULL;

		// Get the critical section.
		EnterCriticalSection(&m_csReqLock);

		// Try to find an event who's key is not currently in our run map.
		// If we don't find one, loop back around and go to sleep until another
		// request wakes us up.
		// Loop through all the request
		for (std::list<PMREQ*>::iterator iPos = m_lstRequests.begin(); iPos != m_lstRequests.end(); iPos++)
		{
			std::map<_KEY,int>::iterator keyPos = m_mapRunning.find((*iPos)->key);

#ifdef _DEBUG
			// Check the qTime for this request - if we seem to be keeping requests
			// queued for too long, spit out a diagnostic warning - this may be 
			// a problem with the worker pool template. This is an internal diagnostic
			// for development purposes.
			if (GetTraceLevel() & TRC_WARNINGS)
			{
				DWORD _dwCurrTick = GetTickCount(), _dwStartTick = (*iPos)->qTime;
				DWORD _dwElapsed = (_dwCurrTick < _dwStartTick) ? ((0xffffffff-_dwStartTick) + _dwCurrTick) : (_dwCurrTick - _dwStartTick);
				if (_dwElapsed >= 60000 && (_dwElapsed % 60000) == 0) // On a minute interval?
				{
					// If this warning ever fires, it generally means that the TAPI server
					// is unable to keep up with the event flow or there is a bottle-neck somewhere
					// in the event processing of the TSP.
					_TSP_DTRACEX(TRC_WARNINGS, _T("CThreadPoolMgr: WARNING - request 0x%lx has been queued against key 0x%lx for 0x%lx seconds!\n"),
									(DWORD)(*iPos)->req, (DWORD)(*iPos)->key, (_dwElapsed / 1000));
				}
			}
#endif

			// See if it is ok to run this event now - we call the inline
			// template function which by default will return the opposite of
			// the passed "fLockedKey" value.
			if (TPM_CanRunEvent((*iPos)->req, (*iPos)->key, (keyPos != m_mapRunning.end())))
			{
				// Assign the task
				pmReq = (*iPos);

				// Get the current "in" count of threads and bump it up by one.
				// Or set it directly to one if it is not currently locked.
				if (keyPos != m_mapRunning.end())
					m_mapRunning[pmReq->key] = (*keyPos).second+1;
				else
					m_mapRunning[pmReq->key] = 1;

				// Take the request out of the request list and return this 
				// thread to run the given event.
				m_lstRequests.erase(iPos);
				break;
			}
		}

		// If we found a request but there are still requests in the list,
		// then reset the event so another worker will wake up to field the
		// remaining requests.
		if (pmReq != NULL && !m_lstRequests.empty())
			SetEvent(m_evtRequest);

		// Exit the critical section.
		LeaveCriticalSection(&m_csReqLock);

	} while (pmReq == NULL);

	return pmReq;

}// CThreadPoolMgr::AvailableRequest

/*****************************************************************************
** Procedure:  CThreadPoolMgr::Add
**
** Arguments:  'req' - REQUEST to add to our list
**
** Returns:    true/false if request was queued
**
** Description:  This queues and potentially creates a worker thread.
**
*****************************************************************************/
template <class _KEY, class _REQ>
inline bool CThreadPoolMgr<_KEY,_REQ>::Add (CTSPIConnection* pConn, _KEY key, _REQ request)
{
	// Make sure we have a connection
	if (pConn == NULL)
		return false;

	// Create a new queued request object
	PMREQ* pReq = new PMREQ;
	if (pReq == NULL)
		return false;

	_TSP_DTRACEX(TRC_WORKERTHRD, _T("CThreadPoolMgr adding block 0x%lx, Conn=0x%lx, Key=0x%lx\n"),
						(DWORD) request, pConn->GetPermanentDeviceID(), (DWORD) key);

	// Store our information about the request
	pReq->key = key;
	pReq->req = request;
	pReq->pConn = pConn;

#ifdef _DEBUG
	pReq->qTime = GetTickCount();
#endif

	// Enter into the critical section
	EnterCriticalSection(&m_csReqLock);

	// Queue the request into the list.
	m_lstRequests.push_back(pReq);

	// Get the total count of queued requests.  
	int iSize = m_lstRequests.size();

	// Release the critical section
	LeaveCriticalSection(&m_csReqLock);

	// If we need to create a new worker thread, then spawn it here.
	if (CheckForThreadCreate(iSize))
		SpawnWorkerThread();

	// Set the event if we have requests.  This will release ONE thread.
	if (iSize > 0) 
		SetEvent(m_evtRequest);
	return true;

}// CThreadPoolMgr::Add

/*****************************************************************************
** Procedure:  CThreadPoolMgr::CheckForThreadCreate
**
** Arguments:  void
**
** Returns:    void
**
** Description:  This function determines whether to create a new worker
**               thread based on the total number currently running.
**               It can be replaced to change the creation behavior.
**
*****************************************************************************/
template <class _KEY, class _REQ>
bool CThreadPoolMgr<_KEY,_REQ>::CheckForThreadCreate(int nQueued) const
{
	// If the count of queued requests exceeds our threshold and most of the
	// threads are running now, then create another worker thread to process requests.
	return (m_iWorkerThreads == 0 ||
		(nQueued > CreationThreshold && m_iWorkerThreads <= m_lRunningThreads+1) &&
		(m_iWorkerThreads < m_iMaxThreadCount));

}// CThreadPoolMgr::CheckForThreadCreate

#endif // _SPLIB_POOLMGR_INC_