// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Brian H. Gilbert
/***************************************************************************

    rp2a03.h

    6502, NES variant

***************************************************************************/
#ifndef MAME_CPU_M6502_RP2A03_H
#define MAME_CPU_M6502_RP2A03_H

#pragma once

#include "m6502.h"

#include <functional>
#include "sound/nes_apu.h"

class rp2a03_core_device : public m6502_device {
public:
	rp2a03_core_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	// DMA unit: called by the generated core for every cycle it spends halted (RDY low) on a read of adr
	virtual void dma_halt_cycle(uint16_t adr) override;

	// last value on the external data bus: what reads of addresses nothing drives return (CPU open bus)
	u8 open_bus() const { return m_bus; }

	// External M2 pin (NESdev "CPU pinout").  A CPU cycle begins when M2 falls; M2 rises m_m2_low/m_m2_period of the
	// way through the cycle and falls again when the cycle ends.  Every cycle, including the cycles the DMA unit
	// spends with the CPU halted, is one M2 pulse.  The access of the cycle (read, write, opcode fetch, DMA transfer)
	// is made at the rising edge, so every device it reaches, the PPU registers included, sees scheduler().time() at
	// M2 rise, and the optional callback gets the rising edge before the access and the falling edge after it, each at
	// its true time.  Nothing is scheduled: the edges are visited in order inside the CPU's own execution, so no timer
	// perturbs device synchronisation.  M2 is high for 15/24 of a cycle on the 2A03E/G/H (default), 17/24 on the
	// original letterless 2A03 and 19/32 on the 2A07 (PAL).
	void set_m2_waveform(u32 low, u32 period) { m_m2_low = low; m_m2_period = period; if (started()) m2_update_rise(); }
	// Where in the cycle a write is made, in the units of set_m2_waveform (0: at the M2 rise).  NESdev "CPU pinout": for
	// writes the value appears on the data bus during phi2 and no sooner, and phi2 does not move with the M2 duty cycle.
	// The write timing of the devices on the bus was established on the 2A03G, whose M2 rises 9/24 of the way through
	// the cycle (1.5 master clocks before phi2, as on the 2A07); a chip whose M2 rises earlier still writes there.
	void set_m2_write_point(u32 low) { m_m2_write_low = low; if (started()) m2_update_rise(); }
	const attotime &m2_write_offset() const { return m_m2_write_at; }
	// The access is only moved to the M2 phase on machines that ask for it (the NES driver does); every other RP2A03
	// machine keeps the access at the start of the cycle.  With read sampling at the fall, reads are made at the end of
	// the cycle, where the CPU latches the data, while writes still happen at the rise.
	void set_m2_phase(bool on) { m_m2_phase = on; }
	void set_m2_read_sample_at_fall(bool on) { m_m2_read_at_fall = on; }
	// 2A07 (PAL): the DMA unit does not repeat the halted CPU's read (NESdev "DMA", Register conflicts)
	void set_dma_2a07(bool on) { m_dma_2a07 = on; }
	bool m2_state() const { return m_m2_state; }
	// address and R/W of the access of the current cycle; they are driven before M2 rises and held until it falls
	u16 m2_address() const { return m_m2_addr; }
	bool m2_is_write() const { return m_m2_write; }
	const attotime &m2_rise_offset() const { return m_m2_rise; }
	u32 m2_low() const { return m_m2_low; }
	u32 m2_period() const { return m_m2_period; }
	template <typename... T> void set_m2_callback(T &&... args) { m_m2_cb.set(std::forward<T>(args)...); m_m2_cb.resolve(); }
	// NMI edge detector (NESdev "CPU interrupts"): given the level of /NMI as a function of time (true: low), /NMI is
	// sampled when phi2 falls at the end of every cycle, and a high sample followed by a low one raises the internal NMI
	// signal for the polls of the following cycles; the NMI input line is not used then
	void set_nmi_level_func(std::function<bool (const attotime &)> &&func) { m_nmi_level = std::move(func); }
	// IRQ level detector (NESdev "CPU interrupts"), always on in the 2A03: IRQ is sampled when phi2 falls at the end of
	// every cycle and the polls of the following cycle see that sample, rather than the level at the moment of the poll
	// time at which the IRQ assertion that follows really happened, for a source that only finds out later (the PPU
	// brings itself up to date lazily, after the A12 edge an MMC3 counts); if the last phi2 sample was taken after it,
	// that sample saw IRQ low
	void set_irq_assert_time(const attotime &when) { m_irq_assert_at = when; m_irq_reported_at = machine().time(); }

protected:
	rp2a03_core_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;

