## Process this file with automake to produce Makefile.in
## Copyright (c) 2001 Philip Kendall

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

noinst_LIBRARIES = libscaler.a

AM_CPPFLAGS += @GTK_CFLAGS@ @GLIB_CFLAGS@ @LIBSPEC_CFLAGS@

libscaler_a_SOURCES = scaler.c
libscaler_a_LIBADD = scalers16.o scalers32.o

scalers16.o: $(srcdir)/scalers.c
	$(COMPILE) -DSCALER_DATA_SIZE=2 -c $(srcdir)/scalers.c -o scalers16.o

scalers32.o: scalers.c
	$(COMPILE) -DSCALER_DATA_SIZE=4 -c $(srcdir)/scalers.c -o scalers32.o

noinst_HEADERS = scaler.h scaler_internals.h

EXTRA_DIST = scalers.c scaler_hq2x.c scaler_hq3x.c
