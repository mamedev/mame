// license:BSD-3-Clause
// copyright-holders:David Haywood

/*** MB60553 **********************************************/
#ifndef MAME_VSYSTEM_MB60553_H
#define MAME_VSYSTEM_MB60553_H

#pragma once

#include "tilemap.h"


class mb60553_zooming_tilemap_device : public device_t
{
public:
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int gfxregion) { m_gfx_region = gfxregion; }

	mb60553_zooming_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_pal_base(int pal_base) { m_pal_base = pal_base; }
	void set_transparent_pen(pen_t pen) { m_tmap->set_transparent_pen(pen); }
	void draw(screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);

	TILEMAP_MAPPER_MEMBER(twc94_scan);

	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void line_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t regs_r(offs_t offset);
	uint16_t vram_r(offs_t offset);
	uint16_t line_r(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// the horizontal pixel counter runs this many pixels ahead of the first visible pixel
	static constexpr int PIXEL_OFFSET = 21;

	void draw_line(bitmap_ind16 &destbitmap, int line, int min_x, int max_x, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy);

	void reg_written(int num_reg);
	TILE_GET_INFO_MEMBER(get_tile_info);

	tilemap_t* m_tmap;
	std::unique_ptr<uint16_t[]> m_vram;
	uint16_t m_regs[8];
	uint8_t m_bank[8];
	uint16_t m_pal_base;

	std::unique_ptr<uint16_t[]> m_lineram;

	uint8_t m_gfx_region;

	required_device<gfxdecode_device> m_gfxdecode;

};

DECLARE_DEVICE_TYPE(MB60553, mb60553_zooming_tilemap_device)

#endif // MAME_VSYSTEM_MB60553_H
