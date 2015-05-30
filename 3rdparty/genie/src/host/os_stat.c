/**
 * \file   os_stat.c
 * \brief  Retrieve information about a file.
 * \author Copyright (c) 2011 Jason Perkins and the Premake project
 */

#include "premake.h"
#include <sys/stat.h>
#include <errno.h>

int os_stat(lua_State* L)
{
	struct stat s;
	
	const char* filename = luaL_checkstring(L, 1);
    if (stat(filename, &s) != 0)
	{
		lua_pushnil(L);
		switch (errno)
		{
		case EACCES:
			lua_pushfstring(L, "'%s' could not be accessed", filename);
			break;
		case ENOENT:
			lua_pushfstring(L, "'%s' was not found", filename);
			break;
		default:	
			lua_pushfstring(L, "An  unknown error %d occured while accessing '%s'", errno, filename);
			break;
		}
		return 2;
	}
	
	
	lua_newtable(L);

	lua_pushstring(L, "mtime");
	lua_pushinteger(L, (lua_Integer)s.st_mtime);
	lua_settable(L, -3);

	lua_pushstring(L, "size");
	lua_pushnumber(L, s.st_size);
	lua_settable(L, -3);

	return 1;
}
