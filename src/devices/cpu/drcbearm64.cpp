// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

Register use:

r0      first function parameter/return value
r1      second function parameter
r2      third function parameter
r3      fourth function parameter
r4
r5
r6
r7
r8
r9      temporary for intermediate values
r10     temporary for intermediate values
r11     temporary for intermediate values
r12     scratch register used by helper functions
r13     scratch register used by helper functions
r14     scratch register used for address calculation
r15     temporary used in opcode functions
r16
r17
r18
r19     UML register I0
r20     UML register I1
r21     UML register I2
r22     UML register I3
r23     UML register I4
r24     UML register I5
r35     UML register I6
r26     UML register I7
r27     near cache pointer
r28     emulated flags
r29     base generated code frame pointer
r30     link register
sp      stack pointer


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

***************************************************************************/

#include "emu.h"
#include "drcbearm64.h"

#include "drcbeut.h"
#include "uml.h"

#include "debug/debugcpu.h"
#include "emuopts.h"

#include "asmjit/src/asmjit/asmjit.h"
#include "asmjit/src/asmjit/a64.h"

#include <cstddef>
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
const a64::Gp SCRATCH_REG1 = a64::x12;
const a64::Gp SCRATCH_REG2 = a64::x13;

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


#define ARM_CONDITION(a, condition)        (condition_map[condition - COND_Z])
#define ARM_NOT_CONDITION(a, condition)    (negateCond(condition_map[condition - COND_Z]))

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
	if (is_valid_immediate(val, 12) || (!(val & 0xfff) && is_valid_immediate(val, 12 + 12)))
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
	if (is_valid_immediate(val, 12) || ((val & 0xfff) == 0 && is_valid_immediate(val >> 12, 12)))
	{
		a.sub(dst, src, val);
		return true;
	}

	return false;
}

