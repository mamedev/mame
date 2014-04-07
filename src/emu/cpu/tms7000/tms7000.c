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

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/* Static variables */

#define RM(Addr) ((unsigned)m_program->read_byte(Addr))
#define WM(Addr,Value) (m_program->write_byte(Addr, Value))

#define IMMBYTE(b)  b = ((unsigned)m_direct->read_raw_byte(pPC)); pPC++
#define SIMMBYTE(b) b = ((signed)m_direct->read_raw_byte(pPC)); pPC++
#define IMMWORD(w)  w.b.h = (unsigned)m_direct->read_raw_byte(pPC++); w.b.l = (unsigned)m_direct->read_raw_byte(pPC++)

#define PUSHBYTE(b) pSP++; WM(pSP,b)
#define PUSHWORD(w) pSP++; WM(pSP,w.b.h); pSP++; WM(pSP,w.b.l)
#define PULLBYTE(b) b = RM(pSP); pSP--
#define PULLWORD(w) w.b.l = RM(pSP); pSP--; w.b.h = RM(pSP); pSP--


const device_type TMS7000 = &device_creator<tms7000_device>;
const device_type TMS7000_EXL = &device_creator<tms7000_exl_device>;


static ADDRESS_MAP_START(tms7000_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0x0000, 0x007f)    AM_READWRITE(tms7000_internal_r, tms7000_internal_w) /* tms7000 internal RAM */
	AM_RANGE(0x0080, 0x00ff)    AM_NOP                      /* reserved */
	AM_RANGE(0x0100, 0x01ff)    AM_READWRITE(tms70x0_pf_r, tms70x0_pf_w)             /* tms7000 internal I/O ports */
ADDRESS_MAP_END


tms7000_device::tms7000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS7000, "TMS7000", tag, owner, clock, "tms7000", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, ADDRESS_MAP_NAME(tms7000_mem))
	, m_io_config("io", ENDIANNESS_BIG, 8, 8, 0)
	, m_opcode(s_opfn)
{
}


tms7000_device::tms7000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, ADDRESS_MAP_NAME(tms7000_mem))
	, m_io_config("io", ENDIANNESS_BIG, 8, 8, 0)
	, m_opcode(s_opfn_exl)
{
}


tms7000_exl_device::tms7000_exl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7000_EXL, "TMS7000_EXL", tag, owner, clock, "tms7000_exl", __FILE__)
{
}


offs_t tms7000_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms7000 );
	return CPU_DISASSEMBLE_NAME(tms7000)(this, buffer, pc, oprom, opram, options);
}


#define pPC     m_pc.w.l
#define PC      m_pc
#define pSP     m_sp
#define pSR     m_sr

#define RDA     RM(0x0000)
#define RDB     RM(0x0001)

#define WRA(Value) (WM(0x0000,Value))
#define WRB(Value) (WM(0x0001,Value))

#define SR_C    0x80        /* Carry */
#define SR_N    0x40        /* Negative */
#define SR_Z    0x20        /* Zero */
#define SR_I    0x10        /* Interrupt */

#define CLR_NZC     pSR&=~(SR_N|SR_Z|SR_C)
#define CLR_NZCI    pSR&=~(SR_N|SR_Z|SR_C|SR_I)
#define SET_C8(a)   pSR|=((a&0x0100)>>1)
#define SET_N8(a)   pSR|=((a&0x0080)>>1)
#define SET_Z(a)    if(!a)pSR|=SR_Z
#define SET_Z8(a)   SET_Z((UINT8)a)
#define SET_Z16(a)  SET_Z((UINT8)a>>8)
#define GET_C       (pSR >> 7)

/* Not working */
#define SET_C16(a)  pSR|=((a&0x010000)>>9)

#define SETC        pSR |= SR_C
#define SETZ        pSR |= SR_Z
#define SETN        pSR |= SR_N


UINT16 tms7000_device::RM16( UINT32 mAddr ) /* Read memory (16-bit) */
{
	UINT32 result = RM(mAddr) << 8;
	return result | RM((mAddr+1)&0xffff);
}

UINT16 tms7000_device::RRF16( UINT32 mAddr )    /*Read register file (16 bit) */
{
	PAIR result;
	result.b.h = RM((mAddr-1)&0xffff);
	result.b.l = RM(mAddr);
	return result.w.l;
}

void tms7000_device::WRF16( UINT32 mAddr, PAIR p )  /*Write register file (16 bit) */
{
	WM( (mAddr-1)&0xffff, p.b.h );
	WM( mAddr, p.b.l );
}


