// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Sunplus Technology S+core

**********************************************************************/

#ifndef MAME_CPU_SCORE_SCORE_H
#define MAME_CPU_SCORE_SCORE_H

#pragma once


//**************************************************************************
//  DEFINITION
//**************************************************************************

enum
{
	SCORE_PC = 1,
	SCORE_CEH,
	SCORE_CEL,
	SCORE_GPR,
	SCORE_CR = SCORE_GPR + 0x20,
	SCORE_SR = SCORE_CR + 0x20
};


// ======================> score7_cpu_device

class score7_cpu_device : public cpu_device
{
public:
	// construction/destruction
	score7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// helpers
	bool check_condition_branch(uint8_t bc);
	bool check_condition(uint8_t bc);
	uint32_t fetch();
	uint8_t read_byte(offs_t offset);
	uint16_t read_word(offs_t offset);
	uint32_t read_dword(offs_t offset);
	void write_byte(offs_t offset, uint8_t data);
	void write_word(offs_t offset, uint16_t data);
	void write_dword(offs_t offset, uint32_t data);
	void check_irq();
	void gen_exception(int cause, uint32_t param = 0);

	void unemulated_op(const char * op);

	// 32-bit opcodes
	void op_specialform();
	void op_iform1();
	void op_jump();
	void op_rixform1();
	void op_branch();
	void op_iform2();
	void op_crform();
	void op_rixform2();
	void op_addri();
	void op_andri();
	void op_orri();
	void op_lw();
	void op_lh();
	void op_lhu();
	void op_lb();
	void op_sw();
	void op_sh();
	void op_lbu();
	void op_sb();
	void op_cache();
	void op_cenew();
	void op_undef();

	// 16-bit opcodes
	void op_rform1();
	void op_rform2();
	void op_jform();
	void op_branch16();
	void op_ldiu();
	void op_iform1a();
	void op_iform1b();

	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	// internal state
	int                   m_icount;
	uint32_t              m_pc;
	uint32_t              m_ppc;
	uint32_t              m_op;
	uint32_t              m_gpr[0x20];
	uint32_t              m_cr[0x20];
	uint32_t              m_sr[3];
	uint32_t              m_ce[2];
	uint64_t              m_pending_interrupt;

	// opcodes tables
	typedef void (score7_cpu_device::*op_handler)();
	static const op_handler s_opcode32_table[4*8];
	static const op_handler s_opcode16_table[8];
};

DECLARE_DEVICE_TYPE(SCORE7, score7_cpu_device)

#endif // MAME_CPU_SCORE_SCORE_H
