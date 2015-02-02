## Process this file with automake to produce Makefile.in
## Copyright (c) 2004 Philip Kendall

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

AM_CPPFLAGS = -I$(srcdir)/..

AM_CPPFLAGS += @LIBSPEC_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@

noinst_LIBRARIES = libmachines.a

libmachines_a_SOURCES = machines_periph.c \
                        pentagon.c \
			pentagon512.c \
			pentagon1024.c \
			scorpion.c \
			spec128.c \
			spec16.c \
			spec48.c \
			spec48_ntsc.c \
			specplus2.c \
			spec_se.c \
			specplus2a.c \
			specplus3.c \
			specplus3e.c \
			tc2048.c \
			tc2068.c \
			ts2068.c

noinst_HEADERS = machines.h \
		 machines_periph.h \
		 pentagon.h \
		 scorpion.h \
		 spec128.h \
		 spec48.h \
		 specplus3.h \
		 tc2068.h
