/*****************************************************************************
 *
 *   m6502.c
 *   Portable 6502/65c02/65sc02/6510/n2a03 emulator V1.2
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   65sc02 core Copyright Peter Trauner.
 *   Deco16 portions Copyright Bryan McPhail.
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
/* 2.February 2000 PeT added 65sc02 subtype */
/* 10.March   2000 PeT added 6502 set overflow input line */
/* 13.September 2000 PeT N2A03 jmp indirect */

#if ((HAS_M65SC02 || HAS_DECO16) && !HAS_M65C02)
#undef HAS_M65C02
#define HAS_M65C02 1
#endif

#include "debugger.h"
#include "m6502.h"
#include "ops02.h"
#include "ill02.h"


#define M6502_NMI_VEC	0xfffa
#define M6502_RST_VEC	0xfffc
#define M6502_IRQ_VEC	0xfffe

#define DECO16_RST_VEC	0xfff0
#define DECO16_IRQ_VEC	0xfff2
#define DECO16_NMI_VEC	0xfff4

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/****************************************************************************
 * The 6502 registers.
 ****************************************************************************/
typedef struct _m6502_Regs m6502_Regs;
struct _m6502_Regs
{
	UINT8	subtype;		/* currently selected cpu sub type */
	void	(*const *insn)(m6502_Regs *); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	PAIR	pc; 			/* program counter */
	PAIR	sp; 			/* stack pointer (always 100 - 1FF) */
	PAIR	zp; 			/* zero page address */
	PAIR	ea; 			/* effective address */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* X index register */
	UINT8	y;				/* Y index register */
	UINT8	p;				/* Processor status */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	UINT8   so_state;

	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *space;
	const address_space *io;
	int		int_occured;
	int		icount;

	m6502_read_indexed_func rdmem_id;					/* readmem callback for indexed instructions */
	m6502_write_indexed_func wrmem_id;					/* writemem callback for indexed instructions */

#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
	UINT8    ddr;
	UINT8    port;
	m6510_port_read_func port_read;
	m6510_port_write_func port_write;
#endif

};


static UINT8 default_rdmem_id(const address_space *space, offs_t offset) { return memory_read_byte_8le(space, offset); }
static void default_wdmem_id(const address_space *space, offs_t offset, UINT8 data) { memory_write_byte_8le(space, offset, data); }

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "t6502.c"

#if (HAS_M6510)
#include "t6510.c"
#endif

#include "opsn2a03.h"

#if (HAS_N2A03)
#include "tn2a03.c"
#endif

#include "opsc02.h"

#if (HAS_M65C02)
#include "t65c02.c"
#endif

#if (HAS_M65SC02)
#include "t65sc02.c"
#endif

#if (HAS_DECO16)
#include "tdeco16.c"
#endif

/*****************************************************************************
 *
 *      6502 CPU interface functions
 *
 *****************************************************************************/

static void m6502_common_init(const device_config *device, int index, int clock, cpu_irq_callback irqcallback, UINT8 subtype, void (*const *insn)(m6502_Regs *cpustate), const char *type)
{
	m6502_Regs *cpustate = device->token;

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->space = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->subtype = subtype;
	cpustate->insn = insn;
	cpustate->rdmem_id = default_rdmem_id;
	cpustate->wrmem_id = default_wdmem_id;

	state_save_register_device_item(device, 0, cpustate->pc.w.l);
	state_save_register_device_item(device, 0, cpustate->sp.w.l);
	state_save_register_device_item(device, 0, cpustate->p);
	state_save_register_device_item(device, 0, cpustate->a);
	state_save_register_device_item(device, 0, cpustate->x);
	state_save_register_device_item(device, 0, cpustate->y);
	state_save_register_device_item(device, 0, cpustate->pending_irq);
	state_save_register_device_item(device, 0, cpustate->after_cli);
	state_save_register_device_item(device, 0, cpustate->nmi_state);
	state_save_register_device_item(device, 0, cpustate->irq_state);
	state_save_register_device_item(device, 0, cpustate->so_state);

#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
	if (subtype == SUBTYPE_6510)
	{
		state_save_register_device_item(device, 0, cpustate->port);
		state_save_register_device_item(device, 0, cpustate->ddr);
	}
#endif
}

