#include <stdafx.h>
#include "onsipxmppevents.h"
#include "Utils.h"
#include "onsip.h"
#include "onsipstanzaextensions.h"
#include "glooxHelper.h"

#include "error.h"
#include "adhoc.h"
#include "stanza.h"
#include "message.h"
#include "dataform.h"
#include "iq.h"
#include "adhoc.h"
#include "adhochandler.h"
#include "pubsub.h"
#include "pubsubmanager.h"
#include "pubsubresulthandler.h"
#include "pubsubevent.h"
#include "subscription.h"

#include "Logger.h"

//***************************************************************************
//***************************************************************************

//TODO::Add tracking to see if we new/delete all XmppEvent instances

XmppEvent::XmppEvent(const XmppEvtType type,const string& id,const JID& to,const JID& from,const Tag *tag,const Error* error,int context)
{
	Logger::log_trace("XmppEvent::XmppEvent:: type=%d this=%x", type, this );

	m_id = id;
	m_tag = (tag != NULL) ? tag->clone() : NULL;
	m_type = type; m_to = to; m_from = from;
	m_context = context;
	m_errorCode = 0;
	m_bError = false;
	
	if ( error != NULL )
	{
		Logger::log_error("XmppEvent %d %s\n\n%s",error->error(), error->text().c_str(), tag->xml().c_str() );
		m_error = error->text();
		m_errorCode = (int) error->error();
		m_bError = true;
	}
}

XmppEvent::~XmppEvent() 
{
	Logger::log_trace("XmppEvent::~XmppEvent:: type=%d this=%x", m_type, this );
}

string XmppEvent::ToString()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	return Strings::stringFormat("XmppEvent type=%d to=%s from=%s err=%s errCode=%d context=%d id=%s",
		m_type,m_to.full().c_str(),m_from.full().c_str(),m_error.c_str(),m_errorCode,m_context,m_id.c_str());
}

//***************************************************************************
//***************************************************************************

// static
XmppEvent* XmppEventFactory::iqFactory(const IQ& iq,int context)
{
	XmppEvent* evt = XmppAuthEvent::checkEvent(iq,context);
	if ( evt != NULL )
		return evt;
	evt = XmppCallRequestEvent::checkEvent(iq,context);
	if ( evt != NULL )
		return evt;
	evt = XmppIqResultEvent::checkEvent(iq,context);
	if ( evt != NULL )
		return evt;
	return evt;
}

// static
XmppEvent* XmppEventFactory::msgFactory(const Message& msg)
{
	XmppEvent* evt = XmppPubSubSubscribedEvent::checkEvent(msg);
	if ( evt != NULL )
		return evt;
	evt = XmppActiveCallEvent::checkEvent(msg);
	if ( evt != NULL )
		return evt;
	evt = XmppRetractCallEvent::checkEvent(msg);
	if ( evt != NULL )
		return evt;
	return evt;
}

// Called for OnConnect and OnDisconnect()
//static 
XmppEvent* XmppEventFactory::connectFactory(bool bConnect)
{
	Logger::log_debug("XmppEventFactory::connectFactory bConnect=%d",bConnect);
	if ( bConnect )
		return new XmppOnConnectEvent();
	return new XmppOnDisconnectEvent();
}

//***************************************************************************
//***************************************************************************

string XmppAuthEvent::ToString()
{
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppAuthEvent expireDate=%s\r\n<%s>",expireDate.c_str(), str.c_str() );
}

// Checks the IQ event to see if it is the OnSIP Authorization result event
// static
XmppAuthEvent* XmppAuthEvent::checkEvent(const IQ& iq,int context)
{
	// See if this is an IQ result responses for an authorize IQ SET
	// This occurs after requesting to enable active call events
	const Adhoc::Command* cmd = iq.findExtension<Adhoc::Command>(ExtAdhocCommand);
	if ( cmd != NULL && (iq.subtype() == IQ::Result || iq.subtype() == IQ::Error) )
	{
		const DataForm* frm = cmd->form();
		if ( frm != NULL && cmd->node() == ONSIP_AUTH_NODE )
		{
			// If an error
			if ( iq.error() != NULL )
				return new XmppAuthEvent( iq.id(), iq.to(), iq.from(), iq.tag(), iq.error(), context );

			// If not an error
			XmppAuthEvent* evt = new XmppAuthEvent( iq.id(), iq.to(), iq.from(), iq.tag(), NULL, context );
			DataFormField* field = frm->field( "expires" );
			_ASSERT( field != NULL );
			evt->expireDate = field->value();
			return evt;
		}
	}
	return NULL;
}

