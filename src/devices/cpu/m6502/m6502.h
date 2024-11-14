// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.h

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502_H
#define MAME_CPU_M6502_M6502_H

#pragma once


class m6502_device : public cpu_device {
public:
	enum {
		IRQ_LINE = INPUT_LINE_IRQ0,
		APU_IRQ_LINE = INPUT_LINE_IRQ1,
		NMI_LINE = INPUT_LINE_NMI,
		V_LINE   = INPUT_LINE_IRQ0 + 16
	};

	class memory_interface {
	public:
		memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache cprogram, csprogram;
		memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific program;
		memory_access<14, 0, 0, ENDIANNESS_LITTLE>::specific program14;

		virtual ~memory_interface() = default;
		virtual uint8_t read(uint16_t adr) = 0;
		virtual uint8_t read_9(uint16_t adr);
		virtual uint8_t read_sync(uint16_t adr) = 0;
		virtual uint8_t read_arg(uint16_t adr) = 0;
		virtual void write(uint16_t adr, uint8_t val) = 0;
		virtual void write_9(uint16_t adr, uint8_t val);
	};

	m6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_address_width(int width, bool custom_interface) {
		program_config.m_addr_width = width;
		sprogram_config.m_addr_width = width;
		uses_custom_memory_interface = custom_interface;
	}

	void set_custom_memory_interface(std::unique_ptr<memory_interface> interface) {
		mintf = std::move(interface);
	}

	bool get_sync() const { return sync; }

	auto sync_cb() { return sync_w.bind(); }

