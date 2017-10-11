// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_XS32DEFS_H
#define MAME_CPU_E132XS_XS32DEFS_H

/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/

#define PC_REGISTER          0
#define SR_REGISTER          1
#define BCR_REGISTER        20
#define TPR_REGISTER        21
#define TCR_REGISTER        22
#define TR_REGISTER         23
#define ISR_REGISTER        25
#define FCR_REGISTER        26
#define MCR_REGISTER        27

#define X_CODE(val)      ((val & 0x7000) >> 12)
#define E_BIT(val)       ((val & 0x8000) >> 15)
#define S_BIT_CONST(val) ((val & 0x4000) >> 14)
#define DD(val)          ((val & 0x3000) >> 12)


/* Extended DSP instructions */
#define EMUL            0x102
#define EMULU           0x104
#define EMULS           0x106
#define EMAC            0x10a
#define EMACD           0x10e
#define EMSUB           0x11a
#define EMSUBD          0x11e
#define EHMAC           0x02a
#define EHMACD          0x02e
#define EHCMULD         0x046
#define EHCMACD         0x04e
#define EHCSUMD         0x086
#define EHCFFTD         0x096
#define EHCFFTSD        0x296

/* Delay values */
#define NO_DELAY        0
#define DELAY_EXECUTE   1

/* IRQ numbers */
#define IRQ_INT1        0
#define IRQ_INT2        1
#define IRQ_INT3        2
#define IRQ_INT4        3
#define IRQ_IO1         4
#define IRQ_IO2         5
#define IRQ_IO3         6

/* Trap numbers */
#define TRAPNO_IO2                  48
#define TRAPNO_IO1                  49
#define TRAPNO_INT4             50
#define TRAPNO_INT3             51
#define TRAPNO_INT2             52
#define TRAPNO_INT1             53
#define TRAPNO_IO3                  54
#define TRAPNO_TIMER                55
#define TRAPNO_RESERVED1            56
#define TRAPNO_TRACE_EXCEPTION      57
#define TRAPNO_PARITY_ERROR     58
#define TRAPNO_EXTENDED_OVERFLOW    59
#define TRAPNO_RANGE_ERROR          60
#define TRAPNO_PRIVILEGE_ERROR      TRAPNO_RANGE_ERROR
#define TRAPNO_FRAME_ERROR          TRAPNO_RANGE_ERROR
#define TRAPNO_RESERVED2            61
#define TRAPNO_RESET                62  // reserved if not mapped @ MEM3
#define TRAPNO_ERROR_ENTRY          63  // for instruction code of all ones

/* Trap codes */
#define TRAPLE      4
#define TRAPGT      5
#define TRAPLT      6
#define TRAPGE      7
#define TRAPSE      8
#define TRAPHT      9
#define TRAPST      10
#define TRAPHE      11
#define TRAPE       12
#define TRAPNE      13
#define TRAPV       14
#define TRAP        15

/* Entry point to get trap locations or emulated code associated */
#define E132XS_ENTRY_MEM0   0
#define E132XS_ENTRY_MEM1   1
#define E132XS_ENTRY_MEM2   2
#define E132XS_ENTRY_IRAM   3
#define E132XS_ENTRY_MEM3   7

#endif // MAME_CPU_E132XS_XS32DEFS_H
