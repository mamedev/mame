/*****************************************************************************
 *
 *   saturn.c
 *   portable saturn emulator interface
 *   (hp calculators)
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
 *****************************************************************************/
#include "cpuintrf.h"
#include "debugger.h"

#include "saturn.h"
#include "sat.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define A 5
#define B 6
#define C 7
#define D 8

typedef int SaturnAdr; // 20 bit
typedef UINT8 SaturnNib; // 4 bit
typedef short SaturnX; // 12 bit
typedef INT64 SaturnM; // 48 bit

typedef union {
	UINT8 b[8];
	UINT16 w[4];
	UINT32 d[2];
	UINT64 q;
} Saturn64;

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif


/****************************************************************************
 * The 6502 registers.
 ****************************************************************************/
typedef struct
{
	SATURN_CONFIG *config;
	Saturn64 reg[9]; //r0,r1,r2,r3,r4,a,b,c,d;

	SaturnAdr d[2], pc, oldpc, rstk[8]; // 20 bit addresses

	int stackpointer; // this is only for debugger stepover support!

	SaturnNib p; // 4 bit pointer

	UINT16 in;
	int out; // 12
	int carry, decimal;
	UINT16 st; // status 16 bit
#define XM 1
#define SR 2
#define SB 4
#define MP 8
	int hst; // hardware status 4 bit
		/*  XM external Modules missing
            SR Service Request
            SB Sticky bit
            MP Module Pulled */

	int irq_state;

	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
}	Saturn_Regs;

static int saturn_ICount = 0;

static Saturn_Regs saturn;

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "satops.c"
#include "sattable.c"

/*****************************************************************************
 *
 *      Saturn CPU interface functions
 *
 *****************************************************************************/

static void saturn_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	saturn.config = (SATURN_CONFIG *) config;
}

static void saturn_reset(void)
{
	saturn.stackpointer=0;
	saturn.pc=0;
	change_pc(saturn.pc);
}

static void saturn_get_context (void *dst)
{
	if( dst )
		*(Saturn_Regs*)dst = saturn;
}

static void saturn_set_context (void *src)
{
	if( src )
	{
		saturn = *(Saturn_Regs*)src;
		change_pc(saturn.pc);
	}
}



