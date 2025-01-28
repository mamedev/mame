// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 4.50
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright Karl Stenerud
 *
 */

#ifndef MAME_CPU_M68000_M68KCPU_H
#define MAME_CPU_M68000_M68KCPU_H

#pragma once

#if defined(__sun__) && defined(__svr4__)
#undef REG_SP
#undef REG_PC
#else
#include <climits>
#endif

/* ======================================================================== */
/* ==================== ARCHITECTURE-DEPENDANT DEFINES ==================== */
/* ======================================================================== */

/* Check for > 32bit sizes */
static constexpr s8 MAKE_INT_8(u32 A) { return (s8)(A); }
static constexpr s16 MAKE_INT_16(u32 A) { return (s16)(A); }
static constexpr s32 MAKE_INT_32(u32 A) { return (s32)(A); }


/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */
/* ======================================================================== */

/* Exception Vectors handled by emulation */
static constexpr int EXCEPTION_RESET                    = 0;
static constexpr int EXCEPTION_BUS_ERROR                = 2; /* This one is not emulated! */
static constexpr int EXCEPTION_ADDRESS_ERROR            = 3; /* This one is partially emulated (doesn't stack a proper frame yet) */
static constexpr int EXCEPTION_ILLEGAL_INSTRUCTION      = 4;
static constexpr int EXCEPTION_ZERO_DIVIDE              = 5;
static constexpr int EXCEPTION_CHK                      = 6;
static constexpr int EXCEPTION_TRAPV                    = 7;
static constexpr int EXCEPTION_PRIVILEGE_VIOLATION      = 8;
static constexpr int EXCEPTION_TRACE                    = 9;
static constexpr int EXCEPTION_1010                    = 10;
static constexpr int EXCEPTION_1111                    = 11;
static constexpr int EXCEPTION_FORMAT_ERROR            = 14;
static constexpr int EXCEPTION_UNINITIALIZED_INTERRUPT = 15;
static constexpr int EXCEPTION_SPURIOUS_INTERRUPT      = 24;
static constexpr int EXCEPTION_INTERRUPT_AUTOVECTOR    = 24;
static constexpr int EXCEPTION_TRAP_BASE               = 32;
static constexpr int EXCEPTION_MMU_CONFIGURATION       = 56; // only on 020/030

/* Function codes set by CPU during data/address bus activity */
static constexpr int FUNCTION_CODE_USER_DATA          = 1;
static constexpr int FUNCTION_CODE_USER_PROGRAM       = 2;
static constexpr int FUNCTION_CODE_SUPERVISOR_DATA    = 5;
static constexpr int FUNCTION_CODE_SUPERVISOR_PROGRAM = 6;
static constexpr int FUNCTION_CODE_CPU_SPACE          = 7;

/* CPU types for deciding what to emulate */
static constexpr int CPU_TYPE_000    = (0x00000001);
static constexpr int CPU_TYPE_008    = (0x00000002);
static constexpr int CPU_TYPE_010    = (0x00000004);
static constexpr int CPU_TYPE_EC020  = (0x00000008);
static constexpr int CPU_TYPE_020    = (0x00000010);
static constexpr int CPU_TYPE_EC030  = (0x00000020);
static constexpr int CPU_TYPE_030    = (0x00000040);
static constexpr int CPU_TYPE_EC040  = (0x00000080);
static constexpr int CPU_TYPE_LC040  = (0x00000100);
static constexpr int CPU_TYPE_040    = (0x00000200);
static constexpr int CPU_TYPE_SCC070 = (0x00000400);
static constexpr int CPU_TYPE_FSCPU32  = (0x00000800);
static constexpr int CPU_TYPE_COLDFIRE = (0x00001000);

/* Different ways to stop the CPU */
static constexpr int STOP_LEVEL_STOP = 1;
static constexpr int STOP_LEVEL_HALT = 2;

/* Used for 68000 address error processing */
static constexpr int INSTRUCTION_YES = 0;
static constexpr int INSTRUCTION_NO  = 0x08;
static constexpr int MODE_READ       = 0x10;
static constexpr int MODE_WRITE      = 0;

static constexpr int RUN_MODE_NORMAL              = 0;
static constexpr int RUN_MODE_BERR_AERR_RESET_WSF = 1; // writing the stack frame
static constexpr int RUN_MODE_BERR_AERR_RESET     = 2; // stack frame done



static constexpr int M68K_CACR_IBE = 0x10; // Instruction Burst Enable
static constexpr int M68K_CACR_CI  = 0x08; // Clear Instruction Cache
static constexpr int M68K_CACR_CEI = 0x04; // Clear Entry in Instruction Cache
static constexpr int M68K_CACR_FI  = 0x02; // Freeze Instruction Cache
static constexpr int M68K_CACR_EI  = 0x01; // Enable Instruction Cache

/* ======================================================================== */
/* ================================ MACROS ================================ */
/* ======================================================================== */


/* ---------------------------- General Macros ---------------------------- */

/* Bit Isolation Macros */
static constexpr u32 BIT_0(u32 A)  { return ((A) & 0x00000001); }
static constexpr u32 BIT_1(u32 A)  { return ((A) & 0x00000002); }
static constexpr u32 BIT_2(u32 A)  { return ((A) & 0x00000004); }
static constexpr u32 BIT_3(u32 A)  { return ((A) & 0x00000008); }
static constexpr u32 BIT_4(u32 A)  { return ((A) & 0x00000010); }
static constexpr u32 BIT_5(u32 A)  { return ((A) & 0x00000020); }
static constexpr u32 BIT_6(u32 A)  { return ((A) & 0x00000040); }
static constexpr u32 BIT_7(u32 A)  { return ((A) & 0x00000080); }
static constexpr u32 BIT_8(u32 A)  { return ((A) & 0x00000100); }
static constexpr u32 BIT_9(u32 A)  { return ((A) & 0x00000200); }
static constexpr u32 BIT_A(u32 A)  { return ((A) & 0x00000400); }
static constexpr u32 BIT_B(u32 A)  { return ((A) & 0x00000800); }
static constexpr u32 BIT_C(u32 A)  { return ((A) & 0x00001000); }
static constexpr u32 BIT_D(u32 A)  { return ((A) & 0x00002000); }
static constexpr u32 BIT_E(u32 A)  { return ((A) & 0x00004000); }
static constexpr u32 BIT_F(u32 A)  { return ((A) & 0x00008000); }
static constexpr u32 BIT_10(u32 A) { return ((A) & 0x00010000); }
static constexpr u32 BIT_11(u32 A) { return ((A) & 0x00020000); }
static constexpr u32 BIT_12(u32 A) { return ((A) & 0x00040000); }
static constexpr u32 BIT_13(u32 A) { return ((A) & 0x00080000); }
static constexpr u32 BIT_14(u32 A) { return ((A) & 0x00100000); }
static constexpr u32 BIT_15(u32 A) { return ((A) & 0x00200000); }
static constexpr u32 BIT_16(u32 A) { return ((A) & 0x00400000); }
static constexpr u32 BIT_17(u32 A) { return ((A) & 0x00800000); }
static constexpr u32 BIT_18(u32 A) { return ((A) & 0x01000000); }
static constexpr u32 BIT_19(u32 A) { return ((A) & 0x02000000); }
static constexpr u32 BIT_1A(u32 A) { return ((A) & 0x04000000); }
static constexpr u32 BIT_1B(u32 A) { return ((A) & 0x08000000); }
static constexpr u32 BIT_1C(u32 A) { return ((A) & 0x10000000); }
static constexpr u32 BIT_1D(u32 A) { return ((A) & 0x20000000); }
static constexpr u32 BIT_1E(u32 A) { return ((A) & 0x40000000); }
static constexpr u32 BIT_1F(u32 A) { return ((A) & 0x80000000); }

/* Get the most significant bit for specific sizes */
static constexpr u32 GET_MSB_8(u32 A)  { return ((A) & 0x80); }
static constexpr u32 GET_MSB_9(u32 A)  { return ((A) & 0x100); }
static constexpr u32 GET_MSB_16(u32 A) { return ((A) & 0x8000); }
static constexpr u32 GET_MSB_17(u32 A) { return ((A) & 0x10000); }
static constexpr u32 GET_MSB_32(u32 A) { return ((A) & 0x80000000); }
static constexpr u64 GET_MSB_33(u64 A) { return ((A) & 0x100000000U); }

