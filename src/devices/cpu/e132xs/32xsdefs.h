// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_XS32DEFS_H
#define MAME_CPU_E132XS_XS32DEFS_H

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


#define READ_OP(addr)          m_direct->read_word((addr), m_opcodexor)

// set C in adds/addsi/subs/sums
#define SETCARRYS 0
#define MISSIONCRAFT_FLAGS 1

/* Registers */

/* Internal registers */

#define SREG  decode.src_value
#define SREGF decode.next_src_value
#define DREG  decode.dst_value
#define DREGF decode.next_dst_value
#define EXTRA_U decode.extra.u
#define EXTRA_S decode.extra.s

#define SET_SREG( _data_ )  (decode.src_is_local ? set_local_register(decode.src, (uint32_t)_data_) : set_global_register(decode.src, (uint32_t)_data_))
#define SET_SREGF( _data_ ) (decode.src_is_local ? set_local_register(decode.src + 1, (uint32_t)_data_) : set_global_register(decode.src + 1, (uint32_t)_data_))
#define SET_DREG( _data_ )  (decode.dst_is_local ? set_local_register(decode.dst, (uint32_t)_data_) : set_global_register(decode.dst, (uint32_t)_data_))
#define SET_DREGF( _data_ ) (decode.dst_is_local ? set_local_register(decode.dst + 1, (uint32_t)_data_) : set_global_register(decode.dst + 1, (uint32_t)_data_))

#define SRC_IS_PC      (!decode.src_is_local && decode.src == PC_REGISTER)
#define DST_IS_PC      (!decode.dst_is_local && decode.dst == PC_REGISTER)
#define SRC_IS_SR      (!decode.src_is_local && decode.src == SR_REGISTER)
#define DST_IS_SR      (!decode.dst_is_local && decode.dst == SR_REGISTER)
#define SAME_SRC_DST   decode.same_src_dst
#define SAME_SRC_DSTF  decode.same_src_dstf
#define SAME_SRCF_DST  decode.same_srcf_dst


#define OP              m_op
#define PC              m_global_regs[0] //Program Counter
#define SR              m_global_regs[1] //Status Register
#define FER             m_global_regs[2] //Floating-Point Exception Register
// 03 - 15  General Purpose Registers
// 16 - 17  Reserved
#define SP              m_global_regs[18] //Stack Pointer
#define UB              m_global_regs[19] //Upper Stack Bound
#define BCR             m_global_regs[20] //Bus Control Register
#define TPR             m_global_regs[21] //Timer Prescaler Register
#define TCR             m_global_regs[22] //Timer Compare Register
#define TR              compute_tr() //Timer Register
#define WCR             m_global_regs[24] //Watchdog Compare Register
#define ISR             m_global_regs[25] //Input Status Register
#define FCR             m_global_regs[26] //Function Control Register
#define MCR             m_global_regs[27] //Memory Control Register
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
#define GET_FL                  m_fl_lut[((SR >> 21) & 0xf)] // bits 24 - 21 //FRAME LENGTH
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


#define CHECK_C(x)              (SR = (SR & ~0x00000001) | (uint32_t)((x & 0x100000000L) >> 32))
#define CHECK_VADD(x,y,z)       (SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) >> 29))
#define CHECK_VADD3(x,y,w,z)    (SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & ((w) ^ (z)) & 0x80000000) >> 29))
#define CHECK_VSUB(x,y,z)       (SR = (SR & ~0x00000008) | (((z ^ y) & (y ^ x) & 0x80000000) >> 29))


/* FER flags */
#define GET_ACCRUED             (FER & 0x0000001f) //bits  4 - 0 //Floating-Point Accrued Exceptions
#define GET_ACTUAL              (FER & 0x00001f00) //bits 12 - 8 //Floating-Point Actual  Exceptions
//other bits are reversed, in particular 7 - 5 for the operating system.
//the user program can only changes the above 2 flags


#endif // MAME_CPU_E132XS_XS32DEFS_H

