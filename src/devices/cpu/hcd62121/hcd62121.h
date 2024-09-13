// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_CPU_HCD62121_HCD62121_H
#define MAME_CPU_HCD62121_HCD62121_H

#pragma once

class hcd62121_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	hcd62121_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto kol_cb() { return m_kol_cb.bind(); }
	auto koh_cb() { return m_koh_cb.bind(); }
	auto port_cb() { return m_port_cb.bind(); }
	auto opt_cb() { return m_opt_cb.bind(); }
	auto ki_cb() { return m_ki_cb.bind(); }
	auto in0_cb() { return m_in0_cb.bind(); }
	auto input_flag_cb() { return m_input_flag_cb.bind(); }

protected:
	enum
	{
		HCD62121_IP=1, HCD62121_SP, HCD62121_F, HCD62121_LAR,
		HCD62121_CS, HCD62121_DS, HCD62121_SS, HCD62121_DSIZE,
		/* 128 byte register file */
		HCD62121_R00, HCD62121_R04, HCD62121_R08, HCD62121_R0C,
		HCD62121_R10, HCD62121_R14, HCD62121_R18, HCD62121_R1C,
		HCD62121_R20, HCD62121_R24, HCD62121_R28, HCD62121_R2C,
		HCD62121_R30, HCD62121_R34, HCD62121_R38, HCD62121_R3C,
		HCD62121_R40, HCD62121_R44, HCD62121_R48, HCD62121_R4C,
		HCD62121_R50, HCD62121_R54, HCD62121_R58, HCD62121_R5C,
		HCD62121_R60, HCD62121_R64, HCD62121_R68, HCD62121_R6C,
		HCD62121_R70, HCD62121_R74, HCD62121_R78, HCD62121_R7C
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 48; }
	virtual u32 execute_input_lines() const noexcept override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	TIMER_CALLBACK_MEMBER(timer_tick);
	u8 read_op();
	u8 datasize(u8 op);
	void read_reg(int size, u8 op1);
	void write_reg(int size, u8 op1);
	void read_ireg(int size, u8 op1);
	void write_ireg(int size, u8 op1);
	void read_regreg(int size, u8 op1, u8 op2, bool copy_extend_immediate);
	void write_regreg(int size, u8 op1, u8 op2);
	void read_iregreg(int size, u8 op1, u8 op2, bool copy_extend_immediate);
	void write_iregreg(int size, u8 op1, u8 op2);
	void write_iregreg2(int size, u8 op1, u8 op2);
	bool check_cond(u8 op);
	void set_zero_flag(bool is_zero);
	void set_carry_flag(bool is_carry);
	void set_zl_flag(bool is_zl);
	void set_zh_flag(bool is_zh);
	void set_cl_flag(bool is_cl);
	void set_input_flag(bool is_input_set);
	void op_msk(int size);
	void op_and(int size);
	void op_or(int size);
	void op_xor(int size);
	void op_add(int size);
	void op_addb(int size);
	void op_subb(int size);
	void op_sub(int size);
	void op_pushw(u16 source);
	u16 op_popw();

	address_space_config m_program_config;

	u32 m_prev_pc;
	u16 m_sp;
	u16 m_ip;
	u8 m_dsize;
	u8 m_cseg;
	u8 m_dseg;
	u8 m_sseg;
	u8 m_f;
	u8 m_time;
	u8 m_time_op;
	s32 m_cycles_until_timeout;
	bool m_is_timer_started;
	bool m_is_infinite_timeout;
	emu_timer *m_timer;
	u16 m_lar;
	u8 m_reg[0x80];

	// OPT7 - OPT0 output pins (pins 65-72)
	u8 m_opt;

	// PORT7 - PORT0 I/O pins (pins 73-80)
	u8 m_port;

	u8 m_temp1[0x10];
	u8 m_temp2[0x10];
	u32 m_rtemp;

	u32 m_debugger_temp;

	address_space *m_program;

	int m_icount;

	devcb_write8 m_kol_cb;
	devcb_write8 m_koh_cb;
	devcb_write8 m_port_cb;
	devcb_write8 m_opt_cb;
	devcb_read8 m_ki_cb;
	devcb_read8 m_in0_cb;
	devcb_read8 m_input_flag_cb;
};


DECLARE_DEVICE_TYPE(HCD62121, hcd62121_cpu_device)

#endif // MAME_CPU_HCD62121_HCD62121_H
