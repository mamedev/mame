// license:BSD-3-Clause
// copyright-holders:windyfairy, Vas Crabb
/***************************************************************************

Register use:

r0      parameter/result    first function parameter/return value
r1      parameter/result    second function parameter
r2      parameter/result    third function parameter
r3      parameter/result    fourth function parameter
r4      parameter/result
r5      parameter/result
r6      parameter/result
r7      parameter/result
r8      result pointer
r9                          temporary for intermediate values
r10                         temporary for intermediate values
r11                         temporary for intermediate values
r12
r13
r14                         scratch register used for address calculation
r15                         temporary used in opcode functions
r16     intra-call scratch  scratch register used by helper functions
r17     intra-call scratch  scratch register used by helper functions
r18     platform register
r19     callee-saved        UML register I0
r20     callee-saved        UML register I1
r21     callee-saved        UML register I2
r22     callee-saved        UML register I3
r23     callee-saved        UML register I4
r24     callee-saved        UML register I5
r35     callee-saved        UML register I6
r26     callee-saved        UML register I7
r27     callee-saved        near cache pointer
r28     callee-saved        emulated flags
r29     frame pointer       base generated code frame pointer
r30     link register
sp      stack pointer

v0      parameter/result
v1      parameter/result
v2      parameter/result
v3      parameter/result
v4      parameter/result
v5      parameter/result
v6      parameter/result
v7      parameter/result
v8                          UML register F0
v9                          UML register F1
v10                         UML register F2
v11                         UML register F3
v12                         UML register F4
v13                         UML register F5
v14                         UML register F6
v15                         UML register F7
v16                         temporary for intermediate values
v17                         temporary for intermediate values
v18                         temporary for intermediate values
v19
v20
v21
v22
v23
v24
v25
v26
v27
v28
v29
v30
v31


Stack layout in top-level generated code frame:

FP -> SP + 0x00  previous FP
      SP + 0x08  top-level return address
      SP + 0x10  saved non-volatile registers
      SP + 0x18  ...

Stack layout in nested generated code subroutine call frame:

SP -> SP + 0x00  saved FP
      SP + 0x08  return address
      ...
      FP - 0x10  saved FP
      FP - 0x08  return address
FP -> FP + 0x00  previous FP
      FP + 0x08  top-level return address

The frame pointer (FP or x29) is only updated by the top-level generated
code entry point.  Generated code subroutines (called using CALLH, EXH or
on a failed HASHJMP) push FP and LR onto the stack but do not update FP.
All the saved FP values will be identical.

A native debugger following the FP chain will see any number of nested
generated code subroutine call frames as a single stack frame.  The return
addresses and duplicate saved FP values for the generated code subroutine
calls will appear as the local variable area of the frame.

You can calculate the generated code subroutine call depth as
(FP - SP) / 0x10.  You can see the return addresses for the generated code
subroutine calls at SP + 0x08, SP + 0x18, SP + 0x28, etc. until reaching
the location FP points to.

TODO:
* Some operations do not clear the upper bits of integer registers when
  they should (e.g. 32-bit MOV with the same register as source and
  destination).

***************************************************************************/

#include "emu.h"
#include "drcbearm64.h"

#include "drcbeut.h"
#include "uml.h"

#include "debug/debugcpu.h"
#include "emuopts.h"

#include "mfpresolve.h"

#include "asmjit/src/asmjit/a64.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <vector>


namespace drc {

namespace {

using namespace uml;

using namespace asmjit;

const uint32_t PTYPE_M   = 1 << parameter::PTYPE_MEMORY;
const uint32_t PTYPE_I   = 1 << parameter::PTYPE_IMMEDIATE;
const uint32_t PTYPE_R   = 1 << parameter::PTYPE_INT_REGISTER;
const uint32_t PTYPE_F   = 1 << parameter::PTYPE_FLOAT_REGISTER;
const uint32_t PTYPE_MR  = PTYPE_M | PTYPE_R;
const uint32_t PTYPE_MRI = PTYPE_M | PTYPE_R | PTYPE_I;
const uint32_t PTYPE_MF  = PTYPE_M | PTYPE_F;

// Pass/receive value registers
const a64::Gp REG_PARAM1 = a64::x0;
const a64::Gp REG_PARAM2 = a64::x1;
const a64::Gp REG_PARAM3 = a64::x2;
const a64::Gp REG_PARAM4 = a64::x3;

// Stable registers that can be assumed to be unchanged by internal functions
const a64::Gp TEMP_REG1 = a64::x9;
const a64::Gp TEMP_REG2 = a64::x10;
const a64::Gp TEMP_REG3 = a64::x11;

// Temporary registers that should not be assumed to live between functions
const a64::Gp SCRATCH_REG1 = a64::x16;
const a64::Gp SCRATCH_REG2 = a64::x17;

// Temporary memory calculation register, should not be used outside of functions that calculate memory addresses
const a64::Gp MEM_SCRATCH_REG = a64::x14;

// Only to be used in an opcode level function. Should not be used in helper functions
const a64::Gp FUNC_SCRATCH_REG = a64::x15;

const a64::Vec TEMPF_REG1 = a64::d16;
const a64::Vec TEMPF_REG2 = a64::d17;
const a64::Vec TEMPF_REG3 = a64::d18;

// Base memory address
const a64::Gp BASE_REG = a64::x27;

// Software emulated flags (bit 0 = FLAG_C, bit 4 = FLAG_U)
const a64::Gp FLAGS_REG = a64::x28;

const a64::Gp::Id int_register_map[REG_I_COUNT] =
{
	a64::Gp::Id(a64::x19.id()),
	a64::Gp::Id(a64::x20.id()),
	a64::Gp::Id(a64::x21.id()),
	a64::Gp::Id(a64::x22.id()),
	a64::Gp::Id(a64::x23.id()),
	a64::Gp::Id(a64::x24.id()),
	a64::Gp::Id(a64::x25.id()),
	a64::Gp::Id(a64::x26.id()),
};

const a64::Gp::Id float_register_map[REG_F_COUNT] =
{
	a64::Gp::Id(a64::d8.id()),
	a64::Gp::Id(a64::d9.id()),
	a64::Gp::Id(a64::d10.id()),
	a64::Gp::Id(a64::d11.id()),
	a64::Gp::Id(a64::d12.id()),
	a64::Gp::Id(a64::d13.id()),
	a64::Gp::Id(a64::d14.id()),
	a64::Gp::Id(a64::d15.id()),
};

// condition mapping table
const a64::CondCode condition_map[uml::COND_MAX - uml::COND_Z] =
{
	a64::CondCode::kEQ,    // COND_Z = 0x80,    requires Z COND_E
	a64::CondCode::kNE,    // COND_NZ,          requires Z COND_NE
	a64::CondCode::kMI,    // COND_S,           requires S
	a64::CondCode::kPL,    // COND_NS,          requires S
	a64::CondCode::kLO,    // COND_C,           requires C COND_B
	a64::CondCode::kHS,    // COND_NC,          requires C COND_AE
	a64::CondCode::kVS,    // COND_V,           requires V
	a64::CondCode::kVC,    // COND_NV,          requires V
	a64::CondCode::kAL,    // COND_U,           requires U (emulated in software)
	a64::CondCode::kAL,    // COND_NU,          requires U (emulated in software)
	a64::CondCode::kHI,    // COND_A,           requires CZ
	a64::CondCode::kLS,    // COND_BE,          requires CZ
	a64::CondCode::kGT,    // COND_G,           requires SVZ
	a64::CondCode::kLE,    // COND_LE,          requires SVZ
	a64::CondCode::kLT,    // COND_L,           requires SV
	a64::CondCode::kGE,    // COND_GE,          requires SV
};

// masks for immediate values that can be generated with movz instructions
constexpr uint64_t LSL0_MASK = 0x00000000'0000ffff;
constexpr uint64_t LSL16_MASK = 0x00000000'ffff0000;
constexpr uint64_t LSL32_MASK = 0x0000ffff'00000000;
constexpr uint64_t LSL48_MASK = 0xffff0000'00000000;


#define ARM_CONDITION(condition)        (condition_map[condition - COND_Z])
#define ARM_NOT_CONDITION(condition)    (negate_cond(condition_map[condition - COND_Z]))

#define assert_no_condition(inst)       assert((inst).condition() == uml::COND_ALWAYS)
#define assert_any_condition(inst)      assert((inst).condition() == uml::COND_ALWAYS || ((inst).condition() >= uml::COND_Z && (inst).condition() < uml::COND_MAX))
#define assert_no_flags(inst)           assert((inst).flags() == 0)
#define assert_flags(inst, valid)       assert(((inst).flags() & ~(valid)) == 0)


class ThrowableErrorHandler : public ErrorHandler
{
public:
	virtual void handle_error(Error err, const char *message, BaseEmitter *origin) override
	{
		throw emu_fatalerror("asmjit error %u: %s", std::underlying_type_t<Error>(err), message);
	}
};


// helper functions

inline a64::Vec select_register(a64::Vec const &reg, uint32_t regsize)
{
	if (regsize == 4)
		return reg.s();
	return reg.d();
}

inline a64::Gp select_register(a64::Gp const &reg, uint32_t regsize)
{
	if (regsize == 4)
		return reg.w();
	return reg.x();
}

inline bool is_valid_immediate_mask(uint64_t val, size_t bytes)
{
	const unsigned bits = bytes * 8;

	// all zeros and all ones aren't allowed, and disallow any value with bits outside of the max bit range
	if ((val == 0) || (val >= util::make_bitmask<uint64_t>(bits)))
		return false;

	// work out if the value is repeating sequence of a power-of-two bit group
	unsigned width = 2;
	uint64_t mask = util::make_bitmask<uint64_t>(bits - width);
	while ((width < bits) && ((val & mask) != (val >> width)))
	{
		mask >>= width;
		width <<= 1;
	}

	// check check that set bits are contiguous
	const auto lz = count_leading_zeros_64(val & make_bitmask<uint64_t>(width));
	const uint64_t invleftaligned = ~(val << lz);
	return !(invleftaligned & (invleftaligned + 1));
}

inline bool is_valid_immediate(uint64_t val, size_t bits)
{
	assert(bits < 64);
	return val < (uint64_t(1) << bits);
}

constexpr bool is_valid_immediate_addsub(uint64_t val)
{
	// 12-bit unsigned immediate value, optionally left-shifted by 12 bits
	return !(val & ~util::make_bitmask<uint64_t>(12)) || !(val & ~(util::make_bitmask<uint64_t>(12) << 12));
}

inline constexpr bool is_valid_immediate_signed(int64_t val, size_t bits)
{
	return util::sext(val, bits) == val;
}

inline constexpr bool is_valid_offset(int64_t diff, int max_shift)
{
	if (is_valid_immediate_signed(diff, 9))
		return true; // 9-bit signed offset
	else if ((diff >= 0) && (diff < (1 << (12 + max_shift))) && !(diff & make_bitmask<int64_t>(max_shift)))
		return true; // 12-bit unsigned offset shifted by operand size
	else
		return false;
}

inline bool is_simple_mov_immediate(uint64_t val, size_t bytes)
{
	if (!(val & ~LSL0_MASK) || !(val & ~LSL16_MASK) || !(val & ~LSL32_MASK) || !(val & ~LSL48_MASK))
		return true; // movz
	else if (!(~val & ~LSL0_MASK) || !(~val & ~LSL16_MASK) || !(~val & ~LSL32_MASK) || !(~val & ~LSL48_MASK))
		return true; // movn
	else if ((val == uint32_t(val)) && (((val & LSL0_MASK) == LSL0_MASK) || ((val & LSL16_MASK) == LSL16_MASK)))
		return true; // movn to w register
	else if (is_valid_immediate_mask(val, bytes))
		return true; // orr with zero register
	else
		return false;
}

inline bool emit_add_optimized(a64::Assembler &a, const a64::Gp &dst, const a64::Gp &src, int64_t val)
{
	// If the bottom 12 bits are 0s then an optimized form can be used if the remaining bits are <= 12
	if (is_valid_immediate_addsub(val))
	{
		a.add(dst, src, val);
		return true;
	}

	return false;
}

inline bool emit_sub_optimized(a64::Assembler &a, const a64::Gp &dst, const a64::Gp &src, int64_t val)
{
	if (val < 0)
		val = -val;

	// If the bottom 12 bits are 0s then an optimized form can be used if the remaining bits are <= 12
	if (is_valid_immediate_addsub(val))
	{
		a.sub(dst, src, val);
		return true;
	}

	return false;
}

void get_imm_absolute(a64::Assembler &a, const a64::Gp &reg, const uint64_t val)
{
	// Check for constants that can be generated with a single instruction
	if (is_simple_mov_immediate(val, reg.size()))
	{
		a.mov(reg, val);
		return;
	}
	else if (reg.is_gp64() && is_valid_immediate_mask(val, 4))
	{
		a.mov(reg.w(), val); // asmjit isn't smart enough to work this out
		return;
	}

	// Values close to the program counter can be generated with a single adr
	const uint64_t codeoffs = a.code()->base_address() + a.offset();
	const int64_t reloffs = int64_t(val) - codeoffs;
	if (is_valid_immediate_signed(reloffs, 21))
	{
		a.adr(reg, val);
		return;
	}

	// Values within 4G of the program counter can be generated with adrp followed by add
	const uint64_t pagebase = codeoffs & ~make_bitmask<uint64_t>(12);
	const int64_t pagerel = int64_t(val) - pagebase;
	if (is_valid_immediate_signed(pagerel, 21 + 12))
	{
		const uint64_t targetpage = val & ~make_bitmask<uint64_t>(12);
		const uint64_t pageoffs = val & util::make_bitmask<uint64_t>(12);

		a.adrp(reg.x(), targetpage);
		if (pageoffs != 0)
			a.add(reg, reg, pageoffs);

		return;
	}

	// up to four instructions
	a.mov(reg, val);
}

inline void get_unordered(a64::Assembler &a, const a64::Gp &reg)
{
	a.ubfx(reg.x(), FLAGS_REG, uml::FLAG_BIT_U, 1);
}

inline void store_carry_reg(a64::Assembler &a, const a64::Gp &reg, unsigned bit = 0)
{
	// this depends on carry being the least significant bit of the flags
	a.bfxil(FLAGS_REG, reg.x(), bit, 1);
}

inline void get_carry(a64::Assembler &a, const a64::Gp &reg, bool inverted = false)
{
	a.and_(reg.x(), FLAGS_REG, 1);

	if (inverted)
		a.eor(reg.x(), reg.x(), 1);
}


class drcbe_arm64 : public drcbe_interface
{
public:
	drcbe_arm64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_arm64();

	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst) override;
	virtual bool hash_exists(uint32_t mode, uint32_t pc) const noexcept override;
	virtual void get_info(drcbe_info &info) const noexcept override;
	virtual bool logging() const noexcept override { return false; }

private:
	enum class carry_state
	{
		POISON,     // does not correspond to UML carry flag
		CANONICAL,  // corresponds directly to UML carry flag
		LOGICAL     // logical borrow state
	};

	class be_parameter
	{
	public:
		// parameter types
		enum be_parameter_type
		{
			PTYPE_NONE = 0,        // invalid
			PTYPE_IMMEDIATE,       // immediate; value = sign-extended to 64 bits
			PTYPE_INT_REGISTER,    // integer register; value = 0-REG_MAX
			PTYPE_FLOAT_REGISTER,  // floating point register; value = 0-REG_MAX
			PTYPE_MEMORY,          // memory; value = pointer to memory
			PTYPE_MAX
		};

		typedef uint64_t be_parameter_value;

		be_parameter() : m_type(PTYPE_NONE), m_value(0), m_coldreg(false) { }
		be_parameter(uint64_t val) : m_type(PTYPE_IMMEDIATE), m_value(val), m_coldreg(false) { }
		be_parameter(drcbe_arm64 &drcbe, const uml::parameter &param, uint32_t allowed);
		be_parameter(const be_parameter &param) = default;

