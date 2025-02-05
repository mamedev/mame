// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_CPU_DRCBEARM64_H
#define MAME_CPU_DRCBEARM64_H

#pragma once

#include "drcuml.h"
#include "drcbeut.h"

#include "asmjit/src/asmjit/asmjit.h"
#include "asmjit/src/asmjit/a64.h"

#include <vector>


namespace drc {

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
	void mov_signed_reg64_param32(asmjit::a64::Assembler &a, const asmjit::a64::Gp &dst, const be_parameter &src) const;
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

} // namespace drc

using drc::drcbe_arm64;

#endif // MAME_CPU_DRCBEARM64_H
