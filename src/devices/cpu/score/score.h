// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Sunplus Technology S+core

**********************************************************************/

#pragma once

#ifndef __SCORE_H__
#define __SCORE_H__


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
	score7_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 64; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	// helpers
	bool check_condition_branch(UINT8 bc);
	bool check_condition(UINT8 bc);
	INT32 sign_extend(UINT32 data, UINT8 len);
	UINT32 fetch();
	UINT8 read_byte(offs_t offset);
	UINT16 read_word(offs_t offset);
	UINT32 read_dword(offs_t offset);
	void write_byte(offs_t offset, UINT8 data);
	void write_word(offs_t offset, UINT16 data);
	void write_dword(offs_t offset, UINT32 data);
	void check_irq();
	void gen_exception(int cause, UINT32 param = 0);

	offs_t disasm(char *buffer, offs_t pc, UINT32 opcode);
	char *disasm32(char *buffer, offs_t pc, UINT32 opcode);
	char *disasm16(char *buffer, offs_t pc, UINT16 opcode);
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

private:
	address_space_config m_program_config;
	address_space *     m_program;
	direct_read_data *  m_direct;

	// internal state
	int                 m_icount;
	UINT32              m_pc;
	UINT32              m_ppc;
	UINT32              m_op;
	UINT32              m_gpr[0x20];
	UINT32              m_cr[0x20];
	UINT32              m_sr[3];
	UINT32              m_ce[2];
	bool                m_pending_interrupt[64];

	// opcodes tables
	typedef void (score7_cpu_device::*op_handler)();
	static const op_handler s_opcode32_table[4*8];
	static const op_handler s_opcode16_table[8];

	// mnemonics
	static const char *const m_cond[16];
	static const char *const m_tcs[4];
	static const char *const m_rix1_op[8];
	static const char *const m_rix2_op[8];
	static const char *const m_r2_op[16];
	static const char *const m_i1_op[8];
	static const char *const m_i2_op[8];
	static const char *const m_ls_op[8];
	static const char *const m_i1a_op[8];
	static const char *const m_i1b_op[8];
	static const char *const m_cr_op[2];
};

extern const device_type SCORE7;

#endif /* __SCORE_H__ */
