/**
 * \file   os_ticks.c
 * \brief  Return the system tick counter (in microsecond units).
 */

#include <stdlib.h>
#include "premake.h"

// Epochs used by Windows and Unix time APIs.  These adjustments,
// when added to the time value returned by the OS, will yield
// the number of microsecond intervals since Jan 1, year 1.
#define TICK_EPOCH_WINDOWS ((lua_Integer)0x0701ce1722770000)
#define TICK_EPOCH_UNIX    ((lua_Integer)0x00dcbffeff2bc000)

#define TICKS_PER_SECOND   ((lua_Integer)1000000)

int os_ticks(lua_State* L)
{
    lua_Integer ticks = 0;

#if PLATFORM_WINDOWS
    FILETIME fileTimeUtc;
    GetSystemTimeAsFileTime(&fileTimeUtc);
    ticks =
        TICK_EPOCH_WINDOWS
        + ((lua_Integer)fileTimeUtc.dwHighDateTime << 32
            | (lua_Integer)fileTimeUtc.dwLowDateTime);
    ticks /= (lua_Integer)10;
#else
    struct timeval tp;
    if (gettimeofday(&tp, NULL) == 0) {
        ticks = 
            TICK_EPOCH_UNIX
            + (lua_Integer)tp.tv_sec * TICKS_PER_SECOND
            + (lua_Integer)tp.tv_usec;
    }
#endif

    lua_pushinteger(L, ticks);
    return 1;
}
