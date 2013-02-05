/* ARM7 core helper Macros / Functions */

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        arm->r[reg]
#define ARM7_ICOUNT         arm->icount


extern void SwitchMode(arm_state *arm, int cpsr_mode_val);

#if 0
#define LOG(x) mame_printf_debug x
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

#define DRCHandleThumbALUAddFlags(rd, rn, op2)													\
	UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | V_MASK | C_MASK));					\
	DRCHandleALUNZFlags(rd);																	\
	UML_XOR(block, I1, rn, ~0);																	\
	UML_CMP(block, I1, op2);																	\
	UML_MOVc(block, COND_B, I1, C_BIT);															\
	UML_MOVc(block, COND_AE, I1, 0);															\
	UML_OR(block, I0, I0, I1);																	\
	UML_XOR(block, I1, rn, op2);																\
	UML_XOR(block, I2, rn, rd);																	\
	UML_AND(block, I1, I1, I2);																	\
	UML_TEST(block, I1, 1 << 31);																\
	UML_MOVc(block, COND_NZ, I1, V_BIT);														\
	UML_MOVc(block, COND_Z, I1, 0);																\
	UML_OR(block, I0, I0, I1);																	\
	UML_OR(block, DRC_CPSR, DRC_CPSR, I0);														\
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

#define DRCHandleThumbALUSubFlags(rd, rn, op2)													\
	UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | V_MASK | C_MASK));					\
	DRCHandleALUNZFlags(rd);																	\
	UML_XOR(block, I1, rn, op2);																\
	UML_XOR(block, I2, rn, rd);																	\
	UML_AND(block, I1, I1, I2);																	\
	UML_TEST(block, I1, 1 << 31);																\
	UML_MOVc(block, COND_NZ, I1, V_BIT);														\
	UML_MOVc(block, COND_Z, I1, 0);																\
	UML_OR(block, I0, I0, I1);																	\
	UML_OR(block, DRC_CPSR, DRC_CPSR, I0);														\
	UML_AND(block, I0, rd, 1 << 31);															\
	UML_AND(block, I1, op2, 1 << 31);															\
	UML_AND(block, I2, rn, 1 << 31);															\
	UML_XOR(block, I2, I2, ~0);																	\
	UML_AND(block, I1, I1, I2);																	\
	UML_AND(block, I2, I2, I0);																	\
	UML_OR(block, I1, I1, I2);																	\
	UML_AND(block, I2, op2, 1 << 31);															\
	UML_AND(block, I2, I2, I0);																	\
	UML_OR(block, I1, I1, I2);																	\
	UML_TEST(block, I1, 1 << 31);																\
	UML_MOVc(block, COND_NZ, I0, C_MASK);														\
	UML_MOVc(block, COND_Z, I0, 0);																\
	UML_OR(block, DRC_CPSR, DRC_CPSR, I0);														\
	UML_ADD(block, DRC_PC, DRC_PC, 2);

/* Set NZC flags for logical operations. */

// This macro (which I didn't write) - doesn't make it obvious that the SIGN BIT = 31, just as the N Bit does,
// therefore, N is set by default
#define HandleALUNZFlags(rd)               \
	(((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))

#define DRCHandleALUNZFlags(rd)					\
	UML_AND(block, I0, rd, SIGN_BIT);			\
	UML_CMP(block, rd, 0);						\
	UML_MOVc(block, COND_E, I1, 1);				\
	UML_MOVc(block, COND_NE, I1, 0);			\
	UML_ROLINS(block, I0, I1, Z_BIT, 1 << Z_BIT);

// Long ALU Functions use bit 63
#define HandleLongALUNZFlags(rd)                            \
	((((rd) & ((UINT64)1 << 63)) >> 32) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc)                  \
	if (insn & INSN_S)                                   \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | C_MASK)) \
				| HandleALUNZFlags(rd)                   \
				| (((sc) != 0) << C_BIT)));              \
	R15 += 4;

#define DRC_RD		mem(&GET_REGISTER(arm, rd))
#define DRC_RS		mem(&GET_REGISTER(arm, rs))
#define DRC_CPSR	mem(&GET_CPSR)
#define DRC_PC		mem(&R15)
#define DRC_REG(i)	mem(&arm->r[(i)]);

