## Process this file with automake to produce Makefile.in
## Copyright (c) 2010 Philip Kendall

## $Id: Makefile.am 4775 2012-11-26 23:03:36Z sbaldovi $

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

noinst_LIBRARIES = libflash.a

AM_CPPFLAGS += @LIBSPEC_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@

libflash_a_SOURCES = \
  am29f010.c

noinst_HEADERS = \
  am29f010.h
