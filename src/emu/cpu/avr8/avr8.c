/*
    Atmel 8-bit AVR simulator

    - Notes -
      Cycle counts are generally considered to be 100% accurate per-instruction, does not support mid-instruction
      interrupts although no software has been countered yet that requires it. Evidence of cycle accuracy is given
      in the form of the demoscene 'wild' demo, Craft, by [lft], which uses an ATmega88 to write video out a 6-bit
      RGB DAC pixel-by-pixel, synchronously with the frame timing. Intentionally modifying the timing of any of
      the existing opcodes has been shown to wildly corrupt the video output in Craft, so one can assume that the
      existing timing is 100% correct.

	  Unimplemented opcodes: CPSR, LD Z+, ST Z+, ST -Z/-Y/-X, ELPM, SPM, SPM Z+, EIJMP, SLEEP, BREAK, WDR, ICALL,
	                         EICALL, JMP, CALL, SBIW

	- Changelist -
	  30 Oct. 2012
	  - Added FMUL, FMULS, FMULSU opcodes [MooglyGuy]
	  - Fixed incorrect flag calculation in ROR opcode [MooglyGuy]
	  - Fixed incorrect bit testing in SBIC/SBIS opcodes [MooglyGuy]

	  25 Oct. 2012
	  - Added MULS, ANDI, STI Z+, LD -Z, LD -Y, LD -X, LD Y+q, LD Z+q, SWAP, ASR, ROR and SBIS opcodes [MooglyGuy]
	  - Corrected cycle counts for LD and ST opcodes [MooglyGuy]
	  - Moved opcycles init into inner while loop, fixes 2-cycle and 3-cycle opcodes effectively forcing
	    all subsequent 1-cycle opcodes to be 2 or 3 cycles [MooglyGuy]
	  - Fixed register behavior in MULSU, LD -Z, and LD -Y opcodes [MooglyGuy]

	  18 Oct. 2012
	  - Added OR, SBCI, ORI, ST Y+, ADIQ opcodes [MooglyGuy]
	  - Fixed COM, NEG, LSR opcodes [MooglyGuy]

*/

#include "emu.h"
#include "debugger.h"
#include "avr8.h"

#define VERBOSE_LEVEL	(0)

#define ENABLE_VERBOSE_LOG (0)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(UINT16 pc, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%05x: %s", pc << 1, buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif

//**************************************************************************
//  ENUMS AND MACROS
//**************************************************************************

enum
{
    AVR8_SREG_C = 0,
    AVR8_SREG_Z,
    AVR8_SREG_N,
    AVR8_SREG_V,
    AVR8_SREG_S,
    AVR8_SREG_H,
    AVR8_SREG_T,
    AVR8_SREG_I,
};

// I/O Enums
enum
{
	WGM1_NORMAL = 0,
	WGM1_PWM_8_PC,
	WGM1_PWM_9_PC,
	WGM1_PWM_10_PC,
	WGM1_CTC_OCR,
	WGM1_FAST_PWM_8,
	WGM1_FAST_PWM_9,
	WGM1_FAST_PWM_10,
	WGM1_PWM_PFC_ICR,
	WGM1_PWM_PFC_OCR,
	WGM1_PWM_PC_ICR,
	WGM1_PWM_PC_OCR,
	WGM1_CTC_ICR,
	WGM1_RESERVED,
	WGM1_FAST_PWM_ICR,
	WGM1_FAST_PWM_OCR
};

enum
{
	WGM2_NORMAL = 0,
	WGM2_PWM_PC,
	WGM2_CTC_CMP,
	WGM2_FAST_PWM,
	WGM2_RESERVED0,
	WGM2_PWM_PC_CMP,
	WGM2_RESERVED1,
	WGM2_FAST_PWM_CMP
};

static const char avr8_reg_name[4] = { 'A', 'B', 'C', 'D' };

#define SREG_R(b) ((m_status & (1 << (b))) >> (b))
#define SREG_W(b,v) m_status = (m_status & ~(1 << (b))) | ((v) << (b))
#define NOT(x) (1 - (x))

// Opcode-Parsing Defines
#define RD2(op)         (((op) >> 4) & 0x0003)
#define RD3(op)         (((op) >> 4) & 0x0007)
#define RD4(op)         (((op) >> 4) & 0x000f)
#define RD5(op)         (((op) >> 4) & 0x001f)
#define RR3(op)         ((op) & 0x0007)
#define RR4(op)         ((op) & 0x000f)
#define RR5(op)         ((((op) >> 5) & 0x0010) | ((op) & 0x000f))
#define DCONST(op)      (((op) >> 4) & 0x0003)
#define KCONST6(op)     ((((op) >> 2) & 0x0030) | ((op) & 0x000f))
#define KCONST7(op)     (((op) >> 3) & 0x007f)
#define KCONST8(op)     ((((op) >> 4) & 0x00f0) | ((op) & 0x000f))
#define KCONST22(op)    (((((UINT32)(op) >> 3) & 0x003e) | ((UINT32)(op) & 0x0001)) << 16)
#define QCONST6(op)     ((((op) >> 8) & 0x0020) | (((op) >> 7) & 0x0018) | ((op) & 0x0007))
#define ACONST5(op)     (((op) >> 3) & 0x001f)
#define ACONST6(op)     ((((op) >> 5) & 0x0030) | ((op) & 0x000f))
#define MULCONST2(op)	((((op) >> 6) & 0x0002) | (((op) >> 3) & 0x0001))

// Register Defines
#define XREG            ((m_r[27] << 8) | m_r[26])
#define YREG            ((m_r[29] << 8) | m_r[28])
#define ZREG            ((m_r[31] << 8) | m_r[30])
#define SPREG			((m_r[AVR8_REGIDX_SPH] << 8) | m_r[AVR8_REGIDX_SPL])

// I/O Defines
#define AVR8_OCR1BH				(m_r[AVR8_REGIDX_OCR1BH])
#define AVR8_OCR1BL				(m_r[AVR8_REGIDX_OCR1BL])
#define AVR8_OCR1AH				(m_r[AVR8_REGIDX_OCR1AH])
#define AVR8_OCR1AL				(m_r[AVR8_REGIDX_OCR1AL])
#define AVR8_ICR1H				(m_r[AVR8_REGIDX_ICR1H])
#define AVR8_ICR1L				(m_r[AVR8_REGIDX_ICR1L])
#define AVR8_TCNT1H				(m_r[AVR8_REGIDX_TCNT1H])
#define AVR8_TCNT1L				(m_r[AVR8_REGIDX_TCNT1L])

#define AVR8_TCCR1B					(m_r[AVR8_REGIDX_TCCR1B])
#define AVR8_TCCR1B_ICNC1_MASK		0x80
#define AVR8_TCCR1B_ICNC1_SHIFT		7
#define AVR8_TCCR1B_ICES1_MASK		0x40
#define AVR8_TCCR1B_ICES1_SHIFT		6
#define AVR8_TCCR1B_WGM1_32_MASK	0x18
#define AVR8_TCCR1B_WGM1_32_SHIFT	3
#define AVR8_TCCR1B_CS_MASK			0x07
#define AVR8_TCCR1B_CS_SHIFT		0
#define AVR8_TIMER1_CLOCK_SELECT	(AVR8_TCCR1B & AVR8_TCCR1B_CS_MASK)

#define AVR8_TCCR1A					(m_r[AVR8_REGIDX_TCCR1A])
#define AVR8_TCCR1A_COM1A_MASK		0xc0
#define AVR8_TCCR1A_COM1A_SHIFT		6
#define AVR8_TCCR1A_COM1B_MASK		0x30
#define AVR8_TCCR1A_COM1B_SHIFT		4
#define AVR8_TCCR1A_WGM1_10_MASK	0x03
#define AVR8_TCCR1A_WGM1_10_SHIFT	0
#define AVR8_TCCR1A_COM1A			((AVR8_TCCR1A & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT)
#define AVR8_TCCR1A_COM1B			((AVR8_TCCR1A & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT)
#define AVR8_TCCR1A_WGM1_10			(AVR8_TCCR1A & AVR8_TCCR1A_WGM1_10_MASK)

