## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2003 Philip Kendall

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

noinst_LIBRARIES = libz80.a

libz80_a_SOURCES = z80.c z80_ops.c

AM_CPPFLAGS += @GTK_CFLAGS@ @GLIB_CFLAGS@ @LIBSPEC_CFLAGS@

BUILT_SOURCES = opcodes_base.c z80_cb.c z80_ddfd.c z80_ddfdcb.c z80_ed.c

opcodes_base.c: $(srcdir)/z80.pl $(srcdir)/opcodes_base.dat
	@PERL@ -I$(srcdir)/../perl $(srcdir)/z80.pl $(srcdir)/opcodes_base.dat > $@.tmp && mv $@.tmp $@

z80_cb.c: $(srcdir)/z80.pl $(srcdir)/opcodes_cb.dat
	@PERL@ -I$(srcdir)/../perl $(srcdir)/z80.pl $(srcdir)/opcodes_cb.dat > $@.tmp && mv $@.tmp $@

z80_ddfd.c: $(srcdir)/z80.pl $(srcdir)/opcodes_ddfd.dat
	@PERL@ -I$(srcdir)/../perl $(srcdir)/z80.pl $(srcdir)/opcodes_ddfd.dat > $@.tmp && mv $@.tmp $@

z80_ddfdcb.c: $(srcdir)/z80.pl $(srcdir)/opcodes_ddfd.dat
	@PERL@ -I$(srcdir)/../perl $(srcdir)/z80.pl $(srcdir)/opcodes_ddfdcb.dat > $@.tmp && mv $@.tmp $@

z80_ed.c: $(srcdir)/z80.pl $(srcdir)/opcodes_ed.dat
	@PERL@ -I$(srcdir)/../perl $(srcdir)/z80.pl $(srcdir)/opcodes_ed.dat > $@.tmp && mv $@.tmp $@

noinst_HEADERS = z80.h \
		 z80_checks.h \
		 z80_macros.h

EXTRA_DIST = opcodes_base.c \
	     opcodes_base.dat \
	     opcodes_cb.dat \
	     opcodes_ddfd.dat \
	     opcodes_ddfdcb.dat \
	     opcodes_ed.dat \
	     z80.pl \
	     z80_cb.c \
	     z80_ddfd.c \
	     z80_ddfdcb.c \
	     z80_ed.c

## The core tester

SUBDIRS = tests

noinst_PROGRAMS = coretest

coretest_SOURCES = coretest.c z80.c
coretest_LDADD = z80_coretest.o @LIBSPEC_LIBS@

coretest.o: $(srcdir)/coretest.c
	$(COMPILE) -DCORETEST -c $(srcdir)/coretest.c -o $@

z80_coretest.o: z80_ops.c
	$(COMPILE) -DCORETEST -c $(srcdir)/z80_ops.c -o $@

test: coretest
	./coretest $(srcdir)/tests/tests.in > tests.actual
	cmp tests.actual $(srcdir)/tests/tests.expected

CLEANFILES = opcodes_base.c \
	     z80_cb.c \
	     z80_ddfd.c \
	     z80_ddfdcb.c \
	     z80_ed.c
