// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex64.c

    64-bit x64 back-end for the universal machine language.

****************************************************************************

    Future improvements/changes:

    * Add support for FP registers

    * Optimize to avoid unnecessary reloads

    * Identify common pairs and optimize output

    * Convert SUB a,0,b to NEG

    * Optimize, e.g., and [r5],i0,$FF to use rbx as temporary register
        (avoid initial move) if i0 is not needed going forward

****************************************************************************

    -------------------------
    ABI/conventions (Windows)
    -------------------------

    Registers:
        RAX        - volatile, function return value
        RBX        - non-volatile
        RCX        - volatile, integer function parameter 1
        RDX        - volatile, integer function parameter 2
        RSI        - non-volatile
        RDI        - non-volatile
        RBP        - non-volatile
        R8         - volatile, integer function parameter 3
        R9         - volatile, integer function parameter 4
        R10        - volatile
        R11        - volatile, scratch immediate storage
        R12        - non-volatile
        R13        - non-volatile
        R14        - non-volatile
        R15        - non-volatile

        XMM0       - volatile, FP function parameter 1
        XMM1       - volatile, FP function parameter 2
        XMM2       - volatile, FP function parameter 3
        XMM3       - volatile, FP function parameter 4
        XMM4       - volatile
        XMM5       - volatile
        XMM6       - non-volatile
        XMM7       - non-volatile
        XMM8       - non-volatile
        XMM9       - non-volatile
        XMM10      - non-volatile
        XMM11      - non-volatile
        XMM12      - non-volatile
        XMM13      - non-volatile
        XMM14      - non-volatile
        XMM15      - non-volatile


    -----------------------------
    ABI/conventions (Linux/MacOS)
    -----------------------------

    Registers:
        RAX        - volatile, function return value
        RBX        - non-volatile
        RCX        - volatile, integer function parameter 4
        RDX        - volatile, integer function parameter 3
        RSI        - volatile, integer function parameter 2
        RDI        - volatile, integer function parameter 1
        RBP        - non-volatile
        R8         - volatile, integer function parameter 5
        R9         - volatile, integer function parameter 6
        R10        - volatile
        R11        - volatile, scratch immediate storage
        R12        - non-volatile
        R13        - non-volatile
        R14        - non-volatile
        R15        - non-volatile

        XMM0       - volatile, FP function parameter 1
        XMM1       - volatile, FP function parameter 2
        XMM2       - volatile, FP function parameter 3
        XMM3       - volatile, FP function parameter 4
        XMM4       - volatile
        XMM5       - volatile
        XMM6       - volatile
        XMM7       - volatile
        XMM8       - volatile
        XMM9       - volatile
        XMM10      - volatile
        XMM11      - volatile
        XMM12      - volatile
        XMM13      - volatile
        XMM14      - volatile
        XMM15      - volatile


    ---------------
    Execution model
    ---------------

    Registers (Windows):
        RAX        - scratch register
        RBX        - maps to I0
        RCX        - scratch register
        RDX        - scratch register
        RSI        - maps to I1
        RDI        - maps to I2
        RBP        - pointer to code cache
        R8         - scratch register
        R9         - scratch register
        R10        - scratch register
        R11        - scratch register
        R12        - maps to I3
        R13        - maps to I4
        R14        - maps to I5
        R15        - maps to I6

    Registers (Linux/MacOS):
        RAX        - scratch register
        RBX        - maps to I0
        RCX        - scratch register
        RDX        - scratch register
        RSI        - unused
        RDI        - unused
        RBP        - pointer to code cache
        R8         - scratch register
        R9         - scratch register
        R10        - scratch register
        R11        - scratch register
        R12        - maps to I1
        R13        - maps to I2
        R14        - maps to I3
        R15        - maps to I4

    Entry point:
        Assumes 1 parameter passed, which is the codeptr of the code
        to execute once the environment is set up.

    Exit point:
        Assumes exit value is in RAX.

    Entry stack:
        [rsp]      - return

    Runtime stack:
        [rsp]      - r9 home
        [rsp+8]    - r8 home
        [rsp+16]   - rdx home
        [rsp+24]   - rcx home
        [rsp+40]   - saved r15
        [rsp+48]   - saved r14
        [rsp+56]   - saved r13
        [rsp+64]   - saved r12
        [rsp+72]   - saved ebp
        [rsp+80]   - saved edi
        [rsp+88]   - saved esi
        [rsp+96]   - saved ebx
        [rsp+104]  - ret

***************************************************************************/

#include "emu.h"
#include "drcbex64.h"

#include "debugger.h"
#include "emuopts.h"

#include <cstddef>


