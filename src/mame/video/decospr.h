
typedef UINT16 (*decospr_priority_callback_func)(UINT16 pri);
typedef UINT16 (*decospr_colour_callback_func)(UINT16 col);

class decospr_device : public device_t,
						public device_video_interface
{
public:
	decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion);
	static void set_pri_callback(device_t &device, decospr_priority_callback_func callback);
	static void set_col_callback(device_t &device, decospr_colour_callback_func callback);

	static void set_is_bootleg(device_t &device, bool is_bootleg)
	{
		decospr_device &dev = downcast<decospr_device &>(device);
		dev.m_is_bootleg = is_bootleg;
	}

	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		decospr_device &dev = downcast<decospr_device &>(device);
		dev.m_x_offset = x_offset;
		dev.m_y_offset = y_offset;
	}

	static void set_flipallx(device_t &device, int flipallx)
	{
		decospr_device &dev = downcast<decospr_device &>(device);
		dev.m_flipallx = flipallx;
	}

	static void set_transpen(device_t &device, int transpen)
	{
		decospr_device &dev = downcast<decospr_device &>(device);
		dev.m_transpen = transpen;
	}


	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip = false );
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip = false );
	void set_pri_callback(decospr_priority_callback_func callback);
	void set_col_callback(decospr_colour_callback_func callback);
	void set_gfxregion(int region) { m_gfxregion = region; };
	void set_alt_format(bool alt) { m_alt_format = alt; };
	void set_pix_mix_mask(UINT16 mask) { m_pixmask = mask; };
	void set_pix_raw_shift(UINT16 shift) { m_raw_shift = shift; };
	void set_is_bootleg(bool is_bootleg) { m_is_bootleg = is_bootleg; };
	void set_offsets(int x_offset, int y_offset) { m_x_offset = x_offset; m_y_offset = y_offset; };
	void set_flipallx(int flipallx) { m_flipallx = flipallx; };
	void set_transpen(int transpen) { m_transpen = transpen; };

	void alloc_sprite_bitmap();
	void inefficient_copy_sprite_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 pri, UINT16 priority_mask, UINT16 colbase, UINT16 palmask, UINT8 alpha = 0xff);
	bitmap_ind16& get_sprite_temp_bitmap() { assert(m_sprite_bitmap.valid()); return m_sprite_bitmap; };

protected:
	virtual void device_start();
	virtual void device_reset();
	UINT8                       m_gfxregion;
	decospr_priority_callback_func m_pricallback;
	decospr_colour_callback_func m_colcallback;
	bitmap_ind16 m_sprite_bitmap;// optional sprite bitmap (should be INDEXED16)
	bool m_alt_format;
	UINT16 m_pixmask;
	UINT16 m_raw_shift;

	// used by various bootleg / clone chips.
	bool m_is_bootleg; // used by various bootlegs (disables masking of sprite tile number when multi-sprite is used)
	int m_x_offset, m_y_offset; // used by various bootlegs
	int m_flipallx; // used by esd16.c - hedpanio, multchmp , and nmg5.c
	int m_transpen; // used by fncywld (tumbleb.c)

private:
	template<class _BitmapClass>
	void draw_sprites_common(_BitmapClass &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

extern const device_type DECO_SPRITE;

#define MCFG_DECO_SPRITE_GFXDECODE(_gfxtag) \
	decospr_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_DECO_SPRITE_PALETTE(_palette_tag) \
	decospr_device::static_set_palette_tag(*device, "^" _palette_tag);
