#define JINGLE_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>


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
      ClientBase() {}
      virtual ~ClientBase() {}
      const std::string getID() { return "id"; }
      virtual void send( IQ& iq, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIDHandler( IqHandler* ) {}
      void registerIqHandler( IqHandler*, int ) {}
      void removeIqHandler( IqHandler*, int ) {}
      void registerStanzaExtension( StanzaExtension* ext ) { delete ext; }
      void removeStanzaExtension( int ) {}
      ConnectionState state() const { return StateConnected; }
      bool authed() { return false; }
  };
}

#define CLIENTBASE_H__
#include "../../jinglesession.h"
#include "../../jinglesession.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  {
    name = "invalid Jingle 1";
    Jingle::Session::Jingle js;
    Tag* t = js.tag();
    if( t )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "invalid Jingle 2";
    Jingle::Session::Jingle js( Jingle::ContentAccept, 0, "somesid" );
    Tag* t = js.tag();
    if( t )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-accept";
    Jingle::Session::Jingle js( Jingle::ContentAccept, 0, "somesid" );
    js.setInitiator( "someinitiator" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
         "action='content-accept' initiator='someinitiator' sid='somesid'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, transport-info";
    Jingle::Session::Jingle js( Jingle::TransportInfo, 0, "somesid" );
    js.setInitiator( "someinitiator" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
         "action='transport-info' initiator='someinitiator' sid='somesid'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, session-initiate";
    Jingle::Session::Jingle js( Jingle::SessionInitiate, 0, "somesid" );
    js.setInitiator( "someinitiator" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
         "action='session-initiate' initiator='someinitiator' sid='somesid'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-replace";
    Jingle::Session::Jingle js( Jingle::ContentReplace, 0, "somesid" );
    js.setInitiator( "someinitiator" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
         "action='content-replace' initiator='someinitiator' sid='somesid'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle w/ initiator & responder";
    Jingle::Session::Jingle js( Jingle::ContentAccept, 0, "somesid" );
    js.setInitiator( "someinitiator" );
    js.setResponder( "someresponder" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
         "action='content-accept' initiator='someinitiator' "
         "responder='someresponder' sid='somesid'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }




  // -------
  name = "Jingle::Session::Jingle/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Jingle::Session::Jingle() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "jingle", "xmlns", XMLNS_JINGLE );
  IQ iq( IQ::Get, JID() );
  sef.addExtensions( iq, f );
  const Jingle::Session::Jingle* se = iq.findExtension<Jingle::Session::Jingle>( ExtJingle );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;



  printf( "Jingle::Session::Jingle: " );
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
