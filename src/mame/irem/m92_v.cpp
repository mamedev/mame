// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    Irem M92 video hardware, Bryan McPhail, mish@tendril.co.uk

    Brief Overview:

        3 scrolling playfields, 512 by 512.
        Each playfield can enable rowscroll, change shape (to 1024 by 512),
        be enabled/disabled and change position in VRAM.

        Tiles can have several priority values:
            0 = standard
            1 = Top 8 pens appear over sprites (split tilemap)
            2 = Whole tile appears over sprites
            3 = ?  Seems to be the whole tile is over sprites (as 2).

        Sprites have 2 priority values:
            0 = standard
            1 = Sprite appears over all tiles, including high priority pf1

        Raster interrupts can be triggered at any line of the screen redraw,
        typically used in games like R-Type Leo to multiplex the top playfield.

*****************************************************************************

    Master Control registers:

        Word 0: Playfield 1 control
            Bit  0x40:  1 = Rowscroll enable, 0 = disable
            Bit  0x10:  0 = Playfield enable, 1 = disable
            Bit  0x04:  0 = 512 x 512 playfield, 1 = 1024 x 512 playfield
            Bits 0x03:  Playfield location in VRAM (0, 0x4000, 0x8000, 0xc000)
        Word 1: Playfield 2 control (as above)
        Word 2: Playfield 3 control (as above)
        Word 3: Raster IRQ position.

    The raster IRQ position is offset by 128+8 from the first visible line,
    suggesting there are 8 lines before the first visible one.

*****************************************************************************/

#include "emu.h"
#include "m92.h"

/*****************************************************************************/

TIMER_CALLBACK_MEMBER(m92_state::spritebuffer_done)
{
	m_sprite_buffer_busy = 1;
	m_upd71059c->ir1_w(1);
}


void m92_state::spritecontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spritecontrol[offset]);
	// offset0: sprite list size (negative)
	// offset1: ? (always 0)
	// offset2: sprite control
	// offset3: ? (always 0)
	// offset4: sprite dma
	// offset5: ?

	/* Sprite control - display all sprites, or partial list */
	if (offset == 2 && ACCESSING_BITS_0_7)
	{
		if ((data & 0xff) == 8)
			m_sprite_list = (((0x100 - m_spritecontrol[0]) & 0xff) * 4);
		else
			m_sprite_list = 0x400;

		/* Bit 0 is also significant */
	}

	/* Sprite buffer - the data written doesn't matter (confirmed by several games) */
	if (offset == 4)
	{
		/* this implementation is not accurate: still some delayed sprites in gunforc2 (might be another issue?) */
		m_spriteram->copy();
		m_sprite_buffer_busy = 0;
		m_upd71059c->ir1_w(0);

		/* Pixel clock is 26.6666MHz (some boards 27MHz??), we have 0x800 bytes, or 0x400 words to copy from
		spriteram to the buffer.  It seems safe to assume 1 word can be copied per clock. */
		m_spritebuffer_timer->adjust(attotime::from_hz(XTAL(26'666'666)) * 0x400);
	}
//  logerror("%s: spritecontrol_w %08x %08x\n",m_maincpu->pc(),offset,data);
}

void m92_state::videocontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videocontrol);
	/*
	    Many games write:
	        0x2000
	        0x201b in alternate frames.

	    Some games write to this both before and after the sprite buffer
	    register - perhaps some kind of acknowledge bit is in there?

	    Lethal Thunder fails it's RAM test with the upper palette bank
	    enabled.  This was one of the earlier games and could actually
	    be a different motherboard revision (most games use M92-A-B top
	    pcb, a M92-A-A revision could exist...).

	    There is a further test case with R-Type Leo. The flickering
	    invulnerability effect when you spawn does not work correctly
	    with the palette bank hooked up, and also causes a 2nd player
	    spawning in to have the incorrect palette at first.

	    It appears that the only games requiring the palette bank logic
	    are Major Title 2, Ninja Baseball Bat Man, Dream Soccer '94
	    and Gun Force 2.  These are also the games with the extended
	    ROM banking, suggesting a difference on those boards is a more
	    likely explanation.
	*/

	/*
	    fedc ba98 7654 3210
	    .x.. x... .xx. ....   always 0?
	    x... .... .... ....   video off? (but that breaks mysticri)
	    ..xx .... .... ....   ? only written at POST - otherwise always 2
	    .... .xxx .... ....   ? only written at POST - otherwise always 0
	    .... .... x... ....   disable sprites
	    .... .... ...x ....   ?
	    .... .... .... x...   ?
	    .... .... .... .x..   ? maybe more palette banks?
	    .... .... .... ..x.   palette bank
	    .... .... .... ...x   ?
	*/

	/* Access to upper palette bank */
	if (m_palette->entries() == 2048)
		m_palette_bank = (m_videocontrol >> 1) & 1;

