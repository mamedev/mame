// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series

***************************************************************************/

#ifndef MAME_CPU_RII_RISCII_H
#define MAME_CPU_RII_RISCII_H

#pragma once



class riscii_series_device : public cpu_device
{
public:
	enum
	{
		RII_PC,
		RII_REPEAT,
		RII_ACC,
		RII_FSR0,
		RII_FSR1,
		RII_BSR,
		RII_BSR1,
		RII_TABPTR,
		RII_STKPTR,
		RII_CPUCON,
		RII_STATUS,
		RII_PROD,
		RII_POST_ID
	};

	// construction/destruction
	riscii_series_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned prgbits, unsigned bankbits, uint8_t maxbank);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// register handlers
	u8 fsr0_r();
	void fsr0_w(u8 data);
	u8 bsr_r();
	void bsr_w(u8 data);
	u8 fsr1_r();
	void fsr1_w(u8 data);
	u8 bsr1_r();
	void bsr1_w(u8 data);
	u8 pcl_r();
	void pcl_w(u8 data);
	u8 pcm_r();
	void pcm_w(u8 data);
	u8 pch_r();
	void pch_w(u8 data);
	u8 tabptrl_r();
	void tabptrl_w(u8 data);
	u8 tabptrm_r();
	void tabptrm_w(u8 data);
	u8 tabptrh_r();
	void tabptrh_w(u8 data);
	u8 acc_r();
	void acc_w(u8 data);
	u8 stkptr_r();
	void stkptr_w(u8 data);
	u8 cpucon_r();
	void cpucon_w(u8 data);
	u8 status_r();
	void status_w(u8 data);
	u8 prodl_r();
	void prodl_w(u8 data);
	u8 prodh_r();
	void prodh_w(u8 data);
	u8 post_id_r();
	void post_id_w(u8 data);

	// memory helpers
	u16 get_banked_address(u8 reg);
	u32 tabptr_offset(int offset) const;

	// execution
	void execute_move(u8 dstreg, u8 srcreg);
	void execute_add(u8 reg, bool a, bool c);
	void execute_sub(u8 reg, bool a, bool c);
	void execute_add_imm(u8 data, bool c);
	void execute_sub_imm(u8 data, bool c);
	void execute_adddc(u8 reg, bool a);
	void execute_subdb(u8 reg, bool a);
	void execute_mul(u8 reg);
	void execute_mul_imm(u8 data);
	void execute_or(u8 reg, bool a);
	void execute_and(u8 reg, bool a);
	void execute_xor(u8 reg, bool a);
	void execute_com(u8 reg, bool a);
	void execute_clr(u8 reg);
	void execute_rrc(u8 reg, bool a);
	void execute_rlc(u8 reg, bool a);
	void execute_shra(u8 reg);
	void execute_shla(u8 reg);
	void execute_jump(u32 addr);
	void execute_call(u32 addr);
	void execute_jcc(bool condition);
	void execute_jdnz(u8 reg, bool a);
	void execute_jinz(u8 reg, bool a);
	void set_z_acc(u8 tmp);
	void execute_load(u8 reg);
	void execute_store(u8 reg);
	void execute_test(u8 reg);
	void execute_swap(u8 reg, bool a);
	void execute_jbc(u8 reg, int b);
	void execute_jbs(u8 reg, int b);
	void execute_bc(u8 reg, int b);
	void execute_bs(u8 reg, int b);
	void execute_btg(u8 reg, int b);
	void execute_inc(u8 reg, bool a);
	void execute_dec(u8 reg, bool a);
	void execute_rpt(u8 reg);
	void execute_ret(bool inte);
	void execute_wdtc();
	void execute_slep();
	void execute_undef(u16 opcode);
	void execute_cycle1(u16 opcode);
	void execute_tbrd(u32 ptr);

	enum exec_state : u8 {
		EXEC_CYCLE1,
		EXEC_TBRD0, EXEC_TBRD1, EXEC_TBRD2, EXEC_TBRDA,
		EXEC_NOJMP,
		EXEC_L0JMP, EXEC_L1JMP, EXEC_L2JMP, EXEC_L3JMP,
		EXEC_L4JMP, EXEC_L5JMP, EXEC_L6JMP, EXEC_L7JMP,
		EXEC_L8JMP, EXEC_L9JMP, EXEC_LAJMP, EXEC_LBJMP,
		EXEC_LCJMP, EXEC_LDJMP, EXEC_LEJMP, EXEC_LFJMP,
		EXEC_L0CALL, EXEC_L1CALL, EXEC_L2CALL, EXEC_L3CALL,
		EXEC_L4CALL, EXEC_L5CALL, EXEC_L6CALL, EXEC_L7CALL,
		EXEC_L8CALL, EXEC_L9CALL, EXEC_LACALL, EXEC_LBCALL,
		EXEC_LCCALL, EXEC_LDCALL, EXEC_LECALL, EXEC_LFCALL
	};

	void regs_map(address_map &map);

	// address spaces
	address_space_config m_program_config;
	address_space_config m_regs_config;
	address_space *m_program;
	address_space *m_regs;
	memory_access_cache<1, -1, ENDIANNESS_LITTLE> *m_cache;

	// model-specific parameters
	const unsigned m_prgbits;
	const uint8_t m_bankmask;
	const uint8_t m_maxbank;

	// internal state
	u32 m_pc;
	u32 m_ppc;
	u8 m_acc;
	u8 m_fsr[2];
	u8 m_bsr[2];
	u32 m_tabptr;
	u8 m_stkptr;
	u8 m_cpucon;
	u8 m_status;
	u16 m_prod;
	u8 m_post_id;

	// execution sequencing
	s32 m_icount;
	exec_state m_exec_state;
	u8 m_repeat;
	u8 m_curreg;
};

DECLARE_DEVICE_TYPE(RISCII, riscii_series_device)

#endif // MAME_CPU_RII_RISCII_H
