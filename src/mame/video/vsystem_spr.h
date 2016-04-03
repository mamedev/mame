// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites


typedef device_delegate<UINT32 (UINT32)> vsystem_tile_indirection_delegate;

#define MCFG_VSYSTEM_SPR_SET_TILE_INDIRECT( _class, _method) \
	vsystem_spr_device::set_tile_indirect_cb(*device, vsystem_tile_indirection_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_VSYSTEM_SPR_SET_GFXREGION( _rgn ) \
	vsystem_spr_device::set_gfx_region(*device, _rgn);
#define MCFG_VSYSTEM_SPR_SET_PALBASE( _palbase ) \
	vsystem_spr_device::CG10103_set_pal_base(*device, _palbase);
#define MCFG_VSYSTEM_SPR_SET_PALMASK( _palmask ) \
	vsystem_spr_device::set_pal_mask(*device, _palmask);
#define MCFG_VSYSTEM_SPR_SET_TRANSPEN( _transpen ) \
	vsystem_spr_device::CG10103_set_transpen(*device, _transpen);
#define MCFG_VSYSTEM_SPR_GFXDECODE(_gfxtag) \
	vsystem_spr_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

/*** CG10103 **********************************************/

class vsystem_spr_device : public device_t
{
public:
	vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_offsets(device_t &device, int xoffs, int yoffs);
	static void set_pdraw(device_t &device, bool pdraw);
	static void set_tile_indirect_cb(device_t &device,vsystem_tile_indirection_delegate newtilecb);
	static void set_gfx_region(device_t &device, int gfx_region);
	static void CG10103_set_pal_base(device_t &device, int pal_base);
	static void set_pal_mask(device_t &device, int pal_mask);
	static void CG10103_set_transpen(device_t &device, int transpen);

	UINT32 tile_callback_noindirect(UINT32 tile);
	vsystem_tile_indirection_delegate m_newtilecb;

	// inline config
	int m_xoffs, m_yoffs;
	bool m_pdraw;
	UINT16 m_pal_mask;
	UINT8 m_gfx_region;
	UINT8 m_transpen;

	UINT16 m_pal_base;

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
	} m_curr_sprite;

	void get_sprite_attributes(UINT16* ram);
	void common_sprite_drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	void draw_sprites(  UINT16* spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int prihack_mask = -1, int prihack_val = -1 );
	void set_pal_base(int pal_base);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;
};


extern const device_type VSYSTEM_SPR;
