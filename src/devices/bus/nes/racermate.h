// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_RACERMATE_H
#define MAME_BUS_NES_RACERMATE_H

#pragma once

#include "nxrom.h"


// ======================> nes_racermate_device
class m6502_device;

class nes_racermate_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_racermate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void ppu_to_mapper(int scanline, unsigned dot, int ppu_tick, uint16_t ppu_address) override;

	virtual void pcb_reset() override;

private:
	void update_banks();
	void clock_irq_counter();

	uint8_t m_latch;
	uint16_t m_irq_count;
	uint8_t m_irq_ppu_divider;
	uint8_t m_irq_delay;
	bool m_irq_enabled;
	
	m6502_device *m_maincpu6502;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_RACERMATE, nes_racermate_device)

#endif // MAME_BUS_NES_RACERMATE_H