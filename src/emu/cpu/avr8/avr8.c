/*
    Atmel 8-bit AVR emulator

    (Skeleton)

    DONE:
    - Disassembler
    - [lft]'s "Craft" depends on on-chip device support now instead of opcodes (it requires unusually few in order to boot)

    TODO:
    - Everything else
      * Finish opcode implementation
      * Add proper cycle timing
      * Add Interrupts
      * Add on-chip hardware (machine driver)

    Written by MooglyGuy
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

#define SREG_R(b) ((cpustate->status & (1 << (b))) >> (b))
#define SREG_W(b,v) cpustate->status = (cpustate->status & ~(1 << (b))) | ((v) << (b))
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
#define XREG            ((cpustate->r[27] << 8) | cpustate->r[26])
#define YREG            ((cpustate->r[29] << 8) | cpustate->r[28])
#define ZREG            ((cpustate->r[31] << 8) | cpustate->r[30])
#define SPREG			((cpustate->r[AVR8_REGIDX_SPH] << 8) | cpustate->r[AVR8_REGIDX_SPL])

// I/O Defines
#define AVR8_OCR1BH				(cpustate->r[AVR8_REGIDX_OCR1BH])
#define AVR8_OCR1BL				(cpustate->r[AVR8_REGIDX_OCR1BL])
#define AVR8_OCR1AH				(cpustate->r[AVR8_REGIDX_OCR1AH])
#define AVR8_OCR1AL				(cpustate->r[AVR8_REGIDX_OCR1AL])
#define AVR8_ICR1H				(cpustate->r[AVR8_REGIDX_ICR1H])
#define AVR8_ICR1L				(cpustate->r[AVR8_REGIDX_ICR1L])
#define AVR8_TCNT1H				(cpustate->r[AVR8_REGIDX_TCNT1H])
#define AVR8_TCNT1L				(cpustate->r[AVR8_REGIDX_TCNT1L])

#define AVR8_TCCR1B					(cpustate->r[AVR8_REGIDX_TCCR1B])
#define AVR8_TCCR1B_ICNC1_MASK		0x80
#define AVR8_TCCR1B_ICNC1_SHIFT		7
#define AVR8_TCCR1B_ICES1_MASK		0x40
#define AVR8_TCCR1B_ICES1_SHIFT		6
#define AVR8_TCCR1B_WGM1_32_MASK	0x18
#define AVR8_TCCR1B_WGM1_32_SHIFT	3
#define AVR8_TCCR1B_CS_MASK			0x07
#define AVR8_TCCR1B_CS_SHIFT		0
#define AVR8_TIMER1_CLOCK_SELECT	(AVR8_TCCR1B & AVR8_TCCR1B_CS_MASK)

#define AVR8_TCCR1A					(cpustate->r[AVR8_REGIDX_TCCR1A])
#define AVR8_TCCR1A_COM1A_MASK		0xc0
#define AVR8_TCCR1A_COM1A_SHIFT		6
#define AVR8_TCCR1A_COM1B_MASK		0x30
#define AVR8_TCCR1A_COM1B_SHIFT		4
#define AVR8_TCCR1A_WGM1_10_MASK	0x03
#define AVR8_TCCR1A_WGM1_10_SHIFT	0
#define AVR8_TCCR1A_COM1A			((AVR8_TCCR1A & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT)
#define AVR8_TCCR1A_COM1B			((AVR8_TCCR1A & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT)
#define AVR8_TCCR1A_WGM1_10			(AVR8_TCCR1A & AVR8_TCCR1A_WGM1_10_MASK)

#define AVR8_TIMSK1				(cpustate->r[AVR8_REGIDX_TIMSK1])
#define AVR8_TIMSK1_ICIE1_MASK	0x20
#define AVR8_TIMSK1_OCIE1B_MASK	0x04
#define AVR8_TIMSK1_OCIE1A_MASK	0x02
#define AVR8_TIMSK1_TOIE1_MASK	0x01
#define AVR8_TIMSK1_ICIE1		((AVR8_TIMSK1 & AVR8_TIMSK1_ICIE1_MASK) >> 5)
#define AVR8_TIMSK1_OCIE1B		((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1B_MASK) >> 2)
#define AVR8_TIMSK1_OCIE1A		((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1A_MASK) >> 1)
#define AVR8_TIMSK1_TOIE1		(AVR8_TIMSK1 & AVR8_TIMSK1_TOIE1_MASK)

#define AVR8_TIFR1				(cpustate->r[AVR8_REGIDX_TIFR1])
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

#define AVR8_TCCR2B					(cpustate->r[AVR8_REGIDX_TCCR1B])
#define AVR8_TCCR2B_FOC2A_MASK		0x80
#define AVR8_TCCR2B_FOC2A_SHIFT		7
#define AVR8_TCCR2B_FOC2B_MASK		0x40
#define AVR8_TCCR2B_FOC2B_SHIFT		6
#define AVR8_TCCR2B_WGM2_2_MASK		0x08
#define AVR8_TCCR2B_WGM2_2_SHIFT	3
#define AVR8_TCCR2B_CS_MASK			0x07
#define AVR8_TCCR2B_CS_SHIFT		0
#define AVR8_TIMER2_CLOCK_SELECT	(AVR8_TCCR2B & AVR8_TCCR2B_CS_MASK)

#define AVR8_TCCR2A					(cpustate->r[AVR8_REGIDX_TCCR1A])
#define AVR8_TCCR2A_COM2A_MASK		0xc0
#define AVR8_TCCR2A_COM2A_SHIFT		6
#define AVR8_TCCR2A_COM2B_MASK		0x30
#define AVR8_TCCR2A_COM2B_SHIFT		4
#define AVR8_TCCR2A_WGM2_10_MASK	0x03
#define AVR8_TCCR2A_WGM2_10_SHIFT	0
#define AVR8_TCCR2A_COM2A			((AVR8_TCCR2A & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT)
#define AVR8_TCCR2A_COM2B			((AVR8_TCCR2A & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT)
#define AVR8_TCCR2A_WGM2_10			(AVR8_TCCR2A & AVR8_TCCR1A_WGM2_10_MASK)

#define AVR8_TIMSK2				(cpustate->r[AVR8_REGIDX_TIMSK2])
#define AVR8_TIMSK2_OCIE2B_MASK	0x04
#define AVR8_TIMSK2_OCIE2A_MASK	0x02
#define AVR8_TIMSK2_TOIE2_MASK	0x01
#define AVR8_TIMSK2_OCIE2B		((AVR8_TIMSK2 & AVR8_TIMSK1_OCIE2B_MASK) >> 2)
#define AVR8_TIMSK2_OCIE2A		((AVR8_TIMSK2 & AVR8_TIMSK1_OCIE2A_MASK) >> 1)
#define AVR8_TIMSK2_TOIE2		(AVR8_TIMSK2 & AVR8_TIMSK1_TOIE2_MASK)

#define AVR8_TIFR2				(cpustate->r[AVR8_REGIDX_TIFR2])
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

#define AVR8_OCR2B				cpustate->r[AVR8_REGIDX_OCR2B]
#define AVR8_OCR2A				cpustate->r[AVR8_REGIDX_OCR2A]
#define AVR8_TCNT2				cpustate->r[AVR8_REGIDX_TCNT2]
#define AVR8_WGM2				(((AVR8_TCCR2B & 0x08) >> 1) | (AVR8_TCCR2A & 0x03))

#define AVR8_GTCCR_PSRASY_MASK	0x02
#define AVR8_GTCCR_PSRASY_SHIFT	1

INLINE avr8_state *get_safe_token(device_t *device)
{
    assert(device != NULL);
    assert(device->type() == ATMEGA88 || device->type() == ATMEGA644);
    return (avr8_state *)downcast<legacy_cpu_device *>(device)->token();
}

/*****************************************************************************/
// Prototypes

// - Utility
static void unimplemented_opcode(avr8_state *cpustate, UINT32 op);

// - Interrupts
static void avr8_set_irq_line(avr8_state *cpustate, UINT16 vector, int state);
static void avr8_update_interrupt_internal(avr8_state *cpustate, int source);
//static void avr8_poll_interrupt(avr8_state *cpustate);

// - Timers
static void avr8_timer_tick(avr8_state *cpustate, int cycles);

// - Timer 0
static void avr8_timer0_tick(avr8_state *cpustate);

