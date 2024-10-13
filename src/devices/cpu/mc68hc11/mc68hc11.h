// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, AJR
#ifndef MAME_CPU_MC68HC11_MC68HC11_H
#define MAME_CPU_MC68HC11_MC68HC11_H

#pragma once


enum {
	MC68HC11_IRQ_LINE           = 0,
	MC68HC11_XIRQ_LINE          = 1
};

DECLARE_DEVICE_TYPE(MC68HC11A1, mc68hc11a1_device)
DECLARE_DEVICE_TYPE(MC68HC11D0, mc68hc11d0_device)
DECLARE_DEVICE_TYPE(MC68HC11E1, mc68hc11e1_device)
DECLARE_DEVICE_TYPE(MC68HC811E2, mc68hc811e2_device)
DECLARE_DEVICE_TYPE(MC68HC11F1, mc68hc11f1_device)
DECLARE_DEVICE_TYPE(MC68HC11K1, mc68hc11k1_device)
DECLARE_DEVICE_TYPE(MC68HC11M0, mc68hc11m0_device)

class mc68hc11_cpu_device : public cpu_device, public device_nvram_interface
{
public:
	// port configuration
	auto in_pa_callback() { return m_port_input_cb[0].bind(); }
	auto in_pb_callback() { return m_port_input_cb[1].bind(); }
	auto in_pc_callback() { return m_port_input_cb[2].bind(); }
	auto in_pd_callback() { return m_port_input_cb[3].bind(); }
	auto in_pe_callback() { return m_port_input_cb[4].bind(); }
	auto in_pf_callback() { return m_port_input_cb[5].bind(); }
	auto in_pg_callback() { return m_port_input_cb[6].bind(); }
	auto in_ph_callback() { return m_port_input_cb[7].bind(); }
	auto out_pa_callback() { return m_port_output_cb[0].bind(); }
	auto out_pb_callback() { return m_port_output_cb[1].bind(); }
	auto out_pc_callback() { return m_port_output_cb[2].bind(); }
	auto out_pd_callback() { return m_port_output_cb[3].bind(); }
	auto out_pe_callback() { return m_port_output_cb[4].bind(); }
	auto out_pf_callback() { return m_port_output_cb[5].bind(); }
	auto out_pg_callback() { return m_port_output_cb[6].bind(); }
	auto out_ph_callback() { return m_port_output_cb[7].bind(); }
	auto in_an0_callback() { return m_analog_cb[0].bind(); }
	auto in_an1_callback() { return m_analog_cb[1].bind(); }
	auto in_an2_callback() { return m_analog_cb[2].bind(); }
	auto in_an3_callback() { return m_analog_cb[3].bind(); }
	auto in_an4_callback() { return m_analog_cb[4].bind(); }
	auto in_an5_callback() { return m_analog_cb[5].bind(); }
	auto in_an6_callback() { return m_analog_cb[6].bind(); }
	auto in_an7_callback() { return m_analog_cb[7].bind(); }
	auto in_spi2_data_callback() { return m_spi2_data_input_cb.bind(); }
	auto out_spi2_data_callback() { return m_spi2_data_output_cb.bind(); }

	void set_default_config(uint8_t data) { assert(!configured()); m_config = data & m_config_mask; }

protected:
	mc68hc11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t ram_size, uint16_t reg_block_size, uint16_t rom_size, uint16_t eeprom_size, uint8_t init_value, uint8_t config_mask, uint8_t option_mask);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 41; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_nvram_interface implementation
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void set_irq_state(uint8_t irqn, bool state);

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) = 0;

	template <int P> uint8_t port_r();
	template <int P> void port_w(uint8_t data);
	template <int P> uint8_t ddr_r();
	template <int P> void ddr_w(uint8_t data);
	uint8_t pioc_r();
	uint8_t tcnt_r(offs_t offset);
	void tcnt_w(offs_t offset, uint8_t data);
	uint8_t toc_r(offs_t offset);
	void toc_w(offs_t offset, uint8_t data);
	uint8_t tctl1_r();
	void tctl1_w(uint8_t data);
	uint8_t tctl2_r();
	void tctl2_w(uint8_t data);
	uint8_t tmsk1_r();
	void tmsk1_w(uint8_t data);
	uint8_t tflg1_r();
	void tflg1_w(uint8_t data);
	uint8_t tflg2_r();
	void tflg2_w(uint8_t data);
	uint8_t tmsk2_r();
	void tmsk2_w(uint8_t data);
	uint8_t pactl_r();
	void pactl_w(uint8_t data);
	uint8_t pactl_ddra_r();
	void pactl_ddra_w(uint8_t data);
	template <int N> uint8_t spcr_r();
	template <int N> uint8_t spsr_r();
	template <int N> uint8_t spdr_r();
	template <int N> void spdr_w(uint8_t data);
	uint8_t adctl_r();
	void adctl_w(uint8_t data);
	uint8_t adr_r(offs_t offset);
	uint8_t opt2_r();
	uint8_t config_r();
	uint8_t config_1s_r();
	void config_w(uint8_t data);
	uint8_t init_r();
	void init_w(uint8_t data);
	uint8_t init2_r();
	void init2_w(uint8_t data);
	uint8_t option_r();
	void option_w(uint8_t data);
	uint8_t scbd_r(offs_t offset);
	uint8_t sccr1_r();
	uint8_t sccr2_r();
	uint8_t scsr1_r();
	uint8_t scrdl_r();
	uint8_t opt4_r();

