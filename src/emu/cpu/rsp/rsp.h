/***************************************************************************

    rsp.h

    Interface file for the universal machine language-based
    Reality Signal Processor (RSP) emulator.

    Copyright the MESS team
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __RSP_H__
#define __RSP_H__



/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
    RSP_PC = 1,
    RSP_R0,
    RSP_R1,
    RSP_R2,
    RSP_R3,
    RSP_R4,
    RSP_R5,
    RSP_R6,
    RSP_R7,
    RSP_R8,
    RSP_R9,
    RSP_R10,
    RSP_R11,
    RSP_R12,
    RSP_R13,
    RSP_R14,
    RSP_R15,
    RSP_R16,
    RSP_R17,
    RSP_R18,
    RSP_R19,
    RSP_R20,
    RSP_R21,
    RSP_R22,
    RSP_R23,
    RSP_R24,
    RSP_R25,
    RSP_R26,
    RSP_R27,
    RSP_R28,
    RSP_R29,
    RSP_R30,
    RSP_R31,
    RSP_SR,
    RSP_NEXTPC,
    RSP_STEPCNT,
};



/***************************************************************************
    STRUCTURES
***************************************************************************/

typedef void (*rsp_set_status_func)(running_device *device, UINT32 status);

typedef struct _rsp_config rsp_config;
struct _rsp_config
{
	read32_device_func dp_reg_r;
	write32_device_func dp_reg_w;
	read32_device_func sp_reg_r;
	write32_device_func sp_reg_w;
	rsp_set_status_func sp_set_status;
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void rspdrc_flush_drc_cache(running_device *device);
void rspdrc_set_options(running_device *device, UINT32 options);
void rspdrc_add_imem(running_device *device, void *base);
void rspdrc_add_dmem(running_device *device, void *base);

/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define REG_LO			32
#define REG_HI			33

#define RSREG			((op >> 21) & 31)
#define RTREG			((op >> 16) & 31)
#define RDREG			((op >> 11) & 31)
#define SHIFT			((op >> 6) & 31)

#define RSVAL			(rsp->r[RSREG])
#define RTVAL			(rsp->r[RTREG])
#define RDVAL			(rsp->r[RDREG])

#define FRREG			((op >> 21) & 31)
#define FTREG			((op >> 16) & 31)
#define FSREG			((op >> 11) & 31)
#define FDREG			((op >> 6) & 31)

#define IS_SINGLE(o)	(((o) & (1 << 21)) == 0)
#define IS_DOUBLE(o)	(((o) & (1 << 21)) != 0)
#define IS_FLOAT(o) 	(((o) & (1 << 23)) == 0)
#define IS_INTEGRAL(o)	(((o) & (1 << 23)) != 0)

#define SIMMVAL			((INT16)op)
#define UIMMVAL			((UINT16)op)
#define LIMMVAL			(op & 0x03ffffff)

#define RSP_STATUS_HALT          0x0001
#define RSP_STATUS_BROKE         0x0002
#define RSP_STATUS_DMABUSY       0x0004
#define RSP_STATUS_DMAFULL       0x0008
#define RSP_STATUS_IOFULL        0x0010
#define RSP_STATUS_SSTEP         0x0020
#define RSP_STATUS_INTR_BREAK    0x0040
#define RSP_STATUS_SIGNAL0       0x0080
#define RSP_STATUS_SIGNAL1       0x0100
#define RSP_STATUS_SIGNAL2       0x0200
#define RSP_STATUS_SIGNAL3       0x0400
#define RSP_STATUS_SIGNAL4       0x0800
#define RSP_STATUS_SIGNAL5       0x1000
#define RSP_STATUS_SIGNAL6       0x2000
#define RSP_STATUS_SIGNAL7       0x4000

#define RSPDRC_STRICT_VERIFY	0x0001			/* verify all instructions */

typedef struct
{
	union
	{
		UINT8 b[16];
		UINT16 s[8];
		UINT32 l[4];
		UINT64 d[2];
	};
} VECTOR_REG;

typedef struct
{
	union
	{
		INT64 l;
#ifdef LSB_FIRST
		struct
		{
			INT16 z;
			INT16 low;
			INT16 mid;
			INT16 high;
		} h;
		struct
		{
			INT32 zl;
			INT32 mh;
		} w;
#else
		struct
		{
			INT16 high;
			INT16 mid;
			INT16 low;
			INT16 z;
		} h;
		struct
		{
			INT32 mh;
			INT32 zl;
		} w;
#endif
	};
} ACCUMULATOR;

typedef struct _rspimp_state rspimp_state;
typedef struct _rsp_state rsp_state;
struct _rsp_state
{
	const rsp_config *config;
	FILE *exec_output;

	UINT32 pc;
	UINT32 r[35];
	VECTOR_REG v[32];
	UINT16 flag[4];
	UINT32 sr;
    UINT32 step_count;

	ACCUMULATOR accum[8];
	INT32 square_root_res;
	INT32 square_root_high;
	INT32 reciprocal_res;
	INT32 reciprocal_high;

	UINT32 ppc;
	UINT32 nextpc;

	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *program;
	int icount;

	rspimp_state* impstate;
};

DECLARE_LEGACY_CPU_DEVICE(RSP, rsp);

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

#endif /* __RSP_H__ */
