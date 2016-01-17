// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Sega system24 hardware

*/

#ifndef __SEGAIC24_H
#define __SEGAIC24_H

#define MCFG_S24TILE_DEVICE_ADD(_tag, tile_mask) \
	MCFG_DEVICE_ADD(_tag, S24TILE, 0) \
	segas24_tile::static_set_tile_mask(*device, tile_mask);

#define MCFG_S24SPRITE_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24SPRITE, 0)

#define MCFG_S24MIXER_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24MIXER, 0)


#define MCFG_S24TILE_DEVICE_GFXDECODE(_gfxtag) \
	segas24_tile::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_S24TILE_DEVICE_PALETTE(_palette_tag) \
	segas24_tile::static_set_palette_tag(*device, "^" _palette_tag);

class segas24_tile : public device_t
{
	friend class segas24_tile_config;

public:
	segas24_tile(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, std::string tag);
	static void static_set_palette_tag(device_t &device, std::string tag);
	static void static_set_tile_mask(device_t &device, UINT16 tile_mask);

	DECLARE_READ16_MEMBER(tile_r);
	DECLARE_WRITE16_MEMBER(tile_w);
	DECLARE_READ16_MEMBER(char_r);
	DECLARE_WRITE16_MEMBER(char_w);

	DECLARE_READ32_MEMBER(tile32_r);
	DECLARE_WRITE32_MEMBER(tile32_w);
	DECLARE_READ32_MEMBER(char32_r);
	DECLARE_WRITE32_MEMBER(char32_w);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int pri, int flags);

protected:
	virtual void device_start() override;

private:
	enum {
		SYS24_TILES = 0x4000
	};

	std::unique_ptr<UINT16[]> char_ram;
	std::unique_ptr<UINT16[]> tile_ram;
	int char_gfx_index;
	tilemap_t *tile_layer[4];
	UINT16 tile_mask;

	static const gfx_layout char_layout;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void tile_info(int offset, tile_data &tileinfo, tilemap_memory_index tile_index);
	TILE_GET_INFO_MEMBER(tile_info_0s);
	TILE_GET_INFO_MEMBER(tile_info_0w);
	TILE_GET_INFO_MEMBER(tile_info_1s);
	TILE_GET_INFO_MEMBER(tile_info_1w);

	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_ind16 &dm, const UINT16 *mask,
					UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);
	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_rgb32 &dm, const UINT16 *mask,
					UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);

	template<class _BitmapClass>
	void draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
};

class segas24_sprite : public device_t
{
	friend class segas24_sprite_config;

public:
	segas24_sprite(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, const int *spri);

protected:
	virtual void device_start() override;

private:
	std::unique_ptr<UINT16[]> sprite_ram;
};


class segas24_mixer : public device_t
{
	friend class segas24_mixer_config;

public:
	segas24_mixer(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	UINT16 get_reg(int reg);

protected:
	virtual void device_start() override;

private:
	UINT16 mixer_reg[16];
};

extern const device_type S24TILE;
extern const device_type S24SPRITE;
extern const device_type S24MIXER;

#endif