INLINE void saturn_take_irq(void)
{
	{
		saturn_ICount -= 7;

		saturn_push(saturn.pc);
		saturn.pc=IRQ_ADDRESS;

		LOG(("Saturn#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), saturn.pc));
		/* call back the cpuintrf to let it clear the line */
		if (saturn.irq_callback) (*saturn.irq_callback)(0);
		change_pc(saturn.pc);
	}
	saturn.pending_irq = 0;
}

int saturn_execute(int cycles)
{
	saturn_ICount = cycles;

	change_pc(saturn.pc);

	do
	{
		saturn.oldpc = saturn.pc;

		CALL_MAME_DEBUG;

		/* if an irq is pending, take it now */
		if( saturn.pending_irq )
			saturn_take_irq();

		saturn_instruction();

		/* check if the I flag was just reset (interrupts enabled) */
		if( saturn.after_cli )
		{
			LOG(("M6502#%d after_cli was >0", cpu_getactivecpu()));
			saturn.after_cli = 0;
			if (saturn.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				saturn.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( saturn.pending_irq )
			saturn_take_irq();

	} while (saturn_ICount > 0);

	return cycles - saturn_ICount;
}

void saturn_set_nmi_line(int state)
{
	if (saturn.nmi_state == state) return;
	saturn.nmi_state = state;
	if( state != CLEAR_LINE )
	{
		LOG(( "M6502#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
		saturn_ICount -= 7;
		saturn_push(saturn.pc);
		saturn.pc=IRQ_ADDRESS;

		LOG(("M6502#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PC));
		change_pc(saturn.pc);
	}
}

void saturn_set_irq_line(int irqline, int state)
{
	saturn.irq_state = state;
	if( state != CLEAR_LINE )
	{
		LOG(( "M6502#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
		saturn.pending_irq = 1;
	}
}

void saturn_set_irq_callback(int (*callback)(int))
{
	saturn.irq_callback = callback;
}

#if 0
static void saturn_state_save(void *file)
{
	int cpu = cpu_getactivecpu();
	state_save_UINT16(file,"m6502",cpu,"PC",&m6502.pc.w.l,2);
	state_save_UINT16(file,"m6502",cpu,"SP",&m6502.sp.w.l,2);
	state_save_UINT8(file,"m6502",cpu,"P",&m6502.p,1);
	state_save_UINT8(file,"m6502",cpu,"A",&m6502.a,1);
	state_save_UINT8(file,"m6502",cpu,"X",&m6502.x,1);
	state_save_UINT8(file,"m6502",cpu,"Y",&m6502.y,1);
	state_save_UINT8(file,"m6502",cpu,"PENDING",&m6502.pending_irq,1);
	state_save_UINT8(file,"m6502",cpu,"AFTER_CLI",&m6502.after_cli,1);
	state_save_UINT8(file,"m6502",cpu,"NMI_STATE",&m6502.nmi_state,1);
	state_save_UINT8(file,"m6502",cpu,"IRQ_STATE",&m6502.irq_state,1);
	state_save_UINT8(file,"m6502",cpu,"SO_STATE",&m6502.so_state,1);
}

static void saturn_state_load(void *file)
{
	int cpu = cpu_getactivecpu();
	state_load_UINT16(file,"m6502",cpu,"PC",&m6502.pc.w.l,2);
	state_load_UINT16(file,"m6502",cpu,"SP",&m6502.sp.w.l,2);
	state_load_UINT8(file,"m6502",cpu,"P",&m6502.p,1);
	state_load_UINT8(file,"m6502",cpu,"A",&m6502.a,1);
	state_load_UINT8(file,"m6502",cpu,"X",&m6502.x,1);
	state_load_UINT8(file,"m6502",cpu,"Y",&m6502.y,1);
	state_load_UINT8(file,"m6502",cpu,"PENDING",&m6502.pending_irq,1);
	state_load_UINT8(file,"m6502",cpu,"AFTER_CLI",&m6502.after_cli,1);
	state_load_UINT8(file,"m6502",cpu,"NMI_STATE",&m6502.nmi_state,1);
	state_load_UINT8(file,"m6502",cpu,"IRQ_STATE",&m6502.irq_state,1);
	state_load_UINT8(file,"m6502",cpu,"SO_STATE",&m6502.so_state,1);
}
#endif

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void saturn_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SATURN_NMI_STATE:	saturn.nmi_state = info->i;	break;
		case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_STATE:	saturn.irq_state = info->i;	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			saturn.pc = info->i; change_pc(saturn.pc);				break;
		case CPUINFO_INT_SP:							saturn.stackpointer = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			saturn.d[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			saturn.d[1] = info->i;							break;
#if 0
		case CPUINFO_INT_REGISTER + SATURN_A:			saturn.reg[A] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_B:			saturn.reg[B] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_C:			saturn.reg[C] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_D:			saturn.reg[D] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_R0:			saturn.reg[R0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_R1:			saturn.reg[R1] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_R2:			saturn.reg[R2] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_R3:			saturn.reg[R3] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_R4:			saturn.reg[R4] = info->i;							break;
#endif
		case CPUINFO_INT_REGISTER + SATURN_P:			saturn.p = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_IN:			saturn.in = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			saturn.out = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		saturn.carry = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			saturn.st = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			saturn.hst = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		saturn.rstk[0] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		saturn.rstk[1] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		saturn.rstk[2] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		saturn.rstk[3] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		saturn.rstk[4] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		saturn.rstk[5] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		saturn.rstk[6] = info->i;							break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		saturn.rstk[7] = info->i;							break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void saturn_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(saturn);				break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20; /* 20 nibbles max */		break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 21;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SATURN_NMI_STATE:	info->i = saturn.nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + SATURN_IRQ_STATE:	info->i = saturn.irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = saturn.oldpc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SATURN_PC:			info->i = saturn.pc;					break;
		case CPUINFO_INT_SP:							info->i = saturn.stackpointer;			break;
		case CPUINFO_INT_REGISTER + SATURN_D0:			info->i = saturn.d[0];					break;
		case CPUINFO_INT_REGISTER + SATURN_D1:			info->i = saturn.d[1];					break;
#if 0
		case CPUINFO_INT_REGISTER + SATURN_A:			info->i = saturn.reg[A];				break;
		case CPUINFO_INT_REGISTER + SATURN_B:			info->i = saturn.reg[B];				break;
		case CPUINFO_INT_REGISTER + SATURN_C:			info->i = saturn.reg[C];				break;
		case CPUINFO_INT_REGISTER + SATURN_D:			info->i = saturn.reg[D];				break;
		case CPUINFO_INT_REGISTER + SATURN_R0:			info->i = saturn.reg[R0];				break;
		case CPUINFO_INT_REGISTER + SATURN_R1:			info->i = saturn.reg[R1];				break;
		case CPUINFO_INT_REGISTER + SATURN_R2:			info->i = saturn.reg[R2];				break;
		case CPUINFO_INT_REGISTER + SATURN_R3:			info->i = saturn.reg[R3];				break;
		case CPUINFO_INT_REGISTER + SATURN_R4:			info->i = saturn.reg[R4];				break;
#endif
		case CPUINFO_INT_REGISTER + SATURN_P:			info->i = saturn.p;						break;
		case CPUINFO_INT_REGISTER + SATURN_IN:			info->i = saturn.in;					break;
		case CPUINFO_INT_REGISTER + SATURN_OUT:			info->i = saturn.out;					break;
		case CPUINFO_INT_REGISTER + SATURN_CARRY:		info->i = saturn.carry;					break;
		case CPUINFO_INT_REGISTER + SATURN_ST:			info->i = saturn.st;					break;
		case CPUINFO_INT_REGISTER + SATURN_HST:			info->i = saturn.hst;					break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK0:		info->i = saturn.rstk[0];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK1:		info->i = saturn.rstk[1];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK2:		info->i = saturn.rstk[2];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK3:		info->i = saturn.rstk[3];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK4:		info->i = saturn.rstk[4];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK5:		info->i = saturn.rstk[5];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK6:		info->i = saturn.rstk[6];				break;
		case CPUINFO_INT_REGISTER + SATURN_RSTK7:		info->i = saturn.rstk[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = saturn_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = saturn_get_context;		break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = saturn_set_context;		break;
		case CPUINFO_PTR_INIT:							info->init = saturn_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = saturn_reset;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = saturn_execute;				break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;							break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = saturn_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &saturn_ICount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s = cpuintrf_temp_str(), "Saturn");	break;
		case CPUINFO_STR_CORE_FAMILY: 					strcpy(info->s = cpuintrf_temp_str(), "Saturn");	break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0alpha");	break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__);	break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Copyright (c) 2000 Peter Trauner, all rights reserved.");	break;

		case CPUINFO_STR_REGISTER + SATURN_PC:		sprintf(info->s = cpuintrf_temp_str(), "PC:   %.5x", saturn.pc);break;
		case CPUINFO_STR_REGISTER + SATURN_D0:		sprintf(info->s = cpuintrf_temp_str(), "D0:   %.5x", saturn.d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_D1:		sprintf(info->s = cpuintrf_temp_str(), "D1:   %.5x", saturn.d[1]);break;
		case CPUINFO_STR_REGISTER + SATURN_A:		sprintf(info->s = cpuintrf_temp_str(), "A: %.8x %.8x", saturn.reg[A].d[1], saturn.reg[A].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_B:		sprintf(info->s = cpuintrf_temp_str(), "B: %.8x %.8x", saturn.reg[B].d[1], saturn.reg[B].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_C:		sprintf(info->s = cpuintrf_temp_str(), "C: %.8x %.8x", saturn.reg[C].d[1], saturn.reg[C].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_D:		sprintf(info->s = cpuintrf_temp_str(), "D: %.8x %.8x", saturn.reg[D].d[1], saturn.reg[D].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_R0:		sprintf(info->s = cpuintrf_temp_str(), "R0:%.8x %.8x", saturn.reg[R0].d[1], saturn.reg[R0].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_R1:		sprintf(info->s = cpuintrf_temp_str(), "R1:%.8x %.8x", saturn.reg[R1].d[1], saturn.reg[R1].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_R2:		sprintf(info->s = cpuintrf_temp_str(), "R2:%.8x %.8x", saturn.reg[R2].d[1], saturn.reg[R2].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_R3:		sprintf(info->s = cpuintrf_temp_str(), "R3:%.8x %.8x", saturn.reg[R3].d[1], saturn.reg[R3].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_R4:		sprintf(info->s = cpuintrf_temp_str(), "R4:%.8x %.8x", saturn.reg[R4].d[1], saturn.reg[R4].d[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_P:		sprintf(info->s = cpuintrf_temp_str(), "P:%x", saturn.p);break;
		case CPUINFO_STR_REGISTER + SATURN_IN:		sprintf(info->s = cpuintrf_temp_str(), "IN:%.4x", saturn.in);break;
		case CPUINFO_STR_REGISTER + SATURN_OUT:		sprintf(info->s = cpuintrf_temp_str(), "OUT:%.3x", saturn.out);break;
		case CPUINFO_STR_REGISTER + SATURN_CARRY:	sprintf(info->s = cpuintrf_temp_str(), "Carry: %d", saturn.carry);break;
		case CPUINFO_STR_REGISTER + SATURN_ST:		sprintf(info->s = cpuintrf_temp_str(), "ST:%.4x", saturn.st);break;
		case CPUINFO_STR_REGISTER + SATURN_HST:		sprintf(info->s = cpuintrf_temp_str(), "HST:%x", saturn.hst);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK0:	sprintf(info->s = cpuintrf_temp_str(), "RSTK0:%.5x", saturn.rstk[0]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK1:	sprintf(info->s = cpuintrf_temp_str(), "RSTK1:%.5x", saturn.rstk[1]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK2:	sprintf(info->s = cpuintrf_temp_str(), "RSTK2:%.5x", saturn.rstk[2]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK3:	sprintf(info->s = cpuintrf_temp_str(), "RSTK3:%.5x", saturn.rstk[3]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK4:	sprintf(info->s = cpuintrf_temp_str(), "RSTK4:%.5x", saturn.rstk[4]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK5:	sprintf(info->s = cpuintrf_temp_str(), "RSTK5:%.5x", saturn.rstk[5]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK6:	sprintf(info->s = cpuintrf_temp_str(), "RSTK6:%.5x", saturn.rstk[6]);break;
		case CPUINFO_STR_REGISTER + SATURN_RSTK7:	sprintf(info->s = cpuintrf_temp_str(), "RSTK7:%.5x", saturn.rstk[7]);break;
		case CPUINFO_STR_REGISTER + SATURN_IRQ_STATE:	sprintf(info->s = cpuintrf_temp_str(), "IRQ:%.4x", saturn.pending_irq);break;
		case CPUINFO_STR_FLAGS:						sprintf(info->s = cpuintrf_temp_str(), "%c%c", saturn.decimal?'D':'.', saturn.carry ? 'C':'.'); break;
	}
}
