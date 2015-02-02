## Process this file with automake to produce Makefile.in
## Copyright (c) 1999-2004 Philip Kendall

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

FILES = cassette.bmp \
	keyboard.scr \
	microdrive.bmp \
	plus3disk.bmp \
	@AUTOLOAD_SNAPS@/disk_plus3.szx \
	@AUTOLOAD_SNAPS@/tape_128.szx \
	@AUTOLOAD_SNAPS@/tape_16.szx \
	@AUTOLOAD_SNAPS@/tape_2048.szx \
	@AUTOLOAD_SNAPS@/tape_2068.szx \
	@AUTOLOAD_SNAPS@/tape_48.szx \
	@AUTOLOAD_SNAPS@/tape_48_ntsc.szx \
	@AUTOLOAD_SNAPS@/tape_plus2.szx \
	@AUTOLOAD_SNAPS@/tape_plus2a.szx \
	@AUTOLOAD_SNAPS@/tape_plus3.szx \
	@AUTOLOAD_SNAPS@/tape_plus3e.szx \
	@AUTOLOAD_SNAPS@/tape_pentagon.szx \
	@AUTOLOAD_SNAPS@/tape_scorpion.szx \
	@AUTOLOAD_SNAPS@/tape_se.szx \
	@AUTOLOAD_SNAPS@/tape_ts2068.szx

pkgdata_DATA = $(FILES)

EXTRA_DIST = $(FILES) \
	compressed/disk_plus3.szx \
	compressed/tape_128.szx \
	compressed/tape_16.szx \
	compressed/tape_2048.szx \
	compressed/tape_2068.szx \
	compressed/tape_48.szx \
	compressed/tape_48_ntsc.szx \
	compressed/tape_plus2.szx \
	compressed/tape_plus2a.szx \
	compressed/tape_plus3.szx \
	compressed/tape_plus3e.szx \
	compressed/tape_pentagon.szx \
	compressed/tape_scorpion.szx \
	compressed/tape_se.szx \
	compressed/tape_ts2068.szx \
 	tests/fatware-success.hdf.bz2 \
 	tests/idedos-success.hdf.bz2 \
        tests/success.mdr \
	tests/success.mgt.bz2 \
	tests/success.opd \
	tests/success.udi \
	uncompressed/disk_plus3.szx \
	uncompressed/tape_128.szx \
	uncompressed/tape_16.szx \
	uncompressed/tape_2048.szx \
	uncompressed/tape_2068.szx \
	uncompressed/tape_48.szx \
	uncompressed/tape_48_ntsc.szx \
	uncompressed/tape_plus2.szx \
	uncompressed/tape_plus2a.szx \
	uncompressed/tape_plus3.szx \
	uncompressed/tape_plus3e.szx \
	uncompressed/tape_pentagon.szx \
	uncompressed/tape_scorpion.szx \
	uncompressed/tape_se.szx \
	uncompressed/tape_ts2068.szx

distclean-local:
	rm -f *.szx
