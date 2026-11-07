// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Sandro Ronco, Felipe Sanches
/***************************************************************************

    Atmel 8-bit AVR simulator

    - Notes -
      Cycle counts are generally considered to be 100% accurate per-instruction. External interrupt sources
      (pin-change and INT0/INT1) may be triggered mid-instruction by an arbitrary external context; these are
      latched into PCIFR/EIFR immediately and the actual interrupt is taken at the next execute_run() instruction
      boundary. Evidence of cycle accuracy is given in the form of the demoscene 'wild' demo, Craft, by [lft],
      which uses an ATmega88 to write video out a 6-bit RGB DAC pixel-by-pixel, synchronously with the frame
      timing. Intentionally modifying the timing of any of
      the existing opcodes has been shown to wildly corrupt the video output in Craft, so one can assume that the
      existing timing is 100% correct.

      Unimplemented opcodes: SPM, SPM Z+, BREAK, WDR, EICALL

    About the "classic" and "modern" AVR8s:
    ------------------------------------------------------------------

    This core, for the time being, only supports the "classic" AVR8s.

    Differences between the classic and modern AVR8s include:
    - SPI programming interface on the classics, UPDI on the moderns
    - Harvard architecture on the classics, unified memory architecture on the moderns
    - Completely different timer scheme on the moderns, including Timer A being
      able to function as one 16-bit timer or two 8-bit timers

    16-bit register accesses:
    ------------------------------------------------------------------

    The datasheets specify that when accessing a 16-bit register, the right way to do this
    is to read the lower 8 bits first. That latches the upper 8 bits in the "high" register
    for some time, so that they can be read. The timing here is sensitive, though, so interrupts
    are typically disabled.

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
#define LOG_INTERRUPTS      (1U << 28)
#define LOG_TIMERS          (LOG_TIMER0 | LOG_TIMER1 | LOG_TIMER2 | LOG_TIMER3 | LOG_TIMER4 | LOG_TIMER5)
#define LOG_TIMER_TICKS     (LOG_TIMER0_TICK | LOG_TIMER1_TICK | LOG_TIMER2_TICK | LOG_TIMER3_TICK | LOG_TIMER4_TICK | LOG_TIMER5_TICK)
#define LOG_ALL             (LOG_UNKNOWN | LOG_BOOT | LOG_TIMERS | LOG_EEPROM | LOG_GPIO | LOG_WDOG | LOG_CLOCK | LOG_POWER \
							 | LOG_OSC | LOG_PINCHG | LOG_EXTMEM | LOG_ADC | LOG_DIGINPUT | LOG_ASYNC | LOG_TWI | LOG_UART \
							 | LOG_INTERRUPTS)

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

#define PCICR_PCIE0_MASK    (1 << 0)
#define PCICR_PCIE1_MASK    (1 << 1)
#define PCICR_PCIE2_MASK    (1 << 2)

#define PCIFR_PCIF0_MASK    (1 << 0)
#define PCIFR_PCIF1_MASK    (1 << 1)
#define PCIFR_PCIF2_MASK    (1 << 2)

#define EIMSK_INT0_MASK     (1 << 0)
#define EIMSK_INT1_MASK     (1 << 1)

#define EIFR_INTF0_MASK     (1 << 0)
#define EIFR_INTF1_MASK     (1 << 1)

#define EICRA_ISC0_SHIFT    0
#define EICRA_ISC0_MASK     0x03
#define EICRA_ISC1_SHIFT    2
#define EICRA_ISC1_MASK     0x0c

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
DEFINE_DEVICE_TYPE(ATMEGA32U4, atmega32u4_device, "atmega32u4", "Atmel ATmega32U4")
DEFINE_DEVICE_TYPE(ATMEGA644,  atmega644_device,  "atmega644",  "Atmel ATmega644")
DEFINE_DEVICE_TYPE(ATMEGA1284, atmega1284_device, "atmega1284", "Atmel ATmega1284")
DEFINE_DEVICE_TYPE(ATMEGA1280, atmega1280_device, "atmega1280", "Atmel ATmega1280")
DEFINE_DEVICE_TYPE(ATMEGA2560, atmega2560_device, "atmega2560", "Atmel ATmega2560")

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

void megaavr_classic_base_device::megaavr_classic_base_internal_map(address_map &map)
{
	// Most classic megaAVRs use this map.
	// The ATMega16 and ATMega32 map is more similar to the ATTinys.

	// Map only the base register area.
	// In the future everything in the I/O space will be a device.
	// And, if any megaAVR device tries accessing a register that doesn't exist,
	// it should cause a logerror().
	map(0x0000, 0x001f).rw(FUNC(megaavr_classic_base_device::regs_r), FUNC(megaavr_classic_base_device::regs_w));

	// 0x0020-0x0034 all reserved for ports; install those in sub-devices only.

	map(0x0035, 0x0035).rw("timer0", &avr8_megaavr_timer_0_device::tifr_r, "timer_0_tifr_r", &avr8_megaavr_timer_0_device::tifr_w, "timer_0_tifr_w");
	map(0x0036, 0x0036).rw("timer1", &avr8_megaavr_timer_1_device::tifr_r, "timer_1_tifr_r", &avr8_megaavr_timer_1_device::tifr_w, "timer_1_tifr_w");
	map(0x0037, 0x0037).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::tifr_r), FUNC(avr8_megaavr_fakertc_timer_device::tifr_w));

	// 0x0037-0x003a: all reserved for other timers

	map(0x003b, 0x003b).rw(FUNC(megaavr_classic_base_device::reg_r<PCIFR>), FUNC(megaavr_classic_base_device::pcifr_w));
	map(0x003c, 0x003c).rw(FUNC(megaavr_classic_base_device::reg_r<EIFR>), FUNC(megaavr_classic_base_device::eifr_w));
	map(0x003d, 0x003d).rw(FUNC(megaavr_classic_base_device::reg_r<EIMSK>), FUNC(megaavr_classic_base_device::reg_w<EIMSK>));
	map(0x003e, 0x003e).rw(FUNC(megaavr_classic_base_device::reg_r<GPIOR0>), FUNC(megaavr_classic_base_device::gpior0_w));
	map(0x003f, 0x003f).rw(FUNC(megaavr_classic_base_device::reg_r<EECR>), FUNC(megaavr_classic_base_device::eecr_w));
	map(0x0040, 0x0040).rw(FUNC(megaavr_classic_base_device::reg_r<EEDR>), FUNC(megaavr_classic_base_device::reg_w<EEDR>));
	map(0x0041, 0x0041).rw(FUNC(megaavr_classic_base_device::reg_r<EEARL>), FUNC(megaavr_classic_base_device::reg_w<EEARL>));
	map(0x0042, 0x0042).rw(FUNC(megaavr_classic_base_device::reg_r<EEARH>), FUNC(megaavr_classic_base_device::reg_w<EEARH>));

	// this GTCCR register is common to all classic megaAVRs; ATMega8/16/32 and the ATTinys don't have it
	map(0x0043, 0x0043).rw(FUNC(megaavr_classic_base_device::gtccr_r), FUNC(megaavr_classic_base_device::gtccr_w));

	// most Timer 0 control registers
	map(0x0044, 0x0044).rw("timer0", &avr8_megaavr_timer_0_device::tccr_a_r, "timer_0_tccr_a_r", &avr8_megaavr_timer_0_device::tccr_a_w, "timer_0_tccr_a_w");
	map(0x0045, 0x0045).rw("timer0", FUNC(avr8_megaavr_timer_0_device::tccr_b_r), FUNC(avr8_megaavr_timer_0_device::tccr_b_w));
	map(0x0046, 0x0046).rw("timer0", FUNC(avr8_megaavr_timer_0_device::tcnt_r), FUNC(avr8_megaavr_timer_0_device::tcnt_w));
	map(0x0047, 0x0047).rw("timer0", FUNC(avr8_megaavr_timer_0_device::ocr_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_0_device::ocr_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x0048, 0x0048).rw("timer0", FUNC(avr8_megaavr_timer_0_device::ocr_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_0_device::ocr_w<AVR8_TIMER_CMPCHANNEL_OC_B>));

	// 0x0049: reserved
	// on the xU4 series, that's where the PLL control register is

	map(0x004a, 0x004a).rw(FUNC(megaavr_classic_base_device::reg_r<GPIOR1>), FUNC(megaavr_classic_base_device::gpior1_w));
	map(0x004b, 0x004b).rw(FUNC(megaavr_classic_base_device::reg_r<GPIOR2>), FUNC(megaavr_classic_base_device::gpior2_w));
	map(0x004c, 0x004c).rw(FUNC(megaavr_classic_base_device::reg_r<SPCR>), FUNC(megaavr_classic_base_device::spcr_w));
	map(0x004d, 0x004d).rw(FUNC(megaavr_classic_base_device::reg_r<SPSR>), FUNC(megaavr_classic_base_device::spsr_w));
	map(0x004e, 0x004e).rw(FUNC(megaavr_classic_base_device::reg_r<SPDR>), FUNC(megaavr_classic_base_device::spdr_w));

	// 0x004f: reserved

	map(0x0050, 0x0050).rw(FUNC(megaavr_classic_base_device::reg_r<ACSR>), FUNC(megaavr_classic_base_device::reg_w<ACSR>));

	// 0x0051: OCDR (not installed on all devices)

	// 0x0052: PLLFREQ (xU4 only)

	map(0x0053, 0x0053).rw(FUNC(megaavr_classic_base_device::reg_r<SMCR>), FUNC(megaavr_classic_base_device::reg_w<SMCR>));
	map(0x0054, 0x0054).rw(FUNC(megaavr_classic_base_device::reg_r<MCUSR>), FUNC(megaavr_classic_base_device::reg_w<MCUSR>));
	map(0x0055, 0x0055).rw(FUNC(megaavr_classic_base_device::reg_r<MCUCR>), FUNC(megaavr_classic_base_device::reg_w<MCUCR>));

	// 0x0056: reserved

	// 0x0057: SPMCSR (to be brought out to dedicated flash controller device later)
	map(0x0057, 0x0057).rw(FUNC(megaavr_classic_base_device::reg_r<SPMCSR>), FUNC(megaavr_classic_base_device::reg_w<SPMCSR>));

	// 0x0058, 0x0059, 0x005a: reserved

	// 0x005b: RAMPZ, on devices with more than 64kbytes flash space

	// 0x005c: EIND (not on all devices)

	map(0x005d, 0x005d).rw(FUNC(megaavr_classic_base_device::reg_r<SPL>), FUNC(megaavr_classic_base_device::reg_w<SPL>));
	map(0x005e, 0x005e).rw(FUNC(megaavr_classic_base_device::reg_r<SPH>), FUNC(megaavr_classic_base_device::reg_w<SPH>));
	map(0x005f, 0x005f).rw(FUNC(megaavr_classic_base_device::reg_r<SREG>), FUNC(megaavr_classic_base_device::reg_w<SREG>));

	map(0x0060, 0x0060).rw(FUNC(megaavr_classic_base_device::reg_r<WDTCSR>), FUNC(megaavr_classic_base_device::wdtcsr_w));
	map(0x0061, 0x0061).rw(FUNC(megaavr_classic_base_device::reg_r<CLKPR>), FUNC(megaavr_classic_base_device::clkpr_w));

	// 0x0062, 0x0063: reserved

	// 0x0064: PRR0 (note: bit layout differs between device types!)
	map(0x0064, 0x0064).rw(FUNC(megaavr_classic_base_device::reg_r<PRR0>), FUNC(megaavr_classic_base_device::prr0_w));

	// 0x0065: PRR1 (not on all devices)

	map(0x0066, 0x0066).rw(FUNC(megaavr_classic_base_device::reg_r<OSCCAL>), FUNC(megaavr_classic_base_device::osccal_w));

	// 0x0067: RCCTRL (not on all devices)

	map(0x0068, 0x0068).rw(FUNC(megaavr_classic_base_device::reg_r<PCICR>), FUNC(megaavr_classic_base_device::pcicr_w));
	map(0x0069, 0x0069).rw(FUNC(megaavr_classic_base_device::reg_r<EICRA>), FUNC(megaavr_classic_base_device::eicra_w));

	// 0x006a: EICRB (not on all devices)

	map(0x006b, 0x006b).rw(FUNC(megaavr_classic_base_device::reg_r<PCMSK0>), FUNC(megaavr_classic_base_device::pcmsk0_w));

	// 0x006c: PMSK1 (not on all devices)
	// 0x006d: PMSK2 (not on all devices)

	map(0x006e, 0x006e).rw("timer0", FUNC(avr8_megaavr_timer_0_device::timsk_r), FUNC(avr8_megaavr_timer_0_device::timsk_w));
	map(0x006f, 0x006f).rw("timer1", FUNC(avr8_megaavr_timer_1_device::timsk_r), FUNC(avr8_megaavr_timer_1_device::timsk_w));
	map(0x0070, 0x0070).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::timsk_r), FUNC(avr8_megaavr_fakertc_timer_device::timsk_w));
	// 0x0071: TIMSK3 (not on all devices)
	// 0x0072: TIMSK4 (not on all devices)
	// 0x0073: TIMSK5 (not on all devices)

	// 0x0074: XMCRA (not on all devices)
	// 0x0075: XMCRB (not on all devices)

	// 0x0076: reserved
	// 0x0077: reserved

	map(0x0078, 0x0078).rw(FUNC(megaavr_classic_base_device::adcl_r), FUNC(megaavr_classic_base_device::adcl_w));
	map(0x0079, 0x0079).rw(FUNC(megaavr_classic_base_device::adch_r), FUNC(megaavr_classic_base_device::adch_w));
	map(0x007a, 0x007a).rw(FUNC(megaavr_classic_base_device::reg_r<ADCSRA>), FUNC(megaavr_classic_base_device::adcsra_w));
	map(0x007b, 0x007b).rw(FUNC(megaavr_classic_base_device::reg_r<ADCSRB>), FUNC(megaavr_classic_base_device::adcsrb_w));
	map(0x007c, 0x007c).rw(FUNC(megaavr_classic_base_device::reg_r<ADMUX>), FUNC(megaavr_classic_base_device::admux_w));

	// 0x007d: DIDR2 (not on all devices)
	map(0x007e, 0x007e).rw(FUNC(megaavr_classic_base_device::reg_r<DIDR0>), FUNC(megaavr_classic_base_device::didr0_w));
	map(0x007f, 0x007f).rw(FUNC(megaavr_classic_base_device::reg_r<DIDR1>), FUNC(megaavr_classic_base_device::didr1_w));

	// 0x0080-0x008d: Timer 1 control registers (common to the classic megaAVRs)
	map(0x0080, 0x0080).rw("timer1", FUNC(avr8_megaavr_timer_1_device::tccr_a_r),  FUNC(avr8_megaavr_timer_1_device::tccr_a_w));
	map(0x0081, 0x0081).rw("timer1", FUNC(avr8_megaavr_timer_1_device::tccr_b_r),  FUNC(avr8_megaavr_timer_1_device::tccr_b_w));
	map(0x0082, 0x0082).rw("timer1", FUNC(avr8_megaavr_timer_1_device::tccr_c_r),  FUNC(avr8_megaavr_timer_1_device::tccr_c_w));
	// 0x0083: reserved
	map(0x0084, 0x0084).rw("timer1", FUNC(avr8_megaavr_timer_1_device::tcnt_l_r), FUNC(avr8_megaavr_timer_1_device::tcnt_l_w));
	map(0x0085, 0x0085).rw("timer1", FUNC(avr8_megaavr_timer_1_device::tcnt_h_r), FUNC(avr8_megaavr_timer_1_device::tcnt_h_w));
	map(0x0086, 0x0086).rw("timer1", FUNC(avr8_megaavr_timer_1_device::icr_l_r), FUNC(avr8_megaavr_timer_1_device::icr_l_w));
	map(0x0087, 0x0087).rw("timer1", FUNC(avr8_megaavr_timer_1_device::icr_h_r), FUNC(avr8_megaavr_timer_1_device::icr_h_w));
	map(0x0088, 0x0088).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x0089, 0x0089).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x008a, 0x008a).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x008b, 0x008b).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>));

	// TODO: these should act as nop r/ws on devices that don't have them
	map(0x008c, 0x008c).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	map(0x008d, 0x008d).rw("timer1", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>));

	// 0x008e, 0x008f: reserved

	// 0x0090-0x009d: Timer 3 control registers (not on all devices)
	// 0x009e, 0x009f: reserved

	// 0x00a0-0x00ad: Timer 4 control registers (not on all devices)
	// (Note that this is for 16-bit timer 4s, not the high-speed timer on the xU4 devices)

	// 0x00ae, 0x00af: reserved

	// 0x00b0-0x00b7: Timer 2 control registers (not on all devices)
	map(0x00b0, 0x00b0).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::tccr_a_r), FUNC(avr8_megaavr_fakertc_timer_device::tccr_a_w));
	map(0x00b1, 0x00b1).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::tccr_b_r), FUNC(avr8_megaavr_fakertc_timer_device::tccr_b_w));
	map(0x00b2, 0x00b2).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::tcnt_r), FUNC(avr8_megaavr_fakertc_timer_device::tcnt_w));
	map(0x00b3, 0x00b3).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::ocr_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_fakertc_timer_device::ocr_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x00b4, 0x00b4).rw("atimer2", FUNC(avr8_megaavr_fakertc_timer_device::ocr_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_fakertc_timer_device::ocr_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	// 0x00b5: reserved
	map(0x00b6, 0x00b6).rw("asyncunit", FUNC(avr8_megaavr_fakertc_async_controller_device::assr_r), FUNC(avr8_megaavr_fakertc_async_controller_device::assr_w));
	// 0x00b7: reserved

	map(0x00b8, 0x00b8).rw(FUNC(megaavr_classic_base_device::reg_r<TWBR>), FUNC(megaavr_classic_base_device::twbr_w));
	map(0x00b9, 0x00b9).rw(FUNC(megaavr_classic_base_device::twsr_r), FUNC(megaavr_classic_base_device::twsr_w));
	map(0x00ba, 0x00ba).rw(FUNC(megaavr_classic_base_device::reg_r<TWAR>), FUNC(megaavr_classic_base_device::twar_w));
	map(0x00bb, 0x00bb).rw(FUNC(megaavr_classic_base_device::reg_r<TWDR>), FUNC(megaavr_classic_base_device::twdr_w));
	map(0x00bc, 0x00bc).rw(FUNC(megaavr_classic_base_device::reg_r<TWCR>), FUNC(megaavr_classic_base_device::twcr_w));
	map(0x00bd, 0x00bd).rw(FUNC(megaavr_classic_base_device::reg_r<TWAMR>), FUNC(megaavr_classic_base_device::twamr_w));

	// space at 0x00be and above varies wildly between devices, so nothing else can be installed here.
}

void atmegax8_base_device::atmegax8_base_internal_map(address_map &map)
{
	megaavr_classic_base_internal_map(map);

	// x8 series has ports PB, PC, PD
	map(0x0023, 0x0023).r(FUNC(atmegax8_base_device::pin_r<1>));
	map(0x0024, 0x0024).rw(FUNC(atmegax8_base_device::reg_r<DDRB>), FUNC(atmegax8_base_device::ddr_w<GPIOB>));
	map(0x0025, 0x0025).rw(FUNC(atmegax8_base_device::gpio_r<GPIOB>), FUNC(atmegax8_base_device::port_w<GPIOB>));
	map(0x0026, 0x0026).r(FUNC(atmegax8_base_device::pin_r<2>));
	map(0x0027, 0x0027).rw(FUNC(atmegax8_base_device::reg_r<DDRC>), FUNC(atmegax8_base_device::ddr_w<GPIOC>));
	map(0x0028, 0x0028).rw(FUNC(atmegax8_base_device::gpio_r<GPIOC>), FUNC(atmegax8_base_device::port_w<GPIOC>));
	map(0x0029, 0x0029).r(FUNC(atmegax8_base_device::pin_r<3>));
	map(0x002a, 0x002a).rw(FUNC(atmegax8_base_device::reg_r<DDRD>), FUNC(atmegax8_base_device::ddr_w<GPIOD>));
	map(0x002b, 0x002b).rw(FUNC(atmegax8_base_device::gpio_r<GPIOD>), FUNC(atmegax8_base_device::port_w<GPIOD>));

	map(0x006c, 0x006c).rw(FUNC(atmegax8_base_device::reg_r<PCMSK1>), FUNC(atmegax8_base_device::pcmsk1_w));
	map(0x006d, 0x006d).rw(FUNC(atmegax8_base_device::reg_r<PCMSK2>), FUNC(atmegax8_base_device::pcmsk2_w));

	// 0x0071-0x0077: reserved

	map(0x00c0, 0x00c0).rw(FUNC(atmegax8_base_device::reg_r<UCSR0A>), FUNC(atmegax8_base_device::ucsr0a_w));
	map(0x00c1, 0x00c1).rw(FUNC(atmegax8_base_device::reg_r<UCSR0B>), FUNC(atmegax8_base_device::ucsr0b_w));
	map(0x00c2, 0x00c2).rw(FUNC(atmegax8_base_device::reg_r<UCSR0C>), FUNC(atmegax8_base_device::ucsr0c_w));
	map(0x00c4, 0x00c4).rw(FUNC(atmegax8_base_device::reg_r<UBRR0L>), FUNC(atmegax8_base_device::reg_w<UBRR0L>));
	map(0x00c5, 0x00c5).rw(FUNC(atmegax8_base_device::reg_r<UBRR0H>), FUNC(atmegax8_base_device::reg_w<UBRR0H>));
	map(0x00c6, 0x00c6).rw(FUNC(atmegax8_base_device::reg_r<UDR0>), FUNC(atmegax8_base_device::reg_w<UDR0>));

	// cells 0x00c7-0x00ff all reserved
}

#if 0
void atmega48_device::atmega48_device_internal_map(address_map &map)
{
	atmegax8_base_internal_map::atmegax8_base_internal_map();

	// TODO: reenable when datamaps are all removed from drivers
	// map(0x0100, 0x02ff).ram();
}
#endif

void atmega88_device::atmega88_internal_map(address_map &map)
{
	atmegax8_base_internal_map(map);

	// TODO: reenable when datamaps are all removed from drivers
	// map(0x0100, 0x04ff).ram();
}

void atmega168_device::atmega168_internal_map(address_map &map)
{
	atmegax8_base_internal_map(map);

	// TODO: reenable when datamaps are all removed from drivers
	// map(0x0100, 0x04ff).ram();
}

void atmega328_device::atmega328_internal_map(address_map &map)
{
	atmegax8_base_internal_map(map);

	// TODO: reenable when datamaps are all removed from drivers
	// map(0x0100, 0x08ff).ram();
}

void atmegaxu4_base_device::atmegaxu4_base_internal_map(address_map &map)
{
	megaavr_classic_base_internal_map(map);

	// xU4 series has ports PB, PC, PD, PE, PF
	// though a lot of them have bits that go nowhere
	map(0x0023, 0x0023).r(FUNC(avr8_device::pin_r<1>));
	map(0x0024, 0x0024).rw(FUNC(atmegaxu4_base_device::reg_r<DDRB>), FUNC(avr8_device::ddr_w<GPIOB>));
	map(0x0025, 0x0025).rw(FUNC(avr8_device::gpio_r<GPIOB>), FUNC(avr8_device::port_w<GPIOB>));
	map(0x0026, 0x0026).r(FUNC(avr8_device::pin_r<2>));
	map(0x0027, 0x0027).rw(FUNC(atmegaxu4_base_device::reg_r<DDRC>), FUNC(avr8_device::ddr_w<GPIOC>));
	map(0x0028, 0x0028).rw(FUNC(avr8_device::gpio_r<GPIOC>), FUNC(avr8_device::port_w<GPIOC>));
	map(0x0029, 0x0029).r(FUNC(avr8_device::pin_r<3>));
	map(0x002a, 0x002a).rw(FUNC(atmegaxu4_base_device::reg_r<DDRD>), FUNC(avr8_device::ddr_w<GPIOD>));
	map(0x002b, 0x002b).rw(FUNC(avr8_device::gpio_r<GPIOD>), FUNC(avr8_device::port_w<GPIOD>));
	map(0x002c, 0x002c).r(FUNC(avr8_device::pin_r<4>));
	map(0x002d, 0x002d).rw(FUNC(atmegaxu4_base_device::reg_r<DDRE>), FUNC(avr8_device::ddr_w<GPIOE>));
	map(0x002e, 0x002e).rw(FUNC(avr8_device::gpio_r<GPIOE>), FUNC(avr8_device::port_w<GPIOE>));
	map(0x002f, 0x002f).r(FUNC(avr8_device::pin_r<5>));
	map(0x0030, 0x0030).rw(FUNC(atmegaxu4_base_device::reg_r<DDRF>), FUNC(avr8_device::ddr_w<GPIOF>));
	map(0x0031, 0x0031).rw(FUNC(avr8_device::gpio_r<GPIOF>), FUNC(avr8_device::port_w<GPIOF>));
	// 0x0032, 0x0033, 0x0034: reserved
	// 0x0035: TIFR0 (already mapped)

	// 0x0037: reserved (no timer 2)
	map(0x0037, 0x0037).unmaprw();

	// 0x0038: TIFR3
	map(0x0038, 0x0038).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tifr_r), FUNC(avr8_megaavr_timer_1_device::tifr_w));

	// USB PLL: pretend it's always locked for now
	map(0x0049, 0x0049).lrw8(
		NAME([this] { return m_r[0x49] | 1; }),
		NAME([this](uint8_t data) { m_r[0x49] = data; }));

	map(0x0065, 0x0065).rw(FUNC(atmegaxu4_base_device::reg_r<PRR1>), FUNC(atmegaxu4_base_device::prr1_w));

	map(0x006a, 0x006a).rw(FUNC(atmegaxu4_base_device::reg_r<EICRB>), FUNC(atmegaxu4_base_device::eicrb_w));

	map(0x0070, 0x0070).unmaprw(); // no Timer 2
	map(0x0071, 0x0071).rw("timer3", FUNC(avr8_megaavr_timer_1_device::timsk_r), FUNC(avr8_megaavr_timer_1_device::timsk_w));

	map(0x0090, 0x0090).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_a_r), FUNC(avr8_megaavr_timer_1_device::tccr_a_w));
	map(0x0091, 0x0091).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_b_r), FUNC(avr8_megaavr_timer_1_device::tccr_b_w));
	map(0x0092, 0x0092).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_c_r), FUNC(avr8_megaavr_timer_1_device::tccr_c_w));
	// 0x0093: reserved
	map(0x0094, 0x0094).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tcnt_l_r), FUNC(avr8_megaavr_timer_1_device::tcnt_l_w));
	map(0x0095, 0x0095).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tcnt_h_r), FUNC(avr8_megaavr_timer_1_device::tcnt_h_w));
	map(0x0096, 0x0096).rw("timer3", FUNC(avr8_megaavr_timer_1_device::icr_l_r), FUNC(avr8_megaavr_timer_1_device::icr_l_w));
	map(0x0097, 0x0097).rw("timer3", FUNC(avr8_megaavr_timer_1_device::icr_h_r), FUNC(avr8_megaavr_timer_1_device::icr_h_w));
	map(0x0098, 0x0098).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x0099, 0x0099).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x009a, 0x009a).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x009b, 0x009b).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x009c, 0x009c).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	map(0x009d, 0x009d).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>));

	map(0x00b0, 0x00b7).unmaprw(); // no Timer 2
}

void atmega32u4_device::atmega32u4_internal_map(address_map &map)
{
	atmegaxu4_base_internal_map(map);

	// TODO: install RAM
}

void atmegax4_base_device::atmegax4_base_internal_map(address_map &map)
{
	megaavr_classic_base_internal_map(map);

	map(0x0020, 0x0020).r(FUNC(avr8_device::pin_r<0>));
	map(0x0021, 0x0021).rw(FUNC(atmegax4_base_device::reg_r<DDRA>), FUNC(avr8_device::ddr_w<GPIOA>));
	map(0x0022, 0x0022).rw(FUNC(avr8_device::gpio_r<GPIOA>), FUNC(avr8_device::port_w<GPIOA>));

	map(0x0023, 0x0023).r(FUNC(avr8_device::pin_r<1>));
	map(0x0024, 0x0024).rw(FUNC(atmegax4_base_device::reg_r<DDRB>), FUNC(avr8_device::ddr_w<GPIOB>));
	map(0x0025, 0x0025).rw(FUNC(avr8_device::gpio_r<GPIOB>), FUNC(avr8_device::port_w<GPIOB>));

	map(0x0026, 0x0026).r(FUNC(avr8_device::pin_r<2>));
	map(0x0027, 0x0027).rw(FUNC(atmegax4_base_device::reg_r<DDRC>), FUNC(avr8_device::ddr_w<GPIOC>));
	map(0x0028, 0x0028).rw(FUNC(avr8_device::gpio_r<GPIOC>), FUNC(avr8_device::port_w<GPIOC>));

	map(0x0029, 0x0029).r(FUNC(avr8_device::pin_r<3>));
	map(0x002a, 0x002a).rw(FUNC(atmegax4_base_device::reg_r<DDRD>), FUNC(avr8_device::ddr_w<GPIOD>));
	map(0x002b, 0x002b).rw(FUNC(avr8_device::gpio_r<GPIOD>), FUNC(avr8_device::port_w<GPIOD>));

	map(0x0065, 0x0065).rw(FUNC(atmegax4_base_device::reg_r<PRR1>), FUNC(atmegax4_base_device::prr1_w));

	map(0x006c, 0x006c).rw(FUNC(atmegax4_base_device::reg_r<PCMSK1>), FUNC(atmegax4_base_device::pcmsk1_w));
	map(0x006d, 0x006d).rw(FUNC(atmegax4_base_device::reg_r<PCMSK2>), FUNC(atmegax4_base_device::pcmsk2_w));

	map(0x00c0, 0x00c0).rw(FUNC(atmegax4_base_device::reg_r<UCSR0A>), FUNC(atmegax4_base_device::ucsr0a_w));
	map(0x00c1, 0x00c1).rw(FUNC(atmegax4_base_device::reg_r<UCSR0B>), FUNC(atmegax4_base_device::ucsr0b_w));
	map(0x00c2, 0x00c2).rw(FUNC(atmegax4_base_device::reg_r<UCSR0C>), FUNC(atmegax4_base_device::ucsr0c_w));
	map(0x00c4, 0x00c4).rw(FUNC(atmegax4_base_device::reg_r<UBRR0L>), FUNC(atmegax4_base_device::reg_w<UBRR0L>));
	map(0x00c5, 0x00c5).rw(FUNC(atmegax4_base_device::reg_r<UBRR0H>), FUNC(atmegax4_base_device::reg_w<UBRR0H>));
	map(0x00c6, 0x00c6).rw(FUNC(atmegax4_base_device::reg_r<UDR0>), FUNC(atmegax4_base_device::reg_w<UDR0>));
}

void atmega644_device::atmega644_internal_map(address_map &map)
{
	atmegax4_base_internal_map(map);

	// TODO: install RAM
}

void atmega1284_device::atmega1284_internal_map(address_map &map)
{
	atmegax4_base_internal_map(map);
	map(0x005b, 0x005b).rw(FUNC(atmega1284_device::reg_r<RAMPZ>), FUNC(atmega1284_device::reg_w<RAMPZ>));

	// TODO: install RAM
}


void atmegax0_base_device::atmegax0_base_internal_map(address_map &map)
{
	megaavr_classic_base_internal_map(map);
	map(0x0020, 0x0020).r(FUNC(avr8_device::pin_r<0>));
	map(0x0021, 0x0021).rw(FUNC(atmegax0_base_device::reg_r<DDRA>), FUNC(avr8_device::ddr_w<GPIOA>));
	map(0x0022, 0x0022).rw(FUNC(avr8_device::gpio_r<GPIOA>), FUNC(avr8_device::port_w<GPIOA>));

	map(0x0023, 0x0023).r(FUNC(avr8_device::pin_r<1>));
	map(0x0024, 0x0024).rw(FUNC(atmegax0_base_device::reg_r<DDRB>), FUNC(avr8_device::ddr_w<GPIOB>));
	map(0x0025, 0x0025).rw(FUNC(avr8_device::gpio_r<GPIOB>), FUNC(avr8_device::port_w<GPIOB>));

	map(0x0026, 0x0026).r(FUNC(avr8_device::pin_r<2>));
	map(0x0027, 0x0027).rw(FUNC(atmegax0_base_device::reg_r<DDRC>), FUNC(avr8_device::ddr_w<GPIOC>));
	map(0x0028, 0x0028).rw(FUNC(avr8_device::gpio_r<GPIOC>), FUNC(avr8_device::port_w<GPIOC>));

	map(0x0029, 0x0029).r(FUNC(avr8_device::pin_r<3>));
	map(0x002a, 0x002a).rw(FUNC(atmegax0_base_device::reg_r<DDRD>), FUNC(avr8_device::ddr_w<GPIOD>));
	map(0x002b, 0x002b).rw(FUNC(avr8_device::gpio_r<GPIOD>), FUNC(avr8_device::port_w<GPIOD>));

	map(0x002c, 0x002c).r(FUNC(avr8_device::pin_r<4>));
	map(0x002d, 0x002d).rw(FUNC(atmegax0_base_device::reg_r<DDRE>), FUNC(avr8_device::ddr_w<GPIOE>));
	map(0x002e, 0x002e).rw(FUNC(avr8_device::gpio_r<GPIOE>), FUNC(avr8_device::port_w<GPIOE>));

	map(0x002f, 0x002f).r(FUNC(avr8_device::pin_r<5>));
	map(0x0030, 0x0030).rw(FUNC(atmegax0_base_device::reg_r<DDRF>), FUNC(avr8_device::ddr_w<GPIOF>));
	map(0x0031, 0x0031).rw(FUNC(avr8_device::gpio_r<GPIOF>), FUNC(avr8_device::port_w<GPIOF>));

	map(0x0032, 0x0032).r(FUNC(avr8_device::pin_r<6>));
	map(0x0033, 0x0033).rw(FUNC(atmegax0_base_device::reg_r<DDRG>), FUNC(avr8_device::ddr_w<GPIOG>));
	map(0x0034, 0x0034).rw(FUNC(avr8_device::gpio_r<GPIOG>), FUNC(avr8_device::port_w<GPIOG>));

	map(0x0038, 0x0038).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tifr_r), FUNC(avr8_megaavr_timer_1_device::tifr_w));
	map(0x0039, 0x0039).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tifr_r), FUNC(avr8_megaavr_timer_1_device::tifr_w));
	map(0x003a, 0x003a).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tifr_r), FUNC(avr8_megaavr_timer_1_device::tifr_w));

	map(0x005c, 0x005c).rw(FUNC(atmegax0_base_device::reg_r<EIND>), FUNC(atmegax0_base_device::reg_w<EIND>));

	map(0x0065, 0x0065).rw(FUNC(atmegax0_base_device::reg_r<PRR1>), FUNC(atmegax0_base_device::prr1_w));

	map(0x006c, 0x006c).rw(FUNC(atmegax0_base_device::reg_r<PCMSK1>), FUNC(atmegax0_base_device::pcmsk1_w));
	map(0x006d, 0x006d).rw(FUNC(atmegax0_base_device::reg_r<PCMSK2>), FUNC(atmegax0_base_device::pcmsk2_w));

	// 0x006e: TIMSK0
	map(0x0071, 0x0071).rw("timer3", FUNC(avr8_megaavr_timer_1_device::timsk_r), FUNC(avr8_megaavr_timer_1_device::timsk_w));
	map(0x0072, 0x0072).rw("timer4", FUNC(avr8_megaavr_timer_1_device::timsk_r), FUNC(avr8_megaavr_timer_1_device::timsk_w));
	map(0x0073, 0x0073).rw("timer5", FUNC(avr8_megaavr_timer_1_device::timsk_r), FUNC(avr8_megaavr_timer_1_device::timsk_w));

	map(0x0074, 0x0074).rw(FUNC(atmegax0_base_device::reg_r<XMCRA>), FUNC(atmegax0_base_device::xmcra_w));
	map(0x0075, 0x0075).rw(FUNC(atmegax0_base_device::reg_r<XMCRB>), FUNC(atmegax0_base_device::xmcrb_w));

	//
	map(0x0090, 0x0090).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_a_r), FUNC(avr8_megaavr_timer_1_device::tccr_a_w));
	map(0x0091, 0x0091).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_b_r), FUNC(avr8_megaavr_timer_1_device::tccr_b_w));
	map(0x0092, 0x0092).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tccr_c_r), FUNC(avr8_megaavr_timer_1_device::tccr_c_w));
	// 0x0093: reserved
	map(0x0094, 0x0094).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tcnt_l_r), FUNC(avr8_megaavr_timer_1_device::tcnt_l_w));
	map(0x0095, 0x0095).rw("timer3", FUNC(avr8_megaavr_timer_1_device::tcnt_h_r), FUNC(avr8_megaavr_timer_1_device::tcnt_h_w));
	map(0x0096, 0x0096).rw("timer3", FUNC(avr8_megaavr_timer_1_device::icr_l_r), FUNC(avr8_megaavr_timer_1_device::icr_l_w));
	map(0x0097, 0x0097).rw("timer3", FUNC(avr8_megaavr_timer_1_device::icr_h_r), FUNC(avr8_megaavr_timer_1_device::icr_h_w));
	map(0x0098, 0x0098).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x0099, 0x0099).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x009a, 0x009a).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x009b, 0x009b).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x009c, 0x009c).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	map(0x009d, 0x009d).rw("timer3", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>));


	// 0x009e, 0x009f: reserved

	map(0x00a0, 0x00a0).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tccr_a_r), FUNC(avr8_megaavr_timer_1_device::tccr_a_w));
	map(0x00a1, 0x00a1).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tccr_b_r), FUNC(avr8_megaavr_timer_1_device::tccr_b_w));
	map(0x00a2, 0x00a2).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tccr_c_r), FUNC(avr8_megaavr_timer_1_device::tccr_c_w));
	// 0x0093: reserved
	map(0x00a4, 0x00a4).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tcnt_l_r), FUNC(avr8_megaavr_timer_1_device::tcnt_l_w));
	map(0x00a5, 0x00a5).rw("timer4", FUNC(avr8_megaavr_timer_1_device::tcnt_h_r), FUNC(avr8_megaavr_timer_1_device::tcnt_h_w));
	map(0x00a6, 0x00a6).rw("timer4", FUNC(avr8_megaavr_timer_1_device::icr_l_r), FUNC(avr8_megaavr_timer_1_device::icr_l_w));
	map(0x00a7, 0x00a7).rw("timer4", FUNC(avr8_megaavr_timer_1_device::icr_h_r), FUNC(avr8_megaavr_timer_1_device::icr_h_w));
	map(0x00a8, 0x00a8).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x00a9, 0x00a9).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x00aa, 0x00aa).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x00ab, 0x00ab).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x00ac, 0x00ac).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	map(0x00ad, 0x00ad).rw("timer4", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>));

	map(0x00c0, 0x00c0).rw(FUNC(atmegax0_base_device::reg_r<UCSR0A>), FUNC(atmegax0_base_device::ucsr0a_w));
	map(0x00c1, 0x00c1).rw(FUNC(atmegax0_base_device::reg_r<UCSR0B>), FUNC(atmegax0_base_device::ucsr0b_w));
	map(0x00c2, 0x00c2).rw(FUNC(atmegax0_base_device::reg_r<UCSR0C>), FUNC(atmegax0_base_device::ucsr0c_w));
	map(0x00c4, 0x00c4).rw(FUNC(atmegax0_base_device::reg_r<UBRR0L>), FUNC(atmegax0_base_device::reg_w<UBRR0L>));
	map(0x00c5, 0x00c5).rw(FUNC(atmegax0_base_device::reg_r<UBRR0H>), FUNC(atmegax0_base_device::reg_w<UBRR0H>));
	map(0x00c6, 0x00c6).rw(FUNC(atmegax0_base_device::reg_r<UDR0>), FUNC(atmegax0_base_device::reg_w<UDR0>));

	map(0x0100, 0x0100).r(FUNC(avr8_device::pin_r<7>));
	map(0x0101, 0x0101).rw(FUNC(atmegax0_base_device::reg_r<DDRH>), FUNC(avr8_device::ddr_w<GPIOH>));
	map(0x0102, 0x0102).rw(FUNC(avr8_device::gpio_r<GPIOH>), FUNC(avr8_device::port_w<GPIOH>));

	map(0x0103, 0x0103).r(FUNC(avr8_device::pin_r<8>));
	map(0x0104, 0x0104).rw(FUNC(atmegax0_base_device::reg_r<DDRH>), FUNC(avr8_device::ddr_w<GPIOH>));
	map(0x0105, 0x0105).rw(FUNC(avr8_device::gpio_r<GPIOH>), FUNC(avr8_device::port_w<GPIOH>));

	map(0x0106, 0x0106).r(FUNC(avr8_device::pin_r<9>));
	map(0x0107, 0x0107).rw(FUNC(atmegax0_base_device::reg_r<DDRK>), FUNC(avr8_device::ddr_w<GPIOK>));
	map(0x0108, 0x0108).rw(FUNC(avr8_device::gpio_r<GPIOK>), FUNC(avr8_device::port_w<GPIOK>));

	map(0x0109, 0x0109).r(FUNC(avr8_device::pin_r<10>));
	map(0x010a, 0x010a).rw(FUNC(atmegax0_base_device::reg_r<DDRL>), FUNC(avr8_device::ddr_w<GPIOL>));
	map(0x010b, 0x010b).rw(FUNC(avr8_device::gpio_r<GPIOL>), FUNC(avr8_device::port_w<GPIOL>));

	// 0x010c-0x011f all reserved

	map(0x0120, 0x0120).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tccr_a_r), FUNC(avr8_megaavr_timer_1_device::tccr_a_w));
	map(0x0121, 0x0121).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tccr_b_r), FUNC(avr8_megaavr_timer_1_device::tccr_b_w));
	map(0x0122, 0x0122).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tccr_c_r), FUNC(avr8_megaavr_timer_1_device::tccr_c_w));
	// 0x0123: reserved
	map(0x0124, 0x0124).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tcnt_l_r), FUNC(avr8_megaavr_timer_1_device::tcnt_l_w));
	map(0x0125, 0x0125).rw("timer5", FUNC(avr8_megaavr_timer_1_device::tcnt_h_r), FUNC(avr8_megaavr_timer_1_device::tcnt_h_w));
	map(0x0126, 0x0126).rw("timer5", FUNC(avr8_megaavr_timer_1_device::icr_l_r), FUNC(avr8_megaavr_timer_1_device::icr_l_w));
	map(0x0127, 0x0127).rw("timer5", FUNC(avr8_megaavr_timer_1_device::icr_h_r), FUNC(avr8_megaavr_timer_1_device::icr_h_w));
	map(0x0128, 0x0128).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x0129, 0x0129).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_A>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>));
	map(0x012a, 0x012a).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x012b, 0x012b).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_B>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>));
	map(0x012c, 0x012c).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_l_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	map(0x012d, 0x012d).rw("timer5", FUNC(avr8_megaavr_timer_1_device::ocr_h_r<AVR8_TIMER_CMPCHANNEL_OC_C>), FUNC(avr8_megaavr_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>));
	// 0x012e, 0x012f both reserved

	// 0x0130-0x013f is USART 3
	// rest to 0x01ff all reserved
}

void atmega1280_device::atmega1280_internal_map(address_map &map)
{
	atmegax0_base_internal_map(map);
	map(0x005b, 0x005b).rw(FUNC(atmega1280_device::reg_r<RAMPZ>), FUNC(atmega1280_device::reg_w<RAMPZ>));
}

void atmega2560_device::atmega2560_internal_map(address_map &map)
{
	atmegax0_base_internal_map(map);
	map(0x005b, 0x005b).rw(FUNC(atmega2560_device::reg_r<RAMPZ>), FUNC(atmega2560_device::reg_w<RAMPZ>));
}


//-------------------------------------------------
//  atmegax8_base_device - constructor
//-------------------------------------------------

atmegax8_base_device::atmegax8_base_device(const machine_config &mconfig, const char *tag, device_t *owner,uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map)
	: megaavr_classic_base_device(mconfig, tag, owner, clock, type, address_mask, internal_map)
{
}

//-------------------------------------------------
//  atmega88_device - constructor
//-------------------------------------------------

atmega88_device::atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax8_base_device(mconfig, tag, owner, clock, ATMEGA88, 0x0fff, address_map_constructor(FUNC(atmega88_device::atmega88_internal_map), this))
{
}

bool atmega88_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOB: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOC: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOD: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

//-------------------------------------------------
//  atmega168_device - constructor
//-------------------------------------------------

atmega168_device::atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax8_base_device(mconfig, tag, owner, clock, ATMEGA168, 0x1fff, address_map_constructor(FUNC(atmega168_device::atmega168_internal_map), this))
{
}

//-------------------------------------------------
//  atmega328_device - constructor
//-------------------------------------------------

atmega328_device::atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax8_base_device(mconfig, tag, owner, clock, ATMEGA328, 0x3fff, address_map_constructor(FUNC(atmega328_device::atmega328_internal_map), this))
{
}

//-------------------------------------------------
//  atmegaxu4_base_device - constructor
//-------------------------------------------------

atmegaxu4_base_device::atmegaxu4_base_device(const machine_config &mconfig, const char *tag, device_t *owner,uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map)
	: megaavr_classic_base_device(mconfig, tag, owner, clock, type, address_mask, internal_map)
{
}

//-------------------------------------------------
//  atmega32u4_device - constructor
//-------------------------------------------------

atmega32u4_device::atmega32u4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegaxu4_base_device(mconfig, tag, owner, clock, ATMEGA32U4, 0x3fff, address_map_constructor(FUNC(atmega32u4_device::atmega32u4_internal_map), this))
{
}

//-------------------------------------------------
//  atmegax4_base_device - constructor
//-------------------------------------------------

atmegax4_base_device::atmegax4_base_device(const machine_config &mconfig, const char *tag, device_t *owner,uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map)
	: megaavr_classic_base_device(mconfig, tag, owner, clock, type, address_mask, internal_map)
{
}

//-------------------------------------------------
//  atmega644_device - constructor
//-------------------------------------------------

atmega644_device::atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax4_base_device(mconfig, tag, owner, clock, ATMEGA644, 0x7fff, address_map_constructor(FUNC(atmega644_device::atmega644_internal_map), this))
{
}

//-------------------------------------------------
//  atmega1284_device - constructor
//-------------------------------------------------

atmega1284_device::atmega1284_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax4_base_device(mconfig, tag, owner, clock, ATMEGA1284, 0xffff, address_map_constructor(FUNC(atmega1284_device::atmega1284_internal_map), this))
{
}


//-------------------------------------------------
//  atmegax0_base_device - constructor
//-------------------------------------------------

atmegax0_base_device::atmegax0_base_device(const machine_config &mconfig, const char *tag, device_t *owner,uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map)
	: megaavr_classic_base_device(mconfig, tag, owner, clock, type, address_mask, internal_map)
{
}

//-------------------------------------------------
//  atmega1280_device - constructor
//-------------------------------------------------

atmega1280_device::atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax0_base_device(mconfig, tag, owner, clock, ATMEGA1280, 0xffff, address_map_constructor(FUNC(atmega1280_device::atmega1280_internal_map), this))
{
}

//-------------------------------------------------
//  atmega2560_device - constructor
//-------------------------------------------------

atmega2560_device::atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atmegax0_base_device(mconfig, tag, owner, clock, ATMEGA2560, 0x1ffff, address_map_constructor(FUNC(atmega2560_device::atmega2560_internal_map), this))
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
	, m_pc(0)
	, m_addr_mask((addr_mask << 1) | 1)
	, m_sleeping(false)
	, m_sei_delay_pending(false)
	, m_int_ack_cbs(*this)
{
}

avr8_device::avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t addr_mask, address_map_constructor internal_map)
	: avr8_base_device(mconfig, tag, owner, clock, type, addr_mask, internal_map)
	, m_gpio_out_cb(*this)
	, m_gpio_in_cb(*this, 0)
	, m_adc_in_cb(*this, 0)
	, m_adc_timer(nullptr)
	, m_spi_active(false)
	, m_spi_prescale(0)
	, m_spi_prescale_count(0)
	, m_spi_rx_shift(0)
	, m_timers(*this, "timer%u", 0U)
	, m_syncprescale(*this, "syncprescale")
	, m_asyncunit(*this, "asyncunit")
{
}

megaavr_classic_base_device::megaavr_classic_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t addr_mask, address_map_constructor internal_map)
	: avr8_device(mconfig, tag, owner, clock, type, addr_mask, internal_map)
{
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
//  unimplemented_opcode - bail on unsupported
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
	save_item(NAME(m_r));
	save_item(NAME(m_boot_size));
	save_item(NAME(m_lfuses));
	save_item(NAME(m_hfuses));
	save_item(NAME(m_efuses));
	save_item(NAME(m_lock_bits));
	save_item(NAME(m_pc));

	// Misc.
	save_item(NAME(m_addr_mask));
	save_item(NAME(m_sleeping));
	save_item(NAME(m_sei_delay_pending));
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

	populate_interrupt_condition_table();
	std::fill_n(m_int_statuses, INTIDX_COUNT, 0);
	save_item(NAME(m_int_statuses));
}

void avr8_device::device_start()
{
	avr8_base_device::device_start();

	m_adc_timer = timer_alloc(FUNC(avr8_device::adc_conversion_complete), this);

	// GPIO
	save_item(NAME(m_gpio_in));
	save_item(NAME(m_pcint_last));

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
	save_item(NAME(m_spi_rx_shift));
}

void megaavr_classic_base_device::device_start()
{
	avr8_device::device_start();

	save_item(NAME(m_tsm));
	save_item(NAME(m_psrasy));
	save_item(NAME(m_psrsync));
}

void atmegax8_base_device::device_start()
{
	megaavr_classic_base_device::device_start();
}

void atmegax0_base_device::device_start()
{
	megaavr_classic_base_device::device_start();
}

void atmegax4_base_device::device_start()
{
	megaavr_classic_base_device::device_start();
}

void atmegaxu4_base_device::device_start()
{
	megaavr_classic_base_device::device_start();
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
		if (m_addr_mask <= 0x3fff) { m_boot_size = 1024; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 2048; }
		else { m_boot_size = 4096; }
		break;
	case 1:
		if (m_addr_mask <= 0x3fff) { m_boot_size = 512; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 1024; }
		else { m_boot_size = 2048; }
		break;
	case 2:
		if (m_addr_mask <= 0x3fff) { m_boot_size = 256; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 512; }
		else { m_boot_size = 1024; }
		break;
	case 3:
		if (m_addr_mask <= 0x3fff) { m_boot_size = 128; }
		else if (m_addr_mask <= 0x7fff) { m_boot_size = 256; }
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

	m_sleeping = false;
	m_sei_delay_pending = false;

	// clear any pending interrupts (the register zero loop above will have already
	// acknowledged any pending and disabled any that could have fired)
	for (int i = 0; i < INTIDX_COUNT; i++)
	{
		m_int_statuses[i] = 0;
	}
}

void avr8_device::device_reset()
{
	avr8_base_device::device_reset();

	m_adc_sample = 0;
	m_adc_result = 0;
	m_adc_data = 0;
	m_adc_first = true;
	m_adc_hold = false;

	m_spi_active = false;
	m_spi_prescale = 0;
	m_spi_prescale_count = 0;
	m_spi_rx_shift = 0;
}

void megaavr_classic_base_device::device_reset()
{
	avr8_device::device_reset();

	// assuming the prescalers will reset their sync flag for us
	m_tsm = false;
	m_psrasy = false;
	m_psrsync = false;
}

void atmegax8_base_device::device_reset()
{
	megaavr_classic_base_device::device_reset();
}

void atmegax0_base_device::device_reset()
{
	megaavr_classic_base_device::device_reset();
}

void atmegax4_base_device::device_reset()
{
	megaavr_classic_base_device::device_reset();
}

void atmegaxu4_base_device::device_reset()
{
	megaavr_classic_base_device::device_reset();
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void avr8_device::device_add_mconfig(machine_config &config)
{
	avr8_base_device::device_add_mconfig(config);

	// all classic AVRs will have a synchronous prescaler; instantiate it here
	AVR8_CLASSIC_10BIT_PRESCALER(config, m_syncprescale, 0);
}

void megaavr_classic_base_device::device_add_mconfig(machine_config &config)
{
	avr8_device::device_add_mconfig(config);

	// Timer 0 basic implementation is the same across the classic megaAVR lineup,
	// but signal routing differs. Each subclass device_add_mconfig() is responsible
	// for routing them.
	auto &timer0(AVR8_CLASSIC_MEGAAVR_TIMER_0(config, "timer0", 0));
	timer0.set_upstream_prescaler_tag(m_syncprescale);
	timer0.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_TOV0>));
	timer0.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF0A>));
	timer0.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF0B>));
	m_int_ack_cbs[INTIDX_TOV0].bind().set("timer0", FUNC(avr8_megaavr_timer_0_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF0A].bind().set("timer0", FUNC(avr8_megaavr_timer_0_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF0B].bind().set("timer0", FUNC(avr8_megaavr_timer_0_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));

	// The megaAVRs all use the same Timer 1 implementation, a 16-bit timer.
	// Any device that uses three-channel mode has to enable it in its device_add_mconfig().
	auto &timer1(AVR8_CLASSIC_MEGAAVR_TIMER_1(config, "timer1", 0));
	timer1.set_upstream_prescaler_tag(m_syncprescale);
	timer1.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_TOV1>));
	timer1.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF1A>));
	timer1.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF1B>));
	// OCF1C to be installed by sub-devices
	m_int_ack_cbs[INTIDX_TOV1].bind().set("timer1", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF1A].bind().set("timer1", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF1B].bind().set("timer1", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));

	// The asynchronous timer subsystem is present on all classic megaAVRs except for the ATMegaXU4 series
	auto &atimer2(AVR8_CLASSIC_MEGAAVR_FAKE_RTC_TIMER(config, "atimer2", 0));
	atimer2.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_TOV2>));
	atimer2.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF2A>));
	atimer2.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(megaavr_classic_base_device::int_line_w<INTIDX_OCF2B>));
	m_int_ack_cbs[INTIDX_TOV2].bind().set(atimer2, FUNC(avr8_generic_fakertc_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF2A].bind().set(atimer2, FUNC(avr8_generic_fakertc_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF2B].bind().set(atimer2, FUNC(avr8_generic_fakertc_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));

	auto &asyncunit(AVR8_CLASSIC_MEGAAVR_FAKE_RTC_ASYNC_CONTROLLER(config, "asyncunit", 0));

	// connect async unit, prescaler, and timer together
	asyncunit.set_child_timer_tag(atimer2.tag());
	atimer2.set_upstream_prescaler_tag("asyncunit:prescaler");
}

// ATMegax8 series (ATMega88, ATMega168, ATMega328)
void atmegax8_base_device::device_add_mconfig(machine_config &config)
{
	megaavr_classic_base_device::device_add_mconfig(config);

	// Timer 0 signals: OC0A = PD6, OC0B = PD5, T0 = PD4
	auto timer0 = subdevice<avr8_megaavr_timer_0_device>("timer0");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::set_gpio_bit<GPIOD, 6>, "set_gpio_bit_pd6");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOD, 6>, "toggle_gpio_bit_pd6");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::set_gpio_bit<GPIOD, 5>, "set_gpio_bit_pd5");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOD, 5>, "toggle_gpio_bit_pd5");
	// TODO: T0 pin

	// Timer 1: generic 16-bit
	// Timer 1 signals: OC1A = PB1, OC1B = PB2, T1 = PD5
	auto timer1 = subdevice<avr8_megaavr_timer_1_device>("timer1");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::set_gpio_bit<GPIOB, 1>, "set_gpio_bit_pb1");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOB, 1>, "toggle_gpio_bit_pb1");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::set_gpio_bit<GPIOB, 2>, "set_gpio_bit_pb2");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOB, 2>, "toggle_gpio_bit_pb2");
	// TODO: T1

	// Timer 2: 8-bit fake RTC
	// Timer 2 signals: OC2A = PB2, OC2B = PD3
	auto atimer2 = subdevice<avr8_generic_fakertc_timer_device>("atimer2");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::set_gpio_bit<GPIOB, 2>, "set_gpio_bit_pb2");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOB, 2>, "toggle_gpio_bit_pb2");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::set_gpio_bit<GPIOD, 3>, "set_gpio_bit_pd3");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax8_base_device::toggle_gpio_bit<GPIOD, 3>, "toggle_gpio_bit_pd3");
}


// ATMegaxU4 series (ATMega16U4, ATMega32U4)
void atmegaxu4_base_device::device_add_mconfig(machine_config &config)
{
	megaavr_classic_base_device::device_add_mconfig(config);

	auto &ocm(AVR8_CLASSIC_OUTPUT_COMPARE_MODULATOR(config, "oc0a_oc1c_ocm", 0));
	// TODO: route input from PORTx
	ocm.gpio_bit_w_cb().set(&atmegaxu4_base_device::set_gpio_bit<GPIOB, 7>, "set_gpio_bit_pb7");

	// Timer 0 signals: OC0A = PB7, OC0B = PD0, T0 = PD7
	auto timer0 = subdevice<avr8_megaavr_timer_0_device>("timer0");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_a_w));
	timer0->oc_en_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_en_a_w));
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_toggle_a_w));

	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegaxu4_base_device::set_gpio_bit<GPIOD, 0>, "set_gpio_bit_pd0");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegaxu4_base_device::toggle_gpio_bit<GPIOD, 0>, "toggle_gpio_bit_pd0");

	// Timer 1 signals: OC1A = PB5, OC1B = PB6, OC1C = PB7, T1 = PD6.
	auto timer1 = subdevice<avr8_megaavr_timer_1_device>("timer1");
	timer1->set_ocr_channel_c_enabled(true);
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegaxu4_base_device::set_gpio_bit<GPIOB, 5>, "set_gpio_bit_pb5");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegaxu4_base_device::toggle_gpio_bit<GPIOB, 5>, "toggle_gpio_bit_pb5");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegaxu4_base_device::set_gpio_bit<GPIOB, 6>, "set_gpio_bit_pb6");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegaxu4_base_device::toggle_gpio_bit<GPIOB, 6>, "toggle_gpio_bit_pb6");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_b_w));
	timer1->oc_en_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_en_b_w));
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_toggle_b_w));
	timer1->int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegaxu4_base_device::int_line_w<INTIDX_OCF1C>));

	// Timer 2 doesn't exist on these chips

	// Timer 3: 16-bit similar to timer 1. Important difference is no Tx pin.
	// Outputs: OC3A = PC6. There is no T3 pin.
	auto &timer3(AVR8_CLASSIC_MEGAAVR_TIMER_1(config, "timer3", 0));
	timer3.set_upstream_prescaler_tag(m_syncprescale);
	timer3.set_ocr_channel_c_enabled(true);
	timer3.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegaxu4_base_device::set_gpio_bit<GPIOC, 6>, "set_gpio_bit_pc6");
	timer3.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegaxu4_base_device::toggle_gpio_bit<GPIOC, 6>, "toggle_gpio_bit_pc6");

	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(atmegaxu4_base_device::int_line_w<INTIDX_TOV3>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(atmegaxu4_base_device::int_line_w<INTIDX_OCF3A>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(atmegaxu4_base_device::int_line_w<INTIDX_OCF3B>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegaxu4_base_device::int_line_w<INTIDX_OCF3C>));
	m_int_ack_cbs[INTIDX_TOV3].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF3A].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF3B].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));
	m_int_ack_cbs[INTIDX_OCF3C].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_C>));

	// Timer 4: 10-bit highspeed, driven by PLL or FCPU
	// Timer 4: OC4A = PC7, /OC4A = PC6,
	//          OC4B = PB6, /OC4B = PB5,
	//          OC4D = PD7, /OC4D = PD6
	// TODO: implement it, then hook it up here
}

// ATMegax0 series (ATMega640, ATMega1280, ATMega2560)
void atmegax0_base_device::device_add_mconfig(machine_config &config)
{
	megaavr_classic_base_device::device_add_mconfig(config);

	// Route output compare modulator: OC0A / OC1C -> PB7
	auto &ocm(AVR8_CLASSIC_OUTPUT_COMPARE_MODULATOR(config, "oc0a_oc1c_ocm", 0));
	// TODO: route input from bits
	ocm.gpio_bit_w_cb().set(&atmegax0_base_device::set_gpio_bit<GPIOB, 7>, "set_gpio_bit_pb7");

	// Timer 0 signals: OC0A = PB7 (via OCM), OC0B = PG5, T0 = PD7
	auto timer0 = subdevice<avr8_megaavr_timer_0_device>("timer0");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_a_w));
	timer0->oc_en_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_en_a_w));
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_toggle_a_w));
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOG, 5>, "set_gpio_bit_pg5");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOG, 5>, "toggle_gpio_bit_pg5");
	// TODO: set T0

	// Timer 1 signals: OC1A = PB5, OC1B = PB6, OC1C = PB7  (via OCM), T1 = PD6
	auto timer1 = subdevice<avr8_megaavr_timer_1_device>("timer1");
	timer1->set_ocr_channel_c_enabled(true);
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::set_gpio_bit<GPIOB, 5>, "set_gpio_bit_pb5");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOB, 5>, "toggle_gpio_bit_pb5");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOB, 6>, "set_gpio_bit_pb6");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOB, 6>, "toggle_gpio_bit_pb6");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_b_w));
	timer1->oc_en_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_en_b_w));
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(ocm, FUNC(avr8_output_compare_modulator_device::compare_out_toggle_b_w));
	timer1->int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF1C>));
	// TODO: set T1

	// Timer 2: 8-bit fake RTC
	// Timer 2 signals: OC2A = PB4, OC2B = PH6
	// TOSC1 = PG4, TOSC2 = PG3
	auto atimer2 = subdevice<avr8_generic_fakertc_timer_device>("atimer2");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::set_gpio_bit<GPIOB, 4>, "set_gpio_bit_pb4");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOB, 4>, "toggle_gpio_bit_pb4");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 6>, "set_gpio_bit_ph6");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 6>, "toggle_gpio_bit_ph6");

	// Timer 3 signals: OC3A = PE3, OC3B = PE4, OC3C = PE5, T3 = PE6
	auto &timer3(AVR8_CLASSIC_MEGAAVR_TIMER_1(config, "timer3", 0));
	timer3.set_upstream_prescaler_tag(m_syncprescale);
	timer3.set_ocr_channel_c_enabled(true);
	timer3.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::set_gpio_bit<GPIOE, 3>, "set_gpio_bit_pe3");
	timer3.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOE, 3>, "toggle_gpio_bit_pe3");
	timer3.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOE, 4>, "set_gpio_bit_pe4");
	timer3.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOB, 5>, "toggle_gpio_bit_pe4");
	timer3.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::set_gpio_bit<GPIOE, 5>, "set_gpio_bit_pe5");
	timer3.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOB, 5>, "toggle_gpio_bit_pe5");
	// TODO: set T3
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_TOV3>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF3A>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF3B>));
	timer3.int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF3C>));
	m_int_ack_cbs[INTIDX_TOV3].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF3A].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF3B].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));
	m_int_ack_cbs[INTIDX_OCF3C].bind().set("timer3", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_C>));

	// Timer 4 signals: OC4A = PH3, OC4B = PH4, OC4C = PH5, T4 = PH7
	// (PH7 is NOT a typo, PH6 is OC2B!)
	auto &timer4(AVR8_CLASSIC_MEGAAVR_TIMER_1(config, "timer4", 0));
	timer4.set_upstream_prescaler_tag(m_syncprescale);
	timer4.set_ocr_channel_c_enabled(true);
	timer4.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 3>, "set_gpio_bit_ph3");
	timer4.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 3>, "toggle_gpio_bit_ph3");
	timer4.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 4>, "set_gpio_bit_ph4");
	timer4.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 5>, "toggle_gpio_bit_ph4");
	timer4.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 5>, "set_gpio_bit_ph5");
	timer4.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 5>, "toggle_gpio_bit_ph5");
	// TODO: set T4
	timer4.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_TOV4>));
	timer4.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF4A>));
	timer4.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF4B>));
	timer4.int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF4C>));
	m_int_ack_cbs[INTIDX_TOV4].bind().set("timer4", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF4A].bind().set("timer4", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF4B].bind().set("timer4", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));
	m_int_ack_cbs[INTIDX_OCF4C].bind().set("timer4", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_C>));

	// Timer 5 signals: OC5A = PL3, OC5B = PL4, OC5C = PL5, T5 = PL2
	auto &timer5(AVR8_CLASSIC_MEGAAVR_TIMER_1(config, "timer5", 0));
	timer5.set_upstream_prescaler_tag(m_syncprescale);
	timer5.set_ocr_channel_c_enabled(true);
	timer5.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 3>, "set_gpio_bit_pl3");
	timer5.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 3>, "toggle_gpio_bit_pl3");
	timer5.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 4>, "set_gpio_bit_pl4");
	timer5.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 5>, "toggle_gpio_bit_pl4");
	timer5.oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::set_gpio_bit<GPIOH, 5>, "set_gpio_bit_pl5");
	timer5.oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_C>().set(&atmegax0_base_device::toggle_gpio_bit<GPIOH, 5>, "toggle_gpio_bit_pl5");
	// TODO: set T5
	timer5.int_cb<AVR8_TIMER_INTCHANNEL_OV>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_TOV5>));
	timer5.int_cb<AVR8_TIMER_INTCHANNEL_OC_A>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF5A>));
	timer5.int_cb<AVR8_TIMER_INTCHANNEL_OC_B>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF5B>));
	timer5.int_cb<AVR8_TIMER_INTCHANNEL_OC_C>().set(FUNC(atmegax0_base_device::int_line_w<INTIDX_OCF5C>));
	m_int_ack_cbs[INTIDX_TOV5].bind().set("timer5", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>));
	m_int_ack_cbs[INTIDX_OCF5A].bind().set("timer5", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>));
	m_int_ack_cbs[INTIDX_OCF5B].bind().set("timer5", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>));
	m_int_ack_cbs[INTIDX_OCF5C].bind().set("timer5", FUNC(avr8_megaavr_timer_1_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_C>));

	// NOTE: ATmega1281/2561 do have timers 4 and 5, but no physical ports to route their output signals to.
}

// ATMegax4 series (ATMega644, ATMega1284, ATMega2564)
void atmegax4_base_device::device_add_mconfig(machine_config &config)
{
	megaavr_classic_base_device::device_add_mconfig(config);

	// Timer 0 signals: OC0A = PB3, OC0B = PB4, T0 = PB0
	auto timer0 = subdevice<avr8_megaavr_timer_0_device>("timer0");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::set_gpio_bit<GPIOB, 3>, "set_gpio_bit_pb3");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOB, 3>, "toggle_gpio_bit_pb3");
	timer0->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::set_gpio_bit<GPIOB, 4>, "set_gpio_bit_pb4");
	timer0->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOB, 4>, "toggle_gpio_bit_pb4");

	// Timer 1 signals: OC1A = PD5, OC1B = PD4, T0 = PB1
	auto timer1 = subdevice<avr8_megaavr_timer_1_device>("timer1");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::set_gpio_bit<GPIOD, 5>, "set_gpio_bit_pd5");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOD, 5>, "toggle_gpio_bit_pd5");
	timer1->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::set_gpio_bit<GPIOD, 4>, "set_gpio_bit_pd4");
	timer1->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOD, 4>, "toggle_gpio_bit_pd4");

	// Timer 2: 8-bit fake RTC
	// Timer 2 signals: OC2A = PD7, OC2B = PD6
	// TOSC1 = PC6, TOSC2 = PC7
	auto atimer2 = subdevice<avr8_generic_fakertc_timer_device>("atimer2");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::set_gpio_bit<GPIOD, 7>, "set_gpio_bit_pd7");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_A>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOD, 7>, "toggle_gpio_bit_pd7");
	atimer2->oc_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::set_gpio_bit<GPIOD, 6>, "set_gpio_bit_pd6");
	atimer2->oc_toggle_w_cb<AVR8_TIMER_CMPCHANNEL_OC_B>().set(&atmegax4_base_device::toggle_gpio_bit<GPIOD, 6>, "toggle_gpio_bit_pd6");
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
		str = string_format("%05x", m_pc >> 1);
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

const avr8_base_device::interrupt_condition avr8_base_device::s_int_conditions[] =
{
	// irq id        vector            dev handled?  irq reg    irq reg mask        flag reg  flag reg mask
	{ INTIDX_SPI,    AVR8_INT_SPI_STC, 0,            SPCR,      SPCR_SPIE_MASK,     SPSR,     SPSR_SPIF_MASK   },
	{ INTIDX_OCF0B,  AVR8_INT_T0COMPB, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF0A,  AVR8_INT_T0COMPA, 1,            0,        0,                  0,        0, },
	{ INTIDX_TOV0,   AVR8_INT_T0OVF,   1,            0,        0,                  0,        0, },
	{ INTIDX_ICF1,   AVR8_INT_T1CAPT,  0,            TIMSK1,   TIMSK1_ICIE1_MASK,  TIFR1,    TIFR1_ICF1_MASK  },
	{ INTIDX_OCF1B,  AVR8_INT_T1COMPB, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1A,  AVR8_INT_T1COMPA, 1,            0,        0,                  0,        0, },
	{ INTIDX_TOV1,   AVR8_INT_T1OVF,   1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2B,  AVR8_INT_T2COMPB, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2A,  AVR8_INT_T2COMPA, 1,            0,        0,                  0,        0, },
	{ INTIDX_TOV2,   AVR8_INT_T2OVF,   1,            0,        0,                  0,        0, },
	{ INTIDX_PCINT0, AVR8_INT_PCINT0,  0,            PCICR,     PCICR_PCIE0_MASK,   PCIFR,    PCIFR_PCIF0_MASK },
	{ INTIDX_PCINT1, AVR8_INT_PCINT1,  0,            PCICR,     PCICR_PCIE1_MASK,   PCIFR,    PCIFR_PCIF1_MASK },
	{ INTIDX_PCINT2, AVR8_INT_PCINT2,  0,            PCICR,     PCICR_PCIE2_MASK,   PCIFR,    PCIFR_PCIF2_MASK },
	{ INTIDX_INT0,   AVR8_INT_INT0,    0,            EIMSK,     EIMSK_INT0_MASK,    EIFR,     EIFR_INTF0_MASK  },
	{ INTIDX_INT1,   AVR8_INT_INT1,    0,            EIMSK,     EIMSK_INT1_MASK,    EIFR,     EIFR_INTF1_MASK  },
	{ 0,             0,                0,            0,         0,                  0,        0,                } // end of list
};

void avr8_base_device::update_interrupt(uint8_t intidx)
{
	// it's preferred you go through set_interrupt() or clear_interrupt() directly
	// when that's the only operation that will be performed in that context.
	// failure to do this might lead to unwanted side effects.
	interrupt_condition condition = m_int_conditions_table[intidx];
	if (!(0 <= condition.m_intidx && condition.m_intidx < INTIDX_COUNT))
	{
		fatalerror("tried to update unmapped interrupt %d\n", intidx);
	}

	if (m_r[condition.m_regindex] & condition.m_regmask)
	{
		set_interrupt(intidx);
	}
	else
	{
		clear_interrupt(intidx);
	}
}

void avr8_base_device::set_interrupt_line(uint8_t intidx, uint8_t state)
{
	interrupt_condition condition = m_int_conditions_table[intidx];
	if (!(0 <= condition.m_intidx && condition.m_intidx < INTIDX_COUNT))
	{
		fatalerror("tried to set/clear unmapped interrupt %d\n", intidx);
	}

	m_int_statuses[intidx] = state != 0;
}

template<uint8_t IntIdx>
void avr8_base_device::int_line_w(uint8_t state)
{
	set_interrupt_line(IntIdx, state);
}

void avr8_base_device::set_interrupt(uint8_t intidx)
{
	interrupt_condition condition = m_int_conditions_table[intidx];
	if (!(0 <= condition.m_intidx && condition.m_intidx < INTIDX_COUNT))
	{
		fatalerror("tried to set unmapped interrupt %d\n", intidx);
	}

	if (!condition.m_intondev)
	{
		m_r[condition.m_regindex] |= condition.m_regmask;
		set_interrupt_line(intidx, (m_r[condition.m_intreg] & condition.m_intmask) != 0);
	}
	else
	{
		logerror("attempt to set intidx %d when mapped to a device\n", intidx);
	}
}

void avr8_base_device::clear_interrupt(uint8_t intidx)
{
	interrupt_condition condition = m_int_conditions_table[intidx];
	if (!(0 <= condition.m_intidx && condition.m_intidx < INTIDX_COUNT))
	{
		fatalerror("tried to clear unmapped interrupt %d\n", intidx);
	}

	if (condition.m_intondev)
	{
		if (m_int_ack_cbs[intidx].isunset())
		{
			fatalerror("intidx %d is mapped to a device, but no ack handler was installed\n", intidx);
		}

		m_int_ack_cbs[intidx](1);
	}
	else
	{
		// Clear event flag that raised the interrupt.
		// This will either happen because the MCU took the interrupt (if interrupts on),
		// or if the interrupt was manually acknowledged (if interrupts off).
		m_r[condition.m_regindex] &= ~condition.m_regmask;
	}

	set_interrupt_line(intidx, 0);
}

void avr8_base_device::fire_interrupts()
{
	int interrupt_winner_idx = -1;
	bool interrupt_firing = false;

	if (BIT(m_r[SREG], SREG_I) == 0)
	{
		return;
	}

	// Basic rules for interrupts handling on the AVR8 series:
	//
	// - When multiple interrupts arrive at the same time, the one with the lowest
	//   vector entry will win the race.
	//
	// - When an interrupt arrives when interrupts are disabled, it stays pending
	//   and its flag stays 1. The interrupt stays pending until the MCU fires it,
	//   or it's manually acknowledged (e.g., TOV0 is set to 1 by the user
	//   while interrupts are disabled).
	//
	//   The same thing happens when multiple interrupts are pending, and
	//   one or more of them loses the race; they simply get deferred until later.
	//
	// - The winning interrupt always clears its respective register flag
	//   and gets acknowledged. The interrupt enable flag will stay set regardless.
	//
	// - There should never be a case where we need a tiebreaker, i.e., every interrupt
	//   should be mapped to its own unique vector.
	for (int i = 0; i < INTIDX_COUNT; i++)
	{
		if (!m_int_statuses[i]) continue;

		interrupt_condition condition = m_int_conditions_table[i];
		if (!(0 <= condition.m_intidx && condition.m_intidx < INTIDX_COUNT))
		{
			fatalerror("interrupt %d set to fire, but wasn't mapped to anything", i);
		}

		if (!condition.m_intondev && !(m_r[condition.m_intreg] & condition.m_intmask))
		{
			logerror("%s: avr8 standard interrupt %d scheduled but its enable flag at %02x is cleared\n",
					 machine().describe_context(),
					 i,
					 condition.m_intreg
					 );
			continue;
		}

		interrupt_firing = true;
		if (interrupt_winner_idx == -1)
		{
			interrupt_winner_idx = i;
		}
		else if (condition.m_intvector >= m_int_conditions_table[interrupt_winner_idx].m_intvector)
		{
			LOGMASKED(LOG_INTERRUPTS, "interrupt %d already losing the race, currently winning is %d\n", i, interrupt_winner_idx);
		}
		else
		{
			LOGMASKED(LOG_INTERRUPTS, "interrupt %d now losing the race, currently winning is %d\n", i, interrupt_winner_idx);
			interrupt_winner_idx = i;
		}
	}

	if (interrupt_firing)
	{
		LOGMASKED(LOG_INTERRUPTS, "interrupt %d won the race, firing it.\n", interrupt_winner_idx);

		m_r[SREG] &= ~SREG_MASK_I;
		push((m_pc >> 1) & 0x00ff);
		push((m_pc >> 9) & 0x00ff);
		// TODO: 24-bit address pushes for 2560 and friends that use bigger flash space

		m_pc = m_int_conditions_table[interrupt_winner_idx].m_intvector * (2 * vector_size_in_words());

		m_sleeping = false;

		clear_interrupt(interrupt_winner_idx);
	}
}

inline bool avr8_base_device::interrupt_condition_entry_is_terminator(interrupt_condition condition)
{
	return std::all_of(
				(uint8_t*)&condition,
				(uint8_t*)&condition + sizeof(interrupt_condition),
				[] (uint8_t value) { return value == 0; });
}

void avr8_base_device::populate_interrupt_condition_table()
{
	const avr8_base_device::interrupt_condition* int_conds = interrupt_conditions();

	m_int_conditions_table = std::make_unique<avr8_base_device::interrupt_condition[]>(INTIDX_COUNT);

	for (int i = 0; i < INTIDX_COUNT; i++)
	{
		// unwire all vectors by default
		m_int_conditions_table[i].m_intidx = INTIDX_COUNT;
	}

	for (int i = 0; !interrupt_condition_entry_is_terminator(int_conds[i]); i++)
	{
		std::memcpy(&m_int_conditions_table[int_conds[i].m_intidx], &int_conds[i], sizeof(avr8_base_device::interrupt_condition));
	}
}

bool atmega168_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOB: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOC: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOD: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

bool atmega328_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOB: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOC: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOD: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

const avr8_base_device::interrupt_condition avr8_base_device::s_mega32u4_int_conditions[] =
{
	// intidx        vector                  dev handled?  irq reg   irq reg mask        flag reg  flag reg mask
	{ INTIDX_SPI,    ATMEGA32U4_INT_SPI_STC, 0,            SPCR,     SPCR_SPIE_MASK,     SPSR,     SPSR_SPIF_MASK,   },
	{ INTIDX_OCF0B,  ATMEGA32U4_INT_T0COMPB, 1,            TIMSK0,   TIMSK0_OCIE0B_MASK, TIFR0,    TIFR0_OCF0B_MASK, },
	{ INTIDX_OCF0A,  ATMEGA32U4_INT_T0COMPA, 1,            TIMSK0,   TIMSK0_OCIE0A_MASK, TIFR0,    TIFR0_OCF0A_MASK, },
	{ INTIDX_TOV0,   ATMEGA32U4_INT_T0OVF,   1,            TIMSK0,   TIMSK0_TOIE0_MASK,  TIFR0,    TIFR0_TOV0_MASK, },
	{ INTIDX_ICF1,   ATMEGA32U4_INT_T1CAPT,  0,            TIMSK1,   TIMSK1_ICIE1_MASK,  TIFR1,    TIFR1_ICF1_MASK, },
	{ INTIDX_OCF1C,  ATMEGA32U4_INT_T1COMPC, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1B,  ATMEGA32U4_INT_T1COMPB, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1A,  ATMEGA32U4_INT_T1COMPA, 1,            0,        0,                  0,        0, },
	{ INTIDX_TOV1,   ATMEGA32U4_INT_T1OVF,   1,            0,        0,                  0,        0, },
	// { INTIDX_ICF3,   ATMEGA32U4_INT_T1CAPT,  0,            TIMSK1,   TIMSK1_ICIE1_MASK,  TIFR1,    TIFR1_ICF1_MASK, },
	{ INTIDX_OCF3C,  ATMEGA32U4_INT_T3COMPC, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF3B,  ATMEGA32U4_INT_T3COMPB, 1,            0,        0,                  0,        0, },
	{ INTIDX_OCF3A,  ATMEGA32U4_INT_T3COMPA, 1,            0,        0,                  0,        0, },
	{ INTIDX_TOV3,   ATMEGA32U4_INT_T3OVF,   1,            0,        0,                  0,        0, },
	{ INTIDX_PCINT0, ATMEGA32U4_INT_PCINT0,  0,            PCICR,    PCICR_PCIE0_MASK,   PCIFR,    PCIFR_PCIF0_MASK, },
	{ INTIDX_INT0,   ATMEGA32U4_INT_INT0,    0,            EIMSK,    EIMSK_INT0_MASK,    EIFR,     EIFR_INTF0_MASK, },
	{ INTIDX_INT1,   ATMEGA32U4_INT_INT1,    0,            EIMSK,    EIMSK_INT1_MASK,    EIFR,     EIFR_INTF1_MASK, },
	{ 0,             0,                      0,            0,        0,                  0,        0, } // end of list
};

bool atmega32u4_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOB: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOC: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOD: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

const avr8_base_device::interrupt_condition avr8_base_device::s_mega640_int_conditions[avr8_base_device::INTIDX_COUNT + 1] =
{
	// irq id        vector                  dev handled?  irq reg   irq reg mask        flag reg  flag reg mask
	{ INTIDX_SPI,    ATMEGA640_INT_SPI_STC,  0,            SPCR,     SPCR_SPIE_MASK,     SPSR,     SPSR_SPIF_MASK },
	{ INTIDX_OCF0B,  ATMEGA640_INT_T0COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF0A,  ATMEGA640_INT_T0COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV0,   ATMEGA640_INT_T0OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_ICF1,   ATMEGA640_INT_T1CAPT,   0,            TIMSK1,   TIMSK1_ICIE1_MASK,  TIFR1,    TIFR1_ICF1_MASK },
	{ INTIDX_OCF1C,  ATMEGA640_INT_T1COMPC,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1B,  ATMEGA640_INT_T1COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1A,  ATMEGA640_INT_T1COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV1,   ATMEGA640_INT_T1OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2B,  ATMEGA640_INT_T2COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2A,  ATMEGA640_INT_T2COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV2,   ATMEGA640_INT_T2OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_PCINT0, ATMEGA640_INT_PCINT0,   0,            PCICR,    PCICR_PCIE0_MASK,   PCIFR,    PCIFR_PCIF0_MASK },
	{ INTIDX_PCINT1, ATMEGA640_INT_PCINT1,   0,            PCICR,    PCICR_PCIE1_MASK,   PCIFR,    PCIFR_PCIF1_MASK },
	{ INTIDX_PCINT2, ATMEGA640_INT_PCINT2,   0,            PCICR,    PCICR_PCIE2_MASK,   PCIFR,    PCIFR_PCIF2_MASK },
	{ INTIDX_INT0,   ATMEGA640_INT_INT0,     0,            EIMSK,    EIMSK_INT0_MASK,    EIFR,     EIFR_INTF0_MASK },
	{ INTIDX_INT1,   ATMEGA640_INT_INT1,     0,            EIMSK,    EIMSK_INT1_MASK,    EIFR,     EIFR_INTF1_MASK },
	{ INTIDX_OCF3A,  ATMEGA640_INT_T3COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF3B,  ATMEGA640_INT_T3COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF3C,  ATMEGA640_INT_T3COMPC,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV3,   ATMEGA640_INT_T3OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_OCF4A,  ATMEGA640_INT_T4COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF4B,  ATMEGA640_INT_T4COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF4C,  ATMEGA640_INT_T4COMPC,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV4,   ATMEGA640_INT_T4OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_OCF5A,  ATMEGA640_INT_T5COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF5B,  ATMEGA640_INT_T5COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF5C,  ATMEGA640_INT_T5COMPC,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV5,   ATMEGA640_INT_T5OVF,    1,            0,        0,                  0,        0, },
	{ 0,             0,                      0,            0,        0,                  0,        0, } // end of list
};

const avr8_base_device::interrupt_condition avr8_base_device::s_mega644_int_conditions[avr8_base_device::INTIDX_COUNT] =
{
	//  irq id        vector                  dev handled?  irq reg   irq reg mask        flag reg  flag reg mask
	{ INTIDX_SPI,     ATMEGA644_INT_SPI_STC,  0,            SPCR,     SPCR_SPIE_MASK,     SPSR,     SPSR_SPIF_MASK },
	{ INTIDX_OCF0B,   ATMEGA644_INT_T0COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF0A,   ATMEGA644_INT_T0COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV0,    ATMEGA644_INT_T0OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_ICF1,    ATMEGA644_INT_T1CAPT,   0,            TIMSK1,   TIMSK1_ICIE1_MASK,  TIFR1,    TIFR1_ICF1_MASK },
	{ INTIDX_OCF1B,   ATMEGA644_INT_T1COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF1A,   ATMEGA644_INT_T1COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV1,    ATMEGA644_INT_T1OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2B,   ATMEGA644_INT_T2COMPB,  1,            0,        0,                  0,        0, },
	{ INTIDX_OCF2A,   ATMEGA644_INT_T2COMPA,  1,            0,        0,                  0,        0, },
	{ INTIDX_TOV2,    ATMEGA644_INT_T2OVF,    1,            0,        0,                  0,        0, },
	{ INTIDX_PCINT0,  ATMEGA644_INT_PCINT0,   0,            PCICR,    PCICR_PCIE0_MASK,   PCIFR,    PCIFR_PCIF0_MASK },
	{ INTIDX_PCINT1,  ATMEGA644_INT_PCINT1,   0,            PCICR,    PCICR_PCIE1_MASK,   PCIFR,    PCIFR_PCIF1_MASK },
	{ INTIDX_PCINT2,  ATMEGA644_INT_PCINT2,   0,            PCICR,    PCICR_PCIE2_MASK,   PCIFR,    PCIFR_PCIF2_MASK },
	{ INTIDX_INT0,    ATMEGA644_INT_INT0,     0,            EIMSK,    EIMSK_INT0_MASK,    EIFR,     EIFR_INTF0_MASK },
	{ INTIDX_INT1,    ATMEGA644_INT_INT1,     0,            EIMSK,    EIMSK_INT1_MASK,    EIFR,     EIFR_INTF1_MASK },
	{ 0,              0,                      0,            0,        0,                  0,        0, } // end of list
};

bool atmega644_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOA: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOB: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOC: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

bool atmega1284_device::pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const
{
	switch (port)
	{
	case GPIOA: pcmsk_reg = PCMSK0; group = 0; return true;
	case GPIOB: pcmsk_reg = PCMSK1; group = 1; return true;
	case GPIOC: pcmsk_reg = PCMSK2; group = 2; return true;
	default: return false;
	}
}

//**************************************************************************
//  PERIPHERAL HANDLING
//**************************************************************************

// Master-mode SPI Mode 0 only (CPOL=0/CPHA=0)
void avr8_device::spi_tick()
{
	uint8_t mosi_mask, miso_mask, sck_mask;
	spi_pins(mosi_mask, miso_mask, sck_mask);

	const int bit_index = SPCR_DORD ? (7 - m_spi_prescale_countdown) : m_spi_prescale_countdown;
	const uint8_t out_bit = BIT(m_r[SPDR], bit_index);

	// SCK rising edge: MOSI is sampled by the slave
	uint8_t data = (m_r[PORTB] & ~(mosi_mask | sck_mask)) | (out_bit ? mosi_mask : 0) | sck_mask;
	m_r[PORTB] = data;
	m_gpio_out_cb[GPIOB](data);

	const uint8_t in_bit = (m_gpio_in_cb[GPIOB]() & miso_mask) ? 1 : 0;
	m_spi_rx_shift = (m_spi_rx_shift << 1) | in_bit;

	// SCK falling edge: the slave shifts out its next MISO bit
	data &= ~sck_mask;
	m_r[PORTB] = data;
	m_gpio_out_cb[GPIOB](data);

	m_spi_prescale_countdown--;

	if (m_spi_prescale_countdown < 0)
	{
		m_r[SPDR] = m_spi_rx_shift;
		set_interrupt(INTIDX_SPI);
		m_spi_active = false;
	}
}

/************************************************************************************************/


