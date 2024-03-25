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

      Unimplemented opcodes: SPM, SPM Z+, SLEEP, BREAK, WDR, EICALL

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

#define LOG_UNKNOWN         (1U << 1)
#define LOG_BOOT            (1U << 2)
#define LOG_TIMER0          (1U << 3)
#define LOG_TIMER1          (1U << 4)
#define LOG_TIMER2          (1U << 5)
#define LOG_TIMER3          (1U << 6)
#define LOG_TIMER4          (1U << 7)
#define LOG_TIMER5          (1U << 8)
#define LOG_TIMER0_TICK     (1U << 9)
#define LOG_TIMER1_TICK     (1U << 10)
#define LOG_TIMER2_TICK     (1U << 11)
#define LOG_TIMER3_TICK     (1U << 12)
#define LOG_TIMER4_TICK     (1U << 13)
#define LOG_TIMER5_TICK     (1U << 14)
#define LOG_EEPROM          (1U << 15)
#define LOG_GPIO            (1U << 16)
#define LOG_WDOG            (1U << 17)
#define LOG_CLOCK           (1U << 18)
#define LOG_POWER           (1U << 19)
#define LOG_OSC             (1U << 20)
#define LOG_PINCHG          (1U << 21)
#define LOG_EXTMEM          (1U << 22)
#define LOG_ADC             (1U << 23)
#define LOG_DIGINPUT        (1U << 24)
#define LOG_ASYNC           (1U << 25)
#define LOG_TWI             (1U << 26)
#define LOG_UART            (1U << 27)
#define LOG_TIMERS          (LOG_TIMER0 | LOG_TIMER1 | LOG_TIMER2 | LOG_TIMER3 | LOG_TIMER4 | LOG_TIMER5)
#define LOG_TIMER_TICKS     (LOG_TIMER0_TICK | LOG_TIMER1_TICK | LOG_TIMER2_TICK | LOG_TIMER3_TICK | LOG_TIMER4_TICK | LOG_TIMER5_TICK)
#define LOG_ALL             (LOG_UNKNOWN | LOG_BOOT | LOG_TIMERS | LOG_EEPROM | LOG_GPIO | LOG_WDOG | LOG_CLOCK | LOG_POWER \
							 | LOG_OSC | LOG_PINCHG | LOG_EXTMEM | LOG_ADC | LOG_DIGINPUT | LOG_ASYNC | LOG_TWI | LOG_UART)

#define VERBOSE             (0)
//#define LOG_OUTPUT_FUNC     osd_printf_info
#include "logmacro.h"


//**************************************************************************
//  ENUMS AND MACROS
//**************************************************************************

enum
{
	SREG_C = 0,
	SREG_Z,
	SREG_N,
	SREG_V,
	SREG_S,
	SREG_H,
	SREG_T,
	SREG_I,

	SREG_MASK_C = 0x01,
	SREG_MASK_Z = 0x02,
	SREG_MASK_N = 0x04,
	SREG_MASK_V = 0x08,
	SREG_MASK_S = 0x10,
	SREG_MASK_H = 0x20,
	SREG_MASK_T = 0x40,
	SREG_MASK_I = 0x80
};

// I/O Enums
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
#define SPREG           ((m_r[SPH] << 8) | m_r[SPL])

// I/O Defines
#define TCCR0B_CS_SHIFT         0
#define TCCR0B_CS_MASK          0x07
#define TCCR0B_WGM0_2_SHIFT     3
#define TCCR0B_WGM0_2_MASK      0x08
#define TCCR0B_FOC0B_SHIFT      6
#define TCCR0B_FOC0B_MASK       0x40
#define TCCR0B_FOC0A_SHIFT      7
#define TCCR0B_FOC0A_MASK       0x80
#define TIMER0_CLOCK_SELECT     (m_r[TCCR0B] & TCCR0B_CS_MASK)

#define TCCR0A_WGM0_10_SHIFT    0
#define TCCR0A_WGM0_10_MASK     0x03
#define TCCR0A_COM0B_SHIFT      4
#define TCCR0A_COM0B_MASK       0x30
#define TCCR0A_COM0A_SHIFT      6
#define TCCR0A_COM0A_MASK       0xc0
#define TCCR0A_COM0A            ((m_r[TCCR0A] & TCCR0A_COM0A_MASK) >> TCCR0A_COM0A_SHIFT)
#define TCCR0A_COM0B            ((m_r[TCCR0A] & TCCR0A_COM0B_MASK) >> TCCR0A_COM0B_SHIFT)
#define TCCR0A_WGM0_10          (m_r[TCCR0A] & TCCR0A_WGM0_10_MASK)

#define TIMSK0_TOIE0_BIT    0
#define TIMSK0_OCIE0A_BIT   1
#define TIMSK0_OCIE0B_BIT   2
#define TIMSK0_TOIE0_MASK   (1 << TIMSK0_TOIE0_BIT)
#define TIMSK0_OCIE0A_MASK  (1 << TIMSK0_OCIE0A_BIT)
#define TIMSK0_OCIE0B_MASK  (1 << TIMSK0_OCIE0B_BIT)
#define TIMSK0_TOIE0        (BIT(m_r[TIMSK0], TIMSK0_TOIE0_BIT))
#define TIMSK0_OCIE0A       (BIT(m_r[TIMSK0], TIMSK0_OCIE0A_BIT))
#define TIMSK0_OCIE0B       (BIT(m_r[TIMSK0], TIMSK0_OCIE0B_BIT))

#define TIFR0_TOV0_SHIFT    0
#define TIFR0_TOV0_MASK     0x01
#define TIFR0_OCF0A_SHIFT   1
#define TIFR0_OCF0A_MASK    0x02
#define TIFR0_OCF0B_SHIFT   2
#define TIFR0_OCF0B_MASK    0x04
#define TIFR0_MASK          (TIFR0_TOV0_MASK | TIFR0_OCF0B_MASK | TIFR0_OCF0A_MASK)

#define TCCR1B_CS_SHIFT         0
#define TCCR1B_CS_MASK          0x07
#define TCCR1B_WGM1_32_SHIFT    3
#define TCCR1B_WGM1_32_MASK     0x18
#define TCCR1B_ICES1_SHIFT      6
#define TCCR1B_ICES1_MASK       0x40
#define TCCR1B_ICNC1_SHIFT      7
#define TCCR1B_ICNC1_MASK       0x80
#define TIMER1_CLOCK_SELECT     (m_r[TCCR1B] & TCCR1B_CS_MASK)

#define TCCR1A_WGM1_10_SHIFT    0
#define TCCR1A_WGM1_10_MASK     0x03
#define TCCR1A_COM1AB_SHIFT     4
#define TCCR1A_COM1AB_MASK      0xf0
#define TCCR1A_COM1B_SHIFT      4
#define TCCR1A_COM1B_MASK       0x30
#define TCCR1A_COM1A_SHIFT      6
#define TCCR1A_COM1A_MASK       0xc0
#define TCCR1A_COM1A            ((m_r[TCCR1A] & TCCR1A_COM1A_MASK) >> TCCR1A_COM1A_SHIFT)
#define TCCR1A_COM1B            ((m_r[TCCR1A] & TCCR1A_COM1B_MASK) >> TCCR1A_COM1B_SHIFT)
#define TCCR1A_WGM1_10          (m_r[TCCR1A] & TCCR1A_WGM1_10_MASK)

#define TIMSK1_TOIE1_BIT    0
#define TIMSK1_OCIE1A_BIT   1
#define TIMSK1_OCIE1B_BIT   2
#define TIMSK1_ICIE1_BIT    5
#define TIMSK1_TOIE1_MASK   (1 << TIMSK1_TOIE1_BIT)
#define TIMSK1_OCIE1A_MASK  (1 << TIMSK1_OCIE1A_BIT)
#define TIMSK1_OCIE1B_MASK  (1 << TIMSK1_OCIE1B_BIT)
#define TIMSK1_ICIE1_MASK   (1 << TIMSK1_ICIE1_BIT)
#define TIMSK1_TOIE1        (BIT(m_r[TIMSK1], TIMSK1_TOIE1_BIT))
#define TIMSK1_OCIE1A       (BIT(m_r[TIMSK1], TIMSK1_OCIE1A_BIT))
#define TIMSK1_OCIE1B       (BIT(m_r[TIMSK1], TIMSK1_OCIE1B_BIT))
#define TIMSK1_ICIE1        (BIT(m_r[TIMSK1], TIMSK1_ICIE1_BIT))

#define TIFR1_TOV1_SHIFT    0
#define TIFR1_TOV1_MASK     0x01
#define TIFR1_OCF1A_SHIFT   1
#define TIFR1_OCF1A_MASK    0x02
#define TIFR1_OCF1B_SHIFT   2
#define TIFR1_OCF1B_MASK    0x04
#define TIFR1_ICF1_SHIFT    5
#define TIFR1_ICF1_MASK     0x20
#define TIFR1_MASK          (TIFR1_ICF1_MASK | TIFR1_TOV1_MASK | TIFR1_OCF1B_MASK | TIFR1_OCF1A_MASK)

#define TCCR2B_CS_SHIFT         0
#define TCCR2B_CS_MASK          0x07
#define TCCR2B_WGM2_2_SHIFT     3
#define TCCR2B_WGM2_2_MASK      0x08
#define TCCR2B_FOC2B_SHIFT      6
#define TCCR2B_FOC2B_MASK       0x40
#define TCCR2B_FOC2A_SHIFT      7
#define TCCR2B_FOC2A_MASK       0x80
#define TIMER2_CLOCK_SELECT     (m_r[TCCR2B] & TCCR2B_CS_MASK)

#define TCCR2A_WGM2_10_SHIFT    0
#define TCCR2A_WGM2_10_MASK     0x03
#define TCCR2A_COM2B_SHIFT      4
#define TCCR2A_COM2B_MASK       0x30
#define TCCR2A_COM2A_SHIFT      6
#define TCCR2A_COM2A_MASK       0xc0
#define TCCR2A_COM2A            ((m_r[TCCR2A] & TCCR2A_COM2A_MASK) >> TCCR2A_COM2A_SHIFT)
#define TCCR2A_COM2B            ((m_r[TCCR2A] & TCCR2A_COM2B_MASK) >> TCCR2A_COM2B_SHIFT)
#define TCCR2A_WGM2_10          (m_r[TCCR2A] & TCCR2A_WGM2_10_MASK)

#define TIMSK2_TOIE2_BIT    0
#define TIMSK2_OCIE2A_BIT   1
#define TIMSK2_OCIE2B_BIT   2
#define TIMSK2_TOIE2_MASK   (1 << TIMSK2_TOIE2_BIT)
#define TIMSK2_OCIE2A_MASK  (1 << TIMSK2_OCIE2A_BIT)
#define TIMSK2_OCIE2B_MASK  (1 << TIMSK2_OCIE2B_BIT)
#define TIMSK2_TOIE2        (BIT(m_r[TIMSK2], TIMSK2_TOIE2_BIT))
#define TIMSK2_OCIE2A       (BIT(m_r[TIMSK2], TIMSK2_OCIE2A_BIT))
#define TIMSK2_OCIE2B       (BIT(m_r[TIMSK2], TIMSK2_OCIE2B_BIT))

#define TIFR2_TOV2_SHIFT    0
#define TIFR2_TOV2_MASK     0x01
#define TIFR2_OCF2A_SHIFT   1
#define TIFR2_OCF2A_MASK    0x02
#define TIFR2_OCF2B_SHIFT   2
#define TIFR2_OCF2B_MASK    0x04
#define TIFR2_MASK          (TIFR2_TOV2_MASK | TIFR2_OCF2B_MASK | TIFR2_OCF2A_MASK)

#define TIMSK3_TOIE3_BIT    0
#define TIMSK3_OCIE3A_BIT   1
#define TIMSK3_OCIE3B_BIT   2
#define TIMSK3_OCIE3C_BIT   3
#define TIMSK3_TOIE3        (BIT(m_r[TIMSK3], TIMSK3_TOIE3_BIT))
#define TIMSK3_OCIE3A       (BIT(m_r[TIMSK3], TIMSK3_OCIE3A_BIT))
#define TIMSK3_OCIE3B       (BIT(m_r[TIMSK3], TIMSK3_OCIE3B_BIT))
#define TIMSK3_OCIE3C       (BIT(m_r[TIMSK3], TIMSK3_OCIE3C_BIT))

#define TCCR4B_CS_SHIFT         0
#define TCCR4B_CS_MASK          0x07
#define TCCR4B_WGM4_32_SHIFT    3
#define TCCR4B_WGM4_32_MASK     0x18
#define TCCR4B_FOC4C_SHIFT      5
#define TCCR4B_FOC4C_MASK       0x20
#define TCCR4B_FOC4B_SHIFT      6
#define TCCR4B_FOC4B_MASK       0x40
#define TCCR4B_FOC4A_SHIFT      7
#define TCCR4B_FOC4A_MASK       0x80
#define TIMER4_CLOCK_SELECT     ((m_r[TCCR4B] & TCCR4B_CS_MASK) >> TCCR4B_CS_SHIFT)

#define TCCR4A_WGM4_10_SHIFT    0
#define TCCR4A_WGM4_10_MASK     0x03
#define TCCR4A_COM4C_SHIFT      2
#define TCCR4A_COM4C_MASK       0x0c
#define TCCR4A_COM4B_SHIFT      4
#define TCCR4A_COM4B_MASK       0x30
#define TCCR4A_COM4A_SHIFT      6
#define TCCR4A_COM4A_MASK       0xc0
#define TCCR4A_COM4A            ((m_r[TCCR4A] & TCCR4A_COM4A_MASK) >> TCCR4A_COM4A_SHIFT)
#define TCCR4A_COM4B            ((m_r[TCCR4A] & TCCR4A_COM4B_MASK) >> TCCR4A_COM4B_SHIFT)
#define TCCR4A_COM4C            ((m_r[TCCR4A] & TCCR4A_COM4C_MASK) >> TCCR4A_COM4C_SHIFT)
#define TCCR4A_WGM2_10          (m_r[TCCR4A] & TCCR4A_WGM2_10_MASK)