void get_imm_absolute(a64::Assembler &a, const a64::Gp &reg, const uint64_t val)
{
	// Check for constants that can be generated with a single instruction
	if (is_simple_mov_immediate(val, reg.isGpX() ? 8 : 4))
	{
		a.mov(reg, val);
		return;
	}
	else if (reg.isGpX() && is_valid_immediate_mask(val, 4))
	{
		a.mov(reg.w(), val); // asmjit isn't smart enough to work this out
		return;
	}

	// Values close to the program counter can be generated with a single adr
	const uint64_t codeoffs = a.code()->baseAddress() + a.offset();
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


class drcbe_arm64 : public drcbe_interface
{
	using arm64_entry_point_func = uint32_t (*)(void *entry);

public:
	drcbe_arm64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_arm64();

	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst) override;
	virtual bool hash_exists(uint32_t mode, uint32_t pc) override;
	virtual void get_info(drcbe_info &info) override;
	virtual bool logging() const override { return false; }

private:
	class be_parameter
	{
		static inline constexpr int REG_MAX = 30;

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

		be_parameter() : m_type(PTYPE_NONE), m_value(0) { }
		be_parameter(uint64_t val) : m_type(PTYPE_IMMEDIATE), m_value(val) { }
		be_parameter(drcbe_arm64 &drcbe, const uml::parameter &param, uint32_t allowed);
		be_parameter(const be_parameter &param) = default;

		static be_parameter make_ireg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_INT_REGISTER, regnum); }
		static be_parameter make_freg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static be_parameter make_memory(void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(base)); }
		static be_parameter make_memory(const void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(const_cast<void *>(base))); }

		bool operator==(const be_parameter &rhs) const { return (m_type == rhs.m_type && m_value == rhs.m_value); }
		bool operator!=(const be_parameter &rhs) const { return (m_type != rhs.m_type || m_value != rhs.m_value); }

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

		asmjit::a64::Vec get_register_float(uint32_t regsize) const;
		asmjit::a64::Gp get_register_int(uint32_t regsize) const;
		asmjit::a64::Vec select_register(asmjit::a64::Vec const &reg, uint32_t regsize) const;
		asmjit::a64::Gp select_register(asmjit::a64::Gp const &reg, uint32_t regsize) const;

	private:
		be_parameter(be_parameter_type type, be_parameter_value value) : m_type(type), m_value(value) { }

		be_parameter_type   m_type;
		be_parameter_value  m_value;
	};

	void op_handle(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_hash(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_label(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_comment(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_mapvar(asmjit::a64::Assembler &a, const uml::instruction &inst);

	void op_nop(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_break(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_debug(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_exit(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_hashjmp(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_jmp(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_exh(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_callh(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_ret(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_callc(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_recover(asmjit::a64::Assembler &a, const uml::instruction &inst);

	void op_setfmod(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_getfmod(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_getexp(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_getflgs(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_setflgs(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_save(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_restore(asmjit::a64::Assembler &a, const uml::instruction &inst);

	void op_load(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_loads(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_store(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_read(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_readm(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_write(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_writem(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_carry(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_set(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_mov(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_sext(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_roland(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_rolins(asmjit::a64::Assembler &a, const uml::instruction &inst);
	template <asmjit::a64::Inst::Id Opcode> void op_add(asmjit::a64::Assembler &a, const uml::instruction &inst);
	template <asmjit::a64::Inst::Id Opcode> void op_sub(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_cmp(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_mulu(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_mululw(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_muls(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_mulslw(asmjit::a64::Assembler &a, const uml::instruction &inst);
	template <asmjit::a64::Inst::Id Opcode> void op_div(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_and(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_test(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_or(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_xor(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_lzcnt(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_tzcnt(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_bswap(asmjit::a64::Assembler &a, const uml::instruction &inst);
	template <asmjit::a64::Inst::Id Opcode> void op_shift(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_rol(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_rolc(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_rorc(asmjit::a64::Assembler &a, const uml::instruction &inst);

	void op_fload(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fstore(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fread(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fwrite(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fmov(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_ftoint(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_ffrint(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_ffrflt(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_frnds(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fcmp(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_fcopyi(asmjit::a64::Assembler &a, const uml::instruction &inst);
	void op_icopyf(asmjit::a64::Assembler &a, const uml::instruction &inst);

	template <asmjit::a64::Inst::Id Opcode> void op_float_alu(asmjit::a64::Assembler &a, const uml::instruction &inst);
	template <asmjit::a64::Inst::Id Opcode> void op_float_alu2(asmjit::a64::Assembler &a, const uml::instruction &inst);

	size_t emit(asmjit::CodeHolder &ch);


	// helper functions
	void get_imm_relative(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const uint64_t ptr) const;

	void emit_ldr_str_base_mem(asmjit::a64::Assembler &a, asmjit::a64::Inst::Id opcode, const asmjit::a64::Reg &reg, int max_shift, const void *ptr) const;
	void emit_ldr_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_ldrb_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_ldrh_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_ldrsb_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_ldrsh_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_ldrsw_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_str_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_strb_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;
	void emit_strh_mem(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const void *ptr) const;

	void emit_float_ldr_mem(asmjit::a64::Assembler &a, const asmjit::a64::Vec &reg, const void *ptr) const;
	void emit_float_str_mem(asmjit::a64::Assembler &a, const asmjit::a64::Vec &reg, const void *ptr) const;

	void get_carry(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, bool inverted = false) const;
	void load_carry(asmjit::a64::Assembler &a, bool inverted = false) const;
	void store_carry(asmjit::a64::Assembler &a, bool inverted = false) const;
	void store_carry_reg(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg) const;

	void store_unordered(asmjit::a64::Assembler &a) const;
	void get_unordered(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg) const;
	void check_unordered_condition(asmjit::a64::Assembler &a, uml::condition_t cond, asmjit::Label condition_met, bool not_equal) const;

	void calculate_carry_shift_left(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const asmjit::a64::Gp &shift, int maxBits) const;
	void calculate_carry_shift_left_imm(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const int shift, int maxBits) const;

	void calculate_carry_shift_right(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const asmjit::a64::Gp &shift) const;
	void calculate_carry_shift_right_imm(asmjit::a64::Assembler &a, const asmjit::a64::Gp &reg, const int shift) const;

	void mov_float_reg_param(asmjit::a64::Assembler &a, uint32_t regsize, asmjit::a64::Vec const &dst, const be_parameter &src) const;
	void mov_float_param_param(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const;
	void mov_float_param_reg(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, asmjit::a64::Vec const &src) const;
	void mov_float_param_int_reg(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, asmjit::a64::Gp const &src) const;

	void mov_reg_param(asmjit::a64::Assembler &a, uint32_t regsize, const asmjit::a64::Gp &dst, const be_parameter &src) const;
	void mov_param_reg(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const asmjit::a64::Gp &src) const;
	void mov_param_imm(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, uint64_t src) const;
	void mov_param_param(asmjit::a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const be_parameter &src) const;
	void mov_mem_param(asmjit::a64::Assembler &a, uint32_t regsize, void *dst, const be_parameter &src) const;
	void mov_r64_imm(asmjit::a64::Assembler &a, const asmjit::a64::Gp &dst, uint64_t const src) const;

	void call_arm_addr(asmjit::a64::Assembler &a, const void *offs) const;

	drc_hash_table m_hash;
	drc_map_variables m_map;
	FILE * m_log_asmjit;

	arm64_entry_point_func m_entry;
	drccodeptr m_exit;
	drccodeptr m_nocode;

	uint8_t *m_baseptr;

	struct near_state
	{
		uint32_t emulated_flags;
	};
	near_state &m_near;

	using opcode_generate_func = void (drcbe_arm64::*)(asmjit::a64::Assembler &, const uml::instruction &);
	struct opcode_table_entry
	{
		uml::opcode_t opcode;
		opcode_generate_func func;
	};
	static const opcode_table_entry s_opcode_table_source[];
	static opcode_generate_func s_opcode_table[uml::OP_MAX];

	struct memory_accessors
	{
		resolved_memory_accessors resolved;
		address_space::specific_access_info specific;
	};
	resolved_member_function m_debug_cpu_instruction_hook;
	resolved_member_function m_drcmap_get_value;
	std::vector<memory_accessors> m_memory_accessors;
};


drcbe_arm64::opcode_generate_func drcbe_arm64::s_opcode_table[OP_MAX];

const drcbe_arm64::opcode_table_entry drcbe_arm64::s_opcode_table_source[] =
{
	// Compile-time opcodes
	{ uml::OP_HANDLE,  &drcbe_arm64::op_handle },     // HANDLE  handle
	{ uml::OP_HASH,    &drcbe_arm64::op_hash },       // HASH    mode,pc
	{ uml::OP_LABEL,   &drcbe_arm64::op_label },      // LABEL   imm
	{ uml::OP_COMMENT, &drcbe_arm64::op_comment },    // COMMENT string
	{ uml::OP_MAPVAR,  &drcbe_arm64::op_mapvar },     // MAPVAR  mapvar,value

	// Control Flow Operations
	{ uml::OP_NOP,     &drcbe_arm64::op_nop },        // NOP
	{ uml::OP_BREAK,   &drcbe_arm64::op_break },      // BREAK
	{ uml::OP_DEBUG,   &drcbe_arm64::op_debug },      // DEBUG   pc
	{ uml::OP_EXIT,    &drcbe_arm64::op_exit },       // EXIT    src1[,c]
	{ uml::OP_HASHJMP, &drcbe_arm64::op_hashjmp },    // HASHJMP mode,pc,handle
	{ uml::OP_JMP,     &drcbe_arm64::op_jmp },        // JMP     imm[,c]
	{ uml::OP_EXH,     &drcbe_arm64::op_exh },        // EXH     handle,param[,c]
	{ uml::OP_CALLH,   &drcbe_arm64::op_callh },      // CALLH   handle[,c]
	{ uml::OP_RET,     &drcbe_arm64::op_ret },        // RET     [c]
	{ uml::OP_CALLC,   &drcbe_arm64::op_callc },      // CALLC   func,ptr[,c]
	{ uml::OP_RECOVER, &drcbe_arm64::op_recover },    // RECOVER dst,mapvar

	// Internal Register Operations
	{ uml::OP_SETFMOD, &drcbe_arm64::op_setfmod },    // SETFMOD src
	{ uml::OP_GETFMOD, &drcbe_arm64::op_getfmod },    // GETFMOD dst
	{ uml::OP_GETEXP,  &drcbe_arm64::op_getexp },     // GETEXP  dst
	{ uml::OP_GETFLGS, &drcbe_arm64::op_getflgs },    // GETFLGS dst[,f]
	{ uml::OP_SETFLGS, &drcbe_arm64::op_setflgs },    // SETFLGS dst[,f]
	{ uml::OP_SAVE,    &drcbe_arm64::op_save },       // SAVE    dst
	{ uml::OP_RESTORE, &drcbe_arm64::op_restore },    // RESTORE dst

	// Integer Operations
	{ uml::OP_LOAD,    &drcbe_arm64::op_load },                     // LOAD    dst,base,index,size
	{ uml::OP_LOADS,   &drcbe_arm64::op_loads },                    // LOADS   dst,base,index,size
	{ uml::OP_STORE,   &drcbe_arm64::op_store },                    // STORE   base,index,src,size
	{ uml::OP_READ,    &drcbe_arm64::op_read },                     // READ    dst,src1,spacesize
	{ uml::OP_READM,   &drcbe_arm64::op_readm },                    // READM   dst,src1,mask,spacesize
	{ uml::OP_WRITE,   &drcbe_arm64::op_write },                    // WRITE   dst,src1,spacesize
	{ uml::OP_WRITEM,  &drcbe_arm64::op_writem },                   // WRITEM  dst,src1,spacesize
	{ uml::OP_CARRY,   &drcbe_arm64::op_carry },                    // CARRY   src,bitnum
	{ uml::OP_SET,     &drcbe_arm64::op_set },                      // SET     dst,c
	{ uml::OP_MOV,     &drcbe_arm64::op_mov },                      // MOV     dst,src[,c]
	{ uml::OP_SEXT,    &drcbe_arm64::op_sext },                     // SEXT    dst,src
	{ uml::OP_ROLAND,  &drcbe_arm64::op_roland },                   // ROLAND  dst,src1,src2,src3
	{ uml::OP_ROLINS,  &drcbe_arm64::op_rolins },                   // ROLINS  dst,src1,src2,src3
	{ uml::OP_ADD,     &drcbe_arm64::op_add<a64::Inst::kIdAdds> },  // ADD     dst,src1,src2[,f]
	{ uml::OP_ADDC,    &drcbe_arm64::op_add<a64::Inst::kIdAdcs> },  // ADDC    dst,src1,src2[,f]
	{ uml::OP_SUB,     &drcbe_arm64::op_sub<a64::Inst::kIdSubs> },  // SUB     dst,src1,src2[,f]
	{ uml::OP_SUBB,    &drcbe_arm64::op_sub<a64::Inst::kIdSbcs> },  // SUBB    dst,src1,src2[,f]
	{ uml::OP_CMP,     &drcbe_arm64::op_cmp },                      // CMP     src1,src2[,f]
	{ uml::OP_MULU,    &drcbe_arm64::op_mulu },                     // MULU    dst,edst,src1,src2[,f]
	{ uml::OP_MULULW,  &drcbe_arm64::op_mululw },                   // MULULW   dst,src1,src2[,f]
	{ uml::OP_MULS,    &drcbe_arm64::op_muls },                     // MULS    dst,edst,src1,src2[,f]
	{ uml::OP_MULSLW,  &drcbe_arm64::op_mulslw },                   // MULSLW   dst,src1,src2[,f]
	{ uml::OP_DIVU,    &drcbe_arm64::op_div<a64::Inst::kIdUdiv> },  // DIVU    dst,edst,src1,src2[,f]
	{ uml::OP_DIVS,    &drcbe_arm64::op_div<a64::Inst::kIdSdiv> },  // DIVS    dst,edst,src1,src2[,f]
	{ uml::OP_AND,     &drcbe_arm64::op_and },                      // AND     dst,src1,src2[,f]
	{ uml::OP_TEST,    &drcbe_arm64::op_test },                     // TEST    src1,src2[,f]
	{ uml::OP_OR,      &drcbe_arm64::op_or },                       // OR      dst,src1,src2[,f]
	{ uml::OP_XOR,     &drcbe_arm64::op_xor },                      // XOR     dst,src1,src2[,f]
	{ uml::OP_LZCNT,   &drcbe_arm64::op_lzcnt },                    // LZCNT   dst,src[,f]
	{ uml::OP_TZCNT,   &drcbe_arm64::op_tzcnt },                    // TZCNT   dst,src[,f]
	{ uml::OP_BSWAP,   &drcbe_arm64::op_bswap },                    // BSWAP   dst,src
	{ uml::OP_SHL,     &drcbe_arm64::op_shift<a64::Inst::kIdLsl> }, // SHL     dst,src,count[,f]
	{ uml::OP_SHR,     &drcbe_arm64::op_shift<a64::Inst::kIdLsr> }, // SHR     dst,src,count[,f]
	{ uml::OP_SAR,     &drcbe_arm64::op_shift<a64::Inst::kIdAsr> }, // SAR     dst,src,count[,f]
	{ uml::OP_ROL,     &drcbe_arm64::op_rol },                      // ROL     dst,src,count[,f]
	{ uml::OP_ROLC,    &drcbe_arm64::op_rolc },                     // ROLC    dst,src,count[,f]
	{ uml::OP_ROR,     &drcbe_arm64::op_shift<a64::Inst::kIdRor> }, // ROR     dst,src,count[,f]
	{ uml::OP_RORC,    &drcbe_arm64::op_rorc },                     // RORC    dst,src,count[,f]

	// Floating Point Operations
	{ uml::OP_FLOAD,   &drcbe_arm64::op_fload },                                // FLOAD   dst,base,index
	{ uml::OP_FSTORE,  &drcbe_arm64::op_fstore },                               // FSTORE  base,index,src
	{ uml::OP_FREAD,   &drcbe_arm64::op_fread },                                // FREAD   dst,space,src1
	{ uml::OP_FWRITE,  &drcbe_arm64::op_fwrite },                               // FWRITE  space,dst,src1
	{ uml::OP_FMOV,    &drcbe_arm64::op_fmov },                                 // FMOV    dst,src1[,c]
	{ uml::OP_FTOINT,  &drcbe_arm64::op_ftoint },                               // FTOINT  dst,src1,size,round
	{ uml::OP_FFRINT,  &drcbe_arm64::op_ffrint },                               // FFRINT  dst,src1,size
	{ uml::OP_FFRFLT,  &drcbe_arm64::op_ffrflt },                               // FFRFLT  dst,src1,size
	{ uml::OP_FRNDS,   &drcbe_arm64::op_frnds },                                // FRNDS   dst,src1
	{ uml::OP_FADD,    &drcbe_arm64::op_float_alu<a64::Inst::kIdFadd_v> },      // FADD    dst,src1,src2
	{ uml::OP_FSUB,    &drcbe_arm64::op_float_alu<a64::Inst::kIdFsub_v> },      // FSUB    dst,src1,src2
	{ uml::OP_FCMP,    &drcbe_arm64::op_fcmp },                                 // FCMP    src1,src2
	{ uml::OP_FMUL,    &drcbe_arm64::op_float_alu<a64::Inst::kIdFmul_v> },      // FMUL    dst,src1,src2
	{ uml::OP_FDIV,    &drcbe_arm64::op_float_alu<a64::Inst::kIdFdiv_v>  },     // FDIV    dst,src1,src2
	{ uml::OP_FNEG,    &drcbe_arm64::op_float_alu2<a64::Inst::kIdFneg_v> },     // FNEG    dst,src1
	{ uml::OP_FABS,    &drcbe_arm64::op_float_alu2<a64::Inst::kIdFabs_v> },     // FABS    dst,src1
	{ uml::OP_FSQRT,   &drcbe_arm64::op_float_alu2<a64::Inst::kIdFsqrt_v> },    // FSQRT   dst,src1
	{ uml::OP_FRECIP,  &drcbe_arm64::op_float_alu2<a64::Inst::kIdFrecpe_v> },   // FRECIP  dst,src1
	{ uml::OP_FRSQRT,  &drcbe_arm64::op_float_alu2<a64::Inst::kIdFrsqrte_v> },  // FRSQRT  dst,src1
	{ uml::OP_FCOPYI,  &drcbe_arm64::op_fcopyi },                               // FCOPYI  dst,src
	{ uml::OP_ICOPYF,  &drcbe_arm64::op_icopyf }                                // ICOPYF  dst,src
};

drcbe_arm64::be_parameter::be_parameter(drcbe_arm64 &drcbe, const parameter &param, uint32_t allowed)
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
			fatalerror("Unexpected parameter type %d\n", param.type());
	}
}

a64::Vec drcbe_arm64::be_parameter::get_register_float(uint32_t regsize) const
{
	assert(m_type == PTYPE_FLOAT_REGISTER);
	return a64::Vec::fromTypeAndId((regsize == 4) ? RegType::kARM_VecS : RegType::kARM_VecD, m_value);
}

a64::Gp drcbe_arm64::be_parameter::get_register_int(uint32_t regsize) const
{
	assert(m_type == PTYPE_INT_REGISTER);
	return a64::Gp::fromTypeAndId((regsize == 4) ? RegType::kARM_GpW : RegType::kARM_GpX, m_value);
}

a64::Vec drcbe_arm64::be_parameter::select_register(a64::Vec const &reg, uint32_t regsize) const
{
	if (m_type == PTYPE_FLOAT_REGISTER)
		return get_register_float(regsize);
	if (regsize == 4)
		return reg.s();
	return reg.d();
}

a64::Gp drcbe_arm64::be_parameter::select_register(a64::Gp const &reg, uint32_t regsize) const
{
	if (m_type == PTYPE_INT_REGISTER)
		return get_register_int(regsize);
	if (regsize == 4)
		return reg.w();
	return reg.x();
}

void drcbe_arm64::get_imm_relative(a64::Assembler &a, const a64::Gp &reg, const uint64_t val) const
{
	// Check for constants that can be generated with a single instruction
	if (is_simple_mov_immediate(val, reg.isGpX() ? 8 : 4))
	{
		a.mov(reg, val);
		return;
	}
	else if (reg.isGpX() && is_valid_immediate_mask(val, 4))
	{
		a.mov(reg.w(), val); // asmjit isn't smart enough to work this out
		return;
	}

	// Values close to the program counter can be generated with a single adr
	const uint64_t codeoffs = a.code()->baseAddress() + a.offset();
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

inline void drcbe_arm64::emit_ldr_str_base_mem(a64::Assembler &a, a64::Inst::Id opcode, const a64::Reg &reg, int max_shift, const void *ptr) const
{
	// If it can fit as an immediate offset
	const int64_t diff = (int64_t)ptr - (int64_t)m_baseptr;
	if (is_valid_offset(diff, max_shift))
	{
		a.emit(opcode, reg, arm::Mem(BASE_REG, diff));
		return;
	}

	// If it can fit as an offset relative to PC
	const uint64_t codeoffs = a.code()->baseAddress() + a.offset();
	const int64_t reloffs = (int64_t)ptr - codeoffs;
	if (is_valid_immediate_signed(reloffs, 21))
	{
		a.adr(MEM_SCRATCH_REG, ptr);
		a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG));
		return;
	}

	if (diff > 0 && is_valid_immediate(diff, 16))
	{
		a.mov(MEM_SCRATCH_REG, diff);
		a.emit(opcode, reg, arm::Mem(BASE_REG, MEM_SCRATCH_REG));
		return;
	}

	if (diff > 0 && emit_add_optimized(a, MEM_SCRATCH_REG, BASE_REG, diff))
	{
		a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG));
		return;
	}
	else if (diff < 0 && emit_sub_optimized(a, MEM_SCRATCH_REG, BASE_REG, diff))
	{
		a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG));
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
			a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG, pageoffs));
		}
		else
		{
			a.add(MEM_SCRATCH_REG, MEM_SCRATCH_REG, pageoffs);
			a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG));
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
				a.emit(opcode, reg, arm::Mem(BASE_REG, MEM_SCRATCH_REG, arm::Shift(arm::ShiftOp::kLSL, shift)));
			else
				a.emit(opcode, reg, arm::Mem(BASE_REG, MEM_SCRATCH_REG));

			return;
		}
	}

	// Can't optimize it at all, most likely becomes 4 MOV commands
	a.mov(MEM_SCRATCH_REG, ptr);
	a.emit(opcode, reg, arm::Mem(MEM_SCRATCH_REG));
}

void drcbe_arm64::emit_ldr_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdr, reg, reg.isGpW() ? 2 : 3, ptr); }
void drcbe_arm64::emit_ldrb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrb, reg, 0, ptr); }
void drcbe_arm64::emit_ldrh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrh, reg, 1, ptr); }
void drcbe_arm64::emit_ldrsb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsb, reg, 0, ptr); }
void drcbe_arm64::emit_ldrsh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsh, reg, 1, ptr); }
void drcbe_arm64::emit_ldrsw_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdrsw, reg, 2, ptr); }
void drcbe_arm64::emit_str_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStr, reg, reg.isGpW() ? 2 : 3, ptr); }
void drcbe_arm64::emit_strb_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStrb, reg, 0, ptr); }
void drcbe_arm64::emit_strh_mem(a64::Assembler &a, const a64::Gp &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStrh, reg, 1, ptr); }

void drcbe_arm64::emit_float_ldr_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdLdr_v, reg, reg.isVecS() ? 2 : 3, ptr); }
void drcbe_arm64::emit_float_str_mem(a64::Assembler &a, const a64::Vec &reg, const void *ptr) const { emit_ldr_str_base_mem(a, a64::Inst::kIdStr_v, reg, reg.isVecS() ? 2 : 3, ptr); }

void drcbe_arm64::mov_reg_param(a64::Assembler &a, uint32_t regsize, const a64::Gp &dst, const be_parameter &src) const
{
	if (src.is_immediate())
		get_imm_relative(a, select_register(dst, regsize), src.immediate());
	else if (src.is_int_register() && dst.id() != src.ireg())
		a.mov(select_register(dst, regsize), src.get_register_int(regsize));
	else if (src.is_memory())
		emit_ldr_mem(a, select_register(dst, regsize), src.memory());
}

void drcbe_arm64::mov_mem_param(a64::Assembler &a, uint32_t regsize, void *dst, const be_parameter &src) const
{
	const a64::Gp scratch = select_register(SCRATCH_REG2, regsize);

	if (src.is_immediate())
	{
		if (src.is_immediate_value(0))
		{
			emit_str_mem(a, select_register(a64::xzr, regsize), dst);
		}
		else
		{
			get_imm_relative(a, scratch.x(), src.immediate());
			emit_str_mem(a, scratch, dst);
		}
	}
	else if (src.is_memory())
	{
		if (regsize == 4)
			emit_ldrsw_mem(a, scratch.x(), src.memory());
		else
			emit_ldr_mem(a, scratch.x(), src.memory());

		emit_str_mem(a, scratch, dst);
	}
	else if (src.is_int_register())
	{
		emit_str_mem(a, src.get_register_int(regsize), dst);
	}
}

void drcbe_arm64::mov_param_reg(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, const a64::Gp &src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
		emit_str_mem(a, select_register(src, regsize), dst.memory());
	else if (dst.is_int_register() && src.id() != dst.ireg())
		a.mov(dst.get_register_int(regsize), select_register(src, regsize));
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
		mov_param_reg(a, regsize, dst, src.get_register_int(regsize));
	}
	else if (src.is_immediate())
	{
		mov_param_imm(a, regsize, dst, src.immediate());
	}
}

void drcbe_arm64::mov_param_imm(a64::Assembler &a, uint32_t regsize, const be_parameter &dst, uint64_t src) const
{
	assert(!dst.is_immediate());

	if (dst.is_memory())
	{
		if (src == 0)
		{
			emit_str_mem(a, select_register(a64::xzr, regsize), dst.memory());
		}
		else
		{
			const a64::Gp scratch = select_register(SCRATCH_REG2, regsize);

			get_imm_relative(a, scratch, src);
			emit_str_mem(a, scratch, dst.memory());
		}
	}
	else if (dst.is_int_register())
	{
		a.mov(dst.get_register_int(regsize), src);
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
	const uint64_t codeoffs = a.code()->baseAddress() + a.offset();
	const int64_t reloffs = (int64_t)offs - codeoffs;
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

void drcbe_arm64::check_unordered_condition(a64::Assembler &a, uml::condition_t cond, Label condition_met, bool not_equal) const
{
	if (cond != uml::COND_U && cond != uml::COND_NU)
		return;

	get_unordered(a, SCRATCH_REG1);

	if (cond == uml::COND_U)
	{
		if (not_equal)
			a.cbz(SCRATCH_REG1, condition_met);
		else
			a.cbnz(SCRATCH_REG1, condition_met);
	}
	else
	{
		if (not_equal)
			a.cbnz(SCRATCH_REG1, condition_met);
		else
			a.cbz(SCRATCH_REG1, condition_met);
	}
}

void drcbe_arm64::store_unordered(a64::Assembler &a) const
{
	a.cset(SCRATCH_REG1, a64::CondCode::kPL);
	a.cset(SCRATCH_REG2, a64::CondCode::kNE);
	a.and_(SCRATCH_REG1, SCRATCH_REG1, SCRATCH_REG2);
	a.cset(SCRATCH_REG2, a64::CondCode::kCS);
	a.and_(SCRATCH_REG1, SCRATCH_REG1, SCRATCH_REG2);
	a.cset(SCRATCH_REG2, a64::CondCode::kVS);
	a.and_(SCRATCH_REG1, SCRATCH_REG1, SCRATCH_REG2);
	a.bfi(FLAGS_REG, SCRATCH_REG2, 4, 1);
}

void drcbe_arm64::get_unordered(a64::Assembler &a, const a64::Gp &reg) const
{
	a.ubfx(reg.x(), FLAGS_REG, 4, 1);
}

void drcbe_arm64::store_carry_reg(a64::Assembler &a, const a64::Gp &reg) const
{
	a.bfi(FLAGS_REG, reg.x(), 0, 1);
}

void drcbe_arm64::store_carry(a64::Assembler &a, bool inverted) const
{
	if (inverted)
		a.cset(SCRATCH_REG1, a64::CondCode::kCC);
	else
		a.cset(SCRATCH_REG1, a64::CondCode::kCS);

	store_carry_reg(a, SCRATCH_REG1);
}

void drcbe_arm64::get_carry(a64::Assembler &a, const a64::Gp &reg, bool inverted) const
{
	a.and_(reg.x(), FLAGS_REG, 1);

	if (inverted)
		a.eor(reg.x(), reg.x(), 1);
}

void drcbe_arm64::load_carry(a64::Assembler &a, bool inverted) const
{
	a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);
	a.bfi(SCRATCH_REG1, FLAGS_REG, 29, 1);

	if (inverted)
		a.eor(SCRATCH_REG1, SCRATCH_REG1, 1 << 29);

	a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
}

void drcbe_arm64::calculate_carry_shift_left(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift, int maxBits) const
{
	Label calc = a.newLabel();
	Label end = a.newLabel();

	a.cbnz(shift, calc);
	store_carry_reg(a, a64::xzr);
	a.b(end);

	a.bind(calc);
	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.isGpW() ? 4 : 8);

	// carry = ((PARAM1 << (shift - 1)) >> maxBits) & 1
	a.movz(scratch, maxBits + 1);
	a.sub(scratch, scratch, shift);
	a.lsr(scratch, reg, scratch);
	store_carry_reg(a, scratch);

	a.bind(end);
}

void drcbe_arm64::calculate_carry_shift_left_imm(a64::Assembler &a, const a64::Gp &reg, const int shift, int maxBits) const
{
	if (shift == 0)
	{
		store_carry_reg(a, a64::xzr);
		return;
	}

	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.isGpW() ? 4 : 8);

	// carry = ((PARAM1 << (shift - 1)) >> maxBits) & 1
	a.lsr(scratch, reg, maxBits + 1 - shift);
	store_carry_reg(a, scratch);
}

void drcbe_arm64::calculate_carry_shift_right(a64::Assembler &a, const a64::Gp &reg, const a64::Gp &shift) const
{
	Label calc = a.newLabel();
	Label end = a.newLabel();

	a.cbnz(shift, calc);
	store_carry_reg(a, a64::xzr);
	a.b(end);

	a.bind(calc);
	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.isGpW() ? 4 : 8);

	// carry = (PARAM1 >> (shift - 1)) & 1
	a.sub(scratch, shift, 1);
	a.lsr(scratch, reg, scratch);
	store_carry_reg(a, scratch);

	a.bind(end);
}

void drcbe_arm64::calculate_carry_shift_right_imm(a64::Assembler &a, const a64::Gp &reg, const int shift) const
{
	if (shift == 0)
	{
		store_carry_reg(a, a64::xzr);
		return;
	}

	const a64::Gp scratch = select_register(SCRATCH_REG1, reg.isGpW() ? 4 : 8);

	// carry = (PARAM1 >> (shift - 1)) & 1
	a.lsr(scratch, reg, shift - 1);
	store_carry_reg(a, scratch);
}

drcbe_arm64::drcbe_arm64(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device)
	, m_hash(cache, modes, addrbits, ignorebits)
	, m_map(cache, 0xaaaaaaaa5555)
	, m_log_asmjit(nullptr)
	, m_entry(nullptr)
	, m_exit(nullptr)
	, m_nocode(nullptr)
	, m_baseptr(cache.near() + 0x100)
	, m_near(*(near_state *)cache.alloc_near(sizeof(m_near)))
{
	m_near.emulated_flags = 0;

	// build the opcode table (static but it doesn't hurt to regenerate it)
	for (auto & elem : s_opcode_table_source)
		s_opcode_table[elem.opcode] = elem.func;

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
			m_memory_accessors[space].resolved.set(*m_space[space]);
			m_memory_accessors[space].specific = m_space[space]->specific_accessors();
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

	size_t const alignment = ch.baseAddress() - uint64_t(m_cache.top());
	size_t const code_size = ch.codeSize();

	// test if enough room remains in drc cache
	drccodeptr *cachetop = m_cache.begin_codegen(alignment + code_size);
	if (cachetop == nullptr)
		return 0;

	err = ch.copyFlattenedData(drccodeptr(ch.baseAddress()), code_size, CopySectionFlags::kPadTargetBuffer);
	if (err)
		throw emu_fatalerror("CodeHolder::copyFlattenedData() error %d", err);

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
		logger.setFlags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.setIndentation(FormatIndentationGroup::kCode, 4);
		ch.setLogger(&logger);
	}

	a64::Assembler a(&ch);
	if (logger.file())
		a.addDiagnosticOptions(DiagnosticOptions::kValidateIntermediate);

	// generate entry point
	m_entry = (arm64_entry_point_func)dst;
	a.bind(a.newNamedLabel("entry_point"));

	FuncDetail entry_point;
	entry_point.init(FuncSignature::build<uint32_t, uint8_t *, uint8_t *>(CallConvId::kHost), Environment::host());

	FuncFrame frame;
	frame.init(entry_point);
	frame.setPreservedFP();
	frame.setAllDirty();

	FuncArgsAssignment args(&entry_point);
	args.assignAll(REG_PARAM1);
	args.updateFuncFrame(frame);

	frame.finalize();

	a.emitProlog(frame);

	get_imm_absolute(a, BASE_REG, uintptr_t(m_baseptr));
	emit_ldr_mem(a, FLAGS_REG.w(), &m_near.emulated_flags);

	a.emitArgsAssignment(frame, args);

	a.br(REG_PARAM1);

	// generate exit point
	m_exit = dst + a.offset();
	a.bind(a.newNamedLabel("exit_point"));

	a.mov(a64::sp, a64::x29);

	a.emitEpilog(frame);
	a.ret(a64::x30);

	// generate a no code point
	m_nocode = dst + a.offset();
	a.bind(a.newNamedLabel("nocode_point"));
	a.br(REG_PARAM1);

	// emit the generated code
	emit(ch);

	// reset our hash tables
	m_hash.reset();

	m_hash.set_default_codeptr(m_nocode);
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
	ch.setErrorHandler(&e);

	FileLogger logger(m_log_asmjit);
	if (logger.file())
	{
		logger.setFlags(FormatFlags::kHexOffsets | FormatFlags::kHexImms | FormatFlags::kMachineCode);
		logger.setIndentation(FormatIndentationGroup::kCode, 4);
		ch.setLogger(&logger);
	}

	a64::Assembler a(&ch);
	if (logger.file())
		a.addDiagnosticOptions(DiagnosticOptions::kValidateIntermediate);

	// generate code
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		assert(inst.opcode() < std::size(s_opcode_table));

		// generate code
		(this->*s_opcode_table[inst.opcode()])(a, inst);
	}

	emit_str_mem(a, FLAGS_REG.w(), &m_near.emulated_flags);

	// emit the generated code
	if (!emit(ch))
		block.abort();

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_map.block_end(block);
}

bool drcbe_arm64::hash_exists(uint32_t mode, uint32_t pc)
{
	return m_hash.code_exists(mode, pc);
}

void drcbe_arm64::get_info(drcbe_info &info)
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

void drcbe_arm64::op_handle(a64::Assembler &a, const uml::instruction &inst)
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
	a.b(skip);

	// register the current pointer for the handle
	inst.param(0).handle().set_codeptr(drccodeptr(a.code()->baseAddress() + a.offset()));

	// the handle points to prologue code that creates a minimal non-leaf frame
	a.stp(a64::x29, a64::x30, arm::Mem(a64::sp, -16).pre());
	a.bind(skip);
}

void drcbe_arm64::op_hash(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 2);
	assert(inst.param(0).is_immediate());
	assert(inst.param(1).is_immediate());

	const uint64_t mode = inst.param(0).immediate();
	const uint64_t pc = inst.param(1).immediate();

	m_hash.set_codeptr(mode, pc, drccodeptr(a.code()->baseAddress() + a.offset()));
}

void drcbe_arm64::op_label(a64::Assembler &a, const uml::instruction &inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 1);
	assert(inst.param(0).is_code_label());

	std::string labelName = util::string_format("PC$%x", inst.param(0).label());
	Label label = a.labelByName(labelName.c_str());
	if (!label.isValid())
		label = a.newNamedLabel(labelName.c_str());

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

	m_map.set_value(drccodeptr(a.code()->baseAddress() + a.offset()), mapvar, value);
}

void drcbe_arm64::op_nop(a64::Assembler &a, const uml::instruction &inst)
{
	a.nop();
}

void drcbe_arm64::op_break(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	static const char *const message = "break from drc";
	get_imm_relative(a, REG_PARAM1, (uintptr_t)message);
	call_arm_addr(a, (const void *)&osd_break_into_debugger);
}

void drcbe_arm64::op_debug(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	const a64::Gp temp = TEMP_REG1.w();

	if (m_device.machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		be_parameter pcp(*this, inst.param(0), PTYPE_MRI);

		Label skip = a.newLabel();

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

	mov_reg_param(a, 4, REG_PARAM1, retp);

	if (inst.condition() == uml::COND_ALWAYS)
	{
		a.b(m_exit);
	}
	else if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
	{
		Label skip = a.newLabel();
		check_unordered_condition(a, inst.condition(), skip, false);
		a.b(m_exit);
		a.bind(skip);
	}
	else
	{
		a.b(ARM_CONDITION(a, inst.condition()), m_exit);
	}
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
			a.ldr(TEMP_REG3, a64::Mem(TEMP_REG1, TEMP_REG3, arm::Shift(arm::ShiftOp::kLSL, 3))); // TEMP_REG3 = m_base[mode][(pc >> m_l1shift) & m_l1mask]

			a.ubfx(TEMP_REG2, TEMP_REG2, m_hash.l2shift(), m_hash.l2bits());
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG3, TEMP_REG2, arm::Shift(arm::ShiftOp::kLSL, 3))); // TEMP_REG1 = m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]
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
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG2, mode, arm::Shift(arm::ShiftOp::kLSL, 3))); // TEMP_REG1 = m_base[modep]
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
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, SCRATCH_REG1, arm::Shift(arm::ShiftOp::kLSL, 3)));
			}

			if (is_valid_immediate(l2val, 15))
			{
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, l2val));
			}
			else
			{
				a.mov(SCRATCH_REG1, l2val >> 3);
				a.ldr(TEMP_REG1, a64::Mem(TEMP_REG1, SCRATCH_REG1, arm::Shift(arm::ShiftOp::kLSL, 3)));
			}
		}
		else
		{
			const a64::Gp pc = pcp.select_register(TEMP_REG2, 8);
			mov_reg_param(a, 4, pc, pcp);

			a.ubfx(TEMP_REG3, pc, m_hash.l1shift(), m_hash.l1bits()); // (pc >> m_l1shift) & m_l1mask
			a.ldr(TEMP_REG3, a64::Mem(TEMP_REG1, TEMP_REG3, arm::Shift(arm::ShiftOp::kLSL, 3))); // TEMP_REG3 = m_base[mode][(pc >> m_l1shift) & m_l1mask]

			a.ubfx(TEMP_REG2, pc, m_hash.l2shift(), m_hash.l2bits()); // (pc >> m_l2shift) & m_l2mask
			a.ldr(TEMP_REG1, a64::Mem(TEMP_REG3, TEMP_REG2, arm::Shift(arm::ShiftOp::kLSL, 3))); // x25 = m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]
		}
	}

	Label lab = a.newLabel();
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
}

void drcbe_arm64::op_jmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &labelp = inst.param(0);
	assert(labelp.is_code_label());

	std::string labelName = util::string_format("PC$%x", labelp.label());
	Label jmptarget = a.labelByName(labelName.c_str());
	if (!jmptarget.isValid())
		jmptarget = a.newNamedLabel(labelName.c_str());

	if (inst.condition() == uml::COND_ALWAYS)
		a.b(jmptarget);
	else
	{
		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), jmptarget, false);
		else
			a.b(ARM_CONDITION(a, inst.condition()), jmptarget);
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
	if (inst.condition() != uml::COND_ALWAYS)
	{
		no_exception = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), no_exception, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), no_exception);
	}

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
}

void drcbe_arm64::op_callh(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);

	const parameter &handp = inst.param(0);
	assert(handp.is_code_handle());

	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), skip, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), skip);
	}

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
}