/****************/
/* ADC Handling */
/****************/

void avr8_device::adc_start_conversion()
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

TIMER_CALLBACK_MEMBER(avr8_device::adc_conversion_complete)
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

void avr8_device::enable_spi()
{
	// TODO
}

void avr8_device::disable_spi()
{
	m_spi_active = false;
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
	m_spi_prescale = s_spi_clock_divisor[SPI_RATE];
	m_spi_prescale_count &= m_spi_prescale - 1;
}

void avr8_device::change_spcr(uint8_t data)
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

void avr8_device::change_spsr(uint8_t data)
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

void megaavr_classic_base_device::gtccr_w(uint8_t data)
{
	m_tsm = BIT(data, 7);
	m_psrasy = BIT(data, 1);
	m_psrsync = BIT(data, 0);

	m_syncprescale->sync_w(m_psrsync != 0);
	// if (m_asyncunit) m_asyncunit->sync_w(m_psrasy != 0);

	// if TSM not active, PSRASY and PSRSYNC will automatically clear on the same cycle
	if (!m_tsm)
	{
		m_psrasy = false;
		m_psrsync = false;

		m_syncprescale->sync_w(false);
		// if (m_asyncunit) m_asyncunit->sync_w(false);
	}
}

uint8_t megaavr_classic_base_device::gtccr_r()
{
	return (m_tsm << 7)    |
		   (m_psrasy << 1) |
		   (m_psrsync << 0);
}