// - Timer 1
static void avr8_timer1_tick(avr8_state *cpustate);
static void avr8_change_timsk1(avr8_state *cpustate, UINT8 data);
static void avr8_update_timer1_waveform_gen_mode(avr8_state *cpustate);
static void avr8_changed_tccr1a(avr8_state *cpustate, UINT8 data);
static void avr8_update_timer1_input_noise_canceler(avr8_state *cpustate);
static void avr8_update_timer1_input_edge_select(avr8_state *cpustate);
static void avr8_update_timer1_clock_source(avr8_state *cpustate);
static void avr8_changed_tccr1b(avr8_state *cpustate, UINT8 data);
static void avr8_update_ocr1(avr8_state *cpustate, UINT16 newval, UINT8 reg);

// - Timer 2
static void avr8_timer2_tick(avr8_state *cpustate);
static void avr8_update_timer2_waveform_gen_mode(avr8_state *cpustate);
static void avr8_changed_tccr2a(avr8_state *cpustate, UINT8 data);
static void avr8_update_timer2_clock_source(avr8_state *cpustate);
static void avr8_timer2_force_output_compare(avr8_state *cpustate, int reg);
static void avr8_changed_tccr2b(avr8_state *cpustate, UINT8 data);
static void avr8_update_ocr2(avr8_state *cpustate, UINT8 newval, UINT8 reg);

// - Register Handling
static bool avr8_io_reg_write(avr8_state *cpustate, UINT16 offset, UINT8 data);
static bool avr8_io_reg_read(avr8_state *cpustate, UINT16 offset, UINT8 *data);

/*****************************************************************************/
// Utility Functions

static void unimplemented_opcode(avr8_state *cpustate, UINT32 op)
{
    fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, cpustate->pc);
}

INLINE bool avr8_is_long_opcode(UINT16 op)
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

INLINE UINT8 READ_PRG_8(avr8_state *cpustate, UINT32 address)
{
    return cpustate->program->read_byte(address);
}

INLINE UINT16 READ_PRG_16(avr8_state *cpustate, UINT32 address)
{
    return cpustate->program->read_word(address << 1);
}

INLINE void WRITE_PRG_8(avr8_state *cpustate, UINT32 address, UINT8 data)
{
    cpustate->program->write_byte(address, data);
}

INLINE void WRITE_PRG_16(avr8_state *cpustate, UINT32 address, UINT16 data)
{
    cpustate->program->write_word(address, data);
}

INLINE UINT8 READ_IO_8(avr8_state *cpustate, UINT16 address)
{
	if (address < 0x100)
	{
		// Allow unhandled internal registers to be handled by external driver
		UINT8 data;
		if (avr8_io_reg_read(cpustate, address, &data))
		{
			return data;
		}
	}
    return cpustate->io->read_byte(address);
}

INLINE void WRITE_IO_8(avr8_state *cpustate, UINT16 address, UINT8 data)
{
	if (address < 0x100)
	{
		// Allow unhandled internal registers to be handled by external driver
		if (avr8_io_reg_write(cpustate, address, data))
		{
			return;
		}
	}
    cpustate->io->write_byte(address, data);
}

INLINE void PUSH(avr8_state *cpustate, UINT8 val)
{
	UINT16 sp = SPREG;
    WRITE_IO_8(cpustate, sp, val);
    sp--;
    WRITE_IO_8(cpustate, AVR8_REGIDX_SPL, sp & 0x00ff);
    WRITE_IO_8(cpustate, AVR8_REGIDX_SPH, (sp >> 8) & 0x00ff);
}

INLINE UINT8 POP(avr8_state *cpustate)
{
	UINT16 sp = SPREG;
    sp++;
	WRITE_IO_8(cpustate, AVR8_REGIDX_SPL, sp & 0x00ff);
	WRITE_IO_8(cpustate, AVR8_REGIDX_SPH, (sp >> 8) & 0x00ff);
    return READ_IO_8(cpustate, sp);
}

UINT64 avr8_get_elapsed_cycles(device_t *device)
{
	return get_safe_token(device)->elapsed_cycles;
}

/*****************************************************************************/
// Interrupts

