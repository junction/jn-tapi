#define JINGLE_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../jingleaudiortp.h"
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
    name = "empty AudioRTP";
    Jingle::Description::PayloadList l;
    Jingle::AudioRTP a( l );
    Tag* t = a.tag();
    if( !t || t->xml() != "<description xmlns='" + XMLNS_JINGLE_AUDIO_RTP + "'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "AudioRTP w/ simple payload";
    Jingle::Description::PayloadList l;
    StringMap attrs;
    attrs.insert( std::make_pair( "id", "96" ) );
    attrs.insert( std::make_pair( "name", "speex" ) );
    attrs.insert( std::make_pair( "clockrate", "16000" ) );
    l.push_back( new Jingle::Description::Payload( attrs, StringMap() ) );
    Jingle::AudioRTP a( l );
    Tag* t = a.tag();
    if( !t || t->xml() != "<description xmlns='" + XMLNS_JINGLE_AUDIO_RTP + "'>"
         "<payload-type clockrate='16000' id='96' name='speex'/>"
         "</description>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }



  printf( "Jingle::AudioRTP: " );
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
