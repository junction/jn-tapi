#include "../../tag.h"
#define ADHOC_COMMANDS_TEST
#include "../../adhoc.h"
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
  Tag* t;


  // -------
  {
    name = "info note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Info, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Info || n.content() != "content"
        || t->xml() != "<note type='info'>content</note>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "warning note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Warning, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Warning || n.content() != "content"
        || t->xml() != "<note type='warn'>content</note>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "error note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Error, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Error || n.content() != "content"
        || t->xml() != "<note type='error'>content</note>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "error note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Error, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Error || n.content() != "content"
        || t->xml() != "<note type='error'>content</note>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }


  // -------
  {
    name = "parse Tag";
    Tag* b = new Tag( "note", "foo" );
    b->addAttribute( "type", "info" );
    Adhoc::Command::Note n( b );
    t = n.tag();
    if( *t != *b )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete b;
    b = 0;
  }

  // -------
  {
    name = "parse Tag w/o type";
    Tag* b = new Tag( "note", "foo" );
    b->addAttribute( "type", "info" );
    Adhoc::Command::Note n( b );
    t = n.tag();
    if( *t != *b )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete b;
    b = 0;
  }



  if( fail == 0 )
  {
    printf( "Adhoc::Command::Note: OK\n" );
    return 0;
  }
  else
  {
    printf( "Adhoc::Command::Note: %d test(s) failed\n", fail );
    return 1;
  }

}
