// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex86.h

    32-bit x86 back-end for the universal machine language.

***************************************************************************/

#ifndef MAME_CPU_DRCBEX86_H
#define MAME_CPU_DRCBEX86_H

#pragma once

#include "drcuml.h"
#include "drcbeut.h"
#include "x86log.h"

#include "asmjit/src/asmjit/asmjit.h"


namespace drc {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class drcbe_x86 : public drcbe_interface
{
	typedef uint32_t (*x86_entry_point_func)(x86code *entry);

public:
	// construction/destruction
	drcbe_x86(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_x86();

	// required overrides
	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst) override;
	virtual bool hash_exists(uint32_t mode, uint32_t pc) override;
	virtual void get_info(drcbe_info &info) override;
	virtual bool logging() const override { return m_log != nullptr; }

private:
	// HACK: leftover from x86emit
	static int const REG_MAX = 16;

	// a be_parameter is similar to a uml::parameter but maps to native registers/memory
	class be_parameter
	{
	public:
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
		be_parameter(drcbe_x86 &drcbe, const uml::parameter &param, uint32_t allowed);

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
		void *memory(uint32_t offset = 0) const { assert(m_type == PTYPE_MEMORY); return reinterpret_cast<void *>(m_value + offset); }

		// type queries
		bool is_immediate() const { return (m_type == PTYPE_IMMEDIATE); }
		bool is_int_register() const { return (m_type == PTYPE_INT_REGISTER); }
		bool is_float_register() const { return (m_type == PTYPE_FLOAT_REGISTER); }
		bool is_memory() const { return (m_type == PTYPE_MEMORY); }

		// other queries
		bool is_immediate_value(uint64_t value) const { return (m_type == PTYPE_IMMEDIATE && m_value == value); }

		// helpers
		asmjit::x86::Gpd select_register(asmjit::x86::Gpd const &defreg) const;
		asmjit::x86::Xmm select_register(asmjit::x86::Xmm defreg) const;
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
	asmjit::x86::Mem MABS(void const *base, u32 const size = 0) const { return asmjit::x86::Mem(u64(base), size); }
	void normalize_commutative(be_parameter &inner, be_parameter &outer);
	void emit_combine_z_flags(asmjit::x86::Assembler &a);
	void emit_combine_z_shl_flags(asmjit::x86::Assembler &a);
	void reset_last_upper_lower_reg();
	void set_last_lower_reg(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Gp const &reglo);
	void set_last_upper_reg(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Gp const &reghi);
	bool can_skip_lower_load(asmjit::x86::Assembler &a, uint32_t *memref, asmjit::x86::Gp const &reglo);
	bool can_skip_upper_load(asmjit::x86::Assembler &a, uint32_t *memref, asmjit::x86::Gp const &reghi);

	static void debug_log_hashjmp(int mode, offs_t pc);

