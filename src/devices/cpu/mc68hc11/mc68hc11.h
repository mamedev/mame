// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, hap
#pragma once

#ifndef __MC68HC11_H__
#define __MC68HC11_H__


#define MC68HC11_IO_PORTA           0x00
#define MC68HC11_IO_PORTB           0x01
#define MC68HC11_IO_PORTC           0x02
#define MC68HC11_IO_PORTD           0x03
#define MC68HC11_IO_PORTE           0x04
#define MC68HC11_IO_PORTF           0x05
#define MC68HC11_IO_PORTG           0x06
#define MC68HC11_IO_PORTH           0x07
#define MC68HC11_IO_SPI1_DATA       0x08
#define MC68HC11_IO_SPI2_DATA       0x09
#define MC68HC11_IO_AD0             0x10
#define MC68HC11_IO_AD1             0x11
#define MC68HC11_IO_AD2             0x12
#define MC68HC11_IO_AD3             0x13
#define MC68HC11_IO_AD4             0x14
#define MC68HC11_IO_AD5             0x15
#define MC68HC11_IO_AD6             0x16
#define MC68HC11_IO_AD7             0x17

#define MC68HC11_IRQ_LINE           0
#define MC68HC11_TOC1_LINE          1


extern const device_type MC68HC11;


#define MCFG_MC68HC11_CONFIG(_has_extended_io, _internal_ram_size, _init_value) \
	mc68hc11_cpu_device::set_has_extended_io(*device, _has_extended_io); \
	mc68hc11_cpu_device::set_internal_ram_size(*device, _internal_ram_size); \
	mc68hc11_cpu_device::set_init_value(*device, _init_value);


class mc68hc11_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mc68hc11_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O enable flag
	static void set_has_extended_io(device_t &device, int has_extended_io) { downcast<mc68hc11_cpu_device &>(device).m_has_extended_io = has_extended_io; }
	static void set_internal_ram_size(device_t &device, int internal_ram_size) { downcast<mc68hc11_cpu_device &>(device).m_internal_ram_size = internal_ram_size; }
	// default value for INIT register
	static void set_init_value(device_t &device, int init_value) { downcast<mc68hc11_cpu_device &>(device).m_init_value = init_value; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 41; }
	virtual UINT32 execute_input_lines() const override { return 2; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr );
	}

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 5; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	union {
		struct {
#ifdef LSB_FIRST
			UINT8 b;
			UINT8 a;
#else
			UINT8 a;
			UINT8 b;
#endif
		} d8;
		UINT16 d16;
	} m_d;

	UINT16 m_ix;
	UINT16 m_iy;
	UINT16 m_sp;
	UINT16 m_pc;
	UINT16 m_ppc;
	UINT8 m_ccr;

	UINT8 m_adctl;
	int m_ad_channel;

	UINT8 m_irq_state[2];
	direct_read_data *m_direct;
	address_space *m_program;
	address_space *m_io;
	int m_icount;

	int m_ram_position;
	int m_reg_position;
	dynamic_buffer m_internal_ram;

	int m_has_extended_io; // extended I/O enable flag
	int m_internal_ram_size;
	int m_init_value;

	UINT8 m_wait_state;
	UINT8 m_stop_state;

	UINT8 m_tflg1;
	UINT8 m_tmsk1;
	UINT16 m_toc1;
	UINT16 m_tcnt;
//  UINT8 m_por;
	UINT8 m_pr;

	UINT64 m_frc_base;

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

	UINT8 hc11_regs_r(UINT32 address);
	void hc11_regs_w(UINT32 address, UINT8 value);
	UINT8 FETCH();
	UINT16 FETCH16();
	UINT8 READ8(UINT32 address);
	void WRITE8(UINT32 address, UINT8 value);
	UINT16 READ16(UINT32 address);
	void WRITE16(UINT32 address, UINT16 value);
	void CYCLES(int cycles);
	void SET_PC(int pc);
	void PUSH8(UINT8 value);
	void PUSH16(UINT16 value);
	UINT8 POP8();
	UINT16 POP16();
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
	void hc11_bcs();
	void hc11_beq();
	void hc11_bhi();
	void hc11_bne();
	void hc11_ble();
	void hc11_bls();
	void hc11_bmi();
	void hc11_bpl();
	void hc11_bra();
	void hc11_brclr_dir();
	void hc11_brclr_indx();
	void hc11_brset_dir();
	void hc11_brset_indx();
	void hc11_brn();
	void hc11_bset_dir();
	void hc11_bset_indx();
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
	void hc11_deca();
	void hc11_decb();
	void hc11_dec_ext();
	void hc11_dec_indx();
	void hc11_dec_indy();
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
	void hc11_idiv();
	void hc11_inca();
	void hc11_incb();
	void hc11_inc_ext();
	void hc11_inc_indx();
	void hc11_inc_indy();
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
	void hc11_rti();
	void hc11_rts();
	void hc11_sba();
	void hc11_sbca_imm();
	void hc11_sbca_indx();
	void hc11_sbca_indy();
	void hc11_sbcb_imm();
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


#endif /* __MC68HC11_H__ */
