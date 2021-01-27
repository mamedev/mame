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

#include "emu.h"
#include "drcbex86.h"

#include "debugger.h"
#include "emuopts.h"
#include "drcuml.h"

#include <cstddef>


namespace drc {

using namespace uml;

using namespace asmjit;
using namespace asmjit::x86;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_HASHJMPS        (0)



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
//static const uint64_t size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, 0xffffffffffffffffU };

// register mapping tables
static const Gp::Id int_register_map[REG_I_COUNT] =
{
	Gp::kIdBx, Gp::kIdSi, Gp::kIdDi, Gp::kIdBp
};

// flags mapping tables
static uint8_t flags_map[0x1000];
static uint32_t flags_unmap[0x20];

// condition mapping table
static const Condition::Code condition_map[uml::COND_MAX - uml::COND_Z] =
{
	Condition::Code::kZ,    // COND_Z = 0x80,    requires Z
	Condition::Code::kNZ,   // COND_NZ,          requires Z
	Condition::Code::kS,    // COND_S,           requires S
	Condition::Code::kNS,   // COND_NS,          requires S
	Condition::Code::kC,    // COND_C,           requires C
	Condition::Code::kNC,   // COND_NC,          requires C
	Condition::Code::kO,    // COND_V,           requires V
	Condition::Code::kNO,   // COND_NV,          requires V
	Condition::Code::kP,    // COND_U,           requires U
	Condition::Code::kNP,   // COND_NU,          requires U
	Condition::Code::kA,    // COND_A,           requires CZ
	Condition::Code::kBE,   // COND_BE,          requires CZ
	Condition::Code::kG,    // COND_G,           requires SVZ
	Condition::Code::kLE,   // COND_LE,          requires SVZ
	Condition::Code::kL,    // COND_L,           requires SV
	Condition::Code::kGE,   // COND_GE,          requires SV
};

// FPU control register mapping
static const uint16_t fp_control[4] =
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
	{ uml::OP_TZCNT,   &drcbe_x86::op_tzcnt },      // TZCNT   dst,src[,f]
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
	{ uml::OP_FRSQRT,  &drcbe_x86::op_frsqrt },     // FRSQRT  dst,src1
	{ uml::OP_FCOPYI,  &drcbe_x86::op_fcopyi },     // FCOPYI  dst,src
	{ uml::OP_ICOPYF,  &drcbe_x86::op_icopyf },     // ICOPYF  dst,src
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

drcbe_x86::be_parameter::be_parameter(drcbe_x86 &drcbe, parameter const &param, uint32_t allowed)
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

inline Gp drcbe_x86::be_parameter::select_register(Gp const &defreg) const
{
	if (m_type == PTYPE_INT_REGISTER)
		return Gpd(m_value);
	return defreg;
}

inline Xmm drcbe_x86::be_parameter::select_register(Xmm defreg) const
{
	if (m_type == PTYPE_FLOAT_REGISTER)
		return Xmm(m_value);
	return defreg;
}

template <typename T> T drcbe_x86::be_parameter::select_register(T defreg, be_parameter const &checkparam) const
{
	if (*this == checkparam)
		return defreg;
	return select_register(defreg);
}

template <typename T> T drcbe_x86::be_parameter::select_register(T defreg, be_parameter const &checkparam, be_parameter const &checkparam2) const
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

inline void drcbe_x86::emit_combine_z_flags(Assembler &a)
{
	// this assumes that the flags from the low 32-bit op are on the stack
	// and the flags from the high 32-bit op are live
	a.pushfd();                                                                         // pushf
	a.mov(ecx, ptr(esp, 4));                                                            // mov   ecx,[esp+4]
	a.or_(ecx, ~0x40);                                                                  // or    ecx,~0x40
	a.and_(ptr(esp, 0), ecx);                                                           // and   [esp],ecx
	a.popfd();                                                                          // popf
	a.lea(esp, ptr(esp, 4));                                                            // lea   esp,[esp+4]
}


//-------------------------------------------------
//  emit_combine_z_shl_flags - combine the Z
//  flags from two 32-bit shift left operations
//-------------------------------------------------

inline void drcbe_x86::emit_combine_z_shl_flags(Assembler &a)
{
	// this assumes that the flags from the high 32-bit op are on the stack
	// and the flags from the low 32-bit op are live
	a.pushfd();                                                                         // pushf
	a.pop(ecx);                                                                         // pop   ecx
	a.or_(ecx, ~0x40);                                                                  // or    ecx,~0x40
	a.and_(ptr(esp, 0), ecx);                                                           // and   [esp],ecx
	a.popfd();                                                                          // popf
}


//-------------------------------------------------
//  reset_last_upper_lower_reg - reset the last
//  upper/lower register state
//-------------------------------------------------

inline void drcbe_x86::reset_last_upper_lower_reg()
{
	m_last_lower_reg = Gp();
	m_last_upper_reg = Gp();
}


//-------------------------------------------------
//  set_last_lower_reg - note that we have just
//  loaded a lower register
//-------------------------------------------------

inline void drcbe_x86::set_last_lower_reg(Assembler &a, be_parameter const &param, Gp const &reglo)
{
	if (param.is_memory())
	{
		m_last_lower_reg = reglo;
		m_last_lower_addr = (uint32_t *)((uintptr_t)param.memory());
		m_last_lower_pc = (x86code *)(a.code()->baseAddress() + a.offset());
	}
}


//-------------------------------------------------
//  set_last_upper_reg - note that we have just
//  loaded an upper register
//-------------------------------------------------

inline void drcbe_x86::set_last_upper_reg(Assembler &a, be_parameter const &param, Gp const &reghi)
{
	m_last_upper_reg = reghi;
	m_last_upper_addr = (param.is_int_register()) ? m_reghi[param.ireg()] : (uint32_t *)((uintptr_t)param.memory(4));
	m_last_upper_pc = (x86code *)(a.code()->baseAddress() + a.offset());
}


//-------------------------------------------------
//  can_skip_lower_load - return true if we can
//  skip re-loading a lower half of a register
//-------------------------------------------------

inline bool drcbe_x86::can_skip_lower_load(Assembler &a, uint32_t *memref, Gp const &reglo)
{
	return ((x86code *)(a.code()->baseAddress() + a.offset()) == m_last_lower_pc && memref == m_last_lower_addr && reglo == m_last_lower_reg);
}


//-------------------------------------------------
//  can_skip_upper_load - return true if we can
//  skip re-loading an upper half of a register
//-------------------------------------------------

inline bool drcbe_x86::can_skip_upper_load(Assembler &a, uint32_t *memref, Gp const &reghi)
{
	return ((x86code *)(a.code()->baseAddress() + a.offset()) == m_last_upper_pc && memref == m_last_upper_addr && reghi == m_last_upper_reg);
}


//-------------------------------------------------
//  drcbe_x86 - constructor
//-------------------------------------------------

drcbe_x86::drcbe_x86(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device),
		m_hash(cache, modes, addrbits, ignorebits),
		m_map(cache, 0),
		m_log(nullptr),
		m_log_asmjit(nullptr),
		m_logged_common(false),
		m_sse3(CpuInfo::host().features().as<Features>().hasSSE3()),
		m_entry(nullptr),
		m_exit(nullptr),
		m_nocode(nullptr),
		m_save(nullptr),
		m_restore(nullptr),
		m_last_lower_reg(Gp()),
		m_last_lower_pc(nullptr),
		m_last_lower_addr(nullptr),
		m_last_upper_reg(Gp()),
		m_last_upper_pc(nullptr),
		m_last_upper_addr(nullptr),
		m_fptemp(0),
		m_fpumode(0),
		m_fmodesave(0),
		m_stacksave(nullptr),
		m_hashstacksave(nullptr),
		m_reslo(0),
		m_reshi(0)
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
		uint8_t flags = 0;
		if (entry & 0x001) flags |= FLAG_C;
		if (entry & 0x004) flags |= FLAG_U;
		if (entry & 0x040) flags |= FLAG_Z;
		if (entry & 0x080) flags |= FLAG_S;
		if (entry & 0x800) flags |= FLAG_V;
		flags_map[entry] = flags;
	}
	for (int entry = 0; entry < ARRAY_LENGTH(flags_unmap); entry++)
	{
		uint32_t flags = 0;
		if (entry & FLAG_C) flags |= 0x001;
		if (entry & FLAG_U) flags |= 0x004;
		if (entry & FLAG_Z) flags |= 0x040;
		if (entry & FLAG_S) flags |= 0x080;
		if (entry & FLAG_V) flags |= 0x800;
		flags_unmap[entry] = flags;
	}

	// build the opcode table (static but it doesn't hurt to regenerate it)
	for (auto & elem : s_opcode_table_source)
		s_opcode_table[elem.opcode] = elem.func;

	// create the log
	if (device.machine().options().drc_log_native())
	{
		std::string filename = std::string("drcbex86_").append(device.shortname()).append(".asm");
		m_log = x86log_create_context(filename.c_str());
		m_log_asmjit = fopen(std::string("drcbex86_asmjit_").append(device.shortname()).append(".asm").c_str(), "w");
	}
}


//-------------------------------------------------
//  ~drcbe_x86 - destructor
//-------------------------------------------------

drcbe_x86::~drcbe_x86()
{
	// free the log context
	if (m_log != nullptr)
		x86log_free_context(m_log);

	if (m_log_asmjit)
		fclose(m_log_asmjit);
}

size_t drcbe_x86::emit(CodeHolder &ch)
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

	err = ch.copyFlattenedData(drccodeptr(ch.baseAddress()), code_size, CodeHolder::kCopyWithPadding);
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

void drcbe_x86::reset()
{
	// output a note to the log
	if (m_log != nullptr)
		x86log_printf(m_log, "%s", "\n\n===========\nCACHE RESET\n===========\n\n");

	// generate a little bit of glue code to set up the environment
	x86code *dst = (x86code *)m_cache.top();

	CodeHolder ch;
	ch.init(hostEnvironment(), uint64_t(dst));

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.setFlags(FormatOptions::Flags::kFlagHexOffsets | FormatOptions::Flags::kFlagHexImms | FormatOptions::Flags::kFlagMachineCode);
		logger.setIndentation(FormatOptions::IndentationType::kIndentationCode, 4);
		ch.setLogger(&logger);
	}

	Assembler a(&ch);
	if (logger.file())
		a.addValidationOptions(BaseEmitter::kValidationOptionIntermediate);

	// generate an entry point
	m_entry = (x86_entry_point_func)dst;
	a.bind(a.newNamedLabel("entry_point"));

	FuncDetail entry_point;
	entry_point.init(FuncSignatureT<uint32_t, x86code *>(CallConv::kIdHost), hostEnvironment());

	FuncFrame frame;
	frame.init(entry_point);
	frame.addDirtyRegs(ebx, esi, edi, ebp);
	FuncArgsAssignment args(&entry_point);
	args.assignAll(eax);
	args.updateFuncFrame(frame);
	frame.finalize();

	a.emitProlog(frame);
	a.emitArgsAssignment(frame, args);
	a.sub(esp, 24);                                                                     // sub   esp,24
	a.mov(MABS(&m_hashstacksave), esp);                                                 // mov   [hashstacksave],esp
	a.sub(esp, 4);                                                                      // sub   esp,4
	a.mov(MABS(&m_stacksave), esp);                                                     // mov   [stacksave],esp
	a.fnstcw(MABS(&m_fpumode));                                                         // fstcw [fpumode]
	a.jmp(eax);                                                                         // jmp   eax

	// generate an exit point
	m_exit = dst + a.offset();
	a.bind(a.newNamedLabel("exit_point"));
	a.fldcw(MABS(&m_fpumode));                                                          // fldcw [fpumode]
	a.mov(esp, MABS(&m_hashstacksave));                                                 // mov   esp,[hashstacksave]
	a.add(esp, 24);                                                                     // add   esp,24
	a.emitEpilog(frame);

	// generate a no code point
	m_nocode = dst + a.offset();
	a.bind(a.newNamedLabel("nocode_point"));
	a.ret();                                                                            // ret

	// generate a save subroutine
	m_save = dst + a.offset();
	a.bind(a.newNamedLabel("save"));
	a.pushfd();                                                                         // pushf
	a.pop(eax);                                                                         // pop    eax
	a.and_(eax, 0x8c5);                                                                 // and    eax,0x8c5
	a.mov(al, ptr(u64(flags_map), eax));                                                // mov    al,[flags_map]
	a.mov(ptr(ecx, offsetof(drcuml_machine_state, flags)), al);                         // mov    state->flags,al
	a.mov(al, MABS(&m_state.fmod));                                                     // mov    al,[fmod]
	a.mov(ptr(ecx, offsetof(drcuml_machine_state, fmod)), al);                          // mov    state->fmod,al
	a.mov(eax, MABS(&m_state.exp));                                                     // mov    eax,[exp]
	a.mov(ptr(ecx, offsetof(drcuml_machine_state, exp)), eax);                          // mov    state->exp,eax
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		uintptr_t regoffsl = (uintptr_t)&((drcuml_machine_state *)nullptr)->r[regnum].w.l;
		uintptr_t regoffsh = (uintptr_t)&((drcuml_machine_state *)nullptr)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			a.mov(ptr(ecx, regoffsl), Gpd(int_register_map[regnum]));
		else
		{
			a.mov(eax, MABS(&m_state.r[regnum].w.l));
			a.mov(ptr(ecx, regoffsl), eax);
		}
		a.mov(eax, MABS(&m_state.r[regnum].w.h));
		a.mov(ptr(ecx, regoffsh), eax);
	}
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		uintptr_t regoffsl = (uintptr_t)&((drcuml_machine_state *)nullptr)->f[regnum].s.l;
		uintptr_t regoffsh = (uintptr_t)&((drcuml_machine_state *)nullptr)->f[regnum].s.h;
		a.mov(eax, MABS(&m_state.f[regnum].s.l));
		a.mov(ptr(ecx, regoffsl), eax);
		a.mov(eax, MABS(&m_state.f[regnum].s.h));
		a.mov(ptr(ecx, regoffsh), eax);
	}
	a.ret();                                                                            // ret

	// generate a restore subroutine
	m_restore = dst + a.offset();
	a.bind(a.newNamedLabel("restore"));
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.r); regnum++)
	{
		uintptr_t regoffsl = (uintptr_t)&((drcuml_machine_state *)nullptr)->r[regnum].w.l;
		uintptr_t regoffsh = (uintptr_t)&((drcuml_machine_state *)nullptr)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			a.mov(Gpd(int_register_map[regnum]), ptr(ecx, regoffsl));
		else
		{
			a.mov(eax, ptr(ecx, regoffsl));
			a.mov(MABS(&m_state.r[regnum].w.l), eax);
		}
		a.mov(eax, ptr(ecx, regoffsh));
		a.mov(MABS(&m_state.r[regnum].w.h), eax);
	}
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_state.f); regnum++)
	{
		uintptr_t regoffsl = (uintptr_t)&((drcuml_machine_state *)nullptr)->f[regnum].s.l;
		uintptr_t regoffsh = (uintptr_t)&((drcuml_machine_state *)nullptr)->f[regnum].s.h;
		a.mov(eax, ptr(ecx, regoffsl));
		a.mov(MABS(&m_state.f[regnum].s.l), eax);
		a.mov(eax, ptr(ecx, regoffsh));
		a.mov(MABS(&m_state.f[regnum].s.h), eax);
	}
	a.movzx(eax, byte_ptr(ecx, offsetof(drcuml_machine_state, fmod)));                  // movzx eax,state->fmod
	a.and_(eax, 3);                                                                     // and    eax,3
	a.mov(MABS(&m_state.fmod), al);                                                     // mov    [fmod],al
	a.fldcw(word_ptr(u64(&fp_control[0]), eax, 1));                                     // fldcw  fp_control[eax*2]
	a.mov(eax, ptr(ecx, offsetof(drcuml_machine_state, exp)));                          // mov    eax,state->exp
	a.mov(MABS(&m_state.exp), eax);                                                     // mov    [exp],eax
	a.movzx(eax, byte_ptr(ecx, offsetof(drcuml_machine_state, flags)));                 // movzx eax,state->flags
	a.push(dword_ptr(u64(flags_unmap), eax, 2));                                        // push   flags_unmap[eax*4]
	a.popfd();                                                                          // popf
	a.ret();                                                                            // ret


	// emit the generated code
	size_t bytes = emit(ch);

	if (m_log != nullptr && !m_logged_common)
	{
		x86log_disasm_code_range(m_log, "entry_point", dst, m_exit);
		x86log_disasm_code_range(m_log, "exit_point", m_exit, m_nocode);
		x86log_disasm_code_range(m_log, "nocode_point", m_nocode, m_save);
		x86log_disasm_code_range(m_log, "save", m_save, m_restore);
		x86log_disasm_code_range(m_log, "restore", m_restore, dst + bytes);

		m_logged_common = true;
	}

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
	m_cache.codegen_complete();
	return (*m_entry)((x86code *)entry.codeptr());
}


//-------------------------------------------------
//  drcbex86_generate - generate code
//-------------------------------------------------

void drcbe_x86::generate(drcuml_block &block, const instruction *instlist, uint32_t numinst)
{
	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_map.block_begin(block);

	// compute the base by aligning the cache top to a cache line (assumed to be 64 bytes)
	x86code *dst = (x86code *)(uint64_t(m_cache.top() + 63) & ~63);

	CodeHolder ch;
	ch.init(hostEnvironment(), uint64_t(dst));
	ThrowableErrorHandler e;
	ch.setErrorHandler(&e);

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.setFlags(FormatOptions::Flags::kFlagHexOffsets | FormatOptions::Flags::kFlagHexImms | FormatOptions::Flags::kFlagMachineCode);
		logger.setIndentation(FormatOptions::IndentationType::kIndentationCode, 4);
		ch.setLogger(&logger);
	}

	Assembler a(&ch);
	if (logger.file())
		a.addValidationOptions(BaseEmitter::kValidationOptionIntermediate);

	// generate code
	std::string blockname;
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		assert(inst.opcode() < ARRAY_LENGTH(s_opcode_table));

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
//  drcbex86_hash_exists - return true if the
//  given mode/pc exists in the hash table
//-------------------------------------------------

bool drcbe_x86::hash_exists(uint32_t mode, uint32_t pc)
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

void drcbe_x86::emit_mov_r32_p32(Assembler &a, Gp const &reg, be_parameter const &param)
{
	if (param.is_immediate())
	{
		if (param.immediate() == 0)
			a.xor_(reg, reg);                                                           // xor   reg,reg
		else
			a.mov(reg, param.immediate());                                              // mov   reg,param
	}
	else if (param.is_memory())
		a.mov(reg, MABS(param.memory()));                                               // mov   reg,[param]
	else if (param.is_int_register())
	{
		if (reg.id() != param.ireg())
			a.mov(reg, Gpd(param.ireg()));                                              // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_r32_p32_keepflags - move a 32-bit
//  parameter into a register without affecting
//  any flags
//-------------------------------------------------

void drcbe_x86::emit_mov_r32_p32_keepflags(Assembler &a, Gp const &reg, be_parameter const &param)
{
	if (param.is_immediate())
		a.mov(reg, param.immediate());                                                  // mov   reg,param
	else if (param.is_memory())
	{
		if (!can_skip_lower_load(a, (uint32_t *)((uintptr_t)param.memory()), reg))
			a.mov(reg, MABS(param.memory()));                                           // mov   reg,[param]
	}
	else if (param.is_int_register())
	{
		if (reg.id() != param.ireg())
			a.mov(reg, Gpd(param.ireg()));                                              // mov   reg,param
	}
}


//-------------------------------------------------
//  emit_mov_m32_p32 - move a 32-bit parameter
//  into a memory location
//-------------------------------------------------

void drcbe_x86::emit_mov_m32_p32(Assembler &a, Mem memref, be_parameter const &param)
{
	if (param.is_immediate())
		a.mov(memref, param.immediate());                                               // mov   [mem],param
	else if (param.is_memory())
	{
		if (!can_skip_lower_load(a, (uint32_t *)((uintptr_t)param.memory()), eax))
			a.mov(eax, MABS(param.memory()));                                           // mov   eax,[param]
		a.mov(memref, eax);                                                             // mov   [mem],eax
	}
	else if (param.is_int_register())
		a.mov(memref, Gpd(param.ireg()));                                               // mov   [mem],param
}


//-------------------------------------------------
//  emit_mov_p32_r32 - move a register into a
//  32-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_mov_p32_r32(Assembler &a, be_parameter const &param, Gp const &reg)
{
	assert(!param.is_immediate());
	if (param.is_memory())
	{
		a.mov(MABS(param.memory()), reg);                                               // mov   [param],reg
		set_last_lower_reg(a, param, reg);
	}
	else if (param.is_int_register())
	{
		if (reg.id() != param.ireg())
			a.mov(Gpd(param.ireg()), reg);                                              // mov   param,reg
	}
}


void drcbe_x86::alu_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param, std::function<bool(Assembler &a, Operand const &dst, be_parameter const &src)> optimize)
{
	if (param.is_immediate())
	{
		if (!optimize(a, dst, param))
			a.emit(opcode, dst, param.immediate());                                     // op    dst,param
	}
	else if (param.is_memory())
	{
		if (dst.isMem())
		{
			// use temporary register for memory,memory
			Gp const reg = param.select_register(eax);

			a.mov(reg, MABS(param.memory()));                                           // mov   reg,param
			a.emit(opcode, dst, reg);                                                   // op    [dst],reg
		}
		else if (opcode != Inst::kIdTest)
			// most instructions are register,memory
			a.emit(opcode, dst, MABS(param.memory()));                                  // op    dst,[param]
		else
			// test instruction requires memory,register
			a.emit(opcode, MABS(param.memory()), dst);                                  // op    [param],dst
	}
	else if (param.is_int_register())
		a.emit(opcode, dst, Gpd(param.ireg()));                                         // op    dst,param
}


void drcbe_x86::shift_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param, std::function<bool(Assembler &a, Operand const &dst, be_parameter const &src)> optimize)
{
	Operand shift = cl;
	if (param.is_immediate())
	{
		if (optimize(a, dst, param))
			return;

		shift = imm(param.immediate());
	}
	else
		emit_mov_r32_p32(a, ecx, param);

	a.emit(opcode, dst, shift);
}



//**************************************************************************
//  EMITTERS FOR 64-BIT OPERATIONS WITH PARAMETERS
//**************************************************************************

//-------------------------------------------------
//  emit_mov_r64_p64 - move a 64-bit parameter
//  into a pair of registers
//-------------------------------------------------

void drcbe_x86::emit_mov_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param)
{
	if (param.is_immediate())
	{
		if (!reglo.isValid())
			;
		else if (u32(param.immediate()) == 0)
			a.xor_(reglo, reglo);                                                       // xor   reglo,reglo
		else
			a.mov(reglo, param.immediate());                                            // mov   reglo,param
		if (!reghi.isValid())
			;
		else if (u32(param.immediate() >> 32) == 0)
			a.xor_(reghi, reghi);                                                       // xor   reghi,reghi
		else
			a.mov(reghi, param.immediate() >> 32);                                      // mov   reghi,param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(a, (uint32_t *)((uintptr_t)param.memory(0)), reglo);
		int skip_upper = can_skip_upper_load(a, (uint32_t *)((uintptr_t)param.memory(4)), reghi);
		if (reglo.isValid() && !skip_lower)
			a.mov(reglo, MABS(param.memory(0)));                                        // mov   reglo,[param]
		if (reghi.isValid() && !skip_upper)
			a.mov(reghi, MABS(param.memory(4)));                                        // mov   reghi,[param+4]
	}
	else if (param.is_int_register())
	{
		int skip_upper = can_skip_upper_load(a, m_reghi[param.ireg()], reghi);
		if (reglo.isValid() && reglo.id() != param.ireg())
			a.mov(reglo, Gpd(param.ireg()));                                            // mov   reglo,param
		if (reghi.isValid() && !skip_upper)
			a.mov(reghi, MABS(m_reghi[param.ireg()]));                                  // mov   reghi,reghi[param]
	}
}


//-------------------------------------------------
//  emit_mov_r64_p64_keepflags - move a 64-bit
//  parameter into a pair of registers without
//  affecting any flags
//-------------------------------------------------

void drcbe_x86::emit_mov_r64_p64_keepflags(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param)
{
	if (param.is_immediate())
	{
		if (reglo.isValid())
			a.mov(reglo, param.immediate());                                            // mov   reglo,param
		if (reghi.isValid())
			a.mov(reghi, param.immediate() >> 32);                                      // mov   reghi,param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(a, (uint32_t *)((uintptr_t)param.memory(0)), reglo);
		int skip_upper = can_skip_upper_load(a, (uint32_t *)((uintptr_t)param.memory(4)), reghi);
		if (reglo.isValid() && !skip_lower)
			a.mov(reglo, MABS(param.memory(0)));                                        // mov   reglo,[param]
		if (reghi.isValid() && !skip_upper)
			a.mov(reghi, MABS(param.memory(4)));                                        // mov   reghi,[param+4]
	}
	else if (param.is_int_register())
	{
		int skip_upper = can_skip_upper_load(a, m_reghi[param.ireg()], reghi);
		if (reglo.isValid() && reglo.id() != param.ireg())
			a.mov(reglo, Gpd(param.ireg()));                                            // mov   reglo,param
		if (reghi.isValid() && !skip_upper)
			a.mov(reghi, MABS(m_reghi[param.ireg()]));                                  // mov   reghi,reghi[param]
	}
}


//-------------------------------------------------
//  emit_mov_m64_p64 - move a 64-bit parameter
//  into a memory location
//-------------------------------------------------

void drcbe_x86::emit_mov_m64_p64(Assembler &a, Mem const &memref, be_parameter const &param)
{
	Mem memref_lo = memref.cloneAdjusted(0); memref_lo.setSize(4);
	Mem memref_hi = memref.cloneAdjusted(4); memref_hi.setSize(4);

	if (param.is_immediate())
	{
		a.mov(memref_lo, param.immediate());                                            // mov   [mem],param
		a.mov(memref_hi, param.immediate() >> 32);                                      // mov   [mem],param >> 32
	}
	else if (param.is_memory())
	{
		int skip_lower = can_skip_lower_load(a, (uint32_t *)((uintptr_t)param.memory()), eax);
		if (!skip_lower)
			a.mov(eax, MABS(param.memory(0)));                                          // mov   eax,[param]
		a.mov(memref_lo, eax);                                                          // mov   [mem],eax
		a.mov(eax, MABS(param.memory(4)));                                              // mov   eax,[param+4]
		a.mov(memref_hi, eax);                                                          // mov   [mem+4],eax
	}
	else if (param.is_int_register())
	{
		a.mov(memref_lo, Gpd(param.ireg()));                                            // mov   [mem],param
		a.mov(eax, MABS(m_reghi[param.ireg()]));                                        // mov   eax,[param.hi]
		a.mov(memref_hi, eax);                                                          // mov   [mem+4],eax
	}
}


//-------------------------------------------------
//  emit_mov_p64_r64 - move a pair of registers
//  into a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_mov_p64_r64(Assembler &a, be_parameter const &param, Gp const &reglo, Gp const &reghi)
{
	assert(!param.is_immediate());
	if (param.is_memory())
	{
		a.mov(MABS(param.memory(0)), reglo);                                            // mov   [param],reglo
		a.mov(MABS(param.memory(4)), reghi);                                            // mov   [param+4],reghi
	}
	else if (param.is_int_register())
	{
		if (reglo.id() != param.ireg())
			a.mov(Gpd(param.ireg()), reglo);                                            // mov   param,reglo
		a.mov(MABS(m_reghi[param.ireg()]), reghi);                                      // mov   reghi[param],reghi
	}
	set_last_lower_reg(a, param, reglo);
	set_last_upper_reg(a, param, reghi);
}


//-------------------------------------------------
//  emit_and_r64_p64 - and operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		a.and_(reglo, MABS(param.memory(0)));                                           // and   reglo,[param]
		if (saveflags) a.pushfd();                                                      // pushf
		a.and_(reghi, MABS(param.memory(4)));                                           // and   reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0)
			a.xor_(reglo, reglo);                                                       // xor   reglo,reglo
		else
			a.and_(reglo, param.immediate());                                           // and   reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			a.xor_(reghi, reghi);                                                       // xor   reghi,reghi
		else
			a.and_(reghi, param.immediate() >> 32);                                     // and   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		a.and_(reglo, Gpd(param.ireg()));                                               // and   reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		a.and_(reghi, MABS(m_reghi[param.ireg()]));                                     // and   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_and_m64_p64 - and operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_and_m64_p64(Assembler &a, Mem const &memref_lo, Mem const &memref_hi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0)
			a.mov(memref_lo, 0);                                                        // mov   [dest],0
		else
			a.and_(memref_lo, param.immediate());                                       // and   [dest],param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			a.mov(memref_hi, 0);                                                        // mov   [dest+4],0
		else
			a.and_(memref_hi, param.immediate() >> 32);                                 // and   [dest+4],param >> 32
	}
	else
	{
		Gp const reglo = (param.is_int_register()) ? Gpd(param.ireg()) : eax;
		emit_mov_r64_p64(a, reglo, edx, param);                                         // mov   edx:reglo,param
		a.and_(memref_lo, reglo);                                                       // and   [dest],reglo
		if (saveflags) a.pushfd();                                                      // pushf
		a.and_(memref_hi, edx);                                                         // and   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_or_r64_p64 - or operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		a.or_(reglo, MABS(param.memory(0)));                                            // or    reglo,[param]
		if (saveflags) a.pushfd();                                                      // pushf
		a.or_(reghi, MABS(param.memory(4)));                                            // or    reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			a.mov(reglo, ~0);                                                           // mov   reglo,-1
		else
			a.or_(reglo, param.immediate());                                            // or    reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			a.mov(reghi, ~0);                                                           // mov   reghi,-1
		else
			a.or_(reghi, param.immediate() >> 32);                                      // or    reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		a.or_(reglo, Gpd(param.ireg()));                                                // or    reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		a.or_(reghi, MABS(m_reghi[param.ireg()]));                                      // or    reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_or_m64_p64 - or operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_or_m64_p64(Assembler &a, Mem const &memref_lo, Mem const &memref_hi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			a.mov(memref_lo, ~0);                                                       // mov   [dest],-1
		else
			a.or_(memref_lo, param.immediate());                                        // or    [dest],param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			a.mov(memref_hi, ~0);                                                       // mov   [dest+4],-1
		else
			a.or_(memref_hi, param.immediate() >> 32);                                  // or    [dest+4],param >> 32
	}
	else
	{
		Gp const reglo = (param.is_int_register()) ? Gpd(param.ireg()) : eax;
		emit_mov_r64_p64(a, reglo, edx, param);                                         // mov   edx:reglo,param
		a.or_(memref_lo, reglo);                                                        // or    [dest],reglo
		if (saveflags) a.pushfd();                                                      // pushf
		a.or_(memref_hi, edx);                                                          // or    [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_xor_r64_p64 - xor operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_memory())
	{
		a.xor_(reglo, MABS(param.memory(0)));                                           // xor   reglo,[param]
		if (saveflags) a.pushfd();                                                      // pushf
		a.xor_(reghi, MABS(param.memory(4)));                                           // xor   reghi,[param]
	}
	else if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			a.not_(reglo);                                                              // not   reglo
		else
			a.xor_(reglo, param.immediate());                                           // xor   reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			a.not_(reghi);                                                              // not   reghi
		else
			a.xor_(reghi, param.immediate() >> 32);                                     // xor   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		a.xor_(reglo, Gpd(param.ireg()));                                               // xor   reglo,param
		if (saveflags) a.pushfd();                                                      // pushf
		a.xor_(reghi, MABS(m_reghi[param.ireg()]));                                     // xor   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_xor_m64_p64 - xor operation to a 64-bit
//  memory location from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_xor_m64_p64(Assembler &a, Mem const &memref_lo, Mem const &memref_hi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		if (!inst.flags() && u32(param.immediate()) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate()) == 0xffffffffU)
			a.not_(memref_lo);                                                          // not   [dest]
		else
			a.xor_(memref_lo, param.immediate());                                       // xor   [dest],param
		if (saveflags) a.pushfd();                                                      // pushf
		if (!inst.flags() && u32(param.immediate() >> 32) == 0)
			;// skip
		else if (!inst.flags() && u32(param.immediate() >> 32) == 0xffffffffU)
			a.not_(memref_hi);                                                          // not   [dest+4]
		else
			a.xor_(memref_hi, param.immediate() >> 32);                                 // xor   [dest+4],param >> 32
	}
	else
	{
		Gp const reglo = (param.is_int_register()) ? Gpd(param.ireg()) : eax;
		emit_mov_r64_p64(a, reglo, edx, param);                                         // mov   edx:reglo,param
		a.xor_(memref_lo, reglo);                                                       // xor   [dest],reglo
		if (saveflags) a.pushfd();                                                      // pushf
		a.xor_(memref_hi, edx);                                                         // xor   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_shl_r64_p64 - shl operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shl_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = (inst.flags() != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (!inst.flags() && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					a.shld(reghi, reglo, 31);                                           // shld  reghi,reglo,31
					a.shl(reglo, 31);                                                   // shl   reglo,31
					count -= 31;
				}
				else
				{
					a.mov(reghi, reglo);                                                // mov   reghi,reglo
					a.xor_(reglo, reglo);                                               // xor   reglo,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				a.shld(reghi, reglo, count);                                            // shld  reghi,reglo,count
				if (saveflags) a.pushfd();                                              // pushf
				a.shl(reglo, count);                                                    // shl   reglo,count
			}
		}
	}
	else
	{
		Label skip1 = a.newLabel();
		Label skip2 = a.newLabel();
		emit_mov_r32_p32(a, ecx, param);                                                // mov   ecx,param
		a.test(ecx, 0x20);                                                              // test  ecx,0x20
		a.short_().jz(skip1);                                                           // jz    skip1
		if (inst.flags() != 0)
		{
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shld(reghi, reglo, 31);                                                   // shld  reghi,reglo,31
			a.shl(reglo, 31);                                                           // shl   reglo,31
			a.test(ecx, 0x20);                                                          // test  ecx,0x20
			a.short_().jz(skip2);                                                       // jz    skip2
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shld(reghi, reglo, 31);                                                   // shld  reghi,reglo,31
			a.shl(reglo, 31);                                                           // shl   reglo,31
			a.bind(skip2);                                                          // skip2:
			reset_last_upper_lower_reg();
		}
		else
		{
			a.mov(reghi, reglo);                                                        // mov   reghi,reglo
			a.xor_(reglo, reglo);                                                       // xor   reglo,reglo
		}
		a.bind(skip1);                                                              // skip1:
		reset_last_upper_lower_reg();
		a.shld(reghi, reglo, cl);                                                       // shld  reghi,reglo,cl
		if (saveflags) a.pushfd();                                                      // pushf
		a.shl(reglo, cl);                                                               // shl   reglo,cl
	}
	if (saveflags)
		emit_combine_z_shl_flags(a);
}


