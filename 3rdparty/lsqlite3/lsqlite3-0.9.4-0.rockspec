package = "lsqlite3"
version = "0.9.4-0"
source = {
    url = "http://lua.sqlite.org/index.cgi/zip/lsqlite3_fsl09x.zip?uuid=fsl_9x",
    file = "lsqlite3_fsl09x.zip"
}
description = {
    summary = "A binding for Lua to the SQLite3 database library",
    detailed = [[
        lsqlite3 is a thin wrapper around the public domain SQLite3 database engine. 
        The lsqlite3 module supports the creation and manipulation of SQLite3 databases. 
        After a require('lsqlite3') the exported functions are called with prefix sqlite3. 
        However, most sqlite3 functions are called via an object-oriented interface to 
        either database or SQL statement objects.
    ]],
    license = "MIT/X11",
    homepage = "http://lua.sqlite.org/"
}
dependencies = {
    "lua >= 5.1, < 5.4"
}
external_dependencies = {
    SQLITE = {
        header = "sqlite3.h"
    }
}
build = {
    type = "builtin",
    modules = {
        lsqlite3 = {
            sources = { "lsqlite3.c" },
            defines = {'LSQLITE_VERSION="0.9.4"'},
            libraries = { "sqlite3" },
            incdirs = { "$(SQLITE_INCDIR)" },
            libdirs = { "$(SQLITE_LIBDIR)" }
        },
        lsqlite3complete = {
            sources = { "lsqlite3.c", "sqlite3.c" },
            defines = {'LSQLITE_VERSION="0.9.4"', 'luaopen_lsqlite3=luaopen_lsqlite3complete'}
        }
    },
	copy_directories = { 'doc', 'examples' }
}
