// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Sega system24 hardware

*/

#ifndef MAME_SEGA_SEGAIC24_H
#define MAME_SEGA_SEGAIC24_H

#pragma once

#include "tilemap.h"


class segas24_tile_device : public device_t, public device_gfx_interface
{
	friend class segas24_tile_config;

public:
	segas24_tile_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint16_t _tile_mask)
		: segas24_tile_device(mconfig, tag, owner, clock)
	{
		set_tile_mask(_tile_mask);
	}

	segas24_tile_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_tile_mask(uint16_t _tile_mask) { tile_mask = _tile_mask; }

	uint16_t tile_r(offs_t offset);
	void tile_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t char_r(offs_t offset);
	void char_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void xhout_w(uint16_t data);
	void xvout_w(uint16_t data);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);

	auto xhout_write_callback() { return m_xhout_write_cb.bind(); }
	auto xvout_write_callback() { return m_xvout_write_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	enum {
		SYS24_TILES = 0x4000
	};

	std::unique_ptr<uint16_t[]> char_ram;
	std::unique_ptr<uint16_t[]> tile_ram;
	int char_gfx_index;
	tilemap_t *tile_layer[4]{};
	uint16_t tile_mask;

	static const gfx_layout char_layout;

	void tile_info(int offset, tile_data &tileinfo, tilemap_memory_index tile_index);
	TILE_GET_INFO_MEMBER(tile_info_0s);
	TILE_GET_INFO_MEMBER(tile_info_0w);
	TILE_GET_INFO_MEMBER(tile_info_1s);
	TILE_GET_INFO_MEMBER(tile_info_1w);

	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_ind16 &dm, const uint16_t *mask,
					uint16_t tpri, uint8_t lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);
	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_rgb32 &dm, const uint16_t *mask,
					uint16_t tpri, uint8_t lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);

	template<class BitmapClass>
	void draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int layer, int pri, int flags);

	devcb_write16 m_xhout_write_cb;
	devcb_write16 m_xvout_write_cb;
};

class segas24_sprite_device : public device_t
{
	friend class segas24_sprite_config;

public:
	segas24_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, const int *spri);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<uint16_t[]> sprite_ram;
};


class segas24_mixer_device : public device_t
{
	friend class segas24_mixer_config;

public:
	segas24_mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t get_reg(int reg);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	uint16_t mixer_reg[16];
};

DECLARE_DEVICE_TYPE(S24TILE,   segas24_tile_device)
DECLARE_DEVICE_TYPE(S24SPRITE, segas24_sprite_device)
DECLARE_DEVICE_TYPE(S24MIXER,  segas24_mixer_device)

#endif // MAME_SEGA_SEGAIC24_H
