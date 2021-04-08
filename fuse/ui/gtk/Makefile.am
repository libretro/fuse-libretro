## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2004 Philip Kendall

## $Id: Makefile.am 4743 2012-10-10 13:20:50Z fredm $

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

AM_CPPFLAGS = -I$(srcdir)/../..

noinst_LTLIBRARIES = libuigtk.la

AM_CPPFLAGS += @GTK_CFLAGS@ @LIBSPEC_CFLAGS@

libuigtk_la_SOURCES = binary.c \
		     browse.c \
		     confirm.c \
	             debugger.c \
		     fileselector.c \
		     gtkdisplay.c \
		     gtkjoystick.c \
		     gtkkeyboard.c \
		     gtkmouse.c \
                     gtkui.c \
		     keysyms.c \
		     memory.c \
		     menu_data.c \
	             options.c \
		     picture.c \
		     pixmaps.c \
		     pokefinder.c \
		     pokemem.c \
		     rollback.c \
		     roms.c \
		     statusbar.c \
		     stock.c

if HAVE_GTK2
libuigtk_la_SOURCES += gtkcompat.c
endif

libuigtk_la_LIBADD = @GTK_LIBS@ -lX11 

BUILT_SOURCES = keysyms.c \
		menu_data.c \
		menu_data.ui \
		options.c \
		options_internals.h

pkgdata_DATA = menu_data.ui

keysyms.c: $(srcdir)/../../keysyms.pl $(srcdir)/../../keysyms.dat
	@PERL@ -I$(srcdir)/../../perl $(srcdir)/../../keysyms.pl gtk $(srcdir)/../../keysyms.dat > $@.tmp && mv $@.tmp $@

menu_data.c: $(srcdir)/../../perl/cpp-perl.pl $(srcdir)/../../menu_data.pl $(srcdir)/../../menu_data.dat ../../config.h
	@PERL@ $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/../../menu_data.dat | @PERL@ -I$(srcdir)/../../perl $(srcdir)/../../menu_data.pl gtk c > $@.tmp && mv $@.tmp $@

menu_data.ui: $(srcdir)/../../perl/cpp-perl.pl $(srcdir)/../../menu_data.pl $(srcdir)/../../menu_data.dat ../../config.h
	@PERL@ $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/../../menu_data.dat | @PERL@ -I$(srcdir)/../../perl $(srcdir)/../../menu_data.pl gtk ui > $@.tmp && mv $@.tmp $@

options.c: $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/options.pl $(srcdir)/../../ui/options.dat $(srcdir)/../../perl/Fuse.pm $(srcdir)/../../perl/Fuse/Dialog.pm
	@PERL@ $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/../../ui/options.dat | @PERL@ -I$(srcdir)/../../perl $(srcdir)/options.pl - > $@.tmp && mv $@.tmp $@

options_internals.h: $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/options-header.pl $(srcdir)/../../ui/options.dat $(srcdir)/../../perl/Fuse.pm $(srcdir)/../../perl/Fuse/Dialog.pm
	@PERL@ $(srcdir)/../../perl/cpp-perl.pl ../../config.h $(srcdir)/../../ui/options.dat | @PERL@ -I$(srcdir)/../../perl $(srcdir)/options-header.pl - > $@.tmp && mv $@.tmp $@

noinst_HEADERS = gtkcompat.h \
		 gtkinternals.h \
		 options_internals.h \
		 options.pl \
		 options-header.pl

CLEANFILES = keysyms.c \
	     menu_data.c \
	     menu_data.ui \
	     options.c \
	     options_internals.h
