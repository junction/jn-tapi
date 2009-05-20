#include <stdafx.h>

#include "onsipcallstatemachine.h"
#include "logger.h"
#include "onsipxmpp.h"

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
void OnSipCallStateHelper::AssignCallStateData(OnSipCallStateData& callStateData,XmppActiveCallEvent* ace,long callId,bool bUpdateCallInfo)
{
	_ASSERT( ace != NULL );
	if ( ace == NULL )
		return;
	// TODO: extract out call information and put in OnSipCallStateData
	if ( callId != -1 )
		callStateData.m_callId = callId;
	callStateData.m_sipCallId = ace->m_sipCallid;		// SIP callId
	// TODO?? Are these correct?  
	if ( bUpdateCallInfo )
	{
		callStateData.m_remoteId = ace->m_to_aor;
		//	callStateData.m_calledId = ace->m_uas_aor;
	}
	// Assign from/to tags
	callStateData.m_fromTag = ace->m_from_tag;
	callStateData.m_toTag = ace->m_to_tag;
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
		case Proceeding:
			return _T("CallState::Proceeding");
		case Connected:
			return _T("CallState::Connected");
		case Dropped:
			return _T("CallState::Dropped");
		case PreMakeCall:
			return _T("CallState::PreMakeCall");
//		case Dialing:
//			return _T("CallState::Dialing");
		case MakeCallSet:
			return _T("CallState::MakeCallSet");
		case MakeCallRequested:
			return _T("CallState::MakeCallRequested");
		case MakeCallRequestedAnswered:
			return _T("CallState::MakeCallRequestedAnswered");
		case MakeCallOutgoingCreated:
			return _T("CallState::MakeCallOutgoingCreated");
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
	return Strings::stringFormat(_T("<OnSipCallStateData callType=%s callId=%ld remote=%s calledId=%s sipCallId=%s>"),
		OnSipXmppCallType::CallTypeToString(m_callType), m_callId, m_remoteId.c_str(), m_calledId.c_str(), m_sipCallId.c_str() );
}

//****************************************************************************
//****************************************************************************
// State Handler for user created manual out-going phone calls
//****************************************************************************

OnSipOutgoingCallStateHandler::OnSipOutgoingCallStateHandler(XmppEvent* pEvent,long callId) : OnSipCallStateHandlerBase( OnSipXmppStates::Proceeding, pEvent )
{ 
	Logger::log_debug("OnSipOutgoingCallStateHandler::OnSipOutgoingCallStateHandler %s callId=%ld", pEvent->ToString().c_str(), callId );
	getCurrentStateData().m_callType = OnSipXmppCallType::PhysicalCall;
	// Set call information from the XmppActiveCallEvent
	OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),OnSipCallStateHelper::getActiveCallEvent(pEvent),callId);
}

//virtual 
bool OnSipOutgoingCallStateHandler::IsYourEvent(XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// See if this event refers to our call
	if ( !OnSipCallStateHelper::IsSameId( getCurrentEvent(), pEvent ) )
		return false;

	// See if an ActiveCallEvent with callstate change
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
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
		return false;
	}

	// See if a retracted (dropped call)
	XmppRetractCallEvent *rce = OnSipCallStateHelper::getRetractCallEvent(pEvent);
	if ( rce != NULL )
	{
		Logger::log_debug("OnSipOutgoingCallStateHandler::IsYourEvent dropped call");
		assignNewState( OnSipXmppStates::Dropped, pEvent );
		return true;
	}

	Logger::log_error("OnSipOutgoingCallStateHandler::IsYourEvent Unknown Event in OnSipOutgoingCallStateHandler %s", pEvent->ToString().c_str() );
	return false;
}

//virtual 
bool OnSipOutgoingCallStateHandler::IsStillExist()
{	
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
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
	OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),OnSipCallStateHelper::getActiveCallEvent(pEvent),callId);
}

