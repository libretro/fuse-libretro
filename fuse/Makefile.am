## Process this file with automake to produce Makefile.in
## Copyright (c) 1999-2009 Philip Kendall

## $Id: Makefile.am 5003 2013-05-26 16:24:03Z sbaldovi $

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
## Author contact information:
##
## E-mail: philip-fuse@shadowmagic.org.uk

AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = fuse

SUBDIRS = @COMPAT_SUBDIR@ \
	  data \
	  debugger \
	  hacking \
	  lib \
	  machines \
	  man \
          peripherals \
	  perl \
	  pokefinder \
	  roms \
	  sound \
	  timer \
	  ui \
	  unittests \
	  z80

DIST_SUBDIRS = data \
	       debugger \
	       hacking \
	       lib \
	       machines \
	       man \
               peripherals \
	       perl \
	       pokefinder \
	       roms \
	       sound \
	       timer \
	       ui \
	       unittests \
	       z80

fuse_SOURCES = display.c \
	event.c \
	fuse.c \
	input.c \
	keyboard.c \
	loader.c \
	machine.c \
	memory.c \
	mempool.c \
	menu.c \
	movie.c \
	module.c \
	periph.c \
	profile.c \
	psg.c \
	rectangle.c \
	rzx.c \
	screenshot.c \
	settings.c \
	slt.c \
	snapshot.c \
	sound.c \
	spectrum.c \
	tape.c \
	ui.c \
	uidisplay.c \
	utils.c

if COMPAT_DIRNAME
fuse_SOURCES += compat/dirname.c
endif

if COMPAT_GETOPT
fuse_SOURCES += compat/getopt.c compat/getopt1.c
endif

if COMPAT_MKSTEMP
fuse_SOURCES += compat/mkstemp.c
endif

if COMPAT_WIN32
fuse_SOURCES += windres.rc
endif

EXTRA_fuse_SOURCES = windres.rc

fuse_LDADD = debugger/libdebugger.a \
@UI_LIBS@ \
debugger/libdebugger.a \
machines/libmachines.a \
peripherals/libperipherals.a \
peripherals/disk/libdisk.a \
peripherals/flash/libflash.a \
peripherals/ide/libide.a \
peripherals/nic/libnic.a \
pokefinder/libpokefinder.a \
sound/libsound.a \
timer/libtimer.a \
ui/scaler/libscaler.a \
unittests/libunittests.a \
z80/libz80.a \
@LIBSPEC_LIBS@ \
@GLIB_LIBS@ \
@PNG_LIBS@ \
@SOUND_LIBS@ \
@SDL_LIBS@ \
@X_LIBS@ \
@XML_LIBS@ \
compat/@COMPAT_OSNAME@/libcompatos.a \
@WINDRES_OBJ@

fuse_DEPENDENCIES = debugger/libdebugger.a \
@UI_LIBS@ \
debugger/libdebugger.a \
machines/libmachines.a \
peripherals/libperipherals.a \
peripherals/disk/libdisk.a \
peripherals/flash/libflash.a \
peripherals/ide/libide.a \
peripherals/nic/libnic.a \
pokefinder/libpokefinder.a \
sound/libsound.a \
timer/libtimer.a \
ui/scaler/libscaler.a \
unittests/libunittests.a \
z80/libz80.a \
compat/@COMPAT_OSNAME@/libcompatos.a \
@WINDRES_OBJ@

BUILT_SOURCES = options.h settings.c settings.h