		static be_parameter make_ireg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_INT_REGISTER, regnum); }
		static be_parameter make_freg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static be_parameter make_memory(void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(base)); }
		static be_parameter make_memory(const void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(const_cast<void *>(base))); }

		bool operator==(const be_parameter &rhs) const { return (m_type == rhs.m_type) && (m_value == rhs.m_value); }
		bool operator!=(const be_parameter &rhs) const { return (m_type != rhs.m_type) || (m_value != rhs.m_value); }

		be_parameter_type type() const { return m_type; }
		uint64_t immediate() const { assert(m_type == PTYPE_IMMEDIATE); return m_value; }
		uint32_t ireg() const { assert(m_type == PTYPE_INT_REGISTER); assert(m_value < REG_MAX); return m_value; }
		uint32_t freg() const { assert(m_type == PTYPE_FLOAT_REGISTER); assert(m_value < REG_MAX); return m_value; }
		void *memory() const { assert(m_type == PTYPE_MEMORY); return reinterpret_cast<void *>(m_value); }

		bool is_immediate() const { return (m_type == PTYPE_IMMEDIATE); }
		bool is_int_register() const { return (m_type == PTYPE_INT_REGISTER); }
		bool is_float_register() const { return (m_type == PTYPE_FLOAT_REGISTER); }
		bool is_memory() const { return (m_type == PTYPE_MEMORY); }

		bool is_immediate_value(uint64_t value) const { return (m_type == PTYPE_IMMEDIATE && m_value == value); }
		bool is_cold_register() const { return m_coldreg; }

		a64::Vec get_register_float(uint32_t regsize) const;
		a64::Gp get_register_int(uint32_t regsize) const;
		a64::Vec select_register(a64::Vec const &reg, uint32_t regsize) const;
		a64::Gp select_register(a64::Gp const &reg, uint32_t regsize) const;

	private:
		static inline constexpr int REG_MAX = 30;

		be_parameter(be_parameter_type type, be_parameter_value value) : m_type(type), m_value(value), m_coldreg(false) { }

		be_parameter_type   m_type;
		be_parameter_value  m_value;
		bool                m_coldreg;
	};

	struct near_state
	{
		uint64_t saved_fpcr;
	};

	struct memory_accessors
	{
		resolved_memory_accessors resolved;
		address_space::specific_access_info specific;
		offs_t address_mask;
		u8 high_bits;
		bool no_mask;
		bool mask_simple;
		bool mask_high_bits;
	};

	using arm64_entry_point_func = uint32_t (*)(void *entry);

	void generate_one(a64::Assembler &a, const uml::instruction &inst);

	void op_handle(a64::Assembler &a, const uml::instruction &inst);
	void op_hash(a64::Assembler &a, const uml::instruction &inst);
	void op_label(a64::Assembler &a, const uml::instruction &inst);
	void op_comment(a64::Assembler &a, const uml::instruction &inst);
	void op_mapvar(a64::Assembler &a, const uml::instruction &inst);

	void op_nop(a64::Assembler &a, const uml::instruction &inst);
	void op_break(a64::Assembler &a, const uml::instruction &inst);
	void op_debug(a64::Assembler &a, const uml::instruction &inst);
	void op_exit(a64::Assembler &a, const uml::instruction &inst);
	void op_hashjmp(a64::Assembler &a, const uml::instruction &inst);
	void op_jmp(a64::Assembler &a, const uml::instruction &inst);
	void op_exh(a64::Assembler &a, const uml::instruction &inst);
	void op_callh(a64::Assembler &a, const uml::instruction &inst);
	void op_ret(a64::Assembler &a, const uml::instruction &inst);
	void op_callc(a64::Assembler &a, const uml::instruction &inst);
	void op_recover(a64::Assembler &a, const uml::instruction &inst);

	void op_setfmod(a64::Assembler &a, const uml::instruction &inst);
	void op_getfmod(a64::Assembler &a, const uml::instruction &inst);
	void op_getexp(a64::Assembler &a, const uml::instruction &inst);
	void op_getflgs(a64::Assembler &a, const uml::instruction &inst);
	void op_setflgs(a64::Assembler &a, const uml::instruction &inst);
	void op_save(a64::Assembler &a, const uml::instruction &inst);
	void op_restore(a64::Assembler &a, const uml::instruction &inst);

	void op_load(a64::Assembler &a, const uml::instruction &inst);
	void op_loads(a64::Assembler &a, const uml::instruction &inst);
	void op_store(a64::Assembler &a, const uml::instruction &inst);
	void op_read(a64::Assembler &a, const uml::instruction &inst);
	void op_readm(a64::Assembler &a, const uml::instruction &inst);
	void op_write(a64::Assembler &a, const uml::instruction &inst);
	void op_writem(a64::Assembler &a, const uml::instruction &inst);
	void op_carry(a64::Assembler &a, const uml::instruction &inst);
	void op_set(a64::Assembler &a, const uml::instruction &inst);
	void op_mov(a64::Assembler &a, const uml::instruction &inst);
	void op_sext(a64::Assembler &a, const uml::instruction &inst);
	void op_roland(a64::Assembler &a, const uml::instruction &inst);
	void op_rolins(a64::Assembler &a, const uml::instruction &inst);
	template <bool CarryIn> void op_add(a64::Assembler &a, const uml::instruction &inst);
	template <bool CarryIn> void op_sub(a64::Assembler &a, const uml::instruction &inst);
	void op_cmp(a64::Assembler &a, const uml::instruction &inst);
	void op_mulu(a64::Assembler &a, const uml::instruction &inst);
	void op_mululw(a64::Assembler &a, const uml::instruction &inst);
	void op_muls(a64::Assembler &a, const uml::instruction &inst);
	void op_mulslw(a64::Assembler &a, const uml::instruction &inst);
	template <a64::Inst::Id Opcode> void op_div(a64::Assembler &a, const uml::instruction &inst);
	void op_and(a64::Assembler &a, const uml::instruction &inst);
	void op_test(a64::Assembler &a, const uml::instruction &inst);
	void op_or(a64::Assembler &a, const uml::instruction &inst);
	void op_xor(a64::Assembler &a, const uml::instruction &inst);
	void op_lzcnt(a64::Assembler &a, const uml::instruction &inst);
	void op_tzcnt(a64::Assembler &a, const uml::instruction &inst);
	void op_bswap(a64::Assembler &a, const uml::instruction &inst);
	template <a64::Inst::Id Opcode> void op_shift(a64::Assembler &a, const uml::instruction &inst);
	void op_rol(a64::Assembler &a, const uml::instruction &inst);
	void op_rolc(a64::Assembler &a, const uml::instruction &inst);
	void op_rorc(a64::Assembler &a, const uml::instruction &inst);

	void op_fload(a64::Assembler &a, const uml::instruction &inst);
	void op_fstore(a64::Assembler &a, const uml::instruction &inst);
	void op_fread(a64::Assembler &a, const uml::instruction &inst);
	void op_fwrite(a64::Assembler &a, const uml::instruction &inst);
	void op_fmov(a64::Assembler &a, const uml::instruction &inst);
	void op_ftoint(a64::Assembler &a, const uml::instruction &inst);
	void op_ffrint(a64::Assembler &a, const uml::instruction &inst);
	void op_ffrflt(a64::Assembler &a, const uml::instruction &inst);
	void op_frnds(a64::Assembler &a, const uml::instruction &inst);
	void op_fcmp(a64::Assembler &a, const uml::instruction &inst);
	void op_fcopyi(a64::Assembler &a, const uml::instruction &inst);
	void op_icopyf(a64::Assembler &a, const uml::instruction &inst);

	template <a64::Inst::Id Opcode> void op_float_alu(a64::Assembler &a, const uml::instruction &inst);
	template <a64::Inst::Id Opcode> void op_float_alu2(a64::Assembler &a, const uml::instruction &inst);

	size_t emit(CodeHolder &ch);


	// helper functions
	void get_imm_relative(a64::Assembler &a, const a64::Gp &reg, const uint64_t ptr) const;

	void emit_ldr_str_base_mem(a64::Assembler &a, a64::Inst::Id opcode, const Reg &reg, int max_shift, const void *ptr) const;
	void emit_ldr_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_ldrb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_ldrh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_ldrsb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_ldrsh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_ldrsw_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_str_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_strb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;
	void emit_strh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const;

	void emit_float_ldr_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const;
	void emit_float_str_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const;

	void emit_skip(a64::Assembler &a, uml::condition_t cond, Label &skip);

	a64::Mem emit_loadstore_address_setup(a64::Assembler &a, const a64::Gp &basereg, const be_parameter &indp, const uml::parameter &scalesizep) const;

	void emit_memaccess_setup(a64::Assembler &a, const be_parameter &addrp, const memory_accessors &accessors, const address_space::specific_access_info::side &side) const;
	void emit_narrow_memaccess_setup(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors, const address_space::specific_access_info::side &side) const;
	void emit_narrow_memread(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors) const;
	void emit_narrow_memwrite(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors) const;

	void store_carry(a64::Assembler &a, bool inverted = false);
	void load_carry(a64::Assembler &a, bool inverted = false);
	void set_flags(a64::Assembler &a, const a64::Gp &reg);

	void calculate_carry_shift_left(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift, int maxBits);
	void calculate_carry_shift_left_imm(a64::Assembler &a, const a64::Gp &reg, const int shift, int maxBits);

	void calculate_carry_shift_right(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift);
	void calculate_carry_shift_right_imm(a64::Assembler &a, const a64::Gp &reg, const int shift);

	void mov_float_reg_param(a64::Assembler &a, uint32_t regsize, a64::Vec const &dst, const be_parameter &src) const;
	void mov_float_param_param(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const;
	void mov_float_param_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, a64::Vec const &src) const;
	void mov_float_param_int_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, a64::Gp const &src) const;

	void mov_reg_param(a64::Assembler &a, uint32_t regsize, const a64::Gp &dst, const be_parameter &src) const;
	void mov_param_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const a64::Gp &src) const;
	void mov_param_imm(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, uint64_t src) const;
	void mov_param_param(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const;
	void mov_mem_param(a64::Assembler &a, uint32_t regsize, void *dst, const be_parameter &src) const;

	void call_arm_addr(a64::Assembler &a, const void *offs) const;

	[[noreturn]] void end_of_block() const;

	drc_hash_table m_hash;
	drc_map_variables m_map;
	FILE *m_log_asmjit;
	carry_state m_carry_state;

	arm64_entry_point_func m_entry;
	drccodeptr m_exit;
	drccodeptr m_nocode;
	drccodeptr m_endofblock;

	uint8_t *m_baseptr;

	near_state &m_near;

	resolved_member_function m_debug_cpu_instruction_hook;
	resolved_member_function m_drcmap_get_value;
	std::vector<memory_accessors> m_memory_accessors;
};


inline void drcbe_arm64::generate_one(a64::Assembler &a, const uml::instruction &inst)
{
	switch (inst.opcode())
	{
	// Compile-time opcodes
	case uml::OP_HANDLE:  op_handle(a, inst);     break; // HANDLE  handle
	case uml::OP_HASH:    op_hash(a, inst);       break; // HASH    mode,pc
	case uml::OP_LABEL:   op_label(a, inst);      break; // LABEL   imm
	case uml::OP_COMMENT: op_comment(a, inst);    break; // COMMENT string
	case uml::OP_MAPVAR:  op_mapvar(a, inst);     break; // MAPVAR  mapvar,value

	// Control Flow Operations
	case uml::OP_NOP:     op_nop(a, inst);        break; // NOP
	case uml::OP_BREAK:   op_break(a, inst);      break; // BREAK
	case uml::OP_DEBUG:   op_debug(a, inst);      break; // DEBUG   pc
	case uml::OP_EXIT:    op_exit(a, inst);       break; // EXIT    src1[,c]
	case uml::OP_HASHJMP: op_hashjmp(a, inst);    break; // HASHJMP mode,pc,handle
	case uml::OP_JMP:     op_jmp(a, inst);        break; // JMP     imm[,c]
	case uml::OP_EXH:     op_exh(a, inst);        break; // EXH     handle,param[,c]
	case uml::OP_CALLH:   op_callh(a, inst);      break; // CALLH   handle[,c]
	case uml::OP_RET:     op_ret(a, inst);        break; // RET     [c]
	case uml::OP_CALLC:   op_callc(a, inst);      break; // CALLC   func,ptr[,c]
	case uml::OP_RECOVER: op_recover(a, inst);    break; // RECOVER dst,mapvar

	// Internal Register Operations
	case uml::OP_SETFMOD: op_setfmod(a, inst);    break; // SETFMOD src
	case uml::OP_GETFMOD: op_getfmod(a, inst);    break; // GETFMOD dst
	case uml::OP_GETEXP:  op_getexp(a, inst);     break; // GETEXP  dst
	case uml::OP_GETFLGS: op_getflgs(a, inst);    break; // GETFLGS dst[,f]
	case uml::OP_SETFLGS: op_setflgs(a, inst);    break; // SETFLGS dst[,f]
	case uml::OP_SAVE:    op_save(a, inst);       break; // SAVE    dst
	case uml::OP_RESTORE: op_restore(a, inst);    break; // RESTORE dst

	// Integer Operations
	case uml::OP_LOAD:    op_load(a, inst);                     break; // LOAD    dst,base,index,size
	case uml::OP_LOADS:   op_loads(a, inst);                    break; // LOADS   dst,base,index,size
	case uml::OP_STORE:   op_store(a, inst);                    break; // STORE   base,index,src,size
	case uml::OP_READ:    op_read(a, inst);                     break; // READ    dst,src1,spacesize
	case uml::OP_READM:   op_readm(a, inst);                    break; // READM   dst,src1,mask,spacesize
	case uml::OP_WRITE:   op_write(a, inst);                    break; // WRITE   dst,src1,spacesize
	case uml::OP_WRITEM:  op_writem(a, inst);                   break; // WRITEM  dst,src1,spacesize
	case uml::OP_CARRY:   op_carry(a, inst);                    break; // CARRY   src,bitnum
	case uml::OP_SET:     op_set(a, inst);                      break; // SET     dst,c
	case uml::OP_MOV:     op_mov(a, inst);                      break; // MOV     dst,src[,c]
	case uml::OP_SEXT:    op_sext(a, inst);                     break; // SEXT    dst,src
	case uml::OP_ROLAND:  op_roland(a, inst);                   break; // ROLAND  dst,src1,src2,src3
	case uml::OP_ROLINS:  op_rolins(a, inst);                   break; // ROLINS  dst,src1,src2,src3
	case uml::OP_ADD:     op_add<false>(a, inst);               break; // ADD     dst,src1,src2[,f]
	case uml::OP_ADDC:    op_add<true>(a, inst);                break; // ADDC    dst,src1,src2[,f]
	case uml::OP_SUB:     op_sub<false>(a, inst);               break; // SUB     dst,src1,src2[,f]
	case uml::OP_SUBB:    op_sub<true>(a, inst);                break; // SUBB    dst,src1,src2[,f]
	case uml::OP_CMP:     op_cmp(a, inst);                      break; // CMP     src1,src2[,f]
	case uml::OP_MULU:    op_mulu(a, inst);                     break; // MULU    dst,edst,src1,src2[,f]
	case uml::OP_MULULW:  op_mululw(a, inst);                   break; // MULULW   dst,src1,src2[,f]
	case uml::OP_MULS:    op_muls(a, inst);                     break; // MULS    dst,edst,src1,src2[,f]
	case uml::OP_MULSLW:  op_mulslw(a, inst);                   break; // MULSLW   dst,src1,src2[,f]
	case uml::OP_DIVU:    op_div<a64::Inst::kIdUdiv>(a, inst);  break; // DIVU    dst,edst,src1,src2[,f]
	case uml::OP_DIVS:    op_div<a64::Inst::kIdSdiv>(a, inst);  break; // DIVS    dst,edst,src1,src2[,f]
	case uml::OP_AND:     op_and(a, inst);                      break; // AND     dst,src1,src2[,f]
	case uml::OP_TEST:    op_test(a, inst);                     break; // TEST    src1,src2[,f]
	case uml::OP_OR:      op_or(a, inst);                       break; // OR      dst,src1,src2[,f]
	case uml::OP_XOR:     op_xor(a, inst);                      break; // XOR     dst,src1,src2[,f]
	case uml::OP_LZCNT:   op_lzcnt(a, inst);                    break; // LZCNT   dst,src[,f]
	case uml::OP_TZCNT:   op_tzcnt(a, inst);                    break; // TZCNT   dst,src[,f]
	case uml::OP_BSWAP:   op_bswap(a, inst);                    break; // BSWAP   dst,src
	case uml::OP_SHL:     op_shift<a64::Inst::kIdLsl>(a, inst); break; // SHL     dst,src,count[,f]
	case uml::OP_SHR:     op_shift<a64::Inst::kIdLsr>(a, inst); break; // SHR     dst,src,count[,f]
	case uml::OP_SAR:     op_shift<a64::Inst::kIdAsr>(a, inst); break; // SAR     dst,src,count[,f]
	case uml::OP_ROL:     op_rol(a, inst);                      break; // ROL     dst,src,count[,f]
	case uml::OP_ROLC:    op_rolc(a, inst);                     break; // ROLC    dst,src,count[,f]
	case uml::OP_ROR:     op_shift<a64::Inst::kIdRor>(a, inst); break; // ROR     dst,src,count[,f]
	case uml::OP_RORC:    op_rorc(a, inst);                     break; // RORC    dst,src,count[,f]

	// Floating Point Operations
	case uml::OP_FLOAD:   op_fload(a, inst);                                break; // FLOAD   dst,base,index
	case uml::OP_FSTORE:  op_fstore(a, inst);                               break; // FSTORE  base,index,src
	case uml::OP_FREAD:   op_fread(a, inst);                                break; // FREAD   dst,space,src1
	case uml::OP_FWRITE:  op_fwrite(a, inst);                               break; // FWRITE  space,dst,src1
	case uml::OP_FMOV:    op_fmov(a, inst);                                 break; // FMOV    dst,src1[,c]
	case uml::OP_FTOINT:  op_ftoint(a, inst);                               break; // FTOINT  dst,src1,size,round
	case uml::OP_FFRINT:  op_ffrint(a, inst);                               break; // FFRINT  dst,src1,size
	case uml::OP_FFRFLT:  op_ffrflt(a, inst);                               break; // FFRFLT  dst,src1,size
	case uml::OP_FRNDS:   op_frnds(a, inst);                                break; // FRNDS   dst,src1
	case uml::OP_FADD:    op_float_alu<a64::Inst::kIdFadd_v>(a, inst);      break; // FADD    dst,src1,src2
	case uml::OP_FSUB:    op_float_alu<a64::Inst::kIdFsub_v>(a, inst);      break; // FSUB    dst,src1,src2
	case uml::OP_FCMP:    op_fcmp(a, inst);                                 break; // FCMP    src1,src2
	case uml::OP_FMUL:    op_float_alu<a64::Inst::kIdFmul_v>(a, inst);      break; // FMUL    dst,src1,src2
	case uml::OP_FDIV:    op_float_alu<a64::Inst::kIdFdiv_v> (a, inst);     break; // FDIV    dst,src1,src2
	case uml::OP_FNEG:    op_float_alu2<a64::Inst::kIdFneg_v>(a, inst);     break; // FNEG    dst,src1
	case uml::OP_FABS:    op_float_alu2<a64::Inst::kIdFabs_v>(a, inst);     break; // FABS    dst,src1
	case uml::OP_FSQRT:   op_float_alu2<a64::Inst::kIdFsqrt_v>(a, inst);    break; // FSQRT   dst,src1
	case uml::OP_FRECIP:  op_float_alu2<a64::Inst::kIdFrecpe_v>(a, inst);   break; // FRECIP  dst,src1
	case uml::OP_FRSQRT:  op_float_alu2<a64::Inst::kIdFrsqrte_v>(a, inst);  break; // FRSQRT  dst,src1
	case uml::OP_FCOPYI:  op_fcopyi(a, inst);                               break; // FCOPYI  dst,src
	case uml::OP_ICOPYF:  op_icopyf(a, inst);                               break; // ICOPYF  dst,src

	default: throw emu_fatalerror("drcbe_arm64(%s): unhandled opcode %u\n", m_device.tag(), inst.opcode());
	}
};

drcbe_arm64::be_parameter::be_parameter(drcbe_arm64 &drcbe, const parameter &param, uint32_t allowed)
{
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
			if (int regnum = int_register_map[param.ireg() - REG_I0]; regnum != 0)
			{
				*this = make_ireg(regnum);
			}
			else
			{
				*this = make_memory(&drcbe.m_state.r[param.ireg() - REG_I0]);
				m_coldreg = true;
			}
			break;

		// if a register maps to a register, keep it as a register; otherwise map it to memory
		case parameter::PTYPE_FLOAT_REGISTER:
			assert(allowed & PTYPE_F);
			assert(allowed & PTYPE_M);
			if (int regnum = float_register_map[param.freg() - REG_F0]; regnum != 0)
			{
				*this = make_freg(regnum);
			}
			else
			{
				*this = make_memory(&drcbe.m_state.f[param.freg() - REG_F0]);
				m_coldreg = true;
			}
			break;

		// everything else is unexpected
		default:
			fatalerror("Unexpected parameter type %d\n", param.type());
	}
}

a64::Vec drcbe_arm64::be_parameter::get_register_float(uint32_t regsize) const
{
	assert(m_type == PTYPE_FLOAT_REGISTER);
	return ((regsize == 4) ? a64::Vec::make_v32 : a64::Vec::make_v64)(m_value);
}

a64::Gp drcbe_arm64::be_parameter::get_register_int(uint32_t regsize) const
{
	assert(m_type == PTYPE_INT_REGISTER);
	return ((regsize == 4) ? a64::Gp::make_r32 : a64::Gp::make_r64)(m_value);
}

a64::Vec drcbe_arm64::be_parameter::select_register(a64::Vec const &reg, uint32_t regsize) const
{
	if (m_type == PTYPE_FLOAT_REGISTER)
		return get_register_float(regsize);
	else if (regsize == 4)
		return reg.s();
	else
		return reg.d();
}

a64::Gp drcbe_arm64::be_parameter::select_register(a64::Gp const &reg, uint32_t regsize) const
{
	if (m_type == PTYPE_INT_REGISTER)
		return get_register_int(regsize);
	else if (regsize == 4)
		return reg.w();
	else
		return reg.x();
}

void drcbe_arm64::get_imm_relative(a64::Assembler &a, const a64::Gp &reg, const uint64_t val) const
{
	// Check for constants that can be generated with a single instruction
	if (is_simple_mov_immediate(val, reg.size()))
	{
		a.mov(reg, val);
		return;
	}
	else if (reg.is_gp64() && is_valid_immediate_mask(val, 4))
	{
		a.mov(reg.w(), val); // asmjit isn't smart enough to work this out
		return;
	}

	// Values close to the program counter can be generated with a single adr
	const uint64_t codeoffs = a.code()->base_address() + a.offset();
	const int64_t reloffs = int64_t(val) - codeoffs;
	if (is_valid_immediate_signed(reloffs, 21))
	{
		a.adr(reg, val);
		return;
	}

	// If a value can be expressed relative to the base register then it's worth using it
	const int64_t diff = int64_t(val) - int64_t(m_baseptr);
	if ((diff > 0) && emit_add_optimized(a, reg, BASE_REG, diff))
		return;
	else if ((diff < 0) && emit_sub_optimized(a, reg, BASE_REG, diff))
		return;

	// Values within 4G of the program counter can be generated with adrp followed by add
	const uint64_t pagebase = codeoffs & ~make_bitmask<uint64_t>(12);
	const int64_t pagerel = int64_t(val) - pagebase;
	if (is_valid_immediate_signed(pagerel, 21 + 12))
	{
		const uint64_t targetpage = val & ~make_bitmask<uint64_t>(12);
		const uint64_t pageoffs = val & util::make_bitmask<uint64_t>(12);

		a.adrp(reg.x(), targetpage);
		if (pageoffs != 0)
			a.add(reg, reg, pageoffs);

		return;
	}

	// up to four instructions
	a.mov(reg, val);
}

