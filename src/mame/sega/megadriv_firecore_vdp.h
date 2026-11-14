// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SEGA_MEGADRIV_FIRECORE_VDP_H
#define MAME_SEGA_MEGADRIV_FIRECORE_VDP_H

#pragma once

#include "video/315_5313.h"

DECLARE_DEVICE_TYPE(FIRECORE_VDP, megadriv_firecore_vdp_device)


class megadriv_firecore_vdp_device : public sega315_5313_device
{
public:
	// construction/destruction
	template <typename T>
	megadriv_firecore_vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: megadriv_firecore_vdp_device(mconfig, tag, owner, clock)
	{
		m_cpu68k.set_tag(std::forward<T>(cpu_tag));
	}

	megadriv_firecore_vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: sega315_5313_device(mconfig, FIRECORE_VDP, tag, owner, clock)
	{
		m_vram_size = 0x20000;
		m_cram_size = 0x800;
	}

	void draw_firecore(bitmap_rgb32& bitmap, const rectangle& cliprect);

	// Firecore extended video mode (VDP register 0x18 bit 7)
	uint16_t xvdp_r(offs_t offset, uint16_t mem_mask);
	void xvdp_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	bool xvdp_enabled() const { return BIT(m_regs[0x18], 7); }

private:
	u8 read_vram8(u32 address);	
	void write_vram8(u32 address, u8 data);
	void xvdp_data_w(uint16_t data);
	void xvdp_ctrl_w(uint16_t data);
	uint32_t xvdp_color(int index) const;
	void xvdp_draw_plane(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t nametable, int plane);
	void xvdp_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint8_t xvdp_tile_pixel(uint32_t tile, int x, int y);
};

#endif // MAME_SEGA_MEGADRIV_FIRECORE_VDP_H
