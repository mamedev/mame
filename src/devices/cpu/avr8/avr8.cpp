// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Sandro Ronco, Felipe Sanches
/***************************************************************************

    Atmel 8-bit AVR simulator

    - Notes -
      Cycle counts are generally considered to be 100% accurate per-instruction, does not support mid-instruction
      interrupts although no software has been encountered yet that requires it. Evidence of cycle accuracy is given
      in the form of the demoscene 'wild' demo, Craft, by [lft], which uses an ATmega88 to write video out a 6-bit
      RGB DAC pixel-by-pixel, synchronously with the frame timing. Intentionally modifying the timing of any of
      the existing opcodes has been shown to wildly corrupt the video output in Craft, so one can assume that the
      existing timing is 100% correct.

      Unimplemented opcodes: SPM, SPM Z+, SLEEP, BREAK, WDR, EICALL, JMP, CALL

    - Changelist -
      05 Jul. 2015 [Felipe Sanches]
      - Implemented EIJMP instruction

      29 Dec. 2013 [Felipe Sanches]
      - Added crude boilerplate code for Timer/Counter #4

      25 Dec. 2013 [Felipe Sanches]
      - Updated AVR8_REGIDX_* enum based on ATMEGA640/1280/2560 datasheet

      24 Dec. 2013 [Felipe Sanches]
      - update data memory mapping so that all 0x200 register addresses are accessible

      23 Dec. 2013 [Felipe Sanches]
      - Added ELPM instructions
      - Added fuse bits macros
      - Added reset logic to decide initial program counter based on fuse bits configuration
      - Added initial support for ATMEGA1280 and ATMEGA2560
      - Use register names in the disassembly of IN and OUT instructions

      23 Dec. 2012 [Sandro Ronco]
      - Added CPSE, LD Z+, ST -Z/-Y/-X and ICALL opcodes
      - Fixed Z flag in CPC, SBC and SBCI opcodes
      - Fixed V and C flags in SBIW opcode

      30 Oct. 2012
      - Added FMUL, FMULS, FMULSU opcodes [Ryan Holtz]
      - Fixed incorrect flag calculation in ROR opcode [Ryan Holtz]
      - Fixed incorrect bit testing in SBIC/SBIS opcodes [Ryan Holtz]

      25 Oct. 2012
      - Added MULS, ANDI, STI Z+, LD -Z, LD -Y, LD -X, LD Y+q, LD Z+q, SWAP, ASR, ROR and SBIS opcodes [Ryan Holtz]
      - Corrected cycle counts for LD and ST opcodes [Ryan Holtz]
      - Moved opcycles init into inner while loop, fixes 2-cycle and 3-cycle opcodes effectively forcing
        all subsequent 1-cycle opcodes to be 2 or 3 cycles [Ryan Holtz]
      - Fixed register behavior in MULSU, LD -Z, and LD -Y opcodes [Ryan Holtz]

      18 Oct. 2012
      - Added OR, SBCI, ORI, ST Y+, ADIQ opcodes [Ryan Holtz]
      - Fixed COM, NEG, LSR opcodes [Ryan Holtz]

*/

#include "emu.h"
#include "avr8.h"
#include "avr8dasm.h"

#define LOG_UNKNOWN         (1 << 1)
#define LOG_BOOT            (1 << 2)
#define LOG_TIMER0          (1 << 3)
#define LOG_TIMER1          (1 << 4)
#define LOG_TIMER2          (1 << 5)
#define LOG_TIMER3          (1 << 6)
#define LOG_TIMER4          (1 << 7)
#define LOG_TIMER5          (1 << 8)
#define LOG_TIMER0_TICK     (1 << 9)
#define LOG_TIMER1_TICK     (1 << 10)
#define LOG_TIMER2_TICK     (1 << 11)
#define LOG_TIMER3_TICK     (1 << 12)
#define LOG_TIMER4_TICK     (1 << 13)
#define LOG_TIMER5_TICK     (1 << 14)
#define LOG_EEPROM          (1 << 15)
#define LOG_GPIO            (1 << 16)
#define LOG_WDOG            (1 << 17)
#define LOG_CLOCK           (1 << 18)
#define LOG_POWER           (1 << 19)
#define LOG_OSC             (1 << 20)
#define LOG_PINCHG          (1 << 21)
#define LOG_EXTMEM          (1 << 22)
#define LOG_ADC             (1 << 23)
#define LOG_DIGINPUT        (1 << 24)
#define LOG_ASYNC           (1 << 25)
#define LOG_TWI             (1 << 26)
#define LOG_UART            (1 << 27)
#define LOG_TIMERS          (LOG_TIMER0 | LOG_TIMER1 | LOG_TIMER2 | LOG_TIMER3 | LOG_TIMER4 | LOG_TIMER5)
#define LOG_TIMER_TICKS     (LOG_TIMER0_TICK | LOG_TIMER1_TICK | LOG_TIMER2_TICK | LOG_TIMER3_TICK | LOG_TIMER4_TICK | LOG_TIMER5_TICK)
#define LOG_ALL             (LOG_UNKNOWN | LOG_BOOT | LOG_TIMERS | LOG_TIMER_TICKS | LOG_EEPROM | LOG_GPIO | LOG_WDOG | LOG_CLOCK | LOG_POWER \
							 | LOG_OSC | LOG_PINCHG | LOG_EXTMEM | LOG_ADC | LOG_DIGINPUT | LOG_ASYNC | LOG_TWI | LOG_UART)

#define VERBOSE             (0)
#include "logmacro.h"

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

	AVR8_SREG_MASK_C = 0x01,
	AVR8_SREG_MASK_Z = 0x02,
	AVR8_SREG_MASK_N = 0x04,
	AVR8_SREG_MASK_V = 0x08,
	AVR8_SREG_MASK_S = 0x10,
	AVR8_SREG_MASK_H = 0x20,
	AVR8_SREG_MASK_T = 0x40,
	AVR8_SREG_MASK_I = 0x80
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
	WGM02_NORMAL = 0,
	WGM02_PWM_PC,
	WGM02_CTC_CMP,
	WGM02_FAST_PWM,
	WGM02_RESERVED0,
	WGM02_PWM_PC_CMP,
	WGM02_RESERVED1,
	WGM02_FAST_PWM_CMP
};

enum
{
	WGM4_NORMAL = 0,
	WGM4_PWM_8_PC,
	WGM4_PWM_9_PC,
	WGM4_PWM_10_PC,
	WGM4_CTC_OCR,
	WGM4_FAST_PWM_8,
	WGM4_FAST_PWM_9,
	WGM4_FAST_PWM_10,
	WGM4_PWM_PFC_ICR,
	WGM4_PWM_PFC_OCR,
	WGM4_PWM_PC_ICR,
	WGM4_PWM_PC_OCR,
	WGM4_CTC_ICR,
	WGM4_RESERVED,
	WGM4_FAST_PWM_ICR,
	WGM4_FAST_PWM_OCR
};

enum
{
	WGM5_NORMAL = 0,
	WGM5_PWM_8_PC,
	WGM5_PWM_9_PC,
	WGM5_PWM_10_PC,
	WGM5_CTC_OCR,
	WGM5_FAST_PWM_8,
	WGM5_FAST_PWM_9,
	WGM5_FAST_PWM_10,
	WGM5_PWM_PFC_ICR,
	WGM5_PWM_PFC_OCR,
	WGM5_PWM_PC_ICR,
	WGM5_PWM_PC_OCR,
	WGM5_CTC_ICR,
	WGM5_RESERVED,
	WGM5_FAST_PWM_ICR,
	WGM5_FAST_PWM_OCR
};

static const char avr8_reg_name[4] = { 'A', 'B', 'C', 'D' };

#define SREG_R(b)   ((m_r[AVR8_REGIDX_SREG] & (1 << (b))) >> (b))
#define SREG_W(b,v) m_r[AVR8_REGIDX_SREG] = (m_r[AVR8_REGIDX_SREG] & ~(1 << (b))) | ((v) << (b))
#define SREG        m_r[AVR8_REGIDX_SREG]
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
#define KCONST22(op)    (((((uint32_t)(op) >> 3) & 0x003e) | ((uint32_t)(op) & 0x0001)) << 16)
#define QCONST6(op)     ((((op) >> 8) & 0x0020) | (((op) >> 7) & 0x0018) | ((op) & 0x0007))
#define ACONST5(op)     (((op) >> 3) & 0x001f)
#define ACONST6(op)     ((((op) >> 5) & 0x0030) | ((op) & 0x000f))
#define MULCONST2(op)   ((((op) >> 6) & 0x0002) | (((op) >> 3) & 0x0001))

// Register Defines
#define XREG            ((m_r[27] << 8) | m_r[26])
#define YREG            ((m_r[29] << 8) | m_r[28])
#define ZREG            ((m_r[31] << 8) | m_r[30])
#define SPREG           ((m_r[AVR8_REGIDX_SPH] << 8) | m_r[AVR8_REGIDX_SPL])

// I/O Defines
#define AVR8_OCR1CH             (m_r[AVR8_REGIDX_OCR1CH])
#define AVR8_OCR1CL             (m_r[AVR8_REGIDX_OCR1CL])
#define AVR8_OCR1BH             (m_r[AVR8_REGIDX_OCR1BH])
#define AVR8_OCR1BL             (m_r[AVR8_REGIDX_OCR1BL])
#define AVR8_OCR1AH             (m_r[AVR8_REGIDX_OCR1AH])
#define AVR8_OCR1AL             (m_r[AVR8_REGIDX_OCR1AL])
#define AVR8_ICR1H              (m_r[AVR8_REGIDX_ICR1H])
#define AVR8_ICR1L              (m_r[AVR8_REGIDX_ICR1L])
#define AVR8_TCNT1H             (m_r[AVR8_REGIDX_TCNT1H])
#define AVR8_TCNT1L             (m_r[AVR8_REGIDX_TCNT1L])

#define AVR8_OCR3CH             (m_r[AVR8_REGIDX_OCR3CH])
#define AVR8_OCR3CL             (m_r[AVR8_REGIDX_OCR3CL])
#define AVR8_OCR3BH             (m_r[AVR8_REGIDX_OCR3BH])
#define AVR8_OCR3BL             (m_r[AVR8_REGIDX_OCR3BL])
#define AVR8_OCR3AH             (m_r[AVR8_REGIDX_OCR3AH])
#define AVR8_OCR3AL             (m_r[AVR8_REGIDX_OCR3AL])
#define AVR8_ICR3H              (m_r[AVR8_REGIDX_ICR3H])
#define AVR8_ICR3L              (m_r[AVR8_REGIDX_ICR3L])
#define AVR8_TCNT3H             (m_r[AVR8_REGIDX_TCNT3H])
#define AVR8_TCNT3L             (m_r[AVR8_REGIDX_TCNT3L])

#define AVR8_OCR4CH             (m_r[AVR8_REGIDX_OCR4CH])
#define AVR8_OCR4CL             (m_r[AVR8_REGIDX_OCR4CL])
#define AVR8_OCR4BH             (m_r[AVR8_REGIDX_OCR4BH])
#define AVR8_OCR4BL             (m_r[AVR8_REGIDX_OCR4BL])
#define AVR8_OCR4AH             (m_r[AVR8_REGIDX_OCR4AH])
#define AVR8_OCR4AL             (m_r[AVR8_REGIDX_OCR4AL])
#define AVR8_ICR4H              (m_r[AVR8_REGIDX_ICR4H])
#define AVR8_ICR4L              (m_r[AVR8_REGIDX_ICR4L])
#define AVR8_TCNT4H             (m_r[AVR8_REGIDX_TCNT4H])
#define AVR8_TCNT4L             (m_r[AVR8_REGIDX_TCNT4L])

#define AVR8_OCR5CH             (m_r[AVR8_REGIDX_OCR5CH])
#define AVR8_OCR5CL             (m_r[AVR8_REGIDX_OCR5CL])
#define AVR8_OCR5BH             (m_r[AVR8_REGIDX_OCR5BH])
#define AVR8_OCR5BL             (m_r[AVR8_REGIDX_OCR5BL])
#define AVR8_OCR5AH             (m_r[AVR8_REGIDX_OCR5AH])
#define AVR8_OCR5AL             (m_r[AVR8_REGIDX_OCR5AL])
#define AVR8_ICR5H              (m_r[AVR8_REGIDX_ICR5H])
#define AVR8_ICR5L              (m_r[AVR8_REGIDX_ICR5L])
#define AVR8_TCNT5H             (m_r[AVR8_REGIDX_TCNT5H])
#define AVR8_TCNT5L             (m_r[AVR8_REGIDX_TCNT5L])

#define AVR8_TCCR0B                 (m_r[AVR8_REGIDX_TCCR0B])
#define AVR8_TCCR0B_FOC0A_MASK      0x80
#define AVR8_TCCR0B_FOC0A_SHIFT     7
#define AVR8_TCCR0B_FOC0B_MASK      0x40
#define AVR8_TCCR0B_FOC0B_SHIFT     6
#define AVR8_TCCR0B_WGM0_2_MASK     0x08
#define AVR8_TCCR0B_WGM0_2_SHIFT    3
#define AVR8_TCCR0B_CS_MASK         0x07
#define AVR8_TCCR0B_CS_SHIFT        0
#define AVR8_TIMER0_CLOCK_SELECT    (AVR8_TCCR0B & AVR8_TCCR0B_CS_MASK)

#define AVR8_TCCR0A                 (m_r[AVR8_REGIDX_TCCR0A])
#define AVR8_TCCR0A_COM0A_MASK      0xc0
#define AVR8_TCCR0A_COM0A_SHIFT     6
#define AVR8_TCCR0A_COM0B_MASK      0x30
#define AVR8_TCCR0A_COM0B_SHIFT     4
#define AVR8_TCCR0A_WGM0_10_MASK    0x03
#define AVR8_TCCR0A_WGM0_10_SHIFT   0
#define AVR8_TCCR0A_COM0A           ((AVR8_TCCR0A & AVR8_TCCR0A_COM0A_MASK) >> AVR8_TCCR0A_COM0A_SHIFT)
#define AVR8_TCCR0A_COM0B           ((AVR8_TCCR0A & AVR8_TCCR0A_COM0B_MASK) >> AVR8_TCCR0A_COM0B_SHIFT)
#define AVR8_TCCR0A_WGM0_10         (AVR8_TCCR0A & AVR8_TCCR0A_WGM0_10_MASK)

#define AVR8_TIMSK0             (m_r[AVR8_REGIDX_TIMSK0])
#define AVR8_TIMSK0_OCIE0B_MASK 0x04
#define AVR8_TIMSK0_OCIE0A_MASK 0x02
#define AVR8_TIMSK0_TOIE0_MASK  0x01
#define AVR8_TIMSK0_OCIE0B      ((AVR8_TIMSK0 & AVR8_TIMSK0_OCIE0B_MASK) >> 2)
#define AVR8_TIMSK0_OCIE0A      ((AVR8_TIMSK0 & AVR8_TIMSK0_OCIE0A_MASK) >> 1)
#define AVR8_TIMSK0_TOIE0       (AVR8_TIMSK0 & AVR8_TIMSK0_TOIE0_MASK)

