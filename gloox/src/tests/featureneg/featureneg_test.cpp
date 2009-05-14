#include "../../tag.h"
#include "../../iq.h"
#include "../../message.h"
#include "../../featureneg.h"
#include "../../dataform.h"
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
    FeatureNeg fn;
    t = fn.tag();
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
    name = "tag() test";
    FeatureNeg fn( new DataForm( TypeForm ) );
    t = fn.tag();
    if( !t || t->xml() != "<feature xmlns='" + XMLNS_FEATURE_NEG + "'>"
                          "<x xmlns='" + XMLNS_X_DATA + "' type='form'/>"
                          "</feature>" )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    // t re-used in next test
  }

  // -------
  {
    name = "parse tag";
    FeatureNeg fn( t ); // re-using t from previous test
    Tag* t2 = fn.tag();
    if( !t2 || *t2 != *t )
    {
      ++fail;
      printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t2;
    delete t;
    t = 0;
  }

  // -------
  name = "FeatureNeg/SEFactory test (IQ)";
  StanzaExtensionFactory sef;
  sef.registerExtension( new FeatureNeg() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "feature", "xmlns", XMLNS_FEATURE_NEG );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const FeatureNeg* se = iq.findExtension<FeatureNeg>( ExtFeatureNeg );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;

  // -------
  name = "FeatureNeg/SEFactory test (Message)";
  f = new Tag( "message" );
  new Tag( f, "feature", "xmlns", XMLNS_FEATURE_NEG );
  Message msg( Message::Normal, JID() );
  sef.addExtensions( msg, f );
  se = msg.findExtension<FeatureNeg>( ExtFeatureNeg );
  if( se == 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;

  if( fail == 0 )
  {
    printf( "FeatureNeg: OK\n" );
    return 0;
  }
  else
  {
    printf( "FeatureNeg: %d test(s) failed\n", fail );
    return 1;
  }

}
