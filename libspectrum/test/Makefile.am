## Process this file with automake to produce Makefile.in
## Copyright (c) 2007 Philip Kendall

## $Id: Makefile.am 4220 2010-12-17 10:54:29Z fredm $

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

noinst_PROGRAMS += test/test

test_test_SOURCES = \
	test/edges.c \
	test/test.c \
	test/test15.c

test_test_CFLAGS = -DSRCDIR='"$(srcdir)"'

noinst_DATA += test/complete-tzx.tzx

noinst_HEADERS += test/test.h

test/complete-tzx.tzx: test/complete-tzx.pl
	perl $< > $@.tmp && mv $@.tmp $@

test_test_LDADD = libspectrum.la

EXTRA_DIST += \
	test/Makefile.am \
	test/complete-tzx.pl \
	test/empty-drb.tzx \
	test/empty.csw \
        test/empty.szx \
	test/empty.z80 \
	test/invalid-archiveinfo.tzx \
	test/invalid-custominfo.tzx \
	test/invalid-gdb.tzx \
	test/invalid-hardwareinfo.tzx \
	test/invalid-warajevo-blockoffset.tap \
	test/invalid.gz \
	test/invalid.szx \
	test/invalid.tzx \
	test/jump.tzx \
	test/loop.tzx \
	test/loop2.tzx \
	test/loopend.tzx \
        test/plus3.z80 \
	test/sp-2000.sna.gz \
	test/sp-ffff.sna.gz \
	test/turbo-zeropilot.tzx \
	test/writeprotected.mdr

CLEANFILES += \
	test/.libs/test \
	test/complete-tzx.tzx