#define AVR8_TIFR0              (m_r[AVR8_REGIDX_TIFR0])
#define AVR8_TIFR0_OCF0B_MASK   0x04
#define AVR8_TIFR0_OCF0B_SHIFT  2
#define AVR8_TIFR0_OCF0A_MASK   0x02
#define AVR8_TIFR0_OCF0A_SHIFT  1
#define AVR8_TIFR0_TOV0_MASK    0x01
#define AVR8_TIFR0_TOV0_SHIFT   0
#define AVR8_TIFR0_MASK         (AVR8_TIFR0_TOV0_MASK | AVR8_TIFR0_OCF0B_MASK | AVR8_TIFR0_OCF0A_MASK)

#define AVR8_TCCR1B                 (m_r[AVR8_REGIDX_TCCR1B])
#define AVR8_TCCR1B_ICNC1_MASK      0x80
#define AVR8_TCCR1B_ICNC1_SHIFT     7
#define AVR8_TCCR1B_ICES1_MASK      0x40
#define AVR8_TCCR1B_ICES1_SHIFT     6
#define AVR8_TCCR1B_WGM1_32_MASK    0x18
#define AVR8_TCCR1B_WGM1_32_SHIFT   3
#define AVR8_TCCR1B_CS_MASK         0x07
#define AVR8_TCCR1B_CS_SHIFT        0
#define AVR8_TIMER1_CLOCK_SELECT    (AVR8_TCCR1B & AVR8_TCCR1B_CS_MASK)

#define AVR8_TCCR1A                 (m_r[AVR8_REGIDX_TCCR1A])
#define AVR8_TCCR1A_COM1A_MASK      0xc0
#define AVR8_TCCR1A_COM1A_SHIFT     6
#define AVR8_TCCR1A_COM1B_MASK      0x30
#define AVR8_TCCR1A_COM1B_SHIFT     4
#define AVR8_TCCR1A_WGM1_10_MASK    0x03
#define AVR8_TCCR1A_WGM1_10_SHIFT   0
#define AVR8_TCCR1A_COM1A           ((AVR8_TCCR1A & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT)
#define AVR8_TCCR1A_COM1B           ((AVR8_TCCR1A & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT)
#define AVR8_TCCR1A_WGM1_10         (AVR8_TCCR1A & AVR8_TCCR1A_WGM1_10_MASK)

#define AVR8_TIMSK1             (m_r[AVR8_REGIDX_TIMSK1])
#define AVR8_TIMSK1_ICIE1_MASK  0x20
#define AVR8_TIMSK1_OCIE1B_MASK 0x04
#define AVR8_TIMSK1_OCIE1A_MASK 0x02
#define AVR8_TIMSK1_TOIE1_MASK  0x01
#define AVR8_TIMSK1_ICIE1       ((AVR8_TIMSK1 & AVR8_TIMSK1_ICIE1_MASK) >> 5)
#define AVR8_TIMSK1_OCIE1B      ((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1B_MASK) >> 2)
#define AVR8_TIMSK1_OCIE1A      ((AVR8_TIMSK1 & AVR8_TIMSK1_OCIE1A_MASK) >> 1)
#define AVR8_TIMSK1_TOIE1       (AVR8_TIMSK1 & AVR8_TIMSK1_TOIE1_MASK)

#define AVR8_TIFR1              (m_r[AVR8_REGIDX_TIFR1])
#define AVR8_TIFR1_ICF1_MASK    0x20
#define AVR8_TIFR1_ICF1_SHIFT   5
#define AVR8_TIFR1_OCF1B_MASK   0x04
#define AVR8_TIFR1_OCF1B_SHIFT  2
#define AVR8_TIFR1_OCF1A_MASK   0x02
#define AVR8_TIFR1_OCF1A_SHIFT  1
#define AVR8_TIFR1_TOV1_MASK    0x01
#define AVR8_TIFR1_TOV1_SHIFT   0
#define AVR8_TIFR1_MASK         (AVR8_TIFR1_ICF1_MASK | AVR8_TIFR1_TOV1_MASK | \
									AVR8_TIFR1_OCF1B_MASK | AVR8_TIFR1_OCF1A_MASK)

#define AVR8_TCCR2B                 (m_r[AVR8_REGIDX_TCCR2B])
#define AVR8_TCCR2B_FOC2A_MASK      0x80
#define AVR8_TCCR2B_FOC2A_SHIFT     7
#define AVR8_TCCR2B_FOC2B_MASK      0x40
#define AVR8_TCCR2B_FOC2B_SHIFT     6
#define AVR8_TCCR2B_WGM2_2_MASK     0x08
#define AVR8_TCCR2B_WGM2_2_SHIFT    3
#define AVR8_TCCR2B_CS_MASK         0x07
#define AVR8_TCCR2B_CS_SHIFT        0
#define AVR8_TIMER2_CLOCK_SELECT    (AVR8_TCCR2B & AVR8_TCCR2B_CS_MASK)

#define AVR8_TCCR2A                 (m_r[AVR8_REGIDX_TCCR2A])
#define AVR8_TCCR2A_COM2A_MASK      0xc0
#define AVR8_TCCR2A_COM2A_SHIFT     6
#define AVR8_TCCR2A_COM2B_MASK      0x30
#define AVR8_TCCR2A_COM2B_SHIFT     4
#define AVR8_TCCR2A_WGM2_10_MASK    0x03
#define AVR8_TCCR2A_WGM2_10_SHIFT   0
#define AVR8_TCCR2A_COM2A           ((AVR8_TCCR2A & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT)
#define AVR8_TCCR2A_COM2B           ((AVR8_TCCR2A & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT)
#define AVR8_TCCR2A_WGM2_10         (AVR8_TCCR2A & AVR8_TCCR2A_WGM2_10_MASK)

#define AVR8_TIMSK2             (m_r[AVR8_REGIDX_TIMSK2])
#define AVR8_TIMSK2_OCIE2B_MASK 0x04
#define AVR8_TIMSK2_OCIE2A_MASK 0x02
#define AVR8_TIMSK2_TOIE2_MASK  0x01
#define AVR8_TIMSK2_OCIE2B      ((AVR8_TIMSK2 & AVR8_TIMSK2_OCIE2B_MASK) >> 2)
#define AVR8_TIMSK2_OCIE2A      ((AVR8_TIMSK2 & AVR8_TIMSK2_OCIE2A_MASK) >> 1)
#define AVR8_TIMSK2_TOIE2       (AVR8_TIMSK2 & AVR8_TIMSK2_TOIE2_MASK)

#define AVR8_TIFR2              (m_r[AVR8_REGIDX_TIFR2])
#define AVR8_TIFR2_OCF2B_MASK   0x04
#define AVR8_TIFR2_OCF2B_SHIFT  2
#define AVR8_TIFR2_OCF2A_MASK   0x02
#define AVR8_TIFR2_OCF2A_SHIFT  1
#define AVR8_TIFR2_TOV2_MASK    0x01
#define AVR8_TIFR2_TOV2_SHIFT   0
#define AVR8_TIFR2_MASK         (AVR8_TIFR2_TOV2_MASK | AVR8_TIFR2_OCF2B_MASK | AVR8_TIFR2_OCF2A_MASK)

#define AVR8_TIMSK3             (m_r[AVR8_REGIDX_TIMSK3])
#define AVR8_TIMSK3_OCIE3C_MASK 0x08
#define AVR8_TIMSK3_OCIE3B_MASK 0x04
#define AVR8_TIMSK3_OCIE3A_MASK 0x02
#define AVR8_TIMSK3_TOIE3_MASK  0x01
#define AVR8_TIMSK3_OCIE3C      ((AVR8_TIMSK3 & AVR8_TIMSK3_OCIE3C_MASK) >> 3)
#define AVR8_TIMSK3_OCIE3B      ((AVR8_TIMSK3 & AVR8_TIMSK3_OCIE3B_MASK) >> 2)
#define AVR8_TIMSK3_OCIE3A      ((AVR8_TIMSK3 & AVR8_TIMSK3_OCIE3A_MASK) >> 1)
#define AVR8_TIMSK3_TOIE3       ((AVR8_TIMSK3 &  AVR8_TIMSK3_TOIE3_MASK) >> 0)

#define AVR8_TCCR4C                 (m_r[AVR8_REGIDX_TCCR4C])

#define AVR8_TCCR4B                 (m_r[AVR8_REGIDX_TCCR4B])
#define AVR8_TCCR4B_FOC4A_MASK      0x80
#define AVR8_TCCR4B_FOC4A_SHIFT     7
#define AVR8_TCCR4B_FOC4B_MASK      0x40
#define AVR8_TCCR4B_FOC4B_SHIFT     6
#define AVR8_TCCR4B_FOC4C_MASK      0x20
#define AVR8_TCCR4B_FOC4C_SHIFT     5
#define AVR8_TCCR4B_WGM4_32_MASK    0x18
#define AVR8_TCCR4B_WGM4_32_SHIFT   3
#define AVR8_TCCR4B_CS_MASK         0x07
#define AVR8_TCCR4B_CS_SHIFT        0
#define AVR8_TIMER4_CLOCK_SELECT    ((AVR8_TCCR4B & AVR8_TCCR4B_CS_MASK) >> AVR8_TCCR4B_CS_SHIFT)

#define AVR8_TCCR4A                 (m_r[AVR8_REGIDX_TCCR4A])
#define AVR8_TCCR4A_COM4A_MASK      0xc0
#define AVR8_TCCR4A_COM4A_SHIFT     6
#define AVR8_TCCR4A_COM4B_MASK      0x30
#define AVR8_TCCR4A_COM4B_SHIFT     4
#define AVR8_TCCR4A_COM4C_MASK      0x0c
#define AVR8_TCCR4A_COM4C_SHIFT     2
#define AVR8_TCCR4A_WGM4_10_MASK    0x03
#define AVR8_TCCR4A_WGM4_10_SHIFT   0
#define AVR8_TCCR4A_COM4A           ((AVR8_TCCR4A & AVR8_TCCR4A_COM4A_MASK) >> AVR8_TCCR4A_COM4A_SHIFT)
#define AVR8_TCCR4A_COM4B           ((AVR8_TCCR4A & AVR8_TCCR4A_COM4B_MASK) >> AVR8_TCCR4A_COM4B_SHIFT)
#define AVR8_TCCR4A_COM4C           ((AVR8_TCCR4A & AVR8_TCCR4A_COM4C_MASK) >> AVR8_TCCR4A_COM4C_SHIFT)
#define AVR8_TCCR4A_WGM2_10         (AVR8_TCCR4A & AVR8_TCCR4A_WGM2_10_MASK)

#define AVR8_WGM4_32 ((AVR8_TCCR4B & AVR8_TCCR4B_WGM4_32_MASK) >> AVR8_TCCR4B_WGM4_32_SHIFT)
#define AVR8_WGM4_10 ((AVR8_TCCR4A & AVR8_TCCR4A_WGM4_10_MASK) >> AVR8_TCCR4A_WGM4_10_SHIFT)
#define AVR8_WGM4 ((AVR8_WGM4_32 << 2) | AVR8_WGM4_10)

#define AVR8_TIMSK4             (m_r[AVR8_REGIDX_TIMSK4])
#define AVR8_TIMSK4_OCIE4B_MASK 0x04
#define AVR8_TIMSK4_OCIE4A_MASK 0x02
#define AVR8_TIMSK4_TOIE4_MASK  0x01
#define AVR8_TIMSK4_OCIE4B      ((AVR8_TIMSK4 & AVR8_TIMSK4_OCIE4B_MASK) >> 2)
#define AVR8_TIMSK4_OCIE4A      ((AVR8_TIMSK4 & AVR8_TIMSK4_OCIE4A_MASK) >> 1)
#define AVR8_TIMSK4_TOIE4       (AVR8_TIMSK4 & AVR8_TIMSK4_TOIE4_MASK)

#define AVR8_TIFR4              (m_r[AVR8_REGIDX_TIFR4])
#define AVR8_TIFR4_OCF4B_MASK   0x04
#define AVR8_TIFR4_OCF4B_SHIFT  2
#define AVR8_TIFR4_OCF4A_MASK   0x02
#define AVR8_TIFR4_OCF4A_SHIFT  1
#define AVR8_TIFR4_TOV4_MASK    0x01
#define AVR8_TIFR4_TOV4_SHIFT   0
#define AVR8_TIFR4_MASK         (AVR8_TIFR4_TOV4_MASK | AVR8_TIFR4_OCF4B_MASK | AVR8_TIFR4_OCF4A_MASK)

//---------------------------------------------------------------
#define AVR8_TCCR5C                 (m_r[AVR8_REGIDX_TCCR5C])
#define AVR8_TCCR5C_FOC5A_MASK      0x80
#define AVR8_TCCR5C_FOC5A_SHIFT     7
#define AVR8_TCCR5C_FOC5B_MASK      0x40
#define AVR8_TCCR5C_FOC5B_SHIFT     6
#define AVR8_TCCR5C_FOC5C_MASK      0x20
#define AVR8_TCCR5C_FOC5C_SHIFT     5

#define AVR8_TCCR5B                 (m_r[AVR8_REGIDX_TCCR5B])
#define AVR8_TCCR5B_ICNC5_MASK      0x80
#define AVR8_TCCR5B_ICNC5_SHIFT     7
#define AVR8_TCCR5B_ICES5_MASK      0x40
#define AVR8_TCCR5B_ICES5_SHIFT     6
#define AVR8_TCCR5B_WGM5_32_MASK    0x18
#define AVR8_TCCR5B_WGM5_32_SHIFT   3
#define AVR8_TCCR5B_CS_MASK         0x07
#define AVR8_TCCR5B_CS_SHIFT        0
#define AVR8_TIMER5_CLOCK_SELECT    ((AVR8_TCCR5B & AVR8_TCCR5B_CS_MASK) >> AVR8_TCCR5B_CS_SHIFT)

