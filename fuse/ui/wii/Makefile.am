## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2009 Philip Kendall, Bjoern Giesler

## $Id: Makefile.am 4980 2013-05-21 12:37:51Z fredm $

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
## E-mail: bjoern@giesler.de

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I$(srcdir)/../..

noinst_LIBRARIES = libuiwii.a

AM_CPPFLAGS += @GLIB_CFLAGS@ @LIBSPEC_CFLAGS@

libuiwii_a_SOURCES = wiidisplay.c \
		    wiijoystick.c \
	 	    wiikeyboard.c \
	 	    wiimouse.c \
		    wiiui.c \
		    keysyms.c

BUILT_SOURCES = keysyms.c

keysyms.c: $(srcdir)/../../keysyms.pl $(srcdir)/../../keysyms.dat
	@PERL@ -I$(srcdir)/../../perl $(srcdir)/../../keysyms.pl wii $(srcdir)/../../keysyms.dat > $@.tmp && mv $@.tmp $@

noinst_HEADERS = wiidisplay.h \
	         wiikeyboard.h \
	         wiikeysyms.h \
	         wiimouse.h

CLEANFILES = keysyms.c