void drcbe_arm64::op_ret(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst.numparams() == 0);

	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), skip, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), skip);
	}

	a.ldp(a64::x29, a64::x30, arm::Mem(a64::sp).post(16));
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
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), skip, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), skip);
	}

	emit_str_mem(a, FLAGS_REG.w(), &m_near.emulated_flags);

	get_imm_relative(a, REG_PARAM1, (uintptr_t)paramp.memory());
	get_imm_relative(a, TEMP_REG1, (uintptr_t)funcp.cfunc());
	a.blr(TEMP_REG1);

	emit_ldr_mem(a, FLAGS_REG.w(), &m_near.emulated_flags);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);
}

void drcbe_arm64::op_recover(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);

	a.ldr(REG_PARAM2, arm::Mem(a64::x29, -8)); // saved LR (x30) from first level CALLH/EXH or failed hash jump
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
	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp scratch = select_register(FUNC_SCRATCH_REG, inst.size());

	if (srcp.is_immediate())
	{
		a.mov(scratch, srcp.immediate() & 3);
	}
	else
	{
		mov_reg_param(a, inst.size(), src, srcp);
		a.and_(scratch, src, 3);
	}

	emit_strb_mem(a, scratch.w(), &m_state.fmod);
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

	a.mov(dst, a64::xzr);

	if (maskp.immediate() & FLAG_C)
		a.and_(dst, FLAGS_REG, 1);

	if (maskp.immediate() & FLAG_V)
	{
		a.cset(SCRATCH_REG1, a64::CondCode::kVS);
		a.orr(dst, dst, SCRATCH_REG1, 1);
	}

	if (maskp.immediate() & FLAG_Z)
	{
		a.cset(SCRATCH_REG1, a64::CondCode::kEQ);
		a.orr(dst, dst, SCRATCH_REG1, 2);
	}

	if (maskp.immediate() & FLAG_S)
	{
		a.cset(SCRATCH_REG1, a64::CondCode::kMI);
		a.orr(dst, dst, SCRATCH_REG1, 3);
	}

	if (maskp.immediate() & FLAG_U)
	{
		get_unordered(a, SCRATCH_REG1);
		a.orr(dst, dst, SCRATCH_REG1, 4);
	}

	mov_param_reg(a, inst.size(), dstp, dst);
}

