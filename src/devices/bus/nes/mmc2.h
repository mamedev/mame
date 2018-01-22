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

	virtual DECLARE_WRITE8_MEMBER(pxrom_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { pxrom_write(space, offset, data, mem_mask); }

	virtual void ppu_latch(offs_t offset) override;
	virtual void pcb_reset() override;

protected:
	nes_pxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	uint8_t m_reg[4];
	int m_latch1, m_latch2;
};


// ======================> nes_fxrom_device

class nes_fxrom_device : public nes_pxrom_device
{
public:
	// construction/destruction
	nes_fxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_PXROM, nes_pxrom_device)
DECLARE_DEVICE_TYPE(NES_FXROM, nes_fxrom_device)

#endif // MAME_BUS_NES_MMC2_H