#define WGM4_32     ((m_r[TCCR4B] & TCCR4B_WGM4_32_MASK) >> TCCR4B_WGM4_32_SHIFT)
#define WGM4_10     ((m_r[TCCR4A] & TCCR4A_WGM4_10_MASK) >> TCCR4A_WGM4_10_SHIFT)
#define WGM4        ((WGM4_32 << 2) | WGM4_10)

#define TIMSK4_TOIE4_BIT    0
#define TIMSK4_OCIE4A_BIT   1
#define TIMSK4_OCIE4B_BIT   2
#define TIMSK4_TOIE4        (BIT(m_r[TIMSK4], TIMSK4_TOIE4_BIT))
#define TIMSK4_OCIE4A       (BIT(m_r[TIMSK4], TIMSK4_OCIE4A_BIT))
#define TIMSK4_OCIE4B       (BIT(m_r[TIMSK4], TIMSK4_OCIE4B_BIT))

#define TIFR4_TOV4_SHIFT    0
#define TIFR4_TOV4_MASK     0x01
#define TIFR4_OCF4A_SHIFT   1
#define TIFR4_OCF4A_MASK    0x02
#define TIFR4_OCF4B_SHIFT   2
#define TIFR4_OCF4B_MASK    0x04
#define TIFR4_MASK          (TIFR4_TOV4_MASK | TIFR4_OCF4B_MASK | TIFR4_OCF4A_MASK)

#define TCCR5C_FOC5C_SHIFT  5
#define TCCR5C_FOC5C_MASK   0x20
#define TCCR5C_FOC5B_SHIFT  6
#define TCCR5C_FOC5B_MASK   0x40
#define TCCR5C_FOC5A_SHIFT  7
#define TCCR5C_FOC5A_MASK   0x80

#define TCCR5B_CS_SHIFT         0
#define TCCR5B_CS_MASK          0x07
#define TCCR5B_WGM5_32_SHIFT    3
#define TCCR5B_WGM5_32_MASK     0x18
#define TCCR5B_ICES5_SHIFT      6
#define TCCR5B_ICES5_MASK       0x40
#define TCCR5B_ICNC5_SHIFT      7
#define TCCR5B_ICNC5_MASK       0x80
#define TIMER5_CLOCK_SELECT     ((m_r[TCCR5B] & TCCR5B_CS_MASK) >> TCCR5B_CS_SHIFT)

#define TCCR5A_WGM5_10_SHIFT    0
#define TCCR5A_WGM5_10_MASK     0x03
#define TCCR5A_COM5C_SHIFT      2
#define TCCR5A_COM5C_MASK       0x0c
#define TCCR5A_COM5B_SHIFT      4
#define TCCR5A_COM5B_MASK       0x30
#define TCCR5A_COM5A_SHIFT      6
#define TCCR5A_COM5A_MASK       0xc0
#define TCCR5A_COM5A            ((m_r[TCCR5A] & TCCR5A_COM5A_MASK) >> TCCR5A_COM5A_SHIFT)
#define TCCR5A_COM5B            ((m_r[TCCR5A] & TCCR5A_COM5B_MASK) >> TCCR5A_COM5B_SHIFT)
#define TCCR5A_COM5C            ((m_r[TCCR5A] & TCCR5A_COM5C_MASK) >> TCCR5A_COM5C_SHIFT)
#define TCCR5A_WGM5_10          (m_r[TCCR5A] & TCCR5A_WGM5_10_MASK)

#define WGM5_32     ((m_r[TCCR5B] & TCCR5B_WGM5_32_MASK) >> TCCR5B_WGM5_32_SHIFT)
#define WGM5_10     ((m_r[TCCR5A] & TCCR5A_WGM5_10_MASK) >> TCCR5A_WGM5_10_SHIFT)
#define WGM5        ((WGM5_32 << 2) | WGM5_10)

#define TIMSK5_TOIE5_BIT    0
#define TIMSK5_OCIE5A_BIT   1
#define TIMSK5_OCIE5B_BIT   2
#define TIMSK5_OCIE5C_BIT   3
#define TIMSK5_ICIE5_BIT    5
#define TIMSK5_TOIE5_MASK   (1 << TIMSK5_TOIE5_BIT)
#define TIMSK5_OCIE5A_MASK  (1 << TIMSK5_OCIE5A_BIT)
#define TIMSK5_OCIE5B_MASK  (1 << TIMSK5_OCIE5B_BIT)
#define TIMSK5_OCIE5C_MASK  (1 << TIMSK5_OCIE5C_BIT)
#define TIMSK5_ICIE5_MASK   (1 << TIMSK5_ICIE5_BIT)
#define TIMSK5_TOIE5        (BIT(m_r[TIMSK5], TIMSK5_TOIE5_BIT))
#define TIMSK5_OCIE5A       (BIT(m_r[TIMSK5], TIMSK5_OCIE5A_BIT))
#define TIMSK5_OCIE5B       (BIT(m_r[TIMSK5], TIMSK5_OCIE5B_BIT))
#define TIMSK5_OCIE5C       (BIT(m_r[TIMSK5], TIMSK5_OCIE5C_BIT))
#define TIMSK5_ICIE5C       (BIT(m_r[TIMSK5], TIMSK5_ICIE5C_BIT))

#define TIFR5_ICF5_MASK     0x20
#define TIFR5_ICF5_SHIFT    5
#define TIFR5_OCF5C_MASK    0x08
#define TIFR5_OCF5C_SHIFT   3
#define TIFR5_OCF5B_MASK    0x04
#define TIFR5_OCF5B_SHIFT   2
#define TIFR5_OCF5A_MASK    0x02
#define TIFR5_OCF5A_SHIFT   1
#define TIFR5_TOV5_MASK     0x01
#define TIFR5_TOV5_SHIFT    0
#define TIFR5_MASK          (TIFR5_ICF5_MASK | TIFR5_OCF5C_MASK | TIFR5_OCF5B_MASK | TIFR5_OCF5A_MASK | TIFR5_TOV5_MASK)
#define TIFR5_ICF5          ((m_r[TIFR5] & TIFR5_ICF5_MASK) >> TIFR5_ICF5_SHIFT)
#define TIFR5_OCF5C         ((m_r[TIFR5] & TIFR5_OCF5C_MASK) >> TIFR5_OCF5C_SHIFT)
#define TIFR5_OCF5B         ((m_r[TIFR5] & TIFR5_OCF5B_MASK) >> TIFR5_OCF5B_SHIFT)
#define TIFR5_OCF5A         ((m_r[TIFR5] & TIFR5_OCF5A_MASK) >> TIFR5_OCF5A_SHIFT)
#define TIFR5_TOV5          ((m_r[TIFR5] & TIFR5_TOV5_MASK) >> TIFR5_TOV5_SHIFT)

//---------------------------------------------------------------

#define WGM0                (((m_r[TCCR0B] & 0x08) >> 1) | (m_r[TCCR0A] & 0x03))

#define OCR1A               ((m_r[OCR1AH] << 8) | m_r[OCR1AL])
#define OCR1B               ((m_r[OCR1BH] << 8) | m_r[OCR1BL])
#define OCR1C               ((m_r[OCR1CH] << 8) | m_r[OCR1CL])
#define ICR1                ((m_r[ICR1H]  << 8) | m_r[ICR1L])
#define WGM1                (((m_r[TCCR1B] & 0x18) >> 1) | (m_r[TCCR1A] & 0x03))

#define WGM2                (((m_r[TCCR2B] & 0x08) >> 1) | (m_r[TCCR2A] & 0x03))

#define ICR3                ((m_r[ICR3H]  << 8) | m_r[ICR3L])
#define OCR3A               ((m_r[OCR3AH] << 8) | m_r[OCR3AL])

#define ICR4                ((m_r[ICR4H]  << 8) | m_r[ICR4L])
#define OCR4A               ((m_r[OCR4AH] << 8) | m_r[OCR4AL])

#define ICR5                ((m_r[ICR5H]  << 8) | m_r[ICR5L])
#define OCR5A               ((m_r[OCR5AH] << 8) | m_r[OCR5AL])

#define GTCCR_PSRASY_MASK   0x02
#define GTCCR_PSRASY_SHIFT  1

#define SPSR_SPR2X          (m_r[SPSR] & SPSR_SPR2X_MASK)

#define SPCR_SPIE           ((m_r[SPCR] & SPCR_SPIE_MASK) >> 7)
#define SPCR_SPE            ((m_r[SPCR] & SPCR_SPE_MASK) >> 6)
#define SPCR_DORD           ((m_r[SPCR] & SPCR_DORD_MASK) >> 5)
#define SPCR_MSTR           ((m_r[SPCR] & SPCR_MSTR_MASK) >> 4)
#define SPCR_CPOL           ((m_r[SPCR] & SPCR_CPOL_MASK) >> 3)
#define SPCR_CPHA           ((m_r[SPCR] & SPCR_CPHA_MASK) >> 2)
#define SPCR_SPR            (m_r[SPCR] & SPCR_SPR_MASK)

#define SPI_RATE            ((SPSR_SPR2X << 2) | SPCR_SPR)

#define PORTB_MOSI          0x08

#define EECR_MASK           0x3f
#define EECR_EERE_BIT       0
#define EECR_EEPE_BIT       1
#define EECR_EEMPE_BIT      2
#define EECR_EERIE_BIT      3
#define EECR_EEPM_BIT       4
#define EECR_EERE_MASK      (1 << EECR_EERE_BIT)
#define EECR_EEPE_MASK      (1 << EECR_EEPE_BIT)
#define EECR_EEMPE_MASK     (1 << EECR_EEMPE_BIT)
#define EECR_EERIE_MASK     (1 << EECR_EERIE_BIT)
#define EECR_EEPM_MASK      (3 << EECR_EEPM_BIT)
#define EECR_EERE           (BIT(m_r[EECR], EECR_EERE_BIT))
#define EECR_EEPE           (BIT(m_r[EECR], EECR_EEPE_BIT))
#define EECR_EEMPE          (BIT(m_r[EECR], EECR_EEMPE_BIT))
#define EECR_EERIE          (BIT(m_r[EECR], EECR_EERIE_BIT))
#define EECR_EEPM           ((m_r[EECR] & EECR_EEPM_MASK) >> EECR_EEPM_BIT)

//---------------------------------------------------------------

#define ADMUX_MUX_MASK      0x0f
#define ADMUX_ADLAR_MASK    0x20
#define ADMUX_REFS_MASK     0xc0
#define ADMUX_MUX           ((m_r[ADMUX] & ADMUX_MUX_MASK) >> 0)
#define ADMUX_ADLAR         ((m_r[ADMUX] & ADMUX_ADLAR_MASK) >> 5)
#define ADMUX_REFS          ((m_r[ADMUX] & ADMUX_REFS_MASK) >> 6)

#define ADCSRB_ACME_MASK    0x40
#define ADCSRB_ADTS_MASK    0x07
#define ADCSRB_ACME         ((m_r[ADCSRB] & ADCSRB_ACME_MASK) >> 6)
#define ADCSRB_ADTS         ((m_r[ADCSRB] & ADCSRB_ADTS_MASK) >> 0)

#define ADCSRA_ADPS_MASK    0x07
#define ADCSRA_ADIE_MASK    0x08
#define ADCSRA_ADIF_MASK    0x10
#define ADCSRA_ADATE_MASK   0x20
#define ADCSRA_ADSC_MASK    0x40
#define ADCSRA_ADEN_MASK    0x80
#define ADCSRA_ADPS         (m_r[ADCSRA] & ADCSRA_ADPS_MASK)
#define ADCSRA_ADIE         ((m_r[ADCSRA] & ADCSRA_ADIE_MASK) >> 3)
#define ADCSRA_ADIF         ((m_r[ADCSRA] & ADCSRA_ADIF_MASK) >> 4)
#define ADCSRA_ADATE        ((m_r[ADCSRA] & ADCSRA_ADATE_MASK) >> 5)
#define ADCSRA_ADSC         ((m_r[ADCSRA] & ADCSRA_ADSC_MASK) >> 6)
#define ADCSRA_ADEN         ((m_r[ADCSRA] & ADCSRA_ADEN_MASK) >> 7)


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

