#ifndef ONSIP_STATEMACHINE_H
#define ONSIP_STATEMACHINE_H

#include "onsipxmppevents.h"
#include "statemachine.h"
#include "onsipstatemachinebase.h"

// Class wrapper around the enum CallStates
// State enum for the OnSip call management state machine
class OnSipXmppStates
{
public:
	enum CallStates { Unknown=0, Offering=1, 
						PhysicalOutProceeding=2, 	// User is placing a call directly on the SIP phone
						Connected=3, Dropped=4, 
						PreMakeCall=5,		 // Make call needs to be done, no XMPP has been done yet
						MakeCallSet=6,       // A Make call IQ SET has been done, no response yet..
						MakeCallRequested=7, // The incoming REQUESTED dialog state for the Make Call request
						MakeCallRequestedAnswered=8,  // Caller answered their physical phone to continue the Make Call request
						MakeCallOutgoingCreated=9,	 // Outgoing call has been created to to complete the call
						PreDropCall=10				 // State before we attempt to drop the call
                     	};

	// Helper method to convert CallStates enum to a string (for debug purposes)
	static TCHAR *CallStateToString(CallStates state);
};

// Class wrapper around the enum CallType
class OnSipXmppCallType
{
public:
	enum CallType {  Unknown=0, MakeCall=1, 				// Call generated by XMPP
							IncomingCall=2, 			// Incoming call
							PhysicalCall=3 			// Outgoing call by person
							};

	// Helper method to convert CallType enum to a string (for debug purposes)
	static TCHAR *CallTypeToString(CallType calltype);
};

// State data maintained for each unique call,
// e.g. unique call-id, caller-id, etc.
class OnSipCallStateData
{
public:
	CheckThread _checkThread;
	tstring m_id;							// XMPP id
	long m_callId;						// Unique call-id, internal value, not related to OnSip callid
	tstring m_remoteId;
	tstring m_calledId;
	tstring m_sipCallId;						// The SIP callId
	OnSipXmppCallType::CallType m_callType;
	tstring m_fromTag;		// needed for DropCall
	tstring m_toTag;			// needed for DropCall
	tstring m_branch;
	tstring m_call_setup_id;		// call-setup-id value that syncs makecall request with incoming async messages

	OnSipCallStateData() 
	{  m_callId = 0;  m_callType = OnSipXmppCallType::Unknown; }

	const OnSipCallStateData& operator=(const OnSipCallStateData& callData)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		if ( this == &callData )
			return *this;
		m_id = callData.m_id;
		m_callType = callData.m_callType;
		m_callId = callData.m_callId; 
		m_remoteId = callData.m_remoteId; 
		m_calledId = callData.m_calledId; 
		m_sipCallId = callData.m_sipCallId;
		m_fromTag = callData.m_fromTag;
		m_toTag = callData.m_toTag;
		m_branch = callData.m_branch;
		m_call_setup_id = callData.m_call_setup_id;
		return *this;
	}

	bool SameBranch(tstring branch)
	{	return m_branch == branch; }

	// Is the passed callSetupId the same callSetupId in the CallStateData.
	bool SameCallSetupId(tstring callSetupId)
	{	return  !m_call_setup_id.empty() && m_call_setup_id == callSetupId; }

	tstring ToString() const;
};

// Class to manage keeping track of branches of a call.
// Branches of a call occur when the user has multiple SIP endpoints registered
// at the same location.
// e.g. an incoming call will get multiple XMPP events, one for each SIP device
class callBranches
{
private:
	vector<OnSipCallStateData> m_branches;
	void _removeById(tstring id);
	// Checks the branches list for a call with the specified XMPP id.
	// Returns the index in the list, or returns -1 if not found
	int _getIdIndex(tstring id);

public:
	bool HasId(tstring id);

	size_t CountBranches()
	{	return m_branches.size(); }
	
	void AddBranch(OnSipCallStateData& callData);

	void AddBranch(XmppActiveCallEvent *ace,long callId,OnSipXmppCallType::CallType callType,bool bUpdateCallerId=true,LPCSTR szCallSetupId=NULL);

	// We need to see which call was dropped, and if the one in our
	// main CallStateData, then move one of the branches to replace it.
	bool CheckDroppedCall( OnSipCallStateData* callData, tstring droppedId );