static CPU_INIT( m6502 )
{
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_6502, insn6502, "m6502");
}

static CPU_RESET( m6502 )
{
	m6502_Regs *cpustate = device->token;
	/* wipe out the rest of the m6502 structure */
	/* read the reset vector into PC */
	PCL = RDMEM(M6502_RST_VEC);
	PCH = RDMEM(M6502_RST_VEC+1);

	cpustate->sp.d = 0x01ff;	/* stack pointer starts at page 1 offset FF */
	cpustate->p = F_T|F_I|F_Z|F_B|(P&F_D);	/* set T, I and Z flags */
	cpustate->pending_irq = 0;	/* nonzero if an IRQ is pending */
	cpustate->after_cli = 0;	/* pending IRQ and last insn cleared I */
	cpustate->irq_state = 0;
	cpustate->nmi_state = 0;
}

static CPU_EXIT( m6502 )
{
	/* nothing to do yet */
}

INLINE void m6502_take_irq(m6502_Regs *cpustate)
{
	if( !(P & F_I) )
	{
		EAD = M6502_IRQ_VEC;
		cpustate->icount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M6502 '%s' takes IRQ ($%04x)\n", cpustate->device->tag, PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static CPU_EXECUTE( m6502 )
{
	m6502_Regs *cpustate = device->token;

	cpustate->icount = cycles;

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			m6502_take_irq(cpustate);

		op = RDOP();
		(*cpustate->insn[op])(cpustate);

		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M6502 '%s' after_cli was >0", cpustate->device->tag));
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
		else {
			if ( cpustate->pending_irq == 2 ) {
				if ( cpustate->int_occured - cpustate->icount > 1 ) {
					cpustate->pending_irq = 1;
				}
			}
			if( cpustate->pending_irq == 1 )
				m6502_take_irq(cpustate);
			if ( cpustate->pending_irq == 2 ) {
				cpustate->pending_irq = 1;
			}
		}

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

static void m6502_set_irq_line(m6502_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag));
			EAD = M6502_NMI_VEC;
			cpustate->icount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6502 '%s' takes NMI ($%04x)\n", cpustate->device->tag, PCD));
		}
	}
	else
	{
		if( irqline == M6502_SET_OVERFLOW )
		{
			if( cpustate->so_state && !state )
			{
				LOG(( "M6502 '%s' set overflow\n", cpustate->device->tag));
				P|=F_V;
			}
			cpustate->so_state=state;
			return;
		}
		cpustate->irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502 '%s' set_irq_line(ASSERT)\n", cpustate->device->tag));
			cpustate->pending_irq = 1;
//          cpustate->pending_irq = 2;
			cpustate->int_occured = cpustate->icount;
		}
	}
}



/****************************************************************************
 * 2A03 section
 ****************************************************************************/
#if (HAS_N2A03)

static CPU_INIT( n2a03 )
{
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_2A03, insn2a03, "n2a03");
}

/* The N2A03 is integrally tied to its PSG (they're on the same die).
   Bit 7 of address $4011 (the PSG's DPCM control register), when set,
   causes an IRQ to be generated.  This function allows the IRQ to be called
   from the PSG core when such an occasion arises. */
void n2a03_irq(const device_config *device)
{
	m6502_Regs *cpustate = device->token;

	m6502_take_irq(cpustate);
}
#endif


/****************************************************************************
 * 6510 section
 ****************************************************************************/
#if (HAS_M6510)

static CPU_INIT( m6510 )
{
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_6510, insn6510, "m6510");
}

