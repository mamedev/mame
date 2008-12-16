/*** konami: Portable Konami cpu emulator ******************************************/

#pragma once

#ifndef __KONAMI_H__
#define __KONAMI_H__

#include "cpuintrf.h"

typedef void (*konami_set_lines_func)(const device_config *device, int lines);

enum
{
	KONAMI_PC=1, KONAMI_S, KONAMI_CC ,KONAMI_A, KONAMI_B, KONAMI_U, KONAMI_X, KONAMI_Y,
	KONAMI_DP
};

enum
{
	CPUINFO_PTR_KONAMI_SETLINES_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC
};

#define KONAMI_SETLINES_CALLBACK(name) void name(const device_config *device, int lines)

#define KONAMI_IRQ_LINE	0	/* IRQ line number */
#define KONAMI_FIRQ_LINE 1   /* FIRQ line number */

/* PUBLIC FUNCTIONS */
CPU_GET_INFO( konami );

CPU_DISASSEMBLE( konami );


INLINE void konami_configure_set_lines(const device_config *device, konami_set_lines_func func)
{
	device_set_info_fct(device, CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)func);
}

#endif /* __KONAMI_H__ */