// This is a trick to make it build on Android where the ARM SDK declares ::REG_Rn
// and the x64 SDK declares ::REG_Exx and ::REG_Rxx
namespace drc {

using namespace uml;

using namespace asmjit;
using namespace asmjit::x86;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_HASHJMPS            (0)

#define USE_RCPSS_FOR_SINGLES   (0)
#define USE_RSQRTSS_FOR_SINGLES (0)
#define USE_RCPSS_FOR_DOUBLES   (0)
#define USE_RSQRTSS_FOR_DOUBLES (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const uint32_t PTYPE_M    = 1 << parameter::PTYPE_MEMORY;
const uint32_t PTYPE_I    = 1 << parameter::PTYPE_IMMEDIATE;
const uint32_t PTYPE_R    = 1 << parameter::PTYPE_INT_REGISTER;
const uint32_t PTYPE_F    = 1 << parameter::PTYPE_FLOAT_REGISTER;
//const uint32_t PTYPE_MI   = PTYPE_M | PTYPE_I;
//const uint32_t PTYPE_RI   = PTYPE_R | PTYPE_I;
const uint32_t PTYPE_MR   = PTYPE_M | PTYPE_R;
const uint32_t PTYPE_MRI  = PTYPE_M | PTYPE_R | PTYPE_I;
const uint32_t PTYPE_MF   = PTYPE_M | PTYPE_F;

#ifdef X64_WINDOWS_ABI

const Gp::Id REG_PARAM1    = Gp::kIdCx;
const Gp::Id REG_PARAM2    = Gp::kIdDx;
const Gp::Id REG_PARAM3    = Gp::kIdR8;
const Gp::Id REG_PARAM4    = Gp::kIdR9;

#else

const Gp::Id REG_PARAM1    = Gp::kIdDi;
const Gp::Id REG_PARAM2    = Gp::kIdSi;
const Gp::Id REG_PARAM3    = Gp::kIdDx;
const Gp::Id REG_PARAM4    = Gp::kIdCx;

#endif



//**************************************************************************
//  MACROS
//**************************************************************************

#define X86_CONDITION(condition)        (condition_map[condition - uml::COND_Z])
#define X86_NOT_CONDITION(condition)    negateCond(condition_map[condition - uml::COND_Z])

#define assert_no_condition(inst)       assert((inst).condition() == uml::COND_ALWAYS)
#define assert_any_condition(inst)      assert((inst).condition() == uml::COND_ALWAYS || ((inst).condition() >= uml::COND_Z && (inst).condition() < uml::COND_MAX))
#define assert_no_flags(inst)           assert((inst).flags() == 0)
#define assert_flags(inst, valid)       assert(((inst).flags() & ~(valid)) == 0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

drcbe_x64::opcode_generate_func drcbe_x64::s_opcode_table[OP_MAX];

// size-to-mask table
//static const uint64_t size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, 0xffffffffffffffffU };

// register mapping tables
static const Gp::Id int_register_map[REG_I_COUNT] =
{
#ifdef X64_WINDOWS_ABI
	Gp::kIdBx, Gp::kIdSi, Gp::kIdDi, Gp::kIdR12, Gp::kIdR13, Gp::kIdR14, Gp::kIdR15,
#else
	Gp::kIdBx, Gp::kIdR12, Gp::kIdR13, Gp::kIdR14, Gp::kIdR15
#endif
};

static uint32_t float_register_map[REG_F_COUNT] =
{
#ifdef X64_WINDOWS_ABI
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15
#else
	// on AMD x64 ABI, XMM0-7 are FP function args.  since this code has no args, and we
	// save/restore them around CALLC, they should be safe for our use.
	0, 1, 2, 3, 4, 5, 6, 7
#endif
};

// condition mapping table
static const CondCode condition_map[uml::COND_MAX - uml::COND_Z] =
{
	CondCode::kZ,    // COND_Z = 0x80,    requires Z
	CondCode::kNZ,   // COND_NZ,          requires Z
	CondCode::kS,    // COND_S,           requires S
	CondCode::kNS,   // COND_NS,          requires S
	CondCode::kC,    // COND_C,           requires C
	CondCode::kNC,   // COND_NC,          requires C
	CondCode::kO,    // COND_V,           requires V
	CondCode::kNO,   // COND_NV,          requires V
	CondCode::kP,    // COND_U,           requires U
	CondCode::kNP,   // COND_NU,          requires U
	CondCode::kA,    // COND_A,           requires CZ
	CondCode::kBE,   // COND_BE,          requires CZ
	CondCode::kG,    // COND_G,           requires SVZ
	CondCode::kLE,   // COND_LE,          requires SVZ
	CondCode::kL,    // COND_L,           requires SV
	CondCode::kGE,   // COND_GE,          requires SV
};

#if 0
// rounding mode mapping table
static const uint8_t fprnd_map[4] =
{
	FPRND_CHOP,     // ROUND_TRUNC,   truncate
	FPRND_NEAR,     // ROUND_ROUND,   round
	FPRND_UP,       // ROUND_CEIL,    round up
	FPRND_DOWN      // ROUND_FLOOR    round down
};
#endif



//**************************************************************************
//  TABLES
//**************************************************************************

const drcbe_x64::opcode_table_entry drcbe_x64::s_opcode_table_source[] =
{
	// Compile-time opcodes
	{ uml::OP_HANDLE,  &drcbe_x64::op_handle },     // HANDLE  handle
	{ uml::OP_HASH,    &drcbe_x64::op_hash },       // HASH    mode,pc
	{ uml::OP_LABEL,   &drcbe_x64::op_label },      // LABEL   imm
	{ uml::OP_COMMENT, &drcbe_x64::op_comment },    // COMMENT string
	{ uml::OP_MAPVAR,  &drcbe_x64::op_mapvar },     // MAPVAR  mapvar,value

	// Control Flow Operations
	{ uml::OP_NOP,     &drcbe_x64::op_nop },        // NOP
	{ uml::OP_DEBUG,   &drcbe_x64::op_debug },      // DEBUG   pc
	{ uml::OP_EXIT,    &drcbe_x64::op_exit },       // EXIT    src1[,c]
	{ uml::OP_HASHJMP, &drcbe_x64::op_hashjmp },    // HASHJMP mode,pc,handle
	{ uml::OP_JMP,     &drcbe_x64::op_jmp },        // JMP     imm[,c]
	{ uml::OP_EXH,     &drcbe_x64::op_exh },        // EXH     handle,param[,c]
	{ uml::OP_CALLH,   &drcbe_x64::op_callh },      // CALLH   handle[,c]
	{ uml::OP_RET,     &drcbe_x64::op_ret },        // RET     [c]
	{ uml::OP_CALLC,   &drcbe_x64::op_callc },      // CALLC   func,ptr[,c]
	{ uml::OP_RECOVER, &drcbe_x64::op_recover },    // RECOVER dst,mapvar

	// Internal Register Operations
	{ uml::OP_SETFMOD, &drcbe_x64::op_setfmod },    // SETFMOD src
	{ uml::OP_GETFMOD, &drcbe_x64::op_getfmod },    // GETFMOD dst
	{ uml::OP_GETEXP,  &drcbe_x64::op_getexp },     // GETEXP  dst
	{ uml::OP_GETFLGS, &drcbe_x64::op_getflgs },    // GETFLGS dst[,f]
	{ uml::OP_SAVE,    &drcbe_x64::op_save },       // SAVE    dst
	{ uml::OP_RESTORE, &drcbe_x64::op_restore },    // RESTORE dst

	// Integer Operations
	{ uml::OP_LOAD,    &drcbe_x64::op_load },       // LOAD    dst,base,index,size
	{ uml::OP_LOADS,   &drcbe_x64::op_loads },      // LOADS   dst,base,index,size
	{ uml::OP_STORE,   &drcbe_x64::op_store },      // STORE   base,index,src,size
	{ uml::OP_READ,    &drcbe_x64::op_read },       // READ    dst,src1,spacesize
	{ uml::OP_READM,   &drcbe_x64::op_readm },      // READM   dst,src1,mask,spacesize
	{ uml::OP_WRITE,   &drcbe_x64::op_write },      // WRITE   dst,src1,spacesize
	{ uml::OP_WRITEM,  &drcbe_x64::op_writem },     // WRITEM  dst,src1,spacesize
	{ uml::OP_CARRY,   &drcbe_x64::op_carry },      // CARRY   src,bitnum
	{ uml::OP_SET,     &drcbe_x64::op_set },        // SET     dst,c
	{ uml::OP_MOV,     &drcbe_x64::op_mov },        // MOV     dst,src[,c]
	{ uml::OP_SEXT,    &drcbe_x64::op_sext },       // SEXT    dst,src
	{ uml::OP_ROLAND,  &drcbe_x64::op_roland },     // ROLAND  dst,src1,src2,src3
	{ uml::OP_ROLINS,  &drcbe_x64::op_rolins },     // ROLINS  dst,src1,src2,src3
	{ uml::OP_ADD,     &drcbe_x64::op_add },        // ADD     dst,src1,src2[,f]
	{ uml::OP_ADDC,    &drcbe_x64::op_addc },       // ADDC    dst,src1,src2[,f]
	{ uml::OP_SUB,     &drcbe_x64::op_sub },        // SUB     dst,src1,src2[,f]
	{ uml::OP_SUBB,    &drcbe_x64::op_subc },       // SUBB    dst,src1,src2[,f]
	{ uml::OP_CMP,     &drcbe_x64::op_cmp },        // CMP     src1,src2[,f]
	{ uml::OP_MULU,    &drcbe_x64::op_mulu },       // MULU    dst,edst,src1,src2[,f]
	{ uml::OP_MULS,    &drcbe_x64::op_muls },       // MULS    dst,edst,src1,src2[,f]
	{ uml::OP_DIVU,    &drcbe_x64::op_divu },       // DIVU    dst,edst,src1,src2[,f]
	{ uml::OP_DIVS,    &drcbe_x64::op_divs },       // DIVS    dst,edst,src1,src2[,f]
	{ uml::OP_AND,     &drcbe_x64::op_and },        // AND     dst,src1,src2[,f]
	{ uml::OP_TEST,    &drcbe_x64::op_test },       // TEST    src1,src2[,f]
	{ uml::OP_OR,      &drcbe_x64::op_or },         // OR      dst,src1,src2[,f]
	{ uml::OP_XOR,     &drcbe_x64::op_xor },        // XOR     dst,src1,src2[,f]
	{ uml::OP_LZCNT,   &drcbe_x64::op_lzcnt },      // LZCNT   dst,src[,f]
	{ uml::OP_TZCNT,   &drcbe_x64::op_tzcnt },      // TZCNT   dst,src[,f]
	{ uml::OP_BSWAP,   &drcbe_x64::op_bswap },      // BSWAP   dst,src
	{ uml::OP_SHL,     &drcbe_x64::op_shift<Inst::kIdShl> },        // SHL     dst,src,count[,f]
	{ uml::OP_SHR,     &drcbe_x64::op_shift<Inst::kIdShr> },        // SHR     dst,src,count[,f]
	{ uml::OP_SAR,     &drcbe_x64::op_shift<Inst::kIdSar> },        // SAR     dst,src,count[,f]
	{ uml::OP_ROL,     &drcbe_x64::op_shift<Inst::kIdRol> },        // ROL     dst,src,count[,f]
	{ uml::OP_ROLC,    &drcbe_x64::op_shift<Inst::kIdRcl> },        // ROLC    dst,src,count[,f]
	{ uml::OP_ROR,     &drcbe_x64::op_shift<Inst::kIdRor> },        // ROR     dst,src,count[,f]
	{ uml::OP_RORC,    &drcbe_x64::op_shift<Inst::kIdRcr> },        // RORC    dst,src,count[,f]

	// Floating Point Operations
	{ uml::OP_FLOAD,   &drcbe_x64::op_fload },      // FLOAD   dst,base,index
	{ uml::OP_FSTORE,  &drcbe_x64::op_fstore },     // FSTORE  base,index,src
	{ uml::OP_FREAD,   &drcbe_x64::op_fread },      // FREAD   dst,space,src1
	{ uml::OP_FWRITE,  &drcbe_x64::op_fwrite },     // FWRITE  space,dst,src1
	{ uml::OP_FMOV,    &drcbe_x64::op_fmov },       // FMOV    dst,src1[,c]
	{ uml::OP_FTOINT,  &drcbe_x64::op_ftoint },     // FTOINT  dst,src1,size,round
	{ uml::OP_FFRINT,  &drcbe_x64::op_ffrint },     // FFRINT  dst,src1,size
	{ uml::OP_FFRFLT,  &drcbe_x64::op_ffrflt },     // FFRFLT  dst,src1,size
	{ uml::OP_FRNDS,   &drcbe_x64::op_frnds },      // FRNDS   dst,src1
	{ uml::OP_FADD,    &drcbe_x64::op_fadd },       // FADD    dst,src1,src2
	{ uml::OP_FSUB,    &drcbe_x64::op_fsub },       // FSUB    dst,src1,src2
	{ uml::OP_FCMP,    &drcbe_x64::op_fcmp },       // FCMP    src1,src2
	{ uml::OP_FMUL,    &drcbe_x64::op_fmul },       // FMUL    dst,src1,src2
	{ uml::OP_FDIV,    &drcbe_x64::op_fdiv },       // FDIV    dst,src1,src2
	{ uml::OP_FNEG,    &drcbe_x64::op_fneg },       // FNEG    dst,src1
	{ uml::OP_FABS,    &drcbe_x64::op_fabs },       // FABS    dst,src1
	{ uml::OP_FSQRT,   &drcbe_x64::op_fsqrt },      // FSQRT   dst,src1
	{ uml::OP_FRECIP,  &drcbe_x64::op_frecip },     // FRECIP  dst,src1
	{ uml::OP_FRSQRT,  &drcbe_x64::op_frsqrt },     // FRSQRT  dst,src1
	{ uml::OP_FCOPYI,  &drcbe_x64::op_fcopyi },     // FCOPYI  dst,src
	{ uml::OP_ICOPYF,  &drcbe_x64::op_icopyf }      // ICOPYF  dst,src
};

class ThrowableErrorHandler : public ErrorHandler
{
public:
	void handleError(Error err, const char *message, BaseEmitter *origin) override
	{
		throw emu_fatalerror("asmjit error %d: %s", err, message);
	}
};


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  param_normalize - convert a full parameter
//  into a reduced set
//-------------------------------------------------

drcbe_x64::be_parameter::be_parameter(drcbe_x64 &drcbe, const parameter &param, uint32_t allowed)
{
	int regnum;

	switch (param.type())
	{
		// immediates pass through
		case parameter::PTYPE_IMMEDIATE:
			assert(allowed & PTYPE_I);
			*this = param.immediate();
			break;

		// memory passes through
		case parameter::PTYPE_MEMORY:
			assert(allowed & PTYPE_M);
			*this = make_memory(param.memory());
			break;

		// if a register maps to a register, keep it as a register; otherwise map it to memory
		case parameter::PTYPE_INT_REGISTER:
			assert(allowed & PTYPE_R);
			assert(allowed & PTYPE_M);
			regnum = int_register_map[param.ireg() - REG_I0];
			if (regnum != 0)
				*this = make_ireg(regnum);
			else
				*this = make_memory(&drcbe.m_state.r[param.ireg() - REG_I0]);
			break;

		// if a register maps to a register, keep it as a register; otherwise map it to memory
		case parameter::PTYPE_FLOAT_REGISTER:
			assert(allowed & PTYPE_F);
			assert(allowed & PTYPE_M);
			regnum = float_register_map[param.freg() - REG_F0];
			if (regnum != 0)
				*this = make_freg(regnum);
			else
				*this = make_memory(&drcbe.m_state.f[param.freg() - REG_F0]);
			break;

		// everything else is unexpected
		default:
			fatalerror("Unexpected parameter type\n");
	}
}


//-------------------------------------------------
//  select_register - select a register to use,
//  avoiding conflicts with the optional
//  checkparam
//-------------------------------------------------

inline Gp drcbe_x64::be_parameter::select_register(Gp defreg) const
{
	if (m_type == PTYPE_INT_REGISTER)
		return Gp(defreg, m_value);
	return defreg;
}

inline Xmm drcbe_x64::be_parameter::select_register(Xmm defreg) const
{
	if (m_type == PTYPE_FLOAT_REGISTER)
		return Xmm(m_value);
	return defreg;
}

Gp drcbe_x64::be_parameter::select_register(Gp defreg, be_parameter const &checkparam) const
{
	if (*this == checkparam)
		return defreg;
	return select_register(defreg);
}

Gp drcbe_x64::be_parameter::select_register(Gp defreg, be_parameter const &checkparam, be_parameter const &checkparam2) const
{
	if (*this == checkparam || *this == checkparam2)
		return defreg;
	return select_register(defreg);
}

Xmm drcbe_x64::be_parameter::select_register(Xmm defreg, be_parameter const &checkparam) const
{
	if (*this == checkparam)
		return defreg;
	return select_register(defreg);
}

//-------------------------------------------------
//  select_register - select a register to use,
//  avoiding conflicts with the optional
//  checkparam
//-------------------------------------------------

inline void drcbe_x64::normalize_commutative(be_parameter &inner, be_parameter &outer)
{
	// if the inner parameter is a memory operand, push it to the outer
	if (inner.is_memory())
	{
		be_parameter temp = inner;
		inner = outer;
		outer = temp;
	}

	// if the inner parameter is an immediate, push it to the outer
	if (inner.is_immediate())
	{
		be_parameter temp = inner;
		inner = outer;
		outer = temp;
	}
}


//-------------------------------------------------
//  offset_from_rbp - return the verified offset
//  from rbp
//-------------------------------------------------

inline int32_t drcbe_x64::offset_from_rbp(const void *ptr) const
{
	const int64_t delta = reinterpret_cast<const uint8_t *>(ptr) - m_rbpvalue;
	if (int32_t(delta) != delta)
		throw emu_fatalerror("drcbe_x64::offset_from_rbp: delta out of range");
	return int32_t(delta);
}


//-------------------------------------------------
//  get_base_register_and_offset - determine right
//  base register and offset to access the given
//  target address
//-------------------------------------------------

inline Gp drcbe_x64::get_base_register_and_offset(Assembler &a, void *target, Gp const &reg, int32_t &offset)
{
	const int64_t delta = reinterpret_cast<uint8_t *>(target) - m_rbpvalue;
	if (short_immediate(delta))
	{
		offset = delta;
		return rbp;
	}
	else
	{
		offset = 0;
		mov_r64_imm(a, reg, uintptr_t(target));                                         // mov   reg,target
		return reg;
	}
}


//-------------------------------------------------
//  smart_call_r64 - generate a call either
//  directly or via a call through pointer
//-------------------------------------------------

inline void drcbe_x64::smart_call_r64(Assembler &a, x86code *target, Gp const &reg)
{
	const int64_t delta = target - (x86code *)(a.code()->baseAddress() + a.offset() + 5);
	if (short_immediate(delta))
		a.call(imm(target));                                                            // call  target
	else
	{
		mov_r64_imm(a, reg, uintptr_t(target));                                         // mov   reg,target
		a.call(reg);                                                                    // call  reg
	}
}


//-------------------------------------------------
//  smart_call_m64 - generate a call either
//  directly or via a call through pointer
//-------------------------------------------------

inline void drcbe_x64::smart_call_m64(Assembler &a, x86code **target)
{
	const int64_t delta = *target - (x86code *)(a.code()->baseAddress() + a.offset() + 5);
	if (short_immediate(delta))
		a.call(imm(*target));                                                           // call  *target
	else
		a.call(MABS(target));                                                           // call  [target]
}



//**************************************************************************
//  BACKEND CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  drcbe_x64 - constructor
//-------------------------------------------------

drcbe_x64::drcbe_x64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device)
	, m_hash(cache, modes, addrbits, ignorebits)
	, m_map(cache, 0xaaaaaaaa5555)
	, m_log(nullptr)
	, m_log_asmjit(nullptr)
	, m_absmask32((uint32_t *)cache.alloc_near(16*2 + 15))
	, m_absmask64(nullptr)
	, m_rbpvalue(cache.near() + 0x80)
	, m_entry(nullptr)
	, m_exit(nullptr)
	, m_nocode(nullptr)
	, m_near(*(near_state *)cache.alloc_near(sizeof(m_near)))
{
	// build up necessary arrays
	static const uint32_t sse_control[4] =
	{
		0xff80,     // ROUND_TRUNC
		0x9f80,     // ROUND_ROUND
		0xdf80,     // ROUND_CEIL
		0xbf80      // ROUND_FLOOR
	};
	memcpy(m_near.ssecontrol, sse_control, sizeof(m_near.ssecontrol));
	m_near.single1 = 1.0f;
	m_near.double1 = 1.0;

	// create absolute value masks that are aligned to SSE boundaries
	m_absmask32 = (uint32_t *)(((uintptr_t)m_absmask32 + 15) & ~15);
	m_absmask32[0] = m_absmask32[1] = m_absmask32[2] = m_absmask32[3] = 0x7fffffff;
	m_absmask64 = (uint64_t *)&m_absmask32[4];
	m_absmask64[0] = m_absmask64[1] = 0x7fffffffffffffffU;

	// get pointers to C functions we need to call
	using debugger_hook_func = void (*)(device_debug *, offs_t);
	static const debugger_hook_func debugger_inst_hook = [] (device_debug *dbg, offs_t pc) { dbg->instruction_hook(pc); }; // TODO: kill trampoline if possible
	m_near.debug_cpu_instruction_hook = (x86code *)debugger_inst_hook;
	if (LOG_HASHJMPS)
	{
		m_near.debug_log_hashjmp = (x86code *)debug_log_hashjmp;
		m_near.debug_log_hashjmp_fail = (x86code *)debug_log_hashjmp_fail;
	}
	m_near.drcmap_get_value = (x86code *)&drc_map_variables::static_get_value;

	// build the flags map
	for (int entry = 0; entry < std::size(m_near.flagsmap); entry++)
	{
		uint8_t flags = 0;
		if (entry & 0x001) flags |= FLAG_C;
		if (entry & 0x004) flags |= FLAG_U;
		if (entry & 0x040) flags |= FLAG_Z;
		if (entry & 0x080) flags |= FLAG_S;
		if (entry & 0x800) flags |= FLAG_V;
		m_near.flagsmap[entry] = flags;
	}
	for (int entry = 0; entry < std::size(m_near.flagsunmap); entry++)
	{
		uint64_t flags = 0;
		if (entry & FLAG_C) flags |= 0x001;
		if (entry & FLAG_U) flags |= 0x004;
		if (entry & FLAG_Z) flags |= 0x040;
		if (entry & FLAG_S) flags |= 0x080;
		if (entry & FLAG_V) flags |= 0x800;
		m_near.flagsunmap[entry] = flags;
	}

	// resolve the actual addresses of the address space handlers
	auto const resolve_accessor =
			[] (resolved_handler &handler, address_space &space, auto accessor)
			{
				if (MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_ITANIUM)
				{
					struct { uintptr_t ptr; ptrdiff_t adj; } equiv;
					assert(sizeof(accessor) == sizeof(equiv));
					*reinterpret_cast<decltype(accessor) *>(&equiv) = accessor;
					handler.obj = uintptr_t(reinterpret_cast<u8 *>(&space) + equiv.adj);
					if (BIT(equiv.ptr, 0))
					{
						auto const vptr = *reinterpret_cast<u8 const *const *>(handler.obj) + equiv.ptr - 1;
						handler.func = *reinterpret_cast<x86code *const *>(vptr);
					}
					else
					{
						handler.func = reinterpret_cast<x86code *>(equiv.ptr);
					}
				}
				else if (MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_MSVC)
				{
					// interpret the pointer to member function ignoring the virtual inheritance variant
					struct single { uintptr_t ptr; };
					struct multi { uintptr_t ptr; int adj; };
					struct { uintptr_t ptr; int adj; int vadj; int vindex; } unknown;
					assert(sizeof(accessor) <= sizeof(unknown));
					*reinterpret_cast<decltype(accessor) *>(&unknown) = accessor;
					handler.func = reinterpret_cast<x86code *>(unknown.ptr);
					handler.obj = uintptr_t(&space);
					if ((sizeof(unknown) == sizeof(accessor)) && unknown.vindex)
					{
						handler.obj += unknown.vadj;
						auto const vptr = *reinterpret_cast<std::uint8_t const *const *>(handler.obj);
						handler.obj += *reinterpret_cast<int const *>(vptr + unknown.vindex);
					}
					if (sizeof(single) < sizeof(accessor))
						handler.obj += unknown.adj;

					// walk past thunks
					while (true)
					{
						if (0xe9 == handler.func[0])
						{
							// absolute jump with 32-bit displacement
							handler.func += 5 + *reinterpret_cast<s32 const *>(handler.func + 1);
						}
						else if ((0x48 == handler.func[0]) && (0x8b == handler.func[1]) && (0x01 == handler.func[2]) && (0xff == handler.func[3]) && ((0x60 == handler.func[4]) || (0xa0 == handler.func[4])))
						{
							// virtual function call thunk
							auto const vptr = *reinterpret_cast<std::uint8_t const *const *>(handler.obj);
							if (0x60 == handler.func[4])
								handler.func = *reinterpret_cast<x86code *const *>(vptr + *reinterpret_cast<s8 const *>(handler.func + 5));
							else
								handler.func = *reinterpret_cast<x86code *const *>(vptr + *reinterpret_cast<s32 const *>(handler.func + 5));
						}
						else
						{
							// not something we can easily bypass
							break;
						}
					}
				}
			};
	m_resolved_accessors.resize(m_space.size());
	for (int space = 0; m_space.size() > space; ++space)
	{
		if (m_space[space])
		{
			resolve_accessor(m_resolved_accessors[space].read_byte,         *m_space[space], static_cast<u8  (address_space::*)(offs_t)     >(&address_space::read_byte));
			resolve_accessor(m_resolved_accessors[space].read_word,         *m_space[space], static_cast<u16 (address_space::*)(offs_t)     >(&address_space::read_word));
			resolve_accessor(m_resolved_accessors[space].read_word_masked,  *m_space[space], static_cast<u16 (address_space::*)(offs_t, u16)>(&address_space::read_word));
			resolve_accessor(m_resolved_accessors[space].read_dword,        *m_space[space], static_cast<u32 (address_space::*)(offs_t)     >(&address_space::read_dword));
			resolve_accessor(m_resolved_accessors[space].read_dword_masked, *m_space[space], static_cast<u32 (address_space::*)(offs_t, u32)>(&address_space::read_dword));
			resolve_accessor(m_resolved_accessors[space].read_qword,        *m_space[space], static_cast<u64 (address_space::*)(offs_t)     >(&address_space::read_qword));
			resolve_accessor(m_resolved_accessors[space].read_qword_masked, *m_space[space], static_cast<u64 (address_space::*)(offs_t, u64)>(&address_space::read_qword));

			resolve_accessor(m_resolved_accessors[space].write_byte,         *m_space[space], static_cast<void (address_space::*)(offs_t, u8)      >(&address_space::write_byte));
			resolve_accessor(m_resolved_accessors[space].write_word,         *m_space[space], static_cast<void (address_space::*)(offs_t, u16)     >(&address_space::write_word));
			resolve_accessor(m_resolved_accessors[space].write_word_masked,  *m_space[space], static_cast<void (address_space::*)(offs_t, u16, u16)>(&address_space::write_word));
			resolve_accessor(m_resolved_accessors[space].write_dword,        *m_space[space], static_cast<void (address_space::*)(offs_t, u32)     >(&address_space::write_dword));
			resolve_accessor(m_resolved_accessors[space].write_dword_masked, *m_space[space], static_cast<void (address_space::*)(offs_t, u32, u32)>(&address_space::write_dword));
			resolve_accessor(m_resolved_accessors[space].write_qword,        *m_space[space], static_cast<void (address_space::*)(offs_t, u64)     >(&address_space::write_qword));
			resolve_accessor(m_resolved_accessors[space].write_qword_masked, *m_space[space], static_cast<void (address_space::*)(offs_t, u64, u64)>(&address_space::write_qword));
		}
	}

	// build the opcode table (static but it doesn't hurt to regenerate it)
	for (auto & elem : s_opcode_table_source)
		s_opcode_table[elem.opcode] = elem.func;

	// create the log
	if (device.machine().options().drc_log_native())
	{
		std::string filename = std::string("drcbex64_").append(device.shortname()).append(".asm");
		m_log = x86log_create_context(filename.c_str());
		m_log_asmjit = fopen(std::string("drcbex64_asmjit_").append(device.shortname()).append(".asm").c_str(), "w");
	}
}


//-------------------------------------------------
//  ~drcbe_x64 - destructor
//-------------------------------------------------

drcbe_x64::~drcbe_x64()
{
	// free the log context
	if (m_log != nullptr)
		x86log_free_context(m_log);

	if (m_log_asmjit)
		fclose(m_log_asmjit);
}

