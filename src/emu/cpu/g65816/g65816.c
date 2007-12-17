/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.93

Copyright (c) 2000 Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/
/* ======================================================================== */
/* ================================= NOTES ================================ */
/* ======================================================================== */
/*

Changes:
    0.94 (2007-06-14):
            Zsolt Vasvari
            - Removed unneccessary checks from MVP and MVN

    0.93 (2003-07-05):
            Angelo Salese <lordkale@libero.it>
            - Fixed the BCD conversion when using the Decimal Flag in ADC and SBC.
            - Removed the two conversion tables for ADC and SBC as they aren't
              needed anymore.

    0.92 (2000-05-28):
            Lee Hammerton <lee-hammerton@hammerhead.ltd.uk>
            - Fixed debugger bug that caused D to be misrepresented.
            - Fixed MVN and MVP (they were reversed)

    0.91 (2000-05-22):
            Lee Hammerton <lee-hammerton@hammerhead.ltd.uk>
            - Fixed reset vector fetch to be little endian
            - Fixed disassembler call bug
            - Fixed C flag in SBC (should be inverted before operation)
            - Fixed JSR to stack PC-1 and RTS to pull PC and add 1

            Karl Stenerud <karl@higashiyama-unet.ocn.ne.jp>
            - Added correct timing for absolute indexed operations
            - SBC: fixed corruption of interim values

    0.90 (2000-05-17):
            Karl Stenerud <karl@higashiyama-unet.ocn.ne.jp>
            - first public release


Note on timings:
    - For instructions that write to memory (ASL, ASR, LSL, ROL, ROR, DEC,
      INC, STA, STZ), the absolute indexed addressing mode takes 1 extra
      cycle to complete.
    - The spec says fc (JMP axi) is 6 cyles, but elsewhere says 8 cycles
      (which is what it should be)


TODO general:
    - WAI will not stop if RDY is held high.

    - RDY internally held low when WAI executed and returned to hi when RES,
      ABORT, NMI, or IRQ asserted.

    - ABORT will terminate WAI instruction but wil not restart the processor

    - If interrupt occurs after ABORT of WAI, processor returns to WAI
      instruction.

    - Add one cycle when indexing across page boundary and E=1 except for STA
      and STZ instructions.

    - Add 1 cycle if branch is taken. In Emulation (E= 1 ) mode only --add 1
      cycle if the branch is taken and crosses a page boundary.

    - Add 1 cycle in Emulation mode (E=1) for (dir),y; abs,x; and abs,y
      addressing modes.

*/
/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

#include "g65816cm.h"

/* Our CPU structure */
g65816i_cpu_struct g65816i_cpu = {0};

int g65816_ICount = 0;

/* Temporary Variables */
uint g65816i_source;
uint g65816i_destination;

static int g65816_readop(UINT32 offset, int size, UINT64 *value)
{
	*value = g65816_read_8_immediate(offset);

	return 1;
}

extern void (*g65816i_opcodes_M0X0[])(void);
extern uint g65816i_get_reg_M0X0(int regnum);
extern void g65816i_set_reg_M0X0(int regnum, uint val);
extern void g65816i_set_line_M0X0(int line, int state);
extern int  g65816i_execute_M0X0(int cycles);

extern void (*g65816i_opcodes_M0X1[])(void);
extern uint g65816i_get_reg_M0X1(int regnum);
extern void g65816i_set_reg_M0X1(int regnum, uint val);
extern void g65816i_set_line_M0X1(int line, int state);
extern int  g65816i_execute_M0X1(int cycles);

extern void (*g65816i_opcodes_M1X0[])(void);
extern uint g65816i_get_reg_M1X0(int regnum);
extern void g65816i_set_reg_M1X0(int regnum, uint val);
extern void g65816i_set_line_M1X0(int line, int state);
extern int  g65816i_execute_M1X0(int cycles);

extern void (*g65816i_opcodes_M1X1[])(void);
extern uint g65816i_get_reg_M1X1(int regnum);
extern void g65816i_set_reg_M1X1(int regnum, uint val);
extern void g65816i_set_line_M1X1(int line, int state);
extern int  g65816i_execute_M1X1(int cycles);

