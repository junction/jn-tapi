#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "onsip.h"
#include "logger.h"

// Class wrapper around OnStateChangeReason enum
class StateChangeReason
{
public:
	// Reasons why OnStateChange virtual are called
	enum eOnStateChangeReason { IsYourEventHandled, NewHandlerCreated, RemovedHandler, PollHandler, AddedHandler };
};

// An external interface that can be implemented
// in order to retrieve external notifications of state
// changes within a state machine
template <class Tstate,class TstateData>
class IStateNotify
{
public:
	virtual void StateChange(Tstate state,TstateData stateData,StateChangeReason::eOnStateChangeReason reason) =0;
};

// A container of IStateNotify instances to be
// notified when a OnStatechange occurs within a state machine.
template <class Tstate,class TstateData>
class IStateNotifyContainer
{
private:
	CheckThread _checkThread;
	std::list< IStateNotify<Tstate,TstateData> * > m_stateNotifys;

public:
	// Add a IStateNotify to be notified for a OnStateChange notify within
	// a state machine.
	// The iStateNotify instance passed is not owned by this object!
	// It should be cleared if this object is being released
	void AddIStateNotify( IStateNotify<Tstate,TstateData>* iStateNotify )
	{	
		Logger::log_debug( _T("IStateNotifyContainer::AddIStateNotify %p sz=%d"), iStateNotify, m_stateNotifys.size() );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		m_stateNotifys.push_back(iStateNotify);
	}

	// Clear all IStateNotifys from our list to be notified
	void Clear()
	{
		Logger::log_debug( _T("IStateNotifyContainer::Clear sz=%d"), m_stateNotifys.size() );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		m_stateNotifys.clear();
	}

	void NotifyStateChange(Tstate state,TstateData stateData,StateChangeReason::eOnStateChangeReason reason)
	{
		Logger::log_debug( _T("IStateNotifyContainer::NotifyStateChange state=%d"), state );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object

		std::list< IStateNotify<Tstate,TstateData> * >::iterator iter = m_stateNotifys.begin();
		while ( iter != m_stateNotifys.end() )
		{
			(*iter)->StateChange(state,stateData,reason);
			iter++;
		}
	}
};

// Template class for internal object held within the state machine.
// Keeps track of a state and the latest EventData
//
//  TState = expected to be an enum that represents the state
//  TeventData = class that is "new" and "deleted" that represents
//      event data that is "signaled" to the state machine to transition
//      it from state to state
//  StateData = class or struct that represents additional state data
//      that is maintained by class.  class/struct is required to have
//      operator= overload so that state data can be copied, as
//      well as a default constructor that will init default state data instance;
template <class Tstate,class TeventData,class TstateData> 
class StateItem
{
private:
	Tstate m_state;
	std::auto_ptr<TeventData> m_eventData;
	TstateData m_stateData;
	CheckThread _checkThread;

public:
	StateItem(Tstate state,TeventData* eventData)
	{
		Logger::log_trace("StateItem::StateItem this=%x state=%d eventData=%x",this,state,eventData);
		m_state = state;
		m_eventData.reset(eventData);
	}

	StateItem(Tstate state,TeventData* eventData,TstateData& stateData)
	{
		Logger::log_trace("StateItem::StateItem this=%x state=%d eventData=%x stateData",this,state,eventData);
		m_state = state;
		m_eventData.reset(eventData);
		m_stateData = stateData;
	}

	// Reset the CheckThread to current executing thread.
	// This is to check proper thread operation, 
	// e.g. do not access single thread objects from multi-threads.
	// StateHandler may be reset when added to StateMachine
	// due to it may have been created initially in separate thread.
	void ResetCheckThread()
	{ 
		Logger::log_debug(_T("StateItem::ResetCheckThread") );
		_checkThread.Reset(); 
	}

	void assignNewState(Tstate state,TeventData* eventData)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		Logger::log_debug("StateItem::assignNewState this=%x state=%d eventData=%x",this,state,eventData);
		m_state = state;
		m_eventData.reset(eventData);
	}

	void assignNewState(Tstate state,TeventData* eventData,TstateData& stateData)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		Logger::log_debug("StateItem::assignNewState this=%x state=%d eventData=%x stateData",this,state,eventData);
		m_state = state;
		m_eventData.reset(eventData);
		m_stateData = stateData;
	}

	Tstate getState()
	{	return m_state; }

	TeventData* getEventData()
	{	
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		return m_eventData.get();	
	}

	TstateData& getStateData()
	{	return m_stateData;	}
};

