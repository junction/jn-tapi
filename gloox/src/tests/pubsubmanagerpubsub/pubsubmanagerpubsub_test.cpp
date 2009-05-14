#include "../../jid.h"
#include "../../stanzaextensionfactory.h"
#include "../../iq.h"
using namespace gloox;

#include <string>


static const JID jid( "aaa@bbb.ccc" );
static const std::string node( "node" );

static const Tag* tag;

#define PUBSUBMANAGER_TEST
#include "../../pubsubmanager.cpp"
#include "../../pubsubmanager.h"

JID jid2( "some@jid.com" );

int main()
{
  std::string name;
  int fail = 0;
  // -------
  {
  }

  // -------
  {
    name = "PubSub::Manager::PubSub/SEFactory test";
    StanzaExtensionFactory sef;
    sef.registerExtension( new PubSub::Manager::PubSub() );
    Tag* f = new Tag( "iq" );
    new Tag( f, "pubsub", "xmlns", XMLNS_PUBSUB );
    IQ iq( IQ::Get, JID(), "" );
    sef.addExtensions( iq, f );
    const PubSub::Manager::PubSub* se = iq.findExtension<PubSub::Manager::PubSub>( ExtPubSub );
    if( se == 0 )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }





  printf( "PubSub::Manager::PubSub: " );
  if( fail )
    printf( "%d test(s) failed\n", fail );
  else
    printf( "OK\n" );

}

