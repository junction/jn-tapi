#include <stdafx.h>
#include "OnSipInitStateMachine.h"
#include "OnSipXmpp.h"
#include "OnSipWorkers.h"

//static 
TCHAR* OnSipInitStates::InitStatesToString(InitStates state)
{
	switch (state)
	{
		case NotSet:
			return _T("NotSet");
		case PreLogin:
			return _T("PreLogin");
		case LoginError:
			return _T("LoginError");
		case Authorizing:
			return _T("Authorizing");
		case Authorized:
			return _T("Authorized");
		case AuthorizedError:
			return _T("AuthorizedError");
		case EnablingCallEvents:
			return _T("EnablingCallEvents");
		case ShuttingDown:
			return _T("ShuttingDown");
		case ShutDown:
			return _T("ShutDown");
		case OK:
			return _T("OK");
//		case ReSubscribe:
//			return _T("ReSubscribe");
		case EnabledCallError:
			return _T("EnabledCallError");
		case Disconnected:
			return _T("Disconnected");
		default:
			Logger::log_error( Strings::stringFormat( _T("OnSipInitStates::InitStatesToString unrecognized state=%d"), state).c_str() );
			return _T("OnSipInitStates::InitStatesToString unrecognized state");
	}
}

//static 
TCHAR* OnSipInitStatesType::InitStatesTypeToString(OnSipInitStatesType::InitStatesType state)
{
	switch ( state )
	{
		case OnSipInitStatesType::NOTSET:
			return _T("NOTSET");
		case OnSipInitStatesType::DISCONNECTED:
			return _T("DISCONNTECTED");
		case OnSipInitStatesType::FATAL:
			return _T("FATAL");
		case OnSipInitStatesType::OK:
			return _T("OK");
		case OnSipInitStatesType::INPROGRESS:
			return _T("INPROGRESS");
		case OnSipInitStatesType::SHUTDOWN:
			return _T("SHUTDOWN");
		default:
			Logger::log_error( _T("UNKNOWN InitStatesType %d"), state );
			return _T("UNKNOWN InitStatesType");
	}
}

//static 
// Maps InitStates state enum to a category/InitStatesType
OnSipInitStatesType::InitStatesType OnSipInitStatesType::GetInitStatesType(OnSipInitStates::InitStates initState)
{
	switch (initState)
	{
		case OnSipInitStates::NotSet:
			return OnSipInitStatesType::NOTSET;

		case OnSipInitStates::PreLogin:
		case OnSipInitStates::Authorizing:
		case OnSipInitStates::EnablingCallEvents:
		case OnSipInitStates::Authorized:
			return OnSipInitStatesType::INPROGRESS;

		case OnSipInitStates::LoginError:
		case OnSipInitStates::AuthorizedError:
		case OnSipInitStates::EnabledCallError:
			return OnSipInitStatesType::FATAL;

		case OnSipInitStates::ShuttingDown:
		case OnSipInitStates::OK:
			return OnSipInitStatesType::OK;

		case OnSipInitStates::Disconnected:
			return OnSipInitStatesType::DISCONNECTED;

		case OnSipInitStates::ShutDown:
			return OnSipInitStatesType::SHUTDOWN;

		// Shouldn't occur!!
		default:
			Logger::log_error( Strings::stringFormat( _T("OnSipInitStatesType::GetInitStatesType unrecognized state=%d"), initState).c_str() );
			return OnSipInitStatesType::FATAL;
	}
}

//***************************************************************************
//***************************************************************************

#define GENERAL_COMMUNICATION_TIMEOUT		( 15 * MSECS_IN_SEC )

//***************************************************************************
//***************************************************************************


OnSipInitStateHandler::OnSipInitStateHandler(OnSipXmpp* pOnSipXmpp) 
		: OnSipStateHandlerBase<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData>( OnSipInitStates::PreLogin, NULL ),
		m_authTO( SUBSCRIBE_TIMEOUT ), m_ping(PINGTIMEOUT)
{
	Logger::log_debug("OnSipInitStateHandler::OnSipInitStateHandler this=%x onSipXmpp=%x", this, pOnSipXmpp );
	m_pOnSipXmpp = pOnSipXmpp;
	_contextId = 0;
	m_pOnSipXmpp = pOnSipXmpp;
	m_bEnabledCallEvents = false;
	m_unsubscribe_contextId = 0;
}

