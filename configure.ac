#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([auctionManager],[0.1],[la.marentes455@uniandes.edu.co])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT
AC_CONFIG_SRCDIR([foundation/src/AuctionManager.cpp])

## ------------------------Begin instantiation part 
## Meaning:
## An instantiating macro is one which defines one or more tags, 
## usually referring to files that are to be translated by the generated 
## configure scripts, from a template containing Autoconf substitution 
## variables.

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Checks for build environment
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL

LT_INIT([dlopen])
AC_PROG_MAKE_SET

AC_LANG([C++])

AC_CHECK_LIB([dl], [dlopen])


dnl # Platform 
case "$target" in
*-linux*)
        OS=LINUX
        AC_DEFINE(LINUX, 1, "Linux OS")
        ;;
*-sunos*)
        OS=SUNOS
        AC_DEFINE(SUNOS, 1, "SUNOS")
        ;;
*-irix*)
        OS=IRIX
        AC_DEFINE(IRIX, 1, "IRIX OS")
        ;;
*-solaris*)
        OS=SOLARIS
        AC_DEFINE(SOLARIS, 1, "Solaris OS")
        ;;
*-freebsd*)
        OS=FREEBSD
        AC_DEFINE(FREEBSD, 1, "FreeBSD OS")
        ;;
*dec-os*)
        OS=ALPHA
        AC_DEFINE(ALPHA, 1, "Alpha")
        ;;
*)
        OS=UNIX
        AC_DEFINE(UNIX, 1, "Unix")
        ;;
esac

dnl # Hardware

case "$target" in
*i*86*)
        PROC=LINUX
        AC_DEFINE(INTEL, 1, "Intel")
        ;;
esac

# use our own defaults
LDFLAGS="$LDFLAGS -L/lib -L/usr/local/lib -L/usr/lib "
CFLAGS="$_CFLAGS -Wall"
CXXFLAGS="$_CXXFLAGS -Wall -Wno-deprecated "

dnl # check the options

AC_ARG_WITH([ssl],[  --with-ssl],[LDFLAGS="$LDFLAGS -L$withval/lib"
   CXXFLAGS="$CXXFLAGS -I$withval/include"
   CFLAGS="$CFLAGS -I$withval/include"
   CPPFLAGS="$CPPFLAGS -I$withval/include"
   SSL_DIR=$withval
],[])

AC_ARG_WITH([xml2],[  --with-xml2],[XML2_DIR=$withval
],[])

AC_ARG_WITH([xslt],[  --with-xslt],[XSLT_DIR=$withval
],[])


AC_ARG_ENABLE(debug,
  [  --enable-debug            enable debug build ],
  [case "${enableval}" in
    yes) debug=true ;;
    no)  debug=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
  esac],[debug=false])
AM_CONDITIONAL(ENABLE_DEBUG, test x$debug = xtrue)
if test $debug = true ; then
  AC_DEFINE(ENABLE_DEBUG, 1, [enable debug build])
fi

AC_ARG_ENABLE(test,
  [  --enable-test            enable all test applications ],
  [case "${enableval}" in
    yes) tst=true ;;
    no)  tst=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-test) ;;
  esac],[tst=false])
AM_CONDITIONAL(ENABLE_TEST, test x$tst = xtrue)
if test $tst = true ; then
  AC_DEFINE(ENABLE_TEST, 1, [enable test applications])
fi

AC_ARG_ENABLE(mp,
  [  --enable-mp            enable mpatrol memory surveillance ],
  [case "${enableval}" in
    yes) mp=true ;;
    no)  mp=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-mp) ;;
  esac],[mp=false])
AM_CONDITIONAL(ENABLE_MP, test x$mp = xtrue)
if test $mp = true ; then
  AC_DEFINE(ENABLE_MP, 1, [enable mpatrol linking])
fi

AC_ARG_ENABLE(ssl,
  [  --enable-ssl            enable ssl ],
  [case "${enableval}" in
    yes) ssl=true ;;
    no)  ssl=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-ssl) ;;
  esac],[ssl=false])