/* Isolate nibbles */
static constexpr u32 LOW_NIBBLE(u32 A)  { return ((A) & 0x0f); }
static constexpr u32 HIGH_NIBBLE(u32 A) { return ((A) & 0xf0); }

/* These are used to isolate 8, 16, and 32 bit sizes */
static constexpr u32 MASK_OUT_ABOVE_2(u32 A)  { return ((A) & 3); }
static constexpr u32 MASK_OUT_ABOVE_8(u32 A)  { return ((A) & 0xff); }
static constexpr u32 MASK_OUT_ABOVE_16(u32 A) { return ((A) & 0xffff); }
static constexpr u32 MASK_OUT_BELOW_2(u32 A)  { return ((A) & ~3); }
static constexpr u32 MASK_OUT_BELOW_8(u32 A)  { return ((A) & ~0xff); }
static constexpr u32 MASK_OUT_BELOW_16(u32 A) { return ((A) & ~0xffff); }

/* No need to mask if we are 32 bit */
static constexpr u32 MASK_OUT_ABOVE_32(u32 A) { return ((A) & u64(0xffffffffU)); }
static constexpr u64 MASK_OUT_BELOW_32(u64 A) { return ((A) & ~u64(0xffffffffU)); }

/* Shift & Rotate Macros. */
static constexpr u32 LSL(u32 A, u32 C) { return ((A) << (C)); }
static constexpr u32 LSR(u32 A, u32 C) { return ((A) >> (C)); }

/* We have to do this because the morons at ANSI decided that shifts
* by >= data size are undefined.
*/
static constexpr u32 LSR_32(u32 A, u32 C) { return ((C) < 32 ? (A) >> (C) : 0); }
static constexpr u32 LSL_32(u32 A, u32 C) { return ((C) < 32 ? (A) << (C) : 0); }

static constexpr u64 LSL_32_64(u64 A, u32 C) { return ((A) << (C)); }
static constexpr u64 LSR_32_64(u64 A, u32 C) { return ((A) >> (C)); }
static constexpr u64 ROL_33_64(u64 A, u32 C) { return (LSL_32_64(A, C) | LSR_32_64(A, 33 - (C))); }
static constexpr u64 ROR_33_64(u64 A, u32 C) { return (LSR_32_64(A, C) | LSL_32_64(A, 33 - (C))); }

static constexpr u32 ROL_8(u32 A, u32 C)     { return MASK_OUT_ABOVE_8(LSL(A, C) | LSR(A, 8-(C))); }
static constexpr u32 ROL_9(u32 A, u32 C)     { return                 (LSL(A, C) | LSR(A, 9-(C))); }
static constexpr u32 ROL_16(u32 A, u32 C)    { return MASK_OUT_ABOVE_16(LSL(A, C) | LSR(A, 16-(C))); }
static constexpr u32 ROL_17(u32 A, u32 C)    { return                 (LSL(A, C) | LSR(A, 17-(C))); }
static constexpr u32 ROL_32(u32 A, u32 C)    { return MASK_OUT_ABOVE_32(LSL_32(A, C) | LSR_32(A, 32-(C))); }

static constexpr u32 ROR_8(u32 A, u32 C)     { return MASK_OUT_ABOVE_8(LSR(A, C) | LSL(A, 8-(C))); }
static constexpr u32 ROR_9(u32 A, u32 C)     { return                  (LSR(A, C) | LSL(A, 9-(C))); }
static constexpr u32 ROR_16(u32 A, u32 C)    { return MASK_OUT_ABOVE_16(LSR(A, C) | LSL(A, 16-(C))); }
static constexpr u32 ROR_17(u32 A, u32 C)    { return                  (LSR(A, C) | LSL(A, 17-(C))); }
static constexpr u32 ROR_32(u32 A, u32 C)    { return MASK_OUT_ABOVE_32(LSR_32(A, C) | LSL_32(A, 32-(C))); }



/* ------------------------------ CPU Access ------------------------------ */

/* Access the CPU registers */
inline u32 (&REG_DA())[16]    { return m_dar; } /* easy access to data and address regs */
inline u32 (&REG_D())[16]     { return m_dar; }
inline u32 *REG_A()         { return (m_dar+8); }
inline u32 (&REG_SP_BASE())[7]{ return m_sp; }
inline u32 &REG_USP()         { return m_sp[0]; }
inline u32 &REG_ISP()         { return m_sp[4]; }
inline u32 &REG_MSP()         { return m_sp[6]; }
inline u32 &REG_SP()          { return m_dar[15]; }


/* ----------------------------- Configuration ---------------------------- */

/* These defines are dependant on the configuration defines in m68kconf.h */

/* Disable certain comparisons if we're not using all CPU types */
inline u32 CPU_TYPE_IS_COLDFIRE() const    { return ((m_cpu_type) & (CPU_TYPE_COLDFIRE)); }

inline u32 CPU_TYPE_IS_040_PLUS() const    { return ((m_cpu_type) & (CPU_TYPE_040 | CPU_TYPE_EC040)); }

inline u32 CPU_TYPE_IS_030_PLUS() const    { return ((m_cpu_type) & (CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040)); }

inline u32 CPU_TYPE_IS_020_PLUS() const    { return ((m_cpu_type) & (CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE)); }

inline u32 CPU_TYPE_IS_020_VARIANT() const { return ((m_cpu_type) & (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_FSCPU32)); }

inline u32 CPU_TYPE_IS_EC020_PLUS() const  { return ((m_cpu_type) & (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE)); }
inline u32 CPU_TYPE_IS_EC020_LESS() const  { return ((m_cpu_type) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020)); }

inline u32 CPU_TYPE_IS_010() const         { return ((m_cpu_type) == CPU_TYPE_010); }
inline u32 CPU_TYPE_IS_010_PLUS() const    { return ((m_cpu_type) & (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE)); }
inline u32 CPU_TYPE_IS_010_LESS() const    { return ((m_cpu_type) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_SCC070)); }

inline u32 CPU_TYPE_IS_000() const         { return ((m_cpu_type) == CPU_TYPE_000 || (m_cpu_type) == CPU_TYPE_008); }

inline u32 CPU_TYPE_IS_070() const         { return ((m_cpu_type) == CPU_TYPE_SCC070); }


/* Initiates trace checking before each instruction (t1) */
inline void m68ki_trace_t1() { m_tracing = m_t1_flag; }
/* adds t0 to trace checking if we encounter change of flow */
inline void m68ki_trace_t0() { m_tracing |= m_t0_flag; }
/* Clear all tracing */
inline void m68ki_clear_trace() { m_tracing = 0; }
/* Cause a trace exception if we are tracing */
inline void m68ki_exception_if_trace() { if(m_tracing) m68ki_exception_trace(); }

/* -------------------------- EA / Operand Access ------------------------- */

/*
 * The general instruction format follows this pattern:
 * .... XXX. .... .YYY
 * where XXX is register X and YYY is register Y
 */
/* Data Register Isolation */
inline u32 &DX() { return (REG_D()[(m_ir >> 9) & 7]); }
inline u32 &DY() { return (REG_D()[m_ir & 7]); }
/* Address Register Isolation */
inline u32 &AX() { return (REG_A()[(m_ir >> 9) & 7]); }
inline u32 &AY() { return (REG_A()[m_ir & 7]); }


