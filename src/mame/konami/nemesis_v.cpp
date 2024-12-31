// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  video.c

***************************************************************************/

#include "emu.h"
#include "nemesis.h"
#include "video/resnet.h"


static const struct
{
	uint8_t width;
	uint8_t height;
	uint8_t char_type;
}
sprite_data[8] =
{
	{ 32, 32, 4 }, { 16, 32, 5 }, { 32, 16, 2 }, { 64, 64, 7 },
	{  8,  8, 0 }, { 16,  8, 6 }, {  8, 16, 3 }, { 16, 16, 1 }
};

template <unsigned Which>
TILE_GET_INFO_MEMBER(gx400_base_state::get_tile_info)
{
	int code = m_videoram[Which][tile_index];
	int color = m_colorram[Which][tile_index];
	int flags = 0;

	if (BIT(color, 7))
		flags |= TILE_FLIPX;

	if (BIT(code, 11))
		flags |= TILE_FLIPY;

	if (BIT(~code, 13) || ((code & 0xc000) == 0x4000))
			flags |= TILE_FORCE_LAYER0;     /* no transparency */

	if (code & 0xf800)
	{
		tileinfo.set(0, code & 0x7ff, color & 0x7f, flags);
	}
	else
	{
		tileinfo.set(0, 0, 0x00, 0);
		tileinfo.pen_data = m_blank_tile;
	}

	int const mask = BIT(code, 12);
	int layer = BIT(code, 14);
	if (mask && !layer)
		layer = 1;

	tileinfo.category = mask | (layer << 1);
}


void gx400_state::gfx_flipx_w(int state)
{
	m_flipscreen = state;

	if (state)
		m_tilemap_flip |= TILEMAP_FLIPX;
	else
		m_tilemap_flip &= ~TILEMAP_FLIPX;

	machine().tilemap().set_flip_all(m_tilemap_flip);
}

void gx400_state::gfx_flipy_w(int state)
{
	if (state)
		m_tilemap_flip |= TILEMAP_FLIPY;
	else
		m_tilemap_flip &= ~TILEMAP_FLIPY;

	machine().tilemap().set_flip_all(m_tilemap_flip);
}


void salamand_state::control_port_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		uint8_t const accessing_bits = data ^ m_irq_port_last;

		m_irq_on = BIT(data, 0);
		m_irq2_on = BIT(data, 1);
		m_flipscreen = BIT(data, 2);

		if (BIT(data, 2))
			m_tilemap_flip |= TILEMAP_FLIPX;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPX;

		if (BIT(data, 3))
			m_tilemap_flip |= TILEMAP_FLIPY;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPY;

		if (accessing_bits & 0x0c)
			machine().tilemap().set_flip_all(m_tilemap_flip);

		m_irq_port_last = data;
	}

	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_lockout_w(0, BIT(data, 9));
		machine().bookkeeping().coin_lockout_w(1, BIT(data, 10));

		if (BIT(data, 11))
			m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

void hcrash_state::citybomb_control_port_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	control_port_word_w(offset, data, mem_mask);
	if (ACCESSING_BITS_8_15)
	{
		m_selected_ip = BIT(~data, 12);     /* citybomb steering & accel */
	}
}

void gx400_state::create_palette_lookups()
{
	// driver is 74LS09 (AND gates with open collector)
	static const res_net_info nemesis_net_info =
	{
		RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
		{
			{ RES_NET_AMP_EMITTER, 1000, 0, 5, { 4700, 2400, 1200, 620, 300 } },
			{ RES_NET_AMP_EMITTER, 1000, 0, 5, { 4700, 2400, 1200, 620, 300 } },
			{ RES_NET_AMP_EMITTER, 1000, 0, 5, { 4700, 2400, 1200, 620, 300 } }
		}
	};

	for (int i = 0; i < 32; i++)
		m_palette_lookup[i] = compute_res_net(i, 0, nemesis_net_info);

	// normalize black/white levels
	double black = m_palette_lookup[0];
	double white = 255.0 / (m_palette_lookup[31] - black);
	for (auto & elem : m_palette_lookup)
		elem = (elem - black) * white + 0.5;
}


void gx400_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	data = m_paletteram[offset];

	int const r = (data >> 0) & 0x1f;
	int const g = (data >> 5) & 0x1f;
	int const b = (data >> 10) & 0x1f;
	m_palette->set_pen_color(offset, m_palette_lookup[r],m_palette_lookup[g],m_palette_lookup[b]);
}


void gx400_base_state::charram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldword = m_charram[offset];

	COMBINE_DATA(m_charram + offset);
	data = m_charram[offset];

	if (oldword != data)
	{
		for (int i = 0; i < 8; i++)
		{
			int const w = sprite_data[i].width;
			int const h = sprite_data[i].height;
			m_gfxdecode->gfx(sprite_data[i].char_type)->mark_dirty(offset * 4 / (w * h));
		}
	}
}


void gx400_base_state::device_post_load()
{
	for (int i = 0; i < 8; i++)
	{
		m_gfxdecode->gfx(i)->mark_all_dirty();
	}
}


