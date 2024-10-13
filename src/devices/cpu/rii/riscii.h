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
		RII_PFS,
		RII_INTCON,
		RII_INTSTA,
		RII_POST_ID,
		RII_PORTB, RII_PORTC, RII_PORTD, RII_PORTE, RII_PORTF, RII_PORTG, RII_PORTH, RII_PORTI, RII_PORTJ, RII_PORTK,
		RII_STBCON,
		RII_PAINTEN, RII_PAINTSTA, RII_PAWAKE,
		RII_DCRB, RII_DCRC, RII_DCRDE, RII_DCRFG, RII_DCRHI, RII_DCRJK,
		RII_PBCON, RII_PCCON,
		RII_UARTCON, RII_UARTSTA,
		RII_SPICON, RII_SPISTA,
		RII_TRL0, RII_TRL1, RII_TRL2,
		RII_TR01CON,
		RII_TR2CON,
		RII_TRLIR,
		RII_SFCR,
		RII_ADD1, RII_ADD2, RII_ADD3, RII_ADD4,
		RII_ENV1, RII_ENV2, RII_ENV3, RII_ENV4,
		RII_MTCON1, RII_MTCON2, RII_MTCON3, RII_MTCON4,
		RII_MTRL1, RII_MTRL2, RII_MTRL3, RII_MTRL4,
		RII_SPHDR,
		RII_SPHTCON,
		RII_SPHTRL,
		RII_VOCON
	};

	enum
	{
		PA0_LINE = 0,
		PA1_LINE,
		PA2_LINE,
		PA3_LINE,
		PA4_LINE,
		PA5_LINE,
		PA6_LINE,
		PA7_LINE
	};

	// callback configuration
	auto in_porta_cb() { return m_porta_in_cb.bind(); }
	auto in_portb_cb() { return m_port_in_cb[0].bind(); }
	auto out_portb_cb() { return m_port_out_cb[0].bind(); }
	auto in_portc_cb() { return m_port_in_cb[1].bind(); }
	auto out_portc_cb() { return m_port_out_cb[1].bind(); }
	auto in_portd_cb() { return m_port_in_cb[2].bind(); }
	auto out_portd_cb() { return m_port_out_cb[2].bind(); }
	auto in_porte_cb() { return m_port_in_cb[3].bind(); }
	auto out_porte_cb() { return m_port_out_cb[3].bind(); }
	auto in_portf_cb() { return m_port_in_cb[4].bind(); }
	auto out_portf_cb() { return m_port_out_cb[4].bind(); }
	auto in_portg_cb() { return m_port_in_cb[5].bind(); }
	auto out_portg_cb() { return m_port_out_cb[5].bind(); }
	auto in_porth_cb() { return m_port_in_cb[6].bind(); }
	auto out_porth_cb() { return m_port_out_cb[6].bind(); }
	auto in_porti_cb() { return m_port_in_cb[7].bind(); }
	auto out_porti_cb() { return m_port_out_cb[7].bind(); }
	auto in_portj_cb() { return m_port_in_cb[8].bind(); }
	auto out_portj_cb() { return m_port_out_cb[8].bind(); }
	auto in_portk_cb() { return m_port_in_cb[9].bind(); }
	auto out_portk_cb() { return m_port_out_cb[9].bind(); }

protected:
	riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned addrbits, unsigned pcbits, unsigned bankbits, u8 maxbank, u8 post_id_mask, address_map_constructor regs);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	void core_regs_map(address_map &map) ATTR_COLD;

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
	u8 pfs_r();
	void pfs_w(u8 data);
	u8 intcon_r();
	void intcon_w(u8 data);
	u8 intsta_r();
	void intsta_w(u8 data);
	u8 post_id_r();
	void post_id_w(u8 data);
	u8 porta_r();
	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);
	u8 stbcon_r();
	void stbcon_w(u8 data);
	u8 painten_r();
	void painten_w(u8 data);
	u8 paintsta_r();
	void paintsta_w(u8 data);
	u8 pawake_r();
	void pawake_w(u8 data);
	u8 portjk_r(offs_t offset);
	void portjk_w(offs_t offset, u8 data);
	u8 dcr_r(offs_t offset);
	void dcr_w(offs_t offset, u8 data);
	u8 pcon_r(offs_t offset);
	void pcon_w(offs_t offset, u8 data);
	void uarttx_w(u8 data);
	u8 uartrx_r();
	u8 uartcon_r();
	void uartcon_w(u8 data);
	u8 uartsta_r();
	void uartsta_w(u8 data);
	u8 spicon_r();
	void spicon_w(u8 data);
	u8 spista_r();
	void spista_w(u8 data);
	u8 sprl_r();
	void sprl_w(u8 data);
	u8 sprm_r();
	void sprm_w(u8 data);
	u8 sprh_r();
	void sprh_w(u8 data);
	void timer0_reload();
	TIMER_CALLBACK_MEMBER(timer0);
	u16 timer0_count() const;
	void timer1_reload();
	TIMER_CALLBACK_MEMBER(timer1);
	u8 timer1_count() const;
	void timer2_reload();
	TIMER_CALLBACK_MEMBER(timer2);
	u8 timer2_count() const;
	u8 trl0l_r();
	void trl0l_w(u8 data);
	u8 trl0h_r();
	void trl0h_w(u8 data);
	u8 trl1_r();
	void trl1_w(u8 data);
	u8 trl2_r();
	void trl2_w(u8 data);
	u8 tr01con_r();
	void tr01con_w(u8 data);
	u8 tr2con_r();
	void tr2con_w(u8 data);
	u8 trlir_r();
	void trlir_w(u8 data);
	u8 t0cl_r();
	u8 t0ch_r();
	u8 tr1c_r();
	u8 tr2c_r();
	u8 sfcr_r();
	void sfcr_w(u8 data);
	void spht_reload();
	TIMER_CALLBACK_MEMBER(speech_timer);
	u8 addl_r();
	void addl_w(u8 data);
	u8 addm_r();
	void addm_w(u8 data);
	u8 addh_r();
	void addh_w(u8 data);
	u8 env_sphdr_r();
	void env_sphdr_w(u8 data);
	u8 mtcon_sphtcon_r();
	void mtcon_sphtcon_w(u8 data);
	u8 mtrl_sphtrl_r();
	void mtrl_sphtrl_w(u8 data);
	u8 vocon_r();
	void vocon_w(u8 data);

