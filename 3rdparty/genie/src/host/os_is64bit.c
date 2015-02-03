/**
 * \file   os_is64bit.c
 * \brief  Native code-side checking for a 64-bit architecture.
 * \author Copyright (c) 2011 Jason Perkins and the Premake project
 */

#include "premake.h"

int os_is64bit(lua_State* L)
{
	// If this code returns true, then the platform is 64-bit. If it
	// returns false, the platform might still be 64-bit, but more 
	// checking will need to be done on the Lua side of things.
#if PLATFORM_WINDOWS
	typedef BOOL (WINAPI* WowFuncSig)(HANDLE, PBOOL);
	WowFuncSig func = (WowFuncSig)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (func)
	{
		BOOL isWow = FALSE;
		if (func(GetCurrentProcess(), &isWow))
		{
			lua_pushboolean(L, isWow);
			return 1;
		}
	}
#endif

	lua_pushboolean(L, 0);
	return 1;
}
