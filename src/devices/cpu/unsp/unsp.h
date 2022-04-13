// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus µ'nSP emulator

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

    Ported to MAME framework by Ryan Holtz

*****************************************************************************/

#ifndef MAME_CPU_UNSP_UNSP_H
#define MAME_CPU_UNSP_UNSP_H

#pragma once

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"
#include "unspdefs.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC               M0
#define MAPVAR_CYCLES           M1

#define SINGLE_INSTRUCTION_MODE (0)

#define ENABLE_UNSP_DRC         (0)

#define UNSP_LOG_OPCODES        (0)
#define UNSP_LOG_REGS           (0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class unsp_frontend;

// ======================> unsp_device

enum
{
	UNSP_SP = 1,
	UNSP_R1,
	UNSP_R2,
	UNSP_R3,
	UNSP_R4,
	UNSP_BP,
	UNSP_SR,
	UNSP_PC,

	UNSP_GPR_COUNT = UNSP_PC,

	UNSP_IRQ_EN,
	UNSP_FIQ_EN,
	UNSP_FIR_MOV_EN,
	UNSP_SB,
	UNSP_AQ,
	UNSP_FRA,
	UNSP_BNK,
	UNSP_INE,
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	UNSP_PRI,
	UNSP_LOG_OPS
#else
	UNSP_PRI
#endif
};

enum
{
	UNSP_FIQ_LINE = 0,
	UNSP_IRQ0_LINE,
	UNSP_IRQ1_LINE,
	UNSP_IRQ2_LINE,
	UNSP_IRQ3_LINE,
	UNSP_IRQ4_LINE,
	UNSP_IRQ5_LINE,
	UNSP_IRQ6_LINE,
	UNSP_IRQ7_LINE,
	UNSP_BRK_LINE,

	UNSP_NUM_LINES
};

class unsp_device : public cpu_device
{
	friend class unsp_frontend;

public:
	// construction/destruction
	unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~unsp_device();

	void set_vectorbase(uint16_t vector) { m_vectorbase = vector; }

	uint8_t get_csb();

	void set_ds(uint16_t ds);
	uint16_t get_ds();

	void set_fr(uint16_t fr);
	uint16_t get_fr();

	inline void ccfunc_unimplemented();
	void invalidate_cache();

#if UNSP_LOG_REGS
	void log_regs();
	void log_write(uint32_t addr, uint32_t data);
	void cfunc_log_write();
#endif