void gx400_base_state::video_start()
{
	m_spriteram_words = m_spriteram.bytes() / 2;

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gx400_base_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gx400_base_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[0]->set_scroll_rows(256);
	m_tilemap[1]->set_scroll_rows(256);

	memset(m_charram, 0, m_charram.bytes());
	std::fill(std::begin(m_blank_tile), std::end(m_blank_tile), 0);
}


void gx400_state::video_start()
{
	create_palette_lookups();

	gx400_base_state::video_start();
}


void gx400_base_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*
	 *  16 bytes per sprite, in memory from 56000-56fff
	 *
	 *  byte    0 : relative priority.
	 *  byte    2 : size (?) value #E0 means not used., bit 0x01 is flipx
	                0xc0 is upper 2 bits of zoom.
	                0x38 is size.
	 *  byte    4 : zoom = 0xff
	 *  byte    6 : low bits sprite code.
	 *  byte    8 : color + hi bits sprite code., bit 0x20 is flipy bit. bit 0x01 is high bit of X pos.
	 *  byte    A : X position.
	 *  byte    C : Y position.
	 *  byte    E : not used.
	 */

	for (int priority = 256 - 1; priority >= 0; priority--)
	{
		for (int address = m_spriteram_words - 8; address >= 0; address -= 8)
		{
			if((m_spriteram[address] & 0xff) != priority)
				continue;

			int zoom = m_spriteram[address + 2] & 0xff;
			int code;   /* start of sprite in obj RAM */
			if (!(m_spriteram[address + 2] & 0xff00) && ((m_spriteram[address + 3] & 0xff00) != 0xff00))
				code = m_spriteram[address + 3] + ((m_spriteram[address + 4] & 0xc0) << 2);
			else
				code = (m_spriteram[address + 3] & 0xff) + ((m_spriteram[address + 4] & 0xc0) << 2);

			if (zoom != 0xff || code != 0)
			{
				int const size = m_spriteram[address + 1];
				zoom += (size & 0xc0) << 2;

				int sx = m_spriteram[address + 5] & 0xff;
				int sy = m_spriteram[address + 6] & 0xff;
				if (BIT(m_spriteram[address + 4], 0))
					sx -= 0x100;  /* fixes left side clip */

				int const color = (m_spriteram[address + 4] & 0x1e) >> 1;
				bool flipx = BIT(m_spriteram[address + 1], 0);
				bool flipy = BIT(m_spriteram[address + 4], 5);

				int const idx = (size >> 3) & 7;
				int const w = sprite_data[idx].width;
				int const h = sprite_data[idx].height;
				code = code * 8 * 16 / (w * h);
				int const char_type = sprite_data[idx].char_type;

				if (zoom)
				{
					zoom = ((1 << 16) * 0x80 / zoom) + 0x02ab;
					if (m_flipscreen)
					{
						sx = 256 - ((zoom * w) >> 16) - sx;
						sy = 256 - ((zoom * h) >> 16) - sy;
						flipx = !flipx;
						flipy = !flipy;
					}

					m_gfxdecode->gfx(char_type)->prio_zoom_transpen(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						zoom, zoom,
						screen.priority(), 0xffcc, 0);
				}
			}
		}
	}
}

/******************************************************************************/

uint32_t gx400_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	clip.min_x = 0;
	clip.max_x = 255;

	m_tilemap[1]->set_scroll_cols(64);
	m_tilemap[0]->set_scroll_cols(64);
	m_tilemap[1]->set_scroll_rows(1);
	m_tilemap[0]->set_scroll_rows(1);

	for (int offs = 0; offs < 64; offs++)
	{
		int offset_x = offs;

		if (m_flipscreen)
			offset_x = (offs + 0x20) & 0x3f;

		m_tilemap[1]->set_scrolly(offs, m_yscroll[1][offset_x]);
		m_tilemap[0]->set_scrolly(offs, m_yscroll[0][offset_x]);
	}

	for (int offs = cliprect.min_y; offs <= cliprect.max_y; offs++)
	{
		int offset_y = offs;

		clip.min_y = offs;
		clip.max_y = offs;

		if (m_flipscreen)
			offset_y = 255 - offs;

		m_tilemap[1]->set_scrollx(0, (m_xscroll[1][offset_y] & 0xff) + ((m_xscroll[1][0x100 + offset_y] & 0x01) << 8) - (m_flipscreen ? 0x107 : 0));
		m_tilemap[0]->set_scrollx(0, (m_xscroll[0][offset_y] & 0xff) + ((m_xscroll[0][0x100 + offset_y] & 0x01) << 8) - (m_flipscreen ? 0x107 : 0));

		for (int i = 0; i < 4; i += 2)
		{
			m_tilemap[1]->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			m_tilemap[1]->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 1), 2);
			m_tilemap[0]->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			m_tilemap[0]->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 1), 2);
		}
	}

	draw_sprites(screen,bitmap,cliprect);

	return 0;
}
