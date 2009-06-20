#include <stdafx.h>
#include "OnSip.h"
#include "OnSipXmpp.h"
#include "OnSipWorkers.h"
#include "OnSipXmppEvents.h"
#include "OnSipCallStateMachine.h"
#include "message.h"

/*

// Execute the Unsubscriber worker
void UnsubscriberAllWorker::Execute()
{
	_count = 0;
	Logger::log_debug("UnsubscriberAllWorker::Execute");
	_onSipXmpp->getSubscriptions(this);
}

//virtual 
void UnsubscriberAllWorker::handleSubscriptions( const std::string& id,
										const JID& service,
										const PubSub::SubscriptionMap& subMap,
										const Error* error)
{ 
	Logger::log_debug("UnsubscriberAllWorker::handleSubscriptions err=%p",error);  

	//typedef std::map< std::string, SubscriptionInfo > SubscriptionMap;
	for ( PubSub::SubscriptionMap::const_iterator iter = subMap.begin(); iter != subMap.end(); iter++ )
	{
		string node = iter->first;
		// Get the list of subscriptions for this node
		const std::list<PubSub::SubscriptionInfo>& lst = iter->second;
		Logger::log_debug("UnsubscriberAllWorker::handleSubscriptions node=%s cnt=%d", node.c_str(), lst.size() );

		// Iterate through each subscription and unsubsribe
		for ( std::list<PubSub::SubscriptionInfo>::const_iterator it2 = lst.begin(); it2 != lst.end(); it2++ )
		{
			_count++;
			_onSipXmpp->UnsubscribeCallEvents( node, it2->subid, this, this );
		}
	}
	_bSent = true;

	Logger::log_debug("UnsubscriberAllWorker::handleSubscriptions allsent=%d",_count);
}

//virtual
void UnsubscriberAllWorker::handleIqID( const IQ& iq, int context )
{
	_count--;
	Logger::log_debug("UnsubscriberAllWorker::handleIqID context=%d cnt=%d", context, _count );  
}

// Are we complete
//virtual 
bool UnsubscriberAllWorker::IsComplete()
{
	bool bComplete = _bSent && _count == 0;
	Logger::log_debug("UnsubscriberAllWorker::IsComplete bSent=%d _count=%d bComplete=%d", _bSent, _count, bComplete );
	return bComplete;
}

*/

//*********************************************************************
//*********************************************************************

/*

// Execute the Unsubscriber worker
void UnsubscriberWorker::Execute(string subId)
{
	Logger::log_debug("UnsubscriberWorker::Execute subId=%s", subId.c_str() );
	_onSipXmpp->UnsubscribeCallEvents(subId,NULL,this);
}

//virtual 
void UnsubscriberWorker::handleIqID( const IQ& iq, int context )
{
	Logger::log_debug("UnsubscriberWorker::handleIqID" );
	_bError = iq.error() != NULL;
	_bComplete = true;
}
*/

//*********************************************************************
//*********************************************************************

void AuthorizeWorker::Execute()
{
	long contextId = _onSipXmpp->getUniqueId();
	Logger::log_debug("AuthorizeWorker::Execute context=%d", contextId );
	_onSipXmpp->Authorize(contextId,this);
}

//virtual 
void AuthorizeWorker::handleIqID( const IQ& iq, int context )
{
	Logger::log_debug("AuthorizeWorker::handleIqID context=%d err=%p", context, iq.error() );
	_bComplete = true; 

	XmppEvent* evt = XmppEventFactory::iqFactory( iq, context );
	XmppAuthEvent* evAuthEvent = OnSipCallStateHelper::getAuthEvent(evt);
	if ( evAuthEvent == NULL || iq.error() != NULL )
	{
		Logger::log_error("AuthorizeWorker::handleIqID context=%d evt=%p err=%p", context, evt, iq.error() );
		_bError = true;
	}
	else
	{
		_expireDate = evAuthEvent->expireDate;
	}
}

//*********************************************************************
//*********************************************************************

void EnableCallEventsWorker::Execute(const string& expireTime)
{
	Logger::log_debug("AuthorizeWorker::Execute expireTime=%s", expireTime.c_str() );

	// Register the message handler
	_bRegister = true;
	_onSipXmpp->getGloox()->registerMessageHandler(this);
	_onSipXmpp->SubscribeCallEvents(expireTime);
}

