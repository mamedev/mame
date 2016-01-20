// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites

typedef device_delegate<UINT32 (UINT32)> vsystem_tile2_indirection_delegate;

#define MCFG_VSYSTEM_SPR2_SET_TILE_INDIRECT( _class, _method) \
	vsystem_spr2_device::set_tile_indirect_cb(*device, vsystem_tile2_indirection_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_VSYSTEM_SPR2_SET_PRITYPE( _val) \
	vsystem_spr2_device::set_pritype(*device, _val);
#define MCFG_VSYSTEM_SPR2_SET_GFXREGION( _rgn ) \
	vsystem_spr2_device::set_gfx_region(*device, _rgn);
#define MCFG_VSYSTEM_SPR2_SET_OFFSETS( _xoffs, _yoffs ) \
	vsystem_spr2_device::set_offsets(*device, _xoffs,_yoffs);
#define MCFG_VSYSTEM_SPR2_GFXDECODE(_gfxtag) \
	vsystem_spr2_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#define MCFG_VSYSTEM_SPR2_PALETTE(_palette_tag) \
	vsystem_spr2_device::static_set_palette_tag(*device, "^" _palette_tag);

class vsystem_spr2_device : public device_t
{
public:
	vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_tile_indirect_cb(device_t &device,vsystem_tile2_indirection_delegate newtilecb);
	static void set_pritype(device_t &device, int pritype);
	static void set_gfx_region(device_t &device, int gfx_region);
	static void set_offsets(device_t &device, int xoffs, int yoffs);

	struct vsystem_sprite_attributes
	{
		int ox;
		int xsize;
		int zoomx;
		int oy;
		int ysize;
		int zoomy;
		int flipx;
		int flipy;
		int color;
		int pri;
		UINT32 map;
	} curr_sprite;

	int get_sprite_attributes(UINT16* ram);
	void handle_xsize_map_inc(void);
	vsystem_tile2_indirection_delegate m_newtilecb;
	UINT32 tile_callback_noindirect(UINT32 tile);
	int m_pritype;
	int m_gfx_region;
	int m_xoffs, m_yoffs;

	template<class _BitmapClass>
	void turbofrc_draw_sprites_common( UINT16* spriteram3,  int spriteram3_bytes, int spritepalettebank, _BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );

	void turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );
	void turbofrc_draw_sprites( UINT16* spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param );



protected:
	virtual void device_start() override;
	virtual void device_reset() override;


private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


extern const device_type VSYSTEM_SPR2;
