/*****************************************************************************
 *
 *   m65ce02.c
 *   Portable 65ce02 emulator V1.0beta3
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved
 *   documentation preliminary databook
 *   documentation by michael steil mist@c64.org
 *   available at ftp://ftp.funet.fi/pub/cbm/c65
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/* 4. February 2000 PeT fixed relative word operand */
/* 4. February 2000 PeT jsr (absolut) jsr (absolut,x) inw dew */
/* 17.February 2000 PeT phw */
/* 16.March 2000 PeT fixed some instructions accordingly to databook */
/* 7. May 2000 PeT splittet into m65ce02 and m4510 */

/*

* neg is now simple 2er komplement negation with set of N and Z

* phw push low order byte, push high order byte!

* tys txs not interruptable, not implemented

*/

#include "debugger.h"
#include "m65ce02.h"

#include "mincce02.h"
#include "opsce02.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


/* Layout of the registers in the debugger */
static UINT8 m65ce02_reg_layout[] = {
	M65CE02_A,M65CE02_X,M65CE02_Y,M65CE02_Z,M65CE02_S,M65CE02_PC,
	M65CE02_P,
	-1,
	M65CE02_EA,M65CE02_ZP,M65CE02_NMI_STATE,M65CE02_IRQ_STATE, M65CE02_B,
	0
};

/* Layout of the debugger windows x,y,w,h */
static UINT8 m65ce02_win_layout[] = {
	25, 0,55, 2,	/* register window (top, right rows) */
	 0, 0,24,22,	/* disassembler window (left colums) */
	25, 3,55, 9,	/* memory #1 window (right, upper middle) */
	25,13,55, 9,	/* memory #2 window (right, lower middle) */
	 0,23,80, 1,	/* command line window (bottom rows) */
};

typedef struct {
	void	(**insn)(void); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	PAIR	pc; 			/* program counter */
	PAIR	sp; 			/* stack pointer (always 100 - 1FF) */
	PAIR	zp; 			/* zero page address */
	/* contains B register zp.b.h */
	PAIR	ea; 			/* effective address */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* X index register */
	UINT8	y;				/* Y index register */
	UINT8	z;				/* Z index register */
	UINT8	p;				/* Processor status */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
}	m65ce02_Regs;


int m65ce02_ICount = 0;

static m65ce02_Regs m65ce02;

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "t65ce02.c"

void m65ce02_reset (void *param)
{
	m65ce02.insn = insn65ce02;

	/* wipe out the rest of the m65ce02 structure */
	/* read the reset vector into PC */
	/* reset z index and b bank */
	PCL = RDMEM(M65CE02_RST_VEC);
	PCH = RDMEM(M65CE02_RST_VEC+1);

	/* after reset in 6502 compatibility mode */
	m65ce02.sp.d = 0x01ff; /* high byte descriped in databook */
	m65ce02.z = 0;
	B = 0;
	m65ce02.p = F_E|F_B|F_I|F_Z;	/* set E, I and Z flags */
	m65ce02.pending_irq = 0;	/* nonzero if an IRQ is pending */
	m65ce02.after_cli = 0;		/* pending IRQ and last insn cleared I */
	m65ce02.irq_callback = NULL;

	change_pc(PCD);
}

void m65ce02_exit(void)
{
	/* nothing to do yet */
}

unsigned m65ce02_get_context (void *dst)
{
	if( dst )
		*(m65ce02_Regs*)dst = m65ce02;
	return sizeof(m65ce02_Regs);
}

void m65ce02_set_context (void *src)
{
	if( src )
	{
		m65ce02 = *(m65ce02_Regs*)src;
		change_pc(PCD);
	}
}

unsigned m65ce02_get_reg (int regnum)
{
	switch( regnum )
	{
		case REG_PC: return PCD;
		case M65CE02_PC: return m65ce02.pc.w.l;
		case REG_SP: return S;
		case M65CE02_S: return m65ce02.sp.w.l;
		case M65CE02_P: return m65ce02.p;
		case M65CE02_A: return m65ce02.a;
		case M65CE02_X: return m65ce02.x;
		case M65CE02_Y: return m65ce02.y;
		case M65CE02_Z: return m65ce02.z;
		case M65CE02_B: return m65ce02.zp.b.h;
		case M65CE02_EA: return m65ce02.ea.w.l;
		case M65CE02_ZP: return m65ce02.zp.b.l;
		case M65CE02_NMI_STATE: return m65ce02.nmi_state;
		case M65CE02_IRQ_STATE: return m65ce02.irq_state;
		case REG_PREVIOUSPC: return m65ce02.ppc.w.l;
	}
	return 0;
}

