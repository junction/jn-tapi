#ifndef ONSIP_XMPPBASE_H
#define ONSIP_XMPPBASE_H

#include "onsip.h"
#include "utils.h"
#include "glooxBaseHandlers.h"
#include "messageeventhandler.h"
#include "messagehandler.h"
#include "pubsubmanager.h"

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
					,public MessageHandler
					,public IqHandlerBase
					,public PubSubResultHandlerBase
{
private:
	TimeOut _tmrConnectLoop;

protected:
	CheckThread _checkThread;

	JID m_jid;
	LoginInfo m_login;
	std::auto_ptr<Client> m_gloox;
	std::auto_ptr<PubSub::Manager> m_pubSub;

	virtual void handleLog( LogLevel level, LogArea area, const std::string& message );
	virtual void onConnect();
	virtual void onDisconnect( ConnectionError e );
	virtual bool onTLSConnect( const CertInfo& info );

	// Virtual called while in Connect/Recv loop inside the Start method.
	// Return false to abort and disconnect
	virtual bool onConnectLoop() { return true; }

public:
	OnSipXmppBase() : _tmrConnectLoop(250)		// Call onConnectLoop every 250 ms.  configureable??
	{ }

	virtual ~OnSipXmppBase() {}

	// Start the Xmpp connection.  This function is synchronous
	// and stays in the method until all connection is complete.
	// Virtual onConnectLoop is periodically called
	ConnectionError Start(LoginInfo& loginInfo,bool bSync=true);
	ConnectionError AsyncPolling(DWORD dwMsecs);
	void AsyncCleanup();
};

#endif