inline void drcbe_arm64::emit_ldr_str_base_mem(a64::Assembler &a, a64::Inst::Id opcode, const Reg &reg, int max_shift, const void *ptr) const
{
	// If it can fit as an immediate offset
	const int64_t diff = int64_t(ptr) - int64_t(m_baseptr);
	if (is_valid_offset(diff, max_shift))
	{
		a.emit(opcode, reg, a64::Mem(BASE_REG, diff));
		return;
	}

	// If it can fit as an offset relative to PC
	const uint64_t codeoffs = a.code()->base_address() + a.offset();
	const int64_t reloffs = int64_t(ptr) - codeoffs;
	if (is_valid_immediate_signed(reloffs, 21))
	{
		a.adr(MEM_SCRATCH_REG, ptr);
		a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG));
		return;
	}

	if (diff > 0 && is_valid_immediate(diff, 16))
	{
		a.mov(MEM_SCRATCH_REG, diff);
		a.emit(opcode, reg, a64::Mem(BASE_REG, MEM_SCRATCH_REG));
		return;
	}

	if (diff > 0 && emit_add_optimized(a, MEM_SCRATCH_REG, BASE_REG, diff))
	{
		a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG));
		return;
	}
	else if (diff < 0 && emit_sub_optimized(a, MEM_SCRATCH_REG, BASE_REG, diff))
	{
		a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG));
		return;
	}

	// If it's in a nearby page
	const uint64_t pagebase = codeoffs & ~make_bitmask<uint64_t>(12);
	const int64_t pagerel = (int64_t)ptr - pagebase;
	if (is_valid_immediate_signed(pagerel, 21 + 12))
	{
		const uint64_t targetpage = (uint64_t)ptr & ~make_bitmask<uint64_t>(12);
		const uint64_t pageoffs = (uint64_t)ptr & util::make_bitmask<uint64_t>(12);

		a.adrp(MEM_SCRATCH_REG, targetpage);
		if (is_valid_offset(pageoffs, max_shift))
		{
			a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG, pageoffs));
		}
		else
		{
			a.add(MEM_SCRATCH_REG, MEM_SCRATCH_REG, pageoffs);
			a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG));
		}
		return;
	}

	if (diff >= 0)
	{
		const int shift = (diff & make_bitmask<int64_t>(max_shift)) ? 0 : max_shift;

		if (is_valid_immediate(diff >> shift, 32))
		{
			a.mov(MEM_SCRATCH_REG, diff >> shift);

			if (shift)
				a.emit(opcode, reg, a64::Mem(BASE_REG, MEM_SCRATCH_REG, a64::lsl(shift)));
			else
				a.emit(opcode, reg, a64::Mem(BASE_REG, MEM_SCRATCH_REG));

			return;
		}
	}

	// Can't optimize it at all, most likely becomes 4 MOV instructions
	a.mov(MEM_SCRATCH_REG, ptr);
	a.emit(opcode, reg, a64::Mem(MEM_SCRATCH_REG));
}

void drcbe_arm64::emit_ldr_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdr, reg, reg.is_gp32() ? 2 : 3, ptr); }
void drcbe_arm64::emit_ldrb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrb, reg, 0, ptr); }
void drcbe_arm64::emit_ldrh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrh, reg, 1, ptr); }
void drcbe_arm64::emit_ldrsb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsb, reg, 0, ptr); }
void drcbe_arm64::emit_ldrsh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsh, reg, 1, ptr); }
void drcbe_arm64::emit_ldrsw_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsw, reg, 2, ptr); }
void drcbe_arm64::emit_str_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStr, reg, reg.is_gp32() ? 2 : 3, ptr); }
void drcbe_arm64::emit_strb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStrb, reg, 0, ptr); }
void drcbe_arm64::emit_strh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStrh, reg, 1, ptr); }

void drcbe_arm64::emit_float_ldr_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdr_v, reg, reg.is_vec32() ? 2 : 3, ptr); }
void drcbe_arm64::emit_float_str_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStr_v, reg, reg.is_vec32() ? 2 : 3, ptr); }

void drcbe_arm64::emit_skip(a64::Assembler &a, uml::condition_t cond, Label &skip)
{
	// Nothing to do if the instruction is unconditional
	if (cond == uml::COND_ALWAYS)
		return;

	// Branch to the skip point if the condition is not met
	skip = a.new_label();
	switch (cond)
	{
		case uml::COND_U:
			a.tbz(FLAGS_REG, uml::FLAG_BIT_U, skip);
			break;
		case uml::COND_NU:
			a.tbnz(FLAGS_REG, uml::FLAG_BIT_U, skip);
			break;
		case uml::COND_C:
		case uml::COND_NC:
			switch (m_carry_state)
			{
				case carry_state::CANONICAL:
					a.b(ARM_CONDITION(cond), skip);
					break;
				case carry_state::LOGICAL:
					a.b(ARM_NOT_CONDITION(cond), skip);
					break;
				default:
					a.emit((cond == uml::COND_C) ? a64::Inst::kIdTbz : a64::Inst::kIdTbnz, FLAGS_REG, uml::FLAG_BIT_C, skip);
			}
			break;
		case uml::COND_A:
		case uml::COND_BE:
			load_carry(a, true);
			[[fallthrough]];
		default:
			a.b(ARM_NOT_CONDITION(cond), skip);
	}
}

inline a64::Mem drcbe_arm64::emit_loadstore_address_setup(a64::Assembler &a, const a64::Gp &basereg, const be_parameter &indp, const uml::parameter &scalesizep) const
{
	assert(!indp.is_immediate());

	const int scale = scalesizep.scale();
	if (scale == 0)
	{
		// if there's no shift, sign extension can be part of the addressing mode
		const a64::Gp offsreg = indp.select_register(TEMP_REG3, 4);
		mov_reg_param(a, 4, offsreg, indp);
		return a64::Mem(basereg, offsreg, a64::sxtw(0));
	}
	else
	{
		const a64::Gp indreg = TEMP_REG3.x();
		if (indp.is_int_register())
			a.sxtw(indreg, indp.get_register_int(4));
		else if ((util::endianness::native == util::endianness::big) && indp.is_cold_register())
			emit_ldrsw_mem(a, indreg, reinterpret_cast<uint8_t *>(indp.memory()) + 4);
		else
			emit_ldrsw_mem(a, indreg, indp.memory());

		// the scale needs to match the size for shifting to be part of the addressing mode
		if (scale == scalesizep.size())
			return a64::Mem(basereg, indreg, a64::lsl(scale));

		a.add(basereg, basereg, indreg, a64::lsl(scale));
		return a64::Mem(basereg);
	}
}

void drcbe_arm64::emit_memaccess_setup(a64::Assembler &a, const be_parameter &addrp, const memory_accessors &accessors, const address_space::specific_access_info::side &side) const
{
	auto const addrreg = (accessors.no_mask || accessors.mask_simple) ? REG_PARAM2 : a64::x6;
	mov_reg_param(a, 4, addrreg, addrp);
	get_imm_relative(a, a64::x8, uintptr_t(side.dispatch));

	// if the high bits aren't affected by the global mask, extract them early
	if (accessors.high_bits && !accessors.mask_high_bits)
		a.ubfx(a64::w7, addrreg.w(), accessors.specific.low_bits, accessors.high_bits);

	if (accessors.mask_simple)
		a.and_(REG_PARAM2.w(), addrreg.w(), accessors.address_mask);
	else if (!accessors.no_mask)
		a.mov(REG_PARAM2.w(), accessors.address_mask); // 32-bit value, no more than two instructions

	// if the high address bits aren't affected by the global mask, load the dispatch table entry now
	if (!accessors.high_bits)
		a.ldr(REG_PARAM1, a64::Mem(a64::x8));
	else if (!accessors.mask_high_bits)
		a.ldr(REG_PARAM1, a64::Mem(a64::x8, a64::x7, a64::lsl(3)));

	// apply non-trivial global mask if necessary
	if (!accessors.no_mask && !accessors.mask_simple)
		a.and_(REG_PARAM2.w(), REG_PARAM2.w(), addrreg.w());

	// if the high address bits are affected by the global mask, load the dispatch table entry now
	if (accessors.mask_high_bits)
	{
		a.lsr(a64::w7, REG_PARAM2.w(), accessors.specific.low_bits);
		a.ldr(REG_PARAM1, a64::Mem(a64::x8, a64::x7, a64::lsl(3)));
	}

	// apply this pointer displacement if necessary
	if (side.displacement)
		a.add(REG_PARAM1, REG_PARAM1, side.displacement); // assume less than 4K

	// adjusted dispatch table entry pointer in REG_PARAM1
	// masked address in REG_PARAM2
	// x8, x7 and potentially x6 clobbered
}

void drcbe_arm64::emit_narrow_memaccess_setup(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors, const address_space::specific_access_info::side &side) const
{
	address_space &space = *m_space[spacesizep.space()];
	auto const addrreg = (accessors.no_mask || accessors.mask_simple) ? REG_PARAM2 : a64::x5;
	mov_reg_param(a, 4, addrreg, addrp);
	get_imm_relative(a, a64::x8, uintptr_t(side.dispatch));

	// get the shift count for the data and offset in w7
	int const shift = space.addr_shift() - 3;
	uint32_t const shiftmask = (accessors.specific.native_bytes - (1 << spacesizep.size())) << 3;
	if (space.endianness() != ENDIANNESS_LITTLE)
	{
		// swizzle for big Endian spaces
		bool const smallshift = (shift <= 0) && (shift >= -3);
		if (!smallshift)
		{
			if (shift < 0)
				a.lsl(a64::w6, addrreg.w(), -shift);
			else
				a.lsr(a64::w6, addrreg.w(), shift);
		}
		a.mov(a64::w7, shiftmask);
		if (smallshift)
			a.bic(a64::w7, a64::w7, addrreg.w(), -shift);
		else
			a.bic(a64::w7, a64::w7, a64::w6);
	}
	else
	{
		if (!shift)
		{
			a.and_(a64::w7, addrreg.w(), shiftmask);
		}
		else
		{
			if (shift < 0)
				a.lsl(a64::w7, addrreg.w(), -shift);
			else
				a.lsr(a64::w7, addrreg.w(), shift);
			a.and_(a64::w7, a64::w7, shiftmask);
		}
	}

	// if the high bits aren't affected by the global mask, extract them early
	if (accessors.high_bits && !accessors.mask_high_bits)
		a.ubfx(a64::w6, addrreg.w(), accessors.specific.low_bits, accessors.high_bits);

	if (accessors.mask_simple)
		a.and_(REG_PARAM2.w(), addrreg.w(), accessors.address_mask);
	else if (!accessors.no_mask)
		a.mov(REG_PARAM2.w(), accessors.address_mask); // 32-bit value, no more than two instructions

	// if the high address bits aren't affected by the global mask, load the dispatch table entry now
	if (!accessors.high_bits)
		a.ldr(REG_PARAM1, a64::Mem(a64::x8));
	else if (!accessors.mask_high_bits)
		a.ldr(REG_PARAM1, a64::Mem(a64::x8, a64::x6, a64::lsl(3)));

	// apply non-trivial global mask if necessary
	if (!accessors.no_mask && !accessors.mask_simple)
		a.and_(REG_PARAM2.w(), REG_PARAM2.w(), addrreg.w());

	// if the high address bits are affected by the global mask, load the dispatch table entry now
	if (accessors.mask_high_bits)
	{
		a.lsr(a64::w6, REG_PARAM2.w(), accessors.specific.low_bits);
		a.ldr(REG_PARAM1, a64::Mem(a64::x8, a64::x6, a64::lsl(3)));
	}

	// apply this pointer displacement if necessary
	if (side.displacement)
		a.add(REG_PARAM1, REG_PARAM1, side.displacement); // assume less than 4K

	// load vtable pointer early if we'll need it
	if (side.is_virtual)
		a.ldr(a64::x8, a64::Mem(REG_PARAM1));

	// adjusted dispatch table entry pointer in REG_PARAM1
	// masked address in REG_PARAM2
	// shift count in w7
	// vtable pointer in x8 if virtual
	// x6 and potentially x8 and x5 clobbered
}

void drcbe_arm64::emit_narrow_memread(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors) const
{
	// expects mask in REG_PARAM3

	emit_narrow_memaccess_setup(a, addrp, spacesizep, accessors, accessors.specific.read);

	// shift the mask
	a.lsl(REG_PARAM3, REG_PARAM3, a64::x7);

	// stash the shift count - flags are clobbered anyway, so use the emulated flags register
	a.mov(FLAGS_REG.w(), a64::w7);

	// call the read function
	if (accessors.specific.read.is_virtual)
	{
		a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.read.function)); // assume no more than 4096 vtable entries
		a.blr(a64::x8);
	}
	else
	{
		call_arm_addr(a, (const void *)accessors.specific.read.function);
	}

	// shift the result
	if (accessors.specific.native_bytes <= 4)
		a.lsr(REG_PARAM1.w(), REG_PARAM1.w(), FLAGS_REG.w());
	else
		a.lsr(REG_PARAM1, REG_PARAM1, FLAGS_REG);
}

void drcbe_arm64::emit_narrow_memwrite(a64::Assembler &a, const be_parameter &addrp, const parameter &spacesizep, const memory_accessors &accessors) const
{
	// expects data in REG_PARAM3 and mask in REG_PARAM4

	emit_narrow_memaccess_setup(a, addrp, spacesizep, accessors, accessors.specific.write);

	// shift the data and mask
	a.lsl(REG_PARAM3, REG_PARAM3, a64::x7);
	a.lsl(REG_PARAM4, REG_PARAM4, a64::x7);

	// call the write function
	if (accessors.specific.write.is_virtual)
	{
		a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.write.function)); // assume no more than 4096 vtable entries
		a.blr(a64::x8);
	}
	else
	{
		call_arm_addr(a, (const void *)accessors.specific.write.function);
	}
}

void drcbe_arm64::mov_reg_param(a64::Assembler &a, uint32_t regsize, const a64::Gp &dst, const be_parameter &src) const
{
	const a64::Gp dstreg = select_register(dst, regsize);
	if (src.is_immediate())
	{
		get_imm_relative(a, dstreg, (regsize == 4) ? uint32_t(src.immediate()) : src.immediate());
	}
	else if (src.is_int_register() && dst.id() != src.ireg())
	{
		a.mov(dstreg, src.get_register_int(regsize));
	}
	else if (src.is_memory())
	{
		if ((util::endianness::native == util::endianness::big) && (regsize == 4) && src.is_cold_register())
			emit_ldr_mem(a, dstreg, reinterpret_cast<uint8_t *>(src.memory()) + 4);
		else
			emit_ldr_mem(a, dstreg, src.memory());
	}
}

void drcbe_arm64::mov_param_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const a64::Gp &src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
	{
		if (dst.is_cold_register())
			emit_str_mem(a, src.x(), dst.memory());
		else
			emit_str_mem(a, select_register(src, regsize), dst.memory());
	}
	else if (dst.is_int_register() && src.id() != dst.ireg())
	{
		a.mov(dst.get_register_int(regsize), select_register(src, regsize));
	}
}

void drcbe_arm64::mov_param_imm(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, uint64_t src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
	{
		const uint32_t movsize = dst.is_cold_register() ? 8 : regsize;

		if (src == 0)
		{
			emit_str_mem(a, select_register(a64::xzr, movsize), dst.memory());
		}
		else
		{
			const a64::Gp scratch = select_register(SCRATCH_REG1, movsize);

			get_imm_relative(a, scratch, (regsize == 4) ? uint32_t(src) : src);
			emit_str_mem(a, scratch, dst.memory());
		}
	}
	else if (dst.is_int_register())
	{
		get_imm_relative(a, dst.get_register_int(regsize), src);
	}
}

void drcbe_arm64::mov_param_param(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const
{
	assert(!dst.is_immediate());

	if (src.is_memory())
	{
		if (dst.is_int_register())
		{
			mov_reg_param(a, regsize, dst.get_register_int(regsize), src);
		}
		else
		{
			mov_reg_param(a, regsize, SCRATCH_REG1, src);
			mov_param_reg(a, regsize, dst, SCRATCH_REG1);
		}
	}
	else if (src.is_int_register())
	{
		if ((regsize == 4) && dst.is_cold_register())
		{
			mov_reg_param(a, regsize, SCRATCH_REG1, src);
			mov_param_reg(a, regsize, dst, SCRATCH_REG1);
		}
		else
		{
			mov_param_reg(a, regsize, dst, src.get_register_int(regsize));
		}
	}
	else if (src.is_immediate())
	{
		mov_param_imm(a, regsize, dst, src.immediate());
	}
}

void drcbe_arm64::mov_mem_param(a64::Assembler &a, uint32_t regsize, void *dst, const be_parameter &src) const
{
	const a64::Gp scratch = select_register(SCRATCH_REG2, regsize);

	if (src.is_immediate_value(0))
	{
		emit_str_mem(a, select_register(a64::xzr, regsize), dst);
	}
	else if (src.is_immediate())
	{
		get_imm_relative(a, scratch, (regsize == 4) ? uint32_t(src.immediate()) : src.immediate());
		emit_str_mem(a, scratch, dst);
	}
	else if (src.is_memory())
	{
		if ((util::endianness::native == util::endianness::big) && (regsize == 4) && src.is_cold_register())
			emit_ldr_mem(a, scratch, reinterpret_cast<uint8_t *>(src.memory()) + 4);
		else
			emit_ldr_mem(a, scratch, src.memory());

		emit_str_mem(a, scratch, dst);
	}
	else if (src.is_int_register())
	{
		emit_str_mem(a, src.get_register_int(regsize), dst);
	}
}

void drcbe_arm64::mov_float_reg_param(a64::Assembler &a, uint32_t regsize, a64::Vec const &dst, const be_parameter &src) const
{
	assert(!src.is_immediate());

	if (src.is_memory())
		emit_float_ldr_mem(a, select_register(dst, regsize), src.memory());
	else if (src.is_float_register() && dst.id() != src.freg())
		a.fmov(select_register(dst, regsize), src.get_register_float(regsize));
}

void drcbe_arm64::mov_float_param_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, a64::Vec const &src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
		emit_float_str_mem(a, select_register(src, regsize), dst.memory());
	else if (dst.is_float_register() && src.id() != dst.freg())
		a.fmov(dst.get_register_float(regsize), select_register(src, regsize));
}

void drcbe_arm64::mov_float_param_int_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, a64::Gp const &src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
		emit_str_mem(a, src, dst.memory());
	else if (dst.is_float_register())
		a.fmov(dst.get_register_float(regsize), src);
}

void drcbe_arm64::mov_float_param_param(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const
{
	assert(!src.is_immediate());
	assert(!dst.is_immediate());

	if (dst.is_float_register())
	{
		mov_float_reg_param(a, regsize, dst.get_register_float(regsize), src);
	}
	else if (dst.is_memory())
	{
		if (src.is_float_register())
		{
			mov_float_param_reg(a, regsize, dst, src.get_register_float(regsize));
		}
		else if (src.is_memory())
		{
			const a64::Gp scratch = select_register(SCRATCH_REG2, regsize);
			emit_ldr_mem(a, scratch, src.memory());
			emit_str_mem(a, scratch, dst.memory());
		}
	}
}

void drcbe_arm64::call_arm_addr(a64::Assembler &a, const void *offs) const
{
	const uint64_t codeoffs = a.code()->base_address() + a.offset();
	const int64_t reloffs = int64_t(offs) - codeoffs;
	if (is_valid_immediate_signed(reloffs, 26 + 2))
	{
		a.bl(offs);
	}
	else
	{
		get_imm_relative(a, SCRATCH_REG1, uintptr_t(offs));
		a.blr(SCRATCH_REG1);
	}
}

void drcbe_arm64::store_carry(a64::Assembler &a, bool inverted)
{
	m_carry_state = inverted ? carry_state::LOGICAL : carry_state::CANONICAL;

	if (inverted)
		a.cset(SCRATCH_REG1, a64::CondCode::kCC);
	else
		a.cset(SCRATCH_REG1, a64::CondCode::kCS);

	store_carry_reg(a, SCRATCH_REG1);
}

void drcbe_arm64::load_carry(a64::Assembler &a, bool inverted)
{
	const carry_state desired = inverted ? carry_state::LOGICAL : carry_state::CANONICAL;
	if (desired != m_carry_state)
	{
		m_carry_state = desired;

		a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);
		a.bfi(SCRATCH_REG1, FLAGS_REG, 29, 1);

		if (inverted)
			a.eor(SCRATCH_REG1, SCRATCH_REG1, 1 << 29);

		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}
}