private:
	// debugging helpers
	void debug_set_pc(u32 pc);

	// memory helpers
	u16 fetch_program_word();
	u16 get_banked_address(u8 reg);
	u32 tabptr_offset(int offset) const;
	void multi_byte_carry(u16 addr, bool cy);
	void multi_byte_borrow(u16 addr, bool cy);

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
	void idle_wakeup();

	// interrupt helpers
	bool interrupt_active() const;
	u32 vector_interrupt() const;

	enum exec_state : u8 {
		EXEC_CYCLE1,
		EXEC_ADCPCM, EXEC_SBCPCM,
		EXEC_TBRD0, EXEC_TBRD1, EXEC_TBRD2, EXEC_TBRDA,
		EXEC_NOJMP,
		EXEC_L0JMP, EXEC_L1JMP, EXEC_L2JMP, EXEC_L3JMP,
		EXEC_L4JMP, EXEC_L5JMP, EXEC_L6JMP, EXEC_L7JMP,
		EXEC_L8JMP, EXEC_L9JMP, EXEC_LAJMP, EXEC_LBJMP,
		EXEC_LCJMP, EXEC_LDJMP, EXEC_LEJMP, EXEC_LFJMP,
		EXEC_L0CALL, EXEC_L1CALL, EXEC_L2CALL, EXEC_L3CALL,
		EXEC_L4CALL, EXEC_L5CALL, EXEC_L6CALL, EXEC_L7CALL,
		EXEC_L8CALL, EXEC_L9CALL, EXEC_LACALL, EXEC_LBCALL,
		EXEC_LCCALL, EXEC_LDCALL, EXEC_LECALL, EXEC_LFCALL,
		EXEC_IDLE
	};

	// address spaces
	address_space_config m_program_config;
	address_space_config m_regs_config;
	memory_access<22, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<22, 1, -1, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<13, 0,  0, ENDIANNESS_LITTLE>::specific m_regs;

	// device callbacks
	devcb_read8 m_porta_in_cb;
	devcb_read8::array<10> m_port_in_cb;
	devcb_write8::array<10> m_port_out_cb;

	// model-specific parameters
	const u32 m_pcmask;
	const u32 m_tbptmask;
	const u8 m_bankmask;
	const u8 m_maxbank;
	const u8 m_post_id_mask;

	// internal state
	u32 m_pc;
	u32 m_ppc;
	u8 m_acc;
	u8 m_fsr[2];
	u8 m_bsr[2];
	u32 m_tabptr;
	u8 m_stkptr;
	std::unique_ptr<u8[]> m_pchstack;
	u8 m_cpucon;
	u8 m_status;
	u16 m_prod;
	u8 m_pfs;
	u8 m_intcon;
	u8 m_intsta;
	u8 m_post_id;

	// port state
	u8 m_port_data[10];
	u8 m_port_dcr[6];
	u8 m_port_control[2];
	u8 m_stbcon;
	u8 m_pa;
	u8 m_painten;
	u8 m_paintsta;
	u8 m_pawake;

	// UART state
	u8 m_uartcon;
	u8 m_uartsta;

	// SPI state
	u8 m_spicon;
	u8 m_spista;

	// timer state
	u16 m_trl0;
	u8 m_trl1;
	u8 m_trl2;
	u8 m_tr01con;
	u8 m_tr2con;
	u8 m_trlir;
	emu_timer *m_timer[3];

	// synthesizer state
	u8 m_sfcr;
	u32 m_add[4];
	u8 m_env[4];
	u8 m_mtcon[4];
	u8 m_mtrl[4];
	u8 m_sphdr;
	u8 m_sphtcon;
	u8 m_sphtrl;
	u8 m_vocon;
	emu_timer *m_speech_timer;

	// execution sequencing
	s32 m_icount;
	exec_state m_exec_state;
	u8 m_repeat;
	u8 m_curreg;
};

class epg3231_device : public riscii_series_device
{
public:
	// construction/destruction
	epg3231_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void regs_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(EPG3231, epg3231_device)

#endif // MAME_CPU_RII_RISCII_H
