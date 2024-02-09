// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8.h

    H8-300 base cpu emulation


***************************************************************************/

#ifndef MAME_CPU_H8_H8_H
#define MAME_CPU_H8_H8_H

#pragma once

class h8gen_dma_device;
class h8_dtc_device;
struct h8_dma_state;
struct h8_dtc_state;

class h8_device;

#include "h8_sci.h"

class h8_device : public cpu_device {
public:
	enum {
		STATE_RESET              = 0x10000,
		STATE_IRQ                = 0x10001,
		STATE_TRACE              = 0x10002,
		STATE_DMA                = 0x10003,
		STATE_DTC                = 0x10004,
		STATE_DTC_VECTOR         = 0x10005,
		STATE_DTC_WRITEBACK      = 0x10006
	};

	template<int Port> auto read_adc() { return m_read_adc[Port].bind(); }
	template<int Sci> auto write_sci_tx() { return m_sci_tx[Sci].bind(); }
	template<int Sci> auto write_sci_clk() { return m_sci_clk[Sci].bind(); }

	void sci_set_external_clock_period(int sci, const attotime &period) {
		m_sci[sci].lookup()->do_set_external_clock_period(period);
	}

	template<int Sci> void sci_rx_w(int state) { m_sci[Sci]->do_rx_w(state); }
	template<int Sci> void sci_clk_w(int state) { m_sci[Sci]->do_clk_w(state); }

	void internal_update();
	void set_irq(int irq_vector, int irq_level, bool irq_nmi);
	bool trigger_dma(int vector);
	void set_dma_channel(h8_dma_state *state);
	void update_active_dma_channel();
	void set_current_dtc(h8_dtc_state *state);
	void request_state(int state);
	bool access_is_dma() const { return m_inst_state == STATE_DMA || m_inst_state == STATE_DTC; }

	u16 do_read_adc(int port) { return m_read_adc[port](); }
	u8 do_read_port(int port) { return m_read_port[port](); }
	void do_write_port(int port, u8 data) { return m_write_port[port](data); }
	void do_sci_tx(int sci, int state) { m_sci_tx[sci](state); }
	void do_sci_clk(int sci, int state) { m_sci_clk[sci](state); }

protected:
	enum {
		// digital I/O ports
		// ports 4-B are valid on 16-bit H8/3xx, ports 1-9 on 8-bit H8/3xx
		// H8S/2394 has 12 ports named 1-6 and A-G
		PORT_1,
		PORT_2,
		PORT_3,
		PORT_4,
		PORT_5,
		PORT_6,
		PORT_7,
		PORT_8,
		PORT_9,
		PORT_A,
		PORT_B,
		PORT_C,
		PORT_D,
		PORT_E,
		PORT_F,
		PORT_G,
		PORT_COUNT
	};

	static const char port_names[];

	enum {
		F_I  = 0x80,
		F_UI = 0x40,
		F_H  = 0x20,
		F_U  = 0x10,
		F_N  = 0x08,
		F_Z  = 0x04,
		F_V  = 0x02,
		F_C  = 0x01,

		EXR_T  = 0x80,
		EXR_NC = 0x78,
		EXR_I  = 0x07
	};

	h8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 12; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	memory_access<32, 1, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 1, 0, ENDIANNESS_BIG>::specific m_program;
	devcb_read16::array<8> m_read_adc;
	devcb_read8::array<PORT_COUNT> m_read_port;
	devcb_write8::array<PORT_COUNT> m_write_port;
	optional_device_array<h8_sci_device, 3> m_sci;
	devcb_write_line::array<3> m_sci_tx, m_sci_clk;

	h8gen_dma_device *m_dma_device;
	h8_dtc_device *m_dtc_device;
	h8_dma_state *m_dma_channel[8];
	int m_current_dma;
	h8_dtc_state *m_current_dtc;

	uint32_t  m_PPC;                    /* previous program counter */
	uint32_t  m_NPC;                    /* next start-of-instruction program counter */
	uint32_t  m_PC;                     /* program counter */
	uint16_t  m_PIR;                    /* Prefetched word */
	uint16_t  m_IR[5];                  /* Fetched instruction */
	uint16_t  m_R[16];                  /* Rn (0-7), En (8-15, h8-300h+) */
	uint8_t   m_EXR;                    /* Interrupt/trace register (h8s/2000+) */
	uint8_t   m_CCR;                    /* Condition-code register */
	int64_t   m_MAC;                    /* Multiply accumulator (h8s/2600+) */
	uint8_t   m_MACF;                   /* MAC flags (h8s/2600+) */
	uint32_t  m_TMP1, m_TMP2;
	uint32_t  m_TMPR;                   /* For debugger ER register import */