//*******************************************************************
//*******************************************************************

// TODO:: Change StateItem to use this??
// Simple class that maintains the association of the TState and TstateData
template <class Tstate,class TstateData> 
class StateAndStateData
{
public:
	Tstate m_state;					// considered to be an enum
	TstateData m_stateData;			// considered to be a struct item with operator= overload

	StateAndStateData() { }
	StateAndStateData(Tstate state,TstateData stateData)
	{	m_state = state; m_stateData = stateData; };

	const StateAndStateData& operator=(const StateAndStateData& stateAndStateData)
	{
		if ( &stateAndStateData == this )
			return *this;
		m_state = stateAndStateData.m_state;
		m_stateData = stateAndStateData.m_stateData;
	}
};

//*******************************************************************
//*******************************************************************

// Template class for a State Handler, an object that 
// can accept events and handle the logic from one state
// to another.	A StateHandler will keep track of the
// current state for its implementation.
//
// There are multiple StateHandlers within an instance of the StateMachine.
// The StateHandler needs to recognize the EventData that is specific
// for its state flow.
template <class Tstate,class TeventData,class TstateData> 
class StateHandler
{
protected:
	std::auto_ptr< StateItem<Tstate,TeventData,TstateData> > m_currentState;
	CheckThread _checkThread;

public:
	// Reset the CheckThread to current executing thread.
	// This is to check proper thread operation, 
	// e.g. do not access single thread objects from multi-threads.
	// StateHandler may be reset when added to StateMachine
	// due to it may have been created initially in separate thread.
	void ResetCheckThread()
	{ 
		Logger::log_debug(_T("StateHandler::ResetCheckThread") );
		_checkThread.Reset(); 
		if ( m_currentState.get() != NULL )
			m_currentState->ResetCheckThread();
	}

	virtual bool IsYourEvent(TeventData *) = 0;
	virtual bool IsStillExist() = 0;

	// Poll into the state machine. Allows update StateHandler to determine
	// if there are any changes needed.  Return true if a state
	// change or state data change did occur.
	virtual bool PollStateHandler()
	{	return false; }

	StateHandler(Tstate m_state,TeventData *eventData)
	{
		m_currentState.reset( new StateItem<Tstate,TeventData,TstateData>(m_state,eventData) );
	}

	StateHandler(Tstate m_state,TeventData *eventData,TstateData& stateData)
	{
		m_currentState.reset( new StateItem<Tstate,TeventData,TstateData>(m_state,eventData,stateData) );
	}
	
	virtual ~StateHandler() { }

	Tstate getCurrentState()
	{	return m_currentState->getState(); }

	bool IsState(Tstate state)
	{	return state == getCurrentState(); }

	TeventData* getCurrentEvent()
	{	return m_currentState->getEventData(); }

	TstateData& getCurrentStateData()
	{	return m_currentState->getStateData(); }

	void assignNewState(Tstate state,TeventData *eventData)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		m_currentState->assignNewState(state,eventData);
	}

	void assignNewState(Tstate state,TeventData *eventData,TstateData& stateData)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		m_currentState->assignNewState(state,eventData,stateData);
	}
};

//*******************************************************************
//*******************************************************************

// Main StateMachine implementation.
// Manages multiple StateHandlers.	Accepts events via the SignalEvent(EventData *)
// method.	It will ask each of the existing StateHandlers if the eventData 
// is for its state.  If not, then will call virtual UnknownEvent
// where the dervice class should return a new StateHandler for this event,
// or return NULL if not recognized.
template <class Tstate,class TeventData,class TstateData> class StateMachine
{
private:
	// List of current StateHandlers that are active
	std::list< StateHandler<Tstate,TeventData,TstateData> * > m_stateHandlers;

	// List of interaces to notify

	// Check with each StateHandler if they still are valid and exist.
	// StateHandlers may be removed during this check
	void _checkStateHandlersExist()
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
//		Logger::log_trace( _T("StateMachine::_checkStateHandlersExist") );

		// Ask each stateHandler if they are still valid
		std::list< StateHandler<Tstate,TeventData,TstateData> *>::iterator iter = m_stateHandlers.begin();
		while ( iter != m_stateHandlers.end() )
		{
//			Logger::log_trace("StateMachine::_checkStateHandlersExist this=%x stillValid=%x",this,iter);

			// It no longer exists, then remove this handler
			if ( !(*iter)->IsStillExist() )
			{
				Logger::log_debug("StateMachine::_checkStateHandlersExist this=%x notValid=%x being removed",this,iter);
				// Assume state changed, so do virtual notify
				OnStateChange( (*iter)->getCurrentState(), (*iter)->getCurrentStateData(), StateChangeReason::RemovedHandler );
				// Remove the StateHandler and iterate to next in the list
				StateHandler<Tstate,TeventData,TstateData>* evt = *iter;
				iter = m_stateHandlers.erase( iter );
				delete evt;
			}
			else
			{
				iter++;
			}
		}
	}

