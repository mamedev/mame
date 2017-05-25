// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
#ifndef MAME_VIDEO_VSYSTEM_SPR2_H
#define MAME_VIDEO_VSYSTEM_SPR2_H

typedef device_delegate<uint32_t (uint32_t)> vsystem_tile2_indirection_delegate;

#define MCFG_VSYSTEM_SPR2_SET_TILE_INDIRECT( _class, _method) \
	vsystem_spr2_device::set_tile_indirect_cb(*device, vsystem_tile2_indirection_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));
#define MCFG_VSYSTEM_SPR2_SET_PRITYPE( _val) \
	vsystem_spr2_device::set_pritype(*device, _val);
#define MCFG_VSYSTEM_SPR2_SET_GFXREGION( _rgn ) \
	vsystem_spr2_device::set_gfx_region(*device, _rgn);
#define MCFG_VSYSTEM_SPR2_SET_OFFSETS( _xoffs, _yoffs ) \
	vsystem_spr2_device::set_offsets(*device, _xoffs,_yoffs);
#define MCFG_VSYSTEM_SPR2_GFXDECODE(_gfxtag) \
	vsystem_spr2_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

class vsystem_spr2_device : public device_t
{
public:
	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_tile_indirect_cb(device_t &device, vsystem_tile2_indirection_delegate newtilecb);
	static void set_pritype(device_t &device, int pritype);
	static void set_gfx_region(device_t &device, int gfx_region);
	static void set_offsets(device_t &device, int xoffs, int yoffs);

	vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void turbofrc_draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );
	void turbofrc_draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct sprite_attributes
	{
		sprite_attributes() { }
		bool get(uint16_t const *ram);
		void handle_xsize_map_inc();

		int ox = 0;
		int xsize = 0;
		int zoomx = 0;
		int oy = 0;
		int ysize = 0;
		int zoomy = 0;
		int flipx = 0;
		int flipy = 0;
		int color = 0;
		int pri = 0;
		uint32_t map = 0;
	};

	uint32_t tile_callback_noindirect(uint32_t tile);

	template<class BitmapClass>
	void turbofrc_draw_sprites_common(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );

	vsystem_tile2_indirection_delegate m_newtilecb;
	int m_pritype;
	int m_gfx_region;
	int m_xoffs, m_yoffs;

	sprite_attributes m_curr_sprite;

	required_device<gfxdecode_device> m_gfxdecode;
};


DECLARE_DEVICE_TYPE(VSYSTEM_SPR2, vsystem_spr2_device)

#endif // MAME_VIDEO_VSYSTEM_SPR2_H
