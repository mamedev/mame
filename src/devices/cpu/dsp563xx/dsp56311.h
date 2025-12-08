// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56311

#ifndef MAME_CPU_DSP563XX_DSP56311_H
#define MAME_CPU_DSP563XX_DSP56311_H

#pragma once

#include "dsp563xx.h"

class dsp56311_device : public dsp563xx_device {
public:
	dsp56311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u32 get_reset_vector() const override;

	void p_map(address_map &map);
	void x_map(address_map &map);
	void y_map(address_map &map);
};

DECLARE_DEVICE_TYPE(DSP56311, dsp56311_device)

#endif
