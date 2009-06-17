#include <stdafx.h>

#include "onsipcallstatemachine.h"
#include "logger.h"
#include "onsipxmpp.h"
#include <algorithm>
#include "Registry.h"

//static 
bool OnSipCallStateHelper::IsSameId(XmppEvent* pEvent1,XmppEvent* pEvent2)
{	
	if ( pEvent1 == NULL || pEvent2 == NULL )
		return false;
	return pEvent1->m_id == pEvent2->m_id;	
}

//static 
bool OnSipCallStateHelper::IsSameContext(XmppEvent* pEvent1,XmppEvent* pEvent2)
{
	if ( pEvent1 == NULL || pEvent2 == NULL )
		return false;
	return pEvent1->m_context == pEvent2->m_context;
}

//static 
XmppActiveCallEvent* OnSipCallStateHelper::getActiveCallEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_PUBSUB_ACTIVECALL_EVENT )
		return NULL;
	return dynamic_cast<XmppActiveCallEvent *>(pEvent);
}

//static 
DropRequestEvent* OnSipCallStateHelper::getDropRequestEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_DROP_REQUEST )
		return NULL;
	return dynamic_cast<DropRequestEvent*>(pEvent);
}

//static 
ShutdownRequestEvent* OnSipCallStateHelper::getShutdownRequestEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_SHUTDOWN_REQUEST )
		return NULL;
	return dynamic_cast<ShutdownRequestEvent*>(pEvent);
}

//static
XmppRetractCallEvent* OnSipCallStateHelper::getRetractCallEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_PUBSUB_RETRACTCALL_EVENT )
		return NULL;
	return dynamic_cast<XmppRetractCallEvent *>(pEvent);
}

//static
XmppIqResultEvent* OnSipCallStateHelper::getXmppIqResultEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_IQ_RESULT )
		return NULL;
	return dynamic_cast<XmppIqResultEvent *>(pEvent);
}

//static
XmppOnConnectEvent* OnSipCallStateHelper::getOnConnectEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_ONCONNECT )
		return NULL;
	return dynamic_cast<XmppOnConnectEvent *>(pEvent);
}

//static
XmppOnDisconnectEvent* OnSipCallStateHelper::getOnDisconnectEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_ONDISCONNECT )
		return NULL;
	return dynamic_cast<XmppOnDisconnectEvent *>(pEvent);
}

//static
XmppAuthEvent* OnSipCallStateHelper::getAuthEvent(XmppEvent* pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_PUBSUB_AUTH )
		return NULL;
	return dynamic_cast<XmppAuthEvent *>(pEvent);
}

//static
XmppPubSubSubscribedEvent* OnSipCallStateHelper::getPubSubSubscribedEvent(XmppEvent *pEvent)
{
	if ( pEvent->m_type != XmppEvent::EVT_PUBSUB_SUBSCRIBE )
		return NULL;
	return dynamic_cast<XmppPubSubSubscribedEvent *>(pEvent);
}

//static
void OnSipCallStateHelper::AssignCallStateData(OnSipCallStateData& callStateData,XmppActiveCallEvent* ace,long callId,bool bUpdateCallerId)
{
	_ASSERT( ace != NULL );
	if ( ace == NULL )
		return;
	if ( callId != -1 )
		callStateData.m_callId = callId;
	callStateData.m_id = ace->m_id;
	callStateData.m_sipCallId = ace->m_sipCallid;		// SIP callId
	callStateData.m_branch = ace->m_branch;

	// Caller-id not supported, so skip this for now.  May check in doing called-id for outgoing calls.
	if ( bUpdateCallerId )
	{
		callStateData.m_remoteId = ace->m_from_uri;
		callStateData.m_calledId = ace->m_to_uri;
	}

	// Assign from/to tags
	callStateData.m_fromTag = ace->m_from_tag;
	callStateData.m_toTag = ace->m_to_tag;
}

//****************************************************************************
//****************************************************************************

// Checks the branches list for a call with the specified XMPP id.
// Returns the index in the list, or returns -1 if not found
int callBranches::_getIdIndex(tstring id)
{
	int index=0;
	for ( vector<OnSipCallStateData>::iterator iter = m_branches.begin(); iter != m_branches.end(); iter++ )
	{
		if ( (*iter).m_id == id )
		{
			Logger::log_debug( _T("callBranches::_getIdIndex id=%s ndx=%d"), id.c_str(), index );
			return index;
		}
		index++;
	}
	// not found
	return -1;
}

// Returns true if the specified XMPP id is in the list of branch calls
bool callBranches::HasId(tstring id)
{
	int index = _getIdIndex(id);
	if ( index >= 0 )
	{
		Logger::log_debug( _T("callBranches::HasId id=%s"), id.c_str() );
		return true;
	}
	return false;
}

// Add the branch call to our branches list.
// Added by the active call event and the callId
void callBranches::AddBranch(OnSipCallStateData& callData)
{
	Logger::log_debug( _T("callBranches::AddBranch id=%s branch=%s callId=%ld"), callData.m_id.c_str(), callData.m_branch.c_str(), callData.m_callId );
	// If we already have this item in the branches
	int index = _getIdIndex( callData.m_id );
	// If already in the list, then just update the calldata
	if ( index >= 0 )
	{
		Logger::log_debug( _T("callBranches::AddBranch id=%s callData updated") );
		m_branches[index] = callData;
		return;
	}
	// Add to branches
	Logger::log_debug( _T("callBranches::AddBranch id=%s newAdded") );
	m_branches.push_back( callData );
}