void drcbe_arm64::op_setflgs(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4);
	assert_no_condition(inst);

	be_parameter flagsp(*this, inst.param(0), PTYPE_MRI);

	mov_reg_param(a, inst.size(), FLAGS_REG, flagsp);

	a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

	a.and_(TEMP_REG2, FLAGS_REG, 0b1100); // zero + sign
	a.ubfx(TEMP_REG3, FLAGS_REG, 1, 1); // overflow flag
	a.orr(TEMP_REG2, TEMP_REG2, TEMP_REG3);
	a.bfi(TEMP_REG1, TEMP_REG2, 28, 4);

	a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);
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

	a.bfi(TEMP_REG2, TEMP_REG1, 1, 1); // overflow flag

	a.strb(TEMP_REG2.w(), arm::Mem(membase, offsetof(drcuml_machine_state, flags)));

	emit_ldrb_mem(a, TEMP_REG1.w(), &m_state.fmod);
	a.strb(TEMP_REG1.w(), arm::Mem(membase, offsetof(drcuml_machine_state, fmod)));

	emit_ldr_mem(a, TEMP_REG1.w(), &m_state.exp);
	a.str(TEMP_REG1.w(), arm::Mem(membase, offsetof(drcuml_machine_state, exp)));

	int regoffs = offsetof(drcuml_machine_state, r);
	for (int regnum = 0; regnum < std::size(m_state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
		{
			a.str(a64::Gp::fromTypeAndId(RegType::kARM_GpX, int_register_map[regnum]), arm::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			emit_ldr_mem(a, TEMP_REG1, &m_state.r[regnum].d);
			a.str(TEMP_REG1, arm::Mem(membase, regoffs + (8 * regnum)));
		}
	}

	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
		{
			a.str(a64::Vec::fromTypeAndId(RegType::kARM_VecD, float_register_map[regnum]), arm::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			emit_ldr_mem(a, TEMP_REG1, &m_state.f[regnum].d);
			a.str(TEMP_REG1, arm::Mem(membase, regoffs + (8 * regnum)));
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
			a.ldr(a64::Gp::fromTypeAndId(RegType::kARM_GpX, int_register_map[regnum]), arm::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			a.ldr(TEMP_REG1, arm::Mem(membase, regoffs + (8 * regnum)));
			emit_str_mem(a, TEMP_REG1, &m_state.r[regnum].d);
		}
	}

	regoffs = offsetof(drcuml_machine_state, f);
	for (int regnum = 0; regnum < std::size(m_state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
		{
			a.ldr(a64::Vec::fromTypeAndId(RegType::kARM_VecD, float_register_map[regnum]), arm::Mem(membase, regoffs + (8 * regnum)));
		}
		else
		{
			a.ldr(TEMP_REG1, arm::Mem(membase, regoffs + (8 * regnum)));
			emit_str_mem(a, TEMP_REG1, &m_state.f[regnum].d);
		}
	}

	a.ldrb(TEMP_REG1.w(), arm::Mem(membase, offsetof(drcuml_machine_state, fmod)));
	emit_strb_mem(a, TEMP_REG1.w(), &m_state.fmod);

	a.ldr(TEMP_REG1.w(), arm::Mem(membase, offsetof(drcuml_machine_state, exp)));
	emit_str_mem(a, TEMP_REG1.w(), &m_state.exp);

	a.ldrb(FLAGS_REG.w(), arm::Mem(membase, offsetof(drcuml_machine_state, flags)));

	a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

	a.and_(TEMP_REG2, FLAGS_REG, 0b1100); // zero + sign
	a.ubfx(TEMP_REG3, FLAGS_REG, 1, 1); // overflow flag
	a.orr(TEMP_REG2, TEMP_REG2, TEMP_REG3);
	a.bfi(TEMP_REG1, TEMP_REG2, 28, 4);

	a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);
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

	const a64::Gp basereg = TEMP_REG1;
	const a64::Gp dstreg = dstp.select_register(TEMP_REG2, inst.size());

	const int32_t offset = indp.is_immediate() ? indp.immediate() << scalesizep.scale() : 0;
	if (indp.is_immediate() && is_valid_immediate(offset, 15))
	{
		const auto memptr = &reinterpret_cast<uint8_t *>(basep.memory())[offset];

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
		get_imm_relative(a, basereg, uint64_t(basep.memory()));

		const a64::Gp offsreg = indp.select_register(TEMP_REG3, 4);
		mov_reg_param(a, 4, offsreg, indp);

		// the scale needs to match the load size for shifting to be allowed
		auto mem = arm::Mem(basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
		if (scalesizep.scale() != size)
		{
			if (scalesizep.scale() != 0)
			{
				a.add(basereg, basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
				mem = arm::Mem(basereg);
			}
			else
			{
				mem = arm::Mem(basereg, offsreg);
			}
		}

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

	const a64::Gp basereg = TEMP_REG1;
	const a64::Gp dstreg = dstp.select_register(TEMP_REG2, inst.size());

	const int32_t offset = indp.is_immediate() ? indp.immediate() << scalesizep.scale() : 0;
	if (indp.is_immediate() && is_valid_immediate(offset, 15))
	{
		// immediate index
		if (size == SIZE_BYTE)
			emit_ldrsb_mem(a, dstreg.x(), (uint8_t*)basep.memory() + offset);
		else if (size == SIZE_WORD)
			emit_ldrsh_mem(a, dstreg.x(), (uint8_t*)basep.memory() + offset);
		else if (size == SIZE_DWORD)
			emit_ldrsw_mem(a, dstreg.x(), (uint8_t*)basep.memory() + offset);
		else
			emit_ldr_mem(a, dstreg.x(), (uint8_t*)basep.memory() + offset);
	}
	else
	{
		get_imm_relative(a, basereg, uint64_t(basep.memory()));

		const a64::Gp offsreg = indp.select_register(TEMP_REG3, 8);
		mov_reg_param(a, 4, offsreg, indp);

		// the scale needs to match the load size for shifting to be allowed
		auto mem = arm::Mem(basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
		if (scalesizep.scale() != size)
		{
			if (scalesizep.scale() != 0)
			{
				a.add(basereg, basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
				mem = arm::Mem(basereg);
			}
			else
			{
				mem = arm::Mem(basereg, offsreg);
			}
		}

		if (size == SIZE_BYTE)
			a.ldrsb(dstreg, mem);
		else if (size == SIZE_WORD)
			a.ldrsh(dstreg, mem);
		else if (size == SIZE_DWORD && inst.size() == 8)
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

	const a64::Gp basereg = TEMP_REG1;

	const int32_t offset = indp.is_immediate() ? indp.immediate() << scalesizep.scale() : 0;
	if (indp.is_immediate() && is_valid_immediate(offset, 15))
	{
		const a64::Gp srcreg = srcp.select_register(TEMP_REG2, inst.size());
		mov_reg_param(a, inst.size(), srcreg, srcp);

		if (size == SIZE_BYTE)
			emit_strb_mem(a, srcreg.w(), (uint8_t*)basep.memory() + offset);
		else if (size == SIZE_WORD)
			emit_strh_mem(a, srcreg.w(), (uint8_t*)basep.memory() + offset);
		else if (size == SIZE_DWORD)
			emit_str_mem(a, srcreg.w(), (uint8_t*)basep.memory() + offset);
		else
			emit_str_mem(a, srcreg.x(), (uint8_t*)basep.memory() + offset);
	}
	else
	{
		get_imm_relative(a, basereg, uint64_t(basep.memory()));

		const a64::Gp srcreg = srcp.select_register(TEMP_REG2, inst.size());
		const a64::Gp offsreg = indp.select_register(TEMP_REG3, 8);

		mov_reg_param(a, 4, srcreg, srcp);
		mov_reg_param(a, 4, offsreg, indp);

		// the scale needs to match the store size for shifting to be allowed
		auto mem = arm::Mem(basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
		if (scalesizep.scale() != size)
		{
			if (scalesizep.scale() != 0)
			{
				a.add(basereg, basereg, offsreg, arm::Shift(arm::ShiftOp::kLSL, scalesizep.scale()));
				mem = arm::Mem(basereg);
			}
			else
			{
				mem = arm::Mem(basereg, offsreg);
			}
		}

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

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];

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

	mov_param_reg(a, inst.size(), dstp, REG_PARAM1);
}

void drcbe_arm64::op_readm(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter addrp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];

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

	mov_param_reg(a, inst.size(), dstp, REG_PARAM1);
}

void drcbe_arm64::op_write(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	const parameter &spacesizep = inst.param(2);
	assert(spacesizep.is_size_space());

	auto const &accessors = m_memory_accessors[spacesizep.space()];

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

void drcbe_arm64::op_writem(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter addrp(*this, inst.param(0), PTYPE_MRI);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);
	be_parameter maskp(*this, inst.param(2), PTYPE_MRI);
	const parameter &spacesizep = inst.param(3);
	assert(spacesizep.is_size_space());

	// set up a call to the write handler
	auto const &accessors = m_memory_accessors[spacesizep.space()];

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

void drcbe_arm64::op_carry(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C);

	be_parameter srcp(*this, inst.param(0), PTYPE_MRI);
	be_parameter bitp(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
	const a64::Gp scratch = select_register(FUNC_SCRATCH_REG, inst.size());

	// load non-immediate bit numbers into a register
	// flags = (flags & ~FLAG_C) | ((src >> (PARAM1 & 31)) & FLAG_C)

	if (srcp.is_immediate() && bitp.is_immediate())
	{
		a.mov(scratch, BIT(srcp.immediate(), bitp.immediate()));
		a.bfi(FLAGS_REG, scratch.x(), 0, 1);
	}
	else if (bitp.is_immediate())
	{
		const auto shift = bitp.immediate() % (inst.size() * 8);

		mov_reg_param(a, inst.size(), src, srcp);

		// move carry bit to lsb
		if (shift != 0)
		{
			a.lsr(scratch, src, shift);
			store_carry_reg(a, scratch);
		}
		else
		{
			store_carry_reg(a, src);
		}
	}
	else
	{
		const a64::Gp shift = bitp.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src, srcp);
		mov_reg_param(a, inst.size(), shift, bitp);

		a.and_(shift, shift, inst.size() * 8 - 1);

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
	else
	{
		const a64::Gp dst = dstp.select_register(TEMP_REG1, inst.size());

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
		{
			get_unordered(a, dst);

			if (inst.condition() == uml::COND_NU)
				a.eor(dst, dst, 1);
		}
		else
			a.cset(dst, ARM_CONDITION(a, inst.condition()));

		mov_param_reg(a, inst.size(), dstp, dst);
	}
}

void drcbe_arm64::op_mov(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter srcp(*this, inst.param(1), PTYPE_MRI);

	// add a conditional branch unless a conditional move is possible
	Label skip;

	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), skip, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), skip);
	}

	mov_param_param(a, inst.size(), dstp, srcp);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);
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
			if (size == SIZE_BYTE)
				emit_ldrsb_mem(a, dstreg.x(), srcp.memory());
			else if (size == SIZE_WORD)
				emit_ldrsh_mem(a, dstreg.x(), srcp.memory());
			else if (size == SIZE_DWORD)
				emit_ldrsw_mem(a, dstreg.x(), srcp.memory());
			else if (size == SIZE_QWORD)
				emit_ldr_mem(a, dstreg.x(), srcp.memory());
		}
		else
		{
			const a64::Gp tempreg = srcp.select_register(dstreg, 8);
			mov_reg_param(a, inst.size(), tempreg, srcp);

			if (size == SIZE_BYTE)
				a.sxtb(dstreg.x(), tempreg.w());
			else if (size == SIZE_WORD)
				a.sxth(dstreg.x(), tempreg.w());
			else if (size == SIZE_DWORD)
				a.sxtw(dstreg.x(), tempreg.w());
		}

		mov_param_reg(a, inst.size(), dstp, dstreg);
	}

	if (inst.flags())
		a.tst(dstreg, dstreg);
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
	const a64::Gp shift = shiftp.select_register(TEMP_REG2, inst.size());
	const a64::Gp scratch = shiftp.select_register(FUNC_SCRATCH_REG, inst.size());
	const uint64_t instbits = inst.size() * 8;

	if (maskp.is_immediate() && maskp.is_immediate_value(0))
	{
		// A zero mask will always result in zero so optimize it out
		const a64::Gp zero = select_register(a64::xzr, inst.size());

		mov_param_reg(a, inst.size(), dstp, zero);

		if (inst.flags())
			a.tst(zero, zero);

		return;
	}

	bool optimized = false;
	if (srcp.is_immediate() && shiftp.is_immediate() && maskp.is_immediate())
	{
		// Optimize all constant inputs into a single mov
		uint64_t result = srcp.immediate();

		if (shiftp.immediate() != 0)
		{
			if (inst.size() == 4)
				result = rotl_32(result, shiftp.immediate());
			else
				result = rotl_64(result, shiftp.immediate());
		}

		a.mov(output, result & maskp.immediate());

		optimized = true;
	}
	else if (maskp.is_immediate() && shiftp.is_immediate() && !maskp.is_immediate_value(util::make_bitmask<uint64_t>(instbits)))
	{
		// A mask of all 1s will be handled efficiently in the unoptimized path, so only optimize for the other cases if possible
		const auto pop = population_count_64(maskp.immediate());
		const auto lz = count_leading_zeros_64(maskp.immediate()) & (instbits - 1);
		const auto invlamask = ~(maskp.immediate() << lz) & util::make_bitmask<uint64_t>(instbits);
		const bool is_right_aligned = (maskp.immediate() & (maskp.immediate() + 1)) == 0;
		const bool is_contiguous = (invlamask & (invlamask + 1)) == 0;
		const auto s = shiftp.immediate() & (instbits - 1);

		if (is_right_aligned || is_contiguous)
		{
			mov_reg_param(a, inst.size(), output, srcp);
			optimized = true;
		}

		if (is_right_aligned)
		{
			// Optimize a contiguous right-aligned mask
			const auto s2 = (instbits - s) & (instbits - 1);

			if (s >= pop)
			{
				a.ubfx(output, output, s2, pop);
			}
			else
			{
				if (s2 > 0)
					a.ror(output, output, s2);

				a.bfc(output, pop, instbits - pop);
			}
		}
		else if (is_contiguous)
		{
			// Optimize a contiguous mask
			auto const rot = -int(s + pop + lz) & (instbits - 1);

			if (rot > 0)
				a.ror(output, output, rot);

			a.ubfiz(output, output, instbits - pop - lz, pop);
		}
	}

	if (!optimized)
	{
		mov_reg_param(a, inst.size(), output, srcp);

		if (shiftp.is_immediate())
		{
			const auto s = -int64_t(shiftp.immediate()) & (instbits - 1);

			if (s != 0)
				a.ror(output, output, s);
		}
		else
		{
			const a64::Gp scratch2 = select_register(SCRATCH_REG2, inst.size());

			mov_reg_param(a, inst.size(), shift, shiftp);

			a.and_(scratch, shift, inst.size() * 8 - 1);
			a.mov(scratch2, instbits);
			a.sub(scratch, scratch2, scratch);
			a.ror(output, output, scratch);
		}

		// srcp and the results of the rors above are already going to the output register, so if the mask is all 1s this can all be skipped
		if (maskp.is_immediate() && is_valid_immediate_mask(maskp.immediate(), inst.size()))
		{
			a.ands(output, output, maskp.immediate());
		}
		else if (!maskp.is_immediate() || maskp.immediate() != util::make_bitmask<uint64_t>(instbits))
		{
			const a64::Gp mask = maskp.select_register(TEMP_REG2, inst.size());
			mov_reg_param(a, inst.size(), mask, maskp);

			a.ands(output, output, mask);
		}
	}

	mov_param_reg(a, inst.size(), dstp, output);

	if (inst.flags())
		a.tst(output, output);
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

	a64::Gp dst;

	if (maskp.is_immediate() && maskp.is_immediate_value(0))
	{
		// A zero mask means no bits will be inserted so it can be optimized out
		if (inst.flags())
		{
			dst = dstp.select_register(TEMP_REG2, inst.size());
			mov_reg_param(a, inst.size(), dst, dstp);
			a.tst(dst, dst);
		}

		return;
	}

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && srcp.is_int_register())
		can_use_dst_reg = srcp.ireg() != dstp.ireg();
	if (can_use_dst_reg && maskp.is_int_register())
		can_use_dst_reg = maskp.ireg() != dstp.ireg();
	if (can_use_dst_reg && shiftp.is_int_register())
		can_use_dst_reg = shiftp.ireg() != dstp.ireg();

	bool optimized = false;
	if (srcp.is_immediate() && maskp.is_immediate() && shiftp.is_immediate() && maskp.is_immediate_value(util::make_bitmask<uint64_t>(instbits)))
	{
		dst = dstp.select_register(TEMP_REG2, inst.size());

		uint64_t result = 0;
		if (inst.size() == 4)
			result = rotl_32(srcp.immediate(), shiftp.immediate());
		else
			result = rotl_64(srcp.immediate(), shiftp.immediate());

		a.mov(dst, result);

		optimized = true;
	}
	else if (maskp.is_immediate() && shiftp.is_immediate() && maskp.is_immediate_value(util::make_bitmask<uint64_t>(instbits)))
	{
		// a mask of all 1s means that the result of the rol will completely overwrite
		// the output value, so just load the source value into the output register and rol on that
		dst = dstp.select_register(TEMP_REG2, inst.size());
		mov_reg_param(a, inst.size(), dst, srcp);

		const auto shift = -int64_t(shiftp.immediate()) & (instbits - 1);

		if (shift != 0)
			a.ror(dst, dst, shift);

		optimized = true;
	}
	else if (maskp.is_immediate() && shiftp.is_immediate())
	{
		const auto pop = population_count_64(maskp.immediate());
		const auto lz = count_leading_zeros_64(maskp.immediate()) & (instbits - 1);
		const auto invlamask = ~(maskp.immediate() << lz) & util::make_bitmask<uint64_t>(instbits);
		const bool is_right_aligned = (maskp.immediate() & (maskp.immediate() + 1)) == 0;
		const bool is_contiguous = (invlamask & (invlamask + 1)) == 0;
		const auto s = shiftp.immediate() & (instbits - 1);

		const a64::Gp src = select_register(SCRATCH_REG2, inst.size());

		if (is_right_aligned || is_contiguous)
		{
			dst = can_use_dst_reg ? dstp.select_register(SCRATCH_REG1, inst.size()) : select_register(SCRATCH_REG1, inst.size());
			mov_reg_param(a, inst.size(), dst, dstp);

			uint32_t rot = 0;
			uint32_t lsb = 0;

			if (is_right_aligned)
			{
				// Optimize a contiguous right-aligned mask
				rot = (instbits - s) & (instbits - 1);
			}
			else if (is_contiguous)
			{
				// Optimize a contiguous mask
				rot = -int32_t(s + pop + lz) & (instbits - 1);
				lsb = instbits - pop - lz;
			}

			if (srcp.is_immediate() && rot > 0)
			{
				// save some instructions by avoid mov to register by computing the ror and storing it into src directly
				uint64_t result = 0;

				if (inst.size() == 4)
					result = rotr_32(srcp.immediate(), rot);
				else
					result = rotr_64(srcp.immediate(), rot);

				a.mov(src, result);
			}
			else
			{
				mov_reg_param(a, inst.size(), src, srcp);

				if (rot > 0)
					a.ror(src, src, rot);
			}

			a.bfi(dst, src, lsb, pop);

			optimized = true;
		}
		else if (srcp.is_immediate())
		{
			const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());

			dst = dstp.select_register(TEMP_REG2, inst.size());

			// val1 = src & ~PARAM3
			if (is_valid_immediate_mask(maskp.immediate(), inst.size()))
			{
				a.and_(dst, dst, ~maskp.immediate());
			}
			else
			{
				a.mov(scratch, ~maskp.immediate());
				a.and_(dst, dst, scratch);
			}

			uint64_t result = 0;
			if (inst.size() == 4)
				result = rotl_32(srcp.immediate(), s) & maskp.immediate();
			else
				result = rotl_64(srcp.immediate(), s) & maskp.immediate();

			if (result != 0)
			{
				if (is_valid_immediate(result, 12))
				{
					a.orr(dst, dst, result);
				}
				else
				{
					a.mov(scratch, result);
					a.orr(dst, dst, select_register(scratch, inst.size()));
				}
			}

			optimized = true;
		}
	}

	if (!optimized)
	{
		dst = can_use_dst_reg ? dstp.select_register(TEMP_REG2, inst.size()) : select_register(TEMP_REG2, inst.size());
		mov_reg_param(a, inst.size(), dst, dstp);

		const a64::Gp src = srcp.select_register(TEMP_REG1, inst.size());
		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());

		mov_reg_param(a, inst.size(), src, srcp);

		if (shiftp.is_immediate())
		{
			const auto shift = -int64_t(shiftp.immediate()) & ((inst.size() * 8) - 1);

			if (shift != 0)
				a.ror(scratch, src, shift);
			else
				a.mov(scratch, src);
		}
		else
		{
			const a64::Gp shift = shiftp.select_register(SCRATCH_REG2, inst.size());
			const a64::Gp scratch2 = shiftp.select_register(FUNC_SCRATCH_REG, inst.size());
			mov_reg_param(a, inst.size(), shift, shiftp);

			a.mov(scratch, inst.size() * 8);
			a.and_(scratch2, shift, inst.size() * 8 - 1);
			a.sub(scratch2, scratch, scratch2);
			a.ror(scratch, src, scratch2);
		}

		const a64::Gp mask = maskp.select_register(SCRATCH_REG2, inst.size());
		mov_reg_param(a, inst.size(), mask, maskp);

		a.bic(dst, dst, mask); // val1 = src & ~PARAM3
		a.and_(scratch, scratch, mask); // val2 = val2 & PARAM3
		a.orr(dst, dst, scratch); // val1 | val2
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
		a.tst(dst, dst);
}

