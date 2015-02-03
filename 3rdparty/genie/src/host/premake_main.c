/**
 * \file   premake_main.c
 * \brief  Program entry point.
 * \author Copyright (c) 2002-2012 Jason Perkins and the Premake project
 */

#include "premake.h"

int main(int argc, const char** argv)
{
	lua_State* L;
	int z = OKAY;

	L = luaL_newstate();
	luaL_openlibs(L);
	z = premake_init(L);

	/* push the location of the Premake executable */
	premake_locate(L, argv[0]);
	lua_setglobal(L, "_PREMAKE_COMMAND");

	if (z == OKAY)
	{
		z = premake_execute(L, argc, argv);
	}
	
	lua_close(L);
	return z;
}