template <int NumTimers>
void avr8_device<NumTimers>::base_internal_map(address_map &map)
{
	map(0x0000, 0x01ff).ram().share(m_r);
	map(0x0020, 0x0020).r(FUNC(avr8_device::pin_r<0>));
	map(0x0022, 0x0022).rw(FUNC(avr8_device::gpio_r<GPIOA>), FUNC(avr8_device::port_w<GPIOA>));
	map(0x0023, 0x0023).r(FUNC(avr8_device::pin_r<1>));
	map(0x0025, 0x0025).rw(FUNC(avr8_device::gpio_r<GPIOB>), FUNC(avr8_device::port_w<GPIOB>));
	map(0x0026, 0x0026).r(FUNC(avr8_device::pin_r<2>));
	map(0x0028, 0x0028).rw(FUNC(avr8_device::gpio_r<GPIOC>), FUNC(avr8_device::port_w<GPIOC>));
	map(0x0029, 0x0029).r(FUNC(avr8_device::pin_r<3>));
	map(0x002b, 0x002b).rw(FUNC(avr8_device::gpio_r<GPIOD>), FUNC(avr8_device::port_w<GPIOD>));
	map(0x002c, 0x002c).r(FUNC(avr8_device::pin_r<4>));
	map(0x002e, 0x002e).rw(FUNC(avr8_device::gpio_r<GPIOE>), FUNC(avr8_device::port_w<GPIOE>));
	map(0x002f, 0x002f).r(FUNC(avr8_device::pin_r<5>));
	map(0x0031, 0x0031).rw(FUNC(avr8_device::gpio_r<GPIOF>), FUNC(avr8_device::port_w<GPIOF>));
	map(0x0032, 0x0032).r(FUNC(avr8_device::pin_r<6>));
	map(0x0034, 0x0034).rw(FUNC(avr8_device::gpio_r<GPIOG>), FUNC(avr8_device::port_w<GPIOG>));
	map(0x0035, 0x0035).w(FUNC(avr8_device::tifr0_w));
	map(0x0036, 0x0036).w(FUNC(avr8_device::tifr1_w));
	map(0x0037, 0x0037).w(FUNC(avr8_device::tifr2_w));
	map(0x003e, 0x003e).w(FUNC(avr8_device::gpior0_w));
	map(0x003f, 0x003f).w(FUNC(avr8_device::eecr_w));
	map(0x0043, 0x0043).w(FUNC(avr8_device::gtccr_w));
	map(0x0044, 0x0044).w(FUNC(avr8_device::tccr0a_w));
	map(0x0045, 0x0045).w(FUNC(avr8_device::tccr0b_w));
	map(0x0047, 0x0047).w(FUNC(avr8_device::ocr0a_w));
	map(0x0048, 0x0048).w(FUNC(avr8_device::ocr0b_w));
	map(0x004a, 0x004a).w(FUNC(avr8_device::gpior1_w));
	map(0x004b, 0x004b).w(FUNC(avr8_device::gpior2_w));
	map(0x004c, 0x004c).w(FUNC(avr8_device::spcr_w));
	map(0x004d, 0x004d).w(FUNC(avr8_device::spsr_w));
	map(0x004e, 0x004e).w(FUNC(avr8_device::spdr_w));
	map(0x0060, 0x0060).w(FUNC(avr8_device::wdtcsr_w));
	map(0x0061, 0x0061).w(FUNC(avr8_device::clkpr_w));
	map(0x0064, 0x0064).w(FUNC(avr8_device::prr0_w));
	map(0x0065, 0x0065).w(FUNC(avr8_device::prr1_w));
	map(0x0066, 0x0066).w(FUNC(avr8_device::osccal_w));
	map(0x0068, 0x0068).w(FUNC(avr8_device::pcicr_w));
	map(0x0069, 0x0069).w(FUNC(avr8_device::eicra_w));
	map(0x006a, 0x006a).w(FUNC(avr8_device::eicrb_w));
	map(0x006b, 0x006b).w(FUNC(avr8_device::pcmsk0_w));
	map(0x006c, 0x006c).w(FUNC(avr8_device::pcmsk1_w));
	map(0x006d, 0x006d).w(FUNC(avr8_device::pcmsk2_w));
	map(0x006e, 0x006e).w(FUNC(avr8_device::timsk0_w));
	map(0x006f, 0x006f).w(FUNC(avr8_device::timsk1_w));
	map(0x0070, 0x0070).w(FUNC(avr8_device::timsk2_w));
	map(0x0071, 0x0071).w(FUNC(avr8_device::timsk3_w));
	map(0x0072, 0x0072).w(FUNC(avr8_device::timsk4_w));
	map(0x0073, 0x0073).w(FUNC(avr8_device::timsk5_w));
	map(0x0074, 0x0074).w(FUNC(avr8_device::xmcra_w));
	map(0x0075, 0x0075).w(FUNC(avr8_device::xmcrb_w));
	map(0x0078, 0x0078).rw(FUNC(avr8_device::adcl_r), FUNC(avr8_device::adcl_w));
	map(0x0079, 0x0079).rw(FUNC(avr8_device::adch_r), FUNC(avr8_device::adch_w));
	map(0x007a, 0x007a).w(FUNC(avr8_device::adcsra_w));
	map(0x007b, 0x007b).w(FUNC(avr8_device::adcsrb_w));
	map(0x007c, 0x007c).w(FUNC(avr8_device::admux_w));
	map(0x007d, 0x007d).w(FUNC(avr8_device::didr2_w));
	map(0x007e, 0x007e).w(FUNC(avr8_device::didr0_w));
	map(0x007f, 0x007f).w(FUNC(avr8_device::didr1_w));
	map(0x0080, 0x0080).w(FUNC(avr8_device::tccr1a_w));
	map(0x0081, 0x0081).w(FUNC(avr8_device::tccr1b_w));
	map(0x0082, 0x0082).w(FUNC(avr8_device::tccr1c_w));
	map(0x0086, 0x0086).w(FUNC(avr8_device::icr1l_w));
	map(0x0087, 0x0087).w(FUNC(avr8_device::icr1h_w));
	map(0x0088, 0x0088).w(FUNC(avr8_device::ocr1al_w));
	map(0x0089, 0x0089).w(FUNC(avr8_device::ocr1ah_w));
	map(0x008a, 0x008a).w(FUNC(avr8_device::ocr1bl_w));
	map(0x008b, 0x008b).w(FUNC(avr8_device::ocr1bh_w));
	map(0x008c, 0x008c).w(FUNC(avr8_device::ocr1cl_w));
	map(0x008d, 0x008d).w(FUNC(avr8_device::ocr1ch_w));
	map(0x0090, 0x0090).w(FUNC(avr8_device::tccr3a_w));
	map(0x0091, 0x0091).w(FUNC(avr8_device::tccr3b_w));
	map(0x0092, 0x0092).w(FUNC(avr8_device::tccr3c_w));
	map(0x0096, 0x0096).w(FUNC(avr8_device::icr3l_w));
	map(0x0097, 0x0097).w(FUNC(avr8_device::icr3h_w));
	map(0x0098, 0x0098).w(FUNC(avr8_device::ocr3al_w));
	map(0x0099, 0x0099).w(FUNC(avr8_device::ocr3ah_w));
	map(0x009a, 0x009a).w(FUNC(avr8_device::ocr3bl_w));
	map(0x009b, 0x009b).w(FUNC(avr8_device::ocr3bh_w));
	map(0x009c, 0x009c).w(FUNC(avr8_device::ocr3cl_w));
	map(0x009d, 0x009d).w(FUNC(avr8_device::ocr3ch_w));
	map(0x00a0, 0x00a0).w(FUNC(avr8_device::tccr4a_w));
	map(0x00a1, 0x00a1).w(FUNC(avr8_device::tccr4b_w));
	map(0x00a2, 0x00a2).w(FUNC(avr8_device::tccr4c_w));
	map(0x00a4, 0x00a4).w(FUNC(avr8_device::tcnt4l_w));
	map(0x00a5, 0x00a5).w(FUNC(avr8_device::tcnt4h_w));
	map(0x00a6, 0x00a6).w(FUNC(avr8_device::icr4l_w));
	map(0x00a7, 0x00a7).w(FUNC(avr8_device::icr4h_w));
	map(0x00a8, 0x00a8).w(FUNC(avr8_device::ocr4al_w));
	map(0x00a9, 0x00a9).w(FUNC(avr8_device::ocr4ah_w));
	map(0x00aa, 0x00aa).w(FUNC(avr8_device::ocr4bl_w));
	map(0x00ab, 0x00ab).w(FUNC(avr8_device::ocr4bh_w));
	map(0x00ac, 0x00ac).w(FUNC(avr8_device::ocr4cl_w));
	map(0x00ad, 0x00ad).w(FUNC(avr8_device::ocr4ch_w));
	map(0x00b0, 0x00b0).w(FUNC(avr8_device::tccr2a_w));
	map(0x00b1, 0x00b1).w(FUNC(avr8_device::tccr2b_w));
	map(0x00b2, 0x00b2).w(FUNC(avr8_device::tcnt2_w));
	map(0x00b3, 0x00b3).w(FUNC(avr8_device::ocr2a_w));
	map(0x00b4, 0x00b4).w(FUNC(avr8_device::ocr2b_w));
	map(0x00b6, 0x00b6).w(FUNC(avr8_device::assr_w));
	map(0x00b8, 0x00b8).w(FUNC(avr8_device::twbr_w));
	map(0x00b9, 0x00b9).rw(FUNC(avr8_device::twsr_r), FUNC(avr8_device::twsr_w));
	map(0x00ba, 0x00ba).w(FUNC(avr8_device::twar_w));
	map(0x00bb, 0x00bb).w(FUNC(avr8_device::twdr_w));
	map(0x00bc, 0x00bc).w(FUNC(avr8_device::twcr_w));
	map(0x00bd, 0x00bd).w(FUNC(avr8_device::twamr_w));
	map(0x00c0, 0x00c0).w(FUNC(avr8_device::ucsr0a_w));
	map(0x00c1, 0x00c1).w(FUNC(avr8_device::ucsr0b_w));
	map(0x00c2, 0x00c2).w(FUNC(avr8_device::ucsr0c_w));
}

void atmega88_device::atmega88_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
}

void atmega168_device::atmega168_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
}

void atmega328_device::atmega328_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
}

void atmega644_device::atmega644_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
}

void atmega1280_device::atmega1280_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
	map(0x0100, 0x0100).r(FUNC(atmega1280_device::pin_r<7>));
	map(0x0102, 0x0102).rw(FUNC(atmega1280_device::gpio_r<GPIOH>), FUNC(atmega1280_device::port_w<GPIOH>));
	map(0x0103, 0x0103).r(FUNC(atmega1280_device::pin_r<8>));
	map(0x0105, 0x0105).rw(FUNC(atmega1280_device::gpio_r<GPIOJ>),FUNC(atmega1280_device::port_w<GPIOJ>));
	map(0x0106, 0x0106).r(FUNC(atmega1280_device::pin_r<9>));
	map(0x0108, 0x0108).rw(FUNC(atmega1280_device::gpio_r<GPIOK>),FUNC(atmega1280_device::port_w<GPIOK>));
	map(0x0109, 0x0109).r(FUNC(atmega1280_device::pin_r<10>));
	map(0x010b, 0x010b).rw(FUNC(atmega1280_device::gpio_r<GPIOL>),FUNC(atmega1280_device::port_w<GPIOL>));
	map(0x0120, 0x0120).w(FUNC(atmega1280_device::tccr5a_w));
	map(0x0121, 0x0121).w(FUNC(atmega1280_device::tccr5b_w));
}

void atmega2560_device::atmega2560_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
	map(0x0100, 0x0100).r(FUNC(atmega2560_device::pin_r<7>));
	map(0x0102, 0x0102).rw(FUNC(atmega2560_device::gpio_r<GPIOH>), FUNC(atmega2560_device::port_w<GPIOH>));
	map(0x0103, 0x0103).r(FUNC(atmega2560_device::pin_r<8>));
	map(0x0105, 0x0105).rw(FUNC(atmega2560_device::gpio_r<GPIOJ>),FUNC(atmega2560_device::port_w<GPIOJ>));
	map(0x0106, 0x0106).r(FUNC(atmega2560_device::pin_r<9>));
	map(0x0108, 0x0108).rw(FUNC(atmega2560_device::gpio_r<GPIOK>),FUNC(atmega2560_device::port_w<GPIOK>));
	map(0x0109, 0x0109).r(FUNC(atmega2560_device::pin_r<10>));
	map(0x010b, 0x010b).rw(FUNC(atmega2560_device::gpio_r<GPIOL>),FUNC(atmega2560_device::port_w<GPIOL>));
	map(0x0120, 0x0120).w(FUNC(atmega2560_device::tccr5a_w));
	map(0x0121, 0x0121).w(FUNC(atmega2560_device::tccr5b_w));
}

void attiny15_device::attiny15_internal_map(address_map &map)
{
	avr8_device::base_internal_map(map);
}

//-------------------------------------------------
//  atmega88_device - constructor
//-------------------------------------------------

atmega88_device::atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<3>(mconfig, tag, owner, clock, ATMEGA88, 0x0fff, address_map_constructor(FUNC(atmega88_device::atmega88_internal_map), this))
{
}

//-------------------------------------------------
//  atmega168_device - constructor
//-------------------------------------------------

atmega168_device::atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<3>(mconfig, tag, owner, clock, ATMEGA168, 0x1fff, address_map_constructor(FUNC(atmega168_device::atmega168_internal_map), this))
{
}

//-------------------------------------------------
//  atmega328_device - constructor
//-------------------------------------------------

atmega328_device::atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<3>(mconfig, tag, owner, clock, ATMEGA328, 0x7fff, address_map_constructor(FUNC(atmega328_device::atmega328_internal_map), this))
{
}

//-------------------------------------------------
//  atmega644_device - constructor
//-------------------------------------------------

atmega644_device::atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<3>(mconfig, tag, owner, clock, ATMEGA644, 0xffff, address_map_constructor(FUNC(atmega644_device::atmega644_internal_map), this))
{
}

//-------------------------------------------------
//  atmega1280_device - constructor
//-------------------------------------------------

atmega1280_device::atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<6>(mconfig, tag, owner, clock, ATMEGA1280, 0x1ffff, address_map_constructor(FUNC(atmega1280_device::atmega1280_internal_map), this))
{
}

//-------------------------------------------------
//  atmega2560_device - constructor
//-------------------------------------------------

atmega2560_device::atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<6>(mconfig, tag, owner, clock, ATMEGA2560, 0x1ffff, address_map_constructor(FUNC(atmega2560_device::atmega2560_internal_map), this))
{
}

