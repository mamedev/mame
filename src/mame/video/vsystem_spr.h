// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
#ifndef MAME_VIDEO_VSYSTEM_SPR_H
#define MAME_VIDEO_VSYSTEM_SPR_H

#pragma once


typedef device_delegate<uint32_t (uint32_t)> vsystem_tile_indirection_delegate;

/*** CG10103 **********************************************/

class vsystem_spr_device : public device_t
{
public:
	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_offsets(int xoffs, int yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
	}
	void set_pdraw(bool pdraw) { m_pdraw = pdraw; }
	template <typename... T> void set_tile_indirect_cb(T &&... args) { m_newtilecb.set(std::forward<T>(args)...); }
	void set_gfx_region(int gfx_region) { m_gfx_region = gfx_region; }
	void set_pal_base(int pal_base) { m_pal_base = pal_base; }
	void set_pal_mask(int pal_mask) { m_pal_mask = pal_mask; }
	void set_transpen(int transpen) { m_transpen = transpen; }

	vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void draw_sprites(uint16_t const *spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int prihack_mask = -1, int prihack_val = -1 );

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
