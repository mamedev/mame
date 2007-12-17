/*****************************************************************************
 *
 *   m6502.c
 *   Portable 6502/65c02/65sc02/6510/n2a03 emulator V1.2
 *
 *   Copyright (c) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *   65sc02 core Copyright (c) 2000 Peter Trauner.
 *   Deco16 portions Copyright (c) 2001-2003 Bryan McPhail.
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
	UINT8	subtype;		/* currently selected cpu sub type */
	void	(**insn)(void); /* pointer to the function pointer table */
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
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
	read8_handler rdmem_id;					/* readmem callback for indexed instructions */
	write8_handler wrmem_id;				/* writemem callback for indexed instructions */

#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
	UINT8    ddr;
	UINT8    port;
	UINT8	(*port_read)(UINT8 direction);
	void	(*port_write)(UINT8 direction, UINT8 data);
#endif

}	m6502_Regs;

int m6502_IntOccured = 0;
int m6502_ICount = 0;

static m6502_Regs m6502;

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

static void m6502_common_init(int index, int clock, const void *config, int (*irqcallback)(int), UINT8 subtype, void (**insn)(void), const char *type)
{
	memset(&m6502, 0, sizeof(m6502));
	m6502.irq_callback = irqcallback;
	m6502.subtype = subtype;
	m6502.insn = insn;
	m6502.rdmem_id = program_read_byte_8;
	m6502.wrmem_id = program_write_byte_8;

	state_save_register_item(type, index, m6502.pc.w.l);
	state_save_register_item(type, index, m6502.sp.w.l);
	state_save_register_item(type, index, m6502.p);
	state_save_register_item(type, index, m6502.a);
	state_save_register_item(type, index, m6502.x);
	state_save_register_item(type, index, m6502.y);
	state_save_register_item(type, index, m6502.pending_irq);
	state_save_register_item(type, index, m6502.after_cli);
	state_save_register_item(type, index, m6502.nmi_state);
	state_save_register_item(type, index, m6502.irq_state);
	state_save_register_item(type, index, m6502.so_state);

#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
	if (subtype == SUBTYPE_6510)
	{
		state_save_register_item(type, index, m6502.port);
		state_save_register_item(type, index, m6502.ddr);
	}
#endif
}

static void m6502_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_6502, insn6502, "m6502");
}

static void m6502_reset(void)
{
	/* wipe out the rest of the m6502 structure */
	/* read the reset vector into PC */
	PCL = RDMEM(M6502_RST_VEC);
	PCH = RDMEM(M6502_RST_VEC+1);

	m6502.sp.d = 0x01ff;	/* stack pointer starts at page 1 offset FF */
	m6502.p = F_T|F_I|F_Z|F_B|(P&F_D);	/* set T, I and Z flags */
	m6502.pending_irq = 0;	/* nonzero if an IRQ is pending */
	m6502.after_cli = 0;	/* pending IRQ and last insn cleared I */
	m6502.irq_state = 0;
	m6502.nmi_state = 0;

	change_pc(PCD);
}

static void m6502_exit(void)
{
	/* nothing to do yet */
}

static void m6502_get_context (void *dst)
{
	if( dst )
		*(m6502_Regs*)dst = m6502;
}

static void m6502_set_context (void *src)
{
	if( src )
	{
		m6502 = *(m6502_Regs*)src;
		change_pc(PCD);
	}
}

INLINE void m6502_take_irq(void)
{
	if( !(P & F_I) )
	{
		EAD = M6502_IRQ_VEC;
		m6502_ICount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M6502#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (m6502.irq_callback) (*m6502.irq_callback)(0);
		change_pc(PCD);
	}
	m6502.pending_irq = 0;
}

static int m6502_execute(int cycles)
{
	m6502_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		CALL_MAME_DEBUG;

		/* if an irq is pending, take it now */
		if( m6502.pending_irq )
			m6502_take_irq();

		op = RDOP();
		(*m6502.insn[op])();

		/* check if the I flag was just reset (interrupts enabled) */
		if( m6502.after_cli )
		{
			LOG(("M6502#%d after_cli was >0", cpu_getactivecpu()));
			m6502.after_cli = 0;
			if (m6502.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				m6502.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else {
			if ( m6502.pending_irq == 2 ) {
				if ( m6502_IntOccured - m6502_ICount > 1 ) {
					m6502.pending_irq = 1;
				}
			}
			if( m6502.pending_irq == 1 )
				m6502_take_irq();
			if ( m6502.pending_irq == 2 ) {
				m6502.pending_irq = 1;
			}
		}

	} while (m6502_ICount > 0);

	return cycles - m6502_ICount;
}

static void m6502_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m6502.nmi_state == state) return;
		m6502.nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
			EAD = M6502_NMI_VEC;
			m6502_ICount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6502#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PCD));
			change_pc(PCD);
		}
	}
	else
	{
		if( irqline == M6502_SET_OVERFLOW )
		{
			if( m6502.so_state && !state )
			{
				LOG(( "M6502#%d set overflow\n", cpu_getactivecpu()));
				P|=F_V;
			}
			m6502.so_state=state;
			return;
		}
		m6502.irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
			m6502.pending_irq = 1;
//          m6502.pending_irq = 2;
			m6502_IntOccured = m6502_ICount;
		}
	}
}



