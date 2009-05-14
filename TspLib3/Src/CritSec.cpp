/******************************************************************************/
//                                                                        
// CRITSEC.CPP - Source code for the CIntCriticalSection object
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the source to manage the synchronization
// object which is used within each class.
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

/*-----------------------------------------------------------------------------*/
// GLOBAL DATA
/*-----------------------------------------------------------------------------*/
#ifdef _DEBUG
int CIntCriticalSection::g_iEvtCount = 0;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::CIntCriticalSection
//
// Constructor for the critical section object
//
CIntCriticalSection::CIntCriticalSection() : 
	m_lInCS(0), m_lLockCount(-1), m_lInThreadCount(0), 
	m_dwLastUse(0), m_dwThreadId(0), m_hevtWait(NULL)
{
#ifdef _DEBUG
	m_dwLastThreadId = 0;
#endif

}// CIntCriticalSection::CIntCriticalSection

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::CIntCriticalSection
//
// Destructor for the critical section object
//
CIntCriticalSection::~CIntCriticalSection()
{
	if (m_hevtWait != NULL)
	{
		CloseHandle(m_hevtWait);
		m_hevtWait = NULL;
#ifdef _DEBUG
		_TSP_DTRACEX(TRC_CRITSEC, _T("~CriticalSection 0x%lx deleting event, count = %d\n"), this, --g_iEvtCount);
#endif
	}

}// CIntCriticalSection::~CIntCriticalSection

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::Lock
//
// Lock method for the critical section
//
bool CIntCriticalSection::Lock(DWORD dwTimeout)
{
	// Get the current thread id.
	DWORD dwThreadId = GetCurrentThreadId();

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_LOCKS, _T("Lock(%ld) 0x%lx [Total=%ld], Thread=0x%lx, LockCnt=%ld, Owner=0x%lx, InThread=%ld, hEvent=0x%lx\n"),
			dwTimeout, this, g_iEvtCount, dwThreadId, m_lLockCount, m_dwThreadId, m_lInThreadCount, m_hevtWait);