//virtual 
bool OnSipInitStateHandler::IsYourEvent(StateMachine<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData>* /*pStateMachine*/,XmppEvent *pEvent)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	Logger::log_debug("OnSipInitStateHandler::IsYourEvent pEvent=%s curState=%d/%s", pEvent->ToString().c_str(), getCurrentState(), OnSipInitStates::InitStatesToString(getCurrentState()) );

	// See which type of event
	XmppOnConnectEvent* evConnect = OnSipCallStateHelper::getOnConnectEvent(pEvent);
	XmppOnDisconnectEvent* evDisconnect = OnSipCallStateHelper::getOnDisconnectEvent(pEvent);
	XmppAuthEvent* evAuthEvent = OnSipCallStateHelper::getAuthEvent(pEvent);
	XmppPubSubSubscribedEvent* evPubSubSubscribed = OnSipCallStateHelper::getPubSubSubscribedEvent(pEvent);
	XmppIqResultEvent* evIqResult = OnSipCallStateHelper::getXmppIqResultEvent(pEvent);
	ShutdownRequestEvent* evShutdown = OnSipCallStateHelper::getShutdownRequestEvent(pEvent);

	Logger::log_debug("OnSipInitStateHandler::IsYourEvent cur=%d ctx=%d evCtx=%d evConnect=%p evDisconnect=%p evAuth=%p evPubSub=%p evIqRes=%p", getCurrentState(), _contextId, pEvent->m_context, evConnect, evDisconnect, evAuthEvent, evPubSubSubscribed, evIqResult );

	// If logging in
	if ( IsState(OnSipInitStates::PreLogin) && (evConnect != NULL || evDisconnect != NULL) )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::IsYourEvent prelogon err=%d"), pEvent->IsError() );
		// If success
		if ( evConnect != NULL && !pEvent->IsError() )
		{
			// Get next unique ID for Authorize event
			_contextId = m_pOnSipXmpp->getUniqueId();
			// Start the authorize event and set current state
			m_pOnSipXmpp->Authorize( _contextId );
			assignNewState( OnSipInitStates::Authorizing, NULL );
		}
		// If error
		else
		{
			// TODO?? Get reason from Disconnect??
			Logger::log_error(_T("OnSipInitStateHandler::IsYourEvent prelogon error. evCon=%p evDCon=%p\r\n%s"), evConnect, evDisconnect, pEvent->ToString().c_str() );
			assignNewState( OnSipInitStates::LoginError, NULL );
		}

		// Delete the event since not keeping
		delete pEvent;
		return true;
	}

	// If authorizing
	if ( IsState(OnSipInitStates::Authorizing) && ((evIqResult!= NULL && pEvent->m_context == _contextId) || evAuthEvent != NULL) )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::IsYourEvent curState=%s authorizing err=%d"),  OnSipInitStates::InitStatesToString(getCurrentState()), pEvent->IsError() );
		// If success
		if ( !pEvent->IsError() )
		{
			assignNewState( OnSipInitStates::EnablingCallEvents, NULL );
			// Enable the call events, save the XMPP id value
			m_enableCallEventsId = m_pOnSipXmpp->SubscribeCallEvents( evAuthEvent->expireDate );
		}
		// If error
		else
		{
			Logger::log_error(_T("OnSipInitStateHandler::IsYourEvent authorizing error=%d"), pEvent->ToString().c_str() );
			assignNewState( OnSipInitStates::AuthorizedError, NULL );
		}
		// Delete the event since not keeping
		delete pEvent;
		return true;
	}

	//	TODO: Should possibly check subscriptionResult for enabling call events.  The id == m_enableCallEventsId for iq subscription result.
	//  Currently the pubsub does not get sent as an event, it is caught by the resultHandler::handleSubscriptionResult.
	//  possible this could have an error, need to check for pending!!
	//  e.g.
	//	<iq from="pubsub.active-calls.xmpp.onsip.com" to="username!domain.onsip.com@dashboard.onsip.com/739776732124026405487880" id="uid4" type="result">
	//	<pubsub xmlns="http://jabber.org/protocol/pubsub">
    //    <subscription node="/me/username!domain.onsip.com@dashboard.onsip.com" jid="username!domain.onsip.com@dashboard.onsip.com" subscription="pending" />
    //	</pubsub>
	//  </iq>

	// If enabling call events
	if ( IsState(OnSipInitStates::EnablingCallEvents) && evPubSubSubscribed != NULL )
	{
		Logger::log_debug("OnSipInitStateHandler::IsYourEvent EnablingCallEvents subid=%s err=%d", evPubSubSubscribed->m_subid.c_str(), pEvent->IsError() );
		// If success
		if ( !pEvent->IsError() )
		{
			assignNewState( OnSipInitStates::OK, NULL );
			// Set the timeout for re-subscribe
			m_authTO.SetMsecs( SUBSCRIBE_TIMEOUT );
			m_bEnabledCallEvents = true;
			// Keep track of the subscribed subid, will need for unsubscribe
			m_subscribed_subid = evPubSubSubscribed->m_subid;
		}
		// If error
		else
		{
			// Set our state, and delete event since not keeping it
			assignNewState( OnSipInitStates::EnabledCallError, NULL );
			m_bEnabledCallEvents = false;
		}
		// Delete event since not keeping it
		delete pEvent;
		return true;
	}

	// If Disconnecting
	if ( evDisconnect != NULL )
	{
		Logger::log_debug("OnSipInitStateHandler::IsYourEvent disconnected");
		// TODO?? Try to re-connect ???  re-authorize, re-enable call events
		// Set our state, and delete event since not keeping it
		assignNewState( OnSipInitStates::Disconnected, NULL );
		delete pEvent;
		return true;
	}

	// If shutdown request
	if ( evShutdown != NULL )
	{
		Logger::log_debug("OnSipInitStateHandler::IsYourEvent shutdown subid=%s", m_subscribed_subid.c_str() );

		// TODO: Need to ask server for all subscribes and do all

		// Do unsubscribe request
		m_unsubscribe_contextId = m_pOnSipXmpp->UnsubscribeCallEvents(m_subscribed_subid);
		// Set our state, and delete event since not keeping it
		assignNewState( OnSipInitStates::ShuttingDown, NULL );
		delete pEvent;
		return true;
	}

	// If shutting down and waiting for the unsubscribe IQ result
	if ( IsState(OnSipInitStates::ShuttingDown) && evIqResult != NULL &&  evIqResult->m_context == m_unsubscribe_contextId )
	{
		Logger::log_debug("OnSipInitStateHandler::IsYourEvent shutdown result err=%d", pEvent->IsError() );
		// Not checking error, not really much we can do
		// Set our state, and delete event since not keeping it
		assignNewState( OnSipInitStates::ShutDown, NULL );
		delete pEvent;
		return true;
	}

	Logger::log_debug("OnSipInitStateHandler::IsYourEvent unhandled=%x/%d",pEvent, pEvent->m_type );
	return false;
}

