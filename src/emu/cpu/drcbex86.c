// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex86.c

    32-bit x86 back-end for the universal machine language.

****************************************************************************

   Future improvements/changes:

    * Optimize to avoid unnecessary reloads
        - especially EDX for 64-bit operations
        - also FCMP/FLAGS has unnecessary PUSHF/POP EAX

    * Identify common pairs and optimize output

    * Convert SUB a,0,b to NEG

    * Optimize, e.g., and [r5],i0,$FF to use ebx as temporary register
        (avoid initial move) if i0 is not needed going forward

****************************************************************************

    ---------------
    ABI/conventions
    ---------------

    Registers:
        EAX        - volatile, function return value
        EBX        - non-volatile
        ECX        - volatile
        EDX        - volatile, function return value (upper 32 bits)
        ESI        - non-volatile
        EDI        - non-volatile
        EBP        - non-volatile

        FP stack   - volatile


    ---------------
    Execution model
    ---------------

    Registers:
        EAX        - scratch register
        EBX        - maps to I0 (low 32 bits)
        ECX        - scratch register
        EDX        - scratch register
        ESI        - maps to I1 (low 32 bits)
        EDI        - maps to I2 (low 32 bits)
        EBP        - maps to I3 (low 32 bits)

        FP stack   - scratch registers

    Entry point:
        Assumes 1 parameter passed, which is the codeptr of the code
        to execute once the environment is set up.

    Exit point:
        Assumes exit value is in EAX.

    Entry stack:
        [esp]      - return
        [esp+4]    - input parameter (entry handle)

    Runtime stack:
        [esp]      - param 0
        [esp+4]    - param 1
        [esp+8]    - param 2
        [esp+12]   - param 3
        [esp+16]   - param 4
        [esp+20]   - alignment
        [esp+24]   - alignment
        [esp+28]   - saved ebp
        [esp+32]   - saved edi
        [esp+36]   - saved esi
        [esp+40]   - saved ebx
        [esp+44]   - ret
        [esp+48]   - input parameter (entry handle)

**************************************************************************/

#include <stddef.h>
#include "emu.h"
#include "debugger.h"
#include "drcuml.h"
#include "drcbex86.h"

using namespace uml;
using namespace x86emit;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_HASHJMPS        (0)



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



//**************************************************************************
//  MACROS
//**************************************************************************

#define X86_CONDITION(condition)        (condition_map[condition - uml::COND_Z])
#define X86_NOT_CONDITION(condition)    (condition_map[condition - uml::COND_Z] ^ 1)

#define assert_no_condition(inst)       assert((inst).condition() == uml::COND_ALWAYS)
#define assert_any_condition(inst)      assert((inst).condition() == uml::COND_ALWAYS || ((inst).condition() >= uml::COND_Z && (inst).condition() < uml::COND_MAX))
#define assert_no_flags(inst)           assert((inst).flags() == 0)
#define assert_flags(inst, valid)       assert(((inst).flags() & ~(valid)) == 0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

drcbe_x86::opcode_generate_func drcbe_x86::s_opcode_table[OP_MAX];

// size-to-mask table
//static const UINT64 size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, U64(0xffffffffffffffff) };

// register mapping tables
static const UINT8 int_register_map[REG_I_COUNT] =
{
	REG_EBX, REG_ESI, REG_EDI, REG_EBP
};

// flags mapping tables
static UINT8 flags_map[0x1000];
static UINT32 flags_unmap[0x20];

// condition mapping table
static const UINT8 condition_map[uml::COND_MAX - uml::COND_Z] =
{
	x86emit::COND_Z,    // COND_Z = 0x80,    requires Z
	x86emit::COND_NZ,   // COND_NZ,          requires Z
	x86emit::COND_S,    // COND_S,           requires S
	x86emit::COND_NS,   // COND_NS,          requires S
	x86emit::COND_C,    // COND_C,           requires C
	x86emit::COND_NC,   // COND_NC,          requires C
	x86emit::COND_O,    // COND_V,           requires V
	x86emit::COND_NO,   // COND_NV,          requires V
	x86emit::COND_P,    // COND_U,           requires U
	x86emit::COND_NP,   // COND_NU,          requires U
	x86emit::COND_A,    // COND_A,           requires CZ
	x86emit::COND_BE,   // COND_BE,          requires CZ
	x86emit::COND_G,    // COND_G,           requires SVZ
	x86emit::COND_LE,   // COND_LE,          requires SVZ
	x86emit::COND_L,    // COND_L,           requires SV
	x86emit::COND_GE,   // COND_GE,          requires SV
};

// FPU control register mapping
static const UINT16 fp_control[4] =
{
	0x0e3f,     // ROUND_TRUNC
	0x023f,     // ROUND_ROUND
	0x0a3f,     // ROUND_CEIL
	0x063f      // ROUND_FLOOR
};



//**************************************************************************
//  TABLES
//**************************************************************************

const drcbe_x86::opcode_table_entry drcbe_x86::s_opcode_table_source[] =
{
	// Compile-time opcodes
	{ uml::OP_HANDLE,  &drcbe_x86::op_handle },     // HANDLE  handle
	{ uml::OP_HASH,    &drcbe_x86::op_hash },       // HASH    mode,pc
	{ uml::OP_LABEL,   &drcbe_x86::op_label },      // LABEL   imm
	{ uml::OP_COMMENT, &drcbe_x86::op_comment },    // COMMENT string
	{ uml::OP_MAPVAR,  &drcbe_x86::op_mapvar },     // MAPVAR  mapvar,value

	// Control Flow Operations
	{ uml::OP_NOP,     &drcbe_x86::op_nop },        // NOP
	{ uml::OP_DEBUG,   &drcbe_x86::op_debug },      // DEBUG   pc
	{ uml::OP_EXIT,    &drcbe_x86::op_exit },       // EXIT    src1[,c]
	{ uml::OP_HASHJMP, &drcbe_x86::op_hashjmp },    // HASHJMP mode,pc,handle
	{ uml::OP_JMP,     &drcbe_x86::op_jmp },        // JMP     imm[,c]
	{ uml::OP_EXH,     &drcbe_x86::op_exh },        // EXH     handle,param[,c]
	{ uml::OP_CALLH,   &drcbe_x86::op_callh },      // CALLH   handle[,c]
	{ uml::OP_RET,     &drcbe_x86::op_ret },        // RET     [c]
	{ uml::OP_CALLC,   &drcbe_x86::op_callc },      // CALLC   func,ptr[,c]
	{ uml::OP_RECOVER, &drcbe_x86::op_recover },    // RECOVER dst,mapvar

	// Internal Register Operations
	{ uml::OP_SETFMOD, &drcbe_x86::op_setfmod },    // SETFMOD src
	{ uml::OP_GETFMOD, &drcbe_x86::op_getfmod },    // GETFMOD dst
	{ uml::OP_GETEXP,  &drcbe_x86::op_getexp },     // GETEXP  dst
	{ uml::OP_GETFLGS, &drcbe_x86::op_getflgs },    // GETFLGS dst[,f]
	{ uml::OP_SAVE,    &drcbe_x86::op_save },       // SAVE    dst
	{ uml::OP_RESTORE, &drcbe_x86::op_restore },    // RESTORE dst

	// Integer Operations
	{ uml::OP_LOAD,    &drcbe_x86::op_load },       // LOAD    dst,base,index,size
	{ uml::OP_LOADS,   &drcbe_x86::op_loads },      // LOADS   dst,base,index,size
	{ uml::OP_STORE,   &drcbe_x86::op_store },      // STORE   base,index,src,size
	{ uml::OP_READ,    &drcbe_x86::op_read },       // READ    dst,src1,spacesize
	{ uml::OP_READM,   &drcbe_x86::op_readm },      // READM   dst,src1,mask,spacesize
	{ uml::OP_WRITE,   &drcbe_x86::op_write },      // WRITE   dst,src1,spacesize
	{ uml::OP_WRITEM,  &drcbe_x86::op_writem },     // WRITEM  dst,src1,spacesize
	{ uml::OP_CARRY,   &drcbe_x86::op_carry },      // CARRY   src,bitnum
	{ uml::OP_SET,     &drcbe_x86::op_set },        // SET     dst,c
	{ uml::OP_MOV,     &drcbe_x86::op_mov },        // MOV     dst,src[,c]
	{ uml::OP_SEXT,    &drcbe_x86::op_sext },       // SEXT    dst,src
	{ uml::OP_ROLAND,  &drcbe_x86::op_roland },     // ROLAND  dst,src1,src2,src3
	{ uml::OP_ROLINS,  &drcbe_x86::op_rolins },     // ROLINS  dst,src1,src2,src3
	{ uml::OP_ADD,     &drcbe_x86::op_add },        // ADD     dst,src1,src2[,f]
	{ uml::OP_ADDC,    &drcbe_x86::op_addc },       // ADDC    dst,src1,src2[,f]
	{ uml::OP_SUB,     &drcbe_x86::op_sub },        // SUB     dst,src1,src2[,f]
	{ uml::OP_SUBB,    &drcbe_x86::op_subc },       // SUBB    dst,src1,src2[,f]
	{ uml::OP_CMP,     &drcbe_x86::op_cmp },        // CMP     src1,src2[,f]
	{ uml::OP_MULU,    &drcbe_x86::op_mulu },       // MULU    dst,edst,src1,src2[,f]
	{ uml::OP_MULS,    &drcbe_x86::op_muls },       // MULS    dst,edst,src1,src2[,f]
	{ uml::OP_DIVU,    &drcbe_x86::op_divu },       // DIVU    dst,edst,src1,src2[,f]
	{ uml::OP_DIVS,    &drcbe_x86::op_divs },       // DIVS    dst,edst,src1,src2[,f]
	{ uml::OP_AND,     &drcbe_x86::op_and },        // AND     dst,src1,src2[,f]
	{ uml::OP_TEST,    &drcbe_x86::op_test },       // TEST    src1,src2[,f]
	{ uml::OP_OR,      &drcbe_x86::op_or },         // OR      dst,src1,src2[,f]
	{ uml::OP_XOR,     &drcbe_x86::op_xor },        // XOR     dst,src1,src2[,f]
	{ uml::OP_LZCNT,   &drcbe_x86::op_lzcnt },      // LZCNT   dst,src[,f]
	{ uml::OP_BSWAP,   &drcbe_x86::op_bswap },      // BSWAP   dst,src
	{ uml::OP_SHL,     &drcbe_x86::op_shl },        // SHL     dst,src,count[,f]
	{ uml::OP_SHR,     &drcbe_x86::op_shr },        // SHR     dst,src,count[,f]
	{ uml::OP_SAR,     &drcbe_x86::op_sar },        // SAR     dst,src,count[,f]
	{ uml::OP_ROL,     &drcbe_x86::op_rol },        // ROL     dst,src,count[,f]
	{ uml::OP_ROLC,    &drcbe_x86::op_rolc },       // ROLC    dst,src,count[,f]
	{ uml::OP_ROR,     &drcbe_x86::op_ror },        // ROR     dst,src,count[,f]
	{ uml::OP_RORC,    &drcbe_x86::op_rorc },       // RORC    dst,src,count[,f]

	// Floating Point Operations
	{ uml::OP_FLOAD,   &drcbe_x86::op_fload },      // FLOAD   dst,base,index
	{ uml::OP_FSTORE,  &drcbe_x86::op_fstore },     // FSTORE  base,index,src
	{ uml::OP_FREAD,   &drcbe_x86::op_fread },      // FREAD   dst,space,src1
	{ uml::OP_FWRITE,  &drcbe_x86::op_fwrite },     // FWRITE  space,dst,src1
	{ uml::OP_FMOV,    &drcbe_x86::op_fmov },       // FMOV    dst,src1[,c]
	{ uml::OP_FTOINT,  &drcbe_x86::op_ftoint },     // FTOINT  dst,src1,size,round
	{ uml::OP_FFRINT,  &drcbe_x86::op_ffrint },     // FFRINT  dst,src1,size
	{ uml::OP_FFRFLT,  &drcbe_x86::op_ffrflt },     // FFRFLT  dst,src1,size
	{ uml::OP_FRNDS,   &drcbe_x86::op_frnds },      // FRNDS   dst,src1
	{ uml::OP_FADD,    &drcbe_x86::op_fadd },       // FADD    dst,src1,src2
	{ uml::OP_FSUB,    &drcbe_x86::op_fsub },       // FSUB    dst,src1,src2
	{ uml::OP_FCMP,    &drcbe_x86::op_fcmp },       // FCMP    src1,src2
	{ uml::OP_FMUL,    &drcbe_x86::op_fmul },       // FMUL    dst,src1,src2
	{ uml::OP_FDIV,    &drcbe_x86::op_fdiv },       // FDIV    dst,src1,src2
	{ uml::OP_FNEG,    &drcbe_x86::op_fneg },       // FNEG    dst,src1
	{ uml::OP_FABS,    &drcbe_x86::op_fabs },       // FABS    dst,src1
	{ uml::OP_FSQRT,   &drcbe_x86::op_fsqrt },      // FSQRT   dst,src1
	{ uml::OP_FRECIP,  &drcbe_x86::op_frecip },     // FRECIP  dst,src1
	{ uml::OP_FRSQRT,  &drcbe_x86::op_frsqrt }      // FRSQRT  dst,src1
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  param_normalize - convert a full parameter
//  into a reduced set
//-------------------------------------------------

drcbe_x86::be_parameter::be_parameter(drcbe_x86 &drcbe, const parameter &param, UINT32 allowed)
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

inline int drcbe_x86::be_parameter::select_register(int defreg) const
{
	if (m_type == PTYPE_INT_REGISTER || m_type == PTYPE_FLOAT_REGISTER || m_type == PTYPE_VECTOR_REGISTER)
		return m_value;
	return defreg;
}

inline int drcbe_x86::be_parameter::select_register(int defreg, const be_parameter &checkparam) const
{
	if (*this == checkparam)
		return defreg;
	return select_register(defreg);
}

inline int drcbe_x86::be_parameter::select_register(int defreg, const be_parameter &checkparam, const be_parameter &checkparam2) const
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

inline void drcbe_x86::normalize_commutative(be_parameter &inner, be_parameter &outer)
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
//  emit_combine_z_flags - combine the Z flag from
//  two 32-bit operations
//-------------------------------------------------

inline void drcbe_x86::emit_combine_z_flags(x86code *&dst)
{
	// this assumes that the flags from the low 32-bit op are on the stack
	// and the flags from the high 32-bit op are live
	emit_pushf(dst);                                                                    // pushf
	emit_mov_r32_m32(dst, REG_ECX, MBD(REG_ESP, 4));                                    // mov   ecx,[esp+4]
	emit_or_r32_imm(dst, REG_ECX, ~0x40);                                               // or    ecx,~0x40
	emit_and_m32_r32(dst, MBD(REG_ESP, 0), REG_ECX);                                    // and   [esp],ecx
	emit_popf(dst);                                                                     // popf
	emit_lea_r32_m32(dst, REG_ESP, MBD(REG_ESP, 4));                                    // lea   esp,[esp+4]
}


//-------------------------------------------------
//  emit_combine_z_shl_flags - combine the Z
//  flags from two 32-bit shift left operations
//-------------------------------------------------

inline void drcbe_x86::emit_combine_z_shl_flags(x86code *&dst)
{
	// this assumes that the flags from the high 32-bit op are on the stack
	// and the flags from the low 32-bit op are live
	emit_pushf(dst);                                                                    // pushf
	emit_pop_r32(dst, REG_ECX);                                                         // pop   ecx
	emit_or_r32_imm(dst, REG_ECX, ~0x40);                                               // or    ecx,~0x40
	emit_and_m32_r32(dst, MBD(REG_ESP, 0), REG_ECX);                                    // and   [esp],ecx
	emit_popf(dst);                                                                     // popf
}


//-------------------------------------------------
//  reset_last_upper_lower_reg - reset the last
//  upper/lower register state
//-------------------------------------------------

inline void drcbe_x86::reset_last_upper_lower_reg()
{
	m_last_lower_reg = REG_NONE;
	m_last_upper_reg = REG_NONE;
}


//-------------------------------------------------
//  set_last_lower_reg - note that we have just
//  loaded a lower register
//-------------------------------------------------

inline void drcbe_x86::set_last_lower_reg(x86code *&dst, const be_parameter &param, UINT8 reglo)
{
	if (param.is_memory())
	{
		m_last_lower_reg = reglo;
		m_last_lower_addr = (UINT32 *)((FPTR)param.memory());
		m_last_lower_pc = dst;
	}
}


//-------------------------------------------------
//  set_last_upper_reg - note that we have just
//  loaded an upper register
//-------------------------------------------------

inline void drcbe_x86::set_last_upper_reg(x86code *&dst, const be_parameter &param, UINT8 reghi)
{
	m_last_upper_reg = reghi;
	m_last_upper_addr = (param.is_int_register()) ? m_reghi[param.ireg()] : (UINT32 *)((FPTR)param.memory(4));
	m_last_upper_pc = dst;
}


//-------------------------------------------------
//  can_skip_lower_load - return true if we can
//  skip re-loading a lower half of a register
//-------------------------------------------------

inline bool drcbe_x86::can_skip_lower_load(x86code *&dst, UINT32 *memref, UINT8 reglo)
{
	return (dst == m_last_lower_pc && memref == m_last_lower_addr && reglo == m_last_lower_reg);
}


//-------------------------------------------------
//  can_skip_upper_load - return true if we can
//  skip re-loading an upper half of a register
//-------------------------------------------------

inline bool drcbe_x86::can_skip_upper_load(x86code *&dst, UINT32 *memref, UINT8 reghi)
{
	return (dst == m_last_upper_pc && memref == m_last_upper_addr && reghi == m_last_upper_reg);
}


//-------------------------------------------------
//  track_resolve_link - wrapper for resolve_link
//  that resets all register tracking info
//-------------------------------------------------

inline void drcbe_x86::track_resolve_link(x86code *&destptr, const emit_link &linkinfo)
{
	reset_last_upper_lower_reg();
	resolve_link(destptr, linkinfo);
}

#define resolve_link INVALID



//**************************************************************************
//  BACKEND CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  drcbe_x86 - constructor
//-------------------------------------------------

drcbe_x86::drcbe_x86(drcuml_state &drcuml, device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device),
		m_hash(cache, modes, addrbits, ignorebits),
		m_map(cache, 0),
		m_labels(cache),
		m_log(NULL),
		m_logged_common(false),
		m_sse3(false),
		m_entry(NULL),
		m_exit(NULL),
		m_nocode(NULL),
		m_save(NULL),
		m_restore(NULL),
		m_last_lower_reg(REG_NONE),
		m_last_lower_pc(NULL),
		m_last_lower_addr(NULL),
		m_last_upper_reg(REG_NONE),
		m_last_upper_pc(NULL),
		m_last_upper_addr(NULL),
		m_fptemp(0),
		m_fpumode(0),
		m_fmodesave(0),
		m_stacksave(0),
		m_hashstacksave(0),
		m_reslo(0),
		m_reshi(0),
		m_fixup_label(FUNC(drcbe_x86::fixup_label), this),
		m_fixup_exception(FUNC(drcbe_x86::fixup_exception), this)
{
	// compute hi pointers for each register
	for (int regnum = 0; regnum < ARRAY_LENGTH(int_register_map); regnum++)
		if (int_register_map[regnum] != 0)
		{
			m_reglo[int_register_map[regnum]] = &m_state.r[regnum].w.l;
			m_reghi[int_register_map[regnum]] = &m_state.r[regnum].w.h;
		}

	// build the flags map (static but it doesn't hurt to regenerate it)
	for (int entry = 0; entry < ARRAY_LENGTH(flags_map); entry++)
	{
		UINT8 flags = 0;
		if (entry & 0x001) flags |= FLAG_C;
		if (entry & 0x004) flags |= FLAG_U;
		if (entry & 0x040) flags |= FLAG_Z;
		if (entry & 0x080) flags |= FLAG_S;
		if (entry & 0x800) flags |= FLAG_V;
		flags_map[entry] = flags;
	}
	for (int entry = 0; entry < ARRAY_LENGTH(flags_unmap); entry++)
	{
		UINT32 flags = 0;
		if (entry & FLAG_C) flags |= 0x001;
		if (entry & FLAG_U) flags |= 0x004;
		if (entry & FLAG_Z) flags |= 0x040;
		if (entry & FLAG_S) flags |= 0x080;
		if (entry & FLAG_V) flags |= 0x800;
		flags_unmap[entry] = flags;
	}

	// build the opcode table (static but it doesn't hurt to regenerate it)
	for (int opnum = 0; opnum < ARRAY_LENGTH(s_opcode_table_source); opnum++)
		s_opcode_table[s_opcode_table_source[opnum].opcode] = s_opcode_table_source[opnum].func;

	// create the log
	if (device.machine().options().drc_log_native())
	{
		std::string filename = std::string("drcbex86_").append(device.shortname()).append(".asm");
		m_log = x86log_create_context(filename.c_str());
	}
}


//-------------------------------------------------
//  ~drcbe_x86 - destructor
//-------------------------------------------------

drcbe_x86::~drcbe_x86()
{
	// free the log context
	if (m_log != NULL)
		x86log_free_context(m_log);
}


//-------------------------------------------------
//  reset - reset back-end specific state
//-------------------------------------------------

void drcbe_x86::reset()
{
	// output a note to the log
	if (m_log != NULL)
		x86log_printf(m_log, "\n\n===========\nCACHE RESET\n===========\n\n");

	// generate a little bit of glue code to set up the environment
	drccodeptr *cachetop = m_cache.begin_codegen(500);
	if (cachetop == NULL)
		fatalerror("Out of cache space after a reset!\n");

	x86code *dst = (x86code *)*cachetop;

	// generate a simple CPUID stub
	UINT32 (*cpuid_ecx_stub)(void) = (UINT32 (*)(void))dst;
	emit_push_r32(dst, REG_EBX);                                                        // push  ebx
	emit_mov_r32_imm(dst, REG_EAX, 1);                                                  // mov   eax,1
	emit_cpuid(dst);                                                                    // cpuid
	emit_mov_r32_r32(dst, REG_EAX, REG_ECX);                                            // mov   eax,ecx
	emit_pop_r32(dst, REG_EBX);                                                         // pop   ebx
	emit_ret(dst);                                                                      // ret

	// call it to determine if we have SSE3 support
	m_sse3 = (((*cpuid_ecx_stub)() & 1) != 0);

	// generate an entry point
	m_entry = (x86_entry_point_func)dst;
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ESP, 4));                                    // mov   eax,[esp+4]
	emit_push_r32(dst, REG_EBX);                                                        // push  ebx
	emit_push_r32(dst, REG_ESI);                                                        // push  esi
	emit_push_r32(dst, REG_EDI);                                                        // push  edi
	emit_push_r32(dst, REG_EBP);                                                        // push  ebp
	emit_sub_r32_imm(dst, REG_ESP, 24);                                                 // sub   esp,24
	emit_mov_m32_r32(dst, MABS(&m_hashstacksave), REG_ESP);                     // mov   [hashstacksave],esp
	emit_sub_r32_imm(dst, REG_ESP, 4);                                                  // sub   esp,4
	emit_mov_m32_r32(dst, MABS(&m_stacksave), REG_ESP);                         // mov   [stacksave],esp
	emit_fstcw_m16(dst, MABS(&m_fpumode));                                          // fstcw [fpumode]
	emit_jmp_r32(dst, REG_EAX);                                                         // jmp   eax
	if (m_log != NULL && !m_logged_common)
		x86log_disasm_code_range(m_log, "entry_point", (x86code *)m_entry, dst);

	// generate an exit point
	m_exit = dst;
	emit_fldcw_m16(dst, MABS(&m_fpumode));                                          // fldcw [fpumode]
	emit_mov_r32_m32(dst, REG_ESP, MABS(&m_hashstacksave));                     // mov   esp,[hashstacksave]
	emit_add_r32_imm(dst, REG_ESP, 24);                                                 // add   esp,24
	emit_pop_r32(dst, REG_EBP);                                                         // pop   ebp
	emit_pop_r32(dst, REG_EDI);                                                         // pop   edi
	emit_pop_r32(dst, REG_ESI);                                                         // pop   esi
	emit_pop_r32(dst, REG_EBX);                                                         // pop   ebx
	emit_ret(dst);                                                                      // ret
	if (m_log != NULL && !m_logged_common)
		x86log_disasm_code_range(m_log, "exit_point", m_exit, dst);

	// generate a no code point
	m_nocode = dst;
	emit_ret(dst);                                                                      // ret
	if (m_log != NULL && !m_logged_common)
		x86log_disasm_code_range(m_log, "nocode", m_nocode, dst);

	// generate a save subroutine
	m_save = dst;
	emit_pushf(dst);                                                                    // pushf
	emit_pop_r32(dst, REG_EAX);                                                         // pop    eax
	emit_and_r32_imm(dst, REG_EAX, 0x8c5);                                              // and    eax,0x8c5
	emit_mov_r8_m8(dst, REG_AL, MABSI(flags_map, REG_EAX));                             // mov    al,[flags_map]
	emit_mov_m8_r8(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)), REG_AL);   // mov    state->flags,al
	emit_mov_r8_m8(dst, REG_AL, MABS(&m_state.fmod));                               // mov    al,[fmod]
	emit_mov_m8_r8(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)), REG_AL);    // mov    state->fmod,al
	emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.exp));                         // mov    eax,[exp]
	emit_mov_m32_r32(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)), REG_EAX);  // mov    state->exp,eax
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		FPTR regoffsl = (FPTR)&((drcuml_machine_state *)NULL)->r[regnum].w.l;
		FPTR regoffsh = (FPTR)&((drcuml_machine_state *)NULL)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), int_register_map[regnum]);
		else
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.r[regnum].w.l));
			emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), REG_EAX);
		}
		emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.r[regnum].w.h));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsh), REG_EAX);
	}
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		FPTR regoffsl = (FPTR)&((drcuml_machine_state *)NULL)->f[regnum].s.l;
		FPTR regoffsh = (FPTR)&((drcuml_machine_state *)NULL)->f[regnum].s.h;
		emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.f[regnum].s.l));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), REG_EAX);
		emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.f[regnum].s.h));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsh), REG_EAX);
	}
	emit_ret(dst);                                                                      // ret
	if (m_log != NULL && !m_logged_common)
		x86log_disasm_code_range(m_log, "save", m_save, dst);

	// generate a restore subroutine
	m_restore = dst;
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		FPTR regoffsl = (FPTR)&((drcuml_machine_state *)NULL)->r[regnum].w.l;
		FPTR regoffsh = (FPTR)&((drcuml_machine_state *)NULL)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			emit_mov_r32_m32(dst, int_register_map[regnum], MBD(REG_ECX, regoffsl));
		else
		{
			emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsl));
			emit_mov_m32_r32(dst, MABS(&m_state.r[regnum].w.l), REG_EAX);
		}
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsh));
		emit_mov_m32_r32(dst, MABS(&m_state.r[regnum].w.h), REG_EAX);
	}
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		FPTR regoffsl = (FPTR)&((drcuml_machine_state *)NULL)->f[regnum].s.l;
		FPTR regoffsh = (FPTR)&((drcuml_machine_state *)NULL)->f[regnum].s.h;
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsl));
		emit_mov_m32_r32(dst, MABS(&m_state.f[regnum].s.l), REG_EAX);
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsh));
		emit_mov_m32_r32(dst, MABS(&m_state.f[regnum].s.h), REG_EAX);
	}
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)));// movzx eax,state->fmod
	emit_and_r32_imm(dst, REG_EAX, 3);                                                  // and    eax,3
	emit_mov_m8_r8(dst, MABS(&m_state.fmod), REG_AL);                               // mov    [fmod],al
	emit_fldcw_m16(dst, MABSI(&fp_control[0], REG_EAX, 2));                             // fldcw  fp_control[eax]
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)));  // mov    eax,state->exp
	emit_mov_m32_r32(dst, MABS(&m_state.exp), REG_EAX);                         // mov    [exp],eax
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)));// movzx eax,state->flags
	emit_push_m32(dst, MABSI(flags_unmap, REG_EAX, 4));                                 // push   flags_unmap[eax*4]
	emit_popf(dst);                                                                     // popf
	emit_ret(dst);                                                                      // ret
	if (m_log != NULL && !m_logged_common)
		x86log_disasm_code_range(m_log, "restore", m_restore, dst);

	// finish up codegen
	*cachetop = dst;
	m_cache.end_codegen();
	m_logged_common = true;

	// reset our hash tables
	m_hash.reset();
	m_hash.set_default_codeptr(m_nocode);
}


