// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  sparcdefs.h - Helpful #defines for emulating the MB86901
//                series of SPARC processor.
//
//================================================================

#pragma once

#ifndef CPU_SPARC_SPARC_DEFS_H
#define CPU_SPARC_SPARC_DEFS_H

#define PSR_CWP_MASK        0x0000001f
#define PSR_ET_SHIFT        5
#define PSR_ET_MASK         0x00000020
#define PSR_PS_SHIFT        6
#define PSR_PS_MASK         0x00000040
#define PSR_S_SHIFT         7
#define PSR_S_MASK          0x00000080
#define PSR_PIL_SHIFT       8
#define PSR_PIL_MASK        0x00000f00
#define PSR_EF_SHIFT        12
#define PSR_EF_MASK         0x00001000
#define PSR_EC_SHIFT        13
#define PSR_EC_MASK         0x00002000
#define PSR_ICC_SHIFT       20
#define PSR_RES_MASK        0x000fc000
#define PSR_ICC_MASK        0x00f00000
#define PSR_N_MASK          0x00800000
#define PSR_N_SHIFT         23
#define PSR_Z_MASK          0x00400000
#define PSR_Z_SHIFT         22
#define PSR_V_MASK          0x00200000
#define PSR_V_SHIFT         21
#define PSR_C_MASK          0x00100000
#define PSR_C_SHIFT         20
#define PSR_VER_SHIFT       24
#define PSR_VER_MASK        0x0f000000
#define PSR_VER             0
#define PSR_IMPL_SHIFT      28
#define PSR_IMPL_MASK       0xf0000000
#define PSR_IMPL            0
#define PSR_ZERO_MASK       (PSR_IMPL_MASK | PSR_VER_MASK | PSR_RES_MASK)

#define ICC_N_SET           (m_psr & PSR_N_MASK)
#define ICC_N               (ICC_N_SET >> PSR_N_SHIFT)
#define ICC_N_CLEAR         (!ICC_N_SET)
#define SET_ICC_N_FLAG      do { m_psr |= PSR_N_MASK; } while(0)
#define CLEAR_ICC_N_FLAG    do { m_psr &= ~PSR_N_MASK; } while(0)

#define ICC_Z_SET           (m_psr & PSR_Z_MASK)
#define ICC_Z               (ICC_Z_SET >> PSR_Z_SHIFT)
#define ICC_Z_CLEAR         (!ICC_Z_SET)
#define SET_ICC_Z_FLAG      do { m_psr |= PSR_Z_MASK; } while(0)
#define CLEAR_ICC_Z_FLAG    do { m_psr &= ~PSR_Z_MASK; } while(0)

#define ICC_V_SET           (m_psr & PSR_V_MASK)
#define ICC_V               (ICC_V_SET >> PSR_V_SHIFT)
#define ICC_V_CLEAR         (!ICC_V_SET)
#define SET_ICC_V_FLAG      do { m_psr |= PSR_V_MASK; } while(0)
#define CLEAR_ICC_V_FLAG    do { m_psr &= ~PSR_V_MASK; } while(0)

#define ICC_C_SET           (m_psr & PSR_C_MASK)
#define ICC_C               (ICC_C_SET >> PSR_C_SHIFT)
#define ICC_C_CLEAR         (!ICC_C_SET)
#define SET_ICC_C_FLAG      do { m_psr |= PSR_C_MASK; } while(0)
#define CLEAR_ICC_C_FLAG    do { m_psr &= ~PSR_C_MASK; } while(0)

#define CLEAR_ICC           do { m_psr &= ~PSR_ICC_MASK; } while(0)

#define TEST_ICC_NZ(x)      do { m_psr &= ~PSR_ICC_MASK; m_psr |= (x & 0x80000000) ? PSR_N_MASK : 0; m_psr |= (x == 0) ? PSR_Z_MASK : 0; } while (0)

#define BREAK_PSR           do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; m_ec = m_psr & PSR_EC_MASK; m_ef = m_psr & PSR_EF_MASK; m_pil = (m_psr & PSR_PIL_MASK) >> PSR_PIL_SHIFT; m_s = m_psr & PSR_S_MASK; m_ps = m_psr & PSR_PS_MASK; m_et = m_psr & PSR_ET_MASK; m_cwp = m_psr & PSR_CWP_MASK; } while(0)
#define MAKE_ICC            do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; } while(0)