	// code generators
	void op_handle(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_hash(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_label(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_comment(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_mapvar(asmjit::x86::Assembler &a, const uml::instruction &inst);

	void op_nop(asmjit::x86::Assembler &a, const uml::instruction &inst);
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
	void op_muls(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_divu(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_divs(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_and(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_test(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_or(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_xor(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_lzcnt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_tzcnt(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_bswap(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_shl(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_shr(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_sar(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_ror(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_rol(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_rorc(asmjit::x86::Assembler &a, const uml::instruction &inst);
	void op_rolc(asmjit::x86::Assembler &a, const uml::instruction &inst);

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

	// 32-bit code emission helpers
	void emit_mov_r32_p32(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reg, be_parameter const &param);
	void emit_mov_r32_p32_keepflags(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reg, be_parameter const &param);
	void emit_mov_m32_p32(asmjit::x86::Assembler &a, asmjit::x86::Mem memref, be_parameter const &param);
	void emit_mov_p32_r32(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Gp const &reg);

	void alu_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode, asmjit::Operand const &dst, be_parameter const &param, std::function<bool(asmjit::x86::Assembler &a, asmjit::Operand const &dst, be_parameter const &src)> optimize = [](asmjit::x86::Assembler &a, asmjit::Operand dst, be_parameter const &src) { return false; });
	void shift_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode, asmjit::Operand const &dst, be_parameter const &param, std::function<bool(asmjit::x86::Assembler &a, asmjit::Operand const &dst, be_parameter const &src)> optimize);

	// 64-bit code emission helpers
	void emit_mov_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param);
	void emit_mov_r64_p64_keepflags(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param);
	void emit_mov_m64_p64(asmjit::x86::Assembler &a, asmjit::x86::Mem const &memref, be_parameter const &param);
	void emit_mov_p64_r64(asmjit::x86::Assembler &a, be_parameter const &param, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi);
	void emit_and_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_and_m64_p64(asmjit::x86::Assembler &a, asmjit::x86::Mem const &memref_lo, asmjit::x86::Mem const &memref_hi, be_parameter const &param, const uml::instruction &inst);
	void emit_or_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_or_m64_p64(asmjit::x86::Assembler &a, asmjit::x86::Mem const &memref_lo, asmjit::x86::Mem const &memref_hi, be_parameter const &param, const uml::instruction &inst);
	void emit_xor_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_xor_m64_p64(asmjit::x86::Assembler &a, asmjit::x86::Mem const &memref_lo, asmjit::x86::Mem const &memref_hi, be_parameter const &param, const uml::instruction &inst);
	void emit_shl_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_shr_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_sar_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_rol_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_ror_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_rcl_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);
	void emit_rcr_r64_p64(asmjit::x86::Assembler &a, asmjit::x86::Gp const &reglo, asmjit::x86::Gp const &reghi, be_parameter const &param, const uml::instruction &inst);

	void alu_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode_lo, asmjit::x86::Inst::Id const opcode_hi, asmjit::x86::Gp const &lo, asmjit::x86::Gp const &hi, be_parameter const &param, bool const saveflags);
	void alu_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode_lo, asmjit::x86::Inst::Id const opcode_hi, asmjit::x86::Mem const &lo, asmjit::x86::Mem const &hi, be_parameter const &param, bool const saveflags);

	// floating-point code emission helpers
	void emit_fld_p(asmjit::x86::Assembler &a, int size, be_parameter const &param);
	void emit_fstp_p(asmjit::x86::Assembler &a, int size, be_parameter const &param);

	// callback helpers
	static int dmulu(uint64_t &dstlo, uint64_t &dsthi, uint64_t src1, uint64_t src2, bool flags);
	static int dmuls(uint64_t &dstlo, uint64_t &dsthi, int64_t src1, int64_t src2, bool flags);
	static int ddivu(uint64_t &dstlo, uint64_t &dsthi, uint64_t src1, uint64_t src2);
	static int ddivs(uint64_t &dstlo, uint64_t &dsthi, int64_t src1, int64_t src2);

	size_t emit(asmjit::CodeHolder &ch);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	x86log_context *        m_log;                  // logging
	FILE *                  m_log_asmjit;
	bool                    m_logged_common;        // logged common code already?
	bool const              m_sse3;                 // do we have SSE3 support?

	x86_entry_point_func    m_entry;                // entry point
	x86code *               m_exit;                 // exit point
	x86code *               m_nocode;               // nocode handler
	x86code *               m_save;                 // save handler
	x86code *               m_restore;              // restore handler

	uint32_t *              m_reglo[REG_MAX];       // pointer to low part of data for each register
	uint32_t *              m_reghi[REG_MAX];       // pointer to high part of data for each register
	asmjit::x86::Gp         m_last_lower_reg;       // last register we stored a lower from
	x86code *               m_last_lower_pc;        // PC after instruction where we last stored a lower register
	uint32_t *              m_last_lower_addr;      // address where we last stored an lower register
	asmjit::x86::Gp         m_last_upper_reg;       // last register we stored an upper from
	x86code *               m_last_upper_pc;        // PC after instruction where we last stored an upper register
	uint32_t *              m_last_upper_addr;      // address where we last stored an upper register
	double                  m_fptemp;               // temporary storage for floating point

	uint16_t                m_fpumode;              // saved FPU mode
	uint16_t                m_fmodesave;            // temporary location for saving

	void *                  m_stacksave;            // saved stack pointer
	void *                  m_hashstacksave;        // saved stack pointer for hashjmp
	uint64_t                m_reslo;                // extended low result
	uint64_t                m_reshi;                // extended high result

	// globals
	typedef void (drcbe_x86::*opcode_generate_func)(asmjit::x86::Assembler &a, const uml::instruction &inst);
	struct opcode_table_entry
	{
		uml::opcode_t           opcode;             // opcode in question
		opcode_generate_func    func;               // function pointer to the work
	};
	static const opcode_table_entry s_opcode_table_source[];
	static opcode_generate_func s_opcode_table[uml::OP_MAX];
};

} // namespace drc

using drc::drcbe_x86;

#endif // MAME_CPU_DRCBEX86_H
