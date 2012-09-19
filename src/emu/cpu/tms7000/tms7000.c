/*****************************************************************************
 *
 *   tms7000.c
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
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
// SJE: Changed RM/WM macros to reference newly created tms7000 read/write handlers & removed unused SRM(cpustate) macro
// SJE: Fixed a mistake in tms70x0_pf_w where the wrong register was referenced
// SJE: Implemented internal register file

#include "emu.h"
#include "debugger.h"
#include "tms7000.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

struct tms7000_state;

/* Private prototypes */

static void tms7000_set_irq_line(tms7000_state *cpustate, int irqline, int state);
static void tms7000_check_IRQ_lines(tms7000_state *cpustate);
static void tms7000_do_interrupt( tms7000_state *cpustate, UINT16 address, UINT8 line );
static CPU_EXECUTE( tms7000 );
static CPU_EXECUTE( tms7000_exl );
static void tms7000_service_timer1( device_t *device );
static UINT16 bcd_add( UINT16 a, UINT16 b );
static UINT16 bcd_tencomp( UINT16 a );
static UINT16 bcd_sub( UINT16 a, UINT16 b);

/* Static variables */

#define RM(Addr) ((unsigned)cpustate->program->read_byte(Addr))
#define WM(Addr,Value) (cpustate->program->write_byte(Addr, Value))

#define IMMBYTE(b)	b = ((unsigned)cpustate->direct->read_raw_byte(pPC)); pPC++
#define SIMMBYTE(b)	b = ((signed)cpustate->direct->read_raw_byte(pPC)); pPC++
#define IMMWORD(w)	w.b.h = (unsigned)cpustate->direct->read_raw_byte(pPC++); w.b.l = (unsigned)cpustate->direct->read_raw_byte(pPC++)

#define PUSHBYTE(b) pSP++; WM(pSP,b)
#define PUSHWORD(w) pSP++; WM(pSP,w.b.h); pSP++; WM(pSP,w.b.l)
#define PULLBYTE(b) b = RM(pSP); pSP--
#define PULLWORD(w) w.b.l = RM(pSP); pSP--; w.b.h = RM(pSP); pSP--

struct tms7000_state
{
	PAIR		pc; 		/* Program counter */
	UINT8		sp;		/* Stack Pointer */
	UINT8		sr;		/* Status Register */
	UINT8		irq_state[3];	/* State of the three IRQs */
	UINT8		rf[0x80];	/* Register file (SJE) */
	UINT8		pf[0x100];	/* Perpherial file */
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int			icount;
	int 		div_by_16_trigger;
	int			cycles_per_INT2;
	UINT8		t1_capture_latch; /* Timer 1 capture latch */
	INT8		t1_prescaler;	/* Timer 1 prescaler (5 bits) */
	INT16		t1_decrementer;	/* Timer 1 decrementer (8 bits) */
	UINT8		idle_state;	/* Set after the execution of an idle instruction */
};