//-------------------------------------------------
//  drcbex86_execute - execute a block of code
//  referenced by the given handle
//-------------------------------------------------

int drcbe_x86::execute(code_handle &entry)
{
	// call our entry point which will jump to the destination
	return (*m_entry)((x86code *)entry.codeptr());
}


//-------------------------------------------------
//  drcbex86_generate - generate code
//-------------------------------------------------

void drcbe_x86::generate(drcuml_block &block, const instruction *instlist, UINT32 numinst)
{
	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_labels.block_begin(block);
	m_map.block_begin(block);

	// begin codegen; fail if we can't
	drccodeptr *cachetop = m_cache.begin_codegen(numinst * 8 * 4);
	if (cachetop == NULL)
		block.abort();

	// compute the base by aligning the cache top to a cache line (assumed to be 64 bytes)
	x86code *base = (x86code *)(((FPTR)*cachetop + 63) & ~63);
	x86code *dst = base;

	// generate code
	std::string tempstring;
	const char *blockname = NULL;
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		assert(inst.opcode() < ARRAY_LENGTH(s_opcode_table));

		// add a comment
		if (m_log != NULL)
		{
			std::string dasm;
			inst.disasm(dasm, &m_drcuml);
			x86log_add_comment(m_log, dst, "%s", dasm.c_str());
		}

		// extract a blockname
		if (blockname == NULL)
		{
			if (inst.opcode() == OP_HANDLE)
				blockname = inst.param(0).handle().string();
			else if (inst.opcode() == OP_HASH)
				blockname = strformat(tempstring, "Code: mode=%d PC=%08X", (UINT32)inst.param(0).immediate(), (offs_t)inst.param(1).immediate()).c_str();
		}

		// generate code
		(this->*s_opcode_table[inst.opcode()])(dst, inst);
	}

	// complete codegen
	*cachetop = (drccodeptr)dst;
	m_cache.end_codegen();

	// log it
	if (m_log != NULL)
		x86log_disasm_code_range(m_log, (blockname == NULL) ? "Unknown block" : blockname, base, m_cache.top());

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_labels.block_end(block);
	m_map.block_end(block);
}


//-------------------------------------------------
//  drcbex86_hash_exists - return true if the
//  given mode/pc exists in the hash table
//-------------------------------------------------

bool drcbe_x86::hash_exists(UINT32 mode, UINT32 pc)
{
	return m_hash.code_exists(mode, pc);
}


//-------------------------------------------------
//  drcbex86_get_info - return information about
//  the back-end implementation
//-------------------------------------------------

void drcbe_x86::get_info(drcbe_info &info)
{
	for (info.direct_iregs = 0; info.direct_iregs < REG_I_COUNT; info.direct_iregs++)
		if (int_register_map[info.direct_iregs] == 0)
			break;
	info.direct_fregs = 0;
}



//**************************************************************************
//  EMITTERS FOR 32-BIT OPERATIONS WITH PARAMETERS
//**************************************************************************

//-------------------------------------------------
//  emit_mov_r32_p32 - move a 32-bit parameter
//  into a register
//-------------------------------------------------

void drcbe_x86::emit_mov_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else
			emit_mov_r32_imm(dst, reg, param.immediate());                                  // mov   reg,param
	}
	else if (param.is_memory())
		emit_mov_r32_m32(dst, reg, MABS(param.memory()));                                   // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg != param.ireg())
			emit_mov_r32_r32(dst, reg, param.ireg());                                   // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_r32_p32_keepflags - move a 32-bit
//  parameter into a register without affecting
//  any flags
//-------------------------------------------------

void drcbe_x86::emit_mov_r32_p32_keepflags(x86code *&dst, UINT8 reg, const be_parameter &param)
{
	if (param.is_immediate())
		emit_mov_r32_imm(dst, reg, param.immediate());                                      // mov   reg,param
	else if (param.is_memory())
	{
		if (!can_skip_lower_load(dst, (UINT32 *)((FPTR)param.memory()), reg))
			emit_mov_r32_m32(dst, reg, MABS(param.memory()));                               // mov   reg,[param]
	}
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

void drcbe_x86::emit_mov_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param)
{
	if (param.is_immediate())
		emit_mov_m32_imm(dst, memref, param.immediate());                                   // mov   [mem],param
	else if (param.is_memory())
	{
		if (!can_skip_lower_load(dst, (UINT32 *)((FPTR)param.memory()), REG_EAX))
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

void drcbe_x86::emit_mov_p32_r32(x86code *&dst, const be_parameter &param, UINT8 reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
	{
		emit_mov_m32_r32(dst, MABS(param.memory()), reg);                                   // mov   [param],reg
		set_last_lower_reg(dst, param, reg);
	}
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

void drcbe_x86::emit_add_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_add_r32_imm(dst, reg, param.immediate());                                  // add   reg,param
	}
	else if (param.is_memory())
		emit_add_r32_m32(dst, reg, MABS(param.memory()));                                   // add   reg,[param]
	else if (param.is_int_register())
		emit_add_r32_r32(dst, reg, param.ireg());                                       // add   reg,param
}


//-------------------------------------------------
//  emit_add_m32_p32 - add operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_add_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_add_m32_imm(dst, memref, param.immediate());                               // add   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_add_m32_r32(dst, memref, reg);                                         // add   [dest],reg
	}
}


//-------------------------------------------------
//  emit_adc_r32_p32 - adc operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_adc_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_adc_r32_imm(dst, reg, param.immediate());                                      // adc   reg,param
	else if (param.is_memory())
		emit_adc_r32_m32(dst, reg, MABS(param.memory()));                                   // adc   reg,[param]
	else if (param.is_int_register())
		emit_adc_r32_r32(dst, reg, param.ireg());                                       // adc   reg,param
}


//-------------------------------------------------
//  emit_adc_m32_p32 - adc operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_adc_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_adc_m32_imm(dst, memref, param.immediate());                                   // adc   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32_keepflags(dst, reg, param);                                // mov   reg,param
		emit_adc_m32_r32(dst, memref, reg);                                         // adc   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sub_r32_p32 - sub operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sub_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_sub_r32_imm(dst, reg, param.immediate());                                  // sub   reg,param
	}
	else if (param.is_memory())
		emit_sub_r32_m32(dst, reg, MABS(param.memory()));                                   // sub   reg,[param]
	else if (param.is_int_register())
		emit_sub_r32_r32(dst, reg, param.ireg());                                       // sub   reg,param
}


//-------------------------------------------------
//  emit_sub_m32_p32 - sub operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sub_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() != 0 || param.immediate() != 0)
			emit_sub_m32_imm(dst, memref, param.immediate());                               // sub   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_sub_m32_r32(dst, memref, reg);                                         // sub   [dest],reg
	}
}


//-------------------------------------------------
//  emit_sbb_r32_p32 - sbb operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sbb_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_sbb_r32_imm(dst, reg, param.immediate());                                      // sbb   reg,param
	else if (param.is_memory())
		emit_sbb_r32_m32(dst, reg, MABS(param.memory()));                                   // sbb   reg,[param]
	else if (param.is_int_register())
		emit_sbb_r32_r32(dst, reg, param.ireg());                                       // sbb   reg,param
}


//-------------------------------------------------
//  emit_sbb_m32_p32 - sbb operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sbb_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_sbb_m32_imm(dst, memref, param.immediate());                                   // sbb   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32_keepflags(dst, reg, param);                                // mov   reg,param
		emit_sbb_m32_r32(dst, memref, reg);                                         // sbb   [dest],reg
	}
}


//-------------------------------------------------
//  emit_cmp_r32_p32 - cmp operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_cmp_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_cmp_r32_imm(dst, reg, param.immediate());                                      // cmp   reg,param
	else if (param.is_memory())
		emit_cmp_r32_m32(dst, reg, MABS(param.memory()));                                   // cmp   reg,[param]
	else if (param.is_int_register())
		emit_cmp_r32_r32(dst, reg, param.ireg());                                       // cmp   reg,param
}


//-------------------------------------------------
//  emit_cmp_m32_p32 - cmp operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_cmp_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_cmp_m32_imm(dst, memref, param.immediate());                                   // cmp   [dest],param
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_cmp_m32_r32(dst, memref, reg);                                         // cmp   [dest],reg
	}
}


//-------------------------------------------------
//  emit_and_r32_p32 - and operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_xor_r32_r32(dst, reg, reg);                                            // xor   reg,reg
		else
			emit_and_r32_imm(dst, reg, param.immediate());                                  // and   reg,param
	}
	else if (param.is_memory())
		emit_and_r32_m32(dst, reg, MABS(param.memory()));                                   // and   reg,[param]
	else if (param.is_int_register())
		emit_and_r32_r32(dst, reg, param.ireg());                                       // and   reg,param
}


//-------------------------------------------------
//  emit_and_m32_p32 - and operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_mov_m32_imm(dst, memref, 0);                                       // mov   [dest],0
		else
			emit_and_m32_imm(dst, memref, param.immediate());                               // and   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_and_m32_r32(dst, memref, reg);                                         // and   [dest],reg
	}
}


//-------------------------------------------------
//  emit_test_r32_p32 - test operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_test_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_test_r32_imm(dst, reg, param.immediate());                                     // test   reg,param
	else if (param.is_memory())
		emit_test_m32_r32(dst, MABS(param.memory()), reg);                              // test   [param],reg
	else if (param.is_int_register())
		emit_test_r32_r32(dst, reg, param.ireg());                                      // test   reg,param
}


