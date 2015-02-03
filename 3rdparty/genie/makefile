#
# Copyright 2011-2014 Branimir Karadzic. All rights reserved.
# License: http://www.opensource.org/licenses/BSD-2-Clause
#

UNAME := $(shell uname)
ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin))
ifeq ($(UNAME),$(filter $(UNAME),Darwin))
OS=darwin
else
OS=linux
endif
else
OS=windows
endif

.PHONY: release

GENIE=bin/$(OS)/genie

SILENT?=@

$(GENIE):
	$(SILENT) make -C build/gmake.$(OS)

all: $(GENIE)

clean:
	$(SILENT) make -C build/gmake.$(OS) clean
	$(SILENT) -rm -rf bin

rebuild:
	$(SILENT) make -C build/gmake.$(OS) clean all

release-windows release-darwin: $(GENIE)
	$(GENIE) release
	$(SILENT) make -C build/gmake.$(OS) clean all
	$(SILENT) git checkout src/host/version.h

release-linux: $(GENIE)
	$(SILENT) $(GENIE) release
	$(SILENT) make -C build/gmake.darwin  clean all CC=x86_64-apple-darwin13-clang++
	$(SILENT) make -C build/gmake.linux   clean all
	$(SILENT) make -C build/gmake.windows clean all CC=x86_64-w64-mingw32-gcc
	$(SILENT) git checkout src/host/version.h

release: release-$(OS)
