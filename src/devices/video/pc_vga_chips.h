// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_VIDEO_PC_VGA_CHIPS_H
#define MAME_VIDEO_PC_VGA_CHIPS_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class f65535_vga_device :  public svga_device
{
public:
	// preliminary, doesn't boot if mounted
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	f65535_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(F65535_VGA, f65535_vga_device)

#endif // MAME_VIDEO_PC_VGA_CHIPS_H
