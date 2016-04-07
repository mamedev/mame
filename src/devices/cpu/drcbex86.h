// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbex86.h

    32-bit x86 back-end for the universal machine language.

***************************************************************************/

#pragma once

#ifndef __DRCBEX86_H__
#define __DRCBEX86_H__

#include "drcuml.h"
#include "drcbeut.h"
#include "x86log.h"

#define X86EMIT_SIZE 32
#include "x86emit.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class drcbe_x86 : public drcbe_interface
{
	typedef UINT32 (*x86_entry_point_func)(x86code *entry);

public:
	// construction/destruction
	drcbe_x86(drcuml_state &drcuml, device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_x86();

	// required overrides
	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst) override;
	virtual bool hash_exists(UINT32 mode, UINT32 pc) override;
	virtual void get_info(drcbe_info &info) override;
	virtual bool logging() const override { return m_log != nullptr; }

private:
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
		typedef UINT64 be_parameter_value;

		// construction
		be_parameter() : m_type(PTYPE_NONE), m_value(0) { }
		be_parameter(const be_parameter &param) : m_type(param.m_type), m_value(param.m_value) { }
		be_parameter(UINT64 val) : m_type(PTYPE_IMMEDIATE), m_value(val) { }
		be_parameter(drcbe_x86 &drcbe, const uml::parameter &param, UINT32 allowed);

		// creators for types that don't safely default
		static inline be_parameter make_ireg(int regnum) { assert(regnum >= 0 && regnum < x86emit::REG_MAX); return be_parameter(PTYPE_INT_REGISTER, regnum); }
		static inline be_parameter make_freg(int regnum) { assert(regnum >= 0 && regnum < x86emit::REG_MAX); return be_parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static inline be_parameter make_vreg(int regnum) { assert(regnum >= 0 && regnum < x86emit::REG_MAX); return be_parameter(PTYPE_VECTOR_REGISTER, regnum); }
		static inline be_parameter make_memory(void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(base)); }
		static inline be_parameter make_memory(const void *base) { return be_parameter(PTYPE_MEMORY, reinterpret_cast<be_parameter_value>(const_cast<void *>(base))); }

		// operators
		bool operator==(const be_parameter &rhs) const { return (m_type == rhs.m_type && m_value == rhs.m_value); }
		bool operator!=(const be_parameter &rhs) const { return (m_type != rhs.m_type || m_value != rhs.m_value); }

		// getters
		be_parameter_type type() const { return m_type; }
		UINT64 immediate() const { assert(m_type == PTYPE_IMMEDIATE); return m_value; }
		int ireg() const { assert(m_type == PTYPE_INT_REGISTER); assert(m_value < x86emit::REG_MAX); return m_value; }
		int freg() const { assert(m_type == PTYPE_FLOAT_REGISTER); assert(m_value < x86emit::REG_MAX); return m_value; }
		int vreg() const { assert(m_type == PTYPE_VECTOR_REGISTER); assert(m_value < x86emit::REG_MAX); return m_value; }
		void *memory(UINT32 offset = 0) const { assert(m_type == PTYPE_MEMORY); return reinterpret_cast<void *>(m_value + offset); }

		// type queries
		bool is_immediate() const { return (m_type == PTYPE_IMMEDIATE); }
		bool is_int_register() const { return (m_type == PTYPE_INT_REGISTER); }
		bool is_float_register() const { return (m_type == PTYPE_FLOAT_REGISTER); }
		bool is_vector_register() const { return (m_type == PTYPE_VECTOR_REGISTER); }
		bool is_memory() const { return (m_type == PTYPE_MEMORY); }

		// other queries
		bool is_immediate_value(UINT64 value) const { return (m_type == PTYPE_IMMEDIATE && m_value == value); }

		// helpers
		int select_register(int defreg) const;
		int select_register(int defreg, const be_parameter &checkparam) const;
		int select_register(int defreg, const be_parameter &checkparam, const be_parameter &checkparam2) const;

	private:
		// private constructor
		be_parameter(be_parameter_type type, be_parameter_value value) : m_type(type), m_value(value) { }

		// internals
		be_parameter_type   m_type;             // parameter type
		be_parameter_value  m_value;            // parameter value
	};

	// helpers
	void normalize_commutative(be_parameter &inner, be_parameter &outer);
	void emit_combine_z_flags(x86code *&dst);
	void emit_combine_z_shl_flags(x86code *&dst);
	void reset_last_upper_lower_reg();
	void set_last_lower_reg(x86code *&dst, const be_parameter &param, UINT8 reglo);
	void set_last_upper_reg(x86code *&dst, const be_parameter &param, UINT8 reghi);
	bool can_skip_lower_load(x86code *&dst, UINT32 *memref, UINT8 reglo);
	bool can_skip_upper_load(x86code *&dst, UINT32 *memref, UINT8 reghi);
	void track_resolve_link(x86code *&destptr, const emit_link &linkinfo);

	void fixup_label(void *parameter, drccodeptr labelcodeptr);
	void fixup_exception(drccodeptr *codeptr, void *param1, void *param2);

	static void debug_log_hashjmp(int mode, offs_t pc);

	// code generators
	void op_handle(x86code *&dst, const uml::instruction &inst);
	void op_hash(x86code *&dst, const uml::instruction &inst);
	void op_label(x86code *&dst, const uml::instruction &inst);
	void op_comment(x86code *&dst, const uml::instruction &inst);
	void op_mapvar(x86code *&dst, const uml::instruction &inst);

	void op_nop(x86code *&dst, const uml::instruction &inst);
	void op_debug(x86code *&dst, const uml::instruction &inst);
	void op_exit(x86code *&dst, const uml::instruction &inst);
	void op_hashjmp(x86code *&dst, const uml::instruction &inst);
	void op_jmp(x86code *&dst, const uml::instruction &inst);
	void op_exh(x86code *&dst, const uml::instruction &inst);
	void op_callh(x86code *&dst, const uml::instruction &inst);
	void op_ret(x86code *&dst, const uml::instruction &inst);
	void op_callc(x86code *&dst, const uml::instruction &inst);
	void op_recover(x86code *&dst, const uml::instruction &inst);

	void op_setfmod(x86code *&dst, const uml::instruction &inst);
	void op_getfmod(x86code *&dst, const uml::instruction &inst);
	void op_getexp(x86code *&dst, const uml::instruction &inst);
	void op_getflgs(x86code *&dst, const uml::instruction &inst);
	void op_save(x86code *&dst, const uml::instruction &inst);
	void op_restore(x86code *&dst, const uml::instruction &inst);

	void op_load(x86code *&dst, const uml::instruction &inst);
	void op_loads(x86code *&dst, const uml::instruction &inst);
	void op_store(x86code *&dst, const uml::instruction &inst);
	void op_read(x86code *&dst, const uml::instruction &inst);
	void op_readm(x86code *&dst, const uml::instruction &inst);
	void op_write(x86code *&dst, const uml::instruction &inst);
	void op_writem(x86code *&dst, const uml::instruction &inst);
	void op_carry(x86code *&dst, const uml::instruction &inst);
	void op_set(x86code *&dst, const uml::instruction &inst);
	void op_mov(x86code *&dst, const uml::instruction &inst);
	void op_sext(x86code *&dst, const uml::instruction &inst);
	void op_roland(x86code *&dst, const uml::instruction &inst);
	void op_rolins(x86code *&dst, const uml::instruction &inst);
	void op_add(x86code *&dst, const uml::instruction &inst);
	void op_addc(x86code *&dst, const uml::instruction &inst);
	void op_sub(x86code *&dst, const uml::instruction &inst);
	void op_subc(x86code *&dst, const uml::instruction &inst);
	void op_cmp(x86code *&dst, const uml::instruction &inst);
	void op_mulu(x86code *&dst, const uml::instruction &inst);
	void op_muls(x86code *&dst, const uml::instruction &inst);
	void op_divu(x86code *&dst, const uml::instruction &inst);
	void op_divs(x86code *&dst, const uml::instruction &inst);
	void op_and(x86code *&dst, const uml::instruction &inst);
	void op_test(x86code *&dst, const uml::instruction &inst);
	void op_or(x86code *&dst, const uml::instruction &inst);
	void op_xor(x86code *&dst, const uml::instruction &inst);
	void op_lzcnt(x86code *&dst, const uml::instruction &inst);
	void op_bswap(x86code *&dst, const uml::instruction &inst);
	void op_shl(x86code *&dst, const uml::instruction &inst);
	void op_shr(x86code *&dst, const uml::instruction &inst);
	void op_sar(x86code *&dst, const uml::instruction &inst);
	void op_ror(x86code *&dst, const uml::instruction &inst);
	void op_rol(x86code *&dst, const uml::instruction &inst);
	void op_rorc(x86code *&dst, const uml::instruction &inst);
	void op_rolc(x86code *&dst, const uml::instruction &inst);

	void op_fload(x86code *&dst, const uml::instruction &inst);
	void op_fstore(x86code *&dst, const uml::instruction &inst);
	void op_fread(x86code *&dst, const uml::instruction &inst);
	void op_fwrite(x86code *&dst, const uml::instruction &inst);
	void op_fmov(x86code *&dst, const uml::instruction &inst);
	void op_ftoint(x86code *&dst, const uml::instruction &inst);
	void op_ffrint(x86code *&dst, const uml::instruction &inst);
	void op_ffrflt(x86code *&dst, const uml::instruction &inst);
	void op_frnds(x86code *&dst, const uml::instruction &inst);
	void op_fadd(x86code *&dst, const uml::instruction &inst);
	void op_fsub(x86code *&dst, const uml::instruction &inst);
	void op_fcmp(x86code *&dst, const uml::instruction &inst);
	void op_fmul(x86code *&dst, const uml::instruction &inst);
	void op_fdiv(x86code *&dst, const uml::instruction &inst);
	void op_fneg(x86code *&dst, const uml::instruction &inst);
	void op_fabs(x86code *&dst, const uml::instruction &inst);
	void op_fsqrt(x86code *&dst, const uml::instruction &inst);
	void op_frecip(x86code *&dst, const uml::instruction &inst);
	void op_frsqrt(x86code *&dst, const uml::instruction &inst);
	void op_fcopyi(x86code *&dst, const uml::instruction &inst);
	void op_icopyf(x86code *&dst, const uml::instruction &inst);

	// 32-bit code emission helpers
	void emit_mov_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param);
	void emit_mov_r32_p32_keepflags(x86code *&dst, UINT8 reg, const be_parameter &param);
	void emit_mov_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param);
	void emit_mov_p32_r32(x86code *&dst, const be_parameter &param, UINT8 reg);
	void emit_add_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_add_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_adc_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_adc_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_sub_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_sub_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_sbb_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_sbb_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_cmp_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_cmp_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_and_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_and_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_test_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_test_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_or_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_or_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_xor_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_xor_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_shl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_shl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_shr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_shr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_sar_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_sar_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_rol_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_rol_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_ror_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_ror_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_rcl_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_rcl_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_rcr_r32_p32(x86code *&dst, UINT8 reg, const be_parameter &param, const uml::instruction &inst);
	void emit_rcr_m32_p32(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);

	// 64-bit code emission helpers
	void emit_mov_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param);
	void emit_mov_r64_p64_keepflags(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param);
	void emit_mov_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param);
	void emit_mov_p64_r64(x86code *&dst, const be_parameter &param, UINT8 reglo, UINT8 reghi);
	void emit_add_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_add_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_adc_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_adc_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_sub_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_sub_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_sbb_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_sbb_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_cmp_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_and_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_and_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_test_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_test_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_or_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_or_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_xor_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_xor_m64_p64(x86code *&dst, x86_memref memref, const be_parameter &param, const uml::instruction &inst);
	void emit_shl_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_shr_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_sar_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_rol_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_ror_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_rcl_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);
	void emit_rcr_r64_p64(x86code *&dst, UINT8 reglo, UINT8 reghi, const be_parameter &param, const uml::instruction &inst);

	// floating-point code emission helpers
	void emit_fld_p(x86code *&dst, int size, const be_parameter &param);
	void emit_fstp_p(x86code *&dst, int size, const be_parameter &param);

	// callback helpers
	static int dmulu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2, int flags);
	static int dmuls(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2, int flags);
	static int ddivu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2);
	static int ddivs(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	drc_label_list          m_labels;               // label list
	x86log_context *        m_log;                  // logging
	bool                    m_logged_common;        // logged common code already?
	bool                    m_sse3;                 // do we have SSE3 support?

	x86_entry_point_func    m_entry;                // entry point
	x86code *               m_exit;                 // exit point
	x86code *               m_nocode;               // nocode handler
	x86code *               m_save;                 // save handler
	x86code *               m_restore;              // restore handler

	UINT32 *                m_reglo[x86emit::REG_MAX];// pointer to low part of data for each register
	UINT32 *                m_reghi[x86emit::REG_MAX];// pointer to high part of data for each register
	UINT8                   m_last_lower_reg;       // last register we stored a lower from
	x86code *               m_last_lower_pc;        // PC after instruction where we last stored a lower register
	UINT32 *                m_last_lower_addr;      // address where we last stored an lower register
	UINT8                   m_last_upper_reg;       // last register we stored an upper from
	x86code *               m_last_upper_pc;        // PC after instruction where we last stored an upper register
	UINT32 *                m_last_upper_addr;      // address where we last stored an upper register
	double                  m_fptemp;               // temporary storage for floating point

	UINT16                  m_fpumode;              // saved FPU mode
	UINT16                  m_fmodesave;            // temporary location for saving

	void *                  m_stacksave;            // saved stack pointer
	void *                  m_hashstacksave;        // saved stack pointer for hashjmp
	UINT64                  m_reslo;                // extended low result
	UINT64                  m_reshi;                // extended high result

	drc_label_fixup_delegate m_fixup_label;         // precomputed delegate for fixups
	drc_oob_delegate        m_fixup_exception;      // precomputed delegate for exception fixups

	// globals
	typedef void (drcbe_x86::*opcode_generate_func)(x86code *&dst, const uml::instruction &inst);
	struct opcode_table_entry
	{
		uml::opcode_t           opcode;             // opcode in question
		opcode_generate_func    func;               // function pointer to the work
	};
	static const opcode_table_entry s_opcode_table_source[];
	static opcode_generate_func s_opcode_table[uml::OP_MAX];
};


#endif /* __DRCBEC_H__ */
