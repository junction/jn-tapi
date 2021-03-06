#ifndef ONSIP_STATEMACHINE_BASE_H
#define ONSIP_STATEMACHINE_BASE_H

#include "statemachine.h"

class OnSipXmpp;
template <class Tstate,class TeventData,class TstateData>
class OnSipStateMachineBase;

// Base State Handler for all OnSip state machines
template <class Tstate,class TeventData,class TstateData>
class OnSipStateHandlerBase : public StateHandler<Tstate,TeventData,TstateData>
{
private:
	bool m_bHasPreExecute;

protected:
	// Set by derived class this StateHandler implements the IPreExecute interface
	void SetHasPreExecute(bool bHasPreExecute)
	{	m_bHasPreExecute = bHasPreExecute; }

public:
	// Virtual that a StateHandler can take over to be called before being added to the state machine.
	// Has access to the OnSipXmpp
	// The SetHasPreExecute(true) must be called by the derived class to enable this virtual.
	virtual bool PreExecute(OnSipStateMachineBase<Tstate,TeventData,TstateData>* /*pStateMachine*/,OnSipXmpp * /*pOnSipXmpp*/)
	{
		// Should never make it here, it should be overridden by derived class
		_ASSERTE(false);
		return false;
	}

	OnSipStateHandlerBase(Tstate m_state,TeventData *eventData) : StateHandler(m_state,eventData)
	{	m_bHasPreExecute = false;	}

	OnSipStateHandlerBase(Tstate m_state,TeventData *eventData,TstateData& stateData) : StateHandler(m_state,eventData,stateData)
	{	m_bHasPreExecute = false;	}

	// Returns true if the derived class takes over the PreExecute virtual
	bool HasPreExecute()
	{ return m_bHasPreExecute; }
};

// Base class for all OnSip state machines
// Provides access to OnSipXmpp
template <class Tstate,class TeventData,class TstateData>
class OnSipStateMachineBase : public StateMachine<Tstate,TeventData,TstateData>
{
protected:
	OnSipXmpp* m_pOnSipXmpp;

private:

	// Internal object that holds OnSipStateHandlerBase object.
	// A list of these are maintained when adding new StateHandlers from outside
	// of this class using AddStateHandler()
	class _stateHandleRequestItem
	{
	private:
		OnSipStateHandlerBase<Tstate,TeventData,TstateData>* m_pStateHandler;
	public:

		_stateHandleRequestItem(OnSipStateHandlerBase<Tstate,TeventData,TstateData>* pStateHandler)
		{	m_pStateHandler = pStateHandler; }

		// Returns true if this instance is holding a StateHandler that takes over the PreExecute virtual
		bool IsPreExecute()
		{	return m_pStateHandler->HasPreExecute(); }

		OnSipStateHandlerBase<Tstate,TeventData,TstateData>* getStateHandler()
		{	return m_pStateHandler;	}
	};

	CriticalSection m_cs;

	// List of XmppEvents to be added to the state machine
	std::list<XmppEvent *> m_events;
	volatile int m_eventChange;

	// List of OnSipStateHandlerBase instances to be added to the state machine
	std::list<_stateHandleRequestItem *> m_handlerRequests;
	volatile int m_requestChange;

	// THREAD-SAFE
	//
	// Retrieves all the pending events to be added to 
	// the state machine via the thread-safe AddEvent.
	// This is a thread-safe method that will return all
	// events from m_events and clear it.
	std::list<TeventData*> _getEvents()
	{
		std::list<TeventData*> lst;

		if ( m_eventChange > 0 )
		{
			// Grab thread safety and copy list
			// so list can be processed safely in its own time
			CriticalSectionScope css(&m_cs);
			Logger::log_debug("OnSipStateMachineBase::_getEvents size=%d", m_events.size() );
			lst = m_events;
			m_events.clear();
			m_eventChange=0;
		}
		return lst;
	}

	// THREAD-SAFE
	//
	// Retrieves all the pending stateHandler requests
	// that have been requested to be added to the state machine.
	// This is a thread-safe method that will return all
	// from the m_handlerRequests list and then clear it.
	std::list<_stateHandleRequestItem*> _getRequests()
	{
		std::list<_stateHandleRequestItem*> lst;

		// See if we have a pending StateHandlerRequests to be added
		// Just checks the volatile int flag, not truely thread-safe
		// but should be fine since we have thread safety below,
		// and it misses one poll, no biggy.
		if ( m_requestChange > 0 )
		{
			// Grab thread safety and copy list
			// so list can be processed safely in its own time
			CriticalSectionScope css(&m_cs);
			Logger::log_debug("OnSipStateMachineBase::_getRequests size=%d", m_handlerRequests.size() );
			lst = m_handlerRequests;
			m_handlerRequests.clear();
			m_requestChange=0;
		}
		return lst;
	}


public:

	// THREAD-SAFE
	//
	// Add a new StateHandler to the state machine.
	// The ownership and lifetime will be passed to this state machine,
	// so be sure to allocate it using "new".  It will be deleted when
	// removed from the state machine.
	void AddStateHandler( OnSipStateHandlerBase<Tstate,TeventData,TstateData> *pStateHandler )
	{
		CriticalSectionScope css(&m_cs);
		m_handlerRequests.push_back( new _stateHandleRequestItem(pStateHandler) );
		// Signify handler requests have been changed
		m_requestChange++;
	}

	// THREAD-SAFE
	//
	// Add an event to the state machine.
	// The ownership and lifetime will be passed to this state machine,
	// so be sure to allocate it using "new".  It will be deleted when
	// removed from the state machine.
	void AddEvent( TeventData *pEvent )
	{
		Logger::log_debug(_T("AddEvent") );
		CriticalSectionScope css(&m_cs);
		m_events.push_back( pEvent );
		// Signify events list has been changed
		m_eventChange++;
	}

	// Poll all state handlers, allows them to signal change in state or state data
	virtual void PollStateHandlers()
	{
		_checkThread.CheckSameThread();	// Verify we are single threaded for this object

		// See if we have a pending StateHandlerRequests to be added
		// Just checks the volatile int flag, no truely thread-safe
		// but should be fine since we have thread safety below
		std::list<_stateHandleRequestItem*> lstRequests = _getRequests();
		if ( lstRequests.size() > 0 )
		{
			std::list<_stateHandleRequestItem*>::iterator iter = lstRequests.begin();
			while ( iter != lstRequests.end() )
			{
				_stateHandleRequestItem* pRequest = (*iter);
				Logger::log_debug(_T("OnSipStateMachineBase::PollStateHandlers handling request"));

				// Reset the CheckThread on the StateHandler since it is now part of the state machine
				pRequest->getStateHandler()->ResetCheckThread();

				// If takes over the PreExecute virtual
				// then execute its request before being added to the state machine
				bool bPreExecute=true;
				if ( pRequest->IsPreExecute() )
				{
					Logger::log_debug(_T("OnSipStateMachineBase::PollStateHandlers do PreExecute"));
					// Allow the StateHandler to do an execute first
					bPreExecute = pRequest->getStateHandler()->PreExecute( this, m_pOnSipXmpp );
				}

				// If PreExecute failed
				if ( !bPreExecute )
				{
					Logger::log_error(_T("OnSipStateMachineBase::PollStateHandlers PreExecute failed"));
					// Delete the StateHandler since it will not be added to the state machine
					delete pRequest->getStateHandler();
				}
				// else if StateHandler is still good, add it 
				else if ( pRequest->getStateHandler()->IsStillExist() )
				{
					// Add and pass off ownership of the StateHandler to the StateMachine
					StateMachine::AddStateHandler( pRequest->getStateHandler(), pRequest->IsPreExecute() );
				}
				// else do not add the state handler
				else
				{
					Logger::log_debug(_T("OnSipStateMachineBase::PollStateHandlers request IsStillExist is false, handler not added"));
					delete pRequest->getStateHandler();
				}

				// Delete the StateHandlerRequest
				delete pRequest;

				// Remove this list item and do next
				iter = lstRequests.erase(iter);
			}
		}

		// See if we have any pending events to be added to the state machine
		// See if we have a pending StateHandlerRequests to be added
		// Just checks the volatile int flag, no truely thread-safe
		// but should be fine since we have thread safety below
		std::list<TeventData*> lstEvents = _getEvents();
		if ( lstEvents.size() > 0 )
		{
			Logger::log_debug(_T("OnSipStateMachineBase::PollStateHandlers addEvents %d"), lstEvents.size() );
			for ( std::list<TeventData*>::iterator iter = lstEvents.begin(); iter != lstEvents.end(); iter++ )
				this->SignalEvent(*iter);
		}

		// Call default polling
		StateMachine::PollStateHandlers();
	}

	OnSipStateMachineBase(OnSipXmpp* pOnSipXmpp)
	{
		m_pOnSipXmpp = pOnSipXmpp;
		m_requestChange = 0;
		m_eventChange = 0;
	}

	// TODO: Destructor to clear any pending m_handlerRequests??
};

#endif