//-------------------------------------------------
//  emit_test_m32_p32 - test operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_test_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
		emit_test_m32_imm(dst, memref, param.immediate());                              // test  [dest],param
	else if (param.is_memory())
	{
		emit_mov_r32_p32(dst, REG_EAX, param);                                  // mov   reg,param
		emit_test_m32_r32(dst, memref, REG_EAX);                                        // test  [dest],reg
	}
	else if (param.is_int_register())
		emit_test_m32_r32(dst, memref, param.ireg());                               // test  [dest],param
}


//-------------------------------------------------
//  emit_or_r32_p32 - or operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_r32_imm(dst, reg, ~0);                                             // mov  reg,-1
		else
			emit_or_r32_imm(dst, reg, param.immediate());                                   // or   reg,param
	}
	else if (param.is_memory())
		emit_or_r32_m32(dst, reg, MABS(param.memory()));                                    // or   reg,[param]
	else if (param.is_int_register())
		emit_or_r32_r32(dst, reg, param.ireg());                                        // or   reg,param
}


//-------------------------------------------------
//  emit_or_m32_p32 - or operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_m32_imm(dst, memref, ~0);                                      // mov   [dest],-1
		else
			emit_or_m32_imm(dst, memref, param.immediate());                                // or   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_or_m32_r32(dst, memref, reg);                                          // or   [dest],reg
	}
}


//-------------------------------------------------
//  emit_xor_r32_p32 - xor operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_r32(dst, reg);                                                     // not   reg
		else
			emit_xor_r32_imm(dst, reg, param.immediate());                                  // xor   reg,param
	}
	else if (param.is_memory())
		emit_xor_r32_m32(dst, reg, MABS(param.memory()));                                   // xor   reg,[param]
	else if (param.is_int_register())
		emit_xor_r32_r32(dst, reg, param.ireg());                                       // xor   reg,param
}


//-------------------------------------------------
//  emit_xor_m32_p32 - xor operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_m32(dst, memref);                                              // not   [dest]
		else
			emit_xor_m32_imm(dst, memref, param.immediate());                               // xor   [dest],param
	}
	else
	{
		int reg = param.select_register(REG_EAX);
		emit_mov_r32_p32(dst, reg, param);                                      // mov   reg,param
		emit_xor_m32_r32(dst, memref, reg);                                         // xor   [dest],reg
	}
}


//-------------------------------------------------
//  emit_shl_r32_p32 - shl operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shl_r32_imm(dst, reg, param.immediate());                                  // shl   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_shl_r32_cl(dst, reg);                                                      // shl   reg,cl
	}
}


//-------------------------------------------------
//  emit_shl_m32_p32 - shl operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shl_m32_imm(dst, memref, param.immediate());                               // shl   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_shl_m32_cl(dst, memref);                                               // shl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_shr_r32_p32 - shr operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shr_r32_imm(dst, reg, param.immediate());                                  // shr   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_shr_r32_cl(dst, reg);                                                      // shr   reg,cl
	}
}


//-------------------------------------------------
//  emit_shr_m32_p32 - shr operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_shr_m32_imm(dst, memref, param.immediate());                               // shr   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_shr_m32_cl(dst, memref);                                               // shr   [dest],cl
	}
}


//-------------------------------------------------
//  emit_sar_r32_p32 - sar operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sar_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_sar_r32_imm(dst, reg, param.immediate());                                  // sar   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_sar_r32_cl(dst, reg);                                                      // sar   reg,cl
	}
}


//-------------------------------------------------
//  emit_sar_m32_p32 - sar operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sar_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_sar_m32_imm(dst, memref, param.immediate());                               // sar   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_sar_m32_cl(dst, memref);                                               // sar   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rol_r32_p32 - rol operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rol_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rol_r32_imm(dst, reg, param.immediate());                                  // rol   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_rol_r32_cl(dst, reg);                                                      // rol   reg,cl
	}
}


//-------------------------------------------------
//  emit_rol_m32_p32 - rol operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rol_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rol_m32_imm(dst, memref, param.immediate());                               // rol   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_rol_m32_cl(dst, memref);                                               // rol   [dest],cl
	}
}


//-------------------------------------------------
//  emit_ror_r32_p32 - ror operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_ror_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_ror_r32_imm(dst, reg, param.immediate());                                  // ror   reg,param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_ror_r32_cl(dst, reg);                                                      // ror   reg,cl
	}
}


//-------------------------------------------------
//  emit_ror_m32_p32 - ror operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_ror_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_ror_m32_imm(dst, memref, param.immediate());                               // ror   [dest],param
	}
	else
	{
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_ror_m32_cl(dst, memref);                                               // ror   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcl_r32_p32 - rcl operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcl_r32_imm(dst, reg, param.immediate());                                  // rcl   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                            // mov   ecx,param
		emit_rcl_r32_cl(dst, reg);                                                      // rcl   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcl_m32_p32 - rcl operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcl_m32_imm(dst, memref, param.immediate());                               // rcl   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                            // mov   ecx,param
		emit_rcl_m32_cl(dst, memref);                                               // rcl   [dest],cl
	}
}


//-------------------------------------------------
//  emit_rcr_r32_p32 - rcr operation to a 32-bit
//  register from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcr_r32_imm(dst, reg, param.immediate());                                  // rcr   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                            // mov   ecx,param
		emit_rcr_r32_cl(dst, reg);                                                      // rcr   reg,cl
	}
}


//-------------------------------------------------
//  emit_rcr_m32_p32 - rcr operation to a 32-bit
//  memory location from a 32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else
			emit_rcr_m32_imm(dst, memref, param.immediate());                               // rcr   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                            // mov   ecx,param
		emit_rcr_m32_cl(dst, memref);                                               // rcr   [dest],cl
	}
}



//**************************************************************************
//  EMITTERS FOR 64-BIT OPERATIONS WITH PARAMETERS
//**************************************************************************

//-------------------------------------------------
//  emit_mov_r64_p64 - move a 64-bit parameter
//  into a pair of registers
//-------------------------------------------------

void drcbe_x86::emit_mov_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (reglo == REG_NONE)
			;
		else if ((UINT32)param.immediate() == 0)
			emit_xor_r32_r32(dst, reglo, reglo);                                        // xor   reglo,reglo
		else
			emit_mov_r32_imm(dst, reglo, param.immediate());                                    // mov   reglo,param
		if (reghi == REG_NONE)
			;
		else if ((UINT32)(param.immediate() >> 32) == 0)
			emit_xor_r32_r32(dst, reghi, reghi);                                        // xor   reghi,reghi
		else
			emit_mov_r32_imm(dst, reghi, param.immediate() >> 32);                          // mov   reghi,param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(dst, (UINT32 *)((FPTR)param.memory()), reglo);
		int skip_upper = can_skip_upper_load(dst, (UINT32 *)((FPTR)param.memory(4)), reghi);
		if (reglo != REG_NONE && !skip_lower)
			emit_mov_r32_m32(dst, reglo, MABS(param.memory()));                         // mov   reglo,[param]
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(param.memory(4)));                        // mov   reghi,[param+4]
	}
	else if (param.is_int_register())
	{
		int skip_upper = can_skip_upper_load(dst, m_reghi[param.ireg()], reghi);
		if (reglo != REG_NONE && reglo != param.ireg())
			emit_mov_r32_r32(dst, reglo, param.ireg());                                 // mov   reglo,param
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));              // mov   reghi,reghi[param]
	}
}


//-------------------------------------------------
//  emit_mov_r64_p64_keepflags - move a 64-bit
//  parameter into a pair of registers without
//  affecting any flags
//-------------------------------------------------

void drcbe_x86::emit_mov_r64_p64_keepflags(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param)
{
	if (param.is_immediate())
	{
		if (reglo != REG_NONE)
			emit_mov_r32_imm(dst, reglo, param.immediate());                                    // mov   reglo,param
		if (reghi != REG_NONE)
			emit_mov_r32_imm(dst, reghi, param.immediate() >> 32);                          // mov   reghi,param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(dst, (UINT32 *)((FPTR)param.memory()), reglo);
		int skip_upper = can_skip_upper_load(dst, (UINT32 *)((FPTR)param.memory(4)), reghi);
		if (reglo != REG_NONE && !skip_lower)
			emit_mov_r32_m32(dst, reglo, MABS(param.memory()));                         // mov   reglo,[param]
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(param.memory(4)));                        // mov   reghi,[param+4]
	}
	else if (param.is_int_register())
	{
		int skip_upper = can_skip_upper_load(dst, m_reghi[param.ireg()], reghi);
		if (reglo != REG_NONE && reglo != param.ireg())
			emit_mov_r32_r32(dst, reglo, param.ireg());                                 // mov   reglo,param
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));              // mov   reghi,reghi[param]
	}
}


//-------------------------------------------------
//  emit_mov_m64_p64 - move a 64-bit parameter
//  into a memory location
//-------------------------------------------------

void drcbe_x86::emit_mov_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param)
{
	if (param.is_immediate())
	{
		emit_mov_m32_imm(dst, memref + 0, param.immediate());                               // mov   [mem],param
		emit_mov_m32_imm(dst, memref + 4, param.immediate() >> 32);                     // mov   [mem],param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(dst, (UINT32 *)((FPTR)param.memory()), REG_EAX);
		if (!skip_lower)
			emit_mov_r32_m32(dst, REG_EAX, MABS(param.memory()));                           // mov   eax,[param]
		emit_mov_m32_r32(dst, memref + 0, REG_EAX);                                 // mov   [mem],eax
		emit_mov_r32_m32(dst, REG_EAX, MABS(param.memory(4)));                          // mov   eax,[param+4]
		emit_mov_m32_r32(dst, memref + 4, REG_EAX);                                 // mov   [mem+4],eax
	}
	else if (param.is_int_register())
	{
		emit_mov_m32_r32(dst, memref + 0, param.ireg());                                // mov   [mem],param
		emit_mov_r32_m32(dst, REG_EAX, MABS(m_reghi[param.ireg()]));                // mov   eax,[param.hi]
		emit_mov_m32_r32(dst, memref + 4, REG_EAX);                                 // mov   [mem+4],eax
	}
}


//-------------------------------------------------
//  emit_mov_p64_r64 - move a pair of registers
//  into a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_mov_p64_r64(x86code *&dst, const be_parameter &param, UINT8 reglo, UINT8 reghi)
{
	assert(!param.is_immediate());
	if (param.is_memory())
	{
		emit_mov_m32_r32(dst, MABS(param.memory()), reglo);                             // mov   [param],reglo
		emit_mov_m32_r32(dst, MABS(param.memory(4)), reghi);                            // mov   [param+4],reghi
	}
	else if (param.is_int_register())
	{
		if (reglo != param.ireg())
			emit_mov_r32_r32(dst, param.ireg(), reglo);                                 // mov   param,reglo
		emit_mov_m32_r32(dst, MABS(m_reghi[param.ireg()]), reghi);                  // mov   reghi[param],reghi
	}
	set_last_lower_reg(dst, param, reglo);
	set_last_upper_reg(dst, param, reghi);
}


//-------------------------------------------------
//  emit_add_r64_p64 - add operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_add_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_add_r32_m32(dst, reglo, MABS(param.memory()));                             // add   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_m32(dst, reghi, MABS(param.memory(4)));                            // adc   reghi,[param]
	}
	else if (param.is_immediate())
	{
		emit_add_r32_imm(dst, reglo, param.immediate());                                        // add   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_imm(dst, reghi, param.immediate() >> 32);                              // adc   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_add_r32_r32(dst, reglo, param.ireg());                                     // add   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // adc   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_add_m64_p64 - add operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_add_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		emit_add_m32_imm(dst, memref, param.immediate());                                   // add   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_m32_imm(dst, memref + 4, param.immediate() >> 32);                     // adc   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_add_m32_r32(dst, memref, reglo);                                       // add   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_m32_r32(dst, memref + 4, REG_EDX);                                 // adc   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_adc_r64_p64 - adc operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_adc_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_adc_r32_m32(dst, reglo, MABS(param.memory()));                             // adc   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_m32(dst, reghi, MABS(param.memory(4)));                            // adc   reghi,[param]
	}
	else if (param.is_immediate())
	{
		emit_adc_r32_imm(dst, reglo, param.immediate());                                        // adc   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_imm(dst, reghi, param.immediate() >> 32);                              // adc   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_adc_r32_r32(dst, reglo, param.ireg());                                     // adc   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // adc   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_adc_m64_p64 - adc operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_adc_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		emit_adc_m32_imm(dst, memref, param.immediate());                                   // adc   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_m32_imm(dst, memref + 4, param.immediate() >> 32);                     // adc   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64_keepflags(dst, reglo, REG_EDX, param);                 // mov   edx:reglo,param
		emit_adc_m32_r32(dst, memref, reglo);                                       // adc   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_adc_m32_r32(dst, memref + 4, REG_EDX);                                 // adc   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_sub_r64_p64 - sub operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sub_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_sub_r32_m32(dst, reglo, MABS(param.memory()));                             // sub   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(param.memory(4)));                            // sbb   reghi,[param]
	}
	else if (param.is_immediate())
	{
		emit_sub_r32_imm(dst, reglo, param.immediate());                                        // sub   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_imm(dst, reghi, param.immediate() >> 32);                              // sbb   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_sub_r32_r32(dst, reglo, param.ireg());                                     // sub   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // sbb   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_sub_m64_p64 - sub operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sub_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		emit_sub_m32_imm(dst, memref, param.immediate());                                   // sub   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_m32_imm(dst, memref + 4, param.immediate() >> 32);                     // sbb   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_sub_m32_r32(dst, memref, reglo);                                       // sub   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_m32_r32(dst, memref + 4, REG_EDX);                                 // sbb   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_sbb_r64_p64 - sbb operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sbb_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_sbb_r32_m32(dst, reglo, MABS(param.memory()));                             // sbb   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(param.memory(4)));                            // sbb   reghi,[param]
	}
	else if (param.is_immediate())
	{
		emit_sbb_r32_imm(dst, reglo, param.immediate());                                        // sbb   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_imm(dst, reghi, param.immediate() >> 32);                              // sbb   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_sbb_r32_r32(dst, reglo, param.ireg());                                     // sbb   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // sbb   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_sbb_m64_p64 - sbb operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sbb_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		emit_sbb_m32_imm(dst, memref, param.immediate());                                   // sbb   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_m32_imm(dst, memref + 4, param.immediate() >> 32);                     // sbb   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64_keepflags(dst, reglo, REG_EDX, param);                 // mov   edx:reglo,param
		emit_sbb_m32_r32(dst, memref, reglo);                                       // sbb   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_m32_r32(dst, memref + 4, REG_EDX);                                 // sbb   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_cmp_r64_p64 - sub operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_cmp_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = (inst.flags() != FLAG_Z && (inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_sub_r32_m32(dst, reglo, MABS(param.memory()));                             // sub   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(param.memory(4)));                            // sbb   reghi,[param]
	}
	else if (param.is_immediate())
	{
		emit_sub_r32_imm(dst, reglo, param.immediate());                                        // sub   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_imm(dst, reghi, param.immediate() >> 32);                              // sbb   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_sub_r32_r32(dst, reglo, param.ireg());                                     // sub   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sbb_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // sbb   reghi,reghi[param]
	}
	if (inst.flags() == FLAG_Z)
		emit_or_r32_r32(dst, reghi, reglo);                                             // or    reghi,reglo
	else if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_and_r64_p64 - and operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_and_r32_m32(dst, reglo, MABS(param.memory()));                             // and   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_and_r32_m32(dst, reghi, MABS(param.memory(4)));                            // and   reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_xor_r32_r32(dst, reglo, reglo);                                        // xor   reglo,reglo
		else
			emit_and_r32_imm(dst, reglo, param.immediate());                                    // and   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			emit_xor_r32_r32(dst, reghi, reghi);                                        // xor   reghi,reghi
		else
			emit_and_r32_imm(dst, reghi, param.immediate() >> 32);                          // and   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_and_r32_r32(dst, reglo, param.ireg());                                     // and   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_and_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // and   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_and_m64_p64 - and operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			emit_mov_m32_imm(dst, memref, 0);                                       // mov   [dest],0
		else
			emit_and_m32_imm(dst, memref, param.immediate());                               // and   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			emit_mov_m32_imm(dst, memref + 4, 0);                                   // mov   [dest+4],0
		else
			emit_and_m32_imm(dst, memref + 4, param.immediate() >> 32);                 // and   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_and_m32_r32(dst, memref, reglo);                                       // and   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_and_m32_r32(dst, memref + 4, REG_EDX);                                 // and   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_test_r64_p64 - test operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_test_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_test_m32_r32(dst, MABS(param.memory()), reglo);                                // test  [param],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_test_m32_r32(dst, MABS(param.memory(4)), reghi);                           // test  [param],reghi
	}
	else if (param.is_immediate())
	{
		emit_test_r32_imm(dst, reglo, param.immediate());                                   // test  reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_test_r32_imm(dst, reghi, param.immediate() >> 32);                             // test  reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_test_r32_r32(dst, reglo, param.ireg());                                    // test  reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_test_m32_r32(dst, MABS(m_reghi[param.ireg()]), reghi);             // test  reghi[param],reghi
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_test_m64_p64 - test operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_test_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		emit_test_m32_imm(dst, memref, param.immediate());                              // test   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_test_m32_imm(dst, memref + 4, param.immediate() >> 32);                        // test   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_test_m32_r32(dst, memref, reglo);                                      // test  [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_test_m32_r32(dst, memref + 4, REG_EDX);                                    // test  [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_or_r64_p64 - or operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_or_r32_m32(dst, reglo, MABS(param.memory()));                              // or    reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_or_r32_m32(dst, reghi, MABS(param.memory(4)));                         // or    reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_r32_imm(dst, reglo, ~0);                                           // mov   reglo,-1
		else
			emit_or_r32_imm(dst, reglo, param.immediate());                                 // or    reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			emit_mov_r32_imm(dst, reghi, ~0);                                           // mov   reghi,-1
		else
			emit_or_r32_imm(dst, reghi, param.immediate() >> 32);                           // or    reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_or_r32_r32(dst, reglo, param.ireg());                                      // or    reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_or_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                   // or    reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_or_m64_p64 - or operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_mov_m32_imm(dst, memref, ~0);                                      // mov   [dest],-1
		else
			emit_or_m32_imm(dst, memref, param.immediate());                                // or    [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			emit_mov_m32_imm(dst, memref + 4, ~0);                                  // mov   [dest+4],-1
		else
			emit_or_m32_imm(dst, memref + 4, param.immediate() >> 32);                  // or    [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_or_m32_r32(dst, memref, reglo);                                            // or    [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_or_m32_r32(dst, memref + 4, REG_EDX);                                  // or    [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_xor_r64_p64 - xor operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		emit_xor_r32_m32(dst, reglo, MABS(param.memory()));                             // xor   reglo,[param]
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_xor_r32_m32(dst, reghi, MABS(param.memory(4)));                            // xor   reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_r32(dst, reglo);                                                   // not   reglo
		else
			emit_xor_r32_imm(dst, reglo, param.immediate());                                    // xor   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			emit_not_r32(dst, reghi);                                                   // not   reghi
		else
			emit_xor_r32_imm(dst, reghi, param.immediate() >> 32);                          // xor   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		emit_xor_r32_r32(dst, reglo, param.ireg());                                     // xor   reglo,param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_xor_r32_m32(dst, reghi, MABS(m_reghi[param.ireg()]));                  // xor   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_xor_m64_p64 - xor operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (inst.flags() == 0 && (UINT32)param.immediate() == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)param.immediate() == 0xffffffff)
			emit_not_m32(dst, memref);                                              // not   [dest]
		else
			emit_xor_m32_imm(dst, memref, param.immediate());                               // xor   [dest],param
		if (saveflags) emit_pushf(dst);                                                 // pushf
		if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0)
			;// skip
		else if (inst.flags() == 0 && (UINT32)(param.immediate() >> 32) == 0xffffffff)
			emit_not_m32(dst, memref + 4);                                          // not   [dest+4]
		else
			emit_xor_m32_imm(dst, memref + 4, param.immediate() >> 32);                 // xor   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param.is_int_register()) ? param.ireg() : REG_EAX;
		emit_mov_r64_p64(dst, reglo, REG_EDX, param);                           // mov   edx:reglo,param
		emit_xor_m32_r32(dst, memref, reglo);                                       // xor   [dest],reglo
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_xor_m32_r32(dst, memref + 4, REG_EDX);                                 // xor   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_shl_r64_p64 - shl operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shl_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = (inst.flags() != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (inst.flags() == 0 && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					emit_shld_r32_r32_imm(dst, reghi, reglo, 31);                       // shld  reghi,reglo,31
					emit_shl_r32_imm(dst, reglo, 31);                                   // shl   reglo,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reghi, reglo);                                // mov   reghi,reglo
					emit_xor_r32_r32(dst, reglo, reglo);                                // xor   reglo,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				emit_shld_r32_r32_imm(dst, reghi, reglo, count);                        // shld  reghi,reglo,count
				if (saveflags) emit_pushf(dst);                                         // pushf
				emit_shl_r32_imm(dst, reglo, count);                                    // shl   reglo,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);                                          // test  ecx,0x20
		emit_jcc_short_link(dst, x86emit::COND_Z, skip1);                               // jz    skip1
		if (inst.flags() != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);                               // shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);                                           // shl   reglo,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);                                      // test  ecx,0x20
			emit_jcc_short_link(dst, x86emit::COND_Z, skip2);                           // jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);                               // shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);                                           // shl   reglo,31
			track_resolve_link(dst, skip2);                                         // skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reghi, reglo);                                        // mov   reghi,reglo
			emit_xor_r32_r32(dst, reglo, reglo);                                        // xor   reglo,reglo
		}
		track_resolve_link(dst, skip1);                                             // skip1:
		emit_shld_r32_r32_cl(dst, reghi, reglo);                                        // shld  reghi,reglo,cl
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_shl_r32_cl(dst, reglo);                                                    // shl   reglo,cl
	}
	if (saveflags)
		emit_combine_z_shl_flags(dst);
}