	void m2_update_rise()
	{
		m_m2_rise = attotime::from_ticks(m_m2_low, clock() * m_m2_period);
		m_m2_write_at = (m_m2_write_low > m_m2_low) ? attotime::from_ticks(m_m2_write_low, clock() * m_m2_period) : m_m2_rise;
	}
	// A pulse is only opened by the outermost access, so an access made from inside another one (a handler that reads
	// memory itself) does not produce a second pair of edges.
	void m2_begin(u16 adr, bool write)
	{
		if (m_m2_depth++ != 0)
			return;
		m_m2_addr = adr;
		m_m2_write = write;
		if (m_m2_phase)
			set_bus_offset(m_m2_rise);
		m_m2_state = true;
		if (!m_m2_cb.isnull())
			m_m2_cb(1, adr, write);
		// reads are sampled at the falling edge: the data only has to be valid there (NESdev "CPU pinout")
		if (m_m2_phase && m_m2_read_at_fall && !write)
			set_bus_offset(cycles_to_attotime(1));
		// a write lands where its data is on the bus, which can be after M2 has risen (see set_m2_write_point)
		if (m_m2_phase && write && (m_m2_write_at > m_m2_rise))
			set_bus_offset(m_m2_write_at);
	}
	void m2_end()
	{
		if (m_m2_depth <= 0 || --m_m2_depth != 0)
			return;
		if (m_m2_phase)
			set_bus_offset(cycles_to_attotime(1));
		if (!m_m2_cb.isnull())
			m_m2_cb(0, m_m2_addr, m_m2_write);
		m_m2_state = false;
		set_bus_offset(attotime::zero);
	}

	// DMA unit, cycle by cycle as TriCNES models the 2A03: a pending DMA turns every CPU read cycle into a halted, get
	// or put cycle; the APU side runs after the CPU's access of each cycle (cycle_end) and decides RDY for the next one
	void oam_dma_request(u8 page);
	bool dma_get_cycle(u64 cycle) const;
	void cycle_end();
	void dmc_request(bool load);
	virtual void dmc_dma_done(u8 data) { }
	virtual void apu_cycle_end(bool get) { }          // the APU's part of the CPU cycle
	virtual bool dmc_dma_enabled() { return true; }    // DMC status (or the implicit-abort window) lets a DMC DMA run
	virtual void dmc_implicit_clear() { }
	virtual u16 dmc_dma_address() { return 0; }

	bool m_oam_active = false;      // DoOAMDMA
	bool m_oam_first = false;       // the next DMA cycle is the sprite DMA's first
	bool m_oam_halt = false;        // OAMDMA_Halt
	bool m_oam_aligned = false;     // OAMDMA_Aligned: a byte was read and waits for a put cycle
	u8 m_oam_page = 0;
	u16 m_oam_index = 0;
	u8 m_oam_byte = 0;
	bool m_dmc_req = false;         // DoDMCDMA
	bool m_dmc_halt = false;        // DMCDMA_Halt
	u8 m_dmc_block = 0;             // CannotRunDMCDMARightNow: no new timer DMC DMA right after a DMC get
	int m_last_dummy_adr = -1;      // address of the read the halted CPU repeated on the previous cycle (-1: none)
	bool m_dma_2a07 = false;        // 2A07: no extra reads of registers from the halted CPU
	u8 m_bus = 0;                   // last value on the external data bus
	u8 m_ibus = 0;                  // last value on the CPU's internal data bus: every CPU read/write, never a DMA cycle
	bool m_m2_state = false;        // M2 is high: the access of this cycle is in progress
	std::function<bool (const attotime &)> m_nmi_level;   // /NMI level at a given time, for the edge detector
	bool m_nmi_sample = false;      // /NMI was low at the last phi2 fall
	attotime m_last_phi2;           // when phi2 last fell (end of the last cycle executed)
	attotime m_irq_assert_at = attotime::never;
	attotime m_irq_reported_at;     // when m_irq_assert_at was reported
	u32 m_m2_low = 9;               // M2 is low for m_m2_low / m_m2_period of each cycle (2A03G: 9/24)
	u32 m_m2_period = 24;
	u32 m_m2_write_low = 0;         // writes are made m_m2_write_low / m_m2_period into the cycle (0: at the M2 rise)
	attotime m_m2_rise;             // time of the rising edge within the cycle
	attotime m_m2_write_at;         // time within the cycle at which a write is made (the rising edge unless set later)
	u16 m_m2_addr = 0;              // address on the bus during the current M2 pulse
	bool m_m2_write = false;        // R/W during the current M2 pulse
	int m_m2_depth = 0;             // nesting depth of accesses inside the current pulse
	bool m_m2_phase = false;        // make the access at the M2 phase instead of at the start of the cycle
	bool m_m2_read_at_fall = false; // sample reads at the M2 fall instead of the rise
	using m2_delegate = device_delegate<void (int, offs_t, bool)>;
	m2_delegate m_m2_cb;