//virtual 
bool OnSipInitStateHandler::IsStillExist()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	if ( IsState( OnSipInitStates::ShutDown ) )
	{
		Logger::log_debug("OnSipInitStateHandler::IsStillExist shutdown" );
		_reauthorizer.reset(NULL);
		return false;
	}
	return true;
}

//Periodically called, return back true if an operation was 
//done that required update to state notification
//virtual 
bool OnSipInitStateHandler::PollStateHandler()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// TODO: Need to check for possible stuck state where we did not get
	// a response, e.g. sent of an IQ but never got a responses.
	// If stuck in a state too long, then some type of error??

	// If time to re-authorize
	if ( IsState(OnSipInitStates::OK) && m_authTO.IsExpired() )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::PollStateHandler authTimeout %ld curState=%d/%s"),m_authTO.Msecs(),getCurrentState(),OnSipInitStates::InitStatesToString(getCurrentState()) );
		// handle the re-authorization,
		// we continue to enter this function while expired in order to drive the state machine.
		// Will return false if some type of error.  If will handle setting the disconnect state, etc.
		if ( !_handleReAuthorize() )
			return true;
	}

	// If time to ping server to keep alive
	if ( IsState(OnSipInitStates::OK) && m_ping.IsExpired() )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::PollStateHandler pingTimeout %ld "),m_ping.Msecs() );

		// Do the ping
		m_pOnSipXmpp->Ping();
		m_ping.Reset();
	}

	// Check for timeout errors, stuck in a state due to no responses from server
	if ( CheckStateTimeout( OnSipInitStates::PreLogin, GENERAL_COMMUNICATION_TIMEOUT ) )
		return true;

	if ( CheckStateTimeout( OnSipInitStates::Authorizing, GENERAL_COMMUNICATION_TIMEOUT ) )
		return true;

	if ( CheckStateTimeout( OnSipInitStates::EnablingCallEvents, GENERAL_COMMUNICATION_TIMEOUT ) )
		return true;

	if ( CheckStateTimeout( OnSipInitStates::ShuttingDown, GENERAL_COMMUNICATION_TIMEOUT ) )
		return true;

	return false;
}

