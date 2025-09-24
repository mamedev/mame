// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_SUN_FCNC_270_4373_07_H
#define MAME_SUN_FCNC_270_4373_07_H

#pragma once

class sun_fcnc_270_4373_07_device : public device_t
{
public:
	sun_fcnc_270_4373_07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void sun_fcnc_270_4373_07(machine_config &config) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SUN_FCNC_270_4373_07, sun_fcnc_270_4373_07_device)

#endif // MAME_SUN_FCNC_270_4373_07_H