void drcbe_arm64::set_flags(a64::Assembler &a, const a64::Gp &reg)
{
	// Set native condition codes after loading flags register
	m_carry_state = carry_state::POISON;

	a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

	a.and_(TEMP_REG2, reg.x(), 0b1100); // zero + sign
	a.bfxil(TEMP_REG2, reg.x(), uml::FLAG_BIT_V, 1); // overflow flag
	a.bfi(TEMP_REG1, TEMP_REG2, 28, 4);

	a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);

	a.mov(TEMP_REG2, FLAG_C | FLAG_U);
	a.and_(FLAGS_REG, reg.x(), TEMP_REG2);
}

inline void drcbe_arm64::calculate_carry_shift_left(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift, int maxBits)
{
	m_carry_state = carry_state::POISON;

	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.size());
	const a64::Gp zero = select_register(a64::xzr, reg.size());

	// scratch = ((PARAM1 << (shift - 1)) >> maxBits) & 1
	a.neg(scratch, shift);
	a.ands(scratch, scratch, maxBits);
	a.lsr(scratch, reg, scratch);

	a.csel(scratch, zero, scratch, a64::CondCode::kZero);
	store_carry_reg(a, scratch);
}

inline void drcbe_arm64::calculate_carry_shift_left_imm(a64::Assembler &a, const a64::Gp &reg, const int shift, int maxBits)
{
	m_carry_state = carry_state::POISON;

	// carry = ((PARAM1 << (shift - 1)) >> maxBits) & 1
	if (shift == 0)
		store_carry_reg(a, a64::xzr);
	else
		store_carry_reg(a, reg, maxBits + 1 - shift);
}

inline void drcbe_arm64::calculate_carry_shift_right(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift)
{
	m_carry_state = carry_state::POISON;

	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.size());
	const a64::Gp zero = select_register(a64::xzr, reg.size());

	a.tst(shift, shift);

	// scratch = (PARAM1 >> (shift - 1)) & 1
	a.sub(scratch, shift, 1);
	a.lsr(scratch, reg, scratch);

	a.csel(scratch, zero, scratch, a64::CondCode::kZero);
	store_carry_reg(a, scratch);
}

inline void drcbe_arm64::calculate_carry_shift_right_imm(a64::Assembler &a, const a64::Gp &reg, const int shift)
{
	m_carry_state = carry_state::POISON;

	// carry = (PARAM1 >> (shift - 1)) & 1
	if (shift == 0)
		store_carry_reg(a, a64::xzr);
	else
		store_carry_reg(a, reg, shift - 1);
}

drcbe_arm64::drcbe_arm64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device)
	, m_hash(cache, modes, addrbits, ignorebits)
	, m_map(cache, 0xaaaaaaaa5555)
	, m_log_asmjit(nullptr)
	, m_carry_state(carry_state::POISON)
	, m_entry(nullptr)
	, m_exit(nullptr)
	, m_nocode(nullptr)
	, m_endofblock(nullptr)
	, m_baseptr(cache.near() + 0x100)
	, m_near(*(near_state *)cache.alloc_near(sizeof(m_near)))
{
	// create the log
	if (device.machine().options().drc_log_native())
	{
		m_log_asmjit = fopen(std::string("drcbearm64_asmjit_").append(device.shortname()).append(".asm").c_str(), "w");
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
			accessors.high_bits = 32 - count_leading_zeros_32(shiftedmask);
			accessors.no_mask = nomask == accessors.address_mask;
			accessors.mask_simple = !accessors.no_mask && is_valid_immediate_mask(accessors.address_mask, 4);
			accessors.mask_high_bits = (shiftedmask & (shiftedmask + 1)) != 0;
		}
	}
}

drcbe_arm64::~drcbe_arm64()
{
	if (m_log_asmjit)
		fclose(m_log_asmjit);
}

size_t drcbe_arm64::emit(CodeHolder &ch)
{
	Error err;

	size_t const alignment = ch.base_address() - uint64_t(m_cache.top());
	size_t const code_size = ch.code_size();

	// test if enough room remains in the DRC cache
	drccodeptr *cachetop = m_cache.begin_codegen(alignment + code_size);
	if (!cachetop)
		return 0;

	err = ch.copy_flattened_data(drccodeptr(ch.base_address()), code_size, CopySectionFlags::kPadTargetBuffer);
	if (err != kErrorOk)
		throw emu_fatalerror("CodeHolder::copy_flattened_data() error %u", std::underlying_type_t<Error>(err));

	// update the drc cache and end codegen
	*cachetop += alignment + code_size;
	m_cache.end_codegen();

	return code_size;
}

void drcbe_arm64::reset()
{
	uint8_t *dst = (uint8_t *)m_cache.top();

	CodeHolder ch;
	ch.init(Environment::host(), uint64_t(dst));

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.set_flags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.set_indentation(FormatIndentationGroup::kCode, 4);
		ch.set_logger(&logger);
	}

	a64::Assembler a(&ch);
	if (logger.file())
		a.add_diagnostic_options(DiagnosticOptions::kValidateIntermediate);

	// generate entry point
	m_entry = (arm64_entry_point_func)dst;
	a.bind(a.new_named_label("entry_point"));

	FuncDetail entry_point;
	entry_point.init(FuncSignature::build<uint32_t, uint8_t *, uint8_t *>(CallConvId::kCDecl), Environment::host());

	FuncFrame frame;
	frame.init(entry_point);
	frame.set_preserved_fp();
	frame.set_all_dirty();

	FuncArgsAssignment args(&entry_point);
	args.assign_all(REG_PARAM1);
	args.update_func_frame(frame);

	frame.finalize();

	a.emit_prolog(frame);

	get_imm_absolute(a, BASE_REG, uintptr_t(m_baseptr));

	a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kFPCR);
	a.mov(FLAGS_REG, 0);
	emit_str_mem(a, SCRATCH_REG1, &m_near.saved_fpcr);

	a.emit_args_assignment(frame, args);

	a.br(REG_PARAM1);

	// generate exit point
	m_exit = dst + a.offset();
	a.bind(a.new_named_label("exit_point"));

	emit_ldr_mem(a, SCRATCH_REG1, &m_near.saved_fpcr);
	a.msr(a64::Predicate::SysReg::kFPCR, SCRATCH_REG1);

	a.mov(a64::sp, a64::x29);

	a.emit_epilog(frame);
	a.ret(a64::x30);

	// generate a no code point
	m_nocode = dst + a.offset();
	a.bind(a.new_named_label("nocode_point"));
	a.br(REG_PARAM1);

	// generate an end-of-block handler point
	m_endofblock = dst + a.offset();
	a.bind(a.new_named_label("end_of_block_point"));
	auto const [entrypoint, adjusted] = util::resolve_member_function(&drcbe_arm64::end_of_block, *this);
	get_imm_relative(a, REG_PARAM1, adjusted);
	call_arm_addr(a, (const void *)entrypoint);

	// emit the generated code
	emit(ch);

	// reset our hash tables
	m_hash.reset();
	m_hash.set_default_codeptr(m_nocode);

	m_carry_state = carry_state::POISON;
}

int drcbe_arm64::execute(code_handle &entry)
{
	m_cache.codegen_complete();
	return (*m_entry)(entry.codeptr());
}

void drcbe_arm64::generate(drcuml_block &block, const instruction *instlist, uint32_t numinst)
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
	m_carry_state = carry_state::POISON;

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
	uint8_t *dst = (uint8_t *)(uint64_t(m_cache.top() + linemask) & ~linemask);

	CodeHolder ch;
	ch.init(Environment::host(), uint64_t(dst));
	ThrowableErrorHandler e;
	ch.set_error_handler(&e);

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.set_flags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.set_indentation(FormatIndentationGroup::kCode, 4);
		ch.set_logger(&logger);
	}

	a64::Assembler a(&ch);
	if (logger.file())
		a.add_diagnostic_options(DiagnosticOptions::kValidateIntermediate);

	// generate code
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];

		// must remain in scope until output
		std::string dasm;

		// add a comment
		if (logger.file())
		{
			dasm = inst.disasm(&m_drcuml);
			a.set_inline_comment(dasm.c_str());
		}

		// generate code
		generate_one(a, inst);
	}

	// catch falling off the end of a block
	if (logger.file())
		a.set_inline_comment("end of block");
	a.b(m_endofblock);

	// emit the generated code
	if (!emit(ch))
		block.abort();

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_map.block_end(block);
}

bool drcbe_arm64::hash_exists(uint32_t mode, uint32_t pc) const noexcept
{
	return m_hash.code_exists(mode, pc);
}

void drcbe_arm64::get_info(drcbe_info &info) const noexcept
{
	for (info.direct_iregs = 0; info.direct_iregs < REG_I_COUNT; info.direct_iregs++)
	{
		if (int_register_map[info.direct_iregs] == 0)
			break;
	}

	for (info.direct_fregs = 0; info.direct_fregs < REG_F_COUNT; info.direct_fregs++)
	{
		if (float_register_map[info.direct_fregs] == 0)
			break;
	}
}


[[noreturn]] void drcbe_arm64::end_of_block() const
{
	osd_printf_error("drcbe_arm64(%s): fell off the end of a generated code block!\n", m_device.tag());
	std::fflush(stdout);
	std::fflush(stderr);
	std::abort();
}


void drcbe_arm64::op_handle(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_handle());

	m_carry_state = carry_state::POISON;

	// make a label for documentation
	Label handle = a.new_named_label(inst.param(0).handle().string());
	a.bind(handle);

	// emit a jump around the stack adjust in case code falls through here
	Label skip = a.new_label();
	a.b(skip);

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(drccodeptr(a.code()->base_address() + a.offset()));

	// the handle points to prologue code that creates a minimal non-leaf frame
	a.stp(a64::x29, a64::x30, a64::ptr_pre(a64::sp, -16));
	a.bind(skip);
}

void drcbe_arm64::op_hash(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	m_carry_state = carry_state::POISON;

	const uint64_t mode = inst.param(0).immediate();
	const uint64_t pc = inst.param(1).immediate();

	m_hash.set_codeptr(mode, pc, drccodeptr(a.code()->base_address() + a.offset()));
}

void drcbe_arm64::op_label(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_label());

	m_carry_state = carry_state::POISON;

	std::string labelName = util::string_format("PC$%x", inst.param(0).label());
	Label label = a.label_by_name(labelName.c_str());
	if (!label.is_valid())
		label = a.new_named_label(labelName.c_str());

	a.bind(label);
}

void drcbe_arm64::op_comment(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_string());
}

void drcbe_arm64::op_mapvar(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_mapvar());
	assert(inst.param(1).is_immediate());

	const int mapvar = inst.param(0).mapvar();
	const uint64_t value = inst.param(1).immediate();

	m_map.set_value(drccodeptr(a.code()->base_address() + a.offset()), mapvar, value);
}

void drcbe_arm64::op_nop(a64::Assembler &a, const uml::instruction &inst)
{
	// nothing
	//a.nop();
}

void drcbe_arm64::op_break(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	static const char *const message = "break from drc";
	get_imm_relative(a, REG_PARAM1, (uintptr_t)message);
	call_arm_addr(a, (const void *)&osd_break_into_debugger);
}

void drcbe_arm64::op_debug(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	if (m_device.machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		m_carry_state = carry_state::POISON;

		const a64::Gp temp = TEMP_REG1.w();

		be_parameter pcp(*this, inst.param(0), PTYPE_MRI);

		Label skip = a.new_label();

		emit_ldr_mem(a, temp, &m_device.machine().debug_flags);
		a.tbz(temp, 1, skip); // DEBUG_FLAG_CALL_HOOK

		get_imm_relative(a, REG_PARAM1, m_debug_cpu_instruction_hook.obj);
		mov_reg_param(a, 4, REG_PARAM2, pcp);

		call_arm_addr(a, m_debug_cpu_instruction_hook.func);

		a.bind(skip);
	}
}

void drcbe_arm64::op_exit(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	be_parameter retp(*this, inst.param(0), PTYPE_MRI);

	Label skip;
	emit_skip(a, inst.condition(), skip);

	mov_reg_param(a, 4, REG_PARAM1, retp);
	a.b(m_exit);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);
}

void drcbe_arm64::op_hashjmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter modep(*this, inst.param(0), PTYPE_MRI);
	be_parameter pcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &exp = inst.param(2);
	assert(exp.is_code_handle());

	a.mov(a64::sp, a64::x29);

	if (modep.is_immediate() && m_hash.is_mode_populated(modep.immediate()))
	{
		if (pcp.is_immediate())
		{
			const uint32_t l1val = (pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask();
			const uint32_t l2val = (pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask();
			emit_ldr_mem(a, TEMP_REG1, &m_hash.base()[modep.immediate()][l1val][l2val]);
		}
		else
		{
			mov_reg_param(a, 4, TEMP_REG2, pcp);

			get_imm_relative(a, TEMP_REG1, (uintptr_t)&m_hash.base()[modep.immediate()][0]); // TEMP_REG1 = m_base[mode]

			a.ubfx(TEMP_REG3, TEMP_REG2, m_hash.l1shift(), m_hash.l1bits());
			a.ldr(TEMP_REG3, a64::Mem(TEMP_REG1, TEMP_REG3, a64::lsl(3))); // TEMP_REG3 = m_base[mode][(pc >> m_l1shift) & m_l1mask]

			a.ubfx(TEMP_REG2, TEMP_REG2, m_hash.l2shift(), m_hash.l2bits());
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG3, TEMP_REG2, a64::lsl(3))); // TEMP_REG1 = m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]
		}
	}
	else
	{
		get_imm_relative(a, TEMP_REG2, (uintptr_t)m_hash.base());

		if (modep.is_immediate())
		{
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG2, modep.immediate() * 8)); // TEMP_REG1 = m_base[modep]
		}
		else
		{
			const a64::Gp mode = modep.select_register(TEMP_REG1, 8);
			mov_reg_param(a, 4, mode, modep);
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG2, mode, a64::lsl(3))); // TEMP_REG1 = m_base[modep]
		}

		if (pcp.is_immediate())
		{
			const uint32_t l1val = ((pcp.immediate() >> m_hash.l1shift()) & m_hash.l1mask()) * 8;
			const uint32_t l2val = ((pcp.immediate() >> m_hash.l2shift()) & m_hash.l2mask()) * 8;

			if (is_valid_immediate(l1val, 15))
			{
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, l1val));
			}
			else
			{
				a.mov(SCRATCH_REG1, l1val >> 3);
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, SCRATCH_REG1, a64::lsl(3)));
			}

			if (is_valid_immediate(l2val, 15))
			{
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, l2val));
			}
			else
			{
				a.mov(SCRATCH_REG1, l2val >> 3);
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, SCRATCH_REG1, a64::lsl(3)));
			}
		}
		else
		{
			const a64::Gp pc = pcp.select_register(TEMP_REG2, 8);
			mov_reg_param(a, 4, pc, pcp);

			a.ubfx(TEMP_REG3, pc, m_hash.l1shift(), m_hash.l1bits()); // (pc >> m_l1shift) & m_l1mask
			a.ldr(TEMP_REG3, a64::Mem(TEMP_REG1, TEMP_REG3, a64::lsl(3))); // TEMP_REG3 = m_base[mode][(pc >> m_l1shift) & m_l1mask]

			a.ubfx(TEMP_REG2, pc, m_hash.l2shift(), m_hash.l2bits()); // (pc >> m_l2shift) & m_l2mask
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG3, TEMP_REG2, a64::lsl(3))); // x25 = m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]
		}
	}

	Label lab = a.new_label();
	a.adr(REG_PARAM1, lab);
	a.br(TEMP_REG1);

	a.bind(lab);

	mov_mem_param(a, 4, &m_state.exp, pcp);

	drccodeptr *const targetptr = exp.handle().codeptr_addr();
	if (*targetptr != nullptr)
	{
		call_arm_addr(a, *targetptr);
	}
	else
	{
		emit_ldr_mem(a, SCRATCH_REG1, targetptr);
		a.blr(SCRATCH_REG1);
	}

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_jmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &labelp = inst.param(0);
	assert(labelp.is_code_label());

	std::string labelName = util::string_format("PC$%x", labelp.label());
	Label jmptarget = a.label_by_name(labelName.c_str());
	if (!jmptarget.is_valid())
		jmptarget = a.new_named_label(labelName.c_str());

	if (inst.condition() == uml::COND_ALWAYS)
	{
		a.b(jmptarget);
		return;
	}

	const bool bound = a.code()->is_label_bound(jmptarget);
	const uint64_t targetoffs = bound ? (a.code()->base_address() + a.code()->label_offset(jmptarget)) : 0;
	const uint64_t codeoffs = a.code()->base_address() + a.offset();
	const bool tbnzrange = bound && is_valid_immediate_signed(int64_t(targetoffs) - codeoffs, 14 + 2);

	switch (inst.condition())
	{
		case uml::COND_U:
		case uml::COND_NU:
			if (tbnzrange)
			{
				const a64::Inst::Id opcode = (inst.condition() == uml::COND_U) ? a64::Inst::kIdTbnz : a64::Inst::kIdTbz;
				a.emit(opcode, FLAGS_REG, uml::FLAG_BIT_U, jmptarget);
			}
			else
			{
				const a64::Inst::Id opcode = (inst.condition() == uml::COND_U) ? a64::Inst::kIdCbnz : a64::Inst::kIdCbz;
				get_unordered(a, SCRATCH_REG1);
				a.emit(opcode, SCRATCH_REG1, jmptarget);
			}
			break;
		case uml::COND_C:
		case uml::COND_NC:
			switch (m_carry_state)
			{
				case carry_state::CANONICAL:
					a.b(ARM_NOT_CONDITION(inst.condition()), jmptarget);
					break;
				case carry_state::LOGICAL:
					a.b(ARM_CONDITION(inst.condition()), jmptarget);
					break;
				default:
					if (tbnzrange)
					{
						const a64::Inst::Id opcode = (inst.condition() == uml::COND_C) ? a64::Inst::kIdTbnz : a64::Inst::kIdTbz;
						a.emit(opcode, FLAGS_REG, uml::FLAG_BIT_C, jmptarget);
					}
					else
					{
						const a64::Inst::Id opcode = (inst.condition() == uml::COND_C) ? a64::Inst::kIdCbnz : a64::Inst::kIdCbz;
						get_carry(a, SCRATCH_REG1);
						a.emit(opcode, SCRATCH_REG1, jmptarget);
					}
			}
			break;
		case uml::COND_A:
		case uml::COND_BE:
			load_carry(a, true);
			[[fallthrough]];
		default:
			a.b(ARM_CONDITION(inst.condition()), jmptarget);
	}
}

void drcbe_arm64::op_exh(a64::Assembler &a, const uml::instruction &inst)
{
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());
	be_parameter exp(*this, inst.param(1), PTYPE_MRI);

	// perform the exception processing
	Label no_exception;
	emit_skip(a, inst.condition(), no_exception);

	mov_mem_param(a, 4, &m_state.exp, exp);

	drccodeptr *const targetptr = handp.handle().codeptr_addr();
	if (*targetptr != nullptr)
	{
		call_arm_addr(a, *targetptr);
	}
	else
	{
		emit_ldr_mem(a, SCRATCH_REG1, targetptr);
		a.blr(SCRATCH_REG1);
	}

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(no_exception);

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_callh(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());

	Label skip;
	emit_skip(a, inst.condition(), skip);

	drccodeptr *const targetptr = handp.handle().codeptr_addr();
	if (*targetptr != nullptr)
	{
		call_arm_addr(a, *targetptr);
	}
	else
	{
		emit_ldr_mem(a, SCRATCH_REG1, targetptr);
		a.blr(SCRATCH_REG1);
	}

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_ret(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	Label skip;
	emit_skip(a, inst.condition(), skip);

	a.ldp(a64::x29, a64::x30, a64::ptr_post(a64::sp, 16));
	a.ret(a64::x30);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);
}

