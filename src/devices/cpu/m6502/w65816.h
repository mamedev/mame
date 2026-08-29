// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

    w65816.h

    WDC W65C816S, 16-bit successor to the 65C02

    Built on the m6502 family generator, so the core is interruptible: a
    device may call defer_access() from a read or write handler and the CPU
    will suspend mid-instruction and replay that access.

    The 65816 decodes differently in each of its five operating modes, so the
    generator's instruction-bank mechanism carries five banks of 256 states:

        0x000  E     emulation mode           (M=1, X=1)
        0x100  M0X0  native, 16-bit A, 16-bit X/Y
        0x200  M1X0  native,  8-bit A, 16-bit X/Y
        0x300  M0X1  native, 16-bit A,  8-bit X/Y
        0x400  M1X1  native,  8-bit A,  8-bit X/Y

    Section references are to the W65C816S datasheet.

***************************************************************************/

#ifndef MAME_CPU_M6502_W65816_H
#define MAME_CPU_M6502_W65816_H

#pragma once

#include "w65816d.h"

class w65816_device : public cpu_device, public w65816_disassembler::config {
public:
	enum {
		IRQ_LINE = INPUT_LINE_IRQ0,
		NMI_LINE = INPUT_LINE_NMI,
		ABORT_LINE = INPUT_LINE_IRQ0 + 8,
		V_LINE = INPUT_LINE_IRQ0 + 16
	};

	// Vector pull space
	enum {
		AS_VECTORS = AS_OPCODES + 1
	};

	w65816_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto wdm_handler() { return m_wdm_w.bind(); }
	auto sync_cb() { return m_sync_w.bind(); }
	bool get_sync() const { return m_sync; }

protected:
	w65816_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	class memory_interface {
	public:
		memory_access<24, 0, 0, ENDIANNESS_LITTLE>::specific m_program, m_cprogram, m_csprogram;
		memory_access<5, 0, 0, ENDIANNESS_LITTLE>::specific m_vectors;
		bool m_has_vectors = false;

		virtual ~memory_interface() = default;
		virtual uint8_t read(uint32_t adr) = 0;
		virtual uint8_t read_sync(uint32_t adr) = 0;
		virtual uint8_t read_arg(uint32_t adr) = 0;
		virtual uint8_t read_vector(uint16_t adr) = 0;
		virtual void write(uint32_t adr, uint8_t val) = 0;
	};

	class mi_default : public memory_interface {
	public:
		virtual ~mi_default() = default;
		virtual uint8_t read(uint32_t adr) override;
		virtual uint8_t read_sync(uint32_t adr) override;
		virtual uint8_t read_arg(uint32_t adr) override;
		virtual uint8_t read_vector(uint16_t adr) override;
		virtual void write(uint32_t adr, uint8_t val) override;
	};

	enum {
		STATE_RESET = 0xff00
	};

	// instruction bank bases; see update_state_base()
	enum {
		STATE_E    = 0x000,
		STATE_M0X0 = 0x100,
		STATE_M1X0 = 0x200,
		STATE_M0X1 = 0x300,
		STATE_M1X1 = 0x400
	};

	enum {
		F_N = 0x80,
		F_V = 0x40,
		F_M = 0x20, // native only: 0 = 16-bit accumulator
		F_X = 0x10, // native only: 0 = 16-bit index registers
		F_B = 0x10, // emulation only: break, as pushed by BRK/PHP
		F_D = 0x08,
		F_I = 0x04,
		F_Z = 0x02,
		F_C = 0x01
	};

	// emulation-mode vectors
	enum {
		VEC_E_COP   = 0xfff4,
		VEC_E_ABORT = 0xfff8,
		VEC_E_NMI   = 0xfffa,
		VEC_RESET   = 0xfffc,
		VEC_E_IRQ   = 0xfffe
	};

	// native-mode vectors
	enum {
		VEC_N_COP   = 0xffe4,
		VEC_N_BRK   = 0xffe6,
		VEC_N_ABORT = 0xffe8,
		VEC_N_NMI   = 0xffea,
		VEC_N_IRQ   = 0xffee
	};

	virtual void init();

	// device_t overrides
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

	// device_disasm_interface override
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 get_state_base() const override;

	devcb_write8 m_wdm_w;
	devcb_write_line m_sync_w;

	address_space_config m_program_config, m_sprogram_config, m_vector_config;