#define AVR8_TCCR5A                 (m_r[AVR8_REGIDX_TCCR4A])
#define AVR8_TCCR5A_COM5A_MASK      0xc0
#define AVR8_TCCR5A_COM5A_SHIFT     6
#define AVR8_TCCR5A_COM5B_MASK      0x30
#define AVR8_TCCR5A_COM5B_SHIFT     4
#define AVR8_TCCR5A_COM5C_MASK      0x0c
#define AVR8_TCCR5A_COM5C_SHIFT     2
#define AVR8_TCCR5A_WGM5_10_MASK    0x03
#define AVR8_TCCR5A_WGM5_10_SHIFT   0
#define AVR8_TCCR5A_COM5A           ((AVR8_TCCR5A & AVR8_TCCR5A_COM5A_MASK) >> AVR8_TCCR5A_COM5A_SHIFT)
#define AVR8_TCCR5A_COM5B           ((AVR8_TCCR5A & AVR8_TCCR5A_COM5B_MASK) >> AVR8_TCCR5A_COM5B_SHIFT)
#define AVR8_TCCR5A_COM5C           ((AVR8_TCCR5A & AVR8_TCCR5A_COM5C_MASK) >> AVR8_TCCR5A_COM5C_SHIFT)
#define AVR8_TCCR5A_WGM5_10         (AVR8_TCCR5A & AVR8_TCCR5A_WGM5_10_MASK)

#define AVR8_WGM5_32 ((AVR8_TCCR5B & AVR8_TCCR5B_WGM5_32_MASK) >> AVR8_TCCR5B_WGM5_32_SHIFT)
#define AVR8_WGM5_10 ((AVR8_TCCR5A & AVR8_TCCR5A_WGM5_10_MASK) >> AVR8_TCCR5A_WGM5_10_SHIFT)
#define AVR8_WGM5 ((AVR8_WGM5_32 << 2) | AVR8_WGM5_10)

#define AVR8_TIMSK5             (m_r[AVR8_REGIDX_TIMSK5])
#define AVR8_TIMSK5_ICIE5_MASK  0x20
#define AVR8_TIMSK5_OCIE5C_MASK 0x08
#define AVR8_TIMSK5_OCIE5B_MASK 0x04
#define AVR8_TIMSK5_OCIE5A_MASK 0x02
#define AVR8_TIMSK5_TOIE5_MASK  0x01

#define AVR8_TIMSK5_ICIE5C      ((AVR8_TIMSK5 & AVR8_TIMSK5_ICIE5C_MASK) >> 5)
#define AVR8_TIMSK5_OCIE5C      ((AVR8_TIMSK5 & AVR8_TIMSK5_OCIE5C_MASK) >> 3)
#define AVR8_TIMSK5_OCIE5B      ((AVR8_TIMSK5 & AVR8_TIMSK5_OCIE5B_MASK) >> 2)
#define AVR8_TIMSK5_OCIE5A      ((AVR8_TIMSK5 & AVR8_TIMSK5_OCIE5A_MASK) >> 1)
#define AVR8_TIMSK5_TOIE5       (AVR8_TIMSK5 & AVR8_TIMSK5_TOIE5_MASK)

#define AVR8_TIFR5              (m_r[AVR8_REGIDX_TIFR5])
#define AVR8_TIFR5_ICF5_MASK   0x20
#define AVR8_TIFR5_ICF5_SHIFT  5
#define AVR8_TIFR5_OCF5C_MASK   0x08
#define AVR8_TIFR5_OCF5C_SHIFT  3
#define AVR8_TIFR5_OCF5B_MASK   0x04
#define AVR8_TIFR5_OCF5B_SHIFT  2
#define AVR8_TIFR5_OCF5A_MASK   0x02
#define AVR8_TIFR5_OCF5A_SHIFT  1
#define AVR8_TIFR5_TOV5_MASK    0x01
#define AVR8_TIFR5_TOV5_SHIFT   0
#define AVR8_TIFR5_MASK         (AVR8_TIFR5_ICF5_MASK | AVR8_TIFR5_OCF5C_MASK | AVR8_TIFR5_OCF5B_MASK | AVR8_TIFR5_OCF5A_MASK | AVR8_TIFR5_TOV5_MASK)


#define AVR8_TIFR5_ICF5 ((AVR8_TIFR5 & AVR8_TIFR5_ICF5_MASK) >> AVR8_TIFR5_ICF5_SHIFT)
#define AVR8_TIFR5_OCF5C ((AVR8_TIFR5 & AVR8_TIFR5_OCF5C_MASK) >> AVR8_TIFR5_OCF5C_SHIFT)
#define AVR8_TIFR5_OCF5B ((AVR8_TIFR5 & AVR8_TIFR5_OCF5B_MASK) >> AVR8_TIFR5_OCF5B_SHIFT)
#define AVR8_TIFR5_OCF5A ((AVR8_TIFR5 & AVR8_TIFR5_OCF5A_MASK) >> AVR8_TIFR5_OCF5A_SHIFT)
#define AVR8_TIFR5_TOV5 ((AVR8_TIFR5 & AVR8_TIFR5_TOV5_MASK) >> AVR8_TIFR5_TOV5_SHIFT)

//---------------------------------------------------------------

#define AVR8_OCR0A              m_r[AVR8_REGIDX_OCR0A]
#define AVR8_OCR0B              m_r[AVR8_REGIDX_OCR0B]
#define AVR8_TCNT0              m_r[AVR8_REGIDX_TCNT0]
#define AVR8_WGM0               (((AVR8_TCCR0B & 0x08) >> 1) | (AVR8_TCCR0A & 0x03))

#define AVR8_OCR1A              ((AVR8_OCR1AH << 8) | AVR8_OCR1AL)
#define AVR8_OCR1B              ((AVR8_OCR1BH << 8) | AVR8_OCR1BL)
#define AVR8_OCR1C              ((AVR8_OCR1CH << 8) | AVR8_OCR1CL)
#define AVR8_ICR1               ((AVR8_ICR1H  << 8) | AVR8_ICR1L)
#define AVR8_TCNT1              ((AVR8_TCNT1H << 8) | AVR8_TCNT1L)
#define AVR8_WGM1               (((AVR8_TCCR1B & 0x18) >> 1) | (AVR8_TCCR1A & 0x03))
#define AVR8_TCNT1_DIR          (state->m_tcnt1_direction)

#define AVR8_OCR2A              m_r[AVR8_REGIDX_OCR2A]
#define AVR8_OCR2B              m_r[AVR8_REGIDX_OCR2B]
#define AVR8_TCNT2              m_r[AVR8_REGIDX_TCNT2]
#define AVR8_WGM2               (((AVR8_TCCR2B & 0x08) >> 1) | (AVR8_TCCR2A & 0x03))

#define AVR8_ICR3               ((AVR8_ICR3H  << 8) | AVR8_ICR3L)
#define AVR8_OCR3A              ((AVR8_OCR3AH << 8) | AVR8_OCR3AL)

#define AVR8_ICR4               ((AVR8_ICR4H  << 8) | AVR8_ICR4L)
#define AVR8_ICR4H              (m_r[AVR8_REGIDX_ICR4H])
#define AVR8_ICR4L              (m_r[AVR8_REGIDX_ICR4L])
#define AVR8_OCR4A              ((AVR8_OCR4AH << 8) | AVR8_OCR4AL)
#define AVR8_OCR4B              m_r[AVR8_REGIDX_OCR4B]
#define AVR8_TCNT4              m_r[AVR8_REGIDX_TCNT4]

#define AVR8_ICR5               ((AVR8_ICR5H  << 8) | AVR8_ICR5L)
#define AVR8_OCR5A              ((AVR8_OCR5AH << 8) | AVR8_OCR5AL)

#define AVR8_GTCCR_PSRASY_MASK  0x02
#define AVR8_GTCCR_PSRASY_SHIFT 1

#define AVR8_SPSR               (m_r[AVR8_REGIDX_SPSR])
#define AVR8_SPSR_SPR2X         (AVR8_SPSR & AVR8_SPSR_SPR2X_MASK)

#define AVR8_SPCR               (m_r[AVR8_REGIDX_SPCR])
#define AVR8_SPCR_SPIE          ((AVR8_SPCR & AVR8_SPCR_SPIE_MASK) >> 7)
#define AVR8_SPCR_SPE           ((AVR8_SPCR & AVR8_SPCR_SPE_MASK) >> 6)
#define AVR8_SPCR_DORD          ((AVR8_SPCR & AVR8_SPCR_DORD_MASK) >> 5)
#define AVR8_SPCR_MSTR          ((AVR8_SPCR & AVR8_SPCR_MSTR_MASK) >> 4)
#define AVR8_SPCR_CPOL          ((AVR8_SPCR & AVR8_SPCR_CPOL_MASK) >> 3)
#define AVR8_SPCR_CPHA          ((AVR8_SPCR & AVR8_SPCR_CPHA_MASK) >> 2)
#define AVR8_SPCR_SPR           (AVR8_SPCR & AVR8_SPCR_SPR_MASK)

#define AVR8_SPI_RATE           ((AVR8_SPSR_SPR2X << 2) | AVR8_SPCR_SPR)

#define AVR8_PORTB_MOSI         0x08

#define AVR8_EECR               m_r[AVR8_REGIDX_EECR] & 0x3F; //bits 6 and 7 are reserved and will always read as zero
#define AVR8_EECR_EEPM_MASK     0x30
#define AVR8_EECR_EERIE_MASK    0x08
#define AVR8_EECR_EEMPE_MASK    0x04
#define AVR8_EECR_EEPE_MASK     0x02
#define AVR8_EECR_EERE_MASK     0x01
#define AVR8_EECR_EEPM          ((AVR8_EECR & AVR8_EECR_EEPM_MASK) >> 4)
#define AVR8_EECR_EERIE         ((AVR8_EECR & AVR8_EECR_EERIE_MASK) >> 3)
#define AVR8_EECR_EEMPE         ((AVR8_EECR & AVR8_EECR_EEMPE_MASK) >> 2)
#define AVR8_EECR_EEPE          ((AVR8_EECR & AVR8_EECR_EEPE_MASK) >> 1)
#define AVR8_EECR_EERE          ((AVR8_EECR & AVR8_EECR_EERE_MASK) >> 0)

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(ATMEGA88,   atmega88_device,   "atmega88",   "Atmel ATmega88")
DEFINE_DEVICE_TYPE(ATMEGA168,  atmega168_device,  "atmega168",  "Atmel ATmega168")
DEFINE_DEVICE_TYPE(ATMEGA328,  atmega328_device,  "atmega328",  "Atmel ATmega328")
DEFINE_DEVICE_TYPE(ATMEGA644,  atmega644_device,  "atmega644",  "Atmel ATmega644")
DEFINE_DEVICE_TYPE(ATMEGA1280, atmega1280_device, "atmega1280", "Atmel ATmega1280")
DEFINE_DEVICE_TYPE(ATMEGA2560, atmega2560_device, "atmega2560", "Atmel ATmega2560")
DEFINE_DEVICE_TYPE(ATTINY15,   attiny15_device,   "attiny15",   "Atmel ATtiny15")

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

void atmega88_device::atmega88_internal_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(atmega88_device::regs_r), FUNC(atmega88_device::regs_w));
}

void atmega168_device::atmega168_internal_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(atmega168_device::regs_r), FUNC(atmega168_device::regs_w));
}

void atmega328_device::atmega328_internal_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(atmega328_device::regs_r), FUNC(atmega328_device::regs_w));
}

void atmega644_device::atmega644_internal_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(atmega644_device::regs_r), FUNC(atmega644_device::regs_w));
}

void atmega1280_device::atmega1280_internal_map(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(atmega1280_device::regs_r), FUNC(atmega1280_device::regs_w));
}

void atmega2560_device::atmega2560_internal_map(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(atmega2560_device::regs_r), FUNC(atmega2560_device::regs_w));
}

void attiny15_device::attiny15_internal_map(address_map &map)
{
	map(0x00, 0x1f).rw(FUNC(attiny15_device::regs_r), FUNC(attiny15_device::regs_w));
}

//-------------------------------------------------
//  atmega88_device - constructor
//-------------------------------------------------

atmega88_device::atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA88, 0x0fff, address_map_constructor(FUNC(atmega88_device::atmega88_internal_map), this), 3)
{
}

//-------------------------------------------------
//  atmega168_device - constructor
//-------------------------------------------------

atmega168_device::atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA168, 0x1fff, address_map_constructor(FUNC(atmega168_device::atmega168_internal_map), this), 3)
{
}

//-------------------------------------------------
//  atmega328_device - constructor
//-------------------------------------------------

atmega328_device::atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA328, 0x7fff, address_map_constructor(FUNC(atmega328_device::atmega328_internal_map), this), 3)
{
}

//-------------------------------------------------
//  atmega644_device - constructor
//-------------------------------------------------

atmega644_device::atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA644, 0xffff, address_map_constructor(FUNC(atmega644_device::atmega644_internal_map), this), 3)
{
}

//-------------------------------------------------
//  atmega1280_device - constructor
//-------------------------------------------------

atmega1280_device::atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA1280, 0x1ffff, address_map_constructor(FUNC(atmega1280_device::atmega1280_internal_map), this), 6)
{
}

//-------------------------------------------------
//  atmega2560_device - constructor
//-------------------------------------------------

atmega2560_device::atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATMEGA2560, 0x1ffff, address_map_constructor(FUNC(atmega2560_device::atmega2560_internal_map), this), 6)
{
}

//-------------------------------------------------
//  attiny15_device - constructor
//-------------------------------------------------

attiny15_device::attiny15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device(mconfig, tag, owner, clock, ATTINY15, 0x03ff, address_map_constructor(FUNC(attiny15_device::attiny15_internal_map), this), 2)
{
}

//-------------------------------------------------
//  avr8_device - constructor
//-------------------------------------------------

avr8_device::avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t addr_mask, address_map_constructor internal_map, int32_t num_timers)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_shifted_pc(0)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 22)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0, internal_map)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_lfuses(0x62)
	, m_hfuses(0x99)
	, m_efuses(0xff)
	, m_lock_bits(0xff)
	, m_pc(0)
	, m_num_timers(num_timers)
	, m_gpio_out_cb(*this)
	, m_gpio_in_cb(*this)
	, m_spi_active(false)
	, m_spi_prescale(0)
	, m_spi_prescale_count(0)
	, m_addr_mask(addr_mask)
	, m_interrupt_pending(false)
{
}


//-------------------------------------------------
//  static_set_low_fuses
//-------------------------------------------------

void avr8_device::set_low_fuses(const uint8_t byte)
{
	m_lfuses = byte;
}

//-------------------------------------------------
//  static_set_high_fuses
//-------------------------------------------------

void avr8_device::set_high_fuses(const uint8_t byte)
{
	m_hfuses = byte;
}

//-------------------------------------------------
//  static_set_extended_fuses
//-------------------------------------------------

void avr8_device::set_extended_fuses(const uint8_t byte)
{
	m_efuses = byte;
}

//-------------------------------------------------
//  static_set_lock_bits
//-------------------------------------------------

void avr8_device::set_lock_bits(const uint8_t byte)
{
	m_lock_bits = byte;
}

//-------------------------------------------------
//  unimplemented_opcode - bail on unspuported
//  instruction
//-------------------------------------------------

void avr8_device::unimplemented_opcode(uint32_t op)
{
//  machine().debug_break();
	fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, m_shifted_pc);
}


