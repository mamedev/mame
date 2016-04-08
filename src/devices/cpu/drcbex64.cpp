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

#include <stddef.h>
#include "emu.h"
#include "debugger.h"
#include "emuopts.h"
#include "drcuml.h"
#include "drcbex64.h"

using namespace uml;
using namespace x64emit;



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

const UINT32 PTYPE_M    = 1 << parameter::PTYPE_MEMORY;
const UINT32 PTYPE_I    = 1 << parameter::PTYPE_IMMEDIATE;
const UINT32 PTYPE_R    = 1 << parameter::PTYPE_INT_REGISTER;
const UINT32 PTYPE_F    = 1 << parameter::PTYPE_FLOAT_REGISTER;
//const UINT32 PTYPE_MI   = PTYPE_M | PTYPE_I;
//const UINT32 PTYPE_RI   = PTYPE_R | PTYPE_I;
const UINT32 PTYPE_MR   = PTYPE_M | PTYPE_R;
const UINT32 PTYPE_MRI  = PTYPE_M | PTYPE_R | PTYPE_I;
const UINT32 PTYPE_MF   = PTYPE_M | PTYPE_F;

#ifdef X64_WINDOWS_ABI

const int REG_PARAM1    = REG_RCX;
const int REG_PARAM2    = REG_RDX;
const int REG_PARAM3    = REG_R8;
const int REG_PARAM4    = REG_R9;

#else

const int REG_PARAM1    = REG_RDI;
const int REG_PARAM2    = REG_RSI;
const int REG_PARAM3    = REG_RDX;
const int REG_PARAM4    = REG_RCX;

#endif



//**************************************************************************
//  MACROS
//**************************************************************************

#define X86_CONDITION(condition)        (condition_map[condition - uml::COND_Z])
#define X86_NOT_CONDITION(condition)    (condition_map[condition - uml::COND_Z] ^ 1)

inline x86_memref drcbe_x64::MABS(const void *ptr)
{
	return MBD(REG_BP, offset_from_rbp(ptr));
}

#define assert_no_condition(inst)       assert((inst).condition() == uml::COND_ALWAYS)
#define assert_any_condition(inst)      assert((inst).condition() == uml::COND_ALWAYS || ((inst).condition() >= uml::COND_Z && (inst).condition() < uml::COND_MAX))
#define assert_no_flags(inst)           assert((inst).flags() == 0)
#define assert_flags(inst, valid)       assert(((inst).flags() & ~(valid)) == 0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

drcbe_x64::opcode_generate_func drcbe_x64::s_opcode_table[OP_MAX];

// size-to-mask table
//static const UINT64 size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, U64(0xffffffffffffffff) };

// register mapping tables
static const UINT8 int_register_map[REG_I_COUNT] =
{
#ifdef X64_WINDOWS_ABI
	REG_RBX, REG_RSI, REG_RDI, REG_R12, REG_R13, REG_R14, REG_R15
#else
	REG_RBX, REG_R12, REG_R13, REG_R14, REG_R15
#endif
};

static UINT8 float_register_map[REG_F_COUNT] =
{
	0
};

// condition mapping table
static const UINT8 condition_map[uml::COND_MAX - uml::COND_Z] =
{
	x64emit::COND_Z,    // COND_Z = 0x80,    requires Z
	x64emit::COND_NZ,   // COND_NZ,          requires Z
	x64emit::COND_S,    // COND_S,           requires S
	x64emit::COND_NS,   // COND_NS,          requires S
	x64emit::COND_C,    // COND_C,           requires C
	x64emit::COND_NC,   // COND_NC,          requires C
	x64emit::COND_O,    // COND_V,           requires V
	x64emit::COND_NO,   // COND_NV,          requires V
	x64emit::COND_P,    // COND_U,           requires U
	x64emit::COND_NP,   // COND_NU,          requires U
	x64emit::COND_A,    // COND_A,           requires CZ
	x64emit::COND_BE,   // COND_BE,          requires CZ
	x64emit::COND_G,    // COND_G,           requires SVZ
	x64emit::COND_LE,   // COND_LE,          requires SVZ
	x64emit::COND_L,    // COND_L,           requires SV
	x64emit::COND_GE,   // COND_GE,          requires SV
};

#if 0
// rounding mode mapping table
static const UINT8 fprnd_map[4] =
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
	{ uml::OP_BSWAP,   &drcbe_x64::op_bswap },      // BSWAP   dst,src
	{ uml::OP_SHL,     &drcbe_x64::op_shl },        // SHL     dst,src,count[,f]
	{ uml::OP_SHR,     &drcbe_x64::op_shr },        // SHR     dst,src,count[,f]
	{ uml::OP_SAR,     &drcbe_x64::op_sar },        // SAR     dst,src,count[,f]
	{ uml::OP_ROL,     &drcbe_x64::op_rol },        // ROL     dst,src,count[,f]
	{ uml::OP_ROLC,    &drcbe_x64::op_rolc },       // ROLC    dst,src,count[,f]
	{ uml::OP_ROR,     &drcbe_x64::op_ror },        // ROR     dst,src,count[,f]
	{ uml::OP_RORC,    &drcbe_x64::op_rorc },       // RORC    dst,src,count[,f]

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



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  param_normalize - convert a full parameter
//  into a reduced set
//-------------------------------------------------

drcbe_x64::be_parameter::be_parameter(drcbe_x64 &drcbe, const parameter &param, UINT32 allowed)
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

inline int drcbe_x64::be_parameter::select_register(int defreg) const
{
	if (m_type == PTYPE_INT_REGISTER || m_type == PTYPE_FLOAT_REGISTER || m_type == PTYPE_VECTOR_REGISTER)
		return m_value;
	return defreg;
}

inline int drcbe_x64::be_parameter::select_register(int defreg, const be_parameter &checkparam) const
{
	if (*this == checkparam)
		return defreg;
	return select_register(defreg);
}

inline int drcbe_x64::be_parameter::select_register(int defreg, const be_parameter &checkparam, const be_parameter &checkparam2) const
{
	if (*this == checkparam || *this == checkparam2)
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

inline INT32 drcbe_x64::offset_from_rbp(const void *ptr)
{
	INT64 delta = reinterpret_cast<UINT8 *>(const_cast<void *>(ptr)) - m_rbpvalue;
	assert_always((INT32)delta == delta, "offset_from_rbp: delta out of range");
	return (INT32)delta;
}


//-------------------------------------------------
//  get_base_register_and_offset - determine right
//  base register and offset to access the given
//  target address
//-------------------------------------------------

inline int drcbe_x64::get_base_register_and_offset(x86code *&dst, void *target, UINT8 reg, INT32 &offset)
{
	INT64 delta = (UINT8 *)target - m_rbpvalue;
	if (short_immediate(delta))
	{
		offset = delta;
		return REG_RBP;
	}
	else
	{
		offset = 0;
		emit_mov_r64_imm(dst, reg, (FPTR)target);                                       // mov   reg,target
		return reg;
	}
}


//-------------------------------------------------
//  emit_smart_call_r64 - generate a call either
//  directly or via a call through pointer
//-------------------------------------------------

inline void drcbe_x64::emit_smart_call_r64(x86code *&dst, x86code *target, UINT8 reg)
{
	INT64 delta = target - (dst + 5);
	if (short_immediate(delta))
		emit_call(dst, target);                                                         // call  target
	else
	{
		emit_mov_r64_imm(dst, reg, (FPTR)target);                                       // mov   reg,target
		emit_call_r64(dst, reg);                                                        // call  reg
	}
}


//-------------------------------------------------
//  emit_smart_call_m64 - generate a call either
//  directly or via a call through pointer
//-------------------------------------------------

inline void drcbe_x64::emit_smart_call_m64(x86code *&dst, x86code **target)
{
	INT64 delta = *target - (dst + 5);
	if (short_immediate(delta))
		emit_call(dst, *target);                                                        // call  *target
	else
		emit_call_m64(dst, MABS(target));                                               // call  [target]
}



//**************************************************************************
//  BACKEND CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  drcbe_x64 - constructor
//-------------------------------------------------

drcbe_x64::drcbe_x64(drcuml_state &drcuml, device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device),
		m_hash(cache, modes, addrbits, ignorebits),
		m_map(cache, 0),
		m_labels(cache),
		m_log(nullptr),
		m_sse41(false),
		m_absmask32((UINT32 *)cache.alloc_near(16*2 + 15)),
		m_absmask64(nullptr),
		m_rbpvalue(cache.near() + 0x80),
		m_entry(nullptr),
		m_exit(nullptr),
		m_nocode(nullptr),
		m_fixup_label(FUNC(drcbe_x64::fixup_label), this),
		m_fixup_exception(FUNC(drcbe_x64::fixup_exception), this),
		m_near(*(near_state *)cache.alloc_near(sizeof(m_near)))
{
	// build up necessary arrays
	static const UINT32 sse_control[4] =
	{
		0xffc0,     // ROUND_TRUNC
		0x9fc0,     // ROUND_ROUND
		0xdfc0,     // ROUND_CEIL
		0xbfc0      // ROUND_FLOOR
	};
	memcpy(m_near.ssecontrol, sse_control, sizeof(m_near.ssecontrol));
	m_near.single1 = 1.0f;
	m_near.double1 = 1.0;

	// create absolute value masks that are aligned to SSE boundaries
	m_absmask32 = (UINT32 *)(((FPTR)m_absmask32 + 15) & ~15);
	m_absmask32[0] = m_absmask32[1] = m_absmask32[2] = m_absmask32[3] = 0x7fffffff;
	m_absmask64 = (UINT64 *)&m_absmask32[4];
	m_absmask64[0] = m_absmask64[1] = U64(0x7fffffffffffffff);

	// get pointers to C functions we need to call
	m_near.debug_cpu_instruction_hook = (x86code *)debugger_instruction_hook;
	if (LOG_HASHJMPS)
	{
		m_near.debug_log_hashjmp = (x86code *)debug_log_hashjmp;
		m_near.debug_log_hashjmp_fail = (x86code *)debug_log_hashjmp_fail;
	}
	m_near.drcmap_get_value = (x86code *)&drc_map_variables::static_get_value;

	// build the flags map
	for (int entry = 0; entry < ARRAY_LENGTH(m_near.flagsmap); entry++)
	{
		UINT8 flags = 0;
		if (entry & 0x001) flags |= FLAG_C;
		if (entry & 0x004) flags |= FLAG_U;
		if (entry & 0x040) flags |= FLAG_Z;
		if (entry & 0x080) flags |= FLAG_S;
		if (entry & 0x800) flags |= FLAG_V;
		m_near.flagsmap[entry] = flags;
	}
	for (int entry = 0; entry < ARRAY_LENGTH(m_near.flagsunmap); entry++)
	{
		UINT64 flags = 0;
		if (entry & FLAG_C) flags |= 0x001;
		if (entry & FLAG_U) flags |= 0x004;
		if (entry & FLAG_Z) flags |= 0x040;
		if (entry & FLAG_S) flags |= 0x080;
		if (entry & FLAG_V) flags |= 0x800;
		m_near.flagsunmap[entry] = flags;
	}

	// build the opcode table (static but it doesn't hurt to regenerate it)
	for (auto & elem : s_opcode_table_source)
		s_opcode_table[elem.opcode] = elem.func;

	// create the log
	if (device.machine().options().drc_log_native())
	{
		std::string filename = std::string("drcbex64_").append(device.shortname()).append(".asm");
		m_log = x86log_create_context(filename.c_str());
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
	drccodeptr *cachetop = m_cache.begin_codegen(500);
	if (cachetop == nullptr)
		fatalerror("Out of cache space after a reset!\n");

	x86code *dst = (x86code *)*cachetop;

	// generate a simple CPUID stub
	UINT32 (*cpuid_ecx_stub)(void) = (UINT32 (*)(void))dst;
	emit_push_r64(dst, REG_RBX);                                                        // push  rbx
	emit_mov_r32_imm(dst, REG_EAX, 1);                                                  // mov   eax,1
	emit_cpuid(dst);                                                                    // cpuid
	emit_mov_r32_r32(dst, REG_EAX, REG_ECX);                                            // mov   eax,ecx
	emit_pop_r64(dst, REG_RBX);                                                         // pop   rbx
	emit_ret(dst);                                                                      // ret

	// call it to determine if we have SSE4.1 support
	m_sse41 = (((*cpuid_ecx_stub)() & 0x80000) != 0);

	// generate an entry point
	m_entry = (x86_entry_point_func)dst;
	emit_push_r64(dst, REG_RBX);                                                        // push  rbx
	emit_push_r64(dst, REG_RSI);                                                        // push  rsi
	emit_push_r64(dst, REG_RDI);                                                        // push  rdi
	emit_push_r64(dst, REG_RBP);                                                        // push  rbp
	emit_push_r64(dst, REG_R12);                                                        // push  r12
	emit_push_r64(dst, REG_R13);                                                        // push  r13
	emit_push_r64(dst, REG_R14);                                                        // push  r14
	emit_push_r64(dst, REG_R15);                                                        // push  r15
	emit_mov_r64_r64(dst, REG_RBP, REG_PARAM1);                                         // mov   rbp,param1
	emit_sub_r64_imm(dst, REG_RSP, 32);                                                 // sub   rsp,32
	emit_mov_m64_r64(dst, MABS(&m_near.hashstacksave), REG_RSP);                        // mov   [hashstacksave],rsp
	emit_sub_r64_imm(dst, REG_RSP, 8);                                                  // sub   rsp,8
	emit_mov_m64_r64(dst, MABS(&m_near.stacksave), REG_RSP);                            // mov   [stacksave],rsp
	emit_stmxcsr_m32(dst, MABS(&m_near.ssemode));                                       // stmxcsr [ssemode]
	emit_jmp_r64(dst, REG_PARAM2);                                                      // jmp   param2
	if (m_log != nullptr)
		x86log_disasm_code_range(m_log, "entry_point", (x86code *)m_entry, dst);

	// generate an exit point
	m_exit = dst;
	emit_ldmxcsr_m32(dst, MABS(&m_near.ssemode));                                       // ldmxcsr [ssemode]
	emit_mov_r64_m64(dst, REG_RSP, MABS(&m_near.hashstacksave));                        // mov   rsp,[hashstacksave]
	emit_add_r64_imm(dst, REG_RSP, 32);                                                 // add   rsp,32
	emit_pop_r64(dst, REG_R15);                                                         // pop   r15
	emit_pop_r64(dst, REG_R14);                                                         // pop   r14
	emit_pop_r64(dst, REG_R13);                                                         // pop   r13
	emit_pop_r64(dst, REG_R12);                                                         // pop   r12
	emit_pop_r64(dst, REG_RBP);                                                         // pop   rbp
	emit_pop_r64(dst, REG_RDI);                                                         // pop   rdi
	emit_pop_r64(dst, REG_RSI);                                                         // pop   rsi
	emit_pop_r64(dst, REG_RBX);                                                         // pop   rbx
	emit_ret(dst);                                                                      // ret
	if (m_log != nullptr)
		x86log_disasm_code_range(m_log, "exit_point", m_exit, dst);

	// generate a no code point
	m_nocode = dst;
	emit_ret(dst);                                                                      // ret
	if (m_log != nullptr)
		x86log_disasm_code_range(m_log, "nocode", m_nocode, dst);

	// finish up codegen
	*cachetop = (drccodeptr)dst;
	m_cache.end_codegen();

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
	return (*m_entry)(m_rbpvalue, (x86code *)entry.codeptr());
}


//-------------------------------------------------
//  drcbex64_generate - generate code
//-------------------------------------------------

void drcbe_x64::generate(drcuml_block &block, const instruction *instlist, UINT32 numinst)
{
	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_labels.block_begin(block);
	m_map.block_begin(block);

	// begin codegen; fail if we can't
	drccodeptr *cachetop = m_cache.begin_codegen(numinst * 8 * 4);
	if (cachetop == nullptr)
		block.abort();

	// compute the base by aligning the cache top to a cache line (assumed to be 64 bytes)
	x86code *base = (x86code *)(((FPTR)*cachetop + 63) & ~63);
	x86code *dst = base;

	// generate code
	const char *blockname = nullptr;
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		assert(inst.opcode() < ARRAY_LENGTH(s_opcode_table));

		// add a comment
		if (m_log != nullptr)
		{
			std::string dasm = inst.disasm(&m_drcuml);
			x86log_add_comment(m_log, dst, "%s", dasm.c_str());
		}

		// extract a blockname
		if (blockname == nullptr)
		{
			if (inst.opcode() == OP_HANDLE)
				blockname = inst.param(0).handle().string();
			else if (inst.opcode() == OP_HASH)
				blockname = string_format("Code: mode=%d PC=%08X", (UINT32)inst.param(0).immediate(), (offs_t)inst.param(1).immediate()).c_str();
		}

		// generate code
		(this->*s_opcode_table[inst.opcode()])(dst, inst);
	}

	// complete codegen
	*cachetop = (drccodeptr)dst;
	m_cache.end_codegen();

	// log it
	if (m_log != nullptr)
		x86log_disasm_code_range(m_log, (blockname == nullptr) ? "Unknown block" : blockname, base, m_cache.top());

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_labels.block_end(block);
	m_map.block_end(block);
}


//-------------------------------------------------
//  hash_exists - return true if the given mode/pc
//  exists in the hash table
//-------------------------------------------------

bool drcbe_x64::hash_exists(UINT32 mode, UINT32 pc)
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



/***************************************************************************
    EMITTERS FOR 32-BIT OPERATIONS WITH PARAMETERS
***************************************************************************/

//-------------------------------------------------
//  emit_mov_r32_p32 - move a 32-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::emit_mov_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else
			emit_mov_r32_imm(dst, reg, param.immediate());                              // mov   reg,param
	}
	else if (param.is_memory())
		emit_mov_r32_m32(dst, reg, MABS(param.memory()));                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r32_r32(dst, reg, param.ireg());                                   // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_movsx_r64_p32 - move a 32-bit parameter
