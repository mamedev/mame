// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
#ifndef MAME_VSYSTEM_VSYSTEM_SPR_H
#define MAME_VSYSTEM_VSYSTEM_SPR_H

#pragma once


/*** CG10103 **********************************************/

class vsystem_spr_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<uint32_t (uint32_t)> vsystem_tile_indirection_delegate;
	typedef device_delegate<uint32_t (uint32_t)> vsystem_pri_delegate;

	vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: vsystem_spr_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	void set_offsets(int xoffs, int yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
	}
	template <typename... T> void set_tile_indirect_cb(T &&... args) { m_newtile_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_pri_cb(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	void set_pal_base(int pal_base) { m_pal_base = pal_base; }
	void set_pal_mask(int pal_mask) { m_pal_mask = pal_mask; }
	void set_transpen(int transpen) { m_transpen = transpen; }

	void draw_sprites(uint16_t const *spriteram, int spriteram_bytes, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct sprite_attributes
	{
		sprite_attributes() { }
		void get(uint16_t const *ram);

		int32_t ox = 0;
		uint8_t xsize = 0;
		int32_t zoomx = 0;
		int32_t oy = 0;
		uint8_t ysize = 0;
		int32_t zoomy = 0;
		bool flipx = false;
		bool flipy = false;
		uint32_t color = 0;
		uint32_t map = 0;
	};

	uint32_t tile_callback_noindirect(uint32_t tile);
	void common_sprite_drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	vsystem_tile_indirection_delegate m_newtile_cb;
	vsystem_pri_delegate m_pri_cb;

	// inline config
	int m_xoffs, m_yoffs;
	uint16_t m_pal_mask;
	uint8_t m_transpen;

	uint16_t m_pal_base;

	sprite_attributes m_curr_sprite;
};


DECLARE_DEVICE_TYPE(VSYSTEM_SPR, vsystem_spr_device)

#endif // MAME_VSYSTEM_VSYSTEM_SPR_H
