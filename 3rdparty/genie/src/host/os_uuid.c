/**
 * \file   os_uuid.c
 * \brief  Create a new UUID.
 * \author Copyright (c) 2002-2012 Jason Perkins and the Premake project
 */

#include "premake.h"

#if PLATFORM_WINDOWS
#include <objbase.h>
#endif


/*
 * Pull off the four lowest bytes of a value and add them to my array,
 * without the help of the determinately sized C99 data types that
 * are not yet universally supported.
 */
static void add(unsigned char* bytes, int offset, unsigned long value)
{
	int i;
	for (i = 0; i < 4; ++i)
	{
		bytes[offset++] = (unsigned char)(value & 0xff);
		value >>= 8;
	}
}


int os_uuid(lua_State* L)
{
	char uuid[38];
	unsigned char bytes[16];
	
	/* If a name argument is supplied, build the UUID from that. For speed we
	 * are using a simple DBJ2 hashing function; if this isn't sufficient we
	 * can switch to a full RFC 4122 §4.3 implementation later. */
	const char* name = luaL_optstring(L, 1, NULL);
	if (name != NULL)
	{
		add(bytes, 0, do_hash(name, 0));
		add(bytes, 4, do_hash(name, 'L'));
		add(bytes, 8, do_hash(name, 'u'));
		add(bytes, 12, do_hash(name, 'a'));
	}

	/* If no name is supplied, try to build one properly */	
	else
	{
#if PLATFORM_WINDOWS
		CoCreateGuid((GUID*)bytes);
#else
		int result;

		/* not sure how to get a UUID here, so I fake it */
		FILE* rnd = fopen("/dev/urandom", "rb");
		result = fread(bytes, 16, 1, rnd);
		fclose(rnd);
		if (!result)
		{
			return 0;
		}
#endif
	}

	sprintf(uuid, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		bytes[0], bytes[1], bytes[2], bytes[3],
		bytes[4], bytes[5],
		bytes[6], bytes[7],
		bytes[8], bytes[9],
		bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);

	lua_pushstring(L, uuid);
	return 1;
}
