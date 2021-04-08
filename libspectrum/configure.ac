dnl Process this file with autoconf to produce a configure script.
dnl Copyright (c) 1999-2012 Philip Kendall

dnl $Id: configure.ac 4986 2013-05-24 19:03:20Z zubzero $

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
dnl
dnl Author contact information:
dnl
dnl E-mail: philip-fuse@shadowmagic.org.uk

dnl Package version
m4_define([libspectrum_version], [1.1.1])

dnl Product full version
m4_define([libspectrum_major_version], [1])
m4_define([libspectrum_minor_version], [1])
m4_define([libspectrum_micro_version], [1])
m4_define([libspectrum_nano_version],  [0])
m4_define([libspectrum_full_version],
          [libspectrum_major_version.libspectrum_minor_version.libspectrum_micro_version.libspectrum_nano_version])
m4_define([libspectrum_rc_version],
          [libspectrum_major_version,libspectrum_minor_version,libspectrum_micro_version,libspectrum_nano_version])

dnl Package info
m4_define([libspectrum_copyright], ["(c) 1999-2013 Philip Kendall and others"])
m4_define([libspectrum_url],
          [http://fuse-emulator.sourceforge.net/libspectrum.php])
m4_define([libspectrum_bugreport],
          [http://sourceforge.net/p/fuse-emulator/bugs/])

AC_INIT([libspectrum],[libspectrum_version],[libspectrum_bugreport],
        [libspectrum],[libspectrum_url])
AC_CONFIG_SRCDIR([libspectrum.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

dnl Use automake to produce `Makefile.in'
AM_INIT_AUTOMAKE

dnl Substitutions for .in files
LIBSPECTRUM_FULL_VERSION=libspectrum_full_version
LIBSPECTRUM_COPYRIGHT=libspectrum_copyright
LIBSPECTRUM_URL=libspectrum_url
AC_SUBST(LIBSPECTRUM_FULL_VERSION)
AC_SUBST(LIBSPECTRUM_COPYRIGHT)
AC_SUBST(LIBSPECTRUM_URL)

dnl Definitions for config.h
AC_DEFINE([LIBSPECTRUM_RC_VERSION], [libspectrum_rc_version],
          [Define version information for win32 dll library])
AC_DEFINE([LIBSPECTRUM_COPYRIGHT], [libspectrum_copyright],
          [Define copyright of libspectrum])

dnl Checks for programs.
AC_PROG_CC

dnl Setup for compiling build tools (make-perl)
if test $cross_compiling = yes; then
  AC_CHECK_PROGS(CC_FOR_BUILD, gcc cc)
else
  CC_FOR_BUILD=$CC
fi
AC_MSG_CHECKING([for a C compiler for build tools])
AC_MSG_RESULT([$CC_FOR_BUILD])
AC_SUBST(CC_FOR_BUILD)

LT_INIT
AC_PATH_PROG(PERL, perl)
AC_SUBST(PERL)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdint.h strings.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Check for host specific programs
WINDRES_OBJ=
WINDRES_LDFLAGS=
case "$host_os" in
  mingw32*)
    AC_CHECK_TOOL([WINDRES], [windres], [no])
    if test "$WINDRES" != no; then
      WINDRES_OBJ="windres.o"
      WINDRES_LDFLAGS="-Xlinker windres.o"
    fi
    AC_DEFINE([WIN32_LEAN_AND_MEAN],, [Exclude rarely used stuff from Windows headers <windows.h>])
    ;;
esac
AC_SUBST(WINDRES_OBJ)
AC_SUBST(WINDRES_LDFLAGS)

dnl Check for big endianness
AC_C_BIGENDIAN

dnl Check for functions
AC_CHECK_FUNCS(_snprintf _stricmp _strnicmp snprintf strcasecmp strncasecmp)

dnl Allow the user to say that various libraries are in one place
AC_ARG_WITH(local-prefix,
[  --with-local-prefix=PFX local libraries installed in PFX (optional)],
CPPFLAGS="$CPPFLAGS -I$withval/include"; LDFLAGS="$LDFLAGS -L$withval/lib",
if test "$prefix" != "NONE"; then
  CPPFLAGS="$CPPFLAGS -I$prefix/include"; LDFLAGS="$LDFLAGS -L$prefix/lib"
fi)

dnl Check whether to use zlib (the UNIX version is called z, Win32 zdll)
AC_MSG_CHECKING(whether to use zlib)
AC_ARG_WITH(zlib,
[  --without-zlib          don't use zlib],
if test "$withval" = no; then zlib=no; else zlib=yes; fi,
zlib=yes)
AC_MSG_RESULT($zlib)
if test "$zlib" = yes; then
  AC_CHECK_HEADERS(
    zlib.h,
    AC_SEARCH_LIBS(compress2, z zdll)
  )
fi

dnl Check whether to use libgcrypt
AC_MSG_CHECKING(whether to use libgcrypt)
AC_ARG_WITH(libgcrypt,
[  --without-libgcrypt     don't use libgcrypt],
if test "$withval" = no; then libgcrypt=no; else libgcrypt=yes; fi,
libgcrypt=yes)
AC_MSG_RESULT($libgcrypt)
if test "$libgcrypt" = yes; then
  AC_CHECK_HEADERS(gcrypt.h,LIBS="$LIBS -lgcrypt")
fi

dnl Check whether to use libbz2 (1.0 or greater)
AC_MSG_CHECKING(whether to use libbz2)
AC_ARG_WITH(bzip2,
[  --without-bzip2         don't use libbz2],
if test "$withval" = no; then bzip2=no; else bzip2=yes; fi,
bzip2=yes)
AC_MSG_RESULT($bzip2)
if test "$bzip2" = yes; then
  AC_CHECK_HEADER(
    bzlib.h,
    AC_CHECK_LIB(bz2,BZ2_bzDecompressInit)
  )
fi

dnl Either find glib or use the replacement
AC_MSG_CHECKING(whether to use glib)
AC_ARG_WITH(glib,
[  --without-glib          don't use glib],
if test "$withval" = no; then glib=no; else glib=yes; fi,
glib=yes)
AC_MSG_RESULT($glib)
AS_IF([test "$glib" = yes], [
  PKG_CHECK_MODULES(
    GLIB,
    glib-2.0,
    AC_DEFINE([HAVE_LIB_GLIB], 1, [Defined if we've got glib]),
    glib="no"
  )
])

AM_CONDITIONAL(USE_MYGLIB, test "$glib" = no)

dnl If it appears we're using gcc as our compiler, turn on warnings
if test "$ac_cv_prog_gcc" = yes; then
  CFLAGS="$CFLAGS -Wall"
  dnl And possibly lots of warnings
  AC_MSG_CHECKING(whether lots of warnings requested)
  AC_ARG_ENABLE(warnings,
  [  --enable-warnings       give lots of warnings if using gcc],
  if test "$enableval" = yes; then
    warnings=yes;
  else
    warnings=no;
  fi,
  warnings=no)
  AC_MSG_RESULT($warnings)
  if test "$warnings" = yes; then
      CFLAGS="$CFLAGS -Wstrict-prototypes -Wmissing-prototypes -Winline -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Waggregate-return -W -Wsign-compare"
  fi
fi

dnl Check whether to use libaudiofile
AC_MSG_CHECKING(whether to use libaudiofile)
AC_ARG_WITH(libaudiofile,
[  --without-libaudiofile  don't use libaudiofile],
if test "$withval" = no; then libaudiofile=no; else libaudiofile=yes; fi,
libaudiofile=yes)
AC_MSG_RESULT($libaudiofile)
AS_IF([test "$libaudiofile" = yes], [
  PKG_CHECK_MODULES(
    AUDIOFILE,
    audiofile,
    AC_DEFINE([HAVE_LIB_AUDIOFILE], 1, [Defined if we've got audiofile]),
    [true]
  )
  if test -z "$AUDIOFILE_LIBS"; then
    AM_PATH_AUDIOFILE(
      0.2.3,
      AC_DEFINE([HAVE_LIB_AUDIOFILE], 1, [Defined if we've got audiofile])
    )
  fi
])

AC_CONFIG_FILES([
  Makefile
  libspectrum.qpg
])

AC_OUTPUT
