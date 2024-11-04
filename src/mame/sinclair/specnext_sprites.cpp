// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Sprites
**********************************************************************/

#include "emu.h"
#include "specnext_sprites.h"

#include "screen.h"


static const gfx_layout gfx_16x16x8 =
{
	16, 16, RGN_FRAC(1, 1), 8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8 * 16) },
	8 * 16 * 16
};

static const gfx_layout gfx_16x16x4 =
{
	16, 16, RGN_FRAC(1, 1), 4,
	{ STEP4(0, 1) },
	{ STEP16(0, 4) },
	{ STEP16(0, 4 * 16) },
	4 * 16 * 16
};

static const gfx_layout gfx_16x16x8_r =
{
	16, 16, RGN_FRAC(1, 1), 8,
	{ STEP8(0, 1) },
	{ STEP16(8 * 16 * 15, -8 * 16) },
	{ STEP16(0, 8) },
	8 * 16 * 16
};

static const gfx_layout gfx_16x16x4_r =
{
	16, 16, RGN_FRAC(1, 1), 4,
	{ STEP4(0, 1) },
	{ STEP16(4 * 16 * 15, -4 * 16) },
	{ STEP16(0, 4) },
	4 * 16 * 16
};

static GFXDECODE_START( gfx_sprites )
	GFXDECODE_DEVICE_RAM( "pattern_ram", 0, gfx_16x16x8, 0, 256 )
	GFXDECODE_DEVICE_RAM( "pattern_ram", 0, gfx_16x16x4, 0, 16 )
	GFXDECODE_DEVICE_RAM( "pattern_ram", 0, gfx_16x16x8_r, 0, 256 )
	GFXDECODE_DEVICE_RAM( "pattern_ram", 0, gfx_16x16x4_r, 0, 16 )
GFXDECODE_END

specnext_sprites_device::specnext_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_SPRITES, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfx_sprites)
	, m_sprite_pattern_ram(*this, "pattern_ram", 64 * 256, ENDIANNESS_LITTLE)
	, m_sprite_attr_ram(*this, "attr_ram", 128 * 8, ENDIANNESS_LITTLE)
{
}

specnext_sprites_device &specnext_sprites_device::set_palette(const char *tag, u16 base_offset, u16 alt_offset)
{
	device_gfx_interface::set_palette(tag);
	m_palette_base_offset = base_offset,
	m_palette_alt_offset = alt_offset;
	update_config();

	return *this;
}

void specnext_sprites_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 pmask)
{
	if (m_sprites_cache.empty()) update_sprites_cache();
	const rectangle clipped = m_clip_window & cliprect;

	for (auto i = 0; i < m_sprites_cache.size(); i++ )
	{
		const u8 x = m_zero_on_top ? i : (m_sprites_cache.size() - 1) - i;
		const sprite_data &spr = m_sprites_cache[x];

		gfx((spr.rotate << 1) | spr.h)->prio_zoom_transpen(bitmap, clipped
			, spr.pattern >> (1 - spr.h), spr.paloff
			, spr.xmirror, spr.ymirror
			, ((spr.x & 0x1ff) << 1) + m_offset_h, (spr.y & 0x1ff) + m_offset_v
			, 0x20000 << spr.xscale, 0x10000 << spr.yscale
			, screen.priority(), pmask
			, m_transp_colour & (spr.h ? 0x0f : 0xff));
	}
}

