## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2004 Philip Kendall

## $Id: Makefile.am 3944 2009-01-10 18:17:04Z pak21 $

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

SUBDIRS = @UI@ @WIDGET@ scaler

DIST_SUBDIRS = fb \
	       gtk \
	       scaler \
	       sdl \
	       svga \
	       widget \
	       wii \
	       win32 \
	       xlib

noinst_HEADERS = ui.h \
		 uidisplay.h \
		 uijoystick.h

EXTRA_DIST = options.dat \
	     uijoystick.c