template <int Port>
uint8_t avr8_device::gpio_r()
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

template <int Port>
uint8_t avr8_device::pin_r()
{
	static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
	// TODO: account for DDRH
	const uint8_t data = sample_port<Port>();
	LOGMASKED(LOG_GPIO, "%s: PIN%c(%d) Read: %02x\n", machine().describe_context(), PORT_CHARS[Port], Port, data);
	return data;
}

template <int Port>
uint8_t avr8_device::sample_port()
{
	return m_gpio_in_cb[Port].isunset() ? m_gpio_in[Port] : m_gpio_in_cb[Port]();
}

template <int Port>
void avr8_device::port_w(uint8_t data)
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

	// pins configured as inputs (DDR bit clear) are not actively driven, so they never pull the line low
	const uint8_t ddr = m_r[PORT_TO_REG[Port] - 1];
	m_gpio_out_cb[Port](data | ~ddr);
}

template <int Port>
void avr8_device::ddr_w(uint8_t data)
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

	LOGMASKED(LOG_GPIO, "%s: DDR%c Write: %02x\n", machine().describe_context(), PORT_CHARS[Port], data);
	m_r[PORT_TO_REG[Port] - 1] = data;

	// switching a pin between input and output changes what it drives even without a PORT write
	const uint8_t port = m_r[PORT_TO_REG[Port]];
	m_gpio_out_cb[Port](port | ~data);
}

