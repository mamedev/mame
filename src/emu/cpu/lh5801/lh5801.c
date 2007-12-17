/*****************************************************************************
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 * based on info found on an artikel for the tandy trs80 pc2
 *
 *****************************************************************************/
#include "cpuintrf.h"
#include "debugger.h"

#include "lh5801.h"

//typedef int bool;

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif

enum {
	LH5801_T=1,
	LH5801_P,
	LH5801_S,
	LH5801_U,
	LH5801_X,
	LH5801_Y,
	LH5801_A,

	LH5801_TM,
	LH5801_IN,
	LH5801_BF,
	LH5801_PU,
	LH5801_PV,
	LH5801_DP,
	LH5801_IRQ_STATE
};

typedef struct
{
	const LH5801_CONFIG *config;

	PAIR s, p, u, x, y;
	int tm; //9 bit

	UINT8 t, a;

	int bf, dp, pu, pv;

	UINT16 oldpc;

	int irq_state;

	int idle;
} LH5801_Regs;

static int lh5801_icount = 0;

static LH5801_Regs lh5801= { 0 };

#define P lh5801.p.w.l
#define S lh5801.s.w.l
#define U lh5801.u.w.l
#define UL lh5801.u.b.l
#define UH lh5801.u.b.h
#define X lh5801.x.w.l
#define XL lh5801.x.b.l
#define XH lh5801.x.b.h
#define Y lh5801.y.w.l
#define YL lh5801.y.b.l
#define YH lh5801.y.b.h

#define C 0x01
#define IE 0x02
#define Z 0x04
#define V 0x08
#define H 0x10

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "5801tbl.c"

static void lh5801_init(int cpu, int clock, const void *config, int (*irqcallback)(int))
{
	memset(&lh5801, 0, sizeof(lh5801));
	lh5801.config = (const LH5801_CONFIG *) config;
}

static void lh5801_reset(void)
{
	P = (program_read_byte(0xfffe)<<8) | program_read_byte(0xffff);

	change_pc(P);

	lh5801.idle=0;
}

static void lh5801_get_context (void *dst)
{
	if( dst )
		*(LH5801_Regs*)dst = lh5801;
}

static void lh5801_set_context (void *src)
{
	if( src )
	{
		lh5801 = *(LH5801_Regs*)src;
		change_pc(P);
	}
}

static int lh5801_execute(int cycles)
{
	lh5801_icount = cycles;

	change_pc(P);

	if (lh5801.idle) {
		lh5801_icount=0;
	} else {
		do
		{
			lh5801.oldpc = P;

			CALL_MAME_DEBUG;
			lh5801_instruction();

		} while (lh5801_icount > 0);
	}

	return cycles - lh5801_icount;
}

static void set_irq_line(int irqline, int state)
{
	lh5801.idle=0;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void lh5801_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE:					set_irq_line(0, info->i);				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + LH5801_P:			P = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + LH5801_S:			S = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_U:			U = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_T:			lh5801.t = info->i;						break;
		case CPUINFO_INT_REGISTER + LH5801_TM:			lh5801.tm = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_BF:			lh5801.bf = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_PV:			lh5801.pv = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_PU:			lh5801.pu = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_DP:			lh5801.dp = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void lh5801_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(lh5801);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 19;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = lh5801.irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = lh5801.oldpc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + LH5801_P:			info->i = P;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + LH5801_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + LH5801_U:			info->i = U;							break;
		case CPUINFO_INT_REGISTER + LH5801_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + LH5801_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + LH5801_T:			info->i = lh5801.t;						break;
		case CPUINFO_INT_REGISTER + LH5801_TM:			info->i = lh5801.tm;					break;
		case CPUINFO_INT_REGISTER + LH5801_IN:			info->i = lh5801.config->in();			break;
		case CPUINFO_INT_REGISTER + LH5801_BF:			info->i = lh5801.bf;					break;
		case CPUINFO_INT_REGISTER + LH5801_PV:			info->i = lh5801.pv;					break;
		case CPUINFO_INT_REGISTER + LH5801_PU:			info->i = lh5801.pu;					break;
		case CPUINFO_INT_REGISTER + LH5801_DP:			info->i = lh5801.dp;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = lh5801_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = lh5801_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = lh5801_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = lh5801_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = lh5801_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = NULL;						break;
		case CPUINFO_PTR_EXECUTE:						info->execute = lh5801_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = lh5801_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &lh5801_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s = cpuintrf_temp_str(), "LH5801"); break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s = cpuintrf_temp_str(), "LH5801"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0alpha"); break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__); break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Copyright (c) 2000 Peter Trauner, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s = cpuintrf_temp_str(), "%s%s%s%s%s%s%s%s",
				lh5801.t&0x80?"1":"0",
				lh5801.t&0x40?"1":"0",
				lh5801.t&0x20?"1":"0",
				lh5801.t&0x10?"H":".",
				lh5801.t&8?"V":".",
				lh5801.t&4?"Z":".",
				lh5801.t&2?"I":".",
				lh5801.t&1?"C":".");
			break;

		case CPUINFO_STR_REGISTER + LH5801_P:			sprintf(info->s = cpuintrf_temp_str(), "P:%04X", lh5801.p.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_S:			sprintf(info->s = cpuintrf_temp_str(), "S:%04X", lh5801.s.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_U:			sprintf(info->s = cpuintrf_temp_str(), "U:%04X", lh5801.u.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_X:			sprintf(info->s = cpuintrf_temp_str(), "X:%04X", lh5801.x.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_Y:			sprintf(info->s = cpuintrf_temp_str(), "Y:%04X", lh5801.y.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_T:			sprintf(info->s = cpuintrf_temp_str(), "T:%02X", lh5801.t); break;
		case CPUINFO_STR_REGISTER + LH5801_A:			sprintf(info->s = cpuintrf_temp_str(), "A:%02X", lh5801.a); break;
		case CPUINFO_STR_REGISTER + LH5801_TM:			sprintf(info->s = cpuintrf_temp_str(), "TM:%03X", lh5801.tm); break;
		case CPUINFO_STR_REGISTER + LH5801_IN:			sprintf(info->s = cpuintrf_temp_str(), "IN:%02X", lh5801.config->in()); break;
		case CPUINFO_STR_REGISTER + LH5801_PV:			sprintf(info->s = cpuintrf_temp_str(), "PV:%04X", lh5801.pv); break;
		case CPUINFO_STR_REGISTER + LH5801_PU:			sprintf(info->s = cpuintrf_temp_str(), "PU:%04X", lh5801.pu); break;
		case CPUINFO_STR_REGISTER + LH5801_BF:			sprintf(info->s = cpuintrf_temp_str(), "BF:%04X", lh5801.bf); break;
		case CPUINFO_STR_REGISTER + LH5801_DP:			sprintf(info->s = cpuintrf_temp_str(), "DP:%04X", lh5801.dp); break;
	}
}
