// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3com.h

    Common MIPS III/IV definitions and functions

***************************************************************************/

#pragma once

#ifndef __MIPS3COM_H__
#define __MIPS3COM_H__

#include "mips3.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_TLB              (0)
#define USE_ABI_REG_NAMES       (1)

#define DISABLE_FAST_REGISTERS          (0)
#define SINGLE_INSTRUCTION_MODE         (0)

#define PRINTF_EXCEPTIONS               (0)
#define PRINTF_MMU                      (0)

#define PROBE_ADDRESS                   ~0


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1

/* modes */
#define MODE_KERNEL                     0
#define MODE_SUPER                      1
#define MODE_USER                       2

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3



#define LOPTR(x)                ((UINT32 *)(x) + NATIVE_ENDIAN_VALUE_LE_BE(0,1))


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* core parameters */
#define MIPS3_MIN_PAGE_SHIFT        12
#define MIPS3_MIN_PAGE_SIZE         (1 << MIPS3_MIN_PAGE_SHIFT)
#define MIPS3_MIN_PAGE_MASK         (MIPS3_MIN_PAGE_SIZE - 1)
#define MIPS3_MAX_PADDR_SHIFT       32

/* cycle parameters */
#define MIPS3_COUNT_READ_CYCLES     250
#define MIPS3_CAUSE_READ_CYCLES     250

/* TLB bits */
#define TLB_GLOBAL              0x01
#define TLB_VALID               0x02
#define TLB_DIRTY               0x04
#define TLB_PRESENT             0x08

/* COP0 registers */
#define COP0_Index              0
#define COP0_Random             1
#define COP0_EntryLo            2
#define COP0_EntryLo0           2
#define COP0_EntryLo1           3
#define COP0_Context            4
#define COP0_PageMask           5
#define COP0_Wired              6
#define COP0_BadVAddr           8
#define COP0_Count              9
#define COP0_EntryHi            10
#define COP0_Compare            11
#define COP0_Status             12
#define COP0_Cause              13
#define COP0_EPC                14
#define COP0_PRId               15
#define COP0_Config             16
#define COP0_LLAddr             17
#define COP0_XContext           20
#define COP0_ECC                26
#define COP0_CacheErr           27
#define COP0_TagLo              28
#define COP0_TagHi              29
#define COP0_ErrorPC            30

/* Status register bits */
#define SR_IE                   0x00000001
#define SR_EXL                  0x00000002
#define SR_ERL                  0x00000004
#define SR_KSU_MASK             0x00000018
#define SR_KSU_KERNEL           0x00000000
#define SR_KSU_SUPERVISOR       0x00000008
#define SR_KSU_USER             0x00000010
#define SR_IMSW0                0x00000100
#define SR_IMSW1                0x00000200
#define SR_IMEX0                0x00000400
#define SR_IMEX1                0x00000800
#define SR_IMEX2                0x00001000
#define SR_IMEX3                0x00002000
#define SR_IMEX4                0x00004000
#define SR_IMEX5                0x00008000
#define SR_DE                   0x00010000
#define SR_CE                   0x00020000
#define SR_CH                   0x00040000
#define SR_SR                   0x00100000
#define SR_TS                   0x00200000
#define SR_BEV                  0x00400000
#define SR_ITS                  0x01000000  /* VR4300 only, Application Note doesn't give purpose */
#define SR_RE                   0x02000000
#define SR_FR                   0x04000000
#define SR_RP                   0x08000000
#define SR_COP0                 0x10000000
#define SR_COP1                 0x20000000
#define SR_COP2                 0x40000000
#define SR_COP3                 0x80000000

/* exception types */
#define EXCEPTION_INTERRUPT     0
#define EXCEPTION_TLBMOD        1
#define EXCEPTION_TLBLOAD       2
#define EXCEPTION_TLBSTORE      3
#define EXCEPTION_ADDRLOAD      4
#define EXCEPTION_ADDRSTORE     5
#define EXCEPTION_BUSINST       6
#define EXCEPTION_BUSDATA       7
#define EXCEPTION_SYSCALL       8
#define EXCEPTION_BREAK         9
#define EXCEPTION_INVALIDOP     10
#define EXCEPTION_BADCOP        11
#define EXCEPTION_OVERFLOW      12
#define EXCEPTION_TRAP          13
#define EXCEPTION_TLBLOAD_FILL  16
#define EXCEPTION_TLBSTORE_FILL 17
#define EXCEPTION_COUNT         18



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define REG_LO          32
#define REG_HI          33

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define FRREG           ((op >> 21) & 31)
#define FTREG           ((op >> 16) & 31)
#define FSREG           ((op >> 11) & 31)
#define FDREG           ((op >> 6) & 31)

#define IS_SINGLE(o)    (((o) & (1 << 21)) == 0)
#define IS_DOUBLE(o)    (((o) & (1 << 21)) != 0)
#define IS_FLOAT(o)     (((o) & (1 << 23)) == 0)
#define IS_INTEGRAL(o)  (((o) & (1 << 23)) != 0)

#define SIMMVAL         ((INT16)op)
#define UIMMVAL         ((UINT16)op)
#define LIMMVAL         (op & 0x03ffffff)


#endif /* __MIPS3COM_H__ */
