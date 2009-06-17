#ifndef ONSIPXMPP_H
#define ONSIPXMPP_H

#include "onsipxmppbase.h"
#include "onsipcallstatemachine.h"
#include "onsipinitstatemachine.h"
#include "utils.h"

class OnSipXmpp : protected OnSipXmppBase, OnSipInitStateMachineNotifyBase, OnSipCallStateMachineNotifyBase
{
private:
	std::auto_ptr<OnSipCallStateMachine> m_callStateMachine;
	std::auto_ptr<OnSipInitStateMachine> m_initStateMachine;

	// Helper method to signal all state machines of the event
	void _signalEvent(XmppEvent *pEvent);

	// Unique ID generator, THREAD-SAFE
	UniqueId_ts _ids;

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
	string SubscribeCallEvents();
	string UnsubscribeCallEvents(string subid);

	// Ping to server to keep it alive
	void Ping()
	{	OnSipXmppBase::Ping();	}

	// Call Number on OnSip PBX
	// Pass unique contextId to be associated with this request,
	// the Iq Result will have the same contextId.
	//   customTag = unique value in form of "x=y" that is to be added to the TO and FROM fields
	// in the call request.  This can be used to uniquely identify the <message> events that
	// are associated with this call.  The TO with customTag can be used to identify messages
	// related to the inbound part, and the FROM/tag can be used to identify messages related
	// to the outbound part.
	//    toField/fromField = optional tstring values to retrieve the exact TO and FROM values used in the XMPP request.
	tstring CallNumber(tstring number,int contextId,tstring customTag,tstring* toField=NULL,tstring* fromField=NULL);

	// Hangup call PBX
	// Pass unique contextId to be associated with this request,
	// the Iq Result will have the same contextId.
	// Returns the XMPP id used for the request
	tstring DropCall(tstring sipCallid,tstring fromTag,tstring toTag, long contextId);

	// THREAD-SAFE
	// Get next unique ID for contextId and other various purposes
	long getUniqueId() { return _ids.getNextId(); }

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