void drcbe_arm64::op_callc(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &funcp = inst.param(0);
	assert(funcp.is_c_function());
	be_parameter paramp(*this, inst.param(1), PTYPE_M);

	Label skip;
	emit_skip(a, inst.condition(), skip);

	get_imm_relative(a, REG_PARAM1, (uintptr_t)paramp.memory());
	get_imm_relative(a, TEMP_REG1, (uintptr_t)funcp.cfunc());
	a.blr(TEMP_REG1);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_recover(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	a.ldr(REG_PARAM2, a64::Mem(a64::x29, -8)); // saved LR (x30) from first level CALLH/EXH or failed hash jump
	get_imm_relative(a, REG_PARAM1, m_drcmap_get_value.obj);
	a.mov(REG_PARAM3, inst.param(1).mapvar());
	a.sub(REG_PARAM2, REG_PARAM2, 4);

	call_arm_addr(a, m_drcmap_get_value.func);

	mov_param_reg(a, inst.size(), dstp, REG_PARAM1);
}

void drcbe_arm64::op_setfmod(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);
	const a64::Gp scratch = select_register(TEMP_REG1, inst.size());

	if (srcp.is_immediate())
	{
		a.mov(scratch, srcp.immediate() & 3);
	}
	else
	{
		const a64::Gp src = srcp.select_register(scratch, inst.size());

		mov_reg_param(a, inst.size(), src, srcp);
		a.and_(scratch, src, 3);
	}

	a.mrs(TEMP_REG2, a64::Predicate::SysReg::kFPCR);
	emit_strb_mem(a, scratch.w(), &m_state.fmod);
	a.sub(scratch.w(), scratch.w(), 1);
	a.bfi(TEMP_REG2, scratch.x(), 22, 2);
	a.msr(a64::Predicate::SysReg::kFPCR, TEMP_REG2);
}

void drcbe_arm64::op_getfmod(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());

	emit_ldrb_mem(a, dst.w(), &m_state.fmod);
	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_getexp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());

	emit_ldr_mem(a, dst.w(), &m_state.exp);
	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_getflgs(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter maskp(*this, inst.param(1), PTYPE_I);
	assert(maskp.is_immediate());

	const a64::Gp dst = dstp.select_register(TEMP_REG1, 8);

	bool first = true;

	if (maskp.immediate() & FLAG_C)
	{
		a.and_(dst, FLAGS_REG, FLAG_C);
		first = false;
	}

	if (maskp.immediate() & FLAG_V)
	{
		if (first)
		{
			a.cset(dst, a64::CondCode::kVS);
			a.lsl(dst, dst, uml::FLAG_BIT_V);
			first = false;
		}
		else
		{
			a.cset(SCRATCH_REG1, a64::CondCode::kVS);
			a.orr(dst, dst, SCRATCH_REG1, uml::FLAG_BIT_V);
		}
	}

	if (maskp.immediate() & FLAG_Z)
	{
		if (first)
		{
			a.cset(dst, a64::CondCode::kEQ);
			a.lsl(dst, dst, uml::FLAG_BIT_Z);
			first = false;
		}
		else
		{
			a.cset(SCRATCH_REG1, a64::CondCode::kEQ);
			a.orr(dst, dst, SCRATCH_REG1, uml::FLAG_BIT_Z);
		}
	}

	if (maskp.immediate() & FLAG_S)
	{
		if (first)
		{
			a.cset(dst, a64::CondCode::kMI);
			a.lsl(dst, dst, uml::FLAG_BIT_S);
			first = false;
		}
		else
		{
			a.cset(SCRATCH_REG1, a64::CondCode::kMI);
			a.orr(dst, dst, SCRATCH_REG1, uml::FLAG_BIT_S);
		}
	}

	if (maskp.immediate() & FLAG_U)
	{
		if (first)
		{
			a.and_(dst, FLAGS_REG, FLAG_U);
			first = false;
		}
		else
		{
			a.and_(SCRATCH_REG1, FLAGS_REG, FLAG_U);
			a.orr(dst, dst, SCRATCH_REG1);
		}
	}

	if (first)
		a.mov(dst, a64::xzr);

	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_setflgs(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);

	be_parameter flagsp(*this, inst.param(0), PTYPE_MRI);

	const a64::Gp flags = flagsp.select_register(FLAGS_REG, inst.size());

	mov_reg_param(a, inst.size(), flags, flagsp);
	set_flags(a, flags);
}

void drcbe_arm64::op_save(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_M);

	const a64::Gp membase = SCRATCH_REG1;

	get_imm_relative(a, membase, (uintptr_t)dstp.memory());

	// Calculate flags to be stored
	a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);
	a.lsr(TEMP_REG1, TEMP_REG1, 28);

	a.and_(TEMP_REG2, TEMP_REG1, 0b1100); // zero + sign
	a.orr(TEMP_REG2, TEMP_REG2, FLAGS_REG); // carry + unordered flags

	a.bfi(TEMP_REG2, TEMP_REG1, uml::FLAG_BIT_V, 1); // overflow flag

	a.strb(TEMP_REG2.w(), a64::Mem(membase, offsetof(drcuml_machine_state, flags)));

	emit_ldrb_mem(a, TEMP_REG1.w(), &m_state.fmod);
	a.strb(TEMP_REG1.w(), a64::Mem(membase, offsetof(drcuml_machine_state, fmod)));

	emit_ldr_mem(a, TEMP_REG1.w(), &m_state.exp);
	a.str(TEMP_REG1.w(), a64::Mem(membase, offsetof(drcuml_machine_state, exp)));

	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < std::size(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
		{
			a.str(a64::Gp::make_r64(int_register_map[regnum]), a64::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			emit_ldr_mem(a, TEMP_REG1, &m_state.r[regnum].d);
			a.str(TEMP_REG1, a64::Mem(membase, regoffs + (8 * regnum)));
		}
	}

	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
		{
			a.str(a64::Vec::make_v64(float_register_map[regnum]), a64::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			emit_ldr_mem(a, TEMP_REG1, &m_state.f[regnum].d);
			a.str(TEMP_REG1, a64::Mem(membase, regoffs + (8 * regnum)));
		}
	}
}

void drcbe_arm64::op_restore(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);

	be_parameter srcp(*this, inst.param(0), PTYPE_M);

	const a64::Gp membase = SCRATCH_REG1;

	get_imm_relative(a, membase, (uintptr_t)srcp.memory());

	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < std::size(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
		{
			a.ldr(a64::Gp::make_r64(int_register_map[regnum]), a64::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			a.ldr(TEMP_REG1, a64::Mem(membase, regoffs + (8 * regnum)));
			emit_str_mem(a, TEMP_REG1, &m_state.r[regnum].d);
		}
	}

	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
		{
			a.ldr(a64::Vec::make_v64(float_register_map[regnum]), a64::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			a.ldr(TEMP_REG1, a64::Mem(membase, regoffs + (8 * regnum)));
			emit_str_mem(a, TEMP_REG1, &m_state.f[regnum].d);
		}
	}

	a.ldrb(TEMP_REG1.w(), a64::Mem(membase, offsetof(drcuml_machine_state, fmod)));
	a.and_(TEMP_REG1.w(), TEMP_REG1.w(), 3);
	a.mrs(TEMP_REG2, a64::Predicate::SysReg::kFPCR);
	emit_strb_mem(a, TEMP_REG1.w(), &m_state.fmod);
	a.sub(TEMP_REG1.w(), TEMP_REG1.w(), 1);
	a.bfi(TEMP_REG2, TEMP_REG1, 22, 2);
	a.msr(a64::Predicate::SysReg::kFPCR, TEMP_REG2);

	a.ldr(TEMP_REG1.w(), a64::Mem(membase, offsetof(drcuml_machine_state, exp)));
	emit_str_mem(a, TEMP_REG1.w(), &m_state.exp);

	a.ldrb(FLAGS_REG.w(), a64::Mem(membase, offsetof(drcuml_machine_state, flags)));
	set_flags(a, FLAGS_REG);
}

void drcbe_arm64::op_load(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	const int size = scalesizep.size();

	const a64::Gp dstreg = dstp.select_register(TEMP_REG2, inst.size());

	if (indp.is_immediate())
	{
		const ptrdiff_t offset = ptrdiff_t(int32_t(uint32_t(indp.immediate()))) << scalesizep.scale();
		const auto memptr = reinterpret_cast<uint8_t *>(basep.memory()) + offset;

		// immediate index
		if (size == SIZE_BYTE)
			emit_ldrb_mem(a, dstreg.w(), memptr);
		else if (size == SIZE_WORD)
			emit_ldrh_mem(a, dstreg.w(), memptr);
		else if (size == SIZE_DWORD)
			emit_ldr_mem(a, dstreg.w(), memptr);
		else
			emit_ldr_mem(a, dstreg.x(), memptr);
	}
	else
	{
		const a64::Gp basereg = TEMP_REG1;

		get_imm_relative(a, basereg, uint64_t(basep.memory()));
		const auto mem = emit_loadstore_address_setup(a, basereg, indp, scalesizep);

		if (size == SIZE_BYTE)
			a.ldrb(dstreg.w(), mem);
		else if (size == SIZE_WORD)
			a.ldrh(dstreg.w(), mem);
		else if (size == SIZE_DWORD)
			a.ldr(dstreg.w(), mem);
		else
			a.ldr(dstreg, mem);
	}

	mov_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_loads(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	assert(scalesizep.is_size_scale());
	const int size = scalesizep.size();

	const a64::Gp dstreg = dstp.select_register(TEMP_REG2, inst.size());

	if (indp.is_immediate())
	{
		const ptrdiff_t offset = ptrdiff_t(int32_t(uint32_t(indp.immediate()))) << scalesizep.scale();
		const auto memptr = reinterpret_cast<uint8_t *>(basep.memory()) + offset;

		// immediate index
		if (size == SIZE_BYTE)
			emit_ldrsb_mem(a, dstreg, memptr);
		else if (size == SIZE_WORD)
			emit_ldrsh_mem(a, dstreg, memptr);
		else if ((size == SIZE_DWORD) && (inst.size() == 8))
			emit_ldrsw_mem(a, dstreg, memptr);
		else
			emit_ldr_mem(a, dstreg, memptr);
	}
	else
	{
		const a64::Gp basereg = TEMP_REG1;

		get_imm_relative(a, basereg, uint64_t(basep.memory()));
		const auto mem = emit_loadstore_address_setup(a, basereg, indp, scalesizep);

		if (size == SIZE_BYTE)
			a.ldrsb(dstreg, mem);
		else if (size == SIZE_WORD)
			a.ldrsh(dstreg, mem);
		else if ((size == SIZE_DWORD) && (inst.size() == 8))
			a.ldrsw(dstreg, mem);
		else
			a.ldr(dstreg, mem);
	}

	mov_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_store(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MRI);
	const parameter &scalesizep = inst.param(3);
	const int size = scalesizep.size();

	if (indp.is_immediate())
	{
		const a64::Gp srcreg = srcp.select_register(TEMP_REG2, inst.size());
		const ptrdiff_t offset = ptrdiff_t(int32_t(uint32_t(indp.immediate()))) << scalesizep.scale();
		const auto memptr = reinterpret_cast<uint8_t *>(basep.memory()) + offset;

		mov_reg_param(a, inst.size(), srcreg, srcp);

		if (size == SIZE_BYTE)
			emit_strb_mem(a, srcreg.w(), memptr);
		else if (size == SIZE_WORD)
			emit_strh_mem(a, srcreg.w(), memptr);
		else if (size == SIZE_DWORD)
			emit_str_mem(a, srcreg.w(), memptr);
		else
			emit_str_mem(a, srcreg.x(), memptr);
	}
	else
	{
		const a64::Gp basereg = TEMP_REG1;
		const a64::Gp srcreg = srcp.select_register(TEMP_REG2, inst.size());

		get_imm_relative(a, basereg, uint64_t(basep.memory()));
		mov_reg_param(a, inst.size(), srcreg, srcp);
		const auto mem = emit_loadstore_address_setup(a, basereg, indp, scalesizep);

		if (size == SIZE_BYTE)
			a.strb(srcreg.w(), mem);
		else if (size == SIZE_WORD)
			a.strh(srcreg.w(), mem);
		else if (size == SIZE_DWORD)
			a.str(srcreg.w(), mem);
		else
			a.str(srcreg, mem);
	}
}

void drcbe_arm64::op_read(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.read.function) || accessors.specific.read.is_virtual;

	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		emit_memaccess_setup(a, addrp, accessors, accessors.specific.read);
		if (accessors.specific.read.is_virtual)
		{
			a.ldr(a64::x8, a64::Mem(REG_PARAM1));
			a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.read.function)); // assume no more than 4096 vtable entries
		}
		a.mov(REG_PARAM3, make_bitmask<uint64_t>(accessors.specific.native_bytes << 3));
		if (accessors.specific.read.is_virtual)
			a.blr(a64::x8);
		else
			call_arm_addr(a, (const void *)accessors.specific.read.function);
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		a.mov(REG_PARAM3, make_bitmask<uint64_t>(8 << spacesizep.size()));
		emit_narrow_memread(a, addrp, spacesizep, accessors);
	}
	else
	{
		mov_reg_param(a, 4, REG_PARAM2, addrp);

		if (spacesizep.size() == SIZE_BYTE)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_byte.obj);
			call_arm_addr(a, accessors.resolved.read_byte.func);
		}
		else if (spacesizep.size() == SIZE_WORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_word.obj);
			call_arm_addr(a, accessors.resolved.read_word.func);
		}
		else if (spacesizep.size() == SIZE_DWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_dword.obj);
			call_arm_addr(a, accessors.resolved.read_dword.func);
		}
		else if (spacesizep.size() == SIZE_QWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_qword.obj);
			call_arm_addr(a, accessors.resolved.read_qword.func);
		}
	}

	const a64::Gp dstreg = dstp.select_register(REG_PARAM1, inst.size());
	if (spacesizep.size() == SIZE_BYTE)
	{
		a.and_(dstreg, select_register(REG_PARAM1, inst.size()), 0x000000ff);
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		a.and_(dstreg, select_register(REG_PARAM1, inst.size()), 0x0000ffff);
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else if ((spacesizep.size() == SIZE_DWORD) && (inst.size() == 8))
	{
		a.mov(dstreg.w(), REG_PARAM1.w());
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else
	{
		mov_param_reg(a, inst.size(), dstp, REG_PARAM1);
	}
}

void drcbe_arm64::op_readm(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.read.function) || accessors.specific.read.is_virtual;

	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		emit_memaccess_setup(a, addrp, accessors, accessors.specific.read);
		mov_reg_param(a, inst.size(), REG_PARAM3, maskp);
		if (accessors.specific.read.is_virtual)
		{
			a.ldr(a64::x8, a64::Mem(REG_PARAM1));
			a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.read.function)); // assume no more than 4096 vtable entries
			a.blr(a64::x8);
		}
		else
		{
			call_arm_addr(a, (const void *)accessors.specific.read.function);
		}
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		mov_reg_param(a, inst.size(), REG_PARAM3, maskp);
		emit_narrow_memread(a, addrp, spacesizep, accessors);
	}
	else
	{
		mov_reg_param(a, 4, REG_PARAM2, addrp);
		mov_reg_param(a, inst.size(), REG_PARAM3, maskp);

		if (spacesizep.size() == SIZE_BYTE)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_byte_masked.obj);
			call_arm_addr(a, accessors.resolved.read_byte_masked.func);
		}
		else if (spacesizep.size() == SIZE_WORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_word_masked.obj);
			call_arm_addr(a, accessors.resolved.read_word_masked.func);
		}
		else if (spacesizep.size() == SIZE_DWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_dword_masked.obj);
			call_arm_addr(a, accessors.resolved.read_dword_masked.func);
		}
		else if (spacesizep.size() == SIZE_QWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.read_qword_masked.obj);
			call_arm_addr(a, accessors.resolved.read_qword_masked.func);
		}
	}

	const a64::Gp dstreg = dstp.select_register(REG_PARAM1, inst.size());
	if (spacesizep.size() == SIZE_BYTE)
	{
		a.and_(dstreg, select_register(REG_PARAM1, inst.size()), 0x000000ff);
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else if (spacesizep.size() == SIZE_WORD)
	{
		a.and_(dstreg, select_register(REG_PARAM1, inst.size()), 0x0000ffff);
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else if ((spacesizep.size() == SIZE_DWORD) && (inst.size() == 8))
	{
		a.mov(dstreg.w(), REG_PARAM1.w());
		mov_param_reg(a, inst.size(), dstp, dstreg);
	}
	else
	{
		mov_param_reg(a, inst.size(), dstp, REG_PARAM1);
	}
}

void drcbe_arm64::op_write(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.write.function) || accessors.specific.write.is_virtual;

	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		emit_memaccess_setup(a, addrp, accessors, accessors.specific.write);
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);
		if (accessors.specific.write.is_virtual)
		{
			a.ldr(a64::x8, a64::Mem(REG_PARAM1));
			a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.write.function)); // assume no more than 4096 vtable entries
		}
		a.mov(REG_PARAM4, make_bitmask<uint64_t>(accessors.specific.native_bytes << 3));
		if (accessors.specific.write.is_virtual)
			a.blr(a64::x8);
		else
			call_arm_addr(a, (const void *)accessors.specific.write.function);
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);
		a.mov(REG_PARAM4, make_bitmask<uint64_t>(8 << spacesizep.size()));
		emit_narrow_memwrite(a, addrp, spacesizep, accessors);
	}
	else
	{
		mov_reg_param(a, 4, REG_PARAM2, addrp);
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);

		if (spacesizep.size() == SIZE_BYTE)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_byte.obj);
			call_arm_addr(a, accessors.resolved.write_byte.func);
		}
		else if (spacesizep.size() == SIZE_WORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_word.obj);
			call_arm_addr(a, accessors.resolved.write_word.func);
		}
		else if (spacesizep.size() == SIZE_DWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_dword.obj);
			call_arm_addr(a, accessors.resolved.write_dword.func);
		}
		else if (spacesizep.size() == SIZE_QWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_qword.obj);
			call_arm_addr(a, accessors.resolved.write_qword.func);
		}
	}
}

void drcbe_arm64::op_writem(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// set up a call to the write handler
	auto const &accessors = m_memory_accessors[spacesizep.space()];
	bool const have_specific = (uintptr_t(nullptr) != accessors.specific.write.function) || accessors.specific.write.is_virtual;

	if (have_specific && ((1 << spacesizep.size()) == accessors.specific.native_bytes))
	{
		emit_memaccess_setup(a, addrp, accessors, accessors.specific.write);
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);
		if (accessors.specific.write.is_virtual)
		{
			a.ldr(a64::x8, a64::Mem(REG_PARAM1));
			a.ldr(a64::x8, a64::Mem(a64::x8, accessors.specific.write.function)); // assume no more than 4096 vtable entries
		}
		mov_reg_param(a, inst.size(), REG_PARAM4, maskp);
		if (accessors.specific.write.is_virtual)
			a.blr(a64::x8);
		else
			call_arm_addr(a, (const void *)accessors.specific.write.function);
	}
	else if (have_specific && ((1 << spacesizep.size()) < accessors.specific.native_bytes))
	{
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);
		mov_reg_param(a, inst.size(), REG_PARAM4, maskp);
		emit_narrow_memwrite(a, addrp, spacesizep, accessors);
	}
	else
	{
		mov_reg_param(a, 4, REG_PARAM2, addrp);
		mov_reg_param(a, inst.size(), REG_PARAM3, srcp);
		mov_reg_param(a, inst.size(), REG_PARAM4, maskp);

		if (spacesizep.size() == SIZE_BYTE)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_byte_masked.obj);
			call_arm_addr(a, accessors.resolved.write_byte_masked.func);
		}
		else if (spacesizep.size() == SIZE_WORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_word_masked.obj);
			call_arm_addr(a, accessors.resolved.write_word_masked.func);
		}
		else if (spacesizep.size() == SIZE_DWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_dword_masked.obj);
			call_arm_addr(a, accessors.resolved.write_dword_masked.func);
		}
		else if (spacesizep.size() == SIZE_QWORD)
		{
			get_imm_relative(a, REG_PARAM1, accessors.resolved.write_qword_masked.obj);
			call_arm_addr(a, accessors.resolved.write_qword_masked.func);
		}
	}
}

