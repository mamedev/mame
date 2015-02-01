/**
 * \file   os_rmdir.c
 * \brief  Remove a subdirectory.
 * \author Copyright (c) 2002-2013 Jason Perkins and the Premake project
 */

#include <stdlib.h>
#include "premake.h"


int os_rmdir(lua_State* L)
{
	int z;
	const char* path = luaL_checkstring(L, 1);

#if PLATFORM_WINDOWS
	z = RemoveDirectory(path);
#else
	z = (0 == rmdir(path));
#endif

	if (!z)
	{
		lua_pushnil(L);
		lua_pushfstring(L, "unable to remove directory '%s'", path);
		return 2;
	}
	else
	{
		lua_pushboolean(L, 1);
		return 1;
	}
}