/* Effective Address Calculations */
inline u32 EA_AY_AI_8()   { return AY(); }                              /* address register indirect */
inline u32 EA_AY_AI_16()  { return EA_AY_AI_8(); }
inline u32 EA_AY_AI_32()  { return EA_AY_AI_8(); }
inline u32 EA_AY_PI_8()   { return (AY()++); }                                /* postincrement (size = byte) */
inline u32 EA_AY_PI_16()  { return ((AY()+=2)-2); }                           /* postincrement (size = word) */
inline u32 EA_AY_PI_32()  { return ((AY()+=4)-4); }                           /* postincrement (size = long) */
inline u32 EA_AY_PD_8()   { return (--AY()); }                                /* predecrement (size = byte) */
inline u32 EA_AY_PD_16()  { return (AY()-=2); }                               /* predecrement (size = word) */
inline u32 EA_AY_PD_32()  { return (AY()-=4); }                               /* predecrement (size = long) */
inline u32 EA_AY_DI_8()   { return (AY()+MAKE_INT_16(m68ki_read_imm_16())); } /* displacement */
inline u32 EA_AY_DI_16()  { return EA_AY_DI_8(); }
inline u32 EA_AY_DI_32()  { return EA_AY_DI_8(); }
inline u32 EA_AY_IX_8()   { return m68ki_get_ea_ix(AY()); }                   /* indirect + index */
inline u32 EA_AY_IX_16()  { return EA_AY_IX_8(); }
inline u32 EA_AY_IX_32()  { return EA_AY_IX_8(); }

inline u32 EA_AX_AI_8()   { return AX(); }
inline u32 EA_AX_AI_16()  { return EA_AX_AI_8(); }
inline u32 EA_AX_AI_32()  { return EA_AX_AI_8(); }
inline u32 EA_AX_PI_8()   { return (AX()++); }
inline u32 EA_AX_PI_16()  { return ((AX()+=2)-2); }
inline u32 EA_AX_PI_32()  { return ((AX()+=4)-4); }
inline u32 EA_AX_PD_8()   { return (--AX()); }
inline u32 EA_AX_PD_16()  { return (AX()-=2); }
inline u32 EA_AX_PD_32()  { return (AX()-=4); }
inline u32 EA_AX_DI_8()   { return (AX()+MAKE_INT_16(m68ki_read_imm_16())); }
inline u32 EA_AX_DI_16()  { return EA_AX_DI_8(); }
inline u32 EA_AX_DI_32()  { return EA_AX_DI_8(); }
inline u32 EA_AX_IX_8()   { return m68ki_get_ea_ix(AX()); }
inline u32 EA_AX_IX_16()  { return EA_AX_IX_8(); }
inline u32 EA_AX_IX_32()  { return EA_AX_IX_8(); }

inline u32 EA_A7_PI_8()   { return ((REG_A()[7]+=2)-2); }
inline u32 EA_A7_PD_8()   { return (REG_A()[7]-=2); }

inline u32 EA_AW_8()      { return MAKE_INT_16(m68ki_read_imm_16()); }      /* absolute word */
inline u32 EA_AW_16()     { return EA_AW_8(); }
inline u32 EA_AW_32()     { return EA_AW_8(); }
inline u32 EA_AL_8()      { return m68ki_read_imm_32(); }                   /* absolute long */
inline u32 EA_AL_16()     { return EA_AL_8(); }
inline u32 EA_AL_32()     { return EA_AL_8(); }
inline u32 EA_PCDI_8()    { return m68ki_get_ea_pcdi(); }                   /* pc indirect + displacement */
inline u32 EA_PCDI_16()   { return EA_PCDI_8(); }
inline u32 EA_PCDI_32()   { return EA_PCDI_8(); }
inline u32 EA_PCIX_8()    { return m68ki_get_ea_pcix(); }                   /* pc indirect + index */
inline u32 EA_PCIX_16()   { return EA_PCIX_8(); }
inline u32 EA_PCIX_32()   { return EA_PCIX_8(); }


inline u32 OPER_I_8() { return m68ki_read_imm_8(); }
inline u32 OPER_I_16() { return m68ki_read_imm_16(); }
inline u32 OPER_I_32() { return m68ki_read_imm_32(); }



/* --------------------------- Status Register ---------------------------- */

/* Flag Calculation Macros */
static constexpr u32 CFLAG_8(u32 A) { return (A); }
static constexpr u32 CFLAG_16(u32 A) { return ((A)>>8); }

static constexpr u32 CFLAG_ADD_32(u32 S, u32 D, u32 R) { return (((S & D) | (~R & (S | D)))>>23); }
static constexpr u32 CFLAG_SUB_32(u32 S, u32 D, u32 R) { return (((S & R) | (~D & (S | R)))>>23); }

static constexpr u32 VFLAG_ADD_8(u32 S, u32 D, u32 R) { return ((S^R) & (D^R)); }
static constexpr u32 VFLAG_ADD_16(u32 S, u32 D, u32 R) { return (((S^R) & (D^R))>>8); }
static constexpr u32 VFLAG_ADD_32(u32 S, u32 D, u32 R) { return (((S^R) & (D^R))>>24); }

static constexpr u32 VFLAG_SUB_8(u32 S, u32 D, u32 R) { return ((S^D) & (R^D)); }
static constexpr u32 VFLAG_SUB_16(u32 S, u32 D, u32 R) { return (((S^D) & (R^D))>>8); }
static constexpr u32 VFLAG_SUB_32(u32 S, u32 D, u32 R) { return (((S^D) & (R^D))>>24); }

static constexpr u32 NFLAG_8(u32 A) { return (A); }
static constexpr u32 NFLAG_16(u32 A) { return ((A)>>8); }
static constexpr u32 NFLAG_32(u32 A) { return ((A)>>24); }
static constexpr u32 NFLAG_64(u64 A) { return ((A)>>56); }

static constexpr u32 ZFLAG_8(u32 A) { return MASK_OUT_ABOVE_8(A); }
static constexpr u32 ZFLAG_16(u32 A) { return MASK_OUT_ABOVE_16(A); }
static constexpr u32 ZFLAG_32(u32 A) { return MASK_OUT_ABOVE_32(A); }


/* Flag values */
static constexpr int NFLAG_SET   = 0x80;
static constexpr int NFLAG_CLEAR = 0;
static constexpr int CFLAG_SET   = 0x100;
static constexpr int CFLAG_CLEAR = 0;
static constexpr int XFLAG_SET   = 0x100;
static constexpr int XFLAG_CLEAR = 0;
static constexpr int VFLAG_SET   = 0x80;
static constexpr int VFLAG_CLEAR = 0;
static constexpr int ZFLAG_SET   = 0;
static constexpr int ZFLAG_CLEAR = 0xffffffff;

static constexpr int SFLAG_SET   = 4;
static constexpr int SFLAG_CLEAR = 0;
static constexpr int MFLAG_SET   = 2;
static constexpr int MFLAG_CLEAR = 0;

/* Turn flag values into 1 or 0 */
inline u32 XFLAG_1() const { return ((m_x_flag>>8)&1); }
inline u32 NFLAG_1() const { return ((m_n_flag>>7)&1); }
inline u32 VFLAG_1() const { return ((m_v_flag>>7)&1); }
inline u32 ZFLAG_1() const { return (!m_not_z_flag); }
inline u32 CFLAG_1() const { return ((m_c_flag>>8)&1); }


/* Conditions */
inline u32 COND_CS() const { return (m_c_flag&0x100); }
inline u32 COND_CC() const { return (!COND_CS()); }
inline u32 COND_VS() const { return (m_v_flag&0x80); }
inline u32 COND_VC() const { return (!COND_VS()); }
inline u32 COND_NE() const { return m_not_z_flag; }
inline u32 COND_EQ() const { return (!COND_NE()); }
inline u32 COND_MI() const { return (m_n_flag&0x80); }
inline u32 COND_PL() const { return (!COND_MI()); }
inline u32 COND_LT() const { return ((m_n_flag^m_v_flag)&0x80); }
inline u32 COND_GE() const { return (!COND_LT()); }
inline u32 COND_HI() const { return (COND_CC() && COND_NE()); }
inline u32 COND_LS() const { return (COND_CS() || COND_EQ()); }
inline u32 COND_GT() const { return (COND_GE() && COND_NE()); }
inline u32 COND_LE() const { return (COND_LT() || COND_EQ()); }

