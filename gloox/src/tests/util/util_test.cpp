#include "../../util.h"

#include <string>

using namespace gloox;

int main()
{
  enum { A, B, C, Inval };
  static const char* values[] = { "a", "b", "c" };
  enum { D = 1<<0, E = 1<<1, F = 1<<2, Inval2 = 1<<3 };
  static const char* values2[] = { "d", "e", "f" };
  int fail = 0;

  // -------
  std::string name = "string lookup";
  if( util::lookup( "a", values ) != A )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid string lookup";
  if( util::lookup( "", values ) != Inval )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "code lookup";
  if( util::lookup( A, values ) != "a" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid code lookup";
  if( !util::lookup( Inval, values ).empty() )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "out-of-range code lookup";
  if( !util::lookup( 700, values ).empty() )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "string lookup (ORable)";
  if( util::lookup2( "d", values2 ) != D )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid string lookup (ORable)";
  if( util::lookup2( "", values2 ) != Inval2 )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "code lookup (ORable)";
  if( util::lookup2( D, values2 ) != "d" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid code lookup (ORable)";
  if( !util::lookup2( Inval2, values2 ).empty() )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "out-of-range code lookup (ORable)";
  if( !util::lookup2( 700, values2 ).empty() )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "string lookup w/ default";
  if( util::deflookup( "a", values, B ) != A )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid string lookup w/ default";
  if( util::deflookup( "", values, B ) != B )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "code lookup w/ default";
  if( util::deflookup( A, values, "foo" ) != "a" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid code lookup w/ default";
  if( util::deflookup( Inval, values, "foo" ) != "foo" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "out-of-range code lookup w/ default";
  if( util::deflookup( 700, values, "foo" ) != "foo" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "string lookup (ORable) w/ default";
  if( util::deflookup2( "d", values2, A ) != D )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid string lookup (ORable) w/ default";
  if( util::deflookup2( "", values2, E ) != E )
  {
    printf( "test '%s' failed: %d\n", name.c_str(), util::deflookup2( "", values2, E ) );
    ++fail;
  }

  // -------
  name = "code lookup (ORable) w/ default";
  if( util::deflookup2( D, values2, "foo" ) != "d" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "invalid code lookup (ORable) w/ default";
  if( util::deflookup2( Inval2, values2, "foo" ) != "foo" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "out-of-range code lookup (ORable) w/ default";
  if( util::deflookup2( 700, values2, "foo" ) != "foo" )
  {
    printf( "test '%s' failed\n", name.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 1";
  int ex = 2147483647;
  std::string re = util::long2string( ex );
  if( "2147483647" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 2";
  ex = 0;
  re = util::long2string( ex );
  if( "0" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 3";
  ex = -1;
  re = util::long2string( ex );
  if( "-1" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 4";
  ex = 1;
  re = util::long2string( ex );
  if( "1" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 5";
  ex = -2147483647;
  re = util::long2string( ex );
  if( "-2147483647" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 6";
  ex = -10;
  re = util::long2string( ex );
  if( "-10" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "long2string 7";
  ex = 11;
  re = util::long2string( ex );
  if( "11" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }

  // -------
  name = "int2string";
  ex = 2147483647;
  re = util::int2string( ex );
  if( "2147483647" != re )
  {
    printf( "test '%s' failed, expected: %d, result: '%s'\n", name.c_str(), ex, re.c_str() );
    ++fail;
  }




  if( fail == 0 )
  {
    printf( "Util: OK\n" );
    return 0;
  }
  else
  {
    printf( "Util: %d test(s) failed\n", fail );
    return 1;
  }

}
