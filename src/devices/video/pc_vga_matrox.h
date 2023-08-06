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

protected:
	virtual void io_3bx_3dx_map(address_map &map) override;

	virtual void device_reset() override;

	void crtcext_map(address_map &map);
private:
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_crtcext_space_config;
	u8 m_crtcext_index = 0;
};

DECLARE_DEVICE_TYPE(MATROX_VGA, matrox_vga_device)

#endif // MAME_VIDEO_PC_VGA_MATROX_H