//-------------------------------------------------
//  emit_shr_r64_p64 - shr operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_shr_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (!inst.flags() && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					a.shrd(reglo, reghi, 31);                                           // shrd  reglo,reghi,31
					a.shr(reghi, 31);                                                   // shr   reghi,31
					count -= 31;
				}
				else
				{
					a.mov(reglo, reghi);                                                // mov   reglo,reghi
					a.xor_(reghi, reghi);                                               // xor   reghi,reghi
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				a.shrd(reglo, reghi, count);                                            // shrd  reglo,reghi,count
				if (saveflags) a.pushfd();                                              // pushf
				a.shr(reghi, count);                                                    // shr   reghi,count
			}
		}
	}
	else
	{
		Label skip1 = a.newLabel();
		Label skip2 = a.newLabel();
		emit_mov_r32_p32(a, ecx, param);                                                // mov   ecx,param
		a.test(ecx, 0x20);                                                              // test  ecx,0x20
		a.short_().jz(skip1);                                                           // jz    skip1
		if (inst.flags() != 0)
		{
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.shr(reghi, 31);                                                           // shr   reghi,31
			a.test(ecx, 0x20);                                                          // test  ecx,0x20
			a.short_().jz(skip2);                                                       // jz    skip2
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.shr(reghi, 31);                                                           // shr   reghi,31
			a.bind(skip2);                                                          // skip2:
			reset_last_upper_lower_reg();
		}
		else
		{
			a.mov(reglo, reghi);                                                        // mov   reglo,reghi
			a.xor_(reghi, reghi);                                                       // xor   reghi,reghi
		}
		a.bind(skip1);                                                              // skip1:
		reset_last_upper_lower_reg();
		a.shrd(reglo, reghi, cl);                                                       // shrd  reglo,reghi,cl
		if (saveflags) a.pushfd();                                                      // pushf
		a.shr(reghi, cl);                                                               // shr   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_sar_r64_p64 - sar operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_sar_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (!inst.flags() && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					a.shrd(reglo, reghi, 31);                                           // shrd  reglo,reghi,31
					a.sar(reghi, 31);                                                   // sar   reghi,31
					count -= 31;
				}
				else
				{
					a.mov(reglo, reghi);                                                // mov   reglo,reghi
					a.sar(reghi, 31);                                                   // sar   reghi,31
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				a.shrd(reglo, reghi, count);                                            // shrd  reglo,reghi,count
				if (saveflags) a.pushfd();                                              // pushf
				a.sar(reghi, count);                                                    // sar   reghi,count
			}
		}
	}
	else
	{
		Label skip1 = a.newLabel();
		Label skip2 = a.newLabel();
		emit_mov_r32_p32(a, ecx, param);                                                // mov   ecx,param
		a.test(ecx, 0x20);                                                              // test  ecx,0x20
		a.short_().jz(skip1);                                                           // jz    skip1
		if (inst.flags() != 0)
		{
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.sar(reghi, 31);                                                           // sar   reghi,31
			a.test(ecx, 0x20);                                                          // test  ecx,0x20
			a.short_().jz(skip2);                                                       // jz    skip
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.sar(reghi, 31);                                                           // sar   reghi,31
			a.bind(skip2);                                                          // skip2:
			reset_last_upper_lower_reg();
		}
		else
		{
			a.mov(reglo, reghi);                                                        // mov   reglo,reghi
			a.sar(reghi, 31);                                                           // sar   reghi,31
		}
		a.bind(skip1);                                                              // skip1:
		reset_last_upper_lower_reg();
		a.shrd(reglo, reghi, cl);                                                       // shrd  reglo,reghi,cl
		if (saveflags) a.pushfd();                                                      // pushf
		a.sar(reghi, cl);                                                               // sar   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_rol_r64_p64 - rol operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rol_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (!inst.flags() && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					a.mov(ecx, reglo);                                                  // mov   ecx,reglo
					a.shld(reglo, reghi, 31);                                           // shld  reglo,reghi,31
					a.shld(reghi, ecx, 31);                                             // shld  reghi,ecx,31
					count -= 31;
				}
				else
				{
					a.xchg(reghi, reglo);                                               // xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				a.mov(ecx, reglo);                                                      // mov   ecx,reglo
				a.shld(reglo, reghi, count);                                            // shld  reglo,reghi,count
				if (saveflags) a.pushfd();                                              // pushf
				a.shld(reghi, ecx, count);                                              // shld  reghi,ecx,count
			}
		}
	}
	else
	{
		Label skip1 = a.newLabel();
		Label skip2 = a.newLabel();
		a.mov(ptr(esp, -8), ebx);                                                       // mov   [esp-8],ebx
		emit_mov_r32_p32(a, ecx, param);                                                // mov   ecx,param
		a.test(ecx, 0x20);                                                              // test  ecx,0x20
		a.short_().jz(skip1);                                                           // jz    skip1
		if (inst.flags() != 0)
		{
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.mov(ebx, reglo);                                                          // mov   ebx,reglo
			a.shld(reglo, reghi, 31);                                                   // shld  reglo,reghi,31
			a.shld(reghi, ebx, 31);                                                     // shld  reghi,ebx,31
			a.test(ecx, 0x20);                                                          // test  ecx,0x20
			a.short_().jz(skip2);                                                       // jz    skip2
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.mov(ebx, reglo);                                                          // mov   ebx,reglo
			a.shld(reglo, reghi, 31);                                                   // shld  reglo,reghi,31
			a.shld(reghi, ebx, 31);                                                     // shld  reghi,ebx,31
			a.bind(skip2);                                                          // skip2:
			reset_last_upper_lower_reg();
		}
		else
			a.xchg(reghi, reglo);                                                       // xchg  reghi,reglo
		a.bind(skip1);                                                              // skip1:
		reset_last_upper_lower_reg();
		a.mov(ebx, reglo);                                                              // mov   ebx,reglo
		a.shld(reglo, reghi, cl);                                                       // shld  reglo,reghi,cl
		if (saveflags) a.pushfd();                                                      // pushf
		a.shld(reghi, ebx, cl);                                                         // shld  reghi,ebx,cl
		a.mov(ebx, ptr(esp, saveflags ? -4 : -8));                                      // mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_ror_r64_p64 - ror operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_ror_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	if (param.is_immediate())
	{
		int count = param.immediate() & 63;
		if (!inst.flags() && count == 0)
			;// skip
		else
		{
			while (count >= 32)
			{
				if (inst.flags() != 0)
				{
					a.mov(ecx, reglo);                                                  // mov   ecx,reglo
					a.shrd(reglo, reghi, 31);                                           // shrd  reglo,reghi,31
					a.shrd(reghi, ecx, 31);                                             // shrd  reghi,ecx,31
					count -= 31;
				}
				else
				{
					a.xchg(reghi, reglo);                                               // xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst.flags() != 0 || count > 0)
			{
				a.mov(ecx, reglo);                                                      // mov   ecx,reglo
				a.shrd(reglo, reghi, count);                                            // shrd  reglo,reghi,count
				if (saveflags) a.pushfd();                                              // pushf
				a.shrd(reghi, ecx, count);                                              // shrd  reghi,ecx,count
			}
		}
	}
	else
	{
		Label skip1 = a.newLabel();
		Label skip2 = a.newLabel();
		a.mov(ptr(esp, -8), ebx);                                                       // mov   [esp-8],ebx
		emit_mov_r32_p32(a, ecx, param);                                                // mov   ecx,param
		a.test(ecx, 0x20);                                                              // test  ecx,0x20
		a.short_().jz(skip1);                                                           // jz    skip1
		if (inst.flags() != 0)
		{
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.mov(ebx, reglo);                                                          // mov   ebx,reglo
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.shrd(reghi, ebx, 31);                                                     // shrd  reghi,ebx,31
			a.test(ecx, 0x20);                                                          // test  ecx,0x20
			a.short_().jz(skip2);                                                       // jz    skip2
			a.sub(ecx, 31);                                                             // sub   ecx,31
			a.mov(ebx, reglo);                                                          // mov   ebx,reglo
			a.shrd(reglo, reghi, 31);                                                   // shrd  reglo,reghi,31
			a.shrd(reghi, ebx, 31);                                                     // shrd  reghi,ebx,31
			a.bind(skip2);                                                          // skip2:
			reset_last_upper_lower_reg();
		}
		else
			a.xchg(reghi, reglo);                                                       // xchg  reghi,reglo
		a.bind(skip1);                                                              // skip1:
		reset_last_upper_lower_reg();
		a.mov(ebx, reglo);                                                              // mov   ebx,reglo
		a.shrd(reglo, reghi, cl);                                                       // shrd  reglo,reghi,cl
		if (saveflags) a.pushfd();                                                      // pushf
		a.shrd(reghi, ebx, cl);                                                         // shrd  reghi,ebx,cl
		a.mov(ebx, ptr(esp, saveflags ? -4 : -8));                                      // mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(a);
}


//-------------------------------------------------
//  emit_rcl_r64_p64 - rcl operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcl_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = ((inst.flags() & FLAG_Z) != 0);
	Label loop = a.newLabel();
	Label skipall = a.newLabel();
	Label skiploop = a.newLabel();

	emit_mov_r32_p32_keepflags(a, ecx, param);                                          // mov   ecx,param
	if (!saveflags)
	{
		a.bind(loop);                                                               // loop:
		a.jecxz(skipall);                                                               // jecxz skipall
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.rcl(reglo, 1);                                                                // rcl   reglo,1
		a.rcl(reghi, 1);                                                                // rcl   reghi,1
		a.jmp(loop);                                                                    // jmp   loop
		a.bind(skipall);                                                            // skipall:
		reset_last_upper_lower_reg();
	}
	else
	{
		a.jecxz(skipall);                                                               // jecxz skipall
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.bind(loop);                                                               // loop:
		a.jecxz(skiploop);                                                              // jecxz skiploop
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.rcl(reglo, 1);                                                                // rcl   reglo,1
		a.rcl(reghi, 1);                                                                // rcl   reghi,1
		a.jmp(loop);                                                                    // jmp   loop
		a.bind(skiploop);                                                           // skiploop:
		reset_last_upper_lower_reg();
		a.rcl(reglo, 1);                                                                // rcl   reglo,1
		a.pushfd();                                                                     // pushf
		a.rcl(reghi, 1);                                                                // rcl   reghi,1
		a.bind(skipall);                                                            // skipall:
		reset_last_upper_lower_reg();
		emit_combine_z_flags(a);
	}
}


//-------------------------------------------------
//  emit_rcr_r64_p64 - rcr operation to a 64-bit
//  pair of registers from a 64-bit parameter
//-------------------------------------------------

void drcbe_x86::emit_rcr_r64_p64(Assembler &a, Gp const &reglo, Gp const &reghi, be_parameter const &param, const instruction &inst)
{
	int saveflags = (inst.flags() != 0);
	Label loop = a.newLabel();
	Label skipall = a.newLabel();
	Label skiploop = a.newLabel();

	emit_mov_r32_p32_keepflags(a, ecx, param);                                          // mov   ecx,param
	if (!saveflags)
	{
		a.bind(loop);                                                               // loop:
		a.jecxz(skipall);                                                               // jecxz skipall
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.rcr(reghi, 1);                                                                // rcr   reghi,1
		a.rcr(reglo, 1);                                                                // rcr   reglo,1
		a.jmp(loop);                                                                    // jmp   loop
		a.bind(skipall);                                                            // skipall:
		reset_last_upper_lower_reg();
	}
	else
	{
		a.jecxz(skipall);                                                               // jecxz skipall
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.bind(loop);                                                               // loop:
		a.jecxz(skiploop);                                                              // jecxz skiploop
		a.lea(ecx, ptr(ecx, -1));                                                       // lea   ecx,[ecx-1]
		a.rcr(reghi, 1);                                                                // rcr   reghi,1
		a.rcr(reglo, 1);                                                                // rcr   reglo,1
		a.jmp(loop);                                                                    // jmp   loop
		a.bind(skiploop);                                                           // skiploop:
		reset_last_upper_lower_reg();
		a.rcr(reghi, 1);                                                                // rcr   reghi,1
		a.pushfd();                                                                     // pushf
		a.rcr(reglo, 1);                                                                // rcr   reglo,1
		a.bind(skipall);                                                            // skipall:
		reset_last_upper_lower_reg();
		emit_combine_z_shl_flags(a);
	}
}


void drcbe_x86::alu_op_param(Assembler &a, Inst::Id const opcode_lo, Inst::Id const opcode_hi, Gp const &lo, Gp const &hi, be_parameter const &param, bool saveflags)
{
	if (param.is_memory())
	{
		a.emit(opcode_lo, lo, MABS(param.memory(0)));                               // opl   reglo,[param]
		if (saveflags) a.pushfd();                                                  // pushf
		a.emit(opcode_hi, hi, MABS(param.memory(4)));                               // oph   reghi,[param]
	}
	else if (param.is_immediate())
	{
		a.emit(opcode_lo, lo, param.immediate());                                   // opl   reglo,param
		if (saveflags) a.pushfd();                                                  // pushf
		a.emit(opcode_hi, hi, param.immediate() >> 32);                             // oph   reghi,param >> 32
	}
	else if (param.is_int_register())
	{
		a.emit(opcode_lo, lo, Gpd(param.ireg()));                                   // opl   reglo,param
		if (saveflags) a.pushfd();                                                  // pushf
		a.emit(opcode_hi, hi, MABS(m_reghi[param.ireg()]));                         // oph   reghi,reghi[param]
	}

	if (saveflags)
		emit_combine_z_flags(a);
}


void drcbe_x86::alu_op_param(Assembler &a, Inst::Id const opcode_lo, Inst::Id const opcode_hi, Mem const &lo, Mem const &hi, be_parameter const &param, bool saveflags)
{
	if (param.is_immediate())
	{
		a.emit(opcode_lo, lo, param.immediate());                                   // opl   [dest],param
		if (saveflags) a.pushfd();                                                  // pushf
		a.emit(opcode_hi, hi, param.immediate() >> 32);                             // oph   [dest+4],param >> 32
	}
	else
	{
		Gp const reg = (param.is_int_register()) ? Gpd(param.ireg()) : eax;

		emit_mov_r64_p64(a, reg, edx, param);                                       // mov   edx:reglo,param
		a.emit(opcode_lo, lo, reg);                                                 // opl   [dest],reglo
		if (saveflags) a.pushfd();                                                  // pushf
		a.emit(opcode_hi, hi, edx);                                                 // oph   [dest+4],edx
	}

	if (saveflags)
		emit_combine_z_flags(a);
}


//**************************************************************************
//  EMITTERS FOR FLOATING POINT
//**************************************************************************

//-------------------------------------------------
//  emit_fld_p - load a floating point parameter
//  onto the stack
//-------------------------------------------------

void drcbe_x86::emit_fld_p(Assembler &a, int size, be_parameter const &param)
{
	assert(param.is_memory());
	assert(size == 4 || size == 8);
	a.fld(ptr(u64(param.memory()), size));
}


//-------------------------------------------------
//  emit_fstp_p - store a floating point parameter
//  from the stack and pop it
//-------------------------------------------------

void drcbe_x86::emit_fstp_p(Assembler &a, int size, be_parameter const &param)
{
	assert(param.is_memory());
	assert(size == 4 || size == 8);

	a.fstp(ptr(u64(param.memory()), size));
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

void drcbe_x86::op_handle(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_handle());

	reset_last_upper_lower_reg();

	// make a label for documentation
	Label handle = a.newNamedLabel(inst.param(0).handle().string());
	a.bind(handle);

	// emit a jump around the stack adjust in case code falls through here
	Label skip = a.newLabel();
	a.short_().jmp(skip);                                                               // jmp   skip

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(drccodeptr(a.code()->baseAddress() + a.offset()));

	// by default, the handle points to prolog code that moves the stack pointer
	a.lea(esp, ptr(esp, -28));                                                          // lea   rsp,[rsp-28]
	a.bind(skip);                                                                   // skip:
	reset_last_upper_lower_reg();
}


//-------------------------------------------------
//  op_hash - process a HASH opcode
//-------------------------------------------------

void drcbe_x86::op_hash(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	// register the current pointer for the mode/PC
	m_hash.set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), drccodeptr(a.code()->baseAddress() + a.offset()));
	reset_last_upper_lower_reg();
}


//-------------------------------------------------
//  op_label - process a LABEL opcode
//-------------------------------------------------

void drcbe_x86::op_label(Assembler &a, const instruction &inst)
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

	reset_last_upper_lower_reg();
}


//-------------------------------------------------
//  op_comment - process a COMMENT opcode
//-------------------------------------------------

void drcbe_x86::op_comment(Assembler &a, const instruction &inst)
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

void drcbe_x86::op_mapvar(Assembler &a, const instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_mapvar());
	assert(inst.param(1).is_immediate());

	// set the value of the specified mapvar
	m_map.set_value(drccodeptr(a.code()->baseAddress() + a.offset()), inst.param(0).mapvar(), inst.param(1).immediate());
}



//**************************************************************************
//  CONTROL FLOW OPCODES
//**************************************************************************

//-------------------------------------------------
//  op_nop - process a NOP opcode
//-------------------------------------------------

void drcbe_x86::op_nop(Assembler &a, const instruction &inst)
{
	// nothing
}


//-------------------------------------------------
//  op_debug - process a DEBUG opcode
//-------------------------------------------------

