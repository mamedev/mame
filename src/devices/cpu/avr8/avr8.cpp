// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Sandro Ronco, Felipe Sanches
/***************************************************************************

    Atmel 8-bit AVR simulator

    - Notes -
      Cycle counts are generally considered to be 100% accurate per-instruction, does not support mid-instruction
      interrupts although no software has been countered yet that requires it. Evidence of cycle accuracy is given
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
#include "debugger.h"
#include "avr8.h"

#define VERBOSE_LEVEL   (0)

#define ENABLE_VERBOSE_LOG (0)

#if ENABLE_VERBOSE_LOG
static inline void ATTR_PRINTF(3,4) verboselog(UINT16 pc, int n_level, const char *s_fmt, ...)
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
#define verboselog(x,y,z, ...)
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
	AVR8_SREG_I
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

//static const char avr8_reg_name[4] = { 'A', 'B', 'C', 'D' };

#define SREG_R(b) ((m_r[AVR8_REGIDX_SREG] & (1 << (b))) >> (b))
#define SREG_W(b,v) m_r[AVR8_REGIDX_SREG] = (m_r[AVR8_REGIDX_SREG] & ~(1 << (b))) | ((v) << (b))
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

const device_type ATMEGA88 = &device_creator<atmega88_device>;
const device_type ATMEGA644 = &device_creator<atmega644_device>;
const device_type ATMEGA1280 = &device_creator<atmega1280_device>;
const device_type ATMEGA2560 = &device_creator<atmega2560_device>;

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

static ADDRESS_MAP_START( atmega88_internal_map, AS_DATA, 8, atmega88_device )
	AM_RANGE(0x0000, 0x00ff) AM_READWRITE( regs_r, regs_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( atmega644_internal_map, AS_DATA, 8, atmega644_device )
	AM_RANGE(0x0000, 0x00ff) AM_READWRITE( regs_r, regs_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( atmega1280_internal_map, AS_DATA, 8, atmega1280_device )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE( regs_r, regs_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( atmega2560_internal_map, AS_DATA, 8, atmega2560_device )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE( regs_r, regs_w )
ADDRESS_MAP_END

//-------------------------------------------------
//  atmega88_device - constructor
//-------------------------------------------------

atmega88_device::atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: avr8_device(mconfig, "ATMEGA88", tag, owner, clock, ATMEGA88, 0x0fff, ADDRESS_MAP_NAME(atmega88_internal_map), CPU_TYPE_ATMEGA88, "atmega88", __FILE__)
{
}

//-------------------------------------------------
//  atmega644_device - constructor
//-------------------------------------------------

atmega644_device::atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: avr8_device(mconfig, "ATMEGA644", tag, owner, clock, ATMEGA644, 0xffff, ADDRESS_MAP_NAME(atmega644_internal_map), CPU_TYPE_ATMEGA644, "atmega644", __FILE__)
{
}

//-------------------------------------------------
//  atmega1280_device - constructor
//-------------------------------------------------

atmega1280_device::atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: avr8_device(mconfig, "ATMEGA1280", tag, owner, clock, ATMEGA1280, 0x1ffff, ADDRESS_MAP_NAME(atmega1280_internal_map), CPU_TYPE_ATMEGA1280, "atmega1280", __FILE__)
{
}

//-------------------------------------------------
//  atmega2560_device - constructor
//-------------------------------------------------

atmega2560_device::atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: avr8_device(mconfig, "ATMEGA2560", tag, owner, clock, ATMEGA2560, 0x1ffff, ADDRESS_MAP_NAME(atmega2560_internal_map), CPU_TYPE_ATMEGA2560, "atmega2560", __FILE__)
{
}

//-------------------------------------------------
//  avr8_device - constructor
//-------------------------------------------------

avr8_device::avr8_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock, const device_type type, UINT32 addr_mask, address_map_constructor internal_map, UINT8 cpu_type, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_shifted_pc(0),
		m_program_config("program", ENDIANNESS_LITTLE, 8, 22),
		m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0, internal_map),
		m_io_config("io", ENDIANNESS_LITTLE, 8, 4),
		m_eeprom_tag(nullptr),
		m_eeprom(nullptr),
		m_cpu_type(cpu_type),
		m_lfuses(0x62),
		m_hfuses(0x99),
		m_efuses(0xFF),
		m_lock_bits(0xFF),
		m_pc(0),
		m_spi_active(false),
		m_spi_prescale(0),
		m_spi_prescale_count(0),
		m_addr_mask(addr_mask),
		m_interrupt_pending(false),
		m_elapsed_cycles(0)
{
	// Allocate & setup

	for (int t=0; t<=5; t++){
		m_timer_top[t] = 0;
		m_timer_increment[t] = 1;
		m_timer_prescale[t] = 0;
		m_timer_prescale_count[t] = 0;
	}
}


//-------------------------------------------------
//  static_set_low_fuses
//-------------------------------------------------

void avr8_device::set_low_fuses(const UINT8 byte)
{
	m_lfuses = byte;
}

//-------------------------------------------------
//  static_set_high_fuses
//-------------------------------------------------

void avr8_device::set_high_fuses(const UINT8 byte)
{
	m_hfuses = byte;
}

//-------------------------------------------------
//  static_set_extended_fuses
//-------------------------------------------------

void avr8_device::set_extended_fuses(const UINT8 byte)
{
	m_efuses = byte;
}

//-------------------------------------------------
//  static_set_lock_bits
//-------------------------------------------------

void avr8_device::set_lock_bits(const UINT8 byte)
{
	m_lock_bits = byte;
}

//-------------------------------------------------
//  unimplemented_opcode - bail on unspuported
//  instruction
//-------------------------------------------------

void avr8_device::unimplemented_opcode(UINT32 op)
{
//  debugger_break(machine());
	fatalerror("AVR8: unknown opcode (%08x) at %08x\n", op, m_shifted_pc);
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
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_shifted_pc).noshow();
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
	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_timer_top[0]));
	save_item(NAME(m_timer_increment[0]));
	save_item(NAME(m_timer_prescale[0]));
	save_item(NAME(m_timer_prescale_count[0]));
	save_item(NAME(m_timer_top[1]));
	save_item(NAME(m_timer_increment[1]));
	save_item(NAME(m_timer_prescale[1]));
	save_item(NAME(m_timer_prescale_count[1]));
	save_item(NAME(m_timer_top[2]));
	save_item(NAME(m_timer_increment[2]));
	save_item(NAME(m_timer_prescale[2]));
	save_item(NAME(m_timer_prescale_count[2]));
	save_item(NAME(m_timer_top[3]));
	save_item(NAME(m_timer_increment[3]));
	save_item(NAME(m_timer_prescale[3]));
	save_item(NAME(m_timer_prescale_count[3]));
	save_item(NAME(m_timer_top[4]));
	save_item(NAME(m_timer_increment[4]));
	save_item(NAME(m_timer_prescale[4]));
	save_item(NAME(m_timer_prescale_count[4]));
	save_item(NAME(m_timer_top[5]));
	save_item(NAME(m_timer_increment[5]));
	save_item(NAME(m_timer_prescale[5]));
	save_item(NAME(m_timer_prescale_count[5]));
	save_item(NAME(m_addr_mask));
	save_item(NAME(m_interrupt_pending));
	save_item(NAME(m_elapsed_cycles));

	// set our instruction counter
	m_icountptr = &m_icount;

	m_eeprom = machine().root_device().memregion(m_eeprom_tag)->base();
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

	switch ((m_hfuses & (BOOTSZ1|BOOTSZ0)) >> 1){
	case 0: m_boot_size = 4096; break;
	case 1: m_boot_size = 2048; break;
	case 2: m_boot_size = 1024; break;
	case 3: m_boot_size = 512; break;
	default: break;
	}

	if (m_hfuses & BOOTRST){
	m_shifted_pc = 0x0000;
	logerror("Booting AVR core from address 0x0000\n");
	} else {
	m_shifted_pc = (m_addr_mask + 1) - 2*m_boot_size;
	logerror("AVR Boot loader section size: %d words\n", m_boot_size);
	}

	for (auto & elem : m_r)
	{
		elem = 0;
	}

	m_spi_active = false;
	m_spi_prescale = 0;
	m_spi_prescale_count = 0;

	for (int t=0; t<=5; t++){
		m_timer_top[t] = 0;
		m_timer_increment[t] = 1;
		m_timer_prescale[t] = 0;
		m_timer_prescale_count[t] = 0;
	}

	m_ocr2_not_reached_yet = true;
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
	else if (spacenum == AS_DATA)
	{
		return &m_data_config;
	}
	else if (spacenum == AS_IO)
	{
		return &m_io_config;
	}
	return nullptr;
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
			strprintf(str, "%c%c%c%c%c%c%c%c",
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
	return CPU_DISASSEMBLE_NAME(avr8)(this, buffer, pc, oprom, opram, options);
}


//**************************************************************************
//  MEMORY ACCESSORS
//**************************************************************************

inline void avr8_device::push(UINT8 val)
{
	UINT16 sp = SPREG;
	m_data->write_byte(sp, val);
	sp--;
	m_r[AVR8_REGIDX_SPL] = sp & 0x00ff;
	m_r[AVR8_REGIDX_SPH] = (sp >> 8) & 0x00ff;
}

inline UINT8 avr8_device::pop()
{
	UINT16 sp = SPREG;
	sp++;
	m_r[AVR8_REGIDX_SPL] = sp & 0x00ff;
	m_r[AVR8_REGIDX_SPH] = (sp >> 8) & 0x00ff;
	return m_data->read_byte(sp);
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
	const CInterruptCondition &condition = s_int_conditions[source];

	int intstate = 0;
	if (m_r[condition.m_intreg] & condition.m_intmask)
		intstate = (m_r[condition.m_regindex] & condition.m_regmask) ? 1 : 0;

	set_irq_line(condition.m_intindex, intstate);

	if (intstate)
	{
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}
}


static const CInterruptCondition s_mega644_int_conditions[AVR8_INTIDX_COUNT] =
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
	const CInterruptCondition &condition = s_mega644_int_conditions[source];

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
	const CInterruptCondition &condition = s_mega644_int_conditions[source];

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
	const CInterruptCondition &condition = s_mega644_int_conditions[source];

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
void avr8_device::timer_tick(int cycles)
{
	for(int count = 0; count < cycles; count++)
	{
		m_elapsed_cycles++;

		if (m_spi_active && m_spi_prescale > 0 && m_spi_prescale_countdown >= 0)
		{
			m_spi_prescale_count++;
			if (m_spi_prescale_count >= m_spi_prescale)
			{
				UINT8 out_bit = (m_r[AVR8_REGIDX_SPDR] & (1 << m_spi_prescale_countdown)) >> m_spi_prescale_countdown;
				m_spi_prescale_countdown--;
				m_io->write_byte(AVR8_IO_PORTB, (m_r[AVR8_REGIDX_PORTB] &~ AVR8_PORTB_MOSI) | (out_bit ? AVR8_PORTB_MOSI : 0));
				m_r[AVR8_REGIDX_PORTB] = (m_r[AVR8_REGIDX_PORTB] &~ AVR8_PORTB_MOSI) | (out_bit ? AVR8_PORTB_MOSI : 0);
				m_spi_prescale_count -= m_spi_prescale;
			}
		}

	for (int t=0; t<=5; t++){
			if (m_timer_prescale[t] != 0)
			{
				m_timer_prescale_count[t]++;
				if (m_timer_prescale_count[t] >= m_timer_prescale[t])
				{
			switch (t){
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

//  UINT8 ocr0[2] = { m_r[AVR8_REGIDX_OCR0A], m_r[AVR8_REGIDX_OCR0B] };
//TODO  UINT8 ocf0[2] = { (1 << AVR8_TIFR0_OCF0A_SHIFT), (1 << AVR8_TIFR4_OCF0B_SHIFT) };
//TODO  UINT8 int0[2] = { AVR8_INTIDX_OCF0A, AVR8_INTIDX_OCF0B };

#define LOG_TIMER_0 0
#define LOG_TIMER_5 0
// Timer 0 Handling
void avr8_device::timer0_tick()
{
#if LOG_TIMER_0
	printf("AVR8_WGM0: %d\n", AVR8_WGM0);
	printf("AVR8_TCCR0A_COM0B: %d\n", AVR8_TCCR0A_COM0B);
#endif

	UINT8 count = m_r[AVR8_REGIDX_TCNT0];
	INT32 increment = m_timer_increment[0];

	switch(AVR8_WGM0)
	{
		case WGM02_NORMAL:
		//printf("WGM02_NORMAL: Unimplemented timer#0 waveform generation mode\n");
		break;

		case WGM02_PWM_PC:
		//printf("WGM02_PWM_PC: Unimplemented timer#0 waveform generation mode\n");
		break;

		case WGM02_CTC_CMP:
		switch(AVR8_TCCR0A_COM0B){
		case 0: /* Normal Operation */
			if (count == m_timer_top[0]){
			m_timer_top[0] = 0;
			}
			break;
		case 1: /* Toggle OC0B on compare match */
			if (count == m_timer_top[0]){
			m_timer_top[0] = 0;
#if LOG_TIMER_0
			printf("[0] Toggle OC0B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) ^ (1 << 5));
			}
			break;
		case 2: /* Clear OC0B on compare match */
			if (count == m_timer_top[0]){
			m_timer_top[0] = 0;
			//Clear OC0B
#if LOG_TIMER_0
			printf("[0] Clear OC0B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) & ~(1 << 5));
			}
			break;
		case 3: /* Set OC0B on compare match */
			if (count == m_timer_top[0]){
			m_timer_top[0] = 0;
#if LOG_TIMER_0
			printf("[0] Set OC0B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) | (1 << 5));
			}
			break;
		}
		break;

		case WGM02_FAST_PWM:
		printf("WGM02_FAST_PWM: Unimplemented timer#0 waveform generation mode\n");
		break;

		case WGM02_PWM_PC_CMP:
		printf("WGM02_PWM_PC_CMP: Unimplemented timer#0 waveform generation mode\n");
		break;

		case WGM02_FAST_PWM_CMP:
		printf("WGM02_FAST_PWM_CMP: Unimplemented timer#0 waveform generation mode\n");
		break;

		default:
			verboselog(m_pc, 0, "update_timer0_compare_mode: Unknown waveform generation mode: %02x\n", AVR8_WGM0);
			break;
	}

	count = count & 0xff;

	count += increment;
	m_r[AVR8_REGIDX_TCNT0] = count & 0xff;
}