extern void (*g65816i_opcodes_E[])(void);
extern uint g65816i_get_reg_E(int regnum);
extern void g65816i_set_reg_E(int regnum, uint val);
extern void g65816i_set_line_E(int line, int state);
extern int  g65816i_execute_E(int cycles);

void (**g65816i_opcodes[5])(void) =
{
	g65816i_opcodes_M0X0,
	g65816i_opcodes_M0X1,
	g65816i_opcodes_M1X0,
	g65816i_opcodes_M1X1,
	g65816i_opcodes_E
};

uint (*g65816i_get_reg[5])(int regnum) =
{
	g65816i_get_reg_M0X0,
	g65816i_get_reg_M0X1,
	g65816i_get_reg_M1X0,
	g65816i_get_reg_M1X1,
	g65816i_get_reg_E
};

void (*g65816i_set_reg[5])(int regnum, uint val) =
{
	g65816i_set_reg_M0X0,
	g65816i_set_reg_M0X1,
	g65816i_set_reg_M1X0,
	g65816i_set_reg_M1X1,
	g65816i_set_reg_E
};

void (*g65816i_set_line[5])(int line, int state) =
{
	g65816i_set_line_M0X0,
	g65816i_set_line_M0X1,
	g65816i_set_line_M1X0,
	g65816i_set_line_M1X1,
	g65816i_set_line_E
};

int (*g65816i_execute[5])(int cycles) =
{
	g65816i_execute_M0X0,
	g65816i_execute_M0X1,
	g65816i_execute_M1X0,
	g65816i_execute_M1X1,
	g65816i_execute_E
};

/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */


static void g65816_reset(void)
{
		/* Start the CPU */
		CPU_STOPPED = 0;

		/* Put into emulation mode */
		REGISTER_D = 0;
		REGISTER_PB = 0;
		REGISTER_DB = 0;
		REGISTER_S = (REGISTER_S & 0xff) | 0x100;
		REGISTER_X &= 0xff;
		REGISTER_Y &= 0xff;
		if(!FLAG_M)
		{
			REGISTER_B = REGISTER_A & 0xff00;
			REGISTER_A &= 0xff;
		}
		FLAG_E = EFLAG_SET;
		FLAG_M = MFLAG_SET;
		FLAG_X = XFLAG_SET;

		/* Clear D and set I */
		FLAG_D = DFLAG_CLEAR;
		FLAG_I = IFLAG_SET;

		/* Clear all pending interrupts (should we really do this?) */
		LINE_IRQ = 0;
		LINE_NMI = 0;
		IRQ_DELAY = 0;

		/* Set the function tables to emulation mode */
		g65816i_set_execution_mode(EXECUTION_MODE_E);

		/* 6502 expects these, but its not in the 65816 spec */
		FLAG_Z = ZFLAG_CLEAR;
		REGISTER_S = 0x1ff;

		/* Fetch the reset vector */
		REGISTER_PC = g65816_read_8(VECTOR_RESET) | (g65816_read_8(VECTOR_RESET+1)<<8);
		g65816i_jumping(REGISTER_PB | REGISTER_PC);
}

/* Exit and clean up */
static void g65816_exit(void)
{
	/* nothing to do yet */
}

/* Execute some instructions */
static int g65816_execute(int cycles)
{
	return FTABLE_EXECUTE(cycles);
}


/* Get the current CPU context */
static void g65816_get_context(void *dst_context)
{
	if(dst_context)
		*(g65816i_cpu_struct*)dst_context = g65816i_cpu;
}

/* Set the current CPU context */
static void g65816_set_context(void *src_context)
{
	if(src_context)
	{
		g65816i_cpu = *(g65816i_cpu_struct*)src_context;
		g65816i_jumping(REGISTER_PB | REGISTER_PC);
	}
}

/* Get the current Program Counter */
static unsigned g65816_get_pc(void)
{
	return REGISTER_PB | REGISTER_PC;
}

/* Set the Program Counter */
static void g65816_set_pc(unsigned val)
{
	REGISTER_PC = MAKE_UINT_16(val);
	REGISTER_PB = (val >> 16) & 0xFF;
	g65816_jumping(REGISTER_PB | REGISTER_PC);
}

