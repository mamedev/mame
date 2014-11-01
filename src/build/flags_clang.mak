CCOMFLAGS += \
	-Wno-cast-align \
	-Wno-tautological-compare \
	-Wno-format-security

# caused by obj/sdl64d/emu/cpu/tms57002/tms57002.inc
CCOMFLAGS += -Wno-self-assign-field

# caused by src/mame/video/jagblit.inc on older clang versions
CCOMFLAGS += -Wno-constant-logical-operand

# TODO: needs to use $(CC)
TEST_CLANG := $(shell clang --version)

ifeq ($(findstring 3.4,$(TEST_CLANG)),3.4)
CCOMFLAGS += -Wno-inline-new-delete
endif

ifeq ($(findstring 3.5,$(TEST_CLANG)),3.5)
CCOMFLAGS += -Wno-inline-new-delete
# caused by src/mess/drivers/x07.c, src/osd/sdl/window.c, src/emu/sound/disc_mth.inc
CCOMFLAGS += -Wno-absolute-value
# caused by src/emu/bus/isa/s3virge.c, src/emu/bus/isa/trident.c, src/emu/video/pc_vga.c
CCOMFLAGS +=-Wno-dynamic-class-memaccess
# these were disabled because of bugs in older clang versions
CCOMFLAGS += -Wformat-security
# these show up when compiling as c++11
CCOMFLAGS += -Wno-deprecated-register -Wno-reserved-user-defined-literal -Wno-c++11-narrowing
# TODO: add proper detection of XCode 6.0.1
# XCode 6.0.1 is built on a pre-release SVN version of clang 3.5, that doesn't support -Wno-absolute-value yet
CCOMFLAGS += -Wno-unknown-warning-option
# XCode 6.0.1 gives this when using SDL2 in /Library/Frameworks/SDL2.framework/Headers/SDL_syswm.h:150 included from src/osd/sdl/sdlinc.h
CCOMFLAGS += -Wno-extern-c-compat
endif

ifeq ($(TARGETOS),emscripten)
CCOMFLAGS += -Qunused-arguments
endif
