// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PARADISE_H
#define MAME_VIDEO_PARADISE_H

#pragma once

#include "video/pc_vga.h"

class pvga1a_vga_device : public svga_device
{
public:
	pvga1a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void gc_map(address_map &map) override;

};

DECLARE_DEVICE_TYPE(PVGA1A, pvga1a_vga_device)

#endif // MAME_VIDEO_PARADISE_H