AM_CONDITIONAL(ENABLE_SSL, test x$ssl = xtrue)
if test $ssl = true ; then
  if test -z "$SSL_DIR" && test -d /usr/local/ssl ; then
    LDFLAGS="$LDFLAGS -L/usr/local/ssl/lib"
    CXXFLAGS="$CXXFLAGS -I/usr/local/ssl/include"
    CFLAGS="$CFLAGS -I/usr/local/ssl/include"
    CPPFLAGS="$CPPFLAGS -I/usr/local/ssl/include"
  fi
  LIBS="$LIBS -lcrypto"

  AC_CHECK_HEADERS([openssl/ssl.h openssl/err.h openssl/bio.h openssl/evp.h],,
  [AC_MSG_ERROR([cannot find ssl headers])])
  AC_CHECK_LIB(ssl, [SSL_library_init],
  [SSLLIB="-lssl"],
  [AC_MSG_ERROR([cannot find open ssl])])
  USE_SSL="-DUSE_SSL"

  SSL_PASSWD=netmate
  AC_DEFINE_UNQUOTED(SSL_PASSWD, "$SSL_PASSWD", [password for ssl encryption])
  AC_DEFINE(USE_SSL, 1, [use ssl flag])
fi


AC_ARG_ENABLE(threads,
  [  --enable-threads            enable threads ],
  [case "${enableval}" in
    yes) threads=true ;;
    no)  threads=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-threads) ;;
  esac],[threads=false])
AM_CONDITIONAL(ENABLE_THREADS, test x$threads = xtrue)
if test $threads = true ; then
  AC_DEFINE(ENABLE_THREADS, 1, [enable multi threading])
fi



if test $mp = true ; then
    LDFLAGS="$LDFLAGS -lbfd"	
    if test $threads = true ; then
       LDFLAGS="$LDFLAGS -lpthread"	
       AC_CHECK_LIB(mpatrolmt, xmalloc,
       [MPATROLLIB="-lmpatrolmt"],
       [AC_MSG_ERROR([cannot find libmpatrolmt])])
    else
       AC_CHECK_LIB(mpatrol, xmalloc,
       [MPATROLLIB="-lmpatrol"],
       [AC_MSG_ERROR([cannot find libmpatrol])])
    fi
fi

READLINELIB=""
# some readlines (Redhat) need -ltermcap
AC_CHECK_LIB(termcap, tgetent, LDFLAGS="$LDFLAGS -ltermcap",[])
AC_CHECK_HEADERS([readline/readline.h readline/history.h],,
  [AC_MSG_ERROR([cannot find libreadline headers])])
AC_CHECK_LIB(readline, readline,
  [READLINELIB="$READLINELIB -lreadline"],
  [AC_MSG_ERROR([cannot find libreadline])])


dnl # check size

AC_CHECK_SIZEOF(off_t)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([net/bpf.h net/ethernet.h ether.h arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h termios.h unistd.h float.h types.h limits.h ])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([alarm bzero floor getaddrinfo inet_ntoa gettimeofday memmove memset select socket strdup strchr strerror strstr atoll strtof getopt_long])


if test $threads = true ; then
  AC_CHECK_HEADERS([pthread.h])
  if test $OS = FREEBSD ; then
    OCCFLAGS=$CXXFLAGS
    OCFLAGS=$CFLAGS
    CXXFLAGS="$CXXFLAGS -D_THREAD_SAFE -I/usr/local/include/pthread/linuxthreads"
    CFLAGS="$CFLAGS -D_THREAD_SAFE -I/usr/local/include/pthread/linuxthreads"
    have_linux_threads="yes"
    AC_CHECK_LIB(lthread, pthread_create,
    [PTHREADLIB="-llthread -llgcc_r"],[ have_linux_threads="no" ] )
    #if no linuxthreads try libc_r 
    CXXFLAGS="$OCXXFLAGS -pthread"
    CFLAGS="$OCFLAGS -pthread"
    AC_CHECK_LIB(c_r, pthread_create, [PTHREADLIB="-lc_r"])
    # actually newer FREEBSD version should have pthread... 
  else
    AC_CHECK_LIB(pthread, pthread_create,
      [PTHREADLIB="-lpthread"])
  fi
fi



dnl # Checks for XML lib
if test -z "$XML2_DIR" ; then 
  AC_PATH_PROG(XML_CONFIG, xml2-config, no) 
  if test "$XML_CONFIG" = "no" ; then
    AC_MSG_WARN([cannot find xml2-config])
  else
    XML_LIBS=`$XML_CONFIG --libs` 
    XML_CFLAGS=`$XML_CONFIG --cflags` 
    LDFLAGS="$LDFLAGS $XML_LIBS" 
    CFLAGS="$CFLAGS $XML_CFLAGS" 
    CXXFLAGS="$CXXFLAGS $XML_CFLAGS" 
    CPPFLAGS="$CPPFLAGS $XML_CFLAGS" 
  fi
