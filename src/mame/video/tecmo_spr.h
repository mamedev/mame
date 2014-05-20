/* Tecmo Sprites */



class tecmo_spr_device : public device_t,
						public device_video_interface
{
public:
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_gfx_region(device_t &device, int gfxregion);
	static void set_alt_format(device_t &device);

	void galspnbl_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16* spriteram, int spriteram_bytes );
	int gaiden_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flipscreen, int pri_hack, bitmap_ind16 &bitmap_prihack );


protected:
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_gfxregion;
	int m_altformat; // the super pinball action proto has some differences

private:
};

extern const device_type TECMO_SPRITE;


#define MCFG_TECMO_SPRITE_GFX_REGION(_region) \
	tecmo_spr_device::set_gfx_region(*device, _region);

#define MCFG_TECMO_SPRITE_ALT_FORMAT \
	tecmo_spr_device::set_alt_format(*device);
