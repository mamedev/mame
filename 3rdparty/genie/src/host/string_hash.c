/**
 * \file   string_hash.c
 * \brief  Computes a hash value for a string.
 * \author Copyright (c) 2012 Jason Perkins and the Premake project
 */

#include "premake.h"
#include <string.h>


int string_hash(lua_State* L)
{
	const char* str = luaL_checkstring(L, 1);
	lua_pushnumber(L, (lua_Number)do_hash(str, 0));
	return 1;	
}


unsigned long do_hash(const char* str, int seed)
{
	/* DJB2 hashing; see http://www.cse.yorku.ca/~oz/hash.html */
	
	unsigned long hash = 5381;
	
	if (seed != 0) {
		hash = hash * 33 + seed;
	}

	while (*str) {
		hash = hash * 33 + (*str);
		str++;
	}

	return hash;
}
