## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2008 Philip Kendall

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

AM_CPPFLAGS += @GLIB_CFLAGS@ @GTK_CFLAGS@ @LIBSPEC_CFLAGS@

noinst_LIBRARIES = libdebugger.a

libdebugger_a_SOURCES = breakpoint.c \
		        command.c \
			commandl.l \
			commandy.y \
			debugger.c \
			disassemble.c \
			event.c \
			expression.c \
			variable.c

commandl.c: commandy.c
commandy.h: commandy.c

noinst_HEADERS = breakpoint.h \
		 commandy.h \
		 debugger.h \
		 debugger_internals.h

CLEANFILES = commandl.c \
	     commandy.c \
	     commandy.h
