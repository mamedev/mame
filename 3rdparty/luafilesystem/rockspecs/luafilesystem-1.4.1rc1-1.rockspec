package = "LuaFileSystem"
version = "1.4.1rc1-1"
source = {
   url = "http://luafilesystem.luaforge.net/luafilesystem-1.4.1rc1.tar.gz",
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
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR) $(STAT64)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)"
       	}
     },
     win32 = {
        type = "make",
       	build_variables = {
         LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib",
         CFLAGS = "/MD $(CFLAGS) /I$(LUA_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
         LUA_DIR = "$(LUADIR)",
	 	 BIN_DIR = "$(BINDIR)"
       	}
     }
  }
}