void avr8_device::eecr_w(uint8_t data)
{
	m_r[EECR] = data & EECR_MASK;

	if (data & EECR_EERE_MASK)
	{
		uint16_t addr = (m_r[EEARH] & eearh_mask()) << 8;
		addr |= m_r[EEARL];
		m_r[EEDR] = m_eeprom[addr];
		LOGMASKED(LOG_EEPROM, "%s: EEPROM read @ %04x data = %02x\n", machine().describe_context(), addr, m_eeprom[addr]);
	}
	if ((data & EECR_EEPE_MASK) && (data & EECR_EEMPE_MASK))
	{
		uint16_t addr = (m_r[EEARH] & eearh_mask()) << 8;
		addr |= m_r[EEARL];
		m_eeprom[addr] = m_r[EEDR];
		LOGMASKED(LOG_EEPROM, "%s: EEPROM write @ %04x data = %02x ('%c')\n", machine().describe_context(), addr, m_eeprom[addr], m_eeprom[addr] >= 0x21 ? m_eeprom[addr] : ' ');

		// Indicate that we've finished writing a value to the EEPROM.
		// TODO: this should only happen after a certain dalay.
		m_r[EECR] = data & ~EECR_EEPE_MASK;
	}
}

void avr8_device::gpior0_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR0 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR0] = data;
}

