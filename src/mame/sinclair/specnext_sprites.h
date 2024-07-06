// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_SPRITES_H
#define MAME_SINCLAIR_SPECNEXT_SPRITES_H

#pragma once

class specnext_sprites_device : public device_t, public device_gfx_interface
{

public:
	specnext_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	specnext_sprites_device &set_raster_offset(u16 offset_h,  u16 offset_v) { m_offset_h = offset_h - (OVER_BORDER << 1); m_offset_v = offset_v - OVER_BORDER; return *this; }
	specnext_sprites_device &set_palette(const char *tag, u16 base_offset, u16 alt_offset);

	void update_sprites_cache();
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 pmask);

	void sprite_palette_select_w(bool sprite_palette_select) { m_sprite_palette_select = sprite_palette_select; update_config(); }

	void zero_on_top_w(bool zero_on_top) { m_zero_on_top = zero_on_top; }
	void border_clip_en_w(bool border_clip_en) { m_border_clip_en = border_clip_en; update_config(); }
	void over_border_w(bool over_border) { m_over_border = over_border; update_config(); }
	void transp_colour_w(u8 transp_colour) { m_transp_colour = transp_colour; }
	void clip_x1_w(u8 clip_x1) { m_clip_x1 = clip_x1; update_config(); }
	void clip_x2_w(u8 clip_x2) { m_clip_x2 = clip_x2; update_config(); }
	void clip_y1_w(u8 clip_y1) { m_clip_y1 = clip_y1; update_config(); }
	void clip_y2_w(u8 clip_y2) { m_clip_y2 = clip_y2; update_config(); }

	void mirror_tie_w(bool mirror_tie) { m_mirror_tie = mirror_tie; }
	void mirror_index_w(u8 mirror_index) { m_mirror_index = mirror_index & 0x07; }
	void io_w(offs_t addr, u8 data);
	void mirror_data_w(u8 mirror_data);
	void mirror_inc_w(bool mirror_inc) { m_mirror_inc = mirror_inc; }
	u8 mirror_num_r() { return BIT(m_mirror_sprite_q, 0, TOTAL_SPRITES_BITS); }

protected:
	static constexpr u8 OVER_BORDER = 32;
	static constexpr u8 TOTAL_SPRITES_BITS = 7;
	static constexpr u8 TOTAL_PATTERN_BITS = 6;
	static constexpr rectangle SCREEN_AREA = { 0, 319, 0, 255 };

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_share_creator<u8> m_sprite_pattern_ram;
	memory_share_creator<u8> m_sprite_attr_ram;

	void update_config();

private:
	struct sprite_data
	{
		bool rel_type;
		bool h;
		u16 x; // u9
		u16 y; // u9
		u8 pattern; // u7
		u8 paloff; // u4
		bool rotate;
		bool xmirror;
		bool ymirror;
		u8 xscale; // u2
		u8 yscale; // u2
	};

	std::vector<sprite_data> m_sprites_cache;
	u16 m_offset_h, m_offset_v;
	rectangle m_clip_window;
	u16 m_palette_base_offset;
	u16 m_palette_alt_offset;
	bool m_sprite_palette_select;

	bool m_zero_on_top;
	bool m_border_clip_en;
	bool m_over_border;
	u8 m_transp_colour;
	u8 m_clip_x1;
	u8 m_clip_x2;
	u8 m_clip_y1;
	u8 m_clip_y2;

	bool m_mirror_tie;
	u8 m_mirror_index; // attributes 0-4, sprite number if 7
	bool m_mirror_inc;

	u16 m_attr_index;
	u16 m_pattern_index;
	u8 m_mirror_sprite_q;
};


DECLARE_DEVICE_TYPE(SPECNEXT_SPRITES, specnext_sprites_device)
#endif // MAME_SINCLAIR_SPECNEXT_SPRITES_H
