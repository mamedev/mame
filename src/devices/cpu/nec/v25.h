// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
/* ASG 971222 -- rewrote this interface */
#ifndef MAME_CPU_NEC_V25_H
#define MAME_CPU_NEC_V25_H

#pragma once

#include "necdasm.h"

#define NEC_INPUT_LINE_INTP0 10
#define NEC_INPUT_LINE_INTP1 11
#define NEC_INPUT_LINE_INTP2 12
#define NEC_INPUT_LINE_POLL 20

enum
{
	V25_PC=0,
	V25_AW, V25_CW, V25_DW, V25_BW, V25_SP, V25_BP, V25_IX, V25_IY,
	V25_DS1, V25_PS, V25_SS, V25_DS0,
	V25_AL, V25_AH, V25_CL, V25_CH, V25_DL, V25_DH, V25_BL, V25_BH,
	V25_PSW,
	V25_IDB,
	V25_PENDING
};

class v25_common_device : public cpu_device, public nec_disassembler::config
{
public:
	// configuration helpers
	void set_decryption_table(const uint8_t *decryption_table) { m_v25v35_decryptiontable = decryption_table; }

	auto pt_in_cb() { return m_pt_in.bind(); }
	auto p0_in_cb() { return m_p0_in.bind(); }
	auto p1_in_cb() { return m_p1_in.bind(); }
	auto p2_in_cb() { return m_p2_in.bind(); }

	auto p0_out_cb() { return m_p0_out.bind(); }
	auto p1_out_cb() { return m_p1_out.bind(); }
	auto p2_out_cb() { return m_p2_out.bind(); }