void avr8_device::gpior1_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR1 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR1] = data;
}

void avr8_device::gpior2_w(uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: GPIOR2 Write: %02x\n", machine().describe_context(), data);
	m_r[GPIOR2] = data;
}

void avr8_device::spsr_w(uint8_t data)
{
	change_spsr(data);
}

void avr8_device::spcr_w(uint8_t data)
{
	change_spcr(data);
}

void avr8_device::spdr_w(uint8_t data)
{
	m_r[SPDR] = data;
	m_r[SPSR] &= ~SPSR_SPIF_MASK; // note: can't clear this flag by writing 1 there
	m_spi_rx_shift = 0;
	m_spi_active = true;
	m_spi_prescale_countdown = 7;
	m_spi_prescale_count = 0;
}

void avr8_device::wdtcsr_w(uint8_t data)
{
	LOGMASKED(LOG_WDOG, "%s: (not yet implemented) WDTCSR = %02x\n", machine().describe_context(), data);
}

void avr8_device::clkpr_w(uint8_t data)
{
	LOGMASKED(LOG_CLOCK, "%s: (not yet implemented) CLKPR = %02x\n", machine().describe_context(), data);
}

void avr8_device::prr0_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR0 = %02x\n", machine().describe_context(), data);
}

void avr8_device::prr1_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) PRR1 = %02x\n", machine().describe_context(), data);
}

