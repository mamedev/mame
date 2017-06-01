/**
 * \file   path_getabsolute.c
 * \brief  Calculate the absolute path from a relative path
 */

#include "premake.h"

int path_getabsolute(lua_State *L)
{
	const char *path = luaL_checkstring(L, -1);
	char buffer[PATH_BUFSIZE];
	get_absolute_path(path, buffer, PATH_BUFSIZE);
	lua_pushstring(L, buffer);
	return 1;
}
