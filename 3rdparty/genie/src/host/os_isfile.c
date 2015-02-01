/**
 * \file   os_isfile.c
 * \brief  Returns true if the given file exists on the file system.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#include <sys/stat.h>
#include "premake.h"


int os_isfile(lua_State* L)
{
	const char* filename = luaL_checkstring(L, 1);
	lua_pushboolean(L, do_isfile(filename));
	return 1;
}


int do_isfile(const char* filename)
{
	struct stat buf;
	if (stat(filename, &buf) == 0)
	{
		return ((buf.st_mode & S_IFDIR) == 0);
	}
	else
	{
		return 0;
	}
}