void avr8_device::osccal_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) OSCCAL = %02x\n", machine().describe_context(), data);
}

void avr8_device::pcicr_w(uint8_t data)
{
	LOGMASKED(LOG_PINCHG, "%s: PCICR = %02x\n", machine().describe_context(), data);
	m_r[PCICR] = data;
}

void avr8_device::pcifr_w(uint8_t data)
{
	// PCIFR is write-one-to-clear: writing a 1 to a PCIFn bit clears it, it does not request an interrupt
	LOGMASKED(LOG_PINCHG, "%s: PCIFR = %02x\n", machine().describe_context(), data);
	m_r[PCIFR] &= ~(data & (PCIFR_PCIF0_MASK | PCIFR_PCIF1_MASK | PCIFR_PCIF2_MASK));
}

void avr8_device::eicra_w(uint8_t data)
{
	LOGMASKED(LOG_PINCHG, "%s: EICRA = %02x\n", machine().describe_context(), data);
	m_r[EICRA] = data;
}

void avr8_device::eifr_w(uint8_t data)
{
	// EIFR is write-one-to-clear: writing a 1 to an INTFn bit clears it, it does not request an interrupt
	LOGMASKED(LOG_PINCHG, "%s: EIFR = %02x\n", machine().describe_context(), data);
	m_r[EIFR] &= ~(data & (EIFR_INTF0_MASK | EIFR_INTF1_MASK));
}