	// memory interface that keeps track of the external data bus; $4015 reads are internal to the 2A03 and
	// never reach it
	class mi_2a03 : public memory_interface
	{
	public:
		mi_2a03(rp2a03_core_device &cpu) : m_cpu(cpu) { }
		virtual uint8_t read(uint16_t adr) override { m_cpu.m2_begin(adr, false); const uint8_t val = track(adr, m_program.read_interruptible(adr)); m_cpu.m2_end(); m_cpu.cycle_end(); return val; }
		virtual uint8_t read_sync(uint16_t adr) override { m_cpu.m2_begin(adr, false); const uint8_t val = track(adr, m_csprogram.read_interruptible(adr)); m_cpu.m2_end(); m_cpu.cycle_end(); return val; }
		virtual uint8_t read_arg(uint16_t adr) override { m_cpu.m2_begin(adr, false); const uint8_t val = track(adr, m_cprogram.read_interruptible(adr)); m_cpu.m2_end(); m_cpu.cycle_end(); return val; }
		virtual void write(uint16_t adr, uint8_t val) override { m_cpu.m2_begin(adr, true); m_cpu.m_bus = val; m_cpu.m_ibus = val; m_program.write_interruptible(adr, val); m_cpu.m2_end(); m_cpu.cycle_end(); }
	private:
		uint8_t track(uint16_t adr, uint8_t val) { m_cpu.m_ibus = val; if (adr != 0x4015) m_cpu.m_bus = val; return val; }
		rp2a03_core_device &m_cpu;
	};

#define O(o) void o ## _full(); void o ## _partial()

	// rp2a03 opcodes - same as 6502 with D disabled
	O(adc_nd_aba); O(adc_nd_abx); O(adc_nd_aby); O(adc_nd_idx); O(adc_nd_idy); O(adc_nd_imm); O(adc_nd_zpg); O(adc_nd_zpx);
	O(arr_nd_imm);
	O(isb_nd_aba); O(isb_nd_abx); O(isb_nd_aby); O(isb_nd_idx); O(isb_nd_idy); O(isb_nd_zpg); O(isb_nd_zpx);
	O(rra_nd_aba); O(rra_nd_abx); O(rra_nd_aby); O(rra_nd_idx); O(rra_nd_idy); O(rra_nd_zpg); O(rra_nd_zpx);
	O(sbc_nd_aba); O(sbc_nd_abx); O(sbc_nd_aby); O(sbc_nd_idx); O(sbc_nd_idy); O(sbc_nd_imm); O(sbc_nd_zpg); O(sbc_nd_zpx);

	// the 2A03's own copies of the NMOS instructions, generated with the interrupt poll points NESdev documents
	// (m6502make.py DOCUMENTED_POLL_DEVICES)

	// NMOS 6502 opcodes
	// documented opcodes
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

	// exceptions
	O(reset);

	// undocumented reliable instructions
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

	// undocumented unreliable instructions
	//   behaviour differs between visual6502 and online docs, which
	//   is a clear sign reliability is not to be expected
	//   implemented version follows visual6502
	O(anc_imm);
	O(ane_imm);
	O(arr_imm);
	O(asr_imm);
	O(las_aby);
	O(lxa_imm);

	// nop variants
	O(nop_imm); O(nop_aba); O(nop_abx); O(nop_zpg); O(nop_zpx);

	// system killers
	O(kil_non);


#undef O

private:
};

class rp2a03_device : public rp2a03_core_device, public device_mixer_interface {
public:
	rp2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rp2a03_map(address_map &map) ATTR_COLD;

	// $4014: sprite DMA, 256 reads from page data written to OAMDATA while the CPU is halted
	void oam_dma_w(u8 data);

	// RP2A03H and RP2A03G made from 1990 on: the DMC's unexpected reload DMA (NESdev "DMA", Bugs)
	void set_dmc_unexpected_dma(bool on) { m_apu->set_dmc_unexpected_dma(on); }

protected:
	rp2a03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<nesapu_device> m_apu;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void apu_irq(int state);
	uint8_t apu_read_mem(offs_t offset);
	void apu_dmc_dma(offs_t offset, u16 data);
	virtual void dmc_dma_done(u8 data) override;
	virtual void apu_cycle_end(bool get) override;
	virtual bool dmc_dma_enabled() override;
	virtual void dmc_implicit_clear() override;
	virtual u16 dmc_dma_address() override;
};

class rp2a03g_device : public rp2a03_device
{
public:
	rp2a03g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

/* These are the official XTAL values and clock rates used by Nintendo for
   manufacturing throughout the production of the 2A03. PALC_APU_CLOCK is
   the clock rate devised by UMC(?) for PAL Famicom clone hardware.        */

#define RP2A03_NTSC_XTAL           XTAL(21'477'272)
#define RP2A03_PAL_XTAL            XTAL(26'601'712)
#define NTSC_APU_CLOCK      (RP2A03_NTSC_XTAL/12) /* 1.7897726666... MHz */
#define PAL_APU_CLOCK       (RP2A03_PAL_XTAL/16) /* 1.662607 MHz */
#define PALC_APU_CLOCK      (RP2A03_PAL_XTAL/15) /* 1.77344746666... MHz */

enum {
	RP2A03_IRQ_LINE = m6502_device::IRQ_LINE,
	RP2A03_APU_IRQ_LINE = m6502_device::APU_IRQ_LINE,
	RP2A03_NMI_LINE = m6502_device::NMI_LINE,
	RP2A03_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(RP2A03_CORE, rp2a03_core_device)
DECLARE_DEVICE_TYPE(RP2A03,      rp2a03_device)
DECLARE_DEVICE_TYPE(RP2A03G,     rp2a03g_device)

#endif // MAME_CPU_M6502_RP2A03_H