void m65ce02_set_reg (int regnum, unsigned val)
{
	switch( regnum )
	{
		case REG_PC: PCW = val; change_pc(PCD); break;
		case M65CE02_PC: m65ce02.pc.w.l = val; break;
		case REG_SP: S = val; break;
		case M65CE02_S: m65ce02.sp.w.l = val; break;
		case M65CE02_P: m65ce02.p = val; break;
		case M65CE02_A: m65ce02.a = val; break;
		case M65CE02_X: m65ce02.x = val; break;
		case M65CE02_Y: m65ce02.y = val; break;
		case M65CE02_Z: m65ce02.z = val; break;
		case M65CE02_B: m65ce02.zp.b.h = val; break;
		case M65CE02_EA: m65ce02.ea.w.l = val; break;
		case M65CE02_ZP: m65ce02.zp.b.l = val; break;
		case M65CE02_NMI_STATE: m65ce02_set_irq_line( INPUT_LINE_NMI, val ); break;
		case M65CE02_IRQ_STATE: m65ce02_set_irq_line( 0, val ); break;
	}
}

INLINE void m65ce02_take_irq(void)
{
	if( !(P & F_I) )
	{
		EAD = M65CE02_IRQ_VEC;
		m65ce02_ICount -= 7;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M65ce02#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (m65ce02.irq_callback) (*m65ce02.irq_callback)(0);
		change_pc(PCD);
	}
	m65ce02.pending_irq = 0;
}

int m65ce02_execute(int cycles)
{
	m65ce02_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		CALL_MAME_DEBUG;

		/* if an irq is pending, take it now */
		if( m65ce02.pending_irq )
			m65ce02_take_irq();

		op = RDOP();
		(*insn65ce02[op])();

		/* check if the I flag was just reset (interrupts enabled) */
		if( m65ce02.after_cli )
		{
			LOG(("M65ce02#%d after_cli was >0", cpu_getactivecpu()));
			m65ce02.after_cli = 0;
			if (m65ce02.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				m65ce02.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( m65ce02.pending_irq )
			m65ce02_take_irq();

	} while (m65ce02_ICount > 0);

	return cycles - m65ce02_ICount;
}

void m65ce02_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m65ce02.nmi_state == state) return;
		m65ce02.nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M65ce02#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
			EAD = M65CE02_NMI_VEC;
			m65ce02_ICount -= 7;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M65ce02#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PCD));
			change_pc(PCD);
		}
	}
	else
	{
		m65ce02.irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M65ce02#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
			m65ce02.pending_irq = 1;
		}
	}
}

void m65ce02_set_irq_callback(int (*callback)(int))
{
	m65ce02.irq_callback = callback;
}

void m65ce02_state_save(void *file)
{
	int cpu = cpu_getactivecpu();
	/* insn is set at restore since it's a pointer */
	state_save_UINT16(file,"m65ce02",cpu,"PC",&m65ce02.pc.w.l,2);
	state_save_UINT16(file,"m65ce02",cpu,"SP",&m65ce02.sp.w.l,2);
	state_save_UINT8(file,"m65ce02",cpu,"P",&m65ce02.p,1);
	state_save_UINT8(file,"m65ce02",cpu,"A",&m65ce02.a,1);
	state_save_UINT8(file,"m65ce02",cpu,"X",&m65ce02.x,1);
	state_save_UINT8(file,"m65ce02",cpu,"Y",&m65ce02.y,1);
	state_save_UINT8(file,"m65ce02",cpu,"Z",&m65ce02.z,1);
	state_save_UINT8(file,"m65ce02",cpu,"B",&m65ce02.zp.b.h,1);
	state_save_UINT8(file,"m65ce02",cpu,"PENDING",&m65ce02.pending_irq,1);
	state_save_UINT8(file,"m65ce02",cpu,"AFTER_CLI",&m65ce02.after_cli,1);
	state_save_UINT8(file,"m65ce02",cpu,"NMI_STATE",&m65ce02.nmi_state,1);
	state_save_UINT8(file,"m65ce02",cpu,"IRQ_STATE",&m65ce02.irq_state,1);
}