//***************************************************************************
//***************************************************************************

string XmppCallRequestEvent::ToString()
{
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppCallRequestEvent callSetupId=%s\r\n<%s>",call_setup_id.c_str(), str.c_str() );
}

// Checks the IQ event to see if it is the OnSIP Authorization result event
// static
XmppCallRequestEvent* XmppCallRequestEvent::checkEvent(const IQ& iq,int context)
{
	// See if this is an IQ result responses for a make call IQ request
	// This occurs after requesting to make a call.
	// The IQ result contains the call-setup-id that can be used to synchronize the  message event
	// with the make call request
	const Adhoc::Command* cmd = iq.findExtension<Adhoc::Command>(ExtAdhocCommand);
	if ( cmd != NULL && (iq.subtype() == IQ::Result || iq.subtype() == IQ::Error) )
	{
		const DataForm* frm = cmd->form();
		if ( frm != NULL && cmd->node() == "create" && iq.from() == ONSIP_ACTIVECALLS_COMMAND )
		{
			// If an error
			if ( iq.error() != NULL )
				return new XmppCallRequestEvent( iq.id(), iq.to(), iq.from(), iq.tag(), iq.error(), context );

			// If not an error
			XmppCallRequestEvent* evt = new XmppCallRequestEvent( iq.id(), iq.to(), iq.from(), iq.tag(), NULL, context );
			DataFormField* field = frm->field( "call-setup-id" );
			_ASSERT( field != NULL );
			evt->call_setup_id = field->value();
			return evt;
		}
	}
	return NULL;
}

//***************************************************************************
//***************************************************************************

string XmppIqResultEvent::ToString()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppIqResultEvent\r\n<%s>", str.c_str() );
}

// Checks the IQ event to see if it a standard IQ result or error event
// static
XmppIqResultEvent* XmppIqResultEvent::checkEvent(const IQ& iq,int context)
{
	// See if Result or Error event
	if ( (iq.subtype() == IQ::Result || iq.subtype() == IQ::Error) )
	{
		XmppIqResultEvent *evt = new XmppIqResultEvent( iq.id(), iq.to(), iq.from(), iq.tag(), iq.error(), context );
		return evt;
	}
	return NULL;
}

//***************************************************************************
//***************************************************************************

// message that comes back after doing a PubSub scribe
string XmppPubSubSubscribedEvent::ToString()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppPubSubSubscribedEvent m_subscr=%s m_node=%s m_jid=%s bSubsc=%d\r\n<%s>",
		m_subscription.c_str(), m_node.c_str(), m_jid.c_str(), m_bSubscribed , str.c_str() );
}

// static
XmppPubSubSubscribedEvent* XmppPubSubSubscribedEvent::checkEvent(const Message& msg)
{
	// See if this message contains the message event with our subscription status
	const PubSubSubscribedExt* pss = msg.findExtension<PubSubSubscribedExt>(ONSIP_STANZAEXT_PUBSUB_SUBSCRIBED);
	if ( pss == NULL )
		return NULL;

	// TODO:: Is the error associated with the Message or with the event??
	XmppPubSubSubscribedEvent* evt = new XmppPubSubSubscribedEvent( msg.id(), msg.to(), msg.from(), pss->tag(), msg.error() );
	evt->m_subscription = pss->subscription();
	evt->m_node = pss->node();
	evt->m_subid = pss->tag()->findAttribute("subid");
	evt->m_bSubscribed = evt->m_subscription == "subscribed";
	evt->m_jid = pss->jid();
	return evt;
}

//***************************************************************************
//***************************************************************************

string XmppActiveCallEvent::ToString()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppActiveCallEvent state=%s toaor=%s callid=%s fromUri=%s toUri=%s fromTag=%s toTag=%s callSetupId=%s branch=%s\r\n<%s>",
		XmppActiveCallEvent::DialogStateToString(m_dialogState), m_to_aor.c_str(), m_sipCallid.c_str(), m_from_uri.c_str(), 
		m_to_uri.c_str(), m_from_tag.c_str(), m_to_tag.c_str(), m_call_setup_id.c_str(), m_branch.c_str(), str.c_str() );
}