#endif

	// Increment the lock count.  If it is zero, we own it.
	if (InterlockedIncrement(&m_lLockCount) == 0)
	{
		m_dwThreadId = dwThreadId;
		m_lInThreadCount = 1;
	} 

	// Otherwise, we play the waiting game.
	else
	{
		// If this thread already owns the critical section
		// then update the in thread count and exit.
		if (m_dwThreadId == dwThreadId)
			++m_lInThreadCount;

		// Otherwise, another thread owns it, now we have to
		// wait for it to be released.
		else 
		{
			// If the timeout is zero, then we never wait.
			// We cannot get the critical section now, so exit.
			if (dwTimeout == 0)
				return false;

			// Wait for the current owner to release the critical
			// section and for us to gain ownership.  If it times-out,
			// return that we don't have the critical section.
			if (!BlockThread(dwTimeout))
				return false;

			// Otherwise, we own the object.
			m_dwThreadId = dwThreadId;
			m_lInThreadCount = 1;
		}
	}
	return true;

}// CIntCriticalSection::Lock

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::Unlock
//
// Unlock method for the critical section
//
bool CIntCriticalSection::Unlock()
{
#ifdef _DEBUG
	_TSP_DTRACEX(TRC_LOCKS, _T("Unlock() 0x%lx [Total=%ld], Thread=0x%lx, LockCnt=%ld, Owner=0x%lx, InThread=%ld, hEvent=0x%lx\n"),
			this, g_iEvtCount, GetCurrentThreadId(), m_lLockCount, m_dwThreadId, m_lInThreadCount, m_hevtWait);
#endif

	// Sanity checks
	_TSP_ASSERTE(GetCurrentThreadId() == m_dwThreadId);
	_TSP_ASSERTE(m_lInThreadCount > 0);
	_TSP_ASSERTE(m_lLockCount >= 0);

	// If our thread entered the critical section more than once,
	// then require an unlock for *each* thread entry.
	if (--m_lInThreadCount > 0)
		InterlockedDecrement(&m_lLockCount);

	// Otherwise, decrement the main lock count
	else
	{
#ifdef _DEBUG
		m_dwLastThreadId = m_dwThreadId;
#endif
		m_dwThreadId = 0;

		// If the lock count is NOT zero, then unblock any
		// thread waiting.  The event is a PULSED event, so only
		// one thread will be tagged for execution.
		if (InterlockedDecrement(&m_lLockCount) >= 0) 
		{
			UnblockWaiter();
		}
		else
		{
			// If the last used counter has timed-out, then
			// delete the event.  We wait for one minute without
			// contention for the critical section.
			if (m_hevtWait != NULL && (m_dwLastUse + 60000) < GetTickCount())
			{
				AquireSpinlock();
				CloseHandle(m_hevtWait);
				m_hevtWait = NULL;
				ReleaseSpinlock();
#ifdef _DEBUG
				_TSP_DTRACEX(TRC_CRITSEC, _T("CriticalSection 0x%lx deleting event, count = %d\n"), this, --g_iEvtCount);
#endif
			}
		}
	}

	return true;

}// CIntCriticalSection::Unlock

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::AquireSpinlock
//
// Wait for the spinlock
//
bool CIntCriticalSection::AquireSpinlock(DWORD dwMsecs)
{
	DWORD dwElapsed, dwCurrTick, dwStartTick = GetTickCount();
#ifdef _DEBUG
	bool fErrorNoted = false;
#endif
	while (InterlockedExchange(&m_lInCS, 1) == 1)
	{
		dwCurrTick = GetTickCount();
		dwElapsed = (dwCurrTick < dwStartTick) ? ((0xffffffff-dwStartTick) + dwCurrTick) : (dwCurrTick - dwStartTick);

		// Timeout if necessary
		if ((dwMsecs > 0 && dwMsecs != INFINITE) && dwElapsed >= dwMsecs)
			return false;

#ifdef _DEBUG
		if (dwElapsed > 60000 && !fErrorNoted)
		{
			fErrorNoted = true;
			_TSP_DTRACEX(TRC_WARNINGS, _T("Warning: thread 0x%lx waiting on spinlock for over 1 min.\n"), GetCurrentThreadId());
		}
#endif
		Sleep(0);
	}
	return true;

}// CIntCriticalSection::AquireSpinlock

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::ReleaseSpinlock
//
// Release the spinlock
//
void CIntCriticalSection::ReleaseSpinlock()
{
	InterlockedDecrement(&m_lInCS);

}// CIntCriticalSection::ReleaseSpinlock

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::BlockThread
//
// Put the current thread to sleep waiting for the critical
// section to release.
//
bool CIntCriticalSection::BlockThread(DWORD dwTimeout)
{
	// The waiting scheme is simple.  We use a spinlock
	// which we hold only long enough to check and
	// possibly create an event.
	if (!AquireSpinlock(dwTimeout))
		return false;

	// We don't want to waste CPU cycles waiting for the
	// critical section, so we use an event.  If we don't
	// yet have an event created (lazy create), then create
	// one now.
	if (m_hevtWait == NULL)
	{
		m_hevtWait = CreateEvent(NULL, FALSE, FALSE, NULL);
#ifdef _DEBUG
		++g_iEvtCount;
#endif
	}

	// Mark the last time we needed the event.  We will
	// delete the event if we find it is not being used anymore.
	m_dwLastUse = GetTickCount();

	// Unlock the internal flag
	ReleaseSpinlock();

	// Wait for our event to trigger the thread.  It is a
	// "pulsed" event so each thread will be woken in turn.
	return (WaitForSingleObject(m_hevtWait, dwTimeout) != WAIT_TIMEOUT);

}// CIntCriticalSection::BlockThread

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIntCriticalSection::UnblockWaiter
//
// Wake up a single thread waiting for this object.
//
void CIntCriticalSection::UnblockWaiter()
{
	// The waiting scheme is simple.  We use a spinlock
	// which we hold only long enough to check and
	// possibly create an event.
	AquireSpinlock(0);

	// We don't want to waste CPU cycles waiting for the
	// critical section, so we use an event.  If we don't
	// yet have an event created (lazy create), then create
	// one now.  This can happen if the owner thread woke up
	// while another thread was in lock but had not yet called
	// BlockThread.
	if (m_hevtWait == NULL)
	{
		m_hevtWait = CreateEvent(NULL, FALSE, FALSE, NULL);
#ifdef _DEBUG
		++g_iEvtCount;
#endif
	}

	// Mark the last time we needed the event.  We will
	// delete the event if we find it is not being used anymore.
	m_dwLastUse = GetTickCount();

	// Unlock the internal flag
	ReleaseSpinlock();

	// Now set the event - this will release one thread.
	_TSP_VERIFY(SetEvent(m_hevtWait));

}// CIntCriticalSection::UnblockWaiter
