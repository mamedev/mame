package = "lsqlite3"
version = "0.9.5-1"
source = {
    url = "http://lua.sqlite.org/index.cgi/zip/lsqlite3_fsl09y.zip?uuid=fsl_9y",
    file = "lsqlite3_fsl09y.zip"
}
description = {
    summary = "A binding for Lua to the SQLite3 database library",
    detailed = [[
        lsqlite3 is a thin wrapper around the public domain SQLite3 database engine. SQLite3 is 
        dynamically linked to lsqlite3. The statically linked alternative is lsqlite3complete.
        The lsqlite3 module supports the creation and manipulation of SQLite3 databases. 
        Most sqlite3 functions are called via an object-oriented interface to either 
        database or SQL statement objects.
    ]],
    license = "MIT",
    homepage = "http://lua.sqlite.org/"
}
dependencies = {
    "lua >= 5.1, < 5.5"
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
            defines = {'LSQLITE_VERSION="0.9.5"'},
            libraries = { "sqlite3" },
            incdirs = { "$(SQLITE_INCDIR)" },
            libdirs = { "$(SQLITE_LIBDIR)" }
        },
    },
	copy_directories = { 'doc', 'examples' }
}