/* The cache only stores visible sprites.
  Sprites becomes visible only it's anchor(+v), or relative(+v) to visible anchor.
*/
void specnext_sprites_device::update_sprites_cache()
{
	m_sprites_cache.clear();
	m_sprites_cache.reserve(128);

	sprite_data *anchor = nullptr;
	bool anchor_vis = false;
	u8 *sprite_attr = m_sprite_attr_ram;
	for (auto i = 0; i < 128; i++, sprite_attr += 8)
	{
		const bool spr_relative = BIT(sprite_attr[3], 6) && ((sprite_attr[4] >> 6) == 0b01);
		bool is_visible = BIT(sprite_attr[3], 7);

		if (spr_relative)
			is_visible &= anchor_vis; // skip relative unless visible anchor is found
		else
			anchor_vis = is_visible;

		if (is_visible)
		{
			u8 spr_cur_attr[5] = {};
			sprite_data spr_cur;
			u8 anchor_pattern = 0;
			if (!spr_relative) // Anchor
				memcpy(spr_cur_attr, sprite_attr, 5);
			else
			{
				const bool anchor_rotate =  anchor->rel_type ? anchor->rotate  : 0;
				const bool anchor_xmirror = anchor->rel_type ? anchor->xmirror : 0;
				const bool anchor_ymirror = anchor->rel_type ? anchor->ymirror : 0;
				const u8 anchor_xscale =    anchor->rel_type ? anchor->xscale  : 0b00;
				const u8 anchor_yscale =    anchor->rel_type ? anchor->yscale  : 0b00;

				const u8 spr_rel_x0 = anchor_rotate ? sprite_attr[1] : sprite_attr[0];
				const u8 spr_rel_y0 = anchor_rotate ? sprite_attr[0] : sprite_attr[1];;
				const u8 spr_rel_x1 = (anchor_rotate xor anchor_xmirror) ? (~spr_rel_x0 + 1) : spr_rel_x0;
				const u8 spr_rel_y1 = anchor_ymirror ? (~spr_rel_y0 + 1) : spr_rel_y0;
				const u16 spr_rel_x2 = (((BIT(spr_rel_x1, 7) << 8) | spr_rel_x1) << anchor_xscale) & 0x1ff;
				const u16 spr_rel_y2 = (((BIT(spr_rel_y1, 7) << 8) | spr_rel_y1) << anchor_yscale) & 0x1ff;
				const u16 spr_rel_x3 = (anchor->x + spr_rel_x2) & 0x1ff;
				const u16 spr_rel_y3 = (anchor->y + spr_rel_y2) & 0x1ff;

				const u8 spr_rel_paloff = BIT(sprite_attr[2], 0) ? anchor->paloff + BIT(sprite_attr[2], 4, 4) : BIT(sprite_attr[2], 4, 4);

				const bool spr_rel_xm = anchor_rotate ? BIT(sprite_attr[2], 2) xor BIT(sprite_attr[2], 1) : BIT(sprite_attr[2], 3);
				const bool spr_rel_ym = anchor_rotate ? BIT(sprite_attr[2], 3) xor BIT(sprite_attr[2], 1) : BIT(sprite_attr[2], 2);

				spr_cur_attr[0] = BIT(spr_rel_x3, 0, 8);
				spr_cur_attr[1] = BIT(spr_rel_y3, 0, 8);
				spr_cur_attr[2] = anchor->rel_type
					? (spr_rel_paloff << 4) | ((anchor_xmirror xor spr_rel_xm) << 3) | ((anchor_ymirror xor spr_rel_ym) << 2) | ((anchor_rotate xor BIT(sprite_attr[2], 1)) << 1) | BIT(spr_rel_x3, 8)
					: (spr_rel_paloff << 4) | (BIT(sprite_attr[2], 1, 3) << 1) | BIT(spr_rel_x3, 8);
				spr_cur_attr[3] = (is_visible << 7) | (0b1 << 6) | BIT(sprite_attr[3], 0, TOTAL_PATTERN_BITS);
				spr_cur_attr[4] = anchor->rel_type
					? (anchor->h << 7) | (BIT(sprite_attr[4], 5) << 6) | (anchor_xscale << 3) | (anchor_yscale << 1) | BIT(spr_rel_y3, 8)
					: (anchor->h << 7) | (BIT(sprite_attr[4], 5) << 6) | (BIT(sprite_attr[4], 1, 4) << 1) | BIT(spr_rel_y3, 8);

				anchor_pattern = anchor->pattern;
			}

			bool y8 = BIT(sprite_attr[3], 6) ? BIT(spr_cur_attr[4], 0) : 0;
			spr_cur.y = (y8 << 8) | spr_cur_attr[1];
			spr_cur.x = (BIT(spr_cur_attr[2], 0) << 8) | spr_cur_attr[0];
			spr_cur.rotate = BIT(spr_cur_attr[2], 1);
			spr_cur.ymirror = BIT(spr_cur_attr[2], 2);
			spr_cur.xmirror = BIT(spr_cur_attr[2], 3);
			spr_cur.paloff = BIT(spr_cur_attr[2], 4, 4);

			spr_cur.h = BIT(spr_cur_attr[4], 7) && BIT(sprite_attr[3], 6);
			bool spr_cur_n6 = BIT(spr_cur_attr[4], 6) && spr_cur.h;
			spr_cur.pattern = (BIT(spr_cur_attr[3], 0, TOTAL_PATTERN_BITS) << 1) | spr_cur_n6;
			if (spr_relative && BIT(sprite_attr[4], 0))
				spr_cur.pattern = (spr_cur.pattern + anchor_pattern) & 0x7f;

			spr_cur.yscale = BIT(spr_cur_attr[4], 1, 2);
			spr_cur.xscale = BIT(spr_cur_attr[4], 3, 2);
			spr_cur.rel_type = BIT(sprite_attr[4], 5) && BIT(sprite_attr[3], 6);

			m_sprites_cache.push_back(spr_cur);
			if (!spr_relative)
				anchor = &m_sprites_cache.back();
		}
	}
}

void specnext_sprites_device::update_config()
{
	if (gfx(0) == nullptr) return;

	for (auto i = 0; i < 4; ++i)
	{
		gfx(i)->set_granularity(16); // Change granularity to 16 for 8bpp
		gfx(i)->set_colorbase(m_sprite_palette_select ? m_palette_alt_offset : m_palette_base_offset);
	}

	m_clip_window = SCREEN_AREA; // over + !clip
	if (!m_over_border)
		m_clip_window = rectangle { m_clip_x1 + OVER_BORDER, m_clip_x2 + OVER_BORDER, m_clip_y1 + OVER_BORDER, m_clip_y2 + OVER_BORDER };
	else if (m_border_clip_en)
		m_clip_window = rectangle { m_clip_x1 << 1, (m_clip_x2 << 1) | 1, m_clip_y1, m_clip_y2 };
	m_clip_window.setx(m_clip_window.left() << 1, (m_clip_window.right() << 1) | 1);
	m_clip_window.offset(m_offset_h, m_offset_v);
}