void tms7000_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	memset(m_pf, 0, 0x100);
	memset(m_rf, 0, 0x80);
	m_cycles_per_INT2 = 0;
	m_t1_capture_latch = 0;
	m_t1_prescaler = 0;
	m_t1_decrementer = 0;

	/* Save register state */
	save_item(NAME(pPC));
	save_item(NAME(pSP));
	save_item(NAME(pSR));

	/* Save Interrupt state */
	save_item(NAME(m_irq_state));

	/* Save register and perpherial file state */
	save_item(NAME(m_rf));
	save_item(NAME(m_pf));

	/* Save timer state */
	save_item(NAME(m_t1_prescaler));
	save_item(NAME(m_t1_capture_latch));
	save_item(NAME(m_t1_decrementer));

	save_item(NAME(m_idle_state));

	state_add( TMS7000_PC,     "PC",    m_pc.w.l).formatstr("%04X");
	state_add( TMS7000_SP,     "S",     m_sp).formatstr("%02X");
	state_add( TMS7000_ST,     "ST",    m_sr).formatstr("%02X");
	state_add( TMS7000_IDLE,   "Idle",  m_idle_state).formatstr("%02X");
	state_add( TMS7000_T1_CL,  "T1CL",  m_t1_capture_latch).formatstr("%02X");
	state_add( TMS7000_T1_PS,  "T1PS",  m_t1_prescaler).mask(0x1f).formatstr("%02X");
	state_add( TMS7000_T1_DEC, "T1DEC", m_t1_decrementer).mask(0xff).formatstr("%02X");

	state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENSP, "GENSP", m_sp).formatstr("%02X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_sr).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}

void tms7000_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
						m_sr & 0x80 ? 'C':'c',
						m_sr & 0x40 ? 'N':'n',
						m_sr & 0x20 ? 'Z':'z',
						m_sr & 0x10 ? 'I':'i',
						m_sr & 0x08 ? '?':'.',
						m_sr & 0x04 ? '?':'.',
						m_sr & 0x02 ? '?':'.',
						m_sr & 0x01 ? '?':'.'
			);
			break;
	}
}

void tms7000_device::device_reset()
{
//  m_architecture = (int)param;

	m_idle_state = 0;
	m_irq_state[ TMS7000_IRQ1_LINE ] = CLEAR_LINE;
	m_irq_state[ TMS7000_IRQ2_LINE ] = CLEAR_LINE;
	m_irq_state[ TMS7000_IRQ3_LINE ] = CLEAR_LINE;

	WM( 0x100 + 9, 0 );     /* Data direction regs are cleared */
	WM( 0x100 + 11, 0 );

//  if( m_architecture == TMS7000_NMOS )
//  {
		WM( 0x100 + 4, 0xff );      /* Output 0xff on port A */
		WM( 0x100 + 8, 0xff );      /* Output 0xff on port C */
		WM( 0x100 + 10, 0xff );     /* Output 0xff on port D */
//  }
//  else
//  {
//      WM( 0x100 + 4, 0xff );      /* Output 0xff on port A */
//  }

	pSP = 0x01;             /* Set stack pointer to r1 */
	pSR = 0x00;             /* Clear status register (disabling interrupts */
	WM( 0x100 + 0, 0 );     /* Write a zero to IOCNT0 */

	/* On TMS70x2 and TMS70Cx2 IOCNT1 is zero */

	WRA( m_pc.b.h );    /* Write previous PC to A:B */
	WRB( m_pc.b.l );
	pPC = RM16(0xfffe);       /* Load reset vector */

	m_div_by_16_trigger = -16;
}

void tms7000_device::execute_set_input(int irqline, int state)
{
	if (m_irq_state[irqline] != state)
	{   /* check for transition */
		m_irq_state[irqline] = state;

		LOG(("tms7000: (cpu '%s') set_irq_line (INT%d, state %d)\n", tag(), irqline+1, state));

		if (state == CLEAR_LINE)
		{
			return;
		}

		m_pf[0] |= (0x02 << (irqline * 2)); /* Set INTx iocntl0 flag */

		if( irqline == TMS7000_IRQ3_LINE )
		{
			/* Latch the value in perpherial file register 3 */
			m_t1_capture_latch = m_t1_decrementer & 0x00ff;
		}

		tms7000_check_IRQ_lines();
	}
}