protected:
	CheckThread _checkThread;
	IStateNotifyContainer<Tstate,TstateData> m_iStateNotifys;

	// Virtual called if a TeventData does not belong to any of the current StateHandlers.
	// Return new StateHandler if one should be created to handle the event,
	// else return null
	virtual StateHandler<Tstate,TeventData,TstateData> *UnknownEvent(TeventData *eventData) = 0;

	// Virtual notify that either the state has changed or the state data has changed
	virtual void OnStateChange(Tstate state,TstateData& stateData,StateChangeReason::eOnStateChangeReason reason)
	{
		Logger::log_debug(_T("StateMachine::OnStateChange state=%d reason=%d"),state,reason);
		// Pass on event to any external notifies that were added
		m_iStateNotifys.NotifyStateChange(state,stateData,reason);
	}

public:

	virtual ~StateMachine() 
	{ 
		Logger::log_debug(_T("StateMachine::~StateMachine"));
		RemoveStateHandlers();
	}

	// Add a IStateNotify to be notified for a OnStateChange notify within
	// a state machine.
	// The iStateNotify instance passed is not owned by this object!
	// It should be cleared if this object is being released
	void AddIStateNotify( IStateNotify<Tstate,TstateData>* iStateNotify )
	{
		Logger::log_debug( _T("StateMachine::AddIStateNotify") );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		// Add the IStateNotify to our container
		m_iStateNotifys.AddIStateNotify( iStateNotify );
	}

	// Clear all IStateNotifys from our list to be notified
	void ClearIStateNotifys()
	{
		Logger::log_debug( _T("StateMachine::ClearIStateNotifys") );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		m_iStateNotifys.Clear();
	}

	// Add a new StateHandler to the state machine.
	// The ownership and lifetime will be passed to this state machine,
	// so be sure to allocate it using "new".  It will be deleted when
	// removed from the state machine.
	void AddStateHandler( StateHandler<Tstate,TeventData,TstateData> *pStateHandler, bool bNotifyState=false )
	{
		Logger::log_debug( _T("StateMachine::AddStateHandler handler=%x bNotifyState=%d"), pStateHandler, bNotifyState );
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		// Reset the CheckThread for the StateHandler since it now will be running under the stateMachine thread
		pStateHandler->ResetCheckThread();
		m_stateHandlers.push_back( pStateHandler );
		// If notity StateChange on new handler 
		if ( bNotifyState )
			OnStateChange( pStateHandler->getCurrentState(),  pStateHandler->getCurrentStateData(), StateChangeReason::AddedHandler );
	}

	// Remove all the state handlers
	void RemoveStateHandlers()
	{
		Logger::log_debug(_T("StateMachine::RemoveStateHandlers sz=%d"),m_stateHandlers.size());
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		std::list< StateHandler<Tstate,TeventData,TstateData> *>::iterator iter = m_stateHandlers.begin();
		while ( iter != m_stateHandlers.end() )
		{
			// Delete the state handler
			delete (*iter);
			iter++;
		}
		m_stateHandlers.clear();
	}

	// Poll all state handlers, allows them to signal change in state or state data
	virtual void PollStateHandlers()
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
//		Logger::log_trace(_T("StateMachine::PollStateHandlers enter"));
		// Go through all state handlers looking for one that owns the event
		std::list< StateHandler<Tstate,TeventData,TstateData> *>::iterator iter = m_stateHandlers.begin();
		while ( iter != m_stateHandlers.end() )
		{
//			Logger::log_trace("StateMachine::PollStateHandlers this=%x checkHandler=%x",this,iter);
			if ( (*iter)->PollStateHandler() )
			{
				Logger::log_debug("StateMachine::PollStateHandlers this=%x checkHandler=%x changed",this,iter);
				// Notify state changed
				OnStateChange( (*iter)->getCurrentState(), (*iter)->getCurrentStateData(), StateChangeReason::PollHandler );
				break;
			}
			iter++;
		}

		// Check to see if any state handlers are no longer valid and should be removed.
		_checkStateHandlersExist();

