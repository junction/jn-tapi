#include "../../tag.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../base64.h"
#include "../../stanzaextensionfactory.h"
#include "../../disco.h"
#include "../../flexoffhandler.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;

  class ClientBase
  {
    public:
      ClientBase() : m_disco( new Disco( this ) ) {}
      virtual ~ClientBase() { delete m_disco; }
      Disco* disco();
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ&, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void removeIDHandler( IqHandler* ) {}
    private:
      Disco* m_disco;
      JID m_jid;
  };
  Disco* ClientBase::disco() { return m_disco; }
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define FLEXOFF_TEST
#include "../../disco.cpp"
#include "../../flexoff.h"
#include "../../flexoff.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  {
    name = "empty tag() test";
    FlexibleOffline::Offline foo;
    if( false )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
  }


  StanzaExtensionFactory sef;
  sef.registerExtension( new FlexibleOffline::Offline() );
  // -------
  {
    name = "FlexibleOffline::Offline/SEFactory test";
    Tag* f = new Tag( "iq" );
    new Tag( f, "offline", "xmlns", XMLNS_OFFLINE );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const FlexibleOffline::Offline* se = iq.findExtension<FlexibleOffline::Offline>( ExtFlexOffline );
    if( se == 0 )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "FlexibleOffline::Offline: " );
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