void tms7000_device::tms7000_check_IRQ_lines()
{
	if( pSR & SR_I ) /* Check Global Interrupt bit: Status register, bit 4 */
	{
		if ((m_irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE) || (m_pf[0] & 0x02))
		{
			if( m_pf[0] & 0x01 ) /* INT1 Enable bit */
			{
				tms7000_do_interrupt( 0xfffc, TMS7000_IRQ1_LINE );
				m_pf[0] &= ~0x02; /* Data Manual, page: 9-41 */
				return;
			}
		}

		if( m_irq_state[ TMS7000_IRQ2_LINE ] == ASSERT_LINE )
		{
			if( m_pf[0] & 0x04 ) /* INT2 Enable bit */
			{
				tms7000_do_interrupt( 0xfffa, TMS7000_IRQ2_LINE );
				return;
			}
		}

		if ((m_irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE) || (m_pf[0] & 0x20))
		{
			if( m_pf[0] & 0x10 ) /* INT3 Enable bit */
			{
				tms7000_do_interrupt( 0xfff8, TMS7000_IRQ3_LINE );
				m_pf[0] &= ~0x20; /* Data Manual, page: 9-41 */
				return;
			}
		}
	}
}

void tms7000_device::tms7000_do_interrupt( UINT16 address, UINT8 line )
{
	PUSHBYTE( pSR );        /* Push Status register */
	PUSHWORD( PC );         /* Push Program Counter */
	pSR = 0;                /* Clear Status register */
	pPC = RM16(address);  /* Load PC with interrupt vector */

	if( m_idle_state == 0 )
		m_icount -= 19;     /* 19 cycles used */
	else
	{
		m_icount -= 17;     /* 17 if idled */
		m_idle_state = 0;
	}

	standard_irq_callback(line);
}

#include "tms70op.inc"
#include "tms70tb.inc"