/****************************************************************************
 * 2A03 section
 ****************************************************************************/
#if (HAS_N2A03)

static void n2a03_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_2A03, insn2a03, "n2a03");
}

/* The N2A03 is integrally tied to its PSG (they're on the same die).
   Bit 7 of address $4011 (the PSG's DPCM control register), when set,
   causes an IRQ to be generated.  This function allows the IRQ to be called
   from the PSG core when such an occasion arises. */
void n2a03_irq(void)
{
  m6502_take_irq();
}
#endif


/****************************************************************************
 * 6510 section
 ****************************************************************************/
#if (HAS_M6510)

static void m6510_init (int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_6510, insn6510, "m6510");
}

static void m6510_reset (void)
{
	m6502_reset();
	m6502.port = 0xff;
	m6502.ddr = 0x00;
}

static UINT8 m6510_get_port(void)
{
	return (m6502.port & m6502.ddr) | (m6502.ddr ^ 0xff);
}

static READ8_HANDLER( m6510_read_0000 )
{
	UINT8 result = 0x00;

	switch(offset)
	{
		case 0x0000:	/* DDR */
			result = m6502.ddr;
			break;
		case 0x0001:	/* Data Port */
			if (m6502.port_read)
				result = m6502.port_read( m6502.ddr );
			result = (m6502.ddr & m6502.port) | (~m6502.ddr & result);
			break;
	}
	return result;
}

static WRITE8_HANDLER( m6510_write_0000 )
{
	switch(offset)
	{
		case 0x0000:	/* DDR */
			m6502.ddr = data;
			break;
		case 0x0001:	/* Data Port */
			m6502.port = data;
			break;
	}

	if (m6502.port_write)
		m6502.port_write( m6502.ddr, m6502.port & m6502.ddr );
}

static ADDRESS_MAP_START(m6510_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(m6510_read_0000, m6510_write_0000)
ADDRESS_MAP_END

#endif


/****************************************************************************
 * 65C02 section
 ****************************************************************************/
#if (HAS_M65C02)

static void m65c02_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_65C02, insn65c02, "m65c02");
}

static void m65c02_reset (void)
{
	m6502_reset();
	P &=~F_D;
}

INLINE void m65c02_take_irq(void)
{
	if( !(P & F_I) )
	{
		EAD = M6502_IRQ_VEC;
		m6502_ICount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M65c02#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (m6502.irq_callback) (*m6502.irq_callback)(0);
		change_pc(PCD);
	}
	m6502.pending_irq = 0;
}

static int m65c02_execute(int cycles)
{
	m6502_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		CALL_MAME_DEBUG;

		op = RDOP();
		(*m6502.insn[op])();

		/* if an irq is pending, take it now */
		if( m6502.pending_irq )
			m65c02_take_irq();


		/* check if the I flag was just reset (interrupts enabled) */
		if( m6502.after_cli )
		{
			LOG(("M6502#%d after_cli was >0", cpu_getactivecpu()));
			m6502.after_cli = 0;
			if (m6502.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				m6502.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( m6502.pending_irq )
			m65c02_take_irq();

	} while (m6502_ICount > 0);

	return cycles - m6502_ICount;
}

static void m65c02_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m6502.nmi_state == state) return;
		m6502.nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
			EAD = M6502_NMI_VEC;
			m6502_ICount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P = (P & ~F_D) | F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6502#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PCD));
			change_pc(PCD);
		}
	}
	else
		m6502_set_irq_line(irqline,state);
}
#endif

/****************************************************************************
 * 65SC02 section
 ****************************************************************************/
#if (HAS_M65SC02)
static void m65sc02_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_65SC02, insn65sc02, "m65sc02");
}
#endif

/****************************************************************************
 * DECO16 section
 ****************************************************************************/
