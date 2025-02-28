// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex64.cpp

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

    Top-level generated code frame:
        [rsp+0x00]   - rcx home/scratch
        [rsp+0x08]   - rdx home/scratch
        [rsp+0x10]   - r8 home/scratch
        [rsp+0x18]   - r9 home/scratch
        [rsp+0x20]   - scratch
        [rsp+0x28]   - saved r15
        [rsp+0x30]   - saved r14
        [rsp+0x38]   - saved r13
        [rsp+0x40]   - saved r12
        [rsp+0x48]   - saved rbp
        [rsp+0x50]   - saved rdi
        [rsp+0x58]   - saved rsi
        [rsp+0x60]   - saved rbx
        [rsp+0x68]   - ret

    Generated code subroutine call frame:
        [rsp+0x00]   - rcx home/scratch
        [rsp+0x08]   - rdx home/scratch
        [rsp+0x10]   - r8 home/scratch
        [rsp+0x18]   - r9 home/scratch
        [rsp+0x20]   - scratch
        [rsp+0x28]   - ret
        ...
                     - rcx home/scratch
                     - rdx home/scratch
                     - r8 home/scratch
                     - r9 home/scratch
                     - scratch
                     - saved r15
                     - saved r14
                     - saved r13
                     - saved r12
                     - saved rdi
                     - saved rsi
                     - saved rbp
                     - saved rbx
                     - ret

***************************************************************************/

#include "emu.h"
#include "drcbex64.h"

#include "drcbeut.h"
#include "x86log.h"

#include "debug/debugcpu.h"
#include "emuopts.h"

#include "asmjit/src/asmjit/asmjit.h"

#include <cstddef>
#include <vector>


// This is a trick to make it build on Android where the ARM SDK declares ::REG_Rn
// and the x64 SDK declares ::REG_Exx and ::REG_Rxx
namespace drc {

namespace {

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

#ifdef _WIN32

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

// register mapping tables
const Gp::Id int_register_map[REG_I_COUNT] =
{
#ifdef _WIN32
	Gp::kIdBx, Gp::kIdSi, Gp::kIdDi, Gp::kIdR12, Gp::kIdR13, Gp::kIdR14, Gp::kIdR15,
#else
	Gp::kIdBx, Gp::kIdR12, Gp::kIdR13, Gp::kIdR14, Gp::kIdR15
#endif
};

uint32_t float_register_map[REG_F_COUNT] =
{
#ifdef _WIN32
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15
#else
	// on SysV x64 ABI, XMM0-7 are FP function args.  since this code has no args, and we
	// save/restore them around CALLC, they should be safe for our use.
	0, 1, 2, 3, 4, 5, 6, 7
#endif
};

// condition mapping table
const CondCode condition_map[uml::COND_MAX - uml::COND_Z] =
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
const uint8_t fprnd_map[4] =
{
	FPRND_CHOP,     // ROUND_TRUNC,   truncate
	FPRND_NEAR,     // ROUND_ROUND,   round
	FPRND_UP,       // ROUND_CEIL,    round up
	FPRND_DOWN      // ROUND_FLOOR    round down
};
#endif

// size-to-mask table
//const uint64_t size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, 0xffffffffffffffffU };



//**************************************************************************
//  MACROS
//**************************************************************************

#define X86_CONDITION(condition)        (condition_map[condition - uml::COND_Z])
#define X86_NOT_CONDITION(condition)    negateCond(condition_map[condition - uml::COND_Z])

#define assert_no_condition(inst)       assert((inst).condition() == uml::COND_ALWAYS)
#define assert_any_condition(inst)      assert((inst).condition() == uml::COND_ALWAYS || ((inst).condition() >= uml::COND_Z && (inst).condition() < uml::COND_MAX))
#define assert_no_flags(inst)           assert((inst).flags() == 0)
#define assert_flags(inst, valid)       assert(((inst).flags() & ~(valid)) == 0)



class ThrowableErrorHandler : public ErrorHandler
{
public:
	void handleError(Error err, const char *message, BaseEmitter *origin) override
	{
		throw emu_fatalerror("asmjit error %d: %s", err, message);
	}
};


inline bool is_nonvolatile_register(Gp reg)
{
	auto const r = reg.r64();
#ifdef _WIN32
	return (r == rbx) || (r == rsi) || (r == rdi) || (r == rbp) || (r == r12) || (r == r13) || (r == r14) || (r == r15);
#else
	return (r == rbx) || (r == rbp) || (r == r12) || (r == r13) || (r == r14) || (r == r15);
#endif
}



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class drcbe_x64 : public drcbe_interface
{
	using x86_entry_point_func = uint32_t (*)(uint8_t *rbpvalue, x86code *entry);

public:
	// construction/destruction
	drcbe_x64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_x64();

	// required overrides
	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst) override;
	virtual bool hash_exists(uint32_t mode, uint32_t pc) override;
	virtual void get_info(drcbe_info &info) override;
	virtual bool logging() const override { return m_log != nullptr; }

private:
	// a be_parameter is similar to a uml::parameter but maps to native registers/memory
	class be_parameter
	{
	public:
		// HACK: leftover from x86emit
		static inline constexpr int REG_MAX = 16;

		// parameter types
		enum be_parameter_type
		{
			PTYPE_NONE = 0,                     // invalid
			PTYPE_IMMEDIATE,                    // immediate; value = sign-extended to 64 bits
			PTYPE_INT_REGISTER,                 // integer register; value = 0-REG_MAX
			PTYPE_FLOAT_REGISTER,               // floating point register; value = 0-REG_MAX
			PTYPE_MEMORY,                       // memory; value = pointer to memory
			PTYPE_MAX
		};

		// represents the value of a parameter
		typedef uint64_t be_parameter_value;

		// construction
		be_parameter() : m_type(PTYPE_NONE), m_value(0) { }
		be_parameter(uint64_t val) : m_type(PTYPE_IMMEDIATE), m_value(val) { }
		be_parameter(drcbe_x64 &drcbe, const uml::parameter &param, uint32_t allowed);
		be_parameter(const be_parameter &param) = default;

