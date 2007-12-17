#ifndef minx_H
#define minx_H
#include "cpuintrf.h"
#include "osd_cpu.h"
#include "driver.h"

enum {
        MINX_PC=1, MINX_SP, MINX_BA, MINX_HL, MINX_X, MINX_Y,
        MINX_U, MINX_V, MINX_F, MINX_E, MINX_N, MINX_I,
        MINX_XI, MINX_YI,
};

#ifdef MAME_DEBUG
extern unsigned minx_dasm( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram );
#endif

#endif

