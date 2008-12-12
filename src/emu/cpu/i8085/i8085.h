#ifndef I8085_H
#define I8085_H

#include "cpuintrf.h"

enum
{
	I8085_PC=1, I8085_SP, I8085_AF ,I8085_BC, I8085_DE, I8085_HL,
	I8085_HALT, I8085_IM, I8085_IREQ, I8085_ISRV, I8085_VECTOR,
	I8085_TRAP_STATE, I8085_INTR_STATE,
	I8085_RST55_STATE, I8085_RST65_STATE, I8085_RST75_STATE, I8085_STATUS
};

enum
{
	CPUINFO_PTR_I8085_SOD_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_I8085_SID_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC + 1,
	CPUINFO_PTR_I8085_INTE_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC + 2,
	CPUINFO_PTR_I8085_STATUS_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC + 3,
	CPUINFO_INT_I8085_SID = CPUINFO_INT_CPU_SPECIFIC
};

typedef void (*i8085_sod_func)(const device_config *device, int state);
typedef int (*i8085_sid_func)(const device_config *device);
typedef void (*i8085_inte_func)(const device_config *device, int state);
typedef void (*i8085_status_func)(const device_config *device, UINT8 status);


#define I8085_INTR_LINE     0
#define I8085_RST55_LINE	1
#define I8085_RST65_LINE	2
#define I8085_RST75_LINE	3

CPU_GET_INFO( i8085 );

/**************************************************************************
 * I8080 section
 **************************************************************************/
#if (HAS_8080)
#define I8080_PC                I8085_PC
#define I8080_SP				I8085_SP
#define I8080_BC				I8085_BC
#define I8080_DE				I8085_DE
#define I8080_HL				I8085_HL
#define I8080_AF				I8085_AF
#define I8080_HALT				I8085_HALT
#define I8080_IREQ				I8085_IREQ
#define I8080_ISRV				I8085_ISRV
#define I8080_VECTOR			I8085_VECTOR
#define I8080_TRAP_STATE		I8085_TRAP_STATE
#define I8080_INTR_STATE		I8085_INTR_STATE
#define I8080_STATUS			I8085_STATUS
#define I8080_REG_LAYOUT \
{	CPU_8080, \
	I8080_AF,I8080_BC,I8080_DE,I8080_HL,I8080_SP,I8080_PC, DBG_ROW, \
	I8080_HALT,I8080_IREQ,I8080_ISRV,I8080_VECTOR, I8080_STATUS, \
    DBG_END }

#define I8080_INTR_LINE         I8085_INTR_LINE

CPU_GET_INFO( i8080 );
#endif

CPU_DISASSEMBLE( i8085 );

INLINE void i8085_set_sod_callback(const device_config *device, i8085_sod_func callback)
{
	cpu_set_info_fct(device, CPUINFO_PTR_I8085_SOD_CALLBACK, (genf *)callback);
}

INLINE void i8085_set_sid_callback(const device_config *device, i8085_sid_func callback)
{
	cpu_set_info_fct(device, CPUINFO_PTR_I8085_SID_CALLBACK, (genf *)callback);
}

INLINE void i8085_set_inte_callback(const device_config *device, i8085_inte_func callback)
{
	cpu_set_info_fct(device, CPUINFO_PTR_I8085_INTE_CALLBACK, (genf *)callback);
}

INLINE void i8085_set_status_callback(const device_config *device, i8085_status_func callback)
{
	cpu_set_info_fct(device, CPUINFO_PTR_I8085_STATUS_CALLBACK, (genf *)callback);
}

INLINE void i8085_set_sid(const device_config *device, int sid)
{
	cpu_set_info_int(device, CPUINFO_INT_I8085_SID, sid);
}

#endif