#define IS_SUPERVISOR       (m_psr & PSR_S_MASK)
#define IS_USER             (!IS_SUPERVISOR)

#define TRAPS_ENABLED       (m_psr & PSR_ET_MASK)
#define TRAPS_DISABLED      (!TRAPS_ENABLED)

#define PSR                 m_psr
#define WIM                 m_wim
#define TBR                 m_tbr

#define OP_NS   (op & 0xc0000000)

#define OP      (op >> 30)
#define OP2     ((op >> 22) & 7)
#define OP3     ((op >> 19) & 63)
#define OPF     ((op >> 5) & 0x1ff)
#define OPC     ((op >> 5) & 0x1ff)
#define OPFLOW  ((op >> 5) & 0x3f)

#define DISP30  (int32_t(op << 2))
#define DISP22  (int32_t(op << 10) >> 8)
#define DISP19  (int32_t(op << 13) >> 11)
#define DISP16  (int32_t(((op << 10) & 0xc0000000) | ((op << 16) & 0x3fff0000)) >> 14)
#define IMM22   (op << 10)
#define CONST22 (op & 0x3fffff)
#define SIMM13  (int32_t(op << 19) >> 19)
#define SIMM11  (int32_t(op << 21) >> 21)
#define SIMM10  (int32_t(op << 22) >> 22)
#define SIMM8   (int32_t(op << 24) >> 24)
#define IMM7    (op & 0x7f)
#define SIMM7   (int32_t(op << 25) >> 25)
#define SHCNT32 (op & 31)
#define SHCNT64 (op & 63)
#define IAMODE  (op & 0x7)
#define USEIMM  (op & (1 << 13))
#define USEEXT  (op & (1 << 12))


#define COND    ((op >> 25) & 15)
#define RCOND   ((op >> 10) & 7)
#define MOVCOND ((op >> 14) & 15)
#define PRED    (op & (1 << 19))
#define ANNUL   (op & (1 << 29))
#define BRCC    ((op >> 20) & 3)
#define MOVCC   (((op >> 11) & 3) | ((op >> 16) & 4))
#define OPFCC   ((op >> 11) & 7)
#define TCCCC   ((op >> 11) & 3)
#define ASI     (uint8_t)(op >> 5)
#define MMASK   (op & 15)
#define CMASK   ((op >> 4) & 7)

#define RD      ((op >> 25) & 31)
#define RD_D    ((op >> 25) & 30)
#define RDBITS  (op & 0x3e000000)
#define RS1     ((op >> 14) & 31)
#define RS1_D   ((op >> 14) & 30)
#define RS2     (op & 31)
#define RS2_D   (op & 30)

#define FREG(x) m_fpr[(x)]
#define FDREG   m_fpr[RD]
#define FSR     m_fsr

#define REG(x)  *m_regs[(x)]
#define RDREG   *m_regs[RD]
#define RS1REG  *m_regs[RS1]
#define RS2REG  *m_regs[RS2]
#define SET_RDREG(x)    do { if(RDBITS) { RDREG = (x); } } while (0)
#define ADDRESS (USEIMM ? (RS1REG + SIMM13) : (RS1REG + RS2REG))

#define PC      m_pc
#define nPC     m_npc

#define Y       m_y

#define MAE         m_mae
#define HOLD_BUS    m_hold_bus

#define BIT31(x)    ((x) & 0x80000000)

#define UPDATE_PC   true
#define PC_UPDATED  false

#define OP_TYPE0    u32(0)
#define OP_CALL     u32(1)
#define OP_ALU      u32(2)
#define OP_LDST     u32(3)

#define OP_TYPE0_NS (OP_TYPE0 << 30)
#define OP_CALL_NS  (OP_CALL << 30)
#define OP_ALU_NS   (OP_ALU << 30)
#define OP_LDST_NS  (OP_LDST << 30)

#define OP2_UNIMP   0
#define OP2_BICC    2
#define OP2_SETHI   4
#define OP2_FBFCC   6
#define OP2_CBCCC   7