//virtual 
bool OnSipIncomingCallStateHandler::IsYourEvent(XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// See if this event refers to our call
	if ( !OnSipCallStateHelper::IsSameId( getCurrentEvent(), pEvent ) )
		return false;

	// See if an ActiveCallEvent with callstate change
	XmppActiveCallEvent *ace = OnSipCallStateHelper::getActiveCallEvent(pEvent);
	if ( ace != NULL )
	{
		// If connected call
		if ( ace->m_dialogState == XmppActiveCallEvent::CONFIRMED )
		{
			Logger::log_debug("OnSipIncomingCallStateHandler::IsYourEvent connected");
			assignNewState( OnSipXmppStates::Connected, pEvent );
			// Update our callData
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1);
			return true;
		}
		Logger::log_error("OnSipIncomingCallStateHandler::IsYourEvent Unknown State in OnSipIncomingCallStateHandler %s", pEvent->ToString().c_str() );
		return false;
	}

	// See if a retracted (dropped call)
	XmppRetractCallEvent *rce = OnSipCallStateHelper::getRetractCallEvent(pEvent);
	if ( rce != NULL )
	{
		Logger::log_debug("OnSipIncomingCallStateHandler::IsYourEvent dropped call");
		assignNewState( OnSipXmppStates::Dropped, pEvent );
		return true;
	}

	Logger::log_error("OnSipIncomingCallStateHandler::IsYourEvent Unknown Event in OnSipIncomingCallStateHandler %s", pEvent->ToString().c_str() );
	return false;
}

//virtual 
bool OnSipIncomingCallStateHandler::IsStillExist()
{	
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return !IsState( OnSipXmppStates::Dropped );
}

//****************************************************************************
//****************************************************************************
// State Handler for generated outgoing call
//****************************************************************************

OnSipMakeCallStateHandler::OnSipMakeCallStateHandler(const tstring& todial,long callId) : OnSipCallStateHandlerBasePreExecute(OnSipXmppStates::PreMakeCall, NULL )
{ 
	Logger::log_debug(_T("OnSipMakeCallStateHandler::OnSipMakeCallStateHandler todial=%s callId=%ld"), todial.c_str(), callId );
	getCurrentStateData().m_callType = OnSipXmppCallType::MakeCall;
	getCurrentStateData().m_callId = callId;
	getCurrentStateData().m_remoteId = todial;
	m_contextId  = 0;
}