//-------------------------------------------------
//  is_long_opcode - returns true if opcode is 4
//  bytes long
//-------------------------------------------------

inline bool avr8_device::is_long_opcode(uint16_t op)
{
	if ((op & 0xf000) == 0x9000)
	{
		if ((op & 0x0f00) < 0x0400)
		{
			if ((op & 0x000f) == 0x0000)
			{
				return true;
			}
		}
		else if ((op & 0x0f00) < 0x0600)
		{
			if ((op & 0x000f) >= 0x000c)
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
	m_data = &space(AS_DATA);

	m_gpio_out_cb.resolve_all_safe();
	m_gpio_in_cb.resolve_all_safe(0);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_shifted_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_shifted_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_r[AVR8_REGIDX_SREG]).callimport().callexport().formatstr("%8s").noshow();
	state_add(AVR8_SREG,       "STATUS",    m_r[AVR8_REGIDX_SREG]).mask(0xff);
	state_add(AVR8_PC,         "PC",        m_shifted_pc).mask(m_addr_mask);
	state_add(AVR8_SPH,        "SPH",       m_r[AVR8_REGIDX_SPH]).mask(0xff);
	state_add(AVR8_SPL,        "SPL",       m_r[AVR8_REGIDX_SPL]).mask(0xff);
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
	save_item(NAME(m_boot_size));
	save_item(NAME(m_lfuses));
	save_item(NAME(m_hfuses));
	save_item(NAME(m_efuses));
	save_item(NAME(m_lock_bits));
	save_item(NAME(m_pc));
	save_item(NAME(m_r));

	// Timers
	save_item(NAME(m_timer_top));
	save_item(NAME(m_timer_increment));
	save_item(NAME(m_timer_prescale));
	save_item(NAME(m_timer_prescale_count));
	save_item(NAME(m_ocr2_not_reached_yet));
	save_item(NAME(m_wgm1));
	save_item(NAME(m_timer1_compare_mode));
	save_item(NAME(m_ocr1));
	save_item(NAME(m_timer1_count));

	// SPI
	save_item(NAME(m_spi_active));
	save_item(NAME(m_spi_prescale));
	save_item(NAME(m_spi_prescale_count));
	save_item(NAME(m_spi_prescale_countdown));

	// Misc.
	save_item(NAME(m_addr_mask));
	save_item(NAME(m_interrupt_pending));
	save_item(NAME(m_opcycles));

	// set our instruction counter
	set_icountptr(m_icount);

	populate_ops();

	m_add_flag_cache = std::make_unique<uint8_t[]>(0x10000);
	m_adc_flag_cache = std::make_unique<uint8_t[]>(0x20000);
	m_sub_flag_cache = std::make_unique<uint8_t[]>(0x10000);
	m_sbc_flag_cache = std::make_unique<uint8_t[]>(0x40000);
	m_bool_flag_cache = std::make_unique<uint8_t[]>(0x100);
	m_shift_flag_cache = std::make_unique<uint8_t[]>(0x10000);
	populate_add_flag_cache();
	populate_adc_flag_cache();
	populate_sub_flag_cache();
	populate_sbc_flag_cache();
	populate_bool_flag_cache();
	populate_shift_flag_cache();
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void avr8_device::device_reset()
{
	logerror("AVR low fuse bits: 0x%02X\n", m_lfuses);
	logerror("AVR high fuse bits: 0x%02X\n", m_hfuses);
	logerror("AVR extended fuse bits: 0x%02X\n", m_efuses);
	logerror("AVR lock bits: 0x%02X\n", m_lock_bits);

	switch ((m_hfuses & (BOOTSZ1 | BOOTSZ0)) >> 1)
	{
	case 0:
		if (m_addr_mask <= 0x0fff) { m_boot_size = 1024; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 2048; }
		else { m_boot_size = 4096; }
		break;
	case 1:
		if (m_addr_mask <= 0x0fff) { m_boot_size = 512; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 1024; }
		else { m_boot_size = 2048; }
		break;
	case 2:
		if (m_addr_mask <= 0x0fff) { m_boot_size = 256; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 512; }
		else { m_boot_size = 1024; }
		break;
	case 3:
		if (m_addr_mask <= 0x0fff) { m_boot_size = 128; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 256; }
		else { m_boot_size = 512; }
		break;
	default:
		break;
	}

	if (m_hfuses & BOOTRST)
	{
		m_shifted_pc = 0x0000;
		LOGMASKED(LOG_BOOT, "Booting AVR core from address 0x0000\n");
	} else {
		m_shifted_pc = (m_addr_mask + 1) - 2*m_boot_size;
		m_pc = m_shifted_pc >> 1;
		LOGMASKED(LOG_BOOT, "AVR Boot loader section size: %d words\n", m_boot_size);
	}

	for (int i = 0; i < 0x200; i++)
	{
		m_r[i] = 0;
	}

	m_spi_active = false;
	m_spi_prescale = 0;
	m_spi_prescale_count = 0;

	for (int t = 0; t < m_num_timers; t++)
	{
		m_timer_top[t] = 0;
		m_timer_increment[t] = 1;
		m_timer_prescale[t] = 0;
		m_timer_prescale_count[t] = 0;
	}

	m_wgm1 = 0;
	m_timer1_compare_mode[0] = 0;
	m_timer1_compare_mode[1] = 0;
	for (int reg = AVR8_REG_A; reg <= AVR8_REG_C; reg++)
	{
		m_ocr1[reg] = 0;
	}
	m_timer1_count = 0;

	m_ocr2_not_reached_yet = true;
	m_interrupt_pending = false;
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the CPU's address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector avr8_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void avr8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c",
			(m_r[AVR8_REGIDX_SREG] & 0x80) ? 'I' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x40) ? 'T' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x20) ? 'H' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x10) ? 'S' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x08) ? 'V' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x04) ? 'N' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x02) ? 'Z' : '-',
			(m_r[AVR8_REGIDX_SREG] & 0x01) ? 'C' : '-');
		break;
	}
}


std::unique_ptr<util::disasm_interface> avr8_device::create_disassembler()
{
	return std::make_unique<avr8_disassembler>();
}


//**************************************************************************
//  MEMORY ACCESSORS
//**************************************************************************

inline void avr8_device::push(uint8_t val)
{
	uint16_t sp = SPREG;
	m_data->write_byte(sp, val);
	sp--;
	m_r[AVR8_REGIDX_SPL] = sp & 0x00ff;
	m_r[AVR8_REGIDX_SPH] = (sp >> 8) & 0x00ff;
}

inline uint8_t avr8_device::pop()
{
	uint16_t sp = SPREG;
	sp++;
	m_r[AVR8_REGIDX_SPL] = sp & 0x00ff;
	m_r[AVR8_REGIDX_SPH] = (sp >> 8) & 0x00ff;
	return m_data->read_byte(sp);
}

//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void avr8_device::set_irq_line(uint16_t vector, int state)
{
	if (state)
	{
		if (SREG_R(AVR8_SREG_I))
		{
			SREG_W(AVR8_SREG_I, 0);
			push(m_pc & 0x00ff);
			push((m_pc >> 8) & 0x00ff);
			m_pc = vector;
			m_shifted_pc = vector << 1;
		}
		else
		{
			m_interrupt_pending = true;
		}
	}
}

struct interrupt_condition
{
	uint8_t m_intindex;
	uint8_t m_intreg;
	uint8_t m_intmask;
	uint8_t m_regindex;
	uint8_t m_regmask;
};