static CPU_RESET( m6510 )
{
	m6502_Regs *cpustate = device->token;

	CPU_RESET_CALL(m6502);
	cpustate->port = 0xff;
	cpustate->ddr = 0x00;
}

static UINT8 m6510_get_port(m6502_Regs *cpustate)
{
	return (cpustate->port & cpustate->ddr) | (cpustate->ddr ^ 0xff);
}

static READ8_HANDLER( m6510_read_0000 )
{
	m6502_Regs *cpustate = space->cpu->token;
	UINT8 result = 0x00;

	switch(offset)
	{
		case 0x0000:	/* DDR */
			result = cpustate->ddr;
			break;
		case 0x0001:	/* Data Port */
			if (cpustate->port_read)
				result = cpustate->port_read( cpustate->device, cpustate->ddr );
			result = (cpustate->ddr & cpustate->port) | (~cpustate->ddr & result);
			break;
	}
	return result;
}

static WRITE8_HANDLER( m6510_write_0000 )
{
	m6502_Regs *cpustate = space->cpu->token;

	switch(offset)
	{
		case 0x0000:	/* DDR */
			cpustate->ddr = data;
			break;
		case 0x0001:	/* Data Port */
			cpustate->port = data;
			break;
	}

	if (cpustate->port_write)
		cpustate->port_write( cpustate->device, cpustate->ddr, cpustate->port & cpustate->ddr );
}

static ADDRESS_MAP_START(m6510_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(m6510_read_0000, m6510_write_0000)
ADDRESS_MAP_END

#endif


/****************************************************************************
 * 65C02 section
 ****************************************************************************/
#if (HAS_M65C02)

static CPU_INIT( m65c02 )
{
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_65C02, insn65c02, "m65c02");
}

static CPU_RESET( m65c02 )
{
	m6502_Regs *cpustate = device->token;

	CPU_RESET_CALL(m6502);
	P &=~F_D;
}

INLINE void m65c02_take_irq(m6502_Regs *cpustate)
{
	if( !(P & F_I) )
	{
		EAD = M6502_IRQ_VEC;
		cpustate->icount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M65c02 '%s' takes IRQ ($%04x)\n", cpustate->device->tag, PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static CPU_EXECUTE( m65c02 )
{
	m6502_Regs *cpustate = device->token;

	cpustate->icount = cycles;

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		op = RDOP();
		(*cpustate->insn[op])(cpustate);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			m65c02_take_irq(cpustate);


		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M6502 '%s' after_cli was >0", cpustate->device->tag));
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
			m65c02_take_irq(cpustate);

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

static void m65c02_set_irq_line(m6502_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag));
			EAD = M6502_NMI_VEC;
			cpustate->icount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6502 '%s' takes NMI ($%04x)\n", cpustate->device->tag, PCD));
		}
	}
	else
		m6502_set_irq_line(cpustate, irqline,state);
}
#endif

/****************************************************************************
 * 65SC02 section
 ****************************************************************************/
#if (HAS_M65SC02)
static CPU_INIT( m65sc02 )
{
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_65SC02, insn65sc02, "m65sc02");
}
#endif

/****************************************************************************
 * DECO16 section
 ****************************************************************************/
#if (HAS_DECO16)

static CPU_INIT( deco16 )
{
	m6502_Regs *cpustate = device->token;
	m6502_common_init(device, index, clock, irqcallback, SUBTYPE_DECO16, insndeco16, "deco16");
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);
}


static CPU_RESET( deco16 )
{
	m6502_Regs *cpustate = device->token;

	CPU_RESET_CALL(m6502);
	cpustate->subtype = SUBTYPE_DECO16;
	cpustate->insn = insndeco16;

    PCL = RDMEM(DECO16_RST_VEC+1);
    PCH = RDMEM(DECO16_RST_VEC);

	cpustate->sp.d = 0x01ff;	/* stack pointer starts at page 1 offset FF */
	cpustate->p = F_T|F_I|F_Z|F_B|(P&F_D);	/* set T, I and Z flags */
	cpustate->pending_irq = 0;	/* nonzero if an IRQ is pending */
	cpustate->after_cli = 0;	/* pending IRQ and last insn cleared I */
}