// Helper method to convert a dialog-state callstate value into a CallDialogState enum
// static
XmppActiveCallEvent::CallDialogState XmppActiveCallEvent::_convertDialogState( const tstring& szCallState )
{
	if ( szCallState == _T("created") )
		return CREATED;
	if ( szCallState == _T("confirmed") )
		return CONFIRMED;
	if ( szCallState == _T("requested") )
		return REQUESTED;
	Logger::log_warn(_T("XmppActiveCallEvent::CallState UNKNOWN %s"),szCallState.c_str());
	return UNKNOWN;
}

// Helper method to converted a DialogState enum to a displayable value.
// This does not have to match the string values in the dialog-state value
// static
TCHAR * XmppActiveCallEvent::DialogStateToString( CallDialogState state )
{
	switch (state)
	{
		case CREATED:
			return _T("created");
		case CONFIRMED:
			return _T("confirmed");
		case REQUESTED:	
			return _T("requested");
		case UNKNOWN:
			return _T("unknown");
		default:
		{
			Logger::log_warn(_T("XmppActiveCallEvent::DialogStatetoString unsupported state %d"),state);
			return _T("unknown??");
		}
	}
}

// static
XmppActiveCallEvent* XmppActiveCallEvent::checkEvent(const Message& msg)
{
	// See if this message contains the message event with our subscription status
	const PubSub::Event* pse = msg.findExtension<PubSub::Event>(ExtPubSubEvent);
	if ( pse == NULL )
		return NULL;

	// See if this appears to be an active-call event
	if ( pse->items().size() != 1 || pse->type() != PubSub::EventItems || msg.from() != ONSIP_PUBSUB_ACTIVECALLS )
		return NULL;

	// Get the first ItemOperation in the container
	PubSub::Event::ItemOperationList::const_iterator iter = pse->items().begin();
	PubSub::Event::ItemOperation* item = *iter;

	// See if item contains an "active-call"
	const Tag* activeCall = item->payload->findChild("active-call","xmlns","onsip:active-calls");
	if ( activeCall == NULL )
		return NULL;

	XmppActiveCallEvent* ce = new XmppActiveCallEvent( item->item, msg.to(), msg.from(), msg.tag(), msg.error() );

	// Get the child node values
	ce->m_dialogState = _convertDialogState( TagHelper::getChildText(activeCall,"dialog-state") );
	ce->m_to_aor  = TagHelper::getChildText(activeCall,"to-aor");
	ce->m_sipCallid  = TagHelper::getChildText(activeCall,"call-id");
	ce->m_from_uri	= TagHelper::getChildText(activeCall,"from-uri");
	ce->m_to_uri  = TagHelper::getChildText(activeCall,"to-uri");
	ce->m_from_tag	= TagHelper::getChildText(activeCall,"from-tag");
	ce->m_to_tag  = TagHelper::getChildText(activeCall,"to-tag");
	ce->m_branch  = TagHelper::getChildText(activeCall,"branch");
	ce->m_call_setup_id = TagHelper::getChildText(activeCall,"call-setup-id");
	return ce;
}

//***************************************************************************
//***************************************************************************

string XmppRetractCallEvent::ToString()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	// Get our base string
	string str = XmppEvent::ToString();
	return Strings::stringFormat("XmppRetractCallEvent <%s>", str.c_str() );
}

// static
XmppRetractCallEvent* XmppRetractCallEvent::checkEvent(const Message& msg)
{
	// See if this message contains the message event with our subscription status
	const PubSub::Event* pse = msg.findExtension<PubSub::Event>(ExtPubSubEvent);
	if ( pse == NULL )
		return NULL;

	// See if this appears to be a retractactive-call event
	if ( pse->items().size() != 1 || pse->type() != PubSub::EventItems || msg.from() != ONSIP_PUBSUB_ACTIVECALLS )
		return NULL;

	// Get the first ItemOperation in the container
	PubSub::Event::ItemOperationList::const_iterator iter = pse->items().begin();
	PubSub::Event::ItemOperation* item = *iter;

	// See if item is a retract
	if ( !item->retract )
		return NULL;

	XmppRetractCallEvent* re = new XmppRetractCallEvent( item->item, msg.to(), msg.from(), msg.tag(), msg.error() );
	return re;
}