		// creators for types that don't safely default
		static be_parameter make_ireg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_INT_REGISTER, regnum); }
		static be_parameter make_freg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static be_parameter make_memory(void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(base)); }
		static be_parameter make_memory(const void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(const_cast<void *>(base))); }

		// operators
		bool operator==(be_parameter const &rhs) const { return (m_type == rhs.m_type && m_value == rhs.m_value); }
		bool operator!=(be_parameter const &rhs) const { return (m_type != rhs.m_type || m_value != rhs.m_value); }

		// getters
		be_parameter_type type() const { return m_type; }
		uint64_t immediate() const { assert(m_type == PTYPE_IMMEDIATE); return m_value; }
		uint32_t ireg() const { assert(m_type == PTYPE_INT_REGISTER); assert(m_value < REG_MAX); return m_value; }
		uint32_t freg() const { assert(m_type == PTYPE_FLOAT_REGISTER); assert(m_value < REG_MAX); return m_value; }
		void *memory() const { assert(m_type == PTYPE_MEMORY); return reinterpret_cast<void *>(m_value); }

		// type queries
		bool is_immediate() const { return (m_type == PTYPE_IMMEDIATE); }
		bool is_int_register() const { return (m_type == PTYPE_INT_REGISTER); }
		bool is_float_register() const { return (m_type == PTYPE_FLOAT_REGISTER); }
		bool is_memory() const { return (m_type == PTYPE_MEMORY); }

		// other queries
		bool is_immediate_value(uint64_t value) const { return (m_type == PTYPE_IMMEDIATE && m_value == value); }

		// helpers
		asmjit::x86::Gp select_register(asmjit::x86::Gp defreg) const;
		asmjit::x86::Xmm select_register(asmjit::x86::Xmm defreg) const;
		asmjit::x86::Gp select_register(asmjit::x86::Gp defreg, be_parameter const &checkparam) const;
		asmjit::x86::Gp select_register(asmjit::x86::Gp defreg, be_parameter const &checkparam, be_parameter const &checkparam2) const;
		asmjit::x86::Xmm select_register(asmjit::x86::Xmm defreg, be_parameter const &checkparam) const;

	private:
		// private constructor
		be_parameter(be_parameter_type type, be_parameter_value value) : m_type(type), m_value(value) { }

		// internals
		be_parameter_type   m_type;             // parameter type
		be_parameter_value  m_value;            // parameter value
	};

	// helpers
	asmjit::x86::Mem MABS(const void *ptr, const uint32_t size = 0) const { return asmjit::x86::Mem(asmjit::x86::rbp, offset_from_rbp(ptr), size); }
	bool short_immediate(int64_t immediate) const { return (int32_t)immediate == immediate; }
	void normalize_commutative(be_parameter &inner, be_parameter &outer);
	int32_t offset_from_rbp(const void *ptr) const;
	asmjit::x86::Gp get_base_register_and_offset(asmjit::x86::Assembler &a, void *target, asmjit::x86::Gp const &reg, int32_t &offset);
	void smart_call_r64(asmjit::x86::Assembler &a, x86code *target, asmjit::x86::Gp const &reg);
	void smart_call_m64(asmjit::x86::Assembler &a, x86code **target);

	static void debug_log_hashjmp(offs_t pc, int mode);
	static void debug_log_hashjmp_fail();

	// code generators
	void op_handle(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_hash(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_label(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_comment(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mapvar(asmjit::x86::Assembler &a, const uml::instruction &inst);

	void op_nop(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_break(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_debug(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_exit(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_hashjmp(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_jmp(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_exh(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_callh(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_ret(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_callc(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_recover(asmjit::x86::Assembler &a, const uml::instruction &inst);

	void op_setfmod(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_getfmod(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_getexp(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_getflgs(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_setflgs(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_save(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_restore(asmjit::x86::Assembler &a, const uml::instruction &inst);

	void op_load(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_loads(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_store(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_read(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_readm(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_write(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_writem(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_carry(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_set(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mov(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_sext(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_roland(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_rolins(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_add(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_addc(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_sub(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_subc(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_cmp(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mulu(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mululw(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_muls(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mulslw(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_divu(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_divs(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_and(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_test(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_or(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_xor(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_lzcnt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_tzcnt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_bswap(asmjit::x86::Assembler &a, const uml::instruction &inst);
	template <asmjit::x86::Inst::Id Opcode> void op_shift(asmjit::x86::Assembler &a, const uml::instruction &inst);

	void op_fload(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fstore(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fread(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fwrite(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fmov(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_ftoint(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_ffrint(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_ffrflt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_frnds(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fadd(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fsub(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fcmp(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fmul(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fdiv(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fneg(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fabs(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fsqrt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_frecip(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_frsqrt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_fcopyi(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_icopyf(asmjit::x86::Assembler &a, const uml::instruction &inst);

	// alu and shift operation helpers
	static bool ones(u64 const value, unsigned const size) noexcept { return (size == 4) ? u32(value) == 0xffffffffU : value == 0xffffffff'ffffffffULL; }
	void alu_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode, asmjit::Operand const &dst, be_parameter const &param, std::function<bool(asmjit::x86::Assembler &a, asmjit::Operand const &dst, be_parameter const &src)> optimize = [](asmjit::x86::Assembler &a, asmjit::Operand dst, be_parameter const &src) { return false; });
	void shift_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode, size_t opsize, asmjit::Operand const &dst, be_parameter const &param, bool update_flags);

	// parameter helpers
	void mov_reg_param(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reg, be_parameter const &param, bool const keepflags = false);
	void mov_param_reg(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Gp const &reg);
	void mov_mem_param(asmjit::x86::Assembler &a, asmjit::x86::Mem const &memref, be_parameter const &param);

	// special-case move helpers
	void movsx_r64_p32(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reg, be_parameter const &param);
	void mov_r64_imm(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reg, uint64_t const imm);

	// floating-point helpers
	void movss_r128_p32(asmjit::x86::Assembler &a, asmjit::x86::Xmm const &reg, be_parameter const &param);
	void movss_p32_r128(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Xmm const &reg);
	void movsd_r128_p64(asmjit::x86::Assembler &a, asmjit::x86::Xmm const &reg, be_parameter const &param);
	void movsd_p64_r128(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Xmm const &reg);

	void calculate_status_flags(asmjit::x86::Assembler &a, uint32_t instsize, asmjit::Operand const &dst, u8 flags);
	void calculate_status_flags_mul(asmjit::x86::Assembler &a, uint32_t instsize, asmjit::x86::Gp const &lo, asmjit::x86::Gp const &hi);
	void calculate_status_flags_mul_low(asmjit::x86::Assembler &a, uint32_t instsize, asmjit::x86::Gp const &lo);

	size_t emit(asmjit::CodeHolder &ch);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	x86log_context *        m_log;                  // logging
	FILE *                  m_log_asmjit;

	uint32_t *              m_absmask32;            // absolute value mask (32-bit)
	uint64_t *              m_absmask64;            // absolute value mask (32-bit)
	uint8_t *               m_rbpvalue;             // value of RBP

	x86_entry_point_func    m_entry;                // entry point
	x86code *               m_exit;                 // exit point
	x86code *               m_nocode;               // nocode handler

	// state to live in the near cache
	struct near_state
	{
		x86code *           debug_log_hashjmp;      // hashjmp debugging
		x86code *           debug_log_hashjmp_fail; // hashjmp debugging

		uint32_t            ssemode;                // saved SSE mode
		uint32_t            ssemodesave;            // temporary location for saving
		uint32_t            ssecontrol[4];          // copy of the sse_control array
		float               single1;                // 1.0 in single-precision
		double              double1;                // 1.0 in double-precision

		void *              stacksave;              // saved stack pointer

		uint8_t             flagsmap[0x1000];       // flags map
		uint64_t            flagsunmap[0x20];       // flags unmapper
	};
	near_state &            m_near;

	// resolved memory handler functions
	struct memory_accessors
	{
		resolved_memory_accessors resolved;
		address_space::specific_access_info specific;
		offs_t address_mask;
		bool no_mask;
		bool has_high_bits;
		bool mask_high_bits;
	};
	resolved_member_function m_debug_cpu_instruction_hook;
	resolved_member_function m_drcmap_get_value;
	std::vector<memory_accessors> m_memory_accessors;

	// globals
	using opcode_generate_func = void (drcbe_x64::*)(asmjit::x86::Assembler &, const uml::instruction &);
	struct opcode_table_entry
	{
		uml::opcode_t           opcode;             // opcode in question
		opcode_generate_func    func;               // function pointer to the work
	};
	static const opcode_table_entry s_opcode_table_source[];
	static opcode_generate_func s_opcode_table[uml::OP_MAX];
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

drcbe_x64::opcode_generate_func drcbe_x64::s_opcode_table[OP_MAX];



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
	{ uml::OP_BREAK,   &drcbe_x64::op_break },      // BREAK
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
	{ uml::OP_SETFLGS, &drcbe_x64::op_setflgs },    // SETFLGS src
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
	{ uml::OP_MULULW,  &drcbe_x64::op_mululw },     // MULULW  dst,src1,src2[,f]
	{ uml::OP_MULS,    &drcbe_x64::op_muls },       // MULS    dst,edst,src1,src2[,f]
	{ uml::OP_MULSLW,  &drcbe_x64::op_mulslw },     // MULSLW  dst,src1,src2[,f]
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
	if (LOG_HASHJMPS)
	{
		m_near.debug_log_hashjmp = (x86code *)debug_log_hashjmp;
		m_near.debug_log_hashjmp_fail = (x86code *)debug_log_hashjmp_fail;
	}

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

	// resolve the actual addresses of member functions we need to call
	m_drcmap_get_value.set(m_map, &drc_map_variables::get_value);
	if (!m_drcmap_get_value)
		throw emu_fatalerror("Error resolving map variable get value function!\n");
	m_memory_accessors.resize(m_space.size());
	for (int space = 0; m_space.size() > space; ++space)
	{
		if (m_space[space])
		{
			auto &accessors = m_memory_accessors[space];
			accessors.resolved.set(*m_space[space]);
			accessors.specific = m_space[space]->specific_accessors();
			accessors.address_mask = m_space[space]->addrmask() & make_bitmask<offs_t>(accessors.specific.address_width) & ~make_bitmask<offs_t>(accessors.specific.native_mask_bits);
			offs_t const shiftedmask = accessors.address_mask >> accessors.specific.low_bits;
			offs_t const nomask = ~offs_t(0);
			accessors.no_mask = nomask == accessors.address_mask;
			accessors.has_high_bits = shiftedmask != 0U;
			accessors.mask_high_bits = !accessors.specific.low_bits || !BIT(accessors.address_mask, (sizeof(offs_t) * 8) - 1) || (shiftedmask & (shiftedmask + 1));
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
	entry_point.init(FuncSignature::build<uint32_t, uint8_t *, x86code *>(CallConvId::kHost), Environment::host());

	FuncFrame frame;
	frame.init(entry_point);
	frame.addDirtyRegs(rbx, rbp, rsi, rdi, r12, r13, r14, r15);
	FuncArgsAssignment args(&entry_point);
	args.assignAll(rbp);
	args.updateFuncFrame(frame);
	frame.finalize();

	a.emitProlog(frame);
	a.emitArgsAssignment(frame, args);

	a.sub(rsp, 40);
	a.mov(MABS(&m_near.stacksave), rsp);
	a.stmxcsr(MABS(&m_near.ssemode));
	a.jmp(Gpq(REG_PARAM2));

	// generate an exit point
	m_exit = dst + a.offset();
	a.bind(a.newNamedLabel("exit_point"));
	a.ldmxcsr(MABS(&m_near.ssemode));
	a.mov(rsp, MABS(&m_near.stacksave));
	a.add(rsp, 40);
	a.emitEpilog(frame);

	// generate a no code point
	m_nocode = dst + a.offset();
	a.bind(a.newNamedLabel("nocode_point"));
	a.jmp(Gpq(REG_PARAM1));

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
	// do this here because device.debug() isn't initialised at construction time
	if (!m_debug_cpu_instruction_hook && (m_device.machine().debug_flags & DEBUG_FLAG_ENABLED))
	{
		m_debug_cpu_instruction_hook.set(*m_device.debug(), &device_debug::instruction_hook);
		if (!m_debug_cpu_instruction_hook)
			throw emu_fatalerror("Error resolving debugger instruction hook member function!\n");
	}

	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_map.block_begin(block);

	// compute the base by aligning the cache top to a cache line
	auto [err, linesize] = osd_get_cache_line_size();
	uintptr_t linemask = 63;
	if (err)
	{
		osd_printf_verbose("Error getting cache line size (%s:%d %s), assuming 64 bytes\n", err.category().name(), err.value(), err.message());
	}
	else
	{
		assert(linesize);
		linemask = linesize - 1;
		for (unsigned shift = 1; linemask & (linemask + 1); ++shift)
			linemask |= linemask >> shift;
	}
	x86code *dst = (x86code *)(uintptr_t(m_cache.top() + linemask) & ~linemask);

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
	bool const is64 = dst.x86RmSize() == 8;

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

			a.mov(tmp, MABS(param.memory()));
			a.emit(opcode, dst, tmp);
		}
		else if (opcode != Inst::kIdTest)
			// most instructions are register,memory
			a.emit(opcode, dst, MABS(param.memory()));
		else
			// test instruction requires memory,register
			a.emit(opcode, MABS(param.memory()), dst);
	}
	else if (param.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId(is64 ? RegType::kX86_Gpq : RegType::kX86_Gpd, param.ireg());

		a.emit(opcode, dst, src);
	}
}

void drcbe_x64::calculate_status_flags(Assembler &a, uint32_t instsize, Operand const &dst, u8 flags)
{
	// calculate status flags in a way that does not modify any other status flags
	uint32_t flagmask = 0;

	// can't get FLAG_V from lahf so implement it using seto if needed in the future
	if (flags & FLAG_C) flagmask |= 0x0100;
	if (flags & FLAG_Z) flagmask |= 0x4000;
	if (flags & FLAG_S) flagmask |= 0x8000;
	if (flags & FLAG_U) flagmask |= 0x0400;

	if ((flags & (FLAG_Z | FLAG_S)) == flags)
	{
		Gp tempreg = r10;
		Gp tempreg2 = r11;

		a.mov(tempreg, rax);

		if (dst.isMem())
			a.mov(tempreg2, dst.as<Mem>());
		else
			a.mov(tempreg2, dst.as<Gpq>().r64());

		a.lahf();
		a.and_(rax, ~flagmask);
		if (instsize == 4)
			a.test(tempreg2.r32(), tempreg2.r32());
		else
			a.test(tempreg2, tempreg2);
		a.mov(tempreg2, rax);

		a.lahf();
		a.and_(rax, flagmask);
		a.or_(rax, tempreg2);
		a.sahf();

		a.mov(rax, tempreg);
	}
	else
	{
		fatalerror("drcbe_x64::calculate_status_flags: unknown flag combination requested: %02x\n", flags);
	}
}

void drcbe_x64::calculate_status_flags_mul(Assembler &a, uint32_t instsize, asmjit::x86::Gp const &lo, asmjit::x86::Gp const &hi)
{
	Gp tempreg = r11;
	Gp tempreg2 = r10;

	if (lo.id() == rax.id() || hi.id() == rax.id())
		a.mov(tempreg2, rax);

	a.seto(al);
	a.movzx(rax, al); // clear out the rest of rax for lahf
	a.mov(tempreg, rax);

	a.lahf();
	a.shl(tempreg, 16);
	a.or_(tempreg, rax);

	if (hi.id() == rax.id())
		a.mov(rax, tempreg2);

	if (instsize == 4)
		a.test(hi.r32(), hi.r32());
	else
		a.test(hi, hi);

	a.lahf();  // will have the sign flag + upper half zero
	a.mov(hi, rax);

	if (lo.id() == rax.id())
		a.mov(rax, tempreg2);

	if (instsize == 4)
		a.test(lo.r32(), lo.r32());
	else
		a.test(lo, lo);

	a.lahf();  // lower half zero
	a.and_(rax, hi); // if top and bottom are zero then this will leave the zero flag
	a.and_(rax, 0x4000); // zero
	a.and_(hi, 0x8000); // sign
	a.or_(rax, hi); // combine sign flag from top and zero flags for both

	a.and_(tempreg, ~(0x4000 | 0x8000));
	a.or_(tempreg, rax);

	// restore overflow flag
	a.mov(rax, tempreg);
	a.shr(rax, 16);
	a.add(al, 0x7f);

	a.mov(rax, tempreg);
	a.sahf();
}

void drcbe_x64::calculate_status_flags_mul_low(Assembler &a, uint32_t instsize, asmjit::x86::Gp const &lo)
{
	// calculate zero, sign flags based on the lower half of the result but keep the overflow from the multiplication
	a.seto(dl);

	if (instsize == 4)
		a.test(lo.r32(), lo.r32());
	else
		a.test(lo, lo);

	// restore overflow flag
	a.lahf();
	a.add(dl, 0x7f);
	a.sahf();
}

void drcbe_x64::shift_op_param(Assembler &a, Inst::Id const opcode, size_t opsize, Operand const &dst, be_parameter const &param, bool update_flags)
{
	if (param.is_immediate())
	{
		const uint32_t bitshift = param.immediate() & (opsize * 8 - 1);

		if (bitshift != 0)
			a.emit(opcode, dst, imm(param.immediate()));

		if (update_flags)
		{
			if (bitshift == 0)
				a.clc(); // throw away carry since it'll never be used

			calculate_status_flags(a, opsize, dst, FLAG_S | FLAG_Z);
		}
	}
	else
	{
		Label calc = a.newLabel();
		Label end = a.newLabel();

		Gp shift = cl;

		a.mov(r10, rax);
		a.lahf();

		mov_reg_param(a, shift, param);

		a.and_(shift, opsize * 8 - 1);
		a.test(shift, shift);

		a.short_().jnz(calc);

		a.mov(rax, r10);

		if (update_flags)
			a.clc(); // throw away carry since it'll never be used

		a.short_().jmp(end);

		a.bind(calc);

		a.sahf(); // restore flags to keep carry for rolc/rorc
		a.mov(rax, r10);

		a.emit(opcode, dst, shift);

		a.bind(end);

		if (update_flags)
			calculate_status_flags(a, opsize, dst, FLAG_S | FLAG_Z); // calculate status flags but preserve carry
	}
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
	{
		const int64_t delta = imm - (a.code()->baseAddress() + a.offset() + 7);
		if (short_immediate(delta))
			a.lea(reg.r64(), ptr(rip, delta));
		else
			a.mov(reg.r64(), imm);
	}
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
	a.short_().jmp(skip);

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(drccodeptr(a.code()->baseAddress() + a.offset()));

	// by default, the handle points to prologue code that moves the stack pointer
	a.lea(rsp, ptr(rsp, -40));
	a.bind(skip);
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
//  op_break - process a BREAK opcode
//-------------------------------------------------

void drcbe_x64::op_break(Assembler &a, const instruction &inst)
{
	static const char *const message = "break from drc";
	mov_r64_imm(a, Gpq(REG_PARAM1), (uintptr_t)message);
	smart_call_r64(a, (x86code *)(uintptr_t)&osd_break_into_debugger, rax);
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
		mov_r64_imm(a, Gpq(REG_PARAM1), m_debug_cpu_instruction_hook.obj);              // mov   param1,device.debug
		mov_reg_param(a, Gpd(REG_PARAM2), pcp);                                         // mov   param2,pcp
		smart_call_r64(a, m_debug_cpu_instruction_hook.func, rax);                      // call  debug_cpu_instruction_hook

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

	// load the stack base
	Label nocode = a.newLabel();
	a.mov(rsp, MABS(&m_near.stacksave));                                            // mov   rsp,[stacksave]

	// fixed mode cases
	if (modep.is_immediate() && m_hash.is_mode_populated(modep.immediate()))
	{
		if (pcp.is_immediate())
		{
			// a straight immediate jump is direct, though we need the PC in EAX in case of failure
			uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			a.short_().lea(Gpq(REG_PARAM1), ptr(nocode));                               // lea   rcx,[rip+nocode]
			a.jmp(MABS(&m_hash.base()[modep.immediate()][l1val][l2val]));               // jmp   hash[modep][l1val][l2val]
		}
		else
		{
			// a fixed mode but variable PC
			mov_reg_param(a, eax, pcp);                                                 // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and  eax,l2mask << l2shift
			a.mov(rdx, ptr(rbp, rdx, 3, offset_from_rbp(&m_hash.base()[modep.immediate()][0])));
																						// mov   rdx,hash[modep+edx*8]
			a.short_().lea(Gpq(REG_PARAM1), ptr(nocode));                               // lea   rcx,[rip+nocode]
			a.jmp(ptr(rdx, rax, 3 - m_hash.l2shift()));                                 // jmp   [rdx+rax*shift]
		}
	}
	else
	{
		// variable mode
		Gp modereg = modep.select_register(ecx);
		mov_reg_param(a, modereg, modep);                                               // mov   modereg,modep
		a.mov(rcx, ptr(rbp, modereg, 3, offset_from_rbp(m_hash.base())));               // mov   rcx,hash[modereg*8]

		if (pcp.is_immediate())
		{
			// fixed PC
			uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			a.mov(rdx, ptr(rcx, l1val * 8));                                            // mov   rdx,[rcx+l1val*8]
			a.short_().lea(Gpq(REG_PARAM1), ptr(nocode));                               // lea   rcx,[rip+nocode]
			a.jmp(ptr(rdx, l2val * 8));                                                 // jmp   [l2val*8]
		}
		else
		{
			// variable PC
			mov_reg_param(a, eax, pcp);                                                 // mov   eax,pcp
			a.mov(edx, eax);                                                            // mov   edx,eax
			a.shr(edx, m_hash.l1shift());                                               // shr   edx,l1shift
			a.mov(rdx, ptr(rcx, rdx, 3));                                               // mov   rdx,[rcx+rdx*8]
			a.and_(eax, m_hash.l2mask() << m_hash.l2shift());                           // and   eax,l2mask << l2shift
			a.short_().lea(Gpq(REG_PARAM1), ptr(nocode));                               // lea   rcx,[rip+nocode]
			a.jmp(ptr(rdx, rax, 3 - m_hash.l2shift()));                                 // jmp   [rdx+rax*shift]
		}
	}

	// in all cases, if there is no code, we return here to generate the exception
	a.bind(nocode);
	if (LOG_HASHJMPS)
		smart_call_m64(a, &m_near.debug_log_hashjmp_fail);

	mov_mem_param(a, MABS(&m_state.exp, 4), pcp);                                       // mov   [exp],param
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
	Label no_exception;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		no_exception = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), no_exception);                // jcc   no_exception
	}
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
	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip
	}

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
	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip
	}

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
	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip
	}

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
	a.mov(rax, MABS(&m_near.stacksave));
	mov_r64_imm(a, Gpq(REG_PARAM1), m_drcmap_get_value.obj);
	mov_r64_imm(a, Gpq(REG_PARAM3), inst.param(1).mapvar());
	a.mov(Gpq(REG_PARAM2), ptr(rax, -8));
	a.sub(Gpq(REG_PARAM2), 1);
	smart_call_r64(a, m_drcmap_get_value.func, rax);
	mov_param_reg(a, dstp, eax);
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

	if (srcp.is_immediate())
	{
		// immediate case
		int value = srcp.immediate() & 3;
		a.mov(MABS(&m_state.fmod, 1), value);                                           // mov   [fmod],srcp
		a.ldmxcsr(MABS(&m_near.ssecontrol[value]));                                     // ldmxcsr fp_control[srcp]
	}
	else
	{
		// register/memory case
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

	a.lahf();
	a.mov(r10, rax);

	// compute mask for flags
	// can't get FLAG_V from lahf
	uint32_t flagmask = 0;
	if (maskp.immediate() & FLAG_C) flagmask |= 0x001;
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
			if (maskp.immediate() & FLAG_V)
			{
				a.seto(al);
				a.movzx(eax, al);
				a.shl(eax, 1);
			}

			a.mov(r11, r10);
			a.shr(r11, 8);
			a.and_(r11, flagmask);
			a.movzx(dstreg, byte_ptr(rbp, r11, 0, offset_from_rbp(&m_near.flagsmap[0]))); // movzx  dstreg,[flags_map]

			if (maskp.immediate() & FLAG_V)
				a.or_(dstreg, eax);
			break;
	}

	// 32-bit form
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);                                                 // mov   dstp,dstreg

	// 64-bit form
	else if (inst.size() == 8)
		mov_param_reg(a, dstp, dstreg.r64());                                           // mov   dstp,dstreg

	if (maskp.immediate() & FLAG_V)
	{
		// Restore overflow flag
		a.mov(eax, dstreg);
		a.shr(eax, 1);
		a.and_(eax, 1);
		a.add(al, 0x7f);
	}

	a.mov(rax, r10);
	a.sahf();
}


//-------------------------------------------------
//  op_setflgs - process a SETFLGS opcode
//-------------------------------------------------

void drcbe_x64::op_setflgs(Assembler &a, const instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);

	a.pushfq();

	mov_reg_param(a, rax, srcp);

	a.mov(rax, ptr(rbp, rax, 3, offset_from_rbp(&m_near.flagsunmap[0])));
	a.and_(qword_ptr(rsp), ~0x8c5);
	a.or_(qword_ptr(rsp), rax);

	a.popfq();
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
			a.mov(ptr(rcx, regoffs + 8 * regnum), Gpq(int_register_map[regnum]));
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
			a.movsd(ptr(rcx, regoffs + 8 * regnum), Xmm(float_register_map[regnum]));
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
			a.mov(Gpq(int_register_map[regnum]), ptr(rcx, regoffs + 8 * regnum));
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
			a.movsd(Xmm(float_register_map[regnum]), ptr(rcx, regoffs + 8 * regnum));
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
	a.push(qword_ptr(rbp, rax, 3, offset_from_rbp(&m_near.flagsunmap[0])));             // push   flags_unmap[eax*8]
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
	Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

	// immediate index
	if (indp.is_immediate())
	{
		s32 const offset = baseoffs + (s32(indp.immediate()) << scalesizep.scale());

		if (size == SIZE_BYTE)
			a.movzx(dstreg, byte_ptr(basereg, offset));
		else if (size == SIZE_WORD)
			a.movzx(dstreg, word_ptr(basereg, offset));
		else if (size == SIZE_DWORD)
			a.mov(dstreg, dword_ptr(basereg, offset));
		else if (size == SIZE_QWORD)
			a.mov(dstreg, ptr(basereg, offset));
	}

	// other index
	else
	{
		Gp indreg = indp.select_register(rcx);
		movsx_r64_p32(a, indreg, indp);
		if (size == SIZE_BYTE)
			a.movzx(dstreg, byte_ptr(basereg, indreg, scalesizep.scale(), baseoffs));
		else if (size == SIZE_WORD)
			a.movzx(dstreg, word_ptr(basereg, indreg, scalesizep.scale(), baseoffs));
		else if (size == SIZE_DWORD)
			a.mov(dstreg, dword_ptr(basereg, indreg, scalesizep.scale(), baseoffs));
		else if (size == SIZE_QWORD)
			a.mov(dstreg, ptr(basereg, indreg, scalesizep.scale(), baseoffs));
	}

	// store result
	mov_param_reg(a, dstp, dstreg);
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
	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.read.function) || accessors.specific.read.is_virtual;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);
	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		// set default mem_mask
		if (accessors.specific.native_bytes <= 4)
			a.mov(Gpd(REG_PARAM3), make_bitmask<uint32_t>(accessors.specific.native_bytes << 3));
		else
			a.mov(Gpq(REG_PARAM3), make_bitmask<uint64_t>(accessors.specific.native_bytes << 3));

		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.read.dispatch));                        // load dispatch table pointer
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // save masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(Gpq(REG_PARAM1), ptr(rax, Gpd(REG_PARAM2), 3));                            // load dispatch table entry
				if (accessors.specific.low_bits)
					a.mov(Gpd(REG_PARAM2), r10d);                                                // restore masked address

			}
			else
			{
				a.mov(Gpq(REG_PARAM1), ptr(rax, r10d, 3));                                       // load dispatch table entry
			}
		}
		else
		{
			a.mov(Gpq(REG_PARAM1), ptr(rax));                                                    // load dispatch table entry
		}

		if (accessors.specific.read.is_virtual)
			a.mov(rax, ptr(Gpq(REG_PARAM1), accessors.specific.read.displacement));              // load vtable pointer
		if (accessors.specific.read.displacement)
			a.add(Gpq(REG_PARAM1), accessors.specific.read.displacement);                        // apply this pointer offset
		if (accessors.specific.read.is_virtual)
			a.call(ptr(rax, accessors.specific.read.function));                                  // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.read.function, rax);                 // call non-virtual member function
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		// if the destination register isn't a non-volatile register, we can use it to save the shift count
		bool need_save = !is_nonvolatile_register(dstreg);
		if (need_save)
			a.mov(ptr(rsp, 32), Gpq(int_register_map[0]));                                       // save I0 register

		if ((accessors.specific.native_bytes <= 4) || (spacesizep.size() != SIZE_QWORD))
			a.mov(Gpd(REG_PARAM3), imm(make_bitmask<uint32_t>(8 << spacesizep.size())));         // set default mem_mask
		else
			a.mov(Gpq(REG_PARAM3), imm(make_bitmask<uint64_t>(8 << spacesizep.size())));         // set default mem_mask

		a.mov(ecx, Gpd(REG_PARAM2));                                                             // copy address for bit offset
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask

		int const shift = m_space[spacesizep.space()]->addr_shift() - 3;
		if (m_space[spacesizep.space()]->endianness() != ENDIANNESS_LITTLE)
			a.not_(ecx);                                                                         // swizzle address for bit Endian spaces
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.read.dispatch));                        // load dispatch table pointer
		if (shift < 0)
			a.shl(ecx, imm(-shift));                                                             // convert address to bits (left shift)
		else if (shift > 0)
			a.shr(ecx, imm(shift));                                                              // convert address to bits (right shift)
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // copy masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(rax, ptr(rax, Gpd(REG_PARAM2), 3));                                        // load dispatch table entry
			}
			else
			{
				a.mov(rax, ptr(rax, r10d, 3));                                                   // load dispatch table entry
			}
		}
		else
		{
			a.mov(Gpq(REG_PARAM1), ptr(rax));                                                    // load dispatch table entry
		}
		a.and_(ecx, imm((accessors.specific.native_bytes - (1 << spacesizep.size())) << 3));     // mask bit address
		if (accessors.has_high_bits && accessors.mask_high_bits && accessors.specific.low_bits)
			a.mov(Gpd(REG_PARAM2), r10d);                                                        // restore masked address
		if (need_save)
			a.mov(Gpd(int_register_map[0]), ecx);                                                // save masked bit address
		else
			a.mov(dstreg.r32(), ecx);                                                            // save masked bit address
		if (accessors.specific.read.is_virtual)
			a.mov(r10, ptr(rax, accessors.specific.read.displacement));                          // load vtable pointer
		if (accessors.specific.read.displacement)
			a.add(rax, accessors.specific.read.displacement);                                    // apply this pointer offset
		if (accessors.specific.native_bytes <= 4)
			a.shl(Gpd(REG_PARAM3), cl);                                                          // shift mem_mask by masked bit address
		else
			a.shl(Gpq(REG_PARAM3), cl);                                                          // shift mem_mask by masked bit address

		// need to do this after finished with CL as REG_PARAM1 is C on Windows
		a.mov(Gpq(REG_PARAM1), rax);
		if (accessors.specific.read.is_virtual)
			a.call(ptr(r10, accessors.specific.read.function));                                  // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.read.function, rax);                 // call non-virtual member function

		if (need_save)
		{
			a.mov(ecx, Gpd(int_register_map[0]));                                                // restore masked bit address
			a.mov(Gpq(int_register_map[0]), ptr(rsp, 32));                                       // restore I0 register
		}
		else
		{
			a.mov(ecx, dstreg.r32());                                                            // restore masked bit address
		}
		if (accessors.specific.native_bytes <= 4)
			a.shr(eax, cl);                                                                      // shift result by masked bit address
		else
			a.shr(rax, cl);                                                                      // shift result by masked bit address
	}
	else if (spacesizep.size() == SIZE_BYTE)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_byte.obj);
		smart_call_r64(a, accessors.resolved.read_byte.func, rax);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_word.obj);
		smart_call_r64(a, accessors.resolved.read_word.func, rax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_dword.obj);
		smart_call_r64(a, accessors.resolved.read_dword.func, rax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_qword.obj);
		smart_call_r64(a, accessors.resolved.read_qword.func, rax);
	}

	// move or zero-extend result if necessary
	if (spacesizep.size() == SIZE_BYTE)
	{
		a.movzx(dstreg, al);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		a.movzx(dstreg, ax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (dstreg != eax || inst.size() == 8)
			a.mov(dstreg, eax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (dstreg != eax)
			a.mov(dstreg.r64(), rax);
	}

	// store result
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);
	else
		mov_param_reg(a, dstp, dstreg.r64());
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
	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.read.function) || accessors.specific.read.is_virtual;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);
	if (spacesizep.size() != SIZE_QWORD)
		mov_reg_param(a, Gpd(REG_PARAM3), maskp);
	else
		mov_reg_param(a, Gpq(REG_PARAM3), maskp);
	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.read.dispatch));                        // load dispatch table pointer
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // save masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(Gpq(REG_PARAM1), ptr(rax, Gpd(REG_PARAM2), 3));                            // load dispatch table entry
				if (accessors.specific.low_bits)
					a.mov(Gpd(REG_PARAM2), r10d);                                                // restore masked address
			}
			else
			{
				a.mov(Gpq(REG_PARAM1), ptr(rax, r10d, 3));                                       // load dispatch table entry
			}
		}
		else
		{
			a.mov(Gpq(REG_PARAM1), ptr(rax));                                                    // load dispatch table entry
		}

		if (accessors.specific.read.is_virtual)
			a.mov(rax, ptr(Gpq(REG_PARAM1), accessors.specific.read.displacement));              // load vtable pointer
		if (accessors.specific.read.displacement)
			a.add(Gpq(REG_PARAM1), accessors.specific.read.displacement);                        // apply this pointer offset
		if (accessors.specific.read.is_virtual)
			a.call(ptr(rax, accessors.specific.read.function));                                  // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.read.function, rax);                 // call non-virtual member function
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		// if the destination register isn't a non-volatile register, we can use it to save the shift count
		bool need_save = !is_nonvolatile_register(dstreg);
		if (need_save)
			a.mov(ptr(rsp, 32), Gpq(int_register_map[0]));                                       // save I0 register

		a.mov(ecx, Gpd(REG_PARAM2));                                                             // copy address for bit offset
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask

		int const shift = m_space[spacesizep.space()]->addr_shift() - 3;
		if (m_space[spacesizep.space()]->endianness() != ENDIANNESS_LITTLE)
			a.not_(ecx);                                                                         // swizzle address for bit Endian spaces
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.read.dispatch));                        // load dispatch table pointer
		if (shift < 0)
			a.shl(ecx, imm(-shift));                                                             // convert address to bits (left shift)
		else if (shift > 0)
			a.shr(ecx, imm(shift));                                                              // convert address to bits (right shift)
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // copy masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(rax, ptr(rax, Gpd(REG_PARAM2), 3));                                        // load dispatch table entry
			}
			else
			{
				a.mov(rax, ptr(rax, r10d, 3));                                                   // load dispatch table entry
			}
		}
		else
		{
			a.mov(rax, ptr(rax));                                                                // load dispatch table entry
		}
		a.and_(ecx, imm((accessors.specific.native_bytes - (1 << spacesizep.size())) << 3));     // mask bit address
		if (accessors.has_high_bits && accessors.mask_high_bits && accessors.specific.low_bits)
			a.mov(Gpd(REG_PARAM2), r10d);                                                        // restore masked address
		if (need_save)
			a.mov(Gpd(int_register_map[0]), ecx);                                                // save masked bit address
		else
			a.mov(dstreg.r32(), ecx);                                                            // save masked bit address
		if (accessors.specific.read.is_virtual)
			a.mov(r10, ptr(rax, accessors.specific.read.displacement));                          // load vtable pointer
		if (accessors.specific.read.displacement)
			a.add(rax, accessors.specific.read.displacement);                                    // apply this pointer offset
		if (accessors.specific.native_bytes <= 4)
			a.shl(Gpd(REG_PARAM3), cl);                                                          // shift mem_mask by masked bit address
		else
			a.shl(Gpq(REG_PARAM3), cl);                                                          // shift mem_mask by masked bit address

		// need to do this after finished with CL as REG_PARAM1 is C on Windows
		a.mov(Gpq(REG_PARAM1), rax);
		if (accessors.specific.read.is_virtual)
			a.call(ptr(r10, accessors.specific.read.function));                                  // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.read.function, rax);                 // call non-virtual member function

		if (need_save)
		{
			a.mov(ecx, Gpd(int_register_map[0]));                                                // restore masked bit address
			a.mov(Gpq(int_register_map[0]), ptr(rsp, 32));                                       // restore I0 register
		}
		else
		{
			a.mov(ecx, dstreg.r32());                                                            // restore masked bit address
		}
		if (accessors.specific.native_bytes <= 4)
			a.shr(eax, cl);                                                                      // shift result by masked bit address
		else
			a.shr(rax, cl);                                                                      // shift result by masked bit address
	}
	else if (spacesizep.size() == SIZE_BYTE)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_byte_masked.obj);
		smart_call_r64(a, accessors.resolved.read_byte_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_word_masked.obj);
		smart_call_r64(a, accessors.resolved.read_word_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_dword_masked.obj);
		smart_call_r64(a, accessors.resolved.read_dword_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.read_qword_masked.obj);
		smart_call_r64(a, accessors.resolved.read_qword_masked.func, rax);
	}

	// move or zero-extend result if necessary
	if (spacesizep.size() == SIZE_BYTE)
	{
		a.movzx(dstreg, al);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		a.movzx(dstreg, ax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		if (dstreg != eax || inst.size() == 8)
			a.mov(dstreg, eax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		if (dstreg != eax)
			a.mov(dstreg.r64(), rax);
	}

	// store result
	if (inst.size() == 4)
		mov_param_reg(a, dstp, dstreg);
	else
		mov_param_reg(a, dstp, dstreg.r64());
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
	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.write.function) || accessors.specific.write.is_virtual;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);
	if (spacesizep.size() != SIZE_QWORD)
		mov_reg_param(a, Gpd(REG_PARAM3), srcp);
	else
		mov_reg_param(a, Gpq(REG_PARAM3), srcp);
	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		// set default mem_mask
		if (accessors.specific.native_bytes <= 4)
			a.mov(Gpd(REG_PARAM4), make_bitmask<uint32_t>(accessors.specific.native_bytes << 3));
		else
			a.mov(Gpq(REG_PARAM4), make_bitmask<uint64_t>(accessors.specific.native_bytes << 3));

		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.write.dispatch));                       // load dispatch table pointer
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // save masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(Gpq(REG_PARAM1), ptr(rax, Gpd(REG_PARAM2), 3));                            // load dispatch table entry
				if (accessors.specific.low_bits)
					a.mov(Gpd(REG_PARAM2), r10d);                                                // restore masked address
			}
			else
			{
				a.mov(Gpq(REG_PARAM1), ptr(rax, r10d, 3));                                       // load dispatch table entry
			}
		}
		else
		{
			a.mov(Gpq(REG_PARAM1), ptr(rax));                                                    // load dispatch table entry
		}

		if (accessors.specific.write.is_virtual)
			a.mov(rax, ptr(Gpq(REG_PARAM1), accessors.specific.write.displacement));             // load vtable pointer
		if (accessors.specific.write.displacement)
			a.add(Gpq(REG_PARAM1), accessors.specific.write.displacement);                       // apply this pointer offset
		if (accessors.specific.write.is_virtual)
			a.call(ptr(rax, accessors.specific.write.function));                                 // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.write.function, rax);                // call non-virtual member function
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		a.mov(ecx, Gpd(REG_PARAM2));                                                             // copy address for bit offset
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask

		int const shift = m_space[spacesizep.space()]->addr_shift() - 3;
		if (m_space[spacesizep.space()]->endianness() != ENDIANNESS_LITTLE)
			a.not_(ecx);                                                                         // swizzle address for bit Endian spaces
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.write.dispatch));                       // load dispatch table pointer
		if (shift < 0)
			a.shl(ecx, imm(-shift));                                                             // convert address to bits (left shift)
		else if (shift > 0)
			a.shr(ecx, imm(shift));                                                              // convert address to bits (right shift)
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // copy masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(rax, ptr(rax, Gpd(REG_PARAM2), 3));                                        // load dispatch table entry
			}
			else
			{
				a.mov(rax, ptr(rax, r10d, 3));                                                   // load dispatch table entry
			}
		}
		else
		{
			a.mov(rax, ptr(rax));                                                                // load dispatch table entry
		}
		a.and_(ecx, imm((accessors.specific.native_bytes - (1 << spacesizep.size())) << 3));     // mask bit address
		if ((accessors.specific.native_bytes <= 4) || (spacesizep.size() != SIZE_QWORD))
			a.mov(r11d, imm(make_bitmask<uint32_t>(8 << spacesizep.size())));                    // set default mem_mask
		else
			a.mov(r11, imm(make_bitmask<uint64_t>(8 << spacesizep.size())));                     // set default mem_mask
		if (accessors.has_high_bits && accessors.mask_high_bits && accessors.specific.low_bits)
			a.mov(Gpd(REG_PARAM2), r10d);                                                        // restore masked address
		if (accessors.specific.write.is_virtual)
			a.mov(r10, ptr(rax, accessors.specific.write.displacement));                         // load vtable pointer
		if (accessors.specific.write.displacement)
			a.add(rax, accessors.specific.write.displacement);                                   // apply this pointer offset
		if (accessors.specific.native_bytes <= 4)
		{
			a.shl(r11d, cl);                                                                     // shift mem_mask by masked bit address
			a.shl(Gpd(REG_PARAM3), cl);                                                          // shift data by masked bit address
		}
		else
		{
			a.shl(r11, cl);                                                                      // shift mem_mask by masked bit address
			a.shl(Gpq(REG_PARAM3), cl);                                                          // shift data by masked bit address
		}

		// need to do this after finished with CL as REG_PARAM1 is C on Windows and REG_PARAM4 is C on SysV
		a.mov(Gpq(REG_PARAM1), rax);
		if (accessors.specific.native_bytes <= 4)
			a.mov(Gpd(REG_PARAM4), r11d);                                                        // copy mem_mask to parameter 4 (ECX on SysV)
		else
			a.mov(Gpq(REG_PARAM4), r11);                                                         // copy mem_mask to parameter 4 (RCX on SysV)
		if (accessors.specific.write.is_virtual)
			a.call(ptr(r10, accessors.specific.write.function));                                 // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.write.function, rax);                // call non-virtual member function
	}
	else if (spacesizep.size() == SIZE_BYTE)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_byte.obj);
		smart_call_r64(a, accessors.resolved.write_byte.func, rax);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_word.obj);
		smart_call_r64(a, accessors.resolved.write_word.func, rax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_dword.obj);
		smart_call_r64(a, accessors.resolved.write_dword.func, rax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_qword.obj);
		smart_call_r64(a, accessors.resolved.write_qword.func, rax);
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
	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.write.function) || accessors.specific.write.is_virtual;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);
	if (spacesizep.size() != SIZE_QWORD)
		mov_reg_param(a, Gpd(REG_PARAM3), srcp);
	else
		mov_reg_param(a, Gpq(REG_PARAM3), srcp);
	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		if (spacesizep.size() != SIZE_QWORD)
			mov_reg_param(a, Gpd(REG_PARAM4), maskp);                                            // get mem_mask
		else
			mov_reg_param(a, Gpq(REG_PARAM4), maskp);                                            // get mem_mask

		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.write.dispatch));                       // load dispatch table pointer
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // save masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(Gpq(REG_PARAM1), ptr(rax, Gpd(REG_PARAM2), 3));                            // load dispatch table entry
				if (accessors.specific.low_bits)
					a.mov(Gpd(REG_PARAM2), r10d);                                                // restore masked address
			}
			else
			{
				a.mov(Gpq(REG_PARAM1), ptr(rax, r10d, 3));                                       // load dispatch table entry
			}
		}
		else
		{
			a.mov(Gpq(REG_PARAM1), ptr(rax));                                                    // load dispatch table entry
		}

		if (accessors.specific.write.is_virtual)
			a.mov(rax, ptr(Gpq(REG_PARAM1), accessors.specific.write.displacement));             // load vtable pointer
		if (accessors.specific.write.displacement)
			a.add(Gpq(REG_PARAM1), accessors.specific.write.displacement);                       // apply this pointer offset
		if (accessors.specific.write.is_virtual)
			a.call(ptr(rax, accessors.specific.write.function));                                 // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.write.function, rax);                // call non-virtual member function
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		a.mov(ecx, Gpd(REG_PARAM2));                                                             // copy address for bit offset
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.mov(r10d, Gpd(REG_PARAM2));                                                        // copy address for dispatch index
		if (spacesizep.size() != SIZE_QWORD)
			mov_reg_param(a, r11d, maskp);                                                       // get mem_mask
		else
			mov_reg_param(a, r11, maskp);                                                        // get mem_mask
		if (!accessors.no_mask)
			a.and_(Gpd(REG_PARAM2), imm(accessors.address_mask));                                // apply address mask

		int const shift = m_space[spacesizep.space()]->addr_shift() - 3;
		if (m_space[spacesizep.space()]->endianness() != ENDIANNESS_LITTLE)
			a.not_(ecx);                                                                         // swizzle address for bit Endian spaces
		if (accessors.has_high_bits && !accessors.mask_high_bits)
			a.shr(r10d, accessors.specific.low_bits);                                            // shift off low bits
		mov_r64_imm(a, rax, uintptr_t(accessors.specific.write.dispatch));                       // load dispatch table pointer
		if (shift < 0)
			a.shl(ecx, imm(-shift));                                                             // convert address to bits (left shift)
		else if (shift > 0)
			a.shr(ecx, imm(shift));                                                              // convert address to bits (right shift)
		if (accessors.has_high_bits)
		{
			if (accessors.mask_high_bits)
			{
				if (accessors.specific.low_bits)
				{
					a.mov(r10d, Gpd(REG_PARAM2));                                                // copy masked address
					a.shr(Gpd(REG_PARAM2), accessors.specific.low_bits);                         // shift off low bits
				}
				a.mov(rax, ptr(rax, Gpd(REG_PARAM2), 3));                                        // load dispatch table entry
			}
			else
			{
				a.mov(rax, ptr(rax, r10d, 3));                                                   // load dispatch table entry
			}
		}
		else
		{
			a.mov(rax, ptr(rax));                                                                // load dispatch table entry
		}
		a.and_(ecx, imm((accessors.specific.native_bytes - (1 << spacesizep.size())) << 3));     // mask bit address
		if (accessors.has_high_bits && accessors.mask_high_bits && accessors.specific.low_bits)
			a.mov(Gpd(REG_PARAM2), r10d);                                                        // restore masked address
		if (accessors.specific.native_bytes <= 4)
		{
			a.shl(r11d, cl);                                                                     // shift mem_mask by masked bit address
			a.shl(Gpd(REG_PARAM3), cl);                                                          // shift data by masked bit address
		}
		else
		{
			a.shl(r11, cl);                                                                      // shift mem_mask by masked bit address
			a.shl(Gpq(REG_PARAM3), cl);                                                          // shift data by masked bit address
		}
		if (accessors.specific.write.is_virtual)
			a.mov(r10, ptr(rax, accessors.specific.write.displacement));                         // load vtable pointer

		// need to do this after finished with CL as REG_PARAM1 is C on Windows and REG_PARAM4 is C on SysV
		a.mov(Gpq(REG_PARAM1), rax);
		if (accessors.specific.native_bytes <= 4)
			a.mov(Gpd(REG_PARAM4), r11d);                                                        // copy mem_mask to parameter 4 (ECX on SysV)
		else
			a.mov(Gpq(REG_PARAM4), r11);                                                         // copy mem_mask to parameter 4 (RCX on SysV)
		if (accessors.specific.write.displacement)
			a.add(Gpq(REG_PARAM1), accessors.specific.write.displacement);                       // apply this pointer offset
		if (accessors.specific.write.is_virtual)
			a.call(ptr(r10, accessors.specific.write.function));                                 // call virtual member function
		else
			smart_call_r64(a, (x86code *)accessors.specific.write.function, rax);                // call non-virtual member function
	}
	else if (spacesizep.size() == SIZE_BYTE)
	{
		mov_reg_param(a, Gpd(REG_PARAM4), maskp);
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_byte_masked.obj);
		smart_call_r64(a, accessors.resolved.write_byte_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		mov_reg_param(a, Gpd(REG_PARAM4), maskp);
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_word_masked.obj);
		smart_call_r64(a, accessors.resolved.write_word_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_DWORD)
	{
		mov_reg_param(a, Gpd(REG_PARAM4), maskp);
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_dword_masked.obj);
		smart_call_r64(a, accessors.resolved.write_dword_masked.func, rax);
	}
	else if (spacesizep.size() == SIZE_QWORD)
	{
		mov_reg_param(a, Gpq(REG_PARAM4), maskp);
		mov_r64_imm(a, Gpq(REG_PARAM1), accessors.resolved.write_qword_masked.obj);
		smart_call_r64(a, accessors.resolved.write_qword_masked.func, rax);
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

	// degenerate case: source is immediate
	if (srcp.is_immediate() && bitp.is_immediate())
	{
		if (srcp.immediate() & ((uint64_t)1 << (bitp.immediate() & (inst.size() * 8 - 1))))
			a.stc();
		else
			a.clc();

		return;
	}

	// load non-immediate bit numbers into a register

	Gp const bitreg = rcx;
	if (!bitp.is_immediate())
	{
		mov_reg_param(a, bitreg, bitp);
		a.and_(bitreg, inst.size() * 8 - 1);
	}

	if (srcp.is_memory())
	{
		if (bitp.is_immediate())
			a.bt(MABS(srcp.memory(), inst.size()), (bitp.immediate() & (inst.size() * 8 - 1)));
		else
			a.bt(MABS(srcp.memory(), inst.size()), bitreg);
	}
	else if (srcp.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId(RegType::kX86_Gpq, srcp.ireg());
		if (bitp.is_immediate())
			a.bt(src, (bitp.immediate() & (inst.size() * 8 - 1)));
		else
			a.bt(src, bitreg);
	}
	else if (srcp.is_immediate())
	{
		a.push(rax);
		mov_reg_param(a, rax, srcp);
		a.bt(rax, bitreg);
		a.pop(rax);
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
	Label skip;
	if (inst.condition() != uml::COND_ALWAYS && !(dstp.is_int_register() && !srcp.is_immediate()))
	{
		skip = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);
	}

	// register to memory
	if (dstp.is_memory() && srcp.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, srcp.ireg());

		a.mov(MABS(dstp.memory()), src);
	}
	// immediate to memory
	else if (dstp.is_memory() && srcp.is_immediate() && short_immediate(srcp.immediate()))
		a.mov(MABS(dstp.memory(), inst.size()), s32(srcp.immediate()));

	// conditional memory to register
	else if (dstp.is_int_register() && srcp.is_memory())
	{
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());

		if (inst.condition() != uml::COND_ALWAYS)
			a.cmov(X86_CONDITION(inst.condition()), dst, MABS(srcp.memory()));
		else
			a.mov(dst, MABS(srcp.memory()));
	}

	// conditional register to register
	else if (dstp.is_int_register() && srcp.is_int_register())
	{
		Gp const src = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, srcp.ireg());
		Gp const dst = Gp::fromTypeAndId((inst.size() == 4) ? RegType::kX86_Gpd : RegType::kX86_Gpq, dstp.ireg());

		if (inst.condition() != uml::COND_ALWAYS)
			a.cmov(X86_CONDITION(inst.condition()), dst, src);
		else
			a.mov(dst, src);
	}

	// general case
	else
	{
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax) : dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp, true);
		mov_param_reg(a, dstp, dstreg);
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
				a.movsx(dstreg, MABS(srcp.memory(), 1));
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, MABS(srcp.memory(), 2));
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, MABS(srcp.memory()));
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, GpbLo(srcp.ireg()));
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, Gpw(srcp.ireg()));
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, Gpd(srcp.ireg()));
		}
		else if (srcp.is_immediate())
		{
			if (sizep.size() == SIZE_BYTE)
				a.mov(dstreg, (int8_t)srcp.immediate());
			else if (sizep.size() == SIZE_WORD)
				a.mov(dstreg, (int16_t)srcp.immediate());
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, (int32_t)srcp.immediate());
		}

		mov_param_reg(a, dstp, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// general case
		if (srcp.is_memory())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, MABS(srcp.memory(), 1));
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, MABS(srcp.memory(), 2));
			else if (sizep.size() == SIZE_DWORD)
				a.movsxd(dstreg, MABS(srcp.memory(), 4));
			else if (sizep.size() == SIZE_QWORD)
				a.mov(dstreg, MABS(srcp.memory()));
		}
		else if (srcp.is_int_register())
		{
			if (sizep.size() == SIZE_BYTE)
				a.movsx(dstreg, GpbLo(srcp.ireg()));
			else if (sizep.size() == SIZE_WORD)
				a.movsx(dstreg, Gpw(srcp.ireg()));
			else if (sizep.size() == SIZE_DWORD)
				a.movsxd(dstreg, Gpd(srcp.ireg()));
			else if (sizep.size() == SIZE_QWORD)
				a.mov(dstreg, Gpq(srcp.ireg()));
		}
		else if (srcp.is_immediate())
		{
			if (sizep.size() == SIZE_BYTE)
				a.mov(dstreg, (int8_t)srcp.immediate());
			else if (sizep.size() == SIZE_WORD)
				a.mov(dstreg, (int16_t)srcp.immediate());
			else if (sizep.size() == SIZE_DWORD)
				a.mov(dstreg, (int32_t)srcp.immediate());
			else if (sizep.size() == SIZE_QWORD)
				a.mov(dstreg, (int64_t)srcp.immediate());
		}

		mov_param_reg(a, dstp, dstreg);
	}

	if (inst.flags() != 0)
		a.test(dstreg, dstreg);
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

	mov_reg_param(a, dstreg, srcp);
	shift_op_param(a, Inst::kIdRol, inst.size(), dstreg, shiftp, false);
	alu_op_param(a, Inst::kIdAnd, dstreg, maskp,
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

		mov_reg_param(a, eax, srcp);
		shift_op_param(a, Inst::kIdRol, inst.size(), eax, shiftp, false);
		mov_reg_param(a, dstreg, dstp);
		if (maskp.is_immediate())
		{
			a.and_(eax, maskp.immediate());
			a.and_(dstreg, ~maskp.immediate());
		}
		else
		{
			mov_reg_param(a, edx, maskp);
			a.and_(eax, edx);
			a.not_(edx);
			a.and_(dstreg, edx);
		}
		a.or_(dstreg, eax);
		mov_param_reg(a, dstp, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(rcx, shiftp, maskp);

		mov_reg_param(a, rax, srcp);
		mov_reg_param(a, rdx, maskp);
		shift_op_param(a, Inst::kIdRol, inst.size(), rax, shiftp, false);
		mov_reg_param(a, dstreg, dstp);
		a.and_(rax, rdx);
		a.not_(rdx);
		a.and_(dstreg, rdx);
		a.or_(dstreg, rax);
		mov_param_reg(a, dstp, dstreg);
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
		Gp dstreg = eax;
		Gp edstreg = edx;

		// general case
		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 4));
		else if (src2p.is_int_register())
			a.mul(Gpd(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			a.mov(edstreg, src2p.immediate());
			a.mul(edstreg);
		}
		mov_param_reg(a, dstp, dstreg);
		if (compute_hi)
			mov_param_reg(a, edstp, edstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = rax;
		Gp edstreg = rdx;

		// general case
		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 8));
		else if (src2p.is_int_register())
			a.mul(Gpq(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			mov_r64_imm(a, edstreg, src2p.immediate());
			a.mul(edstreg);
		}
		mov_param_reg(a, dstp, dstreg);
		if (compute_hi)
			mov_param_reg(a, edstp, edstreg);
	}

	if (inst.flags())
		calculate_status_flags_mul(a, inst.size(), rax, rdx);
}


