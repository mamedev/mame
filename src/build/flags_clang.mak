CCOMFLAGS += \
	-Wno-cast-align \
	-Wno-tautological-compare

# caused by dynamic_array being generally awful
CCOMFLAGS += -Wno-dynamic-class-memaccess

# caused by obj/sdl64d/emu/cpu/tms57002/tms57002.inc
CCOMFLAGS += -Wno-self-assign-field

# caused by popmessage(NULL) on older clang versions
#CCOMFLAGS += -Wno-format-security

ifneq (,$(findstring undefined,$(SANITIZE)))
# TODO: check if linker is clang++
# produces a lot of messages - disable it for now
CCOMFLAGS += -fno-sanitize=alignment
# these are false positives because of the way our delegates work
CCOMFLAGS += -fno-sanitize=function
endif

ifneq (,$(findstring memory,$(SANITIZE)))
CCOMFLAGS += -fsanitize-memory-track-origins -fPIE
endif

ifdef CPP11
CCOMFLAGS += -Wno-deprecated-register -Wno-c++11-narrowing
endif

# TODO: needs to use $(CC)
TEST_CLANG := $(shell clang --version)

ifeq ($(findstring 3.4,$(TEST_CLANG)),3.4)
CCOMFLAGS += -Wno-inline-new-delete

# caused by src/mame/video/jagblit.inc
CCOMFLAGS += -Wno-constant-logical-operand
endif

ifeq ($(findstring 3.5,$(TEST_CLANG)),3.5)
CCOMFLAGS += -Wno-inline-new-delete

# caused by src/mess/drivers/x07.c, src/osd/sdl/window.c, src/emu/sound/disc_mth.inc, src/mame/video/chihiro.c
CCOMFLAGS += -Wno-absolute-value

# TODO: add proper detection of XCode 6.0.1
# XCode 6.0.1 is built on a pre-release SVN version of clang 3.5, that doesn't support -Wno-absolute-value yet
CCOMFLAGS += -Wno-unknown-warning-option
# XCode 6.0.1 gives this when using SDL2 in /Library/Frameworks/SDL2.framework/Headers/SDL_syswm.h:150 included from src/osd/sdl/sdlinc.h
CCOMFLAGS += -Wno-extern-c-compat

ifneq (,$(findstring undefined,$(SANITIZE)))
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=shift
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/hd6309.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=object-size
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/konami.c, src/emu/cpu/m6809/hd6309.c, src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=vptr
# clang takes forever to compile src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=null
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=signed-integer-overflow
endif
endif

ifeq ($(findstring 3.6,$(TEST_CLANG)),3.6)
CCOMFLAGS += -Wno-inline-new-delete

# caused by src/mess/drivers/x07.c, src/osd/sdl/window.c, src/emu/sound/disc_mth.inc, src/mame/video/chihiro.c
CCOMFLAGS += -Wno-absolute-value

ifneq (,$(findstring undefined,$(SANITIZE)))
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=shift
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/hd6309.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=object-size
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/konami.c, src/emu/cpu/m6809/hd6309.c, src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=vptr
# clang takes forever to compile src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=null
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=signed-integer-overflow
endif
endif

ifeq ($(TARGETOS),emscripten)
CCOMFLAGS += -Qunused-arguments
endif