private:
	address_space_config m_program_config;

	union {
		struct {
#ifdef LSB_FIRST
			uint8_t b;
			uint8_t a;
#else
			uint8_t a;
			uint8_t b;
#endif
		} d8;
		uint16_t d16;
	} m_d;

	uint16_t m_ix;
	uint16_t m_iy;
	uint16_t m_sp;
	uint16_t m_pc;
	uint16_t m_ppc;
	uint8_t m_ccr;

protected:
	uint8_t m_port_data[8];
private:
	uint8_t m_port_dir[8];

	uint8_t m_adctl;
	int m_ad_channel;

	uint32_t m_irq_state;
	bool m_irq_asserted;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
	devcb_read8::array<8> m_port_input_cb;
	devcb_write8::array<8> m_port_output_cb;
	devcb_read8::array<8> m_analog_cb;
	devcb_read8 m_spi2_data_input_cb;
	devcb_write8 m_spi2_data_output_cb;
	int m_icount;

	memory_view m_ram_view;
	memory_view m_reg_view;
	memory_view m_eeprom_view;
	optional_shared_ptr<uint8_t> m_eeprom_data;

	const uint16_t m_internal_ram_size;
	const uint16_t m_reg_block_size; // size of internal I/O space
	const uint16_t m_internal_rom_size;
	const uint16_t m_internal_eeprom_size;
	const uint8_t m_init_value; // default value for INIT register
	const uint8_t m_config_mask;
	const uint8_t m_option_mask;

	uint8_t m_wait_state;
	uint8_t m_stop_state;

	uint8_t m_tctl1;
	uint8_t m_tctl2;
	uint8_t m_tflg1;
	uint8_t m_tmsk1;
	uint16_t m_toc[5];
	uint16_t m_tcnt;
//  uint8_t m_por;
	uint8_t m_tflg2;
	uint8_t m_tmsk2;
	uint8_t m_pactl;
	uint8_t m_init;
	uint8_t m_init2;

protected:
	uint8_t m_config;
	uint8_t m_option;

