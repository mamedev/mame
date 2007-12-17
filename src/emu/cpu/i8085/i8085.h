#ifndef I8085_H
#define I8085_H

#include "cpuintrf.h"

enum {
	I8085_PC=1, I8085_SP, I8085_AF ,I8085_BC, I8085_DE, I8085_HL,
	I8085_HALT, I8085_IM, I8085_IREQ, I8085_ISRV, I8085_VECTOR,
	I8085_TRAP_STATE, I8085_INTR_STATE,
	I8085_RST55_STATE, I8085_RST65_STATE, I8085_RST75_STATE};

enum
{
	CPUINFO_PTR_I8085_SOD_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_INT_I8085_SID = CPUINFO_INT_CPU_SPECIFIC
};

#define I8085_INTR_LINE     0
#define I8085_RST55_LINE	1
#define I8085_RST65_LINE	2
#define I8085_RST75_LINE	3

void i8085_get_info(UINT32 state, cpuinfo *info);

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

#define I8080_REG_LAYOUT \
{	CPU_8080, \
	I8080_AF,I8080_BC,I8080_DE,I8080_HL,I8080_SP,I8080_PC, DBG_ROW, \
	I8080_HALT,I8080_IREQ,I8080_ISRV,I8080_VECTOR, \
    DBG_END }

#define I8080_INTR_LINE         I8085_INTR_LINE

void i8080_get_info(UINT32 state, cpuinfo *info);
#endif

#ifdef	MAME_DEBUG
offs_t i8085_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif
