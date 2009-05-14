
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT([gloox], [1.0-beta7], [js@camaya.net])
AC_CONFIG_SRCDIR(src/gloox.cpp)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

dnl
dnl extra paths
dnl

AC_ARG_WITH(extra_include_path, AC_HELP_STRING([--with-extra-include-path],
                                           [use additional include paths]),
            extra_include_path=$withval)
split_includes="`echo $extra_include_path | sed -e 's/:/ /g'`"
for incpath in $split_includes ; do
    CPPFLAGS="-I$incpath $CPPFLAGS"
done

AC_ARG_WITH(extra_library_path, AC_HELP_STRING([--with-extra-library-path],
                                               [use additional library paths]),
            extra_library_path=$withval)
split_libs="`echo $extra_library_path | sed -e 's/:/ /g'`"
for libpath in $split_libs ; do
    LDFLAGS="-L$libpath $LDFLAGS"
done

dnl Checks for libraries.

dnl Libidn
AC_ARG_WITH(libidn, AC_HELP_STRING([--with-libidn=[DIR]],
      [Support IDN (needs GNU Libidn)]),
      libidn=$withval, libidn=yes)
if test "$libidn" != "no"; then
  if test "$libidn" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$libidn/lib"
    CPPFLAGS="${CPPFLAGS} -I$libidn/include"
  fi
  AC_CHECK_HEADER(idna.h,
  AC_CHECK_LIB(idn, stringprep_check_version,
           [libidn=yes LIBS="${LIBS} -lidn"], libidn=no),
           libidn=no)
fi
if test "$libidn" != "no" ; then
  AC_DEFINE(HAVE_LIBIDN, 1, [Define to 1 if you want IDN support.])
else
  AC_MSG_WARN([Libidn not found])
fi
AC_MSG_CHECKING([if Libidn should be used])
AC_MSG_RESULT($libidn)


dnl extra win32 libs
case "$host_os" in
  *mingw*)
      LIBS="${LIBS} -lwsock32 -lgdi32 -ldnsapi";;
esac


ssl=no

dnl schannel
AC_ARG_WITH(schannel, AC_HELP_STRING([--with-schannel],
      [Support Stream Encryption (win32 only)]),
      schannel=yes, schannel=no)
if test "$schannel" == "yes"; then
  case "$host_os" in
    *mingw*)
        ssl=yes
        LIBS="${LIBS} -lcrypt32 -lsecur32";;
    *)
        schannel=no
        echo "*"
        echo "* You provided the '--with-schannel' switch. SChannel is only available in MinGW. Ignoring."
        echo "*";;
  esac
  AC_MSG_CHECKING([if SChannel should be used])
  AC_MSG_RESULT($schannel)
fi

dnl GnuTLS
if test "$ssl" == "no"; then
  AC_ARG_WITH(gnutls, AC_HELP_STRING([--with-gnutls=[DIR]],
        [Support Stream Encryption (needs GnuTLS)]),
        gnutls=$withval, gnutls=yes)
  if test "$gnutls" != "no"; then
    if test "$gnutls" != "yes"; then
      LDFLAGS="${LDFLAGS} -L$gnutls/lib"
      CPPFLAGS="${CPPFLAGS} -I$gnutls/include"
    fi
    AC_CHECK_HEADER(gnutls/gnutls.h,
    AC_CHECK_LIB(gnutls, gnutls_check_version,
            [gnutls=yes LIBS="${LIBS} `libgnutls-config --libs`"], gnutls=no),
            gnutls=no)
  fi
  if test "$gnutls" != "no" ; then
    AC_DEFINE(HAVE_GNUTLS, 1, [Define to 1 if you want TLS support (GnuTLS). Undefine HAVE_OPENSSL.])
    ssl=yes
  else
    AC_MSG_WARN([GnuTLS not found])
  fi
  AC_MSG_CHECKING([if GnuTLS should be used])
  AC_MSG_RESULT($gnutls)
fi

dnl OpenSSL
if test "$ssl" == "no"; then
  AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl=[DIR]],
        [Support Stream Encryption (needs OpenSSL)]),
        openssl=$withval, openssl=yes)
  if test "$openssl" != "no"; then
    if test "$openssl" != "yes"; then
      LDFLAGS="${LDFLAGS} -L$openssl/lib"
      CPPFLAGS="${CPPFLAGS} -I$openssl/include"
    fi
    LIBS_TMP=${LIBS}
    LIBS="${LIBS} -lssl -lcrypto"
    AC_CHECK_HEADER(openssl/ssl.h,
    AC_CHECK_LIB(ssl, SSL_library_init,
            [openssl=yes], openssl=no),
            [openssl=no LIBS=${LIBS_TMP}])
  fi
  if test "$openssl" != "no" ; then
    AC_DEFINE(HAVE_OPENSSL, 1, [Define to 1 if you want TLS support (OpenSSL). Undefine HAVE_GNUTLS.])
  else
    AC_MSG_WARN([OpenSSL not found])
  fi
  AC_MSG_CHECKING([if OpenSSL should be used])
  AC_MSG_RESULT($openssl)
