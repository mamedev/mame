/*****************************************************************************
 *
 *   m4510.c
 *   Portable 4510 emulator V1.0beta1
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

/*
   c65 memory management
   (reset)
   c64 ff
   c65 20 (interface)

a   (c65 mode)
   a:00 x:e3 y:00 z:b3
   c65 64 (interface)
   c64 ff

b   (c65 dosmode?)
   c65 65 (interface, full colorram)
   a:00 x:11 y:80 z:31
   c64 ff

c   (?)
   c64 07
   a:00 x:00 y:00 z:00

   a c65 mode

   diskcontroller accesses


   monitor
   c64 ff
   a:a0 x:82 y:00 z:83

   c64 mode
   c65 0
   c65 2f:0 !
   c64 ff
   a:00 x:00 y:00 z:00

internal 8 mb to 64k switching (jmp routine in rom)
( seams to be incomplete, in chapter 1 1megabyte memory mapper )
         a  x  y  z
g      0 00 e0 00 f0
g  10000 00 e1 00 f1
g  20000 00 e2 00 f2
g  30000 00 e3 00 f3
g  40000 00 e4 00 f4
g  50000 00 e5 00 f5
g  60000 00 e6 00 f6
.
.
g  f0000 00 ef 00 ff
the same for 100000 .. 700000
g 800000 00 e3 00 b3

thesis:
a: ?0?0 0000
   ? ?       only in monitor mode set
x:      xxxx address bits a19 .. a16 for memory accesses with a15 0 ?
   0000      c64 mode
   0001      dosmode
   1110      c65 mode, plain ram access
             (0000-1fff contains the switching code, so not switchable!?)
   1000      monitor
   1         map 6000-7fff
    1        map 4000-5fff
     1       map 2000-3fff
      1      map 0000-1fff
y: ?000 0000
   ?         only in dos mode set
z:      xxxx address bits a19 .. a16 for memory accesses with a15 1 ?
   0000      c64 mode
   0011      dosmode
   1000      monitor
   1011      c65 mode
   1111      plain ram access
   1         map e000-ffff
    1        map c000-dfff
     1       map a000-bfff
      1      map 8000-9fff
 */

#include "emu.h"
#include "debugger.h"
#include "m6502.h"
#include "m4510.h"

#include "minc4510.h"
#include "opsce02.h"
#include "ops4510.h"

#define M6502_NMI_VEC	0xfffa
#define M6502_RST_VEC	0xfffc
#define M6502_IRQ_VEC	0xfffe
#define M4510_RST_VEC	M6502_RST_VEC
#define M4510_IRQ_VEC	M6502_IRQ_VEC
#define M4510_NMI_VEC	M6502_NMI_VEC

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

typedef struct _m4510_Regs m4510_Regs;
struct _m4510_Regs {
	void	(*const *insn)(m4510_Regs *); /* pointer to the function pointer table */
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
	UINT8 interrupt_inhibit;	/* Some instructions, like MAP, inhibit interrupt */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	UINT16  low, high;
	UINT32	mem[8];

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *space;
	direct_read_data *direct;
	int 	icount;

	devcb_resolved_read8 rdmem_id;					/* readmem callback for indexed instructions */
	devcb_resolved_write8 wrmem_id;					/* writemem callback for indexed instructions */

	UINT8    ddr;
	UINT8    port;

	devcb_resolved_read8	in_port_func;
	devcb_resolved_write8	out_port_func;
};

