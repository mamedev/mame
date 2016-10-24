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


#define MCFG_S24TILE_DEVICE_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)

class segas24_tile : public device_t, public device_gfx_interface
{
	friend class segas24_tile_config;

public:
	segas24_tile(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void static_set_tile_mask(device_t &device, uint16_t tile_mask);

	uint16_t tile_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tile_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t char_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void char_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint32_t tile32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void tile32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t char32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void char32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

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
	void tile_info_0s(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_0w(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_1s(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_1w(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_ind16 &dm, const uint16_t *mask,
					uint16_t tpri, uint8_t lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);
	void draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_rgb32 &dm, const uint16_t *mask,
					uint16_t tpri, uint8_t lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);

	template<class _BitmapClass>
	void draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int layer, int pri, int flags);
};

class segas24_sprite : public device_t
{
	friend class segas24_sprite_config;

public:
	segas24_sprite(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, const int *spri);

protected:
	virtual void device_start() override;

private:
	std::unique_ptr<uint16_t[]> sprite_ram;
};


class segas24_mixer : public device_t
{
	friend class segas24_mixer_config;

public:
	segas24_mixer(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t get_reg(int reg);

protected:
	virtual void device_start() override;

private:
	uint16_t mixer_reg[16];
};

extern const device_type S24TILE;
extern const device_type S24SPRITE;
extern const device_type S24MIXER;

#endif