	uint16_t m_PPC;             // previous program counter (bank-less)
	uint16_t m_NPC;             // next start-of-instruction program counter
	uint16_t m_PC;              // program counter; wraps within the bank (§3.4)
	uint16_t m_SP;              // stack pointer; SH forced to 01 while m_E (§7.1)
	uint16_t m_D;               // direct page
	uint16_t m_A;               // full 16-bit C accumulator, at all times
	uint16_t m_X, m_Y;          // index registers
	uint16_t m_TMP, m_TMP3;     // temporaries
	uint16_t m_VAL;             // operand value, 8 or 16 bits wide
	uint32_t m_TMPL, m_SRC, m_DST;
	uint8_t  m_TMP2;
	uint8_t  m_P;
	uint8_t  m_PB;              // program bank
	uint8_t  m_DB;              // data bank
	uint8_t  m_IR;
	uint8_t  m_E;               // emulation mode (0/1; a uint8_t so it can be state_add()ed)
	int      m_inst_state_base;

	std::unique_ptr<memory_interface> m_mintf;
	int m_inst_state, m_inst_substate;
	int m_icount;
	bool m_nmi_state, m_irq_state, m_abort_state, m_v_state;
	bool m_nmi_pending, m_irq_taken, m_sync, m_inhibit_interrupts;

	uint32_t m_XPC;             // debugger-visible PB:PC

	uint8_t read(uint32_t adr) { return m_mintf->read(adr); }
	void write(uint32_t adr, uint8_t val) { m_mintf->write(adr, val); }
	uint8_t read_sync(uint16_t adr) { return m_mintf->read_sync((m_PB << 16) | adr); }
	uint8_t read_arg(uint16_t adr) { return m_mintf->read_arg((m_PB << 16) | adr); }
	uint8_t read_pc() { return read_arg(m_PC); }
	uint8_t read_vector(uint16_t adr) { return m_mintf->read_vector(adr); }

	void prefetch_start();
	void prefetch_end();
	void prefetch_end_noirq();

	void set_nz(uint8_t v);
	void set_nz16(uint16_t v);

	void update_state_base();

	void do_adc(uint8_t val);
	void do_adc16(uint16_t val);
	void do_sbc(uint8_t val);
	void do_sbc16(uint16_t val);
	void do_cmp(uint16_t val1, uint8_t val2);
	void do_cmp16(uint16_t val1, uint16_t val2);
	void do_bit(uint8_t val);
	void do_bit16(uint16_t val);
	void do_bit_imm(uint8_t val);
	void do_bit_imm16(uint16_t val);

	uint8_t do_asl(uint8_t v);
	uint16_t do_asl16(uint16_t v);
	uint8_t do_lsr(uint8_t v);
	uint16_t do_lsr16(uint16_t v);
	uint8_t do_rol(uint8_t v);
	uint16_t do_rol16(uint16_t v);
	uint8_t do_ror(uint8_t v);
	uint16_t do_ror16(uint16_t v);
	uint8_t do_inc(uint8_t v);
	uint16_t do_inc16(uint16_t v);
	uint8_t do_dec(uint8_t v);
	uint16_t do_dec16(uint16_t v);
	uint8_t do_tsb(uint8_t v);
	uint16_t do_tsb16(uint16_t v);
	uint8_t do_trb(uint8_t v);
	uint16_t do_trb16(uint16_t v);

	// RMW helper.  Named for the write so that m6502make.py gives the line a bus cycle.
	void write_modify_cycle(uint32_t adr, uint8_t val) { if(m_E) write(adr, val); else read(adr); }

	virtual offs_t pc_to_external(uint16_t pc);
	virtual void do_exec_full();
	virtual void do_exec_partial();

	static inline bool page_changing(uint16_t base, int delta) { return ((base + delta) ^ base) & 0xff00; }
	static inline uint16_t set_l(uint16_t base, uint8_t val) { return (base & 0xff00) | val; }
	static inline uint16_t set_h(uint16_t base, uint8_t val) { return (base & 0x00ff) | (val << 8); }
	static inline uint32_t set_b(uint32_t base, uint8_t val) { return (base & 0xffff) | (val << 16); }

	// addressing mode helpers
	uint32_t dp(uint16_t off) const {
		return (m_E && !(m_D & 0xff)) ? (m_D | (off & 0xff)) : uint16_t(m_D + off);
	}
	uint32_t dp_idx(uint16_t off, uint16_t idx) const {
		return (m_E && !(m_D & 0xff)) ? (m_D | ((off + idx) & 0xff)) : uint16_t(m_D + off + idx);
	}
	uint32_t dp_ptr(uint16_t off, int n) const { return dp(off + n); }
	uint32_t dp_lng(uint32_t base, int n) const { return uint16_t(base + n); }