#define DRCHandleALULogicalFlags(rd, sc)								\
	if (insn & INSN_S)													\
	{																	\
		UML_AND(block, DRC_CPSR, DRC_CPSR, ~(N_MASK | Z_MASK | C_MASK);	\
		DRCHandleALUNZFlags(rd);										\
		UML_TEST(block, sc, ~0);										\
		UML_MOVc(block, COND_Z, I1, C_BIT);								\
		UML_MOVc(block, COND_NZ, I1, 0);								\
		UML_OR(block, I0, I0, I1);										\
		UML_OR(block, DRC_CPSR, DRC_CPSR, I0);							\
	}																	\
	UML_ADD(block, DRC_PC, DRC_PC, 4);

void set_cpsr( arm_state *arm, UINT32 val);

// used to be functions, but no longer a need, so we'll use define for better speed.
#define GetRegister(arm, rIndex)        ARM7REG(sRegisterTable[GET_MODE][rIndex])
#define SetRegister(arm, rIndex, value) ARM7REG(sRegisterTable[GET_MODE][rIndex]) = value

#define GetModeRegister(arm, mode, rIndex)        ARM7REG(sRegisterTable[mode][rIndex])
#define SetModeRegister(arm, mode, rIndex, value) ARM7REG(sRegisterTable[mode][rIndex]) = value

int arm7_tlb_translate(arm_state *arm, UINT32 *addr, int flags);
void arm7_check_irq_state(arm_state *arm);

typedef const void (*arm7thumb_ophandler)(arm_state*, UINT32, UINT32);
extern arm7thumb_ophandler thumb_handler[0x40*0x10];

typedef const void (*arm7ops_ophandler)(arm_state*, UINT32);

extern arm7ops_ophandler ops_handler[0x10];

extern void (*arm7_coproc_dt_r_callback)(arm_state *arm, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *arm, UINT32 addr));
extern void (*arm7_coproc_dt_w_callback)(arm_state *arm, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *arm, UINT32 addr, UINT32 data));


/***************************************************************************
 * Default Memory Handlers
 ***************************************************************************/
INLINE void arm7_cpu_write32(arm_state *arm, UINT32 addr, UINT32 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~3;
	if ( arm->endian == ENDIANNESS_BIG )
		arm->program->write_dword(addr, data);
	else
		arm->program->write_dword(addr, data);
}


INLINE void arm7_cpu_write16(arm_state *arm, UINT32 addr, UINT16 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~1;
	if ( arm->endian == ENDIANNESS_BIG )
		arm->program->write_word(addr, data);
	else
		arm->program->write_word(addr, data);
}

INLINE void arm7_cpu_write8(arm_state *arm, UINT32 addr, UINT8 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	if ( arm->endian == ENDIANNESS_BIG )
		arm->program->write_byte(addr, data);
	else
		arm->program->write_byte(addr, data);
}

INLINE UINT32 arm7_cpu_read32(arm_state *arm, UINT32 addr)
{
	UINT32 result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	if (addr & 3)
	{
		if ( arm->endian == ENDIANNESS_BIG )
			result = arm->program->read_dword(addr & ~3);
		else
			result = arm->program->read_dword(addr & ~3);
		result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
	}
	else
	{
		if ( arm->endian == ENDIANNESS_BIG )
			result = arm->program->read_dword(addr);
		else
			result = arm->program->read_dword(addr);
	}

	return result;
}

INLINE UINT16 arm7_cpu_read16(arm_state *arm, UINT32 addr)
{
	UINT16 result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	if ( arm->endian == ENDIANNESS_BIG )
		result = arm->program->read_word(addr & ~1);
	else
		result = arm->program->read_word(addr & ~1);

	if (addr & 1)
	{
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
	}

	return result;
}

INLINE UINT8 arm7_cpu_read8(arm_state *arm, UINT32 addr)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( arm, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	// Handle through normal 8 bit handler (for 32 bit cpu)
	if ( arm->endian == ENDIANNESS_BIG )
		return arm->program->read_byte(addr);
	else
		return arm->program->read_byte(addr);
}


/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(arm, addr)
#define WRITE8(addr,data)   arm7_cpu_write8(arm, addr,data)
#define READ16(addr)        arm7_cpu_read16(arm, addr)
#define WRITE16(addr,data)  arm7_cpu_write16(arm, addr,data)
#define READ32(addr)        arm7_cpu_read32(arm, addr)
#define WRITE32(addr,data)  arm7_cpu_write32(arm, addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32