// Add the branch call to our branches list.
// Added by the active call event and the callId
// If call is already part of the 
void callBranches::AddBranch(XmppActiveCallEvent *ace,long callId,OnSipXmppCallType::CallType callType, bool bUpdateCallerId )
{
	Logger::log_debug( _T("callBranches::AddBranch ace id=%s branch=%s callId=%ld callType=%s bUpdateCallerId=%d cnt=%d"), ace->m_id.c_str(), ace->m_branch.c_str(), callId, OnSipXmppCallType::CallTypeToString(callType), bUpdateCallerId, m_branches.size() );

	// Get the callstate data from the call event
	OnSipCallStateData callData;
	callData.m_callType = callType;
	OnSipCallStateHelper::AssignCallStateData(callData,ace,callId,bUpdateCallerId);

	// If we already have this item in the branches
	int index = _getIdIndex( ace->m_id );
	// If already in the list, then just update the calldata
	if ( index >= 0 )
	{
		Logger::log_debug( _T("callBranches::AddBranch id=%s callData updated"), ace->m_id.c_str() );
		m_branches[index] = callData;
		return;
	}
	// Add to branches
	Logger::log_debug( _T("callBranches::AddBranch id=%s newAdded"), ace->m_id.c_str() );
	m_branches.push_back( callData );
}

// Removes the branch call by its XMPP id
void callBranches::_removeById(tstring id)
{
	for ( vector<OnSipCallStateData>::iterator iter = m_branches.begin(); iter != m_branches.end(); iter++ )
	{
		if ( (*iter).m_id == id )
		{
			Logger::log_debug( _T("callBranches::_removeById %s"), id.c_str() );
			m_branches.erase( iter );
			return;
		}
	}
	Logger::log_error( _T("callBranches::_removeById %s not found"), id.c_str() );
}

// Checks to see if the id (XMPP id) of a dropped call is the last one
// of the branches.  If so, returns true to signify the call is really dropping
//
// The callData will be updated with a valid callData of a remaining 
// call in the branches
bool callBranches::CheckDroppedCall( OnSipCallStateData* callData, tstring droppedId )
{
	Logger::log_debug( _T("callBranches::CheckDroppedCall calldata=%s droppedId=%s"), callData->m_id.c_str(), droppedId.c_str() );

	// If this branch list does not have the call
	if ( !HasId( droppedId ) )
		return false;

	// Remove the branch by its XMPP id
	Logger::log_debug( _T("callBranches::CheckDroppedCall found in branch, removing. branches=%d"), m_branches.size() );
	_removeById( droppedId );

	// If there are still more branches in this list,
	// then do not drop the call
	if ( m_branches.size() > 0 )
	{
		// Update the callData with a valid call that still exists
		*callData = m_branches[0];
		Logger::log_debug( _T("callBranches::CheckDroppedCall updated callData %s"), callData->ToString().c_str() );
		return false;
	}

	// No more calls left in the branches, drop the call
	Logger::log_debug( _T("callBranches::CheckDroppedCall signal dropped") );
	return true;
}

tstring callBranches::ToString()
{
	tstring ids;
	for ( vector<OnSipCallStateData>::iterator iter = m_branches.begin(); iter != m_branches.end(); iter++ )
	{
		if ( ids.empty() )
			ids = iter->m_id;
		else
			ids = Strings::stringFormat( _T("%s, %s"), ids.c_str(), iter->m_id.c_str() );
	}
	return Strings::stringFormat( _T("<Branches cnt=%d, %s>"), m_branches.size(), ids.c_str() );
}

//****************************************************************************
//****************************************************************************

// Helper method to convert CallStates enum to a string (for debug purposes)
//static 
TCHAR *OnSipXmppStates::CallStateToString(CallStates state)
{
	switch ( state )
	{
		case Unknown:
			return _T("CallState::Unknown");
		case Offering:
			return _T("CallState::Offering");
		case PhysicalOutProceeding:
			return _T("CallState::PhysicalOutProceeding");
		case Connected:
			return _T("CallState::Connected");
		case Dropped:
			return _T("CallState::Dropped");
		case PreMakeCall:
			return _T("CallState::PreMakeCall");
		case MakeCallSet:
			return _T("CallState::MakeCallSet");
		case MakeCallRequested:
			return _T("CallState::MakeCallRequested");
		case MakeCallRequestedAnswered:
			return _T("CallState::MakeCallRequestedAnswered");
		case MakeCallOutgoingCreated:
			return _T("CallState::MakeCallOutgoingCreated");
		case PreDropCall:
			return _T("CallState::PreDropCall");
		default:
		{
			Logger::log_error( Strings::stringFormat( _T("Unknown CallState in CallStateToString - %d"), state ).c_str() );
			return _T("Unrecognized CallState in CallStateToString");
		}
	}
}

