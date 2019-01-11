// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
#ifndef MAME_VIDEO_VSYSTEM_SPR_H
#define MAME_VIDEO_VSYSTEM_SPR_H

#pragma once


typedef device_delegate<uint32_t (uint32_t)> vsystem_tile_indirection_delegate;

#define MCFG_VSYSTEM_SPR_SET_TILE_INDIRECT(_class, _method) \
	downcast<vsystem_spr_device &>(*device).set_tile_indirect_cb(vsystem_tile_indirection_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));
#define MCFG_VSYSTEM_SPR_SET_GFXREGION(_rgn) \
	downcast<vsystem_spr_device &>(*device).set_gfx_region(_rgn);
#define MCFG_VSYSTEM_SPR_SET_PALBASE(_palbase) \
	downcast<vsystem_spr_device &>(*device).CG10103_set_pal_base(_palbase);
#define MCFG_VSYSTEM_SPR_SET_PALMASK(_palmask) \
	downcast<vsystem_spr_device &>(*device).set_pal_mask(_palmask);
#define MCFG_VSYSTEM_SPR_SET_TRANSPEN(_transpen) \
	downcast<vsystem_spr_device &>(*device).CG10103_set_transpen(_transpen);
#define MCFG_VSYSTEM_SPR_GFXDECODE(_gfxtag) \
	downcast<vsystem_spr_device &>(*device).set_gfxdecode_tag(_gfxtag);
#define MCFG_VSYSTEM_SPR_SET_OFFSETS(_xoffs, _yoffs) \
	downcast<vsystem_spr_device &>(*device).set_offsets(_xoffs,_yoffs);
#define MCFG_VSYSTEM_SPR_SET_PDRAW(_pdraw) \
	downcast<vsystem_spr_device &>(*device).set_pdraw(_pdraw);

/*** CG10103 **********************************************/

class vsystem_spr_device : public device_t
{
public:
	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_offsets(int xoffs, int yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
	}
	void set_pdraw(bool pdraw) { m_pdraw = pdraw; }
	template <typename Object> void set_tile_indirect_cb(Object &&cb) { m_newtilecb = std::forward<Object>(cb); }
	void set_gfx_region(int gfx_region) { m_gfx_region = gfx_region; }
	void CG10103_set_pal_base(int pal_base) { m_pal_base = pal_base; }
	void set_pal_mask(int pal_mask) { m_pal_mask = pal_mask; }
	void CG10103_set_transpen(int transpen) { m_transpen = transpen; }

	vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void draw_sprites(uint16_t const *spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int prihack_mask = -1, int prihack_val = -1 );
	void set_pal_base(int pal_base) { m_pal_base = pal_base; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct sprite_attributes
	{
		sprite_attributes() { }
		void get(uint16_t const *ram);

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
	void common_sprite_drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	vsystem_tile_indirection_delegate m_newtilecb;

	// inline config
	int m_xoffs, m_yoffs;
	bool m_pdraw;
	uint16_t m_pal_mask;
	uint8_t m_gfx_region;
	uint8_t m_transpen;

	uint16_t m_pal_base;

	sprite_attributes m_curr_sprite;

	required_device<gfxdecode_device> m_gfxdecode;
};


DECLARE_DEVICE_TYPE(VSYSTEM_SPR, vsystem_spr_device)

#endif // MAME_VIDEO_VSYSTEM_SPR_H