void avr8_device::eicrb_w(uint8_t data)
{
	LOGMASKED(LOG_POWER, "%s: (not yet implemented) EICRB = %02x\n", machine().describe_context(), data);
}

void avr8_device::pcmsk0_w(uint8_t data)
{
	LOGMASKED(LOG_PINCHG, "%s: PCMSK0 = %02x\n", machine().describe_context(), data);
	m_r[PCMSK0] = data;
}

void avr8_device::pcmsk1_w(uint8_t data)
{
	LOGMASKED(LOG_PINCHG, "%s: PCMSK1 = %02x\n", machine().describe_context(), data);
	m_r[PCMSK1] = data;
}

void avr8_device::pcmsk2_w(uint8_t data)
{
	LOGMASKED(LOG_PINCHG, "%s: PCMSK2 = %02x\n", machine().describe_context(), data);
	m_r[PCMSK2] = data;
}

void avr8_device::check_extint(int line, uint8_t prev, uint8_t state)
{
	const int bit = 2 + line; // INT0 = PD2, INT1 = PD3
	if (!(m_r[EIMSK] & (1 << line)))
		return;

	const int isc = (m_r[EICRA] >> (line * 2)) & 0x03;
	bool fire;
	switch (isc)
	{
		case 0: fire = !BIT(state, bit); break;                          // low level
		case 1: fire = BIT(prev, bit) != BIT(state, bit); break;         // any logical change
		case 2: fire = BIT(prev, bit) && !BIT(state, bit); break;        // falling edge
		default: fire = !BIT(prev, bit) && BIT(state, bit); break;       // rising edge
	}

	if (fire)
	{
		LOGMASKED(LOG_PINCHG, "%s: INT%d triggered (ISC=%d)\n", machine().describe_context(), line, isc);
		m_r[EIFR] |= (1 << line);
	}
}

