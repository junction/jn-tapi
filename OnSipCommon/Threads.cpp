#include "stdafx.h"
#include "threads.h"
#include "logger.h"

// Checks to see if the call to this method is in the same
// thread to when the CheckThread object was created.
// Used to help ensure some parts of code are always called in the same thread.
void CheckThread::CheckSameThread()
{
	if ( IsSameThread() )
		return;
	tstring msg = Strings::stringFormat(_T("CheckThread::CheckSameThread objThread=%ld curThread=%ld"), m_dwThreadId, GetCurrentThreadId() );
	Logger::log_error(msg.c_str());
	_ASSERT(false);
}

//****************************************************************************
//****************************************************************************

CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_cs);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_cs);
}

void CriticalSection::Enter()
{
	EnterCriticalSection(&m_cs);
}

void CriticalSection::Leave()
{
	LeaveCriticalSection(&m_cs);
}

//****************************************************************************
//****************************************************************************

// Simple class that will "Enter" a critical section
// on the constructor, and "Leave" on its destructor
CriticalSectionScope::CriticalSectionScope(CriticalSection *cs)
{
	m_cs = cs;
	cs->Enter();
}

CriticalSectionScope::~CriticalSectionScope()
{
	m_cs->Leave();
}

//****************************************************************************
//****************************************************************************

MyThread::MyThread()
{
	m_thread = NULL;
	m_threadId = 0;
	m_bStopThread = false;
	m_exitCode = 0;
	m_bInThread = false;
	_Create();
}

MyThread::~MyThread()
{
	if ( m_thread != NULL )
	{
		Logger::log_warn( _T("MyThread::~MyThread thread being forced terminated") );
		CloseHandle(m_thread);
	}
	m_thread = NULL;
}

// Create thread in suspend state
bool MyThread::_Create()
{
	if ( m_thread != NULL )
	{
		Logger::log_error(_T("MyThread::Create thread already exists"));
		_ASSERT(false);
		return false;
	}

	// create suspended player thread
	m_thread = CreateThread(  NULL,
							 0,
							 (LPTHREAD_START_ROUTINE)_ThreadMethod,
							 (LPVOID)this,
							 CREATE_SUSPENDED,
							 &m_threadId
							 );
	if ( m_thread == NULL )
	{
		Logger::log_error(_T("MyThread::Create error create %lx"), GetLastError() );
		return false;
	}
	return true;
}

void MyThread::ResumeThread()
{
	Logger::log_debug( _T("MyThread::ResumeThread") );
	_ASSERT( m_thread != NULL );
	::ResumeThread(m_thread);
}

void MyThread::Terminate()
{
	Logger::log_warn( _T("MyThread::Terminate") );
	if ( m_thread != NULL )
		::TerminateThread(m_thread,-1);
	m_bInThread = false;
	m_thread = NULL;
}

void MyThread::SuspendThread()
{
	Logger::log_debug( _T("MyThread::SuspendThread") );
	if ( m_thread != NULL )
		::SuspendThread(m_thread);
}

bool MyThread::Join(DWORD dwMsecs)
{
	Logger::log_debug( _T("MyThread::Join msecs=%ld"), dwMsecs );
	_ASSERT( GetCurrentThreadId() != m_threadId );
	bool ret = WaitForSingleObject(m_thread,dwMsecs) == WAIT_OBJECT_0;
	Logger::log_debug( _T("MyThread::Join msecs=%ld ret=%d"), dwMsecs, ret );
	// If success, then close thread handle
	if ( ret )
	{
		CloseHandle(m_thread);
		m_thread = NULL;
		m_threadId = 0;
		m_bInThread = false;
	}
	return ret;
}

//static
DWORD WINAPI MyThread::_ThreadMethod(LPVOID lpVoid)
{
	MyThread* pMyThread = (MyThread*) lpVoid;
	pMyThread->m_bInThread = true;
	pMyThread->m_exitCode = pMyThread->Run();
	return pMyThread->m_exitCode;
}
