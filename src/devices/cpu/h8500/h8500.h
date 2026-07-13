// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR, Olivier Galibert
/***************************************************************************

	h8500.h

	Hitachi H8/500 family base CPU device.

***************************************************************************/

#ifndef MAME_CPU_H8500_H8500_H
#define MAME_CPU_H8500_H8500_H

#pragma once

#include "cpu/h8/h8_cpu_base.h"
#include "cpu/h8/h8_sci.h"

class h8500_device : public h8_cpu_base
{
public:
	enum {
		H8500_PC,
		H8500_SR, H8500_CCR,
		H8500_CP, H8500_DP, H8500_EP, H8500_TP,
		H8500_BR,
		H8500_R0, H8500_R1, H8500_R2, H8500_R3,
		H8500_R4, H8500_R5, H8500_FP, H8500_SP
	};

	void set_mode(u8 mode) { assert(!configured()); m_mode_control = mode; }

	u8 do_read_port(int port) override { return m_read_port[port](); }
	void do_write_port(int port, u8 data, u8 ddr) override { m_write_port[port](0, data, ddr); }
	void do_sci_tx(int sci, int state) override { m_sci_tx[sci](state); }
	void do_sci_clk(int sci, int state) override { m_sci_clk[sci](state); }

	u64 system_clock() const override { return execute_clocks_to_cycles(clock()); }

	void internal_update() override;
	virtual void internal_update(u64 current_time) = 0;
	virtual void notify_standby(int state) = 0;

	auto standby_cb() { return m_standby_cb.bind(); } // notifier (not an output pin)
	int standby() override { return suspended(SUSPEND_REASON_CLOCK) ? 1 : 0; }
	u64 standby_time() override { return m_standby_time; }

	// h8_cpu_base virtuals that the H8/500 core doesn't model yet.
	// These return sensible defaults so unmigrated peripherals that
	// query them get safe behaviour; replace with real implementations
	// as the corresponding H8/500 feature is wired up.  The H8/510 (and
	// other H8/500 parts) do have an ADC and DTC on-chip - they just
	// aren't routed through these helpers yet.
	u64  now_as_cycles() const override { return machine().time().as_ticks(system_clock()); }
	u16  do_read_adc(int port) override { return m_read_adc[port & 7](); }
	void set_dma_channel(h8_dma_state * /*state*/) override {}
	void set_current_dtc(h8_dtc_state * /*state*/) override {}

	template<int Port> auto read_adc() { return m_read_adc[Port].bind(); }
	template<int Sci> auto write_sci_tx() { return m_sci_tx[Sci].bind(); }
	template<int Sci> auto write_sci_clk() { return m_sci_clk[Sci].bind(); }

	void sci_set_external_clock_period(int sci, const attotime &period) {
		m_sci[sci].lookup()->do_set_external_clock_period(period);
	}

	template<int Sci> void sci_rx_w(int state) { m_sci[Sci]->do_rx_w(state); }
	template<int Sci> void sci_clk_w(int state) { m_sci[Sci]->do_clk_w(state); }

	void set_irq(int irq_vector, int irq_level, bool irq_nmi) override;
	bool trigger_dma(int vector) override;
	void update_active_dma_channel() override;
	void request_state(int state) override;
	bool access_is_dma() const override { return m_inst_state == STATE_DMA || m_inst_state == STATE_DTC; }

protected:
	// Execution states, must match what's in the .lst file
	enum {
		STATE_RESET = 0x10000,
		STATE_IRQ   = 0x10001,
		STATE_DMA   = 0x10002,
		STATE_DTC   = 0x10003
	};

	// Status register bit layout
	enum : u16 {
		SR_C  = 0x0001,
		SR_V  = 0x0002,
		SR_Z  = 0x0004,
		SR_N  = 0x0008,
		SR_I0 = 0x0100,
		SR_I1 = 0x0200,
		SR_I2 = 0x0400,
		SR_T  = 0x8000,

		// bits 14-11 and 7-4 are reserved: they cannot be written and
		// always read as 0 (Programming Manual section 1.3.3)
		SR_MASK = SR_T | SR_I2 | SR_I1 | SR_I0 | SR_N | SR_Z | SR_V | SR_C
	};

	// Bit position in the EA prefix byte (m_ir[0]) that selects the
	// operation size for general-form opcodes: 0 -> byte, 1 -> word.
	// Use `BIT(m_ir[0], OP_WIDTH)` in opcode bodies rather than hand-
	// rolling `m_ir[0] & 0x08`.
	static constexpr u8 OP_WIDTH = 3;