// Class wrapper around the enum CallType
// static
TCHAR *OnSipXmppCallType::CallTypeToString(CallType calltype)
{
	switch ( calltype )
	{
		case Unknown:
			return _T("CallType::Unknown");
		case MakeCall:
			return _T("CallType::MakeCall");
		case IncomingCall:
			return _T("CallType::IncomingCall");
		case PhysicalCall:
			return _T("CallType::PhysicalCall");
		default:
		{
			Logger::log_error( Strings::stringFormat( _T("Unknown CallType in CallTypeToString - %d"), calltype ).c_str() );
			return _T("Unrecognized CallType in CallTypeToString");
		}
	}
}

// Convert OnSipCallStateData instance to a displayable string for debug purposes
tstring OnSipCallStateData::ToString() const
{
	return Strings::stringFormat(_T("<OnSipCallStateData id=%s callType=%s callId=%ld remote=%s calledId=%s sipCallId=%s branch=%s>"),
		m_id.c_str(), OnSipXmppCallType::CallTypeToString(m_callType), m_callId, m_remoteId.c_str(), m_calledId.c_str(), m_sipCallId.c_str(), m_branch.c_str() );
}

//****************************************************************************
//****************************************************************************

// Checks to see if the state has been in the specified state for the specified timeout in msecs.
// If so, then the call will be put in the Dropped state and return true.
bool OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::CallStates callState, long timeout )
{
	// If we are stuck in the state, then most likely we did not get any call events for the request
	if ( IsState( callState ) && MsecsSinceLastStateChange() > timeout  )
	{
		_ASSERTE( !IsState(OnSipXmppStates::Dropped) );

		// See if this is disabled by registry for debugging purposes
		DWORD dwValue=0;
		tstring path = Strings::stringFormat( _T("SOFTWARE\\%s\\DisableTimeout"), COMPANY_NAME );
		Registry::ReadKeyDword( HKEY_LOCAL_MACHINE, path.c_str(), &dwValue );
		if ( dwValue != 0 )
		{
			Logger::log_warn(_T("OnSipCallStateHandlerBase::CheckStateTimeout %s TIMEOUT msecs=%ld.  IGNORED due to registry override"), OnSipXmppStates::CallStateToString(callState), MsecsSinceLastStateChange() );
			return false;
		}

		Logger::log_error(_T("OnSipCallStateHandlerBase::CheckStateTimeout %s TIMEOUT msecs=%ld.  DROPPING call"), OnSipXmppStates::CallStateToString(callState), MsecsSinceLastStateChange() );
		assignNewState( OnSipXmppStates::Dropped, NULL );
		// Do not report that call does not exist yet, let it be handled on the next poll check
		return true;
	}

	return false;
}

// Check to see if the event is a request to drop the call associated with this handler
bool OnSipCallStateHandlerBase::CheckDropRequestEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine, XmppEvent* pEvent,long callId)
{
	// See if request to drop a call
	DropRequestEvent* dre = OnSipCallStateHelper::getDropRequestEvent(pEvent);
	if ( dre == NULL || dre->m_callId != callId )
		return false;

	Logger::log_debug( _T("OnSipCallStateHandlerBase::CheckDropRequestEvent dropRequestEvent. callId=%ld branches=%d"), dre->m_callId, m_branches.CountBranches() );

	_ASSERTE( m_branches.CountBranches() > 0 );

	OnSipCallStateMachine* pCallStateMachine = dynamic_cast<OnSipCallStateMachine*>(pStateMachine);

	// For all branches, drop the call
	vector<OnSipCallStateData> abranches = m_branches.getBranches();
	for ( vector<OnSipCallStateData>::iterator iter = abranches.begin(); iter != abranches.end(); iter++ )
	{
		long contextId=0;
		pCallStateMachine->DropCall( *iter, &contextId );
		// Save the contextId for the IQ request,
		// these will be used to watch for IQ replies
		m_droppedContextIds.push_back(contextId);
	}

	return true;
}

// Check to see if the IQ results for out attempts to drop the call.
// We would try to drop our calls if we got DropRequestEvent and handled (as above)
bool OnSipCallStateHandlerBase::CheckIqReplyDropRequest(XmppEvent* pEvent)
{
	XmppIqResultEvent* iqEvent = OnSipCallStateHelper::getXmppIqResultEvent(pEvent);
	if ( iqEvent == NULL )
		return false;

	// See if the contextId is in our list of dropped Iq request contextIds
	std::vector<long>::iterator iter = std::find( m_droppedContextIds.begin(), m_droppedContextIds.end(), pEvent->m_context );
	if ( iter == m_droppedContextIds.end() )
		return false;

	// It is ours.  Remove the contextId and return true to signify our event.
	m_droppedContextIds.erase( iter );
	Logger::log_debug( _T("OnSipCallStateHandlerBase::CheckIqReplyDropRequest dropRequestEvent reply for contextId=%d, left=%d"),pEvent->m_context, m_droppedContextIds.size() );
	return true;
}


//****************************************************************************
//****************************************************************************

// # msecs timeout that we wait max after requesting a makecall, that we will drop the call
// if there has been no call events reported on the call.
// This can occur if the SIP phone device is down, or the call events server is down
// and we request the call.  It will be stuck in DIALTONE state until we start seeing activity.
#define MAKECALL_CREATE_TIMEOUT				(10 * MSECS_IN_SEC)

// # msecs timeout that we wait max after the inbound call is ringing for the person to answer.
// Maybe the SIP phone is not ringing for some reason.  Allow reasonable time for person to answer
#define MAKECALL_INBOUND_REQUEST_TIMEOUT	(45 * MSECS_IN_SEC)