INLINE void deco16_take_irq(m6502_Regs *cpustate)
{
	if( !(P & F_I) )
	{
		EAD = DECO16_IRQ_VEC;
		cpustate->icount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* set I flag */
		PCL = RDMEM(EAD+1);
		PCH = RDMEM(EAD);
		LOG(("M6502 '%s' takes IRQ ($%04x)\n", cpustate->device->tag, PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static void deco16_set_irq_line(m6502_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag));
			EAD = DECO16_NMI_VEC;
			cpustate->icount -= 7;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* set I flag */
			PCL = RDMEM(EAD+1);
			PCH = RDMEM(EAD);
			LOG(("M6502 '%s' takes NMI ($%04x)\n", cpustate->device->tag, PCD));
		}
	}
	else
	{
		if( irqline == M6502_SET_OVERFLOW )
		{
			if( cpustate->so_state && !state )
			{
				LOG(( "M6502 '%s' set overflow\n", cpustate->device->tag));
				P|=F_V;
			}
			cpustate->so_state=state;
			return;
		}
		cpustate->irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502 '%s' set_irq_line(ASSERT)\n", cpustate->device->tag));
			cpustate->pending_irq = 1;
		}
	}
}

static CPU_EXECUTE( deco16 )
{
	m6502_Regs *cpustate = device->token;

	cpustate->icount = cycles;

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		op = RDOP();
		(*cpustate->insn[op])(cpustate);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			deco16_take_irq(cpustate);


		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M6502 %s after_cli was >0", cpustate->device->tag));
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
			deco16_take_irq(cpustate);

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

#endif



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m6502 )
{
	m6502_Regs *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		m6502_set_irq_line(cpustate, M6502_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	m6502_set_irq_line(cpustate, M6502_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		m6502_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							PCW = info->i; 							break;
		case CPUINFO_INT_REGISTER + M6502_PC:			cpustate->pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6502_S:			cpustate->sp.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6502_P:			cpustate->p = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_A:			cpustate->a = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_X:			cpustate->x = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_Y:			cpustate->y = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_EA:			cpustate->ea.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6502_ZP:			cpustate->zp.w.l = info->i;					break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	cpustate->rdmem_id = (m6502_read_indexed_func) info->f; break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	cpustate->wrmem_id = (m6502_write_indexed_func) info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m6502 )
{
	m6502_Regs *cpustate = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6502_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		info->i = cpustate->irq_state;			break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	info->i = cpustate->so_state;			break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = cpustate->nmi_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER + M6502_PC:			info->i = cpustate->pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6502_S:			info->i = cpustate->sp.b.l;					break;
		case CPUINFO_INT_REGISTER + M6502_P:			info->i = cpustate->p;						break;
		case CPUINFO_INT_REGISTER + M6502_A:			info->i = cpustate->a;						break;
		case CPUINFO_INT_REGISTER + M6502_X:			info->i = cpustate->x;						break;
		case CPUINFO_INT_REGISTER + M6502_Y:			info->i = cpustate->y;						break;
		case CPUINFO_INT_REGISTER + M6502_EA:			info->i = cpustate->ea.w.l;					break;
		case CPUINFO_INT_REGISTER + M6502_ZP:			info->i = cpustate->zp.w.l;					break;
		case CPUINFO_INT_REGISTER + M6502_SUBTYPE:		info->i = cpustate->subtype;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6502);			break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m6502);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m6502);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(m6502);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6502);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6502);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	info->f = (genf *) cpustate->rdmem_id;		break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	info->f = (genf *) cpustate->wrmem_id;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6502");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Mostek 6502");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

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

		case CPUINFO_STR_REGISTER + M6502_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6502_S:			sprintf(info->s, "S:%02X", cpustate->sp.b.l); break;
		case CPUINFO_STR_REGISTER + M6502_P:			sprintf(info->s, "P:%02X", cpustate->p); break;
		case CPUINFO_STR_REGISTER + M6502_A:			sprintf(info->s, "A:%02X", cpustate->a); break;
		case CPUINFO_STR_REGISTER + M6502_X:			sprintf(info->s, "X:%02X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + M6502_Y:			sprintf(info->s, "Y:%02X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + M6502_EA:			sprintf(info->s, "EA:%04X", cpustate->ea.w.l); break;
		case CPUINFO_STR_REGISTER + M6502_ZP:			sprintf(info->s, "ZP:%03X", cpustate->zp.w.l); break;
	}
}


