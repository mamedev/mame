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
	megasys1_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, uint16_t colorbase)
		: megasys1_tilemap_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_colorbase(colorbase);
	}

	megasys1_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_8x8_scroll_factor(int scroll_factor) { m_8x8_scroll_factor = scroll_factor; }
	void set_16x16_scroll_factor(int scroll_factor) { m_16x16_scroll_factor = scroll_factor; }
	void set_bits_per_color_code(int bits) { m_bits_per_color_code = bits; }
	void set_colorbase(uint16_t colorbase) { m_colorbase = colorbase; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// memory handlers
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scroll_r(offs_t offset);
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// drawing and layer control
	void draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, uint32_t flags, uint8_t priority = 0, uint8_t priority_mask = 0xff);
	void enable(bool enable);
	void set_flip(uint32_t attributes);
	void set_tilebank(uint8_t bank);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// shared memory finder
	required_shared_ptr<uint16_t> m_scrollram;
	optional_device<screen_device> m_screen;

	// configuration
	int m_8x8_scroll_factor;
	int m_16x16_scroll_factor;
	int m_bits_per_color_code;
	uint16_t m_colorbase;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// internal state
	uint16_t m_scrollx = 0;
	uint16_t m_scrolly = 0;
	uint16_t m_scroll_flag = 0;
	uint16_t m_tile_bank = 0;
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