//virtual 
bool OnSipMakeCallStateHandler::PreExecute(OnSipStateMachineBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* /*pStateMachine*/,OnSipXmpp *pOnSipXmpp)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

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
bool OnSipMakeCallStateHandler::IsYourEvent(XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Is same contextID
	bool bSameContextId = m_contextId == pEvent->m_context;

	Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent pEvent=%x/%d curState=%s bSameContextId=%d"),pEvent,pEvent->m_type, OnSipXmppStates::CallStateToString(getCurrentState()), bSameContextId );
	Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent inId=%s outId=%s evtId=%s"), m_in_id.c_str(), m_out_id.c_str(), pEvent->m_id.c_str() );

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

		// If MakeCallTrying->MakeCallRequested,  e.g. physical phone inbound call ringing to begin the call sequence
		if ( IsState(OnSipXmppStates::MakeCallSet) && m_fromSipField == ace->m_to_uri )
		{
			// Make sure requested state!
			bool bRequested = ace->m_dialogState == XmppActiveCallEvent::REQUESTED;
			Logger::log_debug(_T("OnSipMakeCallStateHandler::IsYourEvent MakeCallTrying->MakeCallRequested bRequested=%d dialogState=%d "),bRequested,ace->m_dialogState);
			if ( bRequested )
				assignNewState( OnSipXmppStates::MakeCallRequested, pEvent );
			// TODO?? Error condition?? Can this occur
			else
				assignNewState( OnSipXmppStates::Dropped, pEvent );
			m_in_sipCallId = ace->m_sipCallid;
			m_in_id = ace->m_id;
			// Update the CallState data, but not caller info
			OnSipCallStateHelper::AssignCallStateData(getCurrentStateData(),ace,-1,false);
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
				assignNewState( OnSipXmppStates::MakeCallOutgoingCreated, pEvent );
			// TODO?? Error condtion?
			else
				assignNewState( OnSipXmppStates::Dropped, pEvent );
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
		if ( m_in_id == pEvent->m_id ) 
		{
			Logger::log_debug("OnSipMakeCallStateHandler::IsYourEvent initial incall dropped. out_id=%s",m_out_id.c_str());
			// If outbound call has not been created, then initial call has disconnected.  Drop the call
			if ( m_out_id.empty() )
			{
				assignNewState( OnSipXmppStates::Dropped, pEvent );
			}
			else
			{
				// We have outbound call still, so no change in state.
				// Clear out the inbound id, no longer need
				m_in_id.clear();
			}
			return true;
		}

		// If this is the outbound call, then call is dropped
		if ( m_out_id == pEvent->m_id )
		{
			Logger::log_debug("OnSipMakeCallStateHandler::IsYourEvent outcall dropped");
			assignNewState( OnSipXmppStates::Dropped, pEvent );
			return true;
		}

		Logger::log_warn("OnSipMakeCallStateHandler::IsYourEvent unknown dropped id=%s",pEvent->m_id.c_str());
		// Delete event since not keeping it
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

		// TODO!! Need to have a timeout in this state

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
	return !IsState( OnSipXmppStates::Dropped );
}

//****************************************************************************
//****************************************************************************
// State Handler for generated outgoing call
//****************************************************************************

OnSipDropCallStateHandler::OnSipDropCallStateHandler(long callId) : OnSipCallStateHandlerBasePreExecute(OnSipXmppStates::PreDropCall, NULL )
{ 
	Logger::log_debug(_T("OnSipDropCallHandler::OnSipDropCallHandler callId=%ld"), callId );
	m_callId = callId;
}

//virtual 
bool OnSipDropCallStateHandler::PreExecute(OnSipStateMachineBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,OnSipXmpp *pOnSipXmpp)
{
	Logger::log_debug(_T("OnSipDropCallHandler::OnSipDropCallHandler callId=%ld"), m_callId );

	OnSipCallStateMachine* pCallStateMachine = dynamic_cast<OnSipCallStateMachine*>(pStateMachine);

	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Get unique context ID for the drop request
	long contextId = pOnSipXmpp->getUniqueId();
	pCallStateMachine->DropCall( m_callId, contextId );
	return true;
}

//virtual 
bool OnSipDropCallStateHandler::IsYourEvent(XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	_ASSERT(false);
	// IsYourEvent is not used, we do not track the drop call status.
	// Assume this to be handled by the other call state handler that is
	// tracking the active call.
	//
	// We don't have this handler active due to we would have 2 state handlers
	// tracking the same call.
	//
	// We avoid the handler being added to the StateMachein due to IsStillExist() returns false
	return false;
}

//virtual 
bool OnSipDropCallStateHandler::IsStillExist()
{	
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return false;
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

// Find the specified call in the state machine.
// Return its current calsltate and callstatedata.
// Returns true if found
bool OnSipCallStateMachine::FindCallState(long callId,OnSipXmppStates::CallStates* pCallState,OnSipCallStateData* pCallStateData)
{
	Logger::log_debug(_T("OnSipCallStateMachine::FindCallState callId=%ld"), callId );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Get all states from the state machine
	std::list< StateAndStateData<OnSipXmppStates::CallStates,OnSipCallStateData>> ret = GetAllStates();
	// Iterate through them looking for matching callid
	std::list< StateAndStateData<OnSipXmppStates::CallStates,OnSipCallStateData>>::iterator iter = ret.begin();
	while ( iter != ret.end() )
	{
		// If matching unique callId found
		if ( iter->m_stateData.m_callId == callId )
		{
			*pCallState = iter->m_state;
			*pCallStateData = iter->m_stateData;
			Logger::log_debug(_T("OnSipCallStateMachine::FindCallState callId=%ld found state=%s stateData=%s"), 
				callId, OnSipXmppStates::CallStateToString( *pCallState ), pCallStateData->ToString().c_str() );
			return true;
		}
		iter++;
	}
	Logger::log_warn(_T("OnSipCallStateMachine::FindCallState callId=%ld notFound"), callId );
	return false;
}

// NOT thread-safe
//
// Drop a phone call.  
//  callId = unique ID for this call
//  unique contextId for the IQ request, it will be in the IQ reply to match the request
bool OnSipCallStateMachine::DropCall( long callId, int contextId )
{
	Logger::log_debug( _T("OnSipCallStateMachine::DropCall callId=%ld"), callId );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// See if we can find this call in the state machine
	OnSipXmppStates::CallStates callState;
	OnSipCallStateData callData;
	if ( !FindCallState(callId,&callState,&callData) )
	{
		Logger::log_error( _T("OnSipCallStateMachine::DropCall callId=%ld call notFound"), callId );
		return false;
	}
	m_pOnSipXmpp->DropCall( callData.m_sipCallId, callData.m_fromTag, callData.m_toTag, contextId );
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
	// Create Drop CallStateHandler that will do the drop in its PreExecute() method
	AddStateHandler( new OnSipDropCallStateHandler(callId ) );
}
