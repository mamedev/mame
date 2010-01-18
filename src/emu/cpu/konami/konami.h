/*** konami: Portable Konami cpu emulator ******************************************/

#pragma once

#ifndef __KONAMI_H__
#define __KONAMI_H__


typedef void (*konami_set_lines_func)(running_device *device, int lines);

enum
{
	KONAMI_PC=1, KONAMI_S, KONAMI_CC ,KONAMI_A, KONAMI_B, KONAMI_U, KONAMI_X, KONAMI_Y,
	KONAMI_DP
};

#define KONAMI_SETLINES_CALLBACK(name) void name(running_device *device, int lines)

#define KONAMI_IRQ_LINE	0	/* IRQ line number */
#define KONAMI_FIRQ_LINE 1   /* FIRQ line number */

/* PUBLIC FUNCTIONS */
CPU_GET_INFO( konami );
#define CPU_KONAMI CPU_GET_INFO_NAME( konami )

CPU_DISASSEMBLE( konami );

void konami_configure_set_lines(running_device *device, konami_set_lines_func func);


#endif /* __KONAMI_H__ */
