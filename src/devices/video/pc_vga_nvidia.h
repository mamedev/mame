// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_NVIDIA_H
#define MAME_VIDEO_PC_VGA_NVIDIA_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class nvidia_nv3_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	nvidia_nv3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void crtc_map(address_map &map) override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual uint16_t offset() override;

private:
	u8 m_repaint[2]{};
	u16 m_ext_offset = 0;
};

DECLARE_DEVICE_TYPE(NVIDIA_NV3_VGA, nvidia_nv3_vga_device)

#endif // MAME_VIDEO_PC_VGA_NVIDIA_H