#define OP3_ADD     0
#define OP3_AND     1
#define OP3_OR      2
#define OP3_XOR     3
#define OP3_SUB     4
#define OP3_ANDN    5
#define OP3_ORN     6
#define OP3_XNOR    7
#define OP3_ADDX    8
#define OP3_UMUL    10
#define OP3_SMUL    11
#define OP3_SUBX    12
#define OP3_UDIV    14
#define OP3_SDIV    15
#define OP3_ADDCC   16
#define OP3_ANDCC   17
#define OP3_ORCC    18
#define OP3_XORCC   19
#define OP3_SUBCC   20
#define OP3_ANDNCC  21
#define OP3_ORNCC   22
#define OP3_XNORCC  23
#define OP3_ADDXCC  24
#define OP3_UMULCC  26
#define OP3_SMULCC  27
#define OP3_SUBXCC  28
#define OP3_DIVSCC  29
#define OP3_UDIVCC  30
#define OP3_SDIVCC  31
#define OP3_TADDCC  32
#define OP3_TSUBCC  33
#define OP3_SCAN    34
#define OP3_TADDCCTV    34
#define OP3_TSUBCCTV    35
#define OP3_MULSCC  36
#define OP3_SLL     37
#define OP3_SRL     38
#define OP3_SRA     39
#define OP3_RDASR   40
#define OP3_RDPSR   41
#define OP3_RDWIM   42
#define OP3_RDTBR   43
#define OP3_WRASR   48
#define OP3_WRPSR   49
#define OP3_WRWIM   50
#define OP3_WRTBR   51
#define OP3_FPOP1   52
#define OP3_FPOP2   53
#define OP3_JMPL    56
#define OP3_RETT    57
#define OP3_TICC    58
#define OP3_IFLUSH  59
#define OP3_SAVE    60
#define OP3_RESTORE 61

#define OP3_LD      0
#define OP3_LDUB    1
#define OP3_LDUH    2
#define OP3_LDD     3
#define OP3_ST      4
#define OP3_STB     5
#define OP3_STH     6
#define OP3_STD     7
#define OP3_LDSB    9
#define OP3_LDSH    10
#define OP3_LDSTUB  13
#define OP3_SWAP    15
#define OP3_LDA     16
#define OP3_LDUBA   17
#define OP3_LDUHA   18
#define OP3_LDDA    19
#define OP3_STA     20
#define OP3_STBA    21
#define OP3_STHA    22
#define OP3_STDA    23
#define OP3_LDSBA   25
#define OP3_LDSHA   26
#define OP3_LDSTUBA 29
#define OP3_SWAPA   31
#define OP3_LDFPR   32
#define OP3_LDFSR   33
#define OP3_LDDFPR  35
#define OP3_STFPR   36
#define OP3_STFSR   37
#define OP3_STDFQ   38
#define OP3_STDFPR  39
#define OP3_LDCPR   40
#define OP3_LDCSR   41
#define OP3_LDDCPR  43
#define OP3_STCPR   44
#define OP3_STCSR   45
#define OP3_STDCQ   46
#define OP3_STDCPR  47
#define OP3_CPOP1   54
#define OP3_CPOP2   55

#define COND_BN     0
#define COND_BE     1
#define COND_BLE    2
#define COND_BL     3
#define COND_BLEU   4
#define COND_BCS    5
#define COND_BNEG   6
#define COND_BVS    7
#define COND_BA     8
#define COND_BNE    9
#define COND_BG     10
#define COND_BGE    11
#define COND_BGU    12
#define COND_BCC    13
#define COND_BPOS   14
#define COND_BVC    15

#define LDD     (OP3 == OP3_LDD)
#define LD      (OP3 == OP3_LD)
#define LDSH    (OP3 == OP3_LDSH)
#define LDUH    (OP3 == OP3_LDUH)
#define LDSB    (OP3 == OP3_LDSB)
#define LDUB    (OP3 == OP3_LDUB)
#define LDDF    (OP3 == OP3_LDDFPR)
#define LDF     (OP3 == OP3_LDFPR)
#define LDFSR   (OP3 == OP3_LDFSR)
#define LDDC    (OP3 == OP3_LDDCPR)
#define LDC     (OP3 == OP3_LDCPR)
#define LDCSR   (OP3 == OP3_LDCSR)
#define LDDA    (OP3 == OP3_LDDA)
#define LDA     (OP3 == OP3_LDA)
#define LDSHA   (OP3 == OP3_LDSHA)
#define LDUHA   (OP3 == OP3_LDUHA)
#define LDSBA   (OP3 == OP3_LDSBA)
#define LDUBA   (OP3 == OP3_LDUBA)