static const interrupt_condition s_int_conditions[AVR8_INTIDX_COUNT] =
{
	{ AVR8_INT_SPI_STC, AVR8_REGIDX_SPCR,   AVR8_SPCR_SPIE_MASK,     AVR8_REGIDX_SPSR,    AVR8_SPSR_SPIF_MASK },
	{ AVR8_INT_T0COMPB, AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_OCIE0B_MASK, AVR8_REGIDX_TIFR0,   AVR8_TIFR0_OCF0B_MASK },
	{ AVR8_INT_T0COMPA, AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_OCIE0A_MASK, AVR8_REGIDX_TIFR0,   AVR8_TIFR0_OCF0A_MASK },
	{ AVR8_INT_T0OVF,   AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_TOIE0_MASK,  AVR8_REGIDX_TIFR0,   AVR8_TIFR0_TOV0_MASK },
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
	const interrupt_condition &condition = s_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

//TODO: review this!
void atmega168_device::update_interrupt(int source)
{
	const interrupt_condition &condition = s_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

void atmega328_device::update_interrupt(int source)
{
	const interrupt_condition &condition = s_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

static const interrupt_condition s_mega644_int_conditions[AVR8_INTIDX_COUNT] =
{
	{ ATMEGA644_INT_SPI_STC, AVR8_REGIDX_SPCR,   AVR8_SPCR_SPIE_MASK,     AVR8_REGIDX_SPSR,    AVR8_SPSR_SPIF_MASK },
	{ ATMEGA644_INT_T0COMPB, AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_OCIE0B_MASK, AVR8_REGIDX_TIFR0,   AVR8_TIFR0_OCF0B_MASK },
	{ ATMEGA644_INT_T0COMPA, AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_OCIE0A_MASK, AVR8_REGIDX_TIFR0,   AVR8_TIFR0_OCF0A_MASK },
	{ ATMEGA644_INT_T0OVF,   AVR8_REGIDX_TIMSK0, AVR8_TIMSK0_TOIE0_MASK,  AVR8_REGIDX_TIFR0,   AVR8_TIFR0_TOV0_MASK },
	{ ATMEGA644_INT_T1CAPT,  AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_ICIE1_MASK,  AVR8_REGIDX_TIFR1,   AVR8_TIFR1_ICF1_MASK },
	{ ATMEGA644_INT_T1COMPB, AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_OCIE1B_MASK, AVR8_REGIDX_TIFR1,   AVR8_TIFR1_OCF1B_MASK },
	{ ATMEGA644_INT_T1COMPA, AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_OCIE1A_MASK, AVR8_REGIDX_TIFR1,   AVR8_TIFR1_OCF1A_MASK },
	{ ATMEGA644_INT_T1OVF,   AVR8_REGIDX_TIMSK1, AVR8_TIMSK1_TOIE1_MASK,  AVR8_REGIDX_TIFR1,   AVR8_TIFR1_TOV1_MASK },
	{ ATMEGA644_INT_T2COMPB, AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_OCIE2B_MASK, AVR8_REGIDX_TIFR2,   AVR8_TIFR2_OCF2B_MASK },
	{ ATMEGA644_INT_T2COMPA, AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_OCIE2A_MASK, AVR8_REGIDX_TIFR2,   AVR8_TIFR2_OCF2A_MASK },
	{ ATMEGA644_INT_T2OVF,   AVR8_REGIDX_TIMSK2, AVR8_TIMSK2_TOIE2_MASK,  AVR8_REGIDX_TIFR2,   AVR8_TIFR2_TOV2_MASK }
};

void atmega644_device::update_interrupt(int source)
{
	const interrupt_condition &condition = s_mega644_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

//TODO: review this!
void atmega1280_device::update_interrupt(int source)
{
	const interrupt_condition &condition = s_mega644_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}

//TODO: review this!
void atmega2560_device::update_interrupt(int source)
{
	const interrupt_condition &condition = s_mega644_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}


//**************************************************************************
//  REGISTER HANDLING
//**************************************************************************
void avr8_device::timer_tick()
{
	for (int count = 0; count < m_opcycles; count++)
	{
		if (m_spi_active && m_spi_prescale > 0 && m_spi_prescale_countdown >= 0)
		{
			m_spi_prescale_count++;
			if (m_spi_prescale_count >= m_spi_prescale)
			{
				uint8_t out_bit = (m_r[AVR8_REGIDX_SPDR] & (1 << m_spi_prescale_countdown)) >> m_spi_prescale_countdown;
				m_spi_prescale_countdown--;
				write_gpio(AVR8_IO_PORTB, (m_r[AVR8_REGIDX_PORTB] &~ AVR8_PORTB_MOSI) | (out_bit ? AVR8_PORTB_MOSI : 0));
				m_r[AVR8_REGIDX_PORTB] = (m_r[AVR8_REGIDX_PORTB] &~ AVR8_PORTB_MOSI) | (out_bit ? AVR8_PORTB_MOSI : 0);
				m_spi_prescale_count -= m_spi_prescale;
			}
		}

		for (int t = 0; t < m_num_timers; t++)
		{
			if (m_timer_prescale[t] != 0)
			{
				m_timer_prescale_count[t]++;
				if (m_timer_prescale_count[t] >= m_timer_prescale[t])
				{
					switch (t)
					{
					case 0: timer0_tick(); break;
					case 1: timer1_tick(); break;
					case 2: timer2_tick(); break;
					case 3: timer3_tick(); break;
					case 4: timer4_tick(); break;
					case 5: timer5_tick(); break;
					}
					m_timer_prescale_count[t] -= m_timer_prescale[t];
				}
			}
		}
	}
}

// Timer 0 Handling
void avr8_device::timer0_tick()
{
	static const uint8_t s_ocf0[2] = { (1 << AVR8_TIFR0_OCF0A_SHIFT), (1 << AVR8_TIFR0_OCF0B_SHIFT) };
	static const uint8_t s_int0[2] = { AVR8_INTIDX_OCF0A, AVR8_INTIDX_OCF0B };

	LOGMASKED(LOG_TIMER0_TICK, "%s: AVR8_WGM0: %d\n", machine().describe_context(), AVR8_WGM0);
	LOGMASKED(LOG_TIMER0_TICK, "%s: AVR8_TCCR0A_COM0B: %d\n", machine().describe_context(), AVR8_TCCR0A_COM0B);

	uint8_t count = m_r[AVR8_REGIDX_TCNT0];
	count++;

	switch (AVR8_WGM0)
	{
	case WGM02_NORMAL:
		LOGMASKED(LOG_TIMER0, "%s: WGM02_NORMAL: Unimplemented timer#0 waveform generation mode\n", machine().describe_context());
		break;

	case WGM02_PWM_PC:
		LOGMASKED(LOG_TIMER0, "%s: WGM02_PWM_PC: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
		break;

	case WGM02_CTC_CMP:
		switch (AVR8_TCCR0A_COM0B)
		{
		case 0: /* Normal Operation */
			if (count == AVR8_OCR0A)
			{
				m_r[AVR8_REGIDX_TIFR0] |= s_ocf0[AVR8_REG_A];
				update_interrupt(s_int0[AVR8_REG_A]);
				count = 0;
			}
			else if (count == AVR8_OCR0B)
			{
				m_r[AVR8_REGIDX_TIFR0] |= s_ocf0[AVR8_REG_B];
				update_interrupt(s_int0[AVR8_REG_B]);
			}
			break;

		case 1: /* Toggle OC0B on compare match */
			if (count == m_timer_top[0])
			{
				m_timer_top[0] = 0;
				LOGMASKED(LOG_TIMER0, "%s: timer0: Toggle OC0B on match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] ^ (1 << 5));
			}
			break;

		case 2: /* Clear OC0B on compare match */
			if (count == m_timer_top[0])
			{
				m_timer_top[0] = 0;
				LOGMASKED(LOG_TIMER0, "[0] timer0: Clear OC0B on match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] & ~(1 << 5));
			}
			break;

		case 3: /* Set OC0B on compare match */
			if (count == m_timer_top[0])
			{
				m_timer_top[0] = 0;
				LOGMASKED(LOG_TIMER0, "%s: timer0: Set OC0B on match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] | (1 << 5));
			}
			break;
		}
		break;

	case WGM02_FAST_PWM:
		LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_FAST_PWM: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
		break;

	case WGM02_PWM_PC_CMP:
		LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_PWM_PC_CMP: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
		break;

	case WGM02_FAST_PWM_CMP:
		LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_FAST_PWM_CMP: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
		break;

	default:
		LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: update_timer0_compare_mode: Unknown waveform generation mode: %02x\n", machine().describe_context(), AVR8_WGM0);
		break;
	}

	m_r[AVR8_REGIDX_TCNT0] = count;
}

void avr8_device::changed_tccr0a(uint8_t data)
{
	uint8_t oldtccr = AVR8_TCCR0A;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR0A = data;

	if (changed & AVR8_TCCR0A_WGM0_10_MASK)
	{
		update_timer_waveform_gen_mode(0, AVR8_WGM0);
	}
}

void avr8_device::timer0_force_output_compare(int reg)
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: timer0_force_output_compare: TODO; should be forcing OC0%c\n", machine().describe_context(), avr8_reg_name[reg]);
}

void avr8_device::changed_tccr0b(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: changed_tccr0b: data = 0x%02X\n", machine().describe_context(), data);

	uint8_t oldtccr = AVR8_TCCR0B;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR0B = data;

	if (changed & AVR8_TCCR0B_FOC0A_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_A);
	}

	if (changed & AVR8_TCCR0B_FOC0B_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_B);
	}

	if (changed & AVR8_TCCR0B_WGM0_2_MASK)
	{
		update_timer_waveform_gen_mode(0, AVR8_WGM0);
	}

	if (changed & AVR8_TCCR0B_CS_MASK)
	{
		update_timer_clock_source(0, AVR8_TIMER0_CLOCK_SELECT);
	}
}

void avr8_device::update_ocr0(uint8_t newval, uint8_t reg)
{
	m_r[(reg == AVR8_REG_A) ? AVR8_REGIDX_OCR0A : AVR8_REGIDX_OCR0B] = newval;
}

// Timer 1 Handling

inline void avr8_device::timer1_tick()
{
	/* TODO: Handle comparison, setting OC1A pin, detection of BOTTOM and TOP */

	static const uint8_t s_ocf1[2] = { (1 << AVR8_TIFR1_OCF1A_SHIFT), (1 << AVR8_TIFR1_OCF1B_SHIFT) };
	static const uint8_t s_int1[2] = { AVR8_INTIDX_OCF1A, AVR8_INTIDX_OCF1B };
	const uint16_t icr1 = AVR8_ICR1;
	int32_t increment = m_timer_increment[1];

	for (int32_t reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
	{
		switch (m_wgm1)
		{
		case WGM1_CTC_OCR:
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1_count: %04x\n", machine().describe_context(), m_timer1_count);
			LOGMASKED(LOG_TIMER1_TICK, "%s: OCR1%c: %04x\n", machine().describe_context(), reg ? 'A' : 'B', m_ocr1[reg]);
			if (m_timer1_count == 0xffff)
			{
				m_r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
				update_interrupt(AVR8_INTIDX_TOV1);
				m_timer1_count = 0;
				increment = 0;
			}

			if (m_timer1_count == m_ocr1[reg])
			{
				if (reg == AVR8_REG_A)
				{
					m_timer1_count = 0;
					increment = 0;
				}

				switch (m_timer1_compare_mode[reg] & 3)
				{
				case 0: /* Normal Operation; OC1A/B disconnected */
					break;

				case 1: /* Toggle OC1A on compare match */
					if (reg == AVR8_REG_A)
					{
						LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1%c on match\n", machine().describe_context());
						write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] ^ 2);
					}
					break;

				case 2: /* Clear OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] & ~(2 << reg));
					break;

				case 3: /* Set OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] | (2 << reg));
					break;
				}

				m_r[AVR8_REGIDX_TIFR1] |= s_ocf1[reg];
				update_interrupt(s_int1[reg]);
			}
			else if (m_timer1_count == 0)
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR1] &= ~AVR8_TIFR1_TOV1_MASK;
					update_interrupt(AVR8_INTIDX_TOV1);
				}
			}
			break;

		case WGM1_FAST_PWM_OCR:
			if (m_timer1_count == m_ocr1[reg])
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
					update_interrupt(AVR8_INTIDX_TOV1);
					m_timer1_count = 0;
					increment = 0;
				}

				switch (m_timer1_compare_mode[reg] & 3)
				{
				case 0: /* Normal Operation; OC1A/B disconnected */
					break;

				case 1: /* Toggle OC1A on compare match */
					if (reg == AVR8_REG_A)
					{
						LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1%c on match\n", machine().describe_context());
						write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] ^ 2);
					}
					break;

				case 2: /* Clear OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] & ~(2 << reg));
					break;

				case 3: /* Set OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] | (2 << reg));
					break;
				}

				m_r[AVR8_REGIDX_TIFR1] |= s_ocf1[reg];
				update_interrupt(s_int1[reg]);
			}
			else if (m_timer1_count == 0)
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR1] &= ~AVR8_TIFR1_TOV1_MASK;
					update_interrupt(AVR8_INTIDX_TOV1);
				}

				switch (m_timer1_compare_mode[reg] & 3)
				{
				case 0: /* Normal Operation; OC1A/B disconnected */
					break;

				case 1: /* Toggle OC1A at BOTTOM*/
					if (reg == AVR8_REG_A)
					{
						LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A at BOTTOM\n", machine().describe_context());
						write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] ^ 2);
					}
					break;

				case 2: /* Set OC1A/B at BOTTOM*/
					LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1%c at BOTTOM\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] | (2 << reg));
					break;

				case 3: /* Clear OC1A/B at BOTTOM */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1%c at BOTTOM\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] & ~(2 << reg));
					break;
				}
			}
			break;

		case WGM1_FAST_PWM_ICR:
			if (m_timer1_count == m_ocr1[reg])
			{
				switch (m_timer1_compare_mode[reg] & 3)
				{
				case 0: /* Normal Operation; OC1A/B disconnected */
					break;

				case 1: /* Toggle OC1A on compare match */
					if (reg == AVR8_REG_A)
					{
						LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1%c on match\n", machine().describe_context());
						write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] ^ 2);
					}
					break;

				case 2: /* Clear OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] & ~(2 << reg));
					break;

				case 3: /* Set OC1A/B on compare match */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1%c on match\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] | (2 << reg));
					break;
				}

				m_r[AVR8_REGIDX_TIFR1] |= s_ocf1[reg];
				update_interrupt(s_int1[reg]);
			}
			else if (m_timer1_count == 0)
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR1] &= ~AVR8_TIFR1_TOV1_MASK;
					update_interrupt(AVR8_INTIDX_TOV1);
				}

				switch (m_timer1_compare_mode[reg] & 3)
				{
				case 0: /* Normal Operation; OC1A/B disconnected */
					break;

				case 1: /* Toggle OC1A at BOTTOM*/
					if (reg == AVR8_REG_A)
					{
						LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A at BOTTOM\n", machine().describe_context());
						write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] ^ 2);
					}
					break;

				case 2: /* Set OC1A/B at BOTTOM*/
					LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1%c at BOTTOM\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] | (2 << reg));
					break;

				case 3: /* Clear OC1A/B at BOTTOM */
					LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1%c at BOTTOM\n", machine().describe_context(), reg ? 'B' : 'A');
					write_gpio(AVR8_IO_PORTB, m_r[AVR8_REGIDX_PORTB] & ~(2 << reg));
					break;
				}
			}

			if (m_timer1_count == icr1 && reg == 1)
			{
				m_r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
				update_interrupt(AVR8_INTIDX_TOV1);
				m_timer1_count = 0;
				increment = 0;
			}
			break;

		default:
			LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: timer1_tick: Unknown waveform generation mode: %02x\n", machine().describe_context(), m_wgm1);
			break;
		}
	}

	m_timer1_count += increment;
}

void avr8_device::update_timer_waveform_gen_mode(uint8_t t, uint8_t mode)
{
	int32_t oc_val = -1, ic_val = -1;

	switch (t)
	{
	case 0:
		oc_val = AVR8_OCR0A;
		ic_val = -1;
		break;
	case 1:
		oc_val = AVR8_OCR1A;
		ic_val = AVR8_ICR1;
		break;
	case 2:
		oc_val = AVR8_OCR2A;
		ic_val = -1;
		break;
	case 3:
		oc_val = AVR8_OCR3A;
		ic_val = AVR8_ICR3;
		break;
	case 4:
		oc_val = AVR8_OCR4A;
		ic_val = AVR8_ICR4;
		break;
	case 5:
		oc_val = AVR8_OCR5A;
		ic_val = AVR8_ICR5;
		break;
	}

	static const int32_t s_top_values_02[8] =
	{
		0xff, 0xff, oc_val, 0xff, -1, oc_val, -1, oc_val
	}; //table 20-8

	static const int32_t s_top_values_1345[16] =
	{
		0xffff, 0x00ff, 0x01ff, 0x03ff,
		oc_val, 0x00ff, 0x01ff, 0x03ff,
		ic_val, oc_val, ic_val, oc_val,
		ic_val, -1,     ic_val, oc_val
	}; //table 17-2

	switch (t)
	{
	case 0:
	case 2:
		m_timer_top[t] = s_top_values_02[mode];
		break;
	case 1:
	case 3:
	case 4:
	case 5:
		m_timer_top[t] = s_top_values_1345[mode];
		break;
	}

	if (t == 1)
	{
		m_wgm1 = ((m_r[AVR8_REGIDX_TCCR1B] & AVR8_TCCR1B_WGM1_32_MASK) >> 1) | (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_WGM1_10_MASK);
	}
	if (m_timer_top[t] == -1)
	{
		m_timer_top[t] = 0;
		LOGMASKED((LOG_TIMER0 + t), "%s: update_timer_waveform_gen_mode: Timer %d - Unsupported waveform generation type: %d\n", machine().describe_context(), t, mode);
	}
}

void avr8_device::changed_tccr1a(uint8_t data)
{
	uint8_t oldtccr = AVR8_TCCR1A;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	m_r[AVR8_REGIDX_TCCR1A] = newtccr;

	if (changed & AVR8_TCCR1A_WGM1_10_MASK)
	{
		update_timer_waveform_gen_mode(1, AVR8_WGM1);
	}

	m_timer1_compare_mode[AVR8_REG_A] = (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT;
	m_timer1_compare_mode[AVR8_REG_B] = (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT;
}

void avr8_device::update_timer1_input_noise_canceler()
{
	LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: update_timer1_input_noise_canceler: TODO\n", machine().describe_context());
}

void avr8_device::update_timer1_input_edge_select()
{
	LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: update_timer1_input_edge_select: TODO\n", machine().describe_context());
}

void avr8_device::changed_tccr1b(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: changed_tccr1b: data = 0x%02X\n", machine().describe_context(), data);

	uint8_t oldtccr = AVR8_TCCR1B;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	m_r[AVR8_REGIDX_TCCR1B] = newtccr;

	if (changed & AVR8_TCCR1B_ICNC1_MASK)
	{
		update_timer1_input_noise_canceler();
	}

	if (changed & AVR8_TCCR1B_ICES1_MASK)
	{
		update_timer1_input_edge_select();
	}

	if (changed & AVR8_TCCR1B_WGM1_32_MASK)
	{
		update_timer_waveform_gen_mode(1, AVR8_WGM1);
	}

	if (changed & AVR8_TCCR1B_CS_MASK)
	{
		update_timer_clock_source(1, AVR8_TIMER1_CLOCK_SELECT);
	}
}

void avr8_device::update_ocr1(uint16_t newval, uint8_t reg)
{
	static const int32_t s_high_indices[3] = { AVR8_REGIDX_OCR1AH, AVR8_REGIDX_OCR1BH, AVR8_REGIDX_OCR1CH };
	static const int32_t s_low_indices[3] =  { AVR8_REGIDX_OCR1AL, AVR8_REGIDX_OCR1BL, AVR8_REGIDX_OCR1CL };
	m_r[s_high_indices[reg]] = (uint8_t)(newval >> 8);
	m_r[s_low_indices[reg]]  = (uint8_t)newval;
	m_ocr1[reg] = newval;
}

// Timer 2 Handling

void avr8_device::timer2_tick()
{
	uint16_t count = m_r[AVR8_REGIDX_TCNT2];
	int32_t wgm2 = ((m_r[AVR8_REGIDX_TCCR2B] & AVR8_TCCR2B_WGM2_2_MASK) >> 1) |
					(m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_WGM2_10_MASK);

	// Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
	//uint8_t compare_mode[2] = { (m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2A_MASK) >> AVR8_TCCR2A_COM2A_SHIFT,
								//(m_r[AVR8_REGIDX_TCCR2A] & AVR8_TCCR2A_COM2B_MASK) >> AVR8_TCCR2A_COM2B_SHIFT };
	uint8_t ocr2[2] = { m_r[AVR8_REGIDX_OCR2A], m_r[AVR8_REGIDX_OCR2B] };
	uint8_t ocf2[2] = { (1 << AVR8_TIFR2_OCF2A_SHIFT), (1 << AVR8_TIFR2_OCF2B_SHIFT) };
	int32_t increment = m_timer_increment[2];

	for (int32_t reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
	{
		switch (wgm2)
		{
		case WGM02_NORMAL:
			if (count == 0xff)
			{
				m_r[AVR8_REGIDX_TIFR2] |= AVR8_TIFR2_TOV2_MASK;
			}
			break;

		case WGM02_FAST_PWM:
			if (reg == AVR8_REG_A)
			{
				if (count >= m_r[AVR8_REGIDX_OCR2A])
				{
					if (count >= 0xff)
					{
						// Turn on
						write_gpio(AVR8_IO_PORTD, m_r[AVR8_REGIDX_PORTD] | (1 << 7));
						m_r[AVR8_REGIDX_TCNT2] = 0;
						m_ocr2_not_reached_yet = true;
					}
					else
					{
						if (m_ocr2_not_reached_yet)
						{
							// Turn off
							write_gpio(AVR8_IO_PORTD, m_r[AVR8_REGIDX_PORTD] & ~(1 << 7));
							m_ocr2_not_reached_yet = false;
						}
					}
				}
			}
			break;

		case WGM02_FAST_PWM_CMP:
			if (count == ocr2[reg])
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR2] |= AVR8_TIFR2_TOV2_MASK;
					count = 0;
					increment = 0;
				}

				m_r[AVR8_REGIDX_TIFR2] |= ocf2[reg];
			}
			else if (count == 0)
			{
				if (reg == AVR8_REG_A)
				{
					m_r[AVR8_REGIDX_TIFR2] &= ~AVR8_TIFR2_TOV2_MASK;
				}
			}
			break;

		default:
			LOGMASKED(LOG_TIMER2 | LOG_UNKNOWN, "%s: timer2_tick: Unknown waveform generation mode: %02x\n", machine().describe_context(), wgm2);
			break;
		}
	}

	m_r[AVR8_REGIDX_TCNT2] += increment;

	update_interrupt(AVR8_INTIDX_OCF2A);
	update_interrupt(AVR8_INTIDX_OCF2B);
	update_interrupt(AVR8_INTIDX_TOV2);
}

