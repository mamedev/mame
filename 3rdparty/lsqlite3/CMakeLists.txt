# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( lsqlite3 C )
cmake_minimum_required ( VERSION 2.8 )
include ( cmake/dist.cmake )
include ( lua )

# Find SQLite3
find_package ( SQLite3 REQUIRED )
include_directories ( ${SQLITE3_INCLUDE_DIRS} )

# Create lsqlite module
install_lua_module ( lsqlite3 lsqlite3.c lsqlite3.def LINK ${SQLITE3_LIBRARIES} )
install_data ( HISTORY README )
install_doc( doc/ )
install_test ( lunit.lua test.lua tests-sqlite3.lua )
install_example ( examples/ )
