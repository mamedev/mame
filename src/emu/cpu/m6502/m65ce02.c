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

#include "emu.h"
#include "debugger.h"
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

struct	m65ce02_Regs {
	void	(*const *insn)(m65ce02_Regs *); /* pointer to the function pointer table */
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
	int		icount;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *space;
	direct_read_data *direct;
	devcb_resolved_read8 rdmem_id;					/* readmem callback for indexed instructions */
	devcb_resolved_write8 wrmem_id;					/* writemem callback for indexed instructions */
};

INLINE m65ce02_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M65CE02);
	return (m65ce02_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "t65ce02.c"

static CPU_INIT( m65ce02 )
{
	m65ce02_Regs *cpustate = get_safe_token(device);
	const m6502_interface *intf = (const m6502_interface *)device->static_config();

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->space = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->space->direct();

	if ( intf )
	{
		cpustate->rdmem_id.resolve(intf->read_indexed_func, *device);
		cpustate->wrmem_id.resolve(intf->write_indexed_func, *device);
	}
	else
	{
		devcb_read8 nullrcb = DEVCB_NULL;
		devcb_write8 nullwcb = DEVCB_NULL;

		cpustate->rdmem_id.resolve(nullrcb, *device);
		cpustate->wrmem_id.resolve(nullwcb, *device);
	}
}

static CPU_RESET( m65ce02 )
{
	m65ce02_Regs *cpustate = get_safe_token(device);

	cpustate->insn = insn65ce02;

	/* wipe out the rest of the m65ce02 structure */
	/* read the reset vector into PC */
	/* reset z index and b bank */
	PCL = RDMEM(M65CE02_RST_VEC);
	PCH = RDMEM(M65CE02_RST_VEC+1);

	/* after reset in 6502 compatibility mode */
	cpustate->sp.d = 0x01ff; /* high byte descriped in databook */
	cpustate->z = 0;
	B = 0;
	cpustate->p = F_E|F_B|F_I|F_Z;	/* set E, I and Z flags */
	cpustate->pending_irq = 0;	/* nonzero if an IRQ is pending */
	cpustate->after_cli = 0;		/* pending IRQ and last insn cleared I */
	cpustate->irq_callback = NULL;
}

static CPU_EXIT( m65ce02 )
{
	/* nothing to do yet */
}

INLINE void m65ce02_take_irq(m65ce02_Regs *cpustate)
{
	if( !(P & F_I) )
	{
		EAD = M65CE02_IRQ_VEC;
		cpustate->icount -= 7;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M65ce02 '%s' takes IRQ ($%04x)\n", cpustate->device->tag(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static CPU_EXECUTE( m65ce02 )
{
	m65ce02_Regs *cpustate = get_safe_token(device);

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			m65ce02_take_irq(cpustate);

		op = RDOP();
		(*insn65ce02[op])(cpustate);

		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M65ce02 '%s' after_cli was >0", cpustate->device->tag()));
			cpustate->after_cli = 0;
			if (cpustate->irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				cpustate->pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( cpustate->pending_irq )
			m65ce02_take_irq(cpustate);

	} while (cpustate->icount > 0);
}

static void m65ce02_set_irq_line(m65ce02_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M65ce02 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag()));
			EAD = M65CE02_NMI_VEC;
			cpustate->icount -= 7;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M65ce02 '%s' takes NMI ($%04x)\n", cpustate->device->tag(), PCD));
		}
	}
	else
	{
		cpustate->irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M65ce02 '%s' set_irq_line(ASSERT)\n", cpustate->device->tag()));
			cpustate->pending_irq = 1;
		}
	}
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m65ce02 )
{
	m65ce02_Regs *cpustate = get_safe_token(device);

	switch( state )
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M65CE02_IRQ_STATE: m65ce02_set_irq_line( cpustate, M65CE02_IRQ_LINE, info->i ); break;
		case CPUINFO_INT_INPUT_STATE + M65CE02_NMI_STATE: m65ce02_set_irq_line( cpustate, INPUT_LINE_NMI, info->i ); break;

		case CPUINFO_INT_PC: PCW = info->i;  break;
		case CPUINFO_INT_REGISTER + M65CE02_PC: cpustate->pc.w.l = info->i; break;
		case CPUINFO_INT_SP: cpustate->sp.b.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_S: cpustate->sp.w.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_P: cpustate->p = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_A: cpustate->a = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_X: cpustate->x = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_Y: cpustate->y = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_Z: cpustate->z = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_B: cpustate->zp.b.h = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_EA: cpustate->ea.w.l = info->i; break;
		case CPUINFO_INT_REGISTER + M65CE02_ZP: cpustate->zp.b.l = info->i; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m65ce02 )
{
	m65ce02_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m65ce02_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:	info->i = 13;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE+M65CE02_NMI_STATE: info->i = cpustate->nmi_state;			break;
		case CPUINFO_INT_INPUT_STATE+M65CE02_IRQ_STATE: info->i = cpustate->irq_state;			break;

		case CPUINFO_INT_PREVIOUSPC: info->i = cpustate->ppc.w.l; break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER+M65CE02_PC:			info->i = cpustate->pc.w.l;				break;
		case CPUINFO_INT_SP:							info->i = cpustate->sp.b.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_S:			info->i = cpustate->sp.w.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_P:			info->i = cpustate->p;					break;
		case CPUINFO_INT_REGISTER+M65CE02_A:			info->i = cpustate->a;					break;
		case CPUINFO_INT_REGISTER+M65CE02_X:			info->i = cpustate->x;					break;
		case CPUINFO_INT_REGISTER+M65CE02_Y:			info->i = cpustate->y;					break;
		case CPUINFO_INT_REGISTER+M65CE02_Z:			info->i = cpustate->z;					break;
		case CPUINFO_INT_REGISTER+M65CE02_B:			info->i = cpustate->zp.b.h;				break;
		case CPUINFO_INT_REGISTER+M65CE02_EA:			info->i = cpustate->ea.w.l;				break;
		case CPUINFO_INT_REGISTER+M65CE02_ZP:			info->i = cpustate->zp.w.l;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m65ce02);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m65ce02);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m65ce02);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m65ce02);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m65ce02);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m65ce02);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME: strcpy(info->s, "M65CE02");  break;
		case CPUINFO_STR_FAMILY: strcpy(info->s,"CBM Semiconductor Group CSG 65CE02");  break;
		case CPUINFO_STR_VERSION: strcpy(info->s,"1.0beta");  break;
		case CPUINFO_STR_SOURCE_FILE: strcpy(info->s,__FILE__);
		case CPUINFO_STR_CREDITS:
			strcpy(info->s, "Copyright Juergen Buchmueller\n"
				"Copyright Peter Trauner\n"
				"all rights reserved.");  break;
		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->p & 0x80 ? 'N':'.',
				cpustate->p & 0x40 ? 'V':'.',
				cpustate->p & 0x20 ? 'E':'.',
				cpustate->p & 0x10 ? 'B':'.',
				cpustate->p & 0x08 ? 'D':'.',
				cpustate->p & 0x04 ? 'I':'.',
				cpustate->p & 0x02 ? 'Z':'.',
				cpustate->p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M65CE02_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_S:			sprintf(info->s, "S:%02X", cpustate->sp.b.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_P:			sprintf(info->s, "P:%02X", cpustate->p); break;
		case CPUINFO_STR_REGISTER + M65CE02_A:			sprintf(info->s, "A:%02X", cpustate->a); break;
		case CPUINFO_STR_REGISTER + M65CE02_X:			sprintf(info->s, "X:%02X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + M65CE02_Y:			sprintf(info->s, "Y:%02X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + M65CE02_Z:			sprintf(info->s, "Z:%02X", cpustate->z); break;
		case CPUINFO_STR_REGISTER + M65CE02_B:			sprintf(info->s, "B:%02X", cpustate->zp.b.h); break;
		case CPUINFO_STR_REGISTER + M65CE02_EA:			sprintf(info->s, "EA:%04X", cpustate->ea.w.l); break;
		case CPUINFO_STR_REGISTER + M65CE02_ZP:			sprintf(info->s, "ZP:%03X", cpustate->zp.w.l); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(M65CE02, m65ce02);