void avr8_device::changed_tccr2a(uint8_t data)
{
	uint8_t oldtccr = AVR8_TCCR2A;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR2A = data;

	if (changed & AVR8_TCCR2A_WGM2_10_MASK)
	{
		update_timer_waveform_gen_mode(2, AVR8_WGM2);
	}
}

void avr8_device::timer2_force_output_compare(int reg)
{
	LOGMASKED(LOG_TIMER2 | LOG_UNKNOWN, "%s: force_output_compare: TODO; should be forcing OC2%c\n", machine().describe_context(), avr8_reg_name[reg]);
}

void avr8_device::changed_tccr2b(uint8_t data)
{
	LOGMASKED(LOG_TIMER2, "%s: changed_tccr2b: data = 0x%02X\n", machine().describe_context(), data);

	uint8_t oldtccr = AVR8_TCCR2B;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR2B = data;

	if (changed & AVR8_TCCR2B_FOC2A_MASK)
	{
		timer2_force_output_compare(AVR8_REG_A);
	}

	if (changed & AVR8_TCCR2B_FOC2B_MASK)
	{
		timer2_force_output_compare(AVR8_REG_B);
	}

	if (changed & AVR8_TCCR2B_WGM2_2_MASK)
	{
		update_timer_waveform_gen_mode(2, AVR8_WGM2);
	}

	if (changed & AVR8_TCCR2B_CS_MASK)
	{
		update_timer_clock_source(2, AVR8_TIMER2_CLOCK_SELECT);
	}
}

void avr8_device::update_ocr2(uint8_t newval, uint8_t reg)
{
	m_r[(reg == AVR8_REG_A) ? AVR8_REGIDX_OCR2A : AVR8_REGIDX_OCR2B] = newval;

	// Nothing needs to be done? All handled in timer callback
}

/************************************************************************************************/

// Timer 3 Handling
void avr8_device::timer3_tick()
{
}

/************************************************************************************************/

void avr8_device::timer4_tick()
{
	/* TODO: Handle comparison, setting OC1x pins, detection of BOTTOM and TOP */
	LOGMASKED(LOG_TIMER4_TICK, "%s: AVR8_WGM4: %d\n", machine().describe_context(), AVR8_WGM4);
	LOGMASKED(LOG_TIMER4_TICK, "%s: AVR8_TCCR4A_COM4B: %d\n", machine().describe_context(), AVR8_TCCR4A_COM4B);

	uint16_t count = (m_r[AVR8_REGIDX_TCNT4H] << 8) | m_r[AVR8_REGIDX_TCNT4L];

	// Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
	// uint8_t compare_mode[2] = { (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT,
	//                             (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT };
	// uint16_t ocr4[2] = { static_cast<uint16_t>((m_r[AVR8_REGIDX_OCR4AH] << 8) | m_r[AVR8_REGIDX_OCR4AL]),
							// static_cast<uint16_t>((m_r[AVR8_REGIDX_OCR4BH] << 8) | m_r[AVR8_REGIDX_OCR4BL]) };
	// TODO  uint8_t ocf4[2] = { (1 << AVR8_TIFR4_OCF4A_SHIFT), (1 << AVR8_TIFR4_OCF4B_SHIFT) };
	// TODO  uint8_t int4[2] = { AVR8_INTIDX_OCF4A, AVR8_INTIDX_OCF4B };
	int32_t increment = m_timer_increment[4];

	switch (AVR8_WGM4)
	{
	case WGM4_FAST_PWM_8:
	case WGM4_FAST_PWM_9:
	case WGM4_FAST_PWM_10:
		switch (AVR8_TCCR4A_COM4B)
		{
		case 0: /* Normal Operation */
			break;
		case 1: /* TODO */
			break;
		case 2: /* Non-inverting mode */
			if (count == m_timer_top[4])
			{
				// Clear OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: non-inverting mode, Clear OC0B\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] & ~(1 << 5));
			}
			else if (count == 0)
			{
				// Set OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: non-inverting mode, Set OC0B\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] | (1 << 5));
			}
			break;
		case 3: /* Inverting mode */
			if (count == m_timer_top[4])
			{
				// Set OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: inverting mode, Clear OC0B\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] | (1 << 5));
			}
			else if (count == 0)
			{
				// Clear OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: inverting mode, Set OC0B\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTG, m_r[AVR8_REGIDX_PORTG] & ~(1 << 5));
			}
			break;
		}
		break;

	case WGM4_CTC_OCR:
		LOGMASKED(LOG_TIMER4, "%s: timer4: tick WGM4_CTC_OCR: %d\n", machine().describe_context(), count);
		if (count == 0xffff)
		{
			m_r[AVR8_REGIDX_TIFR4] |= AVR8_TIFR4_TOV4_MASK;
			update_interrupt(AVR8_INTIDX_TOV4);
			count = 0;
			increment = 0;
		}

		/* TODO: test for match against all 3 registers */
		break;

	case WGM1_FAST_PWM_OCR:
		/* TODO: test for match against all 3 registers */
		break;

	default:
		LOGMASKED(LOG_TIMER4 | LOG_UNKNOWN, "%s: timer4_tick: Unknown waveform generation mode: %02x\n", machine().describe_context(), AVR8_WGM4);
		break;
	}

	count += increment;
	m_r[AVR8_REGIDX_TCNT4H] = (count >> 8) & 0xff;
	m_r[AVR8_REGIDX_TCNT4L] = count & 0xff;
}

void avr8_device::update_timer_clock_source(uint8_t t, uint8_t clock_select)
{
	static const int s_prescale_values[2][8] =
	{
		{ 0, 1, 8, 64, 256, 1024, -1, -1 }, // T0/T1/T3/T4/T5
		{ 0, 1, 8, 32, 64, 128, 256, 1024 } // T2
	};
	m_timer_prescale[t] = s_prescale_values[(t == 2) ? 1 : 0][clock_select];

	LOGMASKED((LOG_TIMER0 + t), "%s: update_timer_clock_source: t = %d, cs = %d\n", machine().describe_context(), t, clock_select);

	if (m_timer_prescale[t] == 0xffff)
	{
		LOGMASKED((LOG_TIMER0 + t), "%s: timer%d: update_timer_clock_source: External trigger mode not implemented yet\n", machine().describe_context(), t);
		m_timer_prescale[t] = 0;
	}

	if (m_timer_prescale_count[t] > m_timer_prescale[t])
	{
		m_timer_prescale_count[t] = m_timer_prescale[t] - 1;
	}
}

void avr8_device::changed_tccr3a(uint8_t data)
{
	// TODO
	//  AVR8_TCCR3A = data;
}

void avr8_device::changed_tccr3b(uint8_t data)
{
	LOGMASKED(LOG_TIMER3 | LOG_UNKNOWN, "%s: (not yet implemented) changed_tccr3b: data = %02X\n", machine().describe_context(), data);
}

void avr8_device::changed_tccr3c(uint8_t data)
{
	//  uint8_t oldtccr = AVR8_TCCR3C;
	//  uint8_t newtccr = data;
	//  uint8_t changed = newtccr ^ oldtccr;
	LOGMASKED(LOG_TIMER3 | LOG_UNKNOWN, "%s: (not yet implemented) changed_tccr3c: data = %02X\n", machine().describe_context(), data);

	//  AVR8_TCCR3C = data;
}

void avr8_device::changed_tccr4a(uint8_t data)
{
	uint8_t oldtccr = AVR8_TCCR4A;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR4A = data;

	if (changed & AVR8_TCCR4A_WGM4_10_MASK)
	{
		update_timer_waveform_gen_mode(4, AVR8_WGM4);
	}
}

void avr8_device::changed_tccr4b(uint8_t data)
{
	LOGMASKED(LOG_TIMER4, "%s: changed_tccr4b: data = %02X\n", machine().describe_context(), data);

	uint8_t oldtccr = AVR8_TCCR4B;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR4B = data;

	if (changed & AVR8_TCCR4B_FOC4A_MASK)
	{
		// TODO
		// timer4_force_output_compare(AVR8_REG_A);
	}

	if (changed & AVR8_TCCR4B_FOC4B_MASK)
	{
		// TODO
		// timer4_force_output_compare(AVR8_REG_B);
	}

	if (changed & AVR8_TCCR4B_WGM4_32_MASK)
	{
		update_timer_waveform_gen_mode(4, AVR8_WGM4);
	}

	if (changed & AVR8_TCCR4B_CS_MASK)
	{
		update_timer_clock_source(4, AVR8_TIMER4_CLOCK_SELECT);
	}
}

void avr8_device::changed_tccr4c(uint8_t data)
{
	//  uint8_t oldtccr = AVR8_TCCR4C;
	//  uint8_t newtccr = data;
	//  uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR4C = data;
}

/************************************************************************************************/

// Timer 5 Handling
void avr8_device::timer5_tick()
{
	LOGMASKED(LOG_TIMER5_TICK, "%s: AVR8_WGM5: %d\n", machine().describe_context(), AVR8_WGM5);
	LOGMASKED(LOG_TIMER5_TICK, "%s: AVR8_TCCR5A_COM5B: %d\n", machine().describe_context(), AVR8_TCCR5A_COM5B);

	uint16_t count = (AVR8_TCNT5H << 8) + AVR8_TCNT5L;
	int32_t increment = m_timer_increment[5];

	switch (AVR8_WGM5)
	{
	case WGM5_NORMAL:
	case WGM5_PWM_8_PC:
	case WGM5_PWM_9_PC:
	case WGM5_PWM_10_PC:
//    case WGM5_CTC_OCR:
	case WGM5_FAST_PWM_8:
	case WGM5_FAST_PWM_9:
	case WGM5_FAST_PWM_10:
	case WGM5_PWM_PFC_ICR:
	case WGM5_PWM_PFC_OCR:
	case WGM5_PWM_PC_ICR:
	case WGM5_PWM_PC_OCR:
	case WGM5_CTC_ICR:
	case WGM5_FAST_PWM_ICR:
	case WGM5_FAST_PWM_OCR:
		LOGMASKED(LOG_TIMER5 | LOG_UNKNOWN, "%s: Unimplemented timer5 waveform generation mode: AVR8_WGM5 = 0x%02X\n", machine().describe_context(), AVR8_WGM5);
		break;

	case WGM5_CTC_OCR:
		// TODO: Please verify, might be wrong
		switch (AVR8_TCCR5A_COM5B)
		{
		case 0: /* Normal Operation */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
			}
			break;
		case 1: /* Toggle OC5B on compare match */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
				LOGMASKED(LOG_TIMER5, "%s: timer5: Toggle OC5B on compare match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTL, m_r[AVR8_REGIDX_PORTL] ^ (1 << 4));
			}
			break;
		case 2: /* Clear OC5B on compare match */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
				// Clear OC5B
				LOGMASKED(LOG_TIMER5, "%s: timer5: Clear OC5B on compare match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTL, m_r[AVR8_REGIDX_PORTL] & ~(1 << 4));
			}
			break;
		case 3: /* Set OC5B on compare match */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
				LOGMASKED(LOG_TIMER5, "%s: timer5: Set OC5B on compare match\n", machine().describe_context());
				write_gpio(AVR8_IO_PORTL, m_r[AVR8_REGIDX_PORTL] | (1 << 4));
			}
			break;
		}
		break;

	default:
		LOGMASKED(LOG_TIMER5 | LOG_UNKNOWN, "%s: timer5: Unknown waveform generation mode: %02x\n", machine().describe_context(), AVR8_WGM5);
		break;
	}

	count += increment;
	m_r[AVR8_REGIDX_TCNT5H] = (count >> 8) & 0xff;
	m_r[AVR8_REGIDX_TCNT5L] = count & 0xff;
}

void avr8_device::changed_tccr5a(uint8_t data)
{
	uint8_t oldtccr = AVR8_TCCR5A;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR5A = data;

	if (changed & AVR8_TCCR5A_WGM5_10_MASK)
	{
		update_timer_waveform_gen_mode(5, AVR8_WGM5);
	}
}

void avr8_device::changed_tccr5b(uint8_t data)
{
	LOGMASKED(LOG_TIMER5, "%s: changed_tccr5b: data = %02X\n", machine().describe_context(), data);

	uint8_t oldtccr = AVR8_TCCR5B;
	uint8_t newtccr = data;
	uint8_t changed = newtccr ^ oldtccr;

	AVR8_TCCR5B = data;

	if (changed & AVR8_TCCR5C_FOC5A_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_A);
	}

	if (changed & AVR8_TCCR5C_FOC5B_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_B);
	}

	if (changed & AVR8_TCCR5C_FOC5C_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_C);
	}

	if (changed & AVR8_TCCR5B_WGM5_32_MASK)
	{
		update_timer_waveform_gen_mode(5, AVR8_WGM5);
	}

	if (changed & AVR8_TCCR5B_CS_MASK)
	{
		update_timer_clock_source(5, AVR8_TIMER5_CLOCK_SELECT);
	}
}

/************************************************************************************************/


/****************/
/* SPI Handling */
/****************/

void avr8_device::enable_spi()
{
	// TODO
}

void avr8_device::disable_spi()
{
	// TODO
}

void avr8_device::spi_update_masterslave_select()
{
	// TODO
}

void avr8_device::spi_update_clock_polarity()
{
	// TODO
}

void avr8_device::spi_update_clock_phase()
{
	// TODO
}

void avr8_device::spi_update_clock_rate()
{
	static const uint8_t s_spi_clock_divisor[8] =
	{
		4, 16, 64, 128, 2, 8, 32, 64
	};
	m_spi_prescale = s_spi_clock_divisor[AVR8_SPI_RATE];
	m_spi_prescale_count &= m_spi_prescale - 1;
}