	uint32_t ab(uint16_t a) const { return (m_DB << 16) | a; }
	uint32_t ab_idx(uint16_t a, uint32_t idx) const { return ((m_DB << 16) + a + idx) & 0xffffff; }
	uint32_t ab_idx_phantom(uint16_t a, uint16_t idx) const { return (ab(a) & 0xffff00) | ((a + idx) & 0xff); }
	bool idx_extra_cycle(uint16_t a, uint16_t idx) const { return !(m_P & F_X) || page_changing(a, idx); }
	uint32_t lng(uint32_t a) const { return a & 0xffffff; }
	// al,x and [d],y -- carry into the next bank; same widening as ab_idx
	uint32_t lng_idx(uint32_t a, uint32_t idx) const { return (a + idx) & 0xffffff; }
	uint32_t sr(uint16_t off) const { return uint16_t(m_SP + off); }

	inline void dec_SP() { m_SP = m_E ? (0x100 | uint8_t(m_SP - 1)) : uint16_t(m_SP - 1); }
	inline void inc_SP() { m_SP = m_E ? (0x100 | uint8_t(m_SP + 1)) : uint16_t(m_SP + 1); }
	inline void dec_SP_nw() { m_SP = uint16_t(m_SP - 1); }
	inline void inc_SP_nw() { m_SP = uint16_t(m_SP + 1); }

#define O(o) void o ## _full(); void o ## _partial()