//  sign-extended into a register
//-------------------------------------------------

void drcbe_x64::emit_movsx_r64_p32(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else if ((INT32)param.immediate() >= 0)
			emit_mov_r32_imm(dst, reg, param.immediate());                              // mov   reg,param
		else
			emit_mov_r64_imm(dst, reg, (INT32)param.immediate());                       // mov   reg,param
	}
	else if (param.is_memory())
		emit_movsxd_r64_m32(dst, reg, MABS(param.memory()));                            // movsxd reg,[param]
	else if (param.is_int_register())
		emit_movsxd_r64_r32(dst, reg, param.ireg());                                    // movsdx reg,param
}


//-------------------------------------------------
//  emit_mov_r32_p32_keepflags - move a 32-bit
//  parameter into a register without affecting
//  any flags
//-------------------------------------------------

void drcbe_x64::emit_mov_r32_p32_keepflags(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
		emit_mov_r32_imm(dst, reg, param.immediate());                                  // mov   reg,param
	else if (param.is_memory())
		emit_mov_r32_m32(dst, reg, MABS(param.memory()));                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r32_r32(dst, reg, param.ireg());                                   // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_m32_p32 - move a 32-bit parameter
//  into a memory location
//-------------------------------------------------

void drcbe_x64::emit_mov_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param)
{
	if (param.is_immediate())
		emit_mov_m32_imm(dst, memref, param.immediate());                           // mov   [mem],param
	else if (param.is_memory())
	{
		emit_mov_r32_m32(dst, REG_EAX, MABS(param.memory()));                           // mov   eax,[param]
		emit_mov_m32_r32(dst, memref, REG_EAX);                                     // mov   [mem],eax
	}
	else if (param.is_int_register())
		emit_mov_m32_r32(dst, memref, param.ireg());                                    // mov   [mem],param
}


//-------------------------------------------------
//  emit_mov_p32_r32 - move a register into a
//  32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_mov_p32_r32(x86code *&dst, const be_parameter &param, UINT8 reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_mov_m32_r32(dst, MABS(param.memory()), reg);                               // mov   [param],reg
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r32_r32(dst, param.ireg(), reg);                                   // mov   param,reg
	}
}


//-------------------------------------------------
//  emit_add_r32_p32 - add operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_add_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_add_r32_imm(dst, reg, param.immediate());                              // add   reg,param
	}
	else if (param.is_memory())
		emit_add_r32_m32(dst, reg, MABS(param.memory()));                               // add   reg,[param]
	else if (param.is_int_register())
		emit_add_r32_r32(dst, reg, param.ireg());                                       // add   reg,param
}


//-------------------------------------------------
//  emit_add_m32_p32 - add operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_add_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_add_m32_imm(dst, memref, param.immediate());                       // add   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_add_m32_r32(dst, memref, reg);                                         // add   [dest],reg
	}
}


//-------------------------------------------------
//  emit_adc_r32_p32 - adc operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_adc_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_adc_r32_imm(dst, reg, param.immediate());                                  // adc   reg,param
	else if (param.is_memory())
		emit_adc_r32_m32(dst, reg, MABS(param.memory()));                               // adc   reg,[param]
	else if (param.is_int_register())
		emit_adc_r32_r32(dst, reg, param.ireg());                                       // adc   reg,param
}


//-------------------------------------------------
//  emit_adc_m32_p32 - adc operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_adc_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_adc_m32_imm(dst, memref, param.immediate());                           // adc   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32_keepflags(dst, reg, param);                                    // mov   reg,param
		emit_adc_m32_r32(dst, memref, reg);                                         // adc   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sub_r32_p32 - sub operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sub_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_sub_r32_imm(dst, reg, param.immediate());                              // sub   reg,param
	}
	else if (param.is_memory())
		emit_sub_r32_m32(dst, reg, MABS(param.memory()));                               // sub   reg,[param]
	else if (param.is_int_register())
		emit_sub_r32_r32(dst, reg, param.ireg());                                       // sub   reg,param
}


//-------------------------------------------------
//  emit_sub_m32_p32 - sub operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sub_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_sub_m32_imm(dst, memref, param.immediate());                       // sub   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_sub_m32_r32(dst, memref, reg);                                         // sub   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sbb_r32_p32 - sbb operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sbb_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_sbb_r32_imm(dst, reg, param.immediate());                                  // sbb   reg,param
	else if (param.is_memory())
		emit_sbb_r32_m32(dst, reg, MABS(param.memory()));                               // sbb   reg,[param]
	else if (param.is_int_register())
		emit_sbb_r32_r32(dst, reg, param.ireg());                                       // sbb   reg,param
}


//-------------------------------------------------
//  emit_sbb_m32_p32 - sbb operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sbb_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_sbb_m32_imm(dst, memref, param.immediate());                           // sbb   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32_keepflags(dst, reg, param);                                    // mov   reg,param
		emit_sbb_m32_r32(dst, memref, reg);                                         // sbb   [dest],reg
	}
}


//-------------------------------------------------
//  emit_cmp_r32_p32 - cmp operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_cmp_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_cmp_r32_imm(dst, reg, param.immediate());                                  // cmp   reg,param
	else if (param.is_memory())
		emit_cmp_r32_m32(dst, reg, MABS(param.memory()));                               // cmp   reg,[param]
	else if (param.is_int_register())
		emit_cmp_r32_r32(dst, reg, param.ireg());                                       // cmp   reg,param
}


//-------------------------------------------------
//  emit_cmp_m32_p32 - cmp operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_cmp_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_cmp_m32_imm(dst, memref, param.immediate());                           // cmp   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_cmp_m32_r32(dst, memref, reg);                                         // cmp   [dest],reg
	}
}


//-------------------------------------------------
//  emit_and_r32_p32 - and operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_and_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else
			emit_and_r32_imm(dst, reg, param.immediate());                              // and   reg,param
	}
	else if (param.is_memory())
		emit_and_r32_m32(dst, reg, MABS(param.memory()));                               // and   reg,[param]
	else if (param.is_int_register())
		emit_and_r32_r32(dst, reg, param.ireg());                                       // and   reg,param
}


//-------------------------------------------------
//  emit_and_m32_p32 - and operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_and_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_mov_m32_imm(dst, memref, 0);                                       // mov   [dest],0
		else
			emit_and_m32_imm(dst, memref, param.immediate());                       // and   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_and_m32_r32(dst, memref, reg);                                         // and   [dest],reg
	}
}


//-------------------------------------------------
//  emit_test_r32_p32 - test operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_test_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_test_r32_imm(dst, reg, param.immediate());                                 // test   reg,param
	else if (param.is_memory())
		emit_test_m32_r32(dst, MABS(param.memory()), reg);                              // test   [param],reg
	else if (param.is_int_register())
		emit_test_r32_r32(dst, reg, param.ireg());                                      // test   reg,param
}


//-------------------------------------------------
//  emit_test_m32_p32 - test operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_test_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_test_m32_imm(dst, memref, param.immediate());                          // test  [dest],param
	else if (param.is_memory())
	{
		emit_mov_r32_p32(dst, REG_EAX, param);                                          // mov   reg,param
		emit_test_m32_r32(dst, memref, REG_EAX);                                        // test  [dest],reg
	}
	else if (param.is_int_register())
		emit_test_m32_r32(dst, memref, param.ireg());                               // test  [dest],param
}


//-------------------------------------------------
//  emit_or_r32_p32 - or operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_or_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_r32_imm(dst, reg, 0xffffffff);                                     // mov  reg,-1
		else
			emit_or_r32_imm(dst, reg, param.immediate());                               // or   reg,param
	}
	else if (param.is_memory())
		emit_or_r32_m32(dst, reg, MABS(param.memory()));                                // or   reg,[param]
	else if (param.is_int_register())
		emit_or_r32_r32(dst, reg, param.ireg());                                        // or   reg,param
}


//-------------------------------------------------
//  emit_or_m32_p32 - or operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_or_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_m32_imm(dst, memref, 0xffffffff);                                  // mov   [dest],-1
		else
			emit_or_m32_imm(dst, memref, param.immediate());                            // or   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_or_m32_r32(dst, memref, reg);                                          // or   [dest],reg
	}
}


//-------------------------------------------------
//  emit_xor_r32_p32 - xor operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_xor_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_r32(dst, reg);                                                     // not   reg
		else
			emit_xor_r32_imm(dst, reg, param.immediate());                              // xor   reg,param
	}
	else if (param.is_memory())
		emit_xor_r32_m32(dst, reg, MABS(param.memory()));                               // xor   reg,[param]
	else if (param.is_int_register())
		emit_xor_r32_r32(dst, reg, param.ireg());                                       // xor   reg,param
}


//-------------------------------------------------
//  emit_xor_m32_p32 - xor operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_xor_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_m32(dst, memref);                                              // not   [dest]
		else
			emit_xor_m32_imm(dst, memref, param.immediate());                       // xor   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                              // mov   reg,param
		emit_xor_m32_r32(dst, memref, reg);                                         // xor   [dest],reg
	}
}


//-------------------------------------------------
//  emit_shl_r32_p32 - shl operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shl_r32_imm(dst, reg, param.immediate());                              // shl   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_shl_r32_cl(dst, reg);                                                      // shl   reg,cl
	}
}


//-------------------------------------------------
//  emit_shl_m32_p32 - shl operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shl_m32_imm(dst, memref, param.immediate());                       // shl   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_shl_m32_cl(dst, memref);                                               // shl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_shr_r32_p32 - shr operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shr_r32_imm(dst, reg, param.immediate());                              // shr   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_shr_r32_cl(dst, reg);                                                      // shr   reg,cl
	}
}


//-------------------------------------------------
//  emit_shr_m32_p32 - shr operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shr_m32_imm(dst, memref, param.immediate());                       // shr   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_shr_m32_cl(dst, memref);                                               // shr   [dest],cl
	}
}


//-------------------------------------------------
//  emit_sar_r32_p32 - sar operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sar_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_sar_r32_imm(dst, reg, param.immediate());                              // sar   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_sar_r32_cl(dst, reg);                                                      // sar   reg,cl
	}
}


//-------------------------------------------------
//  emit_sar_m32_p32 - sar operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sar_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_sar_m32_imm(dst, memref, param.immediate());                       // sar   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_sar_m32_cl(dst, memref);                                               // sar   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rol_r32_p32 - rol operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rol_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rol_r32_imm(dst, reg, param.immediate());                              // rol   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_rol_r32_cl(dst, reg);                                                      // rol   reg,cl
	}
}


//-------------------------------------------------
//  emit_rol_m32_p32 - rol operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rol_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rol_m32_imm(dst, memref, param.immediate());                       // rol   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_rol_m32_cl(dst, memref);                                               // rol   [dest],cl
	}
}


//-------------------------------------------------
//  emit_ror_r32_p32 - ror operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_ror_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_ror_r32_imm(dst, reg, param.immediate());                              // ror   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_ror_r32_cl(dst, reg);                                                      // ror   reg,cl
	}
}


//-------------------------------------------------
//  emit_ror_m32_p32 - ror operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_ror_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_ror_m32_imm(dst, memref, param.immediate());                       // ror   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                          // mov   ecx,param
		emit_ror_m32_cl(dst, memref);                                               // ror   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcl_r32_p32 - rcl operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcl_r32_imm(dst, reg, param.immediate());                              // rcl   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
		emit_rcl_r32_cl(dst, reg);                                                      // rcl   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcl_m32_p32 - rcl operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcl_m32_imm(dst, memref, param.immediate());                       // rcl   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
		emit_rcl_m32_cl(dst, memref);                                               // rcl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcr_r32_p32 - rcr operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcr_r32_imm(dst, reg, param.immediate());                              // rcr   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
		emit_rcr_r32_cl(dst, reg);                                                      // rcr   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcr_m32_p32 - rcr operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcr_m32_imm(dst, memref, param.immediate());                       // rcr   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
		emit_rcr_m32_cl(dst, memref);                                               // rcr   [dest],cl
	}
}



/***************************************************************************
    EMITTERS FOR 64-BIT OPERATIONS WITH PARAMETERS
***************************************************************************/

//-------------------------------------------------
//  emit_mov_r64_p64 - move a 64-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::emit_mov_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else
			emit_mov_r64_imm(dst, reg, param.immediate());                              // mov   reg,param
	}
	else if (param.is_memory())
		emit_mov_r64_m64(dst, reg, MABS(param.memory()));                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r64_r64(dst, reg, param.ireg());                                   // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_r64_p64_keepflags - move a 64-bit
//  parameter into a register without affecting
//  any flags
//-------------------------------------------------

void drcbe_x64::emit_mov_r64_p64_keepflags(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
		emit_mov_r64_imm(dst, reg, param.immediate());                                  // mov   reg,param
	else if (param.is_memory())
		emit_mov_r64_m64(dst, reg, MABS(param.memory()));                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r64_r64(dst, reg, param.ireg());                                   // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_p64_r64 - move a registers into a
//  64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_mov_p64_r64(x86code *&dst, const be_parameter &param, UINT8 reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_mov_m64_r64(dst, MABS(param.memory()), reg);                               // mov   [param],reg
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r64_r64(dst, param.ireg(), reg);                                   // mov   param,reg
	}
}


//-------------------------------------------------
//  emit_add_r64_p64 - add operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_add_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_add_r64_imm(dst, reg, param.immediate());                          // add   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_add_r64_r64(dst, reg, REG_R11);                                    // add   reg,r11
			}
		}
	}
	else if (param.is_memory())
		emit_add_r64_m64(dst, reg, MABS(param.memory()));                               // add   reg,[param]
	else if (param.is_int_register())
		emit_add_r64_r64(dst, reg, param.ireg());                                       // add   reg,param
}


//-------------------------------------------------
//  emit_add_m64_p64 - add operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_add_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_add_m64_imm(dst, memref, param.immediate());                   // add   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_add_m64_r64(dst, memref, REG_R11);                             // add   [mem],r11
			}
		}
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_add_m64_r64(dst, memref, reg);                                         // add   [dest],reg
	}
}


//-------------------------------------------------
//  emit_adc_r64_p64 - adc operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_adc_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (short_immediate(param.immediate()))
			emit_adc_r64_imm(dst, reg, param.immediate());                              // adc   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param.immediate());                          // mov   r11,param
			emit_adc_r64_r64(dst, reg, REG_R11);                                        // adc   reg,r11
		}
	}
	else if (param.is_memory())
		emit_adc_r64_m64(dst, reg, MABS(param.memory()));                               // adc   reg,[param]
	else if (param.is_int_register())
		emit_adc_r64_r64(dst, reg, param.ireg());                                       // adc   reg,param
}


//-------------------------------------------------
//  emit_adc_m64_p64 - adc operation to a 64-bit
//  memory locaiton from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_adc_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate() && short_immediate(param.immediate()))
		emit_adc_m64_imm(dst, memref, param.immediate());                           // adc   [mem],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64_keepflags(dst, reg, param);                                // mov   reg,param
		emit_adc_m64_r64(dst, memref, reg);                                         // adc   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sub_r64_p64 - sub operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sub_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_sub_r64_imm(dst, reg, param.immediate());                          // sub   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_sub_r64_r64(dst, reg, REG_R11);                                    // sub   reg,r11
			}
		}
	}
	else if (param.is_memory())
		emit_sub_r64_m64(dst, reg, MABS(param.memory()));                               // sub   reg,[param]
	else if (param.is_int_register())
		emit_sub_r64_r64(dst, reg, param.ireg());                                       // sub   reg,param
}


//-------------------------------------------------
//  emit_sub_m64_p64 - sub operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sub_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_sub_m64_imm(dst, memref, param.immediate());                   // sub   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_sub_m64_r64(dst, memref, REG_R11);                             // sub   [mem],r11
			}
		}
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_sub_m64_r64(dst, memref, reg);                                         // sub   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sbb_r64_p64 - sbb operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sbb_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (short_immediate(param.immediate()))
			emit_sbb_r64_imm(dst, reg, param.immediate());                              // sbb   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param.immediate());                          // mov   r11,param
			emit_sbb_r64_r64(dst, reg, REG_R11);                                        // sbb   reg,r11
		}
	}
	else if (param.is_memory())
		emit_sbb_r64_m64(dst, reg, MABS(param.memory()));                               // sbb   reg,[param]
	else if (param.is_int_register())
		emit_sbb_r64_r64(dst, reg, param.ireg());                                       // sbb   reg,param
}


//-------------------------------------------------
//  emit_sbb_m64_p64 - sbb operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sbb_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate() && short_immediate(param.immediate()))
		emit_sbb_m64_imm(dst, memref, param.immediate());                           // sbb   [mem],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64_keepflags(dst, reg, param);                                    // mov   reg,param
		emit_sbb_m64_r64(dst, memref, reg);                                         // sbb   [dest],reg
	}
}


//-------------------------------------------------
//  emit_cmp_r64_p64 - cmp operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_cmp_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (short_immediate(param.immediate()))
			emit_cmp_r64_imm(dst, reg, param.immediate());                              // cmp   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param.immediate());                          // mov   r11,param
			emit_cmp_r64_r64(dst, reg, REG_R11);                                        // cmp   reg,r11
		}
	}
	else if (param.is_memory())
		emit_cmp_r64_m64(dst, reg, MABS(param.memory()));                               // cmp   reg,[param]
	else if (param.is_int_register())
		emit_cmp_r64_r64(dst, reg, param.ireg());                                       // cmp   reg,param
}


