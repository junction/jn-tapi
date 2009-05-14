#include "../../receipt.h"
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
    Receipt r( Receipt::Invalid );
    t = r.tag();
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
    name = "request";
    Receipt r( Receipt::Request );
    t = r.tag();
    if( t->xml() != "<request xmlns='"+ XMLNS_RECEIPTS + "'/>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "received";
    Receipt r( Receipt::Received );
    t = r.tag();
    if( t->xml() != "<received xmlns='"+ XMLNS_RECEIPTS + "'/>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  StanzaExtensionFactory sef;
  sef.registerExtension( new Receipt( Receipt::Invalid ) );
  // -------
  {
    name = "Receipt::Request/SEFactory test";
    Tag* f = new Tag( "message" );
    new Tag( f, "request", "xmlns", XMLNS_RECEIPTS );
    Message msg( Message::Normal, JID(), "" );
    sef.addExtensions( msg, f );
    const Receipt* se = msg.findExtension<Receipt>( ExtReceipt );
    if( se == 0 || se->rcpt() != Receipt::Request )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  // -------
  {
    name = "Receipt::Received/SEFactory test";
    Tag* f = new Tag( "message" );
    new Tag( f, "received", "xmlns", XMLNS_RECEIPTS );
    Message msg( Message::Normal, JID(), "" );
    sef.addExtensions( msg, f );
    const Receipt* se = msg.findExtension<Receipt>( ExtReceipt );
    if( se == 0 || se->rcpt() != Receipt::Received )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  printf( "Receipt: " );
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
