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

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
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

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	void set_chr();
	u8 m_latch;
	u8 m_reg[3]; // mapper ram enable
	u8 m_mmc_vrom_bank[6];
	// Taito X1-017 chip contains 5K of internal ram, battery backed up
	u8 m_x1_017_ram[0x1400];

	u16 m_irq_count;
	u8 m_irq_count_latch;
	u8 m_irq_enable;

	emu_timer *irq_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TC0190FMC,         nes_tc0190fmc_device)
DECLARE_DEVICE_TYPE(NES_TC0190FMC_PAL16R4, nes_tc0190fmc_pal16r4_device)
DECLARE_DEVICE_TYPE(NES_X1_005,            nes_x1_005_device)
DECLARE_DEVICE_TYPE(NES_X1_017,            nes_x1_017_device)

#endif // MAME_BUS_NES_TAITO_H
