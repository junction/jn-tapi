#include "../../tag.h"
#include "../../vcardupdate.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;
  VCardUpdate *d;
  Tag *x = new Tag( "x" );
  x->addAttribute( "xmlns", XMLNS_X_VCARD_UPDATE );
  new Tag( x, "photo", "invalidhash" );

  // -------
  name = "parsing 0 tag";
  d = new VCardUpdate( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/getters";
  d = new VCardUpdate( "invalidhash" );
  if( d->hash() != "invalidhash" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/tag()";
  d = new VCardUpdate( "invalidhash" );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_VCARD_UPDATE )
       || !t->hasChild( "photo" ) || t->findChild( "photo" )->cdata() != "invalidhash" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/getters";
  d = new VCardUpdate( x );
  if( d->hash() != "invalidhash" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "from Tag/tag()";
  d = new VCardUpdate( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_VCARD_UPDATE )
       || !t->hasChild( "photo" ) || t->findChild( "photo" )->cdata() != "invalidhash" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;




  delete x;
  x = 0;


  if( fail == 0 )
  {
    printf( "VCardUpdate: OK\n" );
    return 0;
  }
  else
  {
    printf( "VCardUpdate: %d test(s) failed\n", fail );
    return 1;
  }

}
