/**
 * \file   os_chdir.c
 * \brief  Change the current working directory.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#include "premake.h"


int os_chdir(lua_State* L)
{
	int z;
	const char* path = luaL_checkstring(L, 1);

#if PLATFORM_WINDOWS
	z = SetCurrentDirectory(path);
#else
	z = !chdir(path);
#endif

	if (!z)
	{
		lua_pushnil(L);
		lua_pushfstring(L, "unable to switch to directory '%s'", path);
		return 2;
	}
	else
	{
		lua_pushboolean(L, 1);
		return 1;
	}
}
