// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_32XSDEFS_H
#define MAME_CPU_E132XS_32XSDEFS_H

#pragma once

/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/

#define PC_REGISTER          0
#define SR_REGISTER          1
#define SP_REGISTER         18
#define UB_REGISTER         19
#define BCR_REGISTER        20
#define TPR_REGISTER        21
#define TCR_REGISTER        22
#define TR_REGISTER         23
#define WCR_REGISTER        24
#define ISR_REGISTER        25
#define FCR_REGISTER        26
#define MCR_REGISTER        27

#define X_CODE(val)      ((val & 0x7000) >> 12)
#define E_BIT(val)       ((val & 0x8000) >> 15)
#define S_BIT_CONST(val) ((val & 0x4000) >> 14)
#define DD(val)          ((val & 0x3000) >> 12)

#define S_BIT                   ((OP & 0x100) >> 8)
#define D_BIT                   ((OP & 0x200) >> 9)
#define N_VALUE                 (((OP & 0x100) >> 4) | (OP & 0x0f))
#define HI_N_VALUE              (0x10 | (OP & 0x0f))
#define LO_N_VALUE              (OP & 0x0f)
#define N_OP_MASK               (m_op & 0x10f)
#define DRC_HI_N_VALUE          (0x10 | (op & 0x0f))
#define DRC_LO_N_VALUE          (op & 0x0f)
#define DRC_N_OP_MASK           (op & 0x10f)
#define DST_CODE                ((OP & 0xf0) >> 4)
#define SRC_CODE                (OP & 0x0f)
#define SIGN_BIT(val)           ((val & 0x80000000) >> 31)
#define SIGN_TO_N(val)          ((val & 0x80000000) >> 29)
#define SIGN64_TO_N(val)        ((val & 0x8000000000000000ULL) >> 61)

/* Extended DSP instructions */
#define EHMAC           0x02a
#define EHMACD          0x02e
#define EHCMULD         0x046
#define EHCMACD         0x04e
#define EHCSUMD         0x086
#define EHCFFTD         0x096
#define EMUL_N          0x100
#define EMUL            0x102
#define EMULU           0x104
#define EMULS           0x106
#define EMAC            0x10a
#define EMACD           0x10e
#define EMSUB           0x11a
#define EMSUBD          0x11e
#define EHCFFTSD        0x296

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
#define TRAPNO_INT4                 50
#define TRAPNO_INT3                 51
#define TRAPNO_INT2                 52
#define TRAPNO_INT1                 53
#define TRAPNO_IO3                  54
#define TRAPNO_TIMER                55
#define TRAPNO_RESERVED1            56
#define TRAPNO_TRACE_EXCEPTION      57
#define TRAPNO_PARITY_ERROR         58
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

/* Memory access */
/* read byte */
#define READ_B(addr)            m_program->read_byte((addr))
/* read half-word */
#define READ_HW(addr)           m_program->read_word((addr) & ~1)
/* read word */
#define READ_W(addr)            m_program->read_dword((addr) & ~3)

/* write byte */
#define WRITE_B(addr, data)     m_program->write_byte(addr, data)
/* write half-word */
#define WRITE_HW(addr, data)    m_program->write_word((addr) & ~1, data)
/* write word */
#define WRITE_W(addr, data)     m_program->write_dword((addr) & ~3, data)


/* I/O access */
/* read word */
#define IO_READ_W(addr)         m_io->read_dword(((addr) >> 11) & 0x7ffc)
/* write word */
#define IO_WRITE_W(addr, data)  m_io->write_dword(((addr) >> 11) & 0x7ffc, data)

// set C in adds/addsi/subs/sums
#define SETCARRYS 0
#define MISSIONCRAFT_FLAGS 1

/* Registers */

/* Internal registers */

#define OP              m_op
#define PC              m_core->global_regs[0] //Program Counter
#define SR              m_core->global_regs[1] //Status Register
#define FER             m_core->global_regs[2] //Floating-Point Exception Register
// 03 - 15  General Purpose Registers
// 16 - 17  Reserved
#define SP              m_core->global_regs[18] //Stack Pointer
#define UB              m_core->global_regs[19] //Upper Stack Bound
#define BCR             m_core->global_regs[20] //Bus Control Register
#define TPR             m_core->global_regs[21] //Timer Prescaler Register
#define TCR             m_core->global_regs[22] //Timer Compare Register
#define TR              compute_tr() //Timer Register
#define WCR             m_core->global_regs[24] //Watchdog Compare Register
#define ISR             m_core->global_regs[25] //Input Status Register
#define FCR             m_core->global_regs[26] //Function Control Register
#define MCR             m_core->global_regs[27] //Memory Control Register
// 28 - 31  Reserved