#if (HAS_DECO16)

static void deco16_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6502_common_init(index, clock, config, irqcallback, SUBTYPE_DECO16, insndeco16, "deco16");
}


static void deco16_reset (void)
{
	m6502_reset();
	m6502.subtype = SUBTYPE_DECO16;
	m6502.insn = insndeco16;

    PCL = RDMEM(DECO16_RST_VEC+1);
    PCH = RDMEM(DECO16_RST_VEC);

	m6502.sp.d = 0x01ff;	/* stack pointer starts at page 1 offset FF */
	m6502.p = F_T|F_I|F_Z|F_B|(P&F_D);	/* set T, I and Z flags */
	m6502.pending_irq = 0;	/* nonzero if an IRQ is pending */
	m6502.after_cli = 0;	/* pending IRQ and last insn cleared I */

	change_pc(PCD);
}

INLINE void deco16_take_irq(void)
{
	if( !(P & F_I) )
	{
		EAD = DECO16_IRQ_VEC;
		m6502_ICount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* set I flag */
		PCL = RDMEM(EAD+1);
		PCH = RDMEM(EAD);
		LOG(("M6502#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (m6502.irq_callback) (*m6502.irq_callback)(0);
		change_pc(PCD);
	}
	m6502.pending_irq = 0;
}

static void deco16_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m6502.nmi_state == state) return;
		m6502.nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
			EAD = DECO16_NMI_VEC;
			m6502_ICount -= 7;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* set I flag */
			PCL = RDMEM(EAD+1);
			PCH = RDMEM(EAD);
			LOG(("M6502#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PCD));
			change_pc(PCD);
		}
	}
	else
	{
		if( irqline == M6502_SET_OVERFLOW )
		{
			if( m6502.so_state && !state )
			{
				LOG(( "M6502#%d set overflow\n", cpu_getactivecpu()));
				P|=F_V;
			}
			m6502.so_state=state;
			return;
		}
		m6502.irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6502#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
			m6502.pending_irq = 1;
		}
	}
}

static int deco16_execute(int cycles)
{
	m6502_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		CALL_MAME_DEBUG;

		op = RDOP();
		(*m6502.insn[op])();

		/* if an irq is pending, take it now */
		if( m6502.pending_irq )
			deco16_take_irq();


		/* check if the I flag was just reset (interrupts enabled) */
		if( m6502.after_cli )
		{
			LOG(("M6502#%d after_cli was >0", cpu_getactivecpu()));
			m6502.after_cli = 0;
			if (m6502.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				m6502.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( m6502.pending_irq )
			deco16_take_irq();

	} while (m6502_ICount > 0);

	return cycles - m6502_ICount;
}

#endif



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void m6502_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		m6502_set_irq_line(M6502_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	m6502_set_irq_line(M6502_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		m6502_set_irq_line(INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							PCW = info->i; change_pc(PCD);			break;
		case CPUINFO_INT_REGISTER + M6502_PC:			m6502.pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6502_S:			m6502.sp.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6502_P:			m6502.p = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_A:			m6502.a = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_X:			m6502.x = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_Y:			m6502.y = info->i;						break;
		case CPUINFO_INT_REGISTER + M6502_EA:			m6502.ea.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6502_ZP:			m6502.zp.w.l = info->i;					break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	m6502.rdmem_id = (read8_handler) info->f; break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	m6502.wrmem_id = (write8_handler) info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void m6502_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6502);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
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

		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		info->i = m6502.irq_state;			break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	info->i = m6502.so_state;			break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = m6502.nmi_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m6502.ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER + M6502_PC:			info->i = m6502.pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6502_S:			info->i = m6502.sp.b.l;					break;
		case CPUINFO_INT_REGISTER + M6502_P:			info->i = m6502.p;						break;
		case CPUINFO_INT_REGISTER + M6502_A:			info->i = m6502.a;						break;
		case CPUINFO_INT_REGISTER + M6502_X:			info->i = m6502.x;						break;
		case CPUINFO_INT_REGISTER + M6502_Y:			info->i = m6502.y;						break;
		case CPUINFO_INT_REGISTER + M6502_EA:			info->i = m6502.ea.w.l;					break;
		case CPUINFO_INT_REGISTER + M6502_ZP:			info->i = m6502.zp.w.l;					break;
		case CPUINFO_INT_REGISTER + M6502_SUBTYPE:		info->i = m6502.subtype;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m6502_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m6502_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m6502_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m6502_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m6502_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m6502_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m6502_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m6502_dasm;			break;
#endif
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m6502_ICount;			break;
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	info->f = (genf *) m6502.rdmem_id;		break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	info->f = (genf *) m6502.wrmem_id;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6502");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Mostek 6502");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 1998 Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				m6502.p & 0x80 ? 'N':'.',
				m6502.p & 0x40 ? 'V':'.',
				m6502.p & 0x20 ? 'R':'.',
				m6502.p & 0x10 ? 'B':'.',
				m6502.p & 0x08 ? 'D':'.',
				m6502.p & 0x04 ? 'I':'.',
				m6502.p & 0x02 ? 'Z':'.',
				m6502.p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M6502_PC:			sprintf(info->s, "PC:%04X", m6502.pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6502_S:			sprintf(info->s, "S:%02X", m6502.sp.b.l); break;
		case CPUINFO_STR_REGISTER + M6502_P:			sprintf(info->s, "P:%02X", m6502.p); break;
		case CPUINFO_STR_REGISTER + M6502_A:			sprintf(info->s, "A:%02X", m6502.a); break;
		case CPUINFO_STR_REGISTER + M6502_X:			sprintf(info->s, "X:%02X", m6502.x); break;
		case CPUINFO_STR_REGISTER + M6502_Y:			sprintf(info->s, "Y:%02X", m6502.y); break;
		case CPUINFO_STR_REGISTER + M6502_EA:			sprintf(info->s, "EA:%04X", m6502.ea.w.l); break;
		case CPUINFO_STR_REGISTER + M6502_ZP:			sprintf(info->s, "ZP:%03X", m6502.zp.w.l); break;
	}
}


#if (HAS_N2A03)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void n2a03_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = n2a03_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "N2A03");				break;

		default:										m6502_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void m6510_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6510_PORTREAD:	m6502.port_read = (UINT8 (*)(UINT8)) info->f;	break;
		case CPUINFO_PTR_M6510_PORTWRITE:	m6502.port_write = (void (*)(UINT8,UINT8)) info->f;	break;

		default:							m6502_set_info(state, info);						break;
	}
}

