/*
    This is the previous implementation of the TMS9900 using the common
    core implementation in 99xxcore. The new cycle-precise implementation
    can be found in tms9900.c
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900l.h"

#define TMS99XX_MODEL TMS9900_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TMS9900L, tms9900l);