/* Get the current Stack Pointer */
static unsigned g65816_get_sp(void)
{
	return REGISTER_S;
}

/* Set the Stack Pointer */
static void g65816_set_sp(unsigned val)
{
	REGISTER_S = FLAG_E ? MAKE_UINT_8(val) | 0x100 : MAKE_UINT_16(val);
}

/* Get a register */
static unsigned g65816_get_reg(int regnum)
{
	/* Set the function tables to emulation mode if the FTABLE is NULL */
	if( FTABLE_GET_REG == NULL )
		g65816i_set_execution_mode(EXECUTION_MODE_E);

	return FTABLE_GET_REG(regnum);
}

/* Set a register */
static void g65816_set_reg(int regnum, unsigned value)
{
	FTABLE_SET_REG(regnum, value);
}

/* Set an interrupt line */
static void g65816_set_irq_line(int line, int state)
{
	FTABLE_SET_LINE(line, state);
}

/* Set the callback that is called when servicing an interrupt */
static void g65816_set_irq_callback(int (*callback)(int))
{
	INT_ACK = callback;
}


/* Disassemble an instruction */
#ifdef MAME_DEBUG
#include "g65816ds.h"

static offs_t g65816_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return g65816_disassemble(buffer, (pc & 0x00ffff), (pc & 0xff0000) >> 16, oprom, FLAG_M, FLAG_X);
}
#endif /* MAME_DEBUG */