//-------------------------------------------------
//  attiny15_device - constructor
//-------------------------------------------------

attiny15_device::attiny15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: avr8_device<2>(mconfig, tag, owner, clock, ATTINY15, 0x03ff, address_map_constructor(FUNC(attiny15_device::attiny15_internal_map), this))
{
}

//-------------------------------------------------
//  avr8_base_device - constructor
//-------------------------------------------------

avr8_base_device::avr8_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t addr_mask, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 22)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0, internal_map)
	, m_eeprom(*this, finder_base::DUMMY_TAG)
	, m_lfuses(0x62)
	, m_hfuses(0x99)
	, m_efuses(0xff)
	, m_lock_bits(0xff)
	, m_r(*this, "regs")
	, m_pc(0)
	, m_addr_mask((addr_mask << 1) | 1)
	, m_interrupt_pending(false)
{
}

template <int NumTimers>
avr8_device<NumTimers>::avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t addr_mask, address_map_constructor internal_map)
	: avr8_base_device(mconfig, tag, owner, clock, type, addr_mask, internal_map)
	, m_gpio_out_cb(*this)
	, m_gpio_in_cb(*this, 0)
	, m_adc_in_cb(*this, 0)
	, m_adc_timer(nullptr)
	, m_spi_active(false)
	, m_spi_prescale(0)
	, m_spi_prescale_count(0)
{
	// Fill in default callbacks
	for (int i = 0; i < 8*4; i++)
	{
		m_timer0_ticks[i] = timer_func(&avr8_device<NumTimers>::timer0_tick_default, this);
	}
	for (int i = 0; i < 8; i++)
	{
		m_timer2_ticks[i] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	}

	// Fill in the mode-specific callbacks
	for (int i = 0; i < 8; i++)
	{
		for (int j = 1; j < 4; j++)
		{
			switch (i)
			{
				case 0:
					m_timer0_ticks[(i << 2) + j] = timer_func(&avr8_device<NumTimers>::timer0_tick_norm, this);
					break;
				case 1:
					m_timer0_ticks[(i << 2) + j] = timer_func(&avr8_device<NumTimers>::timer0_tick_pwm_pc, this);
					break;
				case 3:
					m_timer0_ticks[(i << 2) + j] = timer_func(&avr8_device<NumTimers>::timer0_tick_fast_pwm, this);
					break;
				case 5:
					m_timer0_ticks[(i << 2) + j] = timer_func(&avr8_device<NumTimers>::timer0_tick_pwm_pc_cmp, this);
					break;
				case 7:
					m_timer0_ticks[(i << 2) + j] = timer_func(&avr8_device<NumTimers>::timer0_tick_fast_pwm_cmp, this);
					break;
			}
		}
	}

	m_timer0_ticks[(WGM02_CTC_CMP << 2) | 0] = timer_func(&avr8_device<NumTimers>::timer0_tick_ctc_norm, this);
	m_timer0_ticks[(WGM02_CTC_CMP << 2) | 1] = timer_func(&avr8_device<NumTimers>::timer0_tick_ctc_toggle, this);
	m_timer0_ticks[(WGM02_CTC_CMP << 2) | 2] = timer_func(&avr8_device<NumTimers>::timer0_tick_ctc_clear, this);
	m_timer0_ticks[(WGM02_CTC_CMP << 2) | 3] = timer_func(&avr8_device<NumTimers>::timer0_tick_ctc_set, this);

	for (int i = 0; i < 16; i++)
	{
		m_timer1_ticks[(WGM1_NORMAL << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_normal, this);
		m_timer1_ticks[(WGM1_PWM_8_PC << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm8_pc, this);
		m_timer1_ticks[(WGM1_PWM_9_PC << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm9_pc, this);
		m_timer1_ticks[(WGM1_PWM_10_PC << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm10_pc, this);
		m_timer1_ticks[(WGM1_FAST_PWM_8 << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm8, this);
		m_timer1_ticks[(WGM1_FAST_PWM_9 << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm9, this);
		m_timer1_ticks[(WGM1_FAST_PWM_10 << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm10, this);
		m_timer1_ticks[(WGM1_PWM_PFC_ICR << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm_pfc_icr, this);
		m_timer1_ticks[(WGM1_PWM_PFC_OCR << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm_pfc_ocr, this);
		m_timer1_ticks[(WGM1_PWM_PC_ICR << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm_pc_icr, this);
		m_timer1_ticks[(WGM1_PWM_PC_OCR << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_pwm_pc_ocr, this);
		m_timer1_ticks[(WGM1_CTC_ICR << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_icr, this);
		m_timer1_ticks[(WGM1_RESERVED << 4) | i] = timer_func(&avr8_device<NumTimers>::timer1_tick_resv, this);
	}

	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  0] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_norm_norm, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  1] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_norm_toggle, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  2] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_norm_clear, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  3] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_norm_set, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  4] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_toggle_norm, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  5] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_toggle_toggle, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  6] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_toggle_clear, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  7] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_toggle_set, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  8] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_clear_norm, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) |  9] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_clear_toggle, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 10] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_clear_clear, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 11] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_clear_set, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 12] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_set_norm, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 13] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_set_toggle, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 14] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_set_clear, this);
	m_timer1_ticks[(WGM1_CTC_OCR << 4) | 15] = timer_func(&avr8_device<NumTimers>::timer1_tick_ctc_ocr_set_set, this);

	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  0] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_norm_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  1] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_norm_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  2] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_norm_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  3] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_norm_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  4] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_toggle_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  5] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_toggle_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  6] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_toggle_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  7] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_toggle_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  8] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_clear_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) |  9] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_clear_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 10] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_clear_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 11] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_clear_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 12] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_set_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 13] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_set_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 14] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_set_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_ICR << 4) | 15] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_icr_set_set, this);

	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  0] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_norm_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  1] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_norm_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  2] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_norm_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  3] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_norm_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  4] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_toggle_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  5] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_toggle_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  6] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_toggle_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  7] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_toggle_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  8] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_clear_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) |  9] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_clear_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 10] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_clear_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 11] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_clear_set, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 12] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_set_norm, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 13] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_set_toggle, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 14] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_set_clear, this);
	m_timer1_ticks[(WGM1_FAST_PWM_OCR << 4) | 15] = timer_func(&avr8_device<NumTimers>::timer1_tick_fast_pwm_ocr_set_set, this);

	m_timer2_ticks[WGM02_NORMAL] = timer_func(&avr8_device<NumTimers>::timer2_tick_norm, this);
	m_timer2_ticks[WGM02_PWM_PC] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	m_timer2_ticks[WGM02_CTC_CMP] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	m_timer2_ticks[WGM02_FAST_PWM] = timer_func(&avr8_device<NumTimers>::timer2_tick_fast_pwm, this);
	m_timer2_ticks[WGM02_RESERVED0] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	m_timer2_ticks[WGM02_PWM_PC_CMP] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	m_timer2_ticks[WGM02_RESERVED1] = timer_func(&avr8_device<NumTimers>::timer2_tick_default, this);
	m_timer2_ticks[WGM02_FAST_PWM_CMP] = timer_func(&avr8_device<NumTimers>::timer2_tick_fast_pwm_cmp, this);

	m_timer0_tick = m_timer0_ticks[0];
	m_timer1_tick = m_timer1_ticks[0];
	m_timer2_tick = m_timer2_ticks[0];
}

//-------------------------------------------------
//  set_low_fuses
//-------------------------------------------------

void avr8_base_device::set_low_fuses(const uint8_t byte)
{
	m_lfuses = byte;
}

//-------------------------------------------------
//  set_high_fuses
//-------------------------------------------------

void avr8_base_device::set_high_fuses(const uint8_t byte)
{
	m_hfuses = byte;
}

//-------------------------------------------------
//  set_extended_fuses
//-------------------------------------------------

void avr8_base_device::set_extended_fuses(const uint8_t byte)
{
	m_efuses = byte;
}

//-------------------------------------------------
//  set_lock_bits
//-------------------------------------------------

void avr8_base_device::set_lock_bits(const uint8_t byte)
{
	m_lock_bits = byte;
}

//-------------------------------------------------
//  unimplemented_opcode - bail on unspuported
//  instruction
//-------------------------------------------------

void avr8_base_device::unimplemented_opcode(uint32_t op)
{
//  machine().debug_break();
	fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, m_pc);
}


//-------------------------------------------------
//  is_long_opcode - returns true if opcode is 4
//  bytes long
//-------------------------------------------------

inline bool avr8_base_device::is_long_opcode(uint16_t op)
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

void avr8_base_device::device_start()
{
	m_pc = 0;

	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_r[SREG]).callimport().callexport().formatstr("%8s").noshow();
	state_add(AVR8_SREG,       "STATUS",    m_r[SREG]).mask(0xff);
	state_add(AVR8_PC,         "PC",        m_pc).mask(m_addr_mask).callexport();
	state_add(AVR8_SPH,        "SPH",       m_r[SPH]).mask(0xff);
	state_add(AVR8_SPL,        "SPL",       m_r[SPL]).mask(0xff);
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

template <int NumTimers>
void avr8_device<NumTimers>::device_start()
{
	avr8_base_device::device_start();

	m_adc_timer = timer_alloc(FUNC(avr8_device<NumTimers>::adc_conversion_complete), this);

	// Timers
	save_item(NAME(m_timer_top));
	save_item(NAME(m_timer_prescale));
	save_item(NAME(m_timer_prescale_count));
	save_item(NAME(m_ocr2_not_reached_yet));
	save_item(NAME(m_ocr1));

	// ADC
	save_item(NAME(m_adc_sample));
	save_item(NAME(m_adc_result));
	save_item(NAME(m_adc_data));
	save_item(NAME(m_adc_first));
	save_item(NAME(m_adc_hold));

	// SPI
	save_item(NAME(m_spi_active));
	save_item(NAME(m_spi_prescale));
	save_item(NAME(m_spi_prescale_count));
	save_item(NAME(m_spi_prescale_countdown));
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void avr8_base_device::device_reset()
{
	logerror("AVR low fuse bits: 0x%02X\n", m_lfuses);
	logerror("AVR high fuse bits: 0x%02X\n", m_hfuses);
	logerror("AVR extended fuse bits: 0x%02X\n", m_efuses);
	logerror("AVR lock bits: 0x%02X\n", m_lock_bits);

	switch ((m_hfuses & (BOOTSZ1 | BOOTSZ0)) >> 1)
	{
	case 0:
		if (m_addr_mask <= 0x1fff) { m_boot_size = 1024; }
		else if (m_addr_mask <= 0xffff) { m_boot_size = 2048; }
		else { m_boot_size = 4096; }
		break;
	case 1:
		if (m_addr_mask <= 0x1fff) { m_boot_size = 512; }
		else if (m_addr_mask <= 0xffff) { m_boot_size = 1024; }
		else { m_boot_size = 2048; }
		break;
	case 2:
		if (m_addr_mask <= 0x1fff) { m_boot_size = 256; }
		else if (m_addr_mask <= 0xffff) { m_boot_size = 512; }
		else { m_boot_size = 1024; }
		break;
	case 3:
		if (m_addr_mask <= 0x1fff) { m_boot_size = 128; }
		else if (m_addr_mask <= 0xffff) { m_boot_size = 256; }
		else { m_boot_size = 512; }
		break;
	default:
		break;
	}

	if (m_hfuses & BOOTRST)
	{
		m_pc = 0x0000;
		LOGMASKED(LOG_BOOT, "Booting AVR core from address 0x0000\n");
	} else {
		m_pc = (m_addr_mask + 1) - 2 * m_boot_size;
		LOGMASKED(LOG_BOOT, "AVR Boot loader section size: %d words\n", m_boot_size);
	}

	for (int i = 0; i < 0x200; i++)
	{
		m_r[i] = 0;
	}

	m_interrupt_pending = false;
}

template <int NumTimers>
void avr8_device<NumTimers>::device_reset()
{
	m_adc_sample = 0;
	m_adc_result = 0;
	m_adc_data = 0;
	m_adc_first = true;
	m_adc_hold = false;

	m_spi_active = false;
	m_spi_prescale = 0;
	m_spi_prescale_count = 0;

	for (int t = 0; t < NumTimers; t++)
	{
		m_timer_top[t] = 0;
		m_timer_prescale[t] = 0xffff;
		m_timer_prescale_count[t] = 0;
	}

	for (int reg = AVR8_REG_A; reg <= AVR8_REG_C; reg++)
	{
		m_ocr1[reg] = 0;
	}

	m_ocr2_not_reached_yet = true;
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the CPU's address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector avr8_base_device::memory_space_config() const
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

void avr8_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENPC:
	case STATE_GENPCBASE:
	case AVR8_PC:
		str = string_format("%05x", m_pc << 1);
		break;
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c",
			(m_r[SREG] & 0x80) ? 'I' : '-',
			(m_r[SREG] & 0x40) ? 'T' : '-',
			(m_r[SREG] & 0x20) ? 'H' : '-',
			(m_r[SREG] & 0x10) ? 'S' : '-',
			(m_r[SREG] & 0x08) ? 'V' : '-',
			(m_r[SREG] & 0x04) ? 'N' : '-',
			(m_r[SREG] & 0x02) ? 'Z' : '-',
			(m_r[SREG] & 0x01) ? 'C' : '-');
		break;
	}
}

std::unique_ptr<util::disasm_interface> avr8_base_device::create_disassembler()
{
	return std::make_unique<avr8_disassembler>();
}


//**************************************************************************
//  MEMORY ACCESSORS
//**************************************************************************