/* Reversed conditions */
inline u32 COND_NOT_CS() const { return COND_CC(); }
inline u32 COND_NOT_CC() const { return COND_CS(); }
inline u32 COND_NOT_VS() const { return COND_VC(); }
inline u32 COND_NOT_VC() const { return COND_VS(); }
inline u32 COND_NOT_NE() const { return COND_EQ(); }
inline u32 COND_NOT_EQ() const { return COND_NE(); }
inline u32 COND_NOT_MI() const { return COND_PL(); }
inline u32 COND_NOT_PL() const { return COND_MI(); }
inline u32 COND_NOT_LT() const { return COND_GE(); }
inline u32 COND_NOT_GE() const { return COND_LT(); }
inline u32 COND_NOT_HI() const { return COND_LS(); }
inline u32 COND_NOT_LS() const { return COND_HI(); }
inline u32 COND_NOT_GT() const { return COND_LE(); }
inline u32 COND_NOT_LE() const { return COND_GT(); }

/* Not real conditions, but here for convenience */
inline u32 COND_XS() const { return (m_x_flag&0x100); }
inline u32 COND_XC() const { return (!COND_XS()); }


/* Get the condition code register */
inline u32 m68ki_get_ccr() const    { return((COND_XS() >> 4) |
								(COND_MI() >> 4) |
								(COND_EQ() << 2) |
								(COND_VS() >> 6) |
								(COND_CS() >> 8)); }

/* Get the status register */
inline u32 m68ki_get_sr() const     { return (m_t1_flag         |
								m_t0_flag         |
							(m_s_flag << 11) |
							(m_m_flag << 11) |
								m_int_mask        |
								m68ki_get_ccr()); }



/* ----------------------------- Read / Write ----------------------------- */

/* Read from the current address space */
inline u32 m68ki_read_8(u32 address)          { return m68ki_read_8_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }
inline u32 m68ki_read_16(u32 address)         { return m68ki_read_16_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }
inline u32 m68ki_read_32(u32 address)         { return m68ki_read_32_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }

/* Write to the current data space */
inline void m68ki_write_8(u32 address, u32 value)      { m68ki_write_8_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA, value); }
inline void m68ki_write_16(u32 address, u32 value)     { m68ki_write_16_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA, value); }
inline void m68ki_write_32(u32 address, u32 value)     { m68ki_write_32_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA, value); }
inline void m68ki_write_32_pd(u32 address, u32 value)  { m68ki_write_32_pd_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA, value); }

/* map read immediate 8 to read immediate 16 */
inline u32 m68ki_read_imm_8()         { return MASK_OUT_ABOVE_8(m68ki_read_imm_16()); }

/* Map PC-relative reads */
inline u32 m68ki_read_pcrel_8(u32 address)    { return m68k_read_pcrelative_8(address); }
inline u32 m68ki_read_pcrel_16(u32 address)   { return m68k_read_pcrelative_16(address); }
inline u32 m68ki_read_pcrel_32(u32 address)   { return m68k_read_pcrelative_32(address); }

/* Read from the program space */
inline u32 m68ki_read_program_8(u32 address)  { return m68ki_read_8_fc(address, m_s_flag | FUNCTION_CODE_USER_PROGRAM); }
inline u32 m68ki_read_program_16(u32 address) { return m68ki_read_16_fc(address, m_s_flag | FUNCTION_CODE_USER_PROGRAM); }
inline u32 m68ki_read_program_32(u32 address) { return m68ki_read_32_fc(address, m_s_flag | FUNCTION_CODE_USER_PROGRAM); }

/* Read from the data space */
inline u32 m68ki_read_data_8(u32 address)     { return m68ki_read_8_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }
inline u32 m68ki_read_data_16(u32 address)    { return m68ki_read_16_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }
inline u32 m68ki_read_data_32(u32 address)    { return m68ki_read_32_fc(address, m_s_flag | FUNCTION_CODE_USER_DATA); }



/* ======================================================================== */
/* =============================== PROTOTYPES ============================= */
/* ======================================================================== */

void set_irq_line(int irqline, int state);

void m68k_cause_bus_error();



static const u8    m68ki_shift_8_table[65];
static const u16   m68ki_shift_16_table[65];
static const u32   m68ki_shift_32_table[65];
static const u8    m68ki_exception_cycle_table[8][256];
static const u8    m68ki_ea_idx_cycle_table[64];

/* ======================================================================== */
/* =========================== UTILITY FUNCTIONS ========================== */
/* ======================================================================== */


inline unsigned int m68k_read_pcrelative_8(unsigned int address)
{
	return ((m_readimm16(address&~1)>>(8*(1-(address & 1))))&0xff);
}

inline unsigned int m68k_read_pcrelative_16(unsigned int address)
{
	if (!WORD_ALIGNED(address))
		return
			(m_readimm16(address-1) << 8) |
			(m_readimm16(address+1) >> 8);

	else
		return
			(m_readimm16(address  )      );
}

inline unsigned int m68k_read_pcrelative_32(unsigned int address)
{
	if (!WORD_ALIGNED(address))
		return
			(m_readimm16(address-1) << 24) |
			(m_readimm16(address+1) << 8)  |
			(m_readimm16(address+3) >> 8);

	else
		return
			(m_readimm16(address  ) << 16) |
			(m_readimm16(address+2)      );
}


/* Special call to simulate undocumented 68k behavior when move.l with a
 * predecrement destination mode is executed.
 * A real 68k first writes the high word to [address+2], and then writes the
 * low word to [address].
 */
inline void m68kx_write_memory_32_pd(unsigned int address, unsigned int value)
{
	m_write16(address+2, value>>16);
	m_write16(address, value&0xffff);
}


/* ---------------------------- Read Immediate ---------------------------- */

// clear the instruction cache
inline void m68ki_ic_clear()
{
	int i;
	for (i=0; i< M68K_IC_SIZE; i++) {
		m_ic_address[i] = ~0;
	}
}

// read immediate word using the instruction cache

inline u32 m68ki_ic_readimm16(u32 address)
{
	if (m_cacr & M68K_CACR_EI)
	{
		// 68020 series I-cache (MC68020 User's Manual, Section 4 - On-Chip Cache Memory)
		if (m_cpu_type & (CPU_TYPE_EC020 | CPU_TYPE_020))
		{
			u32 tag = (address >> 8) | (m_s_flag ? 0x1000000 : 0);
			int idx = (address >> 2) & 0x3f;    // 1-of-64 select

			// do a cache fill if the line is invalid or the tags don't match
			if ((!m_ic_valid[idx]) || (m_ic_address[idx] != tag))
			{
				// if the cache is frozen, don't update it
				if (m_cacr & M68K_CACR_FI)
				{
					return m_readimm16(address);
				}

				u32 data = m_read32(address & ~3);

				//printf("m68k: doing cache fill at %08x (tag %08x idx %d)\n", address, tag, idx);

				// if no buserror occurred, validate the tag
				if (!m_mmu_tmp_buserror_occurred)
				{
					m_ic_address[idx] = tag;
					m_ic_data[idx] = data;
					m_ic_valid[idx] = true;
				}
				else
				{
					return m_readimm16(address);
				}
			}

			// at this point, the cache is guaranteed to be valid, either as
			// a hit or because we just filled it.
			if (address & 2)
			{
				return m_ic_data[idx] & 0xffff;
			}
			else
			{
				return m_ic_data[idx] >> 16;
			}
		}
	}

	return m_readimm16(address);
}

/* Handles all immediate reads, does address error check, function code setting,
 * and prefetching if they are enabled in m68kconf.h
 */