void m65ce02_state_load(void *file)
{
	int cpu = cpu_getactivecpu();
	m65ce02.insn = insn65ce02;
	state_load_UINT16(file,"m65ce02",cpu,"PC",&m65ce02.pc.w.l,2);
	state_load_UINT16(file,"m65ce02",cpu,"SP",&m65ce02.sp.w.l,2);
	state_load_UINT8(file,"m65ce02",cpu,"P",&m65ce02.p,1);
	state_load_UINT8(file,"m65ce02",cpu,"A",&m65ce02.a,1);
	state_load_UINT8(file,"m65ce02",cpu,"X",&m65ce02.x,1);
	state_load_UINT8(file,"m65ce02",cpu,"Y",&m65ce02.y,1);
	state_load_UINT8(file,"m65ce02",cpu,"Z",&m65ce02.z,1);
	state_load_UINT8(file,"m65ce02",cpu,"B",&m65ce02.zp.b.h,1);
	state_load_UINT8(file,"m65ce02",cpu,"PENDING",&m65ce02.pending_irq,1);
	state_load_UINT8(file,"m65ce02",cpu,"AFTER_CLI",&m65ce02.after_cli,1);
	state_load_UINT8(file,"m65ce02",cpu,"NMI_STATE",&m65ce02.nmi_state,1);
	state_load_UINT8(file,"m65ce02",cpu,"IRQ_STATE",&m65ce02.irq_state,1);
}

/****************************************************************************
 * Return a formatted string for a register
 ****************************************************************************/
const char *m65ce02_info(void *context, int regnum)
{
	char *buffer = cpuintrf_temp_str();
	m65ce02_Regs *r = context;

	if( !context )
		r = &m65ce02;

	switch( regnum )
	{
		case CPU_INFO_REG+M65CE02_PC: sprintf(buffer, "PC:%04X", r->pc.w.l); break;
		case CPU_INFO_REG+M65CE02_S: sprintf(buffer, "S:%04X", r->sp.w.l); break;
		case CPU_INFO_REG+M65CE02_P: sprintf(buffer, "P:%02X", r->p); break;
		case CPU_INFO_REG+M65CE02_A: sprintf(buffer, "A:%02X", r->a); break;
		case CPU_INFO_REG+M65CE02_X: sprintf(buffer, "X:%02X", r->x); break;
		case CPU_INFO_REG+M65CE02_Y: sprintf(buffer, "Y:%02X", r->y); break;
		case CPU_INFO_REG+M65CE02_Z: sprintf(buffer, "Z:%02X", r->z); break;
		case CPU_INFO_REG+M65CE02_B: sprintf(buffer, "B:%02X", r->zp.b.h); break;
		case CPU_INFO_REG+M65CE02_EA: sprintf(buffer, "EA:%04X", r->ea.w.l); break;
		case CPU_INFO_REG+M65CE02_ZP: sprintf(buffer, "ZP:%04X", r->zp.w.l); break;
		case CPU_INFO_REG+M65CE02_NMI_STATE: sprintf(buffer, "NMI:%X", r->nmi_state); break;
		case CPU_INFO_REG+M65CE02_IRQ_STATE: sprintf(buffer, "IRQ:%X", r->irq_state); break;
		case CPU_INFO_FLAGS:
			sprintf(buffer, "%c%c%c%c%c%c%c%c",
				r->p & 0x80 ? 'N':'.',
				r->p & 0x40 ? 'V':'.',
				r->p & 0x20 ? 'E':'.',
				r->p & 0x10 ? 'B':'.',
				r->p & 0x08 ? 'D':'.',
				r->p & 0x04 ? 'I':'.',
				r->p & 0x02 ? 'Z':'.',
				r->p & 0x01 ? 'C':'.');
			break;
		case CPU_INFO_NAME: return "M65CE02";
		case CPU_INFO_FAMILY: return "CBM Semiconductor Group CSG 65CE02";
		case CPU_INFO_VERSION: return "1.0beta";
		case CPU_INFO_CREDITS:
			return "Copyright (c) 1998 Juergen Buchmueller\n"
				"Copyright (c) 2000 Peter Trauner\n"
				"all rights reserved.";
		case CPU_INFO_FILE: return __FILE__;
		case CPU_INFO_REG_LAYOUT: return (const char*)m65ce02_reg_layout;
		case CPU_INFO_WIN_LAYOUT: return (const char*)m65ce02_win_layout;
	}
	return buffer;
}

unsigned int m65ce02_dasm(char *buffer, unsigned pc)
{
#ifdef MAME_DEBUG
	return Dasm65ce02( buffer, pc );
#else
	sprintf( buffer, "$%02X", cpu_readop(pc) );
	return 1;
#endif
}



