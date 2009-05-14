#include "../../nickname.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;


  // -------
  {
    name = "empty nick";
    Nickname n( "" );
    if( !n.nick().empty() || n.tag() )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "nick check";
    Nickname n( "foo" );
    Tag* t = n.tag();
    if( n.nick() != "foo" || !t )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "tag check";
    Nickname n( "foo" );
    Tag* t = n.tag();
    if( t->xml() != "<nick xmlns='http://jabber.org/protocol/nick'>foo</nick>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }



  if( fail == 0 )
  {
    printf( "Nickname: OK\n" );
    return 0;
  }
  else
  {
    printf( "Nickname: %d test(s) failed\n", fail );
    return 1;
  }


}
