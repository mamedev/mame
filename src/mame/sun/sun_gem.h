// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_SUN_SUN_GEM_H
#define MAME_SUN_SUN_GEM_H

#pragma once

class sun_gem_device : public device_t
{
public:
	sun_gem_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void sun_gem(machine_config &config) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SUN_GEM, sun_gem_device)

#endif // MAME_SUN_SUN_GEM_H
