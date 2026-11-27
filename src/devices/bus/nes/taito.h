// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_TAITO_H
#define MAME_BUS_NES_TAITO_H

#pragma once

#include "nxrom.h"


// ======================> nes_tc0190fmc_device

class nes_tc0190fmc_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tc0190fmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_tc0190fmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_tc0190fmc_pal16r4_device

class nes_tc0190fmc_pal16r4_device : public nes_tc0190fmc_device
{
public:
	// construction/destruction
	nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	// the counter is timed from the filtered PPU A12 rise the PPU reports (see the board's banner in taito.cpp);
	// hblank_irq() is the fallback for a PPU that is not running the dot renderer
	virtual bool wants_a12() override { return true; }
	virtual void ppu_a12_rise() override;
	virtual bool wants_a12_filter() override { return true; }
	virtual bool a12_rise_accepted(const attotime &low_since, const attotime &when) override;
	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void irq_clock();
	void update_irq();
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;
	u8 m_irq_delay = 0;     // CPU cycles left before the counter is clocked
	bool m_irq_pending = false;

	u8 m_irq_count, m_irq_count_latch;
	u8 m_irq_enable;
};


// ======================> nes_x1_005_device

class nes_x1_005_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_005_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
	// Taito X1-005 chip contains 80 bytes of internal ram, possibly battery backed up
	u8 m_x1_005_ram[0x80];
};


// ======================> nes_x1_017_device

class nes_x1_017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_017_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_ex(offs_t offset) override { return 0; } // no open bus
	virtual u8 read_l(offs_t offset) override { return 0; } // no open bus
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
	virtual void set_x1_017_old_order(bool val) override { m_old_order = val; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter is clocked by M2: it is counted at the M2 rise, before that cycle's CPU access
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_clock(); }
	void irq_clock();

private:
	bool m_old_order = false;  // iNES mapper 82 dumps: PRG in the pre-2020 bank order
	void set_chr();
	u8 m_latch;
	u8 m_reg[3]; // mapper ram enable
	u8 m_mmc_vrom_bank[6];
	// Taito X1-017 chip contains 5K of internal ram, battery backed up
	u8 m_x1_017_ram[0x1400];

	u16 m_irq_count;
	u8 m_irq_count_latch;
	u8 m_irq_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TC0190FMC,         nes_tc0190fmc_device)
DECLARE_DEVICE_TYPE(NES_TC0190FMC_PAL16R4, nes_tc0190fmc_pal16r4_device)
DECLARE_DEVICE_TYPE(NES_X1_005,            nes_x1_005_device)
DECLARE_DEVICE_TYPE(NES_X1_017,            nes_x1_017_device)

#endif // MAME_BUS_NES_TAITO_H
