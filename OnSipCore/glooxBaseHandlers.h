#ifndef GLOOX_BASEHANDLERS_H
#define GLOOX_BASEHANDLERS_H

// The gloox handlers that are being used with
// default methods defined for all virtuals.
// This is done to allow the real users of these
// handlers to only have to derive the virtuals
// they wish to use.

#include "onsip.h"
#include "logger.h"
#include "iqhandler.h"
#include "connectionlistener.h"
#include "pubsubresulthandler.h"
#include "messagehandler.h"
#include "error.h"
#include "pubsub.h"

class IqHandlerBase : public IqHandler
{
protected:
	virtual bool handleIq( const IQ& /*iq*/ ) { Logger::log_debug("IqHandlerBase::handleIq"); return false; }
	virtual void handleIqID( const IQ& /*iq*/, int /*context*/ ) { Logger::log_debug("IqHandlerBase::handleIqID"); }
};

class ConnectionListenerBase : public ConnectionListener
{
public:
	virtual void onConnect() { Logger::log_debug("ConnectionListenerBase::onConnect"); }
	virtual void onDisconnect( ConnectionError /*e* )	{ Logger::log_debug("ConnectionListenerBase::OnDisconnect");  }
	virtual void onResourceBind( const std::string& /*resource*/ ) { Logger::log_debug("ConnectionListenerBase::onConnect"); }
	virtual void onResourceBindError( const Error* /*error*/ ) { Logger::log_debug("ConnectionListenerBase::onResourceBindError"); }
	virtual void onSessionCreateError( const Error* /*error*/ ) { Logger::log_debug("ConnectionListenerBase::onSessionCreateError"); }
	virtual bool onTLSConnect( const CertInfo& /*info*/ ) { Logger::log_warn("ConnectionListenerBase::onTLSConnect"); return false; }
	virtual void onStreamEvent( StreamEvent /*event*/ ) { Logger::log_debug("ConnectionListenerBase::onStreamEvent");  }
};

class MessageHandlerBase : public MessageHandler
{
public:
	virtual void handleMessage( const Message& msg, MessageSession* session = 0 )
	{	Logger::log_debug("MessageHandlerBase::handleMessage");  }
};

class PubSubResultHandlerBase : public PubSub::ResultHandler
{
public:
	virtual void handleItem( const JID& /*service*/,
							 const std::string& /*node*/,
							 const Tag* /*entry */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleItem"); }

	virtual void handleItems( const std::string& /*id*/,
							  const JID& /*service*/,
							  const std::string& /*node*/,
							  const PubSub::ItemList& /*itemList*/,
							  const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleItems"); }

	virtual void handleItemPublication( const std::string& /*id*/,
										const JID& /*service*/,
										const std::string& /*node*/,
										const PubSub::ItemList& /*item*/,
										const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleItemPublication"); }


	virtual void handleItemDeletion( const std::string& /*id*/,
									 const JID& /*service*/,
									 const std::string& /*node*/,
									 const PubSub::ItemList& /*itemList*/,
									 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleItemDeletion"); }

	virtual void handleSubscriptionResult( const std::string& /*id*/,
										   const JID& /*service*/,
										   const std::string& /*node*/,
										   const std::string& /*sid*/,
										   const JID& /*jid*/,
										   const PubSub::SubscriptionType /*subType*/,
										   const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscriptionResult"); }

	virtual void handleUnsubscriptionResult( const std::string& /*id*/,
											 const JID& /*service*/,
											 const std::string& /*node*/,
											 const std::string& /*sid*/,
											 const JID& /*jid*/,
											 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleUnsubscriptionResult"); }

	virtual void handleSubscriptionOptions( const std::string& /*id*/,
											const JID& /*service*/,
											const JID& /*jid*/,
											const std::string& /*node*/,
											const DataForm* /*options*/,
											const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscriptionOptions"); }

	virtual void handleSubscriptionOptionsResult( const std::string& /*id*/,
												  const JID& /*service*/,
												  const JID& /*jid*/,
												  const std::string& /*node*/,
												  const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscriptionOptionsResult"); }


	virtual void handleSubscribers( const std::string& /*id*/,
									const JID& /*service*/,
									const std::string& /*node*/,
									const PubSub::SubscriberList* /*list*/,
									const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscribers");	}

	virtual void handleSubscribersResult( const std::string& /*id*/,
										  const JID& /*service*/,
										  const std::string& /*node*/,
										  const PubSub::SubscriberList* /*list*/,
										  const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscribersResult"); }

	virtual void handleAffiliates( const std::string& /*id*/,
								   const JID& /*service*/,
								   const std::string& /*node*/,
								   const PubSub::AffiliateList* /*list*/,
								   const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleAffiliates"); }

	virtual void handleAffiliatesResult( const std::string& /*id*/,
										 const JID& /*service*/,
										 const std::string& /*node*/,
										 const PubSub::AffiliateList* /*list*/,
										 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleAffiliatesResult");  }

	virtual void handleNodeConfig( const std::string& /*id*/,
								   const JID& /*service*/,
								   const std::string& /*node*/,
								   const DataForm* /*config*/,
								   const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleNodeConfig");  }

	virtual void handleNodeConfigResult( const std::string& /*id*/,
										 const JID& /*service*/,
										 const std::string& /*node*/,
										 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleNodeConfigResult");  }

	virtual void handleNodeCreation( const std::string& /*id*/,
									 const JID& /*service*/,
									 const std::string& /*node*/,
									 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleNodeCreation");  }

	virtual void handleNodeDeletion( const std::string& /*id*/,
									 const JID& /*service*/,
									 const std::string& /*node*/,
									 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleNodeDeletion");  }

	virtual void handleNodePurge( const std::string& /*id*/,
								  const JID& /*service*/,
								  const std::string& /*node*/,
								  const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleNodePurge"); }

	virtual void handleSubscriptions( const std::string& /*id*/,
									  const JID& /*service*/,
									  const PubSub::SubscriptionMap& /*subMap*/,
									  const Error* /*error = 0*/)
		{ Logger::log_debug("PubSubResultHandlerBase::handleSubscriptions");  }

	virtual void handleAffiliations( const std::string& /*id*/,
									 const JID& /*service*/,
									 const PubSub::AffiliationMap& /*affMap*/,
									 const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleAffiliations"); }

	virtual void handleDefaultNodeConfig( const std::string& /*id*/,
										  const JID& /*service*/,
										  const DataForm* /*config*/,
										  const Error* /*error = 0 */)
		{ Logger::log_debug("PubSubResultHandlerBase::handleDefaultNodeConfig");  }

};

#endif