//-------------------------------------------------
//  emit_cmp_m64_p64 - cmp operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_cmp_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate() && short_immediate(param.immediate()))
		emit_cmp_m64_imm(dst, memref, param.immediate());                           // cmp   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_cmp_m64_r64(dst, memref, reg);                                         // cmp   [dest],reg
	}
}


//-------------------------------------------------
//  emit_and_r64_p64 - and operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_and_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != U64(0xffffffffffffffff))
		{
			if (short_immediate(param.immediate()))
				emit_and_r64_imm(dst, reg, param.immediate());                          // and   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_and_r64_r64(dst, reg, REG_R11);                                    // and   reg,r11
			}
		}
	}
	else if (param.is_memory())
		emit_and_r64_m64(dst, reg, MABS(param.memory()));                               // and   reg,[param]
	else if (param.is_int_register())
		emit_and_r64_r64(dst, reg, param.ireg());                                       // and   reg,param
}


//-------------------------------------------------
//  emit_and_m64_p64 - and operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_and_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != U64(0xffffffffffffffff))
		{
			if (short_immediate(param.immediate()))
				emit_and_m64_imm(dst, memref, param.immediate());                   // and   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_and_m64_r64(dst, memref, REG_R11);                             // and   [mem],r11
			}
		}
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_and_m64_r64(dst, memref, reg);                                         // and   [dest],reg
	}
}


//-------------------------------------------------
//  emit_test_r64_p64 - test operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_test_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (short_immediate(param.immediate()))
			emit_test_r64_imm(dst, reg, param.immediate());                             // test  reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param.immediate());                          // mov   r11,param
			emit_test_r64_r64(dst, reg, REG_R11);                                       // test  reg,r11
		}
	}
	else if (param.is_memory())
		emit_test_m64_r64(dst, MABS(param.memory()), reg);                              // test  [param],reg
	else if (param.is_int_register())
		emit_test_r64_r64(dst, reg, param.ireg());                                      // test  reg,param
}


//-------------------------------------------------
//  emit_test_m64_p64 - test operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_test_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate() && short_immediate(param.immediate()))
		emit_test_m64_imm(dst, memref, param.immediate());                          // test  [dest],param
	else if (param.is_memory())
	{
		emit_mov_r64_p64(dst, REG_EAX, param);                                          // mov   reg,param
		emit_test_m64_r64(dst, memref, REG_EAX);                                        // test  [dest],reg
	}
	else if (param.is_int_register())
		emit_test_m64_r64(dst, memref, param.ireg());                               // test  [dest],param
}


//-------------------------------------------------
//  emit_or_r64_p64 - or operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_or_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_or_r64_imm(dst, reg, param.immediate());                           // or    reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_or_r64_r64(dst, reg, REG_R11);                                     // or    reg,r11
			}
		}
	}
	else if (param.is_memory())
		emit_or_r64_m64(dst, reg, MABS(param.memory()));                                // or    reg,[param]
	else if (param.is_int_register())
		emit_or_r64_r64(dst, reg, param.ireg());                                        // or    reg,param
}


//-------------------------------------------------
//  emit_or_m64_p64 - or operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_or_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (short_immediate(param.immediate()))
				emit_or_m64_imm(dst, memref, param.immediate());                        // or    [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_or_m64_r64(dst, memref, REG_R11);                              // or    [mem],r11
			}
		}
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_or_m64_r64(dst, memref, reg);                                          // or    [dest],reg
	}
}


//-------------------------------------------------
//  emit_xor_r64_p64 - xor operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_xor_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (param.immediate() == U64(0xffffffffffffffff))
				emit_not_r64(dst, reg);                                                 // not   reg
			else if (short_immediate(param.immediate()))
				emit_xor_r64_imm(dst, reg, param.immediate());                          // xor   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_xor_r64_r64(dst, reg, REG_R11);                                    // xor   reg,r11
			}
		}
	}
	else if (param.is_memory())
		emit_xor_r64_m64(dst, reg, MABS(param.memory()));                               // xor   reg,[param]
	else if (param.is_int_register())
		emit_xor_r64_r64(dst, reg, param.ireg());                                       // xor   reg,param
}


//-------------------------------------------------
//  emit_xor_m64_p64 - xor operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_xor_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
		{
			if (param.immediate() == U64(0xffffffffffffffff))
				emit_not_m64(dst, memref);                                          // not   [mem]
			else if (short_immediate(param.immediate()))
				emit_xor_m64_imm(dst, memref, param.immediate());                   // xor   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param.immediate());                      // mov   r11,param
				emit_xor_m64_r64(dst, memref, REG_R11);                             // xor   [mem],r11
			}
		}
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r64_p64(dst, reg, param);                                              // mov   reg,param
		emit_xor_m64_r64(dst, memref, reg);                                         // xor   [dest],reg
	}
}


//-------------------------------------------------
//  emit_shl_r64_p64 - shl operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shl_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shl_r64_imm(dst, reg, param.immediate());                              // shl   reg,param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_shl_r64_cl(dst, reg);                                                      // shl   reg,cl
	}
}


//-------------------------------------------------
//  emit_shl_m64_p64 - shl operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shl_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_shl_m64_imm(dst, memref, param.immediate());                       // shl   [dest],param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_shl_m64_cl(dst, memref);                                               // shl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_shr_r64_p64 - shr operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shr_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shr_r64_imm(dst, reg, param.immediate());                              // shr   reg,param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_shr_r64_cl(dst, reg);                                                      // shr   reg,cl
	}
}


//-------------------------------------------------
//  emit_shr_m64_p64 - shr operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_shr_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_shr_m64_imm(dst, memref, param.immediate());                       // shr   [dest],param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_shr_m64_cl(dst, memref);                                               // shr   [dest],cl
	}
}


//-------------------------------------------------
//  emit_sar_r64_p64 - sar operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sar_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_sar_r64_imm(dst, reg, param.immediate());                              // sar   reg,param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_sar_r64_cl(dst, reg);                                                      // sar   reg,cl
	}
}


//-------------------------------------------------
//  emit_sar_m64_p64 - sar operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_sar_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_sar_m64_imm(dst, memref, param.immediate());                       // sar   [dest],param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_sar_m64_cl(dst, memref);                                               // sar   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rol_r64_p64 - rol operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rol_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rol_r64_imm(dst, reg, param.immediate());                              // rol   reg,param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_rol_r64_cl(dst, reg);                                                      // rol   reg,cl
	}
}


//-------------------------------------------------
//  emit_rol_m64_p64 - rol operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rol_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_rol_m64_imm(dst, memref, param.immediate());                       // rol   [dest],param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_rol_m64_cl(dst, memref);                                               // rol   [dest],cl
	}
}


//-------------------------------------------------
//  emit_ror_r64_p64 - ror operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_ror_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_ror_r64_imm(dst, reg, param.immediate());                              // ror   reg,param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_ror_r64_cl(dst, reg);                                                      // ror   reg,cl
	}
}


//-------------------------------------------------
//  emit_ror_m64_p64 - ror operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_ror_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_ror_m64_imm(dst, memref, param.immediate());                       // ror   [dest],param
	}
	else
	{
		emit_mov_r64_p64(dst, REG_RCX, param);                                          // mov   rcx,param
		emit_ror_m64_cl(dst, memref);                                               // ror   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcl_r64_p64 - rcl operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcl_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcl_r64_imm(dst, reg, param.immediate());                              // rcl   reg,param
	}
	else
	{
		emit_mov_r64_p64_keepflags(dst, REG_RCX, param);                                // mov   rcx,param
		emit_rcl_r64_cl(dst, reg);                                                      // rcl   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcl_m64_p64 - rcl operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcl_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_rcl_m64_imm(dst, memref, param.immediate());                       // rcl   [dest],param
	}
	else
	{
		emit_mov_r64_p64_keepflags(dst, REG_RCX, param);                                // mov   rcx,param
		emit_rcl_m64_cl(dst, memref);                                               // rcl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcr_r64_p64 - rcr operation to a 64-bit
//  register from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcr_r64_p64(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcr_r64_imm(dst, reg, param.immediate());                              // rcr   reg,param
	}
	else
	{
		emit_mov_r64_p64_keepflags(dst, REG_RCX, param);                                // mov   rcx,param
		emit_rcr_r64_cl(dst, reg);                                                      // rcr   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcr_m64_p64 - rcr operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_rcr_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT64)param.immediate() == 0)
			;// skip
		else
			emit_rcr_m64_imm(dst, memref, param.immediate());                       // rcr   [dest],param
	}
	else
	{
		emit_mov_r64_p64_keepflags(dst, REG_RCX, param);                                // mov   rcx,param
		emit_rcr_m64_cl(dst, memref);                                               // rcr   [dest],cl
	}
}



/***************************************************************************
    EMITTERS FOR FLOATING POINT OPERATIONS WITH PARAMETERS
***************************************************************************/

//-------------------------------------------------
//  emit_movss_r128_p32 - move a 32-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::emit_movss_r128_p32(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_movss_r128_m32(dst, reg, MABS(param.memory()));                            // movss reg,[param]
	else if (param.is_float_register())
	{
		if (reg != param.freg())
			emit_movss_r128_r128(dst, reg, param.freg());                               // movss reg,param
	}
}


//-------------------------------------------------
//  emit_movss_p32_r128 - move a register into a
//  32-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_movss_p32_r128(x86code *&dst, const be_parameter &param, UINT8 reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_movss_m32_r128(dst, MABS(param.memory()), reg);                            // movss [param],reg
	else if (param.is_float_register())
	{
		if (reg != param.freg())
			emit_movss_r128_r128(dst, param.freg(), reg);                               // movss param,reg
	}
}


//-------------------------------------------------
//  emit_movsd_r128_p64 - move a 64-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x64::emit_movsd_r128_p64(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_movsd_r128_m64(dst, reg, MABS(param.memory()));                            // movsd reg,[param]
	else if (param.is_float_register())
	{
		if (reg != param.freg())
			emit_movsd_r128_r128(dst, reg, param.freg());                               // movsd reg,param
	}
}


//-------------------------------------------------
//  emit_movsd_p64_r128 - move a register into a
//  64-bit parameter
//-------------------------------------------------

void drcbe_x64::emit_movsd_p64_r128(x86code *&dst, const be_parameter &param, UINT8 reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
		emit_movsd_m64_r128(dst, MABS(param.memory()), reg);                            // movsd [param],reg
	else if (param.is_float_register())
	{
		if (reg != param.freg())
			emit_movsd_r128_r128(dst, param.freg(), reg);                               // movsd param,reg
	}
}



/***************************************************************************
    OUT-OF-BAND CODE FIXUP CALLBACKS
***************************************************************************/

//-------------------------------------------------
//  fixup_label - callback to fixup forward-
//  referenced labels
//-------------------------------------------------

void drcbe_x64::fixup_label(void *parameter, drccodeptr labelcodeptr)
{
	drccodeptr src = (drccodeptr)parameter;

	// find the end of the instruction
	if (src[0] == 0xe3)
	{
		src += 1 + 1;
		src[-1] = labelcodeptr - src;
	}
	else if (src[0] == 0xe9)
	{
		src += 1 + 4;
		((UINT32 *)src)[-1] = labelcodeptr - src;
	}
	else if (src[0] == 0x0f && (src[1] & 0xf0) == 0x80)
	{
		src += 2 + 4;
		((UINT32 *)src)[-1] = labelcodeptr - src;
	}
	else
		fatalerror("fixup_label called with invalid jmp source!\n");
}


//-------------------------------------------------
//  fixup_exception - callback to perform cleanup
//  and jump to an exception handler
//-------------------------------------------------

void drcbe_x64::fixup_exception(drccodeptr *codeptr, void *param1, void *param2)
{
	drccodeptr src = (drccodeptr)param1;
	const instruction &inst = *(const instruction *)param2;

	// normalize parameters
	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());
	be_parameter exp(*this, inst.param(1), PTYPE_MRI);

	// look up the handle target
	drccodeptr *targetptr = handp.handle().codeptr_addr();

	// first fixup the jump to get us here
	drccodeptr dst = *codeptr;
	((UINT32 *)src)[-1] = dst - src;

	// then store the exception parameter
	emit_mov_m32_p32(dst, MABS(&m_state.exp), exp);                                     // mov   [exp],exp

	// push the original return address on the stack
	emit_lea_r64_m64(dst, REG_RAX, MABS(src));                                          // lea   rax,[return]
	emit_push_r64(dst, REG_RAX);                                                        // push  rax
	if (*targetptr != nullptr)
		emit_jmp(dst, *targetptr);                                                      // jmp   *targetptr
	else
		emit_jmp_m64(dst, MABS(targetptr));                                             // jmp   [targetptr]

	*codeptr = dst;
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

void drcbe_x64::op_handle(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_handle());

	// emit a jump around the stack adjust in case code falls through here
	emit_link skip;
	emit_jmp_short_link(dst, skip);                                                     // jmp   skip

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(dst);

	// by default, the handle points to prolog code that moves the stack pointer
	emit_lea_r64_m64(dst, REG_RSP, MBD(REG_RSP, -40));                                  // lea   rsp,[rsp-40]
	resolve_link(dst, skip);                                                        // skip:
}


//-------------------------------------------------
//  op_hash - process a HASH opcode
//-------------------------------------------------

void drcbe_x64::op_hash(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	// register the current pointer for the mode/PC
	m_hash.set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), dst);
}


//-------------------------------------------------
//  op_label - process a LABEL opcode
//-------------------------------------------------

void drcbe_x64::op_label(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_label());

	// register the current pointer for the label
	m_labels.set_codeptr(inst.param(0).label(), dst);
}


//-------------------------------------------------
//  op_comment - process a COMMENT opcode
//-------------------------------------------------

void drcbe_x64::op_comment(x86code *&dst, const instruction &inst)
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

void drcbe_x64::op_mapvar(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_mapvar());
	assert(inst.param(1).is_immediate());

	// set the value of the specified mapvar
	m_map.set_value(dst, inst.param(0).mapvar(), inst.param(1).immediate());
}



/***************************************************************************
    CONTROL FLOW OPCODES
***************************************************************************/

//-------------------------------------------------
//  op_nop - process a NOP opcode
//-------------------------------------------------

void drcbe_x64::op_nop(x86code *&dst, const instruction &inst)
{
	// nothing
}


//-------------------------------------------------
//  op_debug - process a DEBUG opcode
//-------------------------------------------------

void drcbe_x64::op_debug(x86code *&dst, const instruction &inst)
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
		emit_mov_r64_imm(dst, REG_RAX, (FPTR)&m_device.machine().debug_flags);          // mov   rax,&debug_flags
		emit_test_m32_imm(dst, MBD(REG_RAX, 0), DEBUG_FLAG_CALL_HOOK);                  // test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		emit_link skip = { nullptr };
		emit_jcc_short_link(dst, x64emit::COND_Z, skip);                                // jz    skip

		// push the parameter
		emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)&m_device);                             // mov   param1,device
		emit_mov_r32_p32(dst, REG_PARAM2, pcp);                                         // mov   param2,pcp
		emit_smart_call_m64(dst, &m_near.debug_cpu_instruction_hook);                   // call  debug_cpu_instruction_hook

		resolve_link(dst, skip);                                                    // skip:
	}
}


//-------------------------------------------------
//  op_exit - process an EXIT opcode
//-------------------------------------------------

void drcbe_x64::op_exit(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter retp(*this, inst.param(0), PTYPE_MRI);

	// load the parameter into EAX
	emit_mov_r32_p32(dst, REG_EAX, retp);                                               // mov   eax,retp
	if (inst.condition() == uml::COND_ALWAYS)
		emit_jmp(dst, m_exit);                                                          // jmp   exit
	else
		emit_jcc(dst, X86_CONDITION(inst.condition()), m_exit);                         // jcc   exit
}


//-------------------------------------------------
//  op_hashjmp - process a HASHJMP opcode
//-------------------------------------------------