#define STD     (OP3 == OP3_STD)
#define ST      (OP3 == OP3_ST)
#define STH     (OP3 == OP3_STH)
#define STB     (OP3 == OP3_STB)
#define STDA    (OP3 == OP3_STDA)
#define STA     (OP3 == OP3_STA)
#define STHA    (OP3 == OP3_STHA)
#define STBA    (OP3 == OP3_STBA)
#define STF     (OP3 == OP3_STFPR)
#define STFSR   (OP3 == OP3_STFSR)
#define STDFQ   (OP3 == OP3_STDFQ)
#define STDF    (OP3 == OP3_STDFPR)
#define STC     (OP3 == OP3_STCPR)
#define STCSR   (OP3 == OP3_STCSR)
#define STDCQ   (OP3 == OP3_STDCQ)
#define STDC    (OP3 == OP3_STDCPR)

#define JMPL    (OP3 == OP3_JMPL)
#define TICC    (OP3 == OP3_TICC)
#define RETT    (OP3 == OP3_RETT)

#define SWAP    (OP3 == OP3_SWAP)
#define SWAPA   (OP3 == OP3_SWAPA)

#define FPOP1   (OP3 == OP3_FPOP1)
#define FPOP2   (OP3 == OP3_FPOP2)
#define CPOP1   (OP3 == OP3_CPOP1)
#define CPOP2   (OP3 == OP3_CPOP2)

#define LDSTUB  (OP3 == OP3_LDSTUB)
#define LDSTUBA (OP3 == OP3_LDSTUBA)

#define ADD     (OP3 == OP3_ADD)
#define ADDX    (OP3 == OP3_ADDX)
#define ADDCC   (OP3 == OP3_ADDCC)
#define ADDXCC  (OP3 == OP3_ADDXCC)

#define SUB     (OP3 == OP3_SUB)
#define SUBX    (OP3 == OP3_SUBX)
#define SUBCC   (OP3 == OP3_SUBCC)
#define SUBXCC  (OP3 == OP3_SUBXCC)

#define TADDCCTV    (OP3 == OP3_TADDCCTV)
#define TSUBCCTV    (OP3 == OP3_TSUBCCTV)

#define AND     (OP3 == OP3_AND)
#define OR      (OP3 == OP3_OR)
#define XOR     (OP3 == OP3_XOR)
#define ANDN    (OP3 == OP3_ANDN)
#define ORN     (OP3 == OP3_ORN)
#define XNOR    (OP3 == OP3_XNOR)
#define ANDCC   (OP3 == OP3_ANDCC)
#define ORCC    (OP3 == OP3_ORCC)
#define XORCC   (OP3 == OP3_XORCC)
#define ANDNCC  (OP3 == OP3_ANDNCC)
#define ORNCC   (OP3 == OP3_ORNCC)
#define XNORCC  (OP3 == OP3_XNORCC)

#define SLL     (OP3 == OP3_SLL)
#define SRL     (OP3 == OP3_SRL)
#define SRA     (OP3 == OP3_SRA)

#define RDASR   (OP3 == OP3_RDASR)
#define RDPSR   (OP3 == OP3_RDPSR)
#define RDWIM   (OP3 == OP3_RDWIM)
#define RDTBR   (OP3 == OP3_RDTBR)

#define WRASR   (OP3 == OP3_WRASR)
#define WRPSR   (OP3 == OP3_WRPSR)
#define WRWIM   (OP3 == OP3_WRWIM)
#define WRTBR   (OP3 == OP3_WRTBR)

#define SAVE    (OP3 == OP3_SAVE)
#define RESTORE (OP3 == OP3_RESTORE)