size_t drcbe_x64::emit(CodeHolder &ch)
{
	Error err;

	// the following three calls aren't currently required, but may be if
	// other asmjist features are used in future
	if (false)
	{
		err = ch.flatten();
		if (err)
			throw emu_fatalerror("asmjit::CodeHolder::flatten() error %d", err);

		err = ch.resolveUnresolvedLinks();
		if (err)
			throw emu_fatalerror("asmjit::CodeHolder::resolveUnresolvedLinks() error %d", err);

		err = ch.relocateToBase(ch.baseAddress());
		if (err)
			throw emu_fatalerror("asmjit::CodeHolder::relocateToBase() error %d", err);
	}

	size_t const alignment = ch.baseAddress() - uint64_t(m_cache.top());
	size_t const code_size = ch.codeSize();

	// test if enough room remains in drc cache
	drccodeptr *cachetop = m_cache.begin_codegen(alignment + code_size);
	if (cachetop == nullptr)
		return 0;

	err = ch.copyFlattenedData(drccodeptr(ch.baseAddress()), code_size, CopySectionFlags::kPadTargetBuffer);
	if (err)
		throw emu_fatalerror("asmjit::CodeHolder::copyFlattenedData() error %d", err);

	// update the drc cache and end codegen
	*cachetop += alignment + code_size;
	m_cache.end_codegen();

	return code_size;
}

//-------------------------------------------------
//  reset - reset back-end specific state
//-------------------------------------------------

void drcbe_x64::reset()
{
	// output a note to the log
	if (m_log != nullptr)
		x86log_printf(m_log, "%s", "\n\n===========\nCACHE RESET\n===========\n\n");

	// generate a little bit of glue code to set up the environment
	x86code *dst = (x86code *)m_cache.top();

	CodeHolder ch;
	ch.init(Environment::host(), uint64_t(dst));

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.setFlags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.setIndentation(FormatIndentationGroup::kCode, 4);
		ch.setLogger(&logger);
	}

	Assembler a(&ch);
	if (logger.file())
		a.addDiagnosticOptions(DiagnosticOptions::kValidateIntermediate);

	// generate an entry point
	m_entry = (x86_entry_point_func)dst;
	a.bind(a.newNamedLabel("entry_point"));

	FuncDetail entry_point;
	entry_point.init(FuncSignatureT<uint32_t, uint8_t *, x86code *>(CallConvId::kHost), Environment::host());

	FuncFrame frame;
	frame.init(entry_point);
	frame.addDirtyRegs(rbx, rbp, rsi, rdi, r12, r13, r14, r15);
	FuncArgsAssignment args(&entry_point);
	args.assignAll(rbp);
	args.updateFuncFrame(frame);
	frame.finalize();

	a.emitProlog(frame);
	a.emitArgsAssignment(frame, args);

	a.sub(rsp, 32);
	a.mov(MABS(&m_near.hashstacksave), rsp);
	a.sub(rsp, 8);
	a.mov(MABS(&m_near.stacksave), rsp);
	a.stmxcsr(MABS(&m_near.ssemode));
	a.jmp(Gpq(REG_PARAM2));

	// generate an exit point
	m_exit = dst + a.offset();
	a.bind(a.newNamedLabel("exit_point"));
	a.ldmxcsr(MABS(&m_near.ssemode));
	a.mov(rsp, MABS(&m_near.hashstacksave));
	a.add(rsp, 32);
	a.emitEpilog(frame);

	// generate a no code point
	m_nocode = dst + a.offset();
	a.bind(a.newNamedLabel("nocode_point"));
	a.ret();

	// emit the generated code
	size_t bytes = emit(ch);

	if (m_log != nullptr)
	{
		x86log_disasm_code_range(m_log, "entry_point", dst, m_exit);
		x86log_disasm_code_range(m_log, "exit_point", m_exit, m_nocode);
		x86log_disasm_code_range(m_log, "nocode_point", m_nocode, dst + bytes);
	}

	// reset our hash tables
	m_hash.reset();
	m_hash.set_default_codeptr(m_nocode);
}


//-------------------------------------------------
//  execute - execute a block of code referenced
//  by the given handle
//-------------------------------------------------

int drcbe_x64::execute(code_handle &entry)
{
	// call our entry point which will jump to the destination
	m_cache.codegen_complete();
	return (*m_entry)(m_rbpvalue, (x86code *)entry.codeptr());
}


//-------------------------------------------------
//  drcbex64_generate - generate code
//-------------------------------------------------

void drcbe_x64::generate(drcuml_block &block, const instruction *instlist, uint32_t numinst)
{
	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_map.block_begin(block);

	// compute the base by aligning the cache top to a cache line (assumed to be 64 bytes)
	x86code *dst = (x86code *)(uint64_t(m_cache.top() + 63) & ~63);

	CodeHolder ch;
	ch.init(Environment::host(), uint64_t(dst));
	ThrowableErrorHandler e;
	ch.setErrorHandler(&e);

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.setFlags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.setIndentation(FormatIndentationGroup::kCode, 4);
		ch.setLogger(&logger);
	}

	Assembler a(&ch);
	if (logger.file())
		a.addDiagnosticOptions(DiagnosticOptions::kValidateIntermediate);

	// generate code
	std::string blockname;
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		assert(inst.opcode() < std::size(s_opcode_table));

		// must remain in scope until output
		std::string dasm;

		// add a comment
		if (m_log != nullptr)
		{
			dasm = inst.disasm(&m_drcuml);

			x86log_add_comment(m_log, dst + a.offset(), "%s", dasm.c_str());
			a.setInlineComment(dasm.c_str());
		}

		// extract a blockname
		if (blockname.empty())
		{
			if (inst.opcode() == OP_HANDLE)
				blockname = inst.param(0).handle().string();
			else if (inst.opcode() == OP_HASH)
				blockname = string_format("Code: mode=%d PC=%08X", (uint32_t)inst.param(0).immediate(), (offs_t)inst.param(1).immediate());
		}

		// generate code
		(this->*s_opcode_table[inst.opcode()])(a, inst);
	}

	// emit the generated code
	size_t const bytes = emit(ch);
	if (!bytes)
		block.abort();

	// log it
	if (m_log != nullptr)
		x86log_disasm_code_range(m_log, (blockname.empty()) ? "Unknown block" : blockname.c_str(), dst, dst + bytes);

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_map.block_end(block);
}


//-------------------------------------------------
//  hash_exists - return true if the given mode/pc
//  exists in the hash table
//-------------------------------------------------

bool drcbe_x64::hash_exists(uint32_t mode, uint32_t pc)
{
	return m_hash.code_exists(mode, pc);
}


//-------------------------------------------------
//  get_info - return information about the
//  back-end implementation
//-------------------------------------------------

void drcbe_x64::get_info(drcbe_info &info)
{
	for (info.direct_iregs = 0; info.direct_iregs < REG_I_COUNT; info.direct_iregs++)
		if (int_register_map[info.direct_iregs] == 0)
			break;
	for (info.direct_fregs = 0; info.direct_fregs < REG_F_COUNT; info.direct_fregs++)
		if (float_register_map[info.direct_fregs] == 0)
			break;
}

void drcbe_x64::alu_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param, std::function<bool(Assembler &a, Operand const &dst, be_parameter const &src)> optimize)
{
	bool const is64 = dst.size() == 8;

	if (param.is_immediate())
	{
		if (!optimize(a, dst, param))
		{
			if (is64 && !short_immediate(param.immediate()))
			{
				// use scratch register for 64-bit immediate
				a.mov(r11, param.immediate());                                          // mov   r11,param
				a.emit(opcode, dst, r11);                                               // op    dst,r11
			}
			else
				a.emit(opcode, dst, param.immediate());                                 // op    dst,param
		}
	}
	else if (param.is_memory())
	{
		if (dst.isMem())
		{
			// use temporary register for memory,memory
			Gp const tmp = is64 ? param.select_register(rax) : param.select_register(eax);

			a.mov(tmp, MABS(param.memory()));                                           // mov   tmp,param
			a.emit(opcode, dst, tmp);                                                   // op    [dst],tmp
		}
		else if (opcode != Inst::kIdTest)
			// most instructions are register,memory
			a.emit(opcode, dst, MABS(param.memory()));                                  // op    dst,[param]
		else
			// test instruction requires memory,register
			a.emit(opcode, MABS(param.memory()), dst);                                  // op    [param],dst
	}
	else if (param.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId(is64 ? RegType::kX86_Gpq : RegType::kX86_Gpd, param.ireg());

		a.emit(opcode, dst, src);                                                       // op    dst,param
	}
}

void drcbe_x64::shift_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param)
{
	Operand shift = cl;
	if (param.is_immediate())
		shift = imm(param.immediate());
	else
		mov_reg_param(a, ecx, param);

	a.emit(opcode, dst, shift);
}

void drcbe_x64::mov_reg_param(Assembler &a, Gp const &reg, be_parameter const &param, bool const keepflags)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0 && !keepflags)
			a.xor_(reg.r32(), reg.r32());                                               // xor   reg,reg
		else if (reg.isGpq())
			mov_r64_imm(a, reg, param.immediate());
		else
			a.mov(reg, param.immediate());                                              // mov   reg,param
	}
	else if (param.is_memory())
		a.mov(reg, MABS(param.memory()));                                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg.id() != param.ireg())
			a.mov(reg, Gp(reg, param.ireg()));                                          // mov   reg,param
	}
}

void drcbe_x64::mov_param_reg(Assembler &a, be_parameter const &param, Gp const &reg)
{
	assert(!param.is_immediate());

	if (param.is_memory())
		a.mov(MABS(param.memory()), reg);                                               // mov   [param],reg
	else if (param.is_int_register())
	{
		if (reg.id() != param.ireg())
			a.mov(Gp(reg, param.ireg()), reg);                                          // mov   param,reg
	}
}

void drcbe_x64::mov_mem_param(Assembler &a, Mem const &mem, be_parameter const &param)
{
	bool const is64 = mem.size() == 8;

	if (param.is_immediate())
	{
		if (is64 && !short_immediate(param.immediate()))
		{
			a.mov(rax, param.immediate());                                              // mov   tmp,param
			a.mov(mem, rax);                                                            // mov   [mem],tmp
		}
		else
			a.mov(mem, param.immediate());                                              // mov   [mem],param
	}
	else if (param.is_memory())
	{
		Gp const tmp = Gp::fromTypeAndId(is64 ? RegType::kX86_Gpq : RegType::kX86_Gpd, Gp::kIdAx);

		a.mov(tmp, MABS(param.memory()));                                               // mov   tmp,[param]
		a.mov(mem, tmp);                                                                // mov   [mem],tmp
	}
	else if (param.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId(is64 ? RegType::kX86_Gpq : RegType::kX86_Gpd, param.ireg());

		a.mov(mem, src);                                                                // mov   [mem],param
	}
}

void drcbe_x64::movsx_r64_p32(Assembler &a, Gp const &reg, be_parameter const &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			a.xor_(reg.r32(), reg.r32());                                               // xor   reg,reg
		else if ((int32_t)param.immediate() >= 0)
			a.mov(reg.r32(), param.immediate());                                        // mov   reg,param
		else
			mov_r64_imm(a, reg, int32_t(param.immediate()));                            // mov   reg,param
	}
	else if (param.is_memory())
		a.movsxd(reg, MABS(param.memory()));                                            // movsxd reg,[param]
	else if (param.is_int_register())
		a.movsxd(reg, Gpd(param.ireg()));                                               // movsxd reg,param
}

void drcbe_x64::mov_r64_imm(Assembler &a, Gp const &reg, uint64_t const imm)
{
	if (u32(imm) == imm)
		a.mov(reg.r32(), imm);
	else if (s32(imm) == imm)
		a.mov(reg.r64(), s32(imm));
	else
		a.mov(reg.r64(), imm);
}


/***************************************************************************
    EMITTERS FOR FLOATING POINT OPERATIONS WITH PARAMETERS
***************************************************************************/

//-------------------------------------------------
//  movss_r128_p32 - move a 32-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::movss_r128_p32(Assembler &a, Xmm const &reg, be_parameter const &param)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		a.movss(reg, MABS(param.memory(), 4));                                          // movss reg,[param]
	else if (param.is_float_register())
	{
		if (reg.id() != param.freg())
			a.movss(reg, Xmm(param.freg()));                                            // movss reg,param
	}
}


//-------------------------------------------------
//  movss_p32_r128 - move a register into a
//  32-bit parameter
//-------------------------------------------------

void drcbe_x64::movss_p32_r128(Assembler &a, be_parameter const &param, Xmm const &reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		a.movss(MABS(param.memory(), 4), reg);                                          // movss [param],reg
	else if (param.is_float_register())
	{
		if (reg.id() != param.freg())
			a.movss(Xmm(param.freg()), reg);                                            // movss param,reg
	}
}


//-------------------------------------------------
//  movsd_r128_p64 - move a 64-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::movsd_r128_p64(Assembler &a, Xmm const &reg, be_parameter const &param)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		a.movsd(reg, MABS(param.memory(), 8));                                          // movsd reg,[param]
	else if (param.is_float_register())
	{
		if (reg.id() != param.freg())
			a.movsd(reg, Xmm(param.freg()));                                            // movsd reg,param
	}
}


//-------------------------------------------------
//  movsd_p64_r128 - move a register into a
//  64-bit parameter
//-------------------------------------------------

void drcbe_x64::movsd_p64_r128(Assembler &a, be_parameter const &param, Xmm const &reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		a.movsd(MABS(param.memory(), 8), reg);                                          // movsd [param],reg
	else if (param.is_float_register())
	{
		if (reg.id() != param.freg())
			a.movsd(Xmm(param.freg()), reg);                                            // movsd param,reg
	}
}


//**************************************************************************
//  DEBUG HELPERS
//**************************************************************************

//-------------------------------------------------
//  debug_log_hashjmp - callback to handle
//  logging of hashjmps
//-------------------------------------------------

void drcbe_x64::debug_log_hashjmp(offs_t pc, int mode)
{
	printf("mode=%d PC=%08X\n", mode, pc);
}


//-------------------------------------------------
//  debug_log_hashjmp - callback to handle
//  logging of hashjmps
//-------------------------------------------------

void drcbe_x64::debug_log_hashjmp_fail()
{
	printf("  (FAIL)\n");
}


/***************************************************************************
    COMPILE-TIME OPCODES
***************************************************************************/

//-------------------------------------------------
//  op_handle - process a HANDLE opcode
//-------------------------------------------------

void drcbe_x64::op_handle(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_handle());

	// make a label for documentation
	Label handle = a.newNamedLabel(inst.param(0).handle().string());
	a.bind(handle);

	// emit a jump around the stack adjust in case code falls through here
	Label skip = a.newLabel();
	a.short_().jmp(skip);                                                               // jmp   skip

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(drccodeptr(a.code()->baseAddress() + a.offset()));

	// by default, the handle points to prolog code that moves the stack pointer
	a.lea(rsp, ptr(rsp, -40));                                                          // lea   rsp,[rsp-40]
	a.bind(skip);                                                                   // skip:
}


//-------------------------------------------------
//  op_hash - process a HASH opcode
//-------------------------------------------------

void drcbe_x64::op_hash(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	// register the current pointer for the mode/PC
	m_hash.set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), drccodeptr(a.code()->baseAddress() + a.offset()));
}


//-------------------------------------------------
//  op_label - process a LABEL opcode
//-------------------------------------------------

void drcbe_x64::op_label(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_label());

	std::string labelName = util::string_format("PC$%x", inst.param(0).label());
	Label label = a.labelByName(labelName.c_str());
	if (!label.isValid())
		label = a.newNamedLabel(labelName.c_str());

	// register the current pointer for the label
	a.bind(label);
}


//-------------------------------------------------
//  op_comment - process a COMMENT opcode
//-------------------------------------------------

void drcbe_x64::op_comment(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_string());

	// do nothing
}


//-------------------------------------------------
//  op_mapvar - process a MAPVAR opcode
//-------------------------------------------------

void drcbe_x64::op_mapvar(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_mapvar());
	assert(inst.param(1).is_immediate());

	// set the value of the specified mapvar
	m_map.set_value(drccodeptr(a.code()->baseAddress() + a.offset()), inst.param(0).mapvar(), inst.param(1).immediate());
}



/***************************************************************************
    CONTROL FLOW OPCODES
***************************************************************************/

//-------------------------------------------------
//  op_nop - process a NOP opcode
//-------------------------------------------------

void drcbe_x64::op_nop(Assembler &a, const instruction &inst)
{
	// nothing
}


//-------------------------------------------------
//  op_debug - process a DEBUG opcode
//-------------------------------------------------

void drcbe_x64::op_debug(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	if ((m_device.machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		// normalize parameters
		be_parameter pcp(*this, inst.param(0), PTYPE_MRI);

		// test and branch
		mov_r64_imm(a, rax, (uintptr_t)&m_device.machine().debug_flags);                // mov   rax,&debug_flags
		a.test(dword_ptr(rax), DEBUG_FLAG_CALL_HOOK);                                   // test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		Label skip = a.newLabel();
		a.short_().jz(skip);

		// push the parameter
		mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_device.debug());                   // mov   param1,device.debug
		mov_reg_param(a, Gpd(REG_PARAM2), pcp);                                         // mov   param2,pcp
		smart_call_m64(a, &m_near.debug_cpu_instruction_hook);                          // call  debug_cpu_instruction_hook

		a.bind(skip);
	}
}


//-------------------------------------------------
//  op_exit - process an EXIT opcode
//-------------------------------------------------

