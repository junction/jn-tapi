#include "../../md5.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  MD5 md5;



  // -------
  name = "empty string";
  md5.feed( "" );
  md5.finalize();
  if( md5.hex() != "d41d8cd98f00b204e9800998ecf8427e" )
  {
    printf( "expect: d41d8cd98f00b204e9800998ecf8427e\n" );
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), md5.hex().c_str() );
  }
  md5.reset();

  // -------
  name = "The quick brown fox jumps over the lazy dog";
  md5.feed( name );
  md5.finalize();
  if( md5.hex() != "9e107d9d372bb6826bd81d3542a419d6" )
  {
    printf( "expect: 9e107d9d372bb6826bd81d3542a419d6\n" );
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), md5.hex().c_str() );
  }
  md5.reset();

  // -------
  name = "The quick brown fox jumps over the lazy cog";
  md5.feed( name );
  md5.finalize();
  if( md5.hex() != "1055d3e698d289f2af8663725127bd4b" )
  {
    printf( "expect: 1055d3e698d289f2af8663725127bd4b\n" );
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), md5.hex().c_str() );
  }
  md5.reset();

  // -------
  name = "two-step";
  md5.feed( "The quick brown fox ");
  md5.feed( "jumps over the lazy dog" );
  md5.finalize();
  if( md5.hex() != "9e107d9d372bb6826bd81d3542a419d6" )
  {
    printf( "expect: 9e107d9d372bb6826bd81d3542a419d6\n" );
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), md5.hex().c_str() );
  }
  md5.reset();

  // -------
  name = "many-step";
  md5.feed( "The" );
  md5.feed( " quick bro" );
  md5.feed( "" );
  md5.feed( "wn fox " );
  md5.feed( "jumps over the lazy dog" );
  md5.finalize();
  if( md5.hex() != "9e107d9d372bb6826bd81d3542a419d6" )
  {
    printf( "expect: 9e107d9d372bb6826bd81d3542a419d6\n" );
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), md5.hex().c_str() );
  }
  md5.reset();



  if( fail == 0 )
  {
    printf( "MD5: OK\n" );
    return 0;
  }
  else
  {
    printf( "MD5: %d test(s) failed\n", fail );
    return 1;
  }


}
