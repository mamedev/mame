// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#pragma once

#ifndef __G65816CM_H__
#define __G65816CM_H__


#define g65816i_branching(A)
#define g65816i_jumping(A)


#undef G65816_CALL_DEBUGGER
#define G65816_CALL_DEBUGGER(x) debugger_instruction_hook(this, x)

#define g65816_read_8(addr)             m_program->read_byte(addr)
#define g65816_write_8(addr,data)       m_program->write_byte(addr,data)
#define g65816_read_8_immediate(A)      m_program->read_byte(A)
#define g65816_jumping(A)
#define g65816_branching(A)


/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include <limits.h>


/* ======================================================================== */
/* ================================ GENERAL =============================== */
/* ======================================================================== */

/* This should be set to the default size of your processor (min 16 bit) */
#undef uint
#define uint unsigned int

#undef uint8
#define uint8 unsigned char

#undef int8

/* Allow for architectures that don't have 8-bit sizes */
#if UCHAR_MAX == 0xff
#define int8 signed char
#define MAKE_INT_8(A) (int8)((A)&0xff)
#else
#define int8   int
INLINE int MAKE_INT_8(int A) {return (A & 0x80) ? A | ~0xff : A & 0xff;}
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


#define REGISTER_A      m_a     /* Accumulator */
#define REGISTER_B      m_b     /* Accumulator hi byte */
#define REGISTER_X      m_x     /* Index X Register */
#define REGISTER_Y      m_y     /* Index Y Register */
#define REGISTER_S      m_s     /* Stack Pointer */
#define REGISTER_PC     m_pc        /* Program Counter */
#define REGISTER_PPC        m_ppc       /* Previous Program Counter */
#define REGISTER_PB     m_pb        /* Program Bank */
#define REGISTER_DB     m_db        /* Data Bank */
#define REGISTER_D      m_d     /* Direct Register */
#define FLAG_E          m_flag_e    /* Emulation Mode Flag */
#define FLAG_M          m_flag_m    /* Memory/Accumulator Select Flag */
#define FLAG_X          m_flag_x    /* Index Select Flag */
#define FLAG_N          m_flag_n    /* Negative Flag */
#define FLAG_V          m_flag_v    /* Overflow Flag */
#define FLAG_D          m_flag_d    /* Decimal Mode Flag */
#define FLAG_I          m_flag_i    /* Interrupt Mask Flag */
#define FLAG_Z          m_flag_z    /* Zero Flag (inverted) */
#define FLAG_C          m_flag_c    /* Carry Flag */
#define LINE_IRQ        m_line_irq  /* Status of the IRQ line */
#define LINE_NMI        m_line_nmi  /* Status of the NMI line */
#define REGISTER_IR     m_ir        /* Instruction Register */
#define INT_ACK         m_int_ack   /* Interrupt Acknowledge function pointer */
#define READ_VECTOR     m_read_vector   /* Vector reading override */
#define CLOCKS          m_ICount        /* Clock cycles remaining */
#define IRQ_DELAY       m_irq_delay /* Delay 1 instruction before checking IRQ */
#define CPU_STOPPED         m_stopped   /* Stopped status of the CPU */

#define FTABLE_OPCODES  m_opcodes
#define FTABLE_GET_REG  m_get_reg
#define FTABLE_SET_REG  m_set_reg
#define FTABLE_SET_LINE m_set_line
#define FTABLE_EXECUTE  m_execute

#define SRC             m_source        /* Source Operand */
#define DST             m_destination   /* Destination Operand */

#define STOP_LEVEL_WAI  1
#define STOP_LEVEL_STOP 2

#define EXECUTION_MODE_M0X0 0
#define EXECUTION_MODE_M0X1 1
#define EXECUTION_MODE_M1X0 2
#define EXECUTION_MODE_M1X1 3
#define EXECUTION_MODE_E    4

#define VECTOR_RESET    0xfffc      /* Reset */
#define VECTOR_IRQ_E    0xfffe      /* Interrupt Request */
#define VECTOR_NMI_E    0xfffa      /* Non-Maskable Interrupt */
#define VECTOR_ABORT_E  0xfff8      /* ABORT asserted */
#define VECTOR_BRK_E    0xfffe      /* Break Instruction */
#define VECTOR_COP_E    0xfff4      /* Coprocessor instruction */

#define VECTOR_IRQ_N    0xffee      /* Interrupt Request */
#define VECTOR_NMI_N    0xffea      /* Non-Maskable Interrupt */
#define VECTOR_ABORT_N  0xffe8      /* ABORT asserted */
#define VECTOR_BRK_N    0xffe6      /* Break Instruction */
#define VECTOR_COP_N    0xffe4      /* Coprocessor instruction */


/* ======================================================================== */
/* ================================= CLOCK ================================ */
/* ======================================================================== */

#define CLK_OP          1
#define CLK_R8          m_rw8_cycles
#define CLK_R16         m_rw16_cycles
#define CLK_R24         m_rw24_cycles
#define CLK_W8          m_rw8_cycles
#define CLK_W16         m_rw16_cycles
#define CLK_W24         m_rw24_cycles
#define CLK_RMW8        m_rw8_cycles+m_rw8_cycles + 1
#define CLK_RMW16       m_rw16_cycles+m_rw16_cycles + 1

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

#define CLK(A)          CLOCKS -= (m_cpu_type == CPU_TYPE_G65816 ? A : A*6)
#define CLK_BUS(A)      CLOCKS -= A
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
/* ================================== CPU ================================= */
/* ======================================================================== */
#endif /* __G65816CM_H__ */