void drcbe_x86::op_debug(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	using debugger_hook_func = void (*)(device_debug *, offs_t);
	static const debugger_hook_func debugger_inst_hook = [] (device_debug *dbg, offs_t pc) { dbg->instruction_hook(pc); }; // TODO: kill trampoline if possible

	if ((m_device.machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		// normalize parameters
		be_parameter const pcp(*this, inst.param(0), PTYPE_MRI);

		// test and branch
		a.test(MABS(&m_device.machine().debug_flags, 4), DEBUG_FLAG_CALL_HOOK);         // test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		Label skip = a.newLabel();
		a.short_().jz(skip);                                                            // jz    skip

		// push the parameter
		emit_mov_m32_p32(a, dword_ptr(esp, 4), pcp);                                    // mov   [esp+4],pcp
		a.mov(dword_ptr(esp, 0), imm(m_device.debug()));                                // mov   [esp],device.debug
		a.call(imm(debugger_inst_hook));                                                // call  debugger_inst_hook

		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_exit - process an EXIT opcode
//-------------------------------------------------

void drcbe_x86::op_exit(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter retp(*this, inst.param(0), PTYPE_MRI);

	// load the parameter into EAX
	emit_mov_r32_p32(a, eax, retp);                                                     // mov   eax,retp
	if (inst.condition() == uml::COND_ALWAYS)
		a.jmp(imm(m_exit));                                                             // jmp   exit
	else
		a.j(X86_CONDITION(inst.condition()), imm(m_exit));                              // jcc   exit
}


//-------------------------------------------------
//  op_hashjmp - process a HASHJMP opcode
//-------------------------------------------------

void drcbe_x86::op_hashjmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter modep(*this, inst.param(0), PTYPE_MRI);
	be_parameter pcp(*this, inst.param(1), PTYPE_MRI);
	parameter const &exp = inst.param(2);
	assert(exp.is_code_handle());

	if (LOG_HASHJMPS)
	{
		emit_mov_m32_p32(a, dword_ptr(esp, 4), pcp);
		emit_mov_m32_p32(a, dword_ptr(esp, 0), modep);
		a.call(imm(debug_log_hashjmp));
	}

	// load the stack base one word early so we end up at the right spot after our call below
	a.mov(esp, MABS(&m_hashstacksave));                                                 // mov   esp,[hashstacksave]

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
			emit_mov_r32_p32(a, eax, pcp);                                              // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and  eax,l2mask << l2shift
			a.mov(edx, ptr(u64(&m_hash.base()[modep.immediate()][0]), edx, 2));
																						// mov   edx,hash[modep+edx*4]
			a.call(ptr(edx, eax, 2 - m_hash.l2shift()));                                // call  [edx+eax*shift]
		}
	}
	else
	{
		// variable mode
		Gp const modereg = modep.select_register(ecx);
		emit_mov_r32_p32(a, modereg, modep);                                            // mov   modereg,modep
		a.mov(ecx, ptr(u64(m_hash.base()), modereg, 2));                                // mov   ecx,hash[modereg*4]

		// fixed PC
		if (pcp.is_immediate())
		{
			uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			a.mov(edx, ptr(ecx, l1val*4));                                              // mov   edx,[ecx+l1val*4]
			a.call(ptr(edx, l2val*4));                                                  // call  [l2val*4]
		}

		// variable PC
		else
		{
			emit_mov_r32_p32(a, eax, pcp);                                              // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.mov(edx, ptr(ecx, edx, 2));                                               // mov   edx,[ecx+edx*4]
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and  eax,l2mask << l2shift
			a.call(ptr(edx, eax, 2 - m_hash.l2shift()));                                // call  [edx+eax*shift]
		}
	}

	// in all cases, if there is no code, we return here to generate the exception
	emit_mov_m32_p32(a, MABS(&m_state.exp, 4), pcp);                                    // mov   [exp],param
	a.sub(esp, 4);                                                                      // sub   esp,4
	a.call(MABS(exp.handle().codeptr_addr()));                                          // call  [exp]
}


//-------------------------------------------------
//  op_jmp - process a JMP opcode
//-------------------------------------------------

void drcbe_x86::op_jmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	parameter const &labelp = inst.param(0);
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

void drcbe_x86::op_exh(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	parameter const &handp = inst.param(0);
	assert(handp.is_code_handle());
	be_parameter exp(*this, inst.param(1), PTYPE_MRI);

	// look up the handle target
	drccodeptr *targetptr = handp.handle().codeptr_addr();

	// perform the exception processing
	Label no_exception = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.short_().j(X86_NOT_CONDITION(inst.condition()), no_exception);                // jcc   no_exception
	emit_mov_m32_p32(a, MABS(&m_state.exp, 4), exp);                                    // mov   [exp],exp
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

void drcbe_x86::op_callh(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	parameter const &handp = inst.param(0);
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
	{
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_ret - process a RET opcode
//-------------------------------------------------

void drcbe_x86::op_ret(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	// skip if conditional
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.j(X86_NOT_CONDITION(inst.condition()), skip);                                 // jcc   skip

	// return
	a.lea(esp, ptr(esp, 28));                                                           // lea   rsp,[rsp+28]
	a.ret();                                                                            // ret

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
	{
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_callc - process a CALLC opcode
//-------------------------------------------------

void drcbe_x86::op_callc(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	parameter const &funcp = inst.param(0);
	assert(funcp.is_c_function());
	be_parameter paramp(*this, inst.param(1), PTYPE_M);

	// skip if conditional
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.j(X86_NOT_CONDITION(inst.condition()), skip);                                 // jcc   skip

	// perform the call
	a.mov(dword_ptr(esp, 0), imm(paramp.memory()));                                     // mov   [esp],paramp
	a.call(imm(funcp.cfunc()));                                                         // call  funcp

	// resolve the conditional link
	if (inst.condition() != uml::COND_ALWAYS)
	{
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_recover - process a RECOVER opcode
//-------------------------------------------------

void drcbe_x86::op_recover(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// call the recovery code
	a.mov(eax, MABS(&m_stacksave));                                                     // mov   eax,stacksave
	a.mov(eax, ptr(eax, -4));                                                           // mov   eax,[eax-4]
	a.sub(eax, 1);                                                                      // sub   eax,1
	a.mov(dword_ptr(esp, 8), inst.param(1).mapvar());                                   // mov   [esp+8],param1
	a.mov(ptr(esp, 4), eax);                                                            // mov   [esp+4],eax
	a.mov(dword_ptr(esp, 0), imm(&m_map));                                              // mov   [esp],m_map
	a.call(imm(&drc_map_variables::static_get_value));                                  // call  drcmap_get_value
	emit_mov_p32_r32(a, dstp, eax);                                                     // mov   dstp,eax
}



//**************************************************************************
//  INTERNAL REGISTER OPCODES
//**************************************************************************

//-------------------------------------------------
//  op_setfmod - process a SETFMOD opcode
//-------------------------------------------------

void drcbe_x86::op_setfmod(Assembler &a, const instruction &inst)
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
		a.mov(MABS(&m_state.fmod, 1), value);                                           // mov   [fmod],srcp
		a.fldcw(MABS(&fp_control[value], 2));                                           // fldcw fp_control[srcp]
	}

	// register/memory case
	else
	{
		emit_mov_r32_p32(a, eax, srcp);                                                 // mov   eax,srcp
		a.and_(eax, 3);                                                                 // and   eax,3
		a.mov(MABS(&m_state.fmod), al);                                                 // mov   [fmod],al
		a.fldcw(ptr(u64(&fp_control[0]), eax, 1, 2));                                   // fldcw fp_control[eax]
	}
}


//-------------------------------------------------
//  op_getfmod - process a GETFMOD opcode
//-------------------------------------------------

void drcbe_x86::op_getfmod(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// fetch the current mode and store to the destination
	if (dstp.is_int_register())
		a.movzx(Gpd(dstp.ireg()), MABS(&m_state.fmod, 1));                              // movzx reg,[fmod]
	else
	{
		a.movzx(eax, MABS(&m_state.fmod, 1));                                           // movzx eax,[fmod]
		a.mov(MABS(dstp.memory()), eax);                                                // mov   [dstp],eax
	}
}


//-------------------------------------------------
//  op_getexp - process a GETEXP opcode
//-------------------------------------------------

void drcbe_x86::op_getexp(Assembler &a, const instruction &inst)
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

void drcbe_x86::op_getflgs(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter maskp(*this, inst.param(1), PTYPE_I);

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

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
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			break;

		case FLAG_C | FLAG_Z:
			a.setc(al);                                                                 // setc   al
			a.setz(cl);                                                                 // setz   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 2));                                            // lea    dstreg,[eax+ecx*4]
			break;

		case FLAG_C | FLAG_S:
			a.setc(al);                                                                 // setc   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 3));                                            // lea    dstreg,[eax+ecx*8]
			break;

		// overflow plus another flag
		case FLAG_V | FLAG_Z:
			a.seto(al);                                                                 // seto   al
			a.setz(cl);                                                                 // setz   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			a.shl(dstreg, 1);                                                           // shl    dstreg,1
			break;

		case FLAG_V | FLAG_S:
			a.seto(al);                                                                 // seto   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 2));                                            // lea    dstreg,[eax+ecx*4]
			a.shl(dstreg, 1);                                                           // shl    dstreg,1
			break;

		// zero plus another flag
		case FLAG_Z | FLAG_S:
			a.setz(al);                                                                 // setz   al
			a.sets(cl);                                                                 // sets   cl
			a.movzx(eax, al);                                                           // movzx  eax,al
			a.movzx(ecx, cl);                                                           // movzx  ecx,al
			a.lea(dstreg, ptr(eax, ecx, 1));                                            // lea    dstreg,[eax+ecx*2]
			a.shl(dstreg, 2);                                                           // shl    dstreg,2
			break;

		// default cases
		default:
			a.pushfd();                                                                 // pushf
			a.pop(eax);                                                                 // pop    eax
			a.and_(eax, flagmask);                                                      // and    eax,flagmask
			a.movzx(dstreg, byte_ptr(u64(flags_map), eax));                             // movzx  dstreg,[flags_map]
			break;
	}

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// general case
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory(4), 4), 0);                                          // mov   [dstp+4],0
		else if (dstp.is_int_register())
			a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                    // mov   [reghi],0
	}
}


//-------------------------------------------------
//  op_save - process a SAVE opcode
//-------------------------------------------------

void drcbe_x86::op_save(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_M);

	// copy live state to the destination
	a.mov(ecx, imm(dstp.memory()));                                                     // mov    ecx,dstp
	a.call(imm(m_save));                                                                // call   save
}


//-------------------------------------------------
//  op_restore - process a RESTORE opcode
//-------------------------------------------------

void drcbe_x86::op_restore(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4);
	assert_no_condition(inst);

	// normalize parameters
	be_parameter srcp(*this, inst.param(0), PTYPE_M);

	// copy live state from the destination
	a.mov(ecx, imm(srcp.memory()));                                                     // mov    ecx,dstp
	a.call(imm(m_restore));                                                             // call   restore
}



//**************************************************************************
//  INTEGER OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  op_load - process a LOAD opcode
//-------------------------------------------------

void drcbe_x86::op_load(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	parameter const &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	int const size = scalesizep.size();

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// immediate index
	if (indp.is_immediate())
	{
		int const scale = 1 << scalesizep.scale();

		if (size == SIZE_BYTE)
			a.movzx(dstreg, MABS(basep.memory(scale*indp.immediate()), 1));             // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movzx(dstreg, MABS(basep.memory(scale*indp.immediate()), 2));             // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, MABS(basep.memory(scale*indp.immediate())));                  // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			a.mov(edx, MABS(basep.memory(scale*indp.immediate() + 4)));                 // mov   edx,[basep + scale*indp + 4]
			a.mov(dstreg, MABS(basep.memory(scale*indp.immediate())));                  // mov   dstreg,[basep + scale*indp]
		}
	}

	// other index
	else
	{
		Gp const indreg = indp.select_register(ecx);
		emit_mov_r32_p32(a, indreg, indp);
		if (size == SIZE_BYTE)
			a.movzx(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale(), 1));   // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movzx(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale(), 2));   // movzx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale()));        // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			a.mov(edx, ptr(u64(basep.memory(4)), indreg, scalesizep.scale()));          // mov   edx,[basep + scale*indp + 4]
			a.mov(dstreg, ptr(u64(basep.memory(0)), indreg, scalesizep.scale()));       // mov   dstreg,[basep + scale*indp]
		}
	}

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (size != SIZE_QWORD)
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   [dstp+4],0
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4)), edx);                                       // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()]), edx);                                 // mov   [reghi],edx
			set_last_upper_reg(a, dstp, edx);
		}
	}
	set_last_lower_reg(a, dstp, dstreg);
}


//-------------------------------------------------
//  op_loads - process a LOADS opcode
//-------------------------------------------------

void drcbe_x86::op_loads(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	parameter const &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	int const size = scalesizep.size();

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// immediate index
	if (indp.is_immediate())
	{
		int const scale = 1 << scalesizep.scale();

		if (size == SIZE_BYTE)
			a.movsx(dstreg, MABS(basep.memory(scale*indp.immediate()), 1));             // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movsx(dstreg, MABS(basep.memory(scale*indp.immediate()), 2));             // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, MABS(basep.memory(scale*indp.immediate())));                  // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			a.mov(edx, MABS(basep.memory(scale*indp.immediate() + 4)));                 // mov   edx,[basep + scale*indp + 4]
			a.mov(dstreg, MABS(basep.memory(scale*indp.immediate())));                  // mov   dstreg,[basep + scale*indp]
		}
	}

	// other index
	else
	{
		Gp const indreg = indp.select_register(ecx);
		emit_mov_r32_p32(a, indreg, indp);
		if (size == SIZE_BYTE)
			a.movsx(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale(), 1));   // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_WORD)
			a.movsx(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale(), 2));   // movsx dstreg,[basep + scale*indp]
		else if (size == SIZE_DWORD)
			a.mov(dstreg, ptr(u64(basep.memory()), indreg, scalesizep.scale()));        // mov   dstreg,[basep + scale*indp]
		else if (size == SIZE_QWORD)
		{
			a.mov(edx, ptr(u64(basep.memory(4)), indreg, scalesizep.scale()));          // mov   edx,[basep + scale*indp + 4]
			a.mov(dstreg, ptr(u64(basep.memory(0)), indreg, scalesizep.scale()));       // mov   dstreg,[basep + scale*indp]
		}
	}

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		a.cdq();                                                                        // cdq
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory(4)), edx);                                           // mov   [dstp+4],edx
		else if (dstp.is_int_register())
			a.mov(MABS(m_reghi[dstp.ireg()]), edx);                                     // mov   [reghi],edx
		set_last_upper_reg(a, dstp, edx);
	}
	set_last_lower_reg(a, dstp, dstreg);
}


//-------------------------------------------------
//  op_store - process a STORE opcode
//-------------------------------------------------

void drcbe_x86::op_store(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MRI);
	parameter const &scalesizep = inst.param(3);
	int const size = scalesizep.size();

	// pick a source register for the general case
	Gp srcreg = srcp.select_register(eax);
	if (size == SIZE_BYTE && (srcreg.id() & 4)) // FIXME: &4?
		srcreg = eax;

	// degenerate case: constant index
	if (indp.is_immediate())
	{
		int const scale = 1 << (scalesizep.scale());

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				a.mov(MABS(basep.memory(scale*indp.immediate()), 1), srcp.immediate()); // mov   [basep + scale*indp],srcp
			else if (size == SIZE_WORD)
				a.mov(MABS(basep.memory(scale*indp.immediate()), 2), srcp.immediate()); // mov   [basep + scale*indp],srcp
			else if (size == SIZE_DWORD)
				a.mov(MABS(basep.memory(scale*indp.immediate()), 4), srcp.immediate()); // mov   [basep + scale*indp],srcp
			else if (size == SIZE_QWORD)
			{
				a.mov(MABS(basep.memory(scale*indp.immediate()), 4), srcp.immediate()); // mov   [basep + scale*indp],srcp
				a.mov(MABS(basep.memory(scale*indp.immediate() + 4), 4), srcp.immediate() >> 32);
																						// mov   [basep + scale*indp + 4],srcp >> 32
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(a, srcreg, srcp);                                      // mov   srcreg,srcp
			else
				emit_mov_r64_p64(a, srcreg, edx, srcp);                                 // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				a.mov(MABS(basep.memory(scale*indp.immediate())), srcreg.r8());         // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_WORD)
				a.mov(MABS(basep.memory(scale*indp.immediate())), srcreg.r16());        // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_DWORD)
				a.mov(MABS(basep.memory(scale*indp.immediate())), srcreg);              // mov   [basep + scale*indp],srcreg
			else if (size == SIZE_QWORD)
			{
				a.mov(MABS(basep.memory(scale*indp.immediate())), srcreg);              // mov   [basep + scale*indp],srcreg
				a.mov(MABS(basep.memory(scale*indp.immediate() + 4)), edx);             // mov   [basep + scale*indp + 4],edx
			}
		}
	}

	// normal case: variable index
	else
	{
		Gp const indreg = indp.select_register(ecx);
		emit_mov_r32_p32(a, indreg, indp);                                              // mov   indreg,indp

		// immediate source
		if (srcp.is_immediate())
		{
			if (size == SIZE_BYTE)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale(), 1), srcp.immediate());   // mov   [basep + 1*ecx],srcp
			else if (size == SIZE_WORD)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale(), 2), srcp.immediate());  // mov   [basep + 2*ecx],srcp
			else if (size == SIZE_DWORD)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale(), 4), srcp.immediate());  // mov   [basep + 4*ecx],srcp
			else if (size == SIZE_QWORD)
			{
				a.mov(ptr(u64(basep.memory(0)), indreg, scalesizep.scale(), 4), srcp.immediate());  // mov   [basep + 8*ecx],srcp
				a.mov(ptr(u64(basep.memory(4)), indreg, scalesizep.scale(), 4), srcp.immediate() >> 32);
																						// mov   [basep + 8*ecx + 4],srcp >> 32
			}
		}

		// variable source
		else
		{
			if (size != SIZE_QWORD)
				emit_mov_r32_p32(a, srcreg, srcp);                                      // mov   srcreg,srcp
			else
				emit_mov_r64_p64(a, srcreg, edx, srcp);                                 // mov   edx:srcreg,srcp
			if (size == SIZE_BYTE)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale()), srcreg.r8());   // mov   [basep + 1*ecx],srcreg
			else if (size == SIZE_WORD)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale()), srcreg.r16());  // mov   [basep + 2*ecx],srcreg
			else if (size == SIZE_DWORD)
				a.mov(ptr(u64(basep.memory()), indreg, scalesizep.scale()), srcreg);    // mov   [basep + 4*ecx],srcreg
			else if (size == SIZE_QWORD)
			{
				a.mov(ptr(u64(basep.memory(0)), indreg, scalesizep.scale()), srcreg);   // mov   [basep + 8*ecx],srcreg
				a.mov(ptr(u64(basep.memory(4)), indreg, scalesizep.scale()), edx);      // mov   [basep + 8*ecx],edx
			}
		}
	}
}


