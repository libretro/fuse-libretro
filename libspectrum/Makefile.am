## Process this file with automake to produce Makefile.in
## Copyright (c) 2001-2013 Philip Kendall

## $Id: Makefile.am 5002 2013-05-26 16:18:52Z sbaldovi $

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

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libspectrum.la

libspectrum_la_SOURCES = bzip2.c \
			 creator.c \
			 crypto.c \
			 csw.c \
			 dck.c \
			 ide.c \
			 libspectrum.c \
                         memory.c \
			 microdrive.c \
			 plusd.c \
			 pzx_read.c \
			 rzx.c \
			 sna.c \
			 snp.c \
			 snapshot.c \
			 snap_accessors.c \
			 sp.c \
			 symbol_table.c \
			 szx.c \
			 tap.c \
			 tape.c \
			 tape_accessors.c \
			 tape_block.c \
			 tape_set.c \
		 	 timings.c \
			 tzx_read.c \
			 tzx_write.c \
			 utilities.c \
			 warajevo_read.c \
			 wav.c \
                         windres.rc \
			 z80.c \
			 z80em.c \
			 zlib.c \
			 zxs.c

libspectrum_la_LDFLAGS = -version-info 9:0:1 -no-undefined @WINDRES_LDFLAGS@

libspectrum_la_LIBADD = @AUDIOFILE_LIBS@ @GLIB_LIBS@ -lm

libspectrum_la_DEPENDENCIES = @WINDRES_OBJ@

BUILT_SOURCES = libspectrum.h snap_accessors.c tape_accessors.c tape_set.c

nodist_include_HEADERS = libspectrum.h
noinst_HEADERS = internals.h

noinst_PROGRAMS =
noinst_DATA =

AM_CFLAGS = -DLIBSPECTRUM_EXPORTS

make-perl$(EXEEXT): $(srcdir)/make-perl.c config.h
	$(CC_FOR_BUILD) -I. $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $<

libspectrum.h: libspectrum.h.in generate.pl snap_accessors.txt tape_accessors.txt config.h
	@PERL@ -p generate.pl $(srcdir) $(srcdir)/libspectrum.h.in > $@.tmp && mv $@.tmp $@

generate.pl: make-perl$(EXEEXT) generate.pl.in
	./make-perl$(EXEEXT) > $@
	cat $(srcdir)/generate.pl.in >> $@

snap_accessors.c: accessor.pl snap_accessors.txt
	@PERL@ $(srcdir)/accessor.pl $(srcdir)/snap_accessors.txt > $@.tmp && mv $@.tmp $@

tape_accessors.c: tape_accessors.pl tape_accessors.txt
	@PERL@ $(srcdir)/tape_accessors.pl $(srcdir)/tape_accessors.txt > $@.tmp && mv $@.tmp $@

tape_set.c: tape_set.pl tape_accessors.txt
	@PERL@ $(srcdir)/tape_set.pl $(srcdir)/tape_accessors.txt > $@.tmp && mv $@.tmp $@

windres.o: windres.rc
	@WINDRES@ -I. $(srcdir)/windres.rc windres.o

AM_CPPFLAGS = @GLIB_CFLAGS@ @AUDIOFILE_CFLAGS@

EXTRA_DIST = accessor.pl \
	     dll.c \
	     generate.pl.in \
	     m4/audiofile.m4 \
	     make-perl.c \
	     tape_accessors.pl \
	     tape_accessors.txt \
	     tape_block.h \
	     tape_set.pl \
	     libspectrum.h.in \
	     snap_accessors.txt

CLEANFILES = libspectrum.h \
	     snap_accessors.c \
	     tape_accessors.c \
	     tape_set.c \
	     generate.pl \
	     make-perl$(EXEEXT)

include doc/Makefile.am
include hacking/Makefile.am
include myglib/Makefile.am
include test/Makefile.am