void avr8_device::changed_tccr0a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR0A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR0A = data;

	if(changed & AVR8_TCCR0A_WGM0_10_MASK)
	{
		update_timer_waveform_gen_mode(0, AVR8_WGM0);
	}
}

void avr8_device::timer0_force_output_compare(int reg)
{
	// TODO
	verboselog(m_pc, 0, "timer0_force_output_compare: TODO; should be forcing OC0%c\n", avr8_reg_name[reg]);
}

void avr8_device::changed_tccr0b(UINT8 data)
{
	if (VERBOSE_LEVEL) printf("changed_tccr0b: data=0x%02X\n", data);

	UINT8 oldtccr = AVR8_TCCR0B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR0B = data;

	if(changed & AVR8_TCCR0B_FOC0A_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_A);
	}

	if(changed & AVR8_TCCR0B_FOC0B_MASK)
	{
		// TODO
		timer0_force_output_compare(AVR8_REG_B);
	}

	if(changed & AVR8_TCCR0B_WGM0_2_MASK)
	{
		update_timer_waveform_gen_mode(0, AVR8_WGM0);
	}

	if(changed & AVR8_TCCR0B_CS_MASK)
	{
		update_timer_clock_source(0, AVR8_TIMER0_CLOCK_SELECT);
	}
}