void drcbe_x64::op_hashjmp(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, REG_PARAM1, pcp);
		emit_mov_r32_p32(dst, REG_PARAM2, modep);
		emit_smart_call_m64(dst, &m_near.debug_log_hashjmp);
	}

	// load the stack base one word early so we end up at the right spot after our call below
	emit_mov_r64_m64(dst, REG_RSP, MABS(&m_near.hashstacksave));                        // mov   rsp,[hashstacksave]

	// fixed mode cases
	if (modep.is_immediate() && m_hash.is_mode_populated(modep.immediate()))
	{
		// a straight immediate jump is direct, though we need the PC in EAX in case of failure
		if (pcp.is_immediate())
		{
			UINT32 l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			UINT32 l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			emit_call_m64(dst, MABS(&m_hash.base()[modep.immediate()][l1val][l2val]));
																						// call  hash[modep][l1val][l2val]
		}

		// a fixed mode but variable PC
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, pcp);                                        // mov   eax,pcp
			emit_mov_r32_r32(dst, REG_EDX, REG_EAX);                                    // mov   edx,eax
			emit_shr_r32_imm(dst, REG_EDX, m_hash.l1shift());                           // shr   edx,l1shift
			emit_and_r32_imm(dst, REG_EAX, m_hash.l2mask() << m_hash.l2shift());        // and  eax,l2mask << l2shift
			emit_mov_r64_m64(dst, REG_RDX, MBISD(REG_RBP, REG_RDX, 8, offset_from_rbp(&m_hash.base()[modep.immediate()][0])));
																						// mov   rdx,hash[modep+edx*8]
			emit_call_m64(dst, MBISD(REG_RDX, REG_RAX, 8 >> m_hash.l2shift(), 0));      // call  [rdx+rax*shift]
		}
	}
	else
	{
		// variable mode
		int modereg = modep.select_register(REG_ECX);
		emit_mov_r32_p32(dst, modereg, modep);                                          // mov   modereg,modep
		emit_mov_r64_m64(dst, REG_RCX, MBISD(REG_RBP, modereg, 8, offset_from_rbp(m_hash.base())));
																						// mov   rcx,hash[modereg*8]

		// fixed PC
		if (pcp.is_immediate())
		{
			UINT32 l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			UINT32 l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			emit_mov_r64_m64(dst, REG_RDX, MBD(REG_RCX, l1val*8));                      // mov   rdx,[rcx+l1val*8]
			emit_call_m64(dst, MBD(REG_RDX, l2val*8));                                  // call  [l2val*8]
		}

		// variable PC
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, pcp);                                        // mov   eax,pcp
			emit_mov_r32_r32(dst, REG_EDX, REG_EAX);                                    // mov   edx,eax
			emit_shr_r32_imm(dst, REG_EDX, m_hash.l1shift());                           // shr   edx,l1shift
			emit_mov_r64_m64(dst, REG_RDX, MBISD(REG_RCX, REG_RDX, 8, 0));              // mov   rdx,[rcx+rdx*8]
			emit_and_r32_imm(dst, REG_EAX, m_hash.l2mask() << m_hash.l2shift());        // and  eax,l2mask << l2shift
			emit_call_m64(dst, MBISD(REG_RDX, REG_RAX, 8 >> m_hash.l2shift(), 0));      // call  [rdx+rax*shift]
		}
	}

	// in all cases, if there is no code, we return here to generate the exception
	if (LOG_HASHJMPS)
		emit_smart_call_m64(dst, &m_near.debug_log_hashjmp_fail);

	emit_mov_m32_p32(dst, MABS(&m_state.exp), pcp);                                     // mov   [exp],param
	emit_sub_r64_imm(dst, REG_RSP, 8);                                                  // sub   rsp,8
	emit_call_m64(dst, MABS(exp.handle().codeptr_addr()));                              // call  [exp]
}


//-------------------------------------------------
//  op_jmp - process a JMP opcode
//-------------------------------------------------

void drcbe_x64::op_jmp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	const parameter &labelp = inst.param(0);
	assert(labelp.is_code_label());

	// look up the jump target and jump there
	x86code *jmptarget = (x86code *)m_labels.get_codeptr(labelp.label(), m_fixup_label, dst);
	if (jmptarget == nullptr)
		jmptarget = dst + 0x7ffffff0;
	if (inst.condition() == uml::COND_ALWAYS)
		emit_jmp(dst, jmptarget);                                                       // jmp   target
	else
		emit_jcc(dst, X86_CONDITION(inst.condition()), jmptarget);                      // jcc   target

}


//-------------------------------------------------
//  op_exh - process an EXH opcode
//-------------------------------------------------

void drcbe_x64::op_exh(x86code *&dst, const instruction &inst)
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

	// perform the exception processing inline if unconditional
	if (inst.condition() == uml::COND_ALWAYS)
	{
		emit_mov_m32_p32(dst, MABS(&m_state.exp), exp);                                 // mov   [exp],exp
		if (*targetptr != nullptr)
			emit_call(dst, *targetptr);                                                 // call  *targetptr
		else
			emit_call_m64(dst, MABS(targetptr));                                        // call  [targetptr]
	}

	// otherwise, jump to an out-of-band handler
	else
	{
		emit_jcc(dst, X86_CONDITION(inst.condition()), dst + 0x7ffffff0);               // jcc   exception
		m_cache.request_oob_codegen(m_fixup_exception, dst, &const_cast<instruction &>(inst));
	}
}


//-------------------------------------------------
//  op_callh - process a CALLH opcode
//-------------------------------------------------

void drcbe_x64::op_callh(x86code *&dst, const instruction &inst)
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
	emit_link skip = { nullptr };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// jump through the handle; directly if a normal jump
	if (*targetptr != nullptr)
		emit_call(dst, *targetptr);                                                     // call  *targetptr
	else
		emit_call_m64(dst, MABS(targetptr));                                            // call  [targetptr]

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		resolve_link(dst, skip);                                                    // skip:
}


//-------------------------------------------------
//  op_ret - process a RET opcode
//-------------------------------------------------

void drcbe_x64::op_ret(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	// skip if conditional
	emit_link skip = { nullptr };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// return
	emit_lea_r64_m64(dst, REG_RSP, MBD(REG_RSP, 40));                                   // lea   rsp,[rsp+40]
	emit_ret(dst);                                                                      // ret

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		resolve_link(dst, skip);                                                    // skip:
}


//-------------------------------------------------
//  op_callc - process a CALLC opcode
//-------------------------------------------------

void drcbe_x64::op_callc(x86code *&dst, const instruction &inst)
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
	emit_link skip = { nullptr };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// perform the call
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)paramp.memory());                           // mov   param1,paramp
	emit_smart_call_r64(dst, (x86code *)(FPTR)funcp.cfunc(), REG_RAX);                  // call  funcp

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		resolve_link(dst, skip);                                                    // skip:
}


//-------------------------------------------------
//  op_recover - process a RECOVER opcode
//-------------------------------------------------

void drcbe_x64::op_recover(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// call the recovery code
	emit_mov_r64_m64(dst, REG_RAX, MABS(&m_near.stacksave));                            // mov   rax,stacksave
	emit_mov_r64_m64(dst, REG_RAX, MBD(REG_RAX, -8));                                   // mov   rax,[rax-4]
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)&m_map);                                    // mov   param1,m_map
	emit_lea_r64_m64(dst, REG_PARAM2, MBD(REG_RAX, -1));                                // lea   param2,[rax-1]
	emit_mov_r64_imm(dst, REG_PARAM3, inst.param(1).mapvar());                          // mov   param3,param[1].value
	emit_smart_call_m64(dst, &m_near.drcmap_get_value);                                 // call  drcmap_get_value
	emit_mov_p32_r32(dst, dstp, REG_EAX);                                               // mov   dstp,eax

}



/***************************************************************************
    INTERNAL REGISTER OPCODES
***************************************************************************/

//-------------------------------------------------
//  op_setfmod - process a SETFMOD opcode
//-------------------------------------------------

void drcbe_x64::op_setfmod(x86code *&dst, const instruction &inst)
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
		emit_mov_m8_imm(dst, MABS(&m_state.fmod), value);                               // mov   [fmod],srcp
		emit_ldmxcsr_m32(dst, MABS(&m_near.ssecontrol[value]));                         // ldmxcsr fp_control[srcp]
	}

	// register/memory case
	else
	{
		emit_mov_r32_p32(dst, REG_EAX, srcp);                                           // mov   eax,srcp
		emit_and_r32_imm(dst, REG_EAX, 3);                                              // and   eax,3
		emit_mov_m8_r8(dst, MABS(&m_state.fmod), REG_AL);                               // mov   [fmod],al
		emit_ldmxcsr_m32(dst, MBISD(REG_RBP, REG_RAX, 4, offset_from_rbp(&m_near.ssecontrol[0])));
																						// ldmxcsr fp_control[eax]
	}
}


//-------------------------------------------------
//  op_getfmod - process a GETFMOD opcode
//-------------------------------------------------

void drcbe_x64::op_getfmod(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the current mode and store to the destination
	if (dstp.is_int_register())
		emit_movzx_r32_m8(dst, dstp.ireg(), MABS(&m_state.fmod));                       // movzx reg,[fmod]
	else
	{
		emit_movzx_r32_m8(dst, REG_EAX, MABS(&m_state.fmod));                           // movzx eax,[fmod]
		emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                            // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getexp - process a GETEXP opcode
//-------------------------------------------------

void drcbe_x64::op_getexp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the exception parameter and store to the destination
	if (dstp.is_int_register())
		emit_mov_r32_m32(dst, dstp.ireg(), MABS(&m_state.exp));                         // mov   reg,[exp]
	else
	{
		emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.exp));                             // mov   eax,[exp]
		emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                            // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getflgs - process a GETFLGS opcode
//-------------------------------------------------

void drcbe_x64::op_getflgs(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter maskp(*this, inst.param(1), PTYPE_I);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// compute mask for flags
	UINT32 flagmask = 0;
	if (maskp.immediate() & FLAG_C) flagmask |= 0x001;
	if (maskp.immediate() & FLAG_V) flagmask |= 0x800;
	if (maskp.immediate() & FLAG_Z) flagmask |= 0x040;
	if (maskp.immediate() & FLAG_S) flagmask |= 0x080;
	if (maskp.immediate() & FLAG_U) flagmask |= 0x004;

	switch (maskp.immediate())
	{
		// single flags only
		case FLAG_C:
			emit_setcc_r8(dst, x64emit::COND_C, REG_AL);                                            // setc   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
			break;

		case FLAG_V:
			emit_setcc_r8(dst, x64emit::COND_O, REG_AL);                                            // seto   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		case FLAG_Z:
			emit_setcc_r8(dst, x64emit::COND_Z, REG_AL);                                            // setz   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 2);                                           // shl    dstreg,2
			break;

		case FLAG_S:
			emit_setcc_r8(dst, x64emit::COND_S, REG_AL);                                            // sets   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 3);                                           // shl    dstreg,3
			break;

		case FLAG_U:
			emit_setcc_r8(dst, x64emit::COND_P, REG_AL);                                            // setp   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 4);                                           // shl    dstreg,4
			break;

		// carry plus another flag
		case FLAG_C | FLAG_V:
			emit_setcc_r8(dst, x64emit::COND_C, REG_AL);                                            // setc   al
			emit_setcc_r8(dst, x64emit::COND_O, REG_CL);                                            // seto   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,cl
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			break;

		case FLAG_C | FLAG_Z:
			emit_setcc_r8(dst, x64emit::COND_C, REG_AL);                                            // setc   al
			emit_setcc_r8(dst, x64emit::COND_Z, REG_CL);                                            // setz   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,cl
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));               // lea    dstreg,[eax+ecx*4]
			break;

		case FLAG_C | FLAG_S:
			emit_setcc_r8(dst, x64emit::COND_C, REG_AL);                                            // setc   al
			emit_setcc_r8(dst, x64emit::COND_S, REG_CL);                                            // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,cl
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 8, 0));               // lea    dstreg,[eax+ecx*8]
			break;

		// overflow plus another flag
		case FLAG_V | FLAG_Z:
			emit_setcc_r8(dst, x64emit::COND_O, REG_AL);                                            // seto   al
			emit_setcc_r8(dst, x64emit::COND_Z, REG_CL);                                            // setz   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,cl
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		case FLAG_V | FLAG_S:
			emit_setcc_r8(dst, x64emit::COND_O, REG_AL);                                            // seto   al
			emit_setcc_r8(dst, x64emit::COND_S, REG_CL);                                            // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));               // lea    dstreg,[eax+ecx*4]
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		// zero plus another flag
		case FLAG_Z | FLAG_S:
			emit_setcc_r8(dst, x64emit::COND_Z, REG_AL);                                            // setz   al
			emit_setcc_r8(dst, x64emit::COND_S, REG_CL);                                            // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(dst, dstreg, 2);                                           // shl    dstreg,2
			break;

		// default cases
		default:
			emit_pushf(dst);                                                            // pushf
			emit_pop_r64(dst, REG_EAX);                                                 // pop    eax
			emit_and_r32_imm(dst, REG_EAX, flagmask);                                   // and    eax,flagmask
			emit_movzx_r32_m8(dst, dstreg, MBISD(REG_RBP, REG_RAX, 1, offset_from_rbp(&m_near.flagsmap[0])));
																						// movzx  dstreg,[flags_map]
			break;
	}

	// 32-bit form
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg

	// 64-bit form
	else if (inst.size() == 8)
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_save - process a SAVE opcode
//-------------------------------------------------

void drcbe_x64::op_save(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_M);

	// copy live state to the destination
	emit_mov_r64_imm(dst, REG_RCX, (FPTR)dstp.memory());                                // mov    rcx,dstp

	// copy flags
	emit_pushf(dst);                                                                    // pushf
	emit_pop_r64(dst, REG_RAX);                                                         // pop    rax
	emit_and_r32_imm(dst, REG_EAX, 0x8c5);                                              // and    eax,0x8c5
	emit_mov_r8_m8(dst, REG_AL, MBISD(REG_RBP, REG_RAX, 1, offset_from_rbp(&m_near.flagsmap[0])));
																						// mov    al,[flags_map]
	emit_mov_m8_r8(dst, MBD(REG_RCX, offsetof(drcuml_machine_state, flags)), REG_AL);   // mov    state->flags,al

	// copy fmod and exp
	emit_mov_r8_m8(dst, REG_AL, MABS(&m_state.fmod));                                   // mov    al,[fmod]
	emit_mov_m8_r8(dst, MBD(REG_RCX, offsetof(drcuml_machine_state, fmod)), REG_AL);    // mov    state->fmod,al
	emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.exp));                                 // mov    eax,[exp]
	emit_mov_m32_r32(dst, MBD(REG_RCX, offsetof(drcuml_machine_state, exp)), REG_EAX);  // mov    state->exp,eax

	// copy integer registers
	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_m64_r64(dst, MBD(REG_RCX, regoffs + 8 * regnum), int_register_map[regnum]);
		else
		{
			emit_mov_r64_m64(dst, REG_RAX, MABS(&m_state.r[regnum].d));
			emit_mov_m64_r64(dst, MBD(REG_RCX, regoffs + 8 * regnum), REG_RAX);
		}
	}

	// copy FP registers
	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			emit_movsd_m64_r128(dst, MBD(REG_RCX, regoffs + 8 * regnum), float_register_map[regnum]);
		else
		{
			emit_mov_r64_m64(dst, REG_RAX, MABS(&m_state.f[regnum].d));
			emit_mov_m64_r64(dst, MBD(REG_RCX, regoffs + 8 * regnum), REG_RAX);
		}
	}
}


//-------------------------------------------------
//  op_restore - process a RESTORE opcode
//-------------------------------------------------

void drcbe_x64::op_restore(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_M);

	// copy live state from the destination
	emit_mov_r64_imm(dst, REG_ECX, (FPTR)srcp.memory());                                // mov    rcx,dstp

	// copy integer registers
	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_r64_m64(dst, int_register_map[regnum], MBD(REG_RCX, regoffs + 8 * regnum));
		else
		{
			emit_mov_r64_m64(dst, REG_RAX, MBD(REG_RCX, regoffs + 8 * regnum));
			emit_mov_m64_r64(dst, MABS(&m_state.r[regnum].d), REG_RAX);
		}
	}

	// copy FP registers
	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			emit_movsd_r128_m64(dst, float_register_map[regnum], MBD(REG_RCX, regoffs + 8 * regnum));
		else
		{
			emit_mov_r64_m64(dst, REG_RAX, MBD(REG_RCX, regoffs + 8 * regnum));
			emit_mov_m64_r64(dst, MABS(&m_state.f[regnum].d), REG_RAX);
		}
	}

	// copy fmod and exp
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, fmod)));// movzx eax,state->fmod
	emit_and_r32_imm(dst, REG_EAX, 3);                                                  // and   eax,3
	emit_mov_m8_r8(dst, MABS(&m_state.fmod), REG_AL);                                   // mov   [fmod],al
	emit_ldmxcsr_m32(dst, MBISD(REG_RBP, REG_RAX, 4, offset_from_rbp(&m_near.ssecontrol[0])));
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, exp)));  // mov    eax,state->exp
	emit_mov_m32_r32(dst, MABS(&m_state.exp), REG_EAX);                                 // mov    [exp],eax

	// copy flags
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, flags)));// movzx eax,state->flags
	emit_push_m64(dst, MBISD(REG_RBP, REG_RAX, 8, offset_from_rbp(&m_near.flagsunmap[0])));
																						// push   flags_unmap[eax*8]
	emit_popf(dst);                                                                     // popf
}



/***************************************************************************
    INTEGER OPERATIONS
***************************************************************************/

//-------------------------------------------------
//  op_load - process a LOAD opcode
//-------------------------------------------------

void drcbe_x64::op_load(x86code *&dst, const instruction &inst)
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
	int scale = 1 << scalesizep.scale();
	int size = scalesizep.size();

	// determine the pointer base
	INT32 baseoffs;
	int basereg = get_base_register_and_offset(dst, basep.memory(), REG_RDX, baseoffs);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// immediate index
	if (indp.is_immediate())
	{
		if (size == SIZE_BYTE)
			emit_movzx_r32_m8(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movzx_r32_m16(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate())); // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			emit_mov_r64_m64(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate())); // mov   dstreg,[basep + scale*indp]
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_movsx_r64_p32(dst, indreg, indp);
		if (size == SIZE_BYTE)
			emit_movzx_r32_m8(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));    // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movzx_r32_m16(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));   // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));     // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
			emit_mov_r64_m64(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));     // mov   dstreg,[basep + scale*indp]
	}

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	else
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg

}


//-------------------------------------------------
//  op_loads - process a LOADS opcode
//-------------------------------------------------