	// Returns a copy of the branches
	vector<OnSipCallStateData> getBranches()
	{	
		vector<OnSipCallStateData> ret = m_branches; 
		return ret;
	}

	tstring ToString();
};

// Custom non-Xmpp event type used to signal that a
// specified call needs to be dropped.  The event is 
// sent to handlers in the callstate machine 
// to find the one that owns the call and to drop it
class DropRequestEvent : public XmppEvent
{
public:
	long m_callId;
	DropRequestEvent(long callId) : XmppEvent(EVT_DROP_REQUEST,_T(""),JID(),JID(),NULL)
	{	m_callId = callId;	}

	virtual string ToString()
	{	return Strings::stringFormat(_T("DropRequestEvent callId=%ld"), m_callId);	}
};

class ShutdownRequestEvent;

// Utility class to provide helper methods for the OnSip state machine
// Provides all static methods
class OnSipCallStateHelper
{
private:
	// Hide constructor, use static methods
	OnSipCallStateHelper() { }
public:

	static bool IsSameId(XmppEvent* pEvent1,XmppEvent* pEvent2);
	static bool IsSameContext(XmppEvent* pEvent1,XmppEvent* pEvent2);

	static XmppActiveCallEvent* getActiveCallEvent(XmppEvent* pEvent);
	static XmppRetractCallEvent* getRetractCallEvent(XmppEvent* pEvent);
	static XmppIqResultEvent* getXmppIqResultEvent(XmppEvent* pEvent);
	static XmppOnConnectEvent* getOnConnectEvent(XmppEvent* pEvent);
	static XmppOnDisconnectEvent* getOnDisconnectEvent(XmppEvent* pEvent);
	static XmppAuthEvent* getAuthEvent(XmppEvent* pEvent);
	static XmppPubSubSubscribedEvent* getPubSubSubscribedEvent(XmppEvent *pEvent);
	static DropRequestEvent* getDropRequestEvent(XmppEvent* pEvent);
	static ShutdownRequestEvent* getShutdownRequestEvent(XmppEvent* pEvent);
	static XmppCallRequestEvent* getCallRequestIqEvent(XmppEvent* pEvent);

	static void AssignCallStateData(OnSipCallStateData& callStateData,XmppActiveCallEvent* ace,long callId,bool bUpdateCallerId=true,LPCSTR szCallSetupId=NULL);
};

//*************************************************************************
//*************************************************************************

// Call State Handler Base object.
// All Call State handlers inherit from this
class OnSipCallStateHandlerBase : public OnSipStateHandlerBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>
{
protected:
	callBranches m_branches;

	// Common helper code for handling dropped calls and DropRequestEvent

	// IQ contextIds used to drop the call (or calls, possible if branches)
	vector<long> m_droppedContextIds;

	bool CheckDropRequestEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent* pEvent,long callId);

	// Check to see if the IQ results for out attempts to drop the call.
	// We would try to drop our calls if we got DropRequestEvent and handled (as above)
	bool CheckIqReplyDropRequest(XmppEvent* pEvent);

public:
	OnSipCallStateHandlerBase( OnSipXmppStates::CallStates callState, XmppEvent* pEvent) : OnSipStateHandlerBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>( callState , pEvent )
	{ }

	void assignNewState(OnSipXmppStates::CallStates callState,XmppEvent* pEvent)
	{
		Logger::log_debug(_T("OnSipCallStateHandlerBase::assignNewState callState=%d/%s"), callState, OnSipXmppStates::CallStateToString(callState) );
		OnSipStateHandlerBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>::assignNewState(callState,pEvent);
	}

	void assignNewState(OnSipXmppStates::CallStates callState,XmppEvent* pEvent,OnSipCallStateData& stateData)
	{
		Logger::log_debug(_T("OnSipCallStateHandlerBase::assignNewState callState=%d/%s"), callState, OnSipXmppStates::CallStateToString(callState) );
		OnSipStateHandlerBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>::assignNewState(callState,pEvent,stateData);
	}

	// Checks to see if the state has been in the specified state for the specified timeout in msecs.
	// If so, then the call will be put in the Dropped state and return true.
	bool CheckStateTimeout( OnSipXmppStates::CallStates callState, long timeout );
};

