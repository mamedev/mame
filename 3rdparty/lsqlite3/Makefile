# Makefile for lsqlite3 library for Lua

LIBNAME= lsqlite3
LIBNAMECOMPLETE = $(LIBNAME)complete

LUAEXE= lua

ROCKSPEC= $(shell find . -name $(LIBNAME)-*-*.rockspec ! -path './lua_modules/*')
ROCKSPECCOMPLETE= $(shell find . -name $(LIBNAMECOMPLETE)-*-*.rockspec ! -path './lua_modules/*')

all: install

install:
	luarocks make $(ROCKSPEC)
	luarocks make $(ROCKSPECCOMPLETE)

test: 
	$(LUAEXE) test/test.lua
	$(LUAEXE) test/tests-sqlite3.lua lsqlite3
	$(LUAEXE) test/tests-sqlite3.lua lsqlite3complete

.PHONY: all test install