void drcbe_x64::op_exit(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter retp(*this, inst.param(0), PTYPE_MRI);

	// load the parameter into EAX
	mov_reg_param(a, eax, retp);                                                        // mov   eax,retp
	if (inst.condition() == uml::COND_ALWAYS)
		a.jmp(imm(m_exit));                                                             // jmp   exit
	else
		a.j(X86_CONDITION(inst.condition()), imm(m_exit));                              // jcc   exit
}


//-------------------------------------------------
//  op_hashjmp - process a HASHJMP opcode
//-------------------------------------------------

void drcbe_x64::op_hashjmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter modep(*this, inst.param(0), PTYPE_MRI);
	be_parameter pcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &exp = inst.param(2);
	assert(exp.is_code_handle());

	if (LOG_HASHJMPS)
	{
		mov_reg_param(a, Gpd(REG_PARAM1), pcp);
		mov_reg_param(a, Gpd(REG_PARAM2), modep);
		smart_call_m64(a, &m_near.debug_log_hashjmp);
	}

	// load the stack base one word early so we end up at the right spot after our call below
	a.mov(rsp, MABS(&m_near.hashstacksave));                                            // mov   rsp,[hashstacksave]

	// fixed mode cases
	if (modep.is_immediate() && m_hash.is_mode_populated(modep.immediate()))
	{
		// a straight immediate jump is direct, though we need the PC in EAX in case of failure
		if (pcp.is_immediate())
		{
			uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			a.call(MABS(&m_hash.base()[modep.immediate()][l1val][l2val]));              // call  hash[modep][l1val][l2val]
		}

		// a fixed mode but variable PC
		else
		{
			mov_reg_param(a, eax, pcp);                                                 // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and  eax,l2mask << l2shift
			a.mov(rdx, ptr(rbp, rdx, 3, offset_from_rbp(&m_hash.base()[modep.immediate()][0])));
																						// mov   rdx,hash[modep+edx*8]
			a.call(ptr(rdx, rax, 3 - m_hash.l2shift()));                                // call  [rdx+rax*shift]
		}
	}
	else
	{
		// variable mode
		Gp modereg = modep.select_register(ecx);
		mov_reg_param(a, modereg, modep);                                               // mov   modereg,modep
		a.mov(rcx, ptr(rbp, modereg, 3, offset_from_rbp(m_hash.base())));               // mov   rcx,hash[modereg*8]

		// fixed PC
		if (pcp.is_immediate())
		{
			uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			a.mov(rdx, ptr(rcx, l1val * 8));                                            // mov   rdx,[rcx+l1val*8]
			a.call(ptr(rdx, l2val * 8));                                                // call  [l2val*8]
		}

		// variable PC
		else
		{
			mov_reg_param(a, eax, pcp);                                                 // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.mov(rdx, ptr(rcx, rdx, 3));                                               // mov   rdx,[rcx+rdx*8]
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and   eax,l2mask << l2shift
			a.call(ptr(rdx, rax, 3 - m_hash.l2shift()));                                // call  [rdx+rax*shift]
		}
	}

	// in all cases, if there is no code, we return here to generate the exception
	if (LOG_HASHJMPS)
		smart_call_m64(a, &m_near.debug_log_hashjmp_fail);

	mov_mem_param(a, MABS(&m_state.exp, 4), pcp);                                       // mov   [exp],param
	a.sub(rsp, 8);                                                                      // sub   rsp,8
	a.call(MABS(exp.handle().codeptr_addr()));                                          // call  [exp]
}


//-------------------------------------------------
//  op_jmp - process a JMP opcode
//-------------------------------------------------

void drcbe_x64::op_jmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	const parameter &labelp = inst.param(0);
	assert(labelp.is_code_label());

	std::string labelName = util::string_format("PC$%x", labelp.label());
	Label jmptarget = a.labelByName(labelName.c_str());
	if (!jmptarget.isValid())
		jmptarget = a.newNamedLabel(labelName.c_str());

	if (inst.condition() == uml::COND_ALWAYS)
		a.jmp(jmptarget);                                                               // jmp   target
	else
		a.j(X86_CONDITION(inst.condition()), jmptarget);                                // jcc   target
}


//-------------------------------------------------
//  op_exh - process an EXH opcode
//-------------------------------------------------

void drcbe_x64::op_exh(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());
	be_parameter exp(*this, inst.param(1), PTYPE_MRI);

	// look up the handle target
	drccodeptr *targetptr = handp.handle().codeptr_addr();

	// perform the exception processing
	Label no_exception = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), no_exception);                // jcc   no_exception
	mov_mem_param(a, MABS(&m_state.exp, 4), exp);                                       // mov   [exp],exp
	if (*targetptr != nullptr)
		a.call(imm(*targetptr));                                                        // call  *targetptr
	else
		a.call(MABS(targetptr));                                                        // call  [targetptr]
	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(no_exception);
}


//-------------------------------------------------
//  op_callh - process a CALLH opcode
//-------------------------------------------------

void drcbe_x64::op_callh(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());

	// look up the handle target
	drccodeptr *targetptr = handp.handle().codeptr_addr();

	// skip if conditional
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// jump through the handle; directly if a normal jump
	if (*targetptr != nullptr)
		a.call(imm(*targetptr));                                                        // call  *targetptr
	else
		a.call(MABS(targetptr));                                                        // call  [targetptr]

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);                                                               // skip:
}


//-------------------------------------------------
//  op_ret - process a RET opcode
//-------------------------------------------------

void drcbe_x64::op_ret(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	// skip if conditional
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// return
	a.lea(rsp, ptr(rsp, 40));                                                           // lea   rsp,[rsp+40]
	a.ret();                                                                            // ret

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);                                                               // skip:
}


//-------------------------------------------------
//  op_callc - process a CALLC opcode
//-------------------------------------------------

void drcbe_x64::op_callc(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	const parameter &funcp = inst.param(0);
	assert(funcp.is_c_function());
	be_parameter paramp(*this, inst.param(1), PTYPE_M);

	// skip if conditional
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// perform the call
	mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)paramp.memory());                        // mov   param1,paramp
	smart_call_r64(a, (x86code *)(uintptr_t)funcp.cfunc(), rax);                        // call  funcp

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);                                                               // skip:
}


//-------------------------------------------------
//  op_recover - process a RECOVER opcode
//-------------------------------------------------

void drcbe_x64::op_recover(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// call the recovery code
	a.mov(rax, MABS(&m_near.stacksave));                                                // mov   rax,stacksave
	a.mov(rax, ptr(rax, -8));                                                           // mov   rax,[rax-8]
	mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)&m_map);                                 // mov   param1,m_map
	a.lea(Gpq(REG_PARAM2), ptr(rax, -1));                                               // lea   param2,[rax-1]
	mov_r64_imm(a, Gpq(REG_PARAM3), inst.param(1).mapvar());                            // mov   param3,param[1].value
	smart_call_m64(a, &m_near.drcmap_get_value);                                        // call  drcmap_get_value
	mov_param_reg(a, dstp, eax);                                                        // mov   dstp,eax
}



/***************************************************************************
    INTERNAL REGISTER OPCODES
***************************************************************************/

//-------------------------------------------------
//  op_setfmod - process a SETFMOD opcode
//-------------------------------------------------

void drcbe_x64::op_setfmod(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);

	// immediate case
	if (srcp.is_immediate())
	{
		int value = srcp.immediate() & 3;
		a.mov(MABS(&m_state.fmod), value);                                              // mov   [fmod],srcp
		a.ldmxcsr(MABS(&m_near.ssecontrol[value]));                                     // ldmxcsr fp_control[srcp]
	}

	// register/memory case
	else
	{
		mov_reg_param(a, eax, srcp);                                                    // mov   eax,srcp
		a.and_(eax, 3);                                                                 // and   eax,3
		a.mov(MABS(&m_state.fmod), al);                                                 // mov   [fmod],al
		a.ldmxcsr(ptr(rbp, rax, 2, offset_from_rbp(&m_near.ssecontrol[0])));            // ldmxcsr fp_control[eax]
	}
}


//-------------------------------------------------
//  op_getfmod - process a GETFMOD opcode
//-------------------------------------------------

void drcbe_x64::op_getfmod(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	Mem fmod = MABS(&m_state.fmod);
	fmod.setSize(1);

	// fetch the current mode and store to the destination
	if (dstp.is_int_register())
		a.movzx(Gpd(dstp.ireg()), fmod);                                                // movzx reg,[fmod]
	else
	{
		a.movzx(eax, fmod);                                                             // movzx eax,[fmod]
		a.mov(MABS(dstp.memory()), eax);                                                // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getexp - process a GETEXP opcode
//-------------------------------------------------

void drcbe_x64::op_getexp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the exception parameter and store to the destination
	if (dstp.is_int_register())
		a.mov(Gpd(dstp.ireg()), MABS(&m_state.exp));                                    // mov   reg,[exp]
	else
	{
		a.mov(eax, MABS(&m_state.exp));                                                 // mov   eax,[exp]
		a.mov(MABS(dstp.memory()), eax);                                                // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getflgs - process a GETFLGS opcode
//-------------------------------------------------

void drcbe_x64::op_getflgs(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter maskp(*this, inst.param(1), PTYPE_I);

	// pick a target register for the general case
	Gp dstreg = dstp.select_register(eax);

	// compute mask for flags
	uint32_t flagmask = 0;
	if (maskp.immediate() & FLAG_C) flagmask |= 0x001;
	if (maskp.immediate() & FLAG_V) flagmask |= 0x800;
	if (maskp.immediate() & FLAG_Z) flagmask |= 0x040;
	if (maskp.immediate() & FLAG_S) flagmask |= 0x080;
	if (maskp.immediate() & FLAG_U) flagmask |= 0x004;

	switch (maskp.immediate())
	{
		// single flags only
		case FLAG_C:
			a.setc(al);                                                                 // setc   al
			a.movzx(dstreg, al);                                                        // movzx  dstreg,al
			break;

		case FLAG_V:
			a.seto(al);                                                                 // seto   al
			a.movzx(dstreg, al);                                                        // movzx  dstreg,al
			a.shl(dstreg, 1);                                                           // shl    dstreg,1
			break;

		case FLAG_Z:
			a.setz(al);                                                                 // setz   al
			a.movzx(dstreg, al);                                                        // movzx  dstreg,al
			a.shl(dstreg, 2);                                                           // shl    dstreg,2
			break;

		case FLAG_S:
			a.sets(al);                                                                 // sets   al
			a.movzx(dstreg, al);                                                        // movzx  dstreg,al
			a.shl(dstreg, 3);                                                           // shl    dstreg,3
			break;

		case FLAG_U:
			a.setp(al);                                                                 // setp   al
			a.movzx(dstreg, al);                                                        // movzx  dstreg,al
			a.shl(dstreg, 4);                                                           // shl    dstreg,4
			break;

		// carry plus another flag
		case FLAG_C | FLAG_V:
			a.setc(al);                                                                 // setc   al
			a.seto(cl);                                                                 // seto   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			break;

		case FLAG_C | FLAG_Z:
			a.setc(al);                                                                 // setc   al
			a.setz(cl);                                                                 // setz   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 2));                                            // lea    dstreg,[eax+ecx*4]
			break;

		case FLAG_C | FLAG_S:
			a.setc(al);                                                                 // setc   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 3));                                            // lea    dstreg,[eax+ecx*8]
			break;

		// overflow plus another flag
		case FLAG_V | FLAG_Z:
			a.seto(al);                                                                 // seto   al
			a.setz(cl);                                                                 // setz   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			a.shl(dstreg, 1);                                                           // shl    dstreg,1
			break;

		case FLAG_V | FLAG_S:
			a.seto(al);                                                                 // seto   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 2));                                            // lea    dstreg,[eax+ecx*4]
			a.shl(dstreg, 1);                                                           // shl    dstreg,1
			break;

		// zero plus another flag
		case FLAG_Z | FLAG_S:
			a.setz(al);                                                                 // setz   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,cl
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			a.shl(dstreg, 2);                                                           // shl    dstreg,2
			break;

		// default cases
		default:
			a.pushfq();                                                                 // pushf
			a.pop(eax);                                                                 // pop    eax
			a.and_(eax, flagmask);                                                      // and    eax,flagmask
			a.movzx(dstreg, byte_ptr(rbp, rax, 0, offset_from_rbp(&m_near.flagsmap[0]))); // movzx  dstreg,[flags_map]
			break;
	}

	// 32-bit form
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg

	// 64-bit form
	else if (inst.size() == 8)
		mov_param_reg(a, dstp, dstreg.r64());                                           // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_save - process a SAVE opcode
//-------------------------------------------------

void drcbe_x64::op_save(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_M);

	// copy live state to the destination
	mov_r64_imm(a, rcx, (uintptr_t)dstp.memory());                                      // mov    rcx,dstp

	// copy flags
	a.pushfq();                                                                         // pushf
	a.pop(rax);                                                                         // pop    rax
	a.and_(eax, 0x8c5);                                                                 // and    eax,0x8c5
	a.mov(al, ptr(rbp, rax, 0, offset_from_rbp(&m_near.flagsmap[0])));                  // mov    al,[flags_map]
	a.mov(ptr(rcx, offsetof(drcuml_machine_state, flags)), al);                         // mov    state->flags,al

	// copy fmod and exp
	a.mov(al, MABS(&m_state.fmod));                                                     // mov    al,[fmod]
	a.mov(ptr(rcx, offsetof(drcuml_machine_state, fmod)), al);                          // mov    state->fmod,al
	a.mov(eax, MABS(&m_state.exp));                                                     // mov    eax,[exp]
	a.mov(ptr(rcx, offsetof(drcuml_machine_state, exp)), eax);                          // mov    state->exp,eax

	// copy integer registers
	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < std::size(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			a.mov(ptr(rcx, regoffs + 8 * regnum), Gpq(regnum));
		else
		{
			a.mov(rax, MABS(&m_state.r[regnum].d));
			a.mov(ptr(rcx, regoffs + 8 * regnum), rax);
		}
	}

	// copy FP registers
	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			a.movsd(ptr(rcx, regoffs + 8 * regnum), Xmm(regnum));
		else
		{
			a.mov(rax, MABS(&m_state.f[regnum].d));
			a.mov(ptr(rcx, regoffs + 8 * regnum), rax);
		}
	}
}


//-------------------------------------------------
//  op_restore - process a RESTORE opcode
//-------------------------------------------------

void drcbe_x64::op_restore(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_M);

	// copy live state from the destination
	mov_r64_imm(a, rcx, (uintptr_t)srcp.memory());                                      // mov    rcx,dstp

	// copy integer registers
	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < std::size(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			a.mov(Gpq(regnum), ptr(rcx, regoffs + 8 * regnum));
		else
		{
			a.mov(rax, ptr(rcx, regoffs + 8 * regnum));
			a.mov(MABS(&m_state.r[regnum].d), rax);
		}
	}

	// copy FP registers
	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			a.movsd(Xmm(regnum), ptr(rcx, regoffs + 8 * regnum));
		else
		{
			a.mov(rax, ptr(rcx, regoffs + 8 * regnum));
			a.mov(MABS(&m_state.f[regnum].d), rax);
		}
	}

	Mem fmod = MABS(&m_state.fmod);
	fmod.setSize(1);

	// copy fmod and exp
	a.movzx(eax, byte_ptr(rcx, offsetof(drcuml_machine_state, fmod)));                  // movzx  eax,state->fmod
	a.and_(eax, 3);                                                                     // and    eax,3
	a.mov(MABS(&m_state.fmod), al);                                                     // mov    [fmod],al
	a.ldmxcsr(ptr(rbp, rax, 2, offset_from_rbp(&m_near.ssecontrol[0])));                // ldmxcsr fp_control[eax]
	a.mov(eax, ptr(rcx, offsetof(drcuml_machine_state, exp)));                          // mov    eax,state->exp
	a.mov(MABS(&m_state.exp), eax);                                                     // mov    [exp],eax

	// copy flags
	a.movzx(eax, byte_ptr(rcx, offsetof(drcuml_machine_state, flags)));                 // movzx  eax,state->flags
	a.push(ptr(rbp, rax, 3, offset_from_rbp(&m_near.flagsunmap[0])));                   // push   flags_unmap[eax*8]
	a.popfq();                                                                          // popf
}



/***************************************************************************
    INTEGER OPERATIONS
***************************************************************************/

//-------------------------------------------------
//  op_load - process a LOAD opcode
//-------------------------------------------------