#define C_MASK                  0x00000001
#define Z_MASK                  0x00000002
#define N_MASK                  0x00000004
#define V_MASK                  0x00000008
#define M_MASK                  0x00000010
#define H_MASK                  0x00000020
#define I_MASK                  0x00000080
#define L_MASK                  0x00008000
#define T_MASK                  0x00010000
#define P_MASK                  0x00020000
#define S_MASK                  0x00040000
#define ILC_MASK                0x00180000

#define C_SHIFT                 0
#define Z_SHIFT                 1
#define N_SHIFT                 2
#define V_SHIFT                 3
#define S_SHIFT                 18
#define ILC_SHIFT               19

/* SR flags */
#define GET_C                   ( SR & C_MASK)          // bit 0 //CARRY
#define GET_Z                   ((SR & Z_MASK)>>1)      // bit 1 //ZERO
#define GET_N                   ((SR & N_MASK)>>2)      // bit 2 //NEGATIVE
#define GET_V                   ((SR & V_MASK)>>3)      // bit 3 //OVERFLOW
#define GET_M                   ((SR & M_MASK)>>4)      // bit 4 //CACHE-MODE
#define GET_H                   ((SR & H_MASK)>>5)      // bit 5 //HIGHGLOBAL
// bit 6 RESERVED (always 0)
#define GET_I                   ((SR & I_MASK)>>7)      // bit 7 //INTERRUPT-MODE
#define GET_FTE                 ((SR & 0x00001f00)>>8)  // bits 12 - 8  //Floating-Point Trap Enable
#define GET_FRM                 ((SR & 0x00006000)>>13) // bits 14 - 13 //Floating-Point Rounding Mode
#define GET_L                   ((SR & L_MASK)>>15)     // bit 15 //INTERRUPT-LOCK
#define GET_T                   ((SR & T_MASK)>>16)     // bit 16 //TRACE-MODE
#define GET_P                   ((SR & P_MASK)>>17)     // bit 17 //TRACE PENDING
#define GET_S                   ((SR & S_MASK)>>18)     // bit 18 //SUPERVISOR STATE
#define GET_ILC                 ((SR & 0x00180000)>>19) // bits 20 - 19 //INSTRUCTION-LENGTH
/* if FL is zero it is always interpreted as 16 */
#define GET_FL                  m_core->fl_lut[((SR >> 21) & 0xf)] // bits 24 - 21 //FRAME LENGTH
#define GET_FP                  ((SR & 0xfe000000)>>25) // bits 31 - 25 //FRAME POINTER

#define SET_C(val)              (SR = (SR & ~C_MASK) | (val))
#define SET_Z(val)              (SR = (SR & ~Z_MASK) | ((val) << 1))
#define SET_N(val)              (SR = (SR & ~N_MASK) | ((val) << 2))
#define SET_V(val)              (SR = (SR & ~V_MASK) | ((val) << 3))
#define SET_M(val)              (SR = (SR & ~M_MASK) | ((val) << 4))
#define SET_H(val)              (SR = (SR & ~H_MASK) | ((val) << 5))
#define SET_I(val)              (SR = (SR & ~I_MASK) | ((val) << 7))
#define SET_FTE(val)            (SR = (SR & ~0x00001f00) | ((val) << 8))
#define SET_FRM(val)            (SR = (SR & ~0x00006000) | ((val) << 13))
#define SET_L(val)              (SR = (SR & ~L_MASK) | ((val) << 15))
#define SET_T(val)              (SR = (SR & ~T_MASK) | ((val) << 16))
#define SET_P(val)              (SR = (SR & ~P_MASK) | ((val) << 17))
#define SET_S(val)              (SR = (SR & ~S_MASK) | ((val) << 18))
#define SET_ILC(val)            (SR = (SR & 0xffe7ffff) | (val))
#define SET_FL(val)             (SR = (SR & ~0x01e00000) | ((val) << 21))
#define SET_FP(val)             (SR = (SR & ~0xfe000000) | ((val) << 25))

#define SET_PC(val)             PC = ((val) & 0xfffffffe) //PC(0) = 0
#define SET_SP(val)             SP = ((val) & 0xfffffffc) //SP(0) = SP(1) = 0
#define SET_UB(val)             UB = ((val) & 0xfffffffc) //UB(0) = UB(1) = 0

#define SET_LOW_SR(val)         (SR = (SR & 0xffff0000) | ((val) & 0x0000ffff)) // when SR is addressed, only low 16 bits can be changed


/* FER flags */
#define GET_ACCRUED             (FER & 0x0000001f) //bits  4 - 0 //Floating-Point Accrued Exceptions
#define GET_ACTUAL              (FER & 0x00001f00) //bits 12 - 8 //Floating-Point Actual  Exceptions
//other bits are reversed, in particular 7 - 5 for the operating system.
//the user program can only change the above 2 flags

#endif // MAME_CPU_E132XS_32XSDEFS_H
