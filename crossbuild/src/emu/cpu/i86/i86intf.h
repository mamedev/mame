/* ASG 971222 -- rewrote this interface */
#ifndef __I86INTRF_H_
#define __I86INTRF_H_

#include "cpuintrf.h"

enum
{
	I8086_PC=0,
	I8086_IP,
	I8086_AX,
	I8086_CX,
	I8086_DX,
	I8086_BX,
	I8086_SP,
	I8086_BP,
	I8086_SI,
	I8086_DI,
	I8086_FLAGS,
	I8086_ES,
	I8086_CS,
	I8086_SS,
	I8086_DS,
	I8086_VECTOR
};

/* Public functions */
void i8086_get_info(UINT32 state, cpuinfo *info);

#endif
