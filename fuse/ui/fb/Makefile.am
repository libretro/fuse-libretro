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

noinst_LIBRARIES = libuifb.a

AM_CPPFLAGS += @GLIB_CFLAGS@ @LIBSPEC_CFLAGS@

libuifb_a_SOURCES = fbdisplay.c \
		    fbjoystick.c \
	 	    fbkeyboard.c \
	 	    fbmouse.c \
		    fbui.c \
		    keysyms.c

BUILT_SOURCES = keysyms.c

keysyms.c: $(srcdir)/../../keysyms.pl $(srcdir)/../../keysyms.dat
	@PERL@ -I$(srcdir)/../../perl $(srcdir)/../../keysyms.pl fb $(srcdir)/../../keysyms.dat > $@.tmp && mv $@.tmp $@

noinst_HEADERS = fbdisplay.h \
	         fbkeyboard.h \
	         fbmouse.h

CLEANFILES = keysyms.c