//-------------------------------------------------
//  op_mululw - process a MULULW (32x32=32) opcode
//-------------------------------------------------

void drcbe_x64::op_mululw(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		Gp dstreg = eax;
		Gp hireg = edx;

		// general case
		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 4));
		else if (src2p.is_int_register())
			a.mul(Gpd(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			a.mov(hireg, src2p.immediate());
			a.mul(hireg);
		}
		mov_param_reg(a, dstp, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = rax;
		Gp hireg = rdx;

		// general case
		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.mul(MABS(src2p.memory(), 8));
		else if (src2p.is_int_register())
			a.mul(Gpq(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			mov_r64_imm(a, hireg, src2p.immediate());
			a.mul(hireg);
		}
		mov_param_reg(a, dstp, dstreg);
	}

	if (inst.flags())
		calculate_status_flags_mul_low(a, inst.size(), rax);
}


//-------------------------------------------------
//  op_muls - process a MULS opcode
//-------------------------------------------------

void drcbe_x64::op_muls(Assembler &a, const instruction &inst)
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
	normalize_commutative(src1p, src2p);
	bool compute_hi = (dstp != edstp);

	// 32-bit form
	if (inst.size() == 4)
	{
		Gp dstreg = eax;
		Gp edstreg = edx;

		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.imul(MABS(src2p.memory(), 4));
		else if (src2p.is_int_register())
			a.imul(Gpd(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			a.mov(edstreg, src2p.immediate());
			a.imul(edstreg);
		}
		mov_param_reg(a, dstp, dstreg);
		if (compute_hi)
			mov_param_reg(a, edstp, edstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = rax;
		Gp edstreg = rdx;

		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.imul(MABS(src2p.memory(), 8));
		else if (src2p.is_int_register())
			a.imul(Gpq(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			mov_r64_imm(a, edstreg, src2p.immediate());
			a.imul(edstreg);
		}
		mov_param_reg(a, dstp, dstreg);
		if (compute_hi)
			mov_param_reg(a, edstp, edstreg);
	}

	if (inst.flags())
		calculate_status_flags_mul(a, inst.size(), rax, rdx);
}


//-------------------------------------------------
//  op_mulslw - process a MULSLW (32x32=32) opcode
//-------------------------------------------------

void drcbe_x64::op_mulslw(Assembler &a, const instruction &inst)
{
	// validate instruction
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	// normalize parameters
	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);
	normalize_commutative(src1p, src2p);

	// 32-bit form
	if (inst.size() == 4)
	{
		Gp dstreg = eax;
		Gp hireg = edx;

		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.imul(MABS(src2p.memory(), 4));
		else if (src2p.is_int_register())
			a.imul(Gpd(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			a.mov(hireg, src2p.immediate());
			a.imul(hireg);
		}
		mov_param_reg(a, dstp, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = rax;
		Gp hireg = rdx;

		mov_reg_param(a, dstreg, src1p);
		if (src2p.is_memory())
			a.imul(MABS(src2p.memory(), 8));
		else if (src2p.is_int_register())
			a.imul(Gpq(src2p.ireg()));
		else if (src2p.is_immediate())
		{
			mov_r64_imm(a, hireg, src2p.immediate());
			a.imul(hireg);
		}
		mov_param_reg(a, dstp, dstreg);
	}

	if (inst.flags())
		calculate_status_flags_mul_low(a, inst.size(), rax);
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

	if (inst.flags())
	{
		a.xor_(eax, eax); // reset status flags
		a.test(eax, eax);
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(eax);

		mov_reg_param(a, dstreg, srcp);
		a.mov(ecx, 32 ^ 31);
		a.bsr(dstreg, dstreg);
		a.cmovz(dstreg, ecx);
		a.xor_(dstreg, 31);
		mov_param_reg(a, dstp, dstreg);

		if (inst.flags())
			a.test(dstreg, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		// pick a target register
		Gp dstreg = dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp);
		a.mov(ecx, 64 ^ 63);
		a.bsr(dstreg, dstreg);
		a.cmovz(dstreg, rcx);
		a.xor_(dstreg, 63);
		mov_param_reg(a, dstp, dstreg);

		if (inst.flags())
			a.test(dstreg, dstreg);
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

	if (inst.flags())
	{
		a.xor_(eax, eax); // reset status flags
		a.test(eax, eax);
	}

	// 32-bit form
	if (inst.size() == 4)
	{
		Gp dstreg = dstp.select_register(eax);

		mov_reg_param(a, dstreg, srcp);
		a.mov(ecx, 32);
		a.bsf(dstreg, dstreg);
		a.cmovz(dstreg, ecx);
		mov_param_reg(a, dstp, dstreg);
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		Gp dstreg = dstp.select_register(rax);

		mov_reg_param(a, dstreg, srcp);
		a.mov(rcx, 64);
		a.bsf(dstreg, dstreg);
		a.cmovz(dstreg, rcx);
		mov_param_reg(a, dstp, dstreg);
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
	if (!carry && !inst.flags() && src2p.is_immediate() && (src2p.immediate() & (inst.size() * 8 - 1)) == 0)
		return;

	// dstp == src1p in memory
	if (dstp.is_memory() && dstp == src1p)
		shift_op_param(a, Opcode, inst.size(), MABS(dstp.memory(), inst.size()), src2p, true);

	// general case
	else
	{
		// pick a target register
		Gp dstreg = (inst.size() == 4) ? dstp.select_register(eax, src2p) : dstp.select_register(rax, src2p);

		if (carry)
			mov_reg_param(a, dstreg, src1p, true);
		else
			mov_reg_param(a, dstreg, src1p);
		shift_op_param(a, Opcode, inst.size(), dstreg, src2p, true);
		mov_param_reg(a, dstp, dstreg);
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
	auto const &accessors = m_memory_accessors[spacep.space()];
	auto const &accessor = (inst.size() == 4) ? accessors.resolved.read_dword : accessors.resolved.read_qword;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);
	mov_r64_imm(a, Gpq(REG_PARAM1), accessor.obj);
	smart_call_r64(a, accessor.func, rax);

	// store result
	if (inst.size() == 4)
	{
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory()), eax);
		else if (dstp.is_float_register())
			a.movd(Xmm(dstp.freg()), eax);
	}
	else if (inst.size() == 8)
	{
		if (dstp.is_memory())
			a.mov(MABS(dstp.memory()), rax);
		else if (dstp.is_float_register())
			a.movq(Xmm(dstp.freg()), rax);
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
	auto const &accessors = m_memory_accessors[spacep.space()];
	auto const &accessor = (inst.size() == 4) ? accessors.resolved.write_dword : accessors.resolved.write_qword;
	mov_reg_param(a, Gpd(REG_PARAM2), addrp);

	if (inst.size() == 4)
	{
		// 32-bit form
		if (srcp.is_memory())
			a.mov(Gpd(REG_PARAM3), MABS(srcp.memory()));
		else if (srcp.is_float_register())
			a.movd(Gpd(REG_PARAM3), Xmm(srcp.freg()));
	}
	else if (inst.size() == 8)
	{
		// 64-bit form
		if (srcp.is_memory())
			a.mov(Gpq(REG_PARAM3), MABS(srcp.memory()));
		else if (srcp.is_float_register())
			a.movq(Gpq(REG_PARAM3), Xmm(srcp.freg()));
	}
	mov_r64_imm(a, Gpq(REG_PARAM1), accessor.obj);
	smart_call_r64(a, accessor.func, rax);
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
	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();
		a.short_().j(X86_NOT_CONDITION(inst.condition()), skip);                        // jcc   skip
	}

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

	Xmm dstreg = dstp.select_register(xmm0, srcp);
	Xmm tempreg = xmm1;

	// note: using memory addrs with xorpd is dangerous because MAME does not guarantee
	// the memory address will be 16 byte aligned so there's a good chance it'll crash
	if (srcp.is_memory())
	{
		if (inst.size() == 4)
		{
			a.mov(eax, MABS(srcp.memory()));
			a.movd(tempreg, eax);
		}
		else if (inst.size() == 8)
		{
			a.mov(rax, MABS(srcp.memory()));
			a.movq(tempreg, rax);
		}
	}

	if (inst.size() == 4)
	{
		a.mov(rax, 0x80000000);
		a.movd(dstreg, rax);
		if (srcp.is_memory())
			a.xorpd(dstreg, tempreg);
		else if (srcp.is_float_register())
			a.xorpd(dstreg, Xmm(srcp.freg()));
		movss_p32_r128(a, dstp, dstreg);
	}
	else if (inst.size() == 8)
	{
		a.mov(rax, 0x8000000000000000);
		a.movq(dstreg, rax);
		if (srcp.is_memory())
			a.xorpd(dstreg, tempreg);
		else if (srcp.is_float_register())
			a.xorpd(dstreg, Xmm(srcp.freg()));
		movsd_p64_r128(a, dstp, dstreg);
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
			a.movd(dstreg, MABS(srcp.memory()));
			movss_p32_r128(a, dstp, dstreg);
		}
		else if (dstp.is_memory())
		{
			mov_param_reg(a, dstp, Gpd(srcp.ireg()));
		}
		else
		{
			a.movd(dstreg, Gpd(srcp.ireg()));
			movss_p32_r128(a, dstp, dstreg);
		}

	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			a.movq(dstreg, MABS(srcp.memory()));
			movsd_p64_r128(a, dstp, dstreg);
		}
		else if (dstp.is_memory())
		{
			mov_param_reg(a, dstp, Gpq(srcp.ireg()));
		}
		else
		{
			a.movq(dstreg, Gpq(srcp.ireg()));
			movsd_p64_r128(a, dstp, dstreg);
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
			a.mov(dstreg, MABS(srcp.memory()));
			mov_param_reg(a, dstp, dstreg);
		}
		else if (dstp.is_memory())
		{
			a.movd(MABS(dstp.memory()), Xmm(srcp.freg()));
		}
		else
		{
			a.movd(Gpd(dstp.ireg()), Xmm(srcp.freg()));
		}
	}

	// 64-bit form
	else if (inst.size() == 8)
	{
		if (srcp.is_memory())
		{
			Gp dstreg = dstp.select_register(rax);
			a.mov(dstreg, MABS(srcp.memory()));
			mov_param_reg(a, dstp, dstreg);
		}
		else if (dstp.is_memory())
		{
			a.movq(MABS(dstp.memory()), Xmm(srcp.freg()));
		}
		else
		{
			a.movq(Gpq(dstp.ireg()), Xmm(srcp.freg()));
		}
	}
}

} // anonymous namespace


std::unique_ptr<drcbe_interface> make_drcbe_x64(
		drcuml_state &drcuml,
		device_t &device,
		drc_cache &cache,
		uint32_t flags,
		int modes,
		int addrbits,
		int ignorebits)
{
	return std::unique_ptr<drcbe_interface>(new drcbe_x64(drcuml, device, cache, flags, modes, addrbits, ignorebits));
}

} // namespace drc