// Handle the re-authorization, resets the authorization time when complete.
// Continue to call this function while auth timed out in order to drive its state machine.
// Will return false if some type of error.  If will handle setting the disconnect state, etc.
bool OnSipInitStateHandler::_handleReAuthorize()
{
	// If currently not re-authorizing, then start it
	if ( _reauthorizer.get() == NULL )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::_checkReAuthorize start reauthorize"));
		_reauthorizer.reset( new ReAuthorizeSubscribeWorker( m_pOnSipXmpp ) );
		_reauthorizer->Execute(); // m_subscribed_subid );
	}
	// Are we complete reauthorizing
	else if ( _reauthorizer->IsComplete() )
	{
		Logger::log_debug(_T("OnSipInitStateHandler::_checkReAuthorize reauthorize complete. err=%d"),_reauthorizer->IsError());
		// If error
		if ( _reauthorizer->IsError() )
		{
			Logger::log_error(_T("OnSipInitStateHandler::_checkReAuthorize authorizeError"));
			m_authTO.SetMsecs( SUBSCRIBE_QUICK_RETRY );
		}
		// If success
		else
		{
			// Keep track of the subscribed subid, will need for unsubscribe
			m_subscribed_subid  = _reauthorizer->subId();
			m_authTO.SetMsecs( SUBSCRIBE_TIMEOUT );
			Logger::log_debug(_T("OnSipInitStateHandler::_checkReAuthorize authorizeComplete subid=%s"), m_subscribed_subid.c_str());
		}
		// Delete our reauthorizer
		_reauthorizer.reset(NULL);
	}
	// If re-authorize exceeds wait time.  Give it extra time since it is doing multiple steps
	else if ( _reauthorizer->Msecs() > GENERAL_COMMUNICATION_TIMEOUT*3 )
	{
		Logger::log_error(_T("OnSipInitStateHandler::_checkReAuthorize TIMEOUT reauthorize, msecs=%ld.  Disconnecting"), _reauthorizer->Msecs() );
		assignNewState( OnSipInitStates::Disconnected, NULL );
		// Delete our reauthorizer
		_reauthorizer.reset(NULL);
		// return false, signify error
		return false;
	}
	return true;
}

// Checks to see if the state has been in the specified state for the specified timeout in msecs.
// If so, then the call will be put in the Error state and return true.
bool OnSipInitStateHandler::CheckStateTimeout( OnSipInitStates::InitStates initState, long timeout )
{
	// If in specified state past timeout
	if ( IsState( initState ) && MsecsSinceLastStateChange() > timeout  )
	{
		_ASSERTE( !IsState(OnSipInitStates::Disconnected) );
		Logger::log_error(_T("OnSipInitStateHandler::CheckStateTimeout %s TIMEOUT msecs=%ld.  Disconnecting"), OnSipInitStates::InitStatesToString(initState), MsecsSinceLastStateChange() );
		assignNewState( OnSipInitStates::Disconnected, NULL );
		return true;
	}
	return false;
}

//***************************************************************************
//***************************************************************************

//virtual 
StateHandler<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData>* OnSipInitStateMachine::UnknownEvent(XmppEvent* pEvent)
{
	Logger::log_debug("OnSipInitStateMachine ::UnknownEvent" );
	return NULL;
}

// Virtual notify that either the state has changed or the state data has changed
//virtual 
void OnSipInitStateMachine::OnStateChange(OnSipInitStates::InitStates state,OnSipInitStateData& stateData,StateChangeReason::eOnStateChangeReason reason)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	Logger::log_debug( _T("OnSipInitStateMachine::OnStateChange state=%s reason=%d"),OnSipInitStates::InitStatesToString(state), reason );

	// Pass on to parent so external IStateNotify instances will be done
	OnSipStateMachineBase::OnStateChange(state,stateData,reason);
}