//-------------------------------------------------
//  emit_shr_r64_p64 - shr operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shr_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (inst.flags() == 0 && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                       // shrd  reglo,reghi,31
					emit_shr_r32_imm(dst, reghi, 31);                                   // shr   reghi,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reglo, reghi);                                // mov   reglo,reghi
					emit_xor_r32_r32(dst, reghi, reghi);                                // xor   reghi,reghi
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);                        // shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);                                         // pushf
				emit_shr_r32_imm(dst, reghi, count);                                    // shr   reghi,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);                                          // test  ecx,0x20
		emit_jcc_short_link(dst, x86emit::COND_Z, skip1);                               // jz    skip1
		if (inst.flags() != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);                                           // shr   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);                                      // test  ecx,0x20
			emit_jcc_short_link(dst, x86emit::COND_Z, skip2);                           // jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);                                           // shr   reghi,31
			track_resolve_link(dst, skip2);                                         // skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);                                        // mov   reglo,reghi
			emit_xor_r32_r32(dst, reghi, reghi);                                        // xor   reghi,reghi
		}
		track_resolve_link(dst, skip1);                                             // skip1:
		emit_shrd_r32_r32_cl(dst, reglo, reghi);                                        // shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_shr_r32_cl(dst, reghi);                                                    // shr   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_sar_r64_p64 - sar operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sar_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (inst.flags() == 0 && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                       // shrd  reglo,reghi,31
					emit_sar_r32_imm(dst, reghi, 31);                                   // sar   reghi,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reglo, reghi);                                // mov   reglo,reghi
					emit_sar_r32_imm(dst, reghi, 31);                                   // sar   reghi,31
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);                        // shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);                                         // pushf
				emit_sar_r32_imm(dst, reghi, count);                                    // sar   reghi,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);                                          // test  ecx,0x20
		emit_jcc_short_link(dst, x86emit::COND_Z, skip1);                               // jz    skip1
		if (inst.flags() != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);                                           // sar   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);                                      // test  ecx,0x20
			emit_jcc_short_link(dst, x86emit::COND_Z, skip2);                           // jz    skip
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);                                           // sar   reghi,31
			track_resolve_link(dst, skip2);                                         // skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);                                        // mov   reglo,reghi
			emit_sar_r32_imm(dst, reghi, 31);                                           // sar   reghi,31
		}
		track_resolve_link(dst, skip1);                                             // skip1:
		emit_shrd_r32_r32_cl(dst, reglo, reghi);                                        // shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_sar_r32_cl(dst, reghi);                                                    // sar   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_rol_r64_p64 - rol operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rol_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (inst.flags() == 0 && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					emit_mov_r32_r32(dst, REG_ECX, reglo);                              // mov   ecx,reglo
					emit_shld_r32_r32_imm(dst, reglo, reghi, 31);                       // shld  reglo,reghi,31
					emit_shld_r32_r32_imm(dst, reghi, REG_ECX, 31);                     // shld  reghi,ecx,31
					count -= 31;
				}
				else
				{
					emit_xchg_r32_r32(dst, reghi, reglo);                               // xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				emit_mov_r32_r32(dst, REG_ECX, reglo);                                  // mov   ecx,reglo
				emit_shld_r32_r32_imm(dst, reglo, reghi, count);                        // shld  reglo,reghi,count
				if (saveflags) emit_pushf(dst);                                         // pushf
				emit_shld_r32_r32_imm(dst, reghi, REG_ECX, count);                      // shld  reghi,ecx,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		int tempreg = REG_EBX;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), tempreg);                               // mov   [esp-8],ebx
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);                                          // test  ecx,0x20
		emit_jcc_short_link(dst, x86emit::COND_Z, skip1);                               // jz    skip1
		if (inst.flags() != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);                                      // mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);                               // shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, tempreg, 31);                             // shld  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);                                      // test  ecx,0x20
			emit_jcc_short_link(dst, x86emit::COND_Z, skip2);                           // jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);                                      // mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);                               // shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, tempreg, 31);                             // shld  reghi,ebx,31
			track_resolve_link(dst, skip2);                                         // skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);                                       // xchg  reghi,reglo
		track_resolve_link(dst, skip1);                                             // skip1:
		emit_mov_r32_r32(dst, tempreg, reglo);                                          // mov   ebx,reglo
		emit_shld_r32_r32_cl(dst, reglo, reghi);                                        // shld  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_shld_r32_r32_cl(dst, reghi, tempreg);                                      // shld  reghi,ebx,cl
		emit_mov_r32_m32(dst, tempreg, MBD(REG_ESP, saveflags ? -4 : -8));              // mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_ror_r64_p64 - ror operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_ror_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (inst.flags() == 0 && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					emit_mov_r32_r32(dst, REG_ECX, reglo);                              // mov   ecx,reglo
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                       // shrd  reglo,reghi,31
					emit_shrd_r32_r32_imm(dst, reghi, REG_ECX, 31);                     // shrd  reghi,ecx,31
					count -= 31;
				}
				else
				{
					emit_xchg_r32_r32(dst, reghi, reglo);                               // xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				emit_mov_r32_r32(dst, REG_ECX, reglo);                                  // mov   ecx,reglo
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);                        // shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);                                         // pushf
				emit_shrd_r32_r32_imm(dst, reghi, REG_ECX, count);                      // shrd  reghi,ecx,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		int tempreg = REG_EBX;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), tempreg);                               // mov   [esp-8],ebx
		emit_mov_r32_p32(dst, REG_ECX, param);                                  // mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);                                          // test  ecx,0x20
		emit_jcc_short_link(dst, x86emit::COND_Z, skip1);                               // jz    skip1
		if (inst.flags() != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);                                      // mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, tempreg, 31);                             // shrd  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);                                      // test  ecx,0x20
			emit_jcc_short_link(dst, x86emit::COND_Z, skip2);                           // jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);                                         // sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);                                      // mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);                               // shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, tempreg, 31);                             // shrd  reghi,ebx,31
			track_resolve_link(dst, skip2);                                         // skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);                                       // xchg  reghi,reglo
		track_resolve_link(dst, skip1);                                             // skip1:
		emit_mov_r32_r32(dst, tempreg, reglo);                                          // mov   ebx,reglo
		emit_shrd_r32_r32_cl(dst, reglo, reghi);                                        // shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);                                                 // pushf
		emit_shrd_r32_r32_cl(dst, reghi, tempreg);                                      // shrd  reghi,ebx,cl
		emit_mov_r32_m32(dst, tempreg, MBD(REG_ESP, saveflags ? -4 : -8));              // mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


//-------------------------------------------------
//  emit_rcl_r64_p64 - rcl operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcl_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
	if (!saveflags)
	{
		loop = dst;                                                                 // loop:
		emit_jecxz_link(dst, skipall);                                                  // jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		emit_rcl_r32_imm(dst, reglo, 1);                                                // rcl   reglo,1
		emit_rcl_r32_imm(dst, reghi, 1);                                                // rcl   reghi,1
		emit_jmp(dst, loop);                                                            // jmp   loop
		track_resolve_link(dst, skipall);                                           // skipall:
	}
	else
	{
		emit_jecxz_link(dst, skipall);                                                  // jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		loop = dst;                                                                 // loop:
		emit_jecxz_link(dst, skiploop);                                             // jecxz skiploop
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		emit_rcl_r32_imm(dst, reglo, 1);                                                // rcl   reglo,1
		emit_rcl_r32_imm(dst, reghi, 1);                                                // rcl   reghi,1
		emit_jmp(dst, loop);                                                            // jmp   loop
		track_resolve_link(dst, skiploop);                                          // skiploop:
		emit_rcl_r32_imm(dst, reglo, 1);                                                // rcl   reglo,1
		emit_pushf(dst);                                                                // pushf
		emit_rcl_r32_imm(dst, reghi, 1);                                                // rcl   reghi,1
		track_resolve_link(dst, skipall);                                           // skipall:
		emit_combine_z_flags(dst);
	}
}


//-------------------------------------------------
//  emit_rcr_r64_p64 - rcr operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcr_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const instruction &inst)
{
	int saveflags = (inst.flags() != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32_keepflags(dst, REG_ECX, param);                                // mov   ecx,param
	if (!saveflags)
	{
		loop = dst;                                                                 // loop:
		emit_jecxz_link(dst, skipall);                                                  // jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		emit_rcr_r32_imm(dst, reghi, 1);                                                // rcr   reghi,1
		emit_rcr_r32_imm(dst, reglo, 1);                                                // rcr   reglo,1
		emit_jmp(dst, loop);                                                            // jmp   loop
		track_resolve_link(dst, skipall);                                           // skipall:
	}
	else
	{
		emit_jecxz_link(dst, skipall);                                                  // jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		loop = dst;                                                                 // loop:
		emit_jecxz_link(dst, skiploop);                                             // jecxz skiploop
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));                               // lea   ecx,[ecx-1]
		emit_rcr_r32_imm(dst, reghi, 1);                                                // rcr   reghi,1
		emit_rcr_r32_imm(dst, reglo, 1);                                                // rcr   reglo,1
		emit_jmp(dst, loop);                                                            // jmp   loop
		track_resolve_link(dst, skiploop);                                          // skiploop:
		emit_rcr_r32_imm(dst, reghi, 1);                                                // rcr   reghi,1
		emit_pushf(dst);                                                                // pushf
		emit_rcr_r32_imm(dst, reglo, 1);                                                // rcr   reglo,1
		track_resolve_link(dst, skipall);                                           // skipall:
		emit_combine_z_shl_flags(dst);
	}
}



//**************************************************************************
//  EMITTERS FOR FLOATING POINT
//**************************************************************************

//-------------------------------------------------
//  emit_fld_p - load a floating point parameter
//  onto the stack
//-------------------------------------------------

void drcbe_x86::emit_fld_p(x86code *&dst, int size, const be_parameter &param)
{
	assert(param.is_memory());
	assert(size == 4 || size == 8);
	if (size == 4)
		emit_fld_m32(dst, MABS(param.memory()));
	else if (size == 8)
		emit_fld_m64(dst, MABS(param.memory()));
}


//-------------------------------------------------
//  emit_fstp_p - store a floating point parameter
//  from the stack and pop it
//-------------------------------------------------

void drcbe_x86::emit_fstp_p(x86code *&dst, int size, const be_parameter &param)
{
	assert(param.is_memory());
	assert(size == 4 || size == 8);
	if (size == 4)
		emit_fstp_m32(dst, MABS(param.memory()));
	else if (size == 8)
		emit_fstp_m64(dst, MABS(param.memory()));
}



//**************************************************************************
//  OUT-OF-BAND CODE FIXUP CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  fixup_label - callback to fixup forward-
//  referenced labels
//-------------------------------------------------

void drcbe_x86::fixup_label(void *parameter, drccodeptr labelcodeptr)
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

void drcbe_x86::fixup_exception(drccodeptr *codeptr, void *param1, void *param2)
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
	emit_mov_m32_p32(dst, MABS(&m_state.exp), exp);                     // mov   [exp],exp

	// push the original return address on the stack
	emit_push_imm(dst, (FPTR)src);                                                      // push  <return>
	if (*targetptr != NULL)
		emit_jmp(dst, *targetptr);                                                      // jmp   *targetptr
	else
		emit_jmp_m32(dst, MABS(targetptr));                                         // jmp   [targetptr]

	*codeptr = dst;
}



//**************************************************************************
//  DEBUG HELPERS
//**************************************************************************

//-------------------------------------------------
//  debug_log_hashjmp - callback to handle
//  logging of hashjmps
//-------------------------------------------------

void drcbe_x86::debug_log_hashjmp(int mode, offs_t pc)
{
	printf("mode=%d PC=%08X\n", mode, pc);
}



//**************************************************************************
//  COMPILE-TIME OPCODES
//**************************************************************************

//-------------------------------------------------
//  op_handle - process a HANDLE opcode
//-------------------------------------------------

void drcbe_x86::op_handle(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_handle());

	reset_last_upper_lower_reg();

	// emit a jump around the stack adjust in case code falls through here
	emit_link skip;
	emit_jmp_short_link(dst, skip);                                                 // jmp   skip

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(dst);

	// by default, the handle points to prolog code that moves the stack pointer
	emit_lea_r32_m32(dst, REG_ESP, MBD(REG_ESP, -28));                                  // lea   rsp,[rsp-28]
	track_resolve_link(dst, skip);                                                  // skip:
}


//-------------------------------------------------
//  op_hash - process a HASH opcode
//-------------------------------------------------

void drcbe_x86::op_hash(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	// register the current pointer for the mode/PC
	m_hash.set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), dst);
	reset_last_upper_lower_reg();
}


//-------------------------------------------------
//  op_label - process a LABEL opcode
//-------------------------------------------------

void drcbe_x86::op_label(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_label());

	// register the current pointer for the label
	m_labels.set_codeptr(inst.param(0).label(), dst);
	reset_last_upper_lower_reg();
}


//-------------------------------------------------
//  op_comment - process a COMMENT opcode
//-------------------------------------------------

void drcbe_x86::op_comment(x86code *&dst, const instruction &inst)
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

void drcbe_x86::op_mapvar(x86code *&dst, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_mapvar());
	assert(inst.param(1).is_immediate());

	// set the value of the specified mapvar
	m_map.set_value(dst, inst.param(0).mapvar(), inst.param(1).immediate());
}



//**************************************************************************
//  CONTROL FLOW OPCODES
//**************************************************************************

//-------------------------------------------------
//  op_nop - process a NOP opcode
//-------------------------------------------------

void drcbe_x86::op_nop(x86code *&dst, const instruction &inst)
{
	// nothing
}


//-------------------------------------------------
//  op_debug - process a DEBUG opcode
//-------------------------------------------------

void drcbe_x86::op_debug(x86code *&dst, const instruction &inst)
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
		emit_test_m32_imm(dst, MABS(&m_device.machine().debug_flags), DEBUG_FLAG_CALL_HOOK);        // test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		emit_link skip = { 0 };
		emit_jcc_short_link(dst, x86emit::COND_Z, skip);                                        // jz    skip

		// push the parameter
		emit_mov_m32_p32(dst, MBD(REG_ESP, 4), pcp);                            // mov   [esp+4],pcp
		emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_device);                    // mov   [esp],device
		emit_call(dst, (x86code *)debugger_instruction_hook);                           // call  debug_cpu_instruction_hook

		track_resolve_link(dst, skip);                                              // skip:
	}
}


//-------------------------------------------------
//  op_exit - process an EXIT opcode
//-------------------------------------------------

void drcbe_x86::op_exit(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter retp(*this, inst.param(0), PTYPE_MRI);

	// load the parameter into EAX
	emit_mov_r32_p32(dst, REG_EAX, retp);                                       // mov   eax,retp
	if (inst.condition() == uml::COND_ALWAYS)
		emit_jmp(dst, m_exit);                                                  // jmp   exit
	else
		emit_jcc(dst, X86_CONDITION(inst.condition()), m_exit);                 // jcc   exit
}


//-------------------------------------------------
//  op_hashjmp - process a HASHJMP opcode
//-------------------------------------------------

