/* ARM7 core helper Macros / Functions */

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        cpustate->sArmRegister[reg]
#define ARM7_ICOUNT         cpustate->iCount


extern void SwitchMode(arm_state *cpustate, int cpsr_mode_val);

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

/* Set NZC flags for logical operations. */

// This macro (which I didn't write) - doesn't make it obvious that the SIGN BIT = 31, just as the N Bit does,
// therefore, N is set by default
#define HandleALUNZFlags(rd)               \
	(((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))


// Long ALU Functions use bit 63
#define HandleLongALUNZFlags(rd)                            \
	((((rd) & ((UINT64)1 << 63)) >> 32) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc)                  \
	if (insn & INSN_S)                                   \
	SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | C_MASK)) \
				| HandleALUNZFlags(rd)                   \
				| (((sc) != 0) << C_BIT)));              \
	R15 += 4;

void set_cpsr( arm_state *cpustate, UINT32 val);

// used to be functions, but no longer a need, so we'll use define for better speed.
#define GetRegister(cpustate, rIndex)        ARM7REG(sRegisterTable[GET_MODE][rIndex])
#define SetRegister(cpustate, rIndex, value) ARM7REG(sRegisterTable[GET_MODE][rIndex]) = value

#define GetModeRegister(cpustate, mode, rIndex)        ARM7REG(sRegisterTable[mode][rIndex])
#define SetModeRegister(cpustate, mode, rIndex, value) ARM7REG(sRegisterTable[mode][rIndex]) = value

int arm7_tlb_translate(arm_state *cpustate, UINT32 *addr, int flags);
void arm7_check_irq_state(arm_state *cpustate);

typedef const void (*arm7thumb_ophandler)(arm_state*, UINT32, UINT32);

extern arm7thumb_ophandler thumb_handler[0x40*0x10];

typedef const void (*arm7ops_ophandler)(arm_state*, UINT32);

extern arm7ops_ophandler ops_handler[0x10];

extern void (*arm7_coproc_dt_r_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr));
extern void (*arm7_coproc_dt_w_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data));


/***************************************************************************
 * Default Memory Handlers
 ***************************************************************************/
INLINE void arm7_cpu_write32(arm_state *cpustate, UINT32 addr, UINT32 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~3;
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_dword(addr, data);
	else
		cpustate->program->write_dword(addr, data);
}


INLINE void arm7_cpu_write16(arm_state *cpustate, UINT32 addr, UINT16 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~1;
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_word(addr, data);
	else
		cpustate->program->write_word(addr, data);
}

INLINE void arm7_cpu_write8(arm_state *cpustate, UINT32 addr, UINT8 data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_byte(addr, data);
	else
		cpustate->program->write_byte(addr, data);
}

INLINE UINT32 arm7_cpu_read32(arm_state *cpustate, UINT32 addr)
{
	UINT32 result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	if (addr & 3)
	{
		if ( cpustate->endian == ENDIANNESS_BIG )
			result = cpustate->program->read_dword(addr & ~3);
		else
			result = cpustate->program->read_dword(addr & ~3);
		result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
	}
	else
	{
		if ( cpustate->endian == ENDIANNESS_BIG )
			result = cpustate->program->read_dword(addr);
		else
			result = cpustate->program->read_dword(addr);
	}

	return result;
}

INLINE UINT16 arm7_cpu_read16(arm_state *cpustate, UINT32 addr)
{
	UINT16 result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	if ( cpustate->endian == ENDIANNESS_BIG )
		result = cpustate->program->read_word(addr & ~1);
	else
		result = cpustate->program->read_word(addr & ~1);

	if (addr & 1)
	{
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
	}

	return result;
}

INLINE UINT8 arm7_cpu_read8(arm_state *cpustate, UINT32 addr)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!arm7_tlb_translate( cpustate, &addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	// Handle through normal 8 bit handler (for 32 bit cpu)
	if ( cpustate->endian == ENDIANNESS_BIG )
		return cpustate->program->read_byte(addr);
	else
		return cpustate->program->read_byte(addr);
}


/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(cpustate, addr)
#define WRITE8(addr,data)   arm7_cpu_write8(cpustate, addr,data)
#define READ16(addr)        arm7_cpu_read16(cpustate, addr)
#define WRITE16(addr,data)  arm7_cpu_write16(cpustate, addr,data)
#define READ32(addr)        arm7_cpu_read32(cpustate, addr)
#define WRITE32(addr,data)  arm7_cpu_write32(cpustate, addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32