template <a64::Inst::Id Opcode> void drcbe_arm64::op_add(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp output = dstp.select_register(TEMP_REG3, inst.size());

	if (Opcode == a64::Inst::kIdAdcs)
		load_carry(a);

	if (src1p.is_immediate() && is_valid_immediate(src1p.immediate(), 11))
	{
		const a64::Gp src = src2p.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src, src2p);
		if (src1p.immediate() == 0)
			a.emit(Opcode, output, src, select_register(a64::xzr, inst.size()));
		else
			a.emit(Opcode, output, src, src1p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else if (src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 11))
	{
		const a64::Gp src = src1p.select_register(TEMP_REG1, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		if (src2p.is_immediate_value(0))
			a.emit(Opcode, output, src, select_register(a64::xzr, inst.size()));
		else
			a.emit(Opcode, output, src, src2p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);
		a.emit(Opcode, output, src1, src2);
		mov_param_reg(a, inst.size(), dstp, output);
	}

	store_carry(a);
}

template <a64::Inst::Id Opcode> void drcbe_arm64::op_sub(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	if (Opcode == a64::Inst::kIdSbcs)
		load_carry(a, true);

	const a64::Gp output = dstp.select_register(TEMP_REG3, inst.size());

	if (src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 11))
	{
		const a64::Gp src = select_register(TEMP_REG1, inst.size());

		mov_reg_param(a, inst.size(), src, src1p);
		if (src2p.is_immediate_value(0))
			a.emit(Opcode, output, src, select_register(a64::xzr, inst.size()));
		else
			a.emit(Opcode, output, src, src2p.immediate());
		mov_param_reg(a, inst.size(), dstp, output);
	}
	else
	{
		const a64::Gp src1 = select_register(TEMP_REG1, inst.size());
		const a64::Gp src2 = select_register(TEMP_REG2, inst.size());

		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);
		a.emit(Opcode, output, src1, src2);
		mov_param_reg(a, inst.size(), dstp, output);
	}

	store_carry(a, true);
}