template <int Port>
void avr8_device::latch_pin_change(uint8_t state)
{
	const uint8_t prev = m_pcint_last[Port];
	const uint8_t changed = state ^ prev;
	m_pcint_last[Port] = state;
	if (!changed)
		return;

	// only latch flags here: this can be called from an arbitrary, possibly mid-instruction
	// context (another device driving this port), so the actual interrupt-take (which pushes the
	// return PC) must wait for execute_run()'s instruction boundary, not happen synchronously here
	uint8_t pcmsk_reg = 0;
	int group = 0;
	if (pcint_group(gpio_t(Port), pcmsk_reg, group) && (changed & m_r[pcmsk_reg]))
	{
		static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
		LOGMASKED(LOG_PINCHG, "%s: PCINT%c pins changed: %02x\n", machine().describe_context(), PORT_CHARS[Port], changed);
		m_r[PCIFR] |= (1 << group);
	}

	// INT0 (PD2) / INT1 (PD3) share the same fixed pins across every supported chip
	if (Port == GPIOD)
	{
		if (BIT(changed, 2))
			check_extint(0, prev, state);
		if (BIT(changed, 3))
			check_extint(1, prev, state);
	}
}

template<int Port, int Bit>
void avr8_device::set_gpio_bit(uint8_t data)
{
	// static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
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

	auto reg = PORT_TO_REG[Port];

	if (data)
		m_r[reg] |= 1 << Bit;
	else
		m_r[reg] &= ~(1 << Bit);

	m_gpio_out_cb[Port](m_r[reg]);
}

template<int Port, int Bit>
void avr8_device::toggle_gpio_bit(uint8_t _)
{
	// static constexpr char PORT_CHARS[GPIO_COUNT] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L' };
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

	auto reg = PORT_TO_REG[Port];

	m_r[PORT_TO_REG[Port]] ^= 1 << Bit;
	m_gpio_out_cb[Port](m_r[reg]);
}

template <int Port>
void avr8_device::pin_w(int bit, int state)
{
	if (state)
		m_gpio_in[Port] |= (1 << bit);
	else
		m_gpio_in[Port] &= ~(1 << bit);

	latch_pin_change<Port>(sample_port<Port>());
}

void avr8_device::xmcra_w(uint8_t data)
{
	LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRA = %02x\n", machine().describe_context(), data);
}

void avr8_device::xmcrb_w(uint8_t data)
{
	LOGMASKED(LOG_EXTMEM, "%s: (not yet implemented) XMCRB = %02x\n", machine().describe_context(), data);
}

uint8_t avr8_device::adcl_r()
{
	if (!machine().side_effects_disabled())
		m_adc_hold = true;
	if (ADMUX_ADLAR)
		return (m_adc_data & 0x03) << 6;
	else
		return uint8_t(m_adc_data);
}

void avr8_device::adcl_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCL = %02x\n", machine().describe_context(), data);
}

uint8_t avr8_device::adch_r()
{
	uint8_t const result = ADMUX_ADLAR ? BIT(m_adc_data, 2, 8) : BIT(m_adc_data, 8, 2);
	if (!machine().side_effects_disabled())
	{
		m_adc_data = m_adc_result;
		m_adc_hold = false;
	}
	return result;
}

void avr8_device::adch_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCH = %02x\n", machine().describe_context(), data);
}

void avr8_device::adcsra_w(uint8_t data)
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

void avr8_device::adcsrb_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADCSRB = %02x\n", machine().describe_context(), data);
	m_r[ADCSRB] = data & (ADCSRB_ACME_MASK | ADCSRB_ADTS_MASK);
	if (ADCSRB_ADTS != 0)
		logerror("%s: Unimplemented ADC auto trigger source %X selected\n", machine().describe_context(), ADCSRB_ADTS);
}

void avr8_device::admux_w(uint8_t data)
{
	LOGMASKED(LOG_ADC, "%s: ADMUX = %02x\n", machine().describe_context(), data);
	m_r[ADMUX] = data & (ADMUX_REFS_MASK | ADMUX_ADLAR_MASK | ADMUX_MUX_MASK);
}

void avr8_device::didr0_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR0 = %02x\n", machine().describe_context(), data);
}

void avr8_device::didr1_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR1 = %02x\n", machine().describe_context(), data);
}

void avr8_device::didr2_w(uint8_t data)
{
	LOGMASKED(LOG_DIGINPUT, "%s: (not yet implemented) DIDR2 = %02x\n", machine().describe_context(), data);
}

void avr8_device::twbr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWBR = %02x\n", machine().describe_context(), data);
}

uint8_t avr8_device::twsr_r()
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) Read TWSR: %02x\n", machine().describe_context(), 0);
	// HACK for Replicator 1:
	//   By returning a value != 0x08, we induce an error state that makes the object code jump out of the wait loop,
	//   and continue execution failing the 2-wire write operation.
	return 0x00;
}

void avr8_device::twsr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWSR = %02x\n", machine().describe_context(), data);
}

void avr8_device::twar_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAR = %02x\n", machine().describe_context(), data);
}

void avr8_device::twdr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWDR = %02x\n", machine().describe_context(), data);
}

void avr8_device::twcr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWCR = %02x\n", machine().describe_context(), data);
	m_r[TWCR] = data;
}

void avr8_device::twamr_w(uint8_t data)
{
	LOGMASKED(LOG_TWI, "%s: (not yet implemented) TWAMR = %02x\n", machine().describe_context(), data);
}

void avr8_device::ucsr0a_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0A = %02x\n", machine().describe_context(), data);
}

void avr8_device::ucsr0b_w(uint8_t data)
{
	LOGMASKED(LOG_UART, "%s: (not yet implemented) UCSR0B = %02x\n", machine().describe_context(), data);
}

void avr8_device::ucsr0c_w(uint8_t data)
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

void avr8_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_sleeping)
		{
			debugger_wait_hook();
			m_opcycles = 1;
		}
		else
		{
			m_pc &= m_addr_mask;
			debugger_instruction_hook(m_pc);

			const uint16_t op = (uint32_t)m_program->read_word(m_pc);
			m_opcycles = m_op_cycles[op];
			((this)->*(m_op_funcs[op]))(op);
			m_pc += 2;
			m_sei_delay_pending = false;
		}

		// pin_w() may have latched a PCIFR/EIFR flag from an arbitrary (possibly mid-instruction)
		// external context; only take the actual interrupt here, at a safe instruction boundary
		if (m_r[PCIFR])
		{
			update_interrupt(INTIDX_PCINT0);
			update_interrupt(INTIDX_PCINT1);
			update_interrupt(INTIDX_PCINT2);
		}

		if (m_r[EIFR])
		{
			update_interrupt(INTIDX_INT0);
			update_interrupt(INTIDX_INT1);
		}

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

			m_syncprescale->tick();
			for (int i = 0; i < m_timers.size(); i++)
			{
				if (m_timers[i])
				{
					// this is for synchronous timers ONLY.
					// if you name your timer "timerX" it will be ticked here!
					m_timers[i]->tick();
				}
			}

			if (m_asyncunit != NULL)
			{
				m_asyncunit->clk_io_w(1);
				m_asyncunit->clk_io_w(0);
			}

			if (!m_sei_delay_pending)
			{
				fire_interrupts();
			}
		}
	}
}

// explicit instantiations so external devices can call pin_w<Port>() to report input pin changes
template void avr8_device::pin_w<avr8_base_device::GPIOA>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOB>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOC>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOD>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOE>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOF>(int, int);
template void avr8_device::pin_w<avr8_base_device::GPIOG>(int, int);
