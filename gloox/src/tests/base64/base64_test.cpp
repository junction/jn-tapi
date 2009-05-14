#include "../../base64.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  std::string b;
  std::string sample;



  // -------
  name = "empty string";
  sample = "";
  b = Base64::encode64( sample );
  if( sample != Base64::decode64( b ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  b = "";
  sample = "";

  // -------
  name = "leviathan test";
  sample = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
  b = Base64::encode64( sample );
  if( sample != Base64::decode64( b ) )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), b.c_str() );
  }
  b = "";
  sample = "";




  if( fail == 0 )
  {
    printf( "Base64: OK\n" );
    return 0;
  }
  else
  {
    printf( "Base64: %d test(s) failed\n", fail );
    return 1;
  }


}
