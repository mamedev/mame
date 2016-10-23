// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NANJING_H
#define __NES_NANJING_H

#include "nxrom.h"


// ======================> nes_nanjing_device

class nes_nanjing_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nanjing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	uint8_t m_count;
	uint8_t m_reg[2];
	uint8_t m_latch1, m_latch2;
};





// device type definition
extern const device_type NES_NANJING;

#endif
