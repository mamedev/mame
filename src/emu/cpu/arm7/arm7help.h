// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/* ARM7 core helper Macros / Functions */

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        m_r[reg]
#define ARM7_ICOUNT         m_icount


#if 0
#define LOG(x) osd_printf_debug x
#else
#define LOG(x) logerror x
#endif

/***************
 * helper funcs
 ***************/

// TODO LD:
//  - SIGN_BITS_DIFFER = THUMB_SIGN_BITS_DIFFER
//  - do while (0)
//  - HandleALUAddFlags = HandleThumbALUAddFlags except for PC incr
//  - HandleALUSubFlags = HandleThumbALUSubFlags except for PC incr

#define IsNeg(i) ((i) >> 31)
#define IsPos(i) ((~(i)) >> 31)

/* Set NZCV flags for ADDS / SUBS */
#define HandleALUAddFlags(rd, rn, op2)                                                \
	if (insn & INSN_S)                                                                  \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                       \
				| (((!SIGN_BITS_DIFFER(rn, op2)) && SIGN_BITS_DIFFER(rn, rd)) << V_BIT) \
				| (((IsNeg(rn) & IsNeg(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsNeg(op2) & IsPos(rd))) ? C_MASK : 0) \
				| HandleALUNZFlags(rd)));                                               \
	R15 += 4;

#define HandleThumbALUAddFlags(rd, rn, op2)                                                       \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                   \
				| (((!THUMB_SIGN_BITS_DIFFER(rn, op2)) && THUMB_SIGN_BITS_DIFFER(rn, rd)) << V_BIT) \
				| (((~(rn)) < (op2)) << C_BIT)                                                      \
				| HandleALUNZFlags(rd)));                                                           \
	R15 += 2;

#define DRCHandleThumbALUAddFlags(rd, rn, op2)                                                  \
	UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | V_MASK | C_MASK));                   \
	DRCHandleALUNZFlags(rd);                                                                    \
	UML_XOR(block, uml::I1, rn, ~0);                                                            \
	UML_CMP(block, uml::I1, op2);                                                               \
	UML_MOVc(block, uml::COND_B, uml::I1, C_BIT);                                               \
	UML_MOVc(block, uml::COND_AE, uml::I1, 0);                                                  \
	UML_OR(block, uml::I0, uml::I0, uml::I1);                                                   \
	UML_XOR(block, uml::I1, rn, op2);                                                           \
	UML_XOR(block, uml::I2, rn, rd);                                                            \
	UML_AND(block, uml::I1, uml::I1, uml::I2);                                                  \
	UML_TEST(block, uml::I1, 1 << 31);                                                          \
	UML_MOVc(block, uml::COND_NZ, uml::I1, V_BIT);                                              \
	UML_MOVc(block, uml::COND_Z, uml::I1, 0);                                                   \
	UML_OR(block, uml::I0, uml::I0, uml::I1);                                                   \
	UML_OR(block, DRC_CPSR, DRC_CPSR, uml::I0);                                                  \
	UML_ADD(block, DRC_PC, DRC_PC, 2);

#define HandleALUSubFlags(rd, rn, op2)                                                                         \
	if (insn & INSN_S)                                                                                           \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                                \
				| ((SIGN_BITS_DIFFER(rn, op2) && SIGN_BITS_DIFFER(rn, rd)) << V_BIT)                             \
				| (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
				| HandleALUNZFlags(rd)));                                                                        \
	R15 += 4;

#define HandleThumbALUSubFlags(rd, rn, op2)                                                                    \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                                \
				| ((THUMB_SIGN_BITS_DIFFER(rn, op2) && THUMB_SIGN_BITS_DIFFER(rn, rd)) << V_BIT)                 \
				| (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
				| HandleALUNZFlags(rd)));                                                                        \
	R15 += 2;

