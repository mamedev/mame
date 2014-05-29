/* Tecmo Sprites */



class tecmo_spr_device : public device_t
{
public:
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_gfx_region(device_t &device, int gfxregion);
	static void set_bootleg(device_t &device, int bootleg);

	// gaiden.c / spbactn.c / tecmo16.c sprites
	void gaiden_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap);
	
	// tecmo.c sprites
	void draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT8* spriteram, int size, int video_type, int flip_screen);

	typedef void (tecmo_spr_device::*draw_wc90_sprites_func)(bitmap_ind16 &, const rectangle &,  gfxdecode_device *, int, int, int, int, int );

	// wc90.c sprites
	void draw_wc90_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, UINT8* spriteram, int size, int priority);

	void draw_wc90_sprite_16x16(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_16x32(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_16x64(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_32x16(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_32x32(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_32x64(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code, int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_64x16(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_64x32(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_64x64(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code,int sx, int sy, int bank, int flags );
	void draw_wc90_sprite_invalid(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode,int code, int sx, int sy, int bank, int flags );

	// tbowl.c sprites
	void tbowl_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int xscroll, UINT8* spriteram);

protected:
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_gfxregion;
	int m_bootleg; // for Gals Pinball / Hot Pinball


private:
};

extern const device_type TECMO_SPRITE;


#define MCFG_TECMO_SPRITE_GFX_REGION(_region) \
	tecmo_spr_device::set_gfx_region(*device, _region);

#define MCFG_TECMO_SPRITE_BOOTLEG(_bootleg) \
	tecmo_spr_device::set_bootleg(*device, _bootleg);