inline void avr8_base_device::push(uint8_t val)
{
	uint16_t sp = SPREG;
	m_data->write_byte(sp, val);
	sp--;
	m_r[SPL] = sp & 0x00ff;
	m_r[SPH] = (sp >> 8) & 0x00ff;
}

inline uint8_t avr8_base_device::pop()
{
	uint16_t sp = SPREG;
	sp++;
	m_r[SPL] = sp & 0x00ff;
	m_r[SPH] = (sp >> 8) & 0x00ff;
	return m_data->read_byte(sp);
}

//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void avr8_base_device::set_irq_line(uint16_t vector, int state)
{
	if (state)
	{
		if (BIT(m_r[SREG], SREG_I))
		{
			m_r[SREG] &= ~SREG_MASK_I;
			push((m_pc >> 1) & 0x00ff);
			push((m_pc >> 9) & 0x00ff);
			m_pc = vector << 1;
		}
		else
		{
			m_interrupt_pending = true;
		}
	}
}

const avr8_base_device::interrupt_condition avr8_base_device::s_int_conditions[avr8_base_device::INTIDX_COUNT] =
{
	{ AVR8_INT_SPI_STC, SPCR,   SPCR_SPIE_MASK,     SPSR,    SPSR_SPIF_MASK },
	{ AVR8_INT_T0COMPB, TIMSK0, TIMSK0_OCIE0B_MASK, TIFR0,   TIFR0_OCF0B_MASK },
	{ AVR8_INT_T0COMPA, TIMSK0, TIMSK0_OCIE0A_MASK, TIFR0,   TIFR0_OCF0A_MASK },
	{ AVR8_INT_T0OVF,   TIMSK0, TIMSK0_TOIE0_MASK,  TIFR0,   TIFR0_TOV0_MASK },
	{ AVR8_INT_T1CAPT,  TIMSK1, TIMSK1_ICIE1_MASK,  TIFR1,   TIFR1_ICF1_MASK },
	{ AVR8_INT_T1COMPB, TIMSK1, TIMSK1_OCIE1B_MASK, TIFR1,   TIFR1_OCF1B_MASK },
	{ AVR8_INT_T1COMPA, TIMSK1, TIMSK1_OCIE1A_MASK, TIFR1,   TIFR1_OCF1A_MASK },
	{ AVR8_INT_T1OVF,   TIMSK1, TIMSK1_TOIE1_MASK,  TIFR1,   TIFR1_TOV1_MASK },
	{ AVR8_INT_T2COMPB, TIMSK2, TIMSK2_OCIE2B_MASK, TIFR2,   TIFR2_OCF2B_MASK },
	{ AVR8_INT_T2COMPA, TIMSK2, TIMSK2_OCIE2A_MASK, TIFR2,   TIFR2_OCF2A_MASK },
	{ AVR8_INT_T2OVF,   TIMSK2, TIMSK2_TOIE2_MASK,  TIFR2,   TIFR2_TOV2_MASK }
};

void avr8_base_device::update_interrupt(int source)
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

const avr8_base_device::interrupt_condition avr8_base_device::s_mega644_int_conditions[avr8_base_device::INTIDX_COUNT] =
{
	{ ATMEGA644_INT_SPI_STC, SPCR,   SPCR_SPIE_MASK,     SPSR,    SPSR_SPIF_MASK },
	{ ATMEGA644_INT_T0COMPB, TIMSK0, TIMSK0_OCIE0B_MASK, TIFR0,   TIFR0_OCF0B_MASK },
	{ ATMEGA644_INT_T0COMPA, TIMSK0, TIMSK0_OCIE0A_MASK, TIFR0,   TIFR0_OCF0A_MASK },
	{ ATMEGA644_INT_T0OVF,   TIMSK0, TIMSK0_TOIE0_MASK,  TIFR0,   TIFR0_TOV0_MASK },
	{ ATMEGA644_INT_T1CAPT,  TIMSK1, TIMSK1_ICIE1_MASK,  TIFR1,   TIFR1_ICF1_MASK },
	{ ATMEGA644_INT_T1COMPB, TIMSK1, TIMSK1_OCIE1B_MASK, TIFR1,   TIFR1_OCF1B_MASK },
	{ ATMEGA644_INT_T1COMPA, TIMSK1, TIMSK1_OCIE1A_MASK, TIFR1,   TIFR1_OCF1A_MASK },
	{ ATMEGA644_INT_T1OVF,   TIMSK1, TIMSK1_TOIE1_MASK,  TIFR1,   TIFR1_TOV1_MASK },
	{ ATMEGA644_INT_T2COMPB, TIMSK2, TIMSK2_OCIE2B_MASK, TIFR2,   TIFR2_OCF2B_MASK },
	{ ATMEGA644_INT_T2COMPA, TIMSK2, TIMSK2_OCIE2A_MASK, TIFR2,   TIFR2_OCF2A_MASK },
	{ ATMEGA644_INT_T2OVF,   TIMSK2, TIMSK2_TOIE2_MASK,  TIFR2,   TIFR2_TOV2_MASK }
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

	if (intstate) logerror("interrupt %d is 1\n", source);
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

	if (intstate) logerror("interrupt %d is 1\n", source);
	set_irq_line(condition.m_intindex << 1, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}


//**************************************************************************
//  PERIPHERAL HANDLING
//**************************************************************************

template <int NumTimers>
void avr8_device<NumTimers>::spi_tick()
{
	const uint8_t out_bit = (m_r[SPDR] & (1 << m_spi_prescale_countdown)) >> m_spi_prescale_countdown;
	m_spi_prescale_countdown--;
	const uint8_t data = (m_r[PORTB] &~ PORTB_MOSI) | (out_bit ? PORTB_MOSI : 0);
	m_r[PORTB] = data;
	m_gpio_out_cb[GPIOB](data);
	m_r[PORTB] = (m_r[PORTB] &~ PORTB_MOSI) | (out_bit ? PORTB_MOSI : 0);
}

// Timer 0 Handling

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_norm()
{
	LOGMASKED(LOG_TIMER0, "%s: WGM02_NORMAL: Unimplemented timer#0 waveform generation mode\n", machine().describe_context());
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_pwm_pc()
{
	LOGMASKED(LOG_TIMER0, "%s: WGM02_PWM_PC: Unimplemented timer#0 waveform generation mode\n", machine().describe_context());
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_ctc_norm()
{
	static const uint8_t s_ocf0[2] = { (1 << TIFR0_OCF0A_SHIFT), (1 << TIFR0_OCF0B_SHIFT) };
	static const uint8_t s_int0[2] = { INTIDX_OCF0A, INTIDX_OCF0B };

	if (m_r[TCNT0] == m_r[OCR0A] - 1)
	{
		m_r[TIFR0] |= s_ocf0[AVR8_REG_A];
		update_interrupt(s_int0[AVR8_REG_A]);
		m_r[TCNT0] = 0;
	}
	else if (m_r[TCNT0] == m_r[OCR0B] - 1)
	{
		m_r[TIFR0] |= s_ocf0[AVR8_REG_B];
		update_interrupt(s_int0[AVR8_REG_B]);
		m_r[TCNT0]++;
	}
	else
	{
		m_r[TCNT0]++;
	}
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_ctc_toggle()
{
	if (m_r[TCNT0] == m_timer_top[0] - 1)
	{
		m_timer_top[0] = 0;
		LOGMASKED(LOG_TIMER0, "%s: timer0: Toggle OC0B on match\n", machine().describe_context());
		m_r[PORTG] ^= (1 << 5);
		m_gpio_out_cb[PORTG](m_r[PORTG]);
	}
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_ctc_clear()
{
	if (m_r[TCNT0] == m_timer_top[0])
	{
		m_timer_top[0] = 0;
		LOGMASKED(LOG_TIMER0, "[0] timer0: Clear OC0B on match\n", machine().describe_context());
		m_r[PORTG] &= ~(1 << 5);
		m_gpio_out_cb[PORTG](m_r[PORTG]);
	}
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_ctc_set()
{
	if (m_r[TCNT0] == m_timer_top[0])
	{
		m_timer_top[0] = 0;
		LOGMASKED(LOG_TIMER0, "%s: timer0: Set OC0B on match\n", machine().describe_context());
		m_r[PORTG] |= (1 << 5);
		m_gpio_out_cb[PORTG](m_r[PORTG]);
	}
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_fast_pwm()
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_FAST_PWM: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_pwm_pc_cmp()
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_PWM_PC_CMP: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_fast_pwm_cmp()
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: WGM02_FAST_PWM_CMP: Unimplemented timer0 waveform generation mode\n", machine().describe_context());
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_tick_default()
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: timer0_tick_default: Unknown waveform generation mode: %02x\n", machine().describe_context(), WGM0);
	m_r[TCNT0]++;
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer0_force_output_compare(int reg)
{
	LOGMASKED(LOG_TIMER0 | LOG_UNKNOWN, "%s: timer0_force_output_compare: TODO; should be forcing OC0%c\n", machine().describe_context(), 'A' + reg);
	m_timer_prescale_count[0] -= m_timer_prescale[0];
}

template <int NumTimers>
void avr8_device<NumTimers>::update_ocr0(uint8_t newval, uint8_t reg)
{
	m_r[(reg == AVR8_REG_A) ? OCR0A : OCR0B] = newval;
}

// Timer 1 Handling

template <int NumTimers>
template <int TimerMode, int ChannelModeA, int ChannelModeB>
inline void avr8_device<NumTimers>::timer1_tick()
{
	static const uint8_t s_ocf1[2] = { (1 << TIFR1_OCF1A_SHIFT), (1 << TIFR1_OCF1B_SHIFT) };
	static const uint8_t s_int1[2] = { INTIDX_OCF1A, INTIDX_OCF1B };
	const uint16_t icr1 = ICR1;
	uint16_t timer1_count = (m_r[TCNT1H] << 8) | m_r[TCNT1L];
	int32_t increment = 1;

	switch (TimerMode)
	{
	case WGM1_CTC_OCR:
		if (timer1_count == 0xffff)
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 CTC_OCR, TOP, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			m_r[TIFR1] |= TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);
			timer1_count = 0;
			increment = 0;
		}

		if (timer1_count == m_ocr1[AVR8_REG_A])
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 CTC_OCR, OCR1A, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			timer1_count = 0;
			increment = 0;

			switch (ChannelModeA)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
				break;

			case 1: /* Toggle OC1A on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A on match\n", machine().describe_context());
				m_r[PORTB] ^= 2;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1A on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_A);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1A on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_A;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_A];
			update_interrupt(s_int1[AVR8_REG_A]);
		}
		else if (timer1_count == 0)
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 CTC_OCR, BOTTOM, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			m_r[TIFR1] &= ~TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);
		}

		if (timer1_count == m_ocr1[AVR8_REG_B])
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 CTC_OCR, OCR1B, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			switch (ChannelModeB)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
			case 1: /* Toggle OC1A on compare match */
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1B on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_B);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1B on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_B;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_B];
			update_interrupt(s_int1[AVR8_REG_B]);
		}
		break;

	case WGM1_FAST_PWM_OCR:
		if (timer1_count == m_ocr1[AVR8_REG_A])
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_OCR, OCR1A, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			m_r[TIFR1] |= TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);
			timer1_count = 0;
			increment = 0;

			switch (ChannelModeA)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
				break;

			case 1: /* Toggle OC1A on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A on match\n", machine().describe_context());
				m_r[PORTB] ^= 2;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1A on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_A);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1A on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_A;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_A];
			update_interrupt(s_int1[AVR8_REG_A]);
		}
		else if (timer1_count == 0)
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_OCR, BOTTOM A, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			m_r[TIFR1] &= ~TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);

			switch (ChannelModeA)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
				break;

			case 1: /* Toggle OC1A at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] ^= 2;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 2: /* Set OC1A/B at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_A;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Clear OC1A/B at BOTTOM */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_A);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}
		}

		if (timer1_count == m_ocr1[AVR8_REG_B])
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_OCR, OCR1B, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			switch (ChannelModeB)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
			case 1: /* Toggle OC1A on compare match */
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1B on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_B);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1B on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_B;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_B];
			update_interrupt(s_int1[AVR8_REG_B]);
		}
		else if (timer1_count == 0)
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_OCR, BOTTOM B, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			switch (ChannelModeB)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
			case 1: /* Toggle OC1A at BOTTOM*/
				break;

			case 2: /* Set OC1A/B at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1B at BOTTOM\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_B;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Clear OC1A/B at BOTTOM */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1B at BOTTOM\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_B);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}
		}
		break;

	case WGM1_FAST_PWM_ICR:
		if (timer1_count == m_ocr1[AVR8_REG_A])
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_ICR, OCR1A, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			switch (ChannelModeA)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
				break;

			case 1: /* Toggle OC1A on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A on match\n", machine().describe_context());
				m_r[PORTB] ^= 2;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1A on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_A);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1A on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_A;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_A];
			update_interrupt(s_int1[AVR8_REG_A]);
		}
		else if (timer1_count == 0)
		{
			LOGMASKED(LOG_TIMER1_TICK, "%s: timer1 WGM1 FAST_PWM_ICR, BOTTOM A, new count %04x, OCR1A/B %04x/%04x, ICR1 %04x\n", machine().describe_context(), timer1_count, m_ocr1[AVR8_REG_A], m_ocr1[AVR8_REG_B], icr1);
			m_r[TIFR1] &= ~TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);

			switch (ChannelModeA)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
				break;

			case 1: /* Toggle OC1A at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Toggle OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] ^= 2;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 2: /* Set OC1A/B at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_A;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Clear OC1A/B at BOTTOM */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1A at BOTTOM\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_A);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}
		}

		if (timer1_count == m_ocr1[AVR8_REG_B])
		{
			switch (ChannelModeB)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
			case 1: /* Toggle OC1A on compare match */
				break;

			case 2: /* Clear OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1B on match\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_B);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Set OC1A/B on compare match */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1B on match\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_B;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}

			m_r[TIFR1] |= s_ocf1[AVR8_REG_B];
			update_interrupt(s_int1[AVR8_REG_B]);
		}
		else if (timer1_count == 0)
		{
			switch (ChannelModeB)
			{
			case 0: /* Normal Operation; OC1A/B disconnected */
			case 1: /* Toggle OC1A at BOTTOM*/
				break;

			case 2: /* Set OC1A/B at BOTTOM*/
				LOGMASKED(LOG_TIMER1, "%s: timer1: Set OC1B at BOTTOM\n", machine().describe_context());
				m_r[PORTB] |= 2 << AVR8_REG_B;
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;

			case 3: /* Clear OC1A/B at BOTTOM */
				LOGMASKED(LOG_TIMER1, "%s: timer1: Clear OC1B at BOTTOM\n", machine().describe_context());
				m_r[PORTB] &= ~(2 << AVR8_REG_B);
				m_gpio_out_cb[GPIOB](m_r[PORTB]);
				break;
			}
		}

		if (timer1_count == icr1)
		{
			m_r[TIFR1] |= TIFR1_TOV1_MASK;
			update_interrupt(INTIDX_TOV1);
			timer1_count = 0;
			increment = 0;
		}
		break;

	default:
		LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: timer1_tick: Unknown waveform generation mode: %02x\n", machine().describe_context(), WGM1);
		break;
	}

	timer1_count += increment;
	m_timer_prescale_count[1] -= m_timer_prescale[1];

	m_r[TCNT1H] = timer1_count >> 8;
	m_r[TCNT1L] = (uint8_t)timer1_count;
}

