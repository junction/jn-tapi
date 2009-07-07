#include "stdafx.h"

#include "onsiptapi.h"
#include "logger.h"

OnSipTapiCall::OnSipTapiCall(OnSipXmppStates::CallStates callState,OnSipCallStateData callStateData,StateChangeReason::eOnStateChangeReason reason)
{
	m_callState = callState; m_callStateData = callStateData; m_reason = reason;
}

const OnSipTapiCall& OnSipTapiCall::operator=(const OnSipTapiCall& tapiCall)
{
	if ( this == &tapiCall )
		return *this;
	m_callState = tapiCall.m_callState;
	m_callStateData = tapiCall.m_callStateData;
	m_reason = tapiCall.m_reason;
	return *this;
}

tstring OnSipTapiCall::ToString() const
{
	return Strings::stringFormat( _T("<OnSipTapiCall callState=%s callData=%s reason=%d>"), OnSipXmppStates::CallStateToString(m_callState), 
		m_callStateData.ToString().c_str(), m_reason );
}

// Return the TAPI CallState for the OnSipTapiCall
DWORD OnSipTapiCall::GetTapiCallState() const
{	return GetTapiCallState( m_callState, m_callStateData ); }

//static
// Return the TAPI CallState value for the specified callstate and callstatedata
DWORD OnSipTapiCall::GetTapiCallState( OnSipXmppStates::CallStates callState, const OnSipCallStateData& callStateData ) 
{
	DWORD dwCallState = 0;

	// Get type of call, e.g. MakeCall, PhysicalCall, IncomingCall
	OnSipXmppCallType::CallType callType = callStateData.m_callType;

	// Check obvious callstates 
	if ( callState == OnSipXmppStates::Dropped )
		dwCallState = LINECALLSTATE_IDLE ;
	else if ( callState == OnSipXmppStates::Connected || callState == OnSipXmppStates::PreDropCall )
		dwCallState = LINECALLSTATE_CONNECTED;
	else if ( callState == OnSipXmppStates::PhysicalOutProceeding )
		dwCallState = LINECALLSTATE_PROCEEDING;
	else if ( callState == OnSipXmppStates::Offering )
		dwCallState = LINECALLSTATE_OFFERING;
	// Check special cases for MakeCall
	else if ( callType == OnSipXmppCallType::MakeCall )
	{
		if ( callState == OnSipXmppStates::PreMakeCall || callState == OnSipXmppStates::MakeCallSet )
			dwCallState = LINECALLSTATE_DIALTONE ;
		else if ( callState == OnSipXmppStates::MakeCallRequested ||  callState == OnSipXmppStates::MakeCallRequestedAnswered )
			dwCallState = LINECALLSTATE_DIALING;
		else if ( callState == OnSipXmppStates::MakeCallOutgoingCreated )
			dwCallState = LINECALLSTATE_PROCEEDING;
	}

	// If callstate was not set
	if ( dwCallState == 0 )
	{
		Logger::log_error( _T("OnSipTapiCall::GetTapiCallState unrecognized callType=%s callId=%ld callState=%s tapiCallState=%lx"),
			OnSipXmppCallType::CallTypeToString(callType),callStateData.m_callId, OnSipXmppStates::CallStateToString(callState), dwCallState );
		return LINECALLSTATE_UNKNOWN;
	}
	// If callstate recognized
	else
	{
		Logger::log_debug( _T("OnSipTapiCall::GetTapiCallState callType=%s callId=%ld callState=%s tapiCallState=%ld"),
			OnSipXmppCallType::CallTypeToString(callType),callStateData.m_callId, OnSipXmppStates::CallStateToString(callState), dwCallState );
		return dwCallState;
	}
}

//***********************************************************************************
//***********************************************************************************

OnSipInitState::OnSipInitState(OnSipInitStates::InitStates state,OnSipInitStateData stateData,StateChangeReason::eOnStateChangeReason reason)
{
	m_state = state; m_stateData = stateData; m_reason = reason;
}

