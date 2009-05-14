#ifndef ONSIP_STANZAEXTENSIONS_H
#define ONSIP_STANZAEXTENSIONS_H

#include "onsip.h"

// OnSip StanzaExtension types
#define ONSIP_STANZAEXT_PUBSUB_SUBSCRIBED	5000			// message/event/subscription => message reply on successfuly subsciption for active calls notify


// StanzaExtension to recognize the Message/event/subscribed
// XMPP event.  This is nnew type of pubSub subscribe authorized
// model, which currently is not supported in gloox.
// Created our own extension to recognize this event,
// used when subscribing to events such as active-call events.
class PubSubSubscribedExt : public StanzaExtension
{
public:

	/**
	* Creates a new Error object from the given Tag.
	* @param tag The Tag to parse.
	*/
	PubSubSubscribedExt( const Tag* tag = 0 );

	/**
	* Virtual destructor.
	*/
	virtual ~PubSubSubscribedExt();

	// reimplemented from StanzaExtension
	virtual const std::string& filterString() const;

	// reimplemented from StanzaExtension
	virtual StanzaExtension* newInstance( const Tag* tag ) const
	{
		return new PubSubSubscribedExt( tag );
	}

	// reimplemented from StanzaExtension
	virtual Tag* tag() const;

	// reimplemented from StanzaExtension
	virtual StanzaExtension* clone() const
	{
		return new PubSubSubscribedExt( *this );
	}

	string subscription() const
	{ return m_subsciption; }

	string node() const
	{ return m_node; }

	string jid() const
	{ return m_jid; }

private:
	std::string m_subsciption;
	std::string m_node;
	std::string m_jid;
	Tag *m_tag;

	bool _parse( const Tag* tag );
};

#endif