inline u32 m68ki_read_imm_16()
{
	u32 result;

	m_mmu_tmp_fc = m_s_flag | FUNCTION_CODE_USER_PROGRAM;
	m_mmu_tmp_rw = 1;
	m_mmu_tmp_sz = M68K_SZ_WORD;
	m68ki_check_address_error(m_pc, MODE_READ, m_s_flag | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */

	if (m_pc != m_pref_addr)
	{
		m_pref_data = m68ki_ic_readimm16(m_pc);
		m_pref_addr = m_mmu_tmp_buserror_occurred ? ~0 : m_pc;
	}
	result = MASK_OUT_ABOVE_16(m_pref_data);
	m_pc += 2;
	if (!m_mmu_tmp_buserror_occurred) {
		// prefetch only if no bus error occurred in opcode fetch
		m_pref_data = m68ki_ic_readimm16(m_pc);
		m_pref_addr = m_mmu_tmp_buserror_occurred ? ~0 : m_pc;
		// ignore bus error on prefetch
		m_mmu_tmp_buserror_occurred = 0;
	}

	return result;
}

inline u32 m68ki_read_imm_32()
{
	u32 temp_val;

	m_mmu_tmp_fc = m_s_flag | FUNCTION_CODE_USER_PROGRAM;
	m_mmu_tmp_rw = 1;
	m_mmu_tmp_sz = M68K_SZ_LONG;
	m68ki_check_address_error(m_pc, MODE_READ, m_s_flag | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */

	if(m_pc != m_pref_addr)
	{
		m_pref_addr = m_pc;
		m_pref_data = m68ki_ic_readimm16(m_pref_addr);
	}
	temp_val = MASK_OUT_ABOVE_16(m_pref_data);
	m_pc += 2;
	m_pref_addr = m_pc;
	m_pref_data = m68ki_ic_readimm16(m_pref_addr);

	temp_val = MASK_OUT_ABOVE_32((temp_val << 16) | MASK_OUT_ABOVE_16(m_pref_data));
	m_pc += 2;
	m_pref_data = m68ki_ic_readimm16(m_pc);
	m_pref_addr = m_mmu_tmp_buserror_occurred ? ~0 : m_pc;

	return temp_val;
}



/* ------------------------- Top level read/write ------------------------- */

/* Handles all memory accesses (except for immediate reads if they are
 * configured to use separate functions in m68kconf.h).
 * All memory accesses must go through these top level functions.
 * These functions will also check for address error and set the function
 * code if they are enabled in m68kconf.h.
 */
inline u32 m68ki_read_8_fc(u32 address, u32 fc)
{
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 1;
	m_mmu_tmp_sz = M68K_SZ_BYTE;
	// Check for reading the FPU's CIRs if this is an '020 or '030.
	// In CPU space (FC 7), addresses 0x0002xxxx are coprocessor interface registers.
	// Bits 15-13 are the coprocessor ID, and bits 0-4 are the register select.
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			return m6888x_read_cir(address);
		}
	}
	return m_read8(address);
}
inline u32 m68ki_read_16_fc(u32 address, u32 fc)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		m68ki_check_address_error(address, MODE_READ, fc);
	}
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 1;
	m_mmu_tmp_sz = M68K_SZ_WORD;
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			return m6888x_read_cir(address);
		}
	}
	return m_read16(address);
}
inline u32 m68ki_read_32_fc(u32 address, u32 fc)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		m68ki_check_address_error(address, MODE_READ, fc);
	}
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 1;
	m_mmu_tmp_sz = M68K_SZ_LONG;
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			return m6888x_read_cir(address);
		}
	}
	return m_read32(address);
}

inline void m68ki_write_8_fc(u32 address, u32 fc, u32 value)
{
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 0;
	m_mmu_tmp_sz = M68K_SZ_BYTE;
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			m6888x_write_cir(address, value);
			return;
		}
	}
	m_write8(address, value);
}
inline void m68ki_write_16_fc(u32 address, u32 fc, u32 value)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		m68ki_check_address_error(address, MODE_WRITE, fc);
	}
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 0;
	m_mmu_tmp_sz = M68K_SZ_WORD;
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			m6888x_write_cir(address, value);
			return;
		}
	}
	m_write16(address, value);
}
inline void m68ki_write_32_fc(u32 address, u32 fc, u32 value)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		m68ki_check_address_error(address, MODE_WRITE, fc);
	}
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 0;
	m_mmu_tmp_sz = M68K_SZ_LONG;
	if ((fc == 7) && CPU_TYPE_IS_020_PLUS() && !CPU_TYPE_IS_040_PLUS())
	{
		if ((address & 0xffffeff0) == 0x00022000)
		{
			m6888x_write_cir(address, value);
			return;
		}
	}
	m_write32(address, value);
}

/* Special call to simulate undocumented 68k behavior when move.l with a
 * predecrement destination mode is executed.
 * A real 68k first writes the high word to [address+2], and then writes the
 * low word to [address].
 */
inline void m68ki_write_32_pd_fc(u32 address, u32 fc, u32 value)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		m68ki_check_address_error(address, MODE_WRITE, fc);
	}
	m_mmu_tmp_fc = fc;
	m_mmu_tmp_rw = 0;
	m_mmu_tmp_sz = M68K_SZ_LONG;
	m_write16(address+2, value>>16);
	m_write16(address, value&0xffff);
}


/* --------------------- Effective Address Calculation -------------------- */

/* The program counter relative addressing modes cause operands to be
 * retrieved from program space, not data space.
 */
inline u32 m68ki_get_ea_pcdi()
{
	u32 old_pc = m_pc;
	return old_pc + MAKE_INT_16(m68ki_read_imm_16());
}


inline u32 m68ki_get_ea_pcix()
{
	return m68ki_get_ea_ix(m_pc);
}

/* Indexed addressing modes are encoded as follows:
 *
 * Base instruction format:
 * F E D C B A 9 8 7 6 | 5 4 3 | 2 1 0
 * x x x x x x x x x x | 1 1 0 | BASE REGISTER      (An)
 *
 * Base instruction format for destination EA in move instructions:
 * F E D C | B A 9    | 8 7 6 | 5 4 3 2 1 0
 * x x x x | BASE REG | 1 1 0 | X X X X X X       (An)
 *
 * Brief extension format:
 *  F  |  E D C   |  B  |  A 9  | 8 | 7 6 5 4 3 2 1 0
 * D/A | REGISTER | W/L | SCALE | 0 |  DISPLACEMENT
 *
 * Full extension format:
 *  F     E D C      B     A 9    8   7    6    5 4       3   2 1 0
 * D/A | REGISTER | W/L | SCALE | 1 | BS | IS | BD SIZE | 0 | I/IS
 * BASE DISPLACEMENT (0, 16, 32 bit)                (bd)
 * OUTER DISPLACEMENT (0, 16, 32 bit)               (od)
 *
 * D/A:     0 = Dn, 1 = An                          (Xn)
 * W/L:     0 = W (sign extend), 1 = L              (.SIZE)
 * SCALE:   00=1, 01=2, 10=4, 11=8                  (*SCALE)
 * BS:      0=add base reg, 1=suppress base reg     (An suppressed)
 * IS:      0=add index, 1=suppress index           (Xn suppressed)
 * BD SIZE: 00=reserved, 01=NULL, 10=Word, 11=Long  (size of bd)
 *
 * IS I/IS Operation
 * 0  000  No Memory Indirect
 * 0  001  indir prex with null outer
 * 0  010  indir prex with word outer
 * 0  011  indir prex with long outer
 * 0  100  reserved
 * 0  101  indir postx with null outer
 * 0  110  indir postx with word outer
 * 0  111  indir postx with long outer
 * 1  000  no memory indirect
 * 1  001  mem indir with null outer
 * 1  010  mem indir with word outer
 * 1  011  mem indir with long outer
 * 1  100-111  reserved
 */
