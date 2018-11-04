// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Sega system24 hardware

*/

#ifndef MAME_VIDEO_SEGAIC24_H
#define MAME_VIDEO_SEGAIC24_H

#pragma once

#define MCFG_S24TILE_DEVICE_ADD(_tag, tile_mask) \
	MCFG_DEVICE_ADD(_tag, S24TILE, 0) \
	segas24_tile_device::static_set_tile_mask(*device, tile_mask);

#define MCFG_S24SPRITE_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24SPRITE, 0)

#define MCFG_S24MIXER_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24MIXER, 0)


#define MCFG_S24TILE_DEVICE_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)

class segas24_tile_device : public device_t, public device_gfx_interface
{
	friend class segas24_tile_config;

public:
	segas24_tile_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void static_set_tile_mask(device_t &device, uint16_t tile_mask);

	DECLARE_READ16_MEMBER(tile_r);
	DECLARE_WRITE16_MEMBER(tile_w);
	DECLARE_READ16_MEMBER(char_r);
	DECLARE_WRITE16_MEMBER(char_w);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);

protected:
	virtual void device_start() override;

private:
	enum {
		SYS24_TILES = 0x4000
	};

	std::unique_ptr<uint16_t[]> char_ram;
	std::unique_ptr<uint16_t[]> tile_ram;
	int char_gfx_index;
	tilemap_t *tile_layer[4];
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

	template<class _BitmapClass>
	void draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
};

class segas24_sprite_device : public device_t
{
	friend class segas24_sprite_config;

public:
	segas24_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, const int *spri);

protected:
	virtual void device_start() override;

private:
	std::unique_ptr<uint16_t[]> sprite_ram;
};


class segas24_mixer_device : public device_t
{
	friend class segas24_mixer_config;

public:
	segas24_mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	uint16_t get_reg(int reg);

protected:
	virtual void device_start() override;

private:
	uint16_t mixer_reg[16];
};

DECLARE_DEVICE_TYPE(S24TILE,   segas24_tile_device)
DECLARE_DEVICE_TYPE(S24SPRITE, segas24_sprite_device)
DECLARE_DEVICE_TYPE(S24MIXER,  segas24_mixer_device)

#endif // MAME_VIDEO_SEGAIC24_H
