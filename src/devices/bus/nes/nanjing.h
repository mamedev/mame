// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_NANJING_H
#define MAME_BUS_NES_NANJING_H

#pragma once

#include "nxrom.h"


class ppu2c0x_device;

// ======================> nes_nanjing_device

class nes_nanjing_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nanjing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~nes_nanjing_device();

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_count;
	uint8_t m_reg[2];
	uint8_t m_latch1, m_latch2;

	required_device<ppu2c0x_device> m_ppu;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NANJING, nes_nanjing_device)

#endif // MAME_BUS_NES_NANJING_H