windres.o: windres.rc winfuse.ico ui/win32/*.rc
	@WINDRES@ -I$(srcdir) -I. $(srcdir)/windres.rc @LIBSPEC_CFLAGS@ @CPPFLAGS@ windres.o

settings.c: settings.pl settings.dat
	@PERL@ -I$(srcdir)/perl $(srcdir)/settings.pl $(srcdir)/settings.dat > $@.tmp && mv $@.tmp $@

settings.h: settings-header.pl settings.dat
	@PERL@ -I$(srcdir)/perl $(srcdir)/settings-header.pl $(srcdir)/settings.dat > $@.tmp && mv $@.tmp $@

if WIDGET
options.h: $(srcdir)/perl/cpp-perl.pl config.h $(srcdir)/ui/@WIDGET@/options-header.pl $(srcdir)/ui/options.dat $(srcdir)/perl/Fuse.pm $(srcdir)/perl/Fuse/Dialog.pm
	@PERL@ $(srcdir)/perl/cpp-perl.pl config.h $(srcdir)/ui/options.dat | @PERL@ -I$(srcdir)/perl $(srcdir)/ui/@WIDGET@/options-header.pl - public > $@.tmp && mv $@.tmp $@
else
options.h: $(srcdir)/perl/cpp-perl.pl config.h $(srcdir)/ui/@UI@/options-header.pl $(srcdir)/ui/options.dat $(srcdir)/perl/Fuse.pm $(srcdir)/perl/Fuse/Dialog.pm
	@PERL@ $(srcdir)/perl/cpp-perl.pl config.h $(srcdir)/ui/options.dat | @PERL@ -I$(srcdir)/perl $(srcdir)/ui/@UI@/options-header.pl - public > $@.tmp && mv $@.tmp $@
endif

AM_CPPFLAGS = @GLIB_CFLAGS@ @GTK_CFLAGS@ @LIBSPEC_CFLAGS@ \
@XML_CFLAGS@ -DFUSEDATADIR="\"${pkgdatadir}\"" @SDL_CFLAGS@

noinst_HEADERS = bitmap.h \
	compat.h \
	display.h \
	event.h \
	fuse.h \
	input.h \
	keyboard.h \
	loader.h \
	machine.h \
	memory.h \
	mempool.h \
	menu.h \
	movie.h \
	movie_tables.h \
	module.h \
	periph.h \
	psg.h \
	rectangle.h \
	rzx.h \
	screenshot.h \
	settings.h \
	slt.h \
	snapshot.h \
	sound.h \
	spectrum.h \
	tape.h \
	utils.h \
	options.h \
	profile.h

EXTRA_DIST = AUTHORS \
	     INSTALL \
	     PORTING \
	     README \
	     THANKS \
	     keysyms.dat \
	     keysyms.pl \
	     m4/glib-2.0.m4 \
	     m4/gtk-2.0.m4 \
	     m4/libxml.m4 \
	     m4/pkg.m4 \
	     m4/sdl.m4 \
	     menu_data.dat \
	     menu_data.pl \
	     settings.pl \
	     settings-header.pl \
	     settings.dat \
	     winfuse.ico

CLEANFILES = options.h \
	     settings.c \
	     settings.h

include compat/Makefile.am

if COMPAT_WIN32

package_win32=$(PACKAGE)-$(PACKAGE_VERSION)-win32
top_win32dir=$(top_builddir)/$(package_win32)

install-win32: all
	test -n "$(DESTDIR)" || { echo "ERROR: set DESTDIR path"; exit 1; }
	$(MKDIR_P) $(DESTDIR)/roms/ || exit 1
	$(MKDIR_P) $(DESTDIR)/lib/ || exit 1
	cp $(top_srcdir)/roms/*.rom $(DESTDIR)/roms
	cp $(top_srcdir)/roms/README.copyright $(DESTDIR)/roms
	cp $(top_srcdir)/lib/*.bmp $(DESTDIR)/lib
	cp $(top_srcdir)/lib/*.scr $(DESTDIR)/lib
	cp $(top_builddir)/lib/*.szx $(DESTDIR)/lib
#	Copy fuse executable (we should manually copy the required libraries)
	cp $(top_builddir)/.libs/fuse$(EXEEXT) $(DESTDIR) || \
	cp $(top_builddir)/fuse$(EXEEXT) $(DESTDIR)
#	Get text files
	for file in AUTHORS ChangeLog COPYING README; \
	  do cp "$(top_srcdir)/$$file" "$(DESTDIR)/$$file.txt"; \
	done
#	Get manuals
	if test -n "$(MAN2HTML)"; then \
	  $(MAN2HTML) -r $(top_srcdir)/man/fuse.1 | sed '1d' > $(DESTDIR)/fuse.html; \
	else \
	  test -z "$(GROFF)" || $(GROFF) -Thtml -man $(top_srcdir)/man/fuse.1 > $(DESTDIR)/fuse.html; \
	fi
#	Convert to DOS line endings
	test -z "$(UNIX2DOS)" || find $(DESTDIR) -type f \( -name "*.txt" -or -name "*.html" -or -name "*.copyright" \) -exec $(UNIX2DOS) {} \;

install-win32-strip: install-win32
	test -z "$(STRIP)" || $(STRIP) $(DESTDIR)/fuse$(EXEEXT)

dist-win32-dir:
	$(MAKE) DESTDIR="$(top_win32dir)" install-win32-strip

dist-win32-zip: dist-win32-dir
	rm -f -- $(top_builddir)/$(package_win32).zip
	rm -f -- $(top_builddir)/$(package_win32).zip.sha1
	test -n "$(top_win32dir)" || exit 1
	@test `find $(top_win32dir) -type f -name \*.dll -print | wc -l` -ne 0 || \
	{ echo "ERROR: external libraries not found in $(top_win32dir). Please, manually copy them."; exit 1; }
	cd $(top_win32dir) && \
	zip -q -9 -r $(abs_top_builddir)/$(package_win32).zip .
	-sha1sum $(top_builddir)/$(package_win32).zip > $(top_builddir)/$(package_win32).zip.sha1 && \
	{ test -z "$(UNIX2DOS)" || $(UNIX2DOS) $(top_builddir)/$(package_win32).zip.sha1; }

dist-win32-7z: dist-win32-dir
	rm -f -- $(top_builddir)/$(package_win32).7z
	rm -f -- $(top_builddir)/$(package_win32).7z.sha1
	test -n "$(top_win32dir)" || exit 1
	@test `find $(top_win32dir) -type f -name \*.dll -print | wc -l` -ne 0 || \
	{ echo "ERROR: external libraries not found in $(top_win32dir). Please, manually copy them."; exit 1; }
	cd $(top_win32dir) && \
	7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on -bd $(abs_top_builddir)/$(package_win32).7z .
	-sha1sum $(top_builddir)/$(package_win32).7z > $(top_builddir)/$(package_win32).7z.sha1 && \
	{ test -z "$(UNIX2DOS)" || $(UNIX2DOS) $(top_builddir)/$(package_win32).7z.sha1; }

dist-win32-exe: dist-win32-dir
	rm -f -- $(top_builddir)/$(package_win32)-setup.exe
	rm -f -- $(top_builddir)/$(package_win32)-setup.exe.sha1
	test -n "$(top_win32dir)" || exit 1
	@test `find $(top_win32dir) -type f -name \*.dll -print | wc -l` -ne 0 || \
	{ echo "ERROR: external libraries not found in $(top_win32dir). Please, manually copy them."; exit 1; }
#	Locate NSIS in system path, MSYS drive or Cygwin drive
	@NSISFILE="$(abs_top_builddir)/ui/win32/installer/fuse.nsi"; \
	if makensis -VERSION > /dev/null 2>&1; then \
	  MAKENSIS="makensis"; \
	elif [ -x "/c/Program Files/NSIS/makensis" ]; then \
	  MAKENSIS="/c/Program\ Files/NSIS/makensis"; \
	elif [ -x "/cygdrive/c/Program Files/NSIS/makensis" ]; then \
	  MAKENSIS="/cygdrive/c/Program\ Files/NSIS/makensis"; \
	else \
	  echo 'ERROR: cannot locate makensis tool'; exit 1; \
	fi; \
	case "`uname -s`" in \
	  CYGWIN*) NSISFILE=`cygpath -m $$NSISFILE`;; \
	esac; \
	cd $(top_win32dir); \
	eval "$$MAKENSIS -V2 -NOCD $$NSISFILE"
	mv $(top_win32dir)/$(package_win32)-setup.exe $(top_builddir)
	-sha1sum $(top_builddir)/$(package_win32)-setup.exe > $(top_builddir)/$(package_win32)-setup.exe.sha1 && \
	{ test -z "$(UNIX2DOS)" || $(UNIX2DOS) $(top_builddir)/$(package_win32)-setup.exe.sha1; }

dist-win32: dist-win32-zip dist-win32-7z dist-win32-exe

distclean-win32:
	if test -d "$(top_builddir)/$(package_win32)"; then \
	  rm -rf -- "$(top_builddir)/$(package_win32)"; \
	fi
	rm -f -- $(top_builddir)/$(package_win32).zip
	rm -f -- $(top_builddir)/$(package_win32).zip.sha1
	rm -f -- $(top_builddir)/$(package_win32).7z
	rm -f -- $(top_builddir)/$(package_win32).7z.sha1
	rm -f -- $(top_builddir)/$(package_win32)-setup.exe
	rm -f -- $(top_builddir)/$(package_win32)-setup.exe.sha1

.PHONY: install-win32 install-win32-strip dist-win32 dist-win32-dir \
	dist-win32-zip dist-win32-7z dist-win32-exe distclean-win32

endif