inline u32 m68ki_get_ea_ix(u32 An)
{
	/* An = base register */
	u32 extension = m68ki_read_imm_16();
	u32 Xn = 0;                        /* Index register */
	u32 bd = 0;                        /* Base Displacement */
	u32 od = 0;                        /* Outer Displacement */

	if(CPU_TYPE_IS_010_LESS())
	{
		/* Calculate index */
		Xn = REG_DA()[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);

		/* Add base register and displacement and return */
		return An + Xn + MAKE_INT_8(extension);
	}

	/* Brief extension format */
	if(!BIT_8(extension))
	{
		/* Calculate index */
		Xn = REG_DA()[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		/* Add scale if proper CPU type */
		if(CPU_TYPE_IS_EC020_PLUS())
			Xn <<= (extension>>9) & 3;  /* SCALE */

		/* Add base register and displacement and return */
		return An + Xn + MAKE_INT_8(extension);
	}

	/* Full extension format */

	m_icount -= m68ki_ea_idx_cycle_table[extension&0x3f];

	/* Check if base register is present */
	if(BIT_7(extension))                /* BS */
		An = 0;                         /* An */

	/* Check if index is present */
	if(!BIT_6(extension))               /* IS */
	{
		Xn = REG_DA()[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		Xn <<= (extension>>9) & 3;      /* SCALE */
	}

	/* Check if base displacement is present */
	if(BIT_5(extension))                /* BD SIZE */
		bd = BIT_4(extension) ? m68ki_read_imm_32() : MAKE_INT_16(m68ki_read_imm_16());

	/* If no indirect action, we are done */
	if(!(extension&7))                  /* No Memory Indirect */
		return An + bd + Xn;

	/* Check if outer displacement is present */
	if(BIT_1(extension))                /* I/IS:  od */
		od = BIT_0(extension) ? m68ki_read_imm_32() : MAKE_INT_16(m68ki_read_imm_16());

	/* Postindex */
	if(BIT_2(extension))                /* I/IS:  0 = preindex, 1 = postindex */
		return m68ki_read_32(An + bd) + Xn + od;

	/* Preindex */
	return m68ki_read_32(An + bd + Xn) + od;
}


/* Fetch operands */
inline u32 OPER_AY_AI_8()  {u32 ea = EA_AY_AI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AY_AI_16() {u32 ea = EA_AY_AI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AY_AI_32() {u32 ea = EA_AY_AI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AY_PI_8()  {u32 ea = EA_AY_PI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AY_PI_16() {u32 ea = EA_AY_PI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AY_PI_32() {u32 ea = EA_AY_PI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AY_PD_8()  {u32 ea = EA_AY_PD_8();  return m68ki_read_8(ea); }
inline u32 OPER_AY_PD_16() {u32 ea = EA_AY_PD_16(); return m68ki_read_16(ea);}
inline u32 OPER_AY_PD_32() {u32 ea = EA_AY_PD_32(); return m68ki_read_32(ea);}
inline u32 OPER_AY_DI_8()  {u32 ea = EA_AY_DI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AY_DI_16() {u32 ea = EA_AY_DI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AY_DI_32() {u32 ea = EA_AY_DI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AY_IX_8()  {u32 ea = EA_AY_IX_8();  return m68ki_read_8(ea); }
inline u32 OPER_AY_IX_16() {u32 ea = EA_AY_IX_16(); return m68ki_read_16(ea);}
inline u32 OPER_AY_IX_32() {u32 ea = EA_AY_IX_32(); return m68ki_read_32(ea);}

inline u32 OPER_AX_AI_8()  {u32 ea = EA_AX_AI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AX_AI_16() {u32 ea = EA_AX_AI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AX_AI_32() {u32 ea = EA_AX_AI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AX_PI_8()  {u32 ea = EA_AX_PI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AX_PI_16() {u32 ea = EA_AX_PI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AX_PI_32() {u32 ea = EA_AX_PI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AX_PD_8()  {u32 ea = EA_AX_PD_8();  return m68ki_read_8(ea); }
inline u32 OPER_AX_PD_16() {u32 ea = EA_AX_PD_16(); return m68ki_read_16(ea);}
inline u32 OPER_AX_PD_32() {u32 ea = EA_AX_PD_32(); return m68ki_read_32(ea);}
inline u32 OPER_AX_DI_8()  {u32 ea = EA_AX_DI_8();  return m68ki_read_8(ea); }
inline u32 OPER_AX_DI_16() {u32 ea = EA_AX_DI_16(); return m68ki_read_16(ea);}
inline u32 OPER_AX_DI_32() {u32 ea = EA_AX_DI_32(); return m68ki_read_32(ea);}
inline u32 OPER_AX_IX_8()  {u32 ea = EA_AX_IX_8();  return m68ki_read_8(ea); }
inline u32 OPER_AX_IX_16() {u32 ea = EA_AX_IX_16(); return m68ki_read_16(ea);}
inline u32 OPER_AX_IX_32() {u32 ea = EA_AX_IX_32(); return m68ki_read_32(ea);}

inline u32 OPER_A7_PI_8()  {u32 ea = EA_A7_PI_8();  return m68ki_read_8(ea); }
inline u32 OPER_A7_PD_8()  {u32 ea = EA_A7_PD_8();  return m68ki_read_8(ea); }

inline u32 OPER_AW_8()     {u32 ea = EA_AW_8();     return m68ki_read_8(ea); }
inline u32 OPER_AW_16()    {u32 ea = EA_AW_16();    return m68ki_read_16(ea);}
inline u32 OPER_AW_32()    {u32 ea = EA_AW_32();    return m68ki_read_32(ea);}
inline u32 OPER_AL_8()     {u32 ea = EA_AL_8();     return m68ki_read_8(ea); }
inline u32 OPER_AL_16()    {u32 ea = EA_AL_16();    return m68ki_read_16(ea);}
inline u32 OPER_AL_32()    {u32 ea = EA_AL_32();    return m68ki_read_32(ea);}
inline u32 OPER_PCDI_8()   {u32 ea = EA_PCDI_8();   return m68ki_read_pcrel_8(ea); }
inline u32 OPER_PCDI_16()  {u32 ea = EA_PCDI_16();  return m68ki_read_pcrel_16(ea);}
inline u32 OPER_PCDI_32()  {u32 ea = EA_PCDI_32();  return m68ki_read_pcrel_32(ea);}
inline u32 OPER_PCIX_8()   {u32 ea = EA_PCIX_8();   return m68ki_read_pcrel_8(ea); }
inline u32 OPER_PCIX_16()  {u32 ea = EA_PCIX_16();  return m68ki_read_pcrel_16(ea);}
inline u32 OPER_PCIX_32()  {u32 ea = EA_PCIX_32();  return m68ki_read_pcrel_32(ea);}



/* ---------------------------- Stack Functions --------------------------- */

/* Push/pull data from the stack */
inline void m68ki_push_16(u32 value)
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() - 2);
	m68ki_write_16(REG_SP(), value);
}

inline void m68ki_push_32(u32 value)
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() - 4);
	m68ki_write_32(REG_SP(), value);
}

inline u32 m68ki_pull_16()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() + 2);
	return m68ki_read_16(REG_SP()-2);
}

inline u32 m68ki_pull_32()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() + 4);
	return m68ki_read_32(REG_SP()-4);
}


/* Increment/decrement the stack as if doing a push/pull but
 * don't do any memory access.
 */
inline void m68ki_fake_push_16()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() - 2);
}

inline void m68ki_fake_push_32()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() - 4);
}

inline void m68ki_fake_pull_16()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() + 2);
}

inline void m68ki_fake_pull_32()
{
	REG_SP() = MASK_OUT_ABOVE_32(REG_SP() + 4);
}


/* ----------------------------- Program Flow ----------------------------- */

/* Jump to a new program location or vector.
 * These functions will also call the pc_changed callback if it was enabled
 * in m68kconf.h.
 */
inline void m68ki_jump(u32 new_pc)
{
	m_pc = new_pc;
}

inline void m68ki_jump_vector(u32 vector)
{
	m_pc = (vector<<2) + m_vbr;
	m_pc = m68ki_read_data_32(m_pc);
}


/* Branch to a new memory location.
 * The 32-bit branch will call pc_changed if it was enabled in m68kconf.h.
 * So far I've found no problems with not calling pc_changed for 8 or 16
 * bit branches.
 */
inline void m68ki_branch_8(u32 offset)
{
	m_pc += MAKE_INT_8(offset);
}

inline void m68ki_branch_16(u32 offset)
{
	m_pc += MAKE_INT_16(offset);
}

inline void m68ki_branch_32(u32 offset)
{
	m_pc += offset;
}



/* ---------------------------- Status Register --------------------------- */

/* Set the S flag and change the active stack pointer.
 * Note that value MUST be 4 or 0.
 */
inline void m68ki_set_s_flag(u32 value)
{
	u32 old_s_flag = m_s_flag;
	/* Backup the old stack pointer */
	REG_SP_BASE()[m_s_flag | ((m_s_flag>>1) & m_m_flag)] = REG_SP();
	/* Set the S flag */
	m_s_flag = value;
	/* Set the new stack pointer */
	REG_SP() = REG_SP_BASE()[m_s_flag | ((m_s_flag>>1) & m_m_flag)];
	if ((old_s_flag ^ m_s_flag) & SFLAG_SET)
	{
		debugger_privilege_hook();
	}
}

/* Set the S and M flags and change the active stack pointer.
 * Note that value MUST be 0, 2, 4, or 6 (bit2 = S, bit1 = M).
 */
inline void m68ki_set_sm_flag(u32 value)
{
	u32 old_s_flag = m_s_flag;
	/* Backup the old stack pointer */
	REG_SP_BASE()[m_s_flag | ((m_s_flag >> 1) & m_m_flag)] = REG_SP();
	/* Set the S and M flags */
	m_s_flag = value & SFLAG_SET;
	m_m_flag = value & MFLAG_SET;
	/* Set the new stack pointer */
	REG_SP() = REG_SP_BASE()[m_s_flag | ((m_s_flag>>1) & m_m_flag)];
	if ((old_s_flag ^ m_s_flag) & SFLAG_SET)
	{
		debugger_privilege_hook();
	}
}

/* Set the S and M flags.  Don't touch the stack pointer. */
inline void m68ki_set_sm_flag_nosp(u32 value)
{
	u32 old_s_flag = m_s_flag;
	/* Set the S and M flags */
	m_s_flag = value & SFLAG_SET;
	m_m_flag = value & MFLAG_SET;
	if ((old_s_flag ^ m_s_flag) & SFLAG_SET)
	{
		debugger_privilege_hook();
	}
}


/* Set the condition code register */
inline void m68ki_set_ccr(u32 value)
{
	m_x_flag = BIT_4(value)<< 4;
	m_n_flag = BIT_3(value)<< 4;
	m_not_z_flag = !BIT_2(value);
	m_v_flag = BIT_1(value)<< 6;
	m_c_flag = BIT_0(value)<< 8;
}

/* Set the status register but don't check for interrupts */
inline void m68ki_set_sr_noint(u32 value)
{
	/* Mask out the "unimplemented" bits */
	value &= m_sr_mask;

	/* Now set the status register */
	m_t1_flag = BIT_F(value);
	m_t0_flag = BIT_E(value);
	m_int_mask = value & 0x0700;
	m68ki_set_ccr(value);
	m68ki_set_sm_flag((value >> 11) & 6);
}

/* Set the status register but don't check for interrupts nor
 * change the stack pointer
 */
inline void m68ki_set_sr_noint_nosp(u32 value)
{
	/* Mask out the "unimplemented" bits */
	value &= m_sr_mask;

	/* Now set the status register */
	m_t1_flag = BIT_F(value);
	m_t0_flag = BIT_E(value);
	m_int_mask = value & 0x0700;
	m68ki_set_ccr(value);
	m68ki_set_sm_flag_nosp((value >> 11) & 6);
}

/* Set the status register and check for interrupts */
inline void m68ki_set_sr(u32 value)
{
	m68ki_set_sr_noint(value);
	m68ki_check_interrupts();
}


/* ------------------------- Exception Processing ------------------------- */

/* Initiate exception processing */
inline u32 m68ki_init_exception(u32 vector)
{
	debugger_exception_hook(vector);

	/* Save the old status register */
	u32 sr = m68ki_get_sr();

	/* Turn off trace flag, clear pending traces */
	m_t1_flag = m_t0_flag = 0;
	m68ki_clear_trace();
	/* Enter supervisor mode */
	m68ki_set_s_flag(SFLAG_SET);

	return sr;
}

/* 3 word stack frame (68000 only) */
inline void m68ki_stack_frame_3word(u32 pc, u32 sr)
{
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 0 stack frame.
 * This is the standard stack frame for 68010+.
 */
inline void m68ki_stack_frame_0000(u32 pc, u32 sr, u32 vector)
{
	/* Stack a 3-word frame if we are 68000 */
	if(CPU_TYPE_IS_000())
	{
		m68ki_stack_frame_3word(pc, sr);
		return;
	}
	m68ki_push_16(vector<<2);
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 1 stack frame (68020).
 * For 68020, this is the 4 word throwaway frame.
 */
inline void m68ki_stack_frame_0001(u32 pc, u32 sr, u32 vector)
{
	m68ki_push_16(0x1000 | (vector<<2));
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 2 stack frame.
 * This is used only by 68020 for trap exceptions.
 */
inline void m68ki_stack_frame_0010(u32 sr, u32 vector)
{
	m68ki_push_32(m_ppc);
	m68ki_push_16(0x2000 | (vector<<2));
	m68ki_push_32(m_pc);
	m68ki_push_16(sr);
}


/* Bus error stack frame (68000 only).
 */
inline void m68ki_stack_frame_buserr(u32 sr)
{
	m68ki_push_32(m_pc);
	m68ki_push_16(sr);
	m68ki_push_16(m_ir);
	m68ki_push_32(m_aerr_address);    /* access address */
	/* 0 0 0 0 0 0 0 0 0 0 0 R/W I/N FC
	 * R/W  0 = write, 1 = read
	 * I/N  0 = instruction, 1 = not
	 * FC   3-bit function code
	 */
	m68ki_push_16(m_aerr_write_mode | m_instr_mode | m_aerr_fc);
}

/* Format 8 stack frame (68010).
 * 68010 only.  This is the 29 word bus/address error frame.
 */
inline void m68ki_stack_frame_1000(u32 pc, u32 sr, u32 vector, u32 fault_address)
{
	int orig_rw = m_mmu_tmp_buserror_rw;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_mmu_tmp_buserror_fc;

	/* VERSION
	 * NUMBER
	 * INTERNAL INFORMATION, 16 WORDS
	 */
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();

	/* INSTRUCTION INPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* DATA INPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* DATA OUTPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* FAULT ADDRESS */
	m68ki_push_32(fault_address);

	/* SPECIAL STATUS WORD */
	m68ki_push_16(orig_fc | (orig_rw<<8));

	/* 1000, VECTOR OFFSET */
	m68ki_push_16(0x8000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Format 15 stack frame (68070).
 * 68070 only.  This is the 17 word bus/address error frame.
 */
inline void m68ki_stack_frame_1111(u32 pc, u32 sr, u32 vector, u32 fault_address)
{
	/* INTERNAL INFORMATION */
	m68ki_fake_push_16();

	/* INSTRUCTION INPUT BUFFER */
	m68ki_push_16(0);

	/* INSTRUCTION REGISTER */
	m68ki_push_16(m_ir);

	/* DATA INPUT BUFFER */
	m68ki_push_32(0);

	/* FAULT ADDRESS */
	m68ki_push_32(fault_address);

	/* DATA OUTPUT BUFFER */
	m68ki_push_32(0);

	/* INTERNAL INFORMATION */
	m68ki_fake_push_32();

	/* CURRENT MOVE MULTIPLE MASK */
	m68ki_push_16(0);

	/* SPECIAL STATUS WORD */
	m68ki_push_16(0);

	/* 1111, VECTOR OFFSET */
	m68ki_push_16(0xf000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Format A stack frame (short bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens at an instruction boundary.
 * PC stacked is address of next instruction.
 */
inline void m68ki_stack_frame_1010(u32 sr, u32 vector, u32 pc, u32 fault_address)
{
	int orig_rw = m_mmu_tmp_buserror_rw;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_mmu_tmp_buserror_fc;
	int orig_sz = m_mmu_tmp_buserror_sz;

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA OUTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68ki_push_32(fault_address);

	/* INSTRUCTION PIPE STAGE B */
	m68ki_push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	m68ki_push_16(0);

	/* SPECIAL STATUS REGISTER */
	// set bit for: Rerun Faulted bus Cycle, or run pending prefetch
	// set FC
	m68ki_push_16(0x0100 | orig_fc | orig_rw<<6 | orig_sz<<4);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* 1010, VECTOR OFFSET */
	m68ki_push_16(0xa000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Format B stack frame (long bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens during instruction execution.
 * PC stacked is address of instruction in progress.
 */
inline void m68ki_stack_frame_1011(u32 sr, u32 vector, u32 pc, u32 fault_address)
{
	int orig_rw = m_mmu_tmp_buserror_rw;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_mmu_tmp_buserror_fc;
	int orig_sz = m_mmu_tmp_buserror_sz;
	/* INTERNAL REGISTERS (18 words) */
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);

	/* VERSION# (4 bits), INTERNAL INFORMATION */
	m68ki_push_16(0);

	/* INTERNAL REGISTERS (3 words) */
	m68ki_push_32(0);
	m68ki_push_16(0);

	/* DATA INTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTERS (2 words) */
	m68ki_push_32(0);

	/* STAGE B ADDRESS (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER (4 words) */
	m68ki_push_32(0);
	m68ki_push_32(0);

	/* DATA OUTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68ki_push_32(fault_address);

	/* INSTRUCTION PIPE STAGE B */
	m68ki_push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	m68ki_push_16(0);

	/* SPECIAL STATUS REGISTER */
	m68ki_push_16(0x0100 | orig_fc | (orig_rw<<6) | (orig_sz<<4));

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* 1011, VECTOR OFFSET */
	m68ki_push_16(0xb000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Type 7 stack frame (access fault).
 * This is used by the 68040 for bus fault and mmu trap
 * 30 words
 */
inline void m68ki_stack_frame_0111(u32 sr, u32 vector, u32 pc, u32 fault_address, bool in_mmu)
{
	int orig_rw = m_mmu_tmp_buserror_rw;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_mmu_tmp_buserror_fc;

	/* INTERNAL REGISTERS (18 words) */
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);

	/* FAULT ADDRESS (2 words) */
	m68ki_push_32(fault_address);

	/* INTERNAL REGISTERS (3 words) */
	m68ki_push_32(0);
	m68ki_push_16(0);

	/* SPECIAL STATUS REGISTER (1 word) */
	m68ki_push_16((in_mmu ? 0x400 : 0) | orig_fc | (orig_rw<<8));

	/* EFFECTIVE ADDRESS (2 words) */
	m68ki_push_32(fault_address);

	/* 0111, VECTOR OFFSET (1 word) */
	m68ki_push_16(0x7000 | (vector<<2));

	/* PROGRAM COUNTER (2 words) */
	m68ki_push_32(pc);

	/* STATUS REGISTER (1 word) */
	m68ki_push_16(sr);
}


/* Used for Group 2 exceptions.
 * These stack a type 2 frame on the 020.
 */
inline void m68ki_exception_trap(u32 vector)
{
	u32 sr = m68ki_init_exception(vector);

	if(CPU_TYPE_IS_010_LESS())
		m68ki_stack_frame_0000(m_pc, sr, vector);
	else
		m68ki_stack_frame_0010(sr, vector);

	m68ki_jump_vector(vector);

	/* Use up some clock cycles */
	m_icount -= m_cyc_exception[vector];
}

/* Trap#n stacks a 0 frame but behaves like group2 otherwise */
inline void m68ki_exception_trapN(u32 vector)
{
	u32 sr = m68ki_init_exception(vector);
	m68ki_stack_frame_0000(m_pc, sr, vector);
	m68ki_jump_vector(vector);

	/* Use up some clock cycles */
	m_icount -= m_cyc_exception[vector];
}

/* Exception for trace mode */
inline void m68ki_exception_trace()
{
	u32 sr = m68ki_init_exception(EXCEPTION_TRACE);

	if(CPU_TYPE_IS_010_LESS())
	{
		if(CPU_TYPE_IS_000())
		{
			m_instr_mode = INSTRUCTION_NO;
		}
		m68ki_stack_frame_0000(m_pc, sr, EXCEPTION_TRACE);
	}
	else
		m68ki_stack_frame_0010(sr, EXCEPTION_TRACE);

	m68ki_jump_vector(EXCEPTION_TRACE);

	/* Trace nullifies a STOP instruction */
	m_stopped &= ~STOP_LEVEL_STOP;

	/* Use up some clock cycles */
	m_icount -= m_cyc_exception[EXCEPTION_TRACE];
}

/* Exception for privilege violation */
inline void m68ki_exception_privilege_violation()
{
	u32 sr = m68ki_init_exception(EXCEPTION_PRIVILEGE_VIOLATION);

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	m68ki_stack_frame_0000(m_ppc, sr, EXCEPTION_PRIVILEGE_VIOLATION);
	m68ki_jump_vector(EXCEPTION_PRIVILEGE_VIOLATION);

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_PRIVILEGE_VIOLATION] - m_cyc_instruction[m_ir];
}

/* Exception for A-Line instructions */
inline void m68ki_exception_1010()
{
	u32 sr = m68ki_init_exception(EXCEPTION_1010);
	m68ki_stack_frame_0000(m_ppc, sr, EXCEPTION_1010);
	m68ki_jump_vector(EXCEPTION_1010);

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_1010] - m_cyc_instruction[m_ir];
}

/* Exception for F-Line instructions */
inline void m68ki_exception_1111()
{
	u32 sr = m68ki_init_exception(EXCEPTION_1111);
	m68ki_stack_frame_0000(m_ppc, sr, EXCEPTION_1111);
	m68ki_jump_vector(EXCEPTION_1111);

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_1111] - m_cyc_instruction[m_ir];
}

/* Exception for illegal instructions */
inline void m68ki_exception_illegal()
{
	u32 sr = m68ki_init_exception(EXCEPTION_ILLEGAL_INSTRUCTION);

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	m68ki_stack_frame_0000(m_ppc, sr, EXCEPTION_ILLEGAL_INSTRUCTION);
	m68ki_jump_vector(EXCEPTION_ILLEGAL_INSTRUCTION);

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_ILLEGAL_INSTRUCTION] - m_cyc_instruction[m_ir];
}

/* Exception for format errror in RTE */
inline void m68ki_exception_format_error()
{
	u32 sr = m68ki_init_exception(EXCEPTION_FORMAT_ERROR);
	m68ki_stack_frame_0000(m_pc, sr, EXCEPTION_FORMAT_ERROR);
	m68ki_jump_vector(EXCEPTION_FORMAT_ERROR);

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_FORMAT_ERROR] - m_cyc_instruction[m_ir];
}

/* Exception for address error */
inline void m68ki_exception_address_error()
{
	u32 sr = m68ki_init_exception(EXCEPTION_ADDRESS_ERROR);

	/* If we were processing a bus error, address error, or reset,
	 * this is a catastrophic failure.
	 * Halt the CPU
	 */
	if(m_run_mode == RUN_MODE_BERR_AERR_RESET_WSF)
	{
		m_read8(0x00ffff01);
		m_stopped = STOP_LEVEL_HALT;
		return;
	}

	m_run_mode = RUN_MODE_BERR_AERR_RESET_WSF;

	if (CPU_TYPE_IS_000())
	{
		/* Note: This is implemented for 68000 only! */
		m68ki_stack_frame_buserr(sr);
	}
	else if (CPU_TYPE_IS_010())
	{
		/* only the 68010 throws this unique type-1000 frame */
		m68ki_stack_frame_1000(m_ppc, sr, EXCEPTION_BUS_ERROR, m_mmu_tmp_buserror_address);
	}
	else if (CPU_TYPE_IS_070())
	{
		/* only the 68070 throws this unique type-1111 frame */
		m68ki_stack_frame_1111(m_ppc, sr, EXCEPTION_BUS_ERROR, m_mmu_tmp_buserror_address);
	}
	else if (m_mmu_tmp_buserror_address == m_ppc)
	{
		m68ki_stack_frame_1010(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
	}
	else
	{
		m68ki_stack_frame_1011(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
	}

	m68ki_jump_vector(EXCEPTION_ADDRESS_ERROR);

	m_run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Use up some clock cycles and undo the instruction's cycles */
	m_icount -= m_cyc_exception[EXCEPTION_ADDRESS_ERROR] - m_cyc_instruction[m_ir];
}



/* ASG: Check for interrupts */
inline void m68ki_check_interrupts()
{
	if(m_nmi_pending)
	{
		m_nmi_pending = false;
		m68ki_exception_interrupt(7);
	}
	else if(m_int_level > m_int_mask)
		m68ki_exception_interrupt(m_int_level>>8);
}



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif // MAME_CPU_M68000_M68KCPU_H