void drcbe_x86::op_hashjmp(x86code *&dst, const instruction &inst)
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
		emit_mov_m32_p32(dst, MBD(REG_ESP, 4), pcp);
		emit_mov_m32_p32(dst, MBD(REG_ESP, 0), modep);
		emit_call(dst, (x86code *)debug_log_hashjmp);
	}

	// load the stack base one word early so we end up at the right spot after our call below
	emit_mov_r32_m32(dst, REG_ESP, MABS(&m_hashstacksave));                     // mov   esp,[hashstacksave]

	// fixed mode cases
	if (modep.is_immediate() && m_hash.is_mode_populated(modep.immediate()))
	{
		// a straight immediate jump is direct, though we need the PC in EAX in case of failure
		if (pcp.is_immediate())
		{
			UINT32 l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			UINT32 l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			emit_call_m32(dst, MABS(&m_hash.base()[modep.immediate()][l1val][l2val]));  // call  hash[modep][l1val][l2val]
		}

		// a fixed mode but variable PC
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, pcp);                                // mov   eax,pcp
			emit_mov_r32_r32(dst, REG_EDX, REG_EAX);                                    // mov   edx,eax
			emit_shr_r32_imm(dst, REG_EDX, m_hash.l1shift());                   // shr   edx,l1shift
			emit_and_r32_imm(dst, REG_EAX, m_hash.l2mask() << m_hash.l2shift());// and  eax,l2mask << l2shift
			emit_mov_r32_m32(dst, REG_EDX, MABSI(&m_hash.base()[modep.immediate()][0], REG_EDX, 4));
																						// mov   edx,hash[modep+edx*4]
			emit_call_m32(dst, MBISD(REG_EDX, REG_EAX, 4 >> m_hash.l2shift(), 0));// call  [edx+eax*shift]
		}
	}
	else
	{
		// variable mode
		int modereg = modep.select_register(REG_ECX);
		emit_mov_r32_p32(dst, modereg, modep);                                  // mov   modereg,modep
		emit_mov_r32_m32(dst, REG_ECX, MABSI(m_hash.base(), modereg, 4));           // mov   ecx,hash[modereg*4]

		// fixed PC
		if (pcp.is_immediate())
		{
			UINT32 l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			UINT32 l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			emit_mov_r32_m32(dst, REG_EDX, MBD(REG_ECX, l1val*4));                      // mov   edx,[ecx+l1val*4]
			emit_call_m32(dst, MBD(REG_EDX, l2val*4));                                  // call  [l2val*4]
		}

		// variable PC
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, pcp);                                // mov   eax,pcp
			emit_mov_r32_r32(dst, REG_EDX, REG_EAX);                                    // mov   edx,eax
			emit_shr_r32_imm(dst, REG_EDX, m_hash.l1shift());                   // shr   edx,l1shift
			emit_mov_r32_m32(dst, REG_EDX, MBISD(REG_ECX, REG_EDX, 4, 0));              // mov   edx,[ecx+edx*4]
			emit_and_r32_imm(dst, REG_EAX, m_hash.l2mask() << m_hash.l2shift());// and  eax,l2mask << l2shift
			emit_call_m32(dst, MBISD(REG_EDX, REG_EAX, 4 >> m_hash.l2shift(), 0));// call  [edx+eax*shift]
		}
	}

	// in all cases, if there is no code, we return here to generate the exception
	emit_mov_m32_p32(dst, MABS(&m_state.exp), pcp);                     // mov   [exp],param
	emit_sub_r32_imm(dst, REG_ESP, 4);                                                  // sub   esp,4
	emit_call_m32(dst, MABS(exp.handle().codeptr_addr()));                                              // call  [exp]
}


//-------------------------------------------------
//  op_jmp - process a JMP opcode
//-------------------------------------------------

void drcbe_x86::op_jmp(x86code *&dst, const instruction &inst)
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
	if (inst.condition() == uml::COND_ALWAYS)
		emit_jmp(dst, jmptarget);                                                       // jmp   target
	else
		emit_jcc(dst, X86_CONDITION(inst.condition()), jmptarget);                      // jcc   target
}


//-------------------------------------------------
//  op_exh - process an EXH opcode
//-------------------------------------------------

void drcbe_x86::op_exh(x86code *&dst, const instruction &inst)
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
		emit_mov_m32_p32(dst, MABS(&m_state.exp), exp);                 // mov   [exp],exp
		if (*targetptr != NULL)
			emit_call(dst, *targetptr);                                             // call  *targetptr
		else
			emit_call_m32(dst, MABS(targetptr));                                        // call  [targetptr]
	}

	// otherwise, jump to an out-of-band handler
	else
	{
		emit_jcc(dst, X86_CONDITION(inst.condition()), 0);                              // jcc   exception
		m_cache.request_oob_codegen(m_fixup_exception, dst, &const_cast<instruction &>(inst));
	}
}


//-------------------------------------------------
//  op_callh - process a CALLH opcode
//-------------------------------------------------

void drcbe_x86::op_callh(x86code *&dst, const instruction &inst)
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
	emit_link skip = { 0 };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// jump through the handle; directly if a normal jump
	if (*targetptr != NULL)
		emit_call(dst, *targetptr);                                                 // call  *targetptr
	else
		emit_call_m32(dst, MABS(targetptr));                                            // call  [targetptr]

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		track_resolve_link(dst, skip);                                              // skip:
}


//-------------------------------------------------
//  op_ret - process a RET opcode
//-------------------------------------------------

void drcbe_x86::op_ret(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	// skip if conditional
	emit_link skip = { 0 };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// return
	emit_lea_r32_m32(dst, REG_ESP, MBD(REG_ESP, 28));                                   // lea   rsp,[rsp+28]
	emit_ret(dst);                                                                      // ret

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		track_resolve_link(dst, skip);                                              // skip:
}


//-------------------------------------------------
//  op_callc - process a CALLC opcode
//-------------------------------------------------

void drcbe_x86::op_callc(x86code *&dst, const instruction &inst)
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
	emit_link skip = { 0 };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// perform the call
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)paramp.memory());                          // mov   [esp],paramp
	emit_call(dst, (x86code *)(FPTR)funcp.cfunc());                                     // call  funcp

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
		track_resolve_link(dst, skip);                                              // skip:
}


//-------------------------------------------------
//  op_recover - process a RECOVER opcode
//-------------------------------------------------

void drcbe_x86::op_recover(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// call the recovery code
	emit_mov_r32_m32(dst, REG_EAX, MABS(&m_stacksave));                         // mov   eax,stacksave
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_EAX, -4));                                   // mov   eax,[eax-4]
	emit_sub_r32_imm(dst, REG_EAX, 1);                                                  // sub   eax,1
	emit_mov_m32_imm(dst, MBD(REG_ESP, 8), inst.param(1).mapvar());                     // mov   [esp+8],param1
	emit_mov_m32_r32(dst, MBD(REG_ESP, 4), REG_EAX);                                    // mov   [esp+4],eax
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_map);                           // mov   [esp],m_map
	emit_call(dst, (x86code *)&drc_map_variables::static_get_value);                    // call  drcmap_get_value
	emit_mov_p32_r32(dst, dstp, REG_EAX);                                       // mov   dstp,eax
}



//**************************************************************************
//  INTERNAL REGISTER OPCODES
//**************************************************************************

//-------------------------------------------------
//  op_setfmod - process a SETFMOD opcode
//-------------------------------------------------

void drcbe_x86::op_setfmod(x86code *&dst, const instruction &inst)
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
		emit_mov_m8_imm(dst, MABS(&m_state.fmod), value);                   // mov   [fmod],srcp
		emit_fldcw_m16(dst, MABS(&fp_control[value]));                          // fldcw fp_control[srcp]
	}

	// register/memory case
	else
	{
		emit_mov_r32_p32(dst, REG_EAX, srcp);                                   // mov   eax,srcp
		emit_and_r32_imm(dst, REG_EAX, 3);                                              // and   eax,3
		emit_mov_m8_r8(dst, MABS(&m_state.fmod), REG_AL);                           // mov   [fmod],al
		emit_fldcw_m16(dst, MABSI(&fp_control[0], REG_EAX, 2));                         // fldcw fp_control[eax]
	}
}


//-------------------------------------------------
//  op_getfmod - process a GETFMOD opcode
//-------------------------------------------------

void drcbe_x86::op_getfmod(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the current mode and store to the destination
	if (dstp.is_int_register())
		emit_movzx_r32_m8(dst, dstp.ireg(), MABS(&m_state.fmod));                   // movzx reg,[fmod]
	else
	{
		emit_movzx_r32_m8(dst, REG_EAX, MABS(&m_state.fmod));                       // movzx eax,[fmod]
		emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                                // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getexp - process a GETEXP opcode
//-------------------------------------------------

void drcbe_x86::op_getexp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the exception parameter and store to the destination
	if (dstp.is_int_register())
		emit_mov_r32_m32(dst, dstp.ireg(), MABS(&m_state.exp));                 // mov   reg,[exp]
	else
	{
		emit_mov_r32_m32(dst, REG_EAX, MABS(&m_state.exp));                     // mov   eax,[exp]
		emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                                // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getflgs - process a GETFLGS opcode
//-------------------------------------------------

void drcbe_x86::op_getflgs(x86code *&dst, const instruction &inst)
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
			emit_setcc_r8(dst, x86emit::COND_C, REG_AL);                                        // setc   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                 // movzx  dstreg,al
			break;

		case FLAG_V:
			emit_setcc_r8(dst, x86emit::COND_O, REG_AL);                                        // seto   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                 // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		case FLAG_Z:
			emit_setcc_r8(dst, x86emit::COND_Z, REG_AL);                                        // setz   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                 // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 2);                                           // shl    dstreg,2
			break;

		case FLAG_S:
			emit_setcc_r8(dst, x86emit::COND_S, REG_AL);                                        // sets   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                 // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 3);                                           // shl    dstreg,3
			break;

		case FLAG_U:
			emit_setcc_r8(dst, x86emit::COND_P, REG_AL);                                        // setp   al
			emit_movzx_r32_r8(dst, dstreg, REG_AL);                                 // movzx  dstreg,al
			emit_shl_r32_imm(dst, dstreg, 4);                                           // shl    dstreg,4
			break;

		// carry plus another flag
		case FLAG_C | FLAG_V:
			emit_setcc_r8(dst, x86emit::COND_C, REG_AL);                                        // setc   al
			emit_setcc_r8(dst, x86emit::COND_O, REG_CL);                                        // seto   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			break;

		case FLAG_C | FLAG_Z:
			emit_setcc_r8(dst, x86emit::COND_C, REG_AL);                                        // setc   al
			emit_setcc_r8(dst, x86emit::COND_Z, REG_CL);                                        // setz   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));               // lea    dstreg,[eax+ecx*4]
			break;

		case FLAG_C | FLAG_S:
			emit_setcc_r8(dst, x86emit::COND_C, REG_AL);                                        // setc   al
			emit_setcc_r8(dst, x86emit::COND_S, REG_CL);                                        // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 8, 0));               // lea    dstreg,[eax+ecx*8]
			break;

		// overflow plus another flag
		case FLAG_V | FLAG_Z:
			emit_setcc_r8(dst, x86emit::COND_O, REG_AL);                                        // seto   al
			emit_setcc_r8(dst, x86emit::COND_Z, REG_CL);                                        // setz   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		case FLAG_V | FLAG_S:
			emit_setcc_r8(dst, x86emit::COND_O, REG_AL);                                        // seto   al
			emit_setcc_r8(dst, x86emit::COND_S, REG_CL);                                        // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));               // lea    dstreg,[eax+ecx*4]
			emit_shl_r32_imm(dst, dstreg, 1);                                           // shl    dstreg,1
			break;

		// zero plus another flag
		case FLAG_Z | FLAG_S:
			emit_setcc_r8(dst, x86emit::COND_Z, REG_AL);                                        // setz   al
			emit_setcc_r8(dst, x86emit::COND_S, REG_CL);                                        // sets   cl
			emit_movzx_r32_r8(dst, REG_EAX, REG_AL);                                    // movzx  eax,al
			emit_movzx_r32_r8(dst, REG_ECX, REG_CL);                                    // movzx  ecx,al
			emit_lea_r32_m32(dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));               // lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(dst, dstreg, 2);                                           // shl    dstreg,2
			break;

		// default cases
		default:
			emit_pushf(dst);                                                            // pushf
			emit_pop_r32(dst, REG_EAX);                                             // pop    eax
			emit_and_r32_imm(dst, REG_EAX, flagmask);                                   // and    eax,flagmask
			emit_movzx_r32_m8(dst, dstreg, MABSI(flags_map, REG_EAX));                  // movzx  dstreg,[flags_map]
			break;
	}

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// general case
		if (dstp.is_memory())
			emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                         // mov   [dstp+4],0
		else if (dstp.is_int_register())
			emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);                   // mov   [reghi],0
	}
}


//-------------------------------------------------
//  op_save - process a SAVE opcode
//-------------------------------------------------

void drcbe_x86::op_save(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_M);

	// copy live state to the destination
	emit_mov_r32_imm(dst, REG_ECX, (FPTR)dstp.memory());                                        // mov    ecx,dstp
	emit_call(dst, m_save);                                                     // call   save
}


//-------------------------------------------------
//  op_restore - process a RESTORE opcode
//-------------------------------------------------

void drcbe_x86::op_restore(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_M);

	// copy live state from the destination
	emit_mov_r32_imm(dst, REG_ECX, (FPTR)srcp.memory());                                        // mov    ecx,dstp
	emit_call(dst, m_restore);                                                  // call   restore
}



//**************************************************************************
//  INTEGER OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  op_load - process a LOAD opcode
//-------------------------------------------------

void drcbe_x86::op_load(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// immediate index
	if (indp.is_immediate())
	{
		if (size == SIZE_BYTE)
			emit_movzx_r32_m8(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));     // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movzx_r32_m16(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));        // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));      // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			emit_mov_r32_m32(dst, REG_EDX, MABS(basep.memory(scale*indp.immediate() + 4))); // mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));      // mov   dstreg,[basep + scale*indp]
		}
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_mov_r32_p32(dst, indreg, indp);
		if (size == SIZE_BYTE)
			emit_movzx_r32_m8(dst, dstreg, MABSI(basep.memory(), indreg, scale));           // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movzx_r32_m16(dst, dstreg, MABSI(basep.memory(), indreg, scale));          // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MABSI(basep.memory(), indreg, scale));            // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			emit_mov_r32_m32(dst, REG_EDX, MABSI(basep.memory(4), indreg, scale));      // mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(dst, dstreg, MABSI(basep.memory(), indreg, scale));            // mov   dstreg,[basep + scale*indp]
		}
	}

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (size != SIZE_QWORD)
		{
			if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   [dstp+4],0
			else if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                   // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				emit_mov_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);     // mov   [reghi],edx
			set_last_upper_reg(dst, dstp, REG_EDX);
		}
	}
	set_last_lower_reg(dst, dstp, dstreg);
}


//-------------------------------------------------
//  op_loads - process a LOADS opcode
//-------------------------------------------------

void drcbe_x86::op_loads(x86code *&dst, const instruction &inst)
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

	// pick a target register for the general case
	int dstreg = dstp.select_register(REG_EAX);

	// immediate index
	if (indp.is_immediate())
	{
		if (size == SIZE_BYTE)
			emit_movsx_r32_m8(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));     // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movsx_r32_m16(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));        // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));      // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			emit_mov_r32_m32(dst, REG_EDX, MABS(basep.memory(scale*indp.immediate() + 4))); // mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(dst, dstreg, MABS(basep.memory(scale*indp.immediate())));      // mov   dstreg,[basep + scale*indp]
		}
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_mov_r32_p32(dst, indreg, indp);
		if (size == SIZE_BYTE)
			emit_movsx_r32_m8(dst, dstreg, MABSI(basep.memory(), indreg, scale));           // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			emit_movsx_r32_m16(dst, dstreg, MABSI(basep.memory(), indreg, scale));          // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MABSI(basep.memory(), indreg, scale));            // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			emit_mov_r32_m32(dst, REG_EDX, MABSI(basep.memory(4), indreg, scale));      // mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(dst, dstreg, MABSI(basep.memory(), indreg, scale));            // mov   dstreg,[basep + scale*indp]
		}
	}

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		emit_cdq(dst);                                                                  // cdq
		if (dstp.is_memory())
			emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                       // mov   [dstp+4],edx
		else if (dstp.is_int_register())
			emit_mov_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);         // mov   [reghi],edx
		set_last_upper_reg(dst, dstp, REG_EDX);
	}
	set_last_lower_reg(dst, dstp, dstreg);
}


//-------------------------------------------------
//  op_store - process a STORE opcode
//-------------------------------------------------

void drcbe_x86::op_store(x86code *&dst, const instruction &inst)
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

	// pick a source register for the general case
	int srcreg = srcp.select_register(REG_EAX);
	if (size == SIZE_BYTE && (srcreg & 4))
		srcreg = REG_EAX;

	// degenerate case: constant index
	if (indp.is_immediate())
	{
		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				emit_mov_m8_imm(dst, MABS(basep.memory(scale*indp.immediate())), srcp.immediate()); // mov   [basep + scale*indp],srcp
			else if (size == SIZE_WORD)
				emit_mov_m16_imm(dst, MABS(basep.memory(scale*indp.immediate())), srcp.immediate());    // mov   [basep + scale*indp],srcp
			else if (size == SIZE_DWORD)
				emit_mov_m32_imm(dst, MABS(basep.memory(scale*indp.immediate())), srcp.immediate());    // mov   [basep + scale*indp],srcp
			else if (size == SIZE_QWORD)
			{
				emit_mov_m32_imm(dst, MABS(basep.memory(scale*indp.immediate())), srcp.immediate());    // mov   [basep + scale*indp],srcp
				emit_mov_m32_imm(dst, MABS(basep.memory(scale*indp.immediate() + 4)), srcp.immediate() >> 32);
																						// mov   [basep + scale*indp + 4],srcp >> 32
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(dst, srcreg, srcp);                            // mov   srcreg,srcp
			else
				emit_mov_r64_p64(dst, srcreg, REG_EDX, srcp);                   // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				emit_mov_m8_r8(dst, MABS(basep.memory(scale*indp.immediate())), srcreg);        // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_WORD)
				emit_mov_m16_r16(dst, MABS(basep.memory(scale*indp.immediate())), srcreg);  // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_DWORD)
				emit_mov_m32_r32(dst, MABS(basep.memory(scale*indp.immediate())), srcreg);  // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_QWORD)
			{
				emit_mov_m32_r32(dst, MABS(basep.memory(scale*indp.immediate())), srcreg);  // mov   [basep + scale*indp],srcreg
				emit_mov_m32_r32(dst, MABS(basep.memory(scale*indp.immediate() + 4)), REG_EDX); // mov   [basep + scale*indp + 4],edx
			}
		}
	}

	// normal case: variable index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_mov_r32_p32(dst, indreg, indp);                                    // mov   indreg,indp

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				emit_mov_m8_imm(dst, MABSI(basep.memory(), indreg, scale), srcp.immediate());   // mov   [basep + 1*ecx],srcp
			else if (size == SIZE_WORD)
				emit_mov_m16_imm(dst, MABSI(basep.memory(), indreg, scale), srcp.immediate());  // mov   [basep + 2*ecx],srcp
			else if (size == SIZE_DWORD)
				emit_mov_m32_imm(dst, MABSI(basep.memory(), indreg, scale), srcp.immediate());  // mov   [basep + 4*ecx],srcp
			else if (size == SIZE_QWORD)
			{
				emit_mov_m32_imm(dst, MABSI(basep.memory(), indreg, scale), srcp.immediate());  // mov   [basep + 8*ecx],srcp
				emit_mov_m32_imm(dst, MABSI(basep.memory(4), indreg, scale), srcp.immediate() >> 32);
																						// mov   [basep + 8*ecx + 4],srcp >> 32
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(dst, srcreg, srcp);                            // mov   srcreg,srcp
			else
				emit_mov_r64_p64(dst, srcreg, REG_EDX, srcp);                   // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				emit_mov_m8_r8(dst, MABSI(basep.memory(), indreg, scale), srcreg);          // mov   [basep + 1*ecx],srcreg
			else if (size == SIZE_WORD)
				emit_mov_m16_r16(dst, MABSI(basep.memory(), indreg, scale), srcreg);        // mov   [basep + 2*ecx],srcreg
			else if (size == SIZE_DWORD)
				emit_mov_m32_r32(dst, MABSI(basep.memory(), indreg, scale), srcreg);        // mov   [basep + 4*ecx],srcreg
			else if (size == SIZE_QWORD)
			{
				emit_mov_m32_r32(dst, MABSI(basep.memory(), indreg, scale), srcreg);        // mov   [basep + 8*ecx],srcreg
				emit_mov_m32_r32(dst, MABSI(basep.memory(4), indreg, scale), REG_EDX);  // mov   [basep + 8*ecx],edx
			}
		}
	}
}


