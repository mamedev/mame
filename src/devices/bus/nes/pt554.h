// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_PT554_H
#define MAME_BUS_NES_PT554_H

#pragma once

#include "nxrom.h"
#include "sound/samples.h"


// ======================> nes_bandai_pt554_device

class nes_bandai_pt554_device : public nes_cnrom_device
{
public:
	// construction/destruction
	nes_bandai_pt554_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<samples_device> m_samples;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_BANDAI_PT554, nes_bandai_pt554_device)

#endif // MAME_BUS_NES_PT554_H