void avr8_device::update_ocr0(UINT8 newval, UINT8 reg)
{
	m_r[(reg == AVR8_REG_A) ? AVR8_REGIDX_OCR0A : AVR8_REGIDX_OCR0B] = newval;
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
	UINT16 ocr1[2] = { static_cast<UINT16>((m_r[AVR8_REGIDX_OCR1AH] << 8) | m_r[AVR8_REGIDX_OCR1AL]),
						static_cast<UINT16>((m_r[AVR8_REGIDX_OCR1BH] << 8) | m_r[AVR8_REGIDX_OCR1BL]) };
	UINT8 ocf1[2] = { (1 << AVR8_TIFR1_OCF1A_SHIFT), (1 << AVR8_TIFR1_OCF1B_SHIFT) };
	UINT8 int1[2] = { AVR8_INTIDX_OCF1A, AVR8_INTIDX_OCF1B };
	INT32 increment = m_timer_increment[1];

	for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
	{
		switch(wgm1)
		{
			case WGM1_CTC_OCR:
				if (count == 0xffff)
				{
					m_r[AVR8_REGIDX_TIFR1] |= AVR8_TIFR1_TOV1_MASK;
					update_interrupt(AVR8_INTIDX_TOV1);
					count = 0;
					increment = 0;
				}

				if (count == ocr1[reg])
				{
					if (reg == 0)
					{
						count = 0;
						increment = 0;
					}
					m_r[AVR8_REGIDX_TIFR1] |= ocf1[reg];
					update_interrupt(int1[reg]);
				}
				else if (count == 0)
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
				verboselog(m_pc, 0, "update_timer1_compare_mode: Unknown waveform generation mode: %02x\n", wgm1);
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

void avr8_device::update_timer_waveform_gen_mode(UINT8 t, UINT8 mode)
{
	INT32 oc_val = -1, ic_val = -1;

	switch (t){
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

	INT32 top_values_02[8] = {0xFF, 0xFF, oc_val, 0xFF, -1, oc_val, -1, oc_val}; //table 20-8

	INT32 top_values_1345[16] = {0xFFFF, 0x00FF, 0x01FF, 0x03FF,
							oc_val, 0x00FF, 0x01FF, 0x03FF,
							ic_val, oc_val, ic_val, oc_val,
							ic_val, -1,     ic_val, oc_val}; //table 17-2

	switch(t){
	case 0:
	case 2:
		m_timer_top[t] = top_values_02[mode];
		break;
	case 1:
	case 3:
	case 4:
	case 5:
		m_timer_top[t] = top_values_1345[mode];
		break;
	}

	if (m_timer_top[t] == -1){
	m_timer_top[t] = 0;
		printf("update_timer_waveform_gen_mode: Timer #%d - Unsupported waveform generation type: %d\n", t, mode);
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
		update_timer_waveform_gen_mode(1, AVR8_WGM1);
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

void avr8_device::changed_tccr1b(UINT8 data)
{
	if (VERBOSE_LEVEL) printf("changed_tccr1b: data=0x%02X\n", data);

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
		update_timer_waveform_gen_mode(1, AVR8_WGM1);
	}

	if(changed & AVR8_TCCR1B_CS_MASK)
	{
		update_timer_clock_source(1, AVR8_TIMER1_CLOCK_SELECT);
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
	INT32 increment = m_timer_increment[2];

	for(INT32 reg = AVR8_REG_A; reg <= AVR8_REG_B; reg++)
	{
		switch(wgm2)
		{
			case WGM02_FAST_PWM:
				if (reg==0)
				{
					if (count >= m_r[AVR8_REGIDX_OCR2A])
					{
						if (count >= 0xFF)
						{
							//Turn on
							m_io->write_byte(AVR8_IO_PORTD, m_io->read_byte(AVR8_IO_PORTD) | (1 << 7));
							m_r[AVR8_REGIDX_TCNT2] = 0;
							m_ocr2_not_reached_yet = true;
						}
						else
						{
							if (m_ocr2_not_reached_yet)
							{
								//Turn off
								m_io->write_byte(AVR8_IO_PORTD, m_io->read_byte(AVR8_IO_PORTD) & ~(1 << 7));
								m_ocr2_not_reached_yet = false;
							}
						}
					}
				}
				break;

			case WGM02_FAST_PWM_CMP:
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
				else if (count == 0)
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

	m_r[AVR8_REGIDX_TCNT2] += increment;

	update_interrupt(AVR8_INTIDX_OCF2A);
	update_interrupt(AVR8_INTIDX_OCF2B);
	update_interrupt(AVR8_INTIDX_TOV2);
}

void avr8_device::changed_tccr2a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR2A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR2A = data;

	if(changed & AVR8_TCCR2A_WGM2_10_MASK)
	{
		// TODO
		update_timer_waveform_gen_mode(2, AVR8_WGM2);
	}
}

void avr8_device::timer2_force_output_compare(int reg)
{
	// TODO
	verboselog(m_pc, 0, "force_output_compare: TODO; should be forcing OC2%c\n", avr8_reg_name[reg]);
}

void avr8_device::changed_tccr2b(UINT8 data)
{
	if (VERBOSE_LEVEL) printf("changed_tccr2b: data=0x%02X\n", data);

	UINT8 oldtccr = AVR8_TCCR2B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR2B = data;

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
		update_timer_waveform_gen_mode(2, AVR8_WGM2);
	}

	if(changed & AVR8_TCCR2B_CS_MASK)
	{
		update_timer_clock_source(2, AVR8_TIMER2_CLOCK_SELECT);
	}
}

void avr8_device::update_ocr2(UINT8 newval, UINT8 reg)
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
//  printf("AVR8_WGM4: %d\n", AVR8_WGM4);
//  printf("AVR8_TCCR4A_COM4B: %d\n", AVR8_TCCR4A_COM4B);

	UINT16 count = (m_r[AVR8_REGIDX_TCNT4H] << 8) | m_r[AVR8_REGIDX_TCNT4L];

	// Cache things in array form to avoid a compare+branch inside a potentially high-frequency timer
	//UINT8 compare_mode[2] = { (m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1A_MASK) >> AVR8_TCCR1A_COM1A_SHIFT,
								//(m_r[AVR8_REGIDX_TCCR1A] & AVR8_TCCR1A_COM1B_MASK) >> AVR8_TCCR1A_COM1B_SHIFT };
	UINT16 ocr4[2] = { static_cast<UINT16>((m_r[AVR8_REGIDX_OCR4AH] << 8) | m_r[AVR8_REGIDX_OCR4AL]),
						static_cast<UINT16>((m_r[AVR8_REGIDX_OCR4BH] << 8) | m_r[AVR8_REGIDX_OCR4BL]) };
//TODO  UINT8 ocf4[2] = { (1 << AVR8_TIFR4_OCF4A_SHIFT), (1 << AVR8_TIFR4_OCF4B_SHIFT) };
//TODO  UINT8 int4[2] = { AVR8_INTIDX_OCF4A, AVR8_INTIDX_OCF4B };
	INT32 increment = m_timer_increment[4];

	switch(AVR8_WGM4)
	{
		case WGM4_FAST_PWM_8:
		case WGM4_FAST_PWM_9:
		case WGM4_FAST_PWM_10:
		switch(AVR8_TCCR4A_COM4B){
		case 0: /* Normal Operation */
			break;
		case 1: /* TODO */
			break;
		case 2: /* Non-inverting mode */
			if (count == m_timer_top[4]){
			//Clear OC0B
			printf("[2] Clear OC0B\n");
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) & ~(1 << 5));
			} else if (count == 0){
			//Set OC0B
			printf("[2] Set OC0B\n");
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) | (1 << 5));
			}
			break;
		case 3: /* Inverting mode */
			if (count == m_timer_top[4]){
			//Set OC0B
			printf("[3] Set OC0B\n");
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) | (1 << 5));
			} else if (count == 0){
			//Clear OC0B
			printf("[3] Clear OC0B\n");
			m_io->write_byte(AVR8_IO_PORTG, m_io->read_byte(AVR8_IO_PORTG) & ~(1 << 5));
			}
			break;
		}
		break;

		case WGM4_CTC_OCR:
		//printf("[T4] tick WGM4_CTC_OCR: %d\n", count);
			if (count == 0xffff)
			{
				m_r[AVR8_REGIDX_TIFR4] |= AVR8_TIFR4_TOV4_MASK;
				update_interrupt(AVR8_INTIDX_TOV4);
				count = 0;
				increment = 0;
			}

			if (count == ocr4[0]) /*TODO: test for all 3 register*/
			{}
			else if (count == 0)
			{}
			break;

		case WGM1_FAST_PWM_OCR:
			if(count == ocr4[0]) /*TODO: test for all 3 register*/
			{}
			else if(count == 0)
			{}
			break;

		default:
			verboselog(m_pc, 0, "update_timer4_compare_mode: Unknown waveform generation mode: %02x\n", wgm4);
			break;
	}

	count += increment;
	m_r[AVR8_REGIDX_TCNT4H] = (count >> 8) & 0xff;
	m_r[AVR8_REGIDX_TCNT4L] = count & 0xff;
}

