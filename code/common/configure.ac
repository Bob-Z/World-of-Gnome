dnl Process this file with autoconf to produce a configure script.

AC_INIT(libcommon, 0.1.0)
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE

dnl AC_CONFIG_MACRO_DIR([./])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_PROG_RANLIB

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )


dnl jm_CHECK_TYPE_STRUCT_UTIMBUF
AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS(mkfifo)
AC_CHECK_FUNC(mknod)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
dnl Check for libconfig
PKG_CHECK_MODULES([LIBCONFIG], [libconfig >= 0.20],,
  AC_MSG_ERROR([libconfig 0.20 or newer not found.])
)
dnl Check for liblua
PKG_CHECK_MODULES([LIBLUA], [lualib50])
PKG_CHECK_MODULES([LIBSDL2], [ sdl2 >= 2.0.0])
PKG_CHECK_MODULES([LIBSDL2_NET], [ SDL2_net >= 2.0.0])

CFLAGS+=" ${LIBCONFIG_CFLAGS}"
CFLAGS+=" ${LIBLUA_CFLAGS}"
CFLAGS+=" ${LIBSDL2_CFLAGS}"
CFLAGS+=" ${LIBSDL2_NET_CFLAGS}"

CXXFLAGS+=" ${LIBCONFIG_CFLAGS}"
CXXFLAGS+=" ${LIBLUA_CFLAGS}"
CXXFLAGS+=" ${LIBSDL2_CFLAGS}"
CXXFLAGS+=" ${LIBSDL2_NET_CFLAGS}"

ADD_LIB="$LIBCONFIG_LIBS"
ADD_LIB+="$LIBLUA_LIBS"
ADD_LIB+="$LIBSDL2_LIBS"
ADD_LIB+="$LIBSDL2_NET_LIBS"


AC_SUBST(SET_MAKE)

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(DATADIR)
AC_SUBST(LIBCONFIG_CFLAGS)
AC_SUBST(LIBCONFIG_LIBS)
AC_SUBST(LIBSDL2_CFLAGS)
AC_SUBST(LIBSDL2_LIBS)
AC_SUBST(LIBSDL2_NET_CFLAGS)
AC_SUBST(LIBSDL2_NET_LIBS)


AC_OUTPUT(Makefile)
