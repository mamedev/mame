
#----------------------------------------------------------
# directory containing ./src and makefile
#----------------------------------------------------------

MAMESVN = ..

#----------------------------------------------------------
# Specifiy where object files should go. 
# Comment to use default location
#----------------------------------------------------------

DESTOBJ = $(PWD)/obj

#----------------------------------------------------------
# Specifiy where executables should go. 
# Comment to leave in mame top directory
#----------------------------------------------------------

DESTDIR = bin/$(CONFIG_NAME)-$(SUFFIX)

#----------------------------------------------------------
# Specifiy number of processes here
#----------------------------------------------------------

MAKE_PROCESSES = -j 3

#----------------------------------------------------------
#
# No configurable settings below
#
#----------------------------------------------------------

include default.mak

ifndef DESTOBJ
DESTOBJ = $(MAMESVN)/obj/$(CONFIG_NAME)-$(SUFFIX)
endif

-include defconfig.mak

ifdef CONFIG
-include $(CONFDIR)/$(CONFIG).mak
endif

BUILDOUT = $(DESTOBJ)/$(CONFIG_NAME)-$(SUFFIX)/native
OBJDIR = $(DESTOBJ)/$(CONFIG_NAME)-$(SUFFIX)
DIRS = $(DESTOBJ) $(DESTDIR) $(STAMPDIR) $(BUILDOUT)

MAME_PARAM = 

OPTEXP = $(subst $(COMMA), , $(OPT))

ifneq ($(filter debugger,$(OPTEXP)),)
MAME_PARAM += DEBUGGER=1
BUILD_SUFFIX += r
endif

ifneq ($(filter symbols,$(OPTEXP)),)
MAME_PARAM += SYMBOLS=1
BUILD_SUFFIX += s
endif

ifneq ($(filter no_optimize,$(OPTEXP)),)
BUILD_SUFFIX += _noopt
MAME_PARAM += OPTIMIZE=0
else
MAME_PARAM += OPTIMIZE=3
endif

ifneq ($(filter debug,$(OPTEXP)),)
MAME_PARAM += DEBUG=1
BUILD_SUFFIX += d
endif

SUFFIX = $(subst $(SPACE),,$(BUILD_SUFFIX))
ifeq ($(TARGET),$(SUBTARGET))
FULLNAME = $(TARGET)$(SUFFIX)
else
FULLNAME = $(TARGET)$(SUBTARGET)$(SUFFIX)
endif

MAME_PARAM += FULLNAME=$(FULLNAME)

ifneq ($(HAS_PREBUILD),0)
MAME_PARAM += BUILDOUT=$(BUILDOUT)
PREBUILD_PARAM += OBJ=$(BUILDOUT)
endif

BUILD_PARAM += OBJ=$(OBJDIR)

CLEAN_STMP = $(STAMPDIR)/$(CONFIG)_$(SUFFIX)_clean.stmp
BUILD_STMP = $(STAMPDIR)/$(CONFIG)_$(SUFFIX)_build.stmp

ifdef DESTDIR
DESTFILES = $(addprefix $(DESTDIR)/,$(FULLNAME)$(EXE_SUFFIX) $(MAME_TOOLS:%=%$(EXE_SUFFIX)))
endif

# --------------------------------------------------------------------------
#
# External targets
#
# --------------------------------------------------------------------------

all: usage

usage:
	@$(ECHO) "usage: make [CONFIG=<host>-<target>-<osd>]"
	@$(ECHO) "            [OPTS=[debug|debugger|symbols|no_optimize],...]] "
	@$(ECHO) "            list|build|clean|default"
	@$(ECHO) "   "
ifndef WIN32_LIMITED_TOOLSET
	@$(ECHO) "   list:    print a list of supported targets"
endif
	@$(ECHO) "   build:   build for the given or default configuration"
	@$(ECHO) "   clean:   schedule clean before next build"
	@$(ECHO) "   default: make the given configuration the default"
	@$(ECHO) "   "

ifndef WIN32_LIMITED_TOOLSET
list:
	@$(ECHO) Supported builds ...
	@for i in $(basename $(notdir $(wildcard $(CONFDIR)/*.mak))); do $(ECHO) $$i; done
endif

default: prereq
	@echo ifndef CONFIG > defconfig.mak
	@echo CONFIG = $(CONFIG) >> defconfig.mak
	@echo endif >> defconfig.mak
	@echo ifndef OPT >> defconfig.mak
	@echo OPT = $(OPT) >> defconfig.mak
	@echo endif >> defconfig.mak
	@$(ECHO) Set default configuration to $(CONFIG) - $(OPT)

clean: clean_sched prereq $(CLEAN_STMP)

# --------------------------------------------------------------------------
#
# Internal targets
#
# --------------------------------------------------------------------------

prebuild:	prereq $(PREBUILD_LOCAL)
ifneq ($(HAS_PREBUILD),0)
	@$(MAKE) -C $(MAMESVN) $(MAKE_PROCESSES) $(PREBUILD_PARAM) $(MAME_PARAM) $(PREBUILD_TARGETS)
else
	@$(ECHO) No prebuild
endif

postbuild:
	@$(MAKE) -C $(MAMESVN) $(MAKE_PROCESSES) $(BUILD_PARAM) $(MAME_PARAM) $(BUILD_TARGETS)
	@$(TOUCH) $(BUILD_STMP)
	
build:	info prereq $(CLEAN_STMP) prebuild postbuild $(DESTFILES)

info:
	@$(ECHO) Building $(CONFIG_NAME) with $(OPT)
	@$(ECHO)    MAME_PARAM: $(MAME_PARAM)
	@$(ECHO)

prereq:	$(CONFDIR)/$(CONFIG).mak $(DIRS)

clean_sched:
	@$(TOUCH) $(STAMPDIR)/clean.stmp
	@$(ECHO)	Scheduled clean

# --------------------------------------------------------------------------
#
# Clean Stamps
#
# --------------------------------------------------------------------------

$(STAMPDIR)/clean.stmp:
	@$(TOUCH) $@

$(CLEAN_STMP):	$(STAMPDIR)/clean.stmp
	@$(ECHO) Performing scheduled clean ...
	@$(MAKE) -C $(MAMESVN) $(MAKE_PROCESSES) $(BUILD_PARAM) $(MAME_PARAM) clean
ifneq ($(HAS_PREBUILD),0)
	@$(MAKE) -C $(MAMESVN) $(MAKE_PROCESSES) $(PREBUILD_PARAM) $(MAME_PARAM) clean
endif
	@$(TOUCH) $(CLEAN_STMP)

# --------------------------------------------------------------------------
#
# Files and Directories
#
# --------------------------------------------------------------------------

$(CONFDIR)/$(CONFIG).mak:
	@$(ECHO) No valid configuration found: configs/$(CONFIG).mak
	@$(ECHO) Use make usage for help
	exit 1

$(sort $(DIRS)):
	$(MKDIRP) $@

$(DESTDIR)/%: $(MAMESVN)/%
	@$(ECHO) Moving $(subst /,$(PATH_SEP), $< ) to $(subst /,$(PATH_SEP), $@ ) 
	@$(MVF) $(subst /,$(PATH_SEP), $< ) $(subst /,$(PATH_SEP), $@ ) 
	
.PHONY: all prebuild postbuild
.DEFAULT_GOAL := all

