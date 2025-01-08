// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_RACERMATE_H
#define MAME_BUS_NES_RACERMATE_H

#include "nxrom.h"


// ======================> nes_racermate_device

class nes_racermate_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_racermate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

private:
	void update_banks();
	uint8_t m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_RACERMATE, nes_racermate_device)

#endif // MAME_BUS_NES_RACERMATE_H
