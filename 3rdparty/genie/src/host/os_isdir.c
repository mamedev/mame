/**
 * \file   os_isdir.c
 * \brief  Returns true if the specified directory exists.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#include <string.h>
#include <sys/stat.h>
#include "premake.h"


int os_isdir(lua_State* L)
{
	struct stat buf;
	const char* path = luaL_checkstring(L, 1);

	/* empty path is equivalent to ".", must be true */
	if (strlen(path) == 0)
	{
		lua_pushboolean(L, 1);
	}
	else if (stat(path, &buf) == 0)
	{
		lua_pushboolean(L, buf.st_mode & S_IFDIR);
	}
	else
	{
		lua_pushboolean(L, 0);
	}

	return 1;
}


