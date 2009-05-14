#include "../../tag.h"
#define UNIQUEMUCROOM_TEST
#include "../../uniquemucroom.h"
#include "../../iq.h"
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
    name = "create Tag";
    UniqueMUCRoom::Unique uq;
    t = uq.tag();
    if( !t || t->xml() != "<unique xmlns='" + XMLNS_MUC_UNIQUE + "'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "parse Tag";
    Tag u( "unique" );
    u.setXmlns( XMLNS_MUC_UNIQUE );
    u.setCData( "foo" );
    UniqueMUCRoom::Unique uq( &u );
    t = uq.tag();
    if( !t || t->xml() != "<unique xmlns='" + XMLNS_MUC_UNIQUE + "'>"
                          "foo</unique>"
       || uq.name() != "foo" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  name = "UniqueMUCRoom::Unique/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new UniqueMUCRoom::Unique() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "unique", "xmlns", XMLNS_MUC_UNIQUE );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const UniqueMUCRoom::Unique* se = iq.findExtension<UniqueMUCRoom::Unique>( ExtMUCUnique );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "UniqueMUCRoom::Unique: " );
  if( !fail )
    printf( "OK\n" );
  else
    printf( "%d test(s) failed\n", fail );

  return fail;
}