// # msecs timeout that we wait max after the inbound call has been answered and waiting for the outbound call to be created
#define MAKECALL_INBOUND_ANSWERED_TIMEOUT	(15 * MSECS_IN_SEC)

// # msecs timeout that we wait max after the outbound call to be answered.
// We should really wait a long time here, maybe the person is just persistent and
// want the other user's phone to ring over and over again.  Just a check to
// make sure we don't get stuck in this state forever.
#define OUTBOUND_REQUEST_TIMEOUT				(2 * MSECS_IN_MIN)

// # msecs timeout that we will allow a call to be in the CONNECTED state.
// Have a very long time, possible that a person is on a very long call,
// but we just don't want to be stuck in this state forever due to some communication error.
// There will be no negative effects, it will not affect the call, just the call will not be tracked anymore.
#define CONNECTED_CALL_TIMEOUT				(3 * MSECS_IN_HOUR)

// # msecs timeout that we will allow an incoming call to be ringing without an answer.
#define INBOUND_OFFERING_TIMEOUT				(2 * MSECS_IN_MIN)

// # msecs timeout that we allow a request to drop a call to wait before just assuming that it is dropped
#define DROP_REQUEST_TIMEOUT					(10 * MSECS_IN_SEC)

//****************************************************************************
//****************************************************************************
// State Handler for user created manual out-going phone calls
//****************************************************************************

OnSipOutgoingCallStateHandler::OnSipOutgoingCallStateHandler(XmppEvent* pEvent,long callId) : OnSipCallStateHandlerBase( OnSipXmppStates::PhysicalOutProceeding, pEvent )
{ 
	Logger::log_debug("OnSipOutgoingCallStateHandler::OnSipOutgoingCallStateHandler %s callId=%ld", pEvent->ToString().c_str(), callId );
	getCurrentStateData().m_callType = OnSipXmppCallType::PhysicalCall;
	// Set call information from the XmppActiveCallEvent
	OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),OnSipCallStateHelper::getActiveCallEvent(pEvent),callId);

	// Add the initial call to our branch list.  For an out-going call, we should only have 1
	m_branches.AddBranch( getCurrentStateData() );
}

//virtual 
bool OnSipOutgoingCallStateHandler::IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	Logger::log_debug( _T("OnSipOutgoingCallStateHandler::IsYourEvent %s"), pEvent->ToString().c_str()  );

	// See if an ActiveCallEvent with callstate change
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
		// If not same SIP CallId, then not our event
		if ( ace->m_sipCallid != getCurrentStateData().m_sipCallId )
			return false;

		// For outgoing calls, should only have 1 branch
		_ASSERTE( m_branches.CountBranches() == 1 );
		
		Logger::log_debug( _T("OnSipOutgoingCallStateHandler::IsYourEvent activeCallEvent %s"), pEvent->ToString().c_str()  );

		// Add this call to our list of branches.  For outgoing call, should only have 1!
		m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType );

		// For outgoing calls, should only have 1 branch
		_ASSERTE( m_branches.CountBranches() == 1 );

		// If connected call
		if ( ace->m_dialogState == XmppActiveCallEvent::CONFIRMED )
		{
			Logger::log_debug("OnSipOutgoingCallStateHandler::IsYourEvent connected");
			assignNewState( OnSipXmppStates::Connected, pEvent );
			// Update our callData
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1);
			return true;
		}
		Logger::log_error("OnSipOutgoingCallStateHandler::IsYourEvent Unknown State in OnSipOutgoingCallStateHandler %s", pEvent->ToString().c_str() );
		// Not keeping event
		delete pEvent;
		return true;
	}

	// See if a retracted (dropped call)
	XmppRetractCallEvent *rce = OnSipCallStateHelper::getRetractCallEvent(pEvent);
	if ( rce != NULL )
	{
		// See if id is in our branches
		// If not, then not our event
		if ( !m_branches.HasId(rce->m_id) )
			return false;

		// For outgoing calls, should only have 1 branch
		_ASSERTE( m_branches.CountBranches() == 1 );

		Logger::log_debug("OnSipOutgoingCallStateHandler::IsYourEvent dropped call");
		assignNewState( OnSipXmppStates::Dropped, pEvent );
		return true;
	}

	// Check to see if the event is a request to drop the call associated with this handler.
	// If DropRequestEvent for our call, then it will do the IQ requests to drop the calls,
	// delete the pEvent
	if ( CheckDropRequestEvent(pStateMachine,pEvent,getCurrentStateData().m_callId) )
	{
		Logger::log_debug( _T("OnSipOutgoingCallStateHandler::IsYourEvent dropping call callId=%ld"), getCurrentStateData().m_callId );
		// Go into dropping state
		assignNewState( OnSipXmppStates::PreDropCall, NULL );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	// Check to see if the IQ results for out attempts to drop the call.
	// We would try to drop our calls if we got DropRequestEvent and handled (as above)
	if ( CheckIqReplyDropRequest(pEvent) )
	{
		Logger::log_debug( _T("OnSipOutgoingCallStateHandler::IsYourEvent dropcall iqReply") );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	Logger::log_error("OnSipOutgoingCallStateHandler::IsYourEvent Unknown Event in OnSipOutgoingCallStateHandler %s", pEvent->ToString().c_str() );
	return false;
}

//virtual 
bool OnSipOutgoingCallStateHandler::IsStillExist()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// If we are stuck in a proceeding state, then the call is not being answered on the other end.
	// Have a reasonable timeout for this in case server errors.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::PhysicalOutProceeding, OUTBOUND_REQUEST_TIMEOUT	 ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a connected call for VERY long time, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::Connected, CONNECTED_CALL_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a dropping state, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::PreDropCall, DROP_REQUEST_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	return !IsState( OnSipXmppStates::Dropped );
}

