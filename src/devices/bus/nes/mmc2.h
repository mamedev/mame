// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC2_H
#define MAME_BUS_NES_MMC2_H

#pragma once

#include "nxrom.h"


// ======================> nes_pxrom_device

class nes_pxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_pxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pxrom_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { pxrom_write(offset, data); }

	virtual void ppu_latch(offs_t offset) override;
	virtual bool wants_read_latch() override { return true; }
	virtual void ppu_read_latch(offs_t offset) override;
	// MMC2: latch 0 responds only to $0FD8/$0FE8; MMC4: to the ranges $0FD8-$0FDF/$0FE8-$0FEF
	virtual bool latch0_is_range() const { return false; }
	virtual void pcb_reset() override;

protected:
	nes_pxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	uint8_t m_reg[4];
	int m_latch1, m_latch2;
};


// ======================> nes_fxrom_device

class nes_fxrom_device : public nes_pxrom_device
{
public:
	// construction/destruction
	nes_fxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual bool latch0_is_range() const override { return true; }

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_PXROM, nes_pxrom_device)
DECLARE_DEVICE_TYPE(NES_FXROM, nes_fxrom_device)

#endif // MAME_BUS_NES_MMC2_H