	O(adc_aba); O(adc_abl); O(adc_abx); O(adc_aby); O(adc_alx); O(adc_ds); O(adc_dsy); O(adc_idx); O(adc_idy);
	O(adc_imm); O(adc_w_aba); O(adc_w_abl); O(adc_w_abx); O(adc_w_aby); O(adc_w_alx); O(adc_w_ds); O(adc_w_dsy);
	O(adc_w_idx); O(adc_w_idy); O(adc_w_imm16); O(adc_w_zil); O(adc_w_ziy); O(adc_w_zpg); O(adc_w_zpi); O(adc_w_zpx);
	O(adc_zil); O(adc_ziy); O(adc_zpg); O(adc_zpi); O(adc_zpx); O(and_aba); O(and_abl); O(and_abx); O(and_aby);
	O(and_alx); O(and_ds); O(and_dsy); O(and_idx); O(and_idy); O(and_imm); O(and_w_aba); O(and_w_abl); O(and_w_abx);
	O(and_w_aby); O(and_w_alx); O(and_w_ds); O(and_w_dsy); O(and_w_idx); O(and_w_idy); O(and_w_imm16); O(and_w_zil);
	O(and_w_ziy); O(and_w_zpg); O(and_w_zpi); O(and_w_zpx); O(and_zil); O(and_ziy); O(and_zpg); O(and_zpi);
	O(and_zpx); O(asl_aba); O(asl_abx); O(asl_acc); O(asl_w_aba); O(asl_w_abx); O(asl_w_acc); O(asl_w_zpg);
	O(asl_w_zpx); O(asl_zpg); O(asl_zpx); O(bcc_rel); O(bcs_rel); O(beq_rel); O(bit_aba); O(bit_abx); O(bit_imm);
	O(bit_w_aba); O(bit_w_abx); O(bit_w_imm16); O(bit_w_zpg); O(bit_w_zpx); O(bit_zpg); O(bit_zpx); O(bmi_rel);
	O(bne_rel); O(bpl_rel); O(bra_rel); O(brk_e_imm); O(brk_imm); O(brl_rell); O(bvc_rel); O(bvs_rel); O(clc_imp);
	O(cld_imp); O(cli_imp); O(clv_imp); O(cmp_aba); O(cmp_abl); O(cmp_abx); O(cmp_aby); O(cmp_alx); O(cmp_ds);
	O(cmp_dsy); O(cmp_idx); O(cmp_idy); O(cmp_imm); O(cmp_w_aba); O(cmp_w_abl); O(cmp_w_abx); O(cmp_w_aby);
	O(cmp_w_alx); O(cmp_w_ds); O(cmp_w_dsy); O(cmp_w_idx); O(cmp_w_idy); O(cmp_w_imm16); O(cmp_w_zil); O(cmp_w_ziy);
	O(cmp_w_zpg); O(cmp_w_zpi); O(cmp_w_zpx); O(cmp_zil); O(cmp_ziy); O(cmp_zpg); O(cmp_zpi); O(cmp_zpx);
	O(cop_e_imm); O(cop_imm); O(cpx_aba); O(cpx_imm); O(cpx_w_aba); O(cpx_w_imm16); O(cpx_w_zpg); O(cpx_zpg);
	O(cpy_aba); O(cpy_imm); O(cpy_w_aba); O(cpy_w_imm16); O(cpy_w_zpg); O(cpy_zpg); O(dec_aba); O(dec_abx);
	O(dec_acc); O(dec_w_aba); O(dec_w_abx); O(dec_w_acc); O(dec_w_zpg); O(dec_w_zpx); O(dec_zpg); O(dec_zpx);
	O(dex_imp); O(dex_w_imp); O(dey_imp); O(dey_w_imp); O(eor_aba); O(eor_abl); O(eor_abx); O(eor_aby); O(eor_alx);
	O(eor_ds); O(eor_dsy); O(eor_idx); O(eor_idy); O(eor_imm); O(eor_w_aba); O(eor_w_abl); O(eor_w_abx); O(eor_w_aby);
	O(eor_w_alx); O(eor_w_ds); O(eor_w_dsy); O(eor_w_idx); O(eor_w_idy); O(eor_w_imm16); O(eor_w_zil); O(eor_w_ziy);
	O(eor_w_zpg); O(eor_w_zpi); O(eor_w_zpx); O(eor_zil); O(eor_ziy); O(eor_zpg); O(eor_zpi); O(eor_zpx); O(inc_aba);
	O(inc_abx); O(inc_acc); O(inc_w_aba); O(inc_w_abx); O(inc_w_acc); O(inc_w_zpg); O(inc_w_zpx); O(inc_zpg);
	O(inc_zpx); O(inx_imp); O(inx_w_imp); O(iny_imp); O(iny_w_imp); O(jml_ind); O(jmp_abl); O(jmp_adr); O(jmp_iax);
	O(jmp_ind); O(jsl_abl); O(jsr_adr); O(jsr_iax); O(lda_aba); O(lda_abl); O(lda_abx); O(lda_aby); O(lda_alx);
	O(lda_ds); O(lda_dsy); O(lda_idx); O(lda_idy); O(lda_imm); O(lda_w_aba); O(lda_w_abl); O(lda_w_abx); O(lda_w_aby);
	O(lda_w_alx); O(lda_w_ds); O(lda_w_dsy); O(lda_w_idx); O(lda_w_idy); O(lda_w_imm16); O(lda_w_zil); O(lda_w_ziy);
	O(lda_w_zpg); O(lda_w_zpi); O(lda_w_zpx); O(lda_zil); O(lda_ziy); O(lda_zpg); O(lda_zpi); O(lda_zpx); O(ldx_aba);
	O(ldx_aby); O(ldx_imm); O(ldx_w_aba); O(ldx_w_aby); O(ldx_w_imm16); O(ldx_w_zpg); O(ldx_w_zpy); O(ldx_zpg);
	O(ldx_zpy); O(ldy_aba); O(ldy_abx); O(ldy_imm); O(ldy_w_aba); O(ldy_w_abx); O(ldy_w_imm16); O(ldy_w_zpg);
	O(ldy_w_zpx); O(ldy_zpg); O(ldy_zpx); O(lsr_aba); O(lsr_abx); O(lsr_acc); O(lsr_w_aba); O(lsr_w_abx);
	O(lsr_w_acc); O(lsr_w_zpg); O(lsr_w_zpx); O(lsr_zpg); O(lsr_zpx); O(mvn_dbf); O(mvn_w_dbf); O(mvp_dbf);
	O(mvp_w_dbf); O(nop_imp); O(ora_aba); O(ora_abl); O(ora_abx); O(ora_aby); O(ora_alx); O(ora_ds); O(ora_dsy);
	O(ora_idx); O(ora_idy); O(ora_imm); O(ora_w_aba); O(ora_w_abl); O(ora_w_abx); O(ora_w_aby); O(ora_w_alx);
	O(ora_w_ds); O(ora_w_dsy); O(ora_w_idx); O(ora_w_idy); O(ora_w_imm16); O(ora_w_zil); O(ora_w_ziy); O(ora_w_zpg);
	O(ora_w_zpi); O(ora_w_zpx); O(ora_zil); O(ora_ziy); O(ora_zpg); O(ora_zpi); O(ora_zpx); O(pea_imm16); O(pei_zpg);
	O(per_rell); O(pha_imp); O(pha_w_imp); O(phb_imp); O(phd_imp); O(phk_imp); O(php_e_imp); O(php_imp); O(phx_imp);
	O(phx_w_imp); O(phy_imp); O(phy_w_imp); O(pla_imp); O(pla_w_imp); O(plb_imp); O(pld_imp); O(plp_e_imp);
	O(plp_imp); O(plx_imp); O(plx_w_imp); O(ply_imp); O(ply_w_imp); O(rep_e_imm); O(rep_imm); O(reset); O(rol_aba);
	O(rol_abx); O(rol_acc); O(rol_w_aba); O(rol_w_abx); O(rol_w_acc); O(rol_w_zpg); O(rol_w_zpx); O(rol_zpg);
	O(rol_zpx); O(ror_aba); O(ror_abx); O(ror_acc); O(ror_w_aba); O(ror_w_abx); O(ror_w_acc); O(ror_w_zpg);
	O(ror_w_zpx); O(ror_zpg); O(ror_zpx); O(rti_e_imp); O(rti_imp); O(rtl_imp); O(rts_imp); O(sbc_aba); O(sbc_abl);
	O(sbc_abx); O(sbc_aby); O(sbc_alx); O(sbc_ds); O(sbc_dsy); O(sbc_idx); O(sbc_idy); O(sbc_imm); O(sbc_w_aba);
	O(sbc_w_abl); O(sbc_w_abx); O(sbc_w_aby); O(sbc_w_alx); O(sbc_w_ds); O(sbc_w_dsy); O(sbc_w_idx); O(sbc_w_idy);
	O(sbc_w_imm16); O(sbc_w_zil); O(sbc_w_ziy); O(sbc_w_zpg); O(sbc_w_zpi); O(sbc_w_zpx); O(sbc_zil); O(sbc_ziy);
	O(sbc_zpg); O(sbc_zpi); O(sbc_zpx); O(sec_imp); O(sed_imp); O(sei_imp); O(sep_e_imm); O(sep_imm); O(sta_aba);
	O(sta_abl); O(sta_abx); O(sta_aby); O(sta_alx); O(sta_ds); O(sta_dsy); O(sta_idx); O(sta_idy); O(sta_w_aba);
	O(sta_w_abl); O(sta_w_abx); O(sta_w_aby); O(sta_w_alx); O(sta_w_ds); O(sta_w_dsy); O(sta_w_idx); O(sta_w_idy);
	O(sta_w_zil); O(sta_w_ziy); O(sta_w_zpg); O(sta_w_zpi); O(sta_w_zpx); O(sta_zil); O(sta_ziy); O(sta_zpg);
	O(sta_zpi); O(sta_zpx); O(stp_imp); O(stx_aba); O(stx_w_aba); O(stx_w_zpg); O(stx_w_zpy); O(stx_zpg); O(stx_zpy);
	O(sty_aba); O(sty_w_aba); O(sty_w_zpg); O(sty_w_zpx); O(sty_zpg); O(sty_zpx); O(stz_aba); O(stz_abx);
	O(stz_w_aba); O(stz_w_abx); O(stz_w_zpg); O(stz_w_zpx); O(stz_zpg); O(stz_zpx); O(tax_imp); O(tax_w_imp);
	O(tay_imp); O(tay_w_imp); O(tcd_imp); O(tcs_imp); O(tdc_imp); O(trb_aba); O(trb_w_aba); O(trb_w_zpg); O(trb_zpg);
	O(tsb_aba); O(tsb_w_aba); O(tsb_w_zpg); O(tsb_zpg); O(tsc_imp); O(tsx_imp); O(tsx_w_imp); O(txa_imp);
	O(txa_w_imp); O(txs_imp); O(txy_imp); O(txy_w_imp); O(tya_imp); O(tya_w_imp); O(tyx_imp); O(tyx_w_imp);
	O(wai_imp); O(wdm_imm); O(xba_imp); O(xce_imp);

#undef O
};

enum {
	W65816_PC = 1,
	W65816_A,
	W65816_X,
	W65816_Y,
	W65816_P,
	W65816_S,
	W65816_D,
	W65816_DB,
	W65816_PB,
	W65816_E,
	W65816_IR
};

enum {
	W65816_IRQ_LINE = w65816_device::IRQ_LINE,
	W65816_NMI_LINE = w65816_device::NMI_LINE,
	W65816_ABORT_LINE = w65816_device::ABORT_LINE,
	W65816_SET_OVERFLOW = w65816_device::V_LINE
};

DECLARE_DEVICE_TYPE(W65816, w65816_device)

#endif // MAME_CPU_M6502_W65816_H
