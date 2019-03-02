// license:BSD-3-Clause
// copyright-holders:Kaz
#ifndef MAME_BUS_NES_ZEMINA_H
#define MAME_BUS_NES_ZEMINA_H

#include "nxrom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zemina_device

class nes_zemina_device :
		public nes_nrom_device
{
public:
	// construction/destruction
	nes_zemina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ZEMINA, nes_zemina_device)

#endif // MAME_BUS_NES_ZEMINA_H