//-------------------------------------------------
//  op_read - process a READ opcode
//-------------------------------------------------

void drcbe_x86::op_read(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	parameter const &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// set up a call to the read byte handler
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacesizep.space()]));                         // mov    [esp],space
	if (spacesizep.size() == SIZE_BYTE)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_byte));                         // call   read_byte
		a.movzx(dstreg, al);                                                            // movzx  dstreg,al
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_word));                         // call   read_word
		a.movzx(dstreg, ax);                                                            // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_dword));                        // call   read_dword
		a.mov(dstreg, eax);                                                             // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_qword));                        // call   read_qword
		a.mov(dstreg, eax);                                                             // mov    dstreg,eax
	}

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov    dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (spacesizep.size() != SIZE_QWORD)
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   [dstp+4],0
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4)), edx);                                       // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()]), edx);                                 // mov   [reghi],edx
		}
	}
}


//-------------------------------------------------
//  op_readm - process a READM opcode
//-------------------------------------------------

void drcbe_x86::op_readm(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	parameter const &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// set up a call to the read byte handler
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_m32_p32(a, dword_ptr(esp, 8), maskp);                                  // mov    [esp+8],maskp
	else
		emit_mov_m64_p64(a, qword_ptr(esp, 8), maskp);                                  // mov    [esp+8],maskp
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacesizep.space()]));                         // mov    [esp],space
	if (spacesizep.size() == SIZE_WORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_word_masked));                  // call   read_word_masked
		a.movzx(dstreg, ax);                                                            // movzx  dstreg,ax
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_dword_masked));                 // call   read_dword_masked
		a.mov(dstreg, eax);                                                             // mov    dstreg,eax
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		a.call(imm(m_accessors[spacesizep.space()].read_qword_masked));                 // call   read_qword_masked
		a.mov(dstreg, eax);                                                             // mov    dstreg,eax
	}

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov    dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// 1, 2, or 4-byte case
		if (spacesizep.size() != SIZE_QWORD)
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   [dstp+4],0
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   [reghi],0
		}

		// 8-byte case
		else
		{
			if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4)), edx);                                       // mov   [dstp+4],edx
			else if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()]), edx);                                 // mov   [reghi],edx
		}
	}
}


//-------------------------------------------------
//  op_write - process a WRITE opcode
//-------------------------------------------------

void drcbe_x86::op_write(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	parameter const &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	// set up a call to the write byte handler
	if (spacesizep.size() != SIZE_QWORD)
		emit_mov_m32_p32(a, dword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	else
		emit_mov_m64_p64(a, qword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacesizep.space()]));                         // mov    [esp],space
	if (spacesizep.size() == SIZE_BYTE)
		a.call(imm(m_accessors[spacesizep.space()].write_byte));                        // call   write_byte
	else if (spacesizep.size() == SIZE_WORD)
		a.call(imm(m_accessors[spacesizep.space()].write_word));                        // call   write_word
	else if (spacesizep.size() == SIZE_DWORD)
		a.call(imm(m_accessors[spacesizep.space()].write_dword));                       // call   write_dword
	else if (spacesizep.size() == SIZE_QWORD)
		a.call(imm(m_accessors[spacesizep.space()].write_qword));                       // call   write_qword
}


//-------------------------------------------------
//  op_writem - process a WRITEM opcode
//-------------------------------------------------

void drcbe_x86::op_writem(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	parameter const &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// set up a call to the write byte handler
	if (spacesizep.size() != SIZE_QWORD)
	{
		emit_mov_m32_p32(a, dword_ptr(esp, 12), maskp);                                 // mov    [esp+12],maskp
		emit_mov_m32_p32(a, dword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	}
	else
	{
		emit_mov_m64_p64(a, qword_ptr(esp, 16), maskp);                                 // mov    [esp+16],maskp
		emit_mov_m64_p64(a, qword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	}
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacesizep.space()]));                         // mov    [esp],space
	if (spacesizep.size() == SIZE_WORD)
		a.call(imm(m_accessors[spacesizep.space()].write_word_masked));                 // call   write_word_masked
	else if (spacesizep.size() == SIZE_DWORD)
		a.call(imm(m_accessors[spacesizep.space()].write_dword_masked));                // call   write_dword_masked
	else if (spacesizep.size() == SIZE_QWORD)
		a.call(imm(m_accessors[spacesizep.space()].write_qword_masked));                // call   write_qword_masked
}


//-------------------------------------------------
//  op_carry - process a CARRY opcode
//-------------------------------------------------

void drcbe_x86::op_carry(Assembler &a, const instruction &inst)
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
		if (srcp.immediate() & ((uint64_t)1 << bitp.immediate()))
			a.stc();
		else
			a.clc();
		}

	// load non-immediate bit numbers into a register
	if (!bitp.is_immediate())
	{
		emit_mov_r32_p32(a, ecx, bitp);
		a.and_(ecx, inst.size() * 8 - 1);
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		if (bitp.is_immediate())
		{
			if (srcp.is_memory())
				a.bt(MABS(srcp.memory(), 4), bitp.immediate());                         // bt     [srcp],bitp
			else if (srcp.is_int_register())
				a.bt(Gpd(srcp.ireg()), bitp.immediate());                               // bt     srcp,bitp
		}
		else
		{
			if (srcp.is_memory())
				a.bt(MABS(srcp.memory()), ecx);                                         // bt     [srcp],ecx
			else if (srcp.is_int_register())
				a.bt(Gpd(srcp.ireg()), ecx);                                            // bt     [srcp],ecx
		}
	}

	// 64-bit form
	else
	{
		if (bitp.is_immediate())
		{
			if (srcp.is_memory())
				a.bt(MABS(srcp.memory(), 4), bitp.immediate());                         // bt     [srcp],bitp
			else if (srcp.is_int_register() && bitp.immediate() < 32)
				a.bt(Gpd(srcp.ireg()), bitp.immediate());                               // bt     srcp,bitp
			else if (srcp.is_int_register() && bitp.immediate() >= 32)
				a.bt(MABS(m_reghi[srcp.ireg()], 4), bitp.immediate() - 32);             // bt     [srcp.hi],bitp
		}
		else
		{
			if (srcp.is_memory())
				a.bt(MABS(srcp.memory()), ecx);                                         // bt     [srcp],ecx
			else if (srcp.is_int_register())
			{
				a.mov(MABS(m_reglo[srcp.ireg()]), Gpd(srcp.ireg()));                    // mov    [srcp.lo],srcp
				a.bt(MABS(m_reglo[srcp.ireg()]), ecx);                                  // bt     [srcp],ecx
			}
		}
	}
}


//-------------------------------------------------
//  op_set - process a SET opcode
//-------------------------------------------------

void drcbe_x86::op_set(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// set to AL
	a.set(X86_CONDITION(inst.condition()), al);                                         // setcc  al
	a.movzx(dstreg, al);                                                                // movzx  dstreg,al

	// store low 32 bits
	emit_mov_p32_r32(a, dstp, dstreg);                                                  // mov   dstp,dstreg

	// 64-bit form stores upper 32 bits
	if (inst.size() == 8)
	{
		// general case
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory(4), 4), 0);                                          // mov   [dstp+4],0
		else if (dstp.is_int_register())
			a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                    // mov   [reghi],0
	}
}


//-------------------------------------------------
//  op_mov - process a MOV opcode
//-------------------------------------------------

void drcbe_x86::op_mov(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// add a conditional branch unless a conditional move is possible
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS && ((inst.size() == 8) || !(dstp.is_int_register() && !srcp.is_immediate())))
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip

	// 32-bit form
	if (inst.size() == 4)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
			a.mov(MABS(dstp.memory()), Gpd(srcp.ireg()));                               // mov   [dstp],srcp

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate())
			a.mov(MABS(dstp.memory(), 4), srcp.immediate());                            // mov   [dstp],srcp

		// conditional memory to register
		else if (inst.condition() != uml::COND_ALWAYS && dstp.is_int_register() && srcp.is_memory())
			a.cmov(X86_CONDITION(inst.condition()), Gpd(dstp.ireg()), MABS(srcp.memory()));
																						// cmovcc dstp,[srcp]

		// conditional register to register
		else if (inst.condition() != uml::COND_ALWAYS && dstp.is_int_register() && srcp.is_int_register())
			a.cmov(X86_CONDITION(inst.condition()), Gpd(dstp.ireg()), Gpd(srcp.ireg()));
																						// cmovcc dstp,srcp

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(a, dstreg, srcp);                                // mov   dstreg,srcp
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// register to memory
		if (dstp.is_memory() && srcp.is_int_register())
		{
			a.mov(eax, MABS(m_reghi[srcp.ireg()]));                                     // mov   eax,reghi[srcp]
			a.mov(MABS(dstp.memory(0)), Gpd(srcp.ireg()));                              // mov   [dstp],srcp
			a.mov(MABS(dstp.memory(4)), eax);                                           // mov   [dstp+4],eax
		}

		// immediate to memory
		else if (dstp.is_memory() && srcp.is_immediate())
		{
			a.mov(MABS(dstp.memory(0), 4), srcp.immediate());                           // mov   [dstp],srcp
			a.mov(MABS(dstp.memory(4), 4), srcp.immediate() >> 32);                     // mov   [dstp+4],srcp >> 32
		}

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, srcp);                                     // mov   edx:dstreg,srcp
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}

	// bind the label
	if (inst.condition() != uml::COND_ALWAYS && ((inst.size() == 8) || !(dstp.is_int_register() && !srcp.is_immediate())))
	{
		a.bind(skip);
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_sext - process a SEXT opcode
//-------------------------------------------------

void drcbe_x86::op_sext(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	parameter const &sizep = inst.param(2);
	assert(sizep.is_size());

	// pick a target register for the general case
	Gp const dstreg = (inst.size() == 8) ? eax : dstp.select_register(eax);

	// convert 8-bit source registers to EAX
	if (sizep.size() == SIZE_BYTE && srcp.is_int_register() && (srcp.ireg() & 4))
	{
		a.mov(eax, Gpd(srcp.ireg()));                                                   // mov   eax,srcp
		srcp = be_parameter::make_ireg(eax.id());
	}

	// general case
	if (srcp.is_memory())
	{
		if (sizep.size() == SIZE_BYTE)
			a.movsx(dstreg, MABS(srcp.memory(), 1));                                    // movsx dstreg,[srcp]
		else if (sizep.size() == SIZE_WORD)
			a.movsx(dstreg, MABS(srcp.memory(), 2));                                    // movsx dstreg,[srcp]
		else if (sizep.size() == SIZE_DWORD)
			a.mov(dstreg, MABS(srcp.memory()));                                         // mov   dstreg,[srcp]
	}
	else if (srcp.is_int_register())
	{
		if (sizep.size() == SIZE_BYTE)
			a.movsx(dstreg, GpbLo(srcp.ireg()));                                        // movsx dstreg,srcp
		else if (sizep.size() == SIZE_WORD)
			a.movsx(dstreg, Gpw(srcp.ireg()));                                          // movsx dstreg,srcp
		else if (sizep.size() == SIZE_DWORD && dstreg.id() != srcp.ireg())
			a.mov(dstreg, Gpd(srcp.ireg()));                                            // mov   dstreg,srcp
	}
	if (inst.flags() != 0)
		a.test(dstreg, dstreg);                                                         // test  dstreg,dstreg

	// 32-bit form: store the low 32 bits
	if (inst.size() == 4)
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg

	// 64-bit form: sign extend to 64 bits and store edx:eax
	else if (inst.size() == 8)
	{
		a.cdq();                                                                        // cdq
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
	}
}


//-------------------------------------------------
//  op_roland - process an ROLAND opcode
//-------------------------------------------------

void drcbe_x86::op_roland(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, shiftp, maskp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(a, dstreg, srcp);                                              // mov   dstreg,srcp
		shift_op_param(a, Inst::kIdRol, dstreg, shiftp,                                 // rol   dstreg,shiftp
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});
		alu_op_param(a, Inst::kIdAnd, dstreg, maskp,                                    // and   dstreg,maskp
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize all-zero and all-one cases
				if (!inst.flags() && !src.immediate())
				{
					a.xor_(dst.as<Gpd>(), dst.as<Gpd>());
					return true;
				}
				else if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
					return true;

				return false;
			});
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(a, dstreg, edx, srcp);                                         // mov   edx:dstreg,srcp
		emit_rol_r64_p64(a, dstreg, edx, shiftp, inst);                                 // rol   edx:dstreg,shiftp
		emit_and_r64_p64(a, dstreg, edx, maskp, inst);                                  // and   edx:dstreg,maskp
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_rolins - process an ROLINS opcode
//-------------------------------------------------

void drcbe_x86::op_rolins(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(ecx, shiftp, maskp);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(a, eax, srcp);                                                 // mov   eax,srcp
		shift_op_param(a, Inst::kIdRol, eax, shiftp,                                    // rol   eax,shiftp
			[inst](Assembler &a, Operand const &dst, be_parameter const &src)
			{
				// optimize zero case
				return (!inst.flags() && !src.immediate());
			});
		emit_mov_r32_p32(a, dstreg, dstp);                                              // mov   dstreg,dstp
		if (maskp.is_immediate())
		{
			a.and_(eax, maskp.immediate());                                             // and   eax,maskp
			a.and_(dstreg, ~maskp.immediate());                                         // and   dstreg,~maskp
		}
		else
		{
			emit_mov_r32_p32(a, edx, maskp);                                            // mov   edx,maskp
			a.and_(eax, edx);                                                           // and   eax,edx
			a.not_(edx);                                                                // not   edx
			a.and_(dstreg, edx);                                                        // and   dstreg,edx
		}
		a.or_(dstreg, eax);                                                             // or    dstreg,eax
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(a, eax, edx, srcp);                                            // mov   edx:eax,srcp
		emit_rol_r64_p64(a, eax, edx, shiftp, inst);                                    // rol   edx:eax,shiftp
		if (maskp.is_immediate())
		{
			a.and_(eax, maskp.immediate());                                             // and   eax,maskp
			a.and_(edx, maskp.immediate() >> 32);                                       // and   edx,maskp >> 32
			if (dstp.is_int_register())
			{
				a.and_(Gpd(dstp.ireg()), ~maskp.immediate());                           // and   dstp.lo,~maskp
				a.and_(MABS(m_reghi[dstp.ireg()], 4), ~maskp.immediate() >> 32);        // and   dstp.hi,~maskp >> 32
				a.or_(Gpd(dstp.ireg()), eax);                                           // or    dstp.lo,eax
				a.or_(MABS(m_reghi[dstp.ireg()]), edx);                                 // or    dstp.hi,edx
			}
			else
			{
				a.and_(MABS(dstp.memory(0), 4), ~maskp.immediate());                    // and   dstp.lo,~maskp
				a.and_(MABS(dstp.memory(4), 4), ~maskp.immediate() >> 32);              // and   dstp.hi,~maskp >> 32
				a.or_(MABS(dstp.memory(0)), eax);                                       // or    dstp.lo,eax
				a.or_(MABS(dstp.memory(4)), edx);                                       // or    dstp.hi,edx
			}
		}
		else
		{
			a.mov(ptr(esp, -8), ebx);                                                   // mov   [esp-8],ebx
			emit_mov_r64_p64(a, ebx, ecx, maskp);                                       // mov   ecx:ebx,maskp
			a.and_(eax, ebx);                                                           // and   eax,ebx
			a.and_(edx, ecx);                                                           // and   edx,ecx
			a.not_(ebx);                                                                // not   ebx
			a.not_(ecx);                                                                // not   ecx
			if (dstp.is_int_register())
			{
				a.and_(Gpd(dstp.ireg()), ebx);                                          // and   dstp.lo,ebx
				a.and_(MABS(m_reghi[dstp.ireg()]), ecx);                                // and   dstp.hi,ecx
				a.or_(Gpd(dstp.ireg()), eax);                                           // or    dstp.lo,eax
				a.or_(MABS(m_reghi[dstp.ireg()]), edx);                                 // or    dstp.hi,edx
			}
			else
			{
				a.and_(MABS(dstp.memory(0)), ebx);                                      // and   dstp.lo,ebx
				a.and_(MABS(dstp.memory(4)), ecx);                                      // and   dstp.hi,ecx
				a.or_(MABS(dstp.memory(0)), eax);                                       // or    dstp.lo,eax
				a.or_(MABS(dstp.memory(4)), edx);                                       // or    dstp.hi,edx
			}
			a.mov(ebx, ptr(esp, -8));                                                   // mov   ebx,[esp-8]
		}
		if (inst.flags() == FLAG_Z)
			a.or_(eax, edx);                                                            // or    eax,edx
		else if (inst.flags() == FLAG_S)
			;// do nothing -- final OR will have the right result
		else if (inst.flags() == (FLAG_Z | FLAG_S))
		{
			a.movzx(ecx, ax);                                                           // movzx ecx,ax
			a.shr(eax, 16);                                                             // shr   eax,16
			a.or_(edx, ecx);                                                            // or    edx,ecx
			a.or_(edx, eax);                                                            // or    edx,eax
		}
	}
}


