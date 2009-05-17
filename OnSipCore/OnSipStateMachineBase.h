#ifndef ONSIP_STATEMACHINE_BASE_H
#define ONSIP_STATEMACHINE_BASE_H

#include "statemachine.h"

class OnSipXmpp;
template <class Tstate,class TeventData,class TstateData>
class OnSipStateMachineBase;


// A template wrapper around a StateHanlder that has the option
// to pre-execute a XMPP operation before being added to the
// state machine.
// Virtual PreExecute() is called before this StateHandler
// is added to state machine (if using the AddStateHandler() methods)
template <class Tstate,class TeventData,class TstateData>
class StateHandlerPreExecute : public StateHandler<Tstate,TeventData,TstateData>
{
public:
	virtual bool PreExecute(OnSipStateMachineBase<Tstate,TeventData,TstateData>* pStateMachine,OnSipXmpp *pOnSipXmpp) = 0;

public:
	StateHandlerPreExecute(Tstate m_state,TeventData *eventData) : StateHandler(m_state,eventData)
	{ }

	StateHandlerPreExecute(Tstate m_state,TeventData *eventData,TstateData& stateData) : StateHandler(m_state,eventData,stateData)
	{ }
};

// Base class for all OnSip state machines
// Provides access to OnSipXmpp
template <class Tstate,class TeventData,class TstateData>
class OnSipStateMachineBase : public StateMachine<Tstate,TeventData,TstateData>
{
protected:
	OnSipXmpp* m_pOnSipXmpp;

private:

	// Internal object that holds StateHandler or StateHandlerPreExecute object.
	// A list of these are maintained when adding new StateHandlers from outside
	// of this class using AddStateHandler()
	class _stateHandleRequestItem
	{
	private:
		StateHandler<Tstate,TeventData,TstateData>* m_pStateHandler;
		StateHandlerPreExecute<Tstate,TeventData,TstateData>* m_pStateHandlerEx;
	public:

		_stateHandleRequestItem(StateHandler<Tstate,TeventData,TstateData>* pStateHandler)
		{	m_pStateHandler = pStateHandler; m_pStateHandlerEx = NULL; }

		_stateHandleRequestItem(StateHandlerPreExecute<Tstate,TeventData,TstateData>* pStateHandler)
		{	m_pStateHandler = pStateHandler; m_pStateHandlerEx = pStateHandler;	}

		// Returns true if this instance is holding a StateHandlerPreExecute instance
		bool IsPreExecute()
		{	return m_pStateHandlerEx != NULL; }

		StateHandler<Tstate,TeventData,TstateData>* getStateHandler()
		{	return m_pStateHandler;	}

		StateHandlerPreExecute<Tstate,TeventData,TstateData>* getStateHandlerPreExecute()
		{	return m_pStateHandlerEx;	}
	};

	// List of the either StateHandler or StateHandlerPreExecute instances
	std::list<_stateHandleRequestItem *> m_handlerRequests;
	CriticalSection m_cs;
	volatile int m_requestChange;

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
	void AddStateHandler( StateHandler<Tstate,TeventData,TstateData> *pStateHandler )
	{
		CriticalSectionScope css(&m_cs);
		m_handlerRequests.push_back( new _stateHandleRequestItem(pStateHandler) );
		// Signify handler requests have been changed
		m_requestChange++;
	}

	// THREAD-SAFE
	//
	// Add a new StateHandler to the state machine.
	// The ownership and lifetime will be passed to this state machine,
	// so be sure to allocate it using "new".  It will be deleted when
	// removed from the state machine.
	void AddStateHandler( StateHandlerPreExecute<Tstate,TeventData,TstateData> *pStateHandler )
	{
		CriticalSectionScope css(&m_cs);
		m_handlerRequests.push_back( new _stateHandleRequestItem(pStateHandler) );
		// Signify handler requests have been changed
		m_requestChange++;
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

				// If type of StateHandlerPreExecute,
				// then execute its request before being added to the state machine
				bool bPreExecute=true;
				if ( pRequest->IsPreExecute() )
				{
					Logger::log_debug(_T("OnSipStateMachineBase::PollStateHandlers do PreExecute"));
					// Allow the StateHandler to do an execute first
					bPreExecute = pRequest->getStateHandlerPreExecute()->PreExecute(this,m_pOnSipXmpp);
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
					delete pRequest->getStateHandler();

				// Delete the StateHandlerRequest
				delete pRequest;

				// Remove this list item and do next
				iter = lstRequests.erase(iter);
			}
		}

		// Call default polling
		StateMachine::PollStateHandlers();
	}

	OnSipStateMachineBase(OnSipXmpp* pOnSipXmpp)
	{
		m_pOnSipXmpp = pOnSipXmpp;
		m_requestChange = 0;
	}

	// TODO: Destructor to clear any pending m_handlerRequests??
};

#endif