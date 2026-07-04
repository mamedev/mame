// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC6554

***************************************************************************/

#ifndef MAME_CPU_LC6500_LC6554_H
#define MAME_CPU_LC6500_LC6554_H

#pragma once

enum
{
	LC6554_PC = 0,
	LC6554_AC,
	LC6554_E,
	LC6554_DP,
	LC6554_CTL,
	LC6554_TM
};

class lc6554_cpu_device : public cpu_device
{
public:
	lc6554_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto pa_in_cb() { return m_port_in_cb[0].bind(); }
	auto pb_in_cb() { return m_port_in_cb[1].bind(); }
	auto pc_in_cb() { return m_port_in_cb[2].bind(); }
	auto pc_out_cb() { return m_port_out_cb[2].bind(); }
	auto pd_in_cb() { return m_port_in_cb[3].bind(); }
	auto pd_out_cb() { return m_port_out_cb[3].bind(); }
	auto pe_in_cb() { return m_port_in_cb[4].bind(); }
	auto pe_out_cb() { return m_port_out_cb[4].bind(); }
	auto pf_in_cb() { return m_port_in_cb[5].bind(); }
	auto pf_out_cb() { return m_port_out_cb[5].bind(); }
	auto pg_in_cb() { return m_port_in_cb[6].bind(); }
	auto pg_out_cb() { return m_port_out_cb[6].bind(); }
	auto pi_in_cb() { return m_port_in_cb[8].bind(); }
	auto pi_out_cb() { return m_port_out_cb[8].bind(); }
	auto pj_in_cb() { return m_port_in_cb[9].bind(); }
	auto pj_out_cb() { return m_port_out_cb[9].bind(); }
	auto pk_out_cb() { return m_port_out_cb[10].bind(); }
	auto pl_out_cb() { return m_port_out_cb[11].bind(); }
	auto pm_out_cb() { return m_port_out_cb[12].bind(); }
	auto pn_out_cb() { return m_port_out_cb[13].bind(); }
	auto po_out_cb() { return m_port_out_cb[14].bind(); }
	auto pp_out_cb() { return m_port_out_cb[15].bind(); } // 1-bit

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum
	{
		VECTOR_TIMER = 0x38,
		VECTOR_EXT = 0x3a
	};

	enum
	{
		FLAG_C = 0x01,
		FLAG_Z = 0x02,
		FLAG_TMF = 0x04,
		FLAG_EXTF = 0x08
	};

	// exact location in ram unknown, values guessed
	enum
	{
		RAM_OFFSET_F = 0xf0, // flag bits
		RAM_OFFSET_A = 0xf4, // working register A0 to A3
		RAM_OFFSET_H = 0xf8, // working register H0 to H1
		RAM_OFFSET_L = 0xfa  // working register L0 to L1
	};

	enum
	{
		CTL_TIMER_INT_ENABLE = 0x01
		// other bits unknown
	};

	void lc6554_program_map(address_map &map) ATTR_COLD;
	void lc6554_data_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_update);

	// helpers
	void ram_w(offs_t offset, uint8_t data);
	uint8_t ram_r(offs_t offset);

	void push_pc(uint16_t pc);
	void pop_pc();

	void set_cf(uint8_t data);
	void set_zf(uint8_t data);
	void set_cf_and_zf(uint8_t data);

	// opcodes
	void op_cla();
	void op_clc();
	void op_stc();
	void op_cma();
	void op_inc();
	void op_dec();
	void op_ral();
	void op_tae();
	void op_xae();
	void op_inm();
	void op_dem();
	void op_smb();
	void op_rmb();
	void op_ad();
	void op_adc();
	void op_daa();
	void op_das();
	void op_exl();
	void op_and();
	void op_or();
	void op_cm();
	void op_tam();
	void op_ci(uint8_t arg);
	void op_cli(uint8_t arg);
	void op_li();
	void op_s();
	void op_l();
	void op_xm();
	void op_x();
	void op_xi();
	void op_xd();
	void op_rtbl();
	void op_ldz();
	void op_lhi();
	void op_ind();
	void op_ded();
	void op_tal();
	void op_tla();
	void op_xah();
	void op_xa();
	void op_xh();
	void op_xl();
	void op_sfb();
	void op_rfb();
	void op_jmp();
	void op_jpea();
	void op_czp();
	void op_cal();
	void op_rt();
	void op_rti();
	void op_bank();
	void op_sb();
	void op_ba();
	void op_bna();
	void op_bm();
	void op_bnm();
	void op_bp();
	void op_bnp();
	void op_btm();
	void op_bntm();
	void op_bi();
	void op_bni();
	void op_bc();
	void op_bnc();
	void op_bz();
	void op_bnz();
	void op_bf();
	void op_bnf();
	void op_ip();
	void op_op();
	void op_spb();
	void op_rpb();
	void op_sctl(uint8_t arg);
	void op_rctl(uint8_t arg);
	void op_wttm();
	void op_halt();
	void op_nop();

	void op_2c(); // decodes into ci/cli/sctl/rctl
	void op_illegal(); // illegal instructions

	typedef void (lc6554_cpu_device::*op_handler)();

	struct opcode_info
	{
		op_handler func;
		unsigned bytes;
		unsigned cycles;
	};

	static const opcode_info m_opcode_table[256];

	address_space_config m_program_config;
	address_space_config m_data_config;

	memory_access<12, 0, 0, ENDIANNESS_BIG>::cache m_program_cache;
	memory_access<12, 0, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access<8, 0, 0, ENDIANNESS_BIG>::specific m_data;

	emu_timer *m_timer;

	int m_icount;
	uint8_t m_opcode[2]; // currently executing opcode bytes
	bool m_bank;

	uint16_t m_pc; // program counter, 12-bit
	uint16_t m_ppc; // previous pc
	uint8_t m_status; // flags
	uint8_t m_status_save; // cf and zf saved on interrupt
	uint8_t m_ac; // accumulator, 4-bit
	uint8_t m_e; // e register, 4-bit
	uint8_t m_dp; // data pointer, 8-bit (dph and dpl)
	uint8_t m_ctl; // control register, 4-bit
	uint8_t m_tm; // timer, 8-bit

	uint16_t m_stack[8];
	uint8_t m_sp;

	// pseudo-port, mapping and amount unknown
	uint8_t m_gp[16];
	bool m_gp_access;

	// io callbacks
	devcb_read8::array<16> m_port_in_cb;
	devcb_write8::array<16> m_port_out_cb;
};

DECLARE_DEVICE_TYPE(LC6554, lc6554_cpu_device)

#endif // MAME_CPU_LC6500_LC6554_H