void drcbe_x64::op_loads(x86code *&dst, const instruction &inst)
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
	int scale = 1 << scalesizep.scale();
	int size = scalesizep.size();

	// determine the pointer base
	INT32 baseoffs;
	int basereg = get_base_register_and_offset(dst, basep.memory(), REG_RDX, baseoffs);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// immediate index
	if (indp.is_immediate())
	{
		if (inst.size() == 4)
		{
			if (size == SIZE_BYTE)
				emit_movsx_r32_m8(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_WORD)
				emit_movsx_r32_m16(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_DWORD)
				emit_mov_r32_m32(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate())); // mov   dstreg,[basep + scale*indp]
		}
		else if (inst.size() == 8)
		{
			if (size == SIZE_BYTE)
				emit_movsx_r64_m8(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movzx dstreg,[basep + scale*indp]
			else if (size == SIZE_WORD)
				emit_movsx_r64_m16(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movzx dstreg,[basep + scale*indp]
			else if (size == SIZE_DWORD)
				emit_movsxd_r64_m32(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate()));// movsxd dstreg,[basep + scale*indp]
			else if (size == SIZE_QWORD)
				emit_mov_r64_m64(dst, dstreg, MBD(basereg, baseoffs + scale*indp.immediate())); // mov   dstreg,[basep + scale*indp]
		}
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_movsx_r64_p32(dst, indreg, indp);
		if (inst.size() == 4)
		{
			if (size == SIZE_BYTE)
				emit_movsx_r32_m8(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_WORD)
				emit_movsx_r32_m16(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_DWORD)
				emit_mov_r32_m32(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs)); // mov   dstreg,[basep + scale*indp]
		}
		else if (inst.size() == 8)
		{
			if (size == SIZE_BYTE)
				emit_movsx_r64_m8(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_WORD)
				emit_movsx_r64_m16(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));// movsx dstreg,[basep + scale*indp]
			else if (size == SIZE_DWORD)
				emit_movsxd_r64_m32(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));// movsxd dstreg,[basep + scale*indp]
			else if (size == SIZE_QWORD)
				emit_mov_r64_m64(dst, dstreg, MBISD(basereg, indreg, scale, baseoffs)); // mov   dstreg,[basep + scale*indp]
		}
	}

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	else
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg

}


//-------------------------------------------------
//  op_store - process a STORE opcode
//-------------------------------------------------

void drcbe_x64::op_store(x86code *&dst, const instruction &inst)
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
	int scale = 1 << (scalesizep.scale());
	int size = scalesizep.size();

	// determine the pointer base
	INT32 baseoffs;
	int basereg = get_base_register_and_offset(dst, basep.memory(), REG_RDX, baseoffs);

	// pick a source register for the general case
	int srcreg = srcp.select_register(REG_EAX);

	// degenerate case: constant index
	if (indp.is_immediate())
	{
		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				emit_mov_m8_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcp.immediate());// mov   [basep + scale*indp],srcp
			else if (size == SIZE_WORD)
				emit_mov_m16_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcp.immediate());// mov   [basep + scale*indp],srcp
			else if (size == SIZE_DWORD)
				emit_mov_m32_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcp.immediate());// mov   [basep + scale*indp],srcp
			else if (size == SIZE_QWORD)
			{
				if (short_immediate(srcp.immediate()))
					emit_mov_m64_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcp.immediate());// mov   [basep + scale*indp],srcp
				else
				{
					emit_mov_m32_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcp.immediate());// mov   [basep + scale*indp],srcp
					emit_mov_m32_imm(dst, MBD(basereg, baseoffs + scale*indp.immediate() + 4), srcp.immediate() >> 32);
																						// mov   [basep + scale*indp + 4],srcp >> 32
				}
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(dst, srcreg, srcp);                                    // mov   srcreg,srcp
			else
				emit_mov_r64_p64(dst, srcreg, srcp);                                    // mov   srcreg,srcp
			if (size == SIZE_BYTE)
				emit_mov_m8_r8(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcreg);   // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_WORD)
				emit_mov_m16_r16(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcreg); // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_DWORD)
				emit_mov_m32_r32(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcreg); // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_QWORD)
				emit_mov_m64_r64(dst, MBD(basereg, baseoffs + scale*indp.immediate()), srcreg); // mov   [basep + scale*indp],srcreg
		}
	}

	// normal case: variable index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_movsx_r64_p32(dst, indreg, indp);                                          // mov   indreg,indp

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				emit_mov_m8_imm(dst, MBISD(basereg, indreg, scale, baseoffs), srcp.immediate());// mov   [basep + scale*ecx],srcp
			else if (size == SIZE_WORD)
				emit_mov_m16_imm(dst, MBISD(basereg, indreg, scale, baseoffs), srcp.immediate());// mov   [basep + scale*ecx],srcp
			else if (size == SIZE_DWORD)
				emit_mov_m32_imm(dst, MBISD(basereg, indreg, scale, baseoffs), srcp.immediate());// mov   [basep + scale*ecx],srcp
			else if (size == SIZE_QWORD)
			{
				if (short_immediate(srcp.immediate()))
					emit_mov_m64_imm(dst, MBISD(basereg, indreg, scale, baseoffs), srcp.immediate());// mov   [basep + scale*indp],srcp
				else
				{
					emit_mov_m32_imm(dst, MBISD(basereg, indreg, scale, baseoffs), srcp.immediate());// mov   [basep + scale*ecx],srcp
					emit_mov_m32_imm(dst, MBISD(basereg, indreg, scale, baseoffs + 4), srcp.immediate() >> 32);
																						// mov   [basep + scale*ecx + 4],srcp >> 32
				}
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(dst, srcreg, srcp);                                    // mov   srcreg,srcp
			else
				emit_mov_r64_p64(dst, srcreg, srcp);                                    // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				emit_mov_m8_r8(dst, MBISD(basereg, indreg, scale, baseoffs), srcreg);   // mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_WORD)
				emit_mov_m16_r16(dst, MBISD(basereg, indreg, scale, baseoffs), srcreg); // mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_DWORD)
				emit_mov_m32_r32(dst, MBISD(basereg, indreg, scale, baseoffs), srcreg); // mov   [basep + scale*ecx],srcreg
			else if (size == SIZE_QWORD)
				emit_mov_m64_r64(dst, MBISD(basereg, indreg, scale, baseoffs), srcreg); // mov   [basep + scale*ecx],srcreg
		}
	}
}


//-------------------------------------------------
//  op_read - process a READ opcode
//-------------------------------------------------

void drcbe_x64::op_read(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_EAX);

	// set up a call to the read byte handler
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacesizep.space()]));             // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param2,addrp
	if (spacesizep.size() == SIZE_BYTE)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_byte);
																						// call   read_byte
		emit_movzx_r32_r8(dst, dstreg, REG_AL);                                         // movzx  dstreg,al
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_word);
																						// call   read_word
		emit_movzx_r32_r16(dst, dstreg, REG_AX);                                        // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_dword);
																						// call   read_dword
		if (dstreg != REG_EAX || inst.size() == 8)
			emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_qword);
																						// call   read_qword
		if (dstreg != REG_RAX)
			emit_mov_r64_r64(dst, dstreg, REG_RAX);                                     // mov    dstreg,rax
	}

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	else
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_readm - process a READM opcode
//-------------------------------------------------

void drcbe_x64::op_readm(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_EAX);

	// set up a call to the read byte handler
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacesizep.space()]));             // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_r32_p32(dst, REG_PARAM3, maskp);                                       // mov    param3,maskp
	else
		emit_mov_r64_p64(dst, REG_PARAM3, maskp);                                       // mov    param3,maskp
	if (spacesizep.size() == SIZE_WORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_word_masked);
																						// call   read_word_masked
		emit_movzx_r32_r16(dst, dstreg, REG_AX);                                        // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_dword_masked);
																						// call   read_dword_masked
		if (dstreg != REG_EAX || inst.size() == 8)
			emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].read_qword_masked);
																						// call   read_qword_masked
		if (dstreg != REG_RAX)
			emit_mov_r64_r64(dst, dstreg, REG_RAX);                                     // mov    dstreg,rax
	}

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	else
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
}


//-------------------------------------------------
//  op_write - process a WRITE opcode
//-------------------------------------------------

void drcbe_x64::op_write(x86code *&dst, const instruction &inst)
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

	// set up a call to the write byte handler
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacesizep.space()]));             // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_r32_p32(dst, REG_PARAM3, srcp);                                        // mov    param3,srcp
	else
		emit_mov_r64_p64(dst, REG_PARAM3, srcp);                                        // mov    param3,srcp
	if (spacesizep.size() == SIZE_BYTE)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_byte);
																						// call   write_byte
	else if (spacesizep.size() == SIZE_WORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_word);
																						// call   write_word
	else if (spacesizep.size() == SIZE_DWORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_dword);
																						// call   write_dword
	else if (spacesizep.size() == SIZE_QWORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_qword);
																						// call   write_qword
}


//-------------------------------------------------
//  op_writem - process a WRITEM opcode
//-------------------------------------------------

void drcbe_x64::op_writem(x86code *&dst, const instruction &inst)
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

	// set up a call to the write byte handler
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacesizep.space()]));             // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param2,addrp
	if (spacesizep.size() != SIZE_QWORD)
	{
		emit_mov_r32_p32(dst, REG_PARAM3, srcp);                                        // mov    param3,srcp
		emit_mov_r32_p32(dst, REG_PARAM4, maskp);                                       // mov    param4,maskp
	}
	else
	{
		emit_mov_r64_p64(dst, REG_PARAM3, srcp);                                        // mov    param3,srcp
		emit_mov_r64_p64(dst, REG_PARAM4, maskp);                                       // mov    param4,maskp
	}
	if (spacesizep.size() == SIZE_WORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_word_masked);
																						// call   write_word_masked
	else if (spacesizep.size() == SIZE_DWORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_dword_masked);
																						// call   write_dword_masked
	else if (spacesizep.size() == SIZE_QWORD)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacesizep.space()].write_qword_masked);
																						// call   write_qword_masked
}


//-------------------------------------------------
//  op_carry - process a CARRY opcode
//-------------------------------------------------

void drcbe_x64::op_carry(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);
	be_parameter bitp(*this, inst.param(1), PTYPE_MRI);

	// degenerate case: source is immediate
	if (srcp.is_immediate() && bitp.is_immediate())
	{
		if (srcp.immediate() & ((UINT64)1 << bitp.immediate()))
			emit_stc(dst);
		else
			emit_clc(dst);
		}

	// load non-immediate bit numbers into a register
	if (!bitp.is_immediate())
	{
		emit_mov_r32_p32(dst, REG_ECX, bitp);
		emit_and_r32_imm(dst, REG_ECX, inst.size() * 8 - 1);
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		if (bitp.is_immediate())
		{
			if (srcp.is_memory())
				emit_bt_m32_imm(dst, MABS(srcp.memory()), bitp.immediate());            // bt     [srcp],bitp
			else if (srcp.is_int_register())
				emit_bt_r32_imm(dst, srcp.ireg(), bitp.immediate());                    // bt     srcp,bitp
		}
		else
		{
			if (srcp.is_memory())
				emit_bt_m32_r32(dst, MABS(srcp.memory()), REG_ECX);                     // bt     [srcp],ecx
			else if (srcp.is_int_register())
				emit_bt_r32_r32(dst, srcp.ireg(), REG_ECX);                             // bt     srcp,ecx
		}
	}

	// 64-bit form
	else
	{
		if (bitp.is_immediate())
		{
			if (srcp.is_memory())
				emit_bt_m64_imm(dst, MABS(srcp.memory()), bitp.immediate());            // bt     [srcp],bitp
			else if (srcp.is_int_register())
				emit_bt_r64_imm(dst, srcp.ireg(), bitp.immediate());                    // bt     srcp,bitp
		}
		else
		{
			if (srcp.is_memory())
				emit_bt_m64_r64(dst, MABS(srcp.memory()), REG_ECX);                     // bt     [srcp],ecx
			else if (srcp.is_int_register())
				emit_bt_r64_r64(dst, srcp.ireg(), REG_ECX);                             // bt     srcp,ecx
		}
	}
}


//-------------------------------------------------
//  op_set - process a SET opcode
//-------------------------------------------------

void drcbe_x64::op_set(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// set to AL
	emit_setcc_r8(dst, X86_CONDITION(inst.condition()), REG_AL);                        // setcc  al
	emit_movzx_r32_r8(dst, dstreg, REG_AL);                                             // movzx  dstreg,al

	// 32-bit form
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg

	// 64-bit form
	else if (inst.size() == 8)
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg

}


//-------------------------------------------------
//  op_mov - process a MOV opcode
//-------------------------------------------------

void drcbe_x64::op_mov(x86code *&dst, const instruction &inst)
{
	x86code *savedst = dst;

	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// always start with a jmp
	emit_link skip = { nullptr };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// 32-bit form
	if (inst.size() == 4)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
			emit_mov_m32_r32(dst, MABS(dstp.memory()), srcp.ireg());                    // mov   [dstp],srcp

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate())
			emit_mov_m32_imm(dst, MABS(dstp.memory()), srcp.immediate());               // mov   [dstp],srcp

		// conditional memory to register
		else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_memory())
		{
			dst = savedst;
			skip.target = nullptr;
			emit_cmovcc_r32_m32(dst, X86_CONDITION(inst.condition()), dstp.ireg(), MABS(srcp.memory()));
																						// cmovcc dstp,[srcp]
		}

		// conditional register to register
		else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_int_register())
		{
			dst = savedst;
			skip.target = nullptr;
			emit_cmovcc_r32_r32(dst, X86_CONDITION(inst.condition()), dstp.ireg(), srcp.ireg());
																						// cmovcc dstp,srcp
		}

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, srcp);                              // mov   dstreg,srcp
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
			emit_mov_m64_r64(dst, MABS(dstp.memory()), srcp.ireg());                    // mov   [dstp],srcp

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate() && short_immediate(srcp.immediate()))
			emit_mov_m64_imm(dst, MABS(dstp.memory()), srcp.immediate());               // mov   [dstp],srcp

		// conditional memory to register
		else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_memory())
		{
			dst = savedst;
			skip.target = nullptr;
			emit_cmovcc_r64_m64(dst, X86_CONDITION(inst.condition()), dstp.ireg(), MABS(srcp.memory()));
																						// cmovcc dstp,[srcp]
		}

		// conditional register to register
		else if (inst.condition() != 0 && dstp.is_int_register() && srcp.is_int_register())
		{
			dst = savedst;
			skip.target = nullptr;
			emit_cmovcc_r64_r64(dst, X86_CONDITION(inst.condition()), dstp.ireg(), srcp.ireg());
																						// cmovcc dstp,srcp
		}

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, srcp);                              // mov   dstreg,srcp
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// resolve the jump
	if (skip.target != nullptr)
		resolve_link(dst, skip);
}


//-------------------------------------------------
//  op_sext - process a SEXT opcode
//-------------------------------------------------

void drcbe_x64::op_sext(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		if (srcp.is_memory())
		{
			if (sizep.size() == SIZE_BYTE)
				emit_movsx_r32_m8(dst, dstreg, MABS(srcp.memory()));                    // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_WORD)
				emit_movsx_r32_m16(dst, dstreg, MABS(srcp.memory()));                   // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_DWORD)
				emit_mov_r32_m32(dst, dstreg, MABS(srcp.memory()));                     // mov   dstreg,[srcp]
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				emit_movsx_r32_r8(dst, dstreg, srcp.ireg());                            // movsx dstreg,srcp
			else if (sizep.size() == SIZE_WORD)
				emit_movsx_r32_r16(dst, dstreg, srcp.ireg());                           // movsx dstreg,srcp
			else if (sizep.size() == SIZE_DWORD)
				emit_mov_r32_r32(dst, dstreg, srcp.ireg());                             // mov   dstreg,srcp
		}
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, dstreg, dstreg);                                     // test  dstreg,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		if (srcp.is_memory())
		{
			if (sizep.size() == SIZE_BYTE)
				emit_movsx_r64_m8(dst, dstreg, MABS(srcp.memory()));                    // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_WORD)
				emit_movsx_r64_m16(dst, dstreg, MABS(srcp.memory()));                   // movsx dstreg,[srcp]
			else if (sizep.size() == SIZE_DWORD)
				emit_movsxd_r64_m32(dst, dstreg, MABS(srcp.memory()));                  // movsxd dstreg,[srcp]
			else if (sizep.size() == SIZE_QWORD)
				emit_mov_r64_m64(dst, dstreg, MABS(srcp.memory()));                     // mov   dstreg,[srcp]
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				emit_movsx_r64_r8(dst, dstreg, srcp.ireg());                            // movsx dstreg,srcp
			else if (sizep.size() == SIZE_WORD)
				emit_movsx_r64_r16(dst, dstreg, srcp.ireg());                           // movsx dstreg,srcp
			else if (sizep.size() == SIZE_DWORD)
				emit_movsxd_r64_r32(dst, dstreg, srcp.ireg());                          // movsxd dstreg,srcp
			else if (sizep.size() == SIZE_QWORD)
				emit_mov_r64_r64(dst, dstreg, srcp.ireg());                             // mov   dstreg,srcp
		}
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
		if (inst.flags() != 0)
			emit_test_r64_r64(dst, dstreg, dstreg);                                     // test  dstreg,dstreg
	}
}


//-------------------------------------------------
//  op_roland - process an ROLAND opcode
//-------------------------------------------------

void drcbe_x64::op_roland(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, shiftp, maskp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(dst, dstreg, srcp);                                            // mov   dstreg,srcp
		emit_rol_r32_p32(dst, dstreg, shiftp, inst);                                    // rol   dstreg,shiftp
		emit_and_r32_p32(dst, dstreg, maskp, inst);                                     // and   dstreg,maskp
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, dstreg, srcp);                                            // mov   dstreg,srcp
		emit_rol_r64_p64(dst, dstreg, shiftp, inst);                                    // rol   dstreg,shiftp
		emit_and_r64_p64(dst, dstreg, maskp, inst);                                     // and   dstreg,maskp
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_rolins - process an ROLINS opcode
//-------------------------------------------------

