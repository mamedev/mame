// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tecmo Sprites */



class tecmo_spr_device : public device_t
{
public:
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_gfx_region(device_t &device, int gfxregion);
	static void set_bootleg(device_t &device, int bootleg);
	static void set_yoffset(device_t &device, int bootleg);

	// gaiden.c / spbactn.c / tecmo16.c sprites
	void gaiden_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap);

	// tecmo.c sprites
	void draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT8* spriteram, int size, int video_type, int flip_screen);

	// wc90.c sprites
	void draw_wc90_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, UINT8* spriteram, int size, int priority);

	// tbowl.c sprites
	void tbowl_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int xscroll, UINT8* spriteram);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 m_gfxregion;
	int m_bootleg; // for Gals Pinball / Hot Pinball
	int m_yoffset;

private:
};

extern const device_type TECMO_SPRITE;


#define MCFG_TECMO_SPRITE_GFX_REGION(_region) \
	tecmo_spr_device::set_gfx_region(*device, _region);

#define MCFG_TECMO_SPRITE_BOOTLEG(_bootleg) \
	tecmo_spr_device::set_bootleg(*device, _bootleg);

#define MCFG_TECMO_SPRITE_YOFFSET(_yoffset) \
	tecmo_spr_device::set_yoffset(*device, _yoffset);
