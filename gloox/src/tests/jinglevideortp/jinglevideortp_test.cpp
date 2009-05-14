#define JINGLE_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../jinglevideortp.h"
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
    name = "empty VideoRTP";
    Jingle::Description::PayloadList l;
    Jingle::VideoRTP a( l );
    Tag* t = a.tag();
    if( !t || t->xml() != "<description xmlns='" + XMLNS_JINGLE_VIDEO_RTP + "'/>" )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "VideoRTP w/ simple payload";
    Jingle::Description::PayloadList l;
    StringMap attrs;
    attrs.insert( std::make_pair( "id", "28" ) );
    attrs.insert( std::make_pair( "name", "nv" ) );
    attrs.insert( std::make_pair( "clockrate", "90000" ) );
    l.push_back( new Jingle::Description::Payload( attrs, StringMap() ) );
    Jingle::VideoRTP a( l );
    Tag* t = a.tag();
    if( !t || t->xml() != "<description xmlns='" + XMLNS_JINGLE_VIDEO_RTP + "'>"
         "<payload-type clockrate='90000' id='28' name='nv'/>"
         "</description>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "VideoRTP w/ payload & payload parameters";
    Jingle::Description::PayloadList l;
    StringMap attrs;
    attrs.insert( std::make_pair( "id", "96" ) );
    attrs.insert( std::make_pair( "name", "theora" ) );
    attrs.insert( std::make_pair( "clockrate", "90000" ) );
    StringMap params;
    params.insert( std::make_pair( "delivery-method", "inline" ) );
    params.insert( std::make_pair( "configuration", "somebase16string" ) );
    params.insert( std::make_pair( "sampling", "YCbCr-4:2:2" ) );
    l.push_back( new Jingle::Description::Payload( attrs, params ) );
    Jingle::VideoRTP a( l );
    Tag* t = a.tag();
    if( !t || t->xml() != "<description xmlns='" + XMLNS_JINGLE_VIDEO_RTP + "'>"
         "<payload-type clockrate='90000' id='96' name='theora'>"
         "<parameter name='configuration' value='somebase16string'/>"
         "<parameter name='delivery-method' value='inline'/>"
         "<parameter name='sampling' value='YCbCr-4:2:2'/>"
         "</payload-type>"
         "</description>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }


  printf( "Jingle::VideoRTP: " );
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
