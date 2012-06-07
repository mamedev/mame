/*
    generate the tms9980a/tms9981 emulator
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900l.h"

#define TMS99XX_MODEL TMS9980_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TMS9980L, tms9980al);