void drcbe_arm64::op_carry(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C);

	m_carry_state = carry_state::POISON;

	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);
	be_parameter bitp(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp scratch = select_register(TEMP_REG1, inst.size());

	// load non-immediate bit numbers into a register
	// flags = (flags & ~FLAG_C) | ((src >> (PARAM1 & 31)) & FLAG_C)

	if (srcp.is_immediate() && bitp.is_immediate())
	{
		if (BIT(srcp.immediate(), bitp.immediate()))
			a.orr(FLAGS_REG, FLAGS_REG, FLAG_C);
		else
			a.and_(FLAGS_REG, FLAGS_REG, ~FLAG_C);
	}
	else if (bitp.is_immediate())
	{
		const auto shift = bitp.immediate() % (inst.size() * 8);

		mov_reg_param(a, inst.size(), src, srcp);

		// move carry bit to lsb
		store_carry_reg(a, src, shift);
	}
	else
	{
		const a64::Gp bitreg = bitp.select_register(TEMP_REG2, inst.size());
		const a64::Gp shift = select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), bitreg, bitp);
		mov_reg_param(a, inst.size(), src, srcp);

		a.and_(shift, bitreg, (inst.size() * 8) - 1);

		// move carry bit to lsb
		a.lsr(scratch, src, shift);
		store_carry_reg(a, scratch);
	}
}

void drcbe_arm64::op_set(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	if (inst.condition() == uml::COND_ALWAYS)
	{
		mov_param_imm(a, inst.size(), dstp, 1);
		return;
	}

	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());

	switch (inst.condition())
	{
		case uml::COND_U:
		case uml::COND_NU:
			get_unordered(a, dst);
			if (inst.condition() == uml::COND_NU)
				a.eor(dst, dst, 1);
			break;
		case uml::COND_C:
		case uml::COND_NC:
			switch (m_carry_state)
			{
				case carry_state::CANONICAL:
					a.cset(dst, ARM_NOT_CONDITION(inst.condition()));
					break;
				case carry_state::LOGICAL:
					a.cset(dst, ARM_CONDITION(inst.condition()));
					break;
				default:
					get_carry(a, dst);
					if (inst.condition() == uml::COND_NC)
						a.eor(dst, dst, 1);
			}
			break;
		case uml::COND_A:
		case uml::COND_BE:
			load_carry(a, true);
			[[fallthrough]];
		default:
			a.cset(dst, ARM_CONDITION(inst.condition()));
	}

	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_mov(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// decide whether a conditional select will be efficient
	bool usesel = dstp.is_int_register() && (((inst.size() == 8) && srcp.is_int_register()) || (srcp.is_immediate() && is_simple_mov_immediate(srcp.immediate(), inst.size())));
	switch (inst.condition())
	{
		case uml::COND_ALWAYS:
		case uml::COND_U:
		case uml::COND_NU:
			usesel = false;
			break;
		case uml::COND_C:
		case uml::COND_NC:
			switch (m_carry_state)
			{
				case carry_state::CANONICAL:
				case carry_state::LOGICAL:
					break;
				default:
					usesel = false;
			}
			break;
		default:
			break;
	}

	if (usesel)
	{
		const bool srczero = srcp.is_immediate_value(0);
		const bool srcone = srcp.is_immediate_value(1);
		const bool srcnegone = (inst.size() == 8) && srcp.is_immediate_value(uint64_t(int64_t(-1)));
		const bool srcspecial = srczero || srcone || srcnegone;

		const a64::Gp dst = dstp.select_register(TEMP_REG1, 8);
		const a64::Gp src = srcspecial ? a64::xzr : srcp.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), dst, dstp);
		if (!srcspecial)
			mov_reg_param(a, inst.size(), src, srcp);

		switch (inst.condition())
		{
			case uml::COND_C:
			case uml::COND_NC:
				if (m_carry_state == carry_state::CANONICAL)
				{
					if (srcone)
						a.csinc(dst, dst, src.x(), ARM_CONDITION(inst.condition()));
					else if (srcnegone)
						a.csinv(dst, dst, src.x(), ARM_CONDITION(inst.condition()));
					else
						a.csel(dst, src.x(), dst, ARM_NOT_CONDITION(inst.condition()));
				}
				else
				{
					if (srcone)
						a.csinc(dst, dst, src.x(), ARM_NOT_CONDITION(inst.condition()));
					else if (srcnegone)
						a.csinv(dst, dst, src.x(), ARM_NOT_CONDITION(inst.condition()));
					else
						a.csel(dst, src.x(), dst, ARM_CONDITION(inst.condition()));
				}
				break;
			case uml::COND_A:
			case uml::COND_BE:
				load_carry(a, true);
				[[fallthrough]];
			default:
				if (srcone)
					a.csinc(dst, dst, src.x(), ARM_NOT_CONDITION(inst.condition()));
				else if (srcnegone)
					a.csinv(dst, dst, src.x(), ARM_NOT_CONDITION(inst.condition()));
				else
					a.csel(dst, src.x(), dst, ARM_CONDITION(inst.condition()));
		}

		mov_param_reg(a, inst.size(), dstp, dst);
	}
	else
	{
		Label skip;
		emit_skip(a, inst.condition(), skip);

		mov_param_param(a, inst.size(), dstp, srcp);

		if (inst.condition() != uml::COND_ALWAYS)
			a.bind(skip);
	}
}

void drcbe_arm64::op_sext(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());
	const auto size = sizep.size();

	const a64::Gp dstreg = dstp.select_register(TEMP_REG2, inst.size());

	if ((1 << size) >= inst.size())
	{
		mov_param_param(a, inst.size(), dstp, srcp);
	}
	else
	{
		if (srcp.is_memory())
		{
			uintptr_t mem = uintptr_t(srcp.memory());
			if (util::endianness::native == util::endianness::big)
				mem ^= (inst.size() - 1) & ~((1 << size) - 1);

			if (size == SIZE_BYTE)
				emit_ldrsb_mem(a, dstreg, reinterpret_cast<void *>(mem));
			else if (size == SIZE_WORD)
				emit_ldrsh_mem(a, dstreg, reinterpret_cast<void *>(mem));
			else if (size == SIZE_DWORD)
				emit_ldrsw_mem(a, dstreg, reinterpret_cast<void *>(mem));
			else if (size == SIZE_QWORD)
				emit_ldr_mem(a, dstreg, reinterpret_cast<void *>(mem));
		}
		else
		{
			const a64::Gp tempreg = srcp.select_register(dstreg, inst.size());
			mov_reg_param(a, inst.size(), tempreg, srcp);

			if (size == SIZE_BYTE)
				a.sxtb(dstreg, tempreg.w());
			else if (size == SIZE_WORD)
				a.sxth(dstreg, tempreg.w());
			else if (size == SIZE_DWORD)
				a.sxtw(dstreg, tempreg.w());
		}

		mov_param_reg(a, inst.size(), dstp, dstreg);
	}

	if (inst.flags())
	{
		a.tst(dstreg, dstreg);
		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_roland(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter shiftp(*this, inst.param(2), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(3), PTYPE_MRI);

	const a64::Gp output = dstp.select_register(TEMP_REG1, inst.size());
	const a64::Gp src = srcp.select_register(output, inst.size());
	const uint64_t instbits = inst.size() * 8;
	const uint64_t instmask = util::make_bitmask<uint64_t>(instbits);

	bool optimized = false;
	if (maskp.is_immediate() && shiftp.is_immediate() && !maskp.is_immediate_value(util::make_bitmask<uint64_t>(instbits)))
	{
		// A mask of all 1s will be handled efficiently in the unoptimized path, so only optimize for the other cases if possible
		const auto pop = population_count_64(maskp.immediate());
		const auto lz = count_leading_zeros_64(maskp.immediate()) & (instbits - 1);
		const auto invlamask = ~(maskp.immediate() << lz) & instmask;
		const bool is_right_aligned = (maskp.immediate() & (maskp.immediate() + 1)) == 0;
		const bool is_contiguous = (invlamask & (invlamask + 1)) == 0;
		const auto s = shiftp.immediate() & (instbits - 1);

		if (is_right_aligned || is_contiguous)
		{
			mov_reg_param(a, inst.size(), src, srcp);
			optimized = true;
		}

		if (maskp.is_immediate_value(0))
		{
			a.mov(output, select_register(a64::xzr, inst.size()));
		}
		else if (is_right_aligned)
		{
			// Optimize a contiguous right-aligned mask
			const auto s2 = -int(s) & (instbits - 1);

			if (s >= pop)
			{
				a.ubfx(output, src, s2, pop);
			}
			else if (s2 > 0)
			{
				a.ror(output, src, s2);
				a.bfc(output, pop, instbits - pop);
			}
			else
			{
				a.and_(output, src, ~maskp.immediate() & instmask);
			}
		}
		else if (is_contiguous)
		{
			// Optimize a contiguous mask
			auto const rot = -int(s + pop + lz) & (instbits - 1);

			if (rot > 0)
			{
				a.ror(output, src, rot);
				a.ubfiz(output, output, instbits - pop - lz, pop);
			}
			else
			{
				a.ubfiz(output, src, instbits - pop - lz, pop);
			}
		}
	}

	if (!optimized)
	{
		const a64::Gp shift = shiftp.select_register(TEMP_REG2, inst.size());
		const a64::Gp rshift = select_register(TEMP_REG2, inst.size());
		const a64::Gp mask = (dstp != maskp) ? maskp.select_register(SCRATCH_REG1, inst.size()) : select_register(SCRATCH_REG1, inst.size());

		if (!shiftp.is_immediate())
		{
			// do this first as dst and shift could be the same register
			mov_reg_param(a, inst.size(), shift, shiftp);

			a.neg(rshift, shift);
		}

		// mask and dst could also be the same register so do this before rotating dst
		if (!maskp.is_immediate() || !is_valid_immediate_mask(maskp.immediate(), inst.size()))
			mov_reg_param(a, inst.size(), mask, maskp);

		if (shiftp.is_immediate())
		{
			const auto s = -int64_t(shiftp.immediate()) & (instbits - 1);
			if (s != 0)
			{
				mov_reg_param(a, inst.size(), src, srcp);
				a.ror(output, src, s);
			}
			else
			{
				mov_reg_param(a, inst.size(), output, srcp);
			}
		}
		else
		{
			mov_reg_param(a, inst.size(), src, srcp);
			a.and_(rshift, rshift, (inst.size() * 8) - 1);
			a.ror(output, src, rshift);
		}

		const a64::Inst::Id maskop = inst.flags() ? a64::Inst::kIdAnds : a64::Inst::kIdAnd;
		if (maskp.is_immediate() && is_valid_immediate_mask(maskp.immediate(), inst.size()))
			a.emit(maskop, output, output, maskp.immediate());
		else
			a.emit(maskop, output, output, mask);
	}

	mov_param_reg(a, inst.size(), dstp, output);

	if (inst.flags())
	{
		if (optimized)
			a.tst(output, output);

		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_rolins(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_S | FLAG_Z);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter shiftp(*this, inst.param(2), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(3), PTYPE_MRI);
	const uint64_t instbits = inst.size() * 8;

	const a64::Gp dst = dstp.select_register(TEMP_REG2, inst.size());
	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp scratch = select_register(TEMP_REG1, inst.size());

	bool optimized = false;
	if (maskp.is_immediate() && shiftp.is_immediate())
	{
		const auto pop = population_count_64(maskp.immediate());
		const auto lz = count_leading_zeros_64(maskp.immediate()) & (instbits - 1);
		const auto invlamask = ~(maskp.immediate() << lz) & util::make_bitmask<uint64_t>(instbits);
		const bool is_right_aligned = (maskp.immediate() & (maskp.immediate() + 1)) == 0;
		const bool is_contiguous = (invlamask & (invlamask + 1)) == 0;
		const auto s = shiftp.immediate() & (instbits - 1);

		if (maskp.is_immediate_value(0))
		{
			mov_reg_param(a, inst.size(), dst, dstp);

			optimized = true;
		}
		else if (is_right_aligned || is_contiguous)
		{
			mov_reg_param(a, inst.size(), dst, dstp);

			uint32_t rot = 0;
			uint32_t lsb = 0;
			if (is_right_aligned)
			{
				// Optimize a contiguous right-aligned mask
				rot = -int32_t(s) & (instbits - 1);
			}
			else
			{
				// Optimize a contiguous mask - rotate the field to make it right-aligned, then insert it
				rot = -int32_t(s + pop + lz) & (instbits - 1);
				lsb = instbits - pop - lz;
			}

			if (srcp.is_immediate() && (rot > 0))
			{
				// save some instructions by avoid mov to register by computing the ror and storing it into scratch directly
				uint64_t result;
				if (inst.size() == 4)
					result = rotr_32(srcp.immediate(), rot);
				else
					result = rotr_64(srcp.immediate(), rot);

				a.mov(scratch, result);
				a.bfi(dst, scratch, lsb, pop);
			}
			else
			{
				mov_reg_param(a, inst.size(), src, srcp);

				if (is_right_aligned && ((rot + pop) <= instbits))
				{
					// can insert a right-aligned field directly when it doesn't wrap around
					a.bfxil(dst, src, rot, pop);
				}
				else
				{
					if (rot > 0)
						a.ror(scratch, src, rot);

					a.bfi(dst, (rot > 0) ? scratch : src, lsb, pop);
				}
			}

			optimized = true;
		}
		else if (srcp.is_immediate())
		{
			mov_reg_param(a, inst.size(), dst, dstp);

			// val1 = src & ~PARAM3
			if (is_valid_immediate_mask(~maskp.immediate() & util::make_bitmask<uint64_t>(instbits), inst.size()))
			{
				a.and_(dst, dst, ~maskp.immediate() & util::make_bitmask<uint64_t>(instbits));
			}
			else
			{
				get_imm_relative(a, scratch, ~maskp.immediate() & util::make_bitmask<uint64_t>(instbits));
				a.and_(dst, dst, scratch);
			}

			uint64_t result;
			if (inst.size() == 4)
				result = rotl_32(srcp.immediate(), s) & maskp.immediate();
			else
				result = rotl_64(srcp.immediate(), s) & maskp.immediate();

			if (result != 0)
			{
				if (is_valid_immediate_mask(result, inst.size()))
				{
					a.orr(dst, dst, result);
				}
				else
				{
					get_imm_relative(a, scratch, result);
					a.orr(dst, dst, scratch);
				}
			}

			optimized = true;
		}
	}

	if (!optimized)
	{
		const a64::Gp shift = shiftp.select_register(SCRATCH_REG1, inst.size());
		const a64::Gp rshift = select_register(SCRATCH_REG1, inst.size());

		if (!shiftp.is_immediate())
		{
			// do this first as dst could be the same register as shift
			mov_reg_param(a, inst.size(), shift, shiftp);

			a.neg(rshift, shift);
		}

		mov_reg_param(a, inst.size(), dst, dstp);

		if (shiftp.is_immediate())
		{
			const auto shift = -int64_t(shiftp.immediate()) & ((inst.size() * 8) - 1);

			if (shift != 0)
			{
				mov_reg_param(a, inst.size(), src, srcp);
				a.ror(scratch, src, shift);
			}
			else
			{
				mov_reg_param(a, inst.size(), scratch, srcp);
			}
		}
		else
		{
			mov_reg_param(a, inst.size(), src, srcp);

			a.and_(rshift, rshift, (inst.size() * 8) - 1);
			a.ror(scratch, src, rshift);
		}

		const a64::Gp mask = maskp.select_register(SCRATCH_REG1, inst.size());
		if (!maskp.is_immediate() || !is_valid_immediate_mask(maskp.immediate(), inst.size()) || !is_valid_immediate_mask(~maskp.immediate() & util::make_bitmask<uint64_t>(instbits), inst.size()))
			mov_reg_param(a, inst.size(), mask, maskp);

		// val2 = val2 & PARAM3
		if (maskp.is_immediate() && is_valid_immediate_mask(maskp.immediate(), inst.size()))
			a.and_(scratch, scratch, maskp.immediate());
		else
			a.and_(scratch, scratch, mask);

		// val1 = src & ~PARAM3
		if (maskp.is_immediate() && is_valid_immediate_mask(~maskp.immediate() & util::make_bitmask<uint64_t>(instbits), inst.size()))
			a.and_(dst, dst, ~maskp.immediate() & util::make_bitmask<uint64_t>(instbits));
		else
			a.bic(dst, dst, mask);

		a.orr(dst, dst, scratch); // val1 | val2
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.tst(dst, dst);
		m_carry_state = carry_state::POISON;
	}
}

template <bool CarryIn> void drcbe_arm64::op_add(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	const a64::Inst::Id opcode = CarryIn
			? (inst.flags() ? a64::Inst::kIdAdcs : a64::Inst::kIdAdc)
			: (inst.flags() ? a64::Inst::kIdAdds : a64::Inst::kIdAdd);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp zero = select_register(a64::xzr, inst.size());
	const a64::Gp output = dstp.select_register(TEMP_REG3, inst.size());

	if (CarryIn && (m_carry_state != carry_state::CANONICAL))
	{
		m_carry_state = carry_state::CANONICAL;

		a.sbfx(TEMP_REG1, FLAGS_REG, uml::FLAG_BIT_C, 1);
		a.cmn(TEMP_REG1, 1);
	}

	if (src1p.is_immediate_value(0))
	{
		if (src2p.is_immediate_value(0))
		{
			if (CarryIn)
			{
				a.emit(opcode, output, zero, zero);
				mov_param_reg(a, inst.size(), dstp, output);
			}
			else
			{
				mov_param_reg(a, inst.size(), dstp, zero);
				a.emit(opcode, zero, zero, zero);
			}
		}
		else if (!CarryIn && src2p.is_immediate() && is_valid_immediate_addsub(src2p.immediate()))
		{
			a.mov(output, zero);
			a.emit(opcode, output, output, src2p.immediate());
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else if (!CarryIn && src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 24))
		{
			a.mov(output, src2p.immediate() & util::make_bitmask<uint64_t>(12));
			a.emit(opcode, output, output, src2p.immediate() & (util::make_bitmask<uint64_t>(12) << 12));
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else
		{
			const a64::Gp src = src2p.select_register(output, inst.size());

			mov_reg_param(a, inst.size(), src, src2p);
			a.emit(opcode, output, src, zero);
			mov_param_reg(a, inst.size(), dstp, output);
		}
	}
	else if (src2p.is_immediate_value(0))
	{
		if (!CarryIn && src1p.is_immediate() && is_valid_immediate_addsub(src1p.immediate()))
		{
			a.mov(output, zero);
			a.emit(opcode, output, output, src1p.immediate());
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else if (!CarryIn && src1p.is_immediate() && is_valid_immediate(src1p.immediate(), 24))
		{
			a.mov(output, src1p.immediate() & util::make_bitmask<uint64_t>(12));
			a.emit(opcode, output, output, src1p.immediate() & (util::make_bitmask<uint64_t>(12) << 12));
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else
		{
			const a64::Gp src = src1p.select_register(output, inst.size());

			mov_reg_param(a, inst.size(), src, src1p);
			a.emit(opcode, output, src, zero);
			mov_param_reg(a, inst.size(), dstp, output);
		}
	}
	else if (!CarryIn && src1p.is_immediate() && is_valid_immediate_addsub(src1p.immediate()))
	{
		const a64::Gp src = src2p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src2p);
		a.emit(opcode, output, src, src1p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else if (!CarryIn && src2p.is_immediate() && is_valid_immediate_addsub(src2p.immediate()))
	{
		const a64::Gp src = src1p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		a.emit(opcode, output, src, src2p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else if (!CarryIn && !inst.flags() && src1p.is_immediate() && is_valid_immediate(src1p.immediate(), 24))
	{
		const a64::Gp src = src2p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src2p);
		a.emit(opcode, output, src, src1p.immediate() & util::make_bitmask<uint64_t>(12));
		a.emit(opcode, output, output, src1p.immediate() & (util::make_bitmask<uint64_t>(12) << 12));
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else if (!CarryIn && !inst.flags() && src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 24))
	{
		const a64::Gp src = src1p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		a.emit(opcode, output, src, src2p.immediate() & util::make_bitmask<uint64_t>(12));
		a.emit(opcode, output, output, src2p.immediate() & (util::make_bitmask<uint64_t>(12) << 12));
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src1, src1p);
		if (src1p == src2p)
		{
			a.emit(opcode, output, src1, src1);
		}
		else
		{
			mov_reg_param(a, inst.size(), src2, src2p);
			a.emit(opcode, output, src1, src2);
		}
		mov_param_reg(a, inst.size(), dstp, output);
	}

	if (inst.flags())
	{
		if (inst.flags() & FLAG_C)
			store_carry(a);
		else
			m_carry_state = carry_state::POISON;
	}
}

template <bool CarryIn> void drcbe_arm64::op_sub(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	const a64::Inst::Id opcode = CarryIn
			? (inst.flags() ? a64::Inst::kIdSbcs : a64::Inst::kIdSbc)
			: (inst.flags() ? a64::Inst::kIdSubs : a64::Inst::kIdSub);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	if (CarryIn && (m_carry_state != carry_state::LOGICAL))
	{
		m_carry_state = carry_state::LOGICAL;

		a.ubfx(TEMP_REG1, FLAGS_REG, uml::FLAG_BIT_C, 1);
		a.cmp(a64::xzr, TEMP_REG1);
	}

	const a64::Gp zero = select_register(a64::xzr, inst.size());
	const a64::Gp output = dstp.select_register(TEMP_REG3, inst.size());

	if (src1p == src2p)
	{
		if (CarryIn || (inst.flags() && dstp.is_int_register()))
		{
			a.emit(opcode, output, zero, zero);
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else
		{
			mov_param_reg(a, inst.size(), dstp, zero);
			if (inst.flags())
				a.emit(opcode, zero, zero, zero);
		}
	}
	else if (src2p.is_immediate_value(0))
	{
		const a64::Gp src = src1p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		if (CarryIn || (inst.flags() && dstp.is_int_register()))
		{
			a.emit(opcode, output, src, zero);
			mov_param_reg(a, inst.size(), dstp, output);
		}
		else
		{
			mov_param_reg(a, inst.size(), dstp, src);
			if (inst.flags())
				a.emit(opcode, zero, src, zero);
		}
	}
	else if (!CarryIn && src2p.is_immediate() && is_valid_immediate_addsub(src2p.immediate()))
	{
		const a64::Gp src = src1p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		a.emit(opcode, output, src, src2p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else if (!CarryIn && (!inst.flags() || src1p.is_immediate_value(0)) && src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 24))
	{
		const a64::Gp src = src1p.select_register(output, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		a.emit(opcode, output, src, src2p.immediate() & util::make_bitmask<uint64_t>(12));
		a.emit(opcode, output, output, src2p.immediate() & (util::make_bitmask<uint64_t>(12) << 12));
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);
		a.emit(opcode, output, src1, src2);
		mov_param_reg(a, inst.size(), dstp, output);
	}

	if (inst.flags())
	{
		if (inst.flags() & FLAG_C)
			store_carry(a, true);
		else
			m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_cmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	if (src1p == src2p)
	{
		const a64::Gp zero = select_register(a64::xzr, inst.size());

		a.cmp(zero, zero);
	}
	else
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());

		mov_reg_param(a, inst.size(), src1, src1p);

		if (src2p.is_immediate() && is_valid_immediate_addsub(src2p.immediate()))
		{
			if (src2p.is_immediate_value(0))
				a.cmp(src1, select_register(a64::xzr, inst.size()));
			else
				a.cmp(src1, src2p.immediate());
		}
		else
		{
			const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());

			mov_reg_param(a, inst.size(), src2, src2p);
			a.cmp(src1, src2);
		}
	}

	if (inst.flags() & FLAG_C)
		store_carry(a, true);
	else
		m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_mulu(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	const bool compute_hi = (dstp != edstp);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp lo = ((inst.size() == 4) || compute_hi || inst.flags()) ? select_register(TEMP_REG3, inst.size()) : dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, select_register(a64::xzr, inst.size()));
		if (compute_hi || ((inst.size() == 8) && inst.flags()))
			a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			if (compute_hi || inst.flags())
				a.umulh(hi, src1, src2);
		}
		else
		{
			a.umull(lo.x(), src1, src2);
			if (compute_hi)
				a.lsr(hi.x(), lo.x(), 32);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);
	if (compute_hi)
		mov_param_reg(a, inst.size(), edstp, hi);

	if (inst.size() == 8)
	{
		if (inst.flags() & uml::FLAG_Z)
		{
			a.cmp(lo, 0);
			a.ccmp(hi, 0, 0, a64::CondCode::kEQ);
		}
		else if (inst.flags() & FLAG_S)
		{
			a.tst(hi, hi);
		}

		if (((inst.flags() & (uml::FLAG_Z | uml::FLAG_S)) == (uml::FLAG_Z | uml::FLAG_S)) || (inst.flags() & uml::FLAG_V))
		{
			a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

			if (inst.flags() & uml::FLAG_V)
			{
				a.tst(hi, hi); // overflow check
				a.cset(TEMP_REG3, a64::CondCode::kNE);
				a.bfi(SCRATCH_REG1, TEMP_REG3, 28, 1); // overflow flag
			}

			if ((inst.flags() & (uml::FLAG_Z | uml::FLAG_S)) == (uml::FLAG_Z | uml::FLAG_S))
			{
				a.lsr(TEMP_REG3, hi, inst.size() * 8 - 1); // take top bit of result as sign flag
				a.bfi(SCRATCH_REG1, TEMP_REG3, 31, 1); // sign flag
			}

			a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
		}
	}
	else
	{
		if (inst.flags() & (uml::FLAG_Z | uml::FLAG_S))
			a.tst(lo.x(), lo.x());

		if (inst.flags() & uml::FLAG_V)
		{
			a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

			a.tst(lo.x(), ~make_bitmask<uint64_t>(32));
			a.cset(SCRATCH_REG1, a64::CondCode::kNE);
			a.bfi(TEMP_REG1, SCRATCH_REG1, 28, 1); // overflow flag

			a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);
		}
	}

	if (inst.flags())
		m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_mululw(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp lo = (inst.flags() & uml::FLAG_V) ? select_register(TEMP_REG3, inst.size()) : dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, select_register(a64::xzr, inst.size()));
		if ((inst.flags() & uml::FLAG_V) && inst.size() == 8)
			a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			if (inst.flags() & uml::FLAG_V)
				a.umulh(hi, src1, src2);
		}
		else
		{
			if (inst.flags() & uml::FLAG_V)
				a.umull(lo.x(), src1, src2);
			else
				a.mul(lo, src1, src2);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);

	if (inst.flags() & (uml::FLAG_Z | uml::FLAG_S))
		a.tst(lo, lo);

	if (inst.flags() & uml::FLAG_V)
	{
		a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

		if (inst.size() == 8)
			a.tst(hi, hi);
		else
			a.tst(lo.x(), ~make_bitmask<uint64_t>(32));

		a.cset(SCRATCH_REG1, a64::CondCode::kNE);
		a.bfi(TEMP_REG1, SCRATCH_REG1, 28, 1); // overflow flag

		a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);
	}

	if (inst.flags())
		m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_muls(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	const bool compute_hi = (dstp != edstp);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp lo = ((inst.size() == 4) || compute_hi || inst.flags()) ? select_register(TEMP_REG3, inst.size()) : dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, select_register(a64::xzr, inst.size()));
		if (compute_hi || ((inst.size() == 8) && inst.flags()))
			a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			if (compute_hi || inst.flags())
				a.smulh(hi, src1, src2);
		}
		else
		{
			a.smull(lo.x(), src1, src2);
			if (compute_hi)
				a.lsr(hi.x(), lo.x(), 32);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);
	if (compute_hi)
		mov_param_reg(a, inst.size(), edstp, hi);

	if (inst.size() == 8)
	{
		if (inst.flags() & uml::FLAG_Z)
		{
			a.cmp(lo, 0);
			a.ccmp(hi, 0, 0, a64::CondCode::kEQ);
		}
		else if (inst.flags() & FLAG_S)
		{
			a.tst(hi, hi);
		}

		if (((inst.flags() & (uml::FLAG_Z | uml::FLAG_S)) == (uml::FLAG_Z | uml::FLAG_S)) || (inst.flags() & uml::FLAG_V))
		{
			a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

			if (inst.flags() & uml::FLAG_V)
			{
				a.asr(TEMP_REG1, lo, 63);
				a.cmp(TEMP_REG1, hi);
				a.cset(TEMP_REG1, a64::CondCode::kNE);
				a.bfi(SCRATCH_REG1, TEMP_REG1, 28, 1); // overflow flag
			}

			if ((inst.flags() & (uml::FLAG_Z | uml::FLAG_S)) == (uml::FLAG_Z | uml::FLAG_S))
			{
				a.lsr(TEMP_REG1, hi, inst.size() * 8 - 1); // take top bit of result as sign flag
				a.bfi(SCRATCH_REG1, TEMP_REG1, 31, 1); // sign flag
			}

			a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
		}
	}
	else
	{
		if (inst.flags() & (uml::FLAG_Z | uml::FLAG_S))
			a.tst(lo.x(), lo.x());

		if (inst.flags() & uml::FLAG_V)
		{
			a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

			a.sxtw(TEMP_REG1, lo.w());
			a.cmp(TEMP_REG1, lo.x());
			a.cset(TEMP_REG1, a64::CondCode::kNE);
			a.bfi(SCRATCH_REG1, TEMP_REG1, 28, 1); // overflow flag

			a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
		}
	}

	if (inst.flags())
		m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_mulslw(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp lo = (inst.flags() & uml::FLAG_V) ? select_register(TEMP_REG3, inst.size()) : dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, select_register(a64::xzr, inst.size()));
		if ((inst.flags() & uml::FLAG_V) && inst.size() == 8)
			a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			if (inst.flags() & uml::FLAG_V)
				a.smulh(hi, src1, src2);
		}
		else
		{
			if (inst.flags() & uml::FLAG_V)
				a.smull(lo.x(), src1, src2);
			else
				a.mul(lo, src1, src2);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);

	if (inst.flags() & (uml::FLAG_Z | uml::FLAG_S))
		a.tst(lo, lo);

	if (inst.flags() & uml::FLAG_V)
	{
		a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

		if (inst.size() == 4)
		{
			a.sxtw(TEMP_REG1, lo.w());
			a.cmp(TEMP_REG1, lo.x());
		}
		else
		{
			a.asr(TEMP_REG1, lo, 63);
			a.cmp(TEMP_REG1, hi);
		}

		a.cset(TEMP_REG1, a64::CondCode::kNE);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 28, 1); // overflow flag

		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}

	if (inst.flags())
		m_carry_state = carry_state::POISON;
}

