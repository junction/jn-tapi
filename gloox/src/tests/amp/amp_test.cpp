#include "../../tag.h"
#include "../../amp.h"
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
    name = "empty <amp>";
    AMP a;
    t = a.tag();
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
    name = "empty <amp>, per-hop = true";
    AMP a( true );
    t = a.tag();
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
    name = "one rule";
    AMP a;
    a.addRule( new AMP::Rule( "foodate", AMP::ActionDrop ) );
    t = a.tag();
    if( !t || t->xml() != "<amp xmlns='" + XMLNS_AMP + "'>"
                          "<rule condition='expire-at' action='drop' value='foodate'/>"
                          "</amp>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "one rule, per-hop = true";
    AMP a( true );
    a.addRule( new AMP::Rule( "foodate", AMP::ActionDrop ) );
    t = a.tag();
    if( !t || t->xml() != "<amp xmlns='" + XMLNS_AMP + "' per-hop='true'>"
         "<rule condition='expire-at' action='drop' value='foodate'/>"
         "</amp>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "2 rules, per-hop = true";
    AMP a( true );
    a.addRule( new AMP::Rule( "foodate", AMP::ActionDrop ) );
    a.addRule( new AMP::Rule( AMP::DeliverNone, AMP::ActionDrop ) );
    t = a.tag();
    if( !t || t->xml() != "<amp xmlns='" + XMLNS_AMP + "' per-hop='true'>"
         "<rule condition='expire-at' action='drop' value='foodate'/>"
         "<rule condition='deliver' action='drop' value='none'/>"
         "</amp>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "2 rules()";
    AMP a( true );
    a.addRule( new AMP::Rule( "foodate", AMP::ActionDrop ) );
    a.addRule( new AMP::Rule( AMP::DeliverNone, AMP::ActionDrop ) );
    if( a.rules().size() != 2 )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
  }

  // -------
  {
    name = "Tag ctor";
    Tag* q = new Tag( "amp" );
    q->setXmlns( XMLNS_AMP );
    q->addAttribute( "from", "foofrom" );
    q->addAttribute( "to", "footo" );
    q->addAttribute( "status", "notify" );
    q->addAttribute( "per-hop", "true" );
    Tag* r = new Tag( q, "rule" );
    r->addAttribute( "condition", "deliver" );
    r->addAttribute( "action", "error" );
    r->addAttribute( "value", "forward" );
    AMP a( q );
    t = a.tag();
    if( *t != *q )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
    delete q;
    q = 0;
  }

  // -------
  name = "AMP/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new AMP() );
  Tag* f = new Tag( "message" );
  new Tag( f, "amp", "xmlns", XMLNS_AMP );
  Message msg( Message::Normal, JID() );
  sef.addExtensions( msg, f );
  const AMP* se = msg.findExtension<AMP>( ExtAMP );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "AMP: OK\n" );
    return 0;
  }
  else
  {
    printf( "AMP: %d test(s) failed\n", fail );
    return 1;
  }

}
