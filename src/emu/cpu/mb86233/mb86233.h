#pragma once

#ifndef __MB86233_H__
#define __MB86233_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB86233_PC=1,
	MB86233_A,
	MB86233_B,
	MB86233_D,
	MB86233_P,
	MB86233_REP,
	MB86233_SP,
	MB86233_EB,
	MB86233_SHIFT,
	MB86233_FLAGS,
	MB86233_R0,
	MB86233_R1,
	MB86233_R2,
	MB86233_R3,
	MB86233_R4,
	MB86233_R5,
	MB86233_R6,
	MB86233_R7,
	MB86233_R8,
	MB86233_R9,
	MB86233_R10,
	MB86233_R11,
	MB86233_R12,
	MB86233_R13,
	MB86233_R14,
	MB86233_R15
};

/***************************************************************************
    STRUCTURES
***************************************************************************/

typedef int (*mb86233_fifo_read_func)(device_t *device, UINT32 *data);
typedef void (*mb86233_fifo_write_func)(device_t *device, UINT32 data);

struct mb86233_cpu_core
{
	mb86233_fifo_read_func fifo_read_cb;
	mb86233_fifo_write_func fifo_write_cb;
	const char *tablergn;
};

DECLARE_LEGACY_CPU_DEVICE(MB86233, mb86233);

#endif /* __MB86233_H__ */
