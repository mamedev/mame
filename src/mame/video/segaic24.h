#ifndef __SEGAIC24_H
#define __SEGAIC24_H

#define MCFG_S24TILE_DEVICE_ADD(_tag, tile_mask) \
	MCFG_DEVICE_ADD(_tag, S24TILE, 0) \
	segas24_tile_config::static_set_tile_mask(device, tile_mask);

#define MCFG_S24SPRITE_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24SPRITE, 0)

#define MCFG_S24MIXER_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, S24MIXER, 0)

class segas24_tile;
class segas24_sprite;
class segas24_mixer;

class segas24_tile_config : public device_config
{
	friend class segas24_tile;

protected:
	segas24_tile_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	static void static_set_tile_mask(device_config *device, UINT16 tile_mask);

	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

private:
	UINT16 tile_mask;
};

class segas24_sprite_config : public device_config
{
	friend class segas24_sprite;

protected:
	segas24_sprite_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
};

class segas24_mixer_config : public device_config
{
	friend class segas24_mixer;

protected:
	segas24_mixer_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
};


class segas24_tile : public device_t
{
	friend class segas24_tile_config;

	segas24_tile(running_machine &_machine, const segas24_tile_config &config);

public:
	DECLARE_READ16_MEMBER(tile_r);
	DECLARE_WRITE16_MEMBER(tile_w);
	DECLARE_READ16_MEMBER(char_r);
	DECLARE_WRITE16_MEMBER(char_w);

	DECLARE_READ32_MEMBER(tile32_r);
	DECLARE_WRITE32_MEMBER(tile32_w);
	DECLARE_READ32_MEMBER(char32_r);
	DECLARE_WRITE32_MEMBER(char32_w);

	void draw(bitmap_t *bitmap, const rectangle *cliprect, int layer, int pri, int flags);

protected:
	virtual void device_start();

	const segas24_tile_config &config;

private:
	enum {
		SYS24_TILES = 0x4000
	};

	UINT16 *char_ram, *tile_ram;
	int char_gfx_index;
	tilemap_t *tile_layer[4];

	static const gfx_layout char_layout;

	void tile_info(int offset, tile_data *tileinfo, tilemap_memory_index tile_index);
	static TILE_GET_INFO_DEVICE(tile_info_0s);
	static TILE_GET_INFO_DEVICE(tile_info_0w);
	static TILE_GET_INFO_DEVICE(tile_info_1s);
	static TILE_GET_INFO_DEVICE(tile_info_1w);

	void draw_rect(bitmap_t *bm, bitmap_t *tm, bitmap_t *dm, const UINT16 *mask,
				   UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);
	void draw_rect_rgb(bitmap_t *bm, bitmap_t *tm, bitmap_t *dm, const UINT16 *mask,
					   UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2);

};

class segas24_sprite : public device_t
{
	friend class segas24_sprite_config;

	segas24_sprite(running_machine &_machine, const segas24_sprite_config &config);

public:
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	void draw(bitmap_t *bitmap, const rectangle *cliprect, const int *spri);

protected:
	virtual void device_start();

	const segas24_sprite_config &config;

private:
	UINT16 *sprite_ram;
};


class segas24_mixer : public device_t
{
	friend class segas24_mixer_config;

	segas24_mixer(running_machine &_machine, const segas24_mixer_config &config);

public:
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	UINT16 get_reg(int reg);

protected:
	virtual void device_start();

	const segas24_mixer_config &config;

private:
	UINT16 mixer_reg[16];
};

extern const device_type S24TILE, S24SPRITE, S24MIXER;

#endif