#define AVR8_TIMSK1				(m_r[AVR8_REGIDX_TIMSK1])
#define AVR8_TIMSK1_ICIE1_MASK	0x20
#define AVR8_TIMSK1_OCIE1B_MASK	0x04
#define AVR8_TIMSK1_OCIE1A_MASK	0x02
#define AVR8_TIMSK1_TOIE1_MASK	0x01
#define AVR8_TIMSK1_ICIE1		((AVR8_TIMSK1 & AVR8_TIMSK1_ICIE1_MASK) >> 5)
#define AVR8_TIMSK1_OCIE1B		((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1B_MASK) >> 2)
#define AVR8_TIMSK1_OCIE1A		((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1A_MASK) >> 1)
#define AVR8_TIMSK1_TOIE1		(AVR8_TIMSK1 & AVR8_TIMSK1_TOIE1_MASK)

#define AVR8_TIFR1				(m_r[AVR8_REGIDX_TIFR1])
#define AVR8_TIFR1_ICF1_MASK	0x20
#define AVR8_TIFR1_ICF1_SHIFT	5
#define AVR8_TIFR1_OCF1B_MASK	0x04
#define AVR8_TIFR1_OCF1B_SHIFT	2
#define AVR8_TIFR1_OCF1A_MASK	0x02
#define AVR8_TIFR1_OCF1A_SHIFT	1
#define AVR8_TIFR1_TOV1_MASK	0x01
#define AVR8_TIFR1_TOV1_SHIFT	0
#define AVR8_TIFR1_MASK			(AVR8_TIFR1_ICF1_MASK | AVR8_TIFR1_TOV1_MASK | \
								 AVR8_TIFR1_OCF1B_MASK | AVR8_TIFR1_OCF1A_MASK)

#define AVR8_TCCR2B					(m_r[AVR8_REGIDX_TCCR1B])
#define AVR8_TCCR2B_FOC2A_MASK		0x80
#define AVR8_TCCR2B_FOC2A_SHIFT		7
#define AVR8_TCCR2B_FOC2B_MASK		0x40
#define AVR8_TCCR2B_FOC2B_SHIFT		6
#define AVR8_TCCR2B_WGM2_2_MASK		0x08
#define AVR8_TCCR2B_WGM2_2_SHIFT	3
#define AVR8_TCCR2B_CS_MASK			0x07
#define AVR8_TCCR2B_CS_SHIFT		0
#define AVR8_TIMER2_CLOCK_SELECT	(AVR8_TCCR2B & AVR8_TCCR2B_CS_MASK)

#define AVR8_TCCR2A					(m_r[AVR8_REGIDX_TCCR1A])
#define AVR8_TCCR2A_COM2A_MASK		0xc0
#define AVR8_TCCR2A_COM2A_SHIFT		6
#define AVR8_TCCR2A_COM2B_MASK		0x30
#define AVR8_TCCR2A_COM2B_SHIFT		4
#define AVR8_TCCR2A_WGM2_10_MASK	0x03
#define AVR8_TCCR2A_WGM2_10_SHIFT	0
#define AVR8_TCCR2A_COM2A			((AVR8_TCCR2A & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT)
#define AVR8_TCCR2A_COM2B			((AVR8_TCCR2A & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT)
#define AVR8_TCCR2A_WGM2_10			(AVR8_TCCR2A & AVR8_TCCR1A_WGM2_10_MASK)

#define AVR8_TIMSK2				(m_r[AVR8_REGIDX_TIMSK2])
#define AVR8_TIMSK2_OCIE2B_MASK	0x04
#define AVR8_TIMSK2_OCIE2A_MASK	0x02
#define AVR8_TIMSK2_TOIE2_MASK	0x01
#define AVR8_TIMSK2_OCIE2B		((AVR8_TIMSK2 & AVR8_TIMSK1_OCIE2B_MASK) >> 2)
#define AVR8_TIMSK2_OCIE2A		((AVR8_TIMSK2 & AVR8_TIMSK1_OCIE2A_MASK) >> 1)
#define AVR8_TIMSK2_TOIE2		(AVR8_TIMSK2 & AVR8_TIMSK1_TOIE2_MASK)

#define AVR8_TIFR2				(m_r[AVR8_REGIDX_TIFR2])
#define AVR8_TIFR2_OCF2B_MASK	0x04
#define AVR8_TIFR2_OCF2B_SHIFT	2
#define AVR8_TIFR2_OCF2A_MASK	0x02
#define AVR8_TIFR2_OCF2A_SHIFT	1
#define AVR8_TIFR2_TOV2_MASK	0x01
#define AVR8_TIFR2_TOV2_SHIFT	0
#define AVR8_TIFR2_MASK			(AVR8_TIFR2_TOV2_MASK | AVR8_TIFR2_OCF2B_MASK | AVR8_TIFR2_OCF2A_MASK)

#define AVR8_OCR1A				((AVR8_OCR1AH << 8) | AVR8_OCR1AL)
#define AVR8_OCR1B				((AVR8_OCR1BH << 8) | AVR8_OCR1BL)
#define AVR8_ICR1				((AVR8_ICR1H  << 8) | AVR8_ICR1L)
#define AVR8_TCNT1				((AVR8_TCNT1H << 8) | AVR8_TCNT1L)
#define AVR8_WGM1				(((AVR8_TCCR1B & 0x18) >> 1) | (AVR8_TCCR1A & 0x03))
#define AVR8_TCNT1_DIR			(state->m_tcnt1_direction)

#define AVR8_OCR2B				m_r[AVR8_REGIDX_OCR2B]
#define AVR8_OCR2A				m_r[AVR8_REGIDX_OCR2A]
#define AVR8_TCNT2				m_r[AVR8_REGIDX_TCNT2]
#define AVR8_WGM2				(((AVR8_TCCR2B & 0x08) >> 1) | (AVR8_TCCR2A & 0x03))

#define AVR8_GTCCR_PSRASY_MASK	0x02
#define AVR8_GTCCR_PSRASY_SHIFT	1

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type ATMEGA88 = &device_creator<atmega88_device>;
const device_type ATMEGA644 = &device_creator<atmega644_device>;

//-------------------------------------------------
//  atmega8_device - constructor
//-------------------------------------------------

avr8_device::avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, UINT32 addr_mask)
	: cpu_device(mconfig, type, "AVR8", tag, owner, clock),
	  m_program_config("program", ENDIANNESS_LITTLE, 8, 22),
	  m_io_config("io", ENDIANNESS_LITTLE, 8, 16),
      m_pc(0),
      m_debugger_pc(0),
      m_status(0),
      m_timer0_top(0),
      m_timer0_increment(1),
      m_timer0_prescale(0),
      m_timer0_prescale_count(0),
      m_timer1_top(0),
      m_timer1_increment(1),
      m_timer1_prescale(0),
      m_timer1_prescale_count(0),
      m_timer2_top(0),
      m_timer2_increment(1),
      m_timer2_prescale(0),
      m_timer2_prescale_count(0),
      m_addr_mask(addr_mask),
      m_interrupt_pending(false),
      m_icount(0),
      m_elapsed_cycles(0)
{
    // Allocate & setup
}


//-------------------------------------------------
//  static_set_config - set the configuration
//  structure
//-------------------------------------------------

void avr8_device::static_set_config(device_t &device, const avr8_config &config)
{
	avr8_device &avr8 = downcast<avr8_device &>(device);
	static_cast<avr8_config &>(avr8) = config;
}


//-------------------------------------------------
//  unimplemented_opcode - bail on unspuported
//  instruction
//-------------------------------------------------

void avr8_device::unimplemented_opcode(UINT32 op)
{
    fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, m_pc);
}


//-------------------------------------------------
//  is_long_opcode - returns true if opcode is 4
//  bytes long
//-------------------------------------------------

