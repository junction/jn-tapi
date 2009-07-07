#include <stdafx.h>
#include "onsipxmppbase.h"

#include "utils.h"

#include <windows.h>
#include <tchar.h>
#include "time.h"

#include "dataform.h"
#include "iq.h"
#include "adhoc.h"
#include "adhochandler.h"
#include "stanza.h"
#include "pubsub.h"
#include "pubsubmanager.h"
#include "pubsubresulthandler.h"
#include "error.h"
#include "pubsubevent.h"
#include "stanza.h"
#include "subscription.h"
#include "xmlprettyprint.h"

#include "OnSipStanzaExtensions.h"

LoginInfo::LoginInfo(const string& name,const string& password,const string& domain)
{
	m_name = name; m_password = password; m_domain = domain;
}

LoginInfo::LoginInfo(const LoginInfo& logininfo)
{
	m_name = logininfo.m_name; m_password = logininfo.m_password; m_domain = logininfo.m_domain;
}

const LoginInfo& LoginInfo::operator=(const LoginInfo& info)
{
	if ( &info == this )
		return *this;
	m_name = info.m_name; m_password = info.m_password; m_domain =	info.m_domain ;
	return *this;
}

string LoginInfo::SIPAddress()
{	return Strings::stringFormat(_T("%s@%s"), m_name.c_str(), m_domain.c_str() );	}

JID LoginInfo::jid()
{	return JID( Strings::stringFormat(_T("%s!%s@dashboard.onsip.com"), m_name.c_str(), m_domain.c_str()) );	}

//**********************************************************************************
//**********************************************************************************

// bSync = if true, will stay in this Start() method until disconnect
//   else will return and caller must call the Aysync Poll and Cleanup methods
ConnectionError OnSipXmppBase::Start(LoginInfo& loginInfo,bool bSync)
{
	Logger::log_debug( _T("OnSipXmppBase::Start bSync=%d"), bSync );
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	m_login = loginInfo;
	m_jid = m_login.jid();

	Logger::log_debug( _T("OnSipXmppBase::Start jid bare='%s' full='%s'"), m_jid.bare().c_str(), m_jid.full().c_str() );

	m_gloox.reset( new Client( m_jid, m_login.m_password ) );

	m_gloox->registerConnectionListener( this );
	m_gloox->registerMessageHandler(this);

	m_gloox->registerStanzaExtension( new DataForm(0) );
	m_gloox->registerStanzaExtension( new Adhoc::Command( EmptyString, Adhoc::Command::Execute ) );
	m_gloox->registerStanzaExtension( new PubSub::Event(NULL) );
	m_gloox->registerStanzaExtension( new Error(0) );
	m_gloox->registerStanzaExtension( new PubSubSubscribedExt(0) );
	//m_gloox->registerIqHandler( this, 0 );

	//	  j->registerTagHandler( this, "iq", EmptyString );
	m_gloox->disco()->setVersion( "OnSip", GLOOX_VERSION, "Windows" );
	//	  j->disco()->setIdentity( "client", "bot" );
	//	  j->disco()->addFeature( XMLNS_CHAT_STATES );
	m_gloox->disableRoster();
	m_gloox->setPresence(Presence::Available, 5 ); 
	//		  m_client->setServer(server);
	//		  m_client->setPort(port);
	//	  // ???
	//	  StringList ca;
	//	  ca.push_back( "/path/to/cacert.crt" );
	//	  j->setCACerts( ca );
	m_gloox->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
	m_pubSub.reset( new PubSub::Manager(m_gloox.get()) );
	//
	// this code connects to a jabber server through a SOCKS5 proxy
	//
	//		 ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
	//									 new ConnectionTCP( j->logInstance(),
	//														"sockshost", 1080 ),
	//									 j->logInstance(), "example.net" );
	//		 conn->setProxyAuth( "socksuser", "sockspwd" );
	//		 j->setConnectionImpl( conn );
	//
	// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
	//
	//		 ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
	//		 ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
	//		 conn1->setProxyAuth( "socksuser", "sockspwd" );
	//		 ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
	//		 conn2->setProxyAuth( "httpuser", "httppwd" );
	//		 j->setConnectionImpl( conn2 );

	Logger::log_debug( _T("OnSipXmppBase::Start bSync=%d  connecting..."), bSync );

	// If error connect
	if( !m_gloox->connect( false ) )
	{
		Logger::log_error( _T("OnSipXmppBase::Start bSync=%d  connectError") );
		return ConnNotConnected;
	}

	// If asynchronous, return back things are ok
	if ( !bSync )
		return ConnNoError;

	Logger::log_debug( _T("OnSipXmppBase::Start syncLoop enter") );

	// If synchronous, stay in loop until error
	ConnectionError ce = ConnNoError;
	while ( ce == ConnNoError )
		ce = AsyncPolling(INFINITE);

	Logger::log_debug( _T("OnSipXmppBase::Start syncLoop exit ce=%d"), ce );
	AsyncCleanup();
	return ce;
}

Client* OnSipXmppBase::getGloox()
{
	Client* pClient = m_gloox.get();
	if ( pClient == NULL )
	{
		Logger::log_error("OnSipXmpp::getGloox NULL gloox");
		return NULL;
	}
	return pClient;
}

