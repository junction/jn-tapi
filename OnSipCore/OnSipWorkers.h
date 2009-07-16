#ifndef ONSIPWORKERS_H
#define ONSIPWORKERS_H

#include "onsipxmppbase.h"
#include "onsipcallstatemachine.h"
#include "onsipinitstatemachine.h"
#include "utils.h"

class OnSipWorker
{
protected:
	OnSipXmppBase* _onSipXmpp;
	TimeElapsed timer;		// Keep track of how long been around

	OnSipWorker( OnSipXmppBase* onSipXmpp )
	{	
		_onSipXmpp = onSipXmpp;	
		timer.Reset();
	}

public:
	virtual ~OnSipWorker() { }

	// Are we complete
	virtual bool IsComplete() = 0;
	// Error
	virtual bool IsError() = 0;

	// How long have we been created
	long Msecs()
	{	return timer.Msecs(); }

};

/*

// Worker that retrieves all subscriptions, and then
// attempts to unsubscribe each of them
//
// TODO!!  SHOULD WE REALLY BE USING THIS???  Can't we unsubscribe other clients!!
class UnsubscriberAllWorker : public PubSubResultHandlerBase, OnSipWorker, IqHandlerBase 
{
private:
	unsigned _count;
	bool _bSent;

public:
	UnsubscriberAllWorker( OnSipXmpp* onSipXmpp ) : OnSipWorker( onSipXmpp ) 
	{	_count =0;	_bSent = false ; }

	// Execute the UnsubscriberAll worker
	void Execute();

	virtual bool IsComplete();

	virtual bool IsError()
	{	return false; }

protected:

	virtual void handleSubscriptions( const std::string& id,
		const JID& service,
		const PubSub::SubscriptionMap& subMap,
		const Error* error);

	virtual void handleIqID( const IQ& iq, int context );

};

// Worker that retrieves all subscriptions, and then
// attempts to unsubscribe each of them
class UnsubscriberWorker : public OnSipWorker, IqHandlerBase 
{
private:
	bool _bComplete;
	bool _bError;
public:
	UnsubscriberWorker( OnSipXmpp* onSipXmpp ) : OnSipWorker( onSipXmpp ) 
	{ _bComplete = false; _bError = false; }

	// Execute the Unsubscriber worker
	void Execute(string subId);

	virtual bool IsComplete()
	{	return _bComplete;	}

	virtual bool IsError()
	{	return _bError;	}

protected:

	virtual void handleIqID( const IQ& iq, int context );
};
*/

// Worker that retrieves all subscriptions, and then
// attempts to unsubscribe each of them
class AuthorizeWorker : public OnSipWorker, IqHandlerBase 
{
private:
	bool _bComplete;
	bool _bError;
	string _expireDate;

public:
	AuthorizeWorker( OnSipXmppBase* onSipXmpp ) : OnSipWorker( onSipXmpp ) 
	{	_bComplete = false; _bError = false;	}

	void Execute();

	//virtual 
	virtual bool IsComplete()
	{	return _bComplete;	}

	//virtual 
	virtual bool IsError()
	{	return _bError;	}

	string ExpireDate()
	{	return _expireDate; }

protected:
	virtual void handleIqID( const IQ& iq, int context );
};

// Worker that retrieves all subscriptions, and then
// attempts to unsubscribe each of them
class EnableCallEventsWorker : public OnSipWorker, IqHandlerBase, PubSubResultHandlerBase, MessageHandler
{
private:
	bool _bComplete;
	bool _bError;
	bool _bRegister;
	string _subid;

	void _unregister();

public:
	EnableCallEventsWorker( OnSipXmppBase* onSipXmpp ) : OnSipWorker( onSipXmpp ) 
	{	_bComplete = false; _bError = false; _bRegister=false;	}

	virtual ~EnableCallEventsWorker()
	{	_unregister();	};

	void Execute(const string& expireTime);

	virtual bool IsComplete()
	{	return _bComplete;	}

	virtual bool IsError()
	{	return _bError;	}

	string subId()
	{	return _subid; }

protected:
	virtual void EnableCallEventsWorker::handleMessage( const Message& msg, MessageSession* session	) ;

};

// Worker that will re-authorize, subscribe to call events,
// and unsubscribe previous
class ReAuthorizeSubscribeWorker : public OnSipWorker
{
private:
//	std::auto_ptr<UnsubscriberWorker> _unsubscriber;
	std::auto_ptr<AuthorizeWorker> _authorizer;
	std::auto_ptr<EnableCallEventsWorker> _subscriber;
	bool _bComplete;
	bool _bError;
//	string _oldSubId;
	string _subid;

	enum STATE { None, Authorizing, Subscribing, /*Unsubscribing */ };
	STATE _state;

public:
	ReAuthorizeSubscribeWorker( OnSipXmppBase* onSipXmpp ) : OnSipWorker( onSipXmpp ) 
	{	_bComplete = false; _bError = false; _state = None; }

//	void Execute(const string& oldSubId);
	void Execute();

	virtual bool IsComplete();

	virtual bool IsError()
	{	return _bError;	}

	string subId()
	{	return _subid; }
};


#endif