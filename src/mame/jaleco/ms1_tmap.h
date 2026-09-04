// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    ms1_tmap.h

    8x8/16x16 tilemap generator for Jaleco's Mega System 1 and driving
    and mahjong games from the same period.

***************************************************************************/
#ifndef MAME_JALECO_MS1_TMAP_H
#define MAME_JALECO_MS1_TMAP_H

#pragma once

#include "screen.h"
#include "tilemap.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> megasys1_tilemap_device

class megasys1_tilemap_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	template <typename T>
	megasys1_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, u16 colorbase)
		: megasys1_tilemap_device(mconfig, tag, owner, (u32)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_colorbase(colorbase);
	}

	megasys1_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_8x8_scroll_factor(int scroll_factor) { m_8x8_scroll_factor = scroll_factor; }
	void set_16x16_scroll_factor(int scroll_factor) { m_16x16_scroll_factor = scroll_factor; }
	void set_bits_per_color_code(int bits) { m_bits_per_color_code = bits; }
	void set_colorbase(u16 colorbase) { m_colorbase = colorbase; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// memory handlers
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 scroll_r(offs_t offset);
	void scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// drawing and layer control
	void draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags, u8 priority = 0, u8 priority_mask = 0xff);
	void enable(bool enable);
	void set_flip(u32 attributes);
	void set_tilebank(u8 bank);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// shared memory finder
	required_shared_ptr<u16> m_scrollram;
	optional_device<screen_device> m_screen;

	// configuration
	int m_8x8_scroll_factor;
	int m_16x16_scroll_factor;
	int m_bits_per_color_code;
	u16 m_colorbase;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// internal state
	u16 m_scrollx = 0;
	u16 m_scrolly = 0;
	u16 m_scroll_flag = 0;
	u32 m_tile_bank = 0;
	tilemap_t *m_tmap = nullptr;
	tilemap_t *m_tilemap[2][4]{};

	// helpers
	TILEMAP_MAPPER_MEMBER(scan_8x8);
	TILEMAP_MAPPER_MEMBER(scan_16x16);
	TILE_GET_INFO_MEMBER(get_scroll_tile_info_8x8);
	TILE_GET_INFO_MEMBER(get_scroll_tile_info_16x16);
};

// device type definition
DECLARE_DEVICE_TYPE(MEGASYS1_TILEMAP, megasys1_tilemap_device)

#endif  // MAME_JALECO_MS1_TMAP_H
