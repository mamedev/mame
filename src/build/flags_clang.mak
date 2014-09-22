# TODO: some of these are no longer necessary for newer clang versions - re-enable them
CCOMFLAGS += \
	-Wno-cast-align \
	-Wno-tautological-compare \
	-Wno-constant-logical-operand \
	-Wno-format-security \
	-Wno-shift-count-overflow \
	-Wno-self-assign-field

# TODO: needs to use $(CC)
TEST_CLANG := $(shell clang --version)

ifeq ($(findstring 3.4,$(TEST_CLANG)),3.4)
CCOMFLAGS += -Wno-inline-new-delete
endif

ifeq ($(findstring 3.5,$(TEST_CLANG)),3.5)
CCOMFLAGS += -Wno-inline-new-delete -Wno-absolute-value -Wno-dynamic-class-memaccess
# XCode 6.0.1 is built on a pre-release SVN version of clang 3.5, that doesn't support -Wno-absolute-value yet - so ignore that warning for now
CCOMFLAGS += -Wno-unknown-warning-option
endif

ifeq ($(TARGETOS),emscripten)
CCOMFLAGS += -Qunused-arguments
endif