void drcbe_x64::op_load(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	int size = scalesizep.size();

	// determine the pointer base
	int32_t baseoffs;
	Gp basereg = get_base_register_and_offset(a, basep.memory(), rdx, baseoffs);

	// pick a target register for the general case
	Gp dstreg = dstp.select_register(eax);

	// immediate index
	if (indp.is_immediate())
	{
		s32 const offset = baseoffs + (s32(indp.immediate()) << scalesizep.scale());

		if (size == SIZE_BYTE)
			a.movzx(dstreg, byte_ptr(basereg, offset));                                 // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movzx(dstreg, word_ptr(basereg, offset));                                 // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, ptr(basereg, offset));                                        // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			a.mov(dstreg.r64(), ptr(basereg, offset));                                  // mov   dstreg,[basep + scale*indp]
	}

	// other index
	else
	{
		Gp indreg = indp.select_register(rcx);
		movsx_r64_p32(a, indreg, indp);
		if (size == SIZE_BYTE)
			a.movzx(dstreg, byte_ptr(basereg, indreg, scalesizep.scale(), baseoffs));   // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movzx(dstreg, word_ptr(basereg, indreg, scalesizep.scale(), baseoffs));   // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, ptr(basereg, indreg, scalesizep.scale(), baseoffs));          // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			a.mov(dstreg.r64(), ptr(basereg, indreg, scalesizep.scale(), baseoffs));    // mov   dstreg,[basep + scale*indp]
	}

	// store result
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	else
		mov_param_reg(a, dstp, dstreg.r64());                                           // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_loads - process a LOADS opcode
//-------------------------------------------------

void drcbe_x64::op_loads(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	int size = scalesizep.size();

	// determine the pointer base
	int32_t baseoffs;
	Gp basereg = get_base_register_and_offset(a, basep.memory(), rdx, baseoffs);

	// pick a target register for the general case
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

	// immediate index
	if (indp.is_immediate())
	{
		s32 const offset = baseoffs + (s32(indp.immediate()) << scalesizep.scale());

		if (size == SIZE_BYTE)
			a.movsx(dstreg, byte_ptr(basereg, offset));                                 // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movsx(dstreg, word_ptr(basereg, offset));                                 // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD && inst.size() == 4)
			a.mov(dstreg, ptr(basereg, offset));                                        // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.movsxd(dstreg, ptr(basereg, offset));                                     // movsxd dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			a.mov(dstreg, ptr(basereg, offset));                                        // mov   dstreg,[basep + scale*indp]
	}

	// other index
	else
	{
		Gp indreg = indp.select_register(rcx);
		movsx_r64_p32(a, indreg, indp);
		if (size == SIZE_BYTE)
			a.movsx(dstreg, byte_ptr(basereg, indreg, scalesizep.scale(), baseoffs));   // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movsx(dstreg, word_ptr(basereg, indreg, scalesizep.scale(), baseoffs));   // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD && inst.size() == 4)
			a.mov(dstreg, ptr(basereg, indreg, scalesizep.scale(), baseoffs));          // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.movsxd(dstreg, ptr(basereg, indreg, scalesizep.scale(), baseoffs));       // movsxd dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			a.mov(dstreg, ptr(basereg, indreg, scalesizep.scale(), baseoffs));          // mov   dstreg,[basep + scale*indp]
	}

	// store result
	mov_param_reg(a, dstp, dstreg);                                                     // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_store - process a STORE opcode
//-------------------------------------------------

void drcbe_x64::op_store(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	int size = scalesizep.size();

	// determine the pointer base
	int32_t baseoffs;
	Gp basereg = get_base_register_and_offset(a, basep.memory(), rdx, baseoffs);

	// pick a source register for the general case
	Gp srcreg = srcp.select_register(rax);

	// degenerate case: constant index
	if (indp.is_immediate())
	{
		s32 const offset = baseoffs + (s32(indp.immediate()) << scalesizep.scale());

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_QWORD)
			{
				if (short_immediate(srcp.immediate()))
					a.mov(qword_ptr(basereg, offset), s32(srcp.immediate()));           // mov   [basep + scale*indp],srcp
				else
				{
					a.mov(ptr(basereg, offset + 0), u32(srcp.immediate() >>  0));       // mov   [basep + scale*indp],srcp
					a.mov(ptr(basereg, offset + 4), u32(srcp.immediate() >> 32));       // mov   [basep + scale*indp + 4],srcp >> 32
				}
			}
			else
				a.mov(ptr(basereg, offset, 1 << size), srcp.immediate());               // mov   [basep + scale*indp],srcp
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				mov_reg_param(a, srcreg.r32(), srcp);                                   // mov   srcreg,srcp
			else
				mov_reg_param(a, srcreg.r64(), srcp);                                   // mov   srcreg,srcp
			if (size == SIZE_BYTE)
				a.mov(ptr(basereg, offset), srcreg.r8());                               // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_WORD)
				a.mov(ptr(basereg, offset), srcreg.r16());                              // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_DWORD)
				a.mov(ptr(basereg, offset), srcreg.r32());                              // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_QWORD)
				a.mov(ptr(basereg, offset), srcreg.r64());                              // mov   [basep + scale*indp],srcreg
		}
	}

	// normal case: variable index
	else
	{
		Gp indreg = indp.select_register(rcx);
		movsx_r64_p32(a, indreg, indp);                                                 // mov   indreg,indp

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_QWORD)
			{
				if (short_immediate(srcp.immediate()))
					a.mov(qword_ptr(basereg, indreg, scalesizep.scale(), baseoffs), s32(srcp.immediate()));     // mov   [basep + scale*indp],srcp
				else
				{
					a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs + 0), u32(srcp.immediate() >>  0)); // mov   [basep + scale*ecx],srcp
					a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs + 4), u32(srcp.immediate() >> 32)); // mov   [basep + scale*ecx + 4],srcp >> 32
				}
			}
			else
				a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs, 1 << size), srcp.immediate());         // mov   [basep + scale*ecx],srcp
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				mov_reg_param(a, srcreg.r32(), srcp);                                   // mov   srcreg,srcp
			else
				mov_reg_param(a, srcreg.r64(), srcp);                                   // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs), srcreg.r8()); // mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_WORD)
				a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs), srcreg.r16());// mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_DWORD)
				a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs), srcreg.r32());// mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_QWORD)
				a.mov(ptr(basereg, indreg, scalesizep.scale(), baseoffs), srcreg.r64());// mov   [basep + scale*ecx],srcreg
		}
	}
}


//-------------------------------------------------
//  op_read - process a READ opcode
//-------------------------------------------------

void drcbe_x64::op_read(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	// pick a target register for the general case
	Gp dstreg = dstp.select_register(eax);

	// set up a call to the read handler
	auto &trampolines = m_accessors[spacesizep.space()];
	auto &resolved = m_resolved_accessors[spacesizep.space()];
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param2,addrp
	if (spacesizep.size() == SIZE_BYTE)
	{
		if (resolved.read_byte.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_byte.obj);                    // mov    param1,space
			smart_call_r64(a, resolved.read_byte.func, rax);                            // call   read_byte
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_byte);                      // call   read_byte
		}
		a.movzx(dstreg, al);                                                            // movzx  dstreg,al
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		if (resolved.read_word.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_word.obj);                    // mov    param1,space
			smart_call_r64(a, resolved.read_word.func, rax);                            // call   read_word
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_word);                      // call   read_word
		}
		a.movzx(dstreg, ax);                                                            // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (resolved.read_dword.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_dword.obj);                   // mov    param1,space
			smart_call_r64(a, resolved.read_dword.func, rax);                           // call   read_dword
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_dword);                     // call   read_dword
		}
		if (dstreg != eax || inst.size() == 8)
			a.mov(dstreg, eax);                                                         // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (resolved.read_qword.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_qword.obj);                   // mov    param1,space
			smart_call_r64(a, resolved.read_qword.func, rax);                           // call   read_qword
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_qword);                     // call   read_qword
		}
		if (dstreg != eax)
			a.mov(dstreg.r64(), rax);                                                   // mov    dstreg,rax
	}

	// store result
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	else
		mov_param_reg(a, dstp, dstreg.r64());                                           // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_readm - process a READM opcode
//-------------------------------------------------

void drcbe_x64::op_readm(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// pick a target register for the general case
	Gp dstreg = dstp.select_register(eax);

	// set up a call to the read handler
	auto &trampolines = m_accessors[spacesizep.space()];
	auto &resolved = m_resolved_accessors[spacesizep.space()];
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
		mov_reg_param(a, Gpd(REG_PARAM3), maskp);                                       // mov    param3,maskp
	else
		mov_reg_param(a, Gpq(REG_PARAM3), maskp);                                       // mov    param3,maskp
	if (spacesizep.size() == SIZE_WORD)
	{
		if (resolved.read_word_masked.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_word_masked.obj);             // mov    param1,space
			smart_call_r64(a, resolved.read_word_masked.func, rax);                     // call   read_byte_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_word_masked);               // call   read_word_masked
		}
		a.movzx(dstreg, ax);                                                            // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (resolved.read_dword_masked.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_dword_masked.obj);            // mov    param1,space
			smart_call_r64(a, resolved.read_dword_masked.func, rax);                    // call   read_dword_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_dword_masked);              // call   read_word_masked
		}
		if (dstreg != eax || inst.size() == 8)
			a.mov(dstreg, eax);                                                         // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (resolved.read_qword_masked.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.read_qword_masked.obj);            // mov    param1,space
			smart_call_r64(a, resolved.read_qword_masked.func, rax);                    // call   read_qword_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.read_qword_masked);              // call   read_word_masked
		}
		if (dstreg != eax)
			a.mov(dstreg.r64(), rax);                                                   // mov    dstreg,rax
	}

	// store result
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	else
		mov_param_reg(a, dstp, dstreg.r64());                                           // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_write - process a WRITE opcode
//-------------------------------------------------

void drcbe_x64::op_write(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	// set up a call to the write handler
	auto &trampolines = m_accessors[spacesizep.space()];
	auto &resolved = m_resolved_accessors[spacesizep.space()];
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
		mov_reg_param(a, Gpd(REG_PARAM3), srcp);                                        // mov    param3,srcp
	else
		mov_reg_param(a, Gpq(REG_PARAM3), srcp);                                        // mov    param3,srcp
	if (spacesizep.size() == SIZE_BYTE)
	{
		if (resolved.write_byte.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_byte.obj);                   // mov    param1,space
			smart_call_r64(a, resolved.write_byte.func, rax);                           // call   write_byte
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_byte);                     // call   write_byte
		}
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		if (resolved.write_word.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_word.obj);                   // mov    param1,space
			smart_call_r64(a, resolved.write_word.func, rax);                           // call   write_word
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_word);                     // call   write_word
		}
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (resolved.write_dword.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_dword.obj);                  // mov    param1,space
			smart_call_r64(a, resolved.write_dword.func, rax);                          // call   write_dword
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_dword);                    // call   write_dword
		}
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (resolved.write_qword.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_qword.obj);                  // mov    param1,space
			smart_call_r64(a, resolved.write_qword.func, rax);                          // call   write_qword
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_qword);                    // call   write_qword
		}
	}
}


//-------------------------------------------------
//  op_writem - process a WRITEM opcode
//-------------------------------------------------

void drcbe_x64::op_writem(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// set up a call to the write handler
	auto &trampolines = m_accessors[spacesizep.space()];
	auto &resolved = m_resolved_accessors[spacesizep.space()];
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
	{
		mov_reg_param(a, Gpd(REG_PARAM3), srcp);                                        // mov    param3,srcp
		mov_reg_param(a, Gpd(REG_PARAM4), maskp);                                       // mov    param4,maskp
	}
	else
	{
		mov_reg_param(a, Gpq(REG_PARAM3), srcp);                                        // mov    param3,srcp
		mov_reg_param(a, Gpq(REG_PARAM4), maskp);                                       // mov    param4,maskp
	}
	if (spacesizep.size() == SIZE_WORD)
	{
		if (resolved.write_word.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_word_masked.obj);            // mov    param1,space
			smart_call_r64(a, resolved.write_word_masked.func, rax);                    // call   write_word_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_word_masked);              // call   write_word_masked
		}
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (resolved.write_word.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_dword_masked.obj);           // mov    param1,space
			smart_call_r64(a, resolved.write_dword_masked.func, rax);                   // call   write_dword_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_dword_masked);             // call   write_dword_masked
		}
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (resolved.write_word.func)
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), resolved.write_qword_masked.obj);           // mov    param1,space
			smart_call_r64(a, resolved.write_qword_masked.func, rax);                   // call   write_qword_masked
		}
		else
		{
			mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacesizep.space()]);    // mov    param1,space
			smart_call_m64(a, (x86code **)&trampolines.write_qword_masked);             // call   write_qword_masked
		}
	}
}


//-------------------------------------------------
//  op_carry - process a CARRY opcode
//-------------------------------------------------

void drcbe_x64::op_carry(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);
	be_parameter bitp(*this, inst.param(1), PTYPE_MRI);

	Gp const src = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, srcp.ireg());

	// degenerate case: source is immediate
	if (srcp.is_immediate() && bitp.is_immediate())
	{
		if (srcp.immediate() & ((uint64_t)1 << bitp.immediate()))
			a.stc();
		else
			a.clc();
	}

	// load non-immediate bit numbers into a register
	if (!bitp.is_immediate())
	{
		mov_reg_param(a, ecx, bitp);
		a.and_(ecx, inst.size() * 8 - 1);
	}

	if (bitp.is_immediate())
	{
		if (srcp.is_memory())
			a.bt(MABS(srcp.memory(), inst.size()), bitp.immediate());                   // bt     [srcp],bitp
		else if (srcp.is_int_register())
			a.bt(src, bitp.immediate());                                                // bt     srcp,bitp
	}
	else
	{
		if (srcp.is_memory())
			a.bt(MABS(srcp.memory(), inst.size()), ecx);                                // bt     [srcp],ecx
		else if (srcp.is_int_register())
			a.bt(src, ecx);                                                             // bt     srcp,ecx
	}
}


//-------------------------------------------------
//  op_set - process a SET opcode
//-------------------------------------------------

void drcbe_x64::op_set(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// pick a target register for the general case
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

	// set to AL
	a.set(X86_CONDITION(inst.condition()), al);                                         // setcc  al
	a.movzx(dstreg.r32(), al);                                                          // movzx  dstreg,al
	mov_param_reg(a, dstp, dstreg);                                                     // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_mov - process a MOV opcode
//-------------------------------------------------

void drcbe_x64::op_mov(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// add a conditional branch unless a conditional move is possible
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS && !(dstp.is_int_register() && !srcp.is_immediate()))
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// register to memory
	if (dstp.is_memory() && srcp.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, srcp.ireg());

		a.mov(MABS(dstp.memory()), src);                                                // mov   [dstp],srcp
	}
	// immediate to memory
	else if (dstp.is_memory() && srcp.is_immediate() && short_immediate(srcp.immediate()))
		a.mov(MABS(dstp.memory(), inst.size()), s32(srcp.immediate()));                 // mov   [dstp],srcp

	// conditional memory to register
	else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_memory())
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());
		a.cmov(X86_CONDITION(inst.condition()), dst, MABS(srcp.memory()));              // cmovcc dstp,[srcp]
	}

	// conditional register to register
	else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, srcp.ireg());
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());
		a.cmov(X86_CONDITION(inst.condition()), dst, src);                              // cmovcc dstp,srcp
	}

	// general case
	else
	{
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp, true);                                           // mov   dstreg,srcp
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// resolve the jump
	if (inst.condition() != uml::COND_ALWAYS && !(dstp.is_int_register() && !srcp.is_immediate()))
		a.bind(skip);
}


//-------------------------------------------------
//  op_sext - process a SEXT opcode
//-------------------------------------------------

void drcbe_x64::op_sext(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());

	Gp dstreg = dstp.select_register(rax);

	// 32-bit form
	if (inst.size() == 4)
	{
		dstreg = dstreg.r32();

		// general case
		if (srcp.is_memory())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, MABS(srcp.memory(), 1));                                // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, MABS(srcp.memory(), 2));                                // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, MABS(srcp.memory()));                                     // mov   dstreg,[srcp]
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, GpbLo(srcp.ireg()));                                    // movsx dstreg,srcp
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, Gpw(srcp.ireg()));                                      // movsx dstreg,srcp
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, Gpd(srcp.ireg()));                                        // mov   dstreg,srcp
		}
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		if (srcp.is_memory())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, MABS(srcp.memory(), 1));                                // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, MABS(srcp.memory(), 2));                                // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_DWORD)
				a.movsxd(dstreg, MABS(srcp.memory(), 4));                               // movsxd dstreg,[srcp]
			else if (sizep.size() == SIZE_QWORD)
				a.mov(dstreg, MABS(srcp.memory()));                                     // mov   dstreg,[srcp]
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, GpbLo(srcp.ireg()));                                    // movsx dstreg,srcp
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, Gpw(srcp.ireg()));                                      // movsx dstreg,srcp
			else if (sizep.size() == SIZE_DWORD)
				a.movsxd(dstreg, Gpd(srcp.ireg()));                                     // movsxd dstreg,srcp
			else if (sizep.size() == SIZE_QWORD)
				a.mov(dstreg, Gpq(srcp.ireg()));                                        // mov   dstreg,srcp
		}
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	if (inst.flags() != 0)
		a.test(dstreg, dstreg);                                                         // test  dstreg,dstreg
}


//-------------------------------------------------
//  op_roland - process an ROLAND opcode
//-------------------------------------------------