#if (HAS_N2A03)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( n2a03 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(n2a03);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "N2A03");				break;

		default:										CPU_GET_INFO_CALL(m6502);			break;
	}
}
#endif


#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( m6510 )
{
	m6502_Regs *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6510_PORTREAD:	cpustate->port_read = (m6510_port_read_func) info->f;	break;
		case CPUINFO_PTR_M6510_PORTWRITE:	cpustate->port_write = (m6510_port_write_func) info->f;	break;

		default:							CPU_SET_INFO_CALL(m6502);			break;
	}
}

CPU_GET_INFO( m6510 )
{
	m6502_Regs *cpustate = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6510);			break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m6510);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m6510);				break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6510);			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP:			info->internal_map8 = ADDRESS_MAP_NAME(m6510_mem); break;
		case CPUINFO_PTR_M6510_PORTREAD:				info->f = (genf *) cpustate->port_read;		break;
		case CPUINFO_PTR_M6510_PORTWRITE:				info->f = (genf *) cpustate->port_write;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6510");				break;

		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_M6510_PORT:					info->i = m6510_get_port(cpustate);				break;

		default:										CPU_GET_INFO_CALL(m6502);			break;
	}
}
#endif


#if (HAS_M6510T)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6510t )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6510T");				break;

		default:										CPU_GET_INFO_CALL(m6510);			break;
	}
}
#endif


#if (HAS_M7501)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m7501 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M7501");				break;

		default:										CPU_GET_INFO_CALL(m6510);			break;
	}
}
#endif


#if (HAS_M8502)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m8502 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M8502");				break;

		default:										CPU_GET_INFO_CALL(m6510);			break;
	}
}
#endif


#if (HAS_M65C02)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( m65c02 )
{
	m6502_Regs *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	m65c02_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		default:										CPU_SET_INFO_CALL(m6502);			break;
	}
}

CPU_GET_INFO( m65c02 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m65c02);		break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m65c02);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m65c02);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m65c02);			break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m65c02);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M65C02");				break;

		default:										CPU_GET_INFO_CALL(m6502);			break;
	}
}
#endif


#if (HAS_M65SC02)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m65sc02 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m65sc02);				break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m65sc02);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M65SC02");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Metal Oxid Semiconductor MOS 6502"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0beta");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller\nCopyright Peter Trauner\nall rights reserved."); break;

		default:										CPU_GET_INFO_CALL(m65c02);			break;
	}
}
#endif


#if (HAS_DECO16)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( deco16 )
{
	m6502_Regs *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		deco16_set_irq_line(cpustate, M6502_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	deco16_set_irq_line(cpustate, M6502_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		deco16_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		default:											CPU_SET_INFO_CALL(m6502);			break;
	}
}

CPU_GET_INFO( deco16 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(deco16);		break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(deco16);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(deco16);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(deco16);			break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(deco16);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "DECO CPU16");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "DECO");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller\nCopyright Bryan McPhail\nall rights reserved."); break;

		default:										CPU_GET_INFO_CALL(m6502);			break;
	}
}
#endif