inline bool avr8_device::is_long_opcode(UINT16 op)
{
	if((op & 0xf000) == 0x9000)
	{
		if((op & 0x0f00) < 0x0400)
		{
			if((op & 0x000f) == 0x0000)
			{
				return true;
			}
		}
		else if((op & 0x0f00) < 0x0600)
		{
			if((op & 0x000f) >= 0x000c)
			{
				return true;
			}
		}
	}
	return false;
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void avr8_device::device_start()
{
    m_pc = 0;

    m_program = &space(AS_PROGRAM);
    m_io = &space(AS_IO);

	// register our state for the debugger
	astring tempstr;
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_status).callimport().callexport().formatstr("%8s").noshow();
	state_add(AVR8_SREG,       "STATUS",    m_r[AVR8_REGIDX_SREG]).mask(0xff);
	state_add(AVR8_PC,         "PC",        m_debugger_pc).mask(0xffff);
	state_add(AVR8_R0,         "R0",        m_r[ 0]).mask(0xff);
	state_add(AVR8_R1,         "R1",        m_r[ 1]).mask(0xff);
	state_add(AVR8_R2,         "R2",        m_r[ 2]).mask(0xff);
	state_add(AVR8_R3,         "R3",        m_r[ 3]).mask(0xff);
	state_add(AVR8_R4,         "R4",        m_r[ 4]).mask(0xff);
	state_add(AVR8_R5,         "R5",        m_r[ 5]).mask(0xff);
	state_add(AVR8_R6,         "R6",        m_r[ 6]).mask(0xff);
	state_add(AVR8_R7,         "R7",        m_r[ 7]).mask(0xff);
	state_add(AVR8_R8,         "R8",        m_r[ 8]).mask(0xff);
	state_add(AVR8_R9,         "R9",        m_r[ 9]).mask(0xff);
	state_add(AVR8_R10,        "R10",       m_r[10]).mask(0xff);
	state_add(AVR8_R11,        "R11",       m_r[11]).mask(0xff);
	state_add(AVR8_R12,        "R12",       m_r[12]).mask(0xff);
	state_add(AVR8_R13,        "R13",       m_r[13]).mask(0xff);
	state_add(AVR8_R14,        "R14",       m_r[14]).mask(0xff);
	state_add(AVR8_R15,        "R15",       m_r[15]).mask(0xff);
	state_add(AVR8_R16,        "R16",       m_r[16]).mask(0xff);
	state_add(AVR8_R17,        "R17",       m_r[17]).mask(0xff);
	state_add(AVR8_R18,        "R18",       m_r[18]).mask(0xff);
	state_add(AVR8_R19,        "R19",       m_r[19]).mask(0xff);
	state_add(AVR8_R20,        "R20",       m_r[20]).mask(0xff);
	state_add(AVR8_R21,        "R21",       m_r[21]).mask(0xff);
	state_add(AVR8_R22,        "R22",       m_r[22]).mask(0xff);
	state_add(AVR8_R23,        "R23",       m_r[23]).mask(0xff);
	state_add(AVR8_R24,        "R24",       m_r[24]).mask(0xff);
	state_add(AVR8_R25,        "R25",       m_r[25]).mask(0xff);
	state_add(AVR8_R26,        "R26",       m_r[26]).mask(0xff);
	state_add(AVR8_R27,        "R27",       m_r[27]).mask(0xff);
	state_add(AVR8_R28,        "R28",       m_r[28]).mask(0xff);
	state_add(AVR8_R29,        "R29",       m_r[29]).mask(0xff);
	state_add(AVR8_R30,        "R30",       m_r[30]).mask(0xff);
	state_add(AVR8_R31,        "R31",       m_r[31]).mask(0xff);

	// register our state for saving
	save_item(NAME(m_pc));
	save_item(NAME(m_status));
	save_item(NAME(m_r));
	save_item(NAME(m_timer0_top));
	save_item(NAME(m_timer0_increment));
	save_item(NAME(m_timer0_prescale));
	save_item(NAME(m_timer0_prescale_count));
	save_item(NAME(m_timer1_top));
	save_item(NAME(m_timer1_increment));
	save_item(NAME(m_timer1_prescale));
	save_item(NAME(m_timer1_prescale_count));
	save_item(NAME(m_timer2_top));
	save_item(NAME(m_timer2_increment));
	save_item(NAME(m_timer2_prescale));
	save_item(NAME(m_timer2_prescale_count));
	save_item(NAME(m_addr_mask));
	save_item(NAME(m_interrupt_pending));
	save_item(NAME(m_icount));
	save_item(NAME(m_elapsed_cycles));

	// set our instruction counter
	m_icountptr = &m_icount;
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void avr8_device::device_reset()
{
	m_status = 0;
    io_write8(AVR8_REGIDX_SPL, 0);
    io_write8(AVR8_REGIDX_SPH, 0);

	for (int i = 0; i < 32; i++)
	{
		m_r[i] = 0;
	}

    m_timer0_top = 0;
	m_timer0_increment = 1;
	m_timer0_prescale = 0;
	m_timer0_prescale_count = 0;

    m_timer1_top = 0;
	m_timer1_increment = 1;
	m_timer1_prescale = 0;
	m_timer1_prescale_count = 0;

    m_timer2_top = 0;
	m_timer2_increment = 1;
	m_timer2_prescale = 0;
	m_timer2_prescale_count = 0;

    AVR8_TIMSK1 = 0;
    AVR8_OCR1AH = 0;
    AVR8_OCR1AL = 0;
    AVR8_OCR1BH = 0;
    AVR8_OCR1BL = 0;
    AVR8_ICR1H = 0;
    AVR8_ICR1L = 0;
    AVR8_TCNT1H = 0;
    AVR8_TCNT1L = 0;
    AVR8_TCNT2 = 0;

	m_interrupt_pending = false;

	m_elapsed_cycles = 0;
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *avr8_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	else if (spacenum == AS_IO)
	{
		return &m_io_config;
	}
	return NULL;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void avr8_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
			    (m_status & 0x80) ? 'I' : '-',
			    (m_status & 0x40) ? 'T' : '-',
			    (m_status & 0x20) ? 'H' : '-',
			    (m_status & 0x10) ? 'S' : '-',
			    (m_status & 0x08) ? 'V' : '-',
			    (m_status & 0x04) ? 'N' : '-',
			    (m_status & 0x02) ? 'Z' : '-',
			    (m_status & 0x01) ? 'C' : '-');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 avr8_device::disasm_min_opcode_bytes() const
{
	return 2;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 avr8_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t avr8_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( avr8 );
	return CPU_DISASSEMBLE_NAME(avr8)(NULL, buffer, pc, oprom, opram, 0);
}


//**************************************************************************
//  MEMORY ACCESSORS
//**************************************************************************

inline UINT8 avr8_device::program_read8(UINT32 address)
{
    return m_program->read_byte(address);
}

inline UINT16 avr8_device::program_read16(UINT32 address)
{
    return m_program->read_word(address << 1);
}

inline void avr8_device::program_write8(UINT32 address, UINT8 data)
{
    m_program->write_byte(address, data);
}

inline void avr8_device::program_write16(UINT32 address, UINT16 data)
{
    m_program->write_word(address, data);
}

inline UINT8 avr8_device::io_read8(UINT16 address)
{
	if (address < 0x100)
	{
		// Allow unhandled internal registers to be handled by external driver
		UINT8 data;
		if (io_reg_read(address, &data))
		{
			return data;
		}
	}
    return m_io->read_byte(address);
}

inline void avr8_device::io_write8(UINT16 address, UINT8 data)
{
	if (address < 0x100)
	{
		// Allow unhandled internal registers to be handled by external driver
		if (io_reg_write(address, data))
		{
			return;
		}
	}
    m_io->write_byte(address, data);
}

inline void avr8_device::push(UINT8 val)
{
	UINT16 sp = SPREG;
    io_write8(sp, val);
    sp--;
    io_write8(AVR8_REGIDX_SPL, sp & 0x00ff);
    io_write8(AVR8_REGIDX_SPH, (sp >> 8) & 0x00ff);
}

inline UINT8 avr8_device::pop()
{
	UINT16 sp = SPREG;
    sp++;
	io_write8(AVR8_REGIDX_SPL, sp & 0x00ff);
	io_write8(AVR8_REGIDX_SPH, (sp >> 8) & 0x00ff);
    return io_read8(sp);
}

//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void avr8_device::set_irq_line(UINT16 vector, int state)
{
    // Horrible hack, not accurate
    if(state)
    {
		if(SREG_R(AVR8_SREG_I))
		{
			SREG_W(AVR8_SREG_I, 0);
			push((m_pc >> 8) & 0x00ff);
			push(m_pc & 0x00ff);
			m_pc = vector;
		}
		else
		{
			m_interrupt_pending = true;
		}
    }
}

class CInterruptCondition
{
	public:
		UINT8 m_intindex;
		UINT8 m_intreg;
		UINT8 m_intmask;
		UINT8 m_regindex;
		UINT8 m_regmask;
};

static const CInterruptCondition s_int_conditions[AVR8_INTIDX_COUNT] =
{
	{ AVR8_INT_SPI_STC, AVR8_REGIDX_SPCR,   AVR8_SPCR_SPIE_MASK,     AVR8_REGIDX_SPSR,    AVR8_SPSR_SPIF_MASK },
	{ AVR8_INT_T1CAPT,  AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_ICIE1_MASK,  AVR8_REGIDX_TIFR1,   AVR8_TIFR1_ICF1_MASK },
	{ AVR8_INT_T1COMPB, AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_OCIE1B_MASK, AVR8_REGIDX_TIFR1,   AVR8_TIFR1_OCF1B_MASK },
	{ AVR8_INT_T1COMPA, AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_OCIE1A_MASK, AVR8_REGIDX_TIFR1,   AVR8_TIFR1_OCF1A_MASK },
	{ AVR8_INT_T1OVF,   AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_TOIE1_MASK,  AVR8_REGIDX_TIFR1,   AVR8_TIFR1_TOV1_MASK },
	{ AVR8_INT_T2COMPB, AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_OCIE2B_MASK, AVR8_REGIDX_TIFR2,   AVR8_TIFR2_OCF2B_MASK },
	{ AVR8_INT_T2COMPA, AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_OCIE2A_MASK, AVR8_REGIDX_TIFR2,   AVR8_TIFR2_OCF2A_MASK },
	{ AVR8_INT_T2OVF,   AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_TOIE2_MASK,  AVR8_REGIDX_TIFR2,   AVR8_TIFR2_TOV2_MASK }
};

void avr8_device::update_interrupt(int source)
{
    CInterruptCondition condition = s_int_conditions[source];

    int intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;
    intstate = (m_r[condition.m_intreg] & condition.m_intmask) ? intstate : 0;

	set_irq_line(condition.m_intindex, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}


//**************************************************************************
//  REGISTER HANDLING
//**************************************************************************
void avr8_device::timer_tick(int cycles)
{
	if (m_timer0_prescale != 0)
	{
		m_timer0_prescale_count += cycles;
		while(m_timer0_prescale_count >= m_timer0_prescale)
		{
			timer0_tick();
			m_timer0_prescale_count -= m_timer0_prescale;
		}
	}

	if (m_timer1_prescale != 0)
	{
		m_timer1_prescale_count += cycles;
		while(m_timer1_prescale_count >= m_timer1_prescale)
		{
			timer1_tick();
			m_timer1_prescale_count -= m_timer1_prescale;
		}
	}

	if (m_timer2_prescale != 0)
	{
		m_timer2_prescale_count += cycles;
		while(m_timer2_prescale_count >= (m_timer2_prescale))
		{
			timer2_tick();
			m_timer2_prescale_count -= (m_timer2_prescale);
		}
	}
}

// Timer 0 Handling
void avr8_device::timer0_tick()
{
	// TODO
}

// Timer 1 Handling

void avr8_device::timer1_tick()
{
    /* TODO: Handle comparison, setting OC1x pins, detection of BOTTOM and TOP */

    UINT16 count = (m_r[AVR8_REGIDX_TCNT1H] << 8) | m_r[AVR8_REGIDX_TCNT1L];
    INT32 wgm1 = ((m_r[AVR8_REGIDX_TCCR1B] & AVR8_TCCR1B_WGM1_32_MASK) >> 1) |
                 (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_WGM1_10_MASK);

    // Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
    //UINT8 compare_mode[2] = { (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT,
                              //(m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT };
    UINT16 ocr1[2] = { (m_r[AVR8_REGIDX_OCR1AH] << 8) | m_r[AVR8_REGIDX_OCR1AL],
                       (m_r[AVR8_REGIDX_OCR1BH] << 8) | m_r[AVR8_REGIDX_OCR1BL] };
	UINT8 ocf1[2] = { (1 << AVR8_TIFR1_OCF1A_SHIFT), (1 << AVR8_TIFR1_OCF1B_SHIFT) };
	UINT8 int1[2] = { AVR8_INTIDX_OCF1A, AVR8_INTIDX_OCF1B };
    INT32 increment = m_timer1_increment;

    for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
    {
        switch(wgm1)
        {
            case WGM1_FAST_PWM_OCR:
                if(count == ocr1[reg])
                {
                    if (reg == 0)
                    {
                        m_r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
						update_interrupt(AVR8_INTIDX_TOV1);
                        count = 0;
                        increment = 0;
                    }

                    m_r[AVR8_REGIDX_TIFR1] |= ocf1[reg];
					update_interrupt(int1[reg]);
                }
                else if(count == 0)
                {
                    if (reg == 0)
                    {
                        m_r[AVR8_REGIDX_TIFR1] &= ~AVR8_TIFR1_TOV1_MASK;
						update_interrupt(AVR8_INTIDX_TOV1);
                    }

                    m_r[AVR8_REGIDX_TIFR1] &= ~ocf1[reg];
					update_interrupt(int1[reg]);
                }
                break;

            default:
                // TODO
                break;
        }
        /*
        switch(compare_mode[reg])
        {
            case 0:
                //verboselog(m_pc, 0, "update_timer1_compare_mode: Normal port operation (OC1 disconnected)\n");
                break;

            case 1:
            case 2:
                // TODO
                break;

            case 3:
                break;
        }
        */
    }

    count += increment;
    m_r[AVR8_REGIDX_TCNT1H] = (count >> 8) & 0xff;
    m_r[AVR8_REGIDX_TCNT1L] = count & 0xff;
}

void avr8_device::change_timsk1(UINT8 data)
{
	UINT8 oldtimsk = AVR8_TIMSK1;
	UINT8 newtimsk = data;
	UINT8 changed = newtimsk ^ oldtimsk;

    AVR8_TIMSK1 = newtimsk;

	if(changed & AVR8_TIMSK1_ICIE1_MASK)
	{
		// Check for Input Capture Interrupt interrupt condition
		update_interrupt(AVR8_INTIDX_ICF1);
	}

	if(changed & AVR8_TIMSK1_OCIE1B_MASK)
	{
		// Check for Output Compare B Interrupt interrupt condition
		update_interrupt(AVR8_INTIDX_OCF1B);
	}

	if(changed & AVR8_TIMSK1_OCIE1A_MASK)
	{
		// Check for Output Compare A Interrupt interrupt condition
		update_interrupt(AVR8_INTIDX_OCF1A);
	}

	if(changed & AVR8_TIMSK1_TOIE1_MASK)
	{
		// Check for Output Compare A Interrupt interrupt condition
		update_interrupt(AVR8_INTIDX_TOV1);
	}
}

void avr8_device::update_timer1_waveform_gen_mode()
{
	// TODO
	m_timer1_top = 0;
	verboselog(m_pc, 0, "update_timer1_waveform_gen_mode: TODO; WGM1 is %d\n", AVR8_WGM1 );
	switch(AVR8_WGM1)
	{
		case WGM1_NORMAL:
			m_timer1_top = 0xffff;
			break;

		case WGM1_PWM_8_PC:
		case WGM1_FAST_PWM_8:
			m_timer1_top = 0x00ff;
			break;

		case WGM1_PWM_9_PC:
		case WGM1_FAST_PWM_9:
			m_timer1_top = 0x01ff;
			break;

		case WGM1_PWM_10_PC:
		case WGM1_FAST_PWM_10:
			m_timer1_top = 0x03ff;
			break;

		case WGM1_PWM_PFC_ICR:
		case WGM1_PWM_PC_ICR:
		case WGM1_CTC_ICR:
		case WGM1_FAST_PWM_ICR:
			m_timer1_top = AVR8_ICR1;
			break;

		case WGM1_PWM_PFC_OCR:
		case WGM1_PWM_PC_OCR:
		case WGM1_CTC_OCR:
		case WGM1_FAST_PWM_OCR:
			m_timer1_top = AVR8_OCR1A;
			break;

		default:
			verboselog(m_pc, 0, "update_timer1_waveform_gen_mode: Unsupported waveform generation type: %d\n", AVR8_WGM1);
			break;
	}
}

void avr8_device::changed_tccr1a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR1A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

    m_r[AVR8_REGIDX_TCCR1A] = newtccr;

	if(changed & AVR8_TCCR1A_WGM1_10_MASK)
	{
		// TODO
		update_timer1_waveform_gen_mode();
	}
}

void avr8_device::update_timer1_input_noise_canceler()
{
	// TODO
}

void avr8_device::update_timer1_input_edge_select()
{
	// TODO
	//verboselog(m_pc, 0, "update_timer1_input_edge_select: TODO; Clocking edge is %s\n", "test");
}

void avr8_device::update_timer1_clock_source()
{
	switch(AVR8_TIMER1_CLOCK_SELECT)
	{
		case 0: // Counter stopped
			m_timer1_prescale = 0;
			break;
		case 1: // Clk/1; no prescaling
			m_timer1_prescale = 1;
			break;
		case 2: // Clk/8
			m_timer1_prescale = 8;
			break;
		case 3: // Clk/32
			m_timer1_prescale = 32;
			break;
		case 4: // Clk/64
			m_timer1_prescale = 64;
			break;
		case 5: // Clk/128
			m_timer1_prescale = 128;
			break;
		case 6: // T1 trigger, falling edge
		case 7: // T1 trigger, rising edge
			m_timer1_prescale = 0;
			verboselog(m_pc, 0, "update_timer1_clock_source: T1 Trigger mode not implemented yet\n");
			break;
	}

	if (m_timer1_prescale_count > m_timer1_prescale)
	{
		m_timer1_prescale_count = m_timer1_prescale - 1;
	}
}

void avr8_device::changed_tccr1b(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR1B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

    m_r[AVR8_REGIDX_TCCR1B] = newtccr;

	if(changed & AVR8_TCCR1B_ICNC1_MASK)
	{
		// TODO
		update_timer1_input_noise_canceler();
	}

	if(changed & AVR8_TCCR1B_ICES1_MASK)
	{
		// TODO
		update_timer1_input_edge_select();
	}

	if(changed & AVR8_TCCR1B_WGM1_32_MASK)
	{
		// TODO
		update_timer1_waveform_gen_mode();
	}

	if(changed & AVR8_TCCR1B_CS_MASK)
	{
		update_timer1_clock_source();
	}
}

void avr8_device::update_ocr1(UINT16 newval, UINT8 reg)
{
	UINT8 *p_reg_h = (reg == AVR8_REG_A) ? &m_r[AVR8_REGIDX_OCR1AH] : &m_r[AVR8_REGIDX_OCR1BH];
	UINT8 *p_reg_l = (reg == AVR8_REG_A) ? &m_r[AVR8_REGIDX_OCR1AL] : &m_r[AVR8_REGIDX_OCR1BL];
	*p_reg_h = (UINT8)(newval >> 8);
	*p_reg_l = (UINT8)newval;

    // Nothing needs to be done? All handled in timer callback
}

// Timer 2 Handling

void avr8_device::timer2_tick()
{
    UINT16 count = m_r[AVR8_REGIDX_TCNT2];
    INT32 wgm2 = ((m_r[AVR8_REGIDX_TCCR2B] & AVR8_TCCR2B_WGM2_2_MASK) >> 1) |
                 (m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_WGM2_10_MASK);

    // Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
    //UINT8 compare_mode[2] = { (m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT,
                              //(m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT };
    UINT8 ocr2[2] = { m_r[AVR8_REGIDX_OCR2A], m_r[AVR8_REGIDX_OCR2B] };
	UINT8 ocf2[2] = { (1 << AVR8_TIFR2_OCF2A_SHIFT), (1 << AVR8_TIFR2_OCF2B_SHIFT) };
    INT32 increment = m_timer2_increment;

    for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
    {
        switch(wgm2)
        {
            case WGM2_FAST_PWM_CMP:
                if(count == ocr2[reg])
                {
                    if (reg == 0)
                    {
                        m_r[AVR8_REGIDX_TIFR2] |= AVR8_TIFR2_TOV2_MASK;
                        count = 0;
                        increment = 0;
                    }

                    m_r[AVR8_REGIDX_TIFR2] |= ocf2[reg];
                }
                else if(count == 0)
                {
                    if (reg == 0)
                    {
                        m_r[AVR8_REGIDX_TIFR2] &= ~AVR8_TIFR2_TOV2_MASK;
                    }
                }
                break;

            default:
                // TODO
                break;
        }
        /*
        switch(compare_mode[reg])
        {
            case 0:
                //verboselog(m_pc, 0, "update_timer2_compare_mode: Normal port operation (OC2 disconnected)\n");
                break;

            case 1:
            case 2:
                // TODO
                break;

            case 3:
                break;
        }
        */
    }

    m_r[AVR8_REGIDX_TCNT2] = count + increment;

	update_interrupt(AVR8_INTIDX_OCF2A);
	update_interrupt(AVR8_INTIDX_OCF2B);
	update_interrupt(AVR8_INTIDX_TOV2);
}

void avr8_device::update_timer2_waveform_gen_mode()
{
    m_timer2_top = 0;
	switch(AVR8_WGM2)
	{
		case WGM2_NORMAL:
		case WGM2_PWM_PC:
		case WGM2_FAST_PWM:
			m_timer2_top = 0x00ff;
			break;

		case WGM2_CTC_CMP:
		case WGM2_PWM_PC_CMP:
		case WGM2_FAST_PWM_CMP:
			m_timer2_top = AVR8_OCR2A;
			break;

		default:
			verboselog(m_pc, 0, "update_timer2_waveform_gen_mode: Unsupported waveform generation type: %d\n", AVR8_WGM2);
			break;
	}
}

void avr8_device::changed_tccr2a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR2A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	if(changed & AVR8_TCCR2A_WGM2_10_MASK)
	{
		// TODO
		update_timer2_waveform_gen_mode();
	}
}

