dnl Process this file with autoconf to produce a configure script.
dnl $Id: configure.ac 4995 2013-05-24 19:54:19Z zubzero $

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
m4_define([fuse_version], [1.1.1])

dnl Product full version
m4_define([fuse_major_version], [1])
m4_define([fuse_minor_version], [1])
m4_define([fuse_micro_version], [1])
m4_define([fuse_nano_version],  [0])
m4_define([fuse_full_version], [fuse_major_version.fuse_minor_version.fuse_micro_version.fuse_nano_version])
m4_define([fuse_rc_version], [fuse_major_version,fuse_minor_version,fuse_micro_version,fuse_nano_version])

dnl Package info
m4_define([fuse_copyright], ["(c) 1999-2013 Philip Kendall and others"])
m4_define([fuse_url], [http://fuse-emulator.sourceforge.net/])
m4_define([fuse_bugreport], [http://sourceforge.net/p/fuse-emulator/bugs/])

AC_INIT([fuse],[fuse_version],[fuse_bugreport],[fuse],[fuse_url])
AC_CONFIG_SRCDIR(fuse.c)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(config.h)
AC_CANONICAL_HOST

dnl Use automake to produce `Makefile.in'
AM_INIT_AUTOMAKE

dnl Substitutions for .in files
FUSE_FULL_VERSION=fuse_full_version
FUSE_COPYRIGHT=fuse_copyright
FUSE_URL=fuse_url
AC_SUBST(FUSE_FULL_VERSION)
AC_SUBST(FUSE_COPYRIGHT)
AC_SUBST(FUSE_URL)

dnl Definitions for config.h
AC_DEFINE([FUSE_RC_VERSION], [fuse_rc_version],
          [Define version information for win32 executables])
AC_DEFINE([FUSE_COPYRIGHT], [fuse_copyright], [Define copyright of Fuse])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PATH_PROG(PERL, perl)
AC_SUBST(PERL)
AM_PROG_LEX
AC_PROG_YACC
LT_INIT

dnl Check for host specific programs
WINDRES_OBJ=
case "$host_os" in
  mingw32*)
    AC_CHECK_TOOL([WINDRES], [windres], [no])
    if test "$WINDRES" != no; then
      WINDRES_OBJ="windres.o"
    fi
    AC_CHECK_PROGS([MAN2HTML], [man2html])
    AC_CHECK_PROGS([GROFF], [groff])
    AC_CHECK_PROGS([UNIX2DOS], [unix2dos])
    ;;
esac
AC_SUBST(WINDRES_OBJ)

dnl Generate commandy.h
YFLAGS="$YFLAGS -d"
AC_SUBST(YFLAGS)

dnl Check for big endianness
AC_C_BIGENDIAN

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(
  libgen.h \
  siginfo.h \
  strings.h \
  sys/soundcard.h \
  sys/audio.h \
  sys/audioio.h
)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl Checks for library functions.
AC_CHECK_FUNCS(dirname geteuid getopt_long mkstemp fsync)
AC_CHECK_LIB([m],[cos])

dnl Allow the user to say that various libraries are in one place
AC_ARG_WITH(local-prefix,
[  --with-local-prefix=PFX local libraries installed in PFX (optional)],
CPPFLAGS="$CPPFLAGS -I$withval/include"; LDFLAGS="$LDFLAGS -L$withval/lib",
if test "$prefix" != "NONE"; then
  CPPFLAGS="$CPPFLAGS -I$prefix/include"; LDFLAGS="$LDFLAGS -L$prefix/lib"
fi)

dnl Select a user interface

dnl Try various UI options; set the `ui' variable when we've got one
dnl we like, and then don't look at the other options

AC_PATH_XTRA

dnl Look for Win32 UI (default=no)
AC_MSG_CHECKING(whether Win32 UI requested)
AC_ARG_WITH(win32,
[  --with-win32            use Win32 for user interface],
if test "$withval" = no; then win32=no; else win32=yes; fi,
win32=no)
AC_MSG_RESULT($win32)
if test "$win32" = yes; then
  AC_CHECK_HEADER(windows.h,
                  LIBS="$LIBS -mwindows -lcomctl32 -lwinmm";
                  AC_DEFINE([UI_WIN32], 1, [Defined if Win32 UI in use])
                  AC_DEFINE([WINVER], 0x0400, [Minimal supported version of Windows is 95 or NT4])
                  AC_DEFINE([_WIN32_IE], 0x400, [Internet Explorer is 4.0 or higher is required])
                  AC_DEFINE([WIN32_LEAN_AND_MEAN],, [Exclude rarely used stuff from Windows headers <windows.h>])
                  UI=win32; UI_LIBS="ui/win32/libuiwin32.a",
                  AC_MSG_ERROR([Win32 UI not found]))
fi

dnl Look for svgalib (default=no)
AC_MSG_CHECKING(whether svgalib UI requested)
AC_ARG_WITH(svgalib,
[  --with-svgalib          use svgalib for user interface],
if test "$withval" = no; then svgalib=no; else svgalib=yes; fi,
svgalib=no)
AC_MSG_RESULT($svgalib)
if test "$svgalib" = yes; then
  AC_CHECK_HEADER(vga.h,
                  LIBS="$LIBS -lvga";
                  AC_DEFINE([UI_SVGA], 1, [Defined if svgalib UI in use])
		  AC_DEFINE([USE_WIDGET], 1,
		            [Defined if we're using a widget-based UI])
                  UI=svga; WIDGET=widget;
                  UI_LIBS="ui/svga/libuisvga.a ui/widget/libwidget.a",
                  AC_MSG_ERROR([svgalib not found]))
fi

dnl Look for framebuffer interface for SA1110 (default=no)
if test -z "$UI"; then
  AC_MSG_CHECKING(whether framebuffer UI requested)
  AC_ARG_WITH(fb,
  [  --with-fb               use framebuffer for user interface],
  if test "$withval" = no; then fb=no; else fb=yes; fi,
  fb=no)
  AC_MSG_RESULT($fb)
  if test "$fb" = yes; then
    AC_DEFINE([UI_FB], 1, [Defined if framebuffer UI in use])
    AC_DEFINE([USE_WIDGET], 1, [Defined if we're using a widget-based UI])
    UI=fb; WIDGET=widget;
    UI_LIBS="ui/fb/libuifb.a ui/widget/libwidget.a"
    dnl Look for libgpm (default=yes)
    AC_MSG_CHECKING(whether gpm requested)
    AC_ARG_WITH(gpm,
    [  --without-gpm           don't use libgpm for e.g. Kempston mouse],
    if test "$withval" = no; then gpm=no; else gpm=yes; fi,
    gpm=yes)
    AC_MSG_RESULT($gpm)
    dnl Note: we use libdl to get libgpm at runtime
    if test "$gpm" = yes; then
      AC_CHECK_HEADER(gpm.h,
		      UI_LIBS="$UI_LIBS -ldl";
		      AC_DEFINE([HAVE_GPM_H], 1, [Defined if gpm in use]),
		      AC_MSG_ERROR([libgpm not found]))
    fi
  fi
fi

dnl Look for Wii interface (default = 0)
if test -z "$UI"; then
  AC_MSG_CHECKING(whether Wii UI requested)
  AC_ARG_WITH(wii,
  [  --with-wii               use Wii FB for user interface],
  if test "$withval" = no; then wii=no; else wii=yes; fi,
  wii=no)
  AC_MSG_RESULT($wii)
  if test "$wii" = yes; then
    AC_DEFINE([UI_WII], 1, [Defined if Wii UI in use])
    AC_DEFINE([USE_WIDGET], 1, [Defined if we're using a widget-based UI])
    AC_DEFINE([USE_JOYSTICK], 1, [Defined if we're using hardware joysticks])
    UI=wii; WIDGET=widget;
    UI_LIBS="ui/wii/libuiwii.a ui/widget/libwidget.a"
    host_os=wii
    dnl Look for libgpm (default=yes)
    AC_MSG_CHECKING(whether gpm requested)
    AC_ARG_WITH(gpm,
    [  --without-gpm           don't use libgpm for e.g. Kempston mouse],
    if test "$withval" = no; then gpm=no; else gpm=yes; fi,
    gpm=yes)
    AC_MSG_RESULT($gpm)
    dnl Note: we use libdl to get libgpm at runtime
    if test "$gpm" = yes; then
      AC_CHECK_HEADER(gpm.h,
		      UI_LIBS="$UI_LIBS -ldl";
		      AC_DEFINE([HAVE_GPM_H], 1, [Defined if gpm in use]),
		      AC_MSG_ERROR([libgpm not found]))
    fi
  fi
fi

dnl Look for SDL (default=no)
if test -z "$UI"; then
  AC_MSG_CHECKING(whether SDL UI requested)
  AC_ARG_WITH(sdl,
  [  --with-sdl              use SDL for user interface],
  if test "$withval" = no; then sdl=no; else sdl=yes; fi,
  sdl=no)
  AC_MSG_RESULT($sdl)
  if test "$sdl" = yes; then
    SDL_VERSION=1.2.4
    AM_PATH_SDL($SDL_VERSION,
		AC_DEFINE([UI_SDL], 1, [Defined if the SDL UI in use])
		AC_DEFINE([USE_WIDGET], 1,
		          [Defined if we're using a widget-based UI])
		UI=sdl; WIDGET=widget;
	        UI_LIBS="ui/sdl/libuisdl.a ui/widget/libwidget.a", 
                AC_MSG_ERROR([SDL version $SDL_VERSION not found]))
  fi
fi

dnl Look for GTK+ (default=yes)
if test -z "$UI"; then
  AC_MSG_CHECKING(whether GTK+ UI requested)
  AC_ARG_WITH(gtk,
  [  --without-gtk           don't use GTK+ for user interface],
  if test "$withval" = no; then gtk=no; else gtk=yes; fi,
  gtk=yes)
  AC_MSG_RESULT($gtk)

  if test "$gtk" = yes; then

    AC_ARG_ENABLE(gtk3,
      AS_HELP_STRING([--enable-gtk3],[prefer GTK+ 3 to GTK+ 2]),
      if test "$enableval" = yes; then gtk3=yes; else gtk3=no; fi,
      gtk3=auto
    )

    if test "$gtk3" = yes; then
      PKG_CHECK_MODULES(GTK, [gtk+-3.0],,
        AC_MSG_ERROR([GTK+ not found])
      )

    elif test "$gtk3" = no; then
      AM_PATH_GTK_2_0(2.18.0,,
        AC_MSG_WARN([GTK+ not found. Reverting to Xlib])
        gtk=no
      )

    elif test "$gtk3" = auto; then
      AM_PATH_GTK_2_0(2.18.0,
        gtk3=no,
      )

      if test "$gtk3" = auto; then
        PKG_CHECK_MODULES(GTK, [gtk+-3.0],
          gtk3=yes,
          AC_MSG_WARN([GTK+ not found. Reverting to Xlib])
          gtk3=no
          gtk=no
        )
      fi
    fi

  fi

  if test "$gtk" = yes; then
    AC_DEFINE([UI_GTK], 1, [Defined if GTK+ UI is in use])
    AC_DEFINE([HAVE_LIB_GLIB], 1, [Defined if we've got glib])
    UI=gtk; UI_LIBS="ui/gtk/libuigtk.la"
  fi
fi

AM_CONDITIONAL(HAVE_GTK2, test "$gtk3" != yes)

if test "$UI" = gtk; then
  AC_PATH_XTRA
fi

dnl Look for Xlib UI (default=yes)
if test -z "$UI"; then
  if test "$with_x" = no; then
    AC_MSG_ERROR([Please specify --with-svgalib or --with-fb, not --without-x.])
  fi
  AC_PATH_XTRA
  CFLAGS="$CFLAGS $X_CFLAGS";
  LDFLAGS="$LDFLAGS $X_LIBS";
  LIBS="$LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS";

  dnl This apparently needed for SVR4 systems to link correctly
  dnl Basically the same as a part of AC_PATH_XTRA, but using `gethostname'
  dnl instead of `gethostbyname'.
  dnl Thanks to Erik Kunze for pointing out this problem
  AC_CHECK_FUNC(gethostname)
  if test $ac_cv_func_gethostname = no; then
    AC_CHECK_LIB(nsl, gethostname, X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl")
  fi
  AC_CHECK_FUNC(connect)
  if test $ac_cv_func_connect = no; then
    AC_CHECK_LIB(socket, connect, X_EXTRA_LIBS="-lsocket $X_EXTRA_LIBS", ,
	         $X_EXTRA_LIBS)
  fi

  dnl Check for the MIT-SHM extension
  AC_CHECK_HEADERS( X11/extensions/XShm.h,,,[[
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <X11/Xlib.h>
	]])
  AC_CHECK_LIB( Xext, XShmAttach, LIBS="$LIBS -lXext" )

  AC_DEFINE([UI_X], 1, [Defined if Xlib UI in use])
  UI=xlib; WIDGET=widget;
  AC_DEFINE([USE_WIDGET], 1, [Defined if we're using a widget-based UI])
  UI_LIBS="ui/xlib/libuixlib.a ui/widget/libwidget.a"
fi

AC_SUBST(UI)
AC_SUBST(UI_LIBS)
AC_SUBST(WIDGET)
AM_CONDITIONAL(WIDGET, test x$WIDGET != x)

# If we're not using GTK+, must either find glib or use the replacement
# (as supplied by libspectrum)
if test "$UI" != gtk; then
  AC_MSG_CHECKING(whether to use glib)
  AC_ARG_WITH(glib,
  [  --without-glib          don't use glib],
  if test "$withval" = no; then glib=no; else glib=yes; fi,
  glib=yes)
  AC_MSG_RESULT($glib)
  if test "$glib" = yes; then
    AM_PATH_GLIB_2_0(
      2.20.0,
      AC_DEFINE([HAVE_LIB_GLIB], 1, [Defined if we've got glib])
      LIBS="$LIBS `pkg-config --libs glib-2.0`",
      AC_MSG_WARN(glib not found - using libspectrum's replacement)
    )
  fi
fi

dnl Check that libspectrum is available
AC_ARG_WITH(libspectrum-prefix,
[  --with-libspectrum-prefix=PFX where libspectrum is installed (optional)],
LIBSPEC_CFLAGS=-I$withval/include; LIBSPEC_LIBS=-L$withval/lib,
LIBSPEC_CFLAGS=""; LIBSPEC_LIBS="")
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LIBSPEC_CFLAGS $GTK_CFLAGS $GLIB_CFLAGS"
AC_CHECK_HEADERS(libspectrum.h,LIBSPEC_LIBS="$LIBSPEC_LIBS -lspectrum",
	         AC_MSG_ERROR(Sorry - I need libspectrum to be available))
CPPFLAGS="$ac_save_CPPFLAGS"
AC_SUBST(LIBSPEC_CFLAGS)
AC_SUBST(LIBSPEC_LIBS)

dnl Check if supplied libspectrum has support for compressed szx files
AC_MSG_CHECKING(whether libspectrum supports compressed szx snaps)
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LIBSPEC_CFLAGS"
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <libspectrum.h>]],
    [[
      int test = LIBSPECTRUM_SUPPORTS_ZLIB_COMPRESSION;
    ]])
  ],
  [AUTOLOAD_SNAPS=compressed; LIBSPEC_HAVE_ZLIB=yes; AC_MSG_RESULT(yes)],
  [AUTOLOAD_SNAPS=uncompressed; LIBSPEC_HAVE_ZLIB=""; AC_MSG_RESULT(no)]
)
CPPFLAGS="$ac_save_CPPFLAGS"
AC_SUBST(AUTOLOAD_SNAPS)
AC_SUBST(LIBSPEC_HAVE_ZLIB)
AC_CONFIG_LINKS([
	lib/disk_plus3.szx:lib/"$AUTOLOAD_SNAPS"/disk_plus3.szx
	lib/tape_128.szx:lib/"$AUTOLOAD_SNAPS"/tape_128.szx
	lib/tape_16.szx:lib/"$AUTOLOAD_SNAPS"/tape_16.szx
	lib/tape_2048.szx:lib/"$AUTOLOAD_SNAPS"/tape_2048.szx
	lib/tape_2068.szx:lib/"$AUTOLOAD_SNAPS"/tape_2068.szx
	lib/tape_48.szx:lib/"$AUTOLOAD_SNAPS"/tape_48.szx
	lib/tape_plus2.szx:lib/"$AUTOLOAD_SNAPS"/tape_plus2.szx
	lib/tape_plus2a.szx:lib/"$AUTOLOAD_SNAPS"/tape_plus2a.szx
	lib/tape_plus3.szx:lib/"$AUTOLOAD_SNAPS"/tape_plus3.szx
	lib/tape_plus3e.szx:lib/"$AUTOLOAD_SNAPS"/tape_plus3e.szx
	lib/tape_pentagon.szx:lib/"$AUTOLOAD_SNAPS"/tape_pentagon.szx
	lib/tape_scorpion.szx:lib/"$AUTOLOAD_SNAPS"/tape_scorpion.szx
	lib/tape_se.szx:lib/"$AUTOLOAD_SNAPS"/tape_se.szx
	lib/tape_ts2068.szx:lib/"$AUTOLOAD_SNAPS"/tape_ts2068.szx])

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

dnl Check if supplied libspectrum has support for bz2 compressed files
AC_MSG_CHECKING(whether libspectrum supports compressed bz2 files)
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LIBSPEC_CFLAGS"
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <libspectrum.h>]],
    [[
      int test = LIBSPECTRUM_SUPPORTS_BZ2_COMPRESSION;
    ]])
  ],
  [LIBSPEC_HAVE_BZ2=yes; AC_MSG_RESULT(yes)],
  [LIBSPEC_HAVE_BZ2=""; AC_MSG_RESULT(no)]
)
CPPFLAGS="$ac_save_CPPFLAGS"
AC_SUBST(LIBSPEC_HAVE_BZ2)

dnl Check if supplied libspectrum has support for wav files
AC_MSG_CHECKING(whether libspectrum supports wav files)
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LIBSPEC_CFLAGS"
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <libspectrum.h>]],
    [[
      int test = LIBSPECTRUM_SUPPORTS_AUDIOFILE;
    ]])
  ],
  [LIBSPEC_HAVE_AUDIOFILE=yes; AC_MSG_RESULT(yes)],
  [LIBSPEC_HAVE_AUDIOFILE=""; AC_MSG_RESULT(no)]
)
CPPFLAGS="$ac_save_CPPFLAGS"
AC_SUBST(LIBSPEC_HAVE_AUDIOFILE)

dnl Check if a version of libpng which supplies png_write_png is available
AC_MSG_CHECKING(whether PNG support requested)
AC_ARG_WITH(png,
[  --without-png           don't use libpng],
if test "$withval" = no; then libpng=no; else libpng=yes; fi,
libpng=yes)
AC_MSG_RESULT($libpng)
if test "$libpng" = yes; then
  AC_PATH_PROG([LIBPNG_CONFIG], [libpng-config])
  if test -n "$LIBPNG_CONFIG"; then
    PNG_CFLAGS=`libpng-config --cflags`
    PNG_LIBS=`libpng-config --ldflags`
    AC_DEFINE([USE_LIBPNG], 1, [Defined if we're going to be using the installed libpng])
  else
    ac_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PNG_CFLAGS"
    ac_save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$PNG_LIBS $LDFLAGS"

    AC_CHECK_LIB( png, png_write_png,
      [AC_CHECK_HEADER(
        png.h,
        [AC_DEFINE([USE_LIBPNG], 1, [Defined if we're going to be using the installed libpng]) PNG_LIBS='-lpng -lm -lz'],
        [AC_CHECK_HEADERS(libpng/png.h,
                     [CPPFLAGS="$ac_save_CPPFLAGS $PNG_CFLAGS/libpng"
                      AC_DEFINE([USE_LIBPNG], 1, [Defined if we're going to be using the installed libpng]) PNG_LIBS='-lpng -lm -lz'],
                     [AC_MSG_WARN(png.h not found - saving screenshots disabled)
                      PNG_LIBS='']
        )],
      )],
      [AC_MSG_WARN(png_write_png not found - saving screenshots disabled)
       PNG_LIBS='']
    )

    CPPFLAGS="$ac_save_CPPFLAGS"
    LDFLAGS="$ac_save_LDFLAGS"
  fi

  AC_SUBST(PNG_LIBS)
fi

dnl Check if a version of libasound which supplies pcm is available
AC_MSG_CHECKING(whether ALSA requested)
AC_ARG_WITH(alsa,
[  --without-alsa          don't use libasound (ALSA)],
if test "$withval" = no; then libasound=no; else libasound=yes; fi,
libasound=yes)
AC_MSG_RESULT($libasound)
if test "$libasound" = yes; then
  AC_CHECK_LIB( asound, snd_pcm_hw_params_set_period_size_near,
    [AC_CHECK_HEADER(
      alsa/asoundlib.h,
      [alsa_available=yes],
      [AC_MSG_WARN(alsa/asoundlib.h not found - no ALSA output)]
    )],
    [AC_MSG_WARN(snd_pcm_hw_params_set_period_size_near not found - no ALSA output)]
  )
fi

dnl Check if a version of libao which supplies ao_open_live is available
AC_MSG_CHECKING(whether libao requested)
AC_ARG_WITH(libao,
[  --with-libao            use libao for sound output],
if test "$withval" = no; then libao=no; else libao=yes; fi,
libao=no)
AC_MSG_RESULT($libao)
if test "$libao" = yes; then
  AC_CHECK_LIB( ao, ao_open_live,
    [AC_CHECK_HEADER(
      ao/ao.h,
      [ao_available=yes],
      [AC_MSG_WARN(ao/ao.h not found - no libao sound output)]
    )],
    [AC_MSG_WARN(ao_open_live not found - no libao sound output)]
  )
fi

dnl Check if DirectSound is available
AC_CHECK_HEADER(
  dsound.h,
  [dxsound_available=yes],
  [AC_MSG_WARN(dsound.h not found - no DirectSound output)],
  [#define DIRECTSOUND_VERSION 0x0700
   #include <windows.h>
   #include <mmsystem.h>]
)

dnl Check if MS Windows Multimedia API sound is available
AC_CHECK_HEADER(
  mmsystem.h,
  [win32sound_available=yes],
  [AC_MSG_WARN(mmsystem.h not found - no MS Windows Multimedia API sound output)],
  [#include <windows.h>]
)

dnl Check if CoreAudio is available
AC_CHECK_HEADER(
  CoreAudio/AudioHardware.h,
  [coreaudio_available=yes],
  [AC_MSG_WARN(CoreAudio/AudioHardware.h not found - no CoreAudio sound output)]
)

dnl
dnl Decide which sound routines to use
dnl

AC_MSG_CHECKING(which sound routines to use)
if test "$UI" = sdl; then
  SOUND_LIBADD='sdlsound.$(OBJEXT)' SOUND_LIBS='' sound_fifo=yes
  AC_MSG_RESULT(SDL)
elif test "$dxsound_available" = yes; then
  SOUND_LIBADD='dxsound.$(OBJEXT)' SOUND_LIBS='-ldsound -lole32 -ldxguid'
  AC_MSG_RESULT(DirectX)
  AC_DEFINE([DIRECTSOUND_VERSION], 0x0700, [DirectX 7 or higher is required])
elif test "$win32sound_available" = yes; then
  SOUND_LIBADD='win32sound.$(OBJEXT)' SOUND_LIBS='-lwinmm'
  AC_MSG_RESULT(win32sound)
elif test "$alsa_available" = yes; then
  SOUND_LIBADD='alsasound.$(OBJEXT)' SOUND_LIBS='-lasound'
  AC_MSG_RESULT(ALSA)
elif test "$ao_available" = yes; then
  SOUND_LIBADD='aosound.$(OBJEXT)' SOUND_LIBS='-lao'
  AC_MSG_RESULT(libao)
elif test "$ac_cv_header_dsound_h" = yes; then
  # Later selection between these two
  SOUND_LIBADD='sunsound.$(OBJEXT) hpsound.$(OBJEXT)' SOUND_LIBS=''
  AC_MSG_RESULT(Solaris or HP/UX)
elif test "$ac_cv_header_sys_soundcard_h" = yes; then
  SOUND_LIBADD='osssound.$(OBJEXT)' SOUND_LIBS=''
  AC_MSG_RESULT(OSS)
elif test "$ac_cv_header_sys_audioio_h" = yes; then
  dnl OpenBSD
  SOUND_LIBADD='sunsound.$(OBJEXT)' SOUND_LIBS=''
  AC_MSG_RESULT(OpenBSD)
elif test "$coreaudio_available" = yes; then
  SOUND_LIBADD='coreaudiosound.$(OBJEXT)' SOUND_LIBS='-framework CoreAudio -framework AudioUnit -framework CoreServices' sound_fifo=yes
  AC_MSG_RESULT(CoreAudio)
elif test "$wii" = yes; then
  SOUND_LIBADD='wiisound.$(OBJEXT)' SOUND_LIBS='' sound_fifo=yes
  AC_MSG_RESULT(Wii)
else
  SOUND_LIBADD='nullsound.$(OBJEXT)' SOUND_LIBS=''
  AC_MSG_RESULT(none)
  AC_DEFINE([NO_SOUND], 1, [Defined if no sound code is present])
fi

if test "$sound_fifo" = yes; then
  dnl Strange construct used here as += doesn't seem to work on OS X
  SOUND_LIBADD="$SOUND_LIBADD"' sfifo.$(OBJEXT)'
  AC_DEFINE([SOUND_FIFO], 1, [Defined if the sound code uses a fifo])
fi

AC_SUBST(SOUND_LIBADD)
AC_SUBST(SOUND_LIBS)

dnl Are we supplying the ROMs or relying on another package?
AC_MSG_CHECKING(where to find the Spectrum ROM images)
AC_ARG_WITH(roms-dir,
  [  --with-roms-dir=DIR     location of the Spectrum ROM images],
  ,
  unset withval
)
if test "$withval" = ""; then
  echo 'we supply them'
else
  echo "$withval"
  AC_DEFINE_UNQUOTED(ROMSDIR, "$withval", [Location of the ROM images])
fi

AC_MSG_CHECKING(whether hardware joystick support requested)
  AC_ARG_WITH(joystick,
  [  --without-joystick      use a real joystick for Kempston emulation etc.],
  if test "$withval" = no; then stick=no; else stick=yes; fi,
  stick=yes)
AC_MSG_RESULT($stick)
if test "$stick" = yes; then
  dnl We prefer to use UI-specific joystick code. (Default.)
  dnl If there is none, or you override this here, we use libjsw.
  dnl (svgalib appears to dislike switched joysticks on the parallel port...)
  case "$UI" in
    svga|sdl|win32)
      dnl Libraries such as svgalib provide their own joystick code.
      AC_MSG_CHECKING([whether to use $UI's joystick support])
      AC_ARG_ENABLE(ui-joystick,
      AS_HELP_STRING([--disable-ui-joystick], [use libjsw joystick code (where supported)]),
      if test "$enableval" = no; then libjsw=yes; else libjsw=no; fi,
      libjsw=no)
      if test "$libjsw" = yes; then
        AC_MSG_RESULT([no, use libjsw])
      else
        AC_MSG_RESULT(yes)
        AC_DEFINE([USE_JOYSTICK], 1, [Defined if we're using hardware joysticks])
      fi
      ;;
    gtk|xlib)
      dnl These UIs could use SDL library for handling joysticks
      AC_MSG_CHECKING([whether to use $UI's joystick support])
      AC_ARG_ENABLE(ui-joystick,
      AS_HELP_STRING([--disable-ui-joystick], [use libjsw joystick code (where supported)]),
      if test "$enableval" = no; then libjsw=yes; else libjsw=no; fi,
      libjsw=no)
      if test "$libjsw" = yes; then
        AC_MSG_RESULT([no, use libjsw])
      else
        AC_MSG_RESULT(yes)
        SDL_VERSION=1.2.4
        AM_PATH_SDL($SDL_VERSION,		    
          LIBS="$LIBS `sdl-config --libs`"
          CPPFLAGS="$CPPFLAGS `sdl-config --cflags`"
          AC_DEFINE([USE_JOYSTICK], 1, [Defined if we're using hardware joysticks]),
          AC_MSG_WARN([SDL version $SDL_VERSION not found - joystick support disabled]))
      fi
      ;;
    wii)
      dnl Wii UI check this elsewhere
      AC_MSG_RESULT(yes)
      libjsw=no
      ;;
    *)
      dnl Our only option is libjsw.
      libjsw=yes
      ;;
  esac
  if test "$libjsw" = yes; then
    dnl Look for libjsw. If missing, use QAOP<space>.
    AC_CHECK_HEADERS(jsw.h,
                     LIBS="$LIBS -ljsw";
		     AC_DEFINE([USE_JOYSTICK], 1,
		               [Defined if we're using hardware joysticks]),
                     AC_MSG_WARN(jsw.h not found - joystick support disabled))
  fi
fi

dnl See if libxml2 is available (for the config file handling)
AC_MSG_CHECKING(whether libxml2 support requested)
  AC_ARG_WITH(libxml2,
  [  --without-libxml2       use a libxml2 for settings],
  if test "$withval" = no; then libxml2=no; else libxml2=yes; fi,
  libxml2=yes)
AC_MSG_RESULT($libxml2)
if test "$libxml2" = yes; then
  AM_PATH_XML2(2.0.0,AC_DEFINE([HAVE_LIB_XML2], 1,
               [Defined if we've got libxml2]),
               AC_MSG_WARN(libxml2 not found - config file will use ini format))
fi

dnl Work out which timer routines to use
AC_MSG_CHECKING(which timer routines to use)
if test "$UI" = sdl; then
  TIMER_LIBADD='sdl.$(OBJEXT)'
  AC_MSG_RESULT(SDL)
else
  TIMER_LIBADD='native.$(OBJEXT)'
  AC_MSG_RESULT(native)
fi
AC_SUBST(TIMER_LIBADD)

dnl See if sockets are supported
AC_MSG_CHECKING([whether sockets support requested])
AC_ARG_ENABLE(sockets,
AS_HELP_STRING([--disable-sockets], [do not use sockets]),
if test "$enableval" = yes; then sockets=yes; else sockets=no; fi,
sockets=yes)
AC_MSG_RESULT($sockets)
if test "$sockets" = yes; then
case "$host_os" in
  mingw32*)
    AC_CHECK_HEADER([winsock2.h], [LIBS="$LIBS -lws2_32"], [sockets=no])
    ;;
  *)
    AC_CHECK_HEADER([sys/socket.h],, [sockets=no])
    ;;
esac
fi
AM_CONDITIONAL(HAVE_SOCKETS, test "$sockets" = yes)

dnl See if POSIX threads are supported
AC_MSG_CHECKING([whether pthread support requested])
AC_ARG_WITH(pthread,
AS_HELP_STRING([--without-pthread],[do not use POSIX threads]),
if test "$withval" = no; then pthread=no; else pthread=yes; fi,
pthread=yes)
AC_MSG_RESULT($pthread)
if test "$pthread" = yes; then
  AX_PTHREAD([LIBS="$PTHREAD_LIBS $LIBS"
              CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
              CC="$PTHREAD_CC"],
             [AC_MSG_WARN(POSIX threads not found - some peripherals disabled)
              pthread=no])
fi
if test "$pthread" = yes -a "$sockets" = yes; then
  build_spectranet=yes
  AC_DEFINE([BUILD_SPECTRANET], 1, [Defined if we support spectranet])
else
  build_spectranet=no
fi
AM_CONDITIONAL(BUILD_SPECTRANET, test "$build_spectranet" = yes)

dnl See if Linux TAP devices are supported
AC_MSG_CHECKING(whether Linux TAP devices are supported)
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LIBSPEC_CFLAGS"
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[
      #include <linux/if_tun.h>
      #include <net/if.h>
      #include <sys/ioctl.h>
    ]],[[
      int test1 = IFF_TAP | IFF_NO_PI;
      int test2 = TUNSETIFF;
    ]])
  ],
  [AC_DEFINE([BUILD_SPECCYBOOT], 1, [Define to 1 if SpeccyBoot is supported.])
  linux_tap=yes
  AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
CPPFLAGS="$ac_save_CPPFLAGS"
AM_CONDITIONAL(HAVE_TUNTAP, test x$linux_tap != x)
AM_CONDITIONAL(BUILD_SPECCYBOOT, test x$linux_tap != x)

dnl Work out which standard routines we're missing
AC_MSG_CHECKING(which standard routines we're missing)
missing_routines=''
if test "$ac_cv_func_dirname" = no; then
  missing_routines="$missing_routines"'dirname '
fi
AM_CONDITIONAL(COMPAT_DIRNAME, test "$ac_cv_func_dirname" = no)
if test "$ac_cv_func_getopt_long" = no; then
  case "$host_os" in
    amigaos|morphos)
      dnl Our getopt_long replacement doesn't work, so don't use it
      ;;
    *)
      compat_getopt=yes
      missing_routines="$missing_routines"'getopt_long '
      ;;
  esac
fi
AM_CONDITIONAL(COMPAT_GETOPT, test "$compat_getopt" = yes)
if test "$ac_cv_func_mkstemp" = no; then
  missing_routines="$missing_routines"'mkstemp '
fi
AM_CONDITIONAL(COMPAT_MKSTEMP, test "$ac_cv_func_mkstemp" = no)
if test "x$missing_routines" = x; then
  missing_routines='(none)'
fi
AC_MSG_RESULT($missing_routines)

dnl Work out which compatibility routines to use
AC_MSG_CHECKING(which compatibility routines to use)
case "$host_os" in
  amigaos)
    COMPAT_OSNAME='amiga'
    ;;
  linux*)
    COMPAT_OSNAME='linux'
    ;;
  morphos)
    COMPAT_OSNAME='morphos'
    ;;
  mingw32*)
    COMPAT_OSNAME='win32'
    ;;
  wii)
    COMPAT_OSNAME='wii'
    ;;
  *)
    COMPAT_OSNAME='unix'
    ;;
esac
AC_MSG_RESULT($COMPAT_OSNAME)
COMPAT_SUBDIR="compat/$COMPAT_OSNAME"
AC_SUBST(COMPAT_SUBDIR)
AC_SUBST(COMPAT_OSNAME)
AM_CONDITIONAL(COMPAT_WIN32, test "$COMPAT_OSNAME" = 'win32')

dnl Decide whether to install desktop and mime files
AC_ARG_ENABLE(desktop-integration,
AS_HELP_STRING([--enable-desktop-integration], [add menu entry and file associations]),
if test "$enableval" = yes; then desktopintegration=yes; else desktopintegration=no; fi,
desktdesktopintegration=no)
AC_MSG_CHECKING(whether to install desktop file)
AC_MSG_RESULT($desktopintegration)
AM_CONDITIONAL(DESKTOP_INTEGRATION, test "$desktopintegration" = yes)
AC_CHECK_PROGS(GTK_UPDATE_ICON_CACHE,gtk-update-icon-cache,true)

dnl Where to install the desktop and mime files
AC_ARG_WITH(desktop-dir,
AS_HELP_STRING([--with-desktop-dir=DIR], [location of desktop files (default=datadir)]),
if test "$withval" = no; then desktopdir=no; else desktopdir=yes; fi,
desktopdir=no)
if test "$desktopdir" = yes; then
  DESKTOP_DATADIR=$with_desktop_dir
else
  DESKTOP_DATADIR="$datadir"
fi
AM_CONDITIONAL(DESKTOP_DATADIR, test "$desktopdir" == yes)
AC_SUBST(DESKTOP_DATADIR)

dnl Do we want the low memory compile?
AC_MSG_CHECKING(whether low memory compile requested)
AC_ARG_ENABLE(smallmem,
[  --enable-smallmem       low memory compile needed],
if test "$enableval" = yes; then
    smallmem=yes;
else
    smallmem=no;
fi,
smallmem=no)
if test "$smallmem" = no; then
    AC_DEFINE([HAVE_ENOUGH_MEMORY], 1, [Defined if we've got enough memory to compile z80_ops.c])
fi
AC_MSG_RESULT($smallmem)

dnl Do we want lots of warning messages?
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

dnl If it appears we're using gcc as our compiler, turn on warnings
if test "$ac_cv_prog_gcc" = yes; then
    CFLAGS="$CFLAGS -Wall"
    dnl And possibly lots of warnings
    if test "$warnings" = yes; then
        CFLAGS="$CFLAGS -Wstrict-prototypes -Wmissing-prototypes -Winline -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Waggregate-return -W -Wsign-compare"
    fi
fi

AC_CONFIG_FILES([
Makefile
compat/amiga/Makefile
compat/linux/Makefile
compat/morphos/Makefile
compat/unix/Makefile
compat/wii/Makefile
compat/win32/Makefile
data/Makefile
debugger/Makefile
hacking/Makefile
lib/Makefile
machines/Makefile
man/Makefile
peripherals/Makefile
peripherals/disk/Makefile
peripherals/ide/Makefile
peripherals/nic/Makefile
peripherals/flash/Makefile
perl/Makefile
perl/Fuse/Makefile
pokefinder/Makefile
roms/Makefile
sound/Makefile
timer/Makefile
ui/Makefile
ui/fb/Makefile
ui/wii/Makefile
ui/gtk/Makefile
ui/svga/Makefile
ui/sdl/Makefile
ui/scaler/Makefile
ui/widget/Makefile
ui/win32/fuse.manifest
ui/win32/Makefile
ui/win32/icons/Makefile
ui/win32/installer/fuse.nsi
ui/xlib/Makefile
unittests/Makefile
z80/Makefile
z80/tests/Makefile])

AC_OUTPUT
