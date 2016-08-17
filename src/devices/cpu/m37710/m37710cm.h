// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud, hap
#pragma once

#ifndef __M37710CM_H__
#define __M37710CM_H__


/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include <limits.h>
#include "m37710.h"


/* ======================================================================== */
/* ================================= MAME ================================= */
/* ======================================================================== */

#undef M37710_CALL_DEBUGGER

#define M37710_CALL_DEBUGGER(x)         debugger_instruction_hook(this, x)
#define m37710_read_8(addr)             m_program->read_byte(addr)
#define m37710_write_8(addr,data)       m_program->write_byte(addr,data)
#define m37710_read_8_immediate(A)      m_direct->read_byte(A, BYTE_XOR_LE(0))
#define m37710_read_16(addr)            m_program->read_word_unaligned(addr)
#define m37710_write_16(addr,data)      m_program->write_word_unaligned(addr,data)
#define m37710_read_16_immediate(A)     m_direct->read_word(A)


/* ======================================================================== */
/* ================================ GENERAL =============================== */
/* ======================================================================== */

#undef int8

/* Allow for architectures that don't have 8-bit sizes */
#if UCHAR_MAX == 0xff
#define int8 char
#define MAKE_INT_8(A) (int8)((A)&0xff)
#else
#define int8   int
static inline int MAKE_INT_8(int A) {return (A & 0x80) ? A | ~0xff : A & 0xff;}
#endif /* UCHAR_MAX == 0xff */

#define MAKE_UINT_8(A) ((A)&0xff)
#define MAKE_UINT_16(A) ((A)&0xffff)
#define MAKE_UINT_24(A) ((A)&0xffffff)

/* Bits */
#define BIT_0       0x01
#define BIT_1       0x02
#define BIT_2       0x04
#define BIT_3       0x08
#define BIT_4       0x10
#define BIT_5       0x20
#define BIT_6       0x40
#define BIT_7       0x80

/* ======================================================================== */
/* ================================== CPU ================================= */
/* ======================================================================== */

#define REG_A           m_a     /* Accumulator */
#define REG_B           m_b     /* Accumulator hi byte */
#define REG_BA          m_ba        /* Secondary Accumulator */
#define REG_BB          m_bb        /* Secondary Accumulator hi byte */
#define REG_X           m_x     /* Index X Register */
#define REG_Y           m_y     /* Index Y Register */
#define REG_XH          m_xh        /* X high byte */
#define REG_YH          m_yh        /* Y high byte */
#define REG_S           m_s     /* Stack Pointer */
#define REG_PC          m_pc        /* Program Counter */
#define REG_PPC         m_ppc       /* Previous Program Counter */
#define REG_PB          m_pb        /* Program Bank */
#define REG_DB          m_db        /* Data Bank */
#define REG_D           m_d     /* Direct Register */
#define FLAG_M          m_flag_m    /* Memory/Accumulator Select Flag */
#define FLAG_X          m_flag_x    /* Index Select Flag */
#define FLAG_N          m_flag_n    /* Negative Flag */
#define FLAG_V          m_flag_v    /* Overflow Flag */
#define FLAG_D          m_flag_d    /* Decimal Mode Flag */
#define FLAG_I          m_flag_i    /* Interrupt Mask Flag */
#define FLAG_Z          m_flag_z    /* Zero Flag (inverted) */
#define FLAG_C          m_flag_c    /* Carry Flag */
#define LINE_IRQ        m_line_irq  /* Status of the IRQ line */
#define REG_IR          m_ir        /* Instruction Register */
#define REG_IM          m_im        /* Immediate load value */
#define REG_IM2         m_im2       /* Immediate load target */
#define REG_IM3         m_im3       /* Immediate load target */
#define REG_IM4         m_im4       /* Immediate load target */
#define INT_ACK         m_int_ack   /* Interrupt Acknowledge function pointer */
#define CLOCKS          m_ICount        /* Clock cycles remaining */
#define IRQ_DELAY       m_irq_delay /* Delay 1 instruction before checking IRQ */
#define CPU_STOPPED     m_stopped   /* Stopped status of the CPU */