	h8500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, int buswidth, int ramsize, int defmode, address_map_constructor map);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks / 2); }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 32; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	u8 mode_control() const { return m_mode_control; }
	virtual bool h8_maximum_mode() const noexcept { return m_mode_control == 3 || m_mode_control == 4; } // all except H8/510 and 570

	// Instruction-fetch memory accessors (read from the code page).
	u8  read8i(u32 adr);
	u16 read16i(u32 adr);

	// Reads the next byte or word at the current PC and advance the PC.
	u8  read_imm8();
	u16 read_imm16();

	// General memory accessors
	u8   read8(u32 adr);
	void write8(u32 adr, u8 data);
	u16  read16(u32 adr);
	void write16(u32 adr, u16 data);

	// Cycle-count adjustments.
	void internal(int cycles) { m_icount -= cycles; }

	// Dispatch / prefetch state-machine hooks.
	void prefetch_done();
	void prefetch_done_noirq();
	void prefetch_switch(u32 pc, u8 ir) { m_npc = pc & 0xffffff; m_pc = pc + 1; m_pir = ir; }

	virtual void take_interrupt();
	virtual void interrupt_taken() {}
	virtual void irq_setup() {}
	virtual void update_irq_filter() {}
	virtual void illegal();

	static void add_event(u64 &event_time, u64 new_event);
	void recompute_bcount(u64 event_time);

	// Helper to get the current 24-bit CP:PC address
	u32 pc24() const { return (u32(m_cp) << 16) | m_pc; }

	// Effective address prefix helpers
	// Get the address of the current EA prefix, assuming it's not register direct
	u32 ea_addr();
	// Commit the current EA read/write operation once it's safe to do so (we're cycle-by-cycle initerruptable)
	void ea_commit();
	// Read/write the value at the current effective address
	u8   read_ea_8();
	void write_ea_8(u8 val);
	u16  read_ea_16();
	void write_ea_16(u16 val);

	// For instructions with an EA prefix, this returns the actual
	// opcode byte that follows the prefix.
	u8 op_byte() const { return m_inst_state & 0xff; }

	// Register access helpers
	u8 r8_r(int reg) const { return m_r[reg & 7] & 0xff; }
	void r8_w(int reg, u8 val) {
		m_r[reg & 7] &= 0xff00;
		m_r[reg & 7] |= val;
	}
	u16  r16_r(int reg) const  { return m_r[reg & 7]; }
	void r16_w(int reg, u16 v) { m_r[reg & 7] = v; }

	virtual void do_exec_full();
	virtual void do_exec_partial();

	enum
	{
		// digital I/O ports
		PORT_1,
		PORT_2,
		PORT_3,
		PORT_4,
		PORT_5,
		PORT_6,
		PORT_7,
		PORT_8,
		PORT_9,
		PORT_COUNT
	};

	static const char port_names[];
	u16 m_pc;		// Current program counter - may not be the start of the executing instruction due to prefetch
	u16 m_ppc;		// Previous program counter, the start of the currently executing instruction
	u16 m_sr;		// Status register (low 8 bits are CCR)
	u8 m_cp;		// Code page, the top 8 bits of the 24-bit program counter in maximum modes
	u8 m_dp;		// Data page, the top 8 bits of the address for accesses relative to R0-R3 in maximum modes
	u8 m_ep;		// Extra page, like DP but for accesses relative to R4-R5
	u8 m_tp;		// sTack page, the top 8 bits of the address for accesses relative to R6 (FP) and R7 (SP)
	u8 m_br;		// Base register, used as bits 15-8 of the address for "zero page" like instructions
	u16 m_r[8];

	// ---- Dispatch / prefetch working state ----
	u32 m_npc;            // 24-bit address of the just-prefetched byte
	u8  m_pir;            // Prefetched byte
	u8  m_ir[8];          // Current instruction bytes (includes EA prefix and opcode, not any immediates aferwards)
	u8  m_ea_op_bytes;    // Length of current EA prefix

	// EA address cache, used to avoid recalculating (and re-pre/post inc/decrementing registers) on RMW opcodes
	u32  m_ea_addr_cache;
	bool m_ea_addr_cached;
	bool m_ea_committed;

	u32 m_tmp1, m_tmp2, m_tmp3, m_tmp4;   // temporary registers used by the opcode handlers

	int m_inst_state, m_inst_substate, m_requested_state;
	int  m_irq_vector, m_taken_irq_vector;
	int  m_irq_level, m_taken_irq_level;
	bool m_irq_nmi, m_standby_pending;
	u64 m_standby_time;

	int m_icount, m_bcount;
	int m_count_before_instruction_step;

	devcb_read16::array<8> m_read_adc;
	devcb_read8::array<PORT_COUNT> m_read_port;
	devcb_write8::array<PORT_COUNT> m_write_port;
	optional_device_array<h8_sci_device, 2> m_sci;
	devcb_write_line::array<3> m_sci_tx, m_sci_clk;
	devcb_write_line m_standby_cb;

