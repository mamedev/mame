// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex64.h

    64-bit x64 back-end for the universal machine language.

***************************************************************************/

#pragma once

#ifndef MAME_DEVICES_CPU_DRCBEX64_H
#define MAME_DEVICES_CPU_DRCBEX64_H

#include "drcuml.h"
#include "drcbeut.h"
#include "x86log.h"

#include "asmjit/src/asmjit/asmjit.h"

using namespace asmjit;
using namespace asmjit::x86;

namespace drc {
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class drcbe_x64 : public drcbe_interface
{
	typedef uint32_t (*x86_entry_point_func)(uint8_t *rbpvalue, x86code *entry);

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
		static int const REG_MAX = 16;

		// parameter types
		enum be_parameter_type
		{
			PTYPE_NONE = 0,                     // invalid
			PTYPE_IMMEDIATE,                    // immediate; value = sign-extended to 64 bits
			PTYPE_INT_REGISTER,                 // integer register; value = 0-REG_MAX
			PTYPE_FLOAT_REGISTER,               // floating point register; value = 0-REG_MAX
			PTYPE_VECTOR_REGISTER,              // vector register; value = 0-REG_MAX
			PTYPE_MEMORY,                       // memory; value = pointer to memory
			PTYPE_MAX
		};

		// represents the value of a parameter
		typedef uint64_t be_parameter_value;

		// construction
		be_parameter() : m_type(PTYPE_NONE), m_value(0) { }
		be_parameter(be_parameter const &param) : m_type(param.m_type), m_value(param.m_value) { }
		be_parameter(uint64_t val) : m_type(PTYPE_IMMEDIATE), m_value(val) { }
		be_parameter(drcbe_x64 &drcbe, const uml::parameter &param, uint32_t allowed);

		// creators for types that don't safely default
		static inline be_parameter make_ireg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_INT_REGISTER, regnum); }
		static inline be_parameter make_freg(int regnum) { assert(regnum >= 0 && regnum < REG_MAX); return be_parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static inline be_parameter make_memory(void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(base)); }
		static inline be_parameter make_memory(const void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(const_cast<void *>(base))); }

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
		Gp select_register(Gp defreg) const;
		Xmm select_register(Xmm defreg) const;
		template <typename T> T select_register(T defreg, be_parameter const &checkparam) const;
		template <typename T> T select_register(T defreg, be_parameter const &checkparam, be_parameter const &checkparam2) const;

	private:
		// private constructor
		be_parameter(be_parameter_type type, be_parameter_value value) : m_type(type), m_value(value) { }

		// internals
		be_parameter_type   m_type;             // parameter type
		be_parameter_value  m_value;            // parameter value
	};

	// helpers
	Mem MABS(const void *ptr, const uint32_t size = 0) const { return Mem(rbp, offset_from_rbp(ptr), size); }
	bool short_immediate(int64_t immediate) const { return (int32_t)immediate == immediate; }
	void normalize_commutative(be_parameter &inner, be_parameter &outer);
	int32_t offset_from_rbp(const void *ptr) const;
	Gp get_base_register_and_offset(Assembler &a, void *target, Gp const &reg, int32_t &offset);
	void smart_call_r64(Assembler &a, x86code *target, Gp const &reg);
	void smart_call_m64(Assembler &a, x86code **target);

	static void debug_log_hashjmp(offs_t pc, int mode);
	static void debug_log_hashjmp_fail();

	// code generators
	void op_handle(Assembler &a, const uml::instruction &inst);
	void op_hash(Assembler &a, const uml::instruction &inst);
	void op_label(Assembler &a, const uml::instruction &inst);
	void op_comment(Assembler &a, const uml::instruction &inst);
	void op_mapvar(Assembler &a, const uml::instruction &inst);

	void op_nop(Assembler &a, const uml::instruction &inst);
	void op_debug(Assembler &a, const uml::instruction &inst);
	void op_exit(Assembler &a, const uml::instruction &inst);
	void op_hashjmp(Assembler &a, const uml::instruction &inst);
	void op_jmp(Assembler &a, const uml::instruction &inst);
	void op_exh(Assembler &a, const uml::instruction &inst);
	void op_callh(Assembler &a, const uml::instruction &inst);
	void op_ret(Assembler &a, const uml::instruction &inst);
	void op_callc(Assembler &a, const uml::instruction &inst);
	void op_recover(Assembler &a, const uml::instruction &inst);

	void op_setfmod(Assembler &a, const uml::instruction &inst);
	void op_getfmod(Assembler &a, const uml::instruction &inst);
	void op_getexp(Assembler &a, const uml::instruction &inst);
	void op_getflgs(Assembler &a, const uml::instruction &inst);
	void op_save(Assembler &a, const uml::instruction &inst);
	void op_restore(Assembler &a, const uml::instruction &inst);

	void op_load(Assembler &a, const uml::instruction &inst);
	void op_loads(Assembler &a, const uml::instruction &inst);
	void op_store(Assembler &a, const uml::instruction &inst);
	void op_read(Assembler &a, const uml::instruction &inst);
	void op_readm(Assembler &a, const uml::instruction &inst);
	void op_write(Assembler &a, const uml::instruction &inst);
	void op_writem(Assembler &a, const uml::instruction &inst);
	void op_carry(Assembler &a, const uml::instruction &inst);
	void op_set(Assembler &a, const uml::instruction &inst);
	void op_mov(Assembler &a, const uml::instruction &inst);
	void op_sext(Assembler &a, const uml::instruction &inst);
	void op_roland(Assembler &a, const uml::instruction &inst);
	void op_rolins(Assembler &a, const uml::instruction &inst);
	void op_add(Assembler &a, const uml::instruction &inst);
	void op_addc(Assembler &a, const uml::instruction &inst);
	void op_sub(Assembler &a, const uml::instruction &inst);
	void op_subc(Assembler &a, const uml::instruction &inst);
	void op_cmp(Assembler &a, const uml::instruction &inst);
	void op_mulu(Assembler &a, const uml::instruction &inst);
	void op_muls(Assembler &a, const uml::instruction &inst);
	void op_divu(Assembler &a, const uml::instruction &inst);
	void op_divs(Assembler &a, const uml::instruction &inst);
	void op_and(Assembler &a, const uml::instruction &inst);
	void op_test(Assembler &a, const uml::instruction &inst);
	void op_or(Assembler &a, const uml::instruction &inst);
	void op_xor(Assembler &a, const uml::instruction &inst);
	void op_lzcnt(Assembler &a, const uml::instruction &inst);
	void op_tzcnt(Assembler &a, const uml::instruction &inst);
	void op_bswap(Assembler &a, const uml::instruction &inst);
	template <Inst::Id Opcode> void op_shift(Assembler &a, const uml::instruction &inst);

	void op_fload(Assembler &a, const uml::instruction &inst);
	void op_fstore(Assembler &a, const uml::instruction &inst);
	void op_fread(Assembler &a, const uml::instruction &inst);
	void op_fwrite(Assembler &a, const uml::instruction &inst);
	void op_fmov(Assembler &a, const uml::instruction &inst);
	void op_ftoint(Assembler &a, const uml::instruction &inst);
	void op_ffrint(Assembler &a, const uml::instruction &inst);
	void op_ffrflt(Assembler &a, const uml::instruction &inst);
	void op_frnds(Assembler &a, const uml::instruction &inst);
	void op_fadd(Assembler &a, const uml::instruction &inst);
	void op_fsub(Assembler &a, const uml::instruction &inst);
	void op_fcmp(Assembler &a, const uml::instruction &inst);
	void op_fmul(Assembler &a, const uml::instruction &inst);
	void op_fdiv(Assembler &a, const uml::instruction &inst);
	void op_fneg(Assembler &a, const uml::instruction &inst);
	void op_fabs(Assembler &a, const uml::instruction &inst);
	void op_fsqrt(Assembler &a, const uml::instruction &inst);
	void op_frecip(Assembler &a, const uml::instruction &inst);
	void op_frsqrt(Assembler &a, const uml::instruction &inst);
	void op_fcopyi(Assembler &a, const uml::instruction &inst);
	void op_icopyf(Assembler &a, const uml::instruction &inst);

	// alu and shift operation helpers
	static bool ones(u64 const value, unsigned const size) noexcept { return (size == 4) ? u32(value) == 0xffffffffU : value == 0xffffffff'ffffffffULL; }
	void alu_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param, std::function<bool(Assembler &a, Operand const &dst, be_parameter const &src)> optimize = [](Assembler &a, Operand dst, be_parameter const &src) { return false; });
	void shift_op_param(Assembler &a, Inst::Id const opcode, Operand const &dst, be_parameter const &param);

	// parameter helpers
	void mov_reg_param(Assembler &a, Gp const &reg, be_parameter const &param, bool const keepflags = false);
	void mov_param_reg(Assembler &a, be_parameter const &param, Gp const &reg);
	void mov_mem_param(Assembler &a, Mem const &memref, be_parameter const &param);

	// special-case move helpers
	void movsx_r64_p32(Assembler &a, Gp const &reg, be_parameter const &param);
	void mov_r64_imm(Assembler &a, Gp const &reg, uint64_t const imm);

	// floating-point helpers
	void movss_r128_p32(Assembler &a, Xmm const &reg, be_parameter const &param);
	void movss_p32_r128(Assembler &a, be_parameter const &param, Xmm const &reg);
	void movsd_r128_p64(Assembler &a, Xmm const &reg, be_parameter const &param);
	void movsd_p64_r128(Assembler &a, be_parameter const &param, Xmm const &reg);

	size_t emit(CodeHolder &ch);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	x86log_context *        m_log;                  // logging
	FILE *                  m_log_asmjit;

	uint32_t *                m_absmask32;            // absolute value mask (32-bit)
	uint64_t *                m_absmask64;            // absolute value mask (32-bit)
	uint8_t *                 m_rbpvalue;             // value of RBP

	x86_entry_point_func    m_entry;                // entry point
	x86code *               m_exit;                 // exit point
	x86code *               m_nocode;               // nocode handler

	// state to live in the near cache
	struct near_state
	{
		x86code *           debug_cpu_instruction_hook;// debugger callback
		x86code *           debug_log_hashjmp;      // hashjmp debugging
		x86code *           debug_log_hashjmp_fail; // hashjmp debugging
		x86code *           drcmap_get_value;       // map lookup helper

		uint32_t              ssemode;                // saved SSE mode
		uint32_t              ssemodesave;            // temporary location for saving
		uint32_t              ssecontrol[4];          // copy of the sse_control array
		float               single1;                // 1.0 is single-precision
		double              double1;                // 1.0 in double-precision

		void *              stacksave;              // saved stack pointer
		void *              hashstacksave;          // saved stack pointer for hashjmp

		uint8_t               flagsmap[0x1000];       // flags map
		uint64_t              flagsunmap[0x20];       // flags unmapper
	};
	near_state &            m_near;

	// globals
	typedef void (drcbe_x64::*opcode_generate_func)(Assembler &a, const uml::instruction &inst);
	struct opcode_table_entry
	{
		uml::opcode_t           opcode;             // opcode in question
		opcode_generate_func    func;               // function pointer to the work
	};
	static const opcode_table_entry s_opcode_table_source[];
	static opcode_generate_func s_opcode_table[uml::OP_MAX];
};

} // namespace drc

using drc::drcbe_x64;

#endif /* MAME_DEVICES_CPU_DRCBEX64_H */