void specnext_sprites_device::io_w(offs_t addr, u8 data)
{
	bool attr_num_change = 0;
	if (addr == 0x5b)
	{
		// Ugh
		gfx(0)->mark_all_dirty();
		gfx(1)->mark_all_dirty();
		gfx(2)->mark_all_dirty();
		gfx(3)->mark_all_dirty();

		m_sprite_pattern_ram[m_pattern_index] = data;
		++m_pattern_index &= 0x3fff;
	}
	else if (addr == 0x303b)
	{
		m_pattern_index = (BIT(data, 0, TOTAL_PATTERN_BITS) << 8) | (data & 0x80);
		m_attr_index = BIT(data, 0, TOTAL_SPRITES_BITS) << 3;
		attr_num_change = 1;
	}
	else if (addr == 0x57)
	{
		m_sprites_cache.clear();
		m_sprite_attr_ram[m_attr_index] = data;

		const bool index_inc_attr_by_8 = BIT(m_attr_index, 2) || ((BIT(m_attr_index, 0, 3) == 0b011) && (BIT(data, 6) == 0));
		const u16 index_inc_in_s = index_inc_attr_by_8
			? BIT(m_attr_index, 3, TOTAL_SPRITES_BITS)
			: m_attr_index;
		const u16 index_inc_out_s = index_inc_in_s + 1;

		if (index_inc_attr_by_8 == 0)
			m_attr_index = BIT(index_inc_out_s, 0, TOTAL_SPRITES_BITS + 3);
		else
		{
			m_attr_index = BIT(index_inc_out_s, 0, TOTAL_SPRITES_BITS) << 3;
			attr_num_change = 1;
		}
	}

	if (attr_num_change && m_mirror_tie)
	{
		m_mirror_sprite_q &= ~0xff;
		m_mirror_sprite_q |= BIT(m_attr_index, 3, TOTAL_SPRITES_BITS);
		m_mirror_sprite_q |= m_pattern_index & 0x80;
	}
}

void specnext_sprites_device::mirror_data_w(u8 mirror_data)
{
	if (m_mirror_index <= 0b100)
	{
		m_sprites_cache.clear();
		m_sprite_attr_ram[(m_mirror_sprite_q << 3) | m_mirror_index] = mirror_data;
	}

	bool mirror_num_change = 0;
	if (m_mirror_index == 0b111)
	{
		m_mirror_sprite_q = mirror_data;
		mirror_num_change = 1;
	}
	else if (m_mirror_inc)
	{
		m_mirror_sprite_q &= ~0xff;
		m_mirror_sprite_q |= (BIT(m_mirror_sprite_q, 0, TOTAL_SPRITES_BITS) + 1) & 0x7f;
		m_mirror_sprite_q |= m_pattern_index & 0x80;
		mirror_num_change = 1;
	}

	if (mirror_num_change && m_mirror_tie)
	{
		m_pattern_index = (BIT(m_mirror_sprite_q, 0, TOTAL_PATTERN_BITS) << 8) | (m_mirror_sprite_q & 0x80);
		m_attr_index = BIT(m_mirror_sprite_q, 0, TOTAL_SPRITES_BITS) << 3;
	}
}

void specnext_sprites_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
	m_clip_window = SCREEN_AREA;
}

void specnext_sprites_device::device_start()
{
	save_item(NAME(m_sprite_palette_select));

	save_item(NAME(m_zero_on_top));
	save_item(NAME(m_border_clip_en));
	save_item(NAME(m_over_border));
	save_item(NAME(m_transp_colour));
	save_item(NAME(m_clip_x1));
	save_item(NAME(m_clip_x2));
	save_item(NAME(m_clip_y1));
	save_item(NAME(m_clip_y2));

	save_item(NAME(m_mirror_tie));
	save_item(NAME(m_mirror_index));
	save_item(NAME(m_mirror_inc));

	save_item(NAME(m_attr_index));
	save_item(NAME(m_pattern_index));
	save_item(NAME(m_mirror_sprite_q));

	update_config();
}

void specnext_sprites_device::device_reset()
{
	m_attr_index = 0;
	m_pattern_index = 0;
	m_mirror_sprite_q = 0;

	memset(m_sprite_attr_ram, 0, 128 * 8);
	m_sprites_cache.clear();
	update_config();
}

void specnext_sprites_device::device_post_load()
{
	m_sprites_cache.clear();
}


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_SPRITES, specnext_sprites_device, "sprites", "Spectrum Next Sprites")
