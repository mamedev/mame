// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  video.c

***************************************************************************/

#include "emu.h"
#include "includes/nemesis.h"
#include "video/resnet.h"


static const struct
{
	UINT8 width;
	UINT8 height;
	UINT8 char_type;
}
sprite_data[8] =
{
	{ 32, 32, 4 }, { 16, 32, 5 }, { 32, 16, 2 }, { 64, 64, 7 },
	{  8,  8, 0 }, { 16,  8, 6 }, {  8, 16, 3 }, { 16, 16, 1 }
};


TILE_GET_INFO_MEMBER(nemesis_state::get_bg_tile_info)
{
	int code, color, flags, mask, layer;

	code = m_videoram2[tile_index];
	color = m_colorram2[tile_index];
	flags = 0;

	if (color & 0x80)
		flags |= TILE_FLIPX;

	if (code & 0x0800)
		flags |= TILE_FLIPY;

	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
			flags |= TILE_FORCE_LAYER0;     /* no transparency */

	if (code & 0xf800)
	{
		SET_TILE_INFO_MEMBER(0, code & 0x7ff, color & 0x7f, flags );
	}
	else
	{
		SET_TILE_INFO_MEMBER(0, 0, 0x00, 0 );
		tileinfo.pen_data = m_blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo.category = mask | (layer << 1);
}

TILE_GET_INFO_MEMBER(nemesis_state::get_fg_tile_info)
{
	int code, color, flags, mask, layer;

	code = m_videoram1[tile_index];
	color = m_colorram1[tile_index];
	flags = 0;

	if (color & 0x80)
		flags |= TILE_FLIPX;

	if (code & 0x0800)
		flags |= TILE_FLIPY;

	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
			flags |= TILE_FORCE_LAYER0;     /* no transparency */

	if (code & 0xf800)
	{
		SET_TILE_INFO_MEMBER(0, code & 0x7ff, color & 0x7f, flags );
	}
	else
	{
		SET_TILE_INFO_MEMBER(0, 0, 0x00, 0 );
		tileinfo.pen_data = m_blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo.category = mask | (layer << 1);
}


WRITE16_MEMBER(nemesis_state::nemesis_gfx_flipx_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_flipscreen = data & 0x01;

		if (data & 0x01)
			m_tilemap_flip |= TILEMAP_FLIPX;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPX;

		machine().tilemap().set_flip_all(m_tilemap_flip);
	}

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0x0100)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	}
}

WRITE16_MEMBER(nemesis_state::nemesis_gfx_flipy_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x01)
			m_tilemap_flip |= TILEMAP_FLIPY;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPY;

		machine().tilemap().set_flip_all(m_tilemap_flip);
	}
}


WRITE16_MEMBER(nemesis_state::salamand_control_port_word_w)
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 accessing_bits = data ^ m_irq_port_last;

		m_irq_on = data & 0x01;
		m_irq2_on = data & 0x02;
		m_flipscreen = data & 0x04;

		if (data & 0x04)
			m_tilemap_flip |= TILEMAP_FLIPX;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPX;

		if (data & 0x08)
			m_tilemap_flip |= TILEMAP_FLIPY;
		else
			m_tilemap_flip &= ~TILEMAP_FLIPY;

		if (accessing_bits & 0x0c)
			machine().tilemap().set_flip_all(m_tilemap_flip);

		m_irq_port_last = data;
	}

	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_lockout_w(0, data & 0x0200);
		machine().bookkeeping().coin_lockout_w(1, data & 0x0400);

		if (data & 0x0800)
			m_audiocpu->set_input_line(0, HOLD_LINE);

		m_selected_ip = (~data & 0x1000) >> 12;     /* citybomb steering & accel */
	}
}

void nemesis_state::create_palette_lookups()
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


WRITE16_MEMBER(nemesis_state::nemesis_palette_word_w)
{
	COMBINE_DATA(m_paletteram + offset);
	data = m_paletteram[offset];

	int r = (data >> 0) & 0x1f;
	int g = (data >> 5) & 0x1f;
	int b = (data >> 10) & 0x1f;
	m_palette->set_pen_color(offset, m_palette_lookup[r],m_palette_lookup[g],m_palette_lookup[b]);
}


WRITE16_MEMBER(nemesis_state::nemesis_videoram1_word_w)
{
	COMBINE_DATA(m_videoram1 + offset);
	m_foreground->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nemesis_state::nemesis_videoram2_word_w)
{
	COMBINE_DATA(m_videoram2 + offset);
	m_background->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nemesis_state::nemesis_colorram1_word_w)
{
	COMBINE_DATA(m_colorram1 + offset);
	m_foreground->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nemesis_state::nemesis_colorram2_word_w)
{
	COMBINE_DATA(m_colorram2 + offset);
	m_background->mark_tile_dirty(offset);
}


