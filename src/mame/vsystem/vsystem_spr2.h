// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Haywood
// Video System Sprites
#ifndef MAME_VSYSTEM_VSYSTEM_SPR2_H
#define MAME_VSYSTEM_VSYSTEM_SPR2_H

typedef device_delegate<uint32_t (uint32_t)> vsystem_tile2_indirection_delegate;


class vsystem_spr2_device : public device_t, public device_gfx_interface
{
public:
	vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> vsystem_spr2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: vsystem_spr2_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_tile_indirect_cb(T &&... args) { m_newtilecb.set(std::forward<T>(args)...); }
	void set_pritype(int pritype) { m_pritype = pritype; }
	void set_offsets(int xoffs, int yoffs)
	{
		m_xoffs = xoffs;
		m_yoffs = yoffs;
	}

	void draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen = false);
	void draw_sprites(uint16_t const *spriteram3,  int spriteram3_bytes,  int spritepalettebank, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen = false);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct sprite_attributes
	{
		sprite_attributes() { }
		bool get(uint16_t const *ram);
		void handle_xsize_map_inc();

		int32_t ox = 0;
		uint8_t xsize = 0;
		int32_t zoomx = 0;
		int32_t oy = 0;
		uint8_t ysize = 0;
		int32_t zoomy = 0;
		bool flipx = false;
		bool flipy = false;
		uint32_t color = 0;
		uint32_t pri = 0;
		uint32_t map = 0;
	};

	uint32_t tile_callback_noindirect(uint32_t tile);

	template<class BitmapClass>
	void draw_sprites_common(uint16_t const *spriteram3,  int spriteram3_bytes, int spritepalettebank, BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_param, bool flip_screen);

	vsystem_tile2_indirection_delegate m_newtilecb;
	int m_pritype;
	int m_xoffs, m_yoffs;

	sprite_attributes m_curr_sprite;
};


DECLARE_DEVICE_TYPE(VSYSTEM_SPR2, vsystem_spr2_device)

#endif // MAME_VSYSTEM_VSYSTEM_SPR2_H