void drcbe_arm64::op_cmp(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_V | FLAG_Z | FLAG_S);

	be_parameter src1p(*this, inst.param(0), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(1), PTYPE_MRI);

	const a64::Gp temp = select_register(TEMP_REG1, inst.size());
	const a64::Gp temp2 = select_register(TEMP_REG2, inst.size());

	mov_reg_param(a, inst.size(), temp, src1p);

	if (src2p.is_immediate() && is_valid_immediate(src2p.immediate(), 11))
	{
		if (src2p.is_immediate_value(0))
			a.cmp(temp, select_register(a64::xzr, inst.size()));
		else
			a.cmp(temp, src2p.immediate());
	}
	else
	{
		mov_reg_param(a, inst.size(), temp2, src2p);
		a.cmp(temp, temp2);
	}

	store_carry(a, true);
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
	const a64::Gp lo = TEMP_REG3;
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, a64::xzr);
		a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			a.umulh(hi, src1, src2);
		}
		else
		{
			a.umull(lo, src1, src2);
			a.lsr(hi, lo, 32);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);
	if (compute_hi)
		mov_param_reg(a, inst.size(), edstp, hi);

	if (inst.flags())
	{
		a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

		a.tst(lo, lo);
		a.cset(TEMP_REG1, a64::CondCode::kEQ);
		a.tst(hi, hi);
		a.cset(TEMP_REG3, a64::CondCode::kEQ);
		a.and_(TEMP_REG1, TEMP_REG1, TEMP_REG3);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 30, 1); // zero flag

		a.tst(hi, hi); // overflow check
		a.cset(TEMP_REG3, a64::CondCode::kNE);
		a.bfi(SCRATCH_REG1, TEMP_REG3, 28, 1); // overflow flag

		a.lsr(TEMP_REG3, hi, inst.size() * 8 - 1); // take top bit of result as sign flag
		a.bfi(SCRATCH_REG1, TEMP_REG3, 31, 1); // sign flag

		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}
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
	const a64::Gp lo = TEMP_REG3;
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, a64::xzr);
		a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			a.umulh(hi, src1, src2);
		}
		else
		{
			a.umull(lo, src1, src2);
			a.lsr(hi, lo, 32);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);

	if (inst.flags())
	{
		a.mrs(TEMP_REG1, a64::Predicate::SysReg::kNZCV);

		a.tst(select_register(lo, inst.size()), select_register(lo, inst.size()));
		a.cset(SCRATCH_REG1, a64::CondCode::kEQ);
		a.bfi(TEMP_REG1, SCRATCH_REG1, 30, 1); // zero flag

		a.cmp(hi, 0);
		a.cset(SCRATCH_REG1, a64::CondCode::kNE);
		a.bfi(TEMP_REG1, SCRATCH_REG1, 28, 1); // overflow flag

		a.lsr(SCRATCH_REG1, lo, inst.size() * 8 - 1); // take top bit of result as sign flag
		a.bfi(TEMP_REG1, SCRATCH_REG1, 31, 1); // sign flag

		a.msr(a64::Predicate::SysReg::kNZCV, TEMP_REG1);
	}
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
	const a64::Gp lo = TEMP_REG3;
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, a64::xzr);
		a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);
			a.smulh(hi, src1, src2);
		}
		else
		{
			a.smull(lo, src1, src2);
			a.lsr(hi, lo, 32);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);
	if (compute_hi)
		mov_param_reg(a, inst.size(), edstp, hi);

	if (inst.flags())
	{
		a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

		a.tst(lo, lo);
		a.cset(TEMP_REG1, a64::CondCode::kEQ);
		a.tst(hi, hi);
		a.cset(SCRATCH_REG2, a64::CondCode::kEQ);
		a.and_(TEMP_REG1, TEMP_REG1, SCRATCH_REG2);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 30, 1); // zero flag

		if (inst.size() == 4)
		{
			a.sxtw(TEMP_REG1, lo.w());
			a.cmp(TEMP_REG1, lo);
		}
		else
		{
			a.asr(TEMP_REG1, lo, 63);
			a.cmp(TEMP_REG1, hi);
		}

		a.cset(TEMP_REG1, a64::CondCode::kNE);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 28, 1); // overflow flag

		a.lsr(TEMP_REG1, hi, inst.size() * 8 - 1); // take top bit of result as sign flag
		a.bfi(SCRATCH_REG1, TEMP_REG1, 31, 1); // sign flag

		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}
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
	const a64::Gp lo = TEMP_REG3;
	const a64::Gp hi = TEMP_REG2;

	if ((src1p.is_immediate() && src1p.is_immediate_value(0)) || (src2p.is_immediate() && src2p.is_immediate_value(0)))
	{
		a.mov(lo, a64::xzr);

		if (inst.flags() && inst.size() == 8)
			a.mov(hi, a64::xzr);
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		if (inst.size() == 8)
		{
			a.mul(lo, src1, src2);

			if (inst.flags())
				a.smulh(hi, src1, src2);
		}
		else
		{
			a.smull(lo, src1, src2);
		}
	}

	mov_param_reg(a, inst.size(), dstp, lo);

	if (inst.flags())
	{
		a.mrs(SCRATCH_REG1, a64::Predicate::SysReg::kNZCV);

		a.tst(select_register(lo, inst.size()), select_register(lo, inst.size()));
		a.cset(TEMP_REG1, a64::CondCode::kEQ);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 30, 1); // zero flag

		if (inst.size() == 4)
		{
			a.sxtw(TEMP_REG1, lo.w());
			a.cmp(TEMP_REG1, lo);
		}
		else
		{
			a.asr(TEMP_REG1, lo, 63);
			a.cmp(TEMP_REG1, hi);
		}

		a.cset(TEMP_REG1, a64::CondCode::kNE);
		a.bfi(SCRATCH_REG1, TEMP_REG1, 28, 1); // overflow flag

		a.lsr(TEMP_REG1, lo, inst.size() * 8 - 1); // take top bit of result as sign flag
		a.bfi(SCRATCH_REG1, TEMP_REG1, 31, 1); // sign flag

		a.msr(a64::Predicate::SysReg::kNZCV, SCRATCH_REG1);
	}
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
		Label skip_zero = a.newLabel();
		Label skip = a.newLabel();

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
}