#define FTABLE_GET_REG  m_get_reg
#define FTABLE_SET_REG  m_set_reg
#define FTABLE_SET_LINE m_set_line

#define SRC         m_source        /* Source Operand */
#define DST         m_destination   /* Destination Operand */

#define STOP_LEVEL_WAI  1
#define STOP_LEVEL_STOP 2

#define EXECUTION_MODE_M0X0 0
#define EXECUTION_MODE_M0X1 1
#define EXECUTION_MODE_M1X0 2
#define EXECUTION_MODE_M1X1 3


/* ======================================================================== */
/* ================================= CLOCK ================================ */
/* ======================================================================== */

#define CLK_OP          1
#define CLK_R8          1
#define CLK_R16         2
#define CLK_R24         3
#define CLK_W8          1
#define CLK_W16         2
#define CLK_W24         3
#define CLK_RMW8        3
#define CLK_RMW16       5

#define CLK_IMPLIED     1
#define CLK_IMPLIED     1
#define CLK_RELATIVE_8  1
#define CLK_RELATIVE_16 2
#define CLK_IMM         0
#define CLK_AI          4
#define CLK_AXI         4
#define CLK_A           2
#define CLK_AL          3
#define CLK_ALX         3
#define CLK_AX          2
#define CLK_AY          2
#define CLK_D           1
#define CLK_DI          3
#define CLK_DIY         3
#define CLK_DLI         4
#define CLK_DLIY        4
#define CLK_DX          2
#define CLK_DXI         4
#define CLK_DY          2
#define CLK_S           2
#define CLK_SIY         5

/* AX and AY addressing modes take 1 extra cycle when writing */
#define CLK_W_IMM       0
#define CLK_W_AI        4
#define CLK_W_AXI       4
#define CLK_W_A         2
#define CLK_W_AL        3
#define CLK_W_ALX       3
#define CLK_W_AX        3
#define CLK_W_AY        3
#define CLK_W_D         1
#define CLK_W_DI        3
#define CLK_W_DIY       3
#define CLK_W_DLI       4
#define CLK_W_DLIY      4
#define CLK_W_DX        2
#define CLK_W_DXI       4
#define CLK_W_DY        2
#define CLK_W_S         2
#define CLK_W_SIY       5

#define CLK(A)          CLOCKS -= (A)
#define USE_ALL_CLKS()  CLOCKS = 0


/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

/* Flag positions in Processor Status Register */
/* common */
#define FLAGPOS_N       BIT_7   /* Negative         */
#define FLAGPOS_V       BIT_6   /* Overflow         */
#define FLAGPOS_D       BIT_3   /* Decimal Mode     */
#define FLAGPOS_I       BIT_2   /* Interrupt Mask   */
#define FLAGPOS_Z       BIT_1   /* Zero             */
#define FLAGPOS_C       BIT_0   /* Carry            */
/* emulation */
#define FLAGPOS_R       BIT_5   /* Reserved         */
#define FLAGPOS_B       BIT_4   /* BRK Instruction  */
/* native */
#define FLAGPOS_M       BIT_5   /* Mem/Reg Select   */
#define FLAGPOS_X       BIT_4   /* Index Select     */

#define EFLAG_SET       1
#define EFLAG_CLEAR     0
#define MFLAG_SET       FLAGPOS_M
#define MFLAG_CLEAR     0
#define XFLAG_SET       FLAGPOS_X
#define XFLAG_CLEAR     0
#define NFLAG_SET       0x80
#define NFLAG_CLEAR     0
#define VFLAG_SET       0x80
#define VFLAG_CLEAR     0
#define DFLAG_SET       FLAGPOS_D
#define DFLAG_CLEAR     0
#define IFLAG_SET       FLAGPOS_I
#define IFLAG_CLEAR     0
#define BFLAG_SET       FLAGPOS_B
#define BFLAG_CLEAR     0
#define ZFLAG_SET       0
#define ZFLAG_CLEAR     1
#define CFLAG_SET       0x100
#define CFLAG_CLEAR     0

