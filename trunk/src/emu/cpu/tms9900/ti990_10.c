/*
    Generate the tms9900 emulator
*/

#include "emu.h"
#include "debugger.h"
#include "tms9900.h"

#define TMS99XX_MODEL TI990_10_ID

#include "99xxcore.h"

DEFINE_LEGACY_CPU_DEVICE(TI990_10, ti990_10);