void avr8_device::update_timer2_clock_source()
{
	switch(AVR8_TIMER2_CLOCK_SELECT)
	{
		case 0: // Counter stopped
			m_timer2_prescale = 0;
			break;
		case 1: // Clk/1; no prescaling
			m_timer2_prescale = 1;
			break;
		case 2: // Clk/8
			m_timer2_prescale = 8;
			break;
		case 3: // Clk/32
			m_timer2_prescale = 32;
			break;
		case 4: // Clk/64
			m_timer2_prescale = 64;
			break;
		case 5: // Clk/128
			m_timer2_prescale = 128;
			break;
		case 6: // Clk/256
			m_timer2_prescale = 256;
			break;
		case 7: // Clk/1024
			m_timer2_prescale = 1024;
			break;
	}

	if (m_timer2_prescale_count > m_timer2_prescale)
	{
		m_timer2_prescale_count = m_timer2_prescale - 1;
	}
}

void avr8_device::timer2_force_output_compare(int reg)
{
	// TODO
	verboselog(m_pc, 0, "force_output_compare: TODO; should be forcing OC2%c\n", avr8_reg_name[reg]);
}

void avr8_device::changed_tccr2b(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR2B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	if(changed & AVR8_TCCR2B_FOC2A_MASK)
	{
		// TODO
		timer2_force_output_compare(AVR8_REG_A);
	}

	if(changed & AVR8_TCCR2B_FOC2B_MASK)
	{
		// TODO
		timer2_force_output_compare(AVR8_REG_B);
	}

	if(changed & AVR8_TCCR2B_WGM2_2_MASK)
	{
		// TODO
		update_timer2_waveform_gen_mode();
	}

	if(changed & AVR8_TCCR2B_CS_MASK)
	{
		update_timer2_clock_source();
	}
}