/* Codition code tests */
#define COND_CC()       (!(FLAG_C&0x100))   /* Carry Clear */
#define COND_CS()       (FLAG_C&0x100)      /* Carry Set */
#define COND_EQ()       (!FLAG_Z)           /* Equal */
#define COND_NE()       FLAG_Z              /* Not Equal */
#define COND_MI()       (FLAG_N&0x80)       /* Minus */
#define COND_PL()       (!(FLAG_N&0x80))    /* Plus */
#define COND_VC()       (!(FLAG_V&0x80))    /* Overflow Clear */
#define COND_VS()       (FLAG_V&0x80)       /* Overflow Set */

/* Set Overflow flag in math operations */
#define VFLAG_ADD_8(S, D, R)    ((S^R) & (D^R))
#define VFLAG_ADD_16(S, D, R)   (((S^R) & (D^R))>>8)
#define VFLAG_SUB_8(S, D, R)    ((S^D) & (R^D))
#define VFLAG_SUB_16(S, D, R)   (((S^D) & (R^D))>>8)

#define CFLAG_8(A)      (A)
#define CFLAG_16(A)     ((A)>>8)
#define NFLAG_8(A)      (A)
#define NFLAG_16(A)     ((A)>>8)

#define CFLAG_AS_1()    ((FLAG_C>>8)&1)

/* ======================================================================== */
/* ========================== EFFECTIVE ADDRESSES ========================= */
/* ======================================================================== */

/* Effective-address based memory access macros */
#define read_8_NORM(A)      m37710i_read_8_normal(A)
#define read_8_IMM(A)       m37710i_read_8_immediate(A)
#define read_8_D(A)     m37710i_read_8_direct(A)
#define read_8_A(A)     m37710i_read_8_normal(A)
#define read_8_AL(A)        m37710i_read_8_normal(A)
#define read_8_DX(A)        m37710i_read_8_direct(A)
#define read_8_DY(A)        m37710i_read_8_direct(A)
#define read_8_AX(A)        m37710i_read_8_normal(A)
#define read_8_ALX(A)       m37710i_read_8_normal(A)
#define read_8_AY(A)        m37710i_read_8_normal(A)
#define read_8_DI(A)        m37710i_read_8_normal(A)
#define read_8_DLI(A)       m37710i_read_8_normal(A)
#define read_8_AI(A)        m37710i_read_8_normal(A)
#define read_8_ALI(A)       m37710i_read_8_normal(A)
#define read_8_DXI(A)       m37710i_read_8_normal(A)
#define read_8_DIY(A)       m37710i_read_8_normal(A)
#define read_8_DLIY(A)      m37710i_read_8_normal(A)
#define read_8_AXI(A)       m37710i_read_8_normal(A)
#define read_8_S(A)     m37710i_read_8_normal(A)
#define read_8_SIY(A)       m37710i_read_8_normal(A)

#define read_16_NORM(A)     m37710i_read_16_normal(A)
#define read_16_IMM(A)      m37710i_read_16_immediate(A)
#define read_16_D(A)        m37710i_read_16_direct(A)
#define read_16_A(A)        m37710i_read_16_normal(A)
#define read_16_AL(A)       m37710i_read_16_normal(A)
#define read_16_DX(A)       m37710i_read_16_direct(A)
#define read_16_DY(A)       m37710i_read_16_direct(A)
#define read_16_AX(A)       m37710i_read_16_normal(A)
#define read_16_ALX(A)      m37710i_read_16_normal(A)
#define read_16_AY(A)       m37710i_read_16_normal(A)
#define read_16_DI(A)       m37710i_read_16_normal(A)
#define read_16_DLI(A)      m37710i_read_16_normal(A)
#define read_16_AI(A)       m37710i_read_16_normal(A)
#define read_16_ALI(A)      m37710i_read_16_normal(A)
#define read_16_DXI(A)      m37710i_read_16_normal(A)
#define read_16_DIY(A)      m37710i_read_16_normal(A)
#define read_16_DLIY(A)     m37710i_read_16_normal(A)
#define read_16_AXI(A)      m37710i_read_16_normal(A)
#define read_16_S(A)        m37710i_read_16_normal(A)
#define read_16_SIY(A)      m37710i_read_16_normal(A)

