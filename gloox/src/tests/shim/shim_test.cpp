#include "../../shim.h"
#include "../../message.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>



int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;

  // -------
  {
    name = "empty tag() test";
    SHIM shim;
    t = shim.tag();
    if( t )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "headers to tag to headers, part 1/2";
    SHIM::HeaderList hl;
    hl.insert( std::make_pair( "created", "yesterday" ) );
    hl.insert( std::make_pair( "name", "foo" ) );
    SHIM shim( hl );
    t = shim.tag();
    if( t->xml() != "<headers xmlns='"+ XMLNS_SHIM + "'><header name='created'>yesterday</header><header name='name'>foo</header></headers>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }

    name = "headers to tag to headers, part 2/2";
    SHIM s( t );
    if( s.headers() != hl )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  StanzaExtensionFactory sef;
  sef.registerExtension( new SHIM() );
  // -------
  {
    name = "SHIM/SEFactory test";
    Tag* f = new Tag( "message" );
    new Tag( f, "headers", "xmlns", XMLNS_SHIM );
    Message msg( Message::Normal, JID(), "" );
    sef.addExtensions( msg, f );
    const SHIM* se = msg.findExtension<SHIM>( ExtSHIM );
    if( se == 0 )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "SHIM: " );
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
