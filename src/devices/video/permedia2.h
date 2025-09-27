// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_VIDEO_PERMEDIA2_H
#define MAME_VIDEO_PERMEDIA2_H

#pragma once

class permedia2_device : public device_t
{
public:
	permedia2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void permedia2(machine_config &config) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PERMEDIA2, permedia2_device)

#endif // MAME_VIDEO_PERMEDIA2_H
