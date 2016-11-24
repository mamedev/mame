# Makefile for lsqlite3 library for Lua
# This file is old and crufty. 
# We recommend you make lsqlite3 using luarocks.
# A local make+install (to test source changes) can be done with:
# sudo luarocks make lsqlite3-0.9-1.rockspec 
#

# manual setup (change these to reflect your Lua installation)
#
BASE= /usr/local
LUAINC= -I$(BASE)/include
LUALIB=
SQLITE3INC= -I$(BASE)/include
SQLITE3LIB= -L$(BASE)/lib -lsqlite3
#  Windows' LUALIB is the same as the Lua executable's directory...
#  LUALIB= -L$(BASE)/bin -llua51
#
POD2HTML= perl -x -S doc/pod2html.pl

LUAEXE= lua

INSTALL= install -p
INSTALLPATH= $(LUAEXE) installpath.lua

TMP=./tmp
DISTDIR=./archive

# OS detection
#
SHFLAGS=-shared
UNAME= $(shell uname)
ifeq "$(UNAME)" "Linux"
  _SO=so
  SHFLAGS=-shared -fPIC
endif
ifneq "" "$(findstring BSD,$(UNAME))"
  _SO=so
endif
ifeq "$(UNAME)" "Darwin"
  _SO=so
  SHFLAGS=-fPIC -arch i686 -arch x86_64
# SOFLAGS=-dynamiclib -single_module -undefined dynamic_lookup -arch i686 -arch x86_64
  SOFLAGS=-dynamic -bundle -undefined dynamic_lookup -all_load -arch i686 -arch x86_64 
endif
ifneq "" "$(findstring msys,$(OSTYPE))"		# 'msys'
  _SO=dll
endif

ifndef _SO
  $(error $(UNAME))
  $(error Unknown OS)
endif

# no need to change anything below here - HAH!
CFLAGS= $(INCS) $(DEFS) $(WARN) -O2 -fomit-frame-pointer $(SHFLAGS)
WARN= -Wall #-ansi -pedantic -Wall
INCS= $(LUAINC) $(SQLITE3INC)
LIBS= $(LUALIB) $(SQLITE3LIB)

MYNAME= sqlite3
MYLIB= l$(MYNAME)

VER=$(shell svnversion -c . | sed 's/.*://')
TARFILE = $(DISTDIR)/$(MYLIB)-$(VER).tar.gz

OBJS= $(MYLIB).o
T= $(MYLIB).$(_SO)

all: $(T)

test: $(T)
	$(LUAEXE) test.lua
	$(LUAEXE) tests-sqlite3.lua

$(T):	$(OBJS)
	$(CC) $(SHFLAGS) $(SOFLAGS) -o $@ $(OBJS) $(LIBS)

install: $(T)
	$(INSTALL) $< `$(INSTALLPATH) $(MYLIB)`

clean:
	rm -f $(OBJS) $T core core.* a.out test.db

html:
	$(POD2HTML) --title="LuaSQLite 3" --infile=doc/lsqlite3.pod --outfile=doc/lsqlite3.html

dist:	html
	echo 'Exporting...'
	mkdir -p $(TMP)
	mkdir -p $(DISTDIR)
	svn export . $(TMP)/$(MYLIB)-$(VER)
	mkdir -p $(TMP)/$(MYLIB)-$(VER)/doc
	cp -p doc/lsqlite3.html $(TMP)/$(MYLIB)-$(VER)/doc
	echo 'Compressing...'
	tar -zcf $(TARFILE) -C $(TMP) $(MYLIB)-$(VER)
	rm -fr $(TMP)/$(MYLIB)-$(VER)
	echo 'Done.'

.PHONY: all test clean dist install