//-------------------------------------------------
//  op_read - process a READ opcode
//-------------------------------------------------

void drcbe_x86::op_read(x86code *&dst, const instruction &inst)
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
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacesizep.space()]);  // mov    [esp],space
	if (spacesizep.size() == SIZE_BYTE)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_byte);
																						// call   read_byte
		emit_movzx_r32_r8(dst, dstreg, REG_AL);                                     // movzx  dstreg,al
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_word);
																						// call   read_word
		emit_movzx_r32_r16(dst, dstreg, REG_AX);                                        // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_dword);
																						// call   read_dword
		emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_qword);
																						// call   read_qword
		emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov    dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (spacesizep.size() != SIZE_QWORD)
		{
			if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   [dstp+4],0
			else if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                   // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				emit_mov_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);     // mov   [reghi],edx
		}
	}
}


//-------------------------------------------------
//  op_readm - process a READM opcode
//-------------------------------------------------

void drcbe_x86::op_readm(x86code *&dst, const instruction &inst)
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
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_m32_p32(dst, MBD(REG_ESP, 8), maskp);                          // mov    [esp+8],maskp
	else
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), maskp);                          // mov    [esp+8],maskp
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacesizep.space()]);  // mov    [esp],space
	if (spacesizep.size() == SIZE_WORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_word_masked);
																						// call   read_word_masked
		emit_movzx_r32_r16(dst, dstreg, REG_AX);                                        // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_dword_masked);
																						// call   read_dword_masked
		emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].read_qword_masked);
																						// call   read_qword_masked
		emit_mov_r32_r32(dst, dstreg, REG_EAX);                                     // mov    dstreg,eax
	}

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov    dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (spacesizep.size() != SIZE_QWORD)
		{
			if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   [dstp+4],0
			else if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                   // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				emit_mov_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);     // mov   [reghi],edx
		}
	}
}


//-------------------------------------------------
//  op_write - process a WRITE opcode
//-------------------------------------------------

void drcbe_x86::op_write(x86code *&dst, const instruction &inst)
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
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_m32_p32(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	else
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacesizep.space()]);  // mov    [esp],space
	if (spacesizep.size() == SIZE_BYTE)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_byte);
																						// call   write_byte
	else if (spacesizep.size() == SIZE_WORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_word);
																						// call   write_word
	else if (spacesizep.size() == SIZE_DWORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_dword);
																						// call   write_dword
	else if (spacesizep.size() == SIZE_QWORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_qword);
																						// call   write_qword
}


//-------------------------------------------------
//  op_writem - process a WRITEM opcode
//-------------------------------------------------

void drcbe_x86::op_writem(x86code *&dst, const instruction &inst)
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
	if (spacesizep.size() != SIZE_QWORD)
	{
		emit_mov_m32_p32(dst, MBD(REG_ESP, 12), maskp);                     // mov    [esp+12],maskp
		emit_mov_m32_p32(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	}
	else
	{
		emit_mov_m64_p64(dst, MBD(REG_ESP, 16), maskp);                     // mov    [esp+16],maskp
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	}
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacesizep.space()]);  // mov    [esp],space
	if (spacesizep.size() == SIZE_WORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_word_masked);
																						// call   write_word_masked
	else if (spacesizep.size() == SIZE_DWORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_dword_masked);
																						// call   write_dword_masked
	else if (spacesizep.size() == SIZE_QWORD)
		emit_call(dst, (x86code *)m_accessors[spacesizep.space()].write_qword_masked);
																						// call   write_qword_masked
}


//-------------------------------------------------
//  op_carry - process a CARRY opcode
//-------------------------------------------------

void drcbe_x86::op_carry(x86code *&dst, const instruction &inst)
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
				emit_bt_m32_imm(dst, MABS(srcp.memory()), bitp.immediate());                    // bt     [srcp],bitp
			else if (srcp.is_int_register())
				emit_bt_r32_imm(dst, srcp.ireg(), bitp.immediate());                            // bt     srcp,bitp
		}
		else
		{
			if (srcp.is_memory())
				emit_bt_m32_r32(dst, MABS(srcp.memory()), REG_ECX);                     // bt     [srcp],ecx
			else if (srcp.is_int_register())
				emit_bt_r32_r32(dst, srcp.ireg(), REG_ECX);                             // bt     [srcp],ecx
		}
	}

	// 64-bit form
	else
	{
		if (bitp.is_immediate())
		{
			if (srcp.is_memory())
				emit_bt_m32_imm(dst, MABS(srcp.memory()), bitp.immediate());                    // bt     [srcp],bitp
			else if (srcp.is_int_register() && bitp.immediate() < 32)
				emit_bt_r32_imm(dst, srcp.ireg(), bitp.immediate());                            // bt     srcp,bitp
			else if (srcp.is_int_register() && bitp.immediate() >= 32)
				emit_bt_m32_imm(dst, MABS(m_reghi[srcp.ireg()]), bitp.immediate() - 32);    // bt     [srcp.hi],bitp
		}
		else
		{
			if (srcp.is_memory())
				emit_bt_m32_r32(dst, MABS(srcp.memory()), REG_ECX);                     // bt     [srcp],ecx
			else if (srcp.is_int_register())
			{
				emit_mov_m32_r32(dst, MABS(m_reglo[srcp.ireg()]), srcp.ireg());     // mov    [srcp.lo],srcp
				emit_bt_m32_r32(dst, MABS(m_reglo[srcp.ireg()]), REG_ECX);          // bt     [srcp],ecx
			}
		}
	}
}


//-------------------------------------------------
//  op_set - process a SET opcode
//-------------------------------------------------

void drcbe_x86::op_set(x86code *&dst, const instruction &inst)
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
	emit_movzx_r32_r8(dst, dstreg, REG_AL);                                         // movzx  dstreg,al

	// store low 32 bits
	emit_mov_p32_r32(dst, dstp, dstreg);                                        // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// general case
		if (dstp.is_memory())
			emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                         // mov   [dstp+4],0
		else if (dstp.is_int_register())
			emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);                   // mov   [reghi],0
	}
}


//-------------------------------------------------
//  op_mov - process a MOV opcode
//-------------------------------------------------

void drcbe_x86::op_mov(x86code *&dst, const instruction &inst)
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
	emit_link skip = { 0 };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// 32-bit form
	if (inst.size() == 4)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
			emit_mov_m32_r32(dst, MABS(dstp.memory()), srcp.ireg());                        // mov   [dstp],srcp

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate())
			emit_mov_m32_imm(dst, MABS(dstp.memory()), srcp.immediate());                       // mov   [dstp],srcp

		// conditional memory to register
		else if (inst.condition() != uml::COND_ALWAYS && dstp.is_int_register() && srcp.is_memory())
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_m32(dst, X86_CONDITION(inst.condition()), dstp.ireg(), MABS(srcp.memory()));
																						// cmovcc dstp,[srcp]
		}

		// conditional register to register
		else if (inst.condition() != uml::COND_ALWAYS && dstp.is_int_register() && srcp.is_int_register())
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_r32(dst, X86_CONDITION(inst.condition()), dstp.ireg(), srcp.ireg());
																						// cmovcc dstp,srcp
		}

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, srcp);                      // mov   dstreg,srcp
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS(m_reghi[srcp.ireg()]));         // mov   eax,reghi[srcp]
			emit_mov_m32_r32(dst, MABS(dstp.memory()), srcp.ireg());                        // mov   [dstp],srcp
			emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EAX);                       // mov   [dstp+4],eax
		}

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate())
		{
			emit_mov_m32_imm(dst, MABS(dstp.memory()), srcp.immediate());                       // mov   [dstp],srcp
			emit_mov_m32_imm(dst, MABS(dstp.memory(4)), srcp.immediate() >> 32);                // mov   [dstp+4],srcp >> 32
		}

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, srcp);                       // mov   edx:dstreg,srcp
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,edx:dstreg
		}
	}

	// resolve the jump
	if (skip.target != NULL)
		track_resolve_link(dst, skip);
}


//-------------------------------------------------
//  op_sext - process a SEXT opcode
//-------------------------------------------------

void drcbe_x86::op_sext(x86code *&dst, const instruction &inst)
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
	int dstreg = (inst.size() == 8) ? REG_EAX : dstp.select_register(REG_EAX);

	// convert 8-bit source registers to EAX
	if (sizep.size() == SIZE_BYTE && srcp.is_int_register() && (srcp.ireg() & 4))
	{
		emit_mov_r32_r32(dst, REG_EAX, srcp.ireg());                                    // mov   eax,srcp
		srcp = be_parameter::make_ireg(REG_EAX);
	}

	// general case
	if (srcp.is_memory())
	{
		if (sizep.size() == SIZE_BYTE)
			emit_movsx_r32_m8(dst, dstreg, MABS(srcp.memory()));                            // movsx dstreg,[srcp]
		else if (sizep.size() == SIZE_WORD)
			emit_movsx_r32_m16(dst, dstreg, MABS(srcp.memory()));                           // movsx dstreg,[srcp]
		else if (sizep.size() == SIZE_DWORD)
			emit_mov_r32_m32(dst, dstreg, MABS(srcp.memory()));                         // mov   dstreg,[srcp]
	}
	else if (srcp.is_int_register())
	{
		if (sizep.size() == SIZE_BYTE)
			emit_movsx_r32_r8(dst, dstreg, srcp.ireg());                                // movsx dstreg,srcp
		else if (sizep.size() == SIZE_WORD)
			emit_movsx_r32_r16(dst, dstreg, srcp.ireg());                               // movsx dstreg,srcp
		else if (sizep.size() == SIZE_DWORD && dstreg != srcp.ireg())
			emit_mov_r32_r32(dst, dstreg, srcp.ireg());                                 // mov   dstreg,srcp
	}
	if (inst.flags() != 0)
		emit_test_r32_r32(dst, dstreg, dstreg);                                     // test  dstreg,dstreg

	// 32-bit form: store the low 32 bits
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, dstreg);                                    // mov   dstp,dstreg

	// 64-bit form: sign extend to 64 bits and store edx:eax
	else if (inst.size() == 8)
	{
		emit_cdq(dst);                                                                  // cdq
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
	}
}


//-------------------------------------------------
//  op_roland - process an ROLAND opcode
//-------------------------------------------------

void drcbe_x86::op_roland(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, dstreg, srcp);                                    // mov   dstreg,srcp
		emit_rol_r32_p32(dst, dstreg, shiftp, inst);                            // rol   dstreg,shiftp
		emit_and_r32_p32(dst, dstreg, maskp, inst);                         // and   dstreg,maskp
		emit_mov_p32_r32(dst, dstp, dstreg);                                    // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, dstreg, REG_EDX, srcp);                           // mov   edx:dstreg,srcp
		emit_rol_r64_p64(dst, dstreg, REG_EDX, shiftp, inst);                   // rol   edx:dstreg,shiftp
		emit_and_r64_p64(dst, dstreg, REG_EDX, maskp, inst);                    // and   edx:dstreg,maskp
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_rolins - process an ROLINS opcode
//-------------------------------------------------

void drcbe_x86::op_rolins(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, REG_EAX, srcp);                                   // mov   eax,srcp
		emit_rol_r32_p32(dst, REG_EAX, shiftp, inst);                           // rol   eax,shiftp
		emit_mov_r32_p32(dst, dstreg, dstp);                                    // mov   dstreg,dstp
		if (maskp.is_immediate())
		{
			emit_and_r32_imm(dst, REG_EAX, maskp.immediate());                              // and   eax,maskp
			emit_and_r32_imm(dst, dstreg, ~maskp.immediate());                              // and   dstreg,~maskp
		}
		else
		{
			emit_mov_r32_p32(dst, REG_EDX, maskp);                              // mov   edx,maskp
			emit_and_r32_r32(dst, REG_EAX, REG_EDX);                                    // and   eax,edx
			emit_not_r32(dst, REG_EDX);                                             // not   edx
			emit_and_r32_r32(dst, dstreg, REG_EDX);                                 // and   dstreg,edx
		}
		emit_or_r32_r32(dst, dstreg, REG_EAX);                                          // or    dstreg,eax
		emit_mov_p32_r32(dst, dstp, dstreg);                                    // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, REG_EAX, REG_EDX, srcp);                          // mov   edx:eax,srcp
		emit_rol_r64_p64(dst, REG_EAX, REG_EDX, shiftp, inst);                  // rol   edx:eax,shiftp
		if (maskp.is_immediate())
		{
			emit_and_r32_imm(dst, REG_EAX, maskp.immediate());                              // and   eax,maskp
			emit_and_r32_imm(dst, REG_EDX, maskp.immediate() >> 32);                            // and   edx,maskp >> 32
			if (dstp.is_int_register())
			{
				emit_and_r32_imm(dst, dstp.ireg(), ~maskp.immediate());                     // and   dstp.lo,~maskp
				emit_and_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), ~maskp.immediate() >> 32);// and   dstp.hi,~maskp >> 32
				emit_or_r32_r32(dst, dstp.ireg(), REG_EAX);                             // or    dstp.lo,eax
				emit_or_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);          // or    dstp.hi,edx
			}
			else
			{
				emit_and_m32_imm(dst, MABS(dstp.memory()), ~maskp.immediate());                 // and   dstp.lo,~maskp
				emit_and_m32_imm(dst, MABS(dstp.memory(4)), ~maskp.immediate() >> 32);      // and   dstp.hi,~maskp >> 32
				emit_or_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                     // or    dstp.lo,eax
				emit_or_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                    // or    dstp.hi,edx
			}
		}
		else
		{
			int tempreg = REG_EBX;
			emit_mov_m32_r32(dst, MBD(REG_ESP, -8), tempreg);                           // mov   [esp-8],ebx
			emit_mov_r64_p64(dst, tempreg, REG_ECX, maskp);                 // mov   ecx:ebx,maskp
			emit_and_r32_r32(dst, REG_EAX, tempreg);                                    // and   eax,ebx
			emit_and_r32_r32(dst, REG_EDX, REG_ECX);                                    // and   edx,ecx
			emit_not_r32(dst, tempreg);                                             // not   ebx
			emit_not_r32(dst, REG_ECX);                                             // not   ecx
			if (dstp.is_int_register())
			{
				emit_and_r32_r32(dst, dstp.ireg(), tempreg);                            // and   dstp.lo,ebx
				emit_and_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_ECX);     // and   dstp.hi,ecx
				emit_or_r32_r32(dst, dstp.ireg(), REG_EAX);                             // or    dstp.lo,eax
				emit_or_m32_r32(dst, MABS(m_reghi[dstp.ireg()]), REG_EDX);          // or    dstp.hi,edx
			}
			else
			{
				emit_and_m32_r32(dst, MABS(dstp.memory()), tempreg);                        // and   dstp.lo,ebx
				emit_and_m32_r32(dst, MABS(dstp.memory(4)), REG_ECX);                   // and   dstp.hi,ecx
				emit_or_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                     // or    dstp.lo,eax
				emit_or_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                    // or    dstp.hi,edx
			}
			emit_mov_r32_m32(dst, tempreg, MBD(REG_ESP, -8));                           // mov   ebx,[esp-8]
		}
		if (inst.flags() == FLAG_Z)
			emit_or_r32_r32(dst, REG_EAX, REG_EDX);                                 // or    eax,edx
		else if (inst.flags() == FLAG_S)
			;// do nothing -- final OR will have the right result
		else if (inst.flags() == (FLAG_Z | FLAG_S))
		{
			emit_movzx_r32_r16(dst, REG_ECX, REG_AX);                                   // movzx ecx,ax
			emit_shr_r32_imm(dst, REG_EAX, 16);                                     // shr   eax,16
			emit_or_r32_r32(dst, REG_EDX, REG_ECX);                                 // or    edx,ecx
			emit_or_r32_r32(dst, REG_EDX, REG_EAX);                                 // or    edx,eax
		}
	}
}


//-------------------------------------------------
//  op_add - process a ADD opcode
//-------------------------------------------------

void drcbe_x86::op_add(x86code *&dst, const instruction &inst)
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
			emit_add_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // add   [dstp],src2p

		// reg = reg + imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBD(src1p.ireg(), src2p.immediate()));           // lea   dstp,[src1p+src2p]

		// reg = reg + reg
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_int_register() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBISD(src1p.ireg(), src2p.ireg(), 1, 0));    // lea   dstp,[src1p+src2p]

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_add_r32_p32(dst, dstreg, src2p, inst);                     // add   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_add_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // add   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                      // mov   dstreg:dstp,[src1p]
			emit_add_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // add   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_addc - process a ADDC opcode
//-------------------------------------------------

void drcbe_x86::op_addc(x86code *&dst, const instruction &inst)
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
			emit_adc_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // adc   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                 // mov   dstreg,src1p
			emit_adc_r32_p32(dst, dstreg, src2p, inst);                     // adc   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_adc_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // adc   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, REG_EDX, src1p);            // mov   dstreg:dstp,[src1p]
			emit_adc_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // adc   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_sub - process a SUB opcode
//-------------------------------------------------

void drcbe_x86::op_sub(x86code *&dst, const instruction &inst)
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
			emit_sub_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // sub   [dstp],src2p

		// reg = reg - imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && inst.flags() == 0)
			emit_lea_r32_m32(dst, dstp.ireg(), MBD(src1p.ireg(), -src2p.immediate()));          // lea   dstp,[src1p-src2p]

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_sub_r32_p32(dst, dstreg, src2p, inst);                     // sub   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sub_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // sub   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                      // mov   dstreg:dstp,[src1p]
			emit_sub_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // sub   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_subc - process a SUBC opcode
//-------------------------------------------------

void drcbe_x86::op_subc(x86code *&dst, const instruction &inst)
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
			emit_sbb_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // sbb   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                 // mov   dstreg,src1p
			emit_sbb_r32_p32(dst, dstreg, src2p, inst);                     // sbb   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_sbb_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // sbb   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(dst, dstreg, REG_EDX, src1p);            // mov   dstreg:dstp,[src1p]
			emit_sbb_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // sbb   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_cmp - process a CMP opcode
//-------------------------------------------------

void drcbe_x86::op_cmp(x86code *&dst, const instruction &inst)
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
			emit_cmp_m32_p32(dst, MABS(src1p.memory()), src2p, inst);               // cmp   [dstp],src2p

		// general case
		else
		{
			if (src1p.is_immediate())
				emit_mov_r32_imm(dst, src1reg, src1p.immediate());                          // mov   src1reg,imm
			emit_cmp_r32_p32(dst, src1reg, src2p, inst);                        // cmp   src1reg,src2p
		}
	}

	// 64-bit form
	else
	{
		// general case
		emit_mov_r64_p64(dst, REG_EAX, REG_EDX, src1p);                     // mov   eax:dstp,[src1p]
		emit_cmp_r64_p64(dst, REG_EAX, REG_EDX, src2p, inst);                   // cmp   eax:dstp,src2p
	}
}


