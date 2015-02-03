/**
 * \file   os_copyfile.c
 * \brief  Copy a file from one location to another.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#include <stdlib.h>
#include "premake.h"

int os_copyfile(lua_State* L)
{
	int z;
	const char* src = luaL_checkstring(L, 1);
	const char* dst = luaL_checkstring(L, 2);

#if PLATFORM_WINDOWS
	z = CopyFile(src, dst, FALSE);
#else
	lua_pushfstring(L, "cp %s %s", src, dst);
	z = (system(lua_tostring(L, -1)) == 0);
#endif

	if (!z)
	{
		lua_pushnil(L);
		lua_pushfstring(L, "unable to copy file to '%s'", dst);
		return 2;
	}
	else
	{
		lua_pushboolean(L, 1);
		return 1;
	}
}
