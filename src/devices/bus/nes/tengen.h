// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_TENGEN_H
#define MAME_BUS_NES_TENGEN_H

#include "nxrom.h"

class ppu2c0x_device;


// ======================> nes_tengen032_device

class nes_tengen032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tengen032_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	// scanline mode: every raw rise of PPU A12 is reported with its time and the time A12 last fell; the board
	// samples A12 itself on the falling edges of M2, so no rise is passed back to the PPU as accepted
	virtual bool wants_a12() override { return true; }
	virtual void ppu_a12_rise() override {}
	virtual bool wants_a12_filter() override { return true; }
	virtual bool a12_rise_accepted(const attotime& low_since, const attotime& when) override;
	virtual bool wants_a12_level() override { return true; }
	virtual void ppu_a12_level(int state) override { m_a12_level = state; }

	// cycle mode prescaler, the A12 sampling clock and the IRQ output all run on the falling edges of M2
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

	virtual void pcb_reset() override;

protected:
	nes_tengen032_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void update_chr();
	virtual void update_mirroring();

	u8 m_ctrl;          // $8000: C (CHR A12 inversion), P (PRG mode), K (1 KiB CHR mode), RRRR
	u8 m_reg[16];       // R0-R9 and RF; RA-RE have no function
	u8 m_mirror;        // $A000 bit 0
	u8 m_prg_mask;      // PRG bank bits that reach the PRG ROM
	u8 m_chr_mask;      // CHR bank bits that reach the CHR ROM
	int m_a12_level;    // PPU A12 on the bus, reported by the PPU before each access that changes it
	ppu2c0x_device* m_ppu_dev = nullptr;

private:
	void update_prg();
	void irq_clock();
	void a12_sample_high(const attotime& fall, u8 low_falls);

	// M2 falling edge grid
	s64 rel_as(const attotime& t) const;
	attotime fall_time(s64 k) const;
	s64 falls_in(const attotime& after, const attotime& upto) const;

	u8 m_irq_latch;
	u8 m_irq_count;
	bool m_irq_enable;
	bool m_irq_cpu_mode;
	u8 m_irq_prescaler;
	bool m_irq_reload;          // $C001 was written: the next clock loads the latch
	bool m_irq_swallow;         // $C001 was written with A12 low for 16 M2 falls: the next clock does nothing
	bool m_irq_assert_next;     // the counter fired: /IRQ goes low at the next falling edge of M2
	bool m_irq_assert_rise;     // /IRQ went low just after the last falling edge: the CPU sees it from the next cycle

	attotime m_m2_fall;         // time of the latest falling edge of M2
	attotime m_m2_period;       // M2 period, measured between falling edges

	attotime m_a12_rise;        // latest rise of A12 reported by the PPU
	attotime m_a12_high_fall;   // latest falling edge of M2 that saw A12 high
	bool m_a12_pending;         // a rise waits for the first M2 fall after it
	attotime m_a12_pending_fall;
	u8 m_a12_pending_lows;

	bool m_c001_open;           // A12's state at the last $C001 write is not known yet
	attotime m_c001_time;
	int m_c001_level;
};


// ======================> nes_tengen037_device

class nes_tengen037_device : public nes_tengen032_device
{
public:
	// construction/destruction
	nes_tengen037_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

	// device-level overrides
	virtual void write_h(offs_t offset, u8 data) override;

	virtual u8 nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, u8 data) override;

protected:
	virtual void update_chr() override;
	virtual void update_mirroring() override {}   // CIRAM A10 comes from CHR A17, see update_chr

private:
	int ciram_a10(int slot) const;
	int nt_page(offs_t offset) const;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device)
DECLARE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device)

#endif // MAME_BUS_NES_TENGEN_H
