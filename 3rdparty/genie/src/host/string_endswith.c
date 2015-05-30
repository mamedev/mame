/**
 * \file   string_endswith.c
 * \brief  Determines if a string ends with the given sequence.
 * \author Copyright (c) 2002-2009 Jason Perkins and the Premake project
 */

#include "premake.h"
#include <string.h>


int string_endswith(lua_State* L)
{
	const char* haystack = luaL_optstring(L, 1, NULL);
	const char* needle   = luaL_optstring(L, 2, NULL);

	if (haystack && needle)
	{
		int hlen = strlen(haystack);
		int nlen = strlen(needle);
		if (hlen >= nlen) 
		{
			lua_pushboolean(L, strcmp(haystack + hlen - nlen, needle) == 0);
			return 1;
		}
	}

	return 0;
}
