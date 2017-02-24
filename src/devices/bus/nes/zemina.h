// license:BSD-3-Clause
// copyright-holders:Kaz
#ifndef __NES_ZEMINA_H
#define __NES_ZEMINA_H

#include "nxrom.h"

// ======================> nes_zemina_device

class nes_zemina_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_zemina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};

// device type definition
extern const device_type NES_ZEMINA;

#endif