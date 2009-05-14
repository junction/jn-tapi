#include "../../tag.h"
#define VCARD_TEST
#include "../../vcard.h"
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
    name = "empty vcard request";
    VCard v;
    t = v.tag();
    if( !t || t->xml() != "<vCard xmlns='" + XMLNS_VCARD_TEMP + "' version='3.0'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  name = "VCard/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new VCard() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "vCard", "xmlns", XMLNS_VCARD_TEMP );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const VCard* se = iq.findExtension<VCard>( ExtVCard );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "VCard: " );
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
