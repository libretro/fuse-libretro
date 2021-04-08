## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2004 Philip Kendall

## $Id: Makefile.am 4756 2012-11-07 11:11:44Z fredm $

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

noinst_LIBRARIES = libuixlib.a

libuixlib_a_SOURCES = xdisplay.c \
		      xerror.c \
		      xjoystick.c \
		      xkeyboard.c \
		      xui.c \
		      keysyms.c

BUILT_SOURCES = keysyms.c xpixmaps.c

keysyms.c: $(srcdir)/../../keysyms.pl $(srcdir)/../../keysyms.dat
	@PERL@ -I$(srcdir)/../../perl $(srcdir)/../../keysyms.pl x $(srcdir)/../../keysyms.dat > $@.tmp && mv $@.tmp $@

xpixmaps.c: xstatusbar.pl $(srcdir)/../gtk/pixmaps.c
	@PERL@ -I$(srcdir)/../../perl $(srcdir)/xstatusbar.pl $(srcdir)/../gtk/pixmaps.c > $@.tmp && mv $@.tmp $@

AM_CPPFLAGS += @GTK_CFLAGS@ @GLIB_CFLAGS@ @LIBSPEC_CFLAGS@

noinst_HEADERS = xdisplay.h xkeyboard.h xui.h

EXTRA_DIST = xstatusbar.pl

CLEANFILES = keysyms.c xpixmaps.c