#define DRCHandleThumbALUSubFlags(rd, rn, op2)                                                  \
	UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | V_MASK | C_MASK));                   \
	DRCHandleALUNZFlags(rd);                                                                    \
	UML_XOR(block, uml::I1, rn, op2);                                                           \
	UML_XOR(block, uml::I2, rn, rd);                                                            \
	UML_AND(block, uml::I1, uml::I1, uml::I2);                                                  \
	UML_TEST(block, uml::I1, 1 << 31);                                                          \
	UML_MOVc(block, uml::COND_NZ, uml::I1, V_BIT);                                              \
	UML_MOVc(block, uml::COND_Z, uml::I1, 0);                                                   \
	UML_OR(block, uml::I0, uml::I0, uml::I1);                                                   \
	UML_OR(block, DRC_CPSR, DRC_CPSR, uml::I0);                                                 \
	UML_AND(block, uml::I0, rd, 1 << 31);                                                       \
	UML_AND(block, uml::I1, op2, 1 << 31);                                                      \
	UML_AND(block, uml::I2, rn, 1 << 31);                                                       \
	UML_XOR(block, uml::I2, uml::I2, ~0);                                                       \
	UML_AND(block, uml::I1, uml::I1, uml::I2);                                                  \
	UML_AND(block, uml::I2, uml::I2, uml::I0);                                                  \
	UML_OR(block, uml::I1, uml::I1, uml::I2);                                                   \
	UML_AND(block, uml::I2, op2, 1 << 31);                                                      \
	UML_AND(block, uml::I2, uml::I2, uml::I0);                                                  \
	UML_OR(block, uml::I1, uml::I1, uml::I2);                                                   \
	UML_TEST(block, uml::I1, 1 << 31);                                                          \
	UML_MOVc(block, uml::COND_NZ, uml::I0, C_MASK);                                             \
	UML_MOVc(block, uml::COND_Z, uml::I0, 0);                                                   \
	UML_OR(block, DRC_CPSR, DRC_CPSR, uml::I0);                                                 \
	UML_ADD(block, DRC_PC, DRC_PC, 2);

/* Set NZC flags for logical operations. */

// This macro (which I didn't write) - doesn't make it obvious that the SIGN BIT = 31, just as the N Bit does,
// therefore, N is set by default
#define HandleALUNZFlags(rd)               \
	(((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))

#define DRCHandleALUNZFlags(rd)                            \
	UML_AND(block, uml::I0, rd, SIGN_BIT);                 \
	UML_CMP(block, rd, 0);                                 \
	UML_MOVc(block, uml::COND_E, uml::I1, 1);              \
	UML_MOVc(block, uml::COND_NE, uml::I1, 0);             \
	UML_ROLINS(block, uml::I0, uml::I1, Z_BIT, 1 << Z_BIT);

// Long ALU Functions use bit 63
#define HandleLongALUNZFlags(rd)                            \
	((((rd) & ((UINT64)1 << 63)) >> 32) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc)                  \
	if (insn & INSN_S)                                   \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | C_MASK)) \
				| HandleALUNZFlags(rd)                   \
				| (((sc) != 0) << C_BIT)));              \
	R15 += 4;

#define DRC_RD      uml::mem(&GET_REGISTER(rd))
#define DRC_RS      uml::mem(&GET_REGISTER(rs))
#define DRC_CPSR    uml::mem(&GET_CPSR)
#define DRC_PC      uml::mem(&R15)
#define DRC_REG(i)  uml::mem(&m_r[(i)])

#define DRCHandleALULogicalFlags(rd, sc)                                \
	if (insn & INSN_S)                                                  \
	{                                                                   \
		UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | C_MASK); \
		DRCHandleALUNZFlags(rd);                                        \
		UML_TEST(block, sc, ~0);                                        \
		UML_MOVc(block, uml::COND_Z, uml::I1, C_BIT);                   \
		UML_MOVc(block, uml::COND_NZ, uml::I1, 0);                      \
		UML_OR(block, uml::I0, uml::I0, uml::I1);                       \
		UML_OR(block, DRC_CPSR, DRC_CPSR, uml::I0);                     \
	}                                                                   \
	UML_ADD(block, DRC_PC, DRC_PC, 4);


// used to be functions, but no longer a need, so we'll use define for better speed.
#define GetRegister(rIndex)        m_r[sRegisterTable[GET_MODE][rIndex]]
#define SetRegister(rIndex, value) m_r[sRegisterTable[GET_MODE][rIndex]] = value

#define GetModeRegister(mode, rIndex)        m_r[sRegisterTable[mode][rIndex]]
#define SetModeRegister(mode, rIndex, value) m_r[sRegisterTable[mode][rIndex]] = value


/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(addr)
#define WRITE8(addr,data)   arm7_cpu_write8(addr,data)
#define READ16(addr)        arm7_cpu_read16(addr)
#define WRITE16(addr,data)  arm7_cpu_write16(addr,data)
#define READ32(addr)        arm7_cpu_read32(addr)
#define WRITE32(addr,data)  arm7_cpu_write32(addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32