//  logerror("%s: videocontrol_w %d = %02x\n",m_maincpu->pc(),offset,data);
}

uint16_t m92_state::paletteram_r(offs_t offset)
{
	return m_paletteram[offset | (m_palette_bank << 10)];
}

void m92_state::paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_palette->write16(offset | (m_palette_bank << 10), data, mem_mask);
}

/*****************************************************************************/

TILE_GET_INFO_MEMBER(m92_state::get_pf_tile_info)
{
	M92_pf_layer_info *layer = (M92_pf_layer_info *)tilemap.user_data();
	int tile, attrib;
	tile_index = 2 * tile_index + layer->vram_base;

	attrib = m_vram_data[tile_index + 1];
	tile = m_vram_data[tile_index] + ((attrib & 0x8000) << 1);

	tileinfo.set(0,
			tile,
			(m_palette->entries() == 2048) ? (attrib & 0x7f) : (attrib & 0x3f),
			TILE_FLIPYX(attrib >> 9));
	if (attrib & 0x100) tileinfo.group = 2;
	else if (attrib & 0x80) tileinfo.group = 1;
	else tileinfo.group = 0;
}

/*****************************************************************************/

void m92_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_data[offset]);

	for (int laynum = 0; laynum < 3; laynum++)
	{
		if ((offset & 0x6000) == m_pf_layer[laynum].vram_base)
		{
			m_pf_layer[laynum].tmap->mark_tile_dirty((offset & 0x1fff) / 2);
			m_pf_layer[laynum].wide_tmap->mark_tile_dirty((offset & 0x3fff) / 2);
		}
		if ((offset & 0x6000) == m_pf_layer[laynum].vram_base + 0x2000)
			m_pf_layer[laynum].wide_tmap->mark_tile_dirty((offset & 0x3fff) / 2);
	}
}

/*****************************************************************************/

void m92_state::master_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t old = m_pf_master_control[offset];
	M92_pf_layer_info *layer;

	COMBINE_DATA(&m_pf_master_control[offset]);

	switch (offset)
	{
		case 0: /* Playfield 1 (top layer) */
		case 1: /* Playfield 2 (middle layer) */
		case 2: /* Playfield 3 (bottom layer) */
			layer = &m_pf_layer[offset];

			/* update VRAM base (bits 0-1) */
			layer->vram_base = (m_pf_master_control[offset] & 3) * 0x2000;

			/* update size (bit 2) */
			if (m_pf_master_control[offset] & 0x04)
			{
				layer->tmap->enable(false);
				layer->wide_tmap->enable((~m_pf_master_control[offset] >> 4) & 1);
			}
			else
			{
				layer->tmap->enable((~m_pf_master_control[offset] >> 4) & 1);
				layer->wide_tmap->enable(false);
			}

			/* mark everything dirty of the VRAM base or size changes */
			if ((old ^ m_pf_master_control[offset]) & 0x07)
			{
				layer->tmap->mark_all_dirty();
				layer->wide_tmap->mark_all_dirty();
			}
			break;

		case 3:
			m_raster_irq_position = (m_pf_master_control[3] & 0x1ff) - 128;
			m_upd71059c->ir2_w(0);
			break;
	}
}

/*****************************************************************************/