void avr8_device::update_timer_clock_source(UINT8 t, UINT8 clock_select)
{
	int prescale_values[8] = {0, 1, 8, 64, 256, 1024, -1, -1};
	m_timer_prescale[t] = prescale_values[clock_select];

	if (VERBOSE_LEVEL) printf("update_timer_clock_source: t=%d cs=%d\n", t, clock_select);

	if (m_timer_prescale[t] == 0xFFFF){
	printf("[Timer #%d]: update_timer_clock_source: External trigger mode not implemented yet\n", t);
	m_timer_prescale[t] = 0;
	}

	if (m_timer_prescale_count[t] > m_timer_prescale[t])
		m_timer_prescale_count[t] = m_timer_prescale[t] - 1;
}

void avr8_device::changed_tccr3a(UINT8 data)
{
	//TODO: Implement-me
//  AVR8_TCCR3A = data;
}

void avr8_device::changed_tccr3b(UINT8 data)
{
	printf("IMPLEMENT-ME: changed_tccr4b: data=0x%02X\n", data);
}

void avr8_device::changed_tccr3c(UINT8 data)
{
//  UINT8 oldtccr = AVR8_TCCR3C;
//  UINT8 newtccr = data;
//  UINT8 changed = newtccr ^ oldtccr;
	printf("IMPLEMENT-ME: changed_tccr3c: data=0x%02X\n", data);

//  AVR8_TCCR3C = data;

	//TODO: Implement-me
}

void avr8_device::changed_tccr4a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR4A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR4A = data;

	if(changed & AVR8_TCCR4A_WGM4_10_MASK)
	{
		update_timer_waveform_gen_mode(4, AVR8_WGM4);
	}
}

void avr8_device::changed_tccr4b(UINT8 data)
{
	printf("changed_tccr4b: data=0x%02X\n", data);

	UINT8 oldtccr = AVR8_TCCR4B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR4B = data;

	if(changed & AVR8_TCCR4B_FOC4A_MASK)
	{
		// TODO
//      timer4_force_output_compare(AVR8_REG_A);
	}

	if(changed & AVR8_TCCR4B_FOC4B_MASK)
	{
		// TODO
//      timer4_force_output_compare(AVR8_REG_B);
	}

	if(changed & AVR8_TCCR4B_WGM4_32_MASK)
	{
		update_timer_waveform_gen_mode(4, AVR8_WGM4);
	}

	if(changed & AVR8_TCCR4B_CS_MASK)
	{
		update_timer_clock_source(4, AVR8_TIMER4_CLOCK_SELECT);
	}
}

void avr8_device::changed_tccr4c(UINT8 data)
{
//  UINT8 oldtccr = AVR8_TCCR4C;
//  UINT8 newtccr = data;
//  UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR4C = data;

	//TODO: Implement-me
}

/************************************************************************************************/

