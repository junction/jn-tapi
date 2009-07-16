#include <stdafx.h>
#include "OnSip.h"
#include "OnSipXmpp.h"

#include "dataform.h"
#include "iq.h"
#include "adhoc.h"
#include "adhochandler.h"
#include "stanza.h"
#include "pubsub.h"
#include "pubsubmanager.h"
#include "pubsubresulthandler.h"
#include "error.h"
#include "pubsubevent.h"
#include "stanza.h"
#include "subscription.h"
#include "message.h"
#include "OnSipWorkers.h"
#include "onsipXmppEvents.h"

#include "OnSipInitStateMachine.h"

OnSipXmpp::OnSipXmpp()
{
	Logger::log_debug(_T("OnSipXmpp::OnSipXmpp"));
	// Create the state machines
	m_callStateMachine.reset( new OnSipCallStateMachine(this) );
	m_initStateMachine.reset( new OnSipInitStateMachine(this) );

	// Connect the virtual notifies of Init and Call state changes
	// to our class, e.g. virtual InitStateChange, virtual CallStateChange
	m_callStateMachine->AddIStateNotify(this);
	m_initStateMachine->AddIStateNotify(this);

	m_lastInitState = OnSipInitStates::NotSet;
}

OnSipXmpp::~OnSipXmpp()
{
	Logger::log_debug(_T("OnSipXmpp::~OnSipXmpp"));
	m_callStateMachine->ClearIStateNotifys();
	m_initStateMachine->ClearIStateNotifys();
}

// Authorize with OnSip PBX
// Pass unique contextId to be associated with this request,
// the Iq Result will have the same contextId
void OnSipXmpp::Authorize(int contextId)
{
	Logger::log_debug("OnSipXmpp::Authorize contextId %d",contextId);
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	OnSipXmppBase::Authorize(contextId,this);
}