void tms7000_device::execute_run()
{
	int op;

	m_div_by_16_trigger += m_icount;

	tms7000_check_IRQ_lines();

	do
	{
		debugger_instruction_hook(this, pPC);

		if( m_idle_state == 0 )
		{
			op = m_direct->read_decrypted_byte(pPC++);

			(this->*m_opcode[op])();
		}
		else
			m_icount -= 16;

		/* Internal timer system */

		while( m_icount < m_div_by_16_trigger )
		{
			m_div_by_16_trigger -= 16;

			if( (m_pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
			{
				if( (m_pf[0x03] & 0x40) != 0x40) /* Is system clock (divided by 16) the timer source? */
					tms7000_service_timer1();
			}
		}

	} while( m_icount > 0 );

	m_div_by_16_trigger -= m_icount;
}


/****************************************************************************
 * Trigger the event counter
 ****************************************************************************/
void tms7000_device::tms7000_A6EC1()
{
	if( (m_pf[0x03] & 0x80) == 0x80 ) /* Is timer system active? */
	{
		if( (m_pf[0x03] & 0x40) == 0x40) /* Is event counter the timer source? */
			tms7000_service_timer1();
	}
}

void tms7000_device::tms7000_service_timer1()
{
	if( --m_t1_prescaler < 0 ) /* Decrement prescaler and check for underflow */
	{
		m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */

		if( --m_t1_decrementer < 0 ) /* Decrement timer1 register and check for underflow */
		{
			m_t1_decrementer = m_pf[2]; /* Reload decrementer (8 bit) */
			set_input_line(TMS7000_IRQ2_LINE, HOLD_LINE);
			//LOG( ("tms7000: trigger int2 (cycles: %d)\t%d\tdelta %d\n", total_cycles(), total_cycles() - tick, m_cycles_per_INT2-(total_cycles() - tick) );
			//tick = total_cycles() );
			/* Also, cascade out to timer 2 - timer 2 unimplemented */
		}
	}
//  LOG( ( "tms7000: service timer1. 0x%2.2x 0x%2.2x (cycles %d)\t%d\t\n", m_t1_prescaler, m_t1_decrementer, total_cycles(), total_cycles() - tick2 ) );
//  tick2 = total_cycles();
}

WRITE8_MEMBER( tms7000_device::tms70x0_pf_w )   /* Perpherial file write */
{
	UINT8   temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:  /* IOCNT0, Input/Ouput control */
			temp1 = data & 0x2a;                            /* Record which bits to clear */
			temp2 = m_pf[0x00] & 0x2a;              /* Get copy of current bits */
			temp3 = (~temp1) & temp2;                       /* Clear the requested bits */
			m_pf[0x00] = temp3 | (data & (~0x2a) ); /* OR in the remaining data */
			break;
		case 0x02:
			m_t1_decrementer = m_pf[0x02] = data;
			m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
			LOG( ( "tms7000: Timer adjusted. Decrementer: 0x%2.2x (Cycles per interrupt: %d)\n", m_t1_decrementer, m_cycles_per_INT2 ) );
			break;
		case 0x03:  /* T1CTL, timer 1 control */
			if( ((m_pf[0x03] & 0x80) == 0) && ((data & 0x80) == 0x80 ) )   /* Start timer? */
			{
				m_pf[0x03] = data;
				m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
				LOG( ( "tms7000: Timer started. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
			}
			else if( ((data & 0x80) == 0x80 ) && ((m_pf[0x03] & 0x80) == 0) )   /* Timer Stopped? */
			{
				m_pf[0x03] = data;
				m_t1_prescaler = m_pf[3] & 0x1f; /* Reload prescaler (5 bit) */
				m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
				LOG( ( "tms7000: Timer stopped. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
			}
			else /* Don't modify timer state, but still store data */
			{
				m_pf[0x03] = data;
				m_cycles_per_INT2 = 0x10*((m_pf[3] & 0x1f)+1)*(m_pf[0x02]+1);
				LOG( ( "tms7000: Timer adjusted. Prescaler: 0x%2.2x (Cycles per interrupt: %d)\n", m_pf[3] & 0x1f, m_cycles_per_INT2 ) );
			}
			break;

		case 0x04: /* Port A write */
			/* Port A is read only so this is a NOP */
			break;

		case 0x06: /* Port B write */
			m_io->write_byte( TMS7000_PORTB, data );
			m_pf[ 0x06 ] = data;
			break;

		case 0x08: /* Port C write */
			temp1 = data & m_pf[ 0x09 ];    /* Mask off input bits */
			m_io->write_byte( TMS7000_PORTC, temp1 );
			m_pf[ 0x08 ] = temp1;
			break;

		case 0x0a: /* Port D write */
			temp1 = data & m_pf[ 0x0b ];    /* Mask off input bits */
			m_io->write_byte( TMS7000_PORTD, temp1 );
			m_pf[ 0x0a ] = temp1;
			break;

		default:
			/* Just stuff the other registers */
			m_pf[ offset ] = data;
			break;
	}
}

READ8_MEMBER( tms7000_device::tms70x0_pf_r )    /* Perpherial file read */
{
	UINT8 result;
	UINT8   temp1, temp2, temp3;

	switch( offset )
	{
		case 0x00:  /* IOCNT0, Input/Ouput control */
			result = m_pf[0x00];
			if (m_irq_state[TMS7000_IRQ1_LINE] == ASSERT_LINE)
				result |= 0x02;
			if (m_irq_state[TMS7000_IRQ3_LINE] == ASSERT_LINE)
				result |= 0x20;
			break;

		case 0x02:  /* T1DATA, timer 1 8-bit decrementer */
			result = (m_t1_decrementer & 0x00ff);
			break;

		case 0x03:  /* T1CTL, timer 1 capture (latched by INT3) */
			result = m_t1_capture_latch;
			break;

		case 0x04: /* Port A read */
			result = m_io->read_byte( TMS7000_PORTA );
			break;


		case 0x06: /* Port B read */
			/* Port B is write only, return a previous written value */
			result = m_pf[ 0x06 ];
			break;

		case 0x08: /* Port C read */
			temp1 = m_pf[ 0x08 ] & m_pf[ 0x09 ];    /* Get previous output bits */
			temp2 = m_io->read_byte( TMS7000_PORTC );           /* Read port */
			temp3 = temp2 & (~m_pf[ 0x09 ]);                /* Mask off output bits */
			result = temp1 | temp3;                             /* OR together */
			break;

		case 0x0a: /* Port D read */
			temp1 = m_pf[ 0x0a ] & m_pf[ 0x0b ];    /* Get previous output bits */
			temp2 = m_io->read_byte( TMS7000_PORTD );           /* Read port */
			temp3 = temp2 & (~m_pf[ 0x0b ]);                /* Mask off output bits */
			result = temp1 | temp3;                             /* OR together */
			break;

		default:
			/* Just unstuff the other registers */
			result = m_pf[ offset ];
			break;
	}

	return result;
}

// BCD arthrimetic handling
UINT16 tms7000_device::bcd_add( UINT16 a, UINT16 b )
{
	UINT16  t1,t2,t3,t4,t5,t6;

	/* Sure it is a lot of code, but it works! */
	t1 = a + 0x0666;
	t2 = t1 + b;
	t3 = t1 ^ b;
	t4 = t2 ^ t3;
	t5 = ~t4 & 0x1110;
	t6 = (t5 >> 2) | (t5 >> 3);
	return t2-t6;
}

UINT16 tms7000_device::bcd_tencomp( UINT16 a )
{
	UINT16  t1,t2,t3,t4,t5,t6;

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
UINT16 tms7000_device::bcd_sub( UINT16 a, UINT16 b)
{
	//return bcd_tencomp(b) - bcd_tencomp(a);
	return bcd_add(a, bcd_tencomp(b) & 0xff);
}

WRITE8_MEMBER( tms7000_device::tms7000_internal_w )
{
	m_rf[ offset ] = data;
}

READ8_MEMBER( tms7000_device::tms7000_internal_r )
{
	return m_rf[ offset ];
}
