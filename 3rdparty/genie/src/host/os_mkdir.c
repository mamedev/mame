/**
 * \file   os_mkdir.c
 * \brief  Create a subdirectory.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#include <sys/stat.h>
#include "premake.h"


int os_mkdir(lua_State* L)
{
	int z;
	const char* path = luaL_checkstring(L, 1);

#if PLATFORM_WINDOWS
	z = CreateDirectory(path, NULL);
#else
	z = (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
#endif

	if (!z)
	{
		lua_pushnil(L);
		lua_pushfstring(L, "unable to create directory '%s'", path);
		return 2;
	}
	else
	{
		lua_pushboolean(L, 1);
		return 1;
	}
}