INLINE m4510_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M4510);
	return (m4510_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

INLINE int m4510_cpu_readop(m4510_Regs *cpustate)
{
	register UINT16 t=cpustate->pc.w.l++;
	return cpustate->direct->read_decrypted_byte(M4510_MEM(t));
}

INLINE int m4510_cpu_readop_arg(m4510_Regs *cpustate)
{
	register UINT16 t=cpustate->pc.w.l++;
	return cpustate->direct->read_raw_byte(M4510_MEM(t));
}

#define M4510
#include "t65ce02.c"

static CPU_INIT( m4510 )
{
	m4510_Regs *cpustate = get_safe_token(device);
	const m6502_interface *intf = (const m6502_interface *)device->static_config();

	cpustate->interrupt_inhibit = 0;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->space = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->space->direct();

	if ( intf )
	{
		cpustate->rdmem_id.resolve(intf->read_indexed_func, *device);
		cpustate->wrmem_id.resolve(intf->write_indexed_func, *device);
		cpustate->in_port_func.resolve(intf->in_port_func, *device);
		cpustate->out_port_func.resolve(intf->out_port_func, *device);
	}
	else
	{
		devcb_read8 nullrcb = DEVCB_NULL;
		devcb_write8 nullwcb = DEVCB_NULL;

		cpustate->rdmem_id.resolve(nullrcb, *device);
		cpustate->wrmem_id.resolve(nullwcb, *device);
		cpustate->in_port_func.resolve(nullrcb, *device);
		cpustate->out_port_func.resolve(nullwcb, *device);
	}
}

static CPU_RESET( m4510 )
{
	m4510_Regs *cpustate = get_safe_token(device);

	cpustate->insn = insn4510;

	/* wipe out the rest of the m65ce02 structure */
	/* read the reset vector into PC */
	/* reset z index and b bank */
	PCL = RDMEM(M4510_RST_VEC);
	PCH = RDMEM(M4510_RST_VEC+1);

	/* after reset in 6502 compatibility mode */
	cpustate->sp.d = 0x01ff; /* high byte descriped in databook */
	cpustate->z = 0;
	B = 0;
	cpustate->p = F_E|F_B|F_I|F_Z;	/* set E, I and Z flags */
	cpustate->interrupt_inhibit = 0;
	cpustate->pending_irq = 0;	/* nonzero if an IRQ is pending */
	cpustate->after_cli = 0;		/* pending IRQ and last insn cleared I */
	cpustate->irq_callback = NULL;

	/* don't know */
	cpustate->high=0x8200;
	cpustate->mem[7]=0x20000;

	cpustate->port = 0xff;
	cpustate->ddr = 0x00;
}

static CPU_EXIT( m4510 )
{
	/* nothing to do yet */
}

INLINE void m4510_take_irq(m4510_Regs *cpustate)
{
	if(( !(P & F_I) ) && (cpustate->interrupt_inhibit == 0))
	{
		EAD = M4510_IRQ_VEC;
		cpustate->icount -= 7;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M4510 '%s' takes IRQ ($%04x)\n", cpustate->device->tag(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static CPU_EXECUTE( m4510 )
{
	m4510_Regs *cpustate = get_safe_token(device);

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			m4510_take_irq(cpustate);

		op = RDOP();
		(*insn4510[op])(cpustate);

		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M4510 '%s' after_cli was >0", cpustate->device->tag()));
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
			m4510_take_irq(cpustate);

	} while (cpustate->icount > 0);
}

static void m4510_set_irq_line(m4510_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M4510 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag()));
			EAD = M4510_NMI_VEC;
			cpustate->icount -= 7;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M4510 '%s' takes NMI ($%04x)\n", cpustate->device->tag(), PCD));
		}
	}
	else
	{
		cpustate->irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(("M4510 '%s' set_irq_line(ASSERT)\n", cpustate->device->tag()));
			cpustate->pending_irq = 1;
		}
	}
}

UINT8 m4510_get_port(legacy_cpu_device *device)
{
	m4510_Regs *cpustate = get_safe_token(device);
	return (cpustate->port & cpustate->ddr) | (cpustate->ddr ^ 0xff);
}
static READ8_HANDLER( m4510_read_0000 )
{
	UINT8 result = 0x00;
	m4510_Regs *cpustate = get_safe_token(&space->device());

	switch(offset)
	{
		case 0x0000:	/* DDR */
			result = cpustate->ddr;
			break;
		case 0x0001:	/* Data Port */
			result = cpustate->in_port_func(0);
			result = (cpustate->ddr & cpustate->port) | (~cpustate->ddr & result);
			break;
	}
	return result;
}

static WRITE8_HANDLER( m4510_write_0000 )
{
	m4510_Regs *cpustate = get_safe_token(&space->device());

	switch(offset)
	{
		case 0x0000:	/* DDR */
			cpustate->ddr = data;
			break;
		case 0x0001:	/* Data Port */
			cpustate->port = data;
			break;
	}

	cpustate->out_port_func(0, m4510_get_port(downcast<legacy_cpu_device *>(&space->device())));
}

static ADDRESS_MAP_START(m4510_mem, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x0000, 0x0001) AM_READWRITE_LEGACY(m4510_read_0000, m4510_write_0000)
ADDRESS_MAP_END