void drcbe_arm64::op_and(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());

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
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);

		a.ands(dst, src1, src2p.immediate());
	}
	else if (!inst.flags() && (inst.size() == 8) && src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), 4))
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);

		a.and_(dst.w(), src1.w(), src2p.immediate());
	}
	else
	{
		const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
		const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.ands(dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);
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

	mov_reg_param(a, inst.size(), src1, src1p);

	if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		if (src2p.is_immediate_value(0))
			a.tst(src1, select_register(a64::xzr, inst.size()));
		else
			a.tst(src1, src2p.immediate());
	}
	else
	{
		mov_reg_param(a, inst.size(), src2, src2p);
		a.tst(src1, src2);
	}
}

void drcbe_arm64::op_or(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());

	if (src1p.is_immediate() && src2p.is_immediate())
	{
		get_imm_relative(a, dst, src1p.immediate() | src2p.immediate());
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		if (src2p.is_immediate_value(0))
		{
			if (dst.id() != src1.id())
				a.mov(dst, src1);
		}
		else if (is_valid_immediate(src2p.immediate(), 12))
		{
			a.orr(dst, src1, src2p.immediate());
		}
		else
		{
			a.mov(SCRATCH_REG1, src2p.immediate());
			a.orr(dst, src1, select_register(SCRATCH_REG1, inst.size()));
		}
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.orr(dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
		a.tst(dst, dst);
}

void drcbe_arm64::op_xor(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	const a64::Gp src1 = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp src2 = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp dst = dstp.select_register(TEMP_REG3, inst.size());

	if (src1p.is_immediate() && src2p.is_immediate())
	{
		get_imm_relative(a, dst, src1p.immediate() ^ src2p.immediate());
	}
	else if (src2p.is_immediate() && is_valid_immediate_mask(src2p.immediate(), inst.size()))
	{
		mov_reg_param(a, inst.size(), src1, src1p);

		if (src2p.is_immediate_value(0))
		{
			if (dst.id() != src1.id())
				a.mov(dst, src1);
		}
		else
		{
			a.eor(dst, src1, src2p.immediate());
		}
	}
	else
	{
		mov_reg_param(a, inst.size(), src1, src1p);
		mov_reg_param(a, inst.size(), src2, src2p);

		a.eor(dst, src1, src2);
	}

	mov_param_reg(a, inst.size(), dstp, dst);

	if (inst.flags())
		a.tst(dst, dst);
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
		a.tst(dst, dst);
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
		a.tst(dst, dst);
}


template <a64::Inst::Id Opcode> void drcbe_arm64::op_shift(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = inst.size() * 8 - 1;

	// If possible it's more optimal to write directly to the dst register,
	// but be careful to not overwrite one of the source values since they're needed for later calculations
	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp src = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp dst = can_use_dst_reg ? dstp.select_register(TEMP_REG3, inst.size()) : select_register(TEMP_REG3, inst.size());
	const a64::Gp scratch = select_register(FUNC_SCRATCH_REG, inst.size());

	mov_reg_param(a, inst.size(), src, src1p);

	if (src2p.is_immediate() && is_valid_immediate(src2p.immediate(), (inst.size() == 8) ? 5 : 4))
	{
		const auto shift = src2p.immediate() % (inst.size() * 8);

		a.emit(Opcode, dst, src, shift);

		if (Opcode == a64::Inst::kIdRor || Opcode == a64::Inst::kIdLsr || Opcode == a64::Inst::kIdAsr)
			calculate_carry_shift_right_imm(a, src, shift);
		else if (Opcode == a64::Inst::kIdLsl)
			calculate_carry_shift_left_imm(a, src, shift, maxBits);
	}
	else
	{
		mov_reg_param(a, inst.size(), shift, src2p);

		a.and_(scratch, shift, inst.size() * 8 - 1);

		a.emit(Opcode, dst, src, scratch);

		if (Opcode == a64::Inst::kIdRor || Opcode == a64::Inst::kIdLsr || Opcode == a64::Inst::kIdAsr)
			calculate_carry_shift_right(a, src, scratch);
		else if (Opcode == a64::Inst::kIdLsl)
			calculate_carry_shift_left(a, src, scratch, maxBits);
	}

	if (inst.flags())
		a.tst(dst, dst);

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

	size_t const maxBits = inst.size() * 8 - 1;

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp param = src1p.select_register(TEMP_REG1, inst.size());
	const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
	const a64::Gp output = can_use_dst_reg ? dstp.select_register(TEMP_REG3, inst.size()) : select_register(TEMP_REG3, inst.size());
	const a64::Gp scratch2 = select_register(FUNC_SCRATCH_REG, inst.size());

	mov_reg_param(a, inst.size(), param, src1p);

	if (src2p.is_immediate())
	{
		const auto s = src2p.immediate() % (inst.size() * 8);
		const auto s2 = ((inst.size() * 8) - s) % (inst.size() * 8);

		if (s2 == 0)
		{
			if (output.id() != param.id())
				a.mov(output, param);
		}
		else
		{
			a.ror(output, param, s2);
		}

		calculate_carry_shift_left_imm(a, param, s, maxBits);
	}
	else
	{
		mov_reg_param(a, inst.size(), shift, src2p);

		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());
		a.mov(scratch, inst.size() * 8);
		a.and_(scratch2, shift, maxBits);
		a.sub(scratch, scratch, scratch2);
		a.ror(output, param, scratch);

		calculate_carry_shift_left(a, param, scratch2, maxBits);
	}

	if (inst.flags())
		a.tst(output, output);

	mov_param_reg(a, inst.size(), dstp, output);
}