	bool m_has_exr, m_has_mac, m_has_trace, m_supports_advanced, m_mode_advanced, m_mode_a20, m_mac_saturating;
	bool m_has_hc; // GT913's CCR bit 5 is I, not H

	int m_inst_state, m_inst_substate, m_requested_state;
	int m_icount, m_bcount, m_count_before_instruction_step;
	int m_irq_vector, m_taken_irq_vector;
	int m_irq_level, m_taken_irq_level;
	bool m_irq_required, m_irq_nmi;

	virtual void do_exec_full();
	virtual void do_exec_partial();
	static void add_event(uint64_t &event_time, uint64_t new_event);
	virtual bool exr_in_stack() const;
	virtual void update_irq_filter() = 0;
	virtual void interrupt_taken() = 0;
	virtual void internal_update(uint64_t current_time) = 0;
	void recompute_bcount(uint64_t event_time);
	virtual int trace_setup();
	virtual int trapa_setup();
	virtual void irq_setup() = 0;

	virtual uint16_t read16i(uint32_t adr);
	virtual uint8_t read8(uint32_t adr);
	virtual void write8(uint32_t adr, uint8_t data);
	virtual uint16_t read16(uint32_t adr);
	virtual void write16(uint32_t adr, uint16_t data);
	virtual void internal(int cycles);
	void prefetch_switch(uint32_t pc, uint16_t ir) { m_NPC = pc & 0xffffff; m_PC = pc+2; m_PIR = ir; }
	void prefetch_done();
	void prefetch_done_noirq();
	void prefetch_done_noirq_notrace();
	void illegal();
	u16 adc_default(int adc);
	u8 port_default_r(int port);
	void port_default_w(int port, u8 data);

	uint8_t do_addx8(uint8_t a, uint8_t b);
	uint8_t do_subx8(uint8_t a, uint8_t b);

	uint8_t do_inc8(uint8_t a, uint8_t b);
	uint16_t do_inc16(uint16_t a, uint16_t b);
	uint32_t do_inc32(uint32_t a, uint32_t b);

	uint8_t do_add8(uint8_t a, uint8_t b);
	uint16_t do_add16(uint16_t a, uint16_t b);
	uint32_t do_add32(uint32_t a, uint32_t b);

	uint8_t do_dec8(uint8_t a, uint8_t b);
	uint16_t do_dec16(uint16_t a, uint16_t b);
	uint32_t do_dec32(uint32_t a, uint32_t b);

	uint8_t do_sub8(uint8_t a, uint8_t b);
	uint16_t do_sub16(uint16_t a, uint16_t b);
	uint32_t do_sub32(uint32_t a, uint32_t b);

	uint8_t do_shal8(uint8_t v);
	uint16_t do_shal16(uint16_t v);
	uint32_t do_shal32(uint32_t v);

	uint8_t do_shar8(uint8_t v);
	uint16_t do_shar16(uint16_t v);
	uint32_t do_shar32(uint32_t v);

	uint8_t do_shll8(uint8_t v);
	uint16_t do_shll16(uint16_t v);
	uint32_t do_shll32(uint32_t v);

	uint8_t do_shlr8(uint8_t v);
	uint16_t do_shlr16(uint16_t v);
	uint32_t do_shlr32(uint32_t v);

	uint8_t do_rotl8(uint8_t v);
	uint16_t do_rotl16(uint16_t v);
	uint32_t do_rotl32(uint32_t v);

	uint8_t do_rotr8(uint8_t v);
	uint16_t do_rotr16(uint16_t v);
	uint32_t do_rotr32(uint32_t v);

	uint8_t do_rotxl8(uint8_t v);
	uint16_t do_rotxl16(uint16_t v);
	uint32_t do_rotxl32(uint32_t v);

	uint8_t do_rotxr8(uint8_t v);
	uint16_t do_rotxr16(uint16_t v);
	uint32_t do_rotxr32(uint32_t v);

	uint8_t do_shal2_8(uint8_t v);
	uint16_t do_shal2_16(uint16_t v);
	uint32_t do_shal2_32(uint32_t v);

	uint8_t do_shar2_8(uint8_t v);
	uint16_t do_shar2_16(uint16_t v);
	uint32_t do_shar2_32(uint32_t v);