static void avr8_set_irq_line(avr8_state *cpustate, UINT16 vector, int state)
{
    // Horrible hack, not accurate
    if(state)
    {
		if(SREG_R(AVR8_SREG_I))
		{
			SREG_W(AVR8_SREG_I, 0);
			//printf("Push: %04x\n", cpustate->pc);
			PUSH(cpustate, (cpustate->pc >> 8) & 0x00ff);
			PUSH(cpustate, cpustate->pc & 0x00ff);
			cpustate->pc = vector;

			//avr8_timer_tick(cpustate, 3);
		}
		else
		{
			cpustate->interrupt_pending = true;
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

static void avr8_update_interrupt_internal(avr8_state *cpustate, int source)
{
    CInterruptCondition condition = s_int_conditions[source];

    int intstate = (cpustate->r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;
    intstate = (cpustate->r[condition.m_intreg] & condition.m_intmask) ? intstate : 0;

	avr8_set_irq_line(cpustate, condition.m_intindex, intstate);

	if (intstate)
	{
		cpustate->r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

void avr8_update_interrupt(device_t *device, int source)
{
	avr8_state *cpustate = get_safe_token(device);
	avr8_update_interrupt_internal(cpustate, source);
}

/*****************************************************************************/
// Timers

static void avr8_timer_tick(avr8_state *cpustate, int cycles)
{
	if (cpustate->timer0_prescale != 0)
	{
		cpustate->timer0_prescale_count += cycles;
		while(cpustate->timer0_prescale_count >= cpustate->timer0_prescale)
		{
			avr8_timer0_tick(cpustate);
			cpustate->timer0_prescale_count -= cpustate->timer0_prescale;
		}
	}

	if (cpustate->timer1_prescale != 0)
	{
		cpustate->timer1_prescale_count += cycles;
		while(cpustate->timer1_prescale_count >= cpustate->timer1_prescale)
		{
			avr8_timer1_tick(cpustate);
			cpustate->timer1_prescale_count -= cpustate->timer1_prescale;
		}
	}

	if (cpustate->timer2_prescale != 0)
	{
		cpustate->timer2_prescale_count += cycles;
		while(cpustate->timer2_prescale_count >= (cpustate->timer2_prescale))
		{
			avr8_timer2_tick(cpustate);
			cpustate->timer2_prescale_count -= (cpustate->timer2_prescale);
		}
	}
}

// Timer 0 Handling
static void avr8_timer0_tick(avr8_state *cpustate)
{
	// TODO
}

// Timer 1 Handling

static void avr8_timer1_tick(avr8_state *cpustate)
{
    /* TODO: Handle comparison, setting OC1x pins, detection of BOTTOM and TOP */

    UINT16 count = (cpustate->r[AVR8_REGIDX_TCNT1H] << 8) | cpustate->r[AVR8_REGIDX_TCNT1L];
    INT32 wgm1 = ((cpustate->r[AVR8_REGIDX_TCCR1B] & AVR8_TCCR1B_WGM1_32_MASK) >> 1) |
                 (cpustate->r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_WGM1_10_MASK);

    // Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
    //UINT8 compare_mode[2] = { (cpustate->r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT,
                              //(cpustate->r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT };
    UINT16 ocr1[2] = { (cpustate->r[AVR8_REGIDX_OCR1AH] << 8) | cpustate->r[AVR8_REGIDX_OCR1AL],
                       (cpustate->r[AVR8_REGIDX_OCR1BH] << 8) | cpustate->r[AVR8_REGIDX_OCR1BL] };
	UINT8 ocf1[2] = { (1 << AVR8_TIFR1_OCF1A_SHIFT), (1 << AVR8_TIFR1_OCF1B_SHIFT) };
	UINT8 int1[2] = { AVR8_INTIDX_OCF1A, AVR8_INTIDX_OCF1B };
    INT32 increment = cpustate->timer1_increment;

    for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
    {
        switch(wgm1)
        {
            case WGM1_FAST_PWM_OCR:
                if(count == ocr1[reg])
                {
                    if (reg == 0)
                    {
                        cpustate->r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
						avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_TOV1);
                        count = 0;
                        increment = 0;
                    }

                    cpustate->r[AVR8_REGIDX_TIFR1] |= ocf1[reg];
					avr8_update_interrupt_internal(cpustate, int1[reg]);
                }
                else if(count == 0)
                {
                    if (reg == 0)
                    {
                        cpustate->r[AVR8_REGIDX_TIFR1] &= ~AVR8_TIFR1_TOV1_MASK;
						avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_TOV1);
                    }

                    cpustate->r[AVR8_REGIDX_TIFR1] &= ~ocf1[reg];
					avr8_update_interrupt_internal(cpustate, int1[reg]);
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
                //verboselog(cpustate->pc, 0, "avr8_update_timer1_compare_mode: Normal port operation (OC1 disconnected)\n");
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
    cpustate->r[AVR8_REGIDX_TCNT1H] = (count >> 8) & 0xff;
    cpustate->r[AVR8_REGIDX_TCNT1L] = count & 0xff;
}

static void avr8_change_timsk1(avr8_state *cpustate, UINT8 data)
{
	UINT8 oldtimsk = AVR8_TIMSK1;
	UINT8 newtimsk = data;
	UINT8 changed = newtimsk ^ oldtimsk;

    AVR8_TIMSK1 = newtimsk;

	if(changed & AVR8_TIMSK1_ICIE1_MASK)
	{
		// Check for Input Capture Interrupt interrupt condition
		avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_ICF1);
	}

	if(changed & AVR8_TIMSK1_OCIE1B_MASK)
	{
		// Check for Output Compare B Interrupt interrupt condition
		avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF1B);
	}

	if(changed & AVR8_TIMSK1_OCIE1A_MASK)
	{
		// Check for Output Compare A Interrupt interrupt condition
		avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF1A);
	}

	if(changed & AVR8_TIMSK1_TOIE1_MASK)
	{
		// Check for Output Compare A Interrupt interrupt condition
		avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_TOV1);
	}
}

static void avr8_update_timer1_waveform_gen_mode(avr8_state *cpustate)
{
	// TODO
	cpustate->timer1_top = 0;
	verboselog(cpustate->pc, 0, "avr8_update_timer1_waveform_gen_mode: TODO; WGM1 is %d\n", AVR8_WGM1 );
	switch(AVR8_WGM1)
	{
		case WGM1_NORMAL:
			cpustate->timer1_top = 0xffff;
			break;

		case WGM1_PWM_8_PC:
		case WGM1_FAST_PWM_8:
			cpustate->timer1_top = 0x00ff;
			break;

		case WGM1_PWM_9_PC:
		case WGM1_FAST_PWM_9:
			cpustate->timer1_top = 0x01ff;
			break;

		case WGM1_PWM_10_PC:
		case WGM1_FAST_PWM_10:
			cpustate->timer1_top = 0x03ff;
			break;

		case WGM1_PWM_PFC_ICR:
		case WGM1_PWM_PC_ICR:
		case WGM1_CTC_ICR:
		case WGM1_FAST_PWM_ICR:
			cpustate->timer1_top = AVR8_ICR1;
			break;

		case WGM1_PWM_PFC_OCR:
		case WGM1_PWM_PC_OCR:
		case WGM1_CTC_OCR:
		case WGM1_FAST_PWM_OCR:
			cpustate->timer1_top = AVR8_OCR1A;
			break;

		default:
			verboselog(cpustate->pc, 0, "avr8_update_timer1_waveform_gen_mode: Unsupported waveform generation type: %d\n", AVR8_WGM1);
			break;
	}
}

static void avr8_changed_tccr1a(avr8_state *cpustate, UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR1A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

    cpustate->r[AVR8_REGIDX_TCCR1A] = newtccr;

	if(changed & AVR8_TCCR1A_WGM1_10_MASK)
	{
		// TODO
		avr8_update_timer1_waveform_gen_mode(cpustate);
	}
}

static void avr8_update_timer1_input_noise_canceler(avr8_state *cpustate)
{
	// TODO
}

static void avr8_update_timer1_input_edge_select(avr8_state *cpustate)
{
	// TODO
	//verboselog(cpustate->pc, 0, "avr8_update_timer1_input_edge_select: TODO; Clocking edge is %s\n", "test");
}

static void avr8_update_timer1_clock_source(avr8_state *cpustate)
{
	switch(AVR8_TIMER1_CLOCK_SELECT)
	{
		case 0: // Counter stopped
			cpustate->timer1_prescale = 0;
			break;
		case 1: // Clk/1; no prescaling
			cpustate->timer1_prescale = 1;
			break;
		case 2: // Clk/8
			cpustate->timer1_prescale = 8;
			break;
		case 3: // Clk/32
			cpustate->timer1_prescale = 32;
			break;
		case 4: // Clk/64
			cpustate->timer1_prescale = 64;
			break;
		case 5: // Clk/128
			cpustate->timer1_prescale = 128;
			break;
		case 6: // T1 trigger, falling edge
		case 7: // T1 trigger, rising edge
			cpustate->timer1_prescale = 0;
			verboselog(cpustate->pc, 0, "avr8_update_timer1_clock_source: T1 Trigger mode not implemented yet\n");
			break;
	}

	if (cpustate->timer1_prescale_count > cpustate->timer1_prescale)
	{
		cpustate->timer1_prescale_count = cpustate->timer1_prescale - 1;
	}
}

static void avr8_changed_tccr1b(avr8_state *cpustate, UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR1B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

    cpustate->r[AVR8_REGIDX_TCCR1B] = newtccr;

	if(changed & AVR8_TCCR1B_ICNC1_MASK)
	{
		// TODO
		avr8_update_timer1_input_noise_canceler(cpustate);
	}

	if(changed & AVR8_TCCR1B_ICES1_MASK)
	{
		// TODO
		avr8_update_timer1_input_edge_select(cpustate);
	}

	if(changed & AVR8_TCCR1B_WGM1_32_MASK)
	{
		// TODO
		avr8_update_timer1_waveform_gen_mode(cpustate);
	}

	if(changed & AVR8_TCCR1B_CS_MASK)
	{
		avr8_update_timer1_clock_source(cpustate);
	}
}

static void avr8_update_ocr1(avr8_state *cpustate, UINT16 newval, UINT8 reg)
{
	UINT8 *p_reg_h = (reg == AVR8_REG_A) ? &cpustate->r[AVR8_REGIDX_OCR1AH] : &cpustate->r[AVR8_REGIDX_OCR1BH];
	UINT8 *p_reg_l = (reg == AVR8_REG_A) ? &cpustate->r[AVR8_REGIDX_OCR1AL] : &cpustate->r[AVR8_REGIDX_OCR1BL];
	*p_reg_h = (UINT8)(newval >> 8);
	*p_reg_l = (UINT8)newval;

    // Nothing needs to be done? All handled in timer callback
}

// Timer 2 Handling

static void avr8_timer2_tick(avr8_state *cpustate)
{
    UINT16 count = cpustate->r[AVR8_REGIDX_TCNT2];
    INT32 wgm2 = ((cpustate->r[AVR8_REGIDX_TCCR2B] & AVR8_TCCR2B_WGM2_2_MASK) >> 1) |
                 (cpustate->r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_WGM2_10_MASK);

    // Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
    //UINT8 compare_mode[2] = { (cpustate->r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT,
                              //(cpustate->r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT };
    UINT8 ocr2[2] = { cpustate->r[AVR8_REGIDX_OCR2A], cpustate->r[AVR8_REGIDX_OCR2B] };
	UINT8 ocf2[2] = { (1 << AVR8_TIFR2_OCF2A_SHIFT), (1 << AVR8_TIFR2_OCF2B_SHIFT) };
    INT32 increment = cpustate->timer2_increment;

    for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
    {
        switch(wgm2)
        {
            case WGM2_FAST_PWM_CMP:
                if(count == ocr2[reg])
                {
                    if (reg == 0)
                    {
                        cpustate->r[AVR8_REGIDX_TIFR2] |= AVR8_TIFR2_TOV2_MASK;
                        count = 0;
                        increment = 0;
                    }

                    cpustate->r[AVR8_REGIDX_TIFR2] |= ocf2[reg];
                }
                else if(count == 0)
                {
                    if (reg == 0)
                    {
                        cpustate->r[AVR8_REGIDX_TIFR2] &= ~AVR8_TIFR2_TOV2_MASK;
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
                //verboselog(cpustate->pc, 0, "avr8_update_timer2_compare_mode: Normal port operation (OC2 disconnected)\n");
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

    cpustate->r[AVR8_REGIDX_TCNT2] = count + increment;

	avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF2A);
	avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF2B);
	avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_TOV2);
}

static void avr8_update_timer2_waveform_gen_mode(avr8_state *cpustate)
{
    cpustate->timer2_top = 0;
	switch(AVR8_WGM2)
	{
		case WGM2_NORMAL:
		case WGM2_PWM_PC:
		case WGM2_FAST_PWM:
			cpustate->timer2_top = 0x00ff;
			break;

		case WGM2_CTC_CMP:
		case WGM2_PWM_PC_CMP:
		case WGM2_FAST_PWM_CMP:
			cpustate->timer2_top = AVR8_OCR2A;
			break;

		default:
			verboselog(cpustate->pc, 0, "avr8_update_timer2_waveform_gen_mode: Unsupported waveform generation type: %d\n", AVR8_WGM2);
			break;
	}
}

static void avr8_changed_tccr2a(avr8_state *cpustate, UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR2A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	if(changed & AVR8_TCCR2A_WGM2_10_MASK)
	{
		// TODO
		avr8_update_timer2_waveform_gen_mode(cpustate);
	}
}

static void avr8_update_timer2_clock_source(avr8_state *cpustate)
{
	switch(AVR8_TIMER2_CLOCK_SELECT)
	{
		case 0: // Counter stopped
			cpustate->timer2_prescale = 0;
			break;
		case 1: // Clk/1; no prescaling
			cpustate->timer2_prescale = 1;
			break;
		case 2: // Clk/8
			cpustate->timer2_prescale = 8;
			break;
		case 3: // Clk/32
			cpustate->timer2_prescale = 32;
			break;
		case 4: // Clk/64
			cpustate->timer2_prescale = 64;
			break;
		case 5: // Clk/128
			cpustate->timer2_prescale = 128;
			break;
		case 6: // Clk/256
			cpustate->timer2_prescale = 256;
			break;
		case 7: // Clk/1024
			cpustate->timer2_prescale = 1024;
			break;
	}

	if (cpustate->timer2_prescale_count > cpustate->timer2_prescale)
	{
		cpustate->timer2_prescale_count = cpustate->timer2_prescale - 1;
	}
}

static void avr8_timer2_force_output_compare(avr8_state *cpustate, int reg)
{
	// TODO
	verboselog(cpustate->pc, 0, "avr8_force_output_compare: TODO; should be forcing OC2%c\n", avr8_reg_name[reg]);
}

static void avr8_changed_tccr2b(avr8_state *cpustate, UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR2B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	if(changed & AVR8_TCCR2B_FOC2A_MASK)
	{
		// TODO
		avr8_timer2_force_output_compare(cpustate, AVR8_REG_A);
	}

	if(changed & AVR8_TCCR2B_FOC2B_MASK)
	{
		// TODO
		avr8_timer2_force_output_compare(cpustate, AVR8_REG_B);
	}

	if(changed & AVR8_TCCR2B_WGM2_2_MASK)
	{
		// TODO
		avr8_update_timer2_waveform_gen_mode(cpustate);
	}

	if(changed & AVR8_TCCR2B_CS_MASK)
	{
		avr8_update_timer2_clock_source(cpustate);
	}
}

static void avr8_update_ocr2(avr8_state *cpustate, UINT8 newval, UINT8 reg)
{
    cpustate->r[(reg == AVR8_REG_A) ? AVR8_REGIDX_OCR2A : AVR8_REGIDX_OCR2B] = newval;

    // Nothing needs to be done? All handled in timer callback
}

/*****************************************************************************/

static bool avr8_io_reg_write(avr8_state *cpustate, UINT16 offset, UINT8 data)
{
    switch( offset )
    {
		case AVR8_REGIDX_OCR1BH:
			verboselog(cpustate->pc, 0, "AVR8: OCR1BH = %02x\n", data );
			avr8_update_ocr1(cpustate, (AVR8_OCR1B & 0x00ff) | (data << 8), AVR8_REG_B);
			return true;

		case AVR8_REGIDX_OCR1BL:
			verboselog(cpustate->pc, 0, "AVR8: OCR1BL = %02x\n", data );
			avr8_update_ocr1(cpustate, (AVR8_OCR1B & 0xff00) | data, AVR8_REG_B);
			return true;

		case AVR8_REGIDX_OCR1AH:
			verboselog(cpustate->pc, 0, "AVR8: OCR1AH = %02x\n", data );
			avr8_update_ocr1(cpustate, (AVR8_OCR1A & 0x00ff) | (data << 8), AVR8_REG_A);
			return true;

		case AVR8_REGIDX_OCR1AL:
			verboselog(cpustate->pc, 0, "AVR8: OCR1AL = %02x\n", data );
			avr8_update_ocr1(cpustate, (AVR8_OCR1A & 0xff00) | data, AVR8_REG_A);
			return true;

		case AVR8_REGIDX_TCCR1B:
			verboselog(cpustate->pc, 0, "AVR8: TCCR1B = %02x\n", data );
			avr8_changed_tccr1b(cpustate, data);
			return true;

		case AVR8_REGIDX_TCCR1A:
			verboselog(cpustate->pc, 0, "AVR8: TCCR1A = %02x\n", data );
			avr8_changed_tccr1a(cpustate, data);
			return true;

		case AVR8_REGIDX_TIMSK1:
			verboselog(cpustate->pc, 0, "AVR8: TIMSK1 = %02x\n", data );
			avr8_change_timsk1(cpustate, data);
			return true;

		case AVR8_REGIDX_TIFR1:
			verboselog(cpustate->pc, 0, "AVR8: TIFR1 = %02x\n", data );
			cpustate->r[AVR8_REGIDX_TIFR1] &= ~(data & AVR8_TIFR1_MASK);
			avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_ICF1);
			avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF1A);
			avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_OCF1B);
			avr8_update_interrupt_internal(cpustate, AVR8_INTIDX_TOV1);
			return true;

		case AVR8_REGIDX_TCCR2B:
			verboselog(cpustate->pc, 0, "AVR8: TCCR2B = %02x\n", data );
			avr8_changed_tccr2b(cpustate, data);
			return true;

		case AVR8_REGIDX_TCCR2A:
			verboselog(cpustate->pc, 0, "AVR8: TCCR2A = %02x\n", data );
			avr8_changed_tccr2a(cpustate, data);
			return true;

		case AVR8_REGIDX_OCR2A:
			avr8_update_ocr2(cpustate, data, AVR8_REG_A);
			return true;

		case AVR8_REGIDX_OCR2B:
			avr8_update_ocr2(cpustate, data, AVR8_REG_B);
			return true;

        case AVR8_REGIDX_TCNT2:
            AVR8_TCNT2 = data;
            return true;

        case AVR8_REGIDX_GTCCR:
        	if (data & AVR8_GTCCR_PSRASY_MASK)
        	{
				data &= ~AVR8_GTCCR_PSRASY_MASK;
				cpustate->timer2_prescale_count = 0;
			}
            //verboselog(cpustate->pc, 0, "AVR8: GTCCR = %02x\n", data );
            // TODO
            return true;

		case AVR8_REGIDX_SPL:
		case AVR8_REGIDX_SPH:
			cpustate->r[offset] = data;
			return true;

		case AVR8_REGIDX_GPIOR0:
			verboselog(cpustate->pc, 0, "AVR8: GPIOR0 Write: %02x\n", data);
			cpustate->r[offset] = data;
			return true;

		case AVR8_REGIDX_SREG:
			cpustate->status = data;
			return true;

        default:
            return false;
    }
    return false;
}

static bool avr8_io_reg_read(avr8_state *cpustate, UINT16 offset, UINT8 *data)
{
    switch( offset )
    {
		case AVR8_REGIDX_SPL:
		case AVR8_REGIDX_SPH:
		case AVR8_REGIDX_TCNT1L:
		case AVR8_REGIDX_TCNT1H:
		case AVR8_REGIDX_TCNT2:
			*data = cpustate->r[offset];
			return true;

		case AVR8_REGIDX_GPIOR0:
			*data = cpustate->r[offset];
			verboselog(cpustate->pc, 0, "AVR8: GPIOR0 Read: %02x\n", *data);
			return true;

		case AVR8_REGIDX_SREG:
			*data = cpustate->status;
			return true;

        default:
            return false;
    }

    return false;
}

/*****************************************************************************/

static CPU_INIT( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

    cpustate->pc = 0;

    cpustate->device = device;
    cpustate->program = &device->space(AS_PROGRAM);
    cpustate->io = &device->space(AS_IO);

	cpustate->status = 0;
    WRITE_IO_8(cpustate, AVR8_REGIDX_SPL, 0);
    WRITE_IO_8(cpustate, AVR8_REGIDX_SPH, 0);

	for (int i = 0; i < 32; i++)
	{
		cpustate->r[i] = 0;
	}

    cpustate->timer0_top = 0;
	cpustate->timer0_increment = 1;
	cpustate->timer0_prescale = 0;
	cpustate->timer0_prescale_count = 0;

    cpustate->timer1_top = 0;
	cpustate->timer1_increment = 1;
	cpustate->timer1_prescale = 0;
	cpustate->timer1_prescale_count = 0;

    cpustate->timer2_top = 0;
	cpustate->timer2_increment = 1;
	cpustate->timer2_prescale = 0;
	cpustate->timer2_prescale_count = 0;

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

	cpustate->interrupt_pending = false;

	cpustate->elapsed_cycles = 0;

	device->save_item(NAME(cpustate->pc));
}

static CPU_EXIT( avr8 )
{
}

static CPU_RESET( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

	cpustate->status = 0;
    cpustate->pc = 0;
    cpustate->elapsed_cycles = 0;

    cpustate->interrupt_pending = false;
}

static CPU_EXECUTE( avr8 )
{
    UINT32 op = 0;
    INT32 offs = 0;
    UINT8 rd = 0;
    UINT8 rr = 0;
    UINT8 res = 0;
    UINT16 pd = 0;
    INT16 sd = 0;
    INT32 opcycles = 1;
    //UINT16 pr = 0;
    avr8_state *cpustate = get_safe_token(device);

    while (cpustate->icount > 0)
    {
		opcycles = 1;

        cpustate->pc &= cpustate->addr_mask;

        debugger_instruction_hook(device, cpustate->pc << 1);

        op = (UINT32)READ_PRG_16(cpustate, cpustate->pc);

        switch(op & 0xf000)
        {
            case 0x0000:
                switch(op & 0x0f00)
                {
                    case 0x0000:    // NOP
                        break;
                    case 0x0100:    // MOVW Rd+1:Rd,Rr+1:Rd
                    	cpustate->r[(RD4(op) << 1) + 1] = cpustate->r[(RR4(op) << 1) + 1];
                    	cpustate->r[RD4(op) << 1] = cpustate->r[RR4(op) << 1];
                        break;
                    case 0x0200:    // MULS Rd,Rr
                        sd = (INT8)cpustate->r[16 + RD4(op)] * (INT8)cpustate->r[16 + RR4(op)];
                        cpustate->r[1] = (sd >> 8) & 0x00ff;
                        cpustate->r[0] = sd & 0x00ff;
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        break;
                    case 0x0300:    // Multiplicatn
                    	switch(MULCONST2(op))
                    	{
							case 0x0000: // MULSU Rd,Rr
								sd = (INT8)cpustate->r[16 + RD3(op)] * (UINT8)cpustate->r[16 + RR3(op)];
								cpustate->r[1] = (sd >> 8) & 0x00ff;
								cpustate->r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0001: // FMUL Rd,Rr
								sd = (UINT8)cpustate->r[16 + RD3(op)] * (UINT8)cpustate->r[16 + RR3(op)];
								sd <<= 1;
								cpustate->r[1] = (sd >> 8) & 0x00ff;
								cpustate->r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0002: // FMULS Rd,Rr
								sd = (INT8)cpustate->r[16 + RD3(op)] * (INT8)cpustate->r[16 + RR3(op)];
								sd <<= 1;
								cpustate->r[1] = (sd >> 8) & 0x00ff;
								cpustate->r[0] = sd & 0x00ff;
								SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
								SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
								opcycles = 2;
								break;
							case 0x0003: // FMULSU Rd,Rr
								sd = (INT8)cpustate->r[16 + RD3(op)] * (UINT8)cpustate->r[16 + RR3(op)];
								sd <<= 1;
								cpustate->r[1] = (sd >> 8) & 0x00ff;
								cpustate->r[0] = sd & 0x00ff;
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
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
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
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        res = rd - (rr + SREG_R(AVR8_SREG_C));
                        cpustate->r[RD5(op)] = res;
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
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        res = rd + rr;
                        cpustate->r[RD5(op)] = res;
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
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0400:    // CP Rd,Rr
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        res = rd - rr;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0800:    // SUB Rd,Rr
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        res = rd - rr;
                        cpustate->r[RD5(op)] = res;
                        SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                        SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                        SREG_W(AVR8_SREG_N, BIT(res,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                        break;
                    case 0x0c00:    // ADC Rd,Rr
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        res = rd + rr + SREG_R(AVR8_SREG_C);
                        cpustate->r[RD5(op)] = res;
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
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        rd &= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        cpustate->r[RD5(op)] = rd;
                        break;
                    case 0x0400:    // EOR Rd,Rr
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        rd ^= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        cpustate->r[RD5(op)] = rd;
                        break;
                    case 0x0800:    // OR Rd,Rr
                        rd = cpustate->r[RD5(op)];
                        rr = cpustate->r[RR5(op)];
                        rd |= rr;
                        SREG_W(AVR8_SREG_V, 0);
                        SREG_W(AVR8_SREG_N, BIT(rd,7));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                        cpustate->r[RD5(op)] = rd;
                        break;
                    case 0x0c00:    // MOV Rd,Rr
                    	cpustate->r[RD5(op)] = cpustate->r[RR5(op)];
                        break;
                }
                break;
            case 0x3000:    // CPI Rd,K
                rd = cpustate->r[16 + RD4(op)];
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
                rd = cpustate->r[16 + RD4(op)];
                rr = KCONST8(op);
                res = rd - (rr + SREG_R(AVR8_SREG_C));
                cpustate->r[16 + RD4(op)] = res;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x5000:    // SUBI Rd,K
                rd = cpustate->r[16 + RD4(op)];
                rr = KCONST8(op);
                res = rd - rr;
                cpustate->r[16 + RD4(op)] = res;
                SREG_W(AVR8_SREG_H, (NOT(BIT(rd,3)) & BIT(rr,3)) | (BIT(rr,3) & BIT(res,3)) | (BIT(res,3) & NOT(BIT(rd,3))));
                SREG_W(AVR8_SREG_V, (BIT(rd,7) & NOT(BIT(rr,7)) & NOT(BIT(res,7))) | (NOT(BIT(rd,7)) & BIT(rr,7) & BIT(res,7)));
                SREG_W(AVR8_SREG_N, BIT(res,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                SREG_W(AVR8_SREG_C, (NOT(BIT(rd,7)) & BIT(rr,7)) | (BIT(rr,7) & BIT(res,7)) | (BIT(res,7) & NOT(BIT(rd,7))));
                break;
            case 0x6000:    // ORI Rd,K
                rd = cpustate->r[16 + RD4(op)];
                rr = KCONST8(op);
                rd |= rr;
                SREG_W(AVR8_SREG_V, 0);
                SREG_W(AVR8_SREG_N, BIT(rd,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                cpustate->r[16 + RD4(op)] = rd;
                break;
            case 0x7000:    // ANDI Rd,K
                rd = cpustate->r[16 + RD4(op)];
                rr = KCONST8(op);
                rd &= rr;
                SREG_W(AVR8_SREG_V, 0);
                SREG_W(AVR8_SREG_N, BIT(rd,7));
                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                SREG_W(AVR8_SREG_Z, (rd == 0) ? 1 : 0);
                cpustate->r[16 + RD4(op)] = rd;
                break;
            case 0x8000:
            case 0xa000:
                switch(op & 0x0208)
                {
                    case 0x0000:    // LDD Rd,Z+q
                        cpustate->r[RD5(op)] = READ_IO_8(cpustate, ZREG + QCONST6(op));
                        opcycles = 2;
                        break;
                    case 0x0008:    // LDD Rd,Y+q
                        cpustate->r[RD5(op)] = READ_IO_8(cpustate, YREG + QCONST6(op));
                        opcycles = 2;
                        break;
                    case 0x0200:    // STD Z+q,Rr
                        WRITE_IO_8(cpustate, ZREG + QCONST6(op), cpustate->r[RD5(op)]);
                        opcycles = 2;
                        break;
                    case 0x0208:    // STD Y+q,Rr
                        WRITE_IO_8(cpustate, YREG + QCONST6(op), cpustate->r[RD5(op)]);
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
                                cpustate->pc++;
                                op |= READ_PRG_16(cpustate, cpustate->pc);
                                cpustate->r[RD5(op >> 16)] = READ_IO_8(cpustate, op & 0x0000ffff);
                                opcycles = 2;
                                break;
                            case 0x0001:    // LD Rd,Z+
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0002:    // LD Rd,-Z
                                pd = ZREG;
                                pd--;
                                cpustate->r[RD5(op)] = READ_IO_8(cpustate, pd);
                                cpustate->r[31] = (pd >> 8) & 0x00ff;
                                cpustate->r[30] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x0004:    // LPM Rd,Z
                                cpustate->r[RD5(op)] = READ_PRG_8(cpustate, ZREG);
                                opcycles = 3;
                                break;
                            case 0x0005:    // LPM Rd,Z+
                                pd = ZREG;
                                cpustate->r[RD5(op)] = READ_PRG_8(cpustate, pd);
                                pd++;
                                cpustate->r[31] = (pd >> 8) & 0x00ff;
                                cpustate->r[30] = pd & 0x00ff;
                                opcycles = 3;
                                break;
                            case 0x0006:    // ELPM Rd,Z
                                //output += sprintf( output, "ELPM    R%d, Z", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0007:    // ELPM Rd,Z+
                                //output += sprintf( output, "ELPM    R%d, Z+", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0009:    // LD Rd,Y+
                                pd = YREG;
                                cpustate->r[RD5(op)] = READ_IO_8(cpustate, pd);
                                pd++;
                                cpustate->r[29] = (pd >> 8) & 0x00ff;
                                cpustate->r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000a:    // LD Rd,-Y
                                pd = YREG;
                                pd--;
                                cpustate->r[RD5(op)] = READ_IO_8(cpustate, pd);
                                cpustate->r[29] = (pd >> 8) & 0x00ff;
                                cpustate->r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000c:    // LD Rd,X
                            	cpustate->r[RD5(op)] = READ_IO_8(cpustate, XREG);
                            	opcycles = 2;
                                break;
                            case 0x000d:    // LD Rd,X+
                                pd = XREG;
                                cpustate->r[RD5(op)] = READ_IO_8(cpustate, pd);
                                pd++;
                                cpustate->r[27] = (pd >> 8) & 0x00ff;
                                cpustate->r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000e:    // LD Rd,-X
                                pd = XREG;
                                pd--;
                                cpustate->r[RD5(op)] = READ_IO_8(cpustate, pd);
                                cpustate->r[27] = (pd >> 8) & 0x00ff;
                                cpustate->r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000f:    // POP Rd
                                cpustate->r[RD5(op)] = POP(cpustate);
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
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
                                cpustate->pc++;
                                op |= READ_PRG_16(cpustate, cpustate->pc);
                                WRITE_IO_8(cpustate, op & 0x0000ffff, cpustate->r[RD5(op >> 16)]);
                                opcycles = 2;
                                break;
                            case 0x0001:    // ST Z+,Rd
                                //output += sprintf( output, "ST       Z+, R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0002:    // ST -Z,Rd
                                //output += sprintf( output, "ST      -Z , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x0009:    // ST Y+,Rd
                                pd = YREG;
                                WRITE_IO_8(cpustate, pd, cpustate->r[RD5(op)]);
                                pd++;
                                cpustate->r[29] = (pd >> 8) & 0x00ff;
                                cpustate->r[28] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000a:    // ST -Z,Rd
                                //output += sprintf( output, "ST      -Y , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000c:    // ST X,Rd
                                WRITE_IO_8(cpustate, XREG, cpustate->r[RD5(op)]);
                                break;
                            case 0x000d:    // ST X+,Rd
                                pd = XREG;
                                WRITE_IO_8(cpustate, pd, cpustate->r[RD5(op)]);
                                pd++;
                                cpustate->r[27] = (pd >> 8) & 0x00ff;
                                cpustate->r[26] = pd & 0x00ff;
                                opcycles = 2;
                                break;
                            case 0x000e:    // ST -X,Rd
                                //output += sprintf( output, "ST      -X , R%d", RD5(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000f:    // PUSH Rd
                                PUSH(cpustate, cpustate->r[RD5(op)]);
                                opcycles = 2;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0400:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = cpustate->r[RD5(op)];
                                res = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0001:    // NEG Rd
                                rd = cpustate->r[RD5(op)];
                                res = 0 - rd;
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0002:    // SWAP Rd
                            	rd = cpustate->r[RD5(op)];
                            	cpustate->r[RD5(op)] = (rd >> 4) | (rd << 4);
                                break;
                            case 0x0003:    // INC Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0005:    // ASR Rd
                                rd = cpustate->r[RD5(op)];
                                res = (rd & 0x80) | (rd >> 1);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(rd,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0006:    // LSR Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0007:    // ROR Rd
                            	rd = cpustate->r[RD5(op)];
                            	res = rd >> 1;
                            	res |= (SREG_R(AVR8_SREG_C) << 7);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
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
                                        cpustate->pc = ZREG - 1;
                                        opcycles = 2;
                                        break;
                                    case 0x0010:    // EIJMP
                                        //output += sprintf( output, "EIJMP" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
								offs = KCONST22(op) << 16;
                                cpustate->pc++;
                                offs |= READ_PRG_16(cpustate, cpustate->pc);
                                cpustate->pc = offs;
								cpustate->pc--;
								opcycles = 3;
                                break;
                            case 0x000e:    // CALL k
                            case 0x000f:
								PUSH(cpustate, ((cpustate->pc + 1) >> 8) & 0x00ff);
								PUSH(cpustate, (cpustate->pc + 1) & 0x00ff);
								offs = KCONST22(op) << 16;
                                cpustate->pc++;
                                offs |= READ_PRG_16(cpustate, cpustate->pc);
                                cpustate->pc = offs;
								cpustate->pc--;
								opcycles = 4;
                                break;
                            default:
                                unimplemented_opcode(cpustate, op);
                                //output += sprintf( output, "Undefined (%04x)", op );
                                break;
                        }
                        break;
                    case 0x0500:
                        switch(op & 0x000f)
                        {
                            case 0x0000:    // COM Rd
                                rd = cpustate->r[RD5(op)];
                                res = ~rd;
                                SREG_W(AVR8_SREG_C, 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0001:    // NEG Rd
                                rd = cpustate->r[RD5(op)];
                                res = 0 - rd;
                                SREG_W(AVR8_SREG_C, (res == 0) ? 0 : 1);
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, (res == 0x80) ? 1 : 0);
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_H, BIT(res,3) | BIT(rd,3));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0002:    // SWAP Rd
                            	rd = cpustate->r[RD5(op)];
                            	cpustate->r[RD5(op)] = (rd >> 4) | (rd << 4);
                                break;
                            case 0x0003:    // INC Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd + 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0005:    // ASR Rd
                                rd = cpustate->r[RD5(op)];
                                res = (rd & 0x80) | (rd >> 1);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(rd,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0006:    // LSR Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd >> 1;
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, 0);
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0007:    // ROR Rd
                            	rd = cpustate->r[RD5(op)];
                            	res = rd >> 1;
                            	res |= (SREG_R(AVR8_SREG_C) << 7);
                                SREG_W(AVR8_SREG_C, BIT(rd,0));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 :0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_V, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_C));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x0008:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // RET
                                        cpustate->pc = POP(cpustate);
                                        cpustate->pc |= POP(cpustate) << 8;
                                        cpustate->pc--;
                                        opcycles = 4;
                                        break;
                                    case 0x0010:    // RETI
                                        cpustate->pc = POP(cpustate);
                                        cpustate->pc |= POP(cpustate) << 8;
                                        //printf("Pop: %04x\n", cpustate->pc);
                                        cpustate->pc--;
                                        SREG_W(AVR8_SREG_I, 1);
                                        /*if (cpustate->interrupt_pending)
                                        {
                                            avr8_poll_interrupt(cpustate);
                                            cpustate->interrupt_pending = false;
                                        }*/
                                        opcycles = 4;
                                        break;
                                    case 0x0080:    // SLEEP
                                        //output += sprintf( output, "SLEEP" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x0090:    // BREAK
                                        //output += sprintf( output, "BREAK" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00a0:    // WDR
                                        //output += sprintf( output, "WDR" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00c0:    // LPM
                                        cpustate->r[0] = READ_PRG_8(cpustate, ZREG);
                                        opcycles = 3;
                                        break;
                                    case 0x00d0:    // ELPM
                                        //output += sprintf( output, "ELPM" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00e0:    // SPM
                                        //output += sprintf( output, "SPM" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x00f0:    // SPM Z+
                                        //output += sprintf( output, "SPM     Z+" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        unimplemented_opcode(cpustate, op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x0009:
                                switch(op & 0x00f0)
                                {
                                    case 0x0000:    // ICALL
                                        //output += sprintf( output, "ICALL" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    case 0x0010:    // EICALL
                                        //output += sprintf( output, "EICALL" );
                                        unimplemented_opcode(cpustate, op);
                                        break;
                                    default:
                                        unimplemented_opcode(cpustate, op);
                                        //output += sprintf( output, "Undefined (%04x)", op );
                                        break;
                                }
                                break;
                            case 0x000a:    // DEC Rd
                                rd = cpustate->r[RD5(op)];
                                res = rd - 1;
                                SREG_W(AVR8_SREG_V, (rd == 0x7f) ? 1 : 0);
                                SREG_W(AVR8_SREG_N, BIT(res,7));
                                SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                                SREG_W(AVR8_SREG_Z, (res == 0) ? 1 : 0);
                                cpustate->r[RD5(op)] = res;
                                break;
                            case 0x000c:
                            case 0x000d:    // JMP k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "JMP     0x%06x", KCONST22(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                            case 0x000e:
                            case 0x000f:    // CALL k
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //op <<= 8;
                                //op |= oprom[pos++];
                                //output += sprintf( output, "CALL    0x%06x", KCONST22(op) );
                                unimplemented_opcode(cpustate, op);
                                break;
                        }
                        break;
                    case 0x0600:    // ADIW Rd+1:Rd,K
                        rd = cpustate->r[24 + (DCONST(op) << 1)];
                        rr = cpustate->r[25 + (DCONST(op) << 1)];
                        pd = rd;
                        pd |= rr << 8;
                        pd += KCONST6(op);
                        SREG_W(AVR8_SREG_V, BIT(pd,15) & NOT(BIT(rr,7)));
                        SREG_W(AVR8_SREG_N, BIT(pd,15));
                        SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
                        SREG_W(AVR8_SREG_Z, (pd == 0) ? 1 : 0);
                        SREG_W(AVR8_SREG_C, NOT(BIT(pd,15)) & BIT(rr,7));
                        cpustate->r[24 + (DCONST(op) << 1)] = pd & 0x00ff;
                        cpustate->r[25 + (DCONST(op) << 1)] = (pd >> 8) & 0x00ff;
                        opcycles = 2;
                        break;
                    case 0x0700:    // SBIW Rd+1:Rd,K
                        //output += sprintf( output, "SBIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
                        unimplemented_opcode(cpustate, op);
                        break;
                    case 0x0800:    // CBI A,b
                        //output += sprintf( output, "CBI     0x%02x, %d", ACONST5(op), RR3(op) );
                        WRITE_IO_8(cpustate, 32 + ACONST5(op), READ_IO_8(cpustate, 32 + ACONST5(op)) &~ (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0900:    // SBIC A,b
                		if(NOT(BIT(READ_IO_8(cpustate, 32 + ACONST5(op)), RR3(op))))
                		{
                            op = (UINT32)READ_PRG_16(cpustate, cpustate->pc + 1);
							opcycles = avr8_is_long_opcode(op) ? 3 : 2;
                            cpustate->pc += avr8_is_long_opcode(op) ? 2 : 1;
						}
                        break;
                    case 0x0a00:    // SBI A,b
                        WRITE_IO_8(cpustate, 32 + ACONST5(op), READ_IO_8(cpustate, 32 + ACONST5(op)) | (1 << RR3(op)));
                        opcycles = 2;
                        break;
                    case 0x0b00:    // SBIS A,b
                		if(BIT(READ_IO_8(cpustate, 32 + ACONST5(op)), RR3(op)))
                		{
                            op = (UINT32)READ_PRG_16(cpustate, cpustate->pc + 1);
							opcycles = avr8_is_long_opcode(op) ? 3 : 2;
                            cpustate->pc += avr8_is_long_opcode(op) ? 2 : 1;
						}
                        break;
                    case 0x0c00:
                    case 0x0d00:
                    case 0x0e00:
                    case 0x0f00:    // MUL Rd,Rr
                        sd = (UINT8)cpustate->r[RD5(op)] * (UINT8)cpustate->r[RR5(op)];
                        cpustate->r[1] = (sd >> 8) & 0x00ff;
                        cpustate->r[0] = sd & 0x00ff;
                        SREG_W(AVR8_SREG_C, (sd & 0x8000) ? 1 : 0);
                        SREG_W(AVR8_SREG_Z, (sd == 0) ? 1 : 0);
                        opcycles = 2;
                        break;
                }
                break;
            case 0xb000:
                if(op & 0x0800) // OUT A,Rr
                {
                    WRITE_IO_8(cpustate, 32 + ACONST6(op), cpustate->r[RD5(op)]);
                }
                else            // IN Rd,A
                {
                    cpustate->r[RD5(op)] = READ_IO_8(cpustate, 0x20 + ACONST6(op));
                }
                break;
            case 0xc000:    // RJMP k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                cpustate->pc += offs;
                opcycles = 2;
                break;
            case 0xd000:    // RCALL k
                offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
                PUSH(cpustate, ((cpustate->pc + 1) >> 8) & 0x00ff);
                PUSH(cpustate, (cpustate->pc + 1) & 0x00ff);
                cpustate->pc += offs;
                opcycles = 3;
                break;
            case 0xe000:    // LDI Rd,K
                cpustate->r[16 + RD4(op)] = KCONST8(op);
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
                            cpustate->pc += offs;
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
                            cpustate->pc += offs;
                            opcycles = 2;
                        }
                        break;
                    case 0x0800:
                        if(op & 0x0200) // BST Rd, b
                        {
                            SREG_W(AVR8_SREG_T, (BIT(cpustate->r[RD5(op)], RR3(op))) ? 1 : 0);
                        }
                        else            // BLD Rd, b
                        {
                            if(SREG_R(AVR8_SREG_T))
                            {
                                cpustate->r[RD5(op)] |= (1 << RR3(op));
                            }
                            else
                            {
                                cpustate->r[RD5(op)] &= ~(1 << RR3(op));
                            }
                        }
                        break;
                    case 0x0c00:
                        if(op & 0x0200) // SBRS Rd, b
                        {
                            if(BIT(cpustate->r[RD5(op)], RR3(op)))
                            {
                                op = (UINT32)READ_PRG_16(cpustate, cpustate->pc++);
                                cpustate->pc += avr8_is_long_opcode(op) ? 1 : 0;
                                opcycles = avr8_is_long_opcode(op) ? 3 : 2;
                            }
                        }
                        else            // SBRC Rd, b
                        {
                            if(NOT(BIT(cpustate->r[RD5(op)], RR3(op))))
                            {
                                op = (UINT32)READ_PRG_16(cpustate, cpustate->pc++);
                                cpustate->pc += avr8_is_long_opcode(op) ? 1 : 0;
                                opcycles = avr8_is_long_opcode(op) ? 3 : 2;
                            }
                        }
                        break;
                }
                break;
        }

        cpustate->pc++;

        cpustate->icount -= opcycles;

		cpustate->elapsed_cycles += opcycles;

		avr8_timer_tick(cpustate, opcycles);
    }
}

/*****************************************************************************/

static CPU_SET_INFO( avr8 )
{
    avr8_state *cpustate = get_safe_token(device);

    switch (state)
    {
        /* interrupt lines/exceptions */
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_RESET:          avr8_set_irq_line(cpustate, AVR8_INT_RESET, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_INT0:           avr8_set_irq_line(cpustate, AVR8_INT_INT0, info->i);        break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_INT1:           avr8_set_irq_line(cpustate, AVR8_INT_INT1, info->i);        break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT0:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT0, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT1:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT1, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_PCINT2:         avr8_set_irq_line(cpustate, AVR8_INT_PCINT2, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_WDT:            avr8_set_irq_line(cpustate, AVR8_INT_WDT, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T2COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T2COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T2OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T2OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1CAPT:         avr8_set_irq_line(cpustate, AVR8_INT_T1CAPT, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T1COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T1COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T1OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T1OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0COMPA:        avr8_set_irq_line(cpustate, AVR8_INT_T0COMPA, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0COMPB:        avr8_set_irq_line(cpustate, AVR8_INT_T0COMPB, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_T0OVF:          avr8_set_irq_line(cpustate, AVR8_INT_T0OVF, info->i);       break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_SPI_STC:        avr8_set_irq_line(cpustate, AVR8_INT_SPI_STC, info->i);     break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_RX:       avr8_set_irq_line(cpustate, AVR8_INT_USART_RX, info->i);    break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_UDRE:     avr8_set_irq_line(cpustate, AVR8_INT_USART_UDRE, info->i);  break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_USART_TX:       avr8_set_irq_line(cpustate, AVR8_INT_USART_TX, info->i);    break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_ADC:            avr8_set_irq_line(cpustate, AVR8_INT_ADC, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_EE_RDY:         avr8_set_irq_line(cpustate, AVR8_INT_EE_RDY, info->i);      break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_ANALOG_COMP:    avr8_set_irq_line(cpustate, AVR8_INT_ANALOG_COMP, info->i); break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_TWI:            avr8_set_irq_line(cpustate, AVR8_INT_TWI, info->i);         break;
        case CPUINFO_INT_INPUT_STATE + AVR8_INT_SPM_RDY:        avr8_set_irq_line(cpustate, AVR8_INT_SPM_RDY, info->i);     break;

        /* --- the following bits of info are set as 64-bit signed integers --- */
        case CPUINFO_INT_PC:    /* intentional fallthrough */
        case CPUINFO_INT_REGISTER + AVR8_PC:            cpustate->pc = info->i;                 break;
        case CPUINFO_INT_REGISTER + AVR8_SREG:          cpustate->status = info->i; 			break;
        case CPUINFO_INT_REGISTER + AVR8_R0:            cpustate->r[ 0] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R1:            cpustate->r[ 1] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R2:            cpustate->r[ 2] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R3:            cpustate->r[ 3] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R4:            cpustate->r[ 4] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R5:            cpustate->r[ 5] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R6:            cpustate->r[ 6] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R7:            cpustate->r[ 7] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R8:            cpustate->r[ 8] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R9:            cpustate->r[ 9] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R10:           cpustate->r[10] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R11:           cpustate->r[11] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R12:           cpustate->r[12] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R13:           cpustate->r[13] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R14:           cpustate->r[14] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R15:           cpustate->r[15] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R16:           cpustate->r[16] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R17:           cpustate->r[17] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R18:           cpustate->r[18] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R19:           cpustate->r[19] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R20:           cpustate->r[20] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R21:           cpustate->r[21] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R22:           cpustate->r[22] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R23:           cpustate->r[23] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R24:           cpustate->r[24] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R25:           cpustate->r[25] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R26:           cpustate->r[26] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R27:           cpustate->r[27] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R28:           cpustate->r[28] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R29:           cpustate->r[29] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R30:           cpustate->r[30] = info->i;              break;
        case CPUINFO_INT_REGISTER + AVR8_R31:           cpustate->r[31] = info->i;              break;
    }
}

CPU_GET_INFO( avr8 )
{
    avr8_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

    switch(state)
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:          info->i = sizeof(avr8_state);   break;
        case CPUINFO_INT_INPUT_LINES:           info->i = 0;                    break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:    info->i = 0;                    break;
        case CPUINFO_INT_ENDIANNESS:            info->i = ENDIANNESS_LITTLE;    break;
        case CPUINFO_INT_CLOCK_MULTIPLIER:      info->i = 1;                    break;
        case CPUINFO_INT_CLOCK_DIVIDER:         info->i = 1;                    break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES: info->i = 2;                    break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES: info->i = 4;                    break;
        case CPUINFO_INT_MIN_CYCLES:            info->i = 1;                    break;
        case CPUINFO_INT_MAX_CYCLES:            info->i = 4;                    break;

        case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 8;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 22;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 8;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 11;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

        case CPUINFO_INT_PC:    /* intentional fallthrough */
        case CPUINFO_INT_REGISTER + AVR8_PC:    info->i = cpustate->pc << 1;	break;
        case CPUINFO_INT_REGISTER + AVR8_SREG:  info->i = cpustate->status; 	break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(avr8);        break;
        case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(avr8);               break;
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(avr8);             break;
        case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(avr8);               break;
        case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(avr8);         break;
        case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
        case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(avr8); break;
        case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &cpustate->icount;               break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:                          strcpy(info->s, "Atmel 8-bit AVR");     break;
        case CPUINFO_STR_FAMILY:                   strcpy(info->s, "AVR8");                break;
        case CPUINFO_STR_VERSION:                  strcpy(info->s, "1.0");                 break;
        case CPUINFO_STR_SOURCE_FILE:                     strcpy(info->s, __FILE__);              break;
        case CPUINFO_STR_CREDITS:                  strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

        case CPUINFO_STR_FLAGS:                         strcpy(info->s, " ");                   break;

        case CPUINFO_STR_REGISTER + AVR8_SREG:          sprintf(info->s, "SREG: %c%c%c%c%c%c%c%c", (cpustate->status & 0x80) ? 'I' : '-', (cpustate->status & 0x40) ? 'T' : '-', (cpustate->status & 0x20) ? 'H' : '-', (cpustate->status & 0x10) ? 'S' : '-', (cpustate->status & 0x08) ? 'V' : '-', (cpustate->status & 0x04) ? 'N' : '-', (cpustate->status & 0x02) ? 'Z' : '-', (cpustate->status & 0x01) ? 'C' : '-'); break;
        case CPUINFO_STR_REGISTER + AVR8_R0:            sprintf(info->s, "R0:  %02x", cpustate->r[ 0] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R1:            sprintf(info->s, "R1:  %02x", cpustate->r[ 1] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R2:            sprintf(info->s, "R2:  %02x", cpustate->r[ 2] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R3:            sprintf(info->s, "R3:  %02x", cpustate->r[ 3] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R4:            sprintf(info->s, "R4:  %02x", cpustate->r[ 4] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R5:            sprintf(info->s, "R5:  %02x", cpustate->r[ 5] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R6:            sprintf(info->s, "R6:  %02x", cpustate->r[ 6] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R7:            sprintf(info->s, "R7:  %02x", cpustate->r[ 7] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R8:            sprintf(info->s, "R8:  %02x", cpustate->r[ 8] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R9:            sprintf(info->s, "R9:  %02x", cpustate->r[ 9] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R10:           sprintf(info->s, "R10: %02x", cpustate->r[10] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R11:           sprintf(info->s, "R11: %02x", cpustate->r[11] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R12:           sprintf(info->s, "R12: %02x", cpustate->r[12] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R13:           sprintf(info->s, "R13: %02x", cpustate->r[13] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R14:           sprintf(info->s, "R14: %02x", cpustate->r[14] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R15:           sprintf(info->s, "R15: %02x", cpustate->r[15] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R16:           sprintf(info->s, "R16: %02x", cpustate->r[16] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R17:           sprintf(info->s, "R17: %02x", cpustate->r[17] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R18:           sprintf(info->s, "R18: %02x", cpustate->r[18] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R19:           sprintf(info->s, "R19: %02x", cpustate->r[19] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R20:           sprintf(info->s, "R20: %02x", cpustate->r[20] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R21:           sprintf(info->s, "R21: %02x", cpustate->r[21] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R22:           sprintf(info->s, "R22: %02x", cpustate->r[22] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R23:           sprintf(info->s, "R23: %02x", cpustate->r[23] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R24:           sprintf(info->s, "R24: %02x", cpustate->r[24] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R25:           sprintf(info->s, "R25: %02x", cpustate->r[25] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R26:           sprintf(info->s, "R26: %02x", cpustate->r[26] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R27:           sprintf(info->s, "R27: %02x", cpustate->r[27] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R28:           sprintf(info->s, "R28: %02x", cpustate->r[28] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R29:           sprintf(info->s, "R29: %02x", cpustate->r[29] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R30:           sprintf(info->s, "R30: %02x", cpustate->r[30] ); break;
        case CPUINFO_STR_REGISTER + AVR8_R31:           sprintf(info->s, "R31: %02x", cpustate->r[31] ); break;
        case CPUINFO_STR_REGISTER + AVR8_X:             sprintf(info->s, "X: %04x", XREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_Y:             sprintf(info->s, "Y: %04x", YREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_Z:             sprintf(info->s, "Z: %04x", ZREG ); break;
        case CPUINFO_STR_REGISTER + AVR8_SP:            sprintf(info->s, "SP: %04x", SPREG ); break;
    }
}

static CPU_INIT( atmega88 )
{
	CPU_INIT_CALL(avr8);
    avr8_state *cpustate = get_safe_token(device);
	cpustate->addr_mask = 0x0fff;
}

static CPU_INIT( atmega644 )
{
	CPU_INIT_CALL(avr8);
    avr8_state *cpustate = get_safe_token(device);
	cpustate->addr_mask = 0xffff;
}

CPU_GET_INFO( atmega88 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(atmega88);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ATmega88");				break;

		default:										CPU_GET_INFO_CALL(avr8); break;
	}
}

CPU_GET_INFO( atmega644 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(atmega644);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ATmega644");				break;

		default:										CPU_GET_INFO_CALL(avr8); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ATMEGA88, atmega88);
DEFINE_LEGACY_CPU_DEVICE(ATMEGA644, atmega644);