#define UMUL    (OP3 == OP3_UMUL)
#define UMULCC  (OP3 == OP3_UMULCC)
#define SMUL    (OP3 == OP3_SMUL)
#define SMULCC  (OP3 == OP3_SMULCC)

#define UDIV    (OP3 == OP3_UDIV)
#define UDIVCC  (OP3 == OP3_UDIVCC)
#define SDIV    (OP3 == OP3_SDIV)
#define SDIVCC  (OP3 == OP3_SDIVCC)

#define FSR_CEXC_MASK       0x0000001f
#define FSR_CEXC_NXC        0x00000001
#define FSR_CEXC_DZC        0x00000002
#define FSR_CEXC_UFC        0x00000004
#define FSR_CEXC_OFC        0x00000008
#define FSR_CEXC_NVC        0x00000010

#define FSR_AEXC_SHIFT      5
#define FSR_AEXC_MASK       0x000003e0
#define FSR_AEXC_NXA        0x00000020
#define FSR_AEXC_DZA        0x00000040
#define FSR_AEXC_UFA        0x00000080
#define FSR_AEXC_OFA        0x00000100
#define FSR_AEXC_NVA        0x00000200

#define FSR_FCC_SHIFT       10
#define FSR_FCC_MASK        0x00000c00
#define FSR_FCC_EQ          0x00000000
#define FSR_FCC_LT          0x00000400
#define FSR_FCC_GT          0x00000800
#define FSR_FCC_UO          0x00000c00

#define FSR_QNE             0x00002000

#define FSR_FTT_MASK        0x0001c000
#define FSR_FTT_NONE        0x00000000
#define FSR_FTT_IEEE        0x00004000
#define FSR_FTT_UNFIN       0x00008000
#define FSR_FTT_UNIMP       0x0000c000
#define FSR_FTT_SEQ         0x00010000

#define FSR_VER             0x00020000

#define FSR_NS              0x00400000

#define FSR_TEM_SHIFT       23
#define FSR_TEM_MASK        0x0f800000
#define FSR_TEM_NXM         0x00800000
#define FSR_TEM_DZM         0x01000000
#define FSR_TEM_UFM         0x02000000
#define FSR_TEM_OFM         0x04000000
#define FSR_TEM_NVM         0x08000000

#define FSR_RD_SHIFT        30
#define FSR_RD_MASK         0xc0000000
#define FSR_RD_NEAR         0x00000000
#define FSR_RD_ZERO         0x40000000
#define FSR_RD_UP           0x80000000
#define FSR_RD_DOWN         0xc0000000

#define FSR_RESV_MASK       0x30301000

// FPop1
#define FPOP_FMOVS          0x001
#define FPOP_FNEGS          0x005
#define FPOP_FABSS          0x009
#define FPOP_FSQRTS         0x029
#define FPOP_FSQRTD         0x02a
#define FPOP_FSQRTX         0x02b
#define FPOP_FADDS          0x041
#define FPOP_FADDD          0x042
#define FPOP_FADDX          0x043
#define FPOP_FSUBS          0x045
#define FPOP_FSUBD          0x046
#define FPOP_FSUBX          0x047
#define FPOP_FMULS          0x049
#define FPOP_FMULD          0x04a
#define FPOP_FMULX          0x04b
#define FPOP_FDIVS          0x04d
#define FPOP_FDIVD          0x04e
#define FPOP_FDIVX          0x04f
#define FPOP_FITOS          0x0c4
#define FPOP_FDTOS          0x0c6
#define FPOP_FXTOS          0x0c7
#define FPOP_FITOD          0x0c8
#define FPOP_FSTOD          0x0c9
#define FPOP_FXTOD          0x0cb
#define FPOP_FITOX          0x0cc
#define FPOP_FSTOX          0x0cd
#define FPOP_FDTOX          0x0ce
#define FPOP_FSTOI          0x0d1
#define FPOP_FDTOI          0x0d2
#define FPOP_FXTOI          0x0d3

// FPop2
#define FPOP_FCMPS          0x051
#define FPOP_FCMPD          0x052
#define FPOP_FCMPX          0x053
#define FPOP_FCMPES         0x055
#define FPOP_FCMPED         0x056
#define FPOP_FCMPEX         0x057

#endif // CPU_SPARC_SPARC_DEFS_H
