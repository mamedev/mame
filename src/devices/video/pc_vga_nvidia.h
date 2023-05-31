// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_NVIDIA_H
#define MAME_VIDEO_PC_VGA_NVIDIA_H

#pragma once

#include "screen.h"
#include "video/pc_vga.h"

class nvidia_nv3_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	nvidia_nv3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint8_t port_03b0_r(offs_t offset) override;
	virtual void port_03b0_w(offs_t offset, uint8_t data) override;
//	virtual uint8_t port_03c0_r(offs_t offset) override;
//	virtual void port_03c0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t port_03d0_r(offs_t offset) override;
	virtual void port_03d0_w(offs_t offset, uint8_t data) override;
//	virtual uint8_t mem_r(offs_t offset) override;
//	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual uint8_t crtc_reg_read(uint8_t index) override;
	virtual void crtc_reg_write(uint8_t index, uint8_t data) override;

	virtual ioport_constructor device_input_ports() const override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual uint16_t offset() override;

private:
	u8 m_repaint[2]{};
	u16 m_ext_offset = 0;
};

DECLARE_DEVICE_TYPE(NVIDIA_NV3_VGA, nvidia_nv3_vga_device)

#endif // MAME_VIDEO_PC_VGA_NVIDIA_H