private:
	uint64_t m_frc_base;
	uint64_t m_reset_time;

	typedef void (mc68hc11_cpu_device::*ophandler)();
	struct hc11_opcode_list_struct
	{
		int page;
		int opcode;
		ophandler handler;
	};
	static const hc11_opcode_list_struct hc11_opcode_list[];

	ophandler hc11_optable[256];
	ophandler hc11_optable_page2[256];
	ophandler hc11_optable_page3[256];
	ophandler hc11_optable_page4[256];

	void internal_map(address_map &map) ATTR_COLD;

	uint8_t FETCH();
	uint16_t FETCH16();
	uint8_t READ8(uint32_t address);
	void WRITE8(uint32_t address, uint8_t value);
	uint16_t READ16(uint32_t address);
	void WRITE16(uint32_t address, uint16_t value);
	void CYCLES(int cycles);
	void SET_PC(int pc);
	void PUSH8(uint8_t value);
	void PUSH16(uint16_t value);
	uint8_t POP8();
	uint16_t POP16();
	void hc11_aba();
	void hc11_abx();
	void hc11_aby();
	void hc11_adca_imm();
	void hc11_adca_dir();
	void hc11_adca_ext();
	void hc11_adca_indx();
	void hc11_adca_indy();
	void hc11_adcb_imm();
	void hc11_adcb_dir();
	void hc11_adcb_ext();
	void hc11_adcb_indx();
	void hc11_adcb_indy();
	void hc11_adda_imm();
	void hc11_adda_dir();
	void hc11_adda_ext();
	void hc11_adda_indx();
	void hc11_adda_indy();
	void hc11_addb_imm();
	void hc11_addb_dir();
	void hc11_addb_ext();
	void hc11_addb_indx();
	void hc11_addb_indy();
	void hc11_addd_imm();
	void hc11_addd_dir();
	void hc11_addd_ext();
	void hc11_addd_indx();
	void hc11_addd_indy();
	void hc11_anda_imm();
	void hc11_anda_dir();
	void hc11_anda_ext();
	void hc11_anda_indx();
	void hc11_anda_indy();
	void hc11_andb_imm();
	void hc11_andb_dir();
	void hc11_andb_ext();
	void hc11_andb_indx();
	void hc11_andb_indy();
	void hc11_asla();
	void hc11_aslb();
	void hc11_asl_ext();
	void hc11_asl_indx();
	void hc11_asl_indy();
	void hc11_asra();
	void hc11_asrb();
	void hc11_asr_ext();
	void hc11_asr_indx();
	void hc11_asr_indy();
	void hc11_bita_imm();
	void hc11_bita_dir();
	void hc11_bita_ext();
	void hc11_bita_indx();
	void hc11_bita_indy();
	void hc11_bitb_imm();
	void hc11_bitb_dir();
	void hc11_bitb_ext();
	void hc11_bitb_indx();
	void hc11_bitb_indy();
	void hc11_bcc();
	void hc11_bclr_dir();
	void hc11_bclr_indx();
	void hc11_bclr_indy();
	void hc11_bcs();
	void hc11_beq();
	void hc11_bge();
	void hc11_bgt();
	void hc11_bhi();
	void hc11_bne();
	void hc11_ble();
	void hc11_bls();
	void hc11_blt();
	void hc11_bmi();
	void hc11_bpl();
	void hc11_bra();
	void hc11_brclr_dir();
	void hc11_brclr_indx();
	void hc11_brclr_indy();
	void hc11_brset_dir();
	void hc11_brset_indx();
	void hc11_brset_indy();
	void hc11_brn();
	void hc11_bset_dir();
	void hc11_bset_indx();
	void hc11_bset_indy();
	void hc11_bsr();
	void hc11_bvc();
	void hc11_bvs();
	void hc11_cba();
	void hc11_clc();
	void hc11_cli();
	void hc11_clra();
	void hc11_clrb();
	void hc11_clr_ext();
	void hc11_clr_indx();
	void hc11_clr_indy();
	void hc11_clv();
	void hc11_cmpa_imm();
	void hc11_cmpa_dir();
	void hc11_cmpa_ext();
	void hc11_cmpa_indx();
	void hc11_cmpa_indy();
	void hc11_cmpb_imm();
	void hc11_cmpb_dir();
	void hc11_cmpb_ext();
	void hc11_cmpb_indx();
	void hc11_cmpb_indy();
	void hc11_com_ext();
	void hc11_com_indx();
	void hc11_com_indy();
	void hc11_coma();
	void hc11_comb();
	void hc11_cpd_imm();
	void hc11_cpd_dir();
	void hc11_cpd_ext();
	void hc11_cpd_indx();
	void hc11_cpd_indy();
	void hc11_cpx_imm();
	void hc11_cpx_dir();
	void hc11_cpx_ext();
	void hc11_cpx_indx();
	void hc11_cpx_indy();
	void hc11_cpy_imm();
	void hc11_cpy_dir();
	void hc11_cpy_ext();
	void hc11_cpy_indx();
	void hc11_cpy_indy();
	void hc11_daa();
	void hc11_deca();
	void hc11_decb();
	void hc11_dec_ext();
	void hc11_dec_indx();
	void hc11_dec_indy();
	void hc11_des();
	void hc11_dex();
	void hc11_dey();
	void hc11_eora_imm();
	void hc11_eora_dir();
	void hc11_eora_ext();
	void hc11_eora_indx();
	void hc11_eora_indy();
	void hc11_eorb_imm();
	void hc11_eorb_dir();
	void hc11_eorb_ext();
	void hc11_eorb_indx();
	void hc11_eorb_indy();
	void hc11_fdiv();
	void hc11_idiv();
	void hc11_inca();
	void hc11_incb();
	void hc11_inc_ext();
	void hc11_inc_indx();
	void hc11_inc_indy();
	void hc11_ins();
	void hc11_inx();
	void hc11_iny();
	void hc11_jmp_indx();
	void hc11_jmp_indy();
	void hc11_jmp_ext();
	void hc11_jsr_dir();
	void hc11_jsr_ext();
	void hc11_jsr_indx();
	void hc11_jsr_indy();
	void hc11_ldaa_imm();
	void hc11_ldaa_dir();
	void hc11_ldaa_ext();
	void hc11_ldaa_indx();
	void hc11_ldaa_indy();
	void hc11_ldab_imm();
	void hc11_ldab_dir();
	void hc11_ldab_ext();
	void hc11_ldab_indx();
	void hc11_ldab_indy();
	void hc11_ldd_imm();
	void hc11_ldd_dir();
	void hc11_ldd_ext();
	void hc11_ldd_indx();
	void hc11_ldd_indy();
	void hc11_lds_imm();
	void hc11_lds_dir();
	void hc11_lds_ext();
	void hc11_lds_indx();
	void hc11_lds_indy();
	void hc11_ldx_imm();
	void hc11_ldx_dir();
	void hc11_ldx_ext();
	void hc11_ldx_indx();
	void hc11_ldx_indy();
	void hc11_ldy_imm();
	void hc11_ldy_dir();
	void hc11_ldy_ext();
	void hc11_ldy_indx();
	void hc11_ldy_indy();
	void hc11_lsld();
	void hc11_lsra();
	void hc11_lsrb();
	void hc11_lsr_ext();
	void hc11_lsr_indx();
	void hc11_lsr_indy();
	void hc11_lsrd();
	void hc11_mul();
	void hc11_nega();
	void hc11_negb();
	void hc11_neg_ext();
	void hc11_neg_indx();
	void hc11_neg_indy();
	void hc11_nop();
	void hc11_psha();
	void hc11_oraa_imm();
	void hc11_oraa_dir();
	void hc11_oraa_ext();
	void hc11_oraa_indx();
	void hc11_oraa_indy();
	void hc11_orab_imm();
	void hc11_orab_dir();
	void hc11_orab_ext();
	void hc11_orab_indx();
	void hc11_orab_indy();
	void hc11_pshb();
	void hc11_pshx();
	void hc11_pshy();
	void hc11_pula();
	void hc11_pulb();
	void hc11_pulx();
	void hc11_puly();
	void hc11_rola();
	void hc11_rolb();
	void hc11_rol_ext();
	void hc11_rol_indx();
	void hc11_rol_indy();
	void hc11_rora();
	void hc11_rorb();
	void hc11_ror_ext();
	void hc11_ror_indx();
	void hc11_ror_indy();
	void hc11_rti();
	void hc11_rts();
	void hc11_sba();
	void hc11_sbca_imm();
	void hc11_sbca_dir();
	void hc11_sbca_ext();
	void hc11_sbca_indx();
	void hc11_sbca_indy();
	void hc11_sbcb_imm();
	void hc11_sbcb_dir();
	void hc11_sbcb_ext();
	void hc11_sbcb_indx();
	void hc11_sbcb_indy();
	void hc11_sec();
	void hc11_sei();
	void hc11_sev();
	void hc11_staa_dir();
	void hc11_staa_ext();
	void hc11_staa_indx();
	void hc11_staa_indy();
	void hc11_stab_dir();
	void hc11_stab_ext();
	void hc11_stab_indx();
	void hc11_stab_indy();
	void hc11_std_dir();
	void hc11_std_ext();
	void hc11_std_indx();
	void hc11_std_indy();
	void hc11_sts_dir();
	void hc11_sts_ext();
	void hc11_sts_indx();
	void hc11_sts_indy();
	void hc11_stx_dir();
	void hc11_stx_ext();
	void hc11_stx_indx();
	void hc11_stx_indy();
	void hc11_sty_dir();
	void hc11_sty_ext();
	void hc11_sty_indx();
	void hc11_sty_indy();
	void hc11_stop();
	void hc11_suba_imm();
	void hc11_suba_dir();
	void hc11_suba_ext();
	void hc11_suba_indx();
	void hc11_suba_indy();
	void hc11_subb_imm();
	void hc11_subb_dir();
	void hc11_subb_ext();
	void hc11_subb_indx();
	void hc11_subb_indy();
	void hc11_subd_imm();
	void hc11_subd_dir();
	void hc11_subd_ext();
	void hc11_subd_indx();
	void hc11_subd_indy();
	void hc11_swi();
	void hc11_tab();
	void hc11_tap();
	void hc11_tba();
	void hc11_test();
	void hc11_tpa();
	void hc11_tsta();
	void hc11_tstb();
	void hc11_tst_ext();
	void hc11_tst_indx();
	void hc11_tst_indy();
	void hc11_tsx();
	void hc11_tsy();
	void hc11_txs();
	void hc11_tys();
	void hc11_wai();
	void hc11_xgdx();
	void hc11_xgdy();
	void hc11_page2();
	void hc11_page3();
	void hc11_page4();
	void hc11_invalid();
	void check_irq_lines();
};

class mc68hc11a1_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11a1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;

	uint8_t pactl_ddra7_r();
	void pactl_ddra7_w(uint8_t data);
};

class mc68hc11d0_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11d0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;

private:
	uint8_t reg01_r();
};

class mc68hc11e1_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11e1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;
};

class mc68hc811e2_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc811e2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;
};

class mc68hc11f1_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11f1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;
};

class mc68hc11k1_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11k1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;
};

class mc68hc11m0_device : public mc68hc11_cpu_device
{
public:
	// construction/destruction
	mc68hc11m0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void mc68hc11_reg_map(memory_view::memory_view_entry &block, offs_t base) override;
};

#endif // MAME_CPU_MC68HC11_MC68HC11_H