#define read_24_NORM(A)     m37710i_read_24_normal(A)
#define read_24_IMM(A)      m37710i_read_24_immediate(A)
#define read_24_D(A)        m37710i_read_24_direct(A)
#define read_24_A(A)        m37710i_read_24_normal(A)
#define read_24_AL(A)       m37710i_read_24_normal(A)
#define read_24_DX(A)       m37710i_read_24_direct(A)
#define read_24_DY(A)       m37710i_read_24_direct(A)
#define read_24_AX(A)       m37710i_read_24_normal(A)
#define read_24_ALX(A)      m37710i_read_24_normal(A)
#define read_24_AY(A)       m37710i_read_24_normal(A)
#define read_24_DI(A)       m37710i_read_24_normal(A)
#define read_24_DLI(A)      m37710i_read_24_normal(A)
#define read_24_AI(A)       m37710i_read_24_normal(A)
#define read_24_ALI(A)      m37710i_read_24_normal(A)
#define read_24_DXI(A)      m37710i_read_24_normal(A)
#define read_24_DIY(A)      m37710i_read_24_normal(A)
#define read_24_DLIY(A)     m37710i_read_24_normal(A)
#define read_24_AXI(A)      m37710i_read_24_normal(A)
#define read_24_S(A)        m37710i_read_24_normal(A)
#define read_24_SIY(A)      m37710i_read_24_normal(A)

#define write_8_NORM(A, V)  m37710i_write_8_normal(A, V)
#define write_8_D(A, V)     m37710i_write_8_direct(A, V)
#define write_8_A(A, V)     m37710i_write_8_normal(A, V)
#define write_8_AL(A, V)    m37710i_write_8_normal(A, V)
#define write_8_DX(A, V)    m37710i_write_8_direct(A, V)
#define write_8_DY(A, V)    m37710i_write_8_direct(A, V)
#define write_8_AX(A, V)    m37710i_write_8_normal(A, V)
#define write_8_ALX(A, V)   m37710i_write_8_normal(A, V)
#define write_8_AY(A, V)    m37710i_write_8_normal(A, V)
#define write_8_DI(A, V)    m37710i_write_8_normal(A, V)
#define write_8_DLI(A, V)   m37710i_write_8_normal(A, V)
#define write_8_AI(A, V)    m37710i_write_8_normal(A, V)
#define write_8_ALI(A, V)   m37710i_write_8_normal(A, V)
#define write_8_DXI(A, V)   m37710i_write_8_normal(A, V)
#define write_8_DIY(A, V)   m37710i_write_8_normal(A, V)
#define write_8_DLIY(A, V)  m37710i_write_8_normal(A, V)
#define write_8_AXI(A, V)   m37710i_write_8_normal(A, V)
#define write_8_S(A, V)     m37710i_write_8_normal(A, V)
#define write_8_SIY(A, V)   m37710i_write_8_normal(A, V)

#define write_16_NORM(A, V) m37710i_write_16_normal(A, V)
#define write_16_D(A, V)    m37710i_write_16_direct(A, V)
#define write_16_A(A, V)    m37710i_write_16_normal(A, V)
#define write_16_AL(A, V)   m37710i_write_16_normal(A, V)
#define write_16_DX(A, V)   m37710i_write_16_direct(A, V)
#define write_16_DY(A, V)   m37710i_write_16_direct(A, V)
#define write_16_AX(A, V)   m37710i_write_16_normal(A, V)
#define write_16_ALX(A, V)  m37710i_write_16_normal(A, V)
#define write_16_AY(A, V)   m37710i_write_16_normal(A, V)
#define write_16_DI(A, V)   m37710i_write_16_normal(A, V)
#define write_16_DLI(A, V)  m37710i_write_16_normal(A, V)
#define write_16_AI(A, V)   m37710i_write_16_normal(A, V)
#define write_16_ALI(A, V)  m37710i_write_16_normal(A, V)
#define write_16_DXI(A, V)  m37710i_write_16_normal(A, V)
#define write_16_DIY(A, V)  m37710i_write_16_normal(A, V)
#define write_16_DLIY(A, V) m37710i_write_16_normal(A, V)
#define write_16_AXI(A, V)  m37710i_write_16_normal(A, V)
#define write_16_S(A, V)    m37710i_write_16_normal(A, V)
#define write_16_SIY(A, V)  m37710i_write_16_normal(A, V)