//		Logger::log_trace(_T("StateMachine::PollStateHandlers exit"));
	}

	// Returns a list of all State and StateData for all handlers in the StateMachine.
	std::list< StateAndStateData<Tstate,TstateData> > GetAllStates()
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object

		std::list< StateAndStateData<Tstate,TstateData> > ret;

		// iterate through list of active state handlers
		std::list< StateHandler<Tstate,TeventData,TstateData> * >::iterator iter = m_stateHandlers.begin();
		while ( iter != m_stateHandlers.end() )
		{
			StateHandler<Tstate,TeventData,TstateData> *pStateHandler = *iter;
			StateAndStateData<Tstate,TstateData> item( pStateHandler->getCurrentState(), pStateHandler->getCurrentStateData() );
			ret.push_back(item);
			iter++;
		}
		Logger::log_debug(_T("StateMachine::GetAllStates this=%x retsz=%d"), this, ret.size() );
		return ret;
	}

	// Signal the state machine with a new event.
	// State Machine will send to any existing state handlers.
	// If none accept the event, then will call UnknownEvent virtual
	// to see if new StateHandler should be created.
	// Will return true if event was handled.
	//    bDeleteEventData = if true and event was not accepted, then delete eventData
	bool SignalEvent(TeventData *eventData,bool bDeleteEventData=true)
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object
		Logger::log_debug("StateMachine::SignalEvent this=%x bDeleteEventData=%d eventData=%x enter", this, bDeleteEventData,eventData);
		bool bHandled=false;

		// If we got eventData, then ask each StateHandler if it it theirs
		if ( eventData != NULL )
		{
			// Go through all state handlers looking for one that owns the event
			std::list< StateHandler<Tstate,TeventData,TstateData> *>::iterator iter = m_stateHandlers.begin();
			while ( iter != m_stateHandlers.end() )
			{
				Logger::log_trace("StateMachine::SignalEvent this=%x checkHandler=%x",this,iter);
				if ( (*iter)->IsYourEvent(eventData) )
				{
					Logger::log_debug("StateMachine::SignalEvent this=%x checkHandler=%x accepted",this,iter);
					bHandled = true;
					// Assume state changed, so do virtual notify
					OnStateChange( (*iter)->getCurrentState(), (*iter)->getCurrentStateData(), StateChangeReason::IsYourEventHandled );
					break;
				}
				iter++;
			}

			// If reached the end and no owners, then call UnknownEvent to see if create a new StateHandler
			if ( iter == m_stateHandlers.end() )
			{
				Logger::log_trace("StateMachine::SignalEvent this=%x call UnknownEvent",this);

				// Call virtual and see if we create new StateHandler
				StateHandler<Tstate,TeventData,TstateData> *newHandler = UnknownEvent(eventData);
				// If handler found, then assume it now owns the event and add to our list
				if ( newHandler != NULL )
				{
					Logger::log_debug("StateMachine::SignalEvent this=%x UnknownEvent handler=%x",this,newHandler);
					bHandled = true;
					m_stateHandlers.push_back(newHandler);
					// Assume state changed, so do virtual notify
					OnStateChange( newHandler->getCurrentState(), newHandler->getCurrentStateData(), StateChangeReason::NewHandlerCreated );
				}
			}
		}

		Logger::log_debug("StateMachine::SignalEvent this=%x stillValid",this);

		// If event not handled, and specified to delete the EventData
		if ( !bHandled && bDeleteEventData && eventData != NULL )
		{
			Logger::log_debug("StateMachine::SignalEvent this=%x notHandled, delete=%x",this,eventData);
			delete eventData;
		}

		// Check to see if any state handlers are no longer valid and should be removed.
		_checkStateHandlersExist();

		Logger::log_debug("StateMachine::SignalEvent this=%x bHandled=%d exit", this, bHandled);

		// Return true if the event was handled
		return bHandled;
	}
};

#endif
