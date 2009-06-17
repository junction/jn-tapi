#ifndef ONSIP_TAPI_H
#define ONSIP_TAPI_H

#include "onsipxmpp.h"
#include "threads.h"

// Wrapper around the 2 primary values for the OnSipXmpp Call State machine
//    the OnSipXmppStates::CallStates (state) and OnSipCallStateData (stateData)
class OnSipTapiCall
{
public:
	OnSipXmppStates::CallStates m_callState;
	OnSipCallStateData m_callStateData;
	StateChangeReason::eOnStateChangeReason m_reason;

	const OnSipTapiCall& operator=(const OnSipTapiCall&);

	// Return the TAPI CallState for the OnSipTapiCall
	DWORD GetTapiCallState() const;

	// Remote ID
	tstring RemoteID() const
	{	return m_callStateData.m_remoteId; }

	// Called ID
	tstring CalledID() const
	{	return m_callStateData.m_calledId; }

	OnSipTapiCall(OnSipXmppStates::CallStates callState,OnSipCallStateData callStateData,StateChangeReason::eOnStateChangeReason reason);
	OnSipTapiCall() { }

	static DWORD GetTapiCallState( OnSipXmppStates::CallStates m_callState, const OnSipCallStateData& m_callStateData );

	tstring ToString() const;
};

// Wrapper around the 2 primary values for the OnSipXmpp Init State machine
//    the OnSipInitStates::InitStates (state) and OnSipInitStateData (stateData)
class OnSipInitState
{
public:
	OnSipInitStates::InitStates m_state;
	OnSipInitStateData m_stateData;
	StateChangeReason::eOnStateChangeReason m_reason;

	const OnSipInitState& operator=(const OnSipInitState&);

	OnSipInitState(OnSipInitStates::InitStates state,OnSipInitStateData stateData,StateChangeReason::eOnStateChangeReason reason);
	OnSipInitState() { }

	tstring ToString() const;
};

// Primary interface class for the TAPI driver.
// Inherits from the OnSipXmpp and takes over virtuals of call and init state machines.
class OnSipTapi : public OnSipXmpp
{
private:
	LoginInfo m_loginInfo;
	CriticalSection m_cs;
	std::list< OnSipTapiCall > m_lstCallEvents;
	std::list< OnSipInitState > m_lstInitEvents;
	bool Connect(LoginInfo& loginInfo,ConnectionError* ce);

protected:
	// Virtual notify from OnSipInitStateMachine of state changes
	virtual void InitStateChange(OnSipInitStates::InitStates state,OnSipInitStateData stateData,StateChangeReason::eOnStateChangeReason reason) ;

	// Virtual notify from OnSipCallStateMachine of state changes
	virtual void CallStateChange(OnSipXmppStates::CallStates state,OnSipCallStateData stateData,StateChangeReason::eOnStateChangeReason reason);

public:
	OnSipTapi();
	// bShutdown = if true, then try proper shutdown by having
	//   OnSipInitStateMachine do shutdown, unsubscribe, etc.
	void Disconnect(bool bShutdown);
	bool Poll();

	bool InitOnSipTapi(LoginInfo& loginInfo,HANDLE hDevStop,OnSipInitStatesType::InitStatesType* stateType);

	// Make a phone call to the specified number, returns the unique callId for the call
	long MakeCall(const tstring& number)
	{ return OnSipXmpp::MakeCall(number); }

	// Retrieve any InitEvents that have occurred in the last poll.
	// Returns true if events were found.
	bool GetInitEvents( std::list<OnSipInitState>& lstEvents);

	// Retrieve any CallEvents that have occurred in the last poll.
	// Returns true if events were found.
	bool GetCallEvents( std::list<OnSipTapiCall>& lstEvents);

	// Clear all Init and Call Events that were collected during the Poll
	void ClearEvents();
};
#endif