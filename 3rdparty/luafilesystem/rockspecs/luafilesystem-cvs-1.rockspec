package = "LuaFileSystem"
version = "cvs-1"
source = {
   url = "cvs://:pserver:anonymous:@cvs.luaforge.net:/cvsroot/luafilesystem",
   cvs_tag = "HEAD"
}
description = {
   summary = "File System Library for the Lua Programming Language",
   detailed = [[
      LuaFileSystem is a Lua library developed to complement the set of
      functions related to file systems offered by the standard Lua
      distribution. LuaFileSystem offers a portable way to access the
      underlying directory structure and file attributes.
   ]]
}
dependencies = {
   "lua >= 5.1"
}
build = {
   platforms = {
     unix = {
        type = "make",
       	build_variables = {
         LIB_OPTION = "$(LIBFLAG)",
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)"
       	}
     },
     win32 = {
        type = "make",
       	build_variables = {
         LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib",
         CFLAGS = "$(CFLAGS) /I$(LUA_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
         LUA_DIR = "$(LUADIR)",
	 BIN_DIR = "$(BINDIR)"
       	}
     }
  }
}
