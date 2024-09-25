// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_WD902C26_H
#define MAME_VIDEO_WD902C26_H

#pragma once

#include "video/pc_vga_paradise.h"

class wd90c26_vga_device : public wd90c11a_vga_device
{
public:
	wd90c26_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void gc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(WD90C26, wd90c26_vga_device)

#endif // MAME_VIDEO_WD902C26_H