void drcbe_x64::op_roland(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter shiftp(*this, inst.param(2), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(3), PTYPE_MRI);

	// pick a target register
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, shiftp, maskp) : dstp.select_register(rax, shiftp, maskp);

	mov_reg_param(a, dstreg, srcp);                                                     // mov   dstreg,srcp
	if (!shiftp.is_immediate_value(0))
		shift_op_param(a, Inst::kIdRol, dstreg, shiftp);                                // rol   dstreg,shiftp
	alu_op_param(a, Inst::kIdAnd, dstreg, maskp,                                        // and   dstreg,maskp
		[inst](Assembler &a, Operand const &dst, be_parameter const &src)
		{
			// optimize all-zero and all-one cases
			if (!inst.flags() && !src.immediate())
			{
				a.xor_(dst.as<Gpd>(), dst.as<Gpd>());
				return true;
			}
			else if (!inst.flags() && ones(src.immediate(), inst.size()))
				return true;

			return false;
		});
	mov_param_reg(a, dstp, dstreg);                                                     // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_rolins - process an ROLINS opcode
//-------------------------------------------------

void drcbe_x64::op_rolins(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter shiftp(*this, inst.param(2), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(3), PTYPE_MRI);

	// 32-bit form
	if (inst.size() == 4)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(ecx, shiftp, maskp);

		mov_reg_param(a, eax, srcp);                                                    // mov   eax,srcp
		if (!shiftp.is_immediate_value(0))
			shift_op_param(a, Inst::kIdRol, eax, shiftp);                               // rol   eax,shiftp
		mov_reg_param(a, dstreg, dstp);                                                 // mov   dstreg,dstp
		if (maskp.is_immediate())
		{
			a.and_(eax, maskp.immediate());                                             // and   eax,maskp
			a.and_(dstreg, ~maskp.immediate());                                         // and   dstreg,~maskp
		}
		else
		{
			mov_reg_param(a, edx, maskp);                                               // mov   edx,maskp
			a.and_(eax, edx);                                                           // and   eax,edx
			a.not_(edx);                                                                // not   edx
			a.and_(dstreg, edx);                                                        // and   dstreg,edx
		}
		a.or_(dstreg, eax);                                                             // or    dstreg,eax
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(rcx, shiftp, maskp);

		mov_reg_param(a, rax, srcp);                                                    // mov   rax,srcp
		mov_reg_param(a, rdx, maskp);                                                   // mov   rdx,maskp
		if (!shiftp.is_immediate_value(0))
			shift_op_param(a, Inst::kIdRol, rax, shiftp);                               // rol   rax,shiftp
		mov_reg_param(a, dstreg, dstp);                                                 // mov   dstreg,dstp
		a.and_(rax, rdx);                                                               // and   eax,rdx
		a.not_(rdx);                                                                    // not   rdx
		a.and_(dstreg, rdx);                                                            // and   dstreg,rdx
		a.or_(dstreg, rax);                                                             // or    dstreg,rax
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_add - process a ADD opcode
//-------------------------------------------------

void drcbe_x64::op_add(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdAdd, MABS(dstp.memory(), inst.size()), src2p,          // add   [dstp],src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});

	// dstp == src2p in memory
	else if (dstp.is_memory() && dstp == src2p)
		alu_op_param(a, Inst::kIdAdd, MABS(dstp.memory(), inst.size()), src1p,          // add   [dstp],src1p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});

	// reg = reg + imm
	else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && short_immediate(src2p.immediate()) && !inst.flags())
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());
		Gp const src1 = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, src1p.ireg());

		a.lea(dst, ptr(src1, src2p.immediate()));                                       // lea   dstp,[src1p+src2p]
	}

	// reg = reg + reg
	else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_int_register() && !inst.flags())
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());
		Gp const src1 = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, src1p.ireg());
		Gp const src2 = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, src2p.ireg());

		a.lea(dst, ptr(src1, src2));                                                    // lea   dstp,[src1p+src2p]
	}

	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p);                                                // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdAdd, dstreg, src2p,                                    // add   dstreg,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_addc - process a ADDC opcode
//-------------------------------------------------

void drcbe_x64::op_addc(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdAdc, MABS(dstp.memory(), inst.size()), src2p);         // adc   [dstp],src2p

	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p, true);                                          // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdAdc, dstreg, src2p);                                   // adc   dstreg,src2p
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_sub - process a SUB opcode
//-------------------------------------------------

void drcbe_x64::op_sub(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdSub, MABS(dstp.memory(), inst.size()), src2p,          // sub   [dstp],src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});

	// reg = reg - imm
	else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && short_immediate(src2p.immediate()) && !inst.flags())
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());
		Gp const src1 = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, src1p.ireg());

		a.lea(dst, ptr(src1, -src2p.immediate()));                                      // lea   dstp,[src1p-src2p]
	}

	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p);                                                // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdSub, dstreg, src2p,                                    // sub   dstreg,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_subc - process a SUBC opcode
//-------------------------------------------------

void drcbe_x64::op_subc(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdSbb, MABS(dstp.memory(), inst.size()), src2p);         // sbb   [dstp],src2p

	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p, true);                                          // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdSbb, dstreg, src2p);                                   // sbb   dstreg,src2p
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_cmp - process a CMP opcode
//-------------------------------------------------

void drcbe_x64::op_cmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	// memory versus anything
	if (src1p.is_memory())
		alu_op_param(a, Inst::kIdCmp, MABS(src1p.memory(), inst.size()), src2p);        // cmp   [dstp],src2p

	// general case
	else
	{
		// pick a target register for the general case
		Gp src1reg = (inst.size() == 4) ? src1p.select_register(eax) : src1p.select_register(rax);

		if (src1p.is_immediate())
		{
			if (inst.size() == 4)
				a.mov(src1reg, src1p.immediate());                                      // mov   src1reg,imm
			else
				mov_r64_imm(a, src1reg, src1p.immediate());                             // mov   src1reg,imm
		}
		alu_op_param(a, Inst::kIdCmp, src1reg, src2p);                                  // cmp   src1reg,src2p
	}
}


//-------------------------------------------------
//  op_mulu - process a MULU opcode
//-------------------------------------------------

void drcbe_x64::op_mulu(Assembler &a, const instruction &inst)
{
	uint8_t zsflags = inst.flags() & (FLAG_Z | FLAG_S);
	uint8_t vflag = inst.flags() & FLAG_V;

	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	normalize_commutative(src1p, src2p);
	bool compute_hi = (dstp != edstp);

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		mov_reg_param(a, eax, src1p);                                                   // mov   eax,src1p
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 4));                                             // mul   [src2p]
		else if (src2p.is_int_register())
			a.mul(Gpd(src2p.ireg()));                                                   // mul   src2p
		else if (src2p.is_immediate())
		{
			a.mov(edx, src2p.immediate());                                              // mov   edx,src2p
			a.mul(edx);                                                                 // mul   edx
		}
		mov_param_reg(a, dstp, eax);                                                    // mov   dstp,eax
		if (compute_hi)
			mov_param_reg(a, edstp, edx);                                               // mov   edstp,edx

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfq();                                                         // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						a.or_(edx, eax);                                                // or    edx,eax
					else if (zsflags == FLAG_S)
						a.test(edx, edx);                                               // test  edx,edx
					else
					{
						a.movzx(ecx, ax);                                               // movzx ecx,ax
						a.shr(eax, 16);                                                 // shr   eax,16
						a.or_(edx, ecx);                                                // or    edx,ecx
						a.or_(edx, eax);                                                // or    edx,eax
					}
				}
				else
					a.test(eax, eax);                                                   // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					a.pushfq();                                                         // pushf
					a.pop(rax);                                                         // pop   rax
					a.and_(qword_ptr(rsp), ~0x84);                                      // and   [rsp],~0x84
					a.or_(ptr(rsp), rax);                                               // or    [rsp],rax
					a.popfq();                                                          // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		mov_reg_param(a, rax, src1p);                                                   // mov   rax,src1p
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 8));                                             // mul   [src2p]
		else if (src2p.is_int_register())
			a.mul(Gpq(src2p.ireg()));                                                   // mul   src2p
		else if (src2p.is_immediate())
		{
			mov_r64_imm(a, rdx, src2p.immediate());                                     // mov   rdx,src2p
			a.mul(rdx);                                                                 // mul   rdx
		}
		mov_param_reg(a, dstp, rax);                                                    // mov   dstp,rax
		if (compute_hi)
			mov_param_reg(a, edstp, rdx);                                               // mov   edstp,rdx

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfq();                                                         // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						a.or_(rdx, rax);                                                // or    rdx,rax
					else if (zsflags == FLAG_S)
						a.test(rdx, rdx);                                               // test  rdx,rdx
					else
					{
						a.mov(ecx, eax);                                                // mov   ecx,eax
						a.shr(rax, 32);                                                 // shr   rax,32
						a.or_(rdx, rcx);                                                // or    rdx,rcx
						a.or_(rdx, rax);                                                // or    rdx,rax
					}
				}
				else
					a.test(rax, rax);                                                   // test  rax,rax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					a.pushfq();                                                         // pushf
					a.pop(rax);                                                         // pop   rax
					a.and_(qword_ptr(rsp), ~0x84);                                      // and   [rsp],~0x84
					a.or_(ptr(rsp), rax);                                               // or    [rsp],rax
					a.popfq();                                                          // popf
				}
			}
		}
	}
}


//-------------------------------------------------
//  op_muls - process a MULS opcode
//-------------------------------------------------

void drcbe_x64::op_muls(Assembler &a, const instruction &inst)
{
	uint8_t zsflags = inst.flags() & (FLAG_Z | FLAG_S);
	uint8_t vflag =  inst.flags() & FLAG_V;

	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	normalize_commutative(src1p, src2p);
	bool compute_hi = (dstp != edstp);

	// 32-bit form
	if (inst.size() == 4)
	{
		// 32-bit destination with memory/immediate or register/immediate
		if (!compute_hi && !src1p.is_immediate() && src2p.is_immediate())
		{
			Gp dstreg = dstp.is_int_register() ? Gpd(dstp.ireg()) : eax;
			if (src1p.is_memory())
				a.imul(dstreg, MABS(src1p.memory()), src2p.immediate());                // imul  dstreg,[src1p],src2p
			else if (src1p.is_int_register())
				a.imul(dstreg, Gpd(src1p.ireg()), src2p.immediate());                   // imul  dstreg,src1p,src2p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,eax
		}

		// 32-bit destination, general case
		else if (!compute_hi)
		{
			Gp dstreg = dstp.is_int_register() ? Gpd(dstp.ireg()) : eax;
			mov_reg_param(a, dstreg, src1p);                                            // mov   dstreg,src1p
			if (src2p.is_memory())
				a.imul(dstreg, MABS(src2p.memory()));                                   // imul  dstreg,[src2p]
			else if (src2p.is_int_register())
				a.imul(dstreg, Gpd(src2p.ireg()));                                      // imul  dstreg,src2p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,dstreg
		}

		// 64-bit destination, general case
		else
		{
			mov_reg_param(a, eax, src1p);                                               // mov   eax,src1p
			if (src2p.is_memory())
				a.imul(MABS(src2p.memory(), 4));                                        // imul  [src2p]
			else if (src2p.is_int_register())
				a.imul(Gpd(src2p.ireg()));                                              // imul  src2p
			else if (src2p.is_immediate())
			{
				a.mov(edx, src2p.immediate());                                          // mov   edx,src2p
				a.imul(edx);                                                            // imul  edx
			}
			mov_param_reg(a, dstp, eax);                                                // mov   dstp,eax
			mov_param_reg(a, edstp, edx);                                               // mov   edstp,edx
		}

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfq();                                                         // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						a.or_(edx, eax);                                                // or    edx,eax
					else if (zsflags == FLAG_S)
						a.test(edx, edx);                                               // test  edx,edx
					else
					{
						a.movzx(ecx, ax);                                               // movzx ecx,ax
						a.shr(eax, 16);                                                 // shr   eax,16
						a.or_(edx, ecx);                                                // or    edx,ecx
						a.or_(edx, eax);                                                // or    edx,eax
					}
				}
				else
					a.test(eax, eax);                                                   // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					a.pushfq();                                                         // pushf
					a.pop(rax);                                                         // pop   rax
					a.and_(qword_ptr(rsp), ~0x84);                                      // and   [rsp],~0x84
					a.or_(ptr(rsp), rax);                                               // or    [rsp],rax
					a.popfq();                                                          // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// 64-bit destination with memory/immediate or register/immediate
		if (!compute_hi && !src1p.is_immediate() && src2p.is_immediate() && short_immediate(src2p.immediate()))
		{
			Gp dstreg = dstp.is_int_register() ? Gpq(dstp.ireg()) : rax;
			if (src1p.is_memory())
				a.imul(dstreg, MABS(src1p.memory()), src2p.immediate());                // imul  dstreg,[src1p],src2p
			else if (src1p.is_int_register())
				a.imul(dstreg, Gpq(src1p.ireg()), src2p.immediate());                   // imul  rax,src1p,src2p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,rax
		}

		// 64-bit destination, general case
		else if (!compute_hi)
		{
			Gp dstreg = dstp.is_int_register() ? Gpq(dstp.ireg()) : rax;
			mov_reg_param(a, dstreg, src1p);                                            // mov   dstreg,src1p
			if (src2p.is_memory())
				a.imul(dstreg, MABS(src2p.memory()));                                   // imul  dstreg,[src2p]
			else if (src2p.is_int_register())
				a.imul(dstreg, Gpq(src2p.ireg()));                                      // imul  dstreg,src2p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,dstreg
		}

		// 128-bit destination, general case
		else
		{
			mov_reg_param(a, rax, src1p);                                               // mov   rax,src1p
			if (src2p.is_memory())
				a.imul(MABS(src2p.memory(), 8));                                        // imul  [src2p]
			else if (src2p.is_int_register())
				a.imul(Gpq(src2p.ireg()));                                              // imul  src2p
			else if (src2p.is_immediate())
			{
				mov_r64_imm(a, rdx, src2p.immediate());                                 // mov   rdx,src2p
				a.imul(rdx);                                                            // imul  rdx
			}
			mov_param_reg(a, dstp, rax);                                                // mov   dstp,rax
			mov_param_reg(a, edstp, rdx);                                               // mov   edstp,rdx
		}

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfq();                                                         // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						a.or_(rdx, rax);                                                // or    rdx,rax
					else if (zsflags == FLAG_S)
						a.test(rdx, rdx);                                               // test  rdx,rdx
					else
					{
						a.mov(ecx, eax);                                                // mov   ecx,eax
						a.shr(rax, 32);                                                 // shr   rax,32
						a.or_(rdx, rcx);                                                // or    rdx,rcx
						a.or_(rdx, rax);                                                // or    rdx,rax
					}
				}
				else
					a.test(rax, rax);                                                   // test  rax,rax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					a.pushfq();                                                         // pushf
					a.pop(rax);                                                         // pop   rax
					a.and_(qword_ptr(rsp), ~0x84);                                      // and   [rsp],~0x84
					a.or_(ptr(rsp), rax);                                               // or    [rsp],rax
					a.popfq();                                                          // popf
				}
			}
		}
	}
}


//-------------------------------------------------
//  op_divu - process a DIVU opcode
//-------------------------------------------------

void drcbe_x64::op_divu(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	bool compute_rem = (dstp != edstp);

	Label skip = a.newLabel();

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		mov_reg_param(a, ecx, src2p);                                                   // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		a.short_().jecxz(skip);                                                         // jecxz skip
		mov_reg_param(a, eax, src1p);                                                   // mov   eax,src1p
		a.xor_(edx, edx);                                                               // xor   edx,edx
		a.div(ecx);                                                                     // div   ecx
		mov_param_reg(a, dstp, eax);                                                    // mov   dstp,eax
		if (compute_rem)
			mov_param_reg(a, edstp, edx);                                               // mov   edstp,edx
		if (inst.flags() != 0)
			a.test(eax, eax);                                                           // test  eax,eax
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		mov_reg_param(a, rcx, src2p);                                                   // mov   rcx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		a.short_().jecxz(skip);                                                         // jrcxz skip
		mov_reg_param(a, rax, src1p);                                                   // mov   rax,src1p
		a.xor_(edx, edx);                                                               // xor   edx,edx
		a.div(rcx);                                                                     // div   rcx
		mov_param_reg(a, dstp, rax);                                                    // mov   dstp,rax
		if (compute_rem)
			mov_param_reg(a, edstp, rdx);                                               // mov   edstp,rdx
		if (inst.flags() != 0)
			a.test(rax, rax);                                                           // test  eax,eax
	}

	a.bind(skip);                                                                   // skip:
}


//-------------------------------------------------
//  op_divs - process a DIVS opcode
//-------------------------------------------------

