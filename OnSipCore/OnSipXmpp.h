#ifndef ONSIPXMPP_H
#define ONSIPXMPP_H

#include "onsipxmppbase.h"
#include "onsipcallstatemachine.h"
#include "onsipinitstatemachine.h"
#include "utils.h"

class OnSipXmpp : public OnSipXmppBase, protected OnSipInitStateMachineNotifyBase, OnSipCallStateMachineNotifyBase
{
private:
	std::auto_ptr<OnSipCallStateMachine> m_callStateMachine;
	std::auto_ptr<OnSipInitStateMachine> m_initStateMachine;

	// Helper method to signal all state machines of the event
	void _signalEvent(XmppEvent *pEvent);

	// Keep track of the last state from the InitStateMachine
	OnSipInitStates::InitStates m_lastInitState;

protected:
	virtual void onConnect();
	virtual void onDisconnect(gloox::ConnectionError e);
	virtual void handleMessage( const Message& msg, MessageSession* session = 0 );
	virtual void handleIqID( const IQ& iq, int context );
	void handleSubscriptionResult( const std::string& id,
									   const JID& service,
									   const std::string& node,
									   const std::string& sid,
									   const JID& jid,
									   const PubSub::SubscriptionType subType,
									   const Error* error = 0 );

	// Virtual called while in Connect/Recv loop inside the Start method.
	// Return false to abort and disconnect
	virtual bool onConnectLoop() ;

	// Virtual notify from OnSipInitStateMachine of state changes
	virtual void InitStateChange(OnSipInitStates::InitStates state,OnSipInitStateData /*stateData*/,StateChangeReason::eOnStateChangeReason reason) 
	{	
		Logger::log_debug(_T("OnSipXmpp::InitStateChange state=%d/%s reason=%d"), state, OnSipInitStates::InitStatesToString(state), reason );	
		// Keep track of the last InitStateMachine state
		m_lastInitState = state;
		_checkThread.CheckSameThread();	// ensure single thread operation
	}

	// Virtual notify from OnSipCallStateMachine of state changes
	virtual void CallStateChange(OnSipXmppStates::CallStates state,OnSipCallStateData stateData,StateChangeReason::eOnStateChangeReason reason)
	{	
		Logger::log_debug(_T("OnSipXmpp::CallStateChange state=%d/%s stateData=%s reason=%d "), state, OnSipXmppStates::CallStateToString(state), stateData.ToString().c_str(), reason );	
		_checkThread.CheckSameThread();	// ensure single thread operation
	}

public:

	// non-thread-safe functions 
	// providing OnSip specific XMPP communication
	void Authorize(int contextId);
	// Enable call events on OnSIP PBX
	// returns the Id used for event
	//  expireTime in XMPP format, e.g. 2006-03-31T23:59Z
	//  can pass empty string to not have field passed in subscribe request
	string SubscribeCallEvents(const string& expireTime);
	long UnsubscribeCallEvents(const string& subid);
	long UnsubscribeCallEvents(const string& nodeid, const string& subid);
	// Trigger request from server for it to return the list of all subscriptions
	void getSubscriptions();

	// Ping to server to keep it alive
	void Ping()
	{	OnSipXmppBase::Ping();	}

	// Call number on PBX
	// Pass unique contextId to be associated with this request,
	// callSetupId = call setup id to be used in messages and Iq result for this specific call.
	//    this is used to synchronize the events we get back so we know it goes with this call.  This will set server call-setup-id values
	// the Iq Result will have the same contextId.
	//    toField/fromField = optional tstring values to retrieve the exact TO and FROM values used in the XMPP request.
	// Returns the XMPP id used for the request
	tstring CallNumber(tstring number,int contextId,tstring& callSetupId, tstring* toField=NULL,tstring* fromField=NULL);

	// Hangup call PBX
	// Pass unique contextId to be associated with this request,
	// the Iq Result will have the same contextId.
	// Returns the XMPP id used for the request
	tstring DropCall(tstring sipCallid,tstring fromTag,tstring toTag, long contextId);

	// Start the Xmpp connection.  This function is asynchronous,
	// PollXMPP() should be called in tight loop,
	// and Cleanup() called when done
	// Virtual onConnectLoop is periodically called
	//   bSync = if true, then will stay in the Start() method until disconnect.
	// 
	// Will return ConnNoError if all ok
	ConnectionError  Start(LoginInfo& loginInfo);

	// Run and poll the XMPP engine, keeps things going.
	// Virtual onConnectLoop is periodically called
	ConnectionError PollXMPP(DWORD dwMsecs);

	// Start the shutdown process,
	// signal the OnSipInitStateMachine to start unsubscribing, etc.
	// This method is asynchronous.
	// The state machine should be polled or monitored to see if shutdown
	void AsyncShutdown();

	// Returns true if the shutdown has completed.
	// e.g. the InitStateMachine has done the proper shutdown, unsubscribing
	bool IsShutdownComplete();

	// Should be called after XMPP engine is done,
	// after Start() has been called and done calling PollXMPP
	void Cleanup();

	// THREAD-SAFE
	//
	// This is asynchronous and thread-safe.
	// The number will be dialed and a StateHandler will be added
	// to the state machine to track the call.
	// Request a phone number to be dialed.
	// Returns the unique callId for this request.
	long MakeCall(const tstring& number);

	// THREAD-SAFE
	//
	// This is asynchronous and thread-safe.
	// Drop the specified call
	void DropCall(long callId);

	// Return the last notified (current) state of the InitStateMachine
	OnSipInitStates::InitStates GetInitStateMachineState()
	{	
		_checkThread.CheckSameThread();	// ensure single thread operation
		return m_lastInitState;
	}

	OnSipXmpp();
	virtual ~OnSipXmpp();
};

#endif