void drcbe_x64::op_rolins(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_ECX, shiftp, maskp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(dst, REG_EAX, srcp);                                           // mov   eax,srcp
		emit_rol_r32_p32(dst, REG_EAX, shiftp, inst);                                   // rol   eax,shiftp
		emit_mov_r32_p32(dst, dstreg, dstp);                                            // mov   dstreg,dstp
		if (maskp.is_immediate())
		{
			emit_and_r32_imm(dst, REG_EAX, maskp.immediate());                          // and   eax,maskp
			emit_and_r32_imm(dst, dstreg, ~maskp.immediate());                          // and   dstreg,~maskp
		}
		else
		{
			emit_mov_r32_p32(dst, REG_EDX, maskp);                                      // mov   edx,maskp
			emit_and_r32_r32(dst, REG_EAX, REG_EDX);                                    // and   eax,edx
			emit_not_r32(dst, REG_EDX);                                                 // not   edx
			emit_and_r32_r32(dst, dstreg, REG_EDX);                                     // and   dstreg,edx
		}
		emit_or_r32_r32(dst, dstreg, REG_EAX);                                          // or    dstreg,eax
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, REG_RAX, srcp);                                           // mov   rax,srcp
		emit_mov_r64_p64(dst, REG_RDX, maskp);                                          // mov   rdx,maskp
		emit_rol_r64_p64(dst, REG_RAX, shiftp, inst);                                   // rol   rax,shiftp
		emit_mov_r64_p64(dst, dstreg, dstp);                                            // mov   dstreg,dstp
		emit_and_r64_r64(dst, REG_RAX, REG_RDX);                                        // and   eax,rdx
		emit_not_r64(dst, REG_RDX);                                                     // not   rdx
		emit_and_r64_r64(dst, dstreg, REG_RDX);                                         // and   dstreg,rdx
		emit_or_r64_r64(dst, dstreg, REG_RAX);                                          // or    dstreg,rax
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_add - process a ADD opcode
//-------------------------------------------------

void drcbe_x64::op_add(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_add_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // add   [dstp],src2p

		// reg = reg + imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBD(src1p.ireg(), src2p.immediate()));   // lea   dstp,[src1p+src2p]

		// reg = reg + reg
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_int_register() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBISD(src1p.ireg(), src2p.ireg(), 1, 0));// lea   dstp,[src1p+src2p]

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_add_r32_p32(dst, dstreg, src2p, inst);                                 // add   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_add_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // add   [dstp],src2p

		// reg = reg + imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && short_immediate(src2p.immediate()) && inst.flags() == 0)
			emit_lea_r64_m64(dst, dstp.ireg(), MBD(src1p.ireg(), src2p.immediate()));   // lea   dstp,[src1p+src2p]

		// reg = reg + reg
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_int_register() && inst.flags() == 0)
			emit_lea_r64_m64(dst, dstp.ireg(), MBISD(src1p.ireg(), src2p.ireg(), 1, 0));// lea   dstp,[src1p+src2p]

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_add_r64_p64(dst, dstreg, src2p, inst);                                 // add   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_addc - process a ADDC opcode
//-------------------------------------------------

void drcbe_x64::op_addc(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_adc_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // adc   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_adc_r32_p32(dst, dstreg, src2p, inst);                                 // adc   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_adc_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // adc   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_adc_r64_p64(dst, dstreg, src2p, inst);                                 // adc   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_sub - process a SUB opcode
//-------------------------------------------------

void drcbe_x64::op_sub(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sub_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // sub   [dstp],src2p

		// reg = reg - imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBD(src1p.ireg(), -src2p.immediate()));  // lea   dstp,[src1p-src2p]

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_sub_r32_p32(dst, dstreg, src2p, inst);                                 // sub   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sub_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // sub   [dstp],src2p

		// reg = reg - imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && short_immediate(src2p.immediate()) && inst.flags() == 0)
			emit_lea_r64_m64(dst, dstp.ireg(), MBD(src1p.ireg(), -src2p.immediate()));  // lea   dstp,[src1p-src2p]

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_sub_r64_p64(dst, dstreg, src2p, inst);                                 // sub   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_subc - process a SUBC opcode
//-------------------------------------------------

void drcbe_x64::op_subc(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sbb_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // sbb   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_sbb_r32_p32(dst, dstreg, src2p, inst);                                 // sbb   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sbb_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // sbb   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_sbb_r64_p64(dst, dstreg, src2p, inst);                                 // sbb   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_cmp - process a CMP opcode
//-------------------------------------------------

void drcbe_x64::op_cmp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	int src1reg = src1p.select_register(REG_EAX);

	// 32-bit form
	if (inst.size() == 4)
	{
		// memory versus anything
		if (src1p.is_memory())
			emit_cmp_m32_p32(dst, MABS(src1p.memory()), src2p, inst);                   // cmp   [dstp],src2p

		// general case
		else
		{
			if (src1p.is_immediate())
				emit_mov_r32_imm(dst, src1reg, src1p.immediate());                      // mov   src1reg,imm
			emit_cmp_r32_p32(dst, src1reg, src2p, inst);                                // cmp   src1reg,src2p
		}
	}

	// 64-bit form
	else
	{
		// memory versus anything
		if (src1p.is_memory())
			emit_cmp_m64_p64(dst, MABS(src1p.memory()), src2p, inst);                   // cmp   [dstp],src2p

		// general case
		else
		{
			if (src1p.is_immediate())
				emit_mov_r64_imm(dst, src1reg, src1p.immediate());                      // mov   src1reg,imm
			emit_cmp_r64_p64(dst, src1reg, src2p, inst);                                // cmp   src1reg,src2p
		}
	}
}


//-------------------------------------------------
//  op_mulu - process a MULU opcode
//-------------------------------------------------

void drcbe_x64::op_mulu(x86code *&dst, const instruction &inst)
{
	UINT8 zsflags = inst.flags() & (FLAG_Z | FLAG_S);
	UINT8 vflag = inst.flags() & FLAG_V;

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
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                          // mov   eax,src1p
		if (src2p.is_memory())
			emit_mul_m32(dst, MABS(src2p.memory()));                                    // mul   [src2p]
		else if (src2p.is_int_register())
			emit_mul_r32(dst, src2p.ireg());                                            // mul   src2p
		else if (src2p.is_immediate())
		{
			emit_mov_r32_imm(dst, REG_EDX, src2p.immediate());                          // mov   edx,src2p
			emit_mul_r32(dst, REG_EDX);                                                 // mul   edx
		}
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                           // mov   dstp,eax
		if (compute_hi)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                                      // mov   edstp,edx

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(dst);                                                    // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                         // or    edx,eax
					else if (zsflags == FLAG_S)
						emit_test_r32_r32(dst, REG_EDX, REG_EDX);                       // test  edx,edx
					else
					{
						emit_movzx_r32_r16(dst, REG_ECX, REG_AX);                       // movzx ecx,ax
						emit_shr_r32_imm(dst, REG_EAX, 16);                             // shr   eax,16
						emit_or_r32_r32(dst, REG_EDX, REG_ECX);                         // or    edx,ecx
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                         // or    edx,eax
					}
				}
				else
					emit_test_r32_r32(dst, REG_EAX, REG_EAX);                           // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r64(dst, REG_RAX);                                         // pop   rax
					emit_and_m64_imm(dst, MBD(REG_RSP, 0), ~0x84);                      // and   [rsp],~0x84
					emit_or_m64_r64(dst, MBD(REG_RSP, 0), REG_RAX);                     // or    [rsp],rax
					emit_popf(dst);                                                     // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, REG_RAX, src1p);                                          // mov   rax,src1p
		if (src2p.is_memory())
			emit_mul_m64(dst, MABS(src2p.memory()));                                    // mul   [src2p]
		else if (src2p.is_int_register())
			emit_mul_r64(dst, src2p.ireg());                                            // mul   src2p
		else if (src2p.is_immediate())
		{
			emit_mov_r64_imm(dst, REG_RDX, src2p.immediate());                          // mov   rdx,src2p
			emit_mul_r64(dst, REG_RDX);                                                 // mul   rdx
		}
		emit_mov_p64_r64(dst, dstp, REG_RAX);                                           // mov   dstp,rax
		if (compute_hi)
			emit_mov_p64_r64(dst, edstp, REG_RDX);                                      // mov   edstp,rdx

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(dst);                                                    // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						emit_or_r64_r64(dst, REG_RDX, REG_RAX);                         // or    rdx,rax
					else if (zsflags == FLAG_S)
						emit_test_r64_r64(dst, REG_RDX, REG_RDX);                       // test  rdx,rdx
					else
					{
						emit_mov_r32_r32(dst, REG_ECX, REG_EAX);                        // mov   ecx,eax
						emit_shr_r64_imm(dst, REG_RAX, 32);                             // shr   rax,32
						emit_or_r64_r64(dst, REG_RDX, REG_RCX);                         // or    rdx,rcx
						emit_or_r64_r64(dst, REG_RDX, REG_RAX);                         // or    rdx,rax
					}
				}
				else
					emit_test_r64_r64(dst, REG_RAX, REG_RAX);                           // test  rax,rax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r64(dst, REG_RAX);                                         // pop   rax
					emit_and_m64_imm(dst, MBD(REG_RSP, 0), ~0x84);                      // and   [rsp],~0x84
					emit_or_m64_r64(dst, MBD(REG_RSP, 0), REG_RAX);                     // or    [rsp],rax
					emit_popf(dst);                                                     // popf
				}
			}
		}
	}
}


//-------------------------------------------------
//  op_muls - process a MULS opcode
//-------------------------------------------------

void drcbe_x64::op_muls(x86code *&dst, const instruction &inst)
{
	UINT8 zsflags = inst.flags() & (FLAG_Z | FLAG_S);
	UINT8 vflag =  inst.flags() & FLAG_V;

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
			if (src1p.is_memory())
				emit_imul_r32_m32_imm(dst, REG_EAX, MABS(src1p.memory()), src2p.immediate()); // imul  eax,[src1p],src2p
			else if (src1p.is_int_register())
				emit_imul_r32_r32_imm(dst, REG_EAX, src1p.ireg(), src2p.immediate());   // imul  eax,src1p,src2p
			emit_mov_p32_r32(dst, dstp, REG_EAX);                                       // mov   dstp,eax
		}

		// 32-bit destination, general case
		else if (!compute_hi)
		{
			emit_mov_r32_p32(dst, REG_EAX, src1p);                                      // mov   eax,src1p
			if (src2p.is_memory())
				emit_imul_r32_m32(dst, REG_EAX, MABS(src2p.memory()));                  // imul  eax,[src2p]
			else if (src2p.is_int_register())
				emit_imul_r32_r32(dst, REG_EAX, src2p.ireg());                          // imul  eax,src2p
			emit_mov_p32_r32(dst, dstp, REG_EAX);                                       // mov   dstp,eax
		}

		// 64-bit destination, general case
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, src1p);                                      // mov   eax,src1p
			if (src2p.is_memory())
				emit_imul_m32(dst, MABS(src2p.memory()));                               // imul  [src2p]
			else if (src2p.is_int_register())
				emit_imul_r32(dst, src2p.ireg());                                       // imul  src2p
			else if (src2p.is_immediate())
			{
				emit_mov_r32_imm(dst, REG_EDX, src2p.immediate());                      // mov   edx,src2p
				emit_imul_r32(dst, REG_EDX);                                            // imul  edx
			}
			emit_mov_p32_r32(dst, dstp, REG_EAX);                                       // mov   dstp,eax
			emit_mov_p32_r32(dst, edstp, REG_EDX);                                      // mov   edstp,edx
		}

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(dst);                                                    // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                         // or    edx,eax
					else if (zsflags == FLAG_S)
						emit_test_r32_r32(dst, REG_EDX, REG_EDX);                       // test  edx,edx
					else
					{
						emit_movzx_r32_r16(dst, REG_ECX, REG_AX);                       // movzx ecx,ax
						emit_shr_r32_imm(dst, REG_EAX, 16);                             // shr   eax,16
						emit_or_r32_r32(dst, REG_EDX, REG_ECX);                         // or    edx,ecx
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                         // or    edx,eax
					}
				}
				else
					emit_test_r32_r32(dst, REG_EAX, REG_EAX);                           // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r64(dst, REG_RAX);                                         // pop   rax
					emit_and_m64_imm(dst, MBD(REG_RSP, 0), ~0x84);                      // and   [rsp],~0x84
					emit_or_m64_r64(dst, MBD(REG_RSP, 0), REG_RAX);                     // or    [rsp],rax
					emit_popf(dst);                                                     // popf
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
			if (src1p.is_memory())
				emit_imul_r64_m64_imm(dst, REG_RAX, MABS(src1p.memory()), src2p.immediate());// imul  rax,[src1p],src2p
			else if (src1p.is_int_register())
				emit_imul_r64_r64_imm(dst, REG_RAX, src1p.ireg(), src2p.immediate());   // imul  rax,src1p,src2p
			emit_mov_p64_r64(dst, dstp, REG_RAX);                                       // mov   dstp,rax
		}

		// 64-bit destination, general case
		else if (!compute_hi)
		{
			emit_mov_r64_p64(dst, REG_RAX, src1p);                                      // mov   rax,src1p
			if (src2p.is_memory())
				emit_imul_r64_m64(dst, REG_RAX, MABS(src2p.memory()));                  // imul  rax,[src2p]
			else if (src2p.is_int_register())
				emit_imul_r64_r64(dst, REG_RAX, src2p.ireg());                          // imul  rax,src2p
			emit_mov_p64_r64(dst, dstp, REG_RAX);                                       // mov   dstp,rax
		}

		// 128-bit destination, general case
		else
		{
			emit_mov_r64_p64(dst, REG_RAX, src1p);                                      // mov   rax,src1p
			if (src2p.is_memory())
				emit_imul_m64(dst, MABS(src2p.memory()));                               // imul  [src2p]
			else if (src2p.is_int_register())
				emit_imul_r64(dst, src2p.ireg());                                       // imul  src2p
			else if (src2p.is_immediate())
			{
				emit_mov_r64_imm(dst, REG_RDX, src2p.immediate());                      // mov   rdx,src2p
				emit_imul_r64(dst, REG_RDX);                                            // imul  rdx
			}
			emit_mov_p64_r64(dst, dstp, REG_RAX);                                       // mov   dstp,rax
			emit_mov_p64_r64(dst, edstp, REG_RDX);                                      // mov   edstp,rdx
		}

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(dst);                                                    // pushf
				if (compute_hi)
				{
					if (zsflags == FLAG_Z)
						emit_or_r64_r64(dst, REG_RDX, REG_RAX);                         // or    rdx,rax
					else if (zsflags == FLAG_S)
						emit_test_r64_r64(dst, REG_RDX, REG_RDX);                       // test  rdx,rdx
					else
					{
						emit_mov_r32_r32(dst, REG_ECX, REG_EAX);                        // mov   ecx,eax
						emit_shr_r64_imm(dst, REG_RAX, 32);                             // shr   rax,32
						emit_or_r64_r64(dst, REG_RDX, REG_RCX);                         // or    rdx,rcx
						emit_or_r64_r64(dst, REG_RDX, REG_RAX);                         // or    rdx,rax
					}
				}
				else
					emit_test_r64_r64(dst, REG_RAX, REG_RAX);                           // test  rax,rax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r64(dst, REG_RAX);                                         // pop   rax
					emit_and_m64_imm(dst, MBD(REG_RSP, 0), ~0x84);                      // and   [rsp],~0x84
					emit_or_m64_r64(dst, MBD(REG_RSP, 0), REG_RAX);                     // or    [rsp],rax
					emit_popf(dst);                                                     // popf
				}
			}
		}
	}
}


//-------------------------------------------------
//  op_divu - process a DIVU opcode
//-------------------------------------------------

void drcbe_x64::op_divu(x86code *&dst, const instruction &inst)
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

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		emit_mov_r32_p32(dst, REG_ECX, src2p);                                          // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                                 // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jecxz_link(dst, skip);                                                     // jecxz skip
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                          // mov   eax,src1p
		emit_xor_r32_r32(dst, REG_EDX, REG_EDX);                                        // xor   edx,edx
		emit_div_r32(dst, REG_ECX);                                                     // div   ecx
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                           // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                                      // mov   edstp,edx
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, REG_EAX, REG_EAX);                                   // test  eax,eax
		resolve_link(dst, skip);                                                    // skip:
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, REG_RCX, src2p);                                          // mov   rcx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                                 // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jrcxz_link(dst, skip);                                                     // jrcxz skip
		emit_mov_r64_p64(dst, REG_RAX, src1p);                                          // mov   rax,src1p
		emit_xor_r32_r32(dst, REG_EDX, REG_EDX);                                        // xor   edx,edx
		emit_div_r64(dst, REG_RCX);                                                     // div   rcx
		emit_mov_p64_r64(dst, dstp, REG_RAX);                                           // mov   dstp,rax
		if (compute_rem)
			emit_mov_p64_r64(dst, edstp, REG_RDX);                                      // mov   edstp,rdx
		if (inst.flags() != 0)
			emit_test_r64_r64(dst, REG_RAX, REG_RAX);                                   // test  eax,eax
		resolve_link(dst, skip);                                                    // skip:
	}
}


//-------------------------------------------------
//  op_divs - process a DIVS opcode
//-------------------------------------------------

