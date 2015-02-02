## Process this file with automake to produce Makefile.in
## Copyright (c) 2004 Philip Kendall

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
## E-mail: philip-fuse@shadowmagic.org.uk

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I$(srcdir)/..

AM_CPPFLAGS += @LIBSPEC_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @SDL_CFLAGS@

noinst_LIBRARIES = libsound.a

libsound_a_SOURCES = blipbuffer.c

EXTRA_libsound_a_SOURCES = dxsound.c \
		     	   alsasound.c \
		     	   aosound.c \
		     	   coreaudiosound.c \
		     	   hpsound.c \
		     	   nullsound.c \
		     	   osssound.c \
		     	   sdlsound.c \
		     	   sfifo.c \
		     	   sunsound.c \
		     	   wiisound.c \
		     	   win32sound.c

libsound_a_LIBADD = $(SOUND_LIBADD)
libsound_a_DEPENDENCIES = $(SOUND_LIBADD)

noinst_HEADERS = sfifo.h blipbuffer.h