	void cfunc_muls();

protected:
	unsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry& entry) override;
	virtual void state_export(const device_state_entry& entry) override;
	virtual void state_string_export(const device_state_entry& entry, std::string& str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// HACK: IRQ line state can only be modified directly by hardware on-board the SPG SoC itself.
	// Therefore, to avoid an unnecessary scheduler sync when the derived spg2xx_device sets or
	// clears an interrupt line, we provide this direct accessor.
	void set_state_unsynced(int inputnum, int state);

	enum : uint32_t
	{
		REG_SP = 0,
		REG_R1,
		REG_R2,
		REG_R3,
		REG_R4,
		REG_BP,
		REG_SR,
		REG_PC,

		REG_SR1 = 0,
		REG_SR2,
		REG_SR3,
		REG_SR4
	};

	/* internal compiler state */
	struct compiler_state
	{
		compiler_state(compiler_state const&) = delete;
		compiler_state& operator=(compiler_state const&) = delete;

		uint32_t m_cycles;          /* accumulated cycles */
		uml::code_label m_labelnum; /* index for local labels */
	};

	struct internal_unsp_state
	{
		uint32_t m_r[16]; // required to be 32 bits due to DRC
		uint32_t m_secbank[4];
		uint32_t m_enable_irq;
		uint32_t m_enable_fiq;
		uint32_t m_fir_move;
		uint32_t m_fiq;
		uint32_t m_irq;
		uint32_t m_sirq;
		uint32_t m_sb;
		uint32_t m_aq;
		uint32_t m_fra;
		uint32_t m_bnk;
		uint32_t m_ine;
		uint32_t m_pri;

		uint32_t m_divq_bit;
		uint32_t m_divq_dividend;
		uint32_t m_divq_divisor;
		uint32_t m_divq_a;

		uint32_t m_arg0;
		uint32_t m_arg1;
		uint32_t m_jmpdest;

		int m_icount;
	};

	/* core state */
	internal_unsp_state* m_core;

protected:
	uint16_t read16(uint32_t address) { return m_program.read_word(address); }

	void write16(uint32_t address, uint16_t data)
	{
	#if UNSP_LOG_REGS
		log_write(address, data);
	#endif
		m_program.write_word(address, data);
	}

	void add_lpc(const int32_t offset)
	{
		const uint32_t new_lpc = UNSP_LPC + offset;
		m_core->m_r[REG_PC] = (uint16_t)new_lpc;
		m_core->m_r[REG_SR] &= 0xffc0;
		m_core->m_r[REG_SR] |= (new_lpc >> 16) & 0x3f;
	}

	void execute_fxxx_000_group(uint16_t op);
	void execute_fxxx_001_group(uint16_t op);
	void execute_fxxx_010_group(uint16_t op);
	void execute_fxxx_011_group(uint16_t op);
	virtual void execute_fxxx_101_group(uint16_t op);
	void execute_fxxx_110_group(uint16_t op);
	void execute_fxxx_111_group(uint16_t op);
	void execute_fxxx_group(uint16_t op);
	void execute_fxxx_100_group(uint16_t op);
	virtual void execute_extended_group(uint16_t op);
	virtual void execute_exxx_group(uint16_t op);
	void execute_muls_ss(const uint16_t rd, const uint16_t rs, const uint16_t size);
	void unimplemented_opcode(uint16_t op);
	void unimplemented_opcode(uint16_t op, uint16_t ximm);
	void unimplemented_opcode(uint16_t op, uint16_t ximm, uint16_t ximm_2);
	virtual bool op_is_divq(const uint16_t op) { return false; }

	int m_iso;

	static char const *const regs[];
	static char const *const extregs[];
	static char const *const bitops[];
	static char const *const lsft[];
	static char const *const aluops[];
	static char const *const forms[];

	void push(uint32_t value, uint32_t *reg);
	uint16_t pop(uint32_t *reg);

	void update_nz(uint32_t value);
	void update_nzsc(uint32_t value, uint16_t r0, uint16_t r1);
	bool do_basic_alu_ops(const uint16_t& op0, uint32_t& lres, uint16_t& r0, uint16_t& r1, uint32_t& r2, bool update_flags);

private:
	// compilation boundaries -- how far back/forward does the analysis extend?
	enum : uint32_t
	{
		COMPILE_BACKWARDS_BYTES     = 128,
		COMPILE_FORWARDS_BYTES      = 512,
		COMPILE_MAX_INSTRUCTIONS    = (COMPILE_BACKWARDS_BYTES / 4) + (COMPILE_FORWARDS_BYTES / 4),
		COMPILE_MAX_SEQUENCE        = 64
	};

	// exit codes
	enum : int
	{
		EXECUTE_OUT_OF_CYCLES       = 0,
		EXECUTE_MISSING_CODE        = 1,
		EXECUTE_UNMAPPED_CODE       = 2,
		EXECUTE_RESET_CACHE         = 3
	};


	void execute_jumps(const uint16_t op);
	void execute_remaining(const uint16_t op);
	void execute_one(const uint16_t op);



	address_space_config m_program_config;
	memory_access<23, 1, -1, ENDIANNESS_BIG>::cache m_cache;
	memory_access<23, 1, -1, ENDIANNESS_BIG>::specific m_program;

	uint32_t m_debugger_temp;
#if UNSP_LOG_OPCODES || UNSP_LOG_REGS
	uint32_t m_log_ops;
#endif

	inline void trigger_fiq();
	inline void trigger_irq(int line);
	void check_irqs();

	drc_cache m_drccache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<unsp_frontend> m_drcfe;
	uint32_t m_drcoptions;
	uint8_t m_cache_dirty;

	uml::parameter   m_regmap[16];
	uml::code_handle *m_entry;
	uml::code_handle *m_nocode;
	uml::code_handle *m_out_of_cycles;
	uml::code_handle *m_check_interrupts;
	uml::code_handle *m_trigger_fiq;
	uml::code_handle *m_trigger_irq;

	uml::code_handle *m_mem_read;
	uml::code_handle *m_mem_write;

	bool m_enable_drc;
	uint16_t m_vectorbase;

	void execute_run_drc();
	void flush_drc_cache();
	void code_flush_cache();
	void code_compile_block(offs_t pc);
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);

	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_check_interrupts();
	void static_generate_trigger_fiq();
	void static_generate_trigger_irq();
	void static_generate_memory_accessor(bool iswrite, const char *name, uml::code_handle *&handleptr);

	void generate_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_check_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param);
	void generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_add_lpc(drcuml_block &block, int32_t offset);
	void generate_update_nzsc(drcuml_block &block);
	void generate_update_nz(drcuml_block &block);
	void log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

#if UNSP_LOG_REGS
	FILE *m_log_file;
#endif
protected:
	int m_numregs;
};


class unsp_11_device : public unsp_device
{
public:
	// construction/destruction
	unsp_11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	unsp_11_device(const machine_config& mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);

private:
};

class unsp_12_device : public unsp_11_device
{
public:
	// construction/destruction
	unsp_12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	unsp_12_device(const machine_config& mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);

	virtual void execute_fxxx_101_group(uint16_t op) override;
	virtual void execute_exxx_group(uint16_t op) override;
	void execute_divq(uint16_t op);
	bool op_is_divq(const uint16_t op) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class unsp_20_device : public unsp_12_device
{
public:
	// construction/destruction
	unsp_20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	unsp_20_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_extended_group(uint16_t op) override;

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		UNSP20_R8 = 0,
		UNSP20_R9,
		UNSP20_R10,
		UNSP20_R11,
		UNSP20_R12,
		UNSP20_R13,
		UNSP20_R14,
		UNSP20_R15
	};
};



DECLARE_DEVICE_TYPE(UNSP,    unsp_device)
DECLARE_DEVICE_TYPE(UNSP_11, unsp_11_device)
DECLARE_DEVICE_TYPE(UNSP_12, unsp_12_device)
DECLARE_DEVICE_TYPE(UNSP_20, unsp_20_device)


#endif // MAME_CPU_UNSP_UNSP_H