void m92_state::video_start()
{
	m_spritebuffer_timer = timer_alloc(FUNC(m92_state::spritebuffer_done), this);

	memset(m_pf_master_control, 0, sizeof(m_pf_master_control));
	memset(&m_pf_layer, 0, sizeof(m_pf_layer));

	for (int laynum = 0; laynum < 3; laynum++)
	{
		M92_pf_layer_info *layer = &m_pf_layer[laynum];

		/* allocate two tilemaps per layer, one normal, one wide */
		layer->tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m92_state::get_pf_tile_info)), TILEMAP_SCAN_ROWS,  8,8, 64,64);
		layer->wide_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m92_state::get_pf_tile_info)), TILEMAP_SCAN_ROWS,  8,8, 128,64);

		/* set the user data for each one to point to the layer */
		layer->tmap->set_user_data(&m_pf_layer[laynum]);
		layer->wide_tmap->set_user_data(&m_pf_layer[laynum]);

		/* set scroll offsets */
		layer->tmap->set_scrolldx(2 * laynum, -2 * laynum + 8);
		layer->tmap->set_scrolldy(-128, -128);
		layer->wide_tmap->set_scrolldx(2 * laynum - 256, -2 * laynum + 8 - 256);
		layer->wide_tmap->set_scrolldy(-128, -128);

		/* layer group 0 - totally transparent in front half */
		layer->tmap->set_transmask(0, 0xffff, (laynum == 2) ? 0x0000 : 0x0001);
		layer->wide_tmap->set_transmask(0, 0xffff, (laynum == 2) ? 0x0000 : 0x0001);

		/* layer group 1 - pens 0-7 transparent in front half */
		layer->tmap->set_transmask(1, 0x00ff, (laynum == 2) ? 0xff00 : 0xff01);
		layer->wide_tmap->set_transmask(1, 0x00ff, (laynum == 2) ? 0xff00 : 0xff01);

		/* layer group 2 - pen 0 transparent in front half */
		layer->tmap->set_transmask(2, 0x0001, (laynum == 2) ? 0xfffe : 0xffff);
		layer->wide_tmap->set_transmask(2, 0x0001, (laynum == 2) ? 0xfffe : 0xffff);

		save_item(NAME(layer->vram_base), laynum);
		save_item(NAME(layer->control), laynum);
	}

	m_paletteram.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 2);

	memset(m_spriteram->live(),0,0x800);
	memset(m_spriteram->buffer(),0,0x800);

	save_item(NAME(m_pf_master_control));
	save_item(NAME(m_videocontrol));
	save_item(NAME(m_sprite_list));
	save_item(NAME(m_raster_irq_position));
	save_item(NAME(m_sprite_buffer_busy));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_paletteram));
}

void ppan_state::video_start()
{
	m92_state::video_start();

	for (int laynum = 0; laynum < 3; laynum++)
	{
		M92_pf_layer_info *layer = &m_pf_layer[laynum];

		/* set scroll offsets */
		layer->tmap->set_scrolldx(2 * laynum + 11, -2 * laynum + 11);
		layer->tmap->set_scrolldy(-8, -8);
		layer->wide_tmap->set_scrolldx(2 * laynum - 256 + 11, -2 * laynum + 11 - 256);
		layer->wide_tmap->set_scrolldy(-8, -8);
	}
}

/*****************************************************************************/

void m92_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t *source = m_spriteram->buffer();

	for (int layer = 0; layer < 8; layer++)
	{
		for (int offs = 0; offs < m_sprite_list; )
		{
			int x = source[offs+3] & 0x1ff;
			int y = source[offs+0] & 0x1ff;
			int code = source[offs+1];
			int color = source[offs+2];
			color &= (m_palette->entries() == 2048) ? 0x7f : 0x3f;
			int pri = (~source[offs+2] >> 6) & 2;
			int curlayer = (source[offs+0] >> 13) & 7;
			int flipx = (source[offs+2] >> 8) & 1;
			int flipy = (source[offs+2] >> 9) & 1;
			int numcols = 1 << ((source[offs+0] >> 11) & 3);
			int numrows = 1 << ((source[offs+0] >> 9) & 3);

			offs += 4 * numcols;
			if (layer != curlayer) continue;

			x = (x - 16) & 0x1ff;
			y = 384 - 16 - y;

			if (flipx) x += 16 * (numcols - 1);

			for (int col = 0; col < numcols; col++)
			{
				int s_ptr = 8 * col;
				if (!flipy) s_ptr += numrows - 1;

				for (int row = 0; row < numrows; row++)
				{
					if (flip_screen())
					{
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, !flipx, !flipy,
								464 - x, 240 - (y - row * 16),
								screen.priority(), pri, 0);

						// wrap around x
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, !flipx, !flipy,
								464 - x + 512, 240 - (y - row * 16),
								screen.priority(), pri, 0);
					}
					else
					{
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, flipx, flipy,
								x, y - row * 16,
								screen.priority(), pri, 0);

						// wrap around x
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, flipx, flipy,
								x - 512, y - row * 16,
								screen.priority(), pri, 0);
					}
					if (flipy) s_ptr++;
					else s_ptr--;
				}
				if (flipx) x -= 16;
				else x += 16;
			}
		}
	}
}

