// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_HES_H
#define MAME_BUS_NES_HES_H

#pragma once

#include "nxrom.h"


// ======================> nes_hes_device

class nes_hes_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hes_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_HES, nes_hes_device)

#endif // MAME_BUS_NES_HES_H