else          
   LDFLAGS="$LDFLAGS -L$XML2_DIR/lib -lz -lm"          
   CFLAGS="$CFLAGS -I$XML2_DIR/include" 
   CXXFLAGS="$CXXFLAGS -I$XML2_DIR/include" 
   CPPFLAGS="$CPPFLAGS -I$XML2_DIR/include" 
fi

if test "$XSLT_DIR" ; then
   LDFLAGS="$LDFLAGS -L$XSLT_DIR/lib"          
   CFLAGS="$CFLAGS -I$XSLT_DIR/include" 
   CXXFLAGS="$CXXFLAGS -I$XSLT_DIR/include"
   CPPFLAGS="$CPPFLAGS -I$XSLT_DIR/include"
fi

dnl # check xml2 lib
AC_CHECK_HEADERS([libxml/parser.h],,[AC_MSG_ERROR([cannot find libxml2 headers])]) 
AC_CHECK_LIB(xml2, xmlParseFile,
  [XMLLIB="-lxml2"],
  [AC_MSG_ERROR([cannot find libxml2])])

dnl # check xslt lib
AC_CHECK_HEADERS([libxslt/transform.h libxslt/xsltutils.h],,
  [AC_MSG_WARN([cannot find xslt headers])])
AC_CHECK_LIB(xslt, xsltApplyStylesheet,
  [XSLTLIB="-lxslt"],
  [AC_MSG_WARN([cannot find libxsltl])])


if test "$CURL_DIR" ; then
   LDFLAGS="$LDFLAGS -L$CURL_DIR/lib -lxslt"          
   CFLAGS="$CFLAGS -I$CURL_DIR/include" 
   CXXFLAGS="$CXXFLAGS -I$CURL_DIR/include"
   CPPFLAGS="$CPPFLAGS -I$CURL_DIR/include"
fi

dnl # check for libcurl
# Check CURL, needed for install netmate policy rules.
CURL_LIBS=
PKG_CHECK_MODULES(CURL, 
  libcurl, 
  [AC_DEFINE([HAVE_CURL],[1],[Use CURL])])
AM_CONDITIONAL(CURL, test -n "$CURL_LIBS")
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

PKG_CHECK_MODULES(PQXX, libpqxx >= 4.0.1, 
						[AC_DEFINE([HAVE_PQXX40], [1], [Use PQXX40])], 
						[PKG_CHECK_MODULES(PQXX, libpqxx >= 3.1, [AC_DEFINE([HAVE_PQXX31], [1], [Use PQXX31])])]
)
AC_SUBST(PQXX_CFLAGS)
AC_SUBST(PQXX_LIBS)

   
PKG_CHECK_MODULES(OPENSSL, openssl  >= 0.9.8,
        CXXFLAGS="$CXXFLAGS `pkg-config --cflags openssl`";
        LIBS="$LIBS -lcrypto `pkg-config --libs-only-L --libs-only-other openssl`")
        

PKG_CHECK_MODULES([LIBUUID], [libuuid],
   [LIBUUID_FOUND=yes], [LIBUUID_FOUND=no])
   if test "$LIBUUID_FOUND" = "no" ; then
        PKG_CHECK_MODULES([LIBUUID], [uuid],
			      [LIBUUID_FOUND=yes], [LIBUUID_FOUND=no])
	    if test "$LIBUUID_FOUND" = "no" ; then
	  	  AC_MSG_ERROR([libuuid development files required])
	    else
		  LIBUUID_INCLUDEDIR=$(pkg-config --variable=includedir uuid)
		  LIBUUID_CFLAGS+=" -I$LIBUUID_INCLUDEDIR/uuid "
	    fi
   fi
AC_SUBST([LIBUUID_CFLAGS])
AC_SUBST([LIBUUID_LIBS])



PKG_CHECK_MODULES([LIBPROT], [libprot-0.0])
AC_SUBST([LIBPROT_CFLAGS])
AC_SUBST([LIBPROT_LIBS])

PKG_CHECK_MODULES([LIBFASTQUEUE], [libfastqueue-0.0])
AC_SUBST([LIBFASTQUEUE_CFLAGS])
AC_SUBST([LIBFASTQUEUE_LIBS])

PKG_CHECK_MODULES([LIBGIST], [libgist-0.0])
AC_SUBST([LIBGIST_CFLAGS])
AC_SUBST([LIBGIST_LIBS])

