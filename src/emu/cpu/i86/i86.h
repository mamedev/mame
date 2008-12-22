/* ASG 971222 -- rewrote this interface */
#pragma once

#ifndef __I86INTF_H__
#define __I86INTF_H__

#include "cpuintrf.h"

#define INPUT_LINE_TEST 20    /* PJB 03/05 */

enum
{
	I8086_IP,
	I8086_AX,
	I8086_CX,
	I8086_DX,
	I8086_BX,
	I8086_SP,
	I8086_BP,
	I8086_SI,
	I8086_DI,
	I8086_AL,
	I8086_CL,
	I8086_DL,
	I8086_BL,
	I8086_AH,
	I8086_CH,
	I8086_DH,
	I8086_BH,
	I8086_FLAGS,
	I8086_ES,
	I8086_CS,
	I8086_SS,
	I8086_DS,
	I8086_VECTOR,

	I8086_GENPC = REG_GENPC,
	I8086_GENSP = REG_GENSP,
	I8086_GENPCBASE = REG_GENPCBASE
};

/* Public functions */
CPU_GET_INFO( i8086 );
#define CPU_I8086 CPU_GET_INFO_NAME( i8086 )

CPU_GET_INFO( i8088 );
#define CPU_I8088 CPU_GET_INFO_NAME( i8088 )

CPU_GET_INFO( i80186 );
#define CPU_I80186 CPU_GET_INFO_NAME( i80186 )

CPU_GET_INFO( i80188 );
#define CPU_I80188 CPU_GET_INFO_NAME( i80188 )

#endif /* __I86INTF_H__ */
