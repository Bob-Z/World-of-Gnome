dnl Process this file with autoconf to produce a configure script.

AC_INIT(wog_server, 0.1.0)
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE

dnl AC_CONFIG_MACRO_DIR([./])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AM_PROG_CC_C_O

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
PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])
PKG_CHECK_MODULES(GIO, [gio-2.0])
dnl Check for liblua
PKG_CHECK_MODULES([LIBLUA], [lualib50])
PKG_CHECK_MODULES([LIBCONFIG], [libconfig])
PKG_CHECK_MODULES([LIBSDL2], [ sdl2 >= 2.0.0])

CFLAGS="$CFLAGS $GLIB_CFLAGS $GTHREAD_CFLAGS $GIO_CFLAGS $LIBLUA_CFLAGS $LIBCONFIG_CFLAGS $LIBSDL2_CFLAGS"
dnlCPPFLAGS="$CPPFLAGS $GLIB_CFLAGS $GTHREAD_CFLAGS $GIO_CFLAGS $GTK_CFLAGS $LIBLUA_CFLAGS"
ADD_LIB="$GLIB_LIBS $GTHREAD_LIBS $GIO_LIBS $LIBLUA_LIBS $LIBCONFIG_LIBS $LIBSDL2_LIBS"

AC_SUBST(SET_MAKE)
AC_SUBST(ADD_LIB)

AC_OUTPUT(Makefile)