#define OPER_8_IMM()        read_8_IMM(EA_IMM8())
#define OPER_8_D()      read_8_D(EA_D())
#define OPER_8_A()      read_8_A(EA_A())
#define OPER_8_AL()     read_8_AL(EA_AL())
#define OPER_8_DX()     read_8_DX(EA_DX())
#define OPER_8_DY()     read_8_DY(EA_DY())
#define OPER_8_AX()     read_8_AX(EA_AX())
#define OPER_8_ALX()        read_8_ALX(EA_ALX())
#define OPER_8_AY()     read_8_AY(EA_AY())
#define OPER_8_DI()     read_8_DI(EA_DI())
#define OPER_8_DLI()        read_8_DLI(EA_DLI())
#define OPER_8_AI()     read_8_AI(EA_AI())
#define OPER_8_ALI()        read_8_ALI(EA_ALI())
#define OPER_8_DXI()        read_8_DXI(EA_DXI())
#define OPER_8_DIY()        read_8_DIY(EA_DIY())
#define OPER_8_DLIY()   read_8_DLIY(EA_DLIY())
#define OPER_8_AXI()        read_8_AXI(EA_AXI())
#define OPER_8_S()      read_8_S(EA_S())
#define OPER_8_SIY()        read_8_SIY(EA_SIY())

#define OPER_16_IMM()   read_16_IMM(EA_IMM16())
#define OPER_16_D()     read_16_D(EA_D())
#define OPER_16_A()     read_16_A(EA_A())
#define OPER_16_AL()        read_16_AL(EA_AL())
#define OPER_16_DX()        read_16_DX(EA_DX())
#define OPER_16_DY()        read_16_DY(EA_DY())
#define OPER_16_AX()        read_16_AX(EA_AX())
#define OPER_16_ALX()   read_16_ALX(EA_ALX())
#define OPER_16_AY()        read_16_AY(EA_AY())
#define OPER_16_DI()        read_16_DI(EA_DI())
#define OPER_16_DLI()   read_16_DLI(EA_DLI())
#define OPER_16_AI()        read_16_AI(EA_AI())
#define OPER_16_ALI()   read_16_ALI(EA_ALI())
#define OPER_16_DXI()   read_16_DXI(EA_DXI())
#define OPER_16_DIY()   read_16_DIY(EA_DIY())
#define OPER_16_DLIY()  read_16_DLIY(EA_DLIY())
#define OPER_16_AXI()   read_16_AXI(EA_AXI())
#define OPER_16_S()     read_16_S(EA_S())
#define OPER_16_SIY()   read_16_SIY(EA_SIY())

#define OPER_24_IMM()   read_24_IMM(EA_IMM24())
#define OPER_24_D()     read_24_D(EA_D())
#define OPER_24_A()     read_24_A(EA_A())
#define OPER_24_AL()        read_24_AL(EA_AL())
#define OPER_24_DX()        read_24_DX(EA_DX())
#define OPER_24_DY()        read_24_DY(EA_DY())
#define OPER_24_AX()        read_24_AX(EA_AX())
#define OPER_24_ALX()   read_24_ALX(EA_ALX())
#define OPER_24_AY()        read_24_AY(EA_AY())
#define OPER_24_DI()        read_24_DI(EA_DI())
#define OPER_24_DLI()   read_24_DLI(EA_DLI())
#define OPER_24_AI()        read_24_AI(EA_AI())
#define OPER_24_ALI()   read_24_ALI(EA_ALI())
#define OPER_24_DXI()   read_24_DXI(EA_DXI())
#define OPER_24_DIY()   read_24_DIY(EA_DIY())
#define OPER_24_DLIY()  read_24_DLIY(EA_DLIY())
#define OPER_24_AXI()   read_24_AXI(EA_AXI())
#define OPER_24_S()     read_24_S(EA_S())
#define OPER_24_SIY()   read_24_SIY(EA_SIY())

/* ======================================================================== */
/* ================================== CPU ================================= */
/* ======================================================================== */
#endif /* __M37710CM_H__ */
