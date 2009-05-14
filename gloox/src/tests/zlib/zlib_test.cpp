#include "../../compressionzlib.h"
#include "../../compressiondatahandler.h"
using namespace gloox;

#ifdef _WIN32
# include "../../../config.h.win"
#elif defined( _WIN32_WCE )
# include "../../../config.h.win"
#else
# include "config.h"
#endif

#include <stdio.h>
#include <locale.h>
#include <string>

#ifdef HAVE_ZLIB

class ZlibTest : public CompressionDataHandler
{
  public:
    ZlibTest() : m_zlib( this ) { m_zlib.init(); }
    ~ZlibTest() {}
    virtual void handleCompressedData( const std::string& data );
    virtual void handleDecompressedData( const std::string& data );
    const std::string data() { std::string ret = m_decompressed; m_decompressed = ""; return ret; }
    void compress(  const std::string& data );
  private:
    CompressionZlib m_zlib;
    std::string m_decompressed;
};

void ZlibTest::compress( const std::string& data )
{
  m_zlib.compress( data );
}

void ZlibTest::handleCompressedData( const std::string& data )
{
  m_zlib.decompress( data );
}

void ZlibTest::handleDecompressedData( const std::string& data )
{
  m_decompressed += data;
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ZlibTest t;

  // -------
  name = "short test";
  const std::string a( 10, 'a' );
  t.compress( a );
  if( t.data() != a )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "longer test";
  const std::string b( 1000, 'b' );
  t.compress( b );
  if( t.data() != b )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "long test";
  const std::string c( 100000, 'b' );
  t.compress( c );
  if( t.data() != c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "large test";
  const std::string d( 10000000, 'b' );
  t.compress( d );
  if( t.data() != d )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "concat test";
  t.compress( a );
  t.compress( b );
  t.compress( c );
  t.compress( d );
  if( t.data() != a + b + c + d )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }










  if( fail == 0 )
  {
    printf( "CompressionZlib: OK\n" );
    return 0;
  }
  else
  {
    printf( "CompressionZlib: %d test(s) failed\n", fail );
    return 1;
  }

}

#else
int main( int /*argc*/, char** /*argv*/ )
{
  printf( "Zlib not available. Skipped tests.\n" );
}
#endif // HAVE_ZLIB
