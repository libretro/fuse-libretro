## Process this file with automake to produce Makefile.in
## Copyright (c) 2010-2011 Philip Kendall

## $Id: Makefile.am 4898 2013-02-25 22:20:54Z pak21 $

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

noinst_LIBRARIES = libnic.a

AM_CPPFLAGS += @LIBSPEC_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@

libnic_a_SOURCES =

if BUILD_SPECCYBOOT
libnic_a_SOURCES += enc28j60.c
endif

if BUILD_SPECTRANET
libnic_a_SOURCES += w5100.c \
  w5100_socket.c
endif

noinst_HEADERS = \
  enc28j60.h \
  w5100.h \
  w5100_internals.h
