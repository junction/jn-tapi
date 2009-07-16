#ifndef ONSIP_XMPPBASE_H
#define ONSIP_XMPPBASE_H

#include "onsip.h"
#include "utils.h"
#include "glooxBaseHandlers.h"
#include "messageeventhandler.h"
#include "messagehandler.h"
#include "pubsubmanager.h"
#include "pubsubresulthandler.h"

class LoginInfo
{
public:
	string m_name;
	string m_password;
	string m_domain;

	LoginInfo() {}
	LoginInfo(const string& name,const string& password,const string& domain);
	LoginInfo(const LoginInfo& logininfo);

	const LoginInfo& operator=(const LoginInfo& info);

	string SIPAddress();

	JID jid();
};

class OnSipXmppBase : public ConnectionListenerBase
					,LogHandler
					,public MessageHandlerBase
					,public IqHandlerBase
					,public PubSubResultHandlerBase
{
private:
	TimeOut _tmrConnectLoop;

	// Unique ID generator, THREAD-SAFE
	UniqueId_ts _ids;

protected:
	CheckThread _checkThread;

	JID m_jid;
	LoginInfo m_login;
	std::auto_ptr<Client> m_gloox;
	std::auto_ptr<PubSub::Manager> m_pubSub;
	bool m_bConnectEvent;
	bool m_bDisconnectEvent;

	virtual void handleLog( LogLevel level, LogArea area, const std::string& message );
	virtual void onConnect();
	virtual void onDisconnect( ConnectionError e );
	virtual bool onTLSConnect( const CertInfo& info );

	// Virtual called while in Connect/Recv loop inside the Start method.
	// Return false to abort and disconnect
	virtual bool onConnectLoop() { return true; }

public:
	OnSipXmppBase() : m_bConnectEvent (false), m_bDisconnectEvent(false), _tmrConnectLoop(250)		// Call onConnectLoop every 250 ms.  configureable??
	{ }

	virtual ~OnSipXmppBase() {}

	Client* getGloox();

	// THREAD-SAFE
	// Get next unique ID for contextId and other various purposes
	long getUniqueId() { return _ids.getNextId(); }

	// Start the Xmpp connection.  This function is synchronous
	// and stays in the method until all connection is complete.
	// Virtual onConnectLoop is periodically called
	ConnectionError Start(LoginInfo& loginInfo,bool bSync=true);
	ConnectionError AsyncPolling(DWORD dwMsecs);
	void AsyncCleanup();
	// non-thread-safe functions 
	// providing OnSip specific XMPP communication
	void Authorize(int contextId,IqHandler* iqHandler);
	// Enable call events on OnSIP PBX
	// returns the Id used for event
	//  expireTime in XMPP format, e.g. 2006-03-31T23:59Z
	//  can pass empty string to not have field passed in subscribe request
	string SubscribeCallEvents(const string& expireTime,ResultHandler* resultHandler);
	long UnsubscribeCallEvents(const string& subid,ResultHandler* resultHandler,IqHandler* iqHandler);
	long UnsubscribeCallEvents(const string& nodeid, const string& subid,ResultHandler* resultHandler,IqHandler* iqHandler);
	// Trigger request from server for it to return the list of all subscriptions
	void getSubscriptions(ResultHandler *resultHandler);

	void Ping();
	// Return a display error string for the specified ConnectionError enum
	//static
	static tstring GetConnectionErrorString(ConnectionError ce);

	// Return true if we got the Connected event
	bool GotConnectEvent()
	{	return m_bConnectEvent; }

	// Return true if we got the DisConnected event
	bool GotDisconnectEvent()
	{	return m_bDisconnectEvent; }

};

#endif