/*
    generate the tms9980a/tms9981 emulator
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900.h"

#define TMS99XX_MODEL TMS9980_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TMS9980, tms9980a);