//*************************************************************************
//*************************************************************************

// User outgoing call using physical phone device
class OnSipOutgoingCallStateHandler : public OnSipCallStateHandlerBase
{
private:
protected:
	virtual bool IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent);
	virtual bool IsStillExist();
public:
	OnSipOutgoingCallStateHandler(XmppEvent* pEvent,long callId);
};

//*************************************************************************
//*************************************************************************

// Incoming Call State Handler
class OnSipIncomingCallStateHandler : public OnSipCallStateHandlerBase
{
private:
protected:
	virtual bool IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent);
	virtual bool IsStillExist();

public:
	OnSipIncomingCallStateHandler(XmppEvent* pEvent,long callId);
};

//*************************************************************************
//*************************************************************************

// Generated Outgoing Call State Handler
// This is type of "PreExecute" since the number must be dialed
// before this StateHandler is added to the state machine.
class OnSipMakeCallStateHandler : public OnSipCallStateHandlerBase 
{
private:

protected:
	virtual bool IsYourEvent(StateMachine<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,XmppEvent *pEvent);
	virtual bool IsStillExist();

	int m_contextId;		// context for initial IQ request
	//tstring m_in_id;		// Item ID of messages related to inbound call
	tstring m_out_id;		// Item ID of messages related to outbound call
	tstring m_toSipField;		// Exact TO field used in the XMPP request
	tstring m_fromSipField;	// Exact FROM field used in the XMPP request
	tstring m_in_sipCallId;	// SIP call ID associated with the incoming call
	tstring m_out_sipCallId;// SIP call ID associated with the outgoing call
public:
	// PreExecute method that will be called before
	// StateHandler is added to the state machine to
	// take care of the initial make call
	virtual bool PreExecute(OnSipStateMachineBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>* pStateMachine,OnSipXmpp *pOnSipXmpp);

	OnSipMakeCallStateHandler (const tstring& toDial,long callId);
};

//*************************************************************************
//*************************************************************************

class OnSipXmpp;

// Base class for IStateNotify that is associated with the CallStateMachine
// Inherit from this class to get state change notifies outside of the state handler, e.g. in state machine
class OnSipCallStateMachineNotifyBase : protected IStateNotify<OnSipXmppStates::CallStates,OnSipCallStateData>
{
	// Take over virtual that will be called from OnSipCallStateMachine for any changes
	// Pass on to a more meaningful name virtual
	virtual void StateChange(OnSipXmppStates::CallStates state,OnSipCallStateData stateData,StateChangeReason::eOnStateChangeReason reason)
	{	CallStateChange(state,stateData,reason);	}

protected:
	virtual void CallStateChange(OnSipXmppStates::CallStates state,OnSipCallStateData stateData,StateChangeReason::eOnStateChangeReason reason) = 0;
};

// OnSip State Machine that manages calls and their states
// using XmppEvent events
class OnSipCallStateMachine : public OnSipStateMachineBase<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData>
{
	friend class OnSipXmpp;
private:

protected:
	virtual StateHandler<OnSipXmppStates::CallStates,XmppEvent,OnSipCallStateData> *UnknownEvent(XmppEvent* pEvent);

	// Virtual notify that either the state has changed or the state data has changed
	virtual void OnStateChange(OnSipXmppStates::CallStates,OnSipCallStateData&,StateChangeReason::eOnStateChangeReason reason);

public:
	OnSipCallStateMachine(OnSipXmpp* pOnSipXmpp) : OnSipStateMachineBase(pOnSipXmpp)
	{ }

	// NOT thread-safe
	//
	// Drop a phone call for the specified call data
	//  callId = unique ID for this call
	//   returns the unique IQ contextId in the return parameter
	bool DropCall( OnSipCallStateData& callData, long *contextId );

	// THREAD-SAFE
	//
	// Make a phone call.  This will be done asynchrously.
	// The request will be inserted into the state machine for handling.
	// Returns the unique callid that refers to the unique ID for this call
	long MakeCallAsync( const tstring& phoneNumber );

	// THREAD-SAFE
	//
	// Drop a phone call.  This will be done asynchronously
	//  callId = unique ID for this call
	void DropCallAsync( long callId );
};


#endif
