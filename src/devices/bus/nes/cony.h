// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_CONY_H
#define MAME_BUS_NES_CONY_H

#pragma once

#include "nxrom.h"


// ======================> nes_cony_device

class nes_cony_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cony_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_cony_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 extra_addr, u8 mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg();
	virtual void set_chr();

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

	u16 m_irq_count;
	int m_irq_enable;


	u8 m_mmc_prg_bank[4];
	u8 m_mmc_vrom_bank[8];
	u8 m_extra_ram[4];
	const u16 m_extra_addr;
	const u8 m_mask;
	u8 m_mode_reg;
	u8 m_outer_reg;
};


// ======================> nes_cony1k_device

class nes_cony1k_device : public nes_cony_device
{
public:
	// construction/destruction
	nes_cony1k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void set_chr() override;
};


// ======================> nes_yoko_device

class nes_yoko_device : public nes_cony_device
{
public:
	// construction/destruction
	nes_yoko_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_CONY,   nes_cony_device)
DECLARE_DEVICE_TYPE(NES_CONY1K, nes_cony1k_device)
DECLARE_DEVICE_TYPE(NES_YOKO,   nes_yoko_device)

#endif // MAME_BUS_NES_CONY_H
