// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_HOSENKAN_H
#define MAME_BUS_NES_HOSENKAN_H

#pragma once

#include "nxrom.h"


// ======================> nes_hosenkan_device

class nes_hosenkan_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hosenkan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint16_t m_irq_count, m_irq_count_latch;
	uint8_t m_irq_clear;
	int m_irq_enable;

	uint8_t m_latch;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_HOSENKAN, nes_hosenkan_device)

#endif // MAME_BUS_NES_HOSENKAN_H
