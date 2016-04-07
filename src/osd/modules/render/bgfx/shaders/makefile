BGFX_DIR=../../../../../../3rdparty/bgfx
RUNTIME_DIR=../../../../../../bgfx
BUILD_DIR=../../../../../../build

include shader.mk

SUBDIRS := $(patsubst .,,$(patsubst ./,,$(shell find . -type d)))


rebuild: main $(SUBDIRS)

$(SUBDIRS):
	@echo $@
	@make -s --no-print-directory TARGET=0 SHADERS_DIR=$@/ clean all
	@make -s --no-print-directory TARGET=1 SHADERS_DIR=$@/ clean all
	@make -s --no-print-directory TARGET=2 SHADERS_DIR=$@/ clean all
	@make -s --no-print-directory TARGET=3 SHADERS_DIR=$@/ clean all
	@make -s --no-print-directory TARGET=4 SHADERS_DIR=$@/ clean all
	@make -s --no-print-directory TARGET=5 SHADERS_DIR=$@/ clean all

main:
	@make -s --no-print-directory TARGET=0 clean all
	@make -s --no-print-directory TARGET=1 clean all
	@make -s --no-print-directory TARGET=2 clean all
	@make -s --no-print-directory TARGET=3 clean all
	@make -s --no-print-directory TARGET=4 clean all
	@make -s --no-print-directory TARGET=5 clean all
	
.PHONY: main rebuild $(SUBDIRS)