WRITE16_MEMBER(nemesis_state::nemesis_charram_word_w)
{
	UINT16 oldword = m_charram[offset];

	COMBINE_DATA(m_charram + offset);
	data = m_charram[offset];

	if (oldword != data)
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			m_gfxdecode->gfx(sprite_data[i].char_type)->mark_dirty(offset * 4 / (w * h));
		}
	}
}


void nemesis_state::nemesis_postload()
{
	for (int i = 0; i < 8; i++)
	{
		m_gfxdecode->gfx(i)->mark_all_dirty();
	}
}


void nemesis_state::video_start()
{
	create_palette_lookups();

	m_spriteram_words = m_spriteram.bytes() / 2;

	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nemesis_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nemesis_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_background->set_transparent_pen(0);
	m_foreground->set_transparent_pen(0);
	m_background->set_scroll_rows(256);
	m_foreground->set_scroll_rows(256);

	memset(m_charram, 0, m_charram.bytes());
	memset(m_blank_tile, 0, ARRAY_LENGTH(m_blank_tile));

	/* Set up save state */
	machine().save().register_postload(save_prepost_delegate(FUNC(nemesis_state::nemesis_postload), this));
}


void nemesis_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
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

	UINT16 *spriteram = m_spriteram;
	int address;    /* start of sprite in spriteram */
	int sx; /* sprite X-pos */
	int sy; /* sprite Y-pos */
	int code;   /* start of sprite in obj RAM */
	int color;  /* color of the sprite */
	int flipx,flipy;
	int zoom;
	int char_type;
	int priority;
	int size;
	int w,h;
	int idx;

	for (priority = 256 - 1; priority >= 0; priority--)
	{
		for (address = m_spriteram_words - 8; address >= 0; address -= 8)
		{
			if((spriteram[address] & 0xff) != priority)
				continue;

			zoom = spriteram[address + 2] & 0xff;
			if (!(spriteram[address + 2] & 0xff00) && ((spriteram[address + 3] & 0xff00) != 0xff00))
				code = spriteram[address + 3] + ((spriteram[address + 4] & 0xc0) << 2);
			else
				code = (spriteram[address + 3] & 0xff) + ((spriteram[address + 4] & 0xc0) << 2);

			if (zoom != 0xff || code != 0)
			{
				size = spriteram[address + 1];
				zoom += (size & 0xc0) << 2;

				sx = spriteram[address + 5] & 0xff;
				sy = spriteram[address + 6] & 0xff;
				if (spriteram[address + 4] & 0x01)
					sx-=0x100;  /* fixes left side clip */

				color = (spriteram[address + 4] & 0x1e) >> 1;
				flipx = spriteram[address + 1] & 0x01;
				flipy = spriteram[address + 4] & 0x20;

				idx = (size >> 3) & 7;
				w = sprite_data[idx].width;
				h = sprite_data[idx].height;
				code = code * 8 * 16 / (w * h);
				char_type = sprite_data[idx].char_type;

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

					m_gfxdecode->gfx(char_type)->prio_zoom_transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						sx,sy,
						zoom,zoom,
						screen.priority(),0xffcc,0 );
				}
			}
		}
	}
}

/******************************************************************************/

UINT32 nemesis_state::screen_update_nemesis(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	rectangle clip;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	clip.min_x = 0;
	clip.max_x = 255;

	m_background->set_scroll_cols(64);
	m_foreground->set_scroll_cols(64);
	m_background->set_scroll_rows(1);
	m_foreground->set_scroll_rows(1);

	for (offs = 0; offs < 64; offs++)
	{
		int offset_x = offs;

		if (m_flipscreen)
			offset_x = (offs + 0x20) & 0x3f;

		m_background->set_scrolly(offs, m_yscroll2[offset_x]);
		m_foreground->set_scrolly(offs, m_yscroll1[offset_x]);
	}

	for (offs = cliprect.min_y; offs <= cliprect.max_y; offs++)
	{
		int i;
		int offset_y = offs;

		clip.min_y = offs;
		clip.max_y = offs;

		if (m_flipscreen)
			offset_y = 255 - offs;

		m_background->set_scrollx(0, (m_xscroll2[offset_y] & 0xff) + ((m_xscroll2[0x100 + offset_y] & 0x01) << 8) - (m_flipscreen ? 0x107 : 0));
		m_foreground->set_scrollx(0, (m_xscroll1[offset_y] & 0xff) + ((m_xscroll1[0x100 + offset_y] & 0x01) << 8) - (m_flipscreen ? 0x107 : 0));

		for (i = 0; i < 4; i += 2)
		{
			m_background->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			m_background->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 1), 2);
			m_foreground->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			m_foreground->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(i + 1), 2);
		}
	}

	draw_sprites(screen,bitmap,cliprect);

	return 0;
}
