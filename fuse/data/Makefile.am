## Process this file with automake to produce Makefile.in
## Copyright (c) 2011 Philip Kendall

## $Id: Makefile.am 4674 2012-02-20 11:03:58Z fredm $

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

if DESKTOP_INTEGRATION

fusemimedir = $(DESKTOP_DATADIR)/mime/packages
fusemime_DATA = fuse.xml

appdatadir = $(DESKTOP_DATADIR)/applications
appdata_DATA = fuse.desktop

iconsdatadir = $(DESKTOP_DATADIR)/icons/hicolor
icons16dir = $(iconsdatadir)/16x16/apps
icons32dir = $(iconsdatadir)/32x32/apps
icons48dir = $(iconsdatadir)/48x48/apps
icons64dir = $(iconsdatadir)/64x64/apps
icons128dir = $(iconsdatadir)/128x128/apps
icons256dir = $(iconsdatadir)/256x256/apps

dist_icons16_DATA = icons/16x16/fuse.png
dist_icons32_DATA = icons/32x32/fuse.png
dist_icons48_DATA = icons/48x48/fuse.png
dist_icons64_DATA = icons/64x64/fuse.png
dist_icons128_DATA = icons/128x128/fuse.png
dist_icons256_DATA = icons/256x256/fuse.png

mimeicons16dir = $(iconsdatadir)/16x16/mimetypes
mimeicons32dir = $(iconsdatadir)/32x32/mimetypes
mimeicons48dir = $(iconsdatadir)/48x48/mimetypes
mimeicons64dir = $(iconsdatadir)/64x64/mimetypes
mimeicons128dir = $(iconsdatadir)/128x128/mimetypes
mimeicons256dir = $(iconsdatadir)/256x256/mimetypes

dist_mimeicons16_DATA = icons/16x16/application-x-spectrum.png
dist_mimeicons32_DATA = icons/32x32/application-x-spectrum.png
dist_mimeicons48_DATA = icons/48x48/application-x-spectrum.png
dist_mimeicons64_DATA = icons/64x64/application-x-spectrum.png
dist_mimeicons128_DATA = icons/128x128/application-x-spectrum.png
dist_mimeicons256_DATA = icons/256x256/application-x-spectrum.png

# Generate fuse.xml according to libspectrum capabilities
fuse.xml: fuse.xml.in
	if test -z "$(LIBSPEC_HAVE_AUDIOFILE)"; then \
	  mime_audiofile='/LIBSPECTRUM_HAVE_AUDIOFILE_BEGIN/,/LIBSPECTRUM_HAVE_AUDIOFILE_END/d'; \
	else \
	  mime_audiofile='/LIBSPECTRUM_HAVE_AUDIOFILE/d'; \
	fi; \
	if test -z "$(LIBSPEC_HAVE_ZLIB)"; then \
	  mime_zlib='/LIBSPECTRUM_HAVE_ZLIB_BEGIN/,/LIBSPECTRUM_HAVE_ZLIB_END/d'; \
	else \
	  mime_zlib='/LIBSPECTRUM_HAVE_ZLIB/d'; \
	fi; \
	if test -z "$(LIBSPEC_HAVE_BZ2)"; then \
	  mime_bz2='/LIBSPECTRUM_HAVE_BZ2_BEGIN/,/LIBSPECTRUM_HAVE_BZ2_END/d'; \
	else \
	  mime_bz2='/LIBSPECTRUM_HAVE_BZ2/d'; \
	fi; \
	$(SED) -e "$$mime_audiofile" -e "$$mime_zlib" -e "$$mime_bz2" < fuse.xml.in > $@

# Generate fuse.desktop from fuse.xml to keep mime types list in sync
fuse.desktop: fuse.desktop.in $(srcdir)/fuse.xml
	list=`$(SED) -e '/<mime-type/ s/.*<mime-type type="\(.*\)">.*/\1;/' -e t -e d < $(srcdir)/fuse.xml | tr -d '\n'` && \
	$(SED) -e "s|@MIME_TYPES@|$$list|" < fuse.desktop.in > $@

update-database-cache:
	if test -z "$(DESTDIR)"; then \
	  update-desktop-database $(DESKTOP_DATADIR)/applications; \
	  update-mime-database $(DESKTOP_DATADIR)/mime; \
	  touch --no-create $(iconsdatadir); \
	  $(GTK_UPDATE_ICON_CACHE) -t ${iconsdatadir}; \
	fi

install-data-hook: update-database-cache

uninstall-hook: update-database-cache

validate: fuse.desktop
	( dfvalidate=`which desktop-file-validate`; \
	  if test x$$dfvalidate != x && test -x $$dfvalidate; then \
            $$dfvalidate $< || ( echo "* $< INVALID *"; exit 1 ) \
	  fi )

dist-hook: validate

endif

EXTRA_DIST = \
	fuse.desktop.in \
	fuse.xml.in

CLEANFILES = \
	fuse.desktop \
	fuse.xml

