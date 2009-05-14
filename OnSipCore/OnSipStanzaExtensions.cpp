#include "stdafx.h"
#include "onsipstanzaextensions.h"


// StanzaExtension to recognize the Message/event/subscribed
// XMPP event.  This is nnew type of pubSub subscribe authorized
// model, which currently is not supported in gloox.
// Created our own extension to recognize this event,
// used when subscribing to events such as active-call events.
PubSubSubscribedExt::PubSubSubscribedExt( const Tag* tag ) : StanzaExtension(ONSIP_STANZAEXT_PUBSUB_SUBSCRIBED)
{
	if ( tag != 0 )
		m_tag = tag->clone();
	else
		m_tag = NULL;
	_parse(tag);
}

/**
* Virtual destructor.
*/
PubSubSubscribedExt::~PubSubSubscribedExt()
{
	if ( m_tag != NULL )
		delete m_tag;
}

// reimplemented from StanzaExtension
const std::string& PubSubSubscribedExt::filterString() const
{
	static const std::string filter = "/message/event/subscription";
	return filter;
}

// reimplemented from StanzaExtension
// virtual
Tag* PubSubSubscribedExt::tag() const
{
	// TODO: ideally, I think the tag is supposed to be re-generated
	// using the values, and not relying on the tag that was passed
	// since it may contain other un-related child nodes
	return (m_tag == NULL) ? NULL : m_tag->clone();
}

bool PubSubSubscribedExt::_parse( const Tag* tag )
{
//		if( !tag || tag->xmlns() != XMLNS_X_DATA || tag->name() != "x" )
	if( !tag || tag->name() != "subscription" )
		return false;

	m_node = tag->findAttribute( "node" );
	m_jid = tag->findAttribute("jid");
	m_subsciption = tag->findAttribute("subscription");
	return true;
}

