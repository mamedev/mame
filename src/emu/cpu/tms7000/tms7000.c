/*****************************************************************************
 *
 *   tms7000.c
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright (c) 2001 tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************
 *  Currently this source emulates a TMS70x0, not any of the other variants
 *  Unimplemented is the MC pin which (in conjunection with IOCNT0 bits 7 and 6
 *  control the memory mapping.
 *
 *  This source implements the MC pin at Vss and mode bits in single chip mode.
 *****************************************************************************/

// SJE: Changed all references to ICount to icount (to match MAME requirements)
// SJE: Changed RM/WM macros to reference newly created tms7000 read/write handlers & removed unused SRM() macro
// SJE: Fixed a mistake in tms70x0_pf_w where the wrong register was referenced
// SJE: Implemented internal register file

#include "cpuintrf.h"
#include "debugger.h"
#include "tms7000.h"

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif

/* Private prototypes */

static void tms7000_set_irq_line(int irqline, int state);
static void tms7000_get_context(void *dst);
static void tms7000_set_context(void *src);
static void tms7000_check_IRQ_lines( void );
static void tms7000_do_interrupt( UINT16 address, UINT8 line );
static int tms7000_execute(int cycles);
static int tms7000_exl_execute(int cycles);
static void tms7000_service_timer1( void );
static UINT16 bcd_add( UINT16 a, UINT16 b );
static UINT16 bcd_tencomp( UINT16 a );
static UINT16 bcd_sub( UINT16 a, UINT16 b);

/* Static variables */

static int tms7000_icount;
static int tms7000_div_by_16_trigger;
static int tms7000_cycles_per_INT2;

#define RM(Addr) ((unsigned)program_read_byte_8(Addr))
#define WM(Addr,Value) (program_write_byte_8(Addr,Value))

INLINE UINT16 RM16( UINT32 mAddr )	/* Read memory (16-bit) */
{
	UINT32 result = RM(mAddr) << 8;
	return result | RM((mAddr+1)&0xffff);
}

INLINE void WM16( UINT32 mAddr, PAIR p )	/*Write memory file (16 bit) */
{
	WM( mAddr, p.b.h );
	WM( (mAddr+1)&0xffff, p.b.l );
}

INLINE UINT16 RRF16( UINT32 mAddr )	/*Read register file (16 bit) */
{
	PAIR result;
	result.b.h = RM((mAddr-1)&0xffff);
	result.b.l = RM(mAddr);
	return result.w.l;
}

INLINE void WRF16( UINT32 mAddr, PAIR p )	/*Write register file (16 bit) */
{
	WM( (mAddr-1)&0xffff, p.b.h );
	WM( mAddr, p.b.l );
}

#define IMMBYTE(b)	b = ((unsigned)cpu_readop_arg(pPC)); pPC++
#define SIMMBYTE(b)	b = ((signed)cpu_readop_arg(pPC)); pPC++
#define IMMWORD(w)	w.b.h = (unsigned)cpu_readop_arg(pPC++); w.b.l = (unsigned)cpu_readop_arg(pPC++)

#define PUSHBYTE(b) pSP++; WM(pSP,b)
#define PUSHWORD(w) pSP++; WM(pSP,w.b.h); pSP++; WM(pSP,w.b.l)
#define PULLBYTE(b) b = RM(pSP); pSP--
#define PULLWORD(w) w.b.l = RM(pSP); pSP--; w.b.h = RM(pSP); pSP--

typedef struct
{
	PAIR		pc; 		/* Program counter */
	UINT8		sp;		/* Stack Pointer */
	UINT8		sr;		/* Status Register */
	UINT8		irq_state[3];	/* State of the three IRQs */
	UINT8		rf[0x80];	/* Register file (SJE) */
	UINT8		pf[0x100];	/* Perpherial file */
	int 		(*irq_callback)(int irqline);
	UINT8		t1_capture_latch; /* Timer 1 capture latch */
	INT8		t1_prescaler;	/* Timer 1 prescaler (5 bits) */
	INT16		t1_decrementer;	/* Timer 1 decrementer (8 bits) */
	UINT8		idle_state;	/* Set after the execution of an idle instruction */
} tms7000_Regs;