void avr8_device::change_spcr(uint8_t data)
{
	uint8_t oldspcr = AVR8_SPCR;
	uint8_t newspcr = data;
	uint8_t changed = newspcr ^ oldspcr;
	uint8_t high_to_low = ~newspcr & oldspcr;
	uint8_t low_to_high = newspcr & ~oldspcr;

	AVR8_SPCR = data;

	if (changed & AVR8_SPCR_SPIE_MASK)
	{
		// Check for SPI interrupt condition
		update_interrupt(AVR8_INTIDX_SPI);
	}

	if (low_to_high & AVR8_SPCR_SPE_MASK)
	{
		enable_spi();
	}
	else if (high_to_low & AVR8_SPCR_SPE_MASK)
	{
		disable_spi();
	}

	if (changed & AVR8_SPCR_MSTR_MASK)
	{
		spi_update_masterslave_select();
	}

	if (changed & AVR8_SPCR_CPOL_MASK)
	{
		spi_update_clock_polarity();
	}

	if (changed & AVR8_SPCR_CPHA_MASK)
	{
		spi_update_clock_phase();
	}

	if (changed & AVR8_SPCR_SPR_MASK)
	{
		spi_update_clock_rate();
	}
}

void avr8_device::change_spsr(uint8_t data)
{
	uint8_t oldspsr = AVR8_SPSR;
	uint8_t newspsr = data;
	uint8_t changed = newspsr ^ oldspsr;

	AVR8_SPSR &= ~1;
	AVR8_SPSR |= data & 1;

	if (changed & AVR8_SPSR_SPR2X_MASK)
	{
		spi_update_clock_rate();
	}
}

/*****************************************************************************/

void avr8_device::write_gpio(const uint8_t port, const uint8_t data)
{
	static const uint16_t s_port_to_reg[11] =
	{
		AVR8_REGIDX_PORTA,
		AVR8_REGIDX_PORTB,
		AVR8_REGIDX_PORTC,
		AVR8_REGIDX_PORTD,
		AVR8_REGIDX_PORTE,
		AVR8_REGIDX_PORTF,
		AVR8_REGIDX_PORTG,
		AVR8_REGIDX_PORTH,
		AVR8_REGIDX_PORTJ,
		AVR8_REGIDX_PORTK,
		AVR8_REGIDX_PORTL
	};

	m_r[s_port_to_reg[port]] = data;
	m_gpio_out_cb[port](data);
}

uint8_t avr8_device::read_gpio(const uint8_t port)
{
	return m_gpio_in_cb[port]();
}

/*****************************************************************************/

