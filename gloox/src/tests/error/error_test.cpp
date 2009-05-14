#include "../../error.h"
#include "../../tag.h"
#include "../../gloox.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>


int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;

  Tag * error = new Tag( "error" );
  error->addAttribute( "type", "auth" );
  new Tag( error, "feature-not-implemented", "xmlns", XMLNS_XMPP_STANZAS );
  Tag * text = new Tag( error, "text", "xmlns", XMLNS_XMPP_STANZAS );
  text->addAttribute( "xml:lang", "default" );
  text->setCData( "shit happens" );
  Tag * appErr = new Tag( error, "unsupported", "xmlns", "errorNS" );
  appErr->addAttribute( "feature", "f" );

  std::string name = "";
  Error *e = new Error( error );
  if( e->type() != StanzaErrorTypeAuth || e->error() != StanzaErrorFeatureNotImplemented
    || e->appError()->xml() != "<unsupported xmlns='errorNS' feature='f'/>"
    || e->text() != "shit happens" )
  {
    printf( "failed: '%s' test\n", name.c_str() );
    printf( "type == %d, should be %d\n", e->type(), StanzaErrorTypeAuth );
    printf( "err == %d, should be %d\n", e->error(), StanzaErrorFeatureNotImplemented );
    printf( "xml: %s\n", e->appError()->xml().c_str() );
    printf( "text: %s\n", e->text().c_str() );
    ++fail;
  }
  // -------

  delete error;
  delete e;


  if( fail == 0 )
  {
    printf( "Error: OK\n" );
    return 0;
  }
  else
  {
    printf( "Error: %d test(s) failed\n", fail );
    return 1;
  }

}