void drcbe_x64::op_divs(x86code *&dst, const instruction &inst)
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

	// 32-bit form
	if (inst.size() == 4)
	{
		// general case
		emit_mov_r32_p32(dst, REG_ECX, src2p);                                          // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                                 // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jecxz_link(dst, skip);                                                     // jecxz skip
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                          // mov   eax,src1p
		emit_cdq(dst);                                                                  // cdq
		emit_idiv_r32(dst, REG_ECX);                                                    // idiv  ecx
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                           // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                                      // mov   edstp,edx
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, REG_EAX, REG_EAX);                                   // test  eax,eax
		resolve_link(dst, skip);                                                    // skip:
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, REG_RCX, src2p);                                          // mov   rcx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                                 // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jrcxz_link(dst, skip);                                                     // jrcxz skip
		emit_mov_r64_p64(dst, REG_RAX, src1p);                                          // mov   rax,src1p
		emit_cqo(dst);                                                                  // cqo
		emit_idiv_r64(dst, REG_RCX);                                                    // idiv  rcx
		emit_mov_p64_r64(dst, dstp, REG_RAX);                                           // mov   dstp,rax
		if (compute_rem)
			emit_mov_p64_r64(dst, edstp, REG_RDX);                                      // mov   edstp,rdx
		if (inst.flags() != 0)
			emit_test_r64_r64(dst, REG_RAX, REG_RAX);                                   // test  eax,eax
		resolve_link(dst, skip);                                                    // skip:
	}
}


//-------------------------------------------------
//  op_and - process a AND opcode
//-------------------------------------------------

void drcbe_x64::op_and(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_and_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // and   [dstp],src2p

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r8(dst, dstreg, src1p.ireg());                           // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m8(dst, dstreg, MABS(src1p.memory()));                   // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r16(dst, dstreg, src1p.ireg());                          // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m16(dst, dstreg, MABS(src1p.memory()));                  // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_and_r32_p32(dst, dstreg, src2p, inst);                                 // and   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_and_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // and   [dstp],src2p

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r8(dst, dstreg, src1p.ireg());                           // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m8(dst, dstreg, MABS(src1p.memory()));                   // movzx dstreg,[src1p]
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r16(dst, dstreg, src1p.ireg());                          // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m16(dst, dstreg, MABS(src1p.memory()));                  // movzx dstreg,[src1p]
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}

		// AND with immediate 0xffffffff
		else if (src2p.is_immediate_value(0xffffffff) && inst.flags() == 0)
		{
			if (dstp.is_int_register() && src1p == dstp)
				emit_mov_r32_r32(dst, dstreg, dstreg);                                  // mov   dstreg,dstreg
			else
			{
				emit_mov_r32_p32(dst, dstreg, src1p);                                   // mov   dstreg,src1p
				emit_mov_p64_r64(dst, dstp, dstreg);                                    // mov   dstp,dstreg
			}
		}

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_and_r64_p64(dst, dstreg, src2p, inst);                                 // and   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_test - process a TEST opcode
//-------------------------------------------------

void drcbe_x64::op_test(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// pick a target register for the general case
	int src1reg = src1p.select_register(REG_EAX);

	// 32-bit form
	if (inst.size() == 4)
	{
		// src1p in memory
		if (src1p.is_memory())
			emit_test_m32_p32(dst, MABS(src1p.memory()), src2p, inst);                  // test  [src1p],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, src1reg, src1p);                                      // mov   src1reg,src1p
			emit_test_r32_p32(dst, src1reg, src2p, inst);                               // test  src1reg,src2p
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// src1p in memory
		if (src1p.is_memory())
			emit_test_m64_p64(dst, MABS(src1p.memory()), src2p, inst);                  // test  [src1p],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, src1reg, src1p);                                      // mov   src1reg,src1p
			emit_test_r64_p64(dst, src1reg, src2p, inst);                               // test  src1reg,src2p
		}
	}
}


//-------------------------------------------------
//  op_or - process a OR opcode
//-------------------------------------------------

void drcbe_x64::op_or(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_or_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                     // or    [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_or_r32_p32(dst, dstreg, src2p, inst);                                  // or    dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_or_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                     // or    [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_or_r64_p64(dst, dstreg, src2p, inst);                                  // or    dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_xor - process a XOR opcode
//-------------------------------------------------

void drcbe_x64::op_xor(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_xor_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // xor   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_xor_r32_p32(dst, dstreg, src2p, inst);                                 // xor   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_xor_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // xor   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_xor_r64_p64(dst, dstreg, src2p, inst);                                 // xor   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_lzcnt - process a LZCNT opcode
//-------------------------------------------------

void drcbe_x64::op_lzcnt(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(dst, dstreg, srcp);                                            // mov   dstreg,src1p
		emit_mov_r32_imm(dst, REG_ECX, 32 ^ 31);                                        // mov   ecx,32 ^ 31
		emit_bsr_r32_r32(dst, dstreg, dstreg);                                          // bsr   dstreg,dstreg
		emit_cmovcc_r32_r32(dst, x64emit::COND_Z, dstreg, REG_ECX);                             // cmovz dstreg,ecx
		emit_xor_r32_imm(dst, dstreg, 31);                                              // xor   dstreg,31
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, dstreg, srcp);                                            // mov   dstreg,src1p
		emit_mov_r64_imm(dst, REG_RCX, 64 ^ 63);                                        // mov   rcx,64 ^ 63
		emit_bsr_r64_r64(dst, dstreg, dstreg);                                          // bsr   dstreg,dstreg
		emit_cmovcc_r64_r64(dst, x64emit::COND_Z, dstreg, REG_RCX);                             // cmovz dstreg,rcx
		emit_xor_r32_imm(dst, dstreg, 63);                                              // xor   dstreg,63
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_bswap - process a BSWAP opcode
//-------------------------------------------------

void drcbe_x64::op_bswap(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_RAX);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(dst, dstreg, srcp);                                            // mov   dstreg,src1p
		emit_bswap_r32(dst, dstreg);                                                    // bswap dstreg
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, dstreg, dstreg);                                     // test  dstreg,dstreg
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, dstreg, srcp);                                            // mov   dstreg,src1p
		emit_bswap_r64(dst, dstreg);                                                    // bswap dstreg
		if (inst.flags() != 0)
			emit_test_r64_r64(dst, dstreg, dstreg);                                     // test  dstreg,dstreg
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	}
}


//-------------------------------------------------
//  op_shl - process a SHL opcode
//-------------------------------------------------

void drcbe_x64::op_shl(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_shl_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // shl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_shl_r32_p32(dst, dstreg, src2p, inst);                                 // shl   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_shl_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // shl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_shl_r64_p64(dst, dstreg, src2p, inst);                                 // shl   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_shr - process a SHR opcode
//-------------------------------------------------

void drcbe_x64::op_shr(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_shr_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // shr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_shr_r32_p32(dst, dstreg, src2p, inst);                                 // shr   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_shr_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // shr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_shr_r64_p64(dst, dstreg, src2p, inst);                                 // shr   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_sar - process a SAR opcode
//-------------------------------------------------

void drcbe_x64::op_sar(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sar_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // sar   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_sar_r32_p32(dst, dstreg, src2p, inst);                                 // sar   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sar_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // sar   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_sar_r64_p64(dst, dstreg, src2p, inst);                                 // sar   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_rol - process a rol opcode
//-------------------------------------------------

void drcbe_x64::op_rol(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rol_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // rol   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_rol_r32_p32(dst, dstreg, src2p, inst);                                 // rol   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rol_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // rol   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_rol_r64_p64(dst, dstreg, src2p, inst);                                 // rol   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_ror - process a ROR opcode
//-------------------------------------------------

void drcbe_x64::op_ror(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_ror_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // ror   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_ror_r32_p32(dst, dstreg, src2p, inst);                                 // ror   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_ror_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // ror   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, src1p);                                       // mov   dstreg,src1p
			emit_ror_r64_p64(dst, dstreg, src2p, inst);                                 // ror   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_rolc - process a ROLC opcode
//-------------------------------------------------

void drcbe_x64::op_rolc(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rcl_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // rcl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_rcl_r32_p32(dst, dstreg, src2p, inst);                                 // rcl   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rcl_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // rcl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_rcl_r64_p64(dst, dstreg, src2p, inst);                                 // rcl   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}


//-------------------------------------------------
//  op_rorc - process a RORC opcode
//-------------------------------------------------

void drcbe_x64::op_rorc(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rcr_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                    // rcr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_rcr_r32_p32(dst, dstreg, src2p, inst);                                 // rcr   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_rcr_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                    // rcr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, src1p);                             // mov   dstreg,src1p
			emit_rcr_r64_p64(dst, dstreg, src2p, inst);                                 // rcr   dstreg,src2p
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov   dstp,dstreg
		}
	}
}



/***************************************************************************
    FLOATING POINT OPERATIONS
***************************************************************************/

//-------------------------------------------------
//  op_fload - process a FLOAD opcode
//-------------------------------------------------

void drcbe_x64::op_fload(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0);

	// determine the pointer base
	INT32 baseoffs;
	int basereg = get_base_register_and_offset(dst, basep.memory(), REG_RDX, baseoffs);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (indp.is_immediate())
			emit_movss_r128_m32(dst, dstreg, MBD(basereg, baseoffs + 4*indp.immediate()));  // movss  dstreg,[basep + 4*indp]
		else
		{
			int indreg = indp.select_register(REG_ECX);
			emit_mov_r32_p32(dst, indreg, indp);                                        // mov    indreg,indp
			emit_movss_r128_m32(dst, dstreg, MBISD(basereg, indreg, 4, baseoffs));      // movss  dstreg,[basep + 4*indp]
		}
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss  dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (indp.is_immediate())
			emit_movsd_r128_m64(dst, dstreg, MBD(basereg, baseoffs + 8*indp.immediate()));  // movsd  dstreg,[basep + 8*indp]
		else
		{
			int indreg = indp.select_register(REG_ECX);
			emit_mov_r32_p32(dst, indreg, indp);                                        // mov    indreg,indp
			emit_movsd_r128_m64(dst, dstreg, MBISD(basereg, indreg, 8, baseoffs));      // movsd  dstreg,[basep + 8*indp]
		}
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd  dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fstore - process a FSTORE opcode
//-------------------------------------------------

void drcbe_x64::op_fstore(x86code *&dst, const instruction &inst)
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
	int srcreg = srcp.select_register(REG_XMM0);

	// determine the pointer base
	INT32 baseoffs;
	int basereg = get_base_register_and_offset(dst, basep.memory(), REG_RDX, baseoffs);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, srcreg, srcp);                                         // movss  srcreg,srcp
		if (indp.is_immediate())
			emit_movss_m32_r128(dst, MBD(basereg, baseoffs + 4*indp.immediate()), srcreg);  // movss  [basep + 4*indp],srcreg
		else
		{
			int indreg = indp.select_register(REG_ECX);
			emit_mov_r32_p32(dst, indreg, indp);                                        // mov    indreg,indp
			emit_movss_m32_r128(dst, MBISD(basereg, indreg, 4, baseoffs), srcreg);      // movss  [basep + 4*indp],srcreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, srcreg, srcp);                                         // movsd  srcreg,srcp
		if (indp.is_immediate())
			emit_movsd_m64_r128(dst, MBD(basereg, baseoffs + 8*indp.immediate()), srcreg);  // movsd  [basep + 8*indp],srcreg
		else
		{
			int indreg = indp.select_register(REG_ECX);
			emit_mov_r32_p32(dst, indreg, indp);                                        // mov    indreg,indp
			emit_movsd_m64_r128(dst, MBISD(basereg, indreg, 8, baseoffs), srcreg);      // movsd  [basep + 8*indp],srcreg
		}
	}
}


//-------------------------------------------------
//  op_fread - process a FREAD opcode
//-------------------------------------------------

void drcbe_x64::op_fread(x86code *&dst, const instruction &inst)
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
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacep.space()]));                 // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param2,addrp
	if (inst.size() == 4)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacep.space()].read_dword);  // call   read_dword
	else if (inst.size() == 8)
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacep.space()].read_qword);  // call   read_qword

	// store result
	if (inst.size() == 4)
	{
		if (dstp.is_memory())
			emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                        // mov   [dstp],eax
		else if (dstp.is_float_register())
			emit_movd_r128_r32(dst, dstp.freg(), REG_EAX);                              // movd  dstp,eax
	}
	else if (inst.size() == 8)
	{
		if (dstp.is_memory())
			emit_mov_m64_r64(dst, MABS(dstp.memory()), REG_RAX);                        // mov   [dstp],rax
		else if (dstp.is_float_register())
			emit_movq_r128_r64(dst, dstp.freg(), REG_RAX);                              // movq  dstp,rax
	}
}


//-------------------------------------------------
//  op_fwrite - process a FWRITE opcode
//-------------------------------------------------

void drcbe_x64::op_fwrite(x86code *&dst, const instruction &inst)
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
	emit_mov_r64_imm(dst, REG_PARAM1, (FPTR)(m_space[spacep.space()]));                 // mov    param1,space
	emit_mov_r32_p32(dst, REG_PARAM2, addrp);                                           // mov    param21,addrp

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
			emit_mov_r32_m32(dst, REG_PARAM3, MABS(srcp.memory()));                     // mov    param3,[srcp]
		else if (srcp.is_float_register())
			emit_movd_r32_r128(dst, REG_PARAM3, srcp.freg());                           // movd   param3,srcp
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacep.space()].write_dword); // call   write_dword
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
			emit_mov_r64_m64(dst, REG_PARAM3, MABS(srcp.memory()));                     // mov    param3,[srcp]
		else if (srcp.is_float_register())
			emit_movq_r64_r128(dst, REG_PARAM3, srcp.freg());                           // movq   param3,srcp
		emit_smart_call_m64(dst, (x86code **)&m_accessors[spacep.space()].write_qword); // call   write_qword
	}
}


//-------------------------------------------------
//  op_fmov - process a FMOV opcode
//-------------------------------------------------

void drcbe_x64::op_fmov(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// always start with a jmp
	emit_link skip = { nullptr };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, srcp);                                         // movss dstreg,srcp
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, srcp);                                         // movsd dstreg,srcp
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}

	// resolve the jump
	if (inst.condition() != uml::COND_ALWAYS)
		resolve_link(dst, skip);                                                    // skip:
}


//-------------------------------------------------
//  op_ftoint - process a FTOINT opcode
//-------------------------------------------------

void drcbe_x64::op_ftoint(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_EAX);

	// set rounding mode if necessary
	if (roundp.rounding() != ROUND_DEFAULT && roundp.rounding() != ROUND_TRUNC)
	{
		emit_stmxcsr_m32(dst, MABS(&m_near.ssemodesave));                               // stmxcsr [ssemodesave]
		emit_ldmxcsr_m32(dst, MABS(&m_near.ssecontrol[roundp.rounding()]));             // ldmxcsr fpcontrol[mode]
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtss2si_r32_m32(dst, dstreg, MABS(srcp.memory()));            // cvtss2si dstreg,[srcp]
				else
					emit_cvttss2si_r32_m32(dst, dstreg, MABS(srcp.memory()));           // cvttss2si dstreg,[srcp]
			}
			else if (srcp.is_float_register())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtss2si_r32_r128(dst, dstreg, srcp.freg());                   // cvtss2si dstreg,srcp
				else
					emit_cvttss2si_r32_r128(dst, dstreg, srcp.freg());                  // cvttss2si dstreg,srcp
			}
		}

		// 64-bit integer source
		else if (sizep.size() == SIZE_QWORD)
		{
			if (srcp.is_memory())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtss2si_r64_m32(dst, dstreg, MABS(srcp.memory()));            // cvtss2si dstreg,[srcp]
				else
					emit_cvttss2si_r64_m32(dst, dstreg, MABS(srcp.memory()));           // cvttss2si dstreg,[srcp]
			}
			else if (srcp.is_float_register())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtss2si_r64_r128(dst, dstreg, srcp.freg());                   // cvtss2si dstreg,srcp
				else
					emit_cvttss2si_r64_r128(dst, dstreg, srcp.freg());                  // cvttss2si dstreg,srcp
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtsd2si_r32_m64(dst, dstreg, MABS(srcp.memory()));            // cvtsd2si dstreg,[srcp]
				else
					emit_cvttsd2si_r32_m64(dst, dstreg, MABS(srcp.memory()));           // cvttsd2si dstreg,[srcp]
			}
			else if (srcp.is_float_register())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtsd2si_r32_r128(dst, dstreg, srcp.freg());                   // cvtsd2si dstreg,srcp
				else
					emit_cvttsd2si_r32_r128(dst, dstreg, srcp.freg());                  // cvttsd2si dstreg,srcp
			}
		}

		// 64-bit integer source
		else if (sizep.size() == SIZE_QWORD)
		{
			if (srcp.is_memory())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtsd2si_r64_m64(dst, dstreg, MABS(srcp.memory()));            // cvtsd2si dstreg,[srcp]
				else
					emit_cvttsd2si_r64_m64(dst, dstreg, MABS(srcp.memory()));           // cvttsd2si dstreg,[srcp]
			}
			else if (srcp.is_float_register())
			{
				if (roundp.rounding() != ROUND_TRUNC)
					emit_cvtsd2si_r64_r128(dst, dstreg, srcp.freg());                   // cvtsd2si dstreg,srcp
				else
					emit_cvttsd2si_r64_r128(dst, dstreg, srcp.freg());                  // cvttsd2si dstreg,srcp
			}
		}
	}

	// general case
	if (sizep.size() == SIZE_DWORD)
		emit_mov_p32_r32(dst, dstp, dstreg);                                            // mov   dstp,dstreg
	else
		emit_mov_p64_r64(dst, dstp, dstreg);                                            // mov   dstp,dstreg

	// restore rounding mode
	if (roundp.rounding() != ROUND_DEFAULT && roundp.rounding() != ROUND_TRUNC)
		emit_ldmxcsr_m32(dst, MABS(&m_near.ssemodesave));                               // ldmxcsr [ssemodesave]
}


