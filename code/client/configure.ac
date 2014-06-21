dnl Process this file with autoconf to produce a configure script.

AC_INIT(wog_client, 0.1.0)
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
PKG_CHECK_MODULES(LIBAVFORMAT, [libavformat >= 53.21.1])
PKG_CHECK_MODULES(LIBAVCODEC, [libavcodec >= 53.35.0])
PKG_CHECK_MODULES(LIBAVUTIL, [libavutil])
dnl PKG_CHECK_MODULES(LIBAVFILTER, [libavfilter >= 2.15.0])
PKG_CHECK_MODULES(LIBSWSCALE, [libswscale >= 2.1.0])
PKG_CHECK_MODULES(LIBSDL2_TTF, [SDL2_ttf >= 2.0.12])
PKG_CHECK_MODULES(LIBCONFIG, [libconfig])

CFLAGS="$CFLAGS $GLIB_CFLAGS $GTHREAD_CFLAGS $GIO_CFLAGS $LIBLUA_CFLAGS $SDL2_CFLAGS $LIBAVFORMAT_CFLAGS $LIBAVCODEC_CFLAGS $LIBSWSCALE_CFLAGS $LIBSDL2_TTF_CFLAGS $LIBCONFIG_CFLAGS $LIBAVUTIL_CFLAGS"
dnl CPPFLAGS="$CPPFLAGS $GLIB_CFLAGS $GTHREAD_CFLAGS $GIO_CFLAGS $GTK_CFLAGS $LIBLUA_CFLAGS"
ADD_LIB="$GLIB_LIBS $GTHREAD_LIBS $GIO_LIBS $LIBLUA_LIBS $SDL2_LIBS $LIBAVFORMAT_LIBS $LIBAVCODEC_LIBS $LIBSWSCALE_LIBS $LIBSDL2_TTF_LIBS $LIBCONFIG_LIBS -lgif $LIBAVUTIL_LIBS"

AC_SUBST(SET_MAKE)
AC_SUBST(ADD_LIB)

dnl expand ${datadir} and write it out
conftemp="${datadir}"
eval conftemp=\"${conftemp}\"
eval conftemp=\"${conftemp}\"
AC_DEFINE_UNQUOTED(DATADIR, "${conftemp}",[Default path for data files])

AC_SUBST(DATADIR)


AC_OUTPUT(Makefile)