PKG_CHECK_MODULES([LIBIPAP], [libipap-0.0])
AC_SUBST([LIBIPAP_CFLAGS])
AC_SUBST([LIBIPAP_LIBS])

PKG_CHECK_MODULES([LIBANSLP_MSG], [libanslp_msg-0.0])
AC_SUBST([LIBANSLP_MSG_CFLAGS])
AC_SUBST([LIBANSLP_MSG_LIBS])

PKG_CHECK_MODULES([LIBANSLP], [libanslp-0.0])
AC_SUBST([LIBANSLP_CFLAGS])
AC_SUBST([LIBANSLP_LIBS])


dnl # dont try to build nagtrsh if we don't have the libs
AM_CONDITIONAL(BUILD_NAGTRSH, test "$XSLTLIB" && test "$CURL_LIBS") 


dnl # checks for programs

AC_CHECK_PROG(DOXY,doxygen,doxygen,[])
if test "$DOXY" = "" ; then
  AC_MSG_WARN([doxygen not installed, can not build documentation])
fi

dnl # set version and data

dnl # Output substitution

AC_DEFINE_UNQUOTED(BUILD_TIME, "`date +"%d/%m/%Y %H:%M:%S"`", "release date")

dnl # set default directories

if test "${prefix}" = "NONE" ; then
  prefix=${ac_default_prefix}
fi

dnl # set default config directory for testing
sysconfdir="${sysconfdir}/$PACKAGE"

DEF_SYSCONFDIR=`eval echo ${sysconfdir}`
AC_DEFINE_UNQUOTED(DEF_SYSCONFDIR, "$DEF_SYSCONFDIR", "sysconfig dir")

DEF_STATEDIR=`eval echo ${localstatedir}`
AC_DEFINE_UNQUOTED(DEF_STATEDIR, "$DEF_STATEDIR", "state dir")

libdir="${libdir}/$PACKAGE"
if test "${exec_prefix}" = "NONE" ; then
  exec_prefix=${prefix}
fi
DEF_LIBDIR=`eval echo ${libdir}`
AC_DEFINE_UNQUOTED(DEF_LIBDIR, "$DEF_LIBDIR", "lib dir")

DEF_BINDIR=`eval echo ${bindir}`
AC_DEFINE_UNQUOTED(DEF_BINDIR, "$DEF_BINDIR", "bin dir")

pkgconfigdir='${exec_prefix}/lib/pkgconfig'
	
	

AC_SUBST(VERSION)
AC_SUBST(REL_DATE)
AC_SUBST(OS)
AC_SUBST(ENABLE_NF)
AC_SUBST(ENABLE_TEST)
AC_SUBST(ENABLE_DEBUG)
AC_SUBST(READLINELIB)
AC_SUBST(HISTORYLIB)
AC_SUBST(PTHREADLIB)
AC_SUBST(DLLIB)
AC_SUBST(SSLLIB)
AC_SUBST(ULOGLIB)
AC_SUBST(XMLLIB)
AC_SUBST(XSLTLIB)
AC_SUBST(MATHLIB)
AC_SUBST(USE_SSL)
AC_SUBST(SSL_PASSWD)
AC_SUBST(IPTABLES_VERSION)
AC_SUBST(MPATROLLIB)
AC_SUBST(DEF_SYSCONFDIR)
AC_SUBST(DEF_STATEDIR)
AC_SUBST(DEF_LIBDIR)
AC_SUBST(DEF_BINDIR)
AC_SUBST(DOXY)
AC_SUBST(pkgconfigdir)


AC_CONFIG_FILES([Makefile \
				 libauctionfdtion-0.0.pc \
				 lib/Makefile \
				 lib/getpass/Makefile \
  				 lib/httpd/Makefile \
				 lib/getopt_long/Makefile \
				 etc/Makefile \
				 etc/netaum.conf.xml \
				 etc/netagnt.conf.xml \
				 etc/netagnt.thread.conf.xml \
				 nagtrsh/Makefile \
				 foundation/src/Makefile \
				 foundation/test/Makefile \
				 foundation/Makefile \
				 proc_modules/Makefile \
				 auctioner/src/Makefile \
				 auctioner/test/Makefile \
				 auctioner/Makefile \
                 user_agent/src/Makefile
                 user_agent/test/Makefile
                 user_agent/Makefile ] )

AC_OUTPUT
