###########################################################################
#
#   dist.mak
#
#   This is used during MAME release process, it's rather hacky
#
###########################################################################

ifeq ($(OS),Windows_NT)
  OS := windows
else
  UNAME := $(shell uname -mps)
  ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
    OS := linux
  else ifeq ($(firstword $(filter Solaris,$(UNAME))),Solaris)
    OS := solaris
  else ifeq ($(firstword $(filter SunOS,$(UNAME))),SunOS)
    OS := solaris
  else ifeq ($(firstword $(filter FreeBSD,$(UNAME))),FreeBSD)
    OS := freebsd
  else ifeq ($(firstword $(filter GNU/kFreeBSD,$(UNAME))),GNU/kFreeBSD)
    OS := freebsd
  else ifeq ($(firstword $(filter NetBSD,$(UNAME))),NetBSD)
    OS := netbsd
  else ifeq ($(firstword $(filter OpenBSD,$(UNAME))),OpenBSD)
    OS := openbsd
  else ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
    OS := osx
  else ifeq ($(firstword $(filter Haiku,$(UNAME))),Haiku)
    OS := haiku
  else ifndef OS
    $(error Unable to detect OS from uname -a: $(UNAME))
  endif
endif

ifndef TARGETOS
  TARGETOS := $(OS)
endif

EXE :=
ifeq ($(OS),windows)
  EXE := .exe
  ifeq ($(MSYSTEM),CLANGARM64)
    PROJECTTYPE := mingw-clang
  else
    PROJECTTYPE := mingw-gcc
  endif
else ifeq ($(OS),osx)
  PROJECTTYPE := osx_clang
else
  PROJECTTYPE := $(OS)_gcc
endif

ifeq ($(DEBUG),1)
  MAINBINVARIANT := d
  BUILDVARIANT := Debug
else
  MAINBINVARIANT :=
  BUILDVARIANT := Release
endif

ifeq ($(PTR64),1)
  BUILDARCH := x64
else
  BUILDARCH := x32
endif

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
else ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
else ifeq (/bin,$(findstring /bin,$(MAKESHELL)))
  SHELLTYPE := posix
endif

ifeq (posix,$(SHELLTYPE))
  MKDIR = $(SILENT) mkdir -p "$(1)"
  COPY  = $(SILENT) cp -fR "$(1)" "$(2)"
else
  MKDIR = $(SILENT) mkdir "$(subst /,\\,$(1))" 2> nul || exit 0
  COPY  = $(SILENT) copy /Y "$(subst /,\\,$(1))" "$(subst /,\\,$(2))" > nul || exit 0
endif

ifndef TARGET
  TARGET := mame
endif

MAINBIN := $(TARGET)$(MAINBINVARIANT)
BINDIR := build/$(PROJECTTYPE)/bin/$(BUILDARCH)/$(BUILDVARIANT)
STAGEDIR := build/release/$(BUILDARCH)/$(BUILDVARIANT)/$(TARGET)

ifeq ($(OS),windows)
  SYMFILE := $(addsuffix .sym,$(MAINBIN))
else
  SYMFILE :=
endif

BINARIES = $(MAINBIN) castool chdman floptool imgtool jedutil ldresample ldverify nltool nlwav romcmp unidasm
SIMPLE_DIRS := ctrlr docs/legal docs/man docs/swlist hash ini/examples ini/presets
LOCALISATIONS := $(wildcard language/*/*.mo)
COPIED_FILES := COPYING uismall.bdf roms/dir.txt $(foreach DIR,$(SIMPLE_DIRS),$(wildcard $(DIR)/*)) language/LICENSE language/README.md $(LOCALISATIONS)
CREATED_DIRS := docs ini roms $(SIMPLE_DIRS) language $(dir $(LOCALISATIONS))

GEN_FOLDERS := $(addprefix $(STAGEDIR)/,$(CREATED_DIRS))
COPY_BINARIES := $(addprefix $(STAGEDIR)/,$(addsuffix $(EXE),$(BINARIES)) $(SYMFILE))
COPY_FILES := $(addprefix $(STAGEDIR)/,$(COPIED_FILES))

all: $(COPY_BINARIES) $(COPY_FILES) $(STAGEDIR)/docs/MAME.pdf

clean:
	$(SILENT) rm -rf $(STAGEDIR)

$(GEN_FOLDERS):
	$(call MKDIR,$@)

$(STAGEDIR)/%$(EXE): $(BINDIR)/%$(EXE) | $(GEN_FOLDERS)
	$(call COPY,$<,$@)
	$(SILENT) strip $@

$(STAGEDIR)/%.sym: $(BINDIR)/%.sym | $(GEN_FOLDERS)
	$(call COPY,$<,$@)

$(STAGEDIR)/%: % | $(GEN_FOLDERS)
	$(call COPY,$<,$@)

$(STAGEDIR)/docs/MAME.pdf: docs/build/latex/MAME.pdf | $(GEN_FOLDERS)
	$(call COPY,$<,$@)

docs/build/latex/MAME.pdf:
	$(MAKE) -C docs PAPER=a4 latexpdf

.PHONY: all clean