static tms7000_Regs tms7000;

#define pPC		tms7000.pc.w.l
#define PC		tms7000.pc
#define pSP		tms7000.sp
#define pSR		tms7000.sr

#define RDA		RM(0x0000)
#define RDB		RM(0x0001)

#define WRA(Value) (WM(0x0000,Value))
#define WRB(Value) (WM(0x0001,Value))

#define SR_C	0x80		/* Carry */
#define SR_N	0x40		/* Negative */
#define SR_Z	0x20		/* Zero */
#define SR_I	0x10		/* Interrupt */

#define CLR_NZC 	pSR&=~(SR_N|SR_Z|SR_C)
#define CLR_NZCI 	pSR&=~(SR_N|SR_Z|SR_C|SR_I)
#define SET_C8(a)	pSR|=((a&0x0100)>>1)
#define SET_N8(a)	pSR|=((a&0x0080)>>1)
#define SET_Z(a)	if(!a)pSR|=SR_Z
#define SET_Z8(a)	SET_Z((UINT8)a)
#define SET_Z16(a)	SET_Z((UINT8)a>>8)
#define GET_C		(pSR >> 7)

/* Not working */
#define SET_C16(a)	pSR|=((a&0x010000)>>9)

#define SETC		pSR |= SR_C
#define SETZ		pSR |= SR_Z
#define SETN		pSR |= SR_N

#define CHANGE_PC change_pc(pPC)


static READ8_HANDLER( tms7000_internal_r );
static WRITE8_HANDLER( tms7000_internal_w );
static READ8_HANDLER( tms70x0_pf_r );
static WRITE8_HANDLER( tms70x0_pf_w );

static ADDRESS_MAP_START(tms7000_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x007f)	AM_READWRITE(tms7000_internal_r, tms7000_internal_w)	/* tms7000 internal RAM */
	AM_RANGE(0x0080, 0x00ff)	AM_READWRITE(MRA8_NOP, MWA8_NOP)						/* reserved */
	AM_RANGE(0x0100, 0x01ff)	AM_READWRITE(tms70x0_pf_r, tms70x0_pf_w)				/* tms7000 internal I/O ports */
ADDRESS_MAP_END



/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/

static void tms7000_get_context(void *dst)
{
	if( dst )
		*(tms7000_Regs*)dst = tms7000;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static void tms7000_set_context(void *src)
{
	if( src )
		tms7000 = *(tms7000_Regs*)src;

        tms7000_check_IRQ_lines();
}

static void tms7000_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	int cpu = cpu_getactivecpu();

	tms7000.irq_callback = irqcallback;

	memset(tms7000.pf, 0, 0x100);
	memset(tms7000.rf, 0, 0x80);

	/* Save register state */
	state_save_register_item("tms7000", cpu, pPC);
	state_save_register_item("tms7000", cpu, pSP);
	state_save_register_item("tms7000", cpu, pSR);

	/* Save Interrupt state */
	state_save_register_item_array("tms7000", cpu, tms7000.irq_state);

	/* Save register and perpherial file state */
	state_save_register_item_array("tms7000", cpu, tms7000.rf);
	state_save_register_item_array("tms7000", cpu, tms7000.pf);

	/* Save timer state */
	state_save_register_item("tms7000", cpu, tms7000.t1_prescaler);
	state_save_register_item("tms7000", cpu, tms7000.t1_capture_latch);
	state_save_register_item("tms7000", cpu, tms7000.t1_decrementer);

	state_save_register_item("tms7000", cpu, tms7000.idle_state);

	tms7000.irq_callback = irqcallback;
}