// Timer 5 Handling
void avr8_device::timer5_tick()
{
#if LOG_TIMER_5
	printf("AVR8_WGM5: %d\n", AVR8_WGM5);
	printf("AVR8_TCCR5A_COM5B: %d\n", AVR8_TCCR5A_COM5B);
#endif

	UINT16 count = (AVR8_TCNT5H << 8) + AVR8_TCNT5L;
	INT32 increment = m_timer_increment[5];

	switch(AVR8_WGM5)
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
#if LOG_TIMER_5
			printf("Unimplemented timer#5 waveform generation mode: AVR8_WGM5 = 0x%02X\n", AVR8_WGM5);
#endif
			break;

		case WGM5_CTC_OCR:
		//TODO: verify this! Can be very wrong!!!
		switch(AVR8_TCCR5A_COM5B){
		case 0: /* Normal Operation */
			if (count == m_timer_top[5]){
			m_timer_top[5] = 0;
			}
			break;
		case 1: /* Toggle OC5B on compare match */
			if (count == m_timer_top[5]){
			m_timer_top[5] = 0;
#if LOG_TIMER_5
			printf("[5] Toggle OC5B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTL, m_io->read_byte(AVR8_IO_PORTL) ^ (1 << 4));
			}
			break;
		case 2: /* Clear OC5B on compare match */
			if (count == m_timer_top[5]){
			m_timer_top[5] = 0;
			//Clear OC5B
#if LOG_TIMER_5
			printf("[5] Clear OC5B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTL, m_io->read_byte(AVR8_IO_PORTL) & ~(1 << 4));
			}
			break;
		case 3: /* Set OC5B on compare match */
			if (count == m_timer_top[5]){
			m_timer_top[5] = 0;
#if LOG_TIMER_5
			printf("[5] Set OC5B\n");
#endif
			m_io->write_byte(AVR8_IO_PORTL, m_io->read_byte(AVR8_IO_PORTL) | (1 << 4));
			}
			break;
		}
		break;

		default:
			printf("Timer #5: Unknown waveform generation mode: %02x\n", AVR8_WGM5);
			break;
	}

	count += increment;
	m_r[AVR8_REGIDX_TCNT5H] = (count >> 8) & 0xff;
	m_r[AVR8_REGIDX_TCNT5L] = count & 0xff;
}

void avr8_device::changed_tccr5a(UINT8 data)
{
	UINT8 oldtccr = AVR8_TCCR5A;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR5A = data;

	if(changed & AVR8_TCCR5A_WGM5_10_MASK)
	{
		update_timer_waveform_gen_mode(5, AVR8_WGM5);
	}
}

void avr8_device::changed_tccr5b(UINT8 data)
{
	printf("changed_tccr5b: data=0x%02X\n", data);

	UINT8 oldtccr = AVR8_TCCR5B;
	UINT8 newtccr = data;
	UINT8 changed = newtccr ^ oldtccr;

	AVR8_TCCR5B = data;

	if(changed & AVR8_TCCR5C_FOC5A_MASK)
	{
		// TODO
//      timer5_force_output_compare(AVR8_REG_A);
	}

	if(changed & AVR8_TCCR5C_FOC5B_MASK)
	{
		// TODO
//      timer5_force_output_compare(AVR8_REG_B);
	}

	if(changed & AVR8_TCCR5C_FOC5C_MASK)
	{
		// TODO
//      timer5_force_output_compare(AVR8_REG_C);
	}

	if(changed & AVR8_TCCR5B_WGM5_32_MASK)
	{
		update_timer_waveform_gen_mode(5, AVR8_WGM5);
	}

	if(changed & AVR8_TCCR5B_CS_MASK)
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

const UINT8 avr8_device::spi_clock_divisor[8] = { 4, 16, 64, 128, 2, 8, 32, 64 };

void avr8_device::spi_update_clock_rate()
{
	m_spi_prescale = spi_clock_divisor[AVR8_SPI_RATE];
	m_spi_prescale_count &= m_spi_prescale - 1;
}

void avr8_device::change_spcr(UINT8 data)
{
	UINT8 oldspcr = AVR8_SPCR;
	UINT8 newspcr = data;
	UINT8 changed = newspcr ^ oldspcr;
	UINT8 high_to_low = ~newspcr & oldspcr;
	UINT8 low_to_high = newspcr & ~oldspcr;

	AVR8_SPCR = data;

	if(changed & AVR8_SPCR_SPIE_MASK)
	{
		// Check for SPI interrupt condition
		update_interrupt(AVR8_INTIDX_SPI);
	}

	if(low_to_high & AVR8_SPCR_SPE_MASK)
	{
		enable_spi();
	}
	else if(high_to_low & AVR8_SPCR_SPE_MASK)
	{
		disable_spi();
	}

	if(changed & AVR8_SPCR_MSTR_MASK)
	{
		spi_update_masterslave_select();
	}

	if(changed & AVR8_SPCR_CPOL_MASK)
	{
		spi_update_clock_polarity();
	}

	if(changed & AVR8_SPCR_CPHA_MASK)
	{
		spi_update_clock_phase();
	}

	if(changed & AVR8_SPCR_SPR_MASK)
	{
		spi_update_clock_rate();
	}
}

void avr8_device::change_spsr(UINT8 data)
{
	UINT8 oldspsr = AVR8_SPSR;
	UINT8 newspsr = data;
	UINT8 changed = newspsr ^ oldspsr;

	AVR8_SPSR &= ~1;
	AVR8_SPSR |= data & 1;

	if(changed & AVR8_SPSR_SPR2X_MASK)
	{
		spi_update_clock_rate();
	}
}

/*****************************************************************************/

WRITE8_MEMBER( avr8_device::regs_w )
{
	//printf("<<< WRITE offset [%04x]=%02x >>>\n", offset, data);

	switch( offset )
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
		m_io->write_byte(AVR8_IO_PORTA, data);
		m_r[AVR8_REGIDX_PORTA] = data;
		break;

		case AVR8_REGIDX_PORTB:
		m_io->write_byte(AVR8_IO_PORTB, data);
		m_r[AVR8_REGIDX_PORTB] = data;
		break;

		case AVR8_REGIDX_PORTC:
		m_io->write_byte(AVR8_IO_PORTC, data);
		m_r[AVR8_REGIDX_PORTC] = data;
		break;

		case AVR8_REGIDX_PORTD:
		m_io->write_byte(AVR8_IO_PORTD, data);
		m_r[AVR8_REGIDX_PORTD] = data;
		break;

		case AVR8_REGIDX_PORTE:
		m_io->write_byte(AVR8_IO_PORTE, data);
		m_r[AVR8_REGIDX_PORTE] = data;
		break;

		case AVR8_REGIDX_PORTF:
		m_io->write_byte(AVR8_IO_PORTF, data);
		m_r[AVR8_REGIDX_PORTF] = data;
		break;

		case AVR8_REGIDX_PORTG:
		m_io->write_byte(AVR8_IO_PORTG, data);
		m_r[AVR8_REGIDX_PORTG] = data;
		break;

		case AVR8_REGIDX_PORTH:
		m_io->write_byte(AVR8_IO_PORTH, data);
		m_r[AVR8_REGIDX_PORTH] = data;
		break;

		case AVR8_REGIDX_PORTJ:
		m_io->write_byte(AVR8_IO_PORTJ, data);
		m_r[AVR8_REGIDX_PORTJ] = data;
		break;

		case AVR8_REGIDX_PORTK:
		m_io->write_byte(AVR8_IO_PORTK, data);
		m_r[AVR8_REGIDX_PORTK] = data;
		break;

		case AVR8_REGIDX_PORTL:
		m_io->write_byte(AVR8_IO_PORTL, data);
		m_r[AVR8_REGIDX_PORTL] = data;
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
			verboselog(m_pc, 0, "AVR8: TCCR0B = %02x\n", data );
			changed_tccr0b(data);
			break;

		case AVR8_REGIDX_TCCR0A:
			verboselog(m_pc, 0, "AVR8: TCCR0A = %02x\n", data );
			changed_tccr0a(data);
			break;

		case AVR8_REGIDX_OCR0A:
			verboselog(m_pc, 0, "AVR8: OCR0A = %02x\n", data);
			update_ocr0(AVR8_OCR0A, AVR8_REG_A);
			break;

		case AVR8_REGIDX_OCR0B:
			verboselog(m_pc, 0, "AVR8: OCR0B = %02x\n", data );
			update_ocr0(AVR8_OCR0B, AVR8_REG_B);
			break;

		case AVR8_REGIDX_TIFR0:
			verboselog(m_pc, 0, "AVR8: TIFR0 = %02x\n", data );
			m_r[AVR8_REGIDX_TIFR0] &= ~(data & AVR8_TIFR0_MASK);
			update_interrupt(AVR8_INTIDX_OCF0A);
			update_interrupt(AVR8_INTIDX_OCF0B);
			update_interrupt(AVR8_INTIDX_TOV0);
			break;

		case AVR8_REGIDX_TCNT0:
			AVR8_TCNT0 = data;
			break;

		case AVR8_REGIDX_TIFR1:
			verboselog(m_pc, 0, "AVR8: TIFR1 = %02x\n", data );
			m_r[AVR8_REGIDX_TIFR1] &= ~(data & AVR8_TIFR1_MASK);
			update_interrupt(AVR8_INTIDX_ICF1);
			update_interrupt(AVR8_INTIDX_OCF1A);
			update_interrupt(AVR8_INTIDX_OCF1B);
			update_interrupt(AVR8_INTIDX_TOV1);
			break;

		case AVR8_REGIDX_TIFR2:
			verboselog(m_pc, 0, "AVR8: TIFR2 = %02x\n", data );
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

// EEPROM registers:
		case AVR8_REGIDX_EEARL:
		case AVR8_REGIDX_EEARH:
		case AVR8_REGIDX_EEDR:
			m_r[offset] = data;
		break;

		case AVR8_REGIDX_EECR:
			m_r[offset] = data;

			if (data & AVR8_EECR_EERE_MASK)
			{
				UINT16 addr = (m_r[AVR8_REGIDX_EEARH] & AVR8_EEARH_MASK) << 8;
				addr |= m_r[AVR8_REGIDX_EEARL];
				m_r[AVR8_REGIDX_EEDR] = m_eeprom[addr];
		if (VERBOSE_LEVEL) printf("EEPROM read @ 0x%04x data = 0x%02x\n", addr, m_eeprom[addr]);
			}
			if ((data & AVR8_EECR_EEPE_MASK) && (data & AVR8_EECR_EEMPE_MASK))
			{
				UINT16 addr = (m_r[AVR8_REGIDX_EEARH] & AVR8_EEARH_MASK) << 8;
				addr |= m_r[AVR8_REGIDX_EEARL];
				m_eeprom[addr] = m_r[AVR8_REGIDX_EEDR];
		if (VERBOSE_LEVEL) printf("EEPROM write @ 0x%04x data = 0x%02x ('%c')\n", addr, m_eeprom[addr], m_eeprom[addr]);

			m_r[offset] = data & ~AVR8_EECR_EEPE_MASK; //indicates that we've finished writing a value to the EEPROM.
													//TODO: shouldn't this happen only after a certain dalay?
			}
			break;

		case AVR8_REGIDX_GPIOR0:
			verboselog(m_pc, 1, "AVR8: GPIOR0 Write: %02x\n", data);
			m_r[AVR8_REGIDX_GPIOR0] = data;
			break;

		case AVR8_REGIDX_GPIOR1:
		case AVR8_REGIDX_GPIOR2:
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
			verboselog(m_pc, 0, "AVR8: WDTCSR = %02x\n", data );
		//TODO: changed_wdtcsr(data);
		break;

	case AVR8_REGIDX_CLKPR:
			verboselog(m_pc, 0, "AVR8: CLKPR = %02x\n", data );
		//TODO: changed_clkpr(data);
		break;

	case AVR8_REGIDX_PRR0:
			verboselog(m_pc, 0, "AVR8: PRR0 = %02x\n", data );
		//TODO: changed_prr0(data);
		break;

	case AVR8_REGIDX_PRR1:
			verboselog(m_pc, 0, "AVR8: PRR1 = %02x\n", data );
		//TODO: changed_prr1(data);
		break;

	case AVR8_REGIDX_OSCCAL:
			verboselog(m_pc, 0, "AVR8: OSCCAL = %02x\n", data );
		//TODO: changed_osccal(data);
		break;

	case AVR8_REGIDX_PCICR:
			verboselog(m_pc, 0, "AVR8: PCICR = %02x\n", data );
		//TODO: changed_pcicr(data);
		break;

	case AVR8_REGIDX_EICRA:
			verboselog(m_pc, 0, "AVR8: EICRA = %02x\n", data );
		//TODO: changed_eicra(data);
		break;

	case AVR8_REGIDX_EICRB:
			verboselog(m_pc, 0, "AVR8: EICRB = %02x\n", data );
		//TODO: changed_eicrb(data);
		break;

	case AVR8_REGIDX_PCMSK0:
			verboselog(m_pc, 0, "AVR8: PCMSK0 = %02x\n", data );
		//TODO: changed_pcmsk0(data);
		break;

	case AVR8_REGIDX_PCMSK1:
			verboselog(m_pc, 0, "AVR8: PCMSK1 = %02x\n", data );
		//TODO: changed_pcmsk1(data);
		break;

	case AVR8_REGIDX_PCMSK2:
			verboselog(m_pc, 0, "AVR8: PCMSK2 = %02x\n", data );
		//TODO: changed_pcmsk2(data);
		break;

		case AVR8_REGIDX_TIMSK0:
			verboselog(m_pc, 0, "AVR8: TIMSK0 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK0] = data;
			update_interrupt(AVR8_INTIDX_OCF0A);
			update_interrupt(AVR8_INTIDX_OCF0B);
			update_interrupt(AVR8_INTIDX_TOV0);
			break;

		case AVR8_REGIDX_TIMSK1:
			verboselog(m_pc, 0, "AVR8: TIMSK1 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK1] = data;
			update_interrupt(AVR8_INTIDX_ICF1);
			update_interrupt(AVR8_INTIDX_OCF1A);
			update_interrupt(AVR8_INTIDX_OCF1B);
			update_interrupt(AVR8_INTIDX_TOV1);
			break;

		case AVR8_REGIDX_TIMSK2:
			verboselog(m_pc, 0, "AVR8: TIMSK2 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK2] = data;
			update_interrupt(AVR8_INTIDX_OCF2A);
			update_interrupt(AVR8_INTIDX_OCF2B);
			update_interrupt(AVR8_INTIDX_TOV2);
			break;

		case AVR8_REGIDX_TIMSK3:
			verboselog(m_pc, 0, "AVR8: TIMSK3 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK3] = data;
			update_interrupt(AVR8_INTIDX_OCF3A);
			update_interrupt(AVR8_INTIDX_OCF3B);
			update_interrupt(AVR8_INTIDX_TOV3);
			break;

		case AVR8_REGIDX_TIMSK4:
			verboselog(m_pc, 0, "AVR8: TIMSK4 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK4] = data;
			update_interrupt(AVR8_INTIDX_OCF4A);
			update_interrupt(AVR8_INTIDX_OCF4B);
			update_interrupt(AVR8_INTIDX_TOV4);
			break;

		case AVR8_REGIDX_TIMSK5:
			verboselog(m_pc, 0, "AVR8: TIMSK5 = %02x\n", data );
			m_r[AVR8_REGIDX_TIMSK5] = data;
			update_interrupt(AVR8_INTIDX_OCF5A);
			update_interrupt(AVR8_INTIDX_OCF5B);
			update_interrupt(AVR8_INTIDX_TOV5);
			break;

	case AVR8_REGIDX_XMCRA:
			verboselog(m_pc, 0, "AVR8: XMCRA = %02x\n", data );
		//TODO: changed_xmcra(data);
		break;

	case AVR8_REGIDX_XMCRB:
			verboselog(m_pc, 0, "AVR8: XMCRB = %02x\n", data );
		//TODO: changed_xmcrb(data);
		break;

	case AVR8_REGIDX_ADCL:
			verboselog(m_pc, 0, "AVR8: ADCL = %02x\n", data );
		//TODO: changed_adcl(data);
		break;

	case AVR8_REGIDX_ADCH:
			verboselog(m_pc, 0, "AVR8: ADCH = %02x\n", data );
		//TODO: changed_adch(data);
		break;

	case AVR8_REGIDX_ADCSRA:
			verboselog(m_pc, 0, "AVR8: ADCSRA = %02x\n", data );
		//TODO: changed_adcsra(data);
		break;

	case AVR8_REGIDX_ADCSRB:
			verboselog(m_pc, 0, "AVR8: ADCSRB = %02x\n", data );
		//TODO: changed_adcsrb(data);
		break;

	case AVR8_REGIDX_ADMUX:
			verboselog(m_pc, 0, "AVR8: ADMUX = %02x\n", data );
		//TODO: changed_admux(data);
		break;

	case AVR8_REGIDX_DIDR0:
			verboselog(m_pc, 0, "AVR8: DIDR0 = %02x\n", data );
		//TODO: changed_didr0(data);
		break;

	case AVR8_REGIDX_DIDR1:
			verboselog(m_pc, 0, "AVR8: DIDR1 = %02x\n", data );
		//TODO: changed_didr1(data);
		break;

	case AVR8_REGIDX_DIDR2:
			verboselog(m_pc, 0, "AVR8: DIDR2 = %02x\n", data );
		//TODO: changed_didr2(data);
		break;

		case AVR8_REGIDX_TCCR1A:
			verboselog(m_pc, 0, "AVR8: TCCR1A = %02x\n", data );
			changed_tccr1a(data);
			break;

		case AVR8_REGIDX_TCCR1B:
			verboselog(m_pc, 0, "AVR8: TCCR1B = %02x\n", data );
			changed_tccr1b(data);
			break;

		case AVR8_REGIDX_TCCR1C:
			verboselog(m_pc, 0, "AVR8: TCCR1C = %02x\n", data );
			//TODO: changed_tccr1c(data);
			break;

		case AVR8_REGIDX_TCNT1L:
			AVR8_TCNT1L = data;
			break;

		case AVR8_REGIDX_TCNT1H:
			AVR8_TCNT1H = data;
			break;

		case AVR8_REGIDX_ICR1L:
			AVR8_ICR1L = data;
			break;

		case AVR8_REGIDX_ICR1H:
			AVR8_ICR1H = data;
			break;

		case AVR8_REGIDX_OCR1AL:
			verboselog(m_pc, 0, "AVR8: OCR1AL = %02x\n", data );
			update_ocr1((AVR8_OCR1A & 0xff00) | data, AVR8_REG_A);
			break;

		case AVR8_REGIDX_OCR1AH:
			verboselog(m_pc, 0, "AVR8: OCR1AH = %02x\n", data );
			update_ocr1((AVR8_OCR1A & 0x00ff) | (data << 8), AVR8_REG_A);
			break;

		case AVR8_REGIDX_OCR1BL:
			verboselog(m_pc, 0, "AVR8: OCR1BL = %02x\n", data );
			update_ocr1((AVR8_OCR1B & 0xff00) | data, AVR8_REG_B);
			break;

		case AVR8_REGIDX_OCR1BH:
			verboselog(m_pc, 0, "AVR8: OCR1BH = %02x\n", data );
			update_ocr1((AVR8_OCR1B & 0x00ff) | (data << 8), AVR8_REG_B);
			break;

		case AVR8_REGIDX_OCR1CL:
			verboselog(m_pc, 0, "AVR8: OCR1CL = %02x\n", data );
			update_ocr1((AVR8_OCR1C & 0xff00) | data, AVR8_REG_C);
			break;

		case AVR8_REGIDX_OCR1CH:
			verboselog(m_pc, 0, "AVR8: OCR1CH = %02x\n", data );
			update_ocr1((AVR8_OCR1C & 0x00ff) | (data << 8), AVR8_REG_C);
			break;

		case AVR8_REGIDX_TCCR2A:
			verboselog(m_pc, 0, "AVR8: TCCR2A = %02x\n", data );
			changed_tccr2a(data);
			break;

		case AVR8_REGIDX_TCCR2B:
			verboselog(m_pc, 0, "AVR8: TCCR2B = %02x\n", data );
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
			verboselog(m_pc, 0, "AVR8: TCCR3A = %02x\n", data );
		changed_tccr3a(data);
			break;

		case AVR8_REGIDX_TCCR3B:
			verboselog(m_pc, 0, "AVR8: TCCR3B = %02x\n", data );
		changed_tccr3b(data);
			break;

		case AVR8_REGIDX_TCCR3C:
			verboselog(m_pc, 0, "AVR8: TCCR3C = %02x\n", data );
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
			verboselog(m_pc, 0, "AVR8: TCCR4A = %02x\n", data );
		changed_tccr4a(data);
			break;

		case AVR8_REGIDX_TCCR4B:
			verboselog(m_pc, 0, "AVR8: TCCR4B = %02x\n", data );
		changed_tccr4b(data);
			break;

		case AVR8_REGIDX_TCCR4C:
			verboselog(m_pc, 0, "AVR8: TCCR4C = %02x\n", data );
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
			verboselog(m_pc, 0, "AVR8: TCCR5A = %02x\n", data );
		changed_tccr5a(data);
			break;

		case AVR8_REGIDX_TCCR5B:
			verboselog(m_pc, 0, "AVR8: TCCR5B = %02x\n", data );
		changed_tccr5b(data);
			break;

		case AVR8_REGIDX_ASSR:
			verboselog(m_pc, 0, "AVR8: ASSR = %02x\n", data );
			//TODO: changed_assr(data);
			break;

		case AVR8_REGIDX_TWBR:
			verboselog(m_pc, 0, "AVR8: TWBR = %02x\n", data );
			//TODO: changed_twbr(data);
			break;

		case AVR8_REGIDX_TWSR:
			verboselog(m_pc, 0, "AVR8: TWSR = %02x\n", data );
			//TODO: changed_twsr(data);
			break;

		case AVR8_REGIDX_TWAR:
			verboselog(m_pc, 0, "AVR8: TWAR = %02x\n", data );
			//TODO: changed_twar(data);
			break;

		case AVR8_REGIDX_TWDR:
			verboselog(m_pc, 0, "AVR8: TWDR = %02x\n", data );
			//TODO: changed_twdr(data);
			break;

		case AVR8_REGIDX_TWCR:
			verboselog(m_pc, 0, "AVR8: TWCR = %02x\n", data );
			//TODO: changed_twcr(data);
		m_r[AVR8_REGIDX_TWCR] = data;
			break;

		case AVR8_REGIDX_TWAMR:
			verboselog(m_pc, 0, "AVR8: TWAMR = %02x\n", data );
			//TODO: changed_twamr(data);
			break;

		case AVR8_REGIDX_UCSR0A:
			verboselog(m_pc, 0, "AVR8: UCSR0A = %02x\n", data );
			//TODO: changed_ucsr0a(data);
			break;

		case AVR8_REGIDX_UCSR0B:
			verboselog(m_pc, 0, "AVR8: UCSR0B = %02x\n", data );
			//TODO: changed_ucsr0b(data);
			break;

		case AVR8_REGIDX_UCSR0C:
			verboselog(m_pc, 0, "AVR8: UCSR0C = %02x\n", data );
			//TODO: changed_ucsr0c(data);
			break;
/*
        case AVR8_REGIDX_:
            verboselog(m_pc, 0, "AVR8:  = %02x\n", data );
            //TODO: changed_(data);
            break;
*/
		default:
			verboselog(m_pc, 0, "AVR8: Unknown Register Write: %03x = %02x\n", offset, data);
			break;
	}
}

READ8_MEMBER( avr8_device::regs_r )
{
//  printf("--- READ offset %04x ---\n", offset);

	switch( offset )
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
			return m_r[offset];

		case AVR8_REGIDX_PINA:
		// TODO: consider the DDRA
		return m_io->read_byte(AVR8_REG_A);

		case AVR8_REGIDX_PINB:
		// TODO: consider the DDRB
		return m_io->read_byte(AVR8_REG_B);

		case AVR8_REGIDX_PINC:
		// TODO: consider the DDRC
		return m_io->read_byte(AVR8_REG_C);

		case AVR8_REGIDX_PIND:
		// TODO: consider the DDRD
		return m_io->read_byte(AVR8_REG_D);

		case AVR8_REGIDX_PINE:
		// TODO: consider the DDRE
		return m_io->read_byte(AVR8_REG_E);

		case AVR8_REGIDX_PINF:
		// TODO: consider the DDRF
		return m_io->read_byte(AVR8_REG_F);

		case AVR8_REGIDX_PING:
		// TODO: consider the DDRG
		return m_io->read_byte(AVR8_REG_G);

		case AVR8_REGIDX_PINH:
		// TODO: consider the DDRH
		return m_io->read_byte(AVR8_REG_H);

		case AVR8_REGIDX_PINJ:
		// TODO: consider the DDRJ
		return m_io->read_byte(AVR8_REG_J);

		case AVR8_REGIDX_PINK:
		// TODO: consider the DDRK
		return m_io->read_byte(AVR8_REG_K);

		case AVR8_REGIDX_PINL:
		// TODO: consider the DDRL
		return m_io->read_byte(AVR8_REG_L);

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
			return m_r[offset];

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
			return m_r[offset];

// EEPROM registers:
	case AVR8_REGIDX_EECR:
		case AVR8_REGIDX_EEDR:
			return m_r[offset];

/* Misc. registers.
 TODO: implement all registers reads */

		case AVR8_REGIDX_GPIOR0:
		case AVR8_REGIDX_GPIOR1:
		case AVR8_REGIDX_GPIOR2:
//      case AVR8_REGIDX_UCSR0B:/*TODO: needed for Replicator 1 */
		case AVR8_REGIDX_SPDR:  /*TODO: needed for Replicator 1 */
		case AVR8_REGIDX_SPSR:  /*TODO: needed for Replicator 1 */
//    case AVR8_REGIDX_ADCSRA:   /*TODO: needed for Replicator 1 */
//    case AVR8_REGIDX_ADCSRB:   /*TODO: needed for Replicator 1 */
		case AVR8_REGIDX_SPL:
		case AVR8_REGIDX_SPH:
		case AVR8_REGIDX_SREG:
	case AVR8_REGIDX_TIMSK0:
	case AVR8_REGIDX_TIMSK1:
	case AVR8_REGIDX_TIMSK2:
//    case AVR8_REGIDX_TIMSK3:  /*TODO: needed for Replicator 1 */
	case AVR8_REGIDX_TIMSK4:
	case AVR8_REGIDX_TIMSK5:
			return m_r[offset];

/* Two-wire registers: */
		case AVR8_REGIDX_TWCR:
		/*TODO: needed for Replicator 1
		BLOQUEIA PROGRESSO DA EXECU??O DO FIRMWARE no endere?o 105EC*/
			return m_r[offset];

		case AVR8_REGIDX_TWSR:
		//quick hack: by returning a value != 0x08 we induce an error state that makes the object code jump out of the wait loop and continue execution failing the 2-wire write operation.
		//TODO: implement-me!
			return 0x00; /*TODO: needed for Replicator 1 */


		case AVR8_REGIDX_TCNT1L:
		case AVR8_REGIDX_TCNT1H:
		case AVR8_REGIDX_TCNT2:
		case AVR8_REGIDX_UCSR0A:
			return m_r[offset];

		default:
			printf("[%08X] AVR8: Unknown Register Read: 0x%03X\n", m_shifted_pc, offset);
//      debugger_break(machine());
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
	UINT32 op;
	INT32 offs;
	UINT8 rd;
	UINT8 rr;
	UINT8 res;
	UINT16 pd;
	UINT32 pd32;
	INT16 sd;
	INT32 opcycles;

	while (m_icount > 0)
	{
		opcycles = 1;

		m_pc &= m_addr_mask;
		m_shifted_pc &= (m_addr_mask << 1) | 1;

		debugger_instruction_hook(this, m_shifted_pc);

		op = (UINT32)m_program->read_word(m_shifted_pc);

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
						SREG_W(AVR8_SREG_Z, (res == 0) ? SREG_R(AVR8_SREG_Z) : 0);
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
						SREG_W(AVR8_SREG_Z, (res == 0) ? SREG_R(AVR8_SREG_Z) : 0);
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
					case 0x0000:    // CPSE Rd,Rr
						rd = m_r[RD5(op)];
						rr = m_r[RR5(op)];
						if (rd == rr)
						{
							op = (UINT32)m_program->read_word(m_shifted_pc + 2);
							opcycles += is_long_opcode(op) ? 2 : 1;
							m_pc += is_long_opcode(op) ? 2 : 1;
						}
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
				SREG_W(AVR8_SREG_Z, (res == 0) ? SREG_R(AVR8_SREG_Z) : 0);
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
						m_r[RD5(op)] = m_data->read_byte(ZREG + QCONST6(op));
						opcycles = 2;
						break;
					case 0x0008:    // LDD Rd,Y+q
						m_r[RD5(op)] = m_data->read_byte(YREG + QCONST6(op));
						opcycles = 2;
						break;
					case 0x0200:    // STD Z+q,Rr
						m_data->write_byte(ZREG + QCONST6(op), m_r[RD5(op)]);
						opcycles = 2;
						break;
					case 0x0208:    // STD Y+q,Rr
						m_data->write_byte(YREG + QCONST6(op), m_r[RD5(op)]);
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
								m_shifted_pc += 2;
								op |= m_program->read_word(m_shifted_pc);
								m_r[RD5(op >> 16)] = m_data->read_byte(op & 0x0000ffff);
								opcycles = 2;
								break;
							case 0x0001:    // LD Rd,Z+
								pd = ZREG;
								m_r[RD5(op)] = m_data->read_byte(pd);
								pd++;
								m_r[31] = (pd >> 8) & 0x00ff;
								m_r[30] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x0002:    // LD Rd,-Z
								pd = ZREG;
								pd--;
								m_r[RD5(op)] = m_data->read_byte(pd);
								m_r[31] = (pd >> 8) & 0x00ff;
								m_r[30] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x0004:    // LPM Rd,Z
								m_r[RD5(op)] = m_program->read_byte(ZREG);
								opcycles = 3;
								break;
							case 0x0005:    // LPM Rd,Z+
								pd = ZREG;
								m_r[RD5(op)] = m_program->read_byte(pd);
								pd++;
								m_r[31] = (pd >> 8) & 0x00ff;
								m_r[30] = pd & 0x00ff;
								opcycles = 3;
								break;
							case 0x0006:    // ELPM Rd,Z
								m_r[RD5(op)] = m_program->read_byte((m_r[AVR8_REGIDX_RAMPZ] << 16) | ZREG);
								opcycles = 3;
								break;
							case 0x0007:    // ELPM Rd,Z+
								pd32 = (m_r[AVR8_REGIDX_RAMPZ] << 16) | ZREG;
				m_r[RD5(op)] = m_program->read_byte(pd32);
				pd32++;
				m_r[AVR8_REGIDX_RAMPZ] = (pd32 >> 16) & 0x00ff;
				m_r[31] = (pd32 >> 8) & 0x00ff;
								m_r[30] = pd32 & 0x00ff;
				opcycles = 3;
								break;
							case 0x0009:    // LD Rd,Y+
								pd = YREG;
								m_r[RD5(op)] = m_data->read_byte(pd);
								pd++;
								m_r[29] = (pd >> 8) & 0x00ff;
								m_r[28] = pd & 0x00ff;
								opcycles = 2;
				break;
							case 0x000a:    // LD Rd,-Y
								pd = YREG;
								pd--;
								m_r[RD5(op)] = m_data->read_byte(pd);
								m_r[29] = (pd >> 8) & 0x00ff;
								m_r[28] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x000c:    // LD Rd,X
								m_r[RD5(op)] = m_data->read_byte(XREG);
								opcycles = 2;
								break;
							case 0x000d:    // LD Rd,X+
								pd = XREG;
								m_r[RD5(op)] = m_data->read_byte(pd);
								pd++;
								m_r[27] = (pd >> 8) & 0x00ff;
								m_r[26] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x000e:    // LD Rd,-X
								pd = XREG;
								pd--;
								m_r[RD5(op)] = m_data->read_byte(pd);
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
								m_shifted_pc += 2;
								op |= m_program->read_word(m_shifted_pc);
								m_data->write_byte(op & 0x0000ffff, m_r[RD5(op >> 16)]);
								opcycles = 2;
								break;
							case 0x0001:    // ST Z+,Rd
								pd = ZREG;
								m_data->write_byte(pd, m_r[RD5(op)]);
								pd++;
								m_r[31] = (pd >> 8) & 0x00ff;
								m_r[30] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x0002:    // ST -Z,Rd
								pd = ZREG;
								pd--;
								m_data->write_byte(pd, m_r[RD5(op)]);
								m_r[31] = (pd >> 8) & 0x00ff;
								m_r[30] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x0009:    // ST Y+,Rd
								pd = YREG;
								m_data->write_byte(pd, m_r[RD5(op)]);
								pd++;
								m_r[29] = (pd >> 8) & 0x00ff;
								m_r[28] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x000a:    // ST -Y,Rd
								pd = YREG;
								pd--;
								m_data->write_byte(pd, m_r[RD5(op)]);
								m_r[29] = (pd >> 8) & 0x00ff;
								m_r[28] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x000c:    // ST X,Rd
								m_data->write_byte(XREG, m_r[RD5(op)]);
								break;
							case 0x000d:    // ST X+,Rd
								pd = XREG;
								m_data->write_byte(pd, m_r[RD5(op)]);
								pd++;
								m_r[27] = (pd >> 8) & 0x00ff;
								m_r[26] = pd & 0x00ff;
								opcycles = 2;
								break;
							case 0x000e:    // ST -X,Rd
								pd = XREG;
								pd--;
								m_data->write_byte(pd, m_r[RD5(op)]);
								m_r[27] = (pd >> 8) & 0x00ff;
								m_r[26] = pd & 0x00ff;
								opcycles = 2;
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
										m_pc = (m_r[AVR8_REGIDX_EIND] << 16 | ZREG) - 1;
										opcycles = 2;
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
								m_shifted_pc += 2;
								offs |= m_program->read_word(m_shifted_pc);
								m_pc = offs;
								m_pc--;
								opcycles = 3;
								break;
							case 0x000e:    // CALL k
							case 0x000f:
								push((m_pc + 2) & 0x00ff);
								push(((m_pc + 2) >> 8) & 0x00ff);
								offs = KCONST22(op) << 16;
								m_pc++;
								m_shifted_pc += 2;
								offs |= m_program->read_word(m_shifted_pc);
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
										m_pc = pop() << 8;
										m_pc |= pop();
										m_pc--;
										opcycles = 4;
										break;
									case 0x0010:    // RETI
										m_pc = pop() << 8;
										m_pc |= pop();
										m_pc--;
										SREG_W(AVR8_SREG_I, 1);
										opcycles = 4;
										break;
									case 0x0080:    // SLEEP
										//output += sprintf( output, "SLEEP" );
										m_pc--;
										opcycles = 1;
										//unimplemented_opcode(op);
										break;
									case 0x0090:    // BREAK
										//output += sprintf( output, "BREAK" );
										unimplemented_opcode(op);
										break;
									case 0x00a0:    // WDR
										//output += sprintf( output, "WDR" );
										//unimplemented_opcode(op); //TODO: necessary for emulating the Replicator 1
					//printf("Watchdot Reset!\n");
										opcycles = 1;
										break;
									case 0x00c0:    // LPM
										m_r[0] = m_program->read_byte(ZREG);
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
										push((m_pc + 1) & 0x00ff);
										push(((m_pc + 1) >> 8) & 0x00ff);
										m_pc = ZREG;
										m_pc--;
										opcycles = 3;
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
						rd = m_r[24 + (DCONST(op) << 1)];
						rr = m_r[25 + (DCONST(op) << 1)];
						pd = rd;
						pd |= rr << 8;
						pd -= KCONST6(op);
						SREG_W(AVR8_SREG_V, NOT(BIT(pd,15)) & BIT(rr,7));
						SREG_W(AVR8_SREG_N, BIT(pd,15));
						SREG_W(AVR8_SREG_S, SREG_R(AVR8_SREG_N) ^ SREG_R(AVR8_SREG_V));
						SREG_W(AVR8_SREG_Z, (pd == 0) ? 1 : 0);
						SREG_W(AVR8_SREG_C, BIT(pd,15) & NOT(BIT(rr,7)));
						m_r[24 + (DCONST(op) << 1)] = pd & 0x00ff;
						m_r[25 + (DCONST(op) << 1)] = (pd >> 8) & 0x00ff;
						opcycles = 2;
						break;
					case 0x0800:    // CBI A,b
						//output += sprintf( output, "CBI     0x%02x, %d", ACONST5(op), RR3(op) );
						m_data->write_byte(32 + ACONST5(op), m_data->read_byte(32 + ACONST5(op)) &~ (1 << RR3(op)));
						opcycles = 2;
						break;
					case 0x0900:    // SBIC A,b
						if(NOT(BIT(m_data->read_byte(32 + ACONST5(op)), RR3(op))))
						{
							op = (UINT32)m_program->read_word(m_shifted_pc + 2);
							opcycles = is_long_opcode(op) ? 3 : 2;
							m_pc += is_long_opcode(op) ? 2 : 1;
						}
						break;
					case 0x0a00:    // SBI A,b
						m_data->write_byte(32 + ACONST5(op), m_data->read_byte(32 + ACONST5(op)) | (1 << RR3(op)));
						opcycles = 2;
						break;
					case 0x0b00:    // SBIS A,b
						if(BIT(m_data->read_byte(32 + ACONST5(op)), RR3(op)))
						{
							op = (UINT32)m_program->read_word(m_shifted_pc + 2);
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
					m_data->write_byte(32 + ACONST6(op), m_r[RD5(op)]);
				}
				else            // IN Rd,A
				{
					m_r[RD5(op)] = m_data->read_byte(0x20 + ACONST6(op));
				}
				break;
			case 0xc000:    // RJMP k
				offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
				m_pc += offs;
				opcycles = 2;
				break;
			case 0xd000:    // RCALL k
				offs = (INT32)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff));
				push((m_pc + 1) & 0x00ff);
				push(((m_pc + 1) >> 8) & 0x00ff);
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
								op = (UINT32)m_program->read_word(m_shifted_pc + 2);
								m_pc += is_long_opcode(op) ? 2 : 1;
								opcycles = is_long_opcode(op) ? 3 : 2;
							}
						}
						else            // SBRC Rd, b
						{
							if(NOT(BIT(m_r[RD5(op)], RR3(op))))
							{
								op = (UINT32)m_program->read_word(m_shifted_pc + 2);
								m_pc += is_long_opcode(op) ? 2 : 1;
								opcycles = is_long_opcode(op) ? 3 : 2;
							}
						}
						break;
				}
				break;
		}

		m_pc++;

		m_shifted_pc = m_pc << 1;

		m_icount -= opcycles;

		timer_tick(opcycles);
	}
}
