BGFX_DIR=../../../../../../3rdparty/bgfx
RUNTIME_DIR=../../../../../../bgfx
BUILD_DIR=../../../../../../build

include shader.mk

ifneq ($(CHAIN),)
SUBDIRS := $(patsubst .,,$(patsubst ,,$(shell find chains/$(CHAIN) -type d)))
rebuild: $(SUBDIRS)
else
SUBDIRS := $(patsubst .,,$(patsubst ./,,$(shell find . -type d)))
rebuild: main $(SUBDIRS)
endif

.NOTPARALLEL:
$(SUBDIRS):
	@echo $@
ifeq ($(OS),windows)
	$(SILENT) $(MAKE) --no-print-directory TARGET=0 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=1 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
endif
	$(SILENT) $(MAKE) --no-print-directory TARGET=2 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=3 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=4 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=5 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=7 SHADERS_DIR=$@/ clean all SILENT="$(SILENT)"

main:
ifeq ($(OS),windows)
	$(SILENT) $(MAKE) --no-print-directory TARGET=0 clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=1 clean all SILENT="$(SILENT)"
endif
	$(SILENT) $(MAKE) --no-print-directory TARGET=2 clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=3 clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=4 clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=5 clean all SILENT="$(SILENT)"
	$(SILENT) $(MAKE) --no-print-directory TARGET=7 clean all SILENT="$(SILENT)"

.PHONY: main rebuild $(SUBDIRS)
