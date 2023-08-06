// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_MATROX_H
#define MAME_VIDEO_PC_VGA_MATROX_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"

class matrox_vga_device :  public svga_device
{
public:
	matrox_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ramdac_ext_map(address_map &map);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;
protected:
	virtual void io_3bx_3dx_map(address_map &map) override;

	virtual void device_reset() override;

	void crtcext_map(address_map &map);
	virtual uint16_t offset() override;
private:
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_crtcext_space_config;
	u8 ramdac_ext_indexed_r();
	void ramdac_ext_indexed_w(offs_t offset, u8 data);

	u8 m_crtcext_index = 0;
	u8 m_ramdac_mode = 0;
};

DECLARE_DEVICE_TYPE(MATROX_VGA, matrox_vga_device)

#endif // MAME_VIDEO_PC_VGA_MATROX_H