void drcbe_x64::op_divs(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	bool compute_rem = (dstp != edstp);

	Label skip = a.newLabel();

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		mov_reg_param(a, ecx, src2p);                                                   // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		a.short_().jecxz(skip);                                                         // jecxz skip
		mov_reg_param(a, eax, src1p);                                                   // mov   eax,src1p
		a.cdq();                                                                        // cdq
		a.idiv(ecx);                                                                    // idiv  ecx
		mov_param_reg(a, dstp, eax);                                                    // mov   dstp,eax
		if (compute_rem)
			mov_param_reg(a, edstp, edx);                                               // mov   edstp,edx
		if (inst.flags() != 0)
			a.test(eax, eax);                                                           // test  eax,eax
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		mov_reg_param(a, rcx, src2p);                                                   // mov   rcx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		a.short_().jecxz(skip);                                                         // jrcxz skip
		mov_reg_param(a, rax, src1p);                                                   // mov   rax,src1p
		a.cqo();                                                                        // cqo
		a.idiv(rcx);                                                                    // idiv  rcx
		mov_param_reg(a, dstp, rax);                                                    // mov   dstp,rax
		if (compute_rem)
			mov_param_reg(a, edstp, rdx);                                               // mov   edstp,rdx
		if (inst.flags() != 0)
			a.test(rax, rax);                                                           // test  eax,eax
	}

	a.bind(skip);                                                                   // skip:
}


//-------------------------------------------------
//  op_and - process a AND opcode
//-------------------------------------------------

void drcbe_x64::op_and(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// pick a target register
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdAnd, MABS(dstp.memory(), inst.size()), src2p,          // and   [dstp],src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && !src.immediate())
				{
					a.mov(dst.as<Mem>(), imm(0));
					return true;
				}
				else if (!inst.flags() && ones(src.immediate(), inst.size()))
					return true;

				return false;
			});

	// dstp == src2p in memory
	else if (dstp.is_memory() && dstp == src2p)
		alu_op_param(a, Inst::kIdAnd, MABS(dstp.memory(), inst.size()), src1p,          // and   [dstp],src1p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && !src.immediate())
				{
					a.mov(dst.as<Mem>(), imm(0));
					return true;
				}
				else if (!inst.flags() && ones(src.immediate(), inst.size()))
					return true;

				return false;
			});

	// immediate 0xff
	else if (src2p.is_immediate_value(0xff) && !inst.flags())
	{
		if (src1p.is_int_register())
			a.movzx(dstreg, GpbLo(src1p.ireg()));                                       // movzx dstreg,src1p
		else if (src1p.is_memory())
			a.movzx(dstreg, MABS(src1p.memory(), 1));                                   // movzx dstreg,[src1p]
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// immediate 0xffff
	else if (src2p.is_immediate_value(0xffff) && !inst.flags())
	{
		if (src1p.is_int_register())
			a.movzx(dstreg, Gpw(src1p.ireg()));                                         // movzx dstreg,src1p
		else if (src1p.is_memory())
			a.movzx(dstreg, MABS(src1p.memory(), 2));                                   // movzx dstreg,[src1p]
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// immediate 0xffffffff
	else if (src2p.is_immediate_value(0xffffffff) && !inst.flags() && inst.size() == 8)
	{
		if (dstp.is_int_register() && src1p == dstp)
			a.mov(dstreg.r32(), dstreg.r32());                                          // mov   dstreg,dstreg
		else
		{
			mov_reg_param(a, dstreg.r32(), src1p);                                      // mov   dstreg,src1p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,dstreg
		}
	}

	// general case
	else
	{
		mov_reg_param(a, dstreg, src1p);                                                // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdAnd, dstreg, src2p,                                    // and   dstreg,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && !src.immediate())
				{
					a.xor_(dst.as<Gpd>(), dst.as<Gpd>());
					return true;
				}
				else if (!inst.flags() && ones(src.immediate(), inst.size()))
					return true;

				return false;
			});
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_test - process a TEST opcode
//-------------------------------------------------

void drcbe_x64::op_test(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// src1p in memory
	if (src1p.is_memory())
		alu_op_param(a, Inst::kIdTest, MABS(src1p.memory(), inst.size()), src2p);       // test  [src1p],src2p

	// general case
	else
	{
		// pick a target register for the general case
		Gp src1reg = (inst.size() == 4) ? src1p.select_register(eax) : src1p.select_register(rax);

		mov_reg_param(a, src1reg, src1p);                                               // mov   src1reg,src1p
		alu_op_param(a, Inst::kIdTest, src1reg, src2p);                                 // test  src1reg,src2p
	}
}


//-------------------------------------------------
//  op_or - process a OR opcode
//-------------------------------------------------

void drcbe_x64::op_or(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdOr, MABS(dstp.memory(), inst.size()), src2p,           // or    [dstp],src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.mov(dst.as<Mem>(), imm(-1));
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});

	// dstp == src2p in memory
	else if (dstp.is_memory() && dstp == src2p)
		alu_op_param(a, Inst::kIdOr, MABS(dstp.memory(), inst.size()), src1p,           // or    [dstp],src1p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.mov(dst.as<Mem>(), imm(-1));
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});

	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p);                                                // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdOr, dstreg, src2p,                                     // or    dstreg,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.mov(dst.as<Gp>(), imm(-1));
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_xor - process a XOR opcode
//-------------------------------------------------

void drcbe_x64::op_xor(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		alu_op_param(a, Inst::kIdXor, MABS(dstp.memory(), inst.size()), src2p,          // xor   [dstp],src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.not_(dst.as<Mem>());
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});

	// dstp == src2p in memory
	else if (dstp.is_memory() && dstp == src2p)
		alu_op_param(a, Inst::kIdXor, MABS(dstp.memory(), inst.size()), src1p,          // xor   [dstp],src1p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.not_(dst.as<Mem>());
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});

	// dstp == src1p register
	else if (dstp.is_int_register() && dstp == src1p)
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());

		alu_op_param(a, Inst::kIdXor, dst, src2p,                                       // xor   dstp,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.not_(dst.as<Gp>());
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});
	}
	// general case
	else
	{
		// pick a target register for the general case
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		mov_reg_param(a, dstreg, src1p);                                                // mov   dstreg,src1p
		alu_op_param(a, Inst::kIdXor, dstreg, src2p,                                    // xor   dstreg,src2p
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && ones(src.immediate(), inst.size()))
				{
					a.not_(dst.as<Gp>());
					return true;
				}
				else if (!inst.flags() && !src.immediate())
					return true;

				return false;
			});
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_lzcnt - process a LZCNT opcode
//-------------------------------------------------

void drcbe_x64::op_lzcnt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// 32-bit form
	if (inst.size() == 4)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(eax);

		mov_reg_param(a, dstreg, srcp);                                                 // mov   dstreg,src1p
		a.mov(ecx, 32 ^ 31);                                                            // mov   ecx,32 ^ 31
		a.bsr(dstreg, dstreg);                                                          // bsr   dstreg,dstreg
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		a.xor_(dstreg, 31);                                                             // xor   dstreg,31
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp);                                                 // mov   dstreg,src1p
		a.mov(ecx, 64 ^ 63);                                                            // mov   ecx,64 ^ 63
		a.bsr(dstreg, dstreg);                                                          // bsr   dstreg,dstreg
		a.cmovz(dstreg, rcx);                                                           // cmovz dstreg,rcx
		a.xor_(dstreg, 63);                                                             // xor   dstreg,63
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_tzcnt - process a TZCNT opcode
//-------------------------------------------------

void drcbe_x64::op_tzcnt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// 32-bit form
	if (inst.size() == 4)
	{
		Gp dstreg = dstp.select_register(eax);

		mov_reg_param(a, dstreg, srcp);                                                 // mov   dstreg,srcp
		a.mov(ecx, 32);                                                                 // mov   ecx,32
		a.bsf(dstreg, dstreg);                                                          // bsf   dstreg,dstreg
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp);                                                 // mov   dstreg,srcp
		a.mov(ecx, 64);                                                                 // mov   ecx,64
		a.bsf(dstreg, dstreg);                                                          // bsf   dstreg,dstreg
		a.cmovz(dstreg, rcx);                                                           // cmovz dstreg,rcx
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_bswap - process a BSWAP opcode
//-------------------------------------------------

void drcbe_x64::op_bswap(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

	mov_reg_param(a, dstreg, srcp);                                                     // mov   dstreg,src1p
	a.bswap(dstreg);                                                                    // bswap dstreg
	if (inst.flags() != 0)
		a.test(dstreg, dstreg);                                                         // test  dstreg,dstreg
	mov_param_reg(a, dstp, dstreg);                                                     // mov   dstp,dstreg
}

template <Inst::Id Opcode> void drcbe_x64::op_shift(Assembler &a, const uml::instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const bool carry = (Opcode == Inst::kIdRcl) || (Opcode == Inst::kIdRcr);

	// optimize immediate zero case
	if (carry || inst.flags() || !src2p.is_immediate_value(0))
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Opcode, MABS(dstp.memory(), inst.size()), src2p);         // op   [dstp],src2p

		// general case
		else
		{
			// pick a target register
			Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

			if (carry)
				mov_reg_param(a, dstreg, src1p, true);                                  // mov   dstreg,src1p
			else
				mov_reg_param(a, dstreg, src1p);                                        // mov   dstreg,src1p
			shift_op_param(a, Opcode, dstreg, src2p);                                   // op    dstreg,src2p
			mov_param_reg(a, dstp, dstreg);                                             // mov   dstp,dstreg
		}
	}
}


/***************************************************************************
    FLOATING POINT OPERATIONS
***************************************************************************/

//-------------------------------------------------
//  op_fload - process a FLOAD opcode
//-------------------------------------------------

void drcbe_x64::op_fload(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// determine the pointer base
	int32_t baseoffs;
	Gp basereg = get_base_register_and_offset(a, basep.memory(), rdx, baseoffs);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (indp.is_immediate())
			a.movss(dstreg, ptr(basereg, baseoffs + 4*indp.immediate()));               // movss  dstreg,[basep + 4*indp]
		else
		{
			Gp indreg = indp.select_register(ecx);
			mov_reg_param(a, indreg, indp);                                             // mov    indreg,indp
			a.movss(dstreg, ptr(basereg, indreg, 2, baseoffs));                         // movss  dstreg,[basep + 4*indp]
		}
		movss_p32_r128(a, dstp, dstreg);                                                // movss  dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (indp.is_immediate())
			a.movsd(dstreg, ptr(basereg, baseoffs + 8*indp.immediate()));               // movsd  dstreg,[basep + 8*indp]
		else
		{
			Gp indreg = indp.select_register(ecx);
			mov_reg_param(a, indreg, indp);                                             // mov    indreg,indp
			a.movsd(dstreg, ptr(basereg, indreg, 3, baseoffs));                         // movsd  dstreg,[basep + 8*indp]
		}
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd  dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fstore - process a FSTORE opcode
//-------------------------------------------------

void drcbe_x64::op_fstore(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MF);

	// pick a target register for the general case
	Xmm srcreg = srcp.select_register(xmm0);

	// determine the pointer base
	int32_t baseoffs;
	Gp basereg = get_base_register_and_offset(a, basep.memory(), rdx, baseoffs);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, srcreg, srcp);                                                // movss  srcreg,srcp
		if (indp.is_immediate())
			a.movss(ptr(basereg, baseoffs + 4*indp.immediate()), srcreg);               // movss  [basep + 4*indp],srcreg
		else
		{
			Gp indreg = indp.select_register(ecx);
			mov_reg_param(a, indreg, indp);                                             // mov    indreg,indp
			a.movss(ptr(basereg, indreg, 2, baseoffs), srcreg);                         // movss  [basep + 4*indp],srcreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, srcreg, srcp);                                                // movsd  srcreg,srcp
		if (indp.is_immediate())
			a.movsd(ptr(basereg, baseoffs + 8*indp.immediate()), srcreg);               // movsd  [basep + 8*indp],srcreg
		else
		{
			Gp indreg = indp.select_register(ecx);
			mov_reg_param(a, indreg, indp);                                             // mov    indreg,indp
			a.movsd(ptr(basereg, indreg, 3, baseoffs), srcreg);                         // movsd  [basep + 8*indp],srcreg
		}
	}
}


//-------------------------------------------------
//  op_fread - process a FREAD opcode
//-------------------------------------------------

void drcbe_x64::op_fread(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacep = inst.param(2);
	assert(spacep.is_size_space());
	assert((1 << spacep.size()) == inst.size());

	// set up a call to the read dword/qword handler
	mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacep.space()]);                // mov    param1,space
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param2,addrp
	if (inst.size() == 4)
		smart_call_m64(a, (x86code **)&m_accessors[spacep.space()].read_dword);         // call   read_dword
	else if (inst.size() == 8)
		smart_call_m64(a, (x86code **)&m_accessors[spacep.space()].read_qword);         // call   read_qword

	// store result
	if (inst.size() == 4)
	{
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory()), eax);                                            // mov   [dstp],eax
		else if (dstp.is_float_register())
			a.movd(Xmm(dstp.freg()), eax);                                              // movd  dstp,eax
	}
	else if (inst.size() == 8)
	{
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory()), rax);                                            // mov   [dstp],rax
		else if (dstp.is_float_register())
			a.movq(Xmm(dstp.freg()), rax);                                              // movq  dstp,rax
	}
}


//-------------------------------------------------
//  op_fwrite - process a FWRITE opcode
//-------------------------------------------------

void drcbe_x64::op_fwrite(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &spacep = inst.param(2);
	assert(spacep.is_size_space());
	assert((1 << spacep.size()) == inst.size());

	// general case
	mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)m_space[spacep.space()]);                // mov    param1,space
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);                                           // mov    param21,addrp

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
			a.mov(Gpd(REG_PARAM3), MABS(srcp.memory()));                                // mov    param3,[srcp]
		else if (srcp.is_float_register())
			a.movd(Gpd(REG_PARAM3), Xmm(srcp.freg()));                                  // movd   param3,srcp
		smart_call_m64(a, (x86code **)&m_accessors[spacep.space()].write_dword);        // call   write_dword
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
			a.mov(Gpq(REG_PARAM3), MABS(srcp.memory()));                                // mov    param3,[srcp]
		else if (srcp.is_float_register())
			a.movq(Gpq(REG_PARAM3), Xmm(srcp.freg()));                                  // movq   param3,srcp
		smart_call_m64(a, (x86code **)&m_accessors[spacep.space()].write_qword);        // call   write_qword
	}
}


//-------------------------------------------------
//  op_fmov - process a FMOV opcode
//-------------------------------------------------

void drcbe_x64::op_fmov(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// always start with a jmp
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_float_register())
		{
			movss_p32_r128(a, dstp, Xmm(srcp.freg()));                                  // movss dstp,srcp
		}
		else
		{
			movss_r128_p32(a, dstreg, srcp);                                            // movss dstreg,srcp
			movss_p32_r128(a, dstp, dstreg);                                            // movss dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_float_register())
		{
			movsd_p64_r128(a, dstp, Xmm(srcp.freg()));                                  // movsd dstp,srcp
		}
		else
		{
			movsd_r128_p64(a, dstreg, srcp);                                            // movsd dstreg,srcp
			movsd_p64_r128(a, dstp, dstreg);                                            // movsd dstp,dstreg
		}
	}

	// resolve the jump
	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);                                                               // skip:
}


//-------------------------------------------------
//  op_ftoint - process a FTOINT opcode
//-------------------------------------------------

void drcbe_x64::op_ftoint(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());
	const parameter &roundp = inst.param(3);
	assert(roundp.is_rounding());

	// pick a target register for the general case
	Gp dstreg = (sizep.size() == SIZE_DWORD) ? dstp.select_register(eax) : dstp.select_register(rax);

	// set rounding mode if necessary
	if (roundp.rounding() != ROUND_DEFAULT && roundp.rounding() != ROUND_TRUNC)
	{
		a.stmxcsr(MABS(&m_near.ssemodesave));                                           // stmxcsr [ssemodesave]
		a.ldmxcsr(MABS(&m_near.ssecontrol[roundp.rounding()]));                         // ldmxcsr fpcontrol[mode]
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
		{
			if (roundp.rounding() != ROUND_TRUNC)
				a.cvtss2si(dstreg, MABS(srcp.memory()));                                // cvtss2si dstreg,[srcp]
			else
				a.cvttss2si(dstreg, MABS(srcp.memory()));                               // cvttss2si dstreg,[srcp]
		}
		else if (srcp.is_float_register())
		{
			if (roundp.rounding() != ROUND_TRUNC)
				a.cvtss2si(dstreg, Xmm(srcp.freg()));                                   // cvtss2si dstreg,srcp
			else
				a.cvttss2si(dstreg, Xmm(srcp.freg()));                                  // cvttss2si dstreg,srcp
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			if (roundp.rounding() != ROUND_TRUNC)
				a.cvtsd2si(dstreg, MABS(srcp.memory()));                                // cvtsd2si dstreg,[srcp]
			else
				a.cvttsd2si(dstreg, MABS(srcp.memory()));                               // cvttsd2si dstreg,[srcp]
		}
		else if (srcp.is_float_register())
		{
			if (roundp.rounding() != ROUND_TRUNC)
				a.cvtsd2si(dstreg, Xmm(srcp.freg()));                                   // cvtsd2si dstreg,srcp
			else
				a.cvttsd2si(dstreg, Xmm(srcp.freg()));                                  // cvttsd2si dstreg,srcp
		}
	}

	mov_param_reg(a, dstp, dstreg);                                                     // mov   dstp,dstreg

	// restore rounding mode
	if (roundp.rounding() != ROUND_DEFAULT && roundp.rounding() != ROUND_TRUNC)
		a.ldmxcsr(MABS(&m_near.ssemodesave));                                           // ldmxcsr [ssemodesave]
}


