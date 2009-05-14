#define JINGLE_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../jinglerawudp.h"
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
    name = "invalid Jingle 1";
    Jingle::RawUDP a;
    Tag* t = a.tag();
    if( !t || t->xml() != "<transport xmlns='" + XMLNS_JINGLE_RAW_UDP + "'>"
                          "<candidate/>"
                          "</transport>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }




  printf( "Jingle::RawUDP: " );
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