// If Start() was called non-Sync, then this method should
// be called in loop extensively to keep the XMPP engine going.
// It will return ConnNoError if all ok.
// Method will stay in this poll loop for max time of dwMsecs
ConnectionError OnSipXmppBase::AsyncPolling(DWORD dwMsecs)
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	ConnectionError ce = ConnNoError;
	TimeOut to(dwMsecs);
	while( ce == ConnNoError && !to.IsExpired() )
	{
		if ( _tmrConnectLoop.IsExpired() )
		{
			if ( !onConnectLoop() )
				break;
			_tmrConnectLoop.Reset();
		}
		ce = m_gloox->recv(100);
	}
	if ( ce != ConnNoError )
		Logger::log_debug(_T("OnSipXmppBase::AsyncPolling exit ce=%d"), ce );
	return ce;
}

// If Start() was called non-Sync, then this method should
// be called after AsyncPolling and ready to exit XMPP engine
void OnSipXmppBase::AsyncCleanup()
{
	Logger::log_debug(_T("OnSipXmppBase::AsyncCleanup"));
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	if ( m_gloox.get() != NULL )
		m_gloox->disconnect();
	// Delete the gloox object
	m_gloox.reset(NULL);
}

//
void OnSipXmppBase::Ping()
{
	Logger::log_debug(_T("OnSipXmppBase::Ping"));
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	if ( m_gloox.get() != NULL )
		m_gloox->whitespacePing();
}

// Return a display error string for the specified ConnectionError enum
//static
tstring OnSipXmppBase::GetConnectionErrorString(ConnectionError ce)
{
	tstring err;

	switch ( ce )
	{
		case ConnNoError:
			break;
		case ConnStreamError:
		case ConnStreamVersionError:
		case ConnStreamClosed:
			err = Strings::stringFormat(_T("Stream error occurred. %d"), ce );
			break;
		case ConnProxyAuthRequired:
		case ConnProxyAuthFailed:
		case ConnProxyNoSupportedAuth:
			err = Strings::stringFormat(_T("Proxy error occurred. %d"), (int) ce );
			break;
		case ConnIoError:
		case ConnParseError:
			err = Strings::stringFormat(_T("I/O error occurred. %d"), (int) ce );
			break;
		case ConnConnectionRefused:
			err = Strings::stringFormat(_T("Connection was refused. %d"), (int) ce );
			break;
		case ConnDnsError:
			err = Strings::stringFormat(_T("DNS error. %d"), (int) ce );
			break;
		case ConnOutOfMemory:
			err = Strings::stringFormat(_T("Memory error. %d"), (int) ce );
			break;
		case ConnNoSupportedAuth:
		case ConnTlsFailed:
		case ConnTlsNotAvailable:
		case ConnCompressionFailed:
		case ConnUserDisconnected:
			err = Strings::stringFormat(_T("Internal error. %d"), (int) ce );
			break;
		case ConnAuthenticationFailed:
			err = Strings::stringFormat(_T("Authentication failed. %d"), (int) ce );
			break;
		case ConnNotConnected:
			err = Strings::stringFormat(_T("Could not connect to server. %d"), (int) ce );
			break;
		default:
			err = Strings::stringFormat(_T("Unknown error. %d"), (int) ce );
			break;
	}
	Logger::log_debug(_T("OnSipXmppBase::GetConnectionErrorString ce=%d err=%s"),(int)ce, err.c_str());
	return err;
}

/**
* Reimplement this function if you want to receive the chunks of the conversation
* between gloox and server.
* @param level The log message's severity.
* @param area The log message's origin.
* @param message The log message.
*/
//virtual 
void OnSipXmppBase::handleLog( LogLevel level, LogArea area, const std::string& message ) 
{ 
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object

	switch(area)
	{
		case LogAreaXmlIncoming:
		{
			XmlPrettyPrint pp;
			tstring xml = pp.prettyPrint( message );
			Logger::log_debug("OnSipXmppBase::handleLog RECEIVEDXML: \r\n%s",xml.c_str());
			break;
		}
		case LogAreaXmlOutgoing:
		{
			XmlPrettyPrint pp;
			tstring xml = pp.prettyPrint( message );
			Logger::log_debug("OnSipXmppBase::handleLog SENTXML: \r\n%s",xml.c_str());
			break;
		}
		default:
		{
			Logger::log_trace("OnSipXmppBase::handleLog level=%d area=%d message=%s", level, area, message.c_str() );
			break;
		}
	}
}

//virtual 
void OnSipXmppBase::onConnect()
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	Logger::log_debug("OnSipXmppBase::onConnect" );
	m_bConnectEvent = true;
}

//virtual 
void OnSipXmppBase::onDisconnect( ConnectionError e )
{
	_checkThread.CheckSameThread();	// Verify we are single threaded for this object
	Logger::log_debug("OnSipXmppBase::OnDisconnect: %d", e );
	m_bDisconnectEvent = true;
}

//virtual 
bool OnSipXmppBase::onTLSConnect( const CertInfo& info )
{
	time_t from( info.date_from );
	time_t to( info.date_to );

	// TODO put in some verification for TLS connect
	Logger::log_debug( "OnSipXmppBase::onTLSConnect status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s"
							"from: %s\nto: %s\n",
							info.status, info.issuer.c_str(), info.server.c_str(),
							info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
							info.compression.c_str(), 
							DateTimeOperations::getTimeString(from).c_str(), 
							DateTimeOperations::getTimeString(to).c_str()
							);
  return true;
}