	TIMER_CALLBACK_MEMBER(v25_timer_callback);

protected:
	// construction/destruction
	v25_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_16bit, uint8_t prefetch_size, uint8_t prefetch_cycles, uint32_t chip_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { notify_clock_changed(); }

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks / m_PCK; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles * m_PCK; }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 80; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI || (inputnum >= NEC_INPUT_LINE_INTP0 && inputnum <= NEC_INPUT_LINE_INTP2); }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual int get_mode() const override { return 1; }

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_cache8;
	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::cache m_cache16;

	/* internal RAM and register banks */
	required_shared_ptr<uint16_t> m_internal_ram;

	uint16_t  m_ip;
	uint16_t  m_prev_ip;

	/* PSW flags */
	int32_t   m_SignVal;
	uint32_t  m_AuxVal, m_OverVal, m_ZeroVal, m_CarryVal, m_ParityVal;  /* 0 or non-0 valued flags */
	uint8_t   m_IBRK, m_F0, m_F1, m_TF, m_IF, m_DF, m_MF;   /* 0 or 1 valued flags */
	uint8_t   m_RBW, m_RBB;   /* current register bank base, preshifted for word and byte registers */

	/* interrupt related */
	uint32_t  m_pending_irq;
	uint32_t  m_unmasked_irq;
	uint32_t  m_macro_service;
	uint32_t  m_bankswitch_irq;
	uint8_t   m_priority_inttu, m_priority_intd, m_priority_intp, m_priority_ints0, m_priority_ints1;
	uint8_t   m_ems[3];
	uint8_t   m_srms[2];
	uint8_t   m_stms[2];
	uint8_t   m_tmms[3];
	uint8_t   m_IRQS, m_ISPR;
	uint32_t  m_nmi_state;
	uint32_t  m_irq_state;
	uint32_t  m_poll_state;
	uint32_t  m_mode_state;
	uint32_t  m_intp_state[3];
	uint8_t   m_intm;
	uint8_t   m_no_interrupt;
	uint8_t   m_halted;

	/* timer related */
	uint16_t  m_TM0, m_MD0, m_TM1, m_MD1;
	uint8_t   m_TMC0, m_TMC1;
	emu_timer *m_timers[4];

	/* system control */
	uint8_t   m_RAMEN, m_TB, m_PCK; /* PRC register */
	uint8_t   m_RFM;
	uint16_t  m_WTC;
	uint32_t  m_IDB;

	address_space *m_program;
	std::function<u8 (offs_t address)> m_dr8;
	memory_access<9, 1, 0, ENDIANNESS_LITTLE>::specific m_data;
	address_space *m_io;
	int     m_icount;

	/* callbacks */
	devcb_read8 m_pt_in;
	devcb_read8 m_p0_in;
	devcb_read8 m_p1_in;
	devcb_read8 m_p2_in;

	devcb_write8 m_p0_out;
	devcb_write8 m_p1_out;
	devcb_write8 m_p2_out;

	uint8_t   m_prefetch_size;
	uint8_t   m_prefetch_cycles;
	int8_t    m_prefetch_count;
	uint8_t   m_prefetch_reset;
	uint32_t  m_chip_type;

	uint32_t  m_prefix_base;    /* base address of the latest prefix segment */
	uint8_t   m_seg_prefix;     /* prefix segment indicator */

	uint32_t m_EA;
	uint16_t m_EO;
	uint16_t m_E16;

	uint32_t m_debugger_temp;

	const uint8_t *m_v25v35_decryptiontable;  // internal decryption table

	typedef void (v25_common_device::*nec_ophandler)();
	typedef uint32_t (v25_common_device::*nec_eahandler)();
	static const nec_ophandler s_nec_instruction[256];
	static const nec_eahandler s_GetEA[192];

	inline void prefetch();
	void do_prefetch(int previous_ICount);
	inline uint8_t fetch();
	inline uint16_t fetchword();
	inline uint8_t fetchop();
	void nec_interrupt(unsigned int_num, int /*INTSOURCES*/ source);
	void nec_bankswitch(unsigned bank_num);
	void nec_trap();
	void external_int();

	void ida_sfr_map(address_map &map) ATTR_COLD;
	uint8_t read_irqcontrol(int /*INTSOURCES*/ source, uint8_t priority);
	void write_irqcontrol(int /*INTSOURCES*/ source, uint8_t d);
	uint8_t p0_r();
	void p0_w(uint8_t d);
	void pm0_w(uint8_t d);
	void pmc0_w(uint8_t d);
	uint8_t p1_r();
	void p1_w(uint8_t d);
	void pm1_w(uint8_t d);
	void pmc1_w(uint8_t d);
	uint8_t p2_r();
	void p2_w(uint8_t d);
	void pm2_w(uint8_t d);
	void pmc2_w(uint8_t d);
	uint8_t pt_r();
	void pmt_w(uint8_t d);
	uint8_t intm_r();
	void intm_w(uint8_t d);
	uint8_t ems_r(offs_t a);
	void ems_w(offs_t a, uint8_t d);
	uint8_t exic0_r();
	void exic0_w(uint8_t d);
	uint8_t exic1_r();
	void exic1_w(uint8_t d);
	uint8_t exic2_r();
	void exic2_w(uint8_t d);
	uint8_t srms0_r();
	void srms0_w(uint8_t d);
	uint8_t stms0_r();
	void stms0_w(uint8_t d);
	uint8_t seic0_r();
	void seic0_w(uint8_t d);
	uint8_t sric0_r();
	void sric0_w(uint8_t d);
	uint8_t stic0_r();
	void stic0_w(uint8_t d);
	uint8_t srms1_r();
	void srms1_w(uint8_t d);
	uint8_t stms1_r();
	void stms1_w(uint8_t d);
	uint8_t seic1_r();
	void seic1_w(uint8_t d);
	uint8_t sric1_r();
	void sric1_w(uint8_t d);
	uint8_t stic1_r();
	void stic1_w(uint8_t d);
	uint16_t tm0_r();
	void tm0_w(uint16_t d);
	uint16_t md0_r();
	void md0_w(uint16_t d);
	uint16_t tm1_r();
	void tm1_w(uint16_t d);
	uint16_t md1_r();
	void md1_w(uint16_t d);
	void tmc0_w(uint8_t d);
	void tmc1_w(uint8_t d);
	uint8_t tmms_r(offs_t a);
	void tmms_w(offs_t a, uint8_t d);
	uint8_t tmic0_r();
	void tmic0_w(uint8_t d);
	uint8_t tmic1_r();
	void tmic1_w(uint8_t d);
	uint8_t tmic2_r();
	void tmic2_w(uint8_t d);
	uint8_t rfm_r();
	void rfm_w(uint8_t d);
	uint16_t wtc_r();
	void wtc_w(offs_t a, uint16_t d, uint16_t m);
	uint8_t flag_r();
	void flag_w(uint8_t d);
	uint8_t prc_r();
	void prc_w(uint8_t d);
	uint8_t tbic_r();
	void tbic_w(uint8_t d);
	uint8_t irqs_r();
	uint8_t ispr_r();
	uint8_t idb_r();
	void idb_w(uint8_t d);
	uint8_t v25_read_byte(unsigned a);
	uint16_t v25_read_word(unsigned a);
	void v25_write_byte(unsigned a, uint8_t d);
	void v25_write_word(unsigned a, uint16_t d);

	void i_add_br8();
	void i_add_wr16();
	void i_add_r8b();
	void i_add_r16w();
	void i_add_ald8();
	void i_add_axd16();
	void i_push_es();
	void i_pop_es();
	void i_or_br8();
	void i_or_r8b();
	void i_or_wr16();
	void i_or_r16w();
	void i_or_ald8();
	void i_or_axd16();
	void i_push_cs();
	void i_pre_nec();
	void i_pre_v25();
	void i_adc_br8();
	void i_adc_wr16();
	void i_adc_r8b();
	void i_adc_r16w();
	void i_adc_ald8();
	void i_adc_axd16();
	void i_push_ss();
	void i_pop_ss();
	void i_sbb_br8();
	void i_sbb_wr16();
	void i_sbb_r8b();
	void i_sbb_r16w();
	void i_sbb_ald8();
	void i_sbb_axd16();
	void i_push_ds();
	void i_pop_ds();
	void i_and_br8();
	void i_and_r8b();
	void i_and_wr16();
	void i_and_r16w();
	void i_and_ald8();
	void i_and_axd16();
	void i_es();
	void i_daa();
	void i_sub_br8();
	void i_sub_wr16();
	void i_sub_r8b();
	void i_sub_r16w();
	void i_sub_ald8();
	void i_sub_axd16();
	void i_cs();
	void i_das();
	void i_xor_br8();
	void i_xor_r8b();
	void i_xor_wr16();
	void i_xor_r16w();
	void i_xor_ald8();
	void i_xor_axd16();
	void i_ss();
	void i_aaa();
	void i_cmp_br8();
	void i_cmp_wr16();
	void i_cmp_r8b();
	void i_cmp_r16w();
	void i_cmp_ald8();
	void i_cmp_axd16();
	void i_ds();
	void i_aas();
	void i_inc_ax();
	void i_inc_cx();
	void i_inc_dx();
	void i_inc_bx();
	void i_inc_sp();
	void i_inc_bp();
	void i_inc_si();
	void i_inc_di();
	void i_dec_ax();
	void i_dec_cx();
	void i_dec_dx();
	void i_dec_bx();
	void i_dec_sp();
	void i_dec_bp();
	void i_dec_si();
	void i_dec_di();
	void i_push_ax();
	void i_push_cx();
	void i_push_dx();
	void i_push_bx();
	void i_push_sp();
	void i_push_bp();
	void i_push_si();
	void i_push_di();
	void i_pop_ax();
	void i_pop_cx();
	void i_pop_dx();
	void i_pop_bx();
	void i_pop_sp();
	void i_pop_bp();
	void i_pop_si();
	void i_pop_di();
	void i_pusha();
	void i_popa();
	void i_chkind();
	void i_repnc();
	void i_repc();
	void i_push_d16();
	void i_imul_d16();
	void i_push_d8();
	void i_imul_d8();
	void i_insb();
	void i_insw();
	void i_outsb();
	void i_outsw();
	void i_jo();
	void i_jno();
	void i_jc();
	void i_jnc();
	void i_jz();
	void i_jnz();
	void i_jce();
	void i_jnce();
	void i_js();
	void i_jns();
	void i_jp();
	void i_jnp();
	void i_jl();
	void i_jnl();
	void i_jle();
	void i_jnle();
	void i_80pre();
	void i_82pre();
	void i_81pre();
	void i_83pre();
	void i_test_br8();
	void i_test_wr16();
	void i_xchg_br8();
	void i_xchg_wr16();
	void i_mov_br8();
	void i_mov_r8b();
	void i_mov_wr16();
	void i_mov_r16w();
	void i_mov_wsreg();
	void i_lea();
	void i_mov_sregw();
	void i_invalid();
	void i_popw();
	void i_nop();
	void i_xchg_axcx();
	void i_xchg_axdx();
	void i_xchg_axbx();
	void i_xchg_axsp();
	void i_xchg_axbp();
	void i_xchg_axsi();
	void i_xchg_axdi();
	void i_cbw();
	void i_cwd();
	void i_call_far();
	void i_pushf();
	void i_popf();
	void i_sahf();
	void i_lahf();
	void i_mov_aldisp();
	void i_mov_axdisp();
	void i_mov_dispal();
	void i_mov_dispax();
	void i_movsb();
	void i_movsw();
	void i_cmpsb();
	void i_cmpsw();
	void i_test_ald8();
	void i_test_axd16();
	void i_stosb();
	void i_stosw();
	void i_lodsb();
	void i_lodsw();
	void i_scasb();
	void i_scasw();
	void i_mov_ald8();
	void i_mov_cld8();
	void i_mov_dld8();
	void i_mov_bld8();
	void i_mov_ahd8();
	void i_mov_chd8();
	void i_mov_dhd8();
	void i_mov_bhd8();
	void i_mov_axd16();
	void i_mov_cxd16();
	void i_mov_dxd16();
	void i_mov_bxd16();
	void i_mov_spd16();
	void i_mov_bpd16();
	void i_mov_sid16();
	void i_mov_did16();
	void i_rotshft_bd8();
	void i_rotshft_wd8();
	void i_ret_d16();
	void i_ret();
	void i_les_dw();
	void i_lds_dw();
	void i_mov_bd8();
	void i_mov_wd16();
	void i_enter();
	void i_leave();
	void i_retf_d16();
	void i_retf();
	void i_int3();
	void i_int();
	void i_into();
	void i_iret();
	void i_rotshft_b();
	void i_rotshft_w();
	void i_rotshft_bcl();
	void i_rotshft_wcl();
	void i_aam();
	void i_aad();
	void i_setalc();
	void i_trans();
	void i_fpo();
	void i_loopne();
	void i_loope();
	void i_loop();
	void i_jcxz();
	void i_inal();
	void i_inax();
	void i_outal();
	void i_outax();
	void i_call_d16();
	void i_jmp_d16();
	void i_jmp_far();
	void i_jmp_d8();
	void i_inaldx();
	void i_inaxdx();
	void i_outdxal();
	void i_outdxax();
	void i_lock();
	void i_repne();
	void i_repe();
	void i_hlt();
	void i_cmc();
	void i_f6pre();
	void i_f7pre();
	void i_clc();
	void i_stc();
	void i_di();
	void i_ei();
	void i_cld();
	void i_std();
	void i_fepre();
	void i_ffpre();
	void i_wait();
	void i_brkn();
	void i_brks();

	uint32_t EA_000();
	uint32_t EA_001();
	uint32_t EA_002();
	uint32_t EA_003();
	uint32_t EA_004();
	uint32_t EA_005();
	uint32_t EA_006();
	uint32_t EA_007();
	uint32_t EA_100();
	uint32_t EA_101();
	uint32_t EA_102();
	uint32_t EA_103();
	uint32_t EA_104();
	uint32_t EA_105();
	uint32_t EA_106();
	uint32_t EA_107();
	uint32_t EA_200();
	uint32_t EA_201();
	uint32_t EA_202();
	uint32_t EA_203();
	uint32_t EA_204();
	uint32_t EA_205();
	uint32_t EA_206();
	uint32_t EA_207();
};


class v25_device : public v25_common_device
{
public:
	v25_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class v35_device : public v25_common_device
{
public:
	v35_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(V25, v25_device)
DECLARE_DEVICE_TYPE(V35, v35_device)


#endif // MAME_CPU_NEC_V25_H
