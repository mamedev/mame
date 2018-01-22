/**
 * \file   path_getrelative.c
 * \brief  Calculate the relative path from src to dest
 */

#include "premake.h"

int path_getrelative(lua_State *L)
{
	const char *src = luaL_checkstring(L, -2);
	const char *dst = luaL_checkstring(L, -1);
	char buffer[PATH_BUFSIZE];
	get_relative_path(src, dst, buffer, PATH_BUFSIZE);
	lua_pushstring(L, buffer);
	return 1;
}
