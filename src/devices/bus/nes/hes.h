// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_HES_H
#define __NES_HES_H

#include "nxrom.h"


// ======================> nes_hes_device

class nes_hes_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hes_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// device type definition
extern const device_type NES_HES;

#endif