//-------------------------------------------------
//  op_add - process a ADD opcode
//-------------------------------------------------

void drcbe_x86::op_add(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdAdd, MABS(dstp.memory(), 4), src2p,                // add   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// reg = reg + imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && !inst.flags())
			a.lea(Gpd(dstp.ireg()), ptr(Gpd(src1p.ireg()), src2p.immediate()));         // lea   dstp,[src1p+src2p]

		// reg = reg + reg
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_int_register() && !inst.flags())
			a.lea(Gpd(dstp.ireg()), ptr(Gpd(src1p.ireg()), Gpd(src2p.ireg())));         // lea   dstp,[src1p+src2p]

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdAdd, dstreg, src2p,                                // add   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdAdd, Inst::kIdAdc,                                 // add   [dstp],src2p
				MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4), src2p, inst.flags() & FLAG_Z);

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, src1p);                                    // mov   edx:dstreg,[src1p]
			alu_op_param(a, Inst::kIdAdd, Inst::kIdAdc,                                 // add   edx:dstreg,src2p
				dstreg, edx, src2p, inst.flags() & FLAG_Z);
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_addc - process a ADDC opcode
//-------------------------------------------------

void drcbe_x86::op_addc(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdAdc, MABS(dstp.memory(), 4), src2p);               // adc   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(a, dstreg, src1p);                               // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdAdc, dstreg, src2p);                               // adc   dstreg,src2p
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdAdc, Inst::kIdAdc,                                 // adc   [dstp],src2p
				MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4), src2p, inst.flags() & FLAG_Z);

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(a, dstreg, edx, src1p);                          // mov   edx:dstreg,[src1p]
			alu_op_param(a, Inst::kIdAdc, Inst::kIdAdc,                                 // adc   edx:dstreg,src2p
				dstreg, edx, src2p, inst.flags() & FLAG_Z);
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_sub - process a SUB opcode
//-------------------------------------------------

void drcbe_x86::op_sub(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdSub, MABS(dstp.memory(), 4), src2p,                // sub   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// reg = reg - imm
		else if (dstp.is_int_register() && src1p.is_int_register() && src2p.is_immediate() && !inst.flags())
			a.lea(Gpd(dstp.ireg()), ptr(Gpd(src1p.ireg()), -src2p.immediate()));        // lea   dstp,[src1p-src2p]

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdSub, dstreg, src2p,                                // sub   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdSub, Inst::kIdSbb,                                 // sub   [dstp],src2p
				MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4), src2p, inst.flags() & FLAG_Z);

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, src1p);                                    // mov   edx:dstreg,[src1p]
			alu_op_param(a, Inst::kIdSub, Inst::kIdSbb,                                 // sub   edx:dstreg,src2p
				dstreg, edx, src2p, inst.flags() & FLAG_Z);
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_subc - process a SUBC opcode
//-------------------------------------------------

void drcbe_x86::op_subc(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdSbb, MABS(dstp.memory(), 4), src2p);               // sbb   [dstp],src2p

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(a, dstreg, src1p);                               // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdSbb, dstreg, src2p);                               // sbb   dstreg,src2p
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdSbb, Inst::kIdSbb,                                 // sbb   [dstp],src2p
				MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4), src2p, inst.flags() & FLAG_Z);

		// general case
		else
		{
			emit_mov_r64_p64_keepflags(a, dstreg, edx, src1p);                          // mov   edx:dstreg,[src1p]
			alu_op_param(a, Inst::kIdSbb, Inst::kIdSbb,                                 // sbb   edx:dstreg,src2p
				dstreg, edx, src2p, inst.flags() & FLAG_Z);
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_cmp - process a CMP opcode
//-------------------------------------------------

void drcbe_x86::op_cmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	Gp const src1reg = src1p.select_register(eax);

	// 32-bit form
	if (inst.size() == 4)
	{
		// memory versus anything
		if (src1p.is_memory())
			alu_op_param(a, Inst::kIdCmp, MABS(src1p.memory(), 4), src2p);              // cmp   [src1p],src2p

		// general case
		else
		{
			if (src1p.is_immediate())
				a.mov(src1reg, src1p.immediate());                                      // mov   src1reg,imm
			alu_op_param(a, Inst::kIdCmp, src1reg, src2p);                              // cmp   src1reg,src2p
		}
	}

	// 64-bit form
	else
	{
		// general case
		emit_mov_r64_p64(a, eax, edx, src1p);                                           // mov   edx:eax,[src1p]
		alu_op_param(a, Inst::kIdSub, Inst::kIdSbb,                                     // cmp   edx:eax,src2p
			eax, edx, src2p, (inst.flags() & FLAG_Z) && (inst.flags() != FLAG_Z));
		if (inst.flags() == FLAG_Z)
			a.or_(edx, eax);
	}
}


//-------------------------------------------------
//  op_mulu - process a MULU opcode
//-------------------------------------------------

void drcbe_x86::op_mulu(Assembler &a, const instruction &inst)
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
		emit_mov_r32_p32(a, eax, src1p);                                                // mov   eax,src1p
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 4));                                             // mul   [src2p]
		else if (src2p.is_int_register())
			a.mul(Gpd(src2p.ireg()));                                                   // mul   src2p
		else if (src2p.is_immediate())
		{
			a.mov(edx, src2p.immediate());                                              // mov   edx,src2p
			a.mul(edx);                                                                 // mul   edx
		}
		emit_mov_p32_r32(a, dstp, eax);                                                 // mov   dstp,eax
		if (compute_hi)
			emit_mov_p32_r32(a, edstp, edx);                                            // mov   edstp,edx

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfd();                                                         // pushf
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
					a.pushfd();                                                         // pushf
					a.pop(eax);                                                         // pop   eax
					a.and_(dword_ptr(esp, 0), ~0x84);                                   // and   [esp],~0x84
					a.or_(ptr(esp, 0), eax);                                            // or    [esp],eax
					a.popfd();                                                          // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		a.mov(dword_ptr(esp, 24), inst.flags());                                        // mov   [esp+24],flags
		emit_mov_m64_p64(a, qword_ptr(esp, 16), src2p);                                 // mov   [esp+16],src2p
		emit_mov_m64_p64(a, qword_ptr(esp, 8), src1p);                                  // mov   [esp+8],src1p
		if (!compute_hi)
			a.mov(dword_ptr(esp, 4), imm(&m_reslo));                                    // mov   [esp+4],&reslo
		else
			a.mov(dword_ptr(esp, 4), imm(&m_reshi));                                    // mov   [esp+4],&reshi
		a.mov(dword_ptr(esp, 0), imm(&m_reslo));                                        // mov   [esp],&reslo
		a.call(imm(dmulu));                                                             // call  dmulu
		if (inst.flags() != 0)
			a.push(ptr(u64(flags_unmap), eax, 2));                                      // push   flags_unmap[eax*4]
		a.mov(eax, MABS((uint32_t *)&m_reslo + 0));                                     // mov   eax,reslo.lo
		a.mov(edx, MABS((uint32_t *)&m_reslo + 1));                                     // mov   edx,reslo.hi
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
		if (compute_hi)
		{
			a.mov(eax, MABS((uint32_t *)&m_reshi + 0));                                 // mov   eax,reshi.lo
			a.mov(ecx, MABS((uint32_t *)&m_reshi + 1));                                 // mov   ecx,reshi.hi
			emit_mov_p64_r64(a, edstp, eax, ecx);                                       // mov   edstp,ecx:eax
		}
		if (inst.flags() != 0)
			a.popfd();                                                                  // popf
	}
}


//-------------------------------------------------
//  op_muls - process a MULS opcode
//-------------------------------------------------

void drcbe_x86::op_muls(Assembler &a, const instruction &inst)
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
			if (src1p.is_memory())
				a.imul(eax, MABS(src1p.memory(), 4), src2p.immediate());                // imul  eax,[src1p],src2p
			else if (src1p.is_int_register())
				a.imul(eax, Gpd(src1p.ireg()), src2p.immediate());                      // imul  eax,src1p,src2p
			emit_mov_p32_r32(a, dstp, eax);                                             // mov   dstp,eax
		}

		// 32-bit destination, general case
		else if (!compute_hi)
		{
			emit_mov_r32_p32(a, eax, src1p);                                            // mov   eax,src1p
			if (src2p.is_memory())
				a.imul(eax, MABS(src2p.memory(), 4));                                   // imul  eax,[src2p]
			else if (src2p.is_int_register())
				a.imul(eax, Gpd(src2p.ireg()));                                         // imul  eax,src2p
			emit_mov_p32_r32(a, dstp, eax);                                             // mov   dstp,eax
		}

		// 64-bit destination, general case
		else
		{
			emit_mov_r32_p32(a, eax, src1p);                                            // mov   eax,src1p
			if (src2p.is_memory())
				a.imul(MABS(src2p.memory(), 4));                                        // imul  [src2p]
			else if (src2p.is_int_register())
				a.imul(Gpd(src2p.ireg()));                                              // imul  src2p
			else if (src2p.is_immediate())
			{
				a.mov(edx, src2p.immediate());                                          // mov   edx,src2p
				a.imul(edx);                                                            // imul  edx
			}
			emit_mov_p32_r32(a, dstp, eax);                                             // mov   dstp,eax
			emit_mov_p32_r32(a, edstp, edx);                                            // mov   edstp,edx
		}

		// compute flags
		if (inst.flags() != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					a.pushfd();                                                         // pushf
				if (compute_hi)
				{
					if (inst.flags() == FLAG_Z)
						a.or_(edx, eax);                                                // or    edx,eax
					else if (inst.flags() == FLAG_S)
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
					a.pushfd();                                                         // pushf
					a.pop(eax);                                                         // pop   eax
					a.and_(dword_ptr(esp, 0), ~0x84);                                   // and   [esp],~0x84
					a.or_(ptr(esp, 0), eax);                                            // or    [esp],eax
					a.popfd();                                                          // popf
				}
			}
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		a.mov(dword_ptr(esp, 24), inst.flags());                                        // mov   [esp+24],flags
		emit_mov_m64_p64(a, qword_ptr(esp, 16), src2p);                                 // mov   [esp+16],src2p
		emit_mov_m64_p64(a, qword_ptr(esp, 8), src1p);                                  // mov   [esp+8],src1p
		if (!compute_hi)
			a.mov(dword_ptr(esp, 4), imm(&m_reslo));                                    // mov   [esp+4],&reslo
		else
			a.mov(dword_ptr(esp, 4), imm(&m_reshi));                                    // push  [esp+4],&reshi
		a.mov(dword_ptr(esp, 0), imm(&m_reslo));                                        // mov   [esp],&reslo
		a.call(imm(dmuls));                                                             // call  dmuls
		if (inst.flags() != 0)
			a.push(ptr(u64(flags_unmap), eax, 2));                                      // push   flags_unmap[eax*4]
		a.mov(eax, MABS((uint32_t *)&m_reslo + 0));                                     // mov   eax,reslo.lo
		a.mov(edx, MABS((uint32_t *)&m_reslo + 1));                                     // mov   edx,reslo.hi
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
		if (compute_hi)
		{
			a.mov(eax, MABS((uint32_t *)&m_reshi + 0));                                 // mov   eax,reshi.lo
			a.mov(edx, MABS((uint32_t *)&m_reshi + 1));                                 // mov   edx,reshi.hi
			emit_mov_p64_r64(a, edstp, eax, edx);                                       // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			a.popfd();                                                                  // popf
	}
}


//-------------------------------------------------
//  op_divu - process a DIVU opcode
//-------------------------------------------------

void drcbe_x86::op_divu(Assembler &a, const instruction &inst)
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
		emit_mov_r32_p32(a, ecx, src2p);                                                // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		Label skip = a.newLabel();
		a.jecxz(skip);                                                                  // jecxz skip
		emit_mov_r32_p32(a, eax, src1p);                                                // mov   eax,src1p
		a.xor_(edx, edx);                                                               // xor   edx,edx
		a.div(ecx);                                                                     // div   ecx
		emit_mov_p32_r32(a, dstp, eax);                                                 // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(a, edstp, edx);                                            // mov   edstp,edx
		if (inst.flags() != 0)
			a.test(eax, eax);                                                           // test  eax,eax
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m64_p64(a, qword_ptr(esp, 16), src2p);                                 // mov   [esp+16],src2p
		emit_mov_m64_p64(a, qword_ptr(esp, 8), src1p);                                  // mov   [esp+8],src1p
		if (!compute_rem)
			a.mov(dword_ptr(esp, 4), imm(&m_reslo));                                    // mov   [esp+4],&reslo
		else
			a.mov(dword_ptr(esp, 4), imm(&m_reshi));                                    // push  [esp+4],&reshi
		a.mov(dword_ptr(esp, 0), imm(&m_reslo));                                        // mov   [esp],&reslo
		a.call(imm(ddivu));                                                             // call  ddivu
		if (inst.flags() != 0)
			a.push(ptr(u64(flags_unmap), eax, 2));                                      // push   flags_unmap[eax*4]
		a.mov(eax, MABS((uint32_t *)&m_reslo + 0));                                     // mov   eax,reslo.lo
		a.mov(edx, MABS((uint32_t *)&m_reslo + 1));                                     // mov   edx,reslo.hi
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
		if (compute_rem)
		{
			a.mov(eax, MABS((uint32_t *)&m_reshi + 0));                                 // mov   eax,reshi.lo
			a.mov(edx, MABS((uint32_t *)&m_reshi + 1));                                 // mov   edx,reshi.hi
			emit_mov_p64_r64(a, edstp, eax, edx);                                       // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			a.popfd();                                                                  // popf
	}
}


//-------------------------------------------------
//  op_divs - process a DIVS opcode
//-------------------------------------------------

void drcbe_x86::op_divs(Assembler &a, const instruction &inst)
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
		emit_mov_r32_p32(a, ecx, src2p);                                                // mov   ecx,src2p
		if (inst.flags() != 0)
		{
			a.mov(eax, 0xa0000000);                                                     // mov   eax,0xa0000000
			a.add(eax, eax);                                                            // add   eax,eax
		}
		Label skip = a.newLabel();
		a.jecxz(skip);                                                                  // jecxz skip
		emit_mov_r32_p32(a, eax, src1p);                                                // mov   eax,src1p
		a.cdq();                                                                        // cdq
		a.idiv(ecx);                                                                    // idiv  ecx
		emit_mov_p32_r32(a, dstp, eax);                                                 // mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(a, edstp, edx);                                            // mov   edstp,edx
		if (inst.flags() != 0)
			a.test(eax, eax);                                                           // test  eax,eax
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_m64_p64(a, qword_ptr(esp, 16), src2p);                                 // mov   [esp+16],src2p
		emit_mov_m64_p64(a, qword_ptr(esp, 8), src1p);                                  // mov   [esp+8],src1p
		if (!compute_rem)
			a.mov(dword_ptr(esp, 4), imm(&m_reslo));                                    // mov   [esp+4],&reslo
		else
			a.mov(dword_ptr(esp, 4), imm(&m_reshi));                                    // push  [esp+4],&reshi
		a.mov(dword_ptr(esp, 0), imm(&m_reslo));                                        // mov   [esp],&reslo
		a.call(imm(ddivs));                                                             // call  ddivs
		if (inst.flags() != 0)
			a.push(ptr(u64(flags_unmap), eax, 2));                                      // push   flags_unmap[eax*4]
		a.mov(eax, MABS((uint32_t *)&m_reslo + 0));                                     // mov   eax,reslo.lo
		a.mov(edx, MABS((uint32_t *)&m_reslo + 1));                                     // mov   edx,reslo.hi
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
		if (compute_rem)
		{
			a.mov(eax, MABS((uint32_t *)&m_reshi + 0));                                 // mov   eax,reshi.lo
			a.mov(edx, MABS((uint32_t *)&m_reshi + 1));                                 // mov   edx,reshi.hi
			emit_mov_p64_r64(a, edstp, eax, edx);                                       // mov   edstp,edx:eax
		}
		if (inst.flags() != 0)
			a.popfd();                                                                  // popf
	}
}


//-------------------------------------------------
//  op_and - process a AND opcode
//-------------------------------------------------