void avr8_device::update_ocr2(UINT8 newval, UINT8 reg)
{
    m_r[(reg == AVR8_REG_A) ? AVR8_REGIDX_OCR2A : AVR8_REGIDX_OCR2B] = newval;

    // Nothing needs to be done? All handled in timer callback
}

/*****************************************************************************/

bool avr8_device::io_reg_write(UINT16 offset, UINT8 data)
{
    switch( offset )
    {
		case AVR8_REGIDX_OCR1BH:
			verboselog(m_pc, 0, "AVR8: OCR1BH = %02x\n", data );
			update_ocr1((AVR8_OCR1B & 0x00ff) | (data << 8), AVR8_REG_B);
			return true;

		case AVR8_REGIDX_OCR1BL:
			verboselog(m_pc, 0, "AVR8: OCR1BL = %02x\n", data );
			update_ocr1((AVR8_OCR1B & 0xff00) | data, AVR8_REG_B);
			return true;

		case AVR8_REGIDX_OCR1AH:
			verboselog(m_pc, 0, "AVR8: OCR1AH = %02x\n", data );
			update_ocr1((AVR8_OCR1A & 0x00ff) | (data << 8), AVR8_REG_A);
			return true;

		case AVR8_REGIDX_OCR1AL:
			verboselog(m_pc, 0, "AVR8: OCR1AL = %02x\n", data );
			update_ocr1((AVR8_OCR1A & 0xff00) | data, AVR8_REG_A);
			return true;

		case AVR8_REGIDX_TCCR1B:
			verboselog(m_pc, 0, "AVR8: TCCR1B = %02x\n", data );
			changed_tccr1b(data);
			return true;

		case AVR8_REGIDX_TCCR1A:
			verboselog(m_pc, 0, "AVR8: TCCR1A = %02x\n", data );
			changed_tccr1a(data);
			return true;

		case AVR8_REGIDX_TIMSK1:
			verboselog(m_pc, 0, "AVR8: TIMSK1 = %02x\n", data );
			change_timsk1(data);
			return true;

		case AVR8_REGIDX_TIFR1:
			verboselog(m_pc, 0, "AVR8: TIFR1 = %02x\n", data );
			m_r[AVR8_REGIDX_TIFR1] &= ~(data & AVR8_TIFR1_MASK);
			update_interrupt(AVR8_INTIDX_ICF1);
			update_interrupt(AVR8_INTIDX_OCF1A);
			update_interrupt(AVR8_INTIDX_OCF1B);
			update_interrupt(AVR8_INTIDX_TOV1);
			return true;

		case AVR8_REGIDX_TCCR2B:
			verboselog(m_pc, 0, "AVR8: TCCR2B = %02x\n", data );
			changed_tccr2b(data);
			return true;

		case AVR8_REGIDX_TCCR2A:
			verboselog(m_pc, 0, "AVR8: TCCR2A = %02x\n", data );
			changed_tccr2a(data);
			return true;

		case AVR8_REGIDX_OCR2A:
			update_ocr2(data, AVR8_REG_A);
			return true;

		case AVR8_REGIDX_OCR2B:
			update_ocr2(data, AVR8_REG_B);
			return true;

        case AVR8_REGIDX_TCNT2:
            AVR8_TCNT2 = data;
            return true;

        case AVR8_REGIDX_GTCCR:
        	if (data & AVR8_GTCCR_PSRASY_MASK)
        	{
				data &= ~AVR8_GTCCR_PSRASY_MASK;
				m_timer2_prescale_count = 0;
			}
            //verboselog(m_pc, 0, "AVR8: GTCCR = %02x\n", data );
            // TODO
            return true;

		case AVR8_REGIDX_SPL:
		case AVR8_REGIDX_SPH:
			m_r[offset] = data;
			return true;

		case AVR8_REGIDX_GPIOR0:
			verboselog(m_pc, 0, "AVR8: GPIOR0 Write: %02x\n", data);
			m_r[offset] = data;
			return true;

		case AVR8_REGIDX_SREG:
			m_status = data;
			return true;

		case AVR8_REGIDX_PORTB:
			if (m_portb_changed)
			{
				UINT8 changed = m_r[AVR8_REGIDX_PORTB] ^ data;
				(*m_portb_changed)(*this, data, changed);
				m_r[AVR8_REGIDX_PORTB] = data;
			}
			break;

		case AVR8_REGIDX_PORTC:
			if (m_portc_changed)
			{
				UINT8 changed = m_r[AVR8_REGIDX_PORTC] ^ data;
				(*m_portc_changed)(*this, data, changed);
				m_r[AVR8_REGIDX_PORTC] = data;
			}
			break;

        case AVR8_REGIDX_PORTD:
			if (m_portd_changed)
			{
				UINT8 changed = m_r[AVR8_REGIDX_PORTD] ^ data;
				(*m_portd_changed)(*this, data, changed);
				m_r[AVR8_REGIDX_PORTD] = data;
			}
            break;

        default:
            return false;
    }
    return false;
}