INLINE tms7000_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS7000 ||
		   device->type() == TMS7000_EXL);
	return (tms7000_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define pPC		cpustate->pc.w.l
#define PC		cpustate->pc
#define pSP		cpustate->sp
#define pSR		cpustate->sr

#define RDA		RM(0x0000)
#define RDB		RM(0x0001)

#define WRA(Value) (WM(0x0000,Value))
#define WRB(Value) (WM(0x0001,Value))

#define SR_C	0x80		/* Carry */
#define SR_N	0x40		/* Negative */
#define SR_Z	0x20		/* Zero */
#define SR_I	0x10		/* Interrupt */

#define CLR_NZC 	pSR&=~(SR_N|SR_Z|SR_C)
#define CLR_NZCI	pSR&=~(SR_N|SR_Z|SR_C|SR_I)
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

static DECLARE_READ8_HANDLER( tms7000_internal_r );
static DECLARE_WRITE8_HANDLER( tms7000_internal_w );
static DECLARE_READ8_HANDLER( tms70x0_pf_r );
static DECLARE_WRITE8_HANDLER( tms70x0_pf_w );

static ADDRESS_MAP_START(tms7000_mem, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(0x0000, 0x007f)	AM_READWRITE_LEGACY(tms7000_internal_r, tms7000_internal_w)	/* tms7000 internal RAM */
	AM_RANGE(0x0080, 0x00ff)	AM_NOP						/* reserved */
	AM_RANGE(0x0100, 0x01ff)	AM_READWRITE_LEGACY(tms70x0_pf_r, tms70x0_pf_w)				/* tms7000 internal I/O ports */
ADDRESS_MAP_END


INLINE UINT16 RM16( tms7000_state *cpustate, UINT32 mAddr )	/* Read memory (16-bit) */
{
	UINT32 result = RM(mAddr) << 8;
	return result | RM((mAddr+1)&0xffff);
}

INLINE UINT16 RRF16( tms7000_state *cpustate, UINT32 mAddr )	/*Read register file (16 bit) */
{
	PAIR result;
	result.b.h = RM((mAddr-1)&0xffff);
	result.b.l = RM(mAddr);
	return result.w.l;
}

INLINE void WRF16( tms7000_state *cpustate, UINT32 mAddr, PAIR p )	/*Write register file (16 bit) */
{
	WM( (mAddr-1)&0xffff, p.b.h );
	WM( mAddr, p.b.l );
}


static CPU_INIT( tms7000 )
{
	tms7000_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);

	memset(cpustate->pf, 0, 0x100);
	memset(cpustate->rf, 0, 0x80);

	/* Save register state */
	device->save_item(NAME(pPC));
	device->save_item(NAME(pSP));
	device->save_item(NAME(pSR));

	/* Save Interrupt state */
	device->save_item(NAME(cpustate->irq_state));

	/* Save register and perpherial file state */
	device->save_item(NAME(cpustate->rf));
	device->save_item(NAME(cpustate->pf));

	/* Save timer state */
	device->save_item(NAME(cpustate->t1_prescaler));
	device->save_item(NAME(cpustate->t1_capture_latch));
	device->save_item(NAME(cpustate->t1_decrementer));

	device->save_item(NAME(cpustate->idle_state));
}

static CPU_RESET( tms7000 )
{
	tms7000_state *cpustate = get_safe_token(device);

//  cpustate->architecture = (int)param;

	cpustate->idle_state = 0;
	cpustate->irq_state[ TMS7000_IRQ1_LINE ] = CLEAR_LINE;
	cpustate->irq_state[ TMS7000_IRQ2_LINE ] = CLEAR_LINE;
	cpustate->irq_state[ TMS7000_IRQ3_LINE ] = CLEAR_LINE;

	WM( 0x100 + 9, 0 );		/* Data direction regs are cleared */
	WM( 0x100 + 11, 0 );

//  if( cpustate->architecture == TMS7000_NMOS )
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

	WRA( cpustate->pc.b.h );	/* Write previous PC to A:B */
	WRB( cpustate->pc.b.l );
	pPC = RM16(cpustate, 0xfffe);		/* Load reset vector */

	cpustate->div_by_16_trigger = -16;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( tms7000 )
{
	tms7000_state *cpustate = get_safe_token(device);

    switch (state)
    {
        /* --- the following bits of info are set as 64-bit signed integers --- */
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ1_LINE:	tms7000_set_irq_line(cpustate, TMS7000_IRQ1_LINE, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ2_LINE:	tms7000_set_irq_line(cpustate, TMS7000_IRQ2_LINE, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ3_LINE:	tms7000_set_irq_line(cpustate, TMS7000_IRQ3_LINE, info->i);	break;

        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + TMS7000_PC:	pPC = info->i; break;
        case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + TMS7000_SP:	pSP = info->i;	break;
        case CPUINFO_INT_REGISTER + TMS7000_ST:	pSR = info->i;	tms7000_check_IRQ_lines(cpustate);	break;
		case CPUINFO_INT_REGISTER + TMS7000_IDLE: cpustate->idle_state = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_CL: cpustate->t1_capture_latch = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_PS: cpustate->t1_prescaler = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_DEC: cpustate->t1_decrementer = info->i;	break;
    }
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( tms7000 )
{
	tms7000_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

    switch( state )
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:	info->i = sizeof(tms7000_state);	break;
        case CPUINFO_INT_INPUT_LINES:	info->i = 3;	break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:	info->i = 0;	break;
        case CPUINFO_INT_ENDIANNESS:	info->i = ENDIANNESS_BIG;	break;
        case CPUINFO_INT_CLOCK_MULTIPLIER:	info->i = 1;	break;
        case CPUINFO_INT_CLOCK_DIVIDER:	info->i = 1;	break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES:	info->i = 1;	break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES:	info->i = 4;	break;
        case CPUINFO_INT_MIN_CYCLES:	info->i = 1;	break;
        case CPUINFO_INT_MAX_CYCLES:	info->i = 48;	break; /* 48 represents the multiply instruction, the next highest is 17 */

        case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 16;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;	break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;	break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_IO:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:	info->i = 8;	break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:	info->i = 0;	break;

        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ1_LINE:	info->i = cpustate->irq_state[TMS7000_IRQ1_LINE]; break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ2_LINE:	info->i = cpustate->irq_state[TMS7000_IRQ2_LINE]; break;
        case CPUINFO_INT_INPUT_STATE + TMS7000_IRQ3_LINE:	info->i = cpustate->irq_state[TMS7000_IRQ3_LINE]; break;

        case CPUINFO_INT_PREVIOUSPC:	info->i = 0; /* Not supported */	break;

        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + TMS7000_PC:	info->i = pPC;	break;
        case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + TMS7000_SP:	info->i = pSP;	break;
        case CPUINFO_INT_REGISTER + TMS7000_ST:	info->i = pSR;	break;
		case CPUINFO_INT_REGISTER + TMS7000_IDLE: info->i = cpustate->idle_state; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_CL: info->i = cpustate->t1_capture_latch; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_PS: info->i = cpustate->t1_prescaler; break;
		case CPUINFO_INT_REGISTER + TMS7000_T1_DEC: info->i = cpustate->t1_decrementer; break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_FCT_SET_INFO:	info->setinfo = CPU_SET_INFO_NAME(tms7000);	break;
        case CPUINFO_FCT_INIT:	info->init = CPU_INIT_NAME(tms7000);	break;
        case CPUINFO_FCT_RESET:	info->reset = CPU_RESET_NAME(tms7000);	break;
        case CPUINFO_FCT_EXECUTE:	info->execute = CPU_EXECUTE_NAME(tms7000);	break;
        case CPUINFO_FCT_BURN:	info->burn = NULL;	/* Not supported */break;
        case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(tms7000);	break;
        case CPUINFO_PTR_INSTRUCTION_COUNTER:	info->icount = &cpustate->icount;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(tms7000_mem); break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:	strcpy(info->s, "TMS7000"); break;
        case CPUINFO_STR_FAMILY:	strcpy(info->s, "Texas Instriuments TMS7000"); break;
        case CPUINFO_STR_VERSION:	strcpy(info->s, "1.0"); break;
        case CPUINFO_STR_SOURCE_FILE:	strcpy(info->s, __FILE__); break;
        case CPUINFO_STR_CREDITS:	strcpy(info->s, "Copyright tim lindner"); break;

        case CPUINFO_STR_FLAGS:
                sprintf(info->s,  "%c%c%c%c%c%c%c%c",
                        cpustate->sr & 0x80 ? 'C':'c',
                        cpustate->sr & 0x40 ? 'N':'n',
                        cpustate->sr & 0x20 ? 'Z':'z',
                        cpustate->sr & 0x10 ? 'I':'i',
                        cpustate->sr & 0x08 ? '?':'.',
                        cpustate->sr & 0x04 ? '?':'.',
                        cpustate->sr & 0x02 ? '?':'.',
                        cpustate->sr & 0x01 ? '?':'.' );
                break;

        case CPUINFO_STR_REGISTER + TMS7000_PC:	sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
        case CPUINFO_STR_REGISTER + TMS7000_SP:	sprintf(info->s, "S:%02X", cpustate->sp); break;
        case CPUINFO_STR_REGISTER + TMS7000_ST:	sprintf(info->s, "ST:%02X", cpustate->sr); break;
		case CPUINFO_STR_REGISTER + TMS7000_IDLE: sprintf(info->s, "Idle:%02X", cpustate->idle_state); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_CL: sprintf(info->s, "T1CL:%02X", cpustate->t1_capture_latch); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_PS: sprintf(info->s, "T1PS:%02X", cpustate->t1_prescaler & 0x1f); break;
		case CPUINFO_STR_REGISTER + TMS7000_T1_DEC: sprintf(info->s, "T1DEC:%02X", cpustate->t1_decrementer & 0xff); break;

    }
}

CPU_GET_INFO( tms7000_exl )
{
    switch( state )
    {
		case CPUINFO_FCT_EXECUTE:
			info->execute = CPU_EXECUTE_NAME(tms7000_exl);
			break;
		default:
			CPU_GET_INFO_CALL(tms7000);
			break;
	}
}

void tms7000_set_irq_line(tms7000_state *cpustate, int irqline, int state)
{
	if (cpustate->irq_state[irqline] != state)
	{	/* check for transition */
		cpustate->irq_state[irqline] = state;

		LOG(("tms7000: (cpu '%s') set_irq_line (INT%d, state %d)\n", cpustate->device->tag(), irqline+1, state));

		if (state == CLEAR_LINE)
		{
			return;
		}

		cpustate->pf[0] |= (0x02 << (irqline * 2));	/* Set INTx iocntl0 flag */

		if( irqline == TMS7000_IRQ3_LINE )
		{
			/* Latch the value in perpherial file register 3 */
			cpustate->t1_capture_latch = cpustate->t1_decrementer & 0x00ff;
		}

		tms7000_check_IRQ_lines(cpustate);
	}
}

static void tms7000_check_IRQ_lines(tms7000_state *cpustate)
{
	if( pSR & SR_I ) /* Check Global Interrupt bit: Status register, bit 4 */
	{
		if ((cpustate->irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE) || (cpustate->pf[0] & 0x02))
		{
			if( cpustate->pf[0] & 0x01 ) /* INT1 Enable bit */
			{
				tms7000_do_interrupt( cpustate, 0xfffc, TMS7000_IRQ1_LINE );
				cpustate->pf[0] &= ~0x02; /* Data Manual, page: 9-41 */
				return;
			}
		}

		if( cpustate->irq_state[ TMS7000_IRQ2_LINE ] == ASSERT_LINE )
		{
			if( cpustate->pf[0] & 0x04 ) /* INT2 Enable bit */
			{
				tms7000_do_interrupt( cpustate, 0xfffa, TMS7000_IRQ2_LINE );
				return;
			}
		}

		if ((cpustate->irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE) || (cpustate->pf[0] & 0x20))
		{
			if( cpustate->pf[0] & 0x10 ) /* INT3 Enable bit */
			{
				tms7000_do_interrupt( cpustate, 0xfff8, TMS7000_IRQ3_LINE );
				cpustate->pf[0] &= ~0x20; /* Data Manual, page: 9-41 */
				return;
			}
		}
	}
}

static void tms7000_do_interrupt( tms7000_state *cpustate, UINT16 address, UINT8 line )
{
	PUSHBYTE( pSR );		/* Push Status register */
	PUSHWORD( PC );			/* Push Program Counter */
	pSR = 0;				/* Clear Status register */
	pPC = RM16(cpustate, address);	/* Load PC with interrupt vector */

	if( cpustate->idle_state == 0 )
		cpustate->icount -= 19;		/* 19 cycles used */
	else
	{
		cpustate->icount -= 17;		/* 17 if idled */
		cpustate->idle_state = 0;
	}

	(void)(*cpustate->irq_callback)(cpustate->device, line);
}

#include "tms70op.c"
#include "tms70tb.c"

static CPU_EXECUTE( tms7000 )
{
	tms7000_state *cpustate = get_safe_token(device);
	int op;

	cpustate->div_by_16_trigger += cpustate->icount;

    tms7000_check_IRQ_lines(cpustate);

	do
	{
		debugger_instruction_hook(device, pPC);

		if( cpustate->idle_state == 0 )
		{
			op = cpustate->direct->read_decrypted_byte(pPC++);

			opfn[op](cpustate);
		}
		else
			cpustate->icount -= 16;

		/* Internal timer system */

		while( cpustate->icount < cpustate->div_by_16_trigger )
		{
			cpustate->div_by_16_trigger -= 16;

			if( (cpustate->pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
			{
				if( (cpustate->pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
					tms7000_service_timer1(device);
			}
		}

	} while( cpustate->icount > 0 );

	cpustate->div_by_16_trigger -= cpustate->icount;
}

static CPU_EXECUTE( tms7000_exl )
{
	tms7000_state *cpustate = get_safe_token(device);
	int op;

	cpustate->div_by_16_trigger += cpustate->icount;

    tms7000_check_IRQ_lines(cpustate);

	do
	{
		debugger_instruction_hook(device, pPC);

		if( cpustate->idle_state == 0 )
		{

			op = cpustate->direct->read_decrypted_byte(pPC++);

			opfn_exl[op](cpustate);
		}
		else
			cpustate->icount -= 16;

		/* Internal timer system */

		while( cpustate->icount < cpustate->div_by_16_trigger )
		{
			cpustate->div_by_16_trigger -= 16;

			if( (cpustate->pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
			{
				if( (cpustate->pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
					tms7000_service_timer1(device);
			}
		}

	} while( cpustate->icount > 0 );

	cpustate->div_by_16_trigger -= cpustate->icount;
}

/****************************************************************************
 * Trigger the event counter
 ****************************************************************************/
void tms7000_A6EC1( device_t *device )
{
	tms7000_state *cpustate = get_safe_token(device);
    if( (cpustate->pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
    {
        if( (cpustate->pf[0x03] & 0x40) == 0x40) /* Is event counter the timer source? */
            tms7000_service_timer1(device);
    }
}

static void tms7000_service_timer1( device_t *device )
{
	tms7000_state *cpustate = get_safe_token(device);
    if( --cpustate->t1_prescaler < 0 ) /* Decrement prescaler and check for underflow */
    {
        cpustate->t1_prescaler = cpustate->pf[3] & 0x1f; /* Reload prescaler (5 bit) */

        if( --cpustate->t1_decrementer < 0 ) /* Decrement timer1 register and check for underflow */
        {
            cpustate->t1_decrementer = cpustate->pf[2]; /* Reload decrementer (8 bit) */
			device->execute().set_input_line(TMS7000_IRQ2_LINE, HOLD_LINE);
            //LOG( ("tms7000: trigger int2 (cycles: %d)\t%d\tdelta %d\n", cpustate->device->total_cycles(), cpustate->device->total_cycles() - tick, cpustate->cycles_per_INT2-(cpustate->device->total_cycles() - tick) );
			//tick = cpustate->device->total_cycles() );
            /* Also, cascade out to timer 2 - timer 2 unimplemented */
        }
    }
//  LOG( ( "tms7000: service timer1. 0x%2.2x 0x%2.2x (cycles %d)\t%d\t\n", cpustate->t1_prescaler, cpustate->t1_decrementer, cpustate->device->total_cycles(), cpustate->device->total_cycles() - tick2 ) );
//  tick2 = cpustate->device->total_cycles();
}

static WRITE8_HANDLER( tms70x0_pf_w )	/* Perpherial file write */
{
	tms7000_state *cpustate = get_safe_token(&space.device());
	UINT8	temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:	/* IOCNT0, Input/Ouput control */
			temp1 = data & 0x2a;							/* Record which bits to clear */
			temp2 = cpustate->pf[0x00] & 0x2a;				/* Get copy of current bits */
			temp3 = (~temp1) & temp2;						/* Clear the requested bits */
			cpustate->pf[0x00] = temp3 | (data & (~0x2a) );	/* OR in the remaining data */
			break;
		case 0x02:
			cpustate->t1_decrementer = cpustate->pf[0x02] = data;
			cpustate->cycles_per_INT2 = 0x10*((cpustate->pf[3] & 0x1f)+1)*(cpustate->pf[0x02]+1);
			LOG( ( "tms7000: Timer adjusted. Decrementer: 0x%2.2x (Cycles per interrupt: %d)\n", cpustate->t1_decrementer, cpustate->cycles_per_INT2 ) );
			break;
		case 0x03:	/* T1CTL, timer 1 control */
			if( ((cpustate->pf[0x03] & 0x80) == 0) && ((data & 0x80) == 0x80 ) )   /* Start timer? */
			{
				cpustate->pf[0x03] = data;
				cpustate->t1_prescaler = cpustate->pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				cpustate->cycles_per_INT2 = 0x10*((cpustate->pf[3] & 0x1f)+1)*(cpustate->pf[0x02]+1);
				LOG( ( "tms7000: Timer started. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", cpustate->pf[3] & 0x1f, cpustate->cycles_per_INT2 ) );
			}
			else if( ((data & 0x80) == 0x80 ) && ((cpustate->pf[0x03] & 0x80) == 0) )   /* Timer Stopped? */
			{
				cpustate->pf[0x03] = data;
				cpustate->t1_prescaler = cpustate->pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				cpustate->cycles_per_INT2 = 0x10*((cpustate->pf[3] & 0x1f)+1)*(cpustate->pf[0x02]+1);
				LOG( ( "tms7000: Timer stopped. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", cpustate->pf[3] & 0x1f, cpustate->cycles_per_INT2 ) );
			}
			else /* Don't modify timer state, but still store data */
			{
				cpustate->pf[0x03] = data;
				cpustate->cycles_per_INT2 = 0x10*((cpustate->pf[3] & 0x1f)+1)*(cpustate->pf[0x02]+1);
				LOG( ( "tms7000: Timer adjusted. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", cpustate->pf[3] & 0x1f, cpustate->cycles_per_INT2 ) );
			}
			break;

		case 0x04: /* Port A write */
			/* Port A is read only so this is a NOP */
			break;

		case 0x06: /* Port B write */
			cpustate->io->write_byte( TMS7000_PORTB, data );
			cpustate->pf[ 0x06 ] = data;
			break;

		case 0x08: /* Port C write */
			temp1 = data & cpustate->pf[ 0x09 ];	/* Mask off input bits */
			cpustate->io->write_byte( TMS7000_PORTC, temp1 );
			cpustate->pf[ 0x08 ] = temp1;
			break;

		case 0x0a: /* Port D write */
			temp1 = data & cpustate->pf[ 0x0b ];	/* Mask off input bits */
			cpustate->io->write_byte( TMS7000_PORTD, temp1 );
			cpustate->pf[ 0x0a ] = temp1;
			break;

		default:
			/* Just stuff the other registers */
			cpustate->pf[ offset ] = data;
			break;
	}
}

static READ8_HANDLER( tms70x0_pf_r )	/* Perpherial file read */
{
	tms7000_state *cpustate = get_safe_token(&space.device());
	UINT8 result;
	UINT8	temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:	/* IOCNT0, Input/Ouput control */
			result = cpustate->pf[0x00];
			if (cpustate->irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE)
				result |= 0x02;
			if (cpustate->irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE)
				result |= 0x20;
			break;

		case 0x02:	/* T1DATA, timer 1 8-bit decrementer */
			result = (cpustate->t1_decrementer & 0x00ff);
			break;

		case 0x03:	/* T1CTL, timer 1 capture (latched by INT3) */
			result = cpustate->t1_capture_latch;
			break;

		case 0x04: /* Port A read */
			result = cpustate->io->read_byte( TMS7000_PORTA );
			break;


		case 0x06: /* Port B read */
			/* Port B is write only, return a previous written value */
			result = cpustate->pf[ 0x06 ];
			break;

		case 0x08: /* Port C read */
			temp1 = cpustate->pf[ 0x08 ] & cpustate->pf[ 0x09 ];	/* Get previous output bits */
			temp2 = cpustate->io->read_byte( TMS7000_PORTC );			/* Read port */
			temp3 = temp2 & (~cpustate->pf[ 0x09 ]);				/* Mask off output bits */
			result = temp1 | temp3;								/* OR together */
			break;

		case 0x0a: /* Port D read */
			temp1 = cpustate->pf[ 0x0a ] & cpustate->pf[ 0x0b ];	/* Get previous output bits */
			temp2 = cpustate->io->read_byte( TMS7000_PORTD );			/* Read port */
			temp3 = temp2 & (~cpustate->pf[ 0x0b ]);				/* Mask off output bits */
			result = temp1 | temp3;								/* OR together */
			break;

		default:
			/* Just unstuff the other registers */
			result = cpustate->pf[ offset ];
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
	tms7000_state *cpustate = get_safe_token(&space.device());
	cpustate->rf[ offset ] = data;
}

static READ8_HANDLER( tms7000_internal_r ) {
	tms7000_state *cpustate = get_safe_token(&space.device());
	return cpustate->rf[ offset ];
}

DEFINE_LEGACY_CPU_DEVICE(TMS7000, tms7000);
DEFINE_LEGACY_CPU_DEVICE(TMS7000_EXL, tms7000_exl);