//virtual 
void EnableCallEventsWorker::handleMessage( const Message& msg, MessageSession* session	)  
{ 
	Logger::log_debug( "EnableCallEventsWorker::handleMessage type: %d, subject: %s, message: %s, thread id: %s", msg.subtype(),
		msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );

	// If already complete, then we just have not been unregistered yet
	// and we have other messages come in
	if ( _bComplete )
	{
		Logger::log_debug( "EnableCallEventsWorker::handleMessage IGNORED, already complete");
		return;
	}

	// See if one of our recognized events
	XmppEvent* evt = XmppEventFactory::msgFactory(msg);
	XmppPubSubSubscribedEvent* subEvt = OnSipCallStateHelper::getPubSubSubscribedEvent(evt);
	const gloox::Error *err = msg.error();

	// If not Subscribed Event, then assume it is not ours
	if ( subEvt == NULL )
	{
		Logger::log_debug("EnableCallEventsWorker::handleMessage assume not subscribed evt");
		return;
	}
	// If error or not subscribed
	if ( err != NULL || !subEvt->m_bSubscribed )
	{
		Logger::log_error("EnableCallEventsWorker::handleMessage ERROR err=%p subscribed=%d", subEvt, subEvt->m_bSubscribed );
		_bError = true;
	}
	// else subscribed ok, get the subid
	else
	{
		Logger::log_debug("EnableCallEventsWorker::handleMessage subscribed. subid=%s", subEvt->m_subid.c_str() );
		_subid = subEvt->m_subid.c_str();
	}
	_bComplete = true;
}

// Unregister our message handler
void EnableCallEventsWorker::_unregister()
{
	if ( _bRegister )
	{
		Logger::log_debug("EnableCallEventsWorker::_unregister");
		_bRegister = false;
		_onSipXmpp->getGloox()->removeMessageHandler(this);
	}
}

//*********************************************************************
//*********************************************************************

//void ReAuthorizeSubscribeWorker::Execute(const string& oldSubId)
void ReAuthorizeSubscribeWorker::Execute()
{
//	Logger::log_debug("ReAuthorizeSubscribeWorker::Execute oldSubId=%s", oldSubId.c_str());
//	_oldSubId = oldSubId;
	Logger::log_debug("ReAuthorizeSubscribeWorker::Execute" );
	_authorizer.reset( new AuthorizeWorker(_onSipXmpp) );
	_state = Authorizing;
	_authorizer->Execute();
}

//virtual 
bool ReAuthorizeSubscribeWorker::IsComplete()
{	
	// If already complete
	if ( _bComplete )
		return true;

	// If still authorizing
	if ( _state == Authorizing )
	{
		if ( !_authorizer->IsComplete() )
			return false;
		// Authorization complete
		Logger::log_debug("ReAuthorizeSubscribeWorker::IsComplete authorize complete. err=%d expireTime=%s", _authorizer->IsError(), _authorizer->ExpireDate().c_str() );
		bool bError = _authorizer->IsError();
		string expireTime = _authorizer->ExpireDate();
		_authorizer.reset(NULL);

		// If error
		if ( bError )
		{
			Logger::log_error("ReAuthorizeSubscribeWorker::IsComplete authorize error");
			_bError = true;
			_bComplete = true;
			return true;
		}
		// Go to subscribe call events state
		_subscriber.reset( new EnableCallEventsWorker(_onSipXmpp) );
		_subscriber->Execute( expireTime  );
		_state = Subscribing;
		return false;
	}

	// If still subscribing
	if ( _state == Subscribing )
	{
		if ( !_subscriber->IsComplete() )
			return false;

		// Subscribing complete
		_subid = _subscriber->subId();
		Logger::log_debug("ReAuthorizeSubscribeWorker::IsComplete subscriber complete. err=%d subid=%s", _subscriber->IsError(), _subid.c_str() );
		bool bError = _subscriber->IsError();
		_subscriber.reset(NULL);

		// If error
		if ( bError )
		{
			Logger::log_error("ReAuthorizeSubscribeWorker::IsComplete subscriber error");
			_bError = true;
			_bComplete = true;
			return true;
		}

		// Go to unsubscribe the original subscription
//		_unsubscriber.reset( new UnsubscriberWorker(_onSipXmpp) );
//		_unsubscriber->Execute( _oldSubId );
//		_state = Unsubscribing;
//		return false;
		_bComplete =true;
		return true;
	}

/*  Does not appear that we have to unsubscribe the old one, that the re-subscribe takes care of this!!
	// If still Unsubscribing
	if ( _state == Unsubscribing )
	{
		if ( !_unsubscriber->IsComplete() )
			return false;

		// Unsubscribing complete
		Logger::log_debug("ReAuthorizeSubscribeWorker::IsComplete unsubscriber complete. err=%d", _unsubscriber->IsError() );
		_bError = _unsubscriber->IsError();
		_bComplete = true;
		_unsubscriber.reset(NULL);

		// If error
		if ( _bError );
			Logger::log_error("ReAuthorizeSubscribeWorker::IsComplete unsubscriber error");
		return true;
	}
*/
	return false;
}






