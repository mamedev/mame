
typedef UINT16 (*decospr_priority_callback_func)(UINT16 pri);

class decospr_device : public device_t
{
public:
	decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_gfx_region(device_t &device, int gfxregion);
	static void set_pri_callback(device_t &device, decospr_priority_callback_func callback);
	//void decospr_sprite_kludge(int x, int y);
	void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram, int sizewords, bool invert_flip = false );
	void set_pri_callback(decospr_priority_callback_func callback);
	void set_gfxregion(int region) { m_gfxregion = region; };
	void set_alt_format(bool alt) { m_alt_format = alt; };
	void set_pix_mix_mask(UINT16 mask) { m_pixmask = mask; };
	void set_pix_raw_shift(UINT16 shift) { m_raw_shift = shift; };
	void alloc_sprite_bitmap(running_machine& machine);
	void inefficient_copy_sprite_bitmap(running_machine& machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16 pri, UINT16 priority_mask, UINT16 colbase, UINT16 palmask, UINT8 alpha = 0xff);
	bitmap_t* get_sprite_temp_bitmap(void) { return m_sprite_bitmap; };

protected:
	virtual void device_start();
	virtual void device_reset();
	UINT8						m_gfxregion;
	decospr_priority_callback_func m_pricallback;
	bitmap_t *m_sprite_bitmap;// optional sprite bitmap (should be INDEXED16)
	bool m_alt_format;
	UINT16 m_pixmask;
	UINT16 m_raw_shift;
private:

};

extern const device_type DECO_SPRITE;





