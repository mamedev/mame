// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* ASG 971222 -- rewrote this interface */
#ifndef MAME_CPU_NEC_NEC_H
#define MAME_CPU_NEC_NEC_H

#pragma once


#define NEC_INPUT_LINE_POLL 20

enum
{
	NEC_PC=0,
	NEC_IP, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_XA,
	NEC_PENDING
};


class nec_common_device : public cpu_device
{
	friend class device_v5x_interface;

protected:
	// construction/destruction
	nec_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_16bit, uint8_t prefetch_size, uint8_t prefetch_cycles, uint32_t chip_type, address_map_constructor internal_port_map = address_map_constructor());

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 80; }
	virtual uint32_t execute_input_lines() const noexcept override { return 1; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u8 io_read_byte(offs_t a) { return m_io->read_byte(a); }
	virtual u16 io_read_word(offs_t a) { return m_io->read_word_unaligned(a); }
	virtual void io_write_byte(offs_t a, u8 v) { m_io->write_byte(a, v); }
	virtual void io_write_word(offs_t a, u16 v) { m_io->write_word_unaligned(a, v); }

	void set_int_line(int state);
	void set_nmi_line(int state);
	void set_poll_line(int state);

	address_space_config m_program_config;
	address_space_config m_io_config;

private:
	/* NEC registers */
	union necbasicregs
	{                   /* eight general registers */
		uint16_t w[8];    /* viewed as 16 bits registers */
		uint8_t  b[16];   /* or as 8 bit registers */
	};

	necbasicregs m_regs;
	uint16_t  m_sregs[4];

	uint16_t  m_ip;

	/* PSW flags */
	int32_t   m_SignVal;
	uint32_t  m_AuxVal;   /* 0 or non-0 valued flags */
	uint32_t  m_OverVal;
	uint32_t  m_ZeroVal;
	uint32_t  m_CarryVal;
	uint32_t  m_ParityVal;
	uint8_t   m_TF; /* 0 or 1 valued flags */
	uint8_t   m_IF;
	uint8_t   m_DF;
	uint8_t   m_MF;

	/* interrupt related */
	uint32_t  m_pending_irq;
	uint32_t  m_nmi_state;
	uint32_t  m_irq_state;
	uint32_t  m_poll_state;
	uint8_t   m_no_interrupt;
	uint8_t   m_halted;

	address_space *m_program;
	std::function<u8 (offs_t address)> m_dr8;
	address_space *m_io;
	int     m_icount;

	uint8_t   m_prefetch_size;
	uint8_t   m_prefetch_cycles;
	int8_t    m_prefetch_count;
	uint8_t   m_prefetch_reset;
	const uint32_t m_chip_type;

	uint32_t  m_prefix_base;    /* base address of the latest prefix segment */
	uint8_t   m_seg_prefix;     /* prefix segment indicator */

	uint32_t m_EA;
	uint16_t m_EO;
	uint16_t m_E16;

	uint32_t m_debugger_temp;

	typedef void (nec_common_device::*nec_ophandler)();
	typedef uint32_t (nec_common_device::*nec_eahandler)();
	static const nec_ophandler s_nec_instruction[256];
	static const nec_eahandler s_GetEA[192];

protected:
	// FIXME: these belong in v33_base_device
	bool m_xa;
	optional_shared_ptr<uint16_t> m_v33_transtable;

	offs_t v33_translate(offs_t addr);

private:
	inline void prefetch();
	void do_prefetch(int previous_ICount);
	inline uint8_t fetch();
	inline uint16_t fetchword();
	uint8_t fetchop();
	void nec_interrupt(unsigned int_num, int source);
	void nec_trap();
	void nec_brk(unsigned int_num);
	void external_int();

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


class v20_device : public nec_common_device
{
public:
	v20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class v30_device : public nec_common_device
{
public:
	v30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class v33_base_device : public nec_common_device
{
protected:
	v33_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_port_map);

	// device_memory_interface overrides
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	void v33_internal_port_map(address_map &map);
	uint16_t xam_r();
};

class v33_device : public v33_base_device
{
public:
	v33_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class v33a_device : public v33_base_device
{
public:
	v33a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(V20,  v20_device)
DECLARE_DEVICE_TYPE(V30,  v30_device)
DECLARE_DEVICE_TYPE(V33,  v33_device)
DECLARE_DEVICE_TYPE(V33A, v33a_device)

#endif // MAME_CPU_NEC_NEC_H