static void g65816_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	g65816_set_irq_callback(irqcallback);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void g65816_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_IRQ:		g65816_set_irq_line(G65816_LINE_IRQ, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_NMI:		g65816_set_irq_line(G65816_LINE_NMI, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_ABORT:	g65816_set_irq_line(G65816_LINE_ABORT, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_SO:		g65816_set_irq_line(G65816_LINE_SO, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RDY:		g65816_set_irq_line(G65816_LINE_RDY, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RESET:	g65816_set_irq_line(G65816_LINE_RESET, info->i); break;

		case CPUINFO_INT_PC:							g65816_set_pc(info->i);					break;
		case CPUINFO_INT_SP:							g65816_set_sp(info->i);					break;

		case CPUINFO_INT_REGISTER + G65816_PC:			g65816_set_reg(G65816_PC, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_S:			g65816_set_reg(G65816_S, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_P:			g65816_set_reg(G65816_P, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_A:			g65816_set_reg(G65816_A, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_X:			g65816_set_reg(G65816_X, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_Y:			g65816_set_reg(G65816_Y, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_PB:			g65816_set_reg(G65816_PB, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_DB:			g65816_set_reg(G65816_DB, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_D:			g65816_set_reg(G65816_D, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_E:			g65816_set_reg(G65816_E, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_NMI_STATE:	g65816_set_reg(G65816_NMI_STATE, info->i); break;
		case CPUINFO_INT_REGISTER + G65816_IRQ_STATE:	g65816_set_reg(G65816_IRQ_STATE, info->i); break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_G65816_READVECTOR_CALLBACK:	READ_VECTOR = (read8_handler) info->f;	break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void g65816_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(g65816i_cpu);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */			break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + G65816_LINE_IRQ:		info->i = LINE_IRQ;					break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_NMI:		info->i = LINE_NMI;					break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_ABORT:	info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_SO:		info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RDY:		info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RESET:	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:							info->i = g65816_get_pc();				break;
		case CPUINFO_INT_SP:							info->i = g65816_get_sp();				break;

		case CPUINFO_INT_REGISTER + G65816_PC:			info->i = g65816_get_pc();				break;
		case CPUINFO_INT_REGISTER + G65816_S:			info->i = g65816_get_reg(G65816_S);		break;
		case CPUINFO_INT_REGISTER + G65816_P:			info->i = g65816_get_reg(G65816_P);		break;
		case CPUINFO_INT_REGISTER + G65816_A:			info->i = g65816_get_reg(G65816_A);		break;
		case CPUINFO_INT_REGISTER + G65816_X:			info->i = g65816_get_reg(G65816_X);		break;
		case CPUINFO_INT_REGISTER + G65816_Y:			info->i = g65816_get_reg(G65816_Y);		break;
		case CPUINFO_INT_REGISTER + G65816_PB:			info->i = g65816_get_reg(G65816_PB);	break;
		case CPUINFO_INT_REGISTER + G65816_DB:			info->i = g65816_get_reg(G65816_DB);	break;
		case CPUINFO_INT_REGISTER + G65816_D:			info->i = g65816_get_reg(G65816_D);		break;
		case CPUINFO_INT_REGISTER + G65816_E:			info->i = g65816_get_reg(G65816_E);		break;
		case CPUINFO_INT_REGISTER + G65816_NMI_STATE:	info->i = g65816_get_reg(G65816_NMI_STATE); break;
		case CPUINFO_INT_REGISTER + G65816_IRQ_STATE:	info->i = g65816_get_reg(G65816_IRQ_STATE); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = g65816_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = g65816_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = g65816_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = g65816_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = g65816_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = g65816_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = g65816_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = g65816_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &g65816_ICount;			break;
		case CPUINFO_PTR_G65816_READVECTOR_CALLBACK:	info->f = (genf *) READ_VECTOR;			break;

		case CPUINFO_PTR_READOP:						info->readop = g65816_readop;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "G65C816");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "6500");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.94");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2000 Karl Stenerud, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				g65816i_cpu.flag_n & NFLAG_SET ? 'N':'.',
				g65816i_cpu.flag_v & VFLAG_SET ? 'V':'.',
				g65816i_cpu.flag_m & MFLAG_SET ? 'M':'.',
				g65816i_cpu.flag_x & XFLAG_SET ? 'X':'.',
				g65816i_cpu.flag_d & DFLAG_SET ? 'D':'.',
				g65816i_cpu.flag_i & IFLAG_SET ? 'I':'.',
				g65816i_cpu.flag_z == 0        ? 'Z':'.',
				g65816i_cpu.flag_c & CFLAG_SET ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + G65816_PC:			sprintf(info->s, "PC:%06X", g65816_get_pc()); break;
		case CPUINFO_STR_REGISTER + G65816_PB:			sprintf(info->s, "PB:%02X", g65816i_cpu.pb>>16); break;
		case CPUINFO_STR_REGISTER + G65816_DB:			sprintf(info->s, "DB:%02X", g65816i_cpu.db>>16); break;
		case CPUINFO_STR_REGISTER + G65816_D:			sprintf(info->s, "D:%04X", g65816i_cpu.d); break;
		case CPUINFO_STR_REGISTER + G65816_S:			sprintf(info->s, "S:%04X", g65816i_cpu.s); break;
		case CPUINFO_STR_REGISTER + G65816_P:			sprintf(info->s, "P:%02X",
																 (g65816i_cpu.flag_n&0x80)		|
																((g65816i_cpu.flag_v>>1)&0x40)	|
																g65816i_cpu.flag_m				|
																g65816i_cpu.flag_x				|
																g65816i_cpu.flag_d				|
																g65816i_cpu.flag_i				|
																((!g65816i_cpu.flag_z)<<1)		|
																((g65816i_cpu.flag_c>>8)&1)); break;
		case CPUINFO_STR_REGISTER + G65816_E:			sprintf(info->s, "E:%d", g65816i_cpu.flag_e); break;
		case CPUINFO_STR_REGISTER + G65816_A:			sprintf(info->s, "A:%04X", g65816i_cpu.a | g65816i_cpu.b); break;
		case CPUINFO_STR_REGISTER + G65816_X:			sprintf(info->s, "X:%04X", g65816i_cpu.x); break;
		case CPUINFO_STR_REGISTER + G65816_Y:			sprintf(info->s, "Y:%04X", g65816i_cpu.y); break;
		case CPUINFO_STR_REGISTER + G65816_NMI_STATE:	sprintf(info->s, "NMI:%X", g65816i_cpu.line_nmi); break;
		case CPUINFO_STR_REGISTER + G65816_IRQ_STATE:	sprintf(info->s, "IRQ:%X", g65816i_cpu.line_irq); break;
	}
}

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
