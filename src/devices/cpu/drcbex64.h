// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex64.h

    64-bit x64 back-end for the universal machine language.

***************************************************************************/
#ifndef MAME_CPU_DRCBEX64_H
#define MAME_CPU_DRCBEX64_H

#pragma once

#include "drcuml.h"
#include "drcbeut.h"
#include "x86log.h"

#include "asmjit/src/asmjit/asmjit.h"

#include <vector>


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
	void shift_op_param(asmjit::x86::Assembler &a, asmjit::x86::Inst::Id const opcode, asmjit::Operand const &dst, be_parameter const &param);

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
		x86code *           debug_cpu_instruction_hook;// debugger callback
		x86code *           debug_log_hashjmp;      // hashjmp debugging
		x86code *           debug_log_hashjmp_fail; // hashjmp debugging
		x86code *           drcmap_get_value;       // map lookup helper

		uint32_t            ssemode;                // saved SSE mode
		uint32_t            ssemodesave;            // temporary location for saving
		uint32_t            ssecontrol[4];          // copy of the sse_control array
		float               single1;                // 1.0 is single-precision
		double              double1;                // 1.0 in double-precision

		void *              stacksave;              // saved stack pointer
		void *              hashstacksave;          // saved stack pointer for hashjmp

		uint8_t             flagsmap[0x1000];       // flags map
		uint64_t            flagsunmap[0x20];       // flags unmapper
	};
	near_state &            m_near;

	// resolved memory handler functions
	struct resolved_handler { uintptr_t obj = 0; x86code *func = nullptr; };
	struct resolved_accessors
	{

		resolved_handler    read_byte;
		resolved_handler    read_word;
		resolved_handler    read_word_masked;
		resolved_handler    read_dword;
		resolved_handler    read_dword_masked;
		resolved_handler    read_qword;
		resolved_handler    read_qword_masked;

		resolved_handler    write_byte;
		resolved_handler    write_word;
		resolved_handler    write_word_masked;
		resolved_handler    write_dword;
		resolved_handler    write_dword_masked;
		resolved_handler    write_qword;
		resolved_handler    write_qword_masked;
	};
	using resolved_accessors_vector = std::vector<resolved_accessors>;
	resolved_accessors_vector m_resolved_accessors;

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

} // namespace drc

using drc::drcbe_x64;

#endif // MAME_CPU_DRCBEX64_H