void drcbe_x86::op_and(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdAnd, MABS(dstp.memory(), 4), src2p,                // and   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && !src.immediate())
					{
						a.mov(dst.as<Mem>(), imm(0));
						return true;
					}
					else if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
						return true;

					return false;
				});

		// dstp == src2p in memory
		else if (dstp.is_memory() && dstp == src2p)
			alu_op_param(a, Inst::kIdAnd, MABS(dstp.memory(), 4), src1p,                // and   [dstp],src1p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && !src.immediate())
					{
						a.mov(dst.as<Mem>(), imm(0));
						return true;
					}
					else if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
						return true;

					return false;
				});

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && !inst.flags())
		{
			if (src1p.is_int_register())
			{
				if (src1p.ireg() & 4)
				{
					if (dstreg.id() != src1p.ireg())
						a.mov(dstreg, Gpd(src1p.ireg()));                               // mov   dstreg,src1p
					a.and_(dstreg, 0xff);                                               // and   dstreg,0xff
				}
				else
					a.movzx(dstreg, GpbLo(src1p.ireg()));                               // movzx dstreg,src1p
			}
			else if (src1p.is_memory())
				a.movzx(dstreg, MABS(src1p.memory(), 1));                               // movzx dstreg,[src1p]
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && !inst.flags())
		{
			if (src1p.is_int_register())
				a.movzx(dstreg, Gpw(src1p.ireg()));                                     // movzx dstreg,src1p
			else if (src1p.is_memory())
				a.movzx(dstreg, MABS(src1p.memory(), 2));                               // movzx dstreg,[src1p]
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdAnd, dstreg, src2p,                                // and   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && !src.immediate())
					{
						a.xor_(dst.as<Gpd>(), dst.as<Gpd>());
						return true;
					}
					else if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
						return true;

					return false;
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_and_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),       // and   [dstp],src2p
				src2p, inst);

		// dstp == src2p in memory
		else if (dstp.is_memory() && dstp == src2p)
			emit_and_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),       // and   [dstp],src1p
				src1p, inst);

		// AND with immediate 0xff
		else if (src2p.is_immediate_value(0xff) && !inst.flags())
		{
			if (src1p.is_int_register())
			{
				if (src1p.ireg() & 4)
				{
					if (dstreg.id() != src1p.ireg())
						a.mov(dstreg, Gpd(src1p.ireg()));                               // mov   dstreg,src1p
					a.and_(dstreg, 0xff);                                               // and   dstreg,0xff
				}
				else
					a.movzx(dstreg, GpbLo(src1p.ireg()));                               // movzx dstreg,src1p
			}
			else if (src1p.is_memory())
				a.movzx(dstreg, MABS(src1p.memory(), 1));                               // movzx dstreg,[src1p]
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
			if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   dsthi,0
			else if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   dsthi,0
		}

		// AND with immediate 0xffff
		else if (src2p.is_immediate_value(0xffff) && !inst.flags())
		{
			if (src1p.is_int_register())
				a.movzx(dstreg, Gpw(src1p.ireg()));                                     // movzx dstreg,src1p
			else if (src1p.is_memory())
				a.movzx(dstreg, MABS(src1p.memory(), 2));                               // movzx dstreg,[src1p]
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
			if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   dsthi,0
			else if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   dsthi,0
		}

		// AND with immediate 0xffffffff
		else if (src2p.is_immediate_value(0xffffffffU) && !inst.flags())
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
			if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   dsthi,0
			else if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   dsthi,0
		}

		// AND with immediate 0xffffffff00000000
		else if (src2p.is_immediate_value(0xffffffff00000000ULL) && !inst.flags())
		{
			if (src1p != dstp)
			{
				emit_mov_r64_p64(a, Gp(), edx, src1p);                                  // mov   dstreg,src1p
				emit_mov_p64_r64(a, dstp, Gp(), edx);                                   // mov   dstp,dstreg
			}
			if (dstp.is_int_register())
				a.xor_(Gpd(dstp.ireg()), Gpd(dstp.ireg()));                             // xor   dstlo,dstlo
			else if (dstp.is_memory())
				a.mov(MABS(dstp.memory(0), 4), 0);                                      // mov   dstlo,0
		}

		// AND with immediate <= 0xffffffff
		else if (src2p.is_immediate() && src2p.immediate() <= 0xffffffffU && !inst.flags())
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdAnd, dstreg, src2p,                                // and   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && !src.immediate())
					{
						a.xor_(dst.as<Gpd>(), dst.as<Gpd>());
						return true;
					}
					else if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
						return true;

					return false;
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
			if (dstp.is_int_register())
				a.mov(MABS(m_reghi[dstp.ireg()], 4), 0);                                // mov   dsthi,0
			else if (dstp.is_memory())
				a.mov(MABS(dstp.memory(4), 4), 0);                                      // mov   dsthi,0
		}

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, src1p);                                    // mov   edx:dstreg,[src1p]
			emit_and_r64_p64(a, dstreg, edx, src2p, inst);                              // and   edx:dstreg,src2p
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_test - process a TEST opcode
//-------------------------------------------------

void drcbe_x86::op_test(Assembler &a, const instruction &inst)
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
	Gp const src1reg = src1p.select_register(eax);

	// 32-bit form
	if (inst.size() == 4)
	{
		// src1p in memory
		if (src1p.is_memory())
			alu_op_param(a, Inst::kIdTest, MABS(src1p.memory(), 4), src2p);             // test  [src1p],src2p

		// general case
		else
		{
			emit_mov_r32_p32(a, src1reg, src1p);                                        // mov   src1reg,src1p
			alu_op_param(a, Inst::kIdTest, src1reg, src2p);                             // test  src1reg,src2p
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// src1p in memory
		if (src1p.is_memory())
			alu_op_param(a, Inst::kIdTest, Inst::kIdTest,                               // test  [dstp],src2p
				MABS(src1p.memory(0), 4), MABS(src1p.memory(4), 4), src2p, inst.flags() & FLAG_Z);

		// general case
		else
		{
			emit_mov_r64_p64(a, src1reg, edx, src1p);                                   // mov   src1reg:dstp,[src1p]
			alu_op_param(a, Inst::kIdTest, Inst::kIdTest,                               // test  src1reg:dstp,src2p
				src1reg, edx, src2p, inst.flags() & FLAG_Z);
		}
	}
}


//-------------------------------------------------
//  op_or - process a OR opcode
//-------------------------------------------------

void drcbe_x86::op_or(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdOr, MABS(dstp.memory(), 4), src2p,                 // or    [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
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
			alu_op_param(a, Inst::kIdOr, MABS(dstp.memory(), 4), src1p,                 // or    [dstp],src1p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
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
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdOr, dstreg, src2p,                                 // or    dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
					{
						a.mov(dst.as<Gp>(), imm(-1));
						return true;
					}
					else if (!inst.flags() && !src.immediate())
						return true;

					return false;
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_or_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),        // or    [dstp],src2p
				src2p, inst);

		// dstp == src2p in memory
		else if (dstp.is_memory() && dstp == src2p)
			emit_or_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),        // or    [dstp],src1p
				src1p, inst);

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, src1p);                                    // mov   edx:dstreg,[src1p]
			emit_or_r64_p64(a, dstreg, edx, src2p, inst);                               // or    edx:dstreg,src2p
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_xor - process a XOR opcode
//-------------------------------------------------

void drcbe_x86::op_xor(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			alu_op_param(a, Inst::kIdXor, MABS(dstp.memory(), 4), src2p,                // xor   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
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
			alu_op_param(a, Inst::kIdXor, MABS(dstp.memory(), 4), src1p,                // xor   [dstp],src1p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
					{
						a.not_(dst.as<Mem>());
						return true;
					}
					else if (!inst.flags() && !src.immediate())
						return true;

					return false;
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			alu_op_param(a, Inst::kIdXor, dstreg, src2p,                                // xor   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize all-zero and all-one cases
					if (!inst.flags() && u32(src.immediate()) == 0xffffffffU)
					{
						a.not_(dst.as<Gp>());
						return true;
					}
					else if (!inst.flags() && !src.immediate())
						return true;

					return false;
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			emit_xor_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),       // xor   [dstp],src2p
				src2p, inst);

		// dstp == src1p in memory
		else if (dstp.is_memory() && dstp == src2p)
			emit_xor_m64_p64(a, MABS(dstp.memory(0), 4), MABS(dstp.memory(4), 4),       // xor   [dstp],src1p
				src1p, inst);

		// general case
		else
		{
			emit_mov_r64_p64(a, dstreg, edx, src1p);                                    // mov   edx:dstreg,[src1p]
			emit_xor_r64_p64(a, dstreg, edx, src2p, inst);                              // xor   edx:dstreg,src2p
			emit_mov_p64_r64(a, dstp, dstreg, edx);                                     // mov   dstp,edx:dstreg
		}
	}
}


//-------------------------------------------------
//  op_lzcnt - process a LZCNT opcode
//-------------------------------------------------

void drcbe_x86::op_lzcnt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(a, dstreg, srcp);                                              // mov   dstreg,src1p
		a.mov(ecx, 32 ^ 31);                                                            // mov   ecx,32 ^ 31
		a.bsr(dstreg, dstreg);                                                          // bsr   dstreg,dstreg
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		a.xor_(dstreg, 31);                                                             // xor   dstreg,31
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(a, edx, dstreg, srcp);                                         // mov   dstreg:edx,srcp
		a.bsr(dstreg, dstreg);                                                          // bsr   dstreg,dstreg
		Label skip = a.newLabel();
		a.jnz(skip);                                                                    // jnz   skip
		a.mov(ecx, 32 ^ 31);                                                            // mov   ecx,32 ^ 31
		a.bsr(dstreg, edx);                                                             // bsr   dstreg,edx
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		a.add(ecx, 32);                                                                 // add   ecx,32
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
		a.xor_(edx, edx);                                                               // xor   edx,edx
		a.xor_(dstreg, 31);                                                             // xor   dstreg,31
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_tzcnt - process a TZCNT opcode
//-------------------------------------------------

void drcbe_x86::op_tzcnt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	Gp const dstreg = dstp.select_register(eax);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(a, dstreg, srcp);                                              // mov   dstreg,src1p
		a.mov(ecx, 32);                                                                 // mov   ecx,32
		a.bsf(dstreg, dstreg);                                                          // bsf   dstreg,dstreg
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Label skip = a.newLabel();
		emit_mov_r64_p64(a, edx, dstreg, srcp);                                         // mov   dstreg:edx,srcp
		a.bsf(dstreg, dstreg);                                                          // bsf   dstreg,dstreg
		a.jz(skip);                                                                     // jnz   skip
		a.mov(ecx, 32);                                                                 // mov   ecx,32
		a.bsf(dstreg, edx);                                                             // bsf   dstreg,edx
		a.cmovz(dstreg, ecx);                                                           // cmovz dstreg,ecx
		a.add(dstreg, 32);                                                              // add   dstreg,32
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
		a.xor_(edx, edx);                                                               // xor   edx,edx
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_bswap - process a BSWAP opcode
//-------------------------------------------------

void drcbe_x86::op_bswap(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// pick a target register for the general case
	Gp const dstreg = dstp.select_register(eax);

	// 32-bit form
	if (inst.size() == 4)
	{
		emit_mov_r32_p32(a, dstreg, srcp);                                              // mov   dstreg,src1p
		a.bswap(dstreg);                                                                // bswap dstreg
		if (inst.flags() != 0)
			a.test(dstreg, dstreg);                                                     // test  dstreg,dstreg
		emit_mov_p32_r32(a, dstp, dstreg);                                              // mov   dstp,dstreg
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		emit_mov_r64_p64(a, edx, dstreg, srcp);                                         // mov   dstreg:edx,srcp
		a.bswap(dstreg);                                                                // bswap dstreg
		a.bswap(edx);                                                                   // bswap edx
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
		if (inst.flags() == FLAG_Z)
			a.or_(edx, dstreg);                                                         // or    edx,eax
		else if (inst.flags() == FLAG_S)
			a.test(edx, edx);                                                           // test  edx,edx
		else
		{
			a.movzx(ecx, dstreg.r16());                                                 // movzx ecx,dstreg
			a.or_(edx, ecx);                                                            // or    edx,ecx
			a.mov(ecx, dstreg);                                                         // mov   ecx,dstreg
			a.shr(ecx, 16);                                                             // shr   ecx,16
			a.or_(edx, ecx);                                                            // or    edx,ecx
		}
	}
}


//-------------------------------------------------
//  op_shl - process a SHL opcode
//-------------------------------------------------

