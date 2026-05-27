// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 56303

#ifndef MAME_CPU_DSP563XX_DSP56303_H
#define MAME_CPU_DSP563XX_DSP56303_H

#pragma once

#include "dsp563xx.h"

class dsp56303_device : public dsp563xx_device {
public:
	dsp56303_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

DECLARE_DEVICE_TYPE(DSP56303, dsp56303_device)

#endif