// This needs a lot of work...
void ppan_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t *source = m_spriteram->live(); // sprite buffer control is never triggered
	int offs, layer;

	for (layer = 0; layer < 8; layer++)
	{
		for (offs = 0; offs < m_sprite_list; )
		{
			int x = source[offs+3] & 0x1ff;
			int y = source[offs+0] & 0x1ff;
			int code = source[offs+1];
			int color = source[offs+2] & 0x007f;
			int pri = (~source[offs+2] >> 6) & 2;
			int curlayer = (source[offs+0] >> 13) & 7;
			int flipx = (source[offs+2] >> 8) & 1;
			int flipy = (source[offs+2] >> 9) & 1;
			int numcols = 1 << ((source[offs+0] >> 11) & 3);
			int numrows = 1 << ((source[offs+0] >> 9) & 3);
			int row, col, s_ptr;

			offs += 4 * numcols;
			if (layer != curlayer) continue;

			y = 384 - 16 - y - 7;
			y -= 128;
			if (y < 0) y += 512;

			if (flipx) x += 16 * (numcols - 1);

			for (col = 0; col < numcols; col++)
			{
				s_ptr = 8 * col;
				if (!flipy) s_ptr += numrows - 1;

				for (row = 0; row < numrows; row++)
				{
					if (flip_screen())
					{
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, !flipx, !flipy,
								464 - x, 240 - (y - row * 16),
								screen.priority(), pri, 0);

						// wrap around x
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, !flipx, !flipy,
								464 - x + 512, 240 - (y - row * 16),
								screen.priority(), pri, 0);
					}
					else
					{
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, flipx, flipy,
								x, y - row * 16,
								screen.priority(), pri, 0);

						// wrap around x
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								code + s_ptr, color, flipx, flipy,
								x - 512, y - row * 16,
								screen.priority(), pri, 0);
					}
					if (flipy) s_ptr++;
					else s_ptr--;
				}
				if (flipx) x -= 16;
				else x += 16;
			}
		}
	}
}

/*****************************************************************************/

void m92_state::m92_update_scroll_positions()
{
	/*  Playfield 3 rowscroll data is 0xdfc00 - 0xdffff
	    Playfield 2 rowscroll data is 0xdf800 - 0xdfbff
	    Playfield 1 rowscroll data is 0xdf400 - 0xdf7ff

	    It appears to be hardwired to those locations.

	    In addition, each playfield is staggered 2 pixels horizontally from the
	    previous one.  This is most obvious in Hook & Blademaster.

	*/

	for (int laynum = 0; laynum < 3; laynum++)
	{
		M92_pf_layer_info *layer = &m_pf_layer[laynum];

		if (m_pf_master_control[laynum] & 0x40)
		{
			const uint16_t *scrolldata = m_vram_data + (0xf400 + 0x400 * laynum) / 2;

			layer->tmap->set_scroll_rows(512);
			layer->wide_tmap->set_scroll_rows(512);
			for (int i = 0; i < 512; i++)
			{
				layer->tmap->set_scrollx(i, scrolldata[i]);
				layer->wide_tmap->set_scrollx(i, scrolldata[i]);
			}
		}
		else
		{
			layer->tmap->set_scroll_rows(1);
			layer->wide_tmap->set_scroll_rows(1);
			layer->tmap->set_scrollx(0, layer->control[2]);
			layer->wide_tmap->set_scrollx(0, layer->control[2]);
		}

		layer->tmap->set_scrolly(0, layer->control[0]);
		layer->wide_tmap->set_scrolly(0, layer->control[0]);
	}
}

/*****************************************************************************/

void m92_state::m92_draw_tiles(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((~m_pf_master_control[2] >> 4) & 1)
	{
		m_pf_layer[2].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
		m_pf_layer[2].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
		m_pf_layer[2].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
		m_pf_layer[2].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	}

	m_pf_layer[1].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_pf_layer[1].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_pf_layer[1].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_pf_layer[1].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);

	m_pf_layer[0].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_pf_layer[0].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_pf_layer[0].wide_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_pf_layer[0].tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
}


uint32_t m92_state::screen_update_m92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Flipscreen appears hardwired to the dipswitch - strange */
	if (m_dsw->read() & 0x100)
		flip_screen_set(0);
	else
		flip_screen_set(1);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m92_update_scroll_positions();
	m92_draw_tiles(screen, bitmap, cliprect);

	if (~m_videocontrol & 0x80)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}

uint32_t m92_state::screen_update_nbbatman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// nbbatman is the only game using this flag to turn off video? (normally, games just disable each tile layer)
	if (m_videocontrol & 0x8000)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	return screen_update_m92(screen, bitmap, cliprect);
}