static CPU_TRANSLATE( m4510 )
{
	m4510_Regs *cpustate = get_safe_token(device);

	if (space == AS_PROGRAM)
		*address = M4510_MEM(*address);
	return TRUE;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m4510 )
{
	m4510_Regs *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M4510_IRQ_LINE:	m4510_set_irq_line(cpustate, M4510_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	m4510_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							PCW = info->i;								break;
		case CPUINFO_INT_REGISTER + M4510_PC:			cpustate->pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							SPL = info->i;							break;
		case CPUINFO_INT_REGISTER + M4510_S:			cpustate->sp.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_P:			cpustate->p = info->i;						break;
		case CPUINFO_INT_REGISTER + M4510_A:			cpustate->a = info->i;						break;
		case CPUINFO_INT_REGISTER + M4510_X:			cpustate->x = info->i;						break;
		case CPUINFO_INT_REGISTER + M4510_Y:			cpustate->y = info->i;						break;
		case CPUINFO_INT_REGISTER + M4510_Z:			cpustate->z = info->i;						break;
		case CPUINFO_INT_REGISTER + M4510_B:			cpustate->zp.b.h = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM_LOW:		cpustate->low = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM_HIGH:		cpustate->high = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_EA:			cpustate->ea.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_ZP:			cpustate->zp.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM0:			cpustate->mem[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM1:			cpustate->mem[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM2:			cpustate->mem[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM3:			cpustate->mem[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM4:			cpustate->mem[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM5:			cpustate->mem[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM6:			cpustate->mem[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM7:			cpustate->mem[7] = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m4510 )
{
	m4510_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m4510_Regs);			break;
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

		case CPUINFO_INT_INPUT_STATE + M4510_IRQ_LINE:	info->i = cpustate->irq_state;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER + M4510_PC:			info->i = cpustate->pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = SPL;							break;
		case CPUINFO_INT_REGISTER + M4510_S:			info->i = cpustate->sp.b.l;					break;
		case CPUINFO_INT_REGISTER + M4510_P:			info->i = cpustate->p;						break;
		case CPUINFO_INT_REGISTER + M4510_A:			info->i = cpustate->a;						break;
		case CPUINFO_INT_REGISTER + M4510_X:			info->i = cpustate->x;						break;
		case CPUINFO_INT_REGISTER + M4510_Y:			info->i = cpustate->y;						break;
		case CPUINFO_INT_REGISTER + M4510_Z:			info->i = cpustate->z;						break;
		case CPUINFO_INT_REGISTER + M4510_B:			info->i = cpustate->zp.b.h;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM_LOW:		info->i = cpustate->low;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM_HIGH:		info->i = cpustate->high;					break;
		case CPUINFO_INT_REGISTER + M4510_EA:			info->i = cpustate->ea.w.l;					break;
		case CPUINFO_INT_REGISTER + M4510_ZP:			info->i = cpustate->zp.w.l;					break;
		case CPUINFO_INT_REGISTER + M4510_MEM0:			info->i = cpustate->mem[0];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM1:			info->i = cpustate->mem[1];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM2:			info->i = cpustate->mem[2];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM3:			info->i = cpustate->mem[3];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM4:			info->i = cpustate->mem[4];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM5:			info->i = cpustate->mem[5];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM6:			info->i = cpustate->mem[6];					break;
		case CPUINFO_INT_REGISTER + M4510_MEM7:			info->i = cpustate->mem[7];					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m4510);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m4510);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m4510);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m4510);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m4510);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m4510);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(m4510_mem); break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = CPU_TRANSLATE_NAME(m4510);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M4510");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "CBM Semiconductor Group CSG 65CE02"); break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0beta");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller\nCopyright Peter Trauner\nall rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->p & 0x80 ? 'N':'.',
				cpustate->p & 0x40 ? 'V':'.',
				cpustate->p & 0x20 ? 'R':'.',
				cpustate->p & 0x10 ? 'B':'.',
				cpustate->p & 0x08 ? 'D':'.',
				cpustate->p & 0x04 ? 'I':'.',
				cpustate->p & 0x02 ? 'Z':'.',
				cpustate->p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M4510_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M4510_S:			sprintf(info->s, "S:%02X", cpustate->sp.b.l); break;
		case CPUINFO_STR_REGISTER + M4510_P:			sprintf(info->s, "P:%02X", cpustate->p); break;
		case CPUINFO_STR_REGISTER + M4510_A:			sprintf(info->s, "A:%02X", cpustate->a); break;
		case CPUINFO_STR_REGISTER + M4510_X:			sprintf(info->s, "X:%02X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + M4510_Y:			sprintf(info->s, "Y:%02X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + M4510_Z:			sprintf(info->s, "Z:%02X", cpustate->z); break;
		case CPUINFO_STR_REGISTER + M4510_B:			sprintf(info->s, "B:%02X", cpustate->zp.b.h); break;
		case CPUINFO_STR_REGISTER + M4510_MEM_LOW:		sprintf(info->s, "M0:%01X", cpustate->low); break;
		case CPUINFO_STR_REGISTER + M4510_MEM_HIGH:		sprintf(info->s, "M1:%01X", cpustate->high); break;
		case CPUINFO_STR_REGISTER + M4510_EA:			sprintf(info->s, "EA:%04X", cpustate->ea.w.l); break;
		case CPUINFO_STR_REGISTER + M4510_ZP:			sprintf(info->s, "ZP:%03X", cpustate->zp.w.l); break;
	}
}

#undef M4510
DEFINE_LEGACY_CPU_DEVICE(M4510, m4510);
