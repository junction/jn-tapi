#include "stdafx.h"
#include "OnSipThread.h"

OnSipThread::OnSipThread(LoginInfo& loginInfo)
{
	m_loginInfo = loginInfo;
}

// Virtual from within thread to create an instance
// of the OnSipXmpp. Can be taken over to
// create difference instance or provide an initialized
// instance.
//virtual 
OnSipXmpp *OnSipThread::CreateOnSipXMPP()
{
	Logger::log_debug(_T("OnSipXmpp::CreateOnSipXMPP"));
	return new OnSipXmpp();
}

// THREAD-SAFE
//
// Request to drop the specified call
void OnSipThread::DropCall(long callId)
{
	Logger::log_app(_T("OnSipThread::DropCall %ld"), callId );
	// If thread is not running
	if ( !MyThread::IsInThread() )
	{
		Logger::log_error(_T("OnSipThread::DropCall %ld - NOT IN THREAD"), callId );
		return;
	}

	// Access the main XMPP object
	{
		CriticalSectionScope css(&m_cs);
		if ( m_xmpp.get() == NULL )
		{
			Logger::log_error(_T("OnSipThread::DropCall %ld - No XMPP object"), callId );
			return;
		}
		m_xmpp->DropCall(callId);
	}

	Logger::log_app(_T("OnSipThread::DropCall callId=%d"), callId );
}

// THREAD-SAFE
//
// Request to dial the specified number.
// Returns the unique call-id to track the call.
// Returns 0 if error.
long OnSipThread::MakeCall(const tstring& number)
{
	Logger::log_app(_T("OnSipThread::MakeCall %s"), number.c_str() );
	// If thread is not running
	if ( !MyThread::IsInThread() )
	{
		Logger::log_error(_T("OnSipThread::MakeCall %s - NOT IN THREAD"), number.c_str() );
		return 0;
	}
	long callId =0;

	// Access the main XMPP object
	{
		CriticalSectionScope css(&m_cs);
		if ( m_xmpp.get() == NULL )
		{
			Logger::log_error(_T("OnSipThread::MakeCall %s - No XMPP object"), number.c_str() );
			return 0;
		}
		callId = m_xmpp->MakeCall(number);
	}

	Logger::log_app(_T("OnSipThread::MakeCall %s, callId=%d"), number.c_str(), callId );
	return callId;
}

// Start the thread that is handling the OnSipXMPP engine
void OnSipThread::Start()
{
	Logger::log_debug( _T("OnSipThread::Start") );
	MyThread::ResumeThread();
}

// Main Thread Function
//virtual 
DWORD OnSipThread::Run()
{
	Logger::log_debug( _T("OnSipThread::Run enter") );

	// Create the main XMPP object
	{
		CriticalSectionScope css(&m_cs);
		m_xmpp.reset( CreateOnSipXMPP() );		// Call virtual to create instance
	}

	// Async call
	ConnectionError ce = m_xmpp->Start(m_loginInfo);
	if ( ce != ConnNoError )
	{
		Logger::log_error( _T("OnSipThread::Run exiting, error connect=%d"), ce );
		return ce;
	}

	// Stay in pollling loop until error or signal exit
	while ( ce == ConnNoError && !MyThread::IsSignalStop() )
	{
		Sleep(50);
		ce = m_xmpp->PollXMPP(1000);
	}

	m_xmpp->Cleanup();

	// Destroy the main XMPP object
	{
		CriticalSectionScope css(&m_cs);
		m_xmpp.reset(NULL);
	}

	Logger::log_debug( _T("OnSipThread::Run exit. ce=%d signalStop=%d"), ce, MyThread::IsSignalStop() );
	return ce;
}