void drcbe_x86::op_shl(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdShl, MABS(dstp.memory(), 4), src2p,              // shl   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdShl, dstreg, src2p,                              // shl   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(a, dstreg, edx, src1p);                                        // mov   edx:dstreg,[src1p]
		emit_shl_r64_p64(a, dstreg, edx, src2p, inst);                                  // shl   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_shr - process a SHR opcode
//-------------------------------------------------

void drcbe_x86::op_shr(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdShr, MABS(dstp.memory(), 4), src2p,              // shr   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdShr, dstreg, src2p,                              // shr   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(a, dstreg, edx, src1p);                                        // mov   edx:dstreg,[src1p]
		emit_shr_r64_p64(a, dstreg, edx, src2p, inst);                                  // shr   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_sar - process a SAR opcode
//-------------------------------------------------

void drcbe_x86::op_sar(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdSar, MABS(dstp.memory(), 4), src2p,              // sar   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdSar, dstreg, src2p,                              // sar   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(a, dstreg, edx, src1p);                                        // mov   edx:dstreg,[src1p]
		emit_sar_r64_p64(a, dstreg, edx, src2p, inst);                                  // sar   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_rol - process a rol opcode
//-------------------------------------------------

void drcbe_x86::op_rol(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdRol, MABS(dstp.memory(), 4), src2p,              // rol   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdRol, dstreg, src2p,                              // rol   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(a, dstreg, edx, src1p);                                        // mov   edx:dstreg,[src1p]
		emit_rol_r64_p64(a, dstreg, edx, src2p, inst);                                  // rol   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_ror - process a ROR opcode
//-------------------------------------------------

void drcbe_x86::op_ror(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdRor, MABS(dstp.memory(), 4), src2p,              // ror   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32(a, dstreg, src1p);                                         // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdRor, dstreg, src2p,                              // rol   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64(a, dstreg, edx, src1p);                                        // mov   edx:dstreg,[src1p]
		emit_ror_r64_p64(a, dstreg, edx, src2p, inst);                                  // ror   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_rolc - process a ROLC opcode
//-------------------------------------------------

void drcbe_x86::op_rolc(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdRcl, MABS(dstp.memory(), 4), src2p,              // rcl   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(a, dstreg, src1p);                               // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdRcl, dstreg, src2p,                              // rcl   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64_keepflags(a, dstreg, edx, src1p);                              // mov   edx:dstreg,[src1p]
		emit_rcl_r64_p64(a, dstreg, edx, src2p, inst);                                  // rcl   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}


//-------------------------------------------------
//  op_rorc - process a RORC opcode
//-------------------------------------------------

void drcbe_x86::op_rorc(Assembler &a, const instruction &inst)
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
	Gp const dstreg = dstp.select_register(eax, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		// dstp == src1p in memory
		if (dstp.is_memory() && dstp == src1p)
			shift_op_param(a, Inst::kIdRcr, MABS(dstp.memory(), 4), src2p,              // rcr   [dstp],src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});

		// general case
		else
		{
			emit_mov_r32_p32_keepflags(a, dstreg, src1p);                               // mov   dstreg,src1p
			shift_op_param(a, Inst::kIdRcr, dstreg, src2p,                              // rcr   dstreg,src2p
				[inst](Assembler &a, Operand const &dst, be_parameter const &src)
				{
					// optimize zero case
					return (!inst.flags() && !src.immediate());
				});
			emit_mov_p32_r32(a, dstp, dstreg);                                          // mov   dstp,dstreg
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		emit_mov_r64_p64_keepflags(a, dstreg, edx, src1p);                              // mov   edx:dstreg,[src1p]
		emit_rcr_r64_p64(a, dstreg, edx, src2p, inst);                                  // rcr   edx:dstreg,src2p
		emit_mov_p64_r64(a, dstp, dstreg, edx);                                         // mov   dstp,edx:dstreg
	}
}



//**************************************************************************
//  FLOATING POINT OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  op_fload - process a FLOAD opcode
//-------------------------------------------------

void drcbe_x86::op_fload(Assembler &a, const instruction &inst)
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
		a.mov(eax, MABS(basep.memory(4*indp.immediate())));                             // mov   eax,[basep + 4*indp]
		if (inst.size() == 8)
			a.mov(edx, MABS(basep.memory(4 + 4*indp.immediate())));                     // mov   edx,[basep + 4*indp + 4]
	}

	// other index
	else
	{
		Gp const indreg = indp.select_register(ecx);
		emit_mov_r32_p32(a, indreg, indp);
		a.mov(eax, ptr(u64(basep.memory(0)), indreg, 2));                               // mov   eax,[basep + 4*indp]
		if (inst.size() == 8)
			a.mov(edx, ptr(u64(basep.memory(4)), indreg, 2));                           // mov   edx,[basep + 4*indp + 4]
	}

	// general case
	a.mov(MABS(dstp.memory(0)), eax);                                                   // mov   [dstp],eax
	if (inst.size() == 8)
		a.mov(MABS(dstp.memory(4)), edx);                                               // mov   [dstp + 4],edx
}


//-------------------------------------------------
//  op_fstore - process a FSTORE opcode
//-------------------------------------------------

void drcbe_x86::op_fstore(Assembler &a, const instruction &inst)
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
	a.mov(eax, MABS(srcp.memory(0)));                                                   // mov   eax,[srcp]
	if (inst.size() == 8)
		a.mov(edx, MABS(srcp.memory(4)));                                               // mov   edx,[srcp + 4]

	// immediate index
	if (indp.is_immediate())
	{
		a.mov(MABS(basep.memory(4*indp.immediate())), eax);                             // mov   [basep + 4*indp],eax
		if (inst.size() == 8)
			a.mov(MABS(basep.memory(4 + 4*indp.immediate())), edx);                     // mov   [basep + 4*indp + 4],edx
	}

	// other index
	else
	{
		Gp const indreg = indp.select_register(ecx);
		emit_mov_r32_p32(a, indreg, indp);
		a.mov(ptr(u64(basep.memory(0)), indreg, 2), eax);                               // mov   [basep + 4*indp],eax
		if (inst.size() == 8)
			a.mov(ptr(u64(basep.memory(4)), indreg, 2), edx);                           // mov   [basep + 4*indp + 4],edx
	}
}


//-------------------------------------------------
//  op_fread - process a FREAD opcode
//-------------------------------------------------

void drcbe_x86::op_fread(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	parameter const &spacep = inst.param(2);
	assert(spacep.is_size_space());
	assert((1 << spacep.size()) == inst.size());

	// set up a call to the read dword/qword handler
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacep.space()]));                             // mov    [esp],space
	if (inst.size() == 4)
		a.call(imm(m_accessors[spacep.space()].read_dword));                            // call   read_dword
	else if (inst.size() == 8)
		a.call(imm(m_accessors[spacep.space()].read_qword));                            // call   read_qword

	// store result
	if (inst.size() == 4)
		emit_mov_p32_r32(a, dstp, eax);                                                 // mov   dstp,eax
	else if (inst.size() == 8)
		emit_mov_p64_r64(a, dstp, eax, edx);                                            // mov   dstp,edx:eax
}


//-------------------------------------------------
//  op_fwrite - process a FWRITE opcode
//-------------------------------------------------

void drcbe_x86::op_fwrite(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	parameter const &spacep = inst.param(2);
	assert(spacep.is_size_space());
	assert((1 << spacep.size()) == inst.size());

	// set up a call to the write dword/qword handler
	if (inst.size() == 4)
		emit_mov_m32_p32(a, dword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	else if (inst.size() == 8)
		emit_mov_m64_p64(a, qword_ptr(esp, 8), srcp);                                   // mov    [esp+8],srcp
	emit_mov_m32_p32(a, dword_ptr(esp, 4), addrp);                                      // mov    [esp+4],addrp
	a.mov(dword_ptr(esp, 0), imm(m_space[spacep.space()]));                             // mov    [esp],space
	if (inst.size() == 4)
		a.call(imm(m_accessors[spacep.space()].write_dword));                           // call   write_dword
	else if (inst.size() == 8)
		a.call(imm(m_accessors[spacep.space()].write_qword));                           // call   write_qword
}


//-------------------------------------------------
//  op_fmov - process a FMOV opcode
//-------------------------------------------------

void drcbe_x86::op_fmov(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// always start with a jmp
	Label skip = a.newLabel();
	if (inst.condition() != uml::COND_ALWAYS)
		a.j(X86_NOT_CONDITION(inst.condition()), skip);                                 // jcc   skip

	// general case
	a.mov(eax, MABS(srcp.memory(0)));                                                   // mov   eax,[srcp]
	if (inst.size() == 8)
		a.mov(edx, MABS(srcp.memory(4)));                                               // mov   edx,[srcp + 4]
	a.mov(MABS(dstp.memory(0)), eax);                                                   // mov   [dstp],eax
	if (inst.size() == 8)
		a.mov(MABS(dstp.memory(4)), edx);                                               // mov   [dstp + 4],edx

	// resolve the jump
	if (inst.condition() != uml::COND_ALWAYS)
	{
		a.bind(skip);                                                               // skip:
		reset_last_upper_lower_reg();
	}
}


//-------------------------------------------------
//  op_ftoint - process a FTOINT opcode
//-------------------------------------------------

void drcbe_x86::op_ftoint(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	parameter const &sizep = inst.param(2);
	assert(sizep.is_size());
	parameter const &roundp = inst.param(3);
	assert(roundp.is_rounding());

	// set rounding mode if necessary
	if (roundp.rounding() != ROUND_DEFAULT && (!m_sse3 || roundp.rounding() != ROUND_TRUNC))
	{
		a.fstcw(MABS(&m_fmodesave));                                                    // fstcw [fmodesave]
		a.fldcw(MABS(&fp_control[roundp.rounding()]));                                  // fldcw fpcontrol[roundp]
	}

	// general case
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp

	// 4-byte integer case
	if (sizep.size() == SIZE_DWORD)
	{
		if (dstp.is_memory())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				a.fistp(MABS(dstp.memory(), 4));                                        // fistp [dstp]
			else
				a.fisttp(MABS(dstp.memory(), 4));                                       // fisttp [dstp]
		}
		else if (dstp.is_int_register())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				a.fistp(MABS(m_reglo[dstp.ireg()], 4));                                 // fistp reglo[dstp]
			else
				a.fisttp(MABS(m_reglo[dstp.ireg()], 4));                                // fisttp reglo[dstp]
			a.mov(Gpd(dstp.ireg()), MABS(m_reglo[dstp.ireg()]));                        // mov   dstp,reglo[dstp]
		}
	}

	// 8-byte integer case
	else if (sizep.size() == SIZE_QWORD)
	{
		if (dstp.is_memory())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				a.fistp(MABS(dstp.memory(), 8));                                        // fistp [dstp]
			else
				a.fisttp(MABS(dstp.memory(), 8));                                       // fisttp [dstp]
		}
		else if (dstp.is_int_register())
		{
			if (!m_sse3 || roundp.rounding() != ROUND_TRUNC)
				a.fistp(MABS(m_reglo[dstp.ireg()], 8));                                 // fistp reglo[dstp]
			else
				a.fisttp(MABS(m_reglo[dstp.ireg()], 8));                                // fisttp reglo[dstp]
			a.mov(Gpd(dstp.ireg()), MABS(m_reglo[dstp.ireg()]));                        // mov   dstp,reglo[dstp]
		}
	}

	// restore control word and proceed
	if (roundp.rounding() != ROUND_DEFAULT && (!m_sse3 || roundp.rounding() != ROUND_TRUNC))
		a.fldcw(MABS(&m_fmodesave));                                                    // fldcw [fmodesave]
}


//-------------------------------------------------
//  op_ffrint - process a FFRINT opcode
//-------------------------------------------------

void drcbe_x86::op_ffrint(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	parameter const &sizep = inst.param(2);
	assert(sizep.is_size());

	// 4-byte integer case
	if (sizep.size() == SIZE_DWORD)
	{
		if (srcp.is_immediate())
		{
			a.mov(MABS(&m_fptemp, 4), srcp.immediate());                                // mov   [fptemp],srcp
			a.fild(MABS(&m_fptemp, 4));                                                 // fild  [fptemp]
		}
		else if (srcp.is_memory())
			a.fild(MABS(srcp.memory(), 4));                                             // fild  [srcp]
		else if (srcp.is_int_register())
		{
			a.mov(MABS(m_reglo[srcp.ireg()]), Gpd(srcp.ireg()));                        // mov   reglo[srcp],srcp
			a.fild(MABS(m_reglo[srcp.ireg()], 4));                                      // fild  reglo[srcp]
		}
	}

	// 8-bit integer case
	else if (sizep.size() == SIZE_QWORD)
	{
		if (srcp.is_immediate())
		{
			a.mov(MABS(&m_fptemp, 4), srcp.immediate());                                // mov   [fptemp],srcp
			a.mov(MABS((uint8_t *)&m_fptemp + 4, 4), srcp.immediate());                 // mov   [fptemp+4],srcp
			a.fild(MABS(&m_fptemp, 8));                                                 // fild  [fptemp]
		}
		else if (srcp.is_memory())
			a.fild(MABS(srcp.memory(), 8));                                             // fild  [srcp]
		else if (srcp.is_int_register())
		{
			a.mov(MABS(m_reglo[srcp.ireg()]), Gpd(srcp.ireg()));                        // mov   reglo[srcp],srcp
			a.fild(MABS(m_reglo[srcp.ireg()], 8));                                      // fild  reglo[srcp]
		}
	}

	// store the result and be done
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  [dstp]
}


//-------------------------------------------------
//  op_ffrflt - process a FFRFLT opcode
//-------------------------------------------------

void drcbe_x86::op_ffrflt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	parameter const &sizep = inst.param(2);
	assert(sizep.is_size());

	// general case
	if (sizep.size() == SIZE_DWORD)
		a.fld(MABS(srcp.memory(), 4));                                                  // fld   [srcp]
	else if (sizep.size() == SIZE_QWORD)
		a.fld(MABS(srcp.memory(), 8));                                                  // fld   [srcp]
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_frnds - process a FRNDS opcode
//-------------------------------------------------

void drcbe_x86::op_frnds(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fstp(MABS(&m_fptemp, 4));                                                         // fstp  [fptemp]
	a.fld(MABS(&m_fptemp, 4));                                                          // fld   [fptemp]
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  [dstp]
}


//-------------------------------------------------
//  op_fadd - process a FADD opcode
//-------------------------------------------------

void drcbe_x86::op_fadd(Assembler &a, const instruction &inst)
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
	emit_fld_p(a, inst.size(), src1p);                                                  // fld   src1p
	emit_fld_p(a, inst.size(), src2p);                                                  // fld   src2p
	a.faddp();                                                                          // faddp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fsub - process a FSUB opcode
//-------------------------------------------------

void drcbe_x86::op_fsub(Assembler &a, const instruction &inst)
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
	emit_fld_p(a, inst.size(), src1p);                                                  // fld   src1p
	emit_fld_p(a, inst.size(), src2p);                                                  // fld   src2p
	a.fsubp();                                                                          // fsubp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fcmp - process a FCMP opcode
//-------------------------------------------------

void drcbe_x86::op_fcmp(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_U);

	// normalize parameters
	be_parameter src1p(*this, inst.param(0), PTYPE_MF);
	be_parameter src2p(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(a, inst.size(), src2p);                                                  // fld   src2p
	emit_fld_p(a, inst.size(), src1p);                                                  // fld   src1p
	a.fcompp();                                                                         // fcompp
	a.fnstsw(ax);                                                                       // fnstsw ax
	a.sahf();                                                                           // sahf
}


//-------------------------------------------------
//  op_fmul - process a FMUL opcode
//-------------------------------------------------

void drcbe_x86::op_fmul(Assembler &a, const instruction &inst)
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
	emit_fld_p(a, inst.size(), src1p);                                                  // fld   src1p
	emit_fld_p(a, inst.size(), src2p);                                                  // fld   src2p
	a.fmulp();                                                                          // fmulp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fdiv - process a FDIV opcode
//-------------------------------------------------

void drcbe_x86::op_fdiv(Assembler &a, const instruction &inst)
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
	emit_fld_p(a, inst.size(), src1p);                                                  // fld   src1p
	emit_fld_p(a, inst.size(), src2p);                                                  // fld   src2p
	a.fdivp();                                                                          // fdivp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fneg - process a FNEG opcode
//-------------------------------------------------

void drcbe_x86::op_fneg(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fchs();                                                                           // fchs
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fabs - process a FABS opcode
//-------------------------------------------------

void drcbe_x86::op_fabs(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fabs();                                                                           // fabs
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fsqrt - process a FSQRT opcode
//-------------------------------------------------

void drcbe_x86::op_fsqrt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fsqrt();                                                                          // fsqrt
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_frecip - process a FRECIP opcode
//-------------------------------------------------

void drcbe_x86::op_frecip(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	a.fld1();                                                                           // fld1
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fdivp();                                                                          // fdivp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_frsqrt - process a FRSQRT opcode
//-------------------------------------------------

void drcbe_x86::op_frsqrt(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// general case
	a.fld1();                                                                           // fld1
	emit_fld_p(a, inst.size(), srcp);                                                   // fld   srcp
	a.fsqrt();                                                                          // fsqrt
	a.fdivp();                                                                          // fdivp
	emit_fstp_p(a, inst.size(), dstp);                                                  // fstp  dstp
}


//-------------------------------------------------
//  op_fcopyi - process a FCOPYI opcode
//-------------------------------------------------

void drcbe_x86::op_fcopyi(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MR);

	// 32-bit case
	if (inst.size() == 4)
	{
		if (srcp.is_memory())
		{
			a.mov(eax, MABS(srcp.memory()));                                            // mov eax,[srcp]
			a.mov(MABS(dstp.memory()), eax);                                            // mov [dstp],eax
		}
		else if (srcp.is_int_register())
		{
			a.mov(MABS(dstp.memory()), Gpd(srcp.ireg()));                               // mov [dstp],srcp
		}
	}

	// 64-bit case
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			a.mov(eax, MABS(srcp.memory(0)));                                           // mov eax,[srcp]
			a.mov(edx, MABS(srcp.memory(4)));                                           // mov edx,[srcp+4]
		}
		else if (srcp.is_int_register())
		{
			a.mov(edx, MABS(m_reghi[srcp.ireg()]));                                     // mov edx,[reghi[srcp]]
			a.mov(eax, Gpd(srcp.ireg()));                                               // mov eax,srcp
		}

		a.mov(MABS(dstp.memory(0)), eax);                                               // mov [dstp],eax
		a.mov(MABS(dstp.memory(4)), edx);                                               // mov [dstp+4],edx
	}
}


//-------------------------------------------------
//  op_icopyf - process a ICOPYF opcode
//-------------------------------------------------

void drcbe_x86::op_icopyf(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// 32-bit case
	if (inst.size() == 4)
	{
		a.mov(eax, MABS(srcp.memory()));                                                // mov eax,[srcp]

		if (dstp.is_memory())
		{
			a.mov(MABS(dstp.memory()), eax);                                            // mov [dstp],eax
		}
		else if (dstp.is_int_register())
		{
			a.mov(Gpd(dstp.ireg()), eax);                                               // mov dstp,eax
		}
	}

	// 64-bit case
	else if (inst.size() == 8)
	{
		a.mov(eax, MABS(srcp.memory(0)));                                               // mov eax,[srcp]
		a.mov(edx, MABS(srcp.memory(4)));                                               // mov edx,[srcp+4]

		if (dstp.is_memory())
		{
			a.mov(MABS(dstp.memory(0)), eax);                                           // mov [dstp],eax
			a.mov(MABS(dstp.memory(4)), edx);                                           // mov [dstp+4],edx
		}
		else
		{
			a.mov(MABS(m_reghi[dstp.ireg()]), edx);                                     // mov [reghi[dstp]],edx
			a.mov(Gpd(dstp.ireg()), eax);                                               // mov dstp,eax
		}
	}
}



//**************************************************************************
//  MISCELLAENOUS FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  dmulu - perform a double-wide unsigned multiply
//-------------------------------------------------

int drcbe_x86::dmulu(uint64_t &dstlo, uint64_t &dsthi, uint64_t src1, uint64_t src2, bool flags)
{
	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == false)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch source values
	uint64_t a = src1;
	uint64_t b = src2;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	uint64_t lo = (uint64_t)(uint32_t)(a >> 0)  * (uint64_t)(uint32_t)(b >> 0);
	uint64_t hi = (uint64_t)(uint32_t)(a >> 32) * (uint64_t)(uint32_t)(b >> 32);

	// compute middle parts
	uint64_t prevlo = lo;
	uint64_t temp = (uint64_t)(uint32_t)(a >> 32)  * (uint64_t)(uint32_t)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (uint64_t)(uint32_t)(a >> 0)  * (uint64_t)(uint32_t)(b >> 32);
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

int drcbe_x86::dmuls(uint64_t &dstlo, uint64_t &dsthi, int64_t src1, int64_t src2, bool flags)
{
	uint64_t lo, hi, prevlo;
	uint64_t a, b, temp;

	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == false)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch absolute source values
	a = src1; if ((int64_t)a < 0) a = -a;
	b = src2; if ((int64_t)b < 0) b = -b;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	lo = (uint64_t)(uint32_t)(a >> 0)  * (uint64_t)(uint32_t)(b >> 0);
	hi = (uint64_t)(uint32_t)(a >> 32) * (uint64_t)(uint32_t)(b >> 32);

	// compute middle parts
	prevlo = lo;
	temp = (uint64_t)(uint32_t)(a >> 32)  * (uint64_t)(uint32_t)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (uint64_t)(uint32_t)(a >> 0)  * (uint64_t)(uint32_t)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	// adjust for signage
	if ((int64_t)(src1 ^ src2) < 0)
	{
		hi = ~hi + (lo == 0);
		lo = ~lo + 1;
	}

	// store the results
	dsthi = hi;
	dstlo = lo;
	return ((hi >> 60) & FLAG_S) | ((dsthi != ((int64_t)lo >> 63)) << 1);
}


//-------------------------------------------------
//  ddivu - perform a double-wide unsigned divide
//-------------------------------------------------

int drcbe_x86::ddivu(uint64_t &dstlo, uint64_t &dsthi, uint64_t src1, uint64_t src2)
{
	// do nothing if src2 == 0
	if (src2 == 0)
		return FLAG_V;

	dstlo = src1 / src2;
	if (&dstlo != &dsthi)
		dsthi = src1 % src2;
	return ((dstlo == 0) << 2) | ((dstlo >> 60) & FLAG_S);
}


//-------------------------------------------------
//  ddivs - perform a double-wide signed divide
//-------------------------------------------------

int drcbe_x86::ddivs(uint64_t &dstlo, uint64_t &dsthi, int64_t src1, int64_t src2)
{
	// do nothing if src2 == 0
	if (src2 == 0)
		return FLAG_V;

	dstlo = src1 / src2;
	if (&dstlo != &dsthi)
		dsthi = src1 % src2;
	return ((dstlo == 0) << 2) | ((dstlo >> 60) & FLAG_S);
}

} // namespace drc