static void tms7000_reset(void)
{
//  tms7000.architecture = (int)param;

	tms7000.idle_state = 0;
	tms7000.irq_state[ TMS7000_IRQ1_LINE ] = CLEAR_LINE;
	tms7000.irq_state[ TMS7000_IRQ2_LINE ] = CLEAR_LINE;
	tms7000.irq_state[ TMS7000_IRQ3_LINE ] = CLEAR_LINE;

	WM( 0x100 + 9, 0 );		/* Data direction regs are cleared */
	WM( 0x100 + 11, 0 );

//  if( tms7000.architecture == TMS7000_NMOS )
//  {
		WM( 0x100 + 4, 0xff );		/* Output 0xff on port A */
		WM( 0x100 + 8, 0xff );		/* Output 0xff on port C */
		WM( 0x100 + 10, 0xff );		/* Output 0xff on port D */
//  }
//  else
//  {
//      WM( 0x100 + 4, 0xff );      /* Output 0xff on port A */
//  }

	pSP = 0x01;				/* Set stack pointer to r1 */
	pSR = 0x00;				/* Clear status register (disabling interrupts */
	WM( 0x100 + 0, 0 );		/* Write a zero to IOCNT0 */

	/* On TMS70x2 and TMS70Cx2 IOCNT1 is zero */

	WRA( tms7000.pc.b.h );	/* Write previous PC to A:B */
	WRB( tms7000.pc.b.l );
	pPC = RM16(0xfffe);		/* Load reset vector */
	CHANGE_PC;

	tms7000_div_by_16_trigger = -16;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void tms7000_set_info(UINT32 state, cpuinfo *info)
{
    switch (state)
    {
        /* --- the following bits of info are set as 64-bit signed integers --- */
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ1_LINE:	tms7000_set_irq_line(TMS7000_IRQ1_LINE, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ2_LINE:	tms7000_set_irq_line(TMS7000_IRQ2_LINE, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ3_LINE:	tms7000_set_irq_line(TMS7000_IRQ3_LINE, info->i);	break;

        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + TMS7000_PC:	pPC = info->i; CHANGE_PC;	break;
        case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + TMS7000_SP:	pSP = info->i;	break;
        case CPUINFO_INT_REGISTER + TMS7000_ST:	pSR = info->i;	tms7000_check_IRQ_lines();	break;
		case CPUINFO_INT_REGISTER + TMS7000_IDLE: tms7000.idle_state = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_CL: tms7000.t1_capture_latch = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_PS: tms7000.t1_prescaler = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_DEC: tms7000.t1_decrementer = info->i;	break;
    }
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void tms7000_get_info(UINT32 state, cpuinfo *info)
{

    switch( state )
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:	info->i = sizeof(tms7000);	break;
        case CPUINFO_INT_INPUT_LINES:	info->i = 3;	break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:	info->i = 0;	break;
        case CPUINFO_INT_ENDIANNESS:	info->i = CPU_IS_BE;	break;
        case CPUINFO_INT_CLOCK_DIVIDER:	info->i = 1;	break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES:	info->i = 1;	break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES:	info->i = 4;	break;
        case CPUINFO_INT_MIN_CYCLES:	info->i = 1;	break;
        case CPUINFO_INT_MAX_CYCLES:	info->i = 48;	break; /* 48 represents the multiply instruction, the next highest is 17 */

        case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;	break;
        case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;	break;
        case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:	info->i = 0;	break;

        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ1_LINE:	info->i = tms7000.irq_state[TMS7000_IRQ1_LINE]; break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ2_LINE:	info->i = tms7000.irq_state[TMS7000_IRQ2_LINE]; break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ3_LINE:	info->i = tms7000.irq_state[TMS7000_IRQ3_LINE]; break;

        case CPUINFO_INT_PREVIOUSPC:	info->i = 0; /* Not supported */	break;

        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + TMS7000_PC:	info->i = pPC;	break;
        case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + TMS7000_SP:	info->i = pSP;	break;
        case CPUINFO_INT_REGISTER + TMS7000_ST:	info->i = pSR;	break;
		case CPUINFO_INT_REGISTER + TMS7000_IDLE: info->i = tms7000.idle_state; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_CL: info->i = tms7000.t1_capture_latch; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_PS: info->i = tms7000.t1_prescaler; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_DEC: info->i = tms7000.t1_decrementer; break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_PTR_SET_INFO:	info->setinfo = tms7000_set_info;	break;
        case CPUINFO_PTR_GET_CONTEXT:	info->getcontext = tms7000_get_context;	break;
        case CPUINFO_PTR_SET_CONTEXT:	info->setcontext = tms7000_set_context;	break;
        case CPUINFO_PTR_INIT:	info->init = tms7000_init;	break;
        case CPUINFO_PTR_RESET:	info->reset = tms7000_reset;	break;
        case CPUINFO_PTR_EXECUTE:	info->execute = tms7000_execute;	break;
        case CPUINFO_PTR_BURN:	info->burn = NULL;	/* Not supported */break;
#ifdef MAME_DEBUG
        case CPUINFO_PTR_DISASSEMBLE:	info->disassemble = tms7000_dasm;	break;
#endif
        case CPUINFO_PTR_INSTRUCTION_COUNTER:	info->icount = &tms7000_icount;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP:	info->internal_map = construct_map_tms7000_mem; break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:	strcpy(info->s = cpuintrf_temp_str(), "TMS7000"); break;
        case CPUINFO_STR_CORE_FAMILY:	strcpy(info->s = cpuintrf_temp_str(), "Texas Instriuments TMS7000"); break;
        case CPUINFO_STR_CORE_VERSION:	strcpy(info->s = cpuintrf_temp_str(), "1.0"); break;
        case CPUINFO_STR_CORE_FILE:	strcpy(info->s = cpuintrf_temp_str(), __FILE__); break;
        case CPUINFO_STR_CORE_CREDITS:	strcpy(info->s = cpuintrf_temp_str(), "Copyright (C) tim lindner 2003"); break;

        case CPUINFO_STR_FLAGS:
                sprintf(info->s = cpuintrf_temp_str(),  "%c%c%c%c%c%c%c%c",
                        tms7000.sr & 0x80 ? 'C':'c',
                        tms7000.sr & 0x40 ? 'N':'n',
                        tms7000.sr & 0x20 ? 'Z':'z',
                        tms7000.sr & 0x10 ? 'I':'i',
                        tms7000.sr & 0x08 ? '?':'.',
                        tms7000.sr & 0x04 ? '?':'.',
                        tms7000.sr & 0x02 ? '?':'.',
                        tms7000.sr & 0x01 ? '?':'.' );
                break;

        case CPUINFO_STR_REGISTER + TMS7000_PC:	sprintf(info->s = cpuintrf_temp_str(), "PC:%04X", tms7000.pc.w.l); break;
        case CPUINFO_STR_REGISTER + TMS7000_SP:	sprintf(info->s = cpuintrf_temp_str(), "S:%02X", tms7000.sp); break;
        case CPUINFO_STR_REGISTER + TMS7000_ST:	sprintf(info->s = cpuintrf_temp_str(), "ST:%02X", tms7000.sr); break;
		case CPUINFO_STR_REGISTER + TMS7000_IDLE: sprintf(info->s = cpuintrf_temp_str(), "Idle:%02X", tms7000.idle_state); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_CL: sprintf(info->s = cpuintrf_temp_str(), "T1CL:%02X", tms7000.t1_capture_latch); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_PS: sprintf(info->s = cpuintrf_temp_str(), "T1PS:%02X", tms7000.t1_prescaler & 0x1f); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_DEC: sprintf(info->s = cpuintrf_temp_str(), "T1DEC:%02X", tms7000.t1_decrementer & 0xff); break;

    }
}

void tms7000_exl_get_info(UINT32 state, cpuinfo *info)
{
    switch( state )
    {
		case CPUINFO_PTR_EXECUTE:
			info->execute = tms7000_exl_execute;
			break;
		default:
			tms7000_get_info(state, info);
			break;
	}
}

void tms7000_set_irq_line(int irqline, int state)
{
	if (tms7000.irq_state[irqline] != state)
	{	/* check for transition */
		tms7000.irq_state[irqline] = state;

		LOG(("tms7000: (cpu #%d) set_irq_line (INT%d, state %d)\n", cpu_getactivecpu(), irqline+1, state));

		if (state == CLEAR_LINE)
		{
			return;
		}

		tms7000.pf[0] |= (0x02 << (irqline * 2));	/* Set INTx iocntl0 flag */

		if( irqline == TMS7000_IRQ3_LINE )
		{
			/* Latch the value in perpherial file register 3 */
			tms7000.t1_capture_latch = tms7000.t1_decrementer & 0x00ff;
		}

		tms7000_check_IRQ_lines();
	}
}

static void tms7000_check_IRQ_lines( void )
{
	if( pSR & SR_I ) /* Check Global Interrupt bit: Status register, bit 4 */
	{
		if ((tms7000.irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE) || (tms7000.pf[0] & 0x02))
		{
			if( tms7000.pf[0] & 0x01 ) /* INT1 Enable bit */
			{
				tms7000_do_interrupt( 0xfffc, TMS7000_IRQ1_LINE );
				tms7000.pf[0] &= ~0x02; /* Data Manual, page: 9-41 */
				return;
			}
		}

		if( tms7000.irq_state[ TMS7000_IRQ2_LINE ] == ASSERT_LINE )
		{
			if( tms7000.pf[0] & 0x04 ) /* INT2 Enable bit */
			{
				tms7000_do_interrupt( 0xfffa, TMS7000_IRQ2_LINE );
				return;
			}
		}

		if ((tms7000.irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE) || (tms7000.pf[0] & 0x20))
		{
			if( tms7000.pf[0] & 0x10 ) /* INT3 Enable bit */
			{
				tms7000_do_interrupt( 0xfff8, TMS7000_IRQ3_LINE );
				tms7000.pf[0] &= ~0x20; /* Data Manual, page: 9-41 */
				return;
			}
		}
	}
}

static void tms7000_do_interrupt( UINT16 address, UINT8 line )
{
	PUSHBYTE( pSR );		/* Push Status register */
	PUSHWORD( PC );			/* Push Program Counter */
	pSR = 0;				/* Clear Status register */
	pPC = RM16(address);	/* Load PC with interrupt vector */
	CHANGE_PC;

	if( tms7000.idle_state == 0 )
		tms7000_icount -= 19;		/* 19 cycles used */
	else
	{
		tms7000_icount -= 17;		/* 17 if idled */
		tms7000.idle_state = 0;
	}

	(void)(*tms7000.irq_callback)(line);
}

#include "tms70op.c"
#include "tms70tb.c"

static int tms7000_execute(int cycles)
{
	int op;

	tms7000_icount = cycles;
	tms7000_div_by_16_trigger += cycles;

	do
	{
		CALL_MAME_DEBUG;

		if( tms7000.idle_state == 0 )
		{
			op = cpu_readop(pPC++);

			opfn[op]();
		}
		else
			tms7000_icount -= 16;

		/* Internal timer system */

		while( tms7000_icount < tms7000_div_by_16_trigger )
		{
			tms7000_div_by_16_trigger -= 16;

			if( (tms7000.pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
			{
				if( (tms7000.pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
					tms7000_service_timer1();
			}
		}

	} while( tms7000_icount > 0 );

	tms7000_div_by_16_trigger -= tms7000_icount;
	return cycles - tms7000_icount;
}

static int tms7000_exl_execute(int cycles)
{
	int op;

	tms7000_icount = cycles;
	tms7000_div_by_16_trigger += cycles;

	do
	{
		CALL_MAME_DEBUG;

		if( tms7000.idle_state == 0 )
		{

			op = cpu_readop(pPC++);

			opfn_exl[op]();
		}
		else
			tms7000_icount -= 16;

		/* Internal timer system */

		while( tms7000_icount < tms7000_div_by_16_trigger )
		{
			tms7000_div_by_16_trigger -= 16;

			if( (tms7000.pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
			{
				if( (tms7000.pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
					tms7000_service_timer1();
			}
		}

	} while( tms7000_icount > 0 );

	tms7000_div_by_16_trigger -= tms7000_icount;
	return cycles - tms7000_icount;
}

/****************************************************************************
 * Trigger the event counter
 ****************************************************************************/
void tms7000_A6EC1( void )
{
    if( (tms7000.pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
    {
        if( (tms7000.pf[0x03] & 0x40) == 0x40) /* Is event counter the timer source? */
            tms7000_service_timer1();
    }
}

static void tms7000_service_timer1( void )
{
    if( --tms7000.t1_prescaler < 0 ) /* Decrement prescaler and check for underflow */
    {
        tms7000.t1_prescaler = tms7000.pf[3] & 0x1f; /* Reload prescaler (5 bit) */

        if( --tms7000.t1_decrementer < 0 ) /* Decrement timer1 register and check for underflow */
        {
            tms7000.t1_decrementer = tms7000.pf[2]; /* Reload decrementer (8 bit) */
			cpunum_set_input_line( cpu_getactivecpu(), TMS7000_IRQ2_LINE, HOLD_LINE);
            LOG( ("tms7000: trigger int2 (cycles: %d)\t%d\tdelta %d\n", activecpu_gettotalcycles(), activecpu_gettotalcycles() - tick, tms7000_cycles_per_INT2-(activecpu_gettotalcycles() - tick) );
			tick = activecpu_gettotalcycles() );
            /* Also, cascade out to timer 2 - timer 2 unimplemented */
        }
    }
//  LOG( ( "tms7000: service timer1. 0x%2.2x 0x%2.2x (cycles %d)\t%d\t\n", tms7000.t1_prescaler, tms7000.t1_decrementer, activecpu_gettotalcycles(), activecpu_gettotalcycles() - tick2 ) );
//  tick2 = activecpu_gettotalcycles();
}

static WRITE8_HANDLER( tms70x0_pf_w )	/* Perpherial file write */
{
	UINT8	temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:	/* IOCNT0, Input/Ouput control */
			temp1 = data & 0x2a;							/* Record which bits to clear */
			temp2 = tms7000.pf[0x00] & 0x2a;				/* Get copy of current bits */
			temp3 = (~temp1) & temp2;						/* Clear the requested bits */
			tms7000.pf[0x00] = temp3 | (data & (~0x2a) );	/* OR in the remaining data */
			break;
		case 0x02:
			tms7000.t1_decrementer = tms7000.pf[0x02] = data;
			tms7000_cycles_per_INT2 = 0x10*((tms7000.pf[3] & 0x1f)+1)*(tms7000.pf[0x02]+1);
			LOG( ( "tms7000: Timer adjusted. Decrementer: 0x%2.2x (Cycles per interrupt: %d)\n", tms7000.t1_decrementer, tms7000_cycles_per_INT2 ) );
			break;
		case 0x03:	/* T1CTL, timer 1 control */
			if( ((tms7000.pf[0x03] & 0x80) == 0) && ((data & 0x80) == 0x80 ) )   /* Start timer? */
			{
				tms7000.pf[0x03] = data;
				tms7000.t1_prescaler = tms7000.pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				tms7000_cycles_per_INT2 = 0x10*((tms7000.pf[3] & 0x1f)+1)*(tms7000.pf[0x02]+1);
				LOG( ( "tms7000: Timer started. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", tms7000.pf[3] & 0x1f, tms7000_cycles_per_INT2 ) );
			}
			else if( ((data & 0x80) == 0x80 ) && ((tms7000.pf[0x03] & 0x80) == 0) )   /* Timer Stopped? */
			{
				tms7000.pf[0x03] = data;
				tms7000.t1_prescaler = tms7000.pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				tms7000_cycles_per_INT2 = 0x10*((tms7000.pf[3] & 0x1f)+1)*(tms7000.pf[0x02]+1);
				LOG( ( "tms7000: Timer stopped. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", tms7000.pf[3] & 0x1f, tms7000_cycles_per_INT2 ) );
			}
			else /* Don't modify timer state, but still store data */
			{
				tms7000.pf[0x03] = data;
				tms7000_cycles_per_INT2 = 0x10*((tms7000.pf[3] & 0x1f)+1)*(tms7000.pf[0x02]+1);
				LOG( ( "tms7000: Timer adjusted. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", tms7000.pf[3] & 0x1f, tms7000_cycles_per_INT2 ) );
			}
			break;

		case 0x04: /* Port A write */
			/* Port A is read only so this is a NOP */
			break;

		case 0x06: /* Port B write */
			io_write_byte_8( TMS7000_PORTB, data );
			tms7000.pf[ 0x06 ] = data;
			break;

		case 0x08: /* Port C write */
			temp1 = data & tms7000.pf[ 0x09 ];	/* Mask off input bits */
			io_write_byte_8( TMS7000_PORTC, temp1 );
			tms7000.pf[ 0x08 ] = temp1;
			break;

		case 0x0a: /* Port D write */
			temp1 = data & tms7000.pf[ 0x0b ];	/* Mask off input bits */
			io_write_byte_8( TMS7000_PORTD, temp1 );
			tms7000.pf[ 0x0a ] = temp1;
			break;

		default:
			/* Just stuff the other registers */
			tms7000.pf[ offset ] = data;
			break;
	}
}

static READ8_HANDLER( tms70x0_pf_r )	/* Perpherial file read */
{
	UINT8 result;
	UINT8	temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:	/* IOCNT0, Input/Ouput control */
			result = tms7000.pf[0x00];
			if (tms7000.irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE)
				result |= 0x02;
			if (tms7000.irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE)
				result |= 0x20;
			break;

		case 0x02:	/* T1DATA, timer 1 8-bit decrementer */
			result = (tms7000.t1_decrementer & 0x00ff);
			break;

		case 0x03:	/* T1CTL, timer 1 capture (latched by INT3) */
			result = tms7000.t1_capture_latch;
			break;

		case 0x04: /* Port A read */
			result = io_read_byte_8( TMS7000_PORTA );
			break;


		case 0x06: /* Port B read */
			/* Port B is write only, return a previous written value */
			result = tms7000.pf[ 0x06 ];
			break;

		case 0x08: /* Port C read */
			temp1 = tms7000.pf[ 0x08 ] & tms7000.pf[ 0x09 ];	/* Get previous output bits */
			temp2 = io_read_byte_8( TMS7000_PORTC );			/* Read port */
			temp3 = temp2 & (~tms7000.pf[ 0x09 ]);				/* Mask off output bits */
			result = temp1 | temp3;								/* OR together */
			break;

		case 0x0a: /* Port D read */
			temp1 = tms7000.pf[ 0x0a ] & tms7000.pf[ 0x0b ];	/* Get previous output bits */
			temp2 = io_read_byte_8( TMS7000_PORTD );			/* Read port */
			temp3 = temp2 & (~tms7000.pf[ 0x0b ]);				/* Mask off output bits */
			result = temp1 | temp3;								/* OR together */
			break;

		default:
			/* Just unstuff the other registers */
			result = tms7000.pf[ offset ];
			break;
	}

	return result;
}

// BCD arthrimetic handling
static UINT16 bcd_add( UINT16 a, UINT16 b )
{
	UINT16	t1,t2,t3,t4,t5,t6;

	/* Sure it is a lot of code, but it works! */
	t1 = a + 0x0666;
	t2 = t1 + b;
	t3 = t1 ^ b;
	t4 = t2 ^ t3;
	t5 = ~t4 & 0x1110;
	t6 = (t5 >> 2) | (t5 >> 3);
	return t2-t6;
}

static UINT16 bcd_tencomp( UINT16 a )
{
	UINT16	t1,t2,t3,t4,t5,t6;

	t1 = 0xffff - a;
	t2 = -a;
	t3 = t1 ^ 0x0001;
	t4 = t2 ^ t3;
	t5 = ~t4 & 0x1110;
	t6 = (t5 >> 2)|(t5>>3);
	return t2-t6;
}

/*
    Compute difference a-b???
*/
static UINT16 bcd_sub( UINT16 a, UINT16 b)
{
	//return bcd_tencomp(b) - bcd_tencomp(a);
	return bcd_add(a, bcd_tencomp(b) & 0xff);
}

static WRITE8_HANDLER( tms7000_internal_w ) {
	tms7000.rf[ offset ] = data;
}

static READ8_HANDLER( tms7000_internal_r ) {
	return tms7000.rf[ offset ];
}