	uint8_t do_shll2_8(uint8_t v);
	uint16_t do_shll2_16(uint16_t v);
	uint32_t do_shll2_32(uint32_t v);

	uint8_t do_shlr2_8(uint8_t v);
	uint16_t do_shlr2_16(uint16_t v);
	uint32_t do_shlr2_32(uint32_t v);

	uint8_t do_rotl2_8(uint8_t v);
	uint16_t do_rotl2_16(uint16_t v);
	uint32_t do_rotl2_32(uint32_t v);

	uint8_t do_rotr2_8(uint8_t v);
	uint16_t do_rotr2_16(uint16_t v);
	uint32_t do_rotr2_32(uint32_t v);

	uint8_t do_rotxl2_8(uint8_t v);
	uint16_t do_rotxl2_16(uint16_t v);
	uint32_t do_rotxl2_32(uint32_t v);

	uint8_t do_rotxr2_8(uint8_t v);
	uint16_t do_rotxr2_16(uint16_t v);
	uint32_t do_rotxr2_32(uint32_t v);

	void set_nzv8(uint8_t v);
	void set_nzv16(uint16_t v);
	void set_nzv32(uint32_t v);

	void set_nz16(uint16_t v);
	void set_nz32(uint32_t v);

	inline void r8_w(int reg, uint8_t val) {
		if(reg & 8)
			m_R[reg & 7] = (m_R[reg & 7] & 0xff00) | val;
		else
			m_R[reg & 7] = (m_R[reg & 7] & 0xff) | (val << 8);
	}

	inline uint8_t r8_r(int reg) {
		if(reg & 8)
			return m_R[reg & 7];
		else
			return m_R[reg & 7] >> 8;
	}

	// Note that the decode is so that there's no risk of a h8-300
	// hitting the E registers even with the 0xf mask - the
	// instruction would not be called in the first place
	//
	// Well, except for the instructions where the h8-300 mode is r16
	// and the h8-300h is r32 of course, we have to be careful to mask
	// in h8.lst there if the top bit is 1.