void drcbe_arm64::op_rolc(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = inst.size() * 8 - 1;

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp param1 = src1p.select_register(TEMP_REG3, inst.size());
	const a64::Gp output = can_use_dst_reg ? dstp.select_register(TEMP_REG1, inst.size()) : select_register(TEMP_REG1, inst.size());
	const a64::Gp carry = select_register(SCRATCH_REG2, inst.size());

	mov_reg_param(a, inst.size(), param1, src1p);

	// shift > 1: src = (PARAM1 << shift) | (carry << (shift - 1)) | (PARAM1 >> (33 - shift))
	// shift = 1: src = (PARAM1 << shift) | carry

	if (src2p.is_immediate())
	{
		const auto shift = src2p.immediate() % (inst.size() * 8);

		if (shift != 0)
		{
			a.ubfx(carry, param1, (inst.size() * 8) - shift, 1);
			if (shift > 1)
				a.ubfx(output, param1, (inst.size() * 8) - shift + 1, shift - 1);
			a.bfi(output.x(), FLAGS_REG, shift - 1, 1);
			a.bfi(output, param1, shift, (inst.size() * 8) - shift);
			a.bfi(FLAGS_REG, carry.x(), 0, 1);
		}
		else
		{
			a.mov(output, param1);
		}

		calculate_carry_shift_left_imm(a, param1, shift, maxBits);
	}
	else
	{
		const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());
		const a64::Gp scratch2 = select_register(FUNC_SCRATCH_REG, inst.size());

		mov_reg_param(a, inst.size(), shift, src2p);

		a.and_(scratch2, shift, maxBits);

		a.lsl(output, param1, scratch2); // PARAM1 << shift

		Label skip = a.newLabel();
		Label skip3 = a.newLabel();
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

		a.bind(skip3);

		calculate_carry_shift_left(a, param1, scratch2, maxBits);
	}

	if (inst.flags())
		a.tst(output, output);

	mov_param_reg(a, inst.size(), dstp, output);
}

void drcbe_arm64::op_rorc(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_flags(inst, FLAG_C | FLAG_Z | FLAG_S);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
	be_parameter src1p(*this, inst.param(1), PTYPE_MRI);
	be_parameter src2p(*this, inst.param(2), PTYPE_MRI);

	size_t const maxBits = inst.size() * 8 - 1;

	bool can_use_dst_reg = dstp.is_int_register();
	if (can_use_dst_reg && src1p.is_int_register())
		can_use_dst_reg = src1p.ireg() != dstp.ireg();
	if (can_use_dst_reg && src2p.is_int_register())
		can_use_dst_reg = src2p.ireg() != dstp.ireg();

	const a64::Gp param1 = src1p.select_register(TEMP_REG3, inst.size());
	const a64::Gp output = can_use_dst_reg ? dstp.select_register(TEMP_REG1, inst.size()) : select_register(TEMP_REG1, inst.size());
	const a64::Gp carry = select_register(SCRATCH_REG2, inst.size());

	mov_reg_param(a, inst.size(), param1, src1p);

	// if (shift > 1)
	//  src = (PARAM1 >> shift) | (((flags & FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
	// else if (shift == 1)
	//  src = (PARAM1 >> shift) | ((flags & FLAG_C) << 31);

	if (src2p.is_immediate())
	{
		const auto shift = src2p.immediate() % (inst.size() * 8);

		if (shift != 0)
		{
			a.ubfx(carry, param1, shift - 1, 1);
			a.ubfx(output, param1, shift, (inst.size() * 8) - shift);
			a.bfi(output.x(), FLAGS_REG, (inst.size() * 8) - shift, 1);
			if (shift > 1)
				a.bfi(output, param1, (inst.size() * 8) - shift + 1, shift - 1);
			a.bfi(FLAGS_REG, carry.x(), 0, 1);
		}
		else
		{
			a.mov(output, param1);
		}

		calculate_carry_shift_right_imm(a, param1, shift);
	}
	else
	{
		const a64::Gp shift = src2p.select_register(TEMP_REG2, inst.size());
		const a64::Gp scratch = select_register(SCRATCH_REG1, inst.size());
		const a64::Gp scratch2 = select_register(FUNC_SCRATCH_REG, inst.size());

		mov_reg_param(a, inst.size(), shift, src2p);

		a.and_(scratch2, shift, maxBits);

		a.lsr(output, param1, shift); // PARAM1 >> shift

		Label skip = a.newLabel();
		Label skip3 = a.newLabel();
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

		a.bind(skip3);

		calculate_carry_shift_right(a, param1, scratch2);
	}

	if (inst.flags())
		a.tst(output, output);

	mov_param_reg(a, inst.size(), dstp, output);
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
		a.ldr(dstreg, arm::Mem(basereg, indp.immediate() * inst.size()));
	}
	else
	{
		const a64::Gp indreg = indp.select_register(TEMP_REG1, 4);

		mov_reg_param(a, 4, indreg, indp);

		a.ldr(dstreg, arm::Mem(basereg, indreg, arm::Shift(arm::ShiftOp::kLSL, (inst.size() == 4) ? 2 : 3)));
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
		a.str(srcreg, arm::Mem(basereg, indp.immediate() * inst.size()));
	}
	else
	{
		const a64::Gp indreg = indp.select_register(TEMP_REG1, 4);

		mov_reg_param(a, 4, indreg, indp);

		a.str(srcreg, arm::Mem(basereg, indreg, arm::Shift(arm::ShiftOp::kLSL, (inst.size() == 4) ? 2 : 3)));
	}
}

void drcbe_arm64::op_fread(a64::Assembler &a, const uml::instruction &inst)
{
	assert(inst.size() == 4 || inst.size() == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	be_parameter dstp(*this, inst.param(0), PTYPE_MR);
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

	Label skip;
	if (inst.condition() != uml::COND_ALWAYS)
	{
		skip = a.newLabel();

		if (inst.condition() == COND_C || inst.condition() == COND_NC || inst.condition() == COND_A || inst.condition() == COND_BE)
			load_carry(a, true);

		if (inst.condition() == uml::COND_U || inst.condition() == uml::COND_NU)
			check_unordered_condition(a, inst.condition(), skip, true);
		else
			a.b(ARM_NOT_CONDITION(a, inst.condition()), skip);
	}

	mov_float_param_param(a, inst.size(), dstp, srcp);

	if (inst.condition() != uml::COND_ALWAYS)
		a.bind(skip);
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
		case ROUND_ROUND:
			a.fcvtns(dstreg, srcreg);
			break;

		case ROUND_CEIL:
			a.fcvtps(dstreg, srcreg);
			break;

		case ROUND_FLOOR:
			a.fcvtms(dstreg, srcreg);
			break;

		case ROUND_TRUNC:
		case ROUND_DEFAULT:
		default:
			a.fcvtzs(dstreg, srcreg);
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

	store_carry(a, true);
	store_unordered(a);
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