template <int NumTimers>
void avr8_device<NumTimers>::update_timer_waveform_gen_mode(uint8_t t, uint8_t mode)
{
	int32_t oc_val = -1, ic_val = -1;
	switch (t)
	{
	case 0:
		oc_val = m_r[OCR0A];
		ic_val = -1;
		break;
	case 1:
		oc_val = OCR1A;
		ic_val = ICR1;
		break;
	case 2:
		oc_val = OCR2A;
		ic_val = -1;
		break;
	case 3:
		oc_val = OCR3A;
		ic_val = ICR3;
		break;
	case 4:
		oc_val = OCR4A;
		ic_val = ICR4;
		break;
	case 5:
		oc_val = OCR5A;
		ic_val = ICR5;
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

	if (m_timer_top[t] == -1)
	{
		m_timer_top[t] = 0;
		LOGMASKED((LOG_TIMER0 << t), "%s: update_timer_waveform_gen_mode: Timer %d - Unsupported waveform generation type: %d\n", machine().describe_context(), t, mode);
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::update_timer1_input_noise_canceler()
{
	LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: update_timer1_input_noise_canceler: TODO\n", machine().describe_context());
}

template <int NumTimers>
void avr8_device<NumTimers>::update_timer1_input_edge_select()
{
	LOGMASKED(LOG_TIMER1 | LOG_UNKNOWN, "%s: update_timer1_input_edge_select: TODO\n", machine().describe_context());
}

template <int NumTimers>
void avr8_device<NumTimers>::update_ocr1(uint16_t newval, uint8_t reg)
{
	static const int32_t s_high_indices[3] = { OCR1AH, OCR1BH, OCR1CH };
	static const int32_t s_low_indices[3] =  { OCR1AL, OCR1BL, OCR1CL };
	m_r[s_high_indices[reg]] = (uint8_t)(newval >> 8);
	m_r[s_low_indices[reg]]  = (uint8_t)newval;
	m_ocr1[reg] = newval;
}

// Timer 2 Handling

template <int NumTimers>
void avr8_device<NumTimers>::timer2_tick_default()
{
	LOGMASKED(LOG_TIMER2, "%s: WGM02_PWM_PC: Unimplemented timer#2 waveform generation mode %d\n", machine().describe_context(), WGM2);
	m_r[TCNT2]++;
	m_timer_prescale_count[2] -= m_timer_prescale[2];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer2_tick_norm()
{
	LOGMASKED(LOG_TIMER2, "%s: timer2_tick_norm; WGM02_NORMAL\n", machine().describe_context());
	if (m_r[TCNT2] == 0xff)
	{
		m_r[TIFR2] |= TIFR2_TOV2_MASK;
	}
	m_r[TCNT2]++;
	update_interrupt(INTIDX_TOV2);
	m_timer_prescale_count[2] -= m_timer_prescale[2];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer2_tick_fast_pwm()
{
	if (m_r[TCNT2] >= m_r[OCR2A])
	{
		if (m_r[TCNT2] >= 0xff)
		{
			// Turn on
			m_r[PORTD] |= 1 << 7;
			m_gpio_out_cb[GPIOD](m_r[PORTD]);
			m_r[TCNT2] = 0;
			m_ocr2_not_reached_yet = true;
		}
		else
		{
			if (m_ocr2_not_reached_yet)
			{
				// Turn off
				m_r[PORTD] &= ~(1 << 7);
				m_gpio_out_cb[GPIOD](m_r[PORTD]);
				m_ocr2_not_reached_yet = false;
			}
		}
	}

	m_r[TCNT2]++;
	m_timer_prescale_count[2] -= m_timer_prescale[2];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer2_tick_fast_pwm_cmp()
{
	const uint8_t ocf2[2] = { (1 << TIFR2_OCF2A_SHIFT), (1 << TIFR2_OCF2B_SHIFT) };
	uint16_t count = m_r[TCNT2];

	int32_t increment = 1;

	if (count == m_r[OCR2A])
	{
		m_r[TIFR2] |= TIFR2_TOV2_MASK;
		count = 0;
		increment = 0;
		m_r[TIFR2] |= ocf2[AVR8_REG_A];
	}
	else if (count == 0)
	{
		m_r[TIFR2] &= ~TIFR2_TOV2_MASK;
	}

	if (count == m_r[OCR2B])
	{
		m_r[TIFR2] |= ocf2[AVR8_REG_B];
	}

	count += increment;

	update_interrupt(INTIDX_TOV2);
	m_r[TCNT2] = count;
	m_timer_prescale_count[2] -= m_timer_prescale[2];
}

template <int NumTimers>
void avr8_device<NumTimers>::timer2_force_output_compare(int reg)
{
	LOGMASKED(LOG_TIMER2 | LOG_UNKNOWN, "%s: force_output_compare: TODO; should be forcing OC2%c\n", machine().describe_context(), 'A' + reg);
}

template <int NumTimers>
void avr8_device<NumTimers>::update_ocr2(uint8_t newval, uint8_t reg)
{
	m_r[(reg == AVR8_REG_A) ? OCR2A : OCR2B] = newval;

	// Nothing needs to be done? All handled in timer callback
}

/************************************************************************************************/

// Timer 3 Handling
template <int NumTimers>
void avr8_device<NumTimers>::timer3_tick()
{
}

/************************************************************************************************/

template <int NumTimers>
void avr8_device<NumTimers>::timer4_tick()
{
	/* TODO: Handle comparison, setting OC1x pins, detection of BOTTOM and TOP */
	LOGMASKED(LOG_TIMER4_TICK, "%s: WGM4: %d\n", machine().describe_context(), WGM4);
	LOGMASKED(LOG_TIMER4_TICK, "%s: TCCR4A_COM4B: %d\n", machine().describe_context(), TCCR4A_COM4B);

	uint16_t count = (m_r[TCNT4H] << 8) | m_r[TCNT4L];
	int32_t increment = 1;

	switch (WGM4)
	{
	case WGM4_FAST_PWM_8:
	case WGM4_FAST_PWM_9:
	case WGM4_FAST_PWM_10:
		switch (TCCR4A_COM4B)
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
				m_r[PORTG] &= ~(1 << 5);
				m_gpio_out_cb[GPIOG](m_r[PORTG]);
			}
			else if (count == 0)
			{
				// Set OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: non-inverting mode, Set OC0B\n", machine().describe_context());
				m_r[PORTG] |= 1 << 5;
				m_gpio_out_cb[GPIOG](m_r[PORTG]);
			}
			break;
		case 3: /* Inverting mode */
			if (count == m_timer_top[4])
			{
				// Set OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: inverting mode, Clear OC0B\n", machine().describe_context());
				m_r[PORTG] |= 1 << 5;
				m_gpio_out_cb[GPIOG](m_r[PORTG]);
			}
			else if (count == 0)
			{
				// Clear OC0B
				LOGMASKED(LOG_TIMER4, "%s: timer4: inverting mode, Set OC0B\n", machine().describe_context());
				m_r[PORTG] &= ~(1 << 5);
				m_gpio_out_cb[GPIOG](m_r[PORTG]);
			}
			break;
		}
		break;

	case WGM4_CTC_OCR:
		LOGMASKED(LOG_TIMER4, "%s: timer4: tick WGM4_CTC_OCR: %d\n", machine().describe_context(), count);
		if (count == 0xffff)
		{
			m_r[TIFR4] |= TIFR4_TOV4_MASK;
			update_interrupt(INTIDX_TOV4);
			count = 0;
			increment = 0;
		}

		/* TODO: test for match against all 3 registers */
		break;

	case WGM1_FAST_PWM_OCR:
		/* TODO: test for match against all 3 registers */
		break;

	default:
		LOGMASKED(LOG_TIMER4 | LOG_UNKNOWN, "%s: timer4_tick: Unknown waveform generation mode: %02x\n", machine().describe_context(), WGM4);
		break;
	}

	count += increment;
	m_r[TCNT4H] = (count >> 8) & 0xff;
	m_r[TCNT4L] = count & 0xff;
}

template <int NumTimers>
template <int Timer>
void avr8_device<NumTimers>::update_timer_clock_source(uint8_t clock_select, const uint8_t old_clock_select)
{
	static const int32_t s_prescale_values[2][8] =
	{
		{ 0x0000ffff, 1, 8, 64, 256, 1024, -1, -1 }, // T0/T1/T3/T4/T5
		{ 0x0000ffff, 1, 8, 32, 64, 128, 256, 1024 } // T2
	};
	const int32_t prescale_divisor = s_prescale_values[(Timer == 2) ? 1 : 0][clock_select];
	const uint16_t old_prescale = s_prescale_values[(Timer == 2) ? 1 : 0][old_clock_select];
	m_timer_prescale[Timer] = (uint16_t)prescale_divisor;

	LOGMASKED(LOG_TIMER0 << Timer, "%s: update_timer_clock_source: t = %d, cs = %d\n", machine().describe_context(), Timer, clock_select);

	if (prescale_divisor == -1)
	{
		LOGMASKED(LOG_TIMER0 << Timer, "%s: timer%d: update_timer_clock_source: External trigger mode not implemented yet\n", machine().describe_context(), Timer);
		m_timer_prescale[Timer] = 0xffff;
	}

	if (old_prescale == 0xffff || m_timer_prescale_count[Timer] > m_timer_prescale[Timer])
	{
		m_timer_prescale_count[Timer] = m_timer_prescale[Timer];
	}
}

/************************************************************************************************/

// Timer 5 Handling

template <int NumTimers>
void avr8_device<NumTimers>::timer5_tick()
{
	LOGMASKED(LOG_TIMER5_TICK, "%s: WGM5: %d\n", machine().describe_context(), WGM5);
	LOGMASKED(LOG_TIMER5_TICK, "%s: TCCR5A_COM5B: %d\n", machine().describe_context(), TCCR5A_COM5B);

	uint16_t count = (m_r[TCNT5H] << 8) + m_r[TCNT5L];
	int32_t increment = 1;

	switch (WGM5)
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
		LOGMASKED(LOG_TIMER5 | LOG_UNKNOWN, "%s: Unimplemented timer5 waveform generation mode: WGM5 = 0x%02X\n", machine().describe_context(), WGM5);
		break;

	case WGM5_CTC_OCR:
		// TODO: Please verify, might be wrong
		switch (TCCR5A_COM5B)
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
				m_r[PORTL] ^= 1 << 4;
				m_gpio_out_cb[PORTL](m_r[PORTL]);
			}
			break;
		case 2: /* Clear OC5B on compare match */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
				// Clear OC5B
				LOGMASKED(LOG_TIMER5, "%s: timer5: Clear OC5B on compare match\n", machine().describe_context());
				m_r[PORTL] &= ~(1 << 4);
				m_gpio_out_cb[PORTL](m_r[PORTL]);
			}
			break;
		case 3: /* Set OC5B on compare match */
			if (count == m_timer_top[5])
			{
				m_timer_top[5] = 0;
				LOGMASKED(LOG_TIMER5, "%s: timer5: Set OC5B on compare match\n", machine().describe_context());
				m_r[PORTL] |= 1 << 4;
				m_gpio_out_cb[PORTL](m_r[PORTL]);
			}
			break;
		}
		break;

	default:
		LOGMASKED(LOG_TIMER5 | LOG_UNKNOWN, "%s: timer5: Unknown waveform generation mode: %02x\n", machine().describe_context(), WGM5);
		break;
	}

	count += increment;
	m_r[TCNT5H] = (count >> 8) & 0xff;
	m_r[TCNT5L] = count & 0xff;
}

/************************************************************************************************/


/****************/
/* ADC Handling */
/****************/

