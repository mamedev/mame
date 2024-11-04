// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCOS2_ROZ_H
#define MAME_NAMCO_NAMCOS2_ROZ_H

#pragma once

#include "screen.h"
#include "tilemap.h"


class namcos2_roz_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	namcos2_roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_rozram_tag(T &&tag) { m_rozram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_rozctrl_tag(T &&tag) { m_roz_ctrl.set_tag(std::forward<T>(tag)); }

	void rozram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t gfx_ctrl);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);

	TILE_GET_INFO_MEMBER( roz_tile_info );
	tilemap_t *m_tilemap_roz = nullptr;

	required_shared_ptr<uint16_t> m_rozram;
	required_shared_ptr<uint16_t> m_roz_ctrl;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_ROZ, namcos2_roz_device)

#endif // MAME_NAMCO_NAMCOS2_ROZ_H