//-------------------------------------------------
//  op_ffrint - process a FFRINT opcode
//-------------------------------------------------

void drcbe_x64::op_ffrint(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
				emit_cvtsi2ss_r128_m32(dst, dstreg, MABS(srcp.memory()));               // cvtsi2ss dstreg,[srcp]
			else
			{
				int srcreg = srcp.select_register(REG_EAX);
				emit_mov_r32_p32(dst, srcreg, srcp);                                    // mov      srcreg,srcp
				emit_cvtsi2ss_r128_r32(dst, dstreg, srcreg);                            // cvtsi2ss dstreg,srcreg
			}
		}

		// 64-bit integer source
		else
		{
			if (srcp.is_memory())
				emit_cvtsi2ss_r128_m64(dst, dstreg, MABS(srcp.memory()));               // cvtsi2ss dstreg,[srcp]
			else
			{
				int srcreg = srcp.select_register(REG_RAX);
				emit_mov_r64_p64(dst, srcreg, srcp);                                    // mov      srcreg,srcp
				emit_cvtsi2ss_r128_r64(dst, dstreg, srcreg);                            // cvtsi2ss dstreg,srcreg
			}
		}
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss    dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// 32-bit integer source
		if (sizep.size() == SIZE_DWORD)
		{
			if (srcp.is_memory())
				emit_cvtsi2sd_r128_m32(dst, dstreg, MABS(srcp.memory()));               // cvtsi2sd dstreg,[srcp]
			else
			{
				int srcreg = srcp.select_register(REG_EAX);
				emit_mov_r32_p32(dst, srcreg, srcp);                                    // mov      srcreg,srcp
				emit_cvtsi2sd_r128_r32(dst, dstreg, srcreg);                            // cvtsi2sd dstreg,srcreg
			}
		}

		// 64-bit integer source
		else
		{
			if (srcp.is_memory())
				emit_cvtsi2sd_r128_m64(dst, dstreg, MABS(srcp.memory()));               // cvtsi2sd dstreg,[srcp]
			else
			{
				int srcreg = srcp.select_register(REG_EAX);
				emit_mov_r64_p64(dst, srcreg, srcp);                                    // mov      srcreg,srcp
				emit_cvtsi2sd_r128_r64(dst, dstreg, srcreg);                            // cvtsi2sd dstreg,srcreg
			}
		}
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd    dstp,dstreg
	}
}


//-------------------------------------------------
//  op_ffrflt - process a FFRFLT opcode
//-------------------------------------------------

void drcbe_x64::op_ffrflt(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0);

	// single-to-double
	if (inst.size() == 8 && sizep.size() == SIZE_DWORD)
	{
		if (srcp.is_memory())
			emit_cvtss2sd_r128_m32(dst, dstreg, MABS(srcp.memory()));                   // cvtss2sd dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_cvtss2sd_r128_r128(dst, dstreg, srcp.freg());                          // cvtss2sd dstreg,srcp
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd    dstp,dstreg
	}

	// double-to-single
	else if (inst.size() == 4 && sizep.size() == SIZE_QWORD)
	{
		if (srcp.is_memory())
			emit_cvtsd2ss_r128_m64(dst, dstreg, MABS(srcp.memory()));                   // cvtsd2ss dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_cvtsd2ss_r128_r128(dst, dstreg, srcp.freg());                          // cvtsd2ss dstreg,srcp
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss    dstp,dstreg
	}
}


//-------------------------------------------------
//  op_frnds - process a FRNDS opcode
//-------------------------------------------------

void drcbe_x64::op_frnds(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// 64-bit form
	if (srcp.is_memory())
		emit_cvtsd2ss_r128_m64(dst, dstreg, MABS(srcp.memory()));                       // cvtsd2ss dstreg,[srcp]
	else if (srcp.is_float_register())
		emit_cvtsd2ss_r128_r128(dst, dstreg, srcp.freg());                              // cvtsd2ss dstreg,srcp
	emit_cvtss2sd_r128_r128(dst, dstreg, dstreg);                                       // cvtss2sd dstreg,dstreg
	emit_movsd_p64_r128(dst, dstp, dstreg);                                             // movsd    dstp,dstreg
}


//-------------------------------------------------
//  op_fadd - process a FADD opcode
//-------------------------------------------------

void drcbe_x64::op_fadd(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, src1p);                                        // movss dstreg,src1p
		if (src2p.is_memory())
			emit_addss_r128_m32(dst, dstreg, MABS(src2p.memory()));                     // addss dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_addss_r128_r128(dst, dstreg, src2p.freg());                            // addss dstreg,src2p
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, src1p);                                        // movsd dstreg,src1p
		if (src2p.is_memory())
			emit_addsd_r128_m64(dst, dstreg, MABS(src2p.memory()));                     // addsd dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_addsd_r128_r128(dst, dstreg, src2p.freg());                            // addsd dstreg,src2p
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fsub - process a FSUB opcode
//-------------------------------------------------

void drcbe_x64::op_fsub(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, src1p);                                        // movss dstreg,src1p
		if (src2p.is_memory())
			emit_subss_r128_m32(dst, dstreg, MABS(src2p.memory()));                     // subss dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_subss_r128_r128(dst, dstreg, src2p.freg());                            // subss dstreg,src2p
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, src1p);                                        // movsd dstreg,src1p
		if (src2p.is_memory())
			emit_subsd_r128_m64(dst, dstreg, MABS(src2p.memory()));                     // subsd dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_subsd_r128_r128(dst, dstreg, src2p.freg());                            // subsd dstreg,src2p
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fcmp - process a FCMP opcode
//-------------------------------------------------

void drcbe_x64::op_fcmp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_U);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MF);
	be_parameter src2p(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int src1reg = src1p.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, src1reg, src1p);                                       // movss src1reg,src1p
		if (src2p.is_memory())
			emit_comiss_r128_m32(dst, src1reg, MABS(src2p.memory()));                   // comiss src1reg,[src2p]
		else if (src2p.is_float_register())
			emit_comiss_r128_r128(dst, src1reg, src2p.freg());                          // comiss src1reg,src2p
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, src1reg, src1p);                                       // movsd src1reg,src1p
		if (src2p.is_memory())
			emit_comisd_r128_m64(dst, src1reg, MABS(src2p.memory()));                   // comisd src1reg,[src2p]
		else if (src2p.is_float_register())
			emit_comisd_r128_r128(dst, src1reg, src2p.freg());                          // comisd src1reg,src2p
	}
}


//-------------------------------------------------
//  op_fmul - process a FMUL opcode
//-------------------------------------------------

void drcbe_x64::op_fmul(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, src1p);                                        // movss dstreg,src1p
		if (src2p.is_memory())
			emit_mulss_r128_m32(dst, dstreg, MABS(src2p.memory()));                     // mulss dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_mulss_r128_r128(dst, dstreg, src2p.freg());                            // mulss dstreg,src2p
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, src1p);                                        // movsd dstreg,src1p
		if (src2p.is_memory())
			emit_mulsd_r128_m64(dst, dstreg, MABS(src2p.memory()));                     // mulsd dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_mulsd_r128_r128(dst, dstreg, src2p.freg());                            // mulsd dstreg,src2p
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fdiv - process a FDIV opcode
//-------------------------------------------------

void drcbe_x64::op_fdiv(x86code *&dst, const instruction &inst)
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
	int dstreg = dstp.select_register(REG_XMM0, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, src1p);                                        // movss dstreg,src1p
		if (src2p.is_memory())
			emit_divss_r128_m32(dst, dstreg, MABS(src2p.memory()));                     // divss dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_divss_r128_r128(dst, dstreg, src2p.freg());                            // divss dstreg,src2p
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, src1p);                                        // movsd dstreg,src1p
		if (src2p.is_memory())
			emit_divsd_r128_m64(dst, dstreg, MABS(src2p.memory()));                     // divsd dstreg,[src2p]
		else if (src2p.is_float_register())
			emit_divsd_r128_r128(dst, dstreg, src2p.freg());                            // divsd dstreg,src2p
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fneg - process a FNEG opcode
//-------------------------------------------------

void drcbe_x64::op_fneg(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0, srcp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_xorps_r128_r128(dst, dstreg, dstreg);                                      // xorps dstreg,dstreg
		if (srcp.is_memory())
			emit_subss_r128_m32(dst, dstreg, MABS(srcp.memory()));                      // subss dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_subss_r128_r128(dst, dstreg, srcp.freg());                             // subss dstreg,srcp
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_xorpd_r128_r128(dst, dstreg, dstreg);                                      // xorpd dstreg,dstreg
		if (srcp.is_memory())
			emit_subsd_r128_m64(dst, dstreg, MABS(srcp.memory()));                      // subsd dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_subsd_r128_r128(dst, dstreg, srcp.freg());                             // subsd dstreg,srcp
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fabs - process a FABS opcode
//-------------------------------------------------

void drcbe_x64::op_fabs(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0, srcp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_movss_r128_p32(dst, dstreg, srcp);                                         // movss dstreg,srcp
		emit_andps_r128_m128(dst, dstreg, MABS(m_absmask32));                           // andps dstreg,[absmask32]
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_movsd_r128_p64(dst, dstreg, srcp);                                         // movsd dstreg,srcp
		emit_andpd_r128_m128(dst, dstreg, MABS(m_absmask64));                           // andpd dstreg,[absmask64]
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fsqrt - process a FSQRT opcode
//-------------------------------------------------

void drcbe_x64::op_fsqrt(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
			emit_sqrtss_r128_m32(dst, dstreg, MABS(srcp.memory()));                     // sqrtss dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_sqrtss_r128_r128(dst, dstreg, srcp.freg());                            // sqrtss dstreg,srcp
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
			emit_sqrtsd_r128_m64(dst, dstreg, MABS(srcp.memory()));                     // sqrtsd dstreg,[srcp]
		else if (srcp.is_float_register())
			emit_sqrtsd_r128_r128(dst, dstreg, srcp.freg());                            // sqrtsd dstreg,srcp
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_frecip - process a FRECIP opcode
//-------------------------------------------------

void drcbe_x64::op_frecip(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (USE_RCPSS_FOR_SINGLES)
		{
			if (srcp.is_memory())
				emit_rcpss_r128_m32(dst, dstreg, MABS(srcp.memory()));                  // rcpss dstreg,[srcp]
			else if (srcp.is_float_register())
				emit_rcpss_r128_r128(dst, dstreg, srcp.freg());                         // rcpss dstreg,srcp
			emit_movss_p32_r128(dst, dstp, dstreg);                                     // movss dstp,dstreg
		}
		else
		{
			emit_movss_r128_m32(dst, REG_XMM1, MABS(&m_near.single1));                  // movss xmm1,1.0
			if (srcp.is_memory())
				emit_divss_r128_m32(dst, REG_XMM1, MABS(srcp.memory()));                // divss xmm1,[srcp]
			else if (srcp.is_float_register())
				emit_divss_r128_r128(dst, REG_XMM1, srcp.freg());                       // divss xmm1,srcp
			emit_movss_p32_r128(dst, dstp, REG_XMM1);                                   // movss dstp,xmm1
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (USE_RCPSS_FOR_DOUBLES)
		{
			if (srcp.is_memory())
				emit_cvtsd2ss_r128_m64(dst, dstreg, MABS(srcp.memory()));               // cvtsd2ss dstreg,[srcp]
			else if (srcp.is_float_register())
				emit_cvtsd2ss_r128_r128(dst, dstreg, srcp.freg());                      // cvtsd2ss dstreg,srcp
			emit_rcpss_r128_r128(dst, dstreg, dstreg);                                  // rcpss dstreg,dstreg
			emit_cvtss2sd_r128_r128(dst, dstreg, dstreg);                               // cvtss2sd dstreg,dstreg
			emit_movsd_p64_r128(dst, dstp, REG_XMM1);                                   // movsd dstp,dstreg
		}
		else
		{
			emit_movsd_r128_m64(dst, REG_XMM1, MABS(&m_near.double1));                  // movsd xmm1,1.0
			if (srcp.is_memory())
				emit_divsd_r128_m64(dst, REG_XMM1, MABS(srcp.memory()));                // divsd xmm1,[srcp]
			else if (srcp.is_float_register())
				emit_divsd_r128_r128(dst, REG_XMM1, srcp.freg());                       // divsd xmm1,srcp
			emit_movsd_p64_r128(dst, dstp, REG_XMM1);                                   // movsd dstp,xmm1
		}
	}
}


//-------------------------------------------------
//  op_frsqrt - process a FRSQRT opcode
//-------------------------------------------------

void drcbe_x64::op_frsqrt(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (USE_RSQRTSS_FOR_SINGLES)
		{
			if (srcp.is_memory())
				emit_rsqrtss_r128_m32(dst, dstreg, MABS(srcp.memory()));                // rsqrtss dstreg,[srcp]
			else if (srcp.is_float_register())
				emit_rsqrtss_r128_r128(dst, dstreg, srcp.freg());                       // rsqrtss dstreg,srcp
		}
		else
		{
			if (srcp.is_memory())
				emit_sqrtss_r128_m32(dst, REG_XMM1, MABS(srcp.memory()));               // sqrtss xmm1,[srcp]
			else if (srcp.is_float_register())
				emit_sqrtss_r128_r128(dst, REG_XMM1, srcp.freg());                      // sqrtss xmm1,srcp
			emit_movss_r128_m32(dst, dstreg, MABS(&m_near.single1));                    // movss dstreg,1.0
			emit_divss_r128_r128(dst, dstreg, REG_XMM1);                                // divss dstreg,xmm1
		}
		emit_movss_p32_r128(dst, dstp, dstreg);                                         // movss dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (USE_RSQRTSS_FOR_DOUBLES)
		{
			if (srcp.is_memory())
				emit_cvtsd2ss_r128_m64(dst, dstreg, MABS(srcp.memory()));               // cvtsd2ss dstreg,[srcp]
			else if (srcp.is_float_register())
				emit_cvtsd2ss_r128_r128(dst, dstreg, srcp.freg());                      // cvtsd2ss dstreg,srcp
			emit_rsqrtss_r128_r128(dst, dstreg, dstreg);                                // rsqrtss dstreg,dstreg
			emit_cvtss2sd_r128_r128(dst, dstreg, dstreg);                               // cvtss2sd dstreg,dstreg
		}
		else
		{
			if (srcp.is_memory())
				emit_sqrtsd_r128_m64(dst, REG_XMM1, MABS(srcp.memory()));               // sqrtsd xmm1,[srcp]
			else if (srcp.is_float_register())
				emit_sqrtsd_r128_r128(dst, REG_XMM1, srcp.freg());                      // sqrtsd xmm1,srcp
			emit_movsd_r128_m64(dst, dstreg, MABS(&m_near.double1));                    // movsd dstreg,1.0
			emit_divsd_r128_r128(dst, dstreg, REG_XMM1);                                // divsd dstreg,xmm1
		}
		emit_movsd_p64_r128(dst, dstp, dstreg);                                         // movsd dstp,dstreg
	}
}


//-------------------------------------------------
//  op_fcopyi - process a FCOPYI opcode
//-------------------------------------------------

void drcbe_x64::op_fcopyi(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MR);

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_XMM0);

	// 32-bit form
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
		{
			emit_movd_r128_m32(dst, dstreg, MABS(srcp.memory()));                       // movd     dstreg,[srcp]
			emit_movss_p32_r128(dst, dstp, dstreg);                                     // movss    dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				emit_mov_p32_r32(dst, dstp, srcp.ireg());								// mov      dstp,srcp
			}
			else
			{
				emit_movd_r128_r32(dst, dstreg, srcp.ireg());							// movd     dstreg,srcp
				emit_movss_p32_r128(dst, dstp, dstreg);									// movss    dstp,dstreg
			}
		}
		
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		
		if (srcp.is_memory())
		{
			emit_movq_r128_m64(dst, dstreg, MABS(srcp.memory()));                       // movq     dstreg,[srcp]
			emit_movsd_p64_r128(dst, dstp, dstreg);                                     // movsd    dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				emit_mov_p64_r64(dst, dstp, srcp.ireg());								// mov      dstp,srcp
			}
			else
			{
				emit_movq_r128_r64(dst, dstreg, srcp.ireg());							// movq     dstreg,srcp
				emit_movsd_p64_r128(dst, dstp, dstreg);									// movsd    dstp,dstreg
			}
		}

	}
}


//-------------------------------------------------
//  op_icopyf - process a ICOPYF opcode
//-------------------------------------------------

void drcbe_x64::op_icopyf(x86code *&dst, const instruction &inst)
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
			int dstreg = dstp.select_register(REG_EAX);
			emit_mov_r32_m32(dst, dstreg, MABS(srcp.memory()));                         // mov      dstreg,[srcp]
			emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov      dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				emit_movd_m32_r128(dst, MABS(dstp.memory()), srcp.freg());				// movd     dstp,srcp
			}
			else
			{
				emit_movd_r32_r128(dst, dstp.ireg(), srcp.freg());						// movd     dstp,srcp
			}
		}	
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			int dstreg = dstp.select_register(REG_RAX);
			emit_mov_r64_m64(dst, dstreg, MABS(srcp.memory()));                         // mov      dstreg,[srcp]
			emit_mov_p64_r64(dst, dstp, dstreg);                                        // mov      dstp,dstreg
		}
		else
		{
			if (dstp.is_memory())
			{
				emit_movq_m64_r128(dst, MABS(dstp.memory()), srcp.freg());				// movq     dstp,srcp
			}
			else
			{
				emit_movq_r64_r128(dst, dstp.ireg(), srcp.freg());						// movq     dstp,srcp
			}
		}
	}
}