bool avr8_device::io_reg_read(UINT16 offset, UINT8 *data)
{
    switch( offset )
    {
		case AVR8_REGIDX_SPL:
		case AVR8_REGIDX_SPH:
		case AVR8_REGIDX_TCNT1L:
		case AVR8_REGIDX_TCNT1H:
		case AVR8_REGIDX_TCNT2:
		case AVR8_REGIDX_PORTB:
		case AVR8_REGIDX_PORTC:
		case AVR8_REGIDX_PORTD:
			*data = m_r[offset];
			return true;

		case AVR8_REGIDX_GPIOR0:
			*data = m_r[offset];
			verboselog(m_pc, 0, "AVR8: GPIOR0 Read: %02x\n", *data);
			return true;

		case AVR8_REGIDX_SREG:
			*data = m_status;
			return true;

        default:
            return false;
    }

    return false;
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 avr8_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 avr8_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 avr8_device::execute_input_lines() const
{
	return 0;
}


void avr8_device::execute_set_input(int inputnum, int state)
{
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void avr8_device::execute_run()
{
    UINT32 op = 0;
    INT32 offs = 0;
    UINT8 rd = 0;
    UINT8 rr = 0;
    UINT8 res = 0;
    UINT16 pd = 0;
    INT16 sd = 0;
    INT32 opcycles = 1;

    while (m_icount > 0)
    {
		opcycles = 1;

        m_pc &= m_addr_mask;

        debugger_instruction_hook(this, m_debugger_pc);

        op = (UINT32)program_read16(m_pc);

        switch(op & 0xf000)
        {
            case 0x0000:
                switch(op & 0x0f00)
                {
                    case 0x0000:    // NOP
                        break;
                    case 0x0100:    // MOVW Rd+1:Rd,Rr+1:Rd
                    	m_r[(RD4(op) << 1) + 1] = m_r[(RR4(op) << 1) + 1];
                    	m_r[RD4(op) << 1] = m_r[RR4(op) << 1];
                        break;
                    case 0x0200:    // MULS Rd,Rr
                        sd = (INT8)m_r[16 + RD4(op)] * (INT8)m_r[16 + RR4(op)];
                        m_r[1] = (sd >> 8) & 0x00ff;
                        m_r[0] = sd & 0x00ff;
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        break;
                    case 0x0300:    // Multiplicatn
                    	switch(MULCONST2(op))
                    	{
							case 0x0000: // MULSU Rd,Rr
								sd = (INT8)m_r[16 + RD3(op)] * (UINT8)m_r[16 + RR3(op)];
								m_r[1] = (sd >> 8) & 0x00ff;
								m_r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0001: // FMUL Rd,Rr
								sd = (UINT8)m_r[16 + RD3(op)] * (UINT8)m_r[16 + RR3(op)];
								sd <<= 1;
								m_r[1] = (sd >> 8) & 0x00ff;
								m_r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0002: // FMULS Rd,Rr
								sd = (INT8)m_r[16 + RD3(op)] * (INT8)m_r[16 + RR3(op)];
								sd <<= 1;
								m_r[1] = (sd >> 8) & 0x00ff;
								m_r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0003: // FMULSU Rd,Rr
								sd = (INT8)m_r[16 + RD3(op)] * (UINT8)m_r[16 + RR3(op)];
								sd <<= 1;
								m_r[1] = (sd >> 8) & 0x00ff;
								m_r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
						}
                        break;
                    case 0x0400:
                    case 0x0500:
                    case 0x0600:
                    case 0x0700:    // CPC Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd - (rr + SREG_R(AVR8_SREG_C));
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0800:
                    case 0x0900:
                    case 0x0a00:
                    case 0x0b00:    // SBC Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd - (rr + SREG_R(AVR8_SREG_C));
                        m_r[RD5(op)] = res;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0c00:
                    case 0x0d00:
                    case 0x0e00:
                    case 0x0f00:    // ADD Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd + rr;
                        m_r[RD5(op)] = res;
                        SREG_W(AVR8_SREG_H, (BIT(rd,3) & BIT(rr,3)) | (BIT(rr,3) & NOT(BIT(res,3))) | (NOT(BIT(res,3)) & BIT(rd,3)));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & NOT(BIT(rr,7)) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (BIT(rd,7) & BIT(rr,7)) | (BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(res,7)) & BIT(rd,7)));
                        break;
                }
                break;
            case 0x1000:
                switch(op & 0x0c00)
                {
                    case 0x0000:    // CPSR Rd,Rr
                        //output += sprintf( output, "CPSE    R%d, R%d", RD5(op), RR5(op) );
                        unimplemented_opcode(op);
                        break;
                    case 0x0400:    // CP Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd - rr;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0800:    // SUB Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd - rr;
                        m_r[RD5(op)] = res;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0c00:    // ADC Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        res = rd + rr + SREG_R(AVR8_SREG_C);
                        m_r[RD5(op)] = res;
                        SREG_W(AVR8_SREG_H, (BIT(rd,3) & BIT(rr,3)) | (BIT(rr,3) & NOT(BIT(res,3))) | (NOT(BIT(res,3)) & BIT(rd,3)));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & NOT(BIT(rr,7)) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (BIT(rd,7) & BIT(rr,7)) | (BIT(rr,7) & NOT(BIT(res,7))) | (NOT(BIT(res,7)) & BIT(rd,7)));
                        break;
                }
                break;
            case 0x2000:
                switch(op & 0x0c00)
                {
                    case 0x0000:    // AND Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        rd &= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        m_r[RD5(op)] = rd;
                        break;
                    case 0x0400:    // EOR Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        rd ^= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        m_r[RD5(op)] = rd;
                        break;
                    case 0x0800:    // OR Rd,Rr
                        rd = m_r[RD5(op)];
                        rr = m_r[RR5(op)];
                        rd |= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        m_r[RD5(op)] = rd;
                        break;
                    case 0x0c00:    // MOV Rd,Rr
                    	m_r[RD5(op)] = m_r[RR5(op)];
                        break;
                }
                break;
            case 0x3000:    // CPI Rd,K
                rd = m_r[16 + RD4(op)];
                rr = KCONST8(op);
                res = rd - rr;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x4000:    // SBCI Rd,K
                rd = m_r[16 + RD4(op)];
                rr = KCONST8(op);
                res = rd - (rr + SREG_R(AVR8_SREG_C));
                m_r[16 + RD4(op)] = res;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x5000:    // SUBI Rd,K
                rd = m_r[16 + RD4(op)];
                rr = KCONST8(op);
                res = rd - rr;
                m_r[16 + RD4(op)] = res;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x6000:    // ORI Rd,K
                rd = m_r[16 + RD4(op)];
                rr = KCONST8(op);
                rd |= rr;
                SREG_W(AVR8_SREG_V, 0);
                SREG_W(AVR8_SREG_N, BIT(rd,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                m_r[16 + RD4(op)] = rd;
                break;
            case 0x7000:    // ANDI Rd,K
                rd = m_r[16 + RD4(op)];
                rr = KCONST8(op);
                rd &= rr;
                SREG_W(AVR8_SREG_V, 0);
                SREG_W(AVR8_SREG_N, BIT(rd,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                m_r[16 + RD4(op)] = rd;
                break;
            case 0x8000:
            case 0xa000:
                switch(op & 0x0208)
                {
                    case 0x0000:    // LDD Rd,Z+q
                        m_r[RD5(op)] = io_read8(ZREG + QCONST6(op));
                        opcycles = 2;
                        break;
                    case 0x0008:    // LDD Rd,Y+q
                        m_r[RD5(op)] = io_read8(YREG + QCONST6(op));
                        opcycles = 2;
                        break;
                    case 0x0200:    // STD Z+q,Rr
                        io_write8(ZREG + QCONST6(op), m_r[RD5(op)]);
                        opcycles = 2;
                        break;
                    case 0x0208:    // STD Y+q,Rr
                        io_write8(YREG + QCONST6(op), m_r[RD5(op)]);
                        opcycles = 2;
                        break;
                }
                break;
            case 0x9000:
                switch(op & 0x0f00)
                {
                    case 0x0000:
                    case 0x0100:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // LDS Rd,k
                                op <<= 16;
                                m_pc++;
                                op |= program_read16(m_pc);
                                m_r[RD5(op >> 16)] = io_read8(op & 0x0000ffff);
                                opcycles = 2;
                                break;
                            case 0x0001:    // LD Rd,Z+
                                unimplemented_opcode(op);
                                break;
                            case 0x0002:    // LD Rd,-Z
                                pd = ZREG;
                                pd--;
                                m_r[RD5(op)] = io_read8(pd);
                                m_r[31] = (pd >> 8) & 0x00ff;
                                m_r[30] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x0004:    // LPM Rd,Z
                                m_r[RD5(op)] = program_read8(ZREG);
                                opcycles = 3;
                                break;
                            case 0x0005:    // LPM Rd,Z+
                                pd = ZREG;
                                m_r[RD5(op)] = program_read8(pd);
                                pd++;
                                m_r[31] = (pd >> 8) & 0x00ff;
                                m_r[30] = pd & 0x00ff;
                                opcycles = 3;
                                break;
                            case 0x0006:    // ELPM Rd,Z
                                //output += sprintf( output, "ELPM    R%d, Z", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x0007:    // ELPM Rd,Z+
                                //output += sprintf( output, "ELPM    R%d, Z+", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x0009:    // LD Rd,Y+
                                pd = YREG;
                                m_r[RD5(op)] = io_read8(pd);
                                pd++;
                                m_r[29] = (pd >> 8) & 0x00ff;
                                m_r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000a:    // LD Rd,-Y
                                pd = YREG;
                                pd--;
                                m_r[RD5(op)] = io_read8(pd);
                                m_r[29] = (pd >> 8) & 0x00ff;
                                m_r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000c:    // LD Rd,X
                            	m_r[RD5(op)] = io_read8(XREG);
                            	opcycles = 2;
                                break;
                            case 0x000d:    // LD Rd,X+
                                pd = XREG;
                                m_r[RD5(op)] = io_read8(pd);
                                pd++;
                                m_r[27] = (pd >> 8) & 0x00ff;
                                m_r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000e:    // LD Rd,-X
                                pd = XREG;
                                pd--;
                                m_r[RD5(op)] = io_read8(pd);
                                m_r[27] = (pd >> 8) & 0x00ff;
                                m_r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000f:    // POP Rd
                                m_r[RD5(op)] = pop();
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0200:
                    case 0x0300:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // STS k,Rr
                                op <<= 16;
                                m_pc++;
                                op |= program_read16(m_pc);
                                io_write8(op & 0x0000ffff, m_r[RD5(op >> 16)]);
                                opcycles = 2;
                                break;
                            case 0x0001:    // ST Z+,Rd
                                //output += sprintf( output, "ST       Z+, R%d", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x0002:    // ST -Z,Rd
                                //output += sprintf( output, "ST      -Z , R%d", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x0009:    // ST Y+,Rd
                                pd = YREG;
                                io_write8(pd, m_r[RD5(op)]);
                                pd++;
                                m_r[29] = (pd >> 8) & 0x00ff;
                                m_r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000a:    // ST -Y,Rd
                                //output += sprintf( output, "ST      -Y , R%d", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x000c:    // ST X,Rd
                                io_write8(XREG, m_r[RD5(op)]);
                                break;
                            case 0x000d:    // ST X+,Rd
                                pd = XREG;
                                io_write8(pd, m_r[RD5(op)]);
                                pd++;
                                m_r[27] = (pd >> 8) & 0x00ff;
                                m_r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000e:    // ST -X,Rd
                                //output += sprintf( output, "ST      -X , R%d", RD5(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x000f:    // PUSH Rd
                                push(m_r[RD5(op)]);
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0400:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = m_r[RD5(op)];
                                res = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0001:    // NEG Rd
                                rd = m_r[RD5(op)];
                                res = 0 - rd;
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0002:    // SWAP Rd
                            	rd = m_r[RD5(op)];
                            	m_r[RD5(op)] = (rd >> 4) | (rd << 4);
                                break;
                            case 0x0003:    // INC Rd
                                rd = m_r[RD5(op)];
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0005:    // ASR Rd
                                rd = m_r[RD5(op)];
                                res = (rd & 0x80) | (rd >> 1);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(rd,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0006:    // LSR Rd
                                rd = m_r[RD5(op)];
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0007:    // ROR Rd
                            	rd = m_r[RD5(op)];
                            	res = rd >> 1;
                            	res |= (SREG_R(AVR8_SREG_C) << 7);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0008:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // SEC
                                    case 0x0010:    // SEZ
                                    case 0x0020:    // SEN
                                    case 0x0030:    // SEV
                                    case 0x0040:    // SES
                                    case 0x0050:    // SEH
                                    case 0x0060:    // SET
                                    case 0x0070:    // SEI
                                        SREG_W((op >> 4) & 0x07, 1);
                                        break;
                                    case 0x0080:    // CLC
                                    case 0x0090:    // CLZ
                                    case 0x00a0:    // CLN
                                    case 0x00b0:    // CLV
                                    case 0x00c0:    // CLS
                                    case 0x00d0:    // CLH
                                    case 0x00e0:    // CLT
                                    case 0x00f0:    // CLI
                                        SREG_W((op >> 4) & 0x07, 0);
                                        break;
                                }
                                break;
                            case 0x0009:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // IJMP
                                        m_pc = ZREG - 1;
                                        opcycles = 2;
                                        break;
                                    case 0x0010:    // EIJMP
                                        //output += sprintf( output, "EIJMP" );
                                        unimplemented_opcode(op);
                                        break;
                                    default:
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        unimplemented_opcode(op);
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = m_r[RD5(op)];
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                m_r[RD5(op)] = res;
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
								offs = KCONST22(op) << 16;
                                m_pc++;
                                offs |= program_read16(m_pc);
                                m_pc = offs;
								m_pc--;
								opcycles = 3;
                                break;
                            case 0x000e:    // CALL k
                            case 0x000f:
								push(((m_pc + 1) >> 8) & 0x00ff);
								push((m_pc + 1) & 0x00ff);
								offs = KCONST22(op) << 16;
                                m_pc++;
                                offs |= program_read16(m_pc);
                                m_pc = offs;
								m_pc--;
								opcycles = 4;
                                break;
                            default:
                                unimplemented_opcode(op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0500:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = m_r[RD5(op)];
                                res = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0001:    // NEG Rd
                                rd = m_r[RD5(op)];
                                res = 0 - rd;
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0002:    // SWAP Rd
                            	rd = m_r[RD5(op)];
                            	m_r[RD5(op)] = (rd >> 4) | (rd << 4);
                                break;
                            case 0x0003:    // INC Rd
                                rd = m_r[RD5(op)];
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0005:    // ASR Rd
                                rd = m_r[RD5(op)];
                                res = (rd & 0x80) | (rd >> 1);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(rd,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0006:    // LSR Rd
                                rd = m_r[RD5(op)];
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0007:    // ROR Rd
                            	rd = m_r[RD5(op)];
                            	res = rd >> 1;
                            	res |= (SREG_R(AVR8_SREG_C) << 7);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                m_r[RD5(op)] = res;
                                break;
                            case 0x0008:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // RET
                                        m_pc = pop();
                                        m_pc |= pop() << 8;
                                        m_pc--;
                                        opcycles = 4;
                                        break;
                                    case 0x0010:    // RETI
                                        m_pc = pop();
                                        m_pc |= pop() << 8;
                                        m_pc--;
                                        SREG_W(AVR8_SREG_I, 1);
                                        opcycles = 4;
                                        break;
                                    case 0x0080:    // SLEEP
                                        //output += sprintf( output, "SLEEP" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x0090:    // BREAK
                                        //output += sprintf( output, "BREAK" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x00a0:    // WDR
                                        //output += sprintf( output, "WDR" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x00c0:    // LPM
                                        m_r[0] = program_read8(ZREG);
                                        opcycles = 3;
                                        break;
                                    case 0x00d0:    // ELPM
                                        //output += sprintf( output, "ELPM" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x00e0:    // SPM
                                        //output += sprintf( output, "SPM" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x00f0:    // SPM Z+
                                        //output += sprintf( output, "SPM     Z+" );
                                        unimplemented_opcode(op);
                                        break;
                                    default:
                                        unimplemented_opcode(op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x0009:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // ICALL
                                        //output += sprintf( output, "ICALL" );
                                        unimplemented_opcode(op);
                                        break;
                                    case 0x0010:    // EICALL
                                        //output += sprintf( output, "EICALL" );
                                        unimplemented_opcode(op);
                                        break;
                                    default:
                                        unimplemented_opcode(op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = m_r[RD5(op)];
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                m_r[RD5(op)] = res;
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "JMP     0x%06x", KCONST22(op) );
                                unimplemented_opcode(op);
                                break;
                            case 0x000e:
                            case 0x000f:    // CALL k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "CALL    0x%06x", KCONST22(op) );
                                unimplemented_opcode(op);
                                break;
                        }
                        break;
                    case 0x0600:    // ADIW Rd+1:Rd,K
                        rd = m_r[24 + (DCONST(op) << 1)];
                        rr = m_r[25 + (DCONST(op) << 1)];
                        pd = rd;
                        pd |= rr << 8;
                        pd += KCONST6(op);
                        SREG_W(AVR8_SREG_V, BIT(pd,15) & NOT(BIT(rr,7)));
                        SREG_W(AVR8_SREG_N, BIT(pd,15));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (pd == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, NOT(BIT(pd,15)) & BIT(rr,7));
                        m_r[24 + (DCONST(op) << 1)] = pd & 0x00ff;
                        m_r[25 + (DCONST(op) << 1)] = (pd >> 8) & 0x00ff;
                        opcycles = 2;
                        break;
                    case 0x0700:    // SBIW Rd+1:Rd,K
                        //output += sprintf( output, "SBIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
                        unimplemented_opcode(op);
                        break;
                    case 0x0800:    // CBI A,b
                        //output += sprintf( output, "CBI     0x%02x, %d", ACONST5(op), RR3(op) );
                        io_write8(32 + ACONST5(op), io_read8(32 + ACONST5(op)) &~ (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0900:    // SBIC A,b
                		if(NOT(BIT(io_read8(32 + ACONST5(op)), RR3(op))))
                		{
                            op = (UINT32)program_read16(m_pc + 1);
							opcycles = is_long_opcode(op) ? 3 : 2;
                            m_pc += is_long_opcode(op) ? 2 : 1;
						}
                        break;
                    case 0x0a00:    // SBI A,b
                        io_write8(32 + ACONST5(op), io_read8(32 + ACONST5(op)) | (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0b00:    // SBIS A,b
                		if(BIT(io_read8(32 + ACONST5(op)), RR3(op)))
                		{
                            op = (UINT32)program_read16(m_pc + 1);
							opcycles = is_long_opcode(op) ? 3 : 2;
                            m_pc += is_long_opcode(op) ? 2 : 1;
						}
                        break;
                    case 0x0c00:
                    case 0x0d00:
                    case 0x0e00:
                    case 0x0f00:    // MUL Rd,Rr
                        sd = (UINT8)m_r[RD5(op)] * (UINT8)m_r[RR5(op)];
                        m_r[1] = (sd >> 8) & 0x00ff;
                        m_r[0] = sd & 0x00ff;
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        break;
                }
                break;
            case 0xb000:
                if(op & 0x0800) // OUT A,Rr
                {
                    io_write8(32 + ACONST6(op), m_r[RD5(op)]);
                }
                else            // IN Rd,A
                {
                    m_r[RD5(op)] = io_read8(0x20 + ACONST6(op));
                }
                break;
            case 0xc000:    // RJMP k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                m_pc += offs;
                opcycles = 2;
                break;
            case 0xd000:    // RCALL k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                push(((m_pc + 1) >> 8) & 0x00ff);
                push((m_pc + 1) & 0x00ff);
                m_pc += offs;
                opcycles = 3;
                break;
            case 0xe000:    // LDI Rd,K
                m_r[16 + RD4(op)] = KCONST8(op);
                break;
            case 0xf000:
                switch(op & 0x0c00)
                {
                    case 0x0000: // BRLO through BRIE
                        if(SREG_R(op & 0x0007))
                        {
                            offs = (INT32)(KCONST7(op));
                            if(offs & 0x40)
                            {
                                offs |= 0xffffff80;
                            }
                            m_pc += offs;
                            opcycles = 2;
                        }
                        break;
                    case 0x0400: // BRSH through BRID
                        if(SREG_R(op & 0x0007) == 0)
                        {
                            offs = (INT32)(KCONST7(op));
                            if(offs & 0x40)
                            {
                                offs |= 0xffffff80;
                            }
                            m_pc += offs;
                            opcycles = 2;
                        }
                        break;
                    case 0x0800:
                        if(op & 0x0200) // BST Rd, b
                        {
                            SREG_W(AVR8_SREG_T, (BIT(m_r[RD5(op)], RR3(op))) ? 1 : 0);
                        }
                        else            // BLD Rd, b
                        {
                            if(SREG_R(AVR8_SREG_T))
                            {
                                m_r[RD5(op)] |= (1 << RR3(op));
                            }
                            else
                            {
                                m_r[RD5(op)] &= ~(1 << RR3(op));
                            }
                        }
                        break;
                    case 0x0c00:
                        if(op & 0x0200) // SBRS Rd, b
                        {
                            if(BIT(m_r[RD5(op)], RR3(op)))
                            {
                                op = (UINT32)program_read16(m_pc++);
                                m_pc += is_long_opcode(op) ? 1 : 0;
                                opcycles = is_long_opcode(op) ? 3 : 2;
                            }
                        }
                        else            // SBRC Rd, b
                        {
                            if(NOT(BIT(m_r[RD5(op)], RR3(op))))
                            {
                                op = (UINT32)program_read16(m_pc++);
                                m_pc += is_long_opcode(op) ? 1 : 0;
                                opcycles = is_long_opcode(op) ? 3 : 2;
                            }
                        }
                        break;
                }
                break;
        }

        m_pc++;

        m_debugger_pc = m_pc << 1;

        m_icount -= opcycles;

		m_elapsed_cycles += opcycles;

		timer_tick(opcycles);
    }
}