//****************************************************************************
//****************************************************************************
// State Handler for incoming phone calls
//****************************************************************************

OnSipIncomingCallStateHandler::OnSipIncomingCallStateHandler(XmppEvent* pEvent,long callId) : OnSipCallStateHandlerBase( OnSipXmppStates::Offering, pEvent )
{ 
	Logger::log_debug("OnSipIncomingCallStateHandler::OnSipIncomingCallStateHandler pEvent=%s callId=%ld", pEvent->ToString().c_str(), callId );
	getCurrentStateData().m_callType = OnSipXmppCallType::IncomingCall;

	// Set call information from the XmppActiveCallEvent
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	_ASSERTE( ace != NULL );
	OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,callId);

	// Add the initial call to our branch list.
	// Branch list is required to track multi events for the same call when user 
	// has multiple SIP phones registered at the same location.
	m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType );
}

//virtual 
bool OnSipIncomingCallStateHandler::IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent %s"), pEvent->ToString().c_str()  );
	
	// See if an ActiveCallEvent with callstate change
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
		// If not same SIP CallId, then not our event
		if ( ace->m_sipCallid != getCurrentStateData().m_sipCallId )
			return false;

		Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent activeCallEvent matching sipCallId %s, callId=%ld"), ace->m_sipCallid.c_str(), getCurrentStateData().m_callId );

		// See if this is a new branch call event.
		// This can occur if the user has multiple SIP phones registered for the same phone number.
		// Get multiple events, one for each SIP device.
		// Events will have same SIP callid, but a different branch and XMPP id value
		m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType  );

		// If connected call
		if ( ace->m_dialogState == XmppActiveCallEvent::CONFIRMED )
		{
			Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent connected") );
			assignNewState( OnSipXmppStates::Connected, pEvent );
			// Update our callData
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1);
			return true;
		}

		// If not connected, then should be the REQUESTED event of call on another branch.
		// Just ignore, we are only tracking the connected event

		// Delete event, we did not keep
		delete pEvent;
		return true;
	}

	// See if a retracted (dropped call)
	XmppRetractCallEvent *rce = OnSipCallStateHelper::getRetractCallEvent(pEvent);
	if ( rce != NULL )
	{
		// See if id is in our branches
		// If not, then not our event
		if ( !m_branches.HasId(rce->m_id) )
			return false;

		Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent dropped call. branches=%d, callId=%ld"), m_branches.CountBranches(), getCurrentStateData().m_callId );

		// Check to see if this is the last call on a branch hanging up.
		// Also update the callData with a valid CallData of one of the branch calls
		OnSipCallStateData& callData = getCurrentStateData() ;
		if ( m_branches.CheckDroppedCall( &callData, rce->m_id ) )
		{
			assignNewState( OnSipXmppStates::Dropped, pEvent );
			return true;
		}
		// Ignore event, just one of the branches disconnecting
		delete pEvent;
		return true;
	}

	// Check to see if the event is a request to drop the call associated with this handler.
	// If DropRequestEvent for our call, then it will do the IQ requests to drop the calls,
	// delete the pEvent
	if ( CheckDropRequestEvent(pStateMachine,pEvent,getCurrentStateData().m_callId) )
	{
		Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent dropping call callId=%ld"), getCurrentStateData().m_callId );
		// Go into dropping state
		assignNewState( OnSipXmppStates::PreDropCall, NULL );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	// Check to see if the IQ results for out attempts to drop the call.
	// We would try to drop our calls if we got DropRequestEvent and handled (as above)
	if ( CheckIqReplyDropRequest(pEvent) )
	{
		Logger::log_debug( _T("OnSipIncomingCallStateHandler::IsYourEvent dropcall iqReply") );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	Logger::log_error("OnSipIncomingCallStateHandler::IsYourEvent Unknown Event in OnSipIncomingCallStateHandler %s", pEvent->ToString().c_str() );
	return false;
}

//virtual 
bool OnSipIncomingCallStateHandler::IsStillExist()
{	
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// If we are stuck in an incoming call not being answered for long time, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::Offering, INBOUND_OFFERING_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a connected call for VERY long time, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::Connected, CONNECTED_CALL_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a dropping state, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::PreDropCall, DROP_REQUEST_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	return !IsState( OnSipXmppStates::Dropped );
}

//****************************************************************************
//****************************************************************************
// State Handler for generated outgoing call
//****************************************************************************

OnSipMakeCallStateHandler::OnSipMakeCallStateHandler(const tstring& todial,long callId) : OnSipCallStateHandlerBase(OnSipXmppStates::PreMakeCall, NULL )
{ 
	Logger::log_debug(_T("OnSipMakeCallStateHandler::OnSipMakeCallStateHandler todial=%s callId=%ld"), todial.c_str(), callId );
	getCurrentStateData().m_callType = OnSipXmppCallType::MakeCall;
	getCurrentStateData().m_callId = callId;
	getCurrentStateData().m_remoteId = todial;
	m_contextId  = 0;
	// signify that we take over the PreExecute virtual
	SetHasPreExecute(true);
}

//virtual 
bool OnSipMakeCallStateHandler::PreExecute(OnSipStateMachineBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* /*pStateMachine*/,OnSipXmpp *pOnSipXmpp)
{
	// Reset the threads, the OnSipDropCallStateHandler object
	// may have been created in a different thread via the Async method
	_checkThread.Reset();
	getCurrentStateData()._checkThread.Reset();

	m_contextId = pOnSipXmpp->getUniqueId();
	tstring toDial = getCurrentStateData().m_remoteId.c_str();

	// Tag appended to TO/FROM SIP fields for the call, used to identify 
	tstring customTag = Strings::stringFormat(_T("sync=clk%ld"), (long) clock() );

	pOnSipXmpp->CallNumber( toDial, m_contextId, customTag, &m_toSipField, &m_fromSipField );
	Logger::log_debug(_T("OnSipMakeCallStateHandler::PreExecute dial=%s callId=%ld contextId=%d tag=%s to=%s from=%s"), 
		toDial.c_str(), getCurrentStateData().m_callId, m_contextId, customTag.c_str(), m_toSipField.c_str(), m_fromSipField.c_str() );
	// Set our state
	assignNewState( OnSipXmppStates::MakeCallSet, NULL );
	return true;
}

//virtual 
bool OnSipMakeCallStateHandler::IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Is same contextID
	bool bSameContextId = m_contextId == pEvent->m_context;

	Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent pEvent=%x/%d curState=%s bSameContextId=%d"),pEvent,pEvent->m_type, OnSipXmppStates::CallStateToString(getCurrentState()), bSameContextId );
	Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent inIds=%s outId=%s evtId=%s"), m_branches.ToString().c_str(), m_out_id.c_str(), pEvent->m_id.c_str() );

	// See if an ActiveCallEvent with callstate change
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
		Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent isActiveCallEvent dialogState=%s"),XmppActiveCallEvent::DialogStateToString(ace->m_dialogState) );

		// See if the to-sip or from-sip match exactly the values created in our
		// MakeCall request, which are unique due to custom tag added to the SIP values.
		bool bMatchSips=false;
		if ( ace->m_to_uri == m_toSipField || ace->m_to_uri == m_fromSipField || ace->m_from_uri == m_toSipField || ace->m_from_uri == m_fromSipField )
			bMatchSips=true;

		Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent isActiveCallEvent bMatchSip=%d touri=%s fromuri=%s ourTo=%s ourFrom=%s"),
			bMatchSips, ace->m_to_uri.c_str(), ace->m_from_uri.c_str(), m_toSipField.c_str(), m_fromSipField.c_str() );

		// If not a matching TO or FROM, then this is not our event
		if ( !bMatchSips )
		{
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent notEvent") );
			return false;
		}

		// If an error, then assume dropped call
		if ( ace->IsError() )
		{
			Logger::log_error( _T("OnSipMakeCallStateHandler::IsYourEvent EVENTERROR callId=%ld pEvent=%s"), getCurrentStateData().m_callId, pEvent->ToString().c_str() );

			// TODO, only drop if no more branches
			assignNewState( OnSipXmppStates::Dropped, pEvent );
			return true;
		}

		// Add this call to our list of branches
		// Do not update caller-id information at this point, it will be updated later in states if necessary
		m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType, false  );

		// See if REQUESTED incoming call.
		bool bRequested = ace->m_dialogState == XmppActiveCallEvent::REQUESTED;

		// If the user has multiple SIP phones registered, then we could get
		// multiple of these events, one for each SIP device. 
		if ( m_fromSipField == ace->m_to_uri && bRequested )
		{
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallTrying->MakeCallRequested curState=%s"), OnSipXmppStates::CallStateToString( getCurrentState() ) );

			// If we are in the MakeCallSet state, then change to state to MakeCallRequested
			if ( IsState(OnSipXmppStates::MakeCallSet) )
			{
				assignNewState( OnSipXmppStates::MakeCallRequested, pEvent );
				m_in_sipCallId = ace->m_sipCallid;
				// Update the CallState data, but not caller info
				OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1,false);
				return true;
			}
			// if this occurs, it should be the Requested event of another branch
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallTrying->MakeCallRequested - another branch"));
			// Delete event, we did not keep
			delete pEvent;
			return true;
		}

		// If MakeCallRequested -> MakeCallRequestedAnswered , e.g. see if person answered the inbound call on physical phone
		if ( IsState(OnSipXmppStates::MakeCallRequested) && m_fromSipField == ace->m_to_uri )
		{
			// See if confirmed state
			bool bConfirmed = ace->m_dialogState == XmppActiveCallEvent::CONFIRMED;
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallRequested->MakeCallRequestedAnswered bConfirmed=%d dialogState=%d "),bConfirmed,ace->m_dialogState);
			if ( bConfirmed )
				assignNewState( OnSipXmppStates::MakeCallRequestedAnswered, pEvent );
			// TODO?? Error condtion?
			else
				assignNewState( OnSipXmppStates::Dropped, pEvent );
			// Update the CallState data
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1,false);
			return true;
		}

		// If MakeCallRequestedAnswered -> MakeCallOutgoingCreated , e.g. see if now doing outgoing call
		if ( IsState(OnSipXmppStates::MakeCallRequestedAnswered) && m_toSipField == ace->m_to_uri )
		{
			// See if created state
			bool bCreated = ace->m_dialogState == XmppActiveCallEvent::CREATED;
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallRequestedAnswered->MakeCallOutgoingCreated bCreated=%d dialogState=%d "),bCreated,ace->m_dialogState);
			if ( bCreated )
			{
				assignNewState( OnSipXmppStates::MakeCallOutgoingCreated, pEvent );
				// Update branch with proper caller-id info, it is now valid
				m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType, true   );
			}
			// TODO?? Error condtion?
			else
			{
				assignNewState( OnSipXmppStates::Dropped, pEvent );
			}
			m_out_sipCallId = ace->m_sipCallid;
			m_out_id = pEvent->m_id;
			// Update the CallState data
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1);
			return true;
		}

		// If MakeCallOutgoingCreated->Connected , e.g. outgoing call has been answered, phone call complete
		if ( IsState(OnSipXmppStates::MakeCallOutgoingCreated) && m_toSipField == ace->m_to_uri )
		{
			// See if connected state
			bool bConnected = ace->m_dialogState == XmppActiveCallEvent::CONFIRMED;
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallOutgoingCreated->Connected bConnected=%d dialogState=%d "),bConnected,ace->m_dialogState);
			if ( bConnected )
			{
				assignNewState( OnSipXmppStates::Connected, pEvent );
				// Update the CallState data
				OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1);
				// Update branch with proper caller-id info, it is now valid
				m_branches.AddBranch( ace, getCurrentStateData().m_callId, getCurrentStateData().m_callType, true   );
				return true;
			}
			// TODO?? Error condtion?
			else
				assignNewState( OnSipXmppStates::Dropped, pEvent );
			return true;
		}

		// Unknown state we should handle???
		Logger::log_error(_T("OnSipMakeCallStateHandler::IsYourEvent unhandled state %s and event %s "), OnSipXmppStates::CallStateToString(getCurrentState()), ace->ToString().c_str() );
		// Delete event since not keeping it
		delete pEvent;
		return true;
	}
	
	// See if a retracted (dropped call)
	XmppRetractCallEvent *rce = OnSipCallStateHelper::getRetractCallEvent(pEvent);
	if ( rce != NULL )
	{
		// if initial "incoming" call being disconnected
		if ( m_branches.HasId( pEvent->m_id ) )
		{
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent initial incall dropped. inid=%s out_id=%s"), pEvent->m_id.c_str(), m_out_id.c_str());

			// See if this is the last incoming call being dropped.
			// Multiple calls may occur if user has multiple SIP devices registered.
			OnSipCallStateData& callData = getCurrentStateData();

			// If outbound call has not been created, then initial call has disconnected.  Drop the call
			// If just dropped branch, then our callData will get updated with other valid branch calldata
			if ( m_branches.CheckDroppedCall( &callData, pEvent->m_id ) ) // && m_out_id.empty() )
			{
				Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent initial incall dropped, no outcall"));
				assignNewState( OnSipXmppStates::Dropped, pEvent );
				return true;
			}
			// We did not keep event
			delete pEvent;
			return true;
		}

		// If this is the outbound call, then call is dropped
		if ( m_out_id == pEvent->m_id )
		{
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent outcall dropped"));
			assignNewState( OnSipXmppStates::Dropped, pEvent );
			return true;
		}

		Logger::log_warn(_T("OnSipMakeCallStateHandler::IsYourEvent unknown dropped id=%s"),pEvent->m_id.c_str());
		// Delete event since not keeping it
		delete pEvent;
		return true;
	}

	// Check to see if the event is a request to drop the call associated with this handler.
	// If DropRequestEvent for our call, then it will do the IQ requests to drop the calls,
	// delete the pEvent
	if ( CheckDropRequestEvent(pStateMachine,pEvent,getCurrentStateData().m_callId) )
	{
		Logger::log_debug( _T("OnSipMakeCallStateHandler::IsYourEvent dropping call callId=%ld"), getCurrentStateData().m_callId );
		// Go into dropping state
		assignNewState( OnSipXmppStates::PreDropCall, NULL );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	// Check to see if the IQ results for out attempts to drop the call.
	// We would try to drop our calls if we got DropRequestEvent and handled (as above)
	if ( CheckIqReplyDropRequest(pEvent) )
	{
		Logger::log_debug( _T("OnSipMakeCallStateHandler::IsYourEvent dropcall iqReply") );
		// delete the event, we are not keeping
		delete pEvent;
		return true;
	}

	// See if our IQ result
	XmppIqResultEvent *iqr = OnSipCallStateHelper::getXmppIqResultEvent(pEvent);
	if ( iqr != NULL && bSameContextId )
	{
		Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent IQResult iserr=%d"),iqr->IsError());
		if ( iqr->IsError() )
		{
			Logger::log_error(_T("OnSipMakeCallStateHandler::IsYourEvent IQResult error"));
			assignNewState( OnSipXmppStates::Dropped, pEvent  );
			return true;
		}

		// Leave the state in the MakeCallSet, do not have another state here.
		// The reason is that the IQ result is not guaranteed to come in before the message events start coming in.
		// Signify the call is trying, next state will be incoming call
		Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent PreMakeCall IQ result"));

		// Delete event since not keeping it
		delete pEvent;
		return true;
	}

	return false;
}

