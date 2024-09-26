// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_MEDIAGX_H
#define MAME_VIDEO_PC_VGA_MEDIAGX_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class mediagx_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	mediagx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void crtc_map(address_map &map) override ATTR_COLD;

//  virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(MEDIAGX_VGA, mediagx_vga_device)

#endif // MAME_VIDEO_PC_VGA_MEDIAGX_H
