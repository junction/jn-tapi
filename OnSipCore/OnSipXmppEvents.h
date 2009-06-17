#ifndef XMPPEVENTS_H
#define XMPPEVENTS_H

#include "onsip.h"

// Base class for Xmpp Events that are OnSip related
// Wraps events and used in the state machine to track calls.
class XmppEvent
{
private:
	bool m_bError;

protected:
	CheckThread _checkThread;

public:
	enum XmppEvtType { 
			EVT_PUBSUB_AUTH, 
			EVT_PUBSUB_SUBSCRIBE, 
			EVT_PUBSUB_ACTIVECALL_EVENT, 
			EVT_PUBSUB_RETRACTCALL_EVENT,
			EVT_ONCONNECT,
			EVT_ONDISCONNECT,
			EVT_IQ_RESULT,			// Standard IQ Result
			EVT_DROP_REQUEST,			// Non Xmpp, but event used to enter into call state machine
			EVT_SHUTDOWN_REQUEST	// Non Xmpp, but event used to shutdown the Init state machine
		};

	XmppEvtType m_type;
	JID m_to;
	JID m_from;
	Tag *m_tag;
	tstring m_error;
	int m_errorCode;
	int m_context;
	tstring m_id;

	XmppEvent(const XmppEvtType type,const string& id,const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0);

	// Returns true if is an error
	bool IsError()
	{	return m_bError; }

	virtual tstring ToString();
	virtual ~XmppEvent();
};

//*************************************************************************
//*************************************************************************

class XmppEventFactory
{
private:
	// Force use of static methods
	XmppEventFactory() { }
public:
	static XmppEvent* iqFactory(const IQ& iq,int context);

	static XmppEvent* msgFactory(const Message& msg);

	// Called for OnConnect and OnDisconnect()
	static XmppEvent* connectFactory(bool bConnect);
};

//*************************************************************************
//*************************************************************************

// Id response after authenticating with the OnSIP PBX
class XmppAuthEvent : public XmppEvent
{
public:
	// TODO: convert to time value
	string expireDate;

	XmppAuthEvent(const string& id,const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0) : XmppEvent( XmppEvent::EVT_PUBSUB_AUTH, id, to, from, tag, error, context)
	{ }

	virtual string ToString();
	static XmppAuthEvent* checkEvent(const IQ& iq,int context);
};

// Standard IQ Result or Error Event, used if no other events are recognized for the IQ Result
class XmppIqResultEvent : public XmppEvent
{
public:
	XmppIqResultEvent(const string& id,const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0) : XmppEvent( XmppEvent::EVT_IQ_RESULT, id, to, from, tag, error, context)
	{ }

	virtual string ToString();
	static XmppIqResultEvent* checkEvent(const IQ& iq,int context);
};

// message that comes back after doing a PubSub scribe
// Message is in form of	message/event/subscribed
class XmppPubSubSubscribedEvent : public XmppEvent
{
public:
	string m_subscription;
	string m_node;
	string m_jid;
	bool m_bSubscribed;
	string m_subid;

	XmppPubSubSubscribedEvent(const string& id, const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0) : XmppEvent( XmppEvent::EVT_PUBSUB_SUBSCRIBE, id, to, from, tag, error, context)
	{ }

	virtual string ToString();
	static XmppPubSubSubscribedEvent* checkEvent(const Message& msg);
};

// message that comes with active-call state events
// Message is in form of	message/event/items/item/active-call
class XmppActiveCallEvent : public XmppEvent
{
public:
	enum CallDialogState { UNKNOWN, CREATED, CONFIRMED, REQUESTED };

	CallDialogState m_dialogState;
	string m_to_aor;
	string m_sipCallid;
	string m_from_uri;
	string m_to_uri;
	string m_from_tag;
	string m_to_tag;
	string m_branch;

	XmppActiveCallEvent(const string& id, const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0) : XmppEvent( XmppEvent::EVT_PUBSUB_ACTIVECALL_EVENT, id, to, from, tag, error, context)
	{ }
	virtual string ToString();
	static XmppActiveCallEvent* checkEvent(const Message& msg);

	// Helper method to converted a XmppActiveCallEvent::CallState enum to a displayable callstate value.
	// This does not have to match the string values in the dialog-state value
	static TCHAR * DialogStateToString( CallDialogState state );
private:

	// Helper method to convert a dialog-state callstate value into a CallState enum
	static CallDialogState _convertDialogState( const string& szCallState );
};

// message that comes after a call is disconnected
// Message is in form of	message/event/items/retract
class XmppRetractCallEvent : public XmppEvent
{
public:

	XmppRetractCallEvent(const string& id, const JID& to,const JID& from,const Tag *tag,const Error* error=NULL,int context=0) : XmppEvent( XmppEvent::EVT_PUBSUB_RETRACTCALL_EVENT, id, to, from, tag, error, context)
	{ }

	virtual string ToString();
	static XmppRetractCallEvent* checkEvent(const Message& msg);
};

// XmppEvent created when virtual OnConnect is called due to successful connection to server
// All the Xmpp related values really do not matter for this, just the type, e.g. EVT_ONCONNECT
class XmppOnConnectEvent : public XmppEvent
{
public:
	XmppOnConnectEvent() : XmppEvent( XmppEvent::EVT_ONCONNECT, EmptyString, JID(), JID(), NULL, NULL, 0)
	{ }
};

// XmppEvent created when virtual OnDisconnect is called due to successful connection to server
// All the Xmpp related values really do not matter for this, just the type, e.g. EVT_ONDISCONNECT
class XmppOnDisconnectEvent : public XmppEvent
{
public:
	XmppOnDisconnectEvent() : XmppEvent( XmppEvent::EVT_ONDISCONNECT, EmptyString, JID(), JID(), NULL, NULL, 0)
	{ }
};

#endif
