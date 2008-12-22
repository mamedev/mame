#ifndef __I8085_H__
#define __I8085_H__

#include "cpuintrf.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	I8085_PC, I8085_SP, I8085_AF, I8085_BC, I8085_DE, I8085_HL,
	I8085_A, I8085_B, I8085_C, I8085_D, I8085_E, I8085_H, I8085_L,
	I8085_STATUS, I8085_SOD, I8085_SID, I8085_INTE,
	I8085_HALT, I8085_IM,

	I8085_GENPC = REG_GENPC,
	I8085_GENSP = REG_GENSP,
	I8085_GENPCBASE = REG_GENPCBASE
};


#define I8085_INTR_LINE     0
#define I8085_RST55_LINE	1
#define I8085_RST65_LINE	2
#define I8085_RST75_LINE	3



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*i8085_sod_func)(const device_config *device, int state);
typedef int (*i8085_sid_func)(const device_config *device);
typedef void (*i8085_inte_func)(const device_config *device, int state);
typedef void (*i8085_status_func)(const device_config *device, UINT8 status);

typedef struct _i8085_config i8085_config;
struct _i8085_config
{
	i8085_inte_func		inte;				/* INTE changed callback */
	i8085_status_func	status;				/* STATUS changed callback */
	i8085_sod_func		sod;				/* SOD changed callback (8085A only) */
	i8085_sid_func		sid;				/* SID changed callback (8085A only) */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

CPU_GET_INFO( i8080 );
#define CPU_8080 CPU_GET_INFO_NAME( i8080 )

CPU_GET_INFO( i8085 );
#define CPU_8085A CPU_GET_INFO_NAME( i8085 )

CPU_DISASSEMBLE( i8085 );

#define i8085_set_sid(cpu, sid)		cpu_set_reg(cpu, I8085_SID, sid)

#endif
