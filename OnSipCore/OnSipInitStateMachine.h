#ifndef ONSIP_INIT_STATEMACHINE
#define ONSIP_INIT_STATEMACHINE

#include "StateMachine.h"
#include "OnSipCallStateMachine.h"
#include "onsipstatemachinebase.h"

class OnSipXmpp;

// Class wrapper around enum InitStates
class OnSipInitStates
{
public:
	// Note: if new states added, modify InitStatesToString and GetInitStatesType method!!
	enum InitStates { NotSet, PreLogin, LoginError, Authorizing, Authorized, AuthorizedError, EnablingCallEvents, OK, EnabledCallError, Disconnected, ReAuthorizing };

	static TCHAR* InitStatesToString(InitStates state);

};

// Class wrapper around enum InitStatesType.
// Maps the InitStates enum to a category or type to determine type of handling.
class OnSipInitStatesType
{
public:
	enum InitStatesType { NOTSET, INPROGRESS, OK, FATAL, DISCONNECTED };

	static TCHAR* InitStatesTypeToString(OnSipInitStatesType::InitStatesType state);

	// For the specified InitStates enum, return the type/category that it belongs
	static InitStatesType GetInitStatesType(OnSipInitStates::InitStates initState);
};

class OnSipInitStateData
{
public:
	// No State Data at this type
	// TODO??  Keep track of how long within a InitStatesType type of state???
};


// Authorization timeouts
#define AUTHTIMEOUT_QUICK_RETRY	(30 * MSECS_IN_SEC)
#define AUTHTIMEOUT_RETRY			(15 * MSECS_IN_MIN)
#define AUTHTIMEOUT				( 2 * MSECS_IN_HOUR)
#define PINGTIMEOUT					( 5 * MSECS_IN_MIN)

//#define AUTHTIMEOUT_QUICK_RETRY (30 * MSECS_IN_SEC)
//#define AUTHTIMEOUT_RETRY		(1 * MSECS_IN_MIN)
//#define AUTHTIMEOUT				(2 * MSECS_IN_MIN)

// User outgoing call using physical phone device
class OnSipInitStateHandler : public StateHandler<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData>
{
private:
	OnSipXmpp* m_pOnSipXmpp;
	int _contextId;
	tstring m_enableCallEventsId;
	TimeOut m_authTO;
	TimeOut m_ping;
	bool m_bEnabledCallEvents;

protected:
	virtual bool IsYourEvent(XmppEvent *pEvent);
	virtual bool IsStillExist();
	virtual bool PollStateHandler();

public:
	OnSipInitStateHandler(OnSipXmpp* pOnSipXmpp);
};

// Base class for IStateNotify that is associated with the InitStateMachine
class OnSipInitStateMachineNotifyBase : protected IStateNotify<OnSipInitStates::InitStates,OnSipInitStateData>
{
private:
	// Take over virtual that will be called from OnSipIniStateMachine for any changes
	// Pass on to a more meaningful name virtual
	virtual void StateChange(OnSipInitStates::InitStates state,OnSipInitStateData stateData,StateChangeReason::eOnStateChangeReason reason) 
	{	InitStateChange(state,stateData,reason);	}
protected:
	// Required virtual
	virtual void InitStateChange(OnSipInitStates::InitStates,OnSipInitStateData,StateChangeReason::eOnStateChangeReason reason)  = 0;
};

// OnSip State Machine that manages calls and their states
// using XmppEvent events
class OnSipInitStateMachine : public OnSipStateMachineBase<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData>
{
	friend class OnSipXmpp;
private:

protected:
	virtual StateHandler<OnSipInitStates::InitStates,XmppEvent,OnSipInitStateData> *UnknownEvent(XmppEvent* pEvent);

	// Virtual notify that either the state has changed or the state data has changed
	virtual void OnStateChange(OnSipInitStates::InitStates,OnSipInitStateData&,StateChangeReason::eOnStateChangeReason reason);

public:
	OnSipInitStateMachine(OnSipXmpp* pOnSipXmpp) : OnSipStateMachineBase(pOnSipXmpp)
	{ }
};

#endif