fi

dnl Zlib
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib=[DIR]],
      [Support Stream Compression (needs Zlib)]),
      zlib=$withval, zlib=yes)
if test "$zlib" != "no"; then
  if test "$zlib" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$zlib/lib"
    CPPFLAGS="${CPPFLAGS} -I$zlib/include"
  fi
  AC_CHECK_HEADER(zlib.h,
  AC_CHECK_LIB(z, zlibVersion,
           [zlib=yes LIBS="${LIBS} -lz"], zlib=no),
           zlib=no)
fi
if test "$zlib" != "no" ; then
  AC_DEFINE(HAVE_ZLIB, 1, [Define to 1 if you want Stream Compression support.])
else
  AC_MSG_WARN([Zlib not found])
fi
AC_MSG_CHECKING([if Zlib should be used])
AC_MSG_RESULT($zlib)

dnl pthread
ACX_PTHREAD()
LIBS="$PTHREAD_LIBS $LIBS"
CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS"

dnl check for res_querydomain in libc, libbind and libresolv
AC_CHECK_FUNCS(res_querydomain)
if test "x-$ac_cv_func_res_querydomain" = "x-yes" ; then
  have_res_querydomain=yes
else
  AC_CHECK_LIB(resolv, res_querydomain)
  if test "x-$ac_cv_lib_resolv_res_querydomain" = "x-yes" ; then
    have_res_querydomain=yes
    AC_DEFINE(HAVE_RES_QUERYDOMAIN)
  else
    AC_CHECK_LIB(bind, res_querydomain)
    if test "x-$ac_cv_lib_bind_res_querydomain" = "x-yes" ; then
      have_res_querydomain=yes
      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
    else
      AC_MSG_CHECKING([for res_querydomain in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_querydomain(0,0,0,0,0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
                      have_res_querydomain=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for dn_skipname in libc, libbind and libresolv
AC_CHECK_FUNCS(dn_skipname)
if test "x-$ac_cv_func_dn_skipname" = "x-yes" ; then
  have_dn_skipname=yes
else
  AC_CHECK_LIB(resolv, dn_skipname)
  if test "x-$ac_cv_lib_resolv_dn_skipname" = "x-yes" ; then
    have_dn_skipname=yes
    AC_DEFINE(HAVE_DN_SKIPNAME)
  else
    AC_CHECK_LIB(bind, dn_skipname)
    if test "x-$ac_cv_lib_bind_dn_skipname" = "x-yes" ; then
      have_dn_skipname=yes
      AC_DEFINE(HAVE_DN_SKIPNAME)
    else
      AC_MSG_CHECKING([for dn_skipname in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <netinet/in.h>
 #include <resolv.h>]], [[dn_skipname(0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_DN_SKIPNAME)
                      have_dn_skipname=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for res_query in libc, libbind and libresolv
AC_CHECK_FUNCS(res_query)
if test "x-$ac_cv_func_res_query" = "x-yes" ; then
  have_res_query=yes
else
  AC_CHECK_LIB(resolv, res_query)
  if test "x-$ac_cv_lib_resolv_res_query" = "x-yes" ; then
    have_res_query=yes
    AC_DEFINE(HAVE_RES_QUERY)
  else
    AC_CHECK_LIB(bind, res_query)
    if test "x-$ac_cv_lib_bind_res_query" = "x-yes" ; then
      have_res_query=yes
      AC_DEFINE(HAVE_RES_QUERY)
    else
      AC_MSG_CHECKING([for res_query in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_query(0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERY)
                      have_res_query=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi


#  AC_SEARCH_LIBS(res_query,resolv bind,,
#                 AC_SEARCH_LIBS(__res_query,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(dn_expand,resolv bind,,
#                 AC_SEARCH_LIBS(__dn_expand,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(dn_skipname,resolv bind,,
#                 AC_SEARCH_LIBS(__dn_skipname,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(res_querydomain,resolv bind,,
#                 AC_SEARCH_LIBS(__res_querydomain,resolv bind,,have_resolver=no))


dnl Debug
debug="no"
AC_ARG_ENABLE( debug,
               [  --enable-debug          turn on debugging [default=no]],
               [debug="yes"] )
if test "x$debug" = "xyes"; then
      CPPFLAGS="$CPPFLAGS -g3 -fno-inline -DDEBUG"
fi
AC_MSG_CHECKING([whether to enable debug])
AC_MSG_RESULT($debug)

dnl getaddrinfo
getaddrinfo="no"
AC_ARG_ENABLE( getaddrinfo,
               [  --enable-getaddrinfo    use getaddrinfo for dns lookups [default=no]],
               [getaddrinfo="yes"] )
if test "x$getaddrinfo" = "xyes"; then
    AC_CHECK_FUNCS(getaddrinfo,,getaddrinfo="no")
fi
AC_MSG_CHECKING([whether to use getaddrinfo])
AC_MSG_RESULT($getaddrinfo)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h strings.h errno.h arpa/nameser.h)
AC_CHECK_FUNCS(setsockopt,,[AC_CHECK_LIB(socket,setsockopt)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

case "$host_os" in
  *mingw*)
     LIBS="${LIBS} -lwsock32 -lgdi32";;
  *)
     ;;
esac

dnl Generating makefiles.
AC_CONFIG_FILES([
Makefile
gloox.pc
gloox-config
src/Makefile
src/examples/Makefile
src/tests/Makefile
src/tests/adhoc/Makefile
src/tests/adhoccommand/Makefile
src/tests/adhoccommandnote/Makefile
src/tests/amprule/Makefile
src/tests/amp/Makefile
src/tests/base64/Makefile
src/tests/chatstatefilter/Makefile
src/tests/client/Makefile
src/tests/clientbase/Makefile
src/tests/connectionbosh/Makefile
src/tests/dataform/Makefile
src/tests/dataformfield/Makefile
src/tests/dataformitem/Makefile
src/tests/dataformreported/Makefile
src/tests/delayeddelivery/Makefile
src/tests/discoinfo/Makefile
src/tests/discoitems/Makefile
src/tests/disco/Makefile
src/tests/error/Makefile
src/tests/featureneg/Makefile
src/tests/flexofflineoffline/Makefile
src/tests/flexoffline/Makefile
src/tests/gpgencrypted/Makefile
src/tests/gpgsigned/Makefile
src/tests/inbandbytestreamibb/Makefile
src/tests/inbandbytestream/Makefile
src/tests/iq/Makefile
src/tests/jid/Makefile
src/tests/jinglesessionjingle/Makefile
src/tests/jingleaudiortp/Makefile
src/tests/jingleiceudp/Makefile
src/tests/jinglerawudp/Makefile
src/tests/jinglevideortp/Makefile
src/tests/jinglesession/Makefile
src/tests/lastactivityquery/Makefile
src/tests/lastactivity/Makefile
src/tests/md5/Makefile
src/tests/message/Makefile
src/tests/messageeventfilter/Makefile
src/tests/mucroommuc/Makefile
src/tests/mucroommucadmin/Makefile
src/tests/mucroommucowner/Makefile
src/tests/mucroommucuser/Makefile
src/tests/nickname/Makefile
src/tests/nonsaslauthquery/Makefile
src/tests/nonsaslauth/Makefile
src/tests/oob/Makefile
src/tests/parser/Makefile
src/tests/prep/Makefile
src/tests/presence/Makefile
src/tests/privacymanagerquery/Makefile
src/tests/privacymanager/Makefile
src/tests/privatexml/Makefile
src/tests/pubsubmanagerpubsub/Makefile
src/tests/pubsubmanager/Makefile
src/tests/pubsubevent/Makefile
src/tests/receipt/Makefile
src/tests/registrationquery/Makefile
src/tests/registration/Makefile
src/tests/rostermanagerquery/Makefile
src/tests/rostermanager/Makefile
src/tests/searchquery/Makefile
src/tests/search/Makefile
src/tests/sha/Makefile
src/tests/shim/Makefile
src/tests/simanager/Makefile
src/tests/simanagersi/Makefile
src/tests/stanzaextensionfactory/Makefile
src/tests/subscription/Makefile
src/tests/tag/Makefile
src/tests/tlsgnutls/Makefile
src/tests/uniquemucroomunique/Makefile
src/tests/util/Makefile
src/tests/vcard/Makefile
src/tests/vcardupdate/Makefile
src/tests/xpath/Makefile
src/tests/zlib/Makefile
])
AC_OUTPUT