//-------------------------------------------------
//  op_ffrint - process a FFRINT opcode
//-------------------------------------------------

void drcbe_x64::op_ffrint(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
				a.cvtsi2ss(dstreg, MABS(srcp.memory(), 4));                             // cvtsi2ss dstreg,[srcp]
			else
			{
				Gp srcreg = srcp.select_register(eax);
				mov_reg_param(a, srcreg, srcp);                                         // mov      srcreg,srcp
				a.cvtsi2ss(dstreg, srcreg);                                             // cvtsi2ss dstreg,srcreg
			}
		}

		// 64-bit integer source
		else
		{
			if (srcp.is_memory())
				a.cvtsi2ss(dstreg, MABS(srcp.memory(), 8));                             // cvtsi2ss dstreg,[srcp]
			else
			{
				Gp srcreg = srcp.select_register(rax);
				mov_reg_param(a, srcreg, srcp);                                         // mov      srcreg,srcp
				a.cvtsi2ss(dstreg, srcreg);                                             // cvtsi2ss dstreg,srcreg
			}
		}
		movss_p32_r128(a, dstp, dstreg);                                                // movss    dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
				a.cvtsi2sd(dstreg, MABS(srcp.memory(), 4));                             // cvtsi2sd dstreg,[srcp]
			else
			{
				Gp srcreg = srcp.select_register(eax);
				mov_reg_param(a, srcreg, srcp);                                         // mov      srcreg,srcp
				a.cvtsi2sd(dstreg, srcreg);                                             // cvtsi2sd dstreg,srcreg
			}
		}

		// 64-bit integer source
		else
		{
			if (srcp.is_memory())
				a.cvtsi2sd(dstreg, MABS(srcp.memory(), 8));                             // cvtsi2sd dstreg,[srcp]
			else
			{
				Gp srcreg = srcp.select_register(rax);
				mov_reg_param(a, srcreg, srcp);                                         // mov      srcreg,srcp
				a.cvtsi2sd(dstreg, srcreg);                                             // cvtsi2sd dstreg,srcreg
			}
		}
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd    dstp,dstreg
	}
}


//-------------------------------------------------
//  op_ffrflt - process a FFRFLT opcode
//-------------------------------------------------

void drcbe_x64::op_ffrflt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// single-to-double
	if (inst.size() == 8 && sizep.size() == SIZE_DWORD)
	{
		if (srcp.is_memory())
			a.cvtss2sd(dstreg, MABS(srcp.memory()));                                    // cvtss2sd dstreg,[srcp]
		else if (srcp.is_float_register())
			a.cvtss2sd(dstreg, Xmm(srcp.freg()));                                       // cvtss2sd dstreg,srcp
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd    dstp,dstreg
	}

	// double-to-single
	else if (inst.size() == 4 && sizep.size() == SIZE_QWORD)
	{
		if (srcp.is_memory())
			a.cvtsd2ss(dstreg, MABS(srcp.memory()));                                    // cvtsd2ss dstreg,[srcp]
		else if (srcp.is_float_register())
			a.cvtsd2ss(dstreg, Xmm(srcp.freg()));                                       // cvtsd2ss dstreg,srcp
		movss_p32_r128(a, dstp, dstreg);                                                // movss    dstp,dstreg
	}
}


//-------------------------------------------------
//  op_frnds - process a FRNDS opcode
//-------------------------------------------------

void drcbe_x64::op_frnds(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 64-bit form
	if (srcp.is_memory())
		a.cvtsd2ss(dstreg, MABS(srcp.memory(), 8));                                     // cvtsd2ss dstreg,[srcp]
	else if (srcp.is_float_register())
		a.cvtsd2ss(dstreg, Xmm(srcp.freg()));                                           // cvtsd2ss dstreg,srcp
	a.cvtss2sd(dstreg, dstreg);                                                         // cvtss2sd dstreg,dstreg
	movsd_p64_r128(a, dstp, dstreg);                                                    // movsd    dstp,dstreg
}


//-------------------------------------------------
//  op_fadd - process a FADD opcode
//-------------------------------------------------

void drcbe_x64::op_fadd(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);
	normalize_commutative(src1p, src2p);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, dstreg, src1p);                                               // movss dstreg,src1p
		if (src2p.is_memory())
			a.addss(dstreg, MABS(src2p.memory()));                                      // addss dstreg,[src2p]
		else if (src2p.is_float_register())
			a.addss(dstreg, Xmm(src2p.freg()));                                         // addss dstreg,src2p
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, dstreg, src1p);                                               // movsd dstreg,src1p
		if (src2p.is_memory())
			a.addsd(dstreg, MABS(src2p.memory()));                                      // addsd dstreg,[src2p]
		else if (src2p.is_float_register())
			a.addsd(dstreg, Xmm(src2p.freg()));                                         // addsd dstreg,src2p
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fsub - process a FSUB opcode
//-------------------------------------------------

void drcbe_x64::op_fsub(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, dstreg, src1p);                                               // movss dstreg,src1p
		if (src2p.is_memory())
			a.subss(dstreg, MABS(src2p.memory()));                                      // subss dstreg,[src2p]
		else if (src2p.is_float_register())
			a.subss(dstreg, Xmm(src2p.freg()));                                         // subss dstreg,src2p
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, dstreg, src1p);                                               // movsd dstreg,src1p
		if (src2p.is_memory())
			a.subsd(dstreg, MABS(src2p.memory()));                                      // subsd dstreg,[src2p]
		else if (src2p.is_float_register())
			a.subsd(dstreg, Xmm(src2p.freg()));                                         // subsd dstreg,src2p
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fcmp - process a FCMP opcode
//-------------------------------------------------

void drcbe_x64::op_fcmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_U);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MF);
	be_parameter src2p(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm src1reg = src1p.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, src1reg, src1p);                                              // movss src1reg,src1p
		if (src2p.is_memory())
			a.comiss(src1reg, MABS(src2p.memory()));                                    // comiss src1reg,[src2p]
		else if (src2p.is_float_register())
			a.comiss(src1reg, Xmm(src2p.freg()));                                       // comiss src1reg,src2p
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, src1reg, src1p);                                              // movsd src1reg,src1p
		if (src2p.is_memory())
			a.comisd(src1reg, MABS(src2p.memory()));                                    // comisd src1reg,[src2p]
		else if (src2p.is_float_register())
			a.comisd(src1reg, Xmm(src2p.freg()));                                       // comisd src1reg,src2p
	}
}


//-------------------------------------------------
//  op_fmul - process a FMUL opcode
//-------------------------------------------------

void drcbe_x64::op_fmul(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);
	normalize_commutative(src1p, src2p);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, dstreg, src1p);                                               // movss dstreg,src1p
		if (src2p.is_memory())
			a.mulss(dstreg, MABS(src2p.memory()));                                      // mulss dstreg,[src2p]
		else if (src2p.is_float_register())
			a.mulss(dstreg, Xmm(src2p.freg()));                                         // mulss dstreg,src2p
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, dstreg, src1p);                                               // movsd dstreg,src1p
		if (src2p.is_memory())
			a.mulsd(dstreg, MABS(src2p.memory()));                                      // mulsd dstreg,[src2p]
		else if (src2p.is_float_register())
			a.mulsd(dstreg, Xmm(src2p.freg()));                                         // mulsd dstreg,src2p
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fdiv - process a FDIV opcode
//-------------------------------------------------

void drcbe_x64::op_fdiv(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, dstreg, src1p);                                               // movss dstreg,src1p
		if (src2p.is_memory())
			a.divss(dstreg, MABS(src2p.memory()));                                      // divss dstreg,[src2p]
		else if (src2p.is_float_register())
			a.divss(dstreg, Xmm(src2p.freg()));                                         // divss dstreg,src2p
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, dstreg, src1p);                                               // movsd dstreg,src1p
		if (src2p.is_memory())
			a.divsd(dstreg, MABS(src2p.memory()));                                      // divsd dstreg,[src2p]
		else if (src2p.is_float_register())
			a.divsd(dstreg, Xmm(src2p.freg()));                                         // divsd dstreg,src2p
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fneg - process a FNEG opcode
//-------------------------------------------------

void drcbe_x64::op_fneg(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, srcp);

	// 32-bit form
	if (inst.size() == 4)
	{
		a.xorps(dstreg, dstreg);                                                        // xorps dstreg,dstreg
		if (srcp.is_memory())
			a.subss(dstreg, MABS(srcp.memory()));                                       // subss dstreg,[srcp]
		else if (srcp.is_float_register())
			a.subss(dstreg, Xmm(srcp.freg()));                                          // subss dstreg,srcp
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		a.xorpd(dstreg, dstreg);                                                        // xorpd dstreg,dstreg
		if (srcp.is_memory())
			a.subsd(dstreg, MABS(srcp.memory()));                                       // subsd dstreg,[srcp]
		else if (srcp.is_float_register())
			a.subsd(dstreg, Xmm(srcp.freg()));                                          // subsd dstreg,srcp
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fabs - process a FABS opcode
//-------------------------------------------------

void drcbe_x64::op_fabs(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0, srcp);

	// 32-bit form
	if (inst.size() == 4)
	{
		movss_r128_p32(a, dstreg, srcp);                                                // movss dstreg,srcp
		a.andps(dstreg, MABS(m_absmask32));                                             // andps dstreg,[absmask32]
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		movsd_r128_p64(a, dstreg, srcp);                                                // movsd dstreg,srcp
		a.andpd(dstreg, MABS(m_absmask64));                                             // andpd dstreg,[absmask64]
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fsqrt - process a FSQRT opcode
//-------------------------------------------------

void drcbe_x64::op_fsqrt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
			a.sqrtss(dstreg, MABS(srcp.memory()));                                      // sqrtss dstreg,[srcp]
		else if (srcp.is_float_register())
			a.sqrtss(dstreg, Xmm(srcp.freg()));                                         // sqrtss dstreg,srcp
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
			a.sqrtsd(dstreg, MABS(srcp.memory()));                                      // sqrtsd dstreg,[srcp]
		else if (srcp.is_float_register())
			a.sqrtsd(dstreg, Xmm(srcp.freg()));                                         // sqrtsd dstreg,srcp
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_frecip - process a FRECIP opcode
//-------------------------------------------------

void drcbe_x64::op_frecip(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (USE_RCPSS_FOR_SINGLES)
		{
			if (srcp.is_memory())
				a.rcpss(dstreg, MABS(srcp.memory()));                                   // rcpss dstreg,[srcp]
			else if (srcp.is_float_register())
				a.rcpss(dstreg, Xmm(srcp.freg()));                                      // rcpss dstreg,srcp
			movss_p32_r128(a, dstp, dstreg);                                            // movss dstp,dstreg
		}
		else
		{
			a.movss(xmm1, MABS(&m_near.single1));                                       // movss xmm1,1.0
			if (srcp.is_memory())
				a.divss(xmm1, MABS(srcp.memory()));                                     // divss xmm1,[srcp]
			else if (srcp.is_float_register())
				a.divss(xmm1, Xmm(srcp.freg()));                                        // divss xmm1,srcp
			movss_p32_r128(a, dstp, xmm1);                                              // movss dstp,xmm1
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (USE_RCPSS_FOR_DOUBLES)
		{
			if (srcp.is_memory())
				a.cvtsd2ss(dstreg, MABS(srcp.memory()));                                // cvtsd2ss dstreg,[srcp]
			else if (srcp.is_float_register())
				a.cvtsd2ss(dstreg, Xmm(srcp.freg()));                                   // cvtsd2ss dstreg,srcp
			a.rcpss(dstreg, dstreg);                                                    // rcpss dstreg,dstreg
			a.cvtss2sd(dstreg, dstreg);                                                 // cvtss2sd dstreg,dstreg
			movsd_p64_r128(a, dstp, dstreg);                                            // movsd dstp,dstreg
		}
		else
		{
			a.movsd(xmm1, MABS(&m_near.double1));                                       // movsd xmm1,1.0
			if (srcp.is_memory())
				a.divsd(xmm1, MABS(srcp.memory()));                                     // divsd xmm1,[srcp]
			else if (srcp.is_float_register())
				a.divsd(xmm1, Xmm(srcp.freg()));                                        // divsd xmm1,srcp
			movsd_p64_r128(a, dstp, xmm1);                                              // movsd dstp,xmm1
		}
	}
}


//-------------------------------------------------
//  op_frsqrt - process a FRSQRT opcode
//-------------------------------------------------

void drcbe_x64::op_frsqrt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (USE_RSQRTSS_FOR_SINGLES)
		{
			if (srcp.is_memory())
				a.rsqrtss(dstreg, MABS(srcp.memory()));                                 // rsqrtss dstreg,[srcp]
			else if (srcp.is_float_register())
				a.rsqrtss(dstreg, Xmm(srcp.freg()));                                    // rsqrtss dstreg,srcp
		}
		else
		{
			if (srcp.is_memory())
				a.sqrtss(xmm1, MABS(srcp.memory()));                                    // sqrtss xmm1,[srcp]
			else if (srcp.is_float_register())
				a.sqrtss(xmm1, Xmm(srcp.freg()));                                       // sqrtss xmm1,srcp
			a.movss(dstreg, MABS(&m_near.single1));                                     // movss dstreg,1.0
			a.divss(dstreg, xmm1);                                                      // divss dstreg,xmm1
		}
		movss_p32_r128(a, dstp, dstreg);                                                // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (USE_RSQRTSS_FOR_DOUBLES)
		{
			if (srcp.is_memory())
				a.cvtsd2ss(dstreg, MABS(srcp.memory()));                                // cvtsd2ss dstreg,[srcp]
			else if (srcp.is_float_register())
				a.cvtsd2ss(dstreg, Xmm(srcp.freg()));                                   // cvtsd2ss dstreg,srcp
			a.rsqrtss(dstreg, dstreg);                                                  // rsqrtss dstreg,dstreg
			a.cvtss2sd(dstreg, dstreg);                                                 // cvtss2sd dstreg,dstreg
		}
		else
		{
			if (srcp.is_memory())
				a.sqrtsd(xmm1, MABS(srcp.memory()));                                    // sqrtsd xmm1,[srcp]
			else if (srcp.is_float_register())
				a.sqrtsd(xmm1, Xmm(srcp.freg()));                                       // sqrtsd xmm1,srcp
			a.movsd(dstreg, MABS(&m_near.double1));                                     // movsd dstreg,1.0
			a.divsd(dstreg, xmm1);                                                      // divsd dstreg,xmm1
		}
		movsd_p64_r128(a, dstp, dstreg);                                                // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fcopyi - process a FCOPYI opcode
//-------------------------------------------------

void drcbe_x64::op_fcopyi(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MR);

	// pick a target register for the general case
	Xmm dstreg = dstp.select_register(xmm0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
		{
			a.movd(dstreg, MABS(srcp.memory()));                                        // movd     dstreg,[srcp]
			movss_p32_r128(a, dstp, dstreg);                                            // movss    dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				mov_param_reg(a, dstp, Gpd(srcp.ireg()));                               // mov      dstp,srcp
			}
			else
			{
				a.movd(dstreg, Gpd(srcp.ireg()));                                       // movd     dstreg,srcp
				movss_p32_r128(a, dstp, dstreg);                                        // movss    dstp,dstreg
			}
		}

	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			a.movq(dstreg, MABS(srcp.memory()));                                        // movq     dstreg,[srcp]
			movsd_p64_r128(a, dstp, dstreg);                                            // movsd    dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				mov_param_reg(a, dstp, Gpq(srcp.ireg()));                               // mov      dstp,srcp
			}
			else
			{
				a.movq(dstreg, Gpq(srcp.ireg()));                                       // movq     dstreg,srcp
				movsd_p64_r128(a, dstp, dstreg);                                        // movsd    dstp,dstreg
			}
		}

	}
}


//-------------------------------------------------
//  op_icopyf - process a ICOPYF opcode
//-------------------------------------------------

void drcbe_x64::op_icopyf(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
		{
			Gp dstreg = dstp.select_register(eax);
			a.mov(dstreg, MABS(srcp.memory()));                                         // mov      dstreg,[srcp]
			mov_param_reg(a, dstp, dstreg);                                             // mov      dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				a.movd(MABS(dstp.memory()), Xmm(srcp.freg()));                          // movd     dstp,srcp
			}
			else
			{
				a.movd(Gpd(dstp.ireg()), Xmm(srcp.freg()));                             // movd     dstp,srcp
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			Gp dstreg = dstp.select_register(rax);
			a.mov(dstreg, MABS(srcp.memory()));                                         // mov      dstreg,[srcp]
			mov_param_reg(a, dstp, dstreg);                                             // mov      dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				a.movq(MABS(dstp.memory()), Xmm(srcp.freg()));                          // movq     dstp,srcp
			}
			else
			{
				a.movq(Gpq(dstp.ireg()), Xmm(srcp.freg()));                             // movq     dstp,srcp
			}
		}
	}
}

} // namespace drc