//-------------------------------------------------
//  op_mulu - process a MULU opcode
//-------------------------------------------------

void drcbe_x86::op_mulu(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                  // mov   eax,src1p
		if (src2p.is_memory())
			emit_mul_m32(dst, MABS(src2p.memory()));                                        // mul   [src2p]
		else if (src2p.is_int_register())
			emit_mul_r32(dst, src2p.ireg());                                            // mul   src2p
		else if (src2p.is_immediate())
		{
			emit_mov_r32_imm(dst, REG_EDX, src2p.immediate());                              // mov   edx,src2p
			emit_mul_r32(dst, REG_EDX);                                             // mul   edx
		}
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                   // mov   dstp,eax
		if (compute_hi)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                              // mov   edstp,edx

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
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                     // or    edx,eax
					else if (zsflags == FLAG_S)
						emit_test_r32_r32(dst, REG_EDX, REG_EDX);                       // test  edx,edx
					else
					{
						emit_movzx_r32_r16(dst, REG_ECX, REG_AX);                       // movzx ecx,ax
						emit_shr_r32_imm(dst, REG_EAX, 16);                         // shr   eax,16
						emit_or_r32_r32(dst, REG_EDX, REG_ECX);                     // or    edx,ecx
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                     // or    edx,eax
					}
				}
				else
					emit_test_r32_r32(dst, REG_EAX, REG_EAX);                           // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r32(dst, REG_EAX);                                     // pop   eax
					emit_and_m32_imm(dst, MBD(REG_ESP, 0), ~0x84);                      // and   [esp],~0x84
					emit_or_m32_r32(dst, MBD(REG_ESP, 0), REG_EAX);                 // or    [esp],eax
					emit_popf(dst);                                                 // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m32_imm(dst, MBD(REG_ESP, 24), inst.flags());                          // mov   [esp+24],flags
		emit_mov_m64_p64(dst, MBD(REG_ESP, 16), src2p);                     // mov   [esp+16],src2p
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), src1p);                          // mov   [esp+8],src1p
		if (!compute_hi)
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reslo);             // mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reshi);             // mov   [esp+4],&reshi
		emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_reslo);                 // mov   [esp],&reslo
		emit_call(dst, (x86code *)dmulu);                                               // call  dmulu
		if (inst.flags() != 0)
			emit_push_m32(dst, MABSI(flags_unmap, REG_EAX, 4));                         // push   flags_unmap[eax*4]
		emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reslo + 0));               // mov   eax,reslo.lo
		emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reslo + 1));               // mov   edx,reslo.hi
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
		if (compute_hi)
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reshi + 0));           // mov   eax,reshi.lo
			emit_mov_r32_m32(dst, REG_ECX, MABS((UINT32 *)&m_reshi + 1));           // mov   ecx,reshi.hi
			emit_mov_p64_r64(dst, edstp, REG_EAX, REG_ECX);                 // mov   edstp,ecx:eax
		}
		if (inst.flags() != 0)
			emit_popf(dst);                                                         // popf
	}
}


//-------------------------------------------------
//  op_muls - process a MULS opcode
//-------------------------------------------------

void drcbe_x86::op_muls(x86code *&dst, const instruction &inst)
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
				emit_imul_r32_m32_imm(dst, REG_EAX, MABS(src1p.memory()), src2p.immediate());   // imul  eax,[src1p],src2p
			else if (src1p.is_int_register())
				emit_imul_r32_r32_imm(dst, REG_EAX, src1p.ireg(), src2p.immediate());           // imul  eax,src1p,src2p
			emit_mov_p32_r32(dst, dstp, REG_EAX);                               // mov   dstp,eax
		}

		// 32-bit destination, general case
		else if (!compute_hi)
		{
			emit_mov_r32_p32(dst, REG_EAX, src1p);                              // mov   eax,src1p
			if (src2p.is_memory())
				emit_imul_r32_m32(dst, REG_EAX, MABS(src2p.memory()));                  // imul  eax,[src2p]
			else if (src2p.is_int_register())
				emit_imul_r32_r32(dst, REG_EAX, src2p.ireg());                          // imul  eax,src2p
			emit_mov_p32_r32(dst, dstp, REG_EAX);                               // mov   dstp,eax
		}

		// 64-bit destination, general case
		else
		{
			emit_mov_r32_p32(dst, REG_EAX, src1p);                              // mov   eax,src1p
			if (src2p.is_memory())
				emit_imul_m32(dst, MABS(src2p.memory()));                                   // imul  [src2p]
			else if (src2p.is_int_register())
				emit_imul_r32(dst, src2p.ireg());                                       // imul  src2p
			else if (src2p.is_immediate())
			{
				emit_mov_r32_imm(dst, REG_EDX, src2p.immediate());                          // mov   edx,src2p
				emit_imul_r32(dst, REG_EDX);                                            // imul  edx
			}
			emit_mov_p32_r32(dst, dstp, REG_EAX);                               // mov   dstp,eax
			emit_mov_p32_r32(dst, edstp, REG_EDX);                              // mov   edstp,edx
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
					if (inst.flags() == FLAG_Z)
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                     // or    edx,eax
					else if (inst.flags() == FLAG_S)
						emit_test_r32_r32(dst, REG_EDX, REG_EDX);                       // test  edx,edx
					else
					{
						emit_movzx_r32_r16(dst, REG_ECX, REG_AX);                       // movzx ecx,ax
						emit_shr_r32_imm(dst, REG_EAX, 16);                         // shr   eax,16
						emit_or_r32_r32(dst, REG_EDX, REG_ECX);                     // or    edx,ecx
						emit_or_r32_r32(dst, REG_EDX, REG_EAX);                     // or    edx,eax
					}
				}
				else
					emit_test_r32_r32(dst, REG_EAX, REG_EAX);                           // test  eax,eax

				// we rely on the fact that OF is cleared by all logical operations above
				if (vflag)
				{
					emit_pushf(dst);                                                    // pushf
					emit_pop_r32(dst, REG_EAX);                                     // pop   eax
					emit_and_m32_imm(dst, MBD(REG_ESP, 0), ~0x84);                      // and   [esp],~0x84
					emit_or_m32_r32(dst, MBD(REG_ESP, 0), REG_EAX);                 // or    [esp],eax
					emit_popf(dst);                                                 // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m32_imm(dst, MBD(REG_ESP, 24), inst.flags());                          // mov   [esp+24],flags
		emit_mov_m64_p64(dst, MBD(REG_ESP, 16), src2p);                     // mov   [esp+16],src2p
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), src1p);                          // mov   [esp+8],src1p
		if (!compute_hi)
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reslo);             // mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reshi);             // push  [esp+4],&reshi
		emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_reslo);                 // mov   [esp],&reslo
		emit_call(dst, (x86code *)dmuls);                                               // call  dmuls
		if (inst.flags() != 0)
			emit_push_m32(dst, MABSI(flags_unmap, REG_EAX, 4));                         // push   flags_unmap[eax*4]
		emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reslo + 0));               // mov   eax,reslo.lo
		emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reslo + 1));               // mov   edx,reslo.hi
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
		if (compute_hi)
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reshi + 0));           // mov   eax,reshi.lo
			emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reshi + 1));           // mov   edx,reshi.hi
			emit_mov_p64_r64(dst, edstp, REG_EAX, REG_EDX);                 // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			emit_popf(dst);                                                         // popf
	}
}


//-------------------------------------------------
//  op_divu - process a DIVU opcode
//-------------------------------------------------

void drcbe_x86::op_divu(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, REG_ECX, src2p);                                  // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                             // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jecxz_link(dst, skip);                                                 // jecxz skip
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                  // mov   eax,src1p
		emit_xor_r32_r32(dst, REG_EDX, REG_EDX);                                        // xor   edx,edx
		emit_div_r32(dst, REG_ECX);                                                 // div   ecx
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                   // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                              // mov   edstp,edx
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, REG_EAX, REG_EAX);                                   // test  eax,eax
		track_resolve_link(dst, skip);                                              // skip:
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m64_p64(dst, MBD(REG_ESP, 16), src2p);                     // mov   [esp+16],src2p
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), src1p);                          // mov   [esp+8],src1p
		if (!compute_rem)
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reslo);             // mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reshi);             // push  [esp+4],&reshi
		emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_reslo);                 // mov   [esp],&reslo
		emit_call(dst, (x86code *)ddivu);                                               // call  ddivu
		if (inst.flags() != 0)
			emit_push_m32(dst, MABSI(flags_unmap, REG_EAX, 4));                         // push   flags_unmap[eax*4]
		emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reslo + 0));               // mov   eax,reslo.lo
		emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reslo + 1));               // mov   edx,reslo.hi
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
		if (compute_rem)
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reshi + 0));           // mov   eax,reshi.lo
			emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reshi + 1));           // mov   edx,reshi.hi
			emit_mov_p64_r64(dst, edstp, REG_EAX, REG_EDX);                 // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			emit_popf(dst);                                                         // popf
	}
}


//-------------------------------------------------
//  op_divs - process a DIVS opcode
//-------------------------------------------------

void drcbe_x86::op_divs(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, REG_ECX, src2p);                                  // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			emit_mov_r32_imm(dst, REG_EAX, 0xa0000000);                             // mov   eax,0xa0000000
			emit_add_r32_r32(dst, REG_EAX, REG_EAX);                                    // add   eax,eax
		}
		emit_link skip;
		emit_jecxz_link(dst, skip);                                                 // jecxz skip
		emit_mov_r32_p32(dst, REG_EAX, src1p);                                  // mov   eax,src1p
		emit_cdq(dst);                                                                  // cdq
		emit_idiv_r32(dst, REG_ECX);                                                    // idiv  ecx
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                   // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(dst, edstp, REG_EDX);                              // mov   edstp,edx
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, REG_EAX, REG_EAX);                                   // test  eax,eax
		track_resolve_link(dst, skip);                                              // skip:
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m64_p64(dst, MBD(REG_ESP, 16), src2p);                     // mov   [esp+16],src2p
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), src1p);                          // mov   [esp+8],src1p
		if (!compute_rem)
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reslo);             // mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(dst, MBD(REG_ESP, 4), (FPTR)&m_reshi);             // push  [esp+4],&reshi
		emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)&m_reslo);                 // mov   [esp],&reslo
		emit_call(dst, (x86code *)ddivs);                                               // call  ddivs
		if (inst.flags() != 0)
			emit_push_m32(dst, MABSI(flags_unmap, REG_EAX, 4));                         // push   flags_unmap[eax*4]
		emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reslo + 0));               // mov   eax,reslo.lo
		emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reslo + 1));               // mov   edx,reslo.hi
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
		if (compute_rem)
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS((UINT32 *)&m_reshi + 0));           // mov   eax,reshi.lo
			emit_mov_r32_m32(dst, REG_EDX, MABS((UINT32 *)&m_reshi + 1));           // mov   edx,reshi.hi
			emit_mov_p64_r64(dst, edstp, REG_EAX, REG_EDX);                 // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			emit_popf(dst);                                                         // popf
	}
}


//-------------------------------------------------
//  op_and - process a AND opcode
//-------------------------------------------------

void drcbe_x86::op_and(x86code *&dst, const instruction &inst)
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
			emit_and_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // and   [dstp],src2p

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r8(dst, dstreg, src1p.ireg());                           // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m8(dst, dstreg, MABS(src1p.memory()));                       // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r16(dst, dstreg, src1p.ireg());                          // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m16(dst, dstreg, MABS(src1p.memory()));                  // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_and_r32_p32(dst, dstreg, src2p, inst);                     // and   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_and_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // and   [dstp],src2p

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r8(dst, dstreg, src1p.ireg());                           // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m8(dst, dstreg, MABS(src1p.memory()));                       // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
			if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   dsthi,0
			else if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   dsthi,0
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && inst.flags() == 0)
		{
			if (src1p.is_int_register())
				emit_movzx_r32_r16(dst, dstreg, src1p.ireg());                          // movzx dstreg,src1p
			else if (src1p.is_memory())
				emit_movzx_r32_m16(dst, dstreg, MABS(src1p.memory()));                  // movzx dstreg,[src1p]
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
			if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   dsthi,0
			else if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   dsthi,0
		}

		// AND with immediate 0xffffffff
		else if (src2p.is_immediate_value(0xffffffff) && inst.flags() == 0)
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
			if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   dsthi,0
			else if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   dsthi,0
		}

		// AND with immediate 0xffffffff00000000
		else if (src2p.is_immediate_value(U64(0xffffffff00000000)) && inst.flags() == 0)
		{
			if (src1p != dstp)
			{
				emit_mov_r64_p64(dst, REG_NONE, REG_EDX, src1p);                // mov   dstreg,src1p
				emit_mov_p64_r64(dst, dstp, REG_NONE, REG_EDX);             // mov   dstp,dstreg
			}
			if (dstp.is_int_register())
				emit_xor_r32_r32(dst, dstp.ireg(), dstp.ireg());                            // xor   dstlo,dstlo
			else if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory()), 0);                          // mov   dstlo,0
		}

		// AND with immediate <= 0xffffffff
		else if (src2p.is_immediate() && src2p.immediate() <= 0xffffffff && inst.flags() == 0)
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_and_r32_p32(dst, dstreg, src2p, inst);                     // and   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
			if (dstp.is_int_register())
				emit_mov_m32_imm(dst, MABS(m_reghi[dstp.ireg()]), 0);               // mov   dsthi,0
			else if (dstp.is_memory())
				emit_mov_m32_imm(dst, MABS(dstp.memory(4)), 0);                     // mov   dsthi,0
		}

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                      // mov   dstreg:dstp,[src1p]
			emit_and_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // and   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_test - process a TEST opcode
//-------------------------------------------------

void drcbe_x86::op_test(x86code *&dst, const instruction &inst)
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
			emit_test_m32_p32(dst, MABS(src1p.memory()), src2p, inst);          // test  [src1p],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, src1reg, src1p);                              // mov   src1reg,src1p
			emit_test_r32_p32(dst, src1reg, src2p, inst);                       // test  src1reg,src2p
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// src1p in memory
		if (src1p.is_memory())
			emit_test_m64_p64(dst, MABS(src1p.memory()), src2p, inst);          // test  [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, src1reg, REG_EDX, src1p);                 // mov   src1reg:dstp,[src1p]
			emit_test_r64_p64(dst, src1reg, REG_EDX, src2p, inst);              // test  src1reg:dstp,src2p
		}
	}
}


//-------------------------------------------------
//  op_or - process a OR opcode
//-------------------------------------------------

void drcbe_x86::op_or(x86code *&dst, const instruction &inst)
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
			emit_or_m32_p32(dst, MABS(dstp.memory()), src2p, inst);             // or    [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_or_r32_p32(dst, dstreg, src2p, inst);                          // or    dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_or_m64_p64(dst, MABS(dstp.memory()), src2p, inst);             // or    [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                      // mov   dstreg:dstp,[src1p]
			emit_or_r64_p64(dst, dstreg, REG_EDX, src2p, inst);             // or    dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_xor - process a XOR opcode
//-------------------------------------------------

void drcbe_x86::op_xor(x86code *&dst, const instruction &inst)
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
			emit_xor_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // xor   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_xor_r32_p32(dst, dstreg, src2p, inst);                     // xor   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_xor_m64_p64(dst, MABS(dstp.memory()), src2p, inst);                // xor   [dstp],src2p

		// general case
		else
		{
			emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                      // mov   dstreg:dstp,[src1p]
			emit_xor_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                // xor   dstreg:dstp,src2p
			emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                       // mov   dstp,dstreg:eax
		}
	}
}


//-------------------------------------------------
//  op_lzcnt - process a LZCNT opcode
//-------------------------------------------------

void drcbe_x86::op_lzcnt(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, dstreg, srcp);                                    // mov   dstreg,src1p
		emit_mov_r32_imm(dst, REG_ECX, 32 ^ 31);                                        // mov   ecx,32 ^ 31
		emit_bsr_r32_r32(dst, dstreg, dstreg);                                          // bsr   dstreg,dstreg
		emit_cmovcc_r32_r32(dst, x86emit::COND_Z, dstreg, REG_ECX);                             // cmovz dstreg,ecx
		emit_xor_r32_imm(dst, dstreg, 31);                                              // xor   dstreg,31
		emit_mov_p32_r32(dst, dstp, dstreg);                                    // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, REG_EDX, dstreg, srcp);                           // mov   dstreg:edx,srcp
		emit_bsr_r32_r32(dst, dstreg, dstreg);                                          // bsr   dstreg,dstreg
		emit_link skip;
		emit_jcc_short_link(dst, x86emit::COND_NZ, skip);                                       // jnz   skip
		emit_mov_r32_imm(dst, REG_ECX, 32 ^ 31);                                        // mov   ecx,32 ^ 31
		emit_bsr_r32_r32(dst, dstreg, REG_EDX);                                     // bsr   dstreg,edx
		emit_cmovcc_r32_r32(dst, x86emit::COND_Z, dstreg, REG_ECX);                             // cmovz dstreg,ecx
		emit_add_r32_imm(dst, REG_ECX, 32);                                         // add   ecx,32
		track_resolve_link(dst, skip);                                              // skip:
		emit_xor_r32_r32(dst, REG_EDX, REG_EDX);                                        // xor   edx,edx
		emit_xor_r32_imm(dst, dstreg, 31);                                              // xor   dstreg,31
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_bswap - process a BSWAP opcode
//-------------------------------------------------

void drcbe_x86::op_bswap(x86code *&dst, const instruction &inst)
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
		emit_mov_r32_p32(dst, dstreg, srcp);                                    // mov   dstreg,src1p
		emit_bswap_r32(dst, dstreg);                                                    // bswap dstreg
		if (inst.flags() != 0)
			emit_test_r32_r32(dst, dstreg, dstreg);                                 // test  dstreg,dstreg
		emit_mov_p32_r32(dst, dstp, dstreg);                                    // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(dst, REG_EDX, dstreg, srcp);                           // mov   dstreg:edx,srcp
		emit_bswap_r32(dst, dstreg);                                                    // bswap dstreg
		emit_bswap_r32(dst, REG_EDX);                                                   // bswap edx
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,edx:dstreg
		if (inst.flags() == FLAG_Z)
			emit_or_r32_r32(dst, REG_EDX, dstreg);                                      // or    edx,eax
		else if (inst.flags() == FLAG_S)
			emit_test_r32_r32(dst, REG_EDX, REG_EDX);                                   // test  edx,edx
		else
		{
			emit_movzx_r32_r16(dst, REG_ECX, dstreg);                                   // movzx ecx,dstreg
			emit_or_r32_r32(dst, REG_EDX, REG_ECX);                                 // or    edx,ecx
			emit_mov_r32_r32(dst, REG_ECX, dstreg);                                 // mov   ecx,dstreg
			emit_shr_r32_imm(dst, REG_ECX, 16);                                     // shr   ecx,16
			emit_or_r32_r32(dst, REG_EDX, REG_ECX);                                 // or    edx,ecx
		}
	}
}