void m6510_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m6510_set_info;			break;
		case CPUINFO_PTR_INIT:							info->init = m6510_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m6510_reset;				break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m6510_dasm;			break;
#endif
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP:			info->internal_map = construct_map_m6510_mem; break;
		case CPUINFO_PTR_M6510_PORTREAD:				info->f = (genf *) m6502.port_read;		break;
		case CPUINFO_PTR_M6510_PORTWRITE:				info->f = (genf *) m6502.port_write;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6510");				break;

		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_M6510_PORT:					info->i = m6510_get_port();				break;

		default:										m6502_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M6510T)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m6510t_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6510T");				break;

		default:										m6510_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M7501)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m7501_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M7501");				break;

		default:										m6510_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M8502)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m8502_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M8502");				break;

		default:										m6510_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M65C02)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void m65c02_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	m65c02_set_irq_line(INPUT_LINE_NMI, info->i); break;

		default:										m6502_set_info(state, info);			break;
	}
}

void m65c02_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m65c02_set_info;		break;
		case CPUINFO_PTR_INIT:							info->init = m65c02_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m65c02_reset;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m65c02_execute;			break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m65c02_dasm;		break;
#endif

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M65C02");				break;

		default:										m6502_get_info(state, info);			break;
	}
}
#endif


#if (HAS_M65SC02)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m65sc02_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = m65sc02_init;				break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m65sc02_dasm;		break;
#endif

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M65SC02");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Metal Oxid Semiconductor MOS 6502"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0beta");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 1998 Juergen Buchmueller\nCopyright (c) 2000 Peter Trauner\nall rights reserved."); break;

		default:										m65c02_get_info(state, info);			break;
	}
}
#endif


#if (HAS_DECO16)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void deco16_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6502_IRQ_LINE:		deco16_set_irq_line(M6502_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6502_SET_OVERFLOW:	deco16_set_irq_line(M6502_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		deco16_set_irq_line(INPUT_LINE_NMI, info->i); break;

		default:										m6502_set_info(state, info);			break;
	}
}

void deco16_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = deco16_set_info;		break;
		case CPUINFO_PTR_INIT:							info->init = deco16_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = deco16_reset;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = deco16_execute;			break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = deco16_dasm;		break;
#endif

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "DECO CPU16");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "DECO");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 1998 Juergen Buchmueller\nCopyright (c) 2001-2003 Bryan McPhail\nall rights reserved."); break;

		default:										m6502_get_info(state, info);			break;
	}
}
#endif