template <int NumTimers>
void avr8_device<NumTimers>::adc_start_conversion()
{
	// set capture in progress flag
	m_r[ADCSRA] |= ADCSRA_ADSC_MASK;

	// get a sample - the sample-and-hold circuit will hold this for the duration of the conversion
	if (ADMUX_MUX < 0x8)
	{
		m_adc_sample = m_adc_in_cb[ADMUX_MUX]() & 0x3ff;
	}
	else if (ADMUX_MUX == 0xe)
	{
		// 1.1V (Vbg)
		// FIXME: the value this acquires depends on what the reference source is set to
		logerror("%s: Using unimplemented 1.1V reference voltage as ADC input\n", machine().describe_context());
		m_adc_sample = 0x3ff;
	}
	else if (ADMUX_MUX == 0x0f)
	{
		// 0V (GND)
		m_adc_sample = 0;
	}
	else
	{
		logerror("%s: Using reserved ADC input 0x%X\n", machine().describe_context(), ADMUX_MUX);
		m_adc_sample = 0;
	}

	// wait for conversion to complete
	int const scale = 1 << std::max(ADCSRA_ADPS, 1);
	m_adc_timer->adjust(attotime::from_ticks((m_adc_first ? 25 : 13) * scale, clock()));
}

template <int NumTimers>
TIMER_CALLBACK_MEMBER(avr8_device<NumTimers>::adc_conversion_complete)
{
	// set conversion result
	m_adc_result = m_adc_sample;
	if (!m_adc_hold)
		m_adc_data = m_adc_result;

	// clear conversion in progress flag, set conversion interrupt flag
	m_r[ADCSRA] &= ~ADCSRA_ADSC_MASK;
	m_r[ADCSRA] |= ADCSRA_ADIF_MASK;

	// trigger another conversion if appropriate
	if (ADCSRA_ADATE && (ADCSRB_ADTS == 0))
		adc_start_conversion();
}

/************************************************************************************************/

/****************/
/* SPI Handling */
/****************/

template <int NumTimers>
void avr8_device<NumTimers>::enable_spi()
{
	// TODO
}

template <int NumTimers>
void avr8_device<NumTimers>::disable_spi()
{
	// TODO
}

template <int NumTimers>
void avr8_device<NumTimers>::spi_update_masterslave_select()
{
	// TODO
}

template <int NumTimers>
void avr8_device<NumTimers>::spi_update_clock_polarity()
{
	// TODO
}

template <int NumTimers>
void avr8_device<NumTimers>::spi_update_clock_phase()
{
	// TODO
}

template <int NumTimers>
void avr8_device<NumTimers>::spi_update_clock_rate()
{
	static const uint8_t s_spi_clock_divisor[8] =
	{
		4, 16, 64, 128, 2, 8, 32, 64
	};
	m_spi_prescale = s_spi_clock_divisor[SPI_RATE];
	m_spi_prescale_count &= m_spi_prescale - 1;
}

template <int NumTimers>
void avr8_device<NumTimers>::change_spcr(uint8_t data)
{
	uint8_t oldspcr = m_r[SPCR];
	uint8_t newspcr = data;
	uint8_t changed = newspcr ^ oldspcr;
	uint8_t high_to_low = ~newspcr & oldspcr;
	uint8_t low_to_high = newspcr & ~oldspcr;

	m_r[SPCR] = data;

	if (changed & SPCR_SPIE_MASK)
	{
		// Check for SPI interrupt condition
		update_interrupt(INTIDX_SPI);
	}

	if (low_to_high & SPCR_SPE_MASK)
	{
		enable_spi();
	}
	else if (high_to_low & SPCR_SPE_MASK)
	{
		disable_spi();
	}

	if (changed & SPCR_MSTR_MASK)
	{
		spi_update_masterslave_select();
	}

	if (changed & SPCR_CPOL_MASK)
	{
		spi_update_clock_polarity();
	}

	if (changed & SPCR_CPHA_MASK)
	{
		spi_update_clock_phase();
	}

	if (changed & SPCR_SPR_MASK)
	{
		spi_update_clock_rate();
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::change_spsr(uint8_t data)
{
	uint8_t oldspsr = m_r[SPSR];
	uint8_t newspsr = data;
	uint8_t changed = newspsr ^ oldspsr;

	m_r[SPSR] &= ~1;
	m_r[SPSR] |= data & 1;

	if (changed & SPSR_SPR2X_MASK)
	{
		spi_update_clock_rate();
	}
}

/*****************************************************************************/

template <int NumTimers>
template <int Port>
uint8_t avr8_device<NumTimers>::gpio_r()
{
	static constexpr uint16_t PORT_TO_REG[11] =
	{
		PORTA,
		PORTB,
		PORTC,
		PORTD,
		PORTE,
		PORTF,
		PORTG,
		PORTH,
		PORTJ,
		PORTK,
		PORTL
	};
	static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
	LOGMASKED(LOG_GPIO, "%s: Huh%c(%d) Read: %02x\n", machine().describe_context(), PORT_CHARS[Port], Port, m_r[PORT_TO_REG[Port]]);
	return m_r[PORT_TO_REG[Port]];
}

template <int NumTimers>
template <int Port>
uint8_t avr8_device<NumTimers>::pin_r()
{
	static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
	// TODO: account for DDRH
	const uint8_t data = m_gpio_in_cb[Port]();
	LOGMASKED(LOG_GPIO, "%s: PIN%c(%d) Read: %02x\n", machine().describe_context(), PORT_CHARS[Port], Port, data);
	return data;
}

template <int NumTimers>
template <int Port>
void avr8_device<NumTimers>::port_w(uint8_t data)
{
	static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
	static constexpr uint16_t PORT_TO_REG[11] =
	{
		PORTA,
		PORTB,
		PORTC,
		PORTD,
		PORTE,
		PORTF,
		PORTG,
		PORTH,
		PORTJ,
		PORTK,
		PORTL
	};

	LOGMASKED(LOG_GPIO, "%s: PORT%c Write: %02x\n", machine().describe_context(), PORT_CHARS[Port], data);
	m_r[PORT_TO_REG[Port]] = data;
	LOGMASKED(LOG_GPIO, "%s: PORT%c Write, XYZ m_r[%d[%d]] = %02x\n", machine().describe_context(), PORT_CHARS[Port], PORT_TO_REG[Port], Port, data);
	m_gpio_out_cb[Port](data);
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr0a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: TCCR0A = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR0A];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	m_r[TCCR0A] = data;

	if (changed & TCCR0A_WGM0_10_MASK)
	{
		update_timer_waveform_gen_mode(0, WGM0);
	}

	m_timer0_tick = m_timer0_ticks[(WGM0 << 2) | TCCR0A_COM0B];
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr0b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: TCCR0B = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR0B];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	const uint8_t oldcs = m_r[TCCR0B] & TCCR0B_CS_MASK;

	m_r[TCCR0B] = data;

	if (changed & TCCR0B_FOC0A_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_A);
	}

	if (changed & TCCR0B_FOC0B_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_B);
	}

	if (changed & TCCR0B_WGM0_2_MASK)
	{
		update_timer_waveform_gen_mode(0, WGM0);
	}

	if (changed & TCCR0B_CS_MASK)
	{
		update_timer_clock_source<0>(TIMER0_CLOCK_SELECT, oldcs);
	}

	m_timer0_tick = m_timer0_ticks[(WGM0 << 2) | TCCR0A_COM0B];
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr0a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: OCR0A = %02x\n", machine().describe_context(), data);
	update_ocr0(data, AVR8_REG_A);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr0b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: OCR0B = %02x\n", machine().describe_context(), data);
	update_ocr0(data, AVR8_REG_B);
}

template <int NumTimers>
void avr8_device<NumTimers>::tifr0_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: TIFR0 = %02x\n", machine().describe_context(), data);
	m_r[TIFR0] &= ~(data & TIFR0_MASK);
	update_interrupt(INTIDX_OCF0A);
	update_interrupt(INTIDX_OCF0B);
	update_interrupt(INTIDX_TOV0);
}

template <int NumTimers>
void avr8_device<NumTimers>::tifr1_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: TIFR1 = %02x\n", machine().describe_context(), data);
	m_r[TIFR1] &= ~(data & TIFR1_MASK);
	update_interrupt(INTIDX_ICF1);
	update_interrupt(INTIDX_OCF1A);
	update_interrupt(INTIDX_OCF1B);
	update_interrupt(INTIDX_TOV1);
}

template <int NumTimers>
void avr8_device<NumTimers>::tifr2_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER2, "%s: TIFR2 = %02x\n", machine().describe_context(), data);
	m_r[TIFR2] &= ~(data & TIFR2_MASK);
	update_interrupt(INTIDX_OCF2A);
	update_interrupt(INTIDX_OCF2B);
	update_interrupt(INTIDX_TOV2);
}

template <int NumTimers>
void avr8_device<NumTimers>::gtccr_w(uint8_t data)
{
	if (data & GTCCR_PSRASY_MASK)
	{
		data &= ~GTCCR_PSRASY_MASK;
		m_timer_prescale_count[2] = 0;
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::eecr_w(uint8_t data)
{
	m_r[EECR] = data & EECR_MASK;

	if (data & EECR_EERE_MASK)
	{
		uint16_t addr = (m_r[EEARH] & EEARH_MASK) << 8;
		addr |= m_r[EEARL];
		m_r[EEDR] = m_eeprom[addr];
		LOGMASKED(LOG_EEPROM, "%s: EEPROM read @ %04x data = %02x\n", machine().describe_context(), addr, m_eeprom[addr]);
	}
	if ((data & EECR_EEPE_MASK) && (data & EECR_EEMPE_MASK))
	{
		uint16_t addr = (m_r[EEARH] & EEARH_MASK) << 8;
		addr |= m_r[EEARL];
		m_eeprom[addr] = m_r[EEDR];
		LOGMASKED(LOG_EEPROM, "%s: EEPROM write @ %04x data = %02x ('%c')\n", machine().describe_context(), addr, m_eeprom[addr], m_eeprom[addr] >= 0x21 ? m_eeprom[addr] : ' ');

		// Indicate that we've finished writing a value to the EEPROM.
		// TODO: this should only happen after a certain dalay.
		m_r[EECR] = data & ~EECR_EEPE_MASK;
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::gpior0_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR0 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR0] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::gpior1_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR1 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR1] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::gpior2_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR2 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR2] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::spsr_w(uint8_t data)
{
	change_spsr(data);
}

template <int NumTimers>
void avr8_device<NumTimers>::spcr_w(uint8_t data)
{
	change_spcr(data);
}

template <int NumTimers>
void avr8_device<NumTimers>::spdr_w(uint8_t data)
{
	m_r[SPDR] = data;
	m_spi_active = true;
	m_spi_prescale_countdown = 7;
	m_spi_prescale_count = 0;
}

template <int NumTimers>
void avr8_device<NumTimers>::wdtcsr_w(uint8_t data)
{
	LOGMASKED(LOG_WDOG, "%s: (not yet implemented) WDTCSR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::clkpr_w(uint8_t data)
{
	LOGMASKED(LOG_CLOCK, "%s: (not yet implemented) CLKPR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::prr0_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR0 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::prr1_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR1 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::osccal_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) OSCCAL = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::pcicr_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PCICR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::eicra_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) EICRA = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::eicrb_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) EICRB = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::pcmsk0_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PCMSK0 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::pcmsk1_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PCMSK1 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::pcmsk2_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PCMSK2 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk0_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER0, "%s: TIMSK0 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK0] = data;
	update_interrupt(INTIDX_OCF0A);
	update_interrupt(INTIDX_OCF0B);
	update_interrupt(INTIDX_TOV0);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk1_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: TIMSK1 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK1] = data;
	update_interrupt(INTIDX_ICF1);
	update_interrupt(INTIDX_OCF1A);
	update_interrupt(INTIDX_OCF1B);
	update_interrupt(INTIDX_TOV1);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk2_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER2, "%s: TIMSK2 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK2] = data;
	update_interrupt(INTIDX_OCF2A);
	update_interrupt(INTIDX_OCF2B);
	update_interrupt(INTIDX_TOV2);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk3_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER3, "%s: TIMSK3 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK3] = data;
	update_interrupt(INTIDX_OCF3A);
	update_interrupt(INTIDX_OCF3B);
	update_interrupt(INTIDX_TOV3);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk4_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER4, "%s: TIMSK4 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK4] = data;
	update_interrupt(INTIDX_OCF4A);
	update_interrupt(INTIDX_OCF4B);
	update_interrupt(INTIDX_TOV4);
}

template <int NumTimers>
void avr8_device<NumTimers>::timsk5_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER5, "%s: TIMSK5 = %02x\n", machine().describe_context(), data);
	m_r[TIMSK5] = data;
	update_interrupt(INTIDX_OCF5A);
	update_interrupt(INTIDX_OCF5B);
	update_interrupt(INTIDX_TOV5);
}