//-------------------------------------------------
//  op_shl - process a SHL opcode
//-------------------------------------------------

void drcbe_x86::op_shl(x86code *&dst, const instruction &inst)
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
			emit_shl_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // shl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_shl_r32_p32(dst, dstreg, src2p, inst);                     // shl   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                          // mov   dstreg:dstp,[src1p]
		emit_shl_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // shl   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_shr - process a SHR opcode
//-------------------------------------------------

void drcbe_x86::op_shr(x86code *&dst, const instruction &inst)
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
			emit_shr_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // shr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_shr_r32_p32(dst, dstreg, src2p, inst);                     // shr   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                          // mov   dstreg:dstp,[src1p]
		emit_shr_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // shr   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_sar - process a SAR opcode
//-------------------------------------------------

void drcbe_x86::op_sar(x86code *&dst, const instruction &inst)
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
			emit_sar_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // sar   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_sar_r32_p32(dst, dstreg, src2p, inst);                     // sar   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                          // mov   dstreg:dstp,[src1p]
		emit_sar_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // sar   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_rol - process a rol opcode
//-------------------------------------------------

void drcbe_x86::op_rol(x86code *&dst, const instruction &inst)
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
			emit_rol_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // rol   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_rol_r32_p32(dst, dstreg, src2p, inst);                     // rol   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                          // mov   dstreg:dstp,[src1p]
		emit_rol_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // rol   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_ror - process a ROR opcode
//-------------------------------------------------

void drcbe_x86::op_ror(x86code *&dst, const instruction &inst)
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
			emit_ror_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // ror   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32(dst, dstreg, src1p);                               // mov   dstreg,src1p
			emit_ror_r32_p32(dst, dstreg, src2p, inst);                     // ror   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(dst, dstreg, REG_EDX, src1p);                          // mov   dstreg:dstp,[src1p]
		emit_ror_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // ror   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_rolc - process a ROLC opcode
//-------------------------------------------------

void drcbe_x86::op_rolc(x86code *&dst, const instruction &inst)
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
			emit_rcl_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // rcl   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                 // mov   dstreg,src1p
			emit_rcl_r32_p32(dst, dstreg, src2p, inst);                     // rcl   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64_keepflags(dst, dstreg, REG_EDX, src1p);                // mov   dstreg:dstp,[src1p]
		emit_rcl_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // rcl   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}


//-------------------------------------------------
//  op_rorc - process a RORC opcode
//-------------------------------------------------

void drcbe_x86::op_rorc(x86code *&dst, const instruction &inst)
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
			emit_rcr_m32_p32(dst, MABS(dstp.memory()), src2p, inst);                // rcr   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(dst, dstreg, src1p);                 // mov   dstreg,src1p
			emit_rcr_r32_p32(dst, dstreg, src2p, inst);                     // rcr   dstreg,src2p
			emit_mov_p32_r32(dst, dstp, dstreg);                                // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64_keepflags(dst, dstreg, REG_EDX, src1p);                // mov   dstreg:dstp,[src1p]
		emit_rcr_r64_p64(dst, dstreg, REG_EDX, src2p, inst);                    // rcr   dstreg:dstp,src2p
		emit_mov_p64_r64(dst, dstp, dstreg, REG_EDX);                           // mov   dstp,dstreg:eax
	}
}



//**************************************************************************
//  FLOATING POINT OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  op_fload - process a FLOAD opcode
//-------------------------------------------------

void drcbe_x86::op_fload(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);

	// immediate index
	if (indp.is_immediate())
	{
		emit_mov_r32_m32(dst, REG_EAX, MABS(basep.memory(4*indp.immediate())));             // mov   eax,[basep + 4*indp]
		if (inst.size() == 8)
			emit_mov_r32_m32(dst, REG_EDX, MABS(basep.memory(4 + 4*indp.immediate())));     // mov   edx,[basep + 4*indp + 4]
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_mov_r32_p32(dst, indreg, indp);
		emit_mov_r32_m32(dst, REG_EAX, MABSI(basep.memory(), indreg, 4));                   // mov   eax,[basep + 4*indp]
		if (inst.size() == 8)
			emit_mov_r32_m32(dst, REG_EDX, MABSI(basep.memory(4), indreg, 4));          // mov   edx,[basep + 4*indp + 4]
	}

	// general case
	emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                                    // mov   [dstp],eax
	if (inst.size() == 8)
		emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                           // mov   [dstp + 4],edx
}


//-------------------------------------------------
//  op_fstore - process a FSTORE opcode
//-------------------------------------------------

void drcbe_x86::op_fstore(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MF);

	// general case
	emit_mov_r32_m32(dst, REG_EAX, MABS(srcp.memory()));                                    // mov   eax,[srcp]
	if (inst.size() == 8)
		emit_mov_r32_m32(dst, REG_EDX, MABS(srcp.memory(4)));                           // mov   edx,[srcp + 4]

	// immediate index
	if (indp.is_immediate())
	{
		emit_mov_m32_r32(dst, MABS(basep.memory(4*indp.immediate())), REG_EAX);             // mov   [basep + 4*indp],eax
		if (inst.size() == 8)
			emit_mov_m32_r32(dst, MABS(basep.memory(4 + 4*indp.immediate())), REG_EDX);     // mov   [basep + 4*indp + 4],edx
	}

	// other index
	else
	{
		int indreg = indp.select_register(REG_ECX);
		emit_mov_r32_p32(dst, indreg, indp);
		emit_mov_m32_r32(dst, MABSI(basep.memory(), indreg, 4), REG_EAX);                   // mov   [basep + 4*indp],eax
		if (inst.size() == 8)
			emit_mov_m32_r32(dst, MABSI(basep.memory(4), indreg, 4), REG_EDX);          // mov   [basep + 4*indp + 4],edx
	}
}


//-------------------------------------------------
//  op_fread - process a FREAD opcode
//-------------------------------------------------

void drcbe_x86::op_fread(x86code *&dst, const instruction &inst)
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
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacep.space()]);      // mov    [esp],space
	if (inst.size() == 4)
		emit_call(dst, (x86code *)m_accessors[spacep.space()].read_dword);  // call   read_dword
	else if (inst.size() == 8)
		emit_call(dst, (x86code *)m_accessors[spacep.space()].read_qword);  // call   read_qword

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(dst, dstp, REG_EAX);                                   // mov   dstp,eax
	else if (inst.size() == 8)
		emit_mov_p64_r64(dst, dstp, REG_EAX, REG_EDX);                          // mov   dstp,edx:eax
}


//-------------------------------------------------
//  op_fwrite - process a FWRITE opcode
//-------------------------------------------------

void drcbe_x86::op_fwrite(x86code *&dst, const instruction &inst)
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

	// set up a call to the write dword/qword handler
	if (inst.size() == 4)
		emit_mov_m32_p32(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	else if (inst.size() == 8)
		emit_mov_m64_p64(dst, MBD(REG_ESP, 8), srcp);                           // mov    [esp+8],srcp
	emit_mov_m32_p32(dst, MBD(REG_ESP, 4), addrp);                              // mov    [esp+4],addrp
	emit_mov_m32_imm(dst, MBD(REG_ESP, 0), (FPTR)m_space[spacep.space()]);      // mov    [esp],space
	if (inst.size() == 4)
		emit_call(dst, (x86code *)m_accessors[spacep.space()].write_dword); // call   write_dword
	else if (inst.size() == 8)
		emit_call(dst, (x86code *)m_accessors[spacep.space()].write_qword); // call   write_qword
}


//-------------------------------------------------
//  op_fmov - process a FMOV opcode
//-------------------------------------------------

void drcbe_x86::op_fmov(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// always start with a jmp
	emit_link skip = { 0 };
	if (inst.condition() != uml::COND_ALWAYS)
		emit_jcc_short_link(dst, X86_NOT_CONDITION(inst.condition()), skip);            // jcc   skip

	// general case
	emit_mov_r32_m32(dst, REG_EAX, MABS(srcp.memory()));                                    // mov   eax,[srcp]
	if (inst.size() == 8)
		emit_mov_r32_m32(dst, REG_EDX, MABS(srcp.memory(4)));                           // mov   edx,[srcp + 4]
	emit_mov_m32_r32(dst, MABS(dstp.memory()), REG_EAX);                                    // mov   [dstp],eax
	if (inst.size() == 8)
		emit_mov_m32_r32(dst, MABS(dstp.memory(4)), REG_EDX);                           // mov   [dstp + 4],edx

	// resolve the jump
	if (skip.target != NULL)
		track_resolve_link(dst, skip);                                              // skip:
}


//-------------------------------------------------
//  op_ftoint - process a FTOINT opcode
//-------------------------------------------------

void drcbe_x86::op_ftoint(x86code *&dst, const instruction &inst)
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

	// set rounding mode if necessary
	if (roundp.rounding() != ROUND_DEFAULT && (!m_sse3 || roundp.rounding() != ROUND_TRUNC))
	{
		emit_fstcw_m16(dst, MABS(&m_fmodesave));                                    // fstcw [fmodesave]
		emit_fldcw_m16(dst, MABS(&fp_control[roundp.rounding()]));                          // fldcw fpcontrol[roundp]
	}

	// general case
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp

	// 4-byte integer case
	if (sizep.size() == SIZE_DWORD)
	{
		if (dstp.is_memory())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				emit_fistp_m32(dst, MABS(dstp.memory()));                                   // fistp [dstp]
			else
				emit_fisttp_m32(dst, MABS(dstp.memory()));                              // fisttp [dstp]
		}
		else if (dstp.is_int_register())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				emit_fistp_m32(dst, MABS(m_reglo[dstp.ireg()]));                    // fistp reglo[dstp]
			else
				emit_fisttp_m32(dst, MABS(m_reglo[dstp.ireg()]));                   // fisttp reglo[dstp]
			emit_mov_r32_m32(dst, dstp.ireg(), MABS(m_reglo[dstp.ireg()]));         // mov   dstp,reglo[dstp]
		}
	}

	// 8-byte integer case
	else if (sizep.size() == SIZE_QWORD)
	{
		if (dstp.is_memory())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				emit_fistp_m64(dst, MABS(dstp.memory()));                                   // fistp [dstp]
			else
				emit_fisttp_m64(dst, MABS(dstp.memory()));                              // fisttp [dstp]
		}
		else if (dstp.is_int_register())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				emit_fistp_m64(dst, MABS(m_reglo[dstp.ireg()]));                    // fistp reglo[dstp]
			else
				emit_fisttp_m64(dst, MABS(m_reglo[dstp.ireg()]));                   // fisttp reglo[dstp]
			emit_mov_r32_m32(dst, dstp.ireg(), MABS(m_reglo[dstp.ireg()]));         // mov   dstp,reglo[dstp]
		}
	}

	// restore control word and proceed
	if (roundp.rounding() != ROUND_DEFAULT && (!m_sse3 || roundp.rounding() != ROUND_TRUNC))
		emit_fldcw_m16(dst, MABS(&m_fmodesave));                                    // fldcw [fmodesave]
}


//-------------------------------------------------
//  op_ffrint - process a FFRINT opcode
//-------------------------------------------------

void drcbe_x86::op_ffrint(x86code *&dst, const instruction &inst)
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

	// 4-byte integer case
	if (sizep.size() == SIZE_DWORD)
	{
		if (srcp.is_immediate())
		{
			emit_mov_m32_imm(dst, MABS(&m_fptemp), srcp.immediate());                   // mov   [fptemp],srcp
			emit_fild_m32(dst, MABS(&m_fptemp));                                    // fild  [fptemp]
		}
		else if (srcp.is_memory())
			emit_fild_m32(dst, MABS(srcp.memory()));                                        // fild  [srcp]
		else if (srcp.is_int_register())
		{
			emit_mov_m32_r32(dst, MABS(m_reglo[srcp.ireg()]), srcp.ireg());         // mov   reglo[srcp],srcp
			emit_fild_m32(dst, MABS(m_reglo[srcp.ireg()]));                     // fild  reglo[srcp]
		}
	}

	// 8-bit integer case
	else if (sizep.size() == SIZE_QWORD)
	{
		if (srcp.is_immediate())
		{
			emit_mov_m32_imm(dst, MABS(&m_fptemp), srcp.immediate());                   // mov   [fptemp],srcp
			emit_mov_m32_imm(dst, MABS((UINT8 *)&m_fptemp + 4), srcp.immediate());      // mov   [fptemp+4],srcp
			emit_fild_m64(dst, MABS(&m_fptemp));                                    // fild  [fptemp]
		}
		else if (srcp.is_memory())
			emit_fild_m64(dst, MABS(srcp.memory()));                                        // fild  [srcp]
		else if (srcp.is_int_register())
		{
			emit_mov_m32_r32(dst, MABS(m_reglo[srcp.ireg()]), srcp.ireg());         // mov   reglo[srcp],srcp
			emit_fild_m64(dst, MABS(m_reglo[srcp.ireg()]));                     // fild  reglo[srcp]
		}
	}

	// store the result and be done
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  [dstp]
}


//-------------------------------------------------
//  op_ffrflt - process a FFRFLT opcode
//-------------------------------------------------

void drcbe_x86::op_ffrflt(x86code *&dst, const instruction &inst)
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

	// general case
	if (sizep.size() == SIZE_DWORD)
		emit_fld_m32(dst, MABS(srcp.memory()));                                         // fld   [srcp]
	else if (sizep.size() == SIZE_QWORD)
		emit_fld_m64(dst, MABS(srcp.memory()));                                         // fld   [srcp]
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_frnds - process a FRNDS opcode
//-------------------------------------------------

void drcbe_x86::op_frnds(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fstp_m32(dst, MABS(&m_fptemp));                                            // fstp  [fptemp]
	emit_fld_m32(dst, MABS(&m_fptemp));                                         // fld   [fptemp]
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  [dstp]
}


//-------------------------------------------------
//  op_fadd - process a FADD opcode
//-------------------------------------------------

void drcbe_x86::op_fadd(x86code *&dst, const instruction &inst)
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

	// general case
	emit_fld_p(dst, inst.size(), src1p);                                                // fld   src1p
	emit_fld_p(dst, inst.size(), src2p);                                                // fld   src2p
	emit_faddp(dst);                                                                    // faddp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fsub - process a FSUB opcode
//-------------------------------------------------

void drcbe_x86::op_fsub(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), src1p);                                                // fld   src1p
	emit_fld_p(dst, inst.size(), src2p);                                                // fld   src2p
	emit_fsubp(dst);                                                                    // fsubp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fcmp - process a FCMP opcode
//-------------------------------------------------

void drcbe_x86::op_fcmp(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_U);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MF);
	be_parameter src2p(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), src2p);                                                // fld   src2p
	emit_fld_p(dst, inst.size(), src1p);                                                // fld   src1p
	emit_fcompp(dst);                                                                   // fcompp
	emit_fstsw_ax(dst);                                                             // fnstsw ax
	emit_sahf(dst);                                                                 // sahf
}


//-------------------------------------------------
//  op_fmul - process a FMUL opcode
//-------------------------------------------------

void drcbe_x86::op_fmul(x86code *&dst, const instruction &inst)
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

	// general case
	emit_fld_p(dst, inst.size(), src1p);                                                // fld   src1p
	emit_fld_p(dst, inst.size(), src2p);                                                // fld   src2p
	emit_fmulp(dst);                                                                    // fmulp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fdiv - process a FDIV opcode
//-------------------------------------------------

void drcbe_x86::op_fdiv(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), src1p);                                                // fld   src1p
	emit_fld_p(dst, inst.size(), src2p);                                                // fld   src2p
	emit_fdivp(dst);                                                                    // fdivp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fneg - process a FNEG opcode
//-------------------------------------------------

void drcbe_x86::op_fneg(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fchs(dst);                                                                 // fchs
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fabs - process a FABS opcode
//-------------------------------------------------

void drcbe_x86::op_fabs(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fabs(dst);                                                                 // fabs
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_fsqrt - process a FSQRT opcode
//-------------------------------------------------

void drcbe_x86::op_fsqrt(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fsqrt(dst);                                                                    // fsqrt
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_frecip - process a FRECIP opcode
//-------------------------------------------------

void drcbe_x86::op_frecip(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld1(dst);                                                                 // fld1
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fdivp(dst);                                                                    // fdivp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}


//-------------------------------------------------
//  op_frsqrt - process a FRSQRT opcode
//-------------------------------------------------

void drcbe_x86::op_frsqrt(x86code *&dst, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld1(dst);                                                                 // fld1
	emit_fld_p(dst, inst.size(), srcp);                                             // fld   srcp
	emit_fsqrt(dst);                                                                    // fsqrt
	emit_fdivp(dst);                                                                    // fdivp
	emit_fstp_p(dst, inst.size(), dstp);                                                // fstp  dstp
}



//**************************************************************************
//  MISCELLAENOUS FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  dmulu - perform a double-wide unsigned multiply
//-------------------------------------------------

int drcbe_x86::dmulu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2, int flags)
{
	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == 0)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch source values
	UINT64 a = src1;
	UINT64 b = src2;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	UINT64 lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	UINT64 hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	// compute middle parts
	UINT64 prevlo = lo;
	UINT64 temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	// store the results
	dsthi = hi;
	dstlo = lo;
	return ((hi >> 60) & FLAG_S) | ((dsthi != 0) << 1);
}


//-------------------------------------------------
//  dmuls - perform a double-wide signed multiply
//-------------------------------------------------

int drcbe_x86::dmuls(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2, int flags)
{
	UINT64 lo, hi, prevlo;
	UINT64 a, b, temp;

	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == 0)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch absolute source values
	a = src1; if ((INT64)a < 0) a = -a;
	b = src2; if ((INT64)b < 0) b = -b;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	// compute middle parts
	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	// adjust for signage
	if ((INT64)(src1 ^ src2) < 0)
	{
		hi = ~hi + (lo == 0);
		lo = ~lo + 1;
	}

	// store the results
	dsthi = hi;
	dstlo = lo;
	return ((hi >> 60) & FLAG_S) | ((dsthi != ((INT64)lo >> 63)) << 1);
}


//-------------------------------------------------
//  ddivu - perform a double-wide unsigned divide
//-------------------------------------------------

int drcbe_x86::ddivu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2)
{
	// do nothing if src2 == 0
	if (src2 == 0)
		return FLAG_V;

	// shortcut if no remainder
	dstlo = src1 / src2;
	if (dstlo != dsthi)
		dsthi = src1 % src2;
	return ((dstlo == 0) << 2) | ((dstlo >> 60) & FLAG_S);
}


//-------------------------------------------------
//  ddivs - perform a double-wide signed divide
//-------------------------------------------------

int drcbe_x86::ddivs(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2)
{
	// do nothing if src2 == 0
	if (src2 == 0)
		return FLAG_V;

	// shortcut if no remainder
	dstlo = src1 / src2;
	if (dstlo != dsthi)
		dsthi = src1 % src2;
	return ((dstlo == 0) << 2) | ((dstlo >> 60) & FLAG_S);
}
