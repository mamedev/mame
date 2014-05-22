/* Tecmo Sprites */



class tecmo_spr_device : public device_t,
						public device_video_interface
{
public:
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_gfx_region(device_t &device, int gfxregion);

	void galspnbl_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16* spriteram, int spriteram_bytes );
	void gaiden_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap);

protected:
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_gfxregion;

private:
};

extern const device_type TECMO_SPRITE;


#define MCFG_TECMO_SPRITE_GFX_REGION(_region) \
	tecmo_spr_device::set_gfx_region(*device, _region);

