#define GLOOX_TESTS
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../jid.h"
#include "../../stanzaextension.h"

#include <stdio.h>
#include <locale.h>
#include <string>

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;
  class Capabilities : public StanzaExtension
  {
    public:
      Capabilities() : StanzaExtension( ExtUser + 1 ) {}
      const std::string& ver() const { return EmptyString; }
      const std::string& node() const { return EmptyString; }
  };

  class ClientBase
  {
    public:
      ClientBase() : m_jid( "self" ) {}
      virtual ~ClientBase() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ& ) {};
      virtual void send( const IQ&, IqHandler*, int ) {};
      void removeIqHandler( IqHandler* ih, int exttype );
      void removeIDHandler( IqHandler* ih );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
    private:
      JID m_jid;
  };
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::removeIDHandler( IqHandler* ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define CAPABILITIES_H__
#define JINGLESESSION_TEST
#include "../../jinglesession.h"
#include "../../jinglesession.cpp"
#include "../../jinglesessionhandler.h"
class JingleSessionTest : public ClientBase, public Jingle::SessionHandler
{
  public:
    JingleSessionTest() : m_result( false ), m_result2( false )
    {
      m_js = new Jingle::Session( this, JID( "foo@bar" ), this );
    }
    ~JingleSessionTest() { delete m_js; }
    void setTest( int test ) { m_test = test; }
    virtual void send( IQ& iq );
    virtual void send( const IQ& iq, IqHandler*, int );
    virtual void trackID( IqHandler *ih, const std::string& id, int context );
    bool checkResult() { bool t = m_result; m_result = false; return t; }
    bool checkResult2() { bool t = m_result2; m_result2 = false; return t; }
    Jingle::Session* js() { return m_js; }
  private:
    Jingle::Session* m_js;
    int m_test;
    bool m_result;
    bool m_result2;
};

void JingleSessionTest::send( IQ& /*iq*/ )
{
}

void JingleSessionTest::send( const IQ& iq, IqHandler*, int ctx )
{
  switch( m_test )
  {
    case 1:
    {
      break;
    }
    case 2:
    case 3:
    case 4:
    case 5:
    {
      break;
    }
  }
}

void JingleSessionTest::trackID( IqHandler*, const std::string&, int ) {}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  JingleSessionTest* jst = new JingleSessionTest();


  // -------
  name = "initial state";
  if( jst->js()->state() != Jingle::Session::Ended )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }







  delete jst;

  printf( "Jingle::Session: " );
  if( fail == 0 )
  {
    printf( "OK\n" );
    return 0;
  }
  else
  {
    printf( "%d test(s) failed\n", fail );
    return 1;
  }

}
