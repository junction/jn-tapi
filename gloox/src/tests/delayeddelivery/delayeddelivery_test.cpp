#include "../../tag.h"
#include "../../delayeddelivery.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;
  DelayedDelivery *d;
  JID j( "abc@def/ghi" );
  Tag *x = new Tag( "delay", "reason" );
  x->addAttribute( "stamp", "invalidstamp" );
  x->addAttribute( "from", j.full() );
  x->addAttribute( "xmlns", XMLNS_DELAY );

  // -------
  name = "parsing 0 tag";
  d = new DelayedDelivery( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/getters";
  d = new DelayedDelivery( j, "invalidstamp", "reason" );
  if( d->reason() != "reason" || d->stamp() != "invalidstamp" || d->from() != j )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;
  t = 0;

  // -------
  name = "filled object/tag()";
  d = new DelayedDelivery( j, "invalidstamp", "reason" );
  t = d->tag();
  if( !t || t->name() != "delay" || !t->hasAttribute( "xmlns", XMLNS_DELAY )
       || !t->hasAttribute( "from", j.full() ) || !t->hasAttribute( "stamp", "invalidstamp" )
       || t->cdata() != "reason" )
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
  d = new DelayedDelivery( x );
  if( d->reason() != "reason" || d->stamp() != "invalidstamp" || d->from() != j )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new DelayedDelivery( x );
  t = d->tag();
  if( !t || t->name() != "delay" || !t->hasAttribute( "xmlns", XMLNS_DELAY )
       || !t->hasAttribute( "from", j.full() ) || !t->hasAttribute( "stamp", "invalidstamp" )
       || t->cdata() != "reason" )
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
    printf( "DelayedDelivery: OK\n" );
    return 0;
  }
  else
  {
    printf( "DelayedDelivery: %d test(s) failed\n", fail );
    return 1;
  }

}