//virtual 
bool OnSipMakeCallStateHandler::IsStillExist()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// If we are stuck in the MakeCallSet state, then most likely we did not get any call events for the request
	if ( CheckStateTimeout( OnSipXmppStates::MakeCallSet, MAKECALL_CREATE_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in the MakeCallRequested, then for some reason the person is not
	// answering the inbound call.   Assume they will at least answer within a reasonable time
	if ( CheckStateTimeout( OnSipXmppStates::MakeCallRequested, MAKECALL_INBOUND_REQUEST_TIMEOUT  ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in the MakeCallRequestedAnswered, then for some reason the server 
	// is not responding with the outbound call.
	if ( CheckStateTimeout( OnSipXmppStates::MakeCallRequestedAnswered, MAKECALL_INBOUND_ANSWERED_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in the MakeCallOutgoingCreated, then allow reasonable time for the person to answer on the other end.
	// If no answer, then assume some type of server error.  Does not really have any effect on the physical call
	if ( CheckStateTimeout( OnSipXmppStates::MakeCallOutgoingCreated, OUTBOUND_REQUEST_TIMEOUT  ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a connected call for VERY long time, then drop the call.
	// No negative effects, will not affect the actual call
	if ( CheckStateTimeout( OnSipXmppStates::Connected, CONNECTED_CALL_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	// If we are stuck in a dropping state, then drop the call.
	// No negative effects, will not affect the actual call
	if ( OnSipCallStateHandlerBase::CheckStateTimeout( OnSipXmppStates::PreDropCall, DROP_REQUEST_TIMEOUT ) )
		return true;		// Do not report that call does not exist yet, let it be handled on the next poll check

	return !IsState( OnSipXmppStates::Dropped );
}

//****************************************************************************
//****************************************************************************

//virtual 
StateHandler<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData> *OnSipCallStateMachine::UnknownEvent(XmppEvent* pEvent)
{
	// See if Active Call Event
	XmppActiveCallEvent* ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
		Logger::log_debug(_T("OnSipCallStateMachine::UnknownEvent activeCallEvent dialogState=%d"),ace->m_dialogState);

		// See if User outgoing call using physical phone device
		if ( ace->m_dialogState == XmppActiveCallEvent::CREATED )
			return new OnSipOutgoingCallStateHandler(pEvent,m_pOnSipXmpp->getUniqueId());

		// See if incoming call
		if ( ace->m_dialogState == XmppActiveCallEvent::REQUESTED )
			return new OnSipIncomingCallStateHandler(pEvent,m_pOnSipXmpp->getUniqueId());
	}
	Logger::log_warn(_T("OnSipCallStateMachine::UnknownEvent UNKNOWN %s"),pEvent->ToString().c_str());
	return NULL;
}

// Virtual notify that either the state has changed or the state data has changed
//virtual 
void OnSipCallStateMachine::OnStateChange(OnSipXmppStates::CallStates callState,OnSipCallStateData& stateData,StateChangeReason::eOnStateChangeReason reason)
{
	Logger::log_debug(_T("OnSipCallStateMachine::OnStateChange callState=%d/%s stateData=%s reason=%d"),
		callState, OnSipXmppStates::CallStateToString(callState), stateData.ToString().c_str(), reason );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Pass on to parent so external IStateNotify instances will be done
	OnSipStateMachineBase::OnStateChange(callState,stateData,reason);
}

// NOT thread-safe
//
// Drop a phone call for the specified call data
//   returns the unique IQ contextId in the return parameter
bool OnSipCallStateMachine::DropCall( OnSipCallStateData& callData, long* contextId  )
{
	Logger::log_debug( _T("OnSipCallStateMachine::DropCall callId=%ld"), callData.m_callId );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Create unique context id, return to caller
	*contextId = m_pOnSipXmpp->getUniqueId();
	m_pOnSipXmpp->DropCall( callData.m_sipCallId, callData.m_fromTag, callData.m_toTag, *contextId );
	return true;
}

// THREAD-SAFE
//
// Make a phone call.  This will be done asynchrously.
// The request will be inserted into the state machine for handling.
// Returns the unique callid that refers to the unique ID for this call
long OnSipCallStateMachine::MakeCallAsync( const tstring& phoneNumber )
{
	// Generate new unique ID
	long callId = m_pOnSipXmpp->getUniqueId();
	Logger::log_debug( _T("OnSipCallStateMachine::MakeCallAsync %s callId=%ld"), phoneNumber.c_str(), callId );
	AddStateHandler( new OnSipMakeCallStateHandler(phoneNumber, callId ) );
	return callId;
}

// THREAD-SAFE
//
// Drop a phone call.  This will be done asynchronously
//  callId = unique ID for this call
void OnSipCallStateMachine::DropCallAsync( long callId )
{
	Logger::log_debug( _T("OnSipCallStateMachine::DropCallAsync callId=%ld"), callId );
	// Add event asynchrnonously to be sent to all state handlers for 
	// the one handling the specified call to drop it
	AddEvent( new DropRequestEvent(callId) );
}