	inline void r16_w(int reg, uint16_t val) { m_R[reg & 0xf] = val; }
	inline uint16_t r16_r(int reg) { return m_R[reg & 0xf]; }

#define O(o) void o ## _full(); void o ## _partial()
	O(add_b_imm8_r8u); O(add_b_r8h_r8l); O(add_w_r16h_r16l);
	O(adds_l_one_r16l); O(adds_l_two_r16l);
	O(addx_b_imm8_r8u); O(addx_b_r8h_r8l);
	O(and_b_imm8_r8u); O(and_b_r8h_r8l);
	O(andc_imm8_ccr);
	O(band_imm3_abs8); O(band_imm3_r8l); O(band_imm3_r16ihh);
	O(bcc_rel8);
	O(bclr_imm3_abs8); O(bclr_imm3_r8l); O(bclr_imm3_r16ihh); O(bclr_r8h_abs8); O(bclr_r8h_r8l); O(bclr_r8h_r16ihh);
	O(bcs_rel8);
	O(beq_rel8);
	O(bf_rel8);
	O(bge_rel8);
	O(bgt_rel8);
	O(bhi_rel8);
	O(biand_imm3_abs8); O(biand_imm3_r8l); O(biand_imm3_r16ihh);
	O(bild_imm3_abs8); O(bild_imm3_r8l); O(bild_imm3_r16ihh);
	O(bior_imm3_abs8); O(bior_imm3_r8l); O(bior_imm3_r16ihh);
	O(bist_imm3_abs8); O(bist_imm3_r8l); O(bist_imm3_r16ihh);
	O(bixor_imm3_abs8); O(bixor_imm3_r8l); O(bixor_imm3_r16ihh);
	O(bld_imm3_abs8); O(bld_imm3_r8l); O(bld_imm3_r16ihh);
	O(ble_rel8);
	O(bls_rel8);
	O(blt_rel8);
	O(bmi_rel8);
	O(bne_rel8);
	O(bnot_imm3_abs8); O(bnot_imm3_r8l); O(bnot_imm3_r16ihh); O(bnot_r8h_abs8); O(bnot_r8h_r8l); O(bnot_r8h_r16ihh);
	O(bor_imm3_abs8); O(bor_imm3_r8l); O(bor_imm3_r16ihh);
	O(bpl_rel8);
	O(bset_imm3_abs8); O(bset_imm3_r8l); O(bset_imm3_r16ihh); O(bset_r8h_abs8); O(bset_r8h_r8l); O(bset_r8h_r16ihh);
	O(bsr_rel8);
	O(bst_imm3_abs8); O(bst_imm3_r8l); O(bst_imm3_r16ihh);
	O(bt_rel8);
	O(btst_imm3_abs8); O(btst_imm3_r8l); O(btst_imm3_r16ihh); O(btst_r8h_abs8); O(btst_r8h_r8l); O(btst_r8h_r16ihh);
	O(bvc_rel8);
	O(bvs_rel8);
	O(bxor_imm3_abs8); O(bxor_imm3_r8l); O(bxor_imm3_r16ihh);
	O(cmp_b_imm8_r8u); O(cmp_b_r8h_r8l); O(cmp_w_r16h_r16l);
	O(daa_b_r8l);
	O(das_b_r8l);
	O(dec_b_one_r8l);
	O(divxu_b_r8h_r16l);
	O(eepmov_b);
	O(inc_b_one_r8l);
	O(jmp_abs8i); O(jmp_abs16e); O(jmp_r16h);
	O(jsr_abs8i); O(jsr_abs16e); O(jsr_r16h);
	O(ldc_imm8_ccr); O(ldc_r8l_ccr);
	O(mov_b_abs16_r8l); O(mov_b_abs8_r8u); O(mov_b_imm8_r8u); O(mov_b_r8h_r8l); O(mov_b_r8l_abs16); O(mov_b_r8u_abs8); O(mov_b_r16ih_r8l); O(mov_b_r8l_r16ih); O(mov_b_r16d16h_r8l); O(mov_b_r8l_r16d16h); O(mov_b_r16ph_r8l); O(mov_b_r8l_pr16h);
	O(mov_w_abs16_r16l); O(mov_w_imm16_r16l); O(mov_w_r16h_r16l); O(mov_w_r16l_abs16); O(mov_w_r16ih_r16l); O(mov_w_r16l_r16ih); O(mov_w_r16ph_r16l); O(mov_w_r16l_pr16h); O(mov_w_r16l_r16d16h); O(mov_w_r16d16h_r16l);
	O(movfpe_abs16_r8l);
	O(movtpe_r8l_abs16);
	O(mulxu_b_r8h_r16l);
	O(neg_b_r8l);
	O(nop);
	O(not_b_r8l);
	O(or_b_imm8_r8u); O(or_b_r8h_r8l);
	O(orc_imm8_ccr);
	O(rotl_b_r8l);
	O(rotr_b_r8l);
	O(rotxl_b_r8l);
	O(rotxr_b_r8l);
	O(rte);
	O(rts);
	O(shal_b_r8l);
	O(shar_b_r8l);
	O(shll_b_r8l);
	O(shlr_b_r8l);
	O(sleep);
	O(stc_ccr_r8l); O(stc_exr_r8l);
	O(sub_b_r8h_r8l); O(sub_w_r16h_r16l);
	O(subs_l_one_r16l); O(subs_l_two_r16l);
	O(subx_b_imm8_r8u); O(subx_b_r8h_r8l);
	O(xor_b_imm8_r8u); O(xor_b_r8h_r8l);
	O(xorc_imm8_ccr);

	O(dispatch_0100);
	O(dispatch_01007800);
	O(dispatch_0110);
	O(dispatch_0120);
	O(dispatch_0130);
	O(dispatch_0140);
	O(dispatch_01407800);
	O(dispatch_01407880);
	O(dispatch_0141);
	O(dispatch_01417800);
	O(dispatch_01417880);
	O(dispatch_0160);
	O(dispatch_01c0);
	O(dispatch_01d0);
	O(dispatch_01e0);
	O(dispatch_01f0);
	O(dispatch_6a10);
	O(dispatch_6a18);
	O(dispatch_6a30);
	O(dispatch_6a38);
	O(dispatch_7800);
	O(dispatch_7b5c);
	O(dispatch_7bd4);
	O(dispatch_7c00);
	O(dispatch_7d00);
	O(dispatch_7e00);
	O(dispatch_7f00);

	O(state_reset);
	O(state_irq);
	O(state_dma);
#undef O
};

enum {
	H8_PC  = 1,
	H8_R0,
	H8_R1,
	H8_R2,
	H8_R3,
	H8_R4,
	H8_R5,
	H8_R6,
	H8_R7,
	H8_E0,
	H8_E1,
	H8_E2,
	H8_E3,
	H8_E4,
	H8_E5,
	H8_E6,
	H8_E7,
	H8_CCR,
	H8_EXR
};

#endif // MAME_CPU_H8_H8_H