// Enable call events on OnSIP PBX
// returns the Id used for event
//  expireTime in XMPP format, e.g. 2006-03-31T23:59Z
//  can pass empty string to not have field passed in subscribe request
string OnSipXmpp::SubscribeCallEvents(const string& expireTime)
{
	Logger::log_debug("OnSipXmpp::SubscribeCallEvents expireTime='%s", expireTime.c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return OnSipXmppBase::SubscribeCallEvents(expireTime,this);
}

// Unscribe call events on OnSIP PBX, pass the subid
// used in the resulting subscribe success
// returns the context id for the Iq event
long OnSipXmpp::UnsubscribeCallEvents(const string& nodeid, const string& subid)
{
	Logger::log_debug("OnSipXmpp::UnsubscribeCallEvents nodeid=%s subid=%s", nodeid.c_str(), subid.c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return OnSipXmppBase::UnsubscribeCallEvents(nodeid,subid,this,this);
}

// Unscribe call events on OnSIP PBX, pass the subid
// used in the resulting subscribe success
// returns the context id for the Iq event
long OnSipXmpp::UnsubscribeCallEvents(const string& subid)
{
	Logger::log_debug("OnSipXmpp::UnsubscribeCallEvents subid=%s", subid.c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return OnSipXmppBase::UnsubscribeCallEvents(subid,this,this);
}

// Trigger request from server for it to return the list of all subscriptions
void OnSipXmpp::getSubscriptions()
{
	Logger::log_debug("OnSipXmpp::getSubscriptions");
	_checkThread.CheckSameThread();
	OnSipXmppBase::getSubscriptions(this);
}

// Call number on PBX
// Pass unique contextId to be associated with this request,
// the Iq Result will have the same contextId.
//   customTag = unique value in form of "x=y" that is to be added to the TO and FROM fields
// in the call request.  This can be used to uniquely identify the <message> events that
// are associated with this call.  The TO with customTag can be used to identify messages
// related to the inbound part, and the FROM/tag can be used to identify messages related
// to the outbound part.
//    toField/fromField = optional tstring values to retrieve the exact TO and FROM values used in the XMPP request.
// Returns the XMPP id used for the request
tstring OnSipXmpp::CallNumber(tstring number,int contextId,tstring customTag,tstring* toField,tstring* fromField)
{
	Logger::log_debug("OnSipXmpp::CallNumber number='%s' contextId=%d customTag=%s",number.c_str(),contextId,customTag.c_str());
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// If does not contain a "@", then assume it is a phone number
	tstring toNumber = number;
	if ( !Strings::contains(number,_T("@")) )
	{
		// Strip any non-digit values
		tstring tempNumber;
		tstring::iterator iter = number.begin();
		while ( iter != number.end() )
		{
			if ( *iter >= '0' && *iter <= '9' )
				tempNumber += *iter;
			iter++;
		}
		toNumber = Strings::stringFormat(_T("%s@%s"), tempNumber.c_str(), m_login.m_domain.c_str() );
		Logger::log_debug("OnSipXmpp::CallNumber converted phone number=%s to %s", number.c_str(), toNumber.c_str() );
	}
	// if begins with "Tsip:", then it was a sip dial request, 
	// but the application preceded the number with a "T" to signify
	// to dial as tone.  Strip the "T" and dial only the sip number.
	// This occurs when using the Windows Phone Dialer
	else if ( Strings::startsWith( Strings::tolower(number), _T("tsip:") ) )
	{
		toNumber = number.substr(5,number.size()-5);
		Logger::log_debug( _T("OnSipXmpp::CallNumber removed Tsip from number=%s"), toNumber.c_str() );
	}
	// Do same check for SIP TLS format, "sips:"
	else if ( Strings::startsWith( Strings::tolower(number), _T("tsips:") ) )
	{
		toNumber = number.substr(6,number.size()-6);
		Logger::log_debug( _T("OnSipXmpp::CallNumber removed Tsips from number=%s"), toNumber.c_str() );
	}

	tstring fromNumber = Strings::stringFormat( _T("sip:%s"), m_login.SIPAddress().c_str() );

	// If customTag is specified, then append to TO and FROM fields,
	//  e.g. "1234567890@abc.com;tag=value"
	if ( !customTag.empty() )
	{
		toNumber = Strings::stringFormat( _T("%s;%s"), toNumber.c_str(), customTag.c_str() );
		fromNumber = Strings::stringFormat( _T("%s;%s"), fromNumber.c_str(), customTag.c_str() );
	}

	// Ensure number is prefixed with "sip:" or "sips:"
	if ( !Strings::startsWith( Strings::tolower(toNumber), _T("sip:") ) && !Strings::startsWith( Strings::tolower(toNumber), _T("sips:") ) )
		toNumber = Strings::stringFormat( _T("sip:%s"), toNumber.c_str() );

	// If specified return values for the to/from fields
	if ( toField != NULL )
		*toField = toNumber;
	if ( fromField != NULL )
		*fromField = fromNumber;

	// Create DataForm
	DataForm* settings = new DataForm( gloox::TypeSubmit );
	// Add from field
	DataFormField* dff = new DataFormField( "from", fromNumber );
	settings->addField(dff);
	// Add to field
	dff = new DataFormField( "to", toNumber );
	settings->addField(dff);

	// Add command
	JID toJid( ONSIP_ACTIVECALLS_COMMAND );
	string id = m_gloox->getID();
	IQ iq( IQ::Set, toJid, id );
	Adhoc::Command *cmd = new Adhoc::Command( "create", EmptyString, Adhoc::Command::Executing, settings );
	iq.addExtension( cmd );
	Logger::log_debug("OnSipXmpp::CallNumber '%s' using id=%s",number.c_str(),id.c_str());
	m_gloox->send( iq, this, contextId );
	return id;
}

// Hangup call PBX
// Pass unique contextId to be associated with this request,
// the Iq Result will have the same contextId.
// Returns the XMPP id used for the request
tstring OnSipXmpp::DropCall(tstring sipCallid,tstring fromTag,tstring toTag, long contextId)
{
	Logger::log_debug("OnSipXmpp::DropCall sipCallid=%s fromTag=%s toTag=%s contextId=%ld", sipCallid.c_str(), fromTag.c_str(), toTag.c_str(), contextId );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Create DataForm
	DataForm* settings = new DataForm( gloox::TypeSubmit );
	// Add from field
	DataFormField* dff = new DataFormField( "from-tag", fromTag );
	settings->addField(dff);
	// Add to field
	dff = new DataFormField( "to-tag", toTag );
	settings->addField(dff);
	// Add callid field
	dff = new DataFormField( "call-id", sipCallid );
	settings->addField(dff);

	// Add command
	JID toJid( ONSIP_ACTIVECALLS_COMMAND );
	string id = m_gloox->getID();
	IQ iq( IQ::Set, toJid, id );
	Adhoc::Command *cmd = new Adhoc::Command( "terminate", EmptyString, Adhoc::Command::Executing, settings );
	iq.addExtension( cmd );
	Logger::log_debug("OnSipXmpp::DropCall '%s' using id=%s",sipCallid.c_str(),id.c_str());
	m_gloox->send( iq, this, contextId );
	return id;
}


// virtual
void OnSipXmpp::onConnect()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	OnSipXmppBase::onConnect();
	// Pass event on to state machines
	_signalEvent( XmppEventFactory::connectFactory(true) );
};

// virtual
void OnSipXmpp::onDisconnect(gloox::ConnectionError e)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	OnSipXmppBase::onDisconnect(e);
	// Pass event on to state machines
	_signalEvent( XmppEventFactory::connectFactory(false) );
}

// Virtual periodically called while in Connect/Recv loop inside the Start method.
// Return false to abort and disconnect
//virtual 
bool OnSipXmpp::onConnectLoop() 
{ 
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
//	Logger::log_trace( _T("OnSipXmpp::onConnectLoop") );
	// Poll state handlers to see if changes..
	m_initStateMachine->PollStateHandlers();
	m_callStateMachine->PollStateHandlers();
	return true; 
}

// Helper method to signal all state machines of the event.
// Either the XmppEvent is passed off to a stateMachine for it to manage its lifetime,
// or the XmppEvent is deleted if not recognized
void OnSipXmpp::_signalEvent(XmppEvent *pEvent)
{
	Logger::log_trace("OnSipXmpp::_signalEvent pEvent=\r\n%s\r\n", pEvent->ToString().c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Signal the init state machine. Specify for it not to delete the event if not handled
	Logger::log_debug("OnSipXmpp::_signalEvent pEvent=%x.  tryinitState",pEvent);
	if ( m_initStateMachine->SignalEvent(pEvent,false) )
		return;

	// First signal the call state machine. Specify for it not to delete the event if not handled
	Logger::log_debug("OnSipXmpp::_signalEvent pEvent=%x.  trycallstate",pEvent);
	if ( m_callStateMachine->SignalEvent(pEvent,false) )
		return;

	// Delete the event, not recognized
	Logger::log_warn("OnSipXmpp::_signalEvent pEvent=%x.  unrecognized",pEvent);
	delete pEvent;
}

/**
* Reimplement this function if you want to be notified about
* incoming messages.
* @param msg The complete Message.
* @param session If this MessageHandler is used with a MessageSession, this parameter
* holds a pointer to that MessageSession.
* @since 1.0
*/
//virtual 
void OnSipXmpp::handleMessage( const Message& msg, MessageSession* session	)  
{ 
	Logger::log_debug( "OnSipXmpp::handleMessage type: %d, subject: %s, message: %s, thread id: %s", msg.subtype(),
						msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	const gloox::Error *err = msg.error();
	if ( err != NULL )
		Logger::log_error("OnSipXmpp::handleMessage ERROR %s\n", err->text(EmptyString).c_str() );

	// See if one of our recognized events
	XmppEvent* evt = XmppEventFactory::msgFactory(msg);
	if ( evt != NULL )
	{
		Logger::log_debug("OnSipXmpp::handleMessage event found. type=%d", evt->m_type );
		// Pass event on to the state machines
		_signalEvent(evt);
	}
}

/**
* Reimplement this function if you want to be notified about
* incoming IQs with a specific value of the @c id attribute. You
* have to enable tracking of those IDs using Client::trackID().
* This is usually useful for IDs that generate a positive reply, i.e.
* &lt;iq type='result' id='reg'/&gt; where a namespace filter wouldn't
* work.
* @param iq The complete IQ stanza.
* @param context A value to restore context, stored with ClientBase::trackID().
* @note Only IQ stanzas of type 'result' or 'error' can arrive here.
* @since 1.0
*/
//virtual 
void OnSipXmpp::handleIqID( const IQ& iq, int context ) 
{ 
	OnSipXmppBase::handleIqID(iq,context);
	Logger::log_debug("OnSipXmpp::handleIqID type=%d error=%d", iq.subtype(), iq.subtype() == IQ::Error );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	const gloox::Error *err = iq.error();
	if ( err != NULL )
		Logger::log_error("OnSipXmpp::handleIqID ERROR %s\n", err->text(EmptyString).c_str() );

	// See if one of our recognized events
	XmppEvent *evt = XmppEventFactory::iqFactory(iq,context);
	if ( evt != NULL )
	{
		Logger::log_debug("OnSipXmpp::handleIqID event found. type=%d", evt->m_type );
		// Pass event on to the state machines
		_signalEvent(evt);
	}
}

/*
 * Receives the subscription results. In case a problem occured, the
 * Subscription ID and SubscriptionType becomes irrelevant.
 *
 * @param service PubSub service asked for subscription.
 * @param node Node asked for subscription.
 * @param sid Subscription ID.
 * @param subType Type of the subscription.
 * @param error Subscription Error.
 *
 * @see Manager::subscribe
 */
//virtual 
void OnSipXmpp::handleSubscriptionResult( const std::string& id,
									   const JID& service,
									   const std::string& node,
									   const std::string& sid,
									   const JID& jid,
									   const PubSub::SubscriptionType subType,
									   const Error* error)
{
	Logger::log_debug("resultHandler::handleSubscriptionResult subType=%d service=%s id=%s", subType,service.full().c_str(), id.c_str() );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	if ( error != NULL )
		Logger::log_error("resultHandler::handleSubscriptionResult ERROR=%s", error->text(EmptyString).c_str() );
}

// Start the Xmpp connection.  This function is asynchronous,
// PollXMPP() should be called in tight loop,
// and Cleanup() called when done
// Virtual onConnectLoop is periodically called
//   bSync = if true, then will stay in the Start() method until disconnect.
// 
// Will return ConnNoError if all ok
ConnectionError OnSipXmpp::Start(LoginInfo& loginInfo)
{ 
	Logger::log_debug(_T("OnSipXmpp::Start"));
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	// Clear out the State Handlers
	m_initStateMachine->RemoveStateHandlers();
	m_callStateMachine->RemoveStateHandlers();

	// Add our initial OnSipInitStateMachine to handle logon success, failure,
	// authorize, enable call events, etc.
	m_initStateMachine->AddStateHandler( new OnSipInitStateHandler(this) );

	// Start XMPP engine, call it async
	ConnectionError ce = OnSipXmppBase::Start(loginInfo,false); 
	Logger::log_debug(_T("OnSipXmpp::Start ce=%d"),ce);
	return ce;
}

// Run and poll the XMPP engine, keeps things going.
// Virtual onConnectLoop is periodically called
ConnectionError OnSipXmpp::PollXMPP(DWORD dwMsecs)
{
	return OnSipXmppBase::AsyncPolling(dwMsecs);
}

// Start the shutdown process,
// signal the OnSipInitStateMachine to start unsubscribing, etc.
// This method is asynchronous.
// The state machine should be polled or monitored to see if shutdown
void OnSipXmpp::AsyncShutdown()
{
	Logger::log_debug(_T("OnSipXmpp::AsyncShutdown"));
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	m_initStateMachine->AddEvent( new ShutdownRequestEvent() );
}

// Returns true if the shutdown has completed.
// e.g. the InitStateMachine has done the proper shutdown, unsubscribing
bool OnSipXmpp::IsShutdownComplete()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	if ( m_initStateMachine.get() == NULL )
	{
		// Shouldn't occur..
		Logger::log_error( _T("OnSipXmpp::IsShutdownComplete NULL ptr") );
		return true;
	}
	// Get all states.  List should be empty when shutdown,
	// since the InitStateMachine will exit its handler
	list<StateAndStateData<OnSipInitStates::InitStates,OnSipInitStateData>> es = m_initStateMachine->GetAllStates();
	if ( es.size() > 0 )
		return false;
	Logger::log_debug( _T("OnSipXmpp::IsShutdownComplete is complete") );
	return true;
}

// Should be called after XMPP engine is done,
// after Start() has been called and done calling PollXMPP
void OnSipXmpp::Cleanup()
{
	Logger::log_debug(_T("OnSipXmpp::Cleanup"));
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// Clear out the State Handlers
	m_initStateMachine->RemoveStateHandlers();
	m_callStateMachine->RemoveStateHandlers();
	OnSipXmppBase::AsyncCleanup();
}

// THREAD-SAFE
//
// This is asynchronous and thread-safe.
// The number will be dialed and a StateHandler will be added
// to the state machine to track the call.
// Request a phone number to be dialed.
// Returns the unique callId for this request.
long OnSipXmpp::MakeCall(const tstring& number)
{
	return m_callStateMachine->MakeCallAsync(number);
}

// THREAD-SAFE
//
// This is asynchronous and thread-safe.
// Drop the specified call
void OnSipXmpp::DropCall(long callId)
{
	m_callStateMachine->DropCallAsync(callId);
}

