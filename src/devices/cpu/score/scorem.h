// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

// Special-form
#define     GET_S_FUNC6(op)     ((op >>  1) & 0x3f)
#define     GET_S_RB(op)        ((op >> 10) & 0x1f)
#define     GET_S_RA(op)        ((op >> 15) & 0x1f)
#define     GET_S_RD(op)        ((op >> 20) & 0x1f)
#define     GET_S_CU(op)        (op & 0x01)
#define     GET_S_LK(op)        (op & 0x01)

// I-form
#define     GET_I_FUNC3(op)     ((op >> 17) & 0x07)
#define     GET_I_IMM16(op)     ((op >>  1) & 0xffff)
#define     GET_I_RD(op)        ((op >> 20) & 0x1f)
#define     GET_I_CU(op)        (op & 0x01)

// RI-form
#define     GET_RI_IMM14(op)    ((op >>  1) & 0x3fff)
#define     GET_RI_RA(op)       ((op >> 15) & 0x1f)
#define     GET_RI_RD(op)       ((op >> 20) & 0x1f)
#define     GET_RI_CU(op)       (op & 0x01)

// J-form
#define     GET_J_DISP24(op)    ((op >> 1) & 0x00ffffff)
#define     GET_J_DISP11(op)    ((op >> 1) & 0x000007ff)
#define     GET_J_LK(op)        (op & 0x01)

// BC-form
#define     GET_BC_DISP19(op)   (((op >> 6) & 0x07fe00) | ((op >>  1) & 0x0001ff))
#define     GET_BC_BC(op)       ((op >> 10) & 0x1f)
#define     GET_BC_LK(op)       (op & 0x01)

// RIX-form
#define     GET_RIX_FUNC3(op)   (op & 0x07)
#define     GET_RIX_IMM12(op)   ((op >> 3) & 0x0fff)
#define     GET_RIX_RA(op)      ((op >> 15) & 0x1f)
#define     GET_RIX_RD(op)      ((op >> 20) & 0x1f)

// R-form
#define     GET_R_FUNC4(op)     (op & 0x0f)
#define     GET_R_RA(op)        ((op >> 4) & 0x0f)
#define     GET_R_RD(op)        ((op >> 8) & 0x0f)

// BX-form
#define     GET_BX_DISP8(op)    (op & 0xff)
#define     GET_BX_EC(op)       ((op >> 8) & 0x0f)

// LS-form
#define     GET_LS_RD(op)       ((op >> 20) & 0x1f)
#define     GET_LS_RA(op)       ((op >> 15) & 0x1f)
#define     GET_LS_IMM15(op)    (op & 0x7fff)

// I2-form
#define     GET_I2_RD(op)       ((op >> 8) & 0x0f)
#define     GET_I2_IMM8(op)     (op & 0xff)

// I-form
#define     GET_I16_FUNC3(op)   (op & 0x07)
#define     GET_I16_RD(op)      ((op >> 8) & 0x0f)
#define     GET_I16_IMM5(op)    ((op >> 3) & 0x1f)

// CR-form
#define     GET_CR_OP(op)       (op & 0xff)
#define     GET_CR_IMM10(op)    ((op >>  5) & 0x3ff)
#define     GET_CR_CR(op)       ((op >> 15) & 0x1f)
#define     GET_CR_RD(op)       ((op >> 20) & 0x1f)

// PUSH/POP-form
#define     GET_P_RAG(op)       ((op >> 4) & 0x07)
#define     GET_P_RDG(op)       (((op >> 8) & 0x0f) | ((op >> 3) & 0x10))


// flags
#define     FLAG_V  0x01
#define     FLAG_C  0x02
#define     FLAG_Z  0x04
#define     FLAG_N  0x08
#define     FLAG_T  0x10

#define     GET_V   ((REG_CR & FLAG_V)>>0)
#define     GET_C   ((REG_CR & FLAG_C)>>1)
#define     GET_Z   ((REG_CR & FLAG_Z)>>2)
#define     GET_N   ((REG_CR & FLAG_N)>>3)
#define     GET_T   ((REG_CR & FLAG_T)>>4)

#define     SET_V(f)    if (f) REG_CR |= FLAG_V; else  REG_CR &= ~FLAG_V;
#define     SET_C(f)    if (f) REG_CR |= FLAG_C; else  REG_CR &= ~FLAG_C;
#define     SET_Z(f)    if (f) REG_CR |= FLAG_Z; else  REG_CR &= ~FLAG_Z;
#define     SET_N(f)    if (f) REG_CR |= FLAG_N; else  REG_CR &= ~FLAG_N;
#define     SET_T(f)    if (f) REG_CR |= FLAG_T; else  REG_CR &= ~FLAG_T;

#define     CHECK_Z(r)          SET_Z((r)==0)
#define     CHECK_N(r)          SET_N((INT32)(r) < 0)
#define     CHECK_C_ADD(a,b)    SET_C((a) > (0xffffffffu - (b)))
#define     CHECK_C_SUB(a,b)    SET_C((a) >= (b))
#define     CHECK_V_ADD(a,b,d)  SET_V((INT32)(((a) ^ (d)) & ((b) ^ (d))) < 0)
#define     CHECK_V_SUB(a,b,d)  SET_V((INT32)(((a) ^ (b)) & ((a) ^ (d))) < 0)


// registers
#define     REG_BP      m_gpr[2]
#define     REG_LNK     m_gpr[3]
#define     REG_PSR     m_cr[0]
#define     REG_CR      m_cr[1]
#define     REG_ECR     m_cr[2]
#define     REG_EXCPVEC m_cr[3]
#define     REG_CCR     m_cr[4]
#define     REG_EPC     m_cr[5]
#define     REG_EMA     m_cr[6]
#define     REG_TLBLOCK m_cr[7]
#define     REG_TLBPT   m_cr[8]
#define     REG_PEADDR  m_cr[9]
#define     REG_TLBRPT  m_cr[10]
#define     REG_PEVN    m_cr[11]
#define     REG_PECTX   m_cr[12]
#define     REG_LIMPFN  m_cr[15]
#define     REG_LDMPFN  m_cr[16]
#define     REG_PREV    m_cr[18]
#define     REG_DREG    m_cr[29]
#define     REG_DEPC    m_cr[30]
#define     REG_DSAVE   m_cr[31]
#define     REG_CNT     m_sr[0]
#define     REG_LCR     m_sr[1]
#define     REG_SCR     m_sr[2]
#define     REG_CEH     m_ce[0]
#define     REG_CEL     m_ce[1]


// exceptions causes
#define EXCEPTION_RESET              0
#define EXCEPTION_NMI                1
#define EXCEPTION_ADEL_INSTRUCTION   2
#define EXCEPTION_BUSEL_INSTRUCTION  5
#define EXCEPTION_P_EL               6
#define EXCEPTION_SYSCALL            7
#define EXCEPTION_CCU                8
#define EXCEPTION_RI                 9
#define EXCEPTION_TRAP              10
#define EXCEPTION_ADEL_DATA         11
#define EXCEPTION_ADES_DATA         12
#define EXCEPTION_CEE               16
#define EXCEPTION_CPE               17
#define EXCEPTION_BUSEL_DATA        18
#define EXCEPTION_SWI               19
#define EXCEPTION_INTERRUPT         20
