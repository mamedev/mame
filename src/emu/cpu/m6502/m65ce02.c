/*****************************************************************************
 *
 *   m65ce02.c
 *   Portable 65ce02 emulator V1.0beta3
 *
 *   Copyright Peter Trauner, all rights reserved
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
#include "deprecat.h"
#include "m65ce02.h"

#include "mincce02.h"
#include "opsce02.h"

#define M6502_NMI_VEC	0xfffa
#define M6502_RST_VEC	0xfffc
#define M6502_IRQ_VEC	0xfffe
#define M65CE02_RST_VEC	M6502_RST_VEC
#define M65CE02_IRQ_VEC	M6502_IRQ_VEC
#define M65CE02_NMI_VEC	M6502_NMI_VEC

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


typedef struct {
	void	(*const *insn)(void); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	PAIR	pc;				/* program counter */
	PAIR	sp;				/* stack pointer (always 100 - 1FF) */
	PAIR	zp;				/* zero page address */
	/* contains B register zp.b.h */
	PAIR	ea;				/* effective address */
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
	read8_machine_func rdmem_id;					/* readmem callback for indexed instructions */
	write8_machine_func wrmem_id;				/* writemem callback for indexed instructions */
}	m65ce02_Regs;


static int m65ce02_ICount = 0;

static m65ce02_Regs m65ce02;

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "t65ce02.c"

static READ8_HANDLER( default_rdmem_id ) { return program_read_byte_8le(offset); }
static WRITE8_HANDLER( default_wdmem_id ) { program_write_byte_8le(offset, data); }

static void m65ce02_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m65ce02.rdmem_id = default_rdmem_id;
	m65ce02.wrmem_id = default_wdmem_id;
	m65ce02.irq_callback = irqcallback;
}

static void m65ce02_reset(void)
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

static void m65ce02_exit(void)
{
	/* nothing to do yet */
}

static void m65ce02_get_context (void *dst)
{
	if( dst )
		*(m65ce02_Regs*)dst = m65ce02;
}

static void m65ce02_set_context (void *src)
{
	if( src )
	{
		m65ce02 = *(m65ce02_Regs*)src;
		change_pc(PCD);
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

static int m65ce02_execute(int cycles)
{
	m65ce02_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(Machine, PCD);

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

static void m65ce02_set_irq_line(int irqline, int state)
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

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void m65ce02_set_info(UINT32 state, cpuinfo *info)
{
	switch( state )
 	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M65CE02_IRQ_STATE: m65ce02_set_irq_line( M65CE02_IRQ_LINE, info->i ); break;
		case CPUINFO_INT_INPUT_STATE + M65CE02_NMI_STATE: m65ce02_set_irq_line( INPUT_LINE_NMI, info->i ); break;

		case CPUINFO_INT_PC: PCW = info->i; change_pc(PCD); break;
		case CPUINFO_INT_REGISTER + M65CE02_PC: m65ce02.pc.w.l = info->i; break;
		case CPUINFO_INT_SP: m65ce02.sp.b.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_S: m65ce02.sp.w.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_P: m65ce02.p = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_A: m65ce02.a = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_X: m65ce02.x = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_Y: m65ce02.y = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_Z: m65ce02.z = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_B: m65ce02.zp.b.h = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_EA: m65ce02.ea.w.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_ZP: m65ce02.zp.b.l = info->i; break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	m65ce02.rdmem_id = (read8_machine_func) info->f; break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	m65ce02.wrmem_id = (write8_machine_func) info->f; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void m65ce02_get_info(UINT32 state, cpuinfo *info)
{
	switch( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m65ce02);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 13;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE+M65CE02_NMI_STATE: info->i = m65ce02.nmi_state;			break;
		case CPUINFO_INT_INPUT_STATE+M65CE02_IRQ_STATE: info->i = m65ce02.irq_state;			break;

		case CPUINFO_INT_PREVIOUSPC: info->i = m65ce02.ppc.w.l; break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER+M65CE02_PC:			info->i = m65ce02.pc.w.l;				break;
		case CPUINFO_INT_SP:							info->i = m65ce02.sp.b.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_S:			info->i = m65ce02.sp.w.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_P:			info->i = m65ce02.p;					break;
		case CPUINFO_INT_REGISTER+M65CE02_A:			info->i = m65ce02.a;					break;
		case CPUINFO_INT_REGISTER+M65CE02_X:			info->i = m65ce02.x;					break;
		case CPUINFO_INT_REGISTER+M65CE02_Y:			info->i = m65ce02.y;					break;
		case CPUINFO_INT_REGISTER+M65CE02_Z:			info->i = m65ce02.z;					break;
		case CPUINFO_INT_REGISTER+M65CE02_B:			info->i = m65ce02.zp.b.h;				break;
		case CPUINFO_INT_REGISTER+M65CE02_EA:			info->i = m65ce02.ea.w.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_ZP:			info->i = m65ce02.zp.w.l;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m65ce02_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m65ce02_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m65ce02_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m65ce02_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m65ce02_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m65ce02_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m65ce02_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m65ce02_dasm;			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m65ce02_ICount;			break;
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	info->f = (genf *) m65ce02.rdmem_id;		break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	info->f = (genf *) m65ce02.wrmem_id;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME: strcpy(info->s, "M65CE02");  break;
		case CPUINFO_STR_CORE_FAMILY: strcpy(info->s,"CBM Semiconductor Group CSG 65CE02");  break;
		case CPUINFO_STR_CORE_VERSION: strcpy(info->s,"1.0beta");  break;
		case CPUINFO_STR_CORE_FILE: strcpy(info->s,__FILE__);
		case CPUINFO_STR_CORE_CREDITS:
			strcpy(info->s, "Copyright Juergen Buchmueller\n"
				"Copyright Peter Trauner\n"
				"all rights reserved.");  break;
		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				m65ce02.p & 0x80 ? 'N':'.',
				m65ce02.p & 0x40 ? 'V':'.',
				m65ce02.p & 0x20 ? 'E':'.',
				m65ce02.p & 0x10 ? 'B':'.',
				m65ce02.p & 0x08 ? 'D':'.',
				m65ce02.p & 0x04 ? 'I':'.',
				m65ce02.p & 0x02 ? 'Z':'.',
				m65ce02.p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M65CE02_PC:			sprintf(info->s, "PC:%04X", m65ce02.pc.w.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_S:			sprintf(info->s, "S:%02X", m65ce02.sp.b.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_P:			sprintf(info->s, "P:%02X", m65ce02.p); break;
		case CPUINFO_STR_REGISTER + M65CE02_A:			sprintf(info->s, "A:%02X", m65ce02.a); break;
		case CPUINFO_STR_REGISTER + M65CE02_X:			sprintf(info->s, "X:%02X", m65ce02.x); break;
		case CPUINFO_STR_REGISTER + M65CE02_Y:			sprintf(info->s, "Y:%02X", m65ce02.y); break;
		case CPUINFO_STR_REGISTER + M65CE02_Z:			sprintf(info->s, "Z:%02X", m65ce02.z); break;
		case CPUINFO_STR_REGISTER + M65CE02_B:			sprintf(info->s, "B:%02X", m65ce02.zp.b.h); break;
		case CPUINFO_STR_REGISTER + M65CE02_EA:			sprintf(info->s, "EA:%04X", m65ce02.ea.w.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_ZP:			sprintf(info->s, "ZP:%03X", m65ce02.zp.w.l); break;
	}
}
