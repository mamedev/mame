#  flac - Command-line FLAC encoder/decoder
#  Copyright (C) 2002-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

SUBDIRS = . images

if FLaC__HAS_DOXYGEN
all-local: Doxyfile
FLAC.tag: Doxyfile
	doxygen Doxyfile
	rm -rf api
	mv doxytmp/html api
	rm -rf doxytmp

doc_DATA = \
	FLAC.tag
else
if FLaC__HAS_PREBUILT_DOXYGEN
doc_DATA = \
	FLAC.tag
endif
endif

EXTRA_DIST = Doxyfile.in doxygen.footer.html \
	isoflac.txt $(doc_DATA) CMakeLists.txt \
	api

if FLaC__HAS_DOXYGEN
# The install targets don't copy whole directories so we have to
# handle 'api/' specially:
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)/api
	(cd $(builddir)/api && $(INSTALL_DATA) * $(DESTDIR)$(docdir)/api)
uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/api
distclean-local:
	rm -rf FLAC.tag api doxytmp
endif

if FLaC__HAS_PREBUILT_DOXYGEN
# The install targets don't copy whole directories so we have to
# handle 'api/' specially:
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)/api
	(cd $(srcdir)/api && $(INSTALL_DATA) * $(DESTDIR)$(docdir)/api)
uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/api
endif