const OnSipInitState& OnSipInitState::operator=(const OnSipInitState& initInfo)
{
	if ( this == &initInfo )
		return *this;
	m_state = initInfo.m_state;
	m_stateData = initInfo.m_stateData;
	m_reason = initInfo.m_reason;
	return *this;
}

tstring OnSipInitState::ToString() const
{	return Strings::stringFormat( _T("<OnSipInitState initState=%s reason=%d>"), OnSipInitStates::InitStatesToString(m_state), m_reason ); }

//***********************************************************************************
//***********************************************************************************

OnSipTapi::OnSipTapi()
{
	Logger::log_debug(_T("OnSipTapi::OnSipTapi"));
}

// Virtual notify from OnSipInitStateMachine of state changes
//    virtual call will occur during Poll() method
void OnSipTapi::InitStateChange(OnSipInitStates::InitStates state,OnSipInitStateData stateData,StateChangeReason::eOnStateChangeReason reason) 
{	
	Logger::log_debug( _T("OnSipTapi::InitStateChange state=%d reason=%d"), state, reason );
	OnSipXmpp::InitStateChange( state, stateData, reason );
	// Keep event data
	OnSipInitState initInfo(state,stateData,reason);
	m_lstInitEvents.push_back(initInfo);
}

// Virtual notify from OnSipCallStateMachine of state changes
//    virtual call will occur during Poll() method
void OnSipTapi::CallStateChange(OnSipXmppStates::CallStates state,OnSipCallStateData stateData,StateChangeReason::eOnStateChangeReason reason)
{	
	Logger::log_debug( _T("OnSipTapi::CallStateChange state=%d reason=%d"), state, reason );
	OnSipXmpp::CallStateChange( state, stateData, reason );
	// Keep event data
	OnSipTapiCall callInfo( state, stateData, reason );
	m_lstCallEvents.push_back( callInfo );
}

// Retrieve any InitState event data that may have occurred the Poll call.
// Data will be returned in lstEvents and internal list will be cleared out.
//   For sake of performance, lstEvents is not cleared on each call.  Be sure to check return value on whether filled
bool OnSipTapi::GetInitEvents( std::list<OnSipInitState>& lstEvents)
{
	if ( m_lstInitEvents.size() == 0 )
		return false;
	_checkThread.CheckSameThread();
	lstEvents = m_lstInitEvents;
	m_lstInitEvents.clear();
	return true;
}

// Retrieve any InitState event data that may have occurred the Poll call.
// Data will be returned in lstEvents and internal list will be cleared out.
//   For sake of performance, lstEvents is not cleared on each call.  Be sure to check return value on whether filled
bool OnSipTapi::GetCallEvents( std::list<OnSipTapiCall>& lstEvents)
{
	if ( m_lstCallEvents.size() == 0 )
		return false;
	_checkThread.CheckSameThread();
	lstEvents = m_lstCallEvents;
	m_lstCallEvents.clear();
	return true;
}

// Clear all Init and Call Events that were collected during the Poll
void OnSipTapi::ClearEvents()
{
	Logger::log_debug(_T("OnSipTapi::ClearEvents") );
	_checkThread.CheckSameThread();
	m_lstCallEvents.clear();
	m_lstInitEvents.clear();
}

bool OnSipTapi::Connect(LoginInfo& loginInfo,ConnectionError* ce)
{
	Logger::log_debug(_T("OnSipTapi::Connect enter"));

	// Clear out any current events and state
	ClearEvents();

	// Async call
	*ce = Start(loginInfo);
	if ( *ce != ConnNoError )
	{
		Logger::log_error( _T("OnSipTapi::Connect exiting, error connect=%d"), ce );
		// TODO:  Set reason why?  Invalid Logon? etc??
		return false;
	}
	Logger::log_debug(_T("OnSipTapi::Connect exit success"));
	return true;
}

bool OnSipTapi::Poll()
{
	ConnectionError ce = PollXMPP(1000);
	if ( ce == ConnNoError )
		return true;
	Logger::log_error(_T("OnSipTapi::Poll error=%d"), ce );
	return false;
}