void avr8_device::regs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case AVR8_REGIDX_R0:
	case AVR8_REGIDX_R1:
	case AVR8_REGIDX_R2:
	case AVR8_REGIDX_R3:
	case AVR8_REGIDX_R4:
	case AVR8_REGIDX_R5:
	case AVR8_REGIDX_R6:
	case AVR8_REGIDX_R7:
	case AVR8_REGIDX_R8:
	case AVR8_REGIDX_R9:
	case AVR8_REGIDX_R10:
	case AVR8_REGIDX_R11:
	case AVR8_REGIDX_R12:
	case AVR8_REGIDX_R13:
	case AVR8_REGIDX_R14:
	case AVR8_REGIDX_R15:
	case AVR8_REGIDX_R16:
	case AVR8_REGIDX_R17:
	case AVR8_REGIDX_R18:
	case AVR8_REGIDX_R19:
	case AVR8_REGIDX_R20:
	case AVR8_REGIDX_R21:
	case AVR8_REGIDX_R22:
	case AVR8_REGIDX_R23:
	case AVR8_REGIDX_R24:
	case AVR8_REGIDX_R25:
	case AVR8_REGIDX_R26:
	case AVR8_REGIDX_R27:
	case AVR8_REGIDX_R28:
	case AVR8_REGIDX_R29:
	case AVR8_REGIDX_R30:
	case AVR8_REGIDX_R31:
		m_r[offset] = data;
		break;

	case AVR8_REGIDX_PORTA:
		LOGMASKED(LOG_GPIO, "%s: PORTA Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTA, data);
		break;

	case AVR8_REGIDX_PORTB:
		LOGMASKED(LOG_GPIO, "%s: PORTB Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTB, data);
		break;

	case AVR8_REGIDX_PORTC:
		LOGMASKED(LOG_GPIO, "%s: PORTC Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTC, data);
		break;

	case AVR8_REGIDX_PORTD:
		LOGMASKED(LOG_GPIO, "%s: PORTD Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTD, data);
		break;

	case AVR8_REGIDX_PORTE:
		LOGMASKED(LOG_GPIO, "%s: PORTE Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTE, data);
		break;

	case AVR8_REGIDX_PORTF:
		LOGMASKED(LOG_GPIO, "%s: PORTF Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTF, data);
		break;

	case AVR8_REGIDX_PORTG:
		LOGMASKED(LOG_GPIO, "%s: PORTG Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTG, data);
		break;

	case AVR8_REGIDX_PORTH:
		LOGMASKED(LOG_GPIO, "%s: PORTH Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTH, data);
		break;

	case AVR8_REGIDX_PORTJ:
		LOGMASKED(LOG_GPIO, "%s: PORTJ Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTJ, data);
		break;

	case AVR8_REGIDX_PORTK:
		LOGMASKED(LOG_GPIO, "%s: PORTK Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTK, data);
		break;

	case AVR8_REGIDX_PORTL:
		LOGMASKED(LOG_GPIO, "%s: PORTL Write: %02x\n", machine().describe_context(), data);
		write_gpio(AVR8_IO_PORTL, data);
		break;

	case AVR8_REGIDX_DDRA:
	case AVR8_REGIDX_DDRB:
	case AVR8_REGIDX_DDRC:
	case AVR8_REGIDX_DDRD:
	case AVR8_REGIDX_SREG:
	case AVR8_REGIDX_RAMPZ:
	case AVR8_REGIDX_SPH:
	case AVR8_REGIDX_SPL:
		m_r[offset] = data;
		break;

	case AVR8_REGIDX_TCCR0B:
		LOGMASKED(LOG_TIMER0, "%s: TCCR0B = %02x\n", machine().describe_context(), data);
		changed_tccr0b(data);
		break;

	case AVR8_REGIDX_TCCR0A:
		LOGMASKED(LOG_TIMER0, "%s: TCCR0A = %02x\n", machine().describe_context(), data);
		changed_tccr0a(data);
		break;

	case AVR8_REGIDX_OCR0A:
		LOGMASKED(LOG_TIMER0, "%s: OCR0A = %02x\n", machine().describe_context(), data);
		update_ocr0(data, AVR8_REG_A);
		break;

	case AVR8_REGIDX_OCR0B:
		LOGMASKED(LOG_TIMER0, "%s: OCR0B = %02x\n", machine().describe_context(), data);
		update_ocr0(data, AVR8_REG_B);
		break;

	case AVR8_REGIDX_TIFR0:
		LOGMASKED(LOG_TIMER0, "%s: TIFR0 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIFR0] &= ~(data & AVR8_TIFR0_MASK);
		update_interrupt(AVR8_INTIDX_OCF0A);
		update_interrupt(AVR8_INTIDX_OCF0B);
		update_interrupt(AVR8_INTIDX_TOV0);
		break;

	case AVR8_REGIDX_TCNT0:
		AVR8_TCNT0 = data;
		break;

	case AVR8_REGIDX_TIFR1:
		LOGMASKED(LOG_TIMER1, "%s: TIFR1 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIFR1] &= ~(data & AVR8_TIFR1_MASK);
		update_interrupt(AVR8_INTIDX_ICF1);
		update_interrupt(AVR8_INTIDX_OCF1A);
		update_interrupt(AVR8_INTIDX_OCF1B);
		update_interrupt(AVR8_INTIDX_TOV1);
		break;

	case AVR8_REGIDX_TIFR2:
		LOGMASKED(LOG_TIMER2, "%s: TIFR2 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIFR2] &= ~(data & AVR8_TIFR2_MASK);
		update_interrupt(AVR8_INTIDX_OCF2A);
		update_interrupt(AVR8_INTIDX_OCF2B);
		update_interrupt(AVR8_INTIDX_TOV2);
		break;

	case AVR8_REGIDX_GTCCR:
		if (data & AVR8_GTCCR_PSRASY_MASK)
		{
			data &= ~AVR8_GTCCR_PSRASY_MASK;
			m_timer_prescale_count[2] = 0;
		}
		break;

	// EEPROM registers
	case AVR8_REGIDX_EEARL:
	case AVR8_REGIDX_EEARH:
	case AVR8_REGIDX_EEDR:
		m_r[offset] = data;
		break;

	case AVR8_REGIDX_EECR:
		m_r[offset] = data;

		if (data & AVR8_EECR_EERE_MASK)
		{
			uint16_t addr = (m_r[AVR8_REGIDX_EEARH] & AVR8_EEARH_MASK) << 8;
			addr |= m_r[AVR8_REGIDX_EEARL];
			m_r[AVR8_REGIDX_EEDR] = m_eeprom[addr];
			LOGMASKED(LOG_EEPROM, "%s: EEPROM read @ %04x data = %02x\n", machine().describe_context(), addr, m_eeprom[addr]);
		}
		if ((data & AVR8_EECR_EEPE_MASK) && (data & AVR8_EECR_EEMPE_MASK))
		{
			uint16_t addr = (m_r[AVR8_REGIDX_EEARH] & AVR8_EEARH_MASK) << 8;
			addr |= m_r[AVR8_REGIDX_EEARL];
			m_eeprom[addr] = m_r[AVR8_REGIDX_EEDR];
			LOGMASKED(LOG_EEPROM, "%s: EEPROM write @ %04x data = %02x ('%c')\n", machine().describe_context(), addr, m_eeprom[addr], m_eeprom[addr]);

			// Indicate that we've finished writing a value to the EEPROM.
			// TODO: this should only happen after a certain dalay.
			m_r[offset] = data & ~AVR8_EECR_EEPE_MASK;
		}
		break;

	case AVR8_REGIDX_GPIOR0:
		LOGMASKED(LOG_GPIO, "%s: GPIOR0 Write: %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_GPIOR0] = data;
		break;

	case AVR8_REGIDX_GPIOR1:
		LOGMASKED(LOG_GPIO, "%s: GPIOR1 Write: %02x\n", machine().describe_context(), data);
		m_r[offset] = data;
		break;

	case AVR8_REGIDX_GPIOR2:
		LOGMASKED(LOG_GPIO, "%s: GPIOR2 Write: %02x\n", machine().describe_context(), data);
		m_r[offset] = data;
		break;

	case AVR8_REGIDX_SPSR:
		change_spsr(data);
		break;

	case AVR8_REGIDX_SPCR:
		change_spcr(data);
		break;

	case AVR8_REGIDX_SPDR:
	{
		m_r[AVR8_REGIDX_SPDR] = data;
		m_spi_active = true;
		m_spi_prescale_countdown = 7;
		m_spi_prescale_count = 0;
		break;
	}

	case AVR8_REGIDX_WDTCSR:
		LOGMASKED(LOG_WDOG, "%s: (not yet implemented) WDTCSR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_CLKPR:
		LOGMASKED(LOG_CLOCK, "%s: (not yet implemented) CLKPR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PRR0:
		LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR0 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PRR1:
		LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR1 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_OSCCAL:
		LOGMASKED(LOG_OSC, "%s: (not yet implemented) OSCCAL = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PCICR:
		LOGMASKED(LOG_PINCHG, "%s: (not yet implemented) PCICR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_EICRA:
		LOGMASKED(LOG_GPIO, "%s: (not yet implemented) EICRA = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_EICRB:
		LOGMASKED(LOG_GPIO, "%s: (not yet implemented) EICRB = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PCMSK0:
		LOGMASKED(LOG_PINCHG, "%s: (not yet implemented) PCMSK0 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PCMSK1:
		LOGMASKED(LOG_PINCHG, "%s: (not yet implemented) PCMSK1 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_PCMSK2:
		LOGMASKED(LOG_PINCHG, "%s: (not yet implemented) PCMSK2 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TIMSK0:
		LOGMASKED(LOG_TIMER0, "%s: TIMSK0 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK0] = data;
		update_interrupt(AVR8_INTIDX_OCF0A);
		update_interrupt(AVR8_INTIDX_OCF0B);
		update_interrupt(AVR8_INTIDX_TOV0);
		break;

	case AVR8_REGIDX_TIMSK1:
		LOGMASKED(LOG_TIMER1, "%s: TIMSK1 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK1] = data;
		update_interrupt(AVR8_INTIDX_ICF1);
		update_interrupt(AVR8_INTIDX_OCF1A);
		update_interrupt(AVR8_INTIDX_OCF1B);
		update_interrupt(AVR8_INTIDX_TOV1);
		break;

	case AVR8_REGIDX_TIMSK2:
		LOGMASKED(LOG_TIMER2, "%s: TIMSK2 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK2] = data;
		update_interrupt(AVR8_INTIDX_OCF2A);
		update_interrupt(AVR8_INTIDX_OCF2B);
		update_interrupt(AVR8_INTIDX_TOV2);
		break;

	case AVR8_REGIDX_TIMSK3:
		LOGMASKED(LOG_TIMER3, "%s: TIMSK3 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK3] = data;
		update_interrupt(AVR8_INTIDX_OCF3A);
		update_interrupt(AVR8_INTIDX_OCF3B);
		update_interrupt(AVR8_INTIDX_TOV3);
		break;

	case AVR8_REGIDX_TIMSK4:
		LOGMASKED(LOG_TIMER4, "%s: TIMSK4 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK4] = data;
		update_interrupt(AVR8_INTIDX_OCF4A);
		update_interrupt(AVR8_INTIDX_OCF4B);
		update_interrupt(AVR8_INTIDX_TOV4);
		break;

	case AVR8_REGIDX_TIMSK5:
		LOGMASKED(LOG_TIMER5, "%s: TIMSK5 = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TIMSK5] = data;
		update_interrupt(AVR8_INTIDX_OCF5A);
		update_interrupt(AVR8_INTIDX_OCF5B);
		update_interrupt(AVR8_INTIDX_TOV5);
		break;

	case AVR8_REGIDX_XMCRA:
		LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRA = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_XMCRB:
		LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRB = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_ADCL:
		LOGMASKED(LOG_ADC, "%s: (not yet implemented) ADCL = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_ADCH:
		LOGMASKED(LOG_ADC, "%s: (not yet implemented) ADCH = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_ADCSRA:
		LOGMASKED(LOG_ADC, "%s: (not yet implemented) ADCSRA = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_ADCSRB:
		LOGMASKED(LOG_ADC, "%s: (not yet implemented) ADCSRB = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_ADMUX:
		LOGMASKED(LOG_ADC, "%s: (not yet implemented) ADMUX = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_DIDR0:
		LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR0 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_DIDR1:
		LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR1 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_DIDR2:
		LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR2 = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TCCR1A:
		LOGMASKED(LOG_TIMER1, "%s: TCCR1A = %02x\n", machine().describe_context(), data);
		changed_tccr1a(data);
		break;

	case AVR8_REGIDX_TCCR1B:
		LOGMASKED(LOG_TIMER1, "%s: TCCR1B = %02x\n", machine().describe_context(), data);
		changed_tccr1b(data);
		break;

	case AVR8_REGIDX_TCCR1C:
		LOGMASKED(LOG_TIMER1, "%s: (not yet implemented) TCCR1C = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TCNT1L:
		m_timer1_count = (m_timer1_count & 0xff00) | data;
		break;

	case AVR8_REGIDX_TCNT1H:
		m_timer1_count = (m_timer1_count & 0x00ff) | (data << 8);
		break;

	case AVR8_REGIDX_ICR1L:
		AVR8_ICR1L = data;
		break;

	case AVR8_REGIDX_ICR1H:
		AVR8_ICR1H = data;
		break;

	case AVR8_REGIDX_OCR1AL:
		LOGMASKED(LOG_TIMER1, "%s: OCR1AL = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1A & 0xff00) | data, AVR8_REG_A);
		break;

	case AVR8_REGIDX_OCR1AH:
		LOGMASKED(LOG_TIMER1, "%s: OCR1AH = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1A & 0x00ff) | (data << 8), AVR8_REG_A);
		break;

	case AVR8_REGIDX_OCR1BL:
		LOGMASKED(LOG_TIMER1, "%s: OCR1BL = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1B & 0xff00) | data, AVR8_REG_B);
		break;

	case AVR8_REGIDX_OCR1BH:
		LOGMASKED(LOG_TIMER1, "%s: OCR1BH = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1B & 0x00ff) | (data << 8), AVR8_REG_B);
		break;

	case AVR8_REGIDX_OCR1CL:
		LOGMASKED(LOG_TIMER1, "%s: OCR1CL = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1C & 0xff00) | data, AVR8_REG_C);
		break;

	case AVR8_REGIDX_OCR1CH:
		LOGMASKED(LOG_TIMER1, "%s: OCR1CH = %02x\n", machine().describe_context(), data);
		update_ocr1((AVR8_OCR1C & 0x00ff) | (data << 8), AVR8_REG_C);
		break;

	case AVR8_REGIDX_TCCR2A:
		LOGMASKED(LOG_TIMER2, "%s: TCCR2A = %02x\n", machine().describe_context(), data);
		changed_tccr2a(data);
		break;

	case AVR8_REGIDX_TCCR2B:
		LOGMASKED(LOG_TIMER2, "%s: TCCR2B = %02x\n", machine().describe_context(), data);
		changed_tccr2b(data);
		break;

	case AVR8_REGIDX_TCNT2:
		AVR8_TCNT2 = data;
		break;

	case AVR8_REGIDX_OCR2A:
		update_ocr2(data, AVR8_REG_A);
		break;

	case AVR8_REGIDX_OCR2B:
		update_ocr2(data, AVR8_REG_B);
		break;

	case AVR8_REGIDX_TCCR3A:
		LOGMASKED(LOG_TIMER3, "%s: TCCR3A = %02x\n", machine().describe_context(), data);
		changed_tccr3a(data);
		break;

	case AVR8_REGIDX_TCCR3B:
		LOGMASKED(LOG_TIMER3, "%s: TCCR3B = %02x\n", machine().describe_context(), data);
		changed_tccr3b(data);
		break;

	case AVR8_REGIDX_TCCR3C:
		LOGMASKED(LOG_TIMER3, "%s: TCCR3C = %02x\n", machine().describe_context(), data);
		changed_tccr3c(data);
		break;

	case AVR8_REGIDX_TCNT3L:
		AVR8_TCNT3L = data;
		break;

	case AVR8_REGIDX_TCNT3H:
		AVR8_TCNT3H = data;
		break;

	case AVR8_REGIDX_ICR3L:
		AVR8_ICR3L = data;
		break;

	case AVR8_REGIDX_ICR3H:
		AVR8_ICR3H = data;
		break;

	case AVR8_REGIDX_OCR3AL:
		AVR8_OCR3AL = data;
		break;

	case AVR8_REGIDX_OCR3AH:
		AVR8_OCR3AH = data;
		break;

	case AVR8_REGIDX_OCR3BL:
		AVR8_OCR3BL = data;
		break;

	case AVR8_REGIDX_OCR3BH:
		AVR8_OCR3BH = data;
		break;

	case AVR8_REGIDX_OCR3CL:
		AVR8_OCR3CL = data;
		break;

	case AVR8_REGIDX_OCR3CH:
		AVR8_OCR3CH = data;
		break;

	case AVR8_REGIDX_TCCR4A:
		LOGMASKED(LOG_TIMER4, "%s: TCCR4A = %02x\n", machine().describe_context(), data);
		changed_tccr4a(data);
		break;

	case AVR8_REGIDX_TCCR4B:
		LOGMASKED(LOG_TIMER4, "%s: TCCR4B = %02x\n", machine().describe_context(), data);
		changed_tccr4b(data);
		break;

	case AVR8_REGIDX_TCCR4C:
		LOGMASKED(LOG_TIMER4, "%s: TCCR4C = %02x\n", machine().describe_context(), data);
		changed_tccr4c(data);
		break;

	case AVR8_REGIDX_TCNT4L:
		AVR8_TCNT4L = data;
		break;

	case AVR8_REGIDX_TCNT4H:
		AVR8_TCNT4H = data;
		break;

	case AVR8_REGIDX_ICR4L:
		AVR8_ICR4L = data;
		break;

	case AVR8_REGIDX_ICR4H:
		AVR8_ICR4H = data;
		break;

	case AVR8_REGIDX_OCR4AL:
		AVR8_OCR4AL = data;
		break;

	case AVR8_REGIDX_OCR4AH:
		AVR8_OCR4AH = data;
		break;

	case AVR8_REGIDX_OCR4BL:
		AVR8_OCR4BL = data;
		break;

	case AVR8_REGIDX_OCR4BH:
		AVR8_OCR4BH = data;
		break;

	case AVR8_REGIDX_OCR4CL:
		AVR8_OCR4CL = data;
		break;

	case AVR8_REGIDX_OCR4CH:
		AVR8_OCR4CH = data;
		break;

	case AVR8_REGIDX_TCCR5A:
		LOGMASKED(LOG_TIMER5, "%s: TCCR5A = %02x\n", machine().describe_context(), data);
		changed_tccr5a(data);
		break;

	case AVR8_REGIDX_TCCR5B:
		LOGMASKED(LOG_TIMER5, "%s: TCCR5B = %02x\n", machine().describe_context(), data);
		changed_tccr5b(data);
		break;

	case AVR8_REGIDX_ASSR:
		LOGMASKED(LOG_ASYNC, "%s: (not yet implemented) ASSR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TWBR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWBR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TWSR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWSR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TWAR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TWDR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWDR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_TWCR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWCR = %02x\n", machine().describe_context(), data);
		m_r[AVR8_REGIDX_TWCR] = data;
		break;

	case AVR8_REGIDX_TWAMR:
		LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAMR = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_UCSR0A:
		LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0A = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_UCSR0B:
		LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0B = %02x\n", machine().describe_context(), data);
		break;

	case AVR8_REGIDX_UCSR0C:
		LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0C = %02x\n", machine().describe_context(), data);
		break;

	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown Register Write: %03x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

uint8_t avr8_device::regs_r(offs_t offset)
{
	uint8_t data = m_r[offset];

	switch (offset)
	{
	case AVR8_REGIDX_R0:
	case AVR8_REGIDX_R1:
	case AVR8_REGIDX_R2:
	case AVR8_REGIDX_R3:
	case AVR8_REGIDX_R4:
	case AVR8_REGIDX_R5:
	case AVR8_REGIDX_R6:
	case AVR8_REGIDX_R7:
	case AVR8_REGIDX_R8:
	case AVR8_REGIDX_R9:
	case AVR8_REGIDX_R10:
	case AVR8_REGIDX_R11:
	case AVR8_REGIDX_R12:
	case AVR8_REGIDX_R13:
	case AVR8_REGIDX_R14:
	case AVR8_REGIDX_R15:
	case AVR8_REGIDX_R16:
	case AVR8_REGIDX_R17:
	case AVR8_REGIDX_R18:
	case AVR8_REGIDX_R19:
	case AVR8_REGIDX_R20:
	case AVR8_REGIDX_R21:
	case AVR8_REGIDX_R22:
	case AVR8_REGIDX_R23:
	case AVR8_REGIDX_R24:
	case AVR8_REGIDX_R25:
	case AVR8_REGIDX_R26:
	case AVR8_REGIDX_R27:
	case AVR8_REGIDX_R28:
	case AVR8_REGIDX_R29:
	case AVR8_REGIDX_R30:
	case AVR8_REGIDX_R31:
		return data;

	case AVR8_REGIDX_PINA:
		// TODO: account for DDRA
		return read_gpio(AVR8_IO_PORTA);

	case AVR8_REGIDX_PINB:
		// TODO: account for DDRB
		return read_gpio(AVR8_IO_PORTB);

	case AVR8_REGIDX_PINC:
		// TODO: account for DDRC
		return read_gpio(AVR8_IO_PORTC);

	case AVR8_REGIDX_PIND:
		// TODO: account for DDRD
		return read_gpio(AVR8_IO_PORTD);

	case AVR8_REGIDX_PINE:
		// TODO: account for DDRE
		return read_gpio(AVR8_IO_PORTE);

	case AVR8_REGIDX_PINF:
		// TODO: account for DDRF
		return read_gpio(AVR8_IO_PORTF);

	case AVR8_REGIDX_PING:
		// TODO: account for DDRG
		return read_gpio(AVR8_IO_PORTG);

	case AVR8_REGIDX_PINH:
		// TODO: account for DDRH
		return read_gpio(AVR8_IO_PORTH);

	case AVR8_REGIDX_PINJ:
		// TODO: account for DDRJ
		return read_gpio(AVR8_IO_PORTJ);

	case AVR8_REGIDX_PINK:
		// TODO: account for DDRK
		return read_gpio(AVR8_IO_PORTK);

	case AVR8_REGIDX_PINL:
		// TODO: account for DDRL
		return read_gpio(AVR8_IO_PORTL);

	case AVR8_REGIDX_PORTA:
	case AVR8_REGIDX_PORTB:
	case AVR8_REGIDX_PORTC:
	case AVR8_REGIDX_PORTD:
	case AVR8_REGIDX_PORTE:
	case AVR8_REGIDX_PORTF:
	case AVR8_REGIDX_PORTG:
	case AVR8_REGIDX_PORTH:
	case AVR8_REGIDX_PORTJ:
	case AVR8_REGIDX_PORTK:
	case AVR8_REGIDX_PORTL:
		return data;

	case AVR8_REGIDX_DDRA:
	case AVR8_REGIDX_DDRB:
	case AVR8_REGIDX_DDRC:
	case AVR8_REGIDX_DDRD:
	case AVR8_REGIDX_DDRE:
	case AVR8_REGIDX_DDRF:
	case AVR8_REGIDX_DDRG:
	case AVR8_REGIDX_DDRH:
	case AVR8_REGIDX_DDRJ:
	case AVR8_REGIDX_DDRK:
	case AVR8_REGIDX_DDRL:
		return data;

	// EEPROM registers
	case AVR8_REGIDX_EECR:
	case AVR8_REGIDX_EEDR:
		return data;

	// Miscellaneous registers
	// TODO: Implement readback for all applicable registers.

	case AVR8_REGIDX_GPIOR0:
		LOGMASKED(LOG_GPIO, "%s: GPIOR0 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_GPIOR1:
		LOGMASKED(LOG_GPIO, "%s: GPIOR1 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_GPIOR2:
		LOGMASKED(LOG_GPIO, "%s: GPIOR2 Read: %02x\n", machine().describe_context(), data);
		return data;
//    case AVR8_REGIDX_UCSR0B:   // TODO: needed for Replicator 1
	case AVR8_REGIDX_SPDR:   // TODO: needed for Replicator 1
	case AVR8_REGIDX_SPSR:   // TODO: needed for Replicator 1
//    case AVR8_REGIDX_ADCSRA:   // TODO: needed for Replicator 1
//    case AVR8_REGIDX_ADCSRB:   // TODO: needed for Replicator 1
	case AVR8_REGIDX_SPL:
	case AVR8_REGIDX_SPH:
	case AVR8_REGIDX_SREG:
	case AVR8_REGIDX_TIMSK0:
		LOGMASKED(LOG_TIMER0, "%s: TIMSK0 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIMSK1:
		LOGMASKED(LOG_TIMER1, "%s: TIMSK1 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIMSK2:
		LOGMASKED(LOG_TIMER2, "%s: TIMSK2 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIMSK3:
		LOGMASKED(LOG_TIMER3, "%s: TIMSK3 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIMSK4:
		LOGMASKED(LOG_TIMER4, "%s: TIMSK4 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIMSK5:
		LOGMASKED(LOG_TIMER5, "%s: TIMSK5 Read: %02x\n", machine().describe_context(), data);
		return data;
	case AVR8_REGIDX_TIFR1:
		//LOGMASKED(LOG_TIMER0, "%s: TIFR1 Read: %02x\n", machine().describe_context(), data);
		return data;

	// Two-wire registers
	case AVR8_REGIDX_TWCR:
		// TODO: needed for Replicator 1
		return m_r[offset];

	case AVR8_REGIDX_TWSR:
		// HACK for Replicator 1:
		//   By returning a value != 0x08, we induce an error state that makes the object code jump out of the wait loop,
		//   and continue execution failing the 2-wire write operation.
		return 0x00;


	case AVR8_REGIDX_TCNT1L:
		return (uint8_t)m_timer1_count;
	case AVR8_REGIDX_TCNT1H:
		return (uint8_t)(m_timer1_count >> 8);
	case AVR8_REGIDX_TCNT2:
	case AVR8_REGIDX_UCSR0A:
		return m_r[offset];

	default:
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown Register Read: %03X\n", machine().describe_context(), offset);
		return 0;
	}
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t avr8_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t avr8_device::execute_max_cycles() const noexcept
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

uint32_t avr8_device::execute_input_lines() const noexcept
{
	return 0;
}


void avr8_device::execute_set_input(int inputnum, int state)
{
}

#include "avr8ops.hxx"

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void avr8_device::execute_run()
{
	while (m_icount > 0)
	{
		m_pc &= m_addr_mask;
		m_shifted_pc &= (m_addr_mask << 1) | 1;

		debugger_instruction_hook(m_shifted_pc);

		const uint16_t op = (uint32_t)m_program->read_word(m_shifted_pc);
		m_opcycles = m_op_cycles[op];
		((this)->*(m_op_funcs[op]))(op);
		m_pc++;

		m_shifted_pc = m_pc << 1;

		m_icount -= m_opcycles;

		timer_tick();
	}
}