	devcb_write_line sync_w;

protected:
	m6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	class mi_default : public memory_interface {
	public:
		virtual ~mi_default() = default;
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	class mi_default14 : public mi_default {
	public:
		virtual ~mi_default14() = default;
		virtual uint8_t read(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	enum {
		STATE_RESET = 0xff00
	};

	enum {
		F_N = 0x80,
		F_V = 0x40,
		F_E = 0x20, // 65ce02
		F_T = 0x20, // M740: replaces A with $00,X in some opcodes when set
		F_B = 0x10,
		F_D = 0x08,
		F_I = 0x04,
		F_Z = 0x02,
		F_C = 0x01
	};

	virtual void init();

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config program_config, sprogram_config;

	uint16_t  PPC;                    /* previous program counter */
	uint16_t  NPC;                    /* next start-of-instruction program counter */
	uint16_t  PC;                     /* program counter */
	uint16_t  SP;                     /* stack pointer (always 100 - 1FF) */
	uint16_t  TMP;                    /* temporary internal values */
	uint8_t   TMP2;                   /* another temporary internal value, 8 bits this time */
	uint8_t   A;                      /* Accumulator */
	uint8_t   X;                      /* X index register */
	uint8_t   Y;                      /* Y index register */
	uint8_t   P;                      /* Processor status */
	uint8_t   IR;                     /* Prefetched instruction register */
	int       inst_state_base;        /* Current instruction bank */

	std::unique_ptr<memory_interface> mintf;
	int inst_state, inst_substate;
	int icount, bcount, count_before_instruction_step;
	bool nmi_state, irq_state, apu_irq_state, v_state;
	bool nmi_pending, irq_taken, sync, inhibit_interrupts;
	bool uses_custom_memory_interface;

	uint8_t read(uint16_t adr) { return mintf->read(adr); }
	uint8_t read_9(uint16_t adr) { return mintf->read_9(adr); }
	void write(uint16_t adr, uint8_t val) { mintf->write(adr, val); }
	void write_9(uint16_t adr, uint8_t val) { mintf->write_9(adr, val); }
	uint8_t read_arg(uint16_t adr) { return mintf->read_arg(adr); }
	uint8_t read_pc() { return mintf->read_arg(PC); }
	void prefetch_start();
	void prefetch_end();
	void prefetch_end_noirq();
	void set_nz(uint8_t v);

	u32 XPC;
	virtual offs_t pc_to_external(u16 pc); // For paged PCs
	virtual void do_exec_full();
	virtual void do_exec_partial();

	// inline helpers
	static inline bool page_changing(uint16_t base, int delta) { return ((base + delta) ^ base) & 0xff00; }
	static inline uint16_t set_l(uint16_t base, uint8_t val) { return (base & 0xff00) | val; }
	static inline uint16_t set_h(uint16_t base, uint8_t val) { return (base & 0x00ff) | (val << 8); }

	inline void dec_SP() { SP = set_l(SP, SP-1); }
	inline void inc_SP() { SP = set_l(SP, SP+1); }

	void do_adc_d(uint8_t val);
	void do_adc_nd(uint8_t val);
	void do_sbc_d(uint8_t val);
	void do_sbc_nd(uint8_t val);
	void do_arr_d();
	void do_arr_nd();

	void do_adc(uint8_t val);
	void do_cmp(uint8_t val1, uint8_t val2);
	void do_sbc(uint8_t val);
	void do_bit(uint8_t val);
	void do_arr();
	uint8_t do_asl(uint8_t v);
	uint8_t do_lsr(uint8_t v);
	uint8_t do_ror(uint8_t v);
	uint8_t do_rol(uint8_t v);
	uint8_t do_asr(uint8_t v);

#define O(o) void o ## _full(); void o ## _partial()

	// NMOS 6502 opcodes
	//   documented opcodes
	O(adc_aba); O(adc_abx); O(adc_aby); O(adc_idx); O(adc_idy); O(adc_imm); O(adc_zpg); O(adc_zpx);
	O(and_aba); O(and_abx); O(and_aby); O(and_imm); O(and_idx); O(and_idy); O(and_zpg); O(and_zpx);
	O(asl_aba); O(asl_abx); O(asl_acc); O(asl_zpg); O(asl_zpx);
	O(bcc_rel);
	O(bcs_rel);
	O(beq_rel);
	O(bit_aba); O(bit_zpg);
	O(bmi_rel);
	O(bne_rel);
	O(bpl_rel);
	O(brk_imp);
	O(bvc_rel);
	O(bvs_rel);
	O(clc_imp);
	O(cld_imp);
	O(cli_imp);
	O(clv_imp);
	O(cmp_aba); O(cmp_abx); O(cmp_aby); O(cmp_idx); O(cmp_idy); O(cmp_imm); O(cmp_zpg); O(cmp_zpx);
	O(cpx_aba); O(cpx_imm); O(cpx_zpg);
	O(cpy_aba); O(cpy_imm); O(cpy_zpg);
	O(dec_aba); O(dec_abx); O(dec_zpg); O(dec_zpx);
	O(dex_imp);
	O(dey_imp);
	O(eor_aba); O(eor_abx); O(eor_aby); O(eor_idx); O(eor_idy); O(eor_imm); O(eor_zpg); O(eor_zpx);
	O(inc_aba); O(inc_abx); O(inc_zpg); O(inc_zpx);
	O(inx_imp);
	O(iny_imp);
	O(jmp_adr); O(jmp_ind);
	O(jsr_adr);
	O(lda_aba); O(lda_abx); O(lda_aby); O(lda_idx); O(lda_idy); O(lda_imm); O(lda_zpg); O(lda_zpx);
	O(ldx_aba); O(ldx_aby); O(ldx_imm); O(ldx_zpg); O(ldx_zpy);
	O(ldy_aba); O(ldy_abx); O(ldy_imm); O(ldy_zpg); O(ldy_zpx);
	O(lsr_aba); O(lsr_abx); O(lsr_acc); O(lsr_zpg); O(lsr_zpx);
	O(nop_imp);
	O(ora_aba); O(ora_abx); O(ora_aby); O(ora_imm); O(ora_idx); O(ora_idy); O(ora_zpg); O(ora_zpx);
	O(pha_imp);
	O(php_imp);
	O(pla_imp);
	O(plp_imp);
	O(rol_aba); O(rol_abx); O(rol_acc); O(rol_zpg); O(rol_zpx);
	O(ror_aba); O(ror_abx); O(ror_acc); O(ror_zpg); O(ror_zpx);
	O(rti_imp);
	O(rts_imp);
	O(sbc_aba); O(sbc_abx); O(sbc_aby); O(sbc_idx); O(sbc_idy); O(sbc_imm); O(sbc_zpg); O(sbc_zpx);
	O(sec_imp);
	O(sed_imp);
	O(sei_imp);
	O(sta_aba); O(sta_abx); O(sta_aby); O(sta_idx); O(sta_idy); O(sta_zpg); O(sta_zpx);
	O(stx_aba); O(stx_zpg); O(stx_zpy);
	O(sty_aba); O(sty_zpg); O(sty_zpx);
	O(tax_imp);
	O(tay_imp);
	O(tsx_imp);
	O(txa_imp);
	O(txs_imp);
	O(tya_imp);

	//   exceptions
	O(reset);

	//   undocumented reliable instructions
	O(dcp_aba); O(dcp_abx); O(dcp_aby); O(dcp_idx); O(dcp_idy); O(dcp_zpg); O(dcp_zpx);
	O(isb_aba); O(isb_abx); O(isb_aby); O(isb_idx); O(isb_idy); O(isb_zpg); O(isb_zpx);
	O(lax_aba); O(lax_aby); O(lax_idx); O(lax_idy); O(lax_zpg); O(lax_zpy);
	O(rla_aba); O(rla_abx); O(rla_aby); O(rla_idx); O(rla_idy); O(rla_zpg); O(rla_zpx);
	O(rra_aba); O(rra_abx); O(rra_aby); O(rra_idx); O(rra_idy); O(rra_zpg); O(rra_zpx);
	O(sax_aba); O(sax_idx); O(sax_zpg); O(sax_zpy);
	O(sbx_imm);
	O(sha_aby); O(sha_idy);
	O(shs_aby);
	O(shx_aby);
	O(shy_abx);
	O(slo_aba); O(slo_abx); O(slo_aby); O(slo_idx); O(slo_idy); O(slo_zpg); O(slo_zpx);
	O(sre_aba); O(sre_abx); O(sre_aby); O(sre_idx); O(sre_idy); O(sre_zpg); O(sre_zpx);

	//   undocumented unreliable instructions
	//     behaviour differs between visual6502 and online docs, which
	//     is a clear sign reliability is not to be expected
	//     implemented version follows visual6502
	O(anc_imm);
	O(ane_imm);
	O(arr_imm);
	O(asr_imm);
	O(las_aby);
	O(lxa_imm);

	//   nop variants
	O(nop_imm); O(nop_aba); O(nop_abx); O(nop_zpg); O(nop_zpx);

	//   system killers
	O(kil_non);

#undef O
};

class m6512_device : public m6502_device {
public:
	m6512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	M6502_PC = 1,
	M6502_A,
	M6502_X,
	M6502_Y,
	M6502_P,
	M6502_S,
	M6502_IR
};

enum {
	M6502_IRQ_LINE = m6502_device::IRQ_LINE,
	M6502_NMI_LINE = m6502_device::NMI_LINE,
	M6502_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(M6502, m6502_device)
DECLARE_DEVICE_TYPE(M6512, m6512_device)

#endif // MAME_CPU_M6502_M6502_H