template <int NumTimers>
void avr8_device<NumTimers>::xmcra_w(uint8_t data)
{
	LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRA = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::xmcrb_w(uint8_t data)
{
	LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRB = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
uint8_t avr8_device<NumTimers>::adcl_r()
{
	if (!machine().side_effects_disabled())
		m_adc_hold = true;
	if (ADMUX_ADLAR)
		return (m_adc_data & 0x03) << 6;
	else
		return uint8_t(m_adc_data);
}

template <int NumTimers>
void avr8_device<NumTimers>::adcl_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCL = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
uint8_t avr8_device<NumTimers>::adch_r()
{
	uint8_t const result = ADMUX_ADLAR ? BIT(m_adc_data, 2, 8) : BIT(m_adc_data, 8, 2);
	if (!machine().side_effects_disabled())
	{
		m_adc_data = m_adc_result;
		m_adc_hold = false;
	}
	return result;
}

template <int NumTimers>
void avr8_device<NumTimers>::adch_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCH = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::adcsra_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCSRA = %02x\n", machine().describe_context(), data);

	// set auto trigger enable, interrupt enable, and prescaler directly
	m_r[ADCSRA] &= ~(ADCSRA_ADATE_MASK | ADCSRA_ADIE_MASK | ADCSRA_ADPS_MASK);
	m_r[ADCSRA] |= data & (ADCSRA_ADATE_MASK | ADCSRA_ADIE_MASK | ADCSRA_ADPS_MASK);

	// check enable bit
	if (!(data & ADCSRA_ADEN_MASK))
	{
		// disable ADC, terminate any conversion in progress
		m_r[ADCSRA] &= ~(ADCSRA_ADEN_MASK | ADCSRA_ADSC_MASK);
		m_adc_timer->reset();
	}
	else
	{
		// first conversion after initial enable takes longer
		if (!ADCSRA_ADEN)
		{
			m_adc_first = true;
			m_r[ADCSRA] |= ADCSRA_ADEN_MASK;
		}

		// trigger conversion if necessary
		if (!ADCSRA_ADSC && (data & ADCSRA_ADSC_MASK))
			adc_start_conversion();
	}

	// writing with ADIF set clears the interrupt flag manually
	if (data & ADCSRA_ADIF_MASK)
		m_r[ADCSRA] &= ~ADCSRA_ADIF_MASK;

	if (ADCSRA_ADIE)
		logerror("%s: Unimplemented ADC interrupt enabled\n");
}

template <int NumTimers>
void avr8_device<NumTimers>::adcsrb_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCSRB = %02x\n", machine().describe_context(), data);
	m_r[ADCSRB] = data & (ADCSRB_ACME_MASK | ADCSRB_ADTS_MASK);
	if (ADCSRB_ADTS != 0)
		logerror("%s: Unimplemented ADC auto trigger source %X selected\n", machine().describe_context(), ADCSRB_ADTS);
}

template <int NumTimers>
void avr8_device<NumTimers>::admux_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADMUX = %02x\n", machine().describe_context(), data);
	m_r[ADMUX] = data & (ADMUX_REFS_MASK | ADMUX_ADLAR_MASK | ADMUX_MUX_MASK);
}

template <int NumTimers>
void avr8_device<NumTimers>::didr0_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR0 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::didr1_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR1 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::didr2_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR2 = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr1a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: TCCR1A = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR1A];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	m_r[TCCR1A] = newtccr;

	if (changed & TCCR1A_WGM1_10_MASK)
	{
		update_timer_waveform_gen_mode(1, WGM1);
	}

	m_timer1_tick = m_timer1_ticks[(WGM1 << 4) | ((m_r[TCCR1A] & TCCR1A_COM1AB_MASK) >> TCCR1A_COM1AB_SHIFT)];
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr1b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: TCCR1B = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR1B];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	const uint8_t oldcs = m_r[TCCR1B] & TCCR1B_CS_MASK;

	m_r[TCCR1B] = newtccr;

	if (changed & TCCR1B_ICNC1_MASK)
	{
		update_timer1_input_noise_canceler();
	}

	if (changed & TCCR1B_ICES1_MASK)
	{
		update_timer1_input_edge_select();
	}

	if (changed & TCCR1B_WGM1_32_MASK)
	{
		update_timer_waveform_gen_mode(1, WGM1);
	}

	if (changed & TCCR1B_CS_MASK)
	{
		update_timer_clock_source<1>(TIMER1_CLOCK_SELECT, oldcs);
	}

	m_timer1_tick = m_timer1_ticks[(WGM1 << 4) | ((m_r[TCCR1A] & TCCR1A_COM1AB_MASK) >> TCCR1A_COM1AB_SHIFT)];
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr1c_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: (not yet implemented) TCCR1C = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::icr1l_w(uint8_t data)
{
	m_r[ICR1L] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::icr1h_w(uint8_t data)
{
	m_r[ICR1H] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1al_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1AL = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1A & 0xff00) | data, AVR8_REG_A);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1ah_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1AH = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1A & 0x00ff) | (data << 8), AVR8_REG_A);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1bl_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1BL = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1B & 0xff00) | data, AVR8_REG_B);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1bh_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1BH = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1B & 0x00ff) | (data << 8), AVR8_REG_B);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1cl_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1CL = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1C & 0xff00) | data, AVR8_REG_C);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr1ch_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER1, "%s: OCR1CH = %02x\n", machine().describe_context(), data);
	update_ocr1((OCR1C & 0x00ff) | (data << 8), AVR8_REG_C);
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr2a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER2, "%s: TCCR2A = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR2A];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;

	m_r[TCCR2A] = data;

	if (changed & TCCR2A_WGM2_10_MASK)
	{
		update_timer_waveform_gen_mode(2, WGM2);
	}

	m_timer2_tick = m_timer2_ticks[((m_r[TCCR2B] & TCCR2B_WGM2_2_MASK) >> 1) | (m_r[TCCR2A] & TCCR2A_WGM2_10_MASK)];
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr2b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER2, "%s: TCCR2B = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR2B];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	const uint8_t oldcs = m_r[TCCR2B] & TCCR2B_CS_MASK;

	m_r[TCCR2B] = data;

	if (changed & TCCR2B_FOC2A_MASK)
	{
		timer2_force_output_compare(AVR8_REG_A);
	}

	if (changed & TCCR2B_FOC2B_MASK)
	{
		timer2_force_output_compare(AVR8_REG_B);
	}

	if (changed & TCCR2B_WGM2_2_MASK)
	{
		update_timer_waveform_gen_mode(2, WGM2);
	}

	if (changed & TCCR2B_CS_MASK)
	{
		update_timer_clock_source<2>(TIMER2_CLOCK_SELECT, oldcs);
	}

	m_timer2_tick = m_timer2_ticks[((m_r[TCCR2B] & TCCR2B_WGM2_2_MASK) >> 1) | (m_r[TCCR2A] & TCCR2A_WGM2_10_MASK)];
}

template <int NumTimers>
void avr8_device<NumTimers>::tcnt2_w(uint8_t data)
{
	m_r[TCNT2] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr2a_w(uint8_t data)
{
	update_ocr2(data, AVR8_REG_A);
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr2b_w(uint8_t data)
{
	update_ocr2(data, AVR8_REG_B);
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr3a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER3, "%s: (not yet implemented) TCCR3A = %02x\n", machine().describe_context(), data);
	// TODO
	//  AVR8_TCCR3A = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr3b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER3, "%s: (not yet implemented) TCCR3B = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr3c_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER3, "%s: (not yet implemented) TCCR3C = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::icr3l_w(uint8_t data)
{
	m_r[ICR3L] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::icr3h_w(uint8_t data)
{
	m_r[ICR3H] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3al_w(uint8_t data)
{
	m_r[OCR3AL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3ah_w(uint8_t data)
{
	m_r[OCR3AH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3bl_w(uint8_t data)
{
	m_r[OCR3BL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3bh_w(uint8_t data)
{
	m_r[OCR3BH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3cl_w(uint8_t data)
{
	m_r[OCR3CL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr3ch_w(uint8_t data)
{
	m_r[OCR3CH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr4a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER4, "%s: TCCR4A = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR4A];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;

	m_r[TCCR4A] = data;

	if (changed & TCCR4A_WGM4_10_MASK)
	{
		update_timer_waveform_gen_mode(4, WGM4);
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr4b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER4, "%s: TCCR4B = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR4B];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	const uint8_t oldcs = (m_r[TCCR4B] & TCCR4B_CS_MASK) >> TCCR4B_CS_SHIFT;

	m_r[TCCR4B] = data;

	if (changed & TCCR4B_FOC4A_MASK)
	{
		// TODO
		// timer4_force_output_compare(AVR8_REG_A);
	}

	if (changed & TCCR4B_FOC4B_MASK)
	{
		// TODO
		// timer4_force_output_compare(AVR8_REG_B);
	}

	if (changed & TCCR4B_WGM4_32_MASK)
	{
		update_timer_waveform_gen_mode(4, WGM4);
	}

	if (changed & TCCR4B_CS_MASK)
	{
		update_timer_clock_source<4>(TIMER4_CLOCK_SELECT, oldcs);
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr4c_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER4, "%s: TCCR4C = %02x\n", machine().describe_context(), data);
	//  uint8_t oldtccr = m_r[TCCR4C];
	//  uint8_t newtccr = data;
	//  uint8_t changed = newtccr ^ oldtccr;

	m_r[TCCR4C] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::tcnt4l_w(uint8_t data)
{
	m_r[TCNT4L] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::tcnt4h_w(uint8_t data)
{
	m_r[TCNT4H] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::icr4l_w(uint8_t data)
{
	m_r[ICR4L] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::icr4h_w(uint8_t data)
{
	m_r[ICR4H] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4al_w(uint8_t data)
{
	m_r[OCR4AL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4ah_w(uint8_t data)
{
	m_r[OCR4AH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4bl_w(uint8_t data)
{
	m_r[OCR4BL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4bh_w(uint8_t data)
{
	m_r[OCR4BH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4cl_w(uint8_t data)
{
	m_r[OCR4CL] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::ocr4ch_w(uint8_t data)
{
	m_r[OCR4CH] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr5a_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER5, "%s: TCCR5A = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR5A];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;

	m_r[TCCR5A] = data;

	if (changed & TCCR5A_WGM5_10_MASK)
	{
		update_timer_waveform_gen_mode(5, WGM5);
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::tccr5b_w(uint8_t data)
{
	LOGMASKED(LOG_TIMER5, "%s: TCCR5B = %02x\n", machine().describe_context(), data);
	const uint8_t oldtccr = m_r[TCCR5B];
	const uint8_t newtccr = data;
	const uint8_t changed = newtccr ^ oldtccr;
	const uint8_t oldcs = (m_r[TCCR5B] & TCCR5B_CS_MASK) >> TCCR5B_CS_SHIFT;

	m_r[TCCR5B] = data;

	if (changed & TCCR5C_FOC5A_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_A);
	}

	if (changed & TCCR5C_FOC5B_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_B);
	}

	if (changed & TCCR5C_FOC5C_MASK)
	{
		// TODO
		// timer5_force_output_compare(AVR8_REG_C);
	}

	if (changed & TCCR5B_WGM5_32_MASK)
	{
		update_timer_waveform_gen_mode(5, WGM5);
	}

	if (changed & TCCR5B_CS_MASK)
	{
		update_timer_clock_source<5>(TIMER5_CLOCK_SELECT, oldcs);
	}
}

template <int NumTimers>
void avr8_device<NumTimers>::assr_w(uint8_t data)
{
	LOGMASKED(LOG_ASYNC, "%s: (not yet implemented) ASSR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::twbr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWBR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
uint8_t avr8_device<NumTimers>::twsr_r()
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) Read TWSR: %02x\n", machine().describe_context(), 0);
	// HACK for Replicator 1:
	//   By returning a value != 0x08, we induce an error state that makes the object code jump out of the wait loop,
	//   and continue execution failing the 2-wire write operation.
	return 0x00;
}

template <int NumTimers>
void avr8_device<NumTimers>::twsr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWSR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::twar_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::twdr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWDR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::twcr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWCR = %02x\n", machine().describe_context(), data);
	m_r[TWCR] = data;
}

template <int NumTimers>
void avr8_device<NumTimers>::twamr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAMR = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::ucsr0a_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0A = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::ucsr0b_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0B = %02x\n", machine().describe_context(), data);
}

template <int NumTimers>
void avr8_device<NumTimers>::ucsr0c_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0C = %02x\n", machine().describe_context(), data);
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

#include "avr8ops.hxx"

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

template <int NumTimers>
void avr8_device<NumTimers>::execute_run()
{
	while (m_icount > 0)
	{
		m_pc &= m_addr_mask;
		debugger_instruction_hook(m_pc);

		const uint16_t op = (uint32_t)m_program->read_word(m_pc);
		m_opcycles = m_op_cycles[op];
		((this)->*(m_op_funcs[op]))(op);
		m_pc += 2;

		m_icount -= m_opcycles;

		for (int i = 0; i < m_opcycles; i++)
		{
			if (m_spi_active && m_spi_prescale > 0)
			{
				if (m_spi_prescale_countdown >= 0)
				{
					m_spi_prescale_count++;
					if (m_spi_prescale_count >= m_spi_prescale)
					{
						spi_tick();
						m_spi_prescale_count -= m_spi_prescale;
					}
				}
			}

			m_timer_prescale_count[0]++;
			if (m_timer_prescale_count[0] > m_timer_prescale[0])
				m_timer0_tick();

			m_timer_prescale_count[1]++;
			if (m_timer_prescale_count[1] > m_timer_prescale[1])
				m_timer1_tick();

			m_timer_prescale_count[2]++;
			if (m_timer_prescale_count[2] > m_timer_prescale[2])
				m_timer2_tick();

			if (NumTimers > 5)
			{
				m_timer_prescale_count[3]++;
				if (m_timer_prescale_count[3] > m_timer_prescale[3])
				{
					timer3_tick();
					m_timer_prescale_count[3] -= m_timer_prescale[3];
				}

				m_timer_prescale_count[4]++;
				if (m_timer_prescale_count[4] > m_timer_prescale[4])
				{
					timer4_tick();
					m_timer_prescale_count[4] -= m_timer_prescale[4];
				}

				m_timer_prescale_count[5]++;
				if (m_timer_prescale_count[5] > m_timer_prescale[5])
				{
					timer5_tick();
					m_timer_prescale_count[5] -= m_timer_prescale[5];
				}
			}
		}
	}
}

template class avr8_device<2>;
template class avr8_device<3>;
template class avr8_device<6>;