// bShutdown = if true, then try proper shutdown by having
//   OnSipInitStateMachine do shutdown, unsubscribe, etc.
void OnSipTapi::Disconnect(bool bShutdown)
{
	Logger::log_debug(_T("OnSipTapi::Disconnect bShutdown=%d"),bShutdown);
	if ( bShutdown )
	{
		AsyncShutdown();
		TimeOut tmout(5000);
		while ( Poll() && !tmout.IsExpired() )
		{
			Sleep(50);
			if ( IsShutdownComplete() )
			{
				Logger::log_debug(_T("OnSipTapi::Disconnect shutdown complete") );
				break;
			}
		}
	}
	Cleanup();
}

// Connect and do initialize of the IniStateMachine, e.g. full connect, authorize, and enable call events.
// Returns false if error.
//    loginInfo = login information
//    hDevStop = event handle to check if to exit immediately
//    stateType = return state type of initialize state machine, specifies whether error due to disconnected, fatal (e.g. authorize), etc.
bool OnSipTapi::InitOnSipTapi(LoginInfo& loginInfo,HANDLE hDevStop,OnSipInitStatesType::InitStatesType* stateType)
{
	*stateType = OnSipInitStatesType::NOTSET;

	// Try first Connect to XMPP
	ConnectionError ce;
	if ( !Connect(loginInfo,&ce) )
	{
		// See if fatal type of error, such as authorize, etc.
		// Else, assume just normal DISCONNECT error
		if ( ce == ConnAuthenticationFailed || ce == ConnOutOfMemory )
			*stateType = OnSipInitStatesType::FATAL;
		else
			*stateType = OnSipInitStatesType::DISCONNECTED;

		// Signal OpenDevice thread that we have done our initial connect
		Logger::log_error(_T("OnSipTapi::InitOnSipTapi Connect error, exiting. ce=%d. stateType=%s"),ce, OnSipInitStatesType::InitStatesTypeToString(*stateType));
		return false;
	}

	Logger::log_debug(_T("OnSipTapi::ConnectionThread poll for init"));

	// Wait for initial connect...
	while ( true )
	{
		// See if requested to exit by main thread
		if ( WaitForSingleObject(hDevStop, 0) != WAIT_TIMEOUT )
		{
			*stateType = OnSipInitStatesType::FATAL;
			Logger::log_error(_T("OnSipTapi::InitOnSipTapi Poll Init SIGNAL stop") );
			return false;
		}

		// Poll to keep the state machine going and process XMPP events
		if ( !Poll() )
		{
			Logger::log_error(_T("OnSipTapi::InitOnSipTapi Poll error exiting"));
			*stateType = OnSipInitStatesType::DISCONNECTED;
			return false;
		}
		// Shouldn't require a sleep, the Poll does this some, but have anyway
		Sleep(50);

		// Get the current state and state type
		OnSipInitStates::InitStates curState = GetInitStateMachineState();
		*stateType  = OnSipInitStatesType::GetInitStatesType( curState  );
		Logger::log_debug(_T("OnSipTapi::InitOnSipTapi Poll Init state=%s stateType=%s"), 
			OnSipInitStates::InitStatesToString(curState), OnSipInitStatesType::InitStatesTypeToString( *stateType  ) );

		// See if error type of state
		if ( *stateType == OnSipInitStatesType::DISCONNECTED || *stateType == OnSipInitStatesType::FATAL )
		{
			Logger::log_error(_T("OnSipTapi::InitOnSipTapi Poll init error, state=%s stateType=%s, exiting"), 
				OnSipInitStates::InitStatesToString(curState), OnSipInitStatesType::InitStatesTypeToString( *stateType  ) );
			return false;
		}

		// See if initialized
		if ( *stateType == OnSipInitStatesType::OK )
		{
			Logger::log_debug(_T("OnSipTapi::InitOnSipTapi Poll Init OK" ));
			break;
		}
	}

	// Clear all events that occurred during the initialization
	ClearEvents();
	return true;
}