template <a64::Inst::Id Opcode> void drcbe_arm64::op_div(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter edstp(*this, inst.param(1), PTYPE_MR);
	be_parameter src1p(*this, inst.param(2), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(3), PTYPE_MRI);
	const bool compute_rem = (dstp != edstp);

	if (!src2p.is_immediate() || (src2p.is_immediate() && !src2p.is_immediate_value(0)))
	{
		Label skip_zero = a.new_label();
		Label skip = a.new_label();

		const a64::Gp temp = select_register(TEMP_REG1, inst.size());
		const a64::Gp temp2 = select_register(TEMP_REG2, inst.size());
		const a64::Gp temp3 = select_register(TEMP_REG3, inst.size());

		mov_reg_param(a, inst.size(), temp2, src2p);
		a.cbz(temp2, skip_zero);

		mov_reg_param(a, inst.size(), temp, src1p);

		a.emit(Opcode, temp3, temp, temp2);

		mov_param_reg(a, inst.size(), dstp, temp3);

		if (compute_rem)
		{
			a.msub(temp2, temp3, temp2, temp);
			mov_param_reg(a, inst.size(), edstp, temp2);
		}

		if (inst.flags())
			a.tst(temp3, temp3);

		a.b(skip);

		a.bind(skip_zero);
		a.mov(SCRATCH_REG1, 1 << 28); // set overflow flag
		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);

		a.bind(skip);
	}
	else
	{
		a.mov(SCRATCH_REG1, 1 << 28); // set overflow flag
		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}
	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_and(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	const a64::Inst::Id opcode = inst.flags() ? a64::Inst::kIdAnds : a64::Inst::kIdAnd;

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	if (inst.param(0) == inst.param(2))
	{
		using std::swap;
		swap(src1p, src2p);
	}

	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp src1 = src1p.select_register(dst, inst.size());

	if (src1p.is_immediate_value(0) || src2p.is_immediate_value(0))
	{
		if (inst.flags())
			a.ands(dst, select_register(a64::xzr, inst.size()), 1); // immediate value doesn't matter, result will be zero
		else
			a.mov(dst, 0);
	}
	else if (src1p.is_immediate() && src2p.is_immediate())
	{
		get_imm_relative(a, dst, src1p.immediate() & src2p.immediate());

		if (inst.flags())
			a.tst(dst, dst);
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		a.emit(opcode, dst, src1, src2p.immediate());
	}
	else if ((inst.size() == 8) && src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), 4) && (!inst.flags() || !BIT(src2p.immediate(), 31)))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		a.emit(opcode, dst.w(), src1.w(), src2p.immediate());
	}
	else if (src2p.is_immediate_value(util::make_bitmask<uint64_t>(inst.size() * 8)) || (src1p == src2p))
	{
		if ((dstp == src1p) && !inst.flags())
		{
			if ((inst.size() == 8) || (dstp.is_memory() && !dstp.is_cold_register()))
				return;
		}

		mov_reg_param(a, inst.size(), src1, src1p);

		if ((dst.id() != src1.id()) || ((inst.size() == 4) && (dstp == src1p) && dstp.is_int_register()))
			a.emit(opcode, dst, src1, src1);
		else if (inst.flags())
			a.tst(dst, dst);
	}
	else
	{
		const a64::Gp src2 = src2p.select_register(TEMP_REG1, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.emit(opcode, dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
		m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_test(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());

	if (src2p.is_immediate_value(0))
	{
		const a64::Gp zero = select_register(a64::xzr, inst.size());

		a.tst(zero, zero);
	}
	else if (src2p.is_immediate_value(util::make_bitmask<uint64_t>(inst.size() * 8)))
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		a.tst(src1, src1);
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		a.tst(src1, src2p.immediate());
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);
		a.tst(src1, src2);
	}

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_or(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	if (inst.param(0) == inst.param(2))
	{
		using std::swap;
		swap(src1p, src2p);
	}

	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp src1 = src1p.select_register(dst, inst.size());

	if (src2p.is_immediate_value(util::make_bitmask<uint64_t>(inst.size() * 8)))
	{
		a.mov(dst, src2p.immediate());
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		a.orr(dst, src1, src2p.immediate());
	}
	else
	{
		const a64::Gp src2 = src2p.select_register(TEMP_REG1, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.orr(dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.tst(dst, dst);
		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_xor(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	if (inst.param(0) == inst.param(2))
	{
		using std::swap;
		swap(src1p, src2p);
	}

	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());
	const a64::Gp src1 = src1p.select_register(dst, inst.size());

	if (src2p.is_immediate_value(util::make_bitmask<uint64_t>(inst.size() * 8)))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		a.mvn(dst, src1);
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		a.eor(dst, src1, src2p.immediate());
	}
	else
	{
		const a64::Gp src2 = src2p.select_register(TEMP_REG1, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.eor(dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.tst(dst, dst);
		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_lzcnt(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp dst = dstp.select_register(TEMP_REG2, inst.size());

	mov_reg_param(a, inst.size(), src, srcp);

	a.clz(dst, src);

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.tst(dst, dst);
		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_tzcnt(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp dst = dstp.select_register(TEMP_REG2, inst.size());
	const a64::Gp temp = select_register(TEMP_REG3, inst.size());

	mov_reg_param(a, inst.size(), src, srcp);

	a.rbit(dst, src); // reverse bits to count the tail bits from the head
	a.clz(dst, dst);

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.eor(temp, dst, inst.size() * 8);
		a.tst(temp, temp);
		m_carry_state = carry_state::POISON;
	}
}

void drcbe_arm64::op_bswap(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());

	mov_reg_param(a, inst.size(), src, srcp);

	if (inst.size() == 8)
		a.rev64(dst, src);
	else
		a.rev32(dst, src);

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
	{
		a.tst(dst, dst);
		m_carry_state = carry_state::POISON;
	}
}


template <a64::Inst::Id Opcode> void drcbe_arm64::op_shift(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = (inst.size() * 8) - 1;
	bool const rightShift = (Opcode == a64::Inst::kIdRor) || (Opcode == a64::Inst::kIdLsr) || (Opcode == a64::Inst::kIdAsr);

	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());
	const a64::Gp src = src1p.select_register(dst, inst.size());
	const a64::Gp scratch = select_register(TEMP_REG2, inst.size());
	const a64::Gp shift = src2p.select_register(scratch, inst.size());

	if (src2p.is_immediate())
	{
		const auto si = src2p.immediate() & maxBits;

		if (!si)
		{
			if (inst.flags() & FLAG_C)
				store_carry_reg(a, a64::xzr);

			mov_reg_param(a, inst.size(), dst, src1p);
		}
		else
		{
			mov_reg_param(a, inst.size(), src, src1p);

			if (inst.flags() & FLAG_C)
			{
				if (rightShift)
					calculate_carry_shift_right_imm(a, src, si);
				else
					calculate_carry_shift_left_imm(a, src, si, maxBits);
			}

			a.emit(Opcode, dst, src, si);
		}
	}
	else
	{
		mov_reg_param(a, inst.size(), shift, src2p);
		a.and_(scratch, shift, maxBits);

		mov_reg_param(a, inst.size(), src, src1p);

		if (inst.flags() & FLAG_C)
		{
			if (rightShift)
				calculate_carry_shift_right(a, src, scratch);
			else
				calculate_carry_shift_left(a, src, scratch, maxBits);
		}

		a.emit(Opcode, dst, src, scratch);
	}

	if (inst.flags() & (FLAG_Z | FLAG_S))
		a.tst(dst, dst);
	if (inst.flags())
		m_carry_state = carry_state::POISON;

	// save dst after using inputs for calculations so the registers have no chance of being overwritten
	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_rol(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = (inst.size() * 8) - 1;

	const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());
	const a64::Gp src = src1p.select_register(dst, inst.size());
	const a64::Gp scratch = select_register(TEMP_REG2, inst.size());
	const a64::Gp shift = src2p.select_register(scratch, inst.size());

	if (src2p.is_immediate())
	{
		const auto si = -src2p.immediate() & maxBits;

		if (!si)
		{
			mov_reg_param(a, inst.size(), dst, src1p);

			if (inst.flags() & FLAG_C)
				store_carry_reg(a, a64::xzr);
		}
		else
		{
			mov_reg_param(a, inst.size(), src, src1p);

			a.ror(dst, src, si);

			if (inst.flags() & FLAG_C)
				store_carry_reg(a, dst);
		}
	}
	else
	{
		mov_reg_param(a, inst.size(), shift, src2p);
		a.neg(scratch, shift);

		mov_reg_param(a, inst.size(), src, src1p);

		if (inst.flags() & FLAG_C)
			a.ands(scratch, scratch, maxBits);
		else
			a.and_(scratch, scratch, maxBits);

		a.ror(dst, src, scratch);

		if (inst.flags() & FLAG_C)
		{
			const a64::Gp zero = select_register(a64::xzr, inst.size());

			a.csel(scratch, zero, dst, a64::CondCode::kZero);
			store_carry_reg(a, scratch);
		}
	}

	if (inst.flags() & (FLAG_Z | FLAG_S))
		a.tst(dst, dst);
	if (inst.flags())
		m_carry_state = carry_state::POISON;

	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_rolc(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = (inst.size() * 8) - 1;

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp param1 = src1p.select_register(TEMP_REG3, inst.size());
	const a64::Gp output = can_use_dst_reg ? dstp.select_register(TEMP_REG1, inst.size()) : select_register(TEMP_REG1, inst.size());

	bool zs_flags_done = false;

	// shift > 1: src = (PARAM1 << shift) | (carry << (shift - 1)) | (PARAM1 >> (33 - shift))
	// shift = 1: src = (PARAM1 << shift) | carry

	if (src2p.is_immediate())
	{
		const auto si = src2p.immediate() & maxBits;

		if (!si)
		{
			mov_reg_param(a, inst.size(), output, src1p);
		}
		else if (src1p.is_immediate_value(0))
		{
			// this depends on carry being the least significant bit of the flags
			a.ubfiz(output, select_register(FLAGS_REG, inst.size()), si - 1, 1);

			if (inst.flags() & FLAG_C)
				store_carry_reg(a, a64::xzr);

			m_carry_state = carry_state::POISON;
		}
		else if ((si == 1) && (m_carry_state == carry_state::CANONICAL))
		{
			mov_reg_param(a, inst.size(), param1, src1p);

			if (inst.flags())
			{
				a.adcs(output, param1, param1);

				if (inst.flags() & FLAG_C)
					store_carry(a);
				else
					m_carry_state = carry_state::POISON;
			}
			else
			{
				a.adc(output, param1, param1);
			}

			zs_flags_done = true;
		}
		else
		{
			mov_reg_param(a, inst.size(), param1, src1p);

			// this depends on carry being the least significant bit of the flags
			if (si > 1)
				a.lsr(output, param1, (inst.size() * 8) + 1 - si);
			a.bfi(output, select_register(FLAGS_REG, inst.size()), si - 1, 1);
			a.bfi(output, param1, si, (inst.size() * 8) - si);

			if (inst.flags() & FLAG_C)
				calculate_carry_shift_left_imm(a, param1, si, maxBits);
		}
	}
	else if (src1p.is_immediate_value(0))
	{
		const a64::Gp scratch = select_register(TEMP_REG2, inst.size());
		const a64::Gp shift = src2p.select_register(scratch, inst.size());
		const a64::Gp zero = select_register(a64::xzr, inst.size());

		mov_reg_param(a, inst.size(), shift, src2p);

		a.ands(scratch, shift, maxBits);
		get_carry(a, output);
		a.sub(scratch, scratch, 1);
		a.lsl(output, output, scratch);
		a.csel(output, zero, output, a64::CondCode::kZero);

		if (inst.flags() & FLAG_C)
		{
			// this depends on carry being the least significant bit of the flags
			a.csel(scratch.x(), FLAGS_REG, a64::xzr, a64::CondCode::kZero);
			store_carry_reg(a, scratch);

			m_carry_state = carry_state::POISON;
		}
	}
	else
	{
		const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());
		const a64::Gp carry = select_register(SCRATCH_REG2, inst.size());
		const a64::Gp scratch2 = select_register(FUNC_SCRATCH_REG, inst.size());

		mov_reg_param(a, inst.size(), param1, src1p);
		mov_reg_param(a, inst.size(), shift, src2p);

		a.and_(scratch2, shift, maxBits);

		a.lsl(output, param1, scratch2); // PARAM1 << shift

		Label skip = a.new_label();
		Label skip3 = a.new_label();
		a.cbz(scratch2, skip3);

		get_carry(a, carry);

		a.sub(scratch, scratch2, 1);
		a.cbz(scratch, skip);

		// add carry flag to output
		a.lsl(carry, carry, scratch);

		a.mov(scratch, maxBits + 2); // PARAM1 >> (33 - shift)
		a.sub(scratch, scratch, scratch2);
		a.lsr(scratch, param1, scratch);
		a.orr(output, output, scratch);

		a.bind(skip);

		a.orr(output, output, carry);

		if (inst.flags() & FLAG_C)
			calculate_carry_shift_left(a, param1, scratch2, maxBits);

		a.bind(skip3);
	}

	if (!zs_flags_done && (inst.flags() & (FLAG_Z | FLAG_S)))
		a.tst(output, output);

	mov_param_reg(a, inst.size(), dstp, output);

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_rorc(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = (inst.size() * 8) - 1;

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp param1 = src1p.select_register(TEMP_REG3, inst.size());
	const a64::Gp output = can_use_dst_reg ? dstp.select_register(TEMP_REG1, inst.size()) : select_register(TEMP_REG1, inst.size());

	// if (shift > 1)
	//  src = (PARAM1 >> shift) | (((flags & FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
	// else if (shift == 1)
	//  src = (PARAM1 >> shift) | ((flags & FLAG_C) << 31);

	if (src2p.is_immediate())
	{
		const auto si = src2p.immediate() & maxBits;

		if (!si)
		{
			mov_reg_param(a, inst.size(), output, src1p);
		}
		else if (src1p.is_immediate_value(0))
		{
			// this depends on carry being the least significant bit of the flags
			a.ubfiz(output, select_register(FLAGS_REG, inst.size()), (inst.size() * 8) - si, 1);

			if (inst.flags() & FLAG_C)
				store_carry_reg(a, a64::xzr);

			m_carry_state = carry_state::POISON;
		}
		else
		{
			mov_reg_param(a, inst.size(), param1, src1p);

			// this depends on carry being the least significant bit of the flags
			a.lsr(output, param1, si);
			a.bfi(output, select_register(FLAGS_REG, inst.size()), (inst.size() * 8) - si, 1);
			if (si > 1)
				a.bfi(output, param1, (inst.size() * 8) + 1 - si, si - 1);

			if (inst.flags() & FLAG_C)
				calculate_carry_shift_right_imm(a, param1, si);
		}
	}
	else if (src1p.is_immediate_value(0))
	{
		const a64::Gp scratch = select_register(TEMP_REG2, inst.size());
		const a64::Gp shift = src2p.select_register(scratch, inst.size());
		const a64::Gp zero = select_register(a64::xzr, inst.size());

		mov_reg_param(a, inst.size(), shift, src2p);

		a.ands(scratch, shift, maxBits);
		get_carry(a, output);
		a.ror(output, output, scratch);
		a.csel(output, zero, output, a64::CondCode::kZero);

		if (inst.flags() & FLAG_C)
		{
			a.csel(scratch.x(), FLAGS_REG, a64::xzr, a64::CondCode::kZero);
			store_carry_reg(a, scratch, uml::FLAG_BIT_C);

			m_carry_state = carry_state::POISON;
		}
	}
	else
	{
		const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());
		const a64::Gp carry = select_register(SCRATCH_REG2, inst.size());
		const a64::Gp scratch2 = select_register(FUNC_SCRATCH_REG, inst.size());

		mov_reg_param(a, inst.size(), param1, src1p);
		mov_reg_param(a, inst.size(), shift, src2p);

		a.and_(scratch2, shift, maxBits);

		a.lsr(output, param1, shift); // PARAM1 >> shift

		Label skip = a.new_label();
		Label skip3 = a.new_label();
		a.cbz(scratch2, skip3);

		get_carry(a, carry);
		a.lsl(carry, carry, maxBits); // (flags & FLAG_C) << 31

		a.sub(scratch, scratch2, 1); // carry >> (shift - 1)
		a.cbz(scratch, skip);

		// add carry flag to output
		a.lsr(carry, carry, scratch);

		a.mov(scratch, maxBits + 2); // PARAM1 << (33 - shift)
		a.sub(scratch, scratch, scratch2);
		a.lsl(scratch, param1, scratch);
		a.orr(output, output, scratch);

		a.bind(skip);

		a.orr(output, output, carry);

		if (inst.flags() & FLAG_C)
			calculate_carry_shift_right(a, param1, scratch2);

		a.bind(skip3);
	}

	if (inst.flags() & (FLAG_Z | FLAG_S))
		a.tst(output, output);

	mov_param_reg(a, inst.size(), dstp, output);

	m_carry_state = carry_state::POISON;
}

void drcbe_arm64::op_fload(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter basep(*this, inst.param(1), PTYPE_M);
	be_parameter indp(*this, inst.param(2), PTYPE_MRI);

	const a64::Vec dstreg = dstp.select_register(TEMPF_REG1, inst.size());
	const a64::Gp basereg = TEMP_REG1;

	get_imm_relative(a, basereg, uint64_t(basep.memory()));

	if (indp.is_immediate())
	{
		a.ldr(dstreg, a64::Mem(basereg, int32_t(uint32_t(indp.immediate())) * inst.size()));
	}
	else
	{
		const a64::Gp indreg = TEMP_REG1.x();
		if (indp.is_int_register())
			a.sxtw(indreg, indp.get_register_int(4));
		else if ((util::endianness::native == util::endianness::big) && indp.is_cold_register())
			emit_ldrsw_mem(a, indreg, reinterpret_cast<uint8_t *>(indp.memory()) + 4);
		else
			emit_ldrsw_mem(a, indreg, indp.memory());

		a.ldr(dstreg, a64::Mem(basereg, indreg, a64::lsl((inst.size() == 4) ? 2 : 3)));
	}

	mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_fstore(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter basep(*this, inst.param(0), PTYPE_M);
	be_parameter indp(*this, inst.param(1), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(2), PTYPE_MF);

	const a64::Vec srcreg = srcp.select_register(TEMPF_REG1, inst.size());
	const a64::Gp basereg = TEMP_REG1;

	get_imm_relative(a, basereg, uint64_t(basep.memory()));

	mov_float_reg_param(a, inst.size(), srcreg, srcp);

	if (indp.is_immediate())
	{
		a.str(srcreg, a64::Mem(basereg, int32_t(uint32_t(indp.immediate())) * inst.size()));
	}
	else
	{
		const a64::Gp indreg = TEMP_REG1.x();
		if (indp.is_int_register())
			a.sxtw(indreg, indp.get_register_int(4));
		else if ((util::endianness::native == util::endianness::big) && indp.is_cold_register())
			emit_ldrsw_mem(a, indreg, reinterpret_cast<uint8_t *>(indp.memory()) + 4);
		else
			emit_ldrsw_mem(a, indreg, indp.memory());

		a.str(srcreg, a64::Mem(basereg, indreg, a64::lsl((inst.size() == 4) ? 2 : 3)));
	}
}

void drcbe_arm64::op_fread(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());
	assert((1 << spacesizep.size()) == inst.size());

	auto const &accessors = m_memory_accessors[spacesizep.space()];

	mov_reg_param(a, 4, REG_PARAM2, addrp);

	if (inst.size() == 4)
	{
		get_imm_relative(a, REG_PARAM1, accessors.resolved.read_dword.obj);
		call_arm_addr(a, accessors.resolved.read_dword.func);

		mov_float_param_int_reg(a, inst.size(), dstp, REG_PARAM1.w());
	}
	else if (inst.size() == 8)
	{
		get_imm_relative(a, REG_PARAM1, accessors.resolved.read_qword.obj);
		call_arm_addr(a, accessors.resolved.read_qword.func);

		mov_float_param_int_reg(a, inst.size(), dstp, REG_PARAM1);
	}
}

void drcbe_arm64::op_fwrite(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	m_carry_state = carry_state::POISON;

	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());
	assert((1 << spacesizep.size()) == inst.size());

	auto const &accessors = m_memory_accessors[spacesizep.space()];

	mov_reg_param(a, 4, REG_PARAM2, addrp);
	mov_float_reg_param(a, inst.size(), TEMPF_REG1, srcp);

	a.fmov(select_register(REG_PARAM3, inst.size()), select_register(TEMPF_REG1, inst.size()));

	if (inst.size() == 4)
	{
		get_imm_relative(a, REG_PARAM1, accessors.resolved.write_dword.obj);
		call_arm_addr(a, accessors.resolved.write_dword.func);
	}
	else if (inst.size() == 8)
	{
		get_imm_relative(a, REG_PARAM1, accessors.resolved.write_qword.obj);
		call_arm_addr(a, accessors.resolved.write_qword.func);
	}
}

void drcbe_arm64::op_fmov(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// decide whether a conditional select will be efficient
	bool usesel = dstp.is_float_register() && srcp.is_float_register();
	switch (inst.condition())
	{
		case uml::COND_ALWAYS:
		case uml::COND_U:
		case uml::COND_NU:
			usesel = false;
			break;
		case uml::COND_C:
		case uml::COND_NC:
			switch (m_carry_state)
			{
				case carry_state::CANONICAL:
				case carry_state::LOGICAL:
					break;
				default:
					usesel = false;
			}
			break;
		default:
			break;
	}

	if (usesel)
	{
		const a64::Vec dstreg = dstp.select_register(TEMPF_REG1, inst.size());
		const a64::Vec srcreg = srcp.select_register(TEMPF_REG2, inst.size());

		mov_float_reg_param(a, inst.size(), dstreg, dstp);
		mov_float_reg_param(a, inst.size(), srcreg, srcp);

		switch (inst.condition())
		{
			case uml::COND_C:
			case uml::COND_NC:
				if (m_carry_state == carry_state::CANONICAL)
					a.fcsel(dstreg, srcreg, dstreg, ARM_NOT_CONDITION(inst.condition()));
				else
					a.fcsel(dstreg, srcreg, dstreg, ARM_CONDITION(inst.condition()));
				break;
			case uml::COND_A:
			case uml::COND_BE:
				load_carry(a, true);
				[[fallthrough]];
			default:
				a.fcsel(dstreg, srcreg, dstreg, ARM_CONDITION(inst.condition()));
		}

		mov_float_param_reg(a, inst.size(), dstp, dstreg);
	}
	else
	{
		Label skip;
		emit_skip(a, inst.condition(), skip);

		mov_float_param_param(a, inst.size(), dstp, srcp);

		if (inst.condition() != uml::COND_ALWAYS)
			a.bind(skip);
	}
}

void drcbe_arm64::op_ftoint(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());
	const parameter &roundp = inst.param(3);
	assert(roundp.is_rounding());

	const a64::Gp dstreg = dstp.select_register(TEMP_REG1, 1 << sizep.size());
	const a64::Vec srcreg = srcp.select_register(TEMPF_REG1, inst.size());

	if (!srcp.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg, srcp);

	switch (roundp.rounding())
	{
		case ROUND_TRUNC:
			a.fcvtzs(dstreg, srcreg);
			break;

		case ROUND_ROUND:
			a.fcvtns(dstreg, srcreg);
			break;

		case ROUND_CEIL:
			a.fcvtps(dstreg, srcreg);
			break;

		case ROUND_FLOOR:
			a.fcvtms(dstreg, srcreg);
			break;

		case ROUND_DEFAULT:
		default:
			{
				Label base = a.new_label();
				Label done = a.new_label();

				// this depends on each case being two instructions
				emit_ldrb_mem(a, TEMP_REG1.w(), &m_state.fmod);
				a.adr(TEMP_REG2, base);
				a.add(TEMP_REG2, TEMP_REG2, TEMP_REG1, a64::lsl(3));
				a.br(TEMP_REG2);

				a.bind(base);
				a.fcvtzs(dstreg, srcreg); // 0 - TRUNC
				a.b(done);
				a.fcvtns(dstreg, srcreg); // 1 - ROUND
				a.b(done);
				a.fcvtps(dstreg, srcreg); // 2 - CEIL
				a.b(done);
				a.fcvtms(dstreg, srcreg); // 3 - FLOOR
				a.bind(done);
			}
			break;
	}

	mov_param_reg(a, 1 << sizep.size(), dstp, dstreg);
}

void drcbe_arm64::op_ffrint(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());

	const a64::Vec dstreg = dstp.select_register(TEMPF_REG1, inst.size());
	const a64::Gp srcreg = srcp.select_register(TEMP_REG1, 1 << sizep.size());

	if (!srcp.is_int_register())
		mov_reg_param(a, 1 << sizep.size(), srcreg, srcp);

	a.scvtf(dstreg, srcreg);

	if (!dstp.is_float_register())
		mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_ffrflt(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);
	const parameter &sizep = inst.param(2);
	assert(sizep.is_size());

	const a64::Vec dstreg = dstp.select_register(TEMPF_REG1, inst.size());
	const a64::Vec srcreg = srcp.select_register(TEMPF_REG2, 1 << sizep.size());

	if (!srcp.is_float_register())
		mov_float_reg_param(a, 1 << sizep.size(), srcreg, srcp);

	// double to float, or float to double
	a.fcvt(dstreg, srcreg);

	if (!dstp.is_float_register())
		mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_frnds(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	const a64::Vec dstreg = dstp.select_register(TEMPF_REG2, inst.size());
	const a64::Vec srcreg = srcp.select_register(TEMPF_REG1, inst.size());

	if (!srcp.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg, srcp);

	a.fcvt(dstreg.s(), srcreg.d()); // convert double to short
	a.fcvt(dstreg.d(), dstreg.s()); // convert short to double

	if (!dstp.is_float_register())
		mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_fcmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_U);

	be_parameter src1p(*this, inst.param(0), PTYPE_MF);
	be_parameter src2p(*this, inst.param(1), PTYPE_MF);

	const a64::Vec srcreg1 = src1p.select_register(TEMPF_REG1, inst.size());
	const a64::Vec srcreg2 = src2p.select_register(TEMPF_REG2, inst.size());

	if (!src1p.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg1, src1p);
	if (!src2p.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg2, src2p);

	a.fcmp(srcreg1, srcreg2);

	if (inst.flags() & FLAG_C)
		store_carry(a, true);
	else
		m_carry_state = carry_state::POISON;
	if (inst.flags() & FLAG_U)
	{
		a.cset(SCRATCH_REG1, a64::CondCode::kVS);
		a.bfi(FLAGS_REG, SCRATCH_REG1, uml::FLAG_BIT_U, 1);
	}
}

template <a64::Inst::Id Opcode> void drcbe_arm64::op_float_alu(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter src1p(*this, inst.param(1), PTYPE_MF);
	be_parameter src2p(*this, inst.param(2), PTYPE_MF);

	// pick a target register for the general case
	const a64::Vec dstreg = dstp.select_register(TEMPF_REG3, inst.size());
	const a64::Vec srcreg1 = src1p.select_register(TEMPF_REG1, inst.size());
	const a64::Vec srcreg2 = src2p.select_register(TEMPF_REG2, inst.size());

	if (!src1p.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg1, src1p);
	if (!src2p.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg2, src2p);

	a.emit(Opcode, dstreg, srcreg1, srcreg2);

	if (!dstp.is_float_register())
		mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

template <a64::Inst::Id Opcode> void drcbe_arm64::op_float_alu2(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	// pick a target register for the general case
	const a64::Vec dstreg = dstp.select_register(TEMPF_REG2, inst.size());
	const a64::Vec srcreg = srcp.select_register(TEMPF_REG1, inst.size());

	if (!srcp.is_float_register())
		mov_float_reg_param(a, inst.size(), srcreg, srcp);

	a.emit(Opcode, dstreg, srcreg);

	if (!dstp.is_float_register())
		mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_fcopyi(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MF);
	be_parameter srcp(*this, inst.param(1), PTYPE_MR);

	const a64::Vec dstreg = dstp.select_register(TEMPF_REG1, inst.size());
	const a64::Gp srcreg = srcp.select_register(TEMP_REG1, inst.size());

	mov_reg_param(a, inst.size(), srcreg, srcp);
	a.fmov(dstreg, srcreg);
	mov_float_param_reg(a, inst.size(), dstp, dstreg);
}

void drcbe_arm64::op_icopyf(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MF);

	const a64::Gp dstreg = dstp.select_register(TEMP_REG1, inst.size());
	const a64::Vec srcreg = srcp.select_register(TEMPF_REG1, inst.size());

	mov_float_reg_param(a, inst.size(), srcreg, srcp);
	a.fmov(dstreg, srcreg);
	mov_param_reg(a, inst.size(), dstp, dstreg);
}

} // anonymous namespace


std::unique_ptr<drcbe_interface> make_drcbe_arm64(
		drcuml_state &drcuml,
		device_t &device,
		drc_cache &cache,
		uint32_t flags,
		int modes,
		int addrbits,
		int ignorebits)
{
	return std::unique_ptr<drcbe_interface>(new drcbe_arm64(drcuml, device, cache, flags, modes, addrbits, ignorebits));
}

} // namespace drc
