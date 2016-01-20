// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood


class deco_zoomspr_device : public device_t
{
public:
	deco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_palette_tag(device_t &device, const char *tag);
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	void dragngun_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const UINT32 *spritedata, UINT32* dragngun_sprite_layout_0_ram, UINT32* dragngun_sprite_layout_1_ram, UINT32* dragngun_sprite_lookup_0_ram, UINT32* dragngun_sprite_lookup_1_ram, UINT32 dragngun_sprite_ctrl, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	void dragngun_drawgfxzoom(
		bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int transparent_color,
		int scalex, int scaley, bitmap_ind8 *pri_buffer, UINT32 pri_mask, int sprite_screen_width, int  sprite_screen_height, UINT8 alpha, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap,
		int priority);

};


extern const device_type DECO_ZOOMSPR;

#define MCFG_DECO_ZOOMSPR_PALETTE(_palette_tag) \
	deco_zoomspr_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_DECO_ZOOMSPR_GFXDECODE(_gfxtag) \
	deco_zoomspr_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