#define O(o) void o ## _full(); void o ## _partial()
	O(nop);
	O(add);
	O(add_q);
	O(adds);
	O(addx);
	O(and);
	O(andc);
	O(bclr);
	O(bclr_imm);
	O(bnot);
	O(bnot_imm);
	O(bset);
	O(bset_imm);
	O(bsr);
	O(bsr_rel16);
	O(btst_imm);
	O(btst_rn);
	O(bt);
	O(bf);
	O(bhi);
	O(bls);
	O(bcc);
	O(bcs);
	O(bne);
	O(beq);
	O(bvc);
	O(bvs);
	O(bpl);
	O(bmi);
	O(bge);
	O(blt);
	O(bgt);
	O(ble);
	O(bt_w);
	O(bf_w);
	O(bhi_w);
	O(bls_w);
	O(bcc_w);
	O(bcs_w);
	O(bne_w);
	O(beq_w);
	O(bvc_w);
	O(bvs_w);
	O(bpl_w);
	O(bmi_w);
	O(bge_w);
	O(blt_w);
	O(bgt_w);
	O(ble_w);
	O(clr);
	O(cmp_b);
	O(cmp_w);
	O(cmp_ea_8);
	O(cmp_ea_16);
	O(cmp_ea_rd);
	O(dadd);
	O(divxu);
	O(exts);
	O(extu);
	O(jmp);
	O(jmp_d8r);
	O(jmp_d16r);
	O(jmp_rn);
	O(jsr);
	O(jsr_d8r);
	O(jsr_d16r);
	O(jsr_rn);
	O(ldc);
	O(ldm);
	O(link);
	O(link_16);
	O(mov_b_imm);
	O(mov_w_imm);
	O(mov_b_imm_ea);
	O(mov_w_imm_ea);
	O(mov_aa8_rn);
	O(mov_rn_aa8);
	O(movf_load);
	O(movf_store);
	O(movg_r);
	O(movg_w);
	O(mulxu);
	O(neg);
	O(not);
	O(or);
	O(orc);
	O(pjmp);
	O(pjmp_rn);
	O(pjsr);
	O(pjsr_rn);
	O(prtd);
	O(prtd_16);
	O(prts);
	O(rotl);
	O(rotr);
	O(rotxl);
	O(rotxr);
	O(rte);
	O(rtd);
	O(rtd_16);
	O(rts);
	O(scb_f);
	O(scb_ne);
	O(scb_eq);
	O(shal);
	O(shar);
	O(shll);
	O(shlr);
	O(sleep);
	O(stc);
	O(stm);
	O(sub);
	O(subs);
	O(subx);
	O(swap);
	O(tas);
	O(trapa);
	O(trap_vs);
	O(tst);
	O(unlk);
	O(xch);
	O(xor);
	O(xorc);
	// Multi-byte standalone dispatch transitions (byte 0 -> byte 1)
	O(dispatch_00);
	O(dispatch_01);
	O(dispatch_06);
	O(dispatch_07);
	O(dispatch_08);
	O(dispatch_11);
	// Special states
	O(state_reset);
	O(state_irq);
	// Auto-generated EA prefix dispatchers (one per H8/500 EA range)
	O(ea_04); O(ea_0c);
	O(ea_05); O(ea_0d); O(ea_15); O(ea_1d);
	O(ea_a0); O(ea_b0); O(ea_c0); O(ea_d0);
	O(ea_e0); O(ea_f0);
#undef O

private:
	void debug_set_pc(offs_t pc) noexcept;

	// flags helpers
	u16 nz8(u8 val);
	u16 nz16(u16 val);
	u16 nz32(u32 val);

	// opcode helpers
	u8 do_add8(u8 a, u8 b);
	u16 do_add16(u16 a, u16 b);
	u8 do_sub8(u8 a, u8 b);
	u16 do_sub16(u16 a, u16 b);
	u8 do_shll8(u8 a);
	u16 do_shll16(u16 a);
	u8 do_shlr8(u8 a);
	u16 do_shlr16(u16 a);
	u8 do_rotl8(u8 v);
	u16 do_rotl16(u16 v);
	u8 do_rotr8(u8 v);
	u16 do_rotr16(u16 v);
	u8 do_addx8(u8 a, u8 b);
	u16 do_addx16(u16 a, u16 b);
	u16 do_adds16(u16 a, u16 b);
	u8 do_subx8(u8 a, u8 b);
	u16 do_subx16(u16 a, u16 b);
	u8 do_subs8(u8 a, u8 b);
	u16 do_subs16(u16 a, u16 b);
	u8 do_dadd8(u8 a, u8 b);
	u8 do_rotxl8(u8 v);
	u16 do_rotxl16(u16 v);
	u8 do_rotxr8(u8 v);
	u16 do_rotxr16(u16 v);
	u8 do_shal8(u8 v);
	u16 do_shal16(u16 v);
	u8 do_shar8(u8 v);
	u16 do_shar16(u16 v);

	u16 adc_default(int adc);
	u8 port_default_r(int port);
	void port_default_w(int port, u8 data);

	// address spaces
	address_space_config m_program_config;
	address_space *m_program;

	// misc. configuration
	u8 m_mode_control;
	[[maybe_unused]] u16 m_ram_size;
};

#endif // MAME_CPU_H8500_H8500_H
