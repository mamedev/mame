/**
 * \file   path_isabsolute.c
 * \brief  Determines if a path is absolute or relative.
 * \author Copyright (c) 2002-2009 Jason Perkins and the Premake project
 */

#include "premake.h"

int path_isabsolute(lua_State* L)
{
	const char* path = luaL_checkstring(L, -1);
	if (is_absolute_path(path)) {
		lua_pushboolean(L, 1);
		return 1;
	}

	return 0;
}
