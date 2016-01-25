// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/****************************************************************************

    Irem M107 video hardware, Bryan McPhail, mish@tendril.co.uk

    Close to M92 hardware, but with 4 playfields, not 3.
    Twice as many colours, twice as many sprites.

*****************************************************************************

    Port:
        0x80: pf1 Y scroll
        0x82: pf1 X scroll
        0x84: pf2 Y scroll
        0x86: pf2 X scroll
        0x88: pf3 Y scroll
        0x8a: pf3 X scroll
        0x8c: pf4 Y scroll
        0x8e: pf4 X scroll

        0x90: pf1 control
        0x92: pf2 control
        0x94: pf3 control
        0x96: pf4 control

        0x98: Priority?
        0x9a:
        0x9c:
        0x9e: Raster interrupt value

    Playfield control:
        Bit  0x0f00:    Playfield location in VRAM (in steps of 0x1000)
        Bit  0x0080:    0 = Playfield enable, 1 = disable
        Bit  0x0002:    1 = Rowselect enable, 0 = disable
        Bit  0x0001:    1 = Rowscroll enable, 0 = disable

*****************************************************************************/

#include "emu.h"
#include "includes/m107.h"



/*****************************************************************************/

TILE_GET_INFO_MEMBER(m107_state::get_pf_tile_info)
{
	pf_layer_info *layer = (pf_layer_info *)tilemap.user_data();
	int tile, attrib;
	tile_index = 2 * tile_index + layer->vram_base;

	attrib = m_vram_data[tile_index + 1];
	tile = m_vram_data[tile_index] + ((attrib & 0x1000) << 4);

	SET_TILE_INFO_MEMBER(0,
			tile,
			attrib & 0x7f,
			TILE_FLIPYX(attrib >> 10));

	/* Priority 1 = tile appears above sprites */
	tileinfo.category = (attrib >> 9) & 1;
}

/*****************************************************************************/

WRITE16_MEMBER(m107_state::vram_w)
{
	int laynum;

	COMBINE_DATA(&m_vram_data[offset]);
	for (laynum = 0; laynum < 4; laynum++)
		if ((offset & 0x6000) == m_pf_layer[laynum].vram_base)
			m_pf_layer[laynum].tmap->mark_tile_dirty((offset & 0x1fff) / 2);
}

/*****************************************************************************/

WRITE16_MEMBER(m107_state::control_w)
{
	UINT16 old = m_control[offset];
	pf_layer_info *layer;

	COMBINE_DATA(&m_control[offset]);

	switch (offset*2)
	{
		case 0x10: /* Playfield 1 (top layer) */
		case 0x12: /* Playfield 2 */
		case 0x14: /* Playfield 3 */
		case 0x16: /* Playfield 4 (bottom layer) */
			layer = &m_pf_layer[offset - 0x08];

			/* update VRAM base (bits 8-11) */
			layer->vram_base = ((m_control[offset] >> 8) & 15) * 0x800;

			/* update enable (bit 7) */
			layer->tmap->enable((~m_control[offset] >> 7) & 1);

			/* mark everything dirty of the VRAM base changes */
			if ((old ^ m_control[offset]) & 0x0f00)
				layer->tmap->mark_all_dirty();

			if(m_control[offset] & 0xf07c)
				printf("%04x %02x\n",m_control[offset],offset*2);

			break;

		case 0x18:
		case 0x1a:
		case 0x1c:
			break;

		case 0x1e:
			m_raster_irq_position = m_control[offset] - 128;
			break;
	}
}

/*****************************************************************************/

void m107_state::video_start()
{
	int laynum;

	for (laynum = 0; laynum < 4; laynum++)
	{
		pf_layer_info *layer = &m_pf_layer[laynum];

		/* allocate a tilemaps per layer */
		layer->tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m107_state::get_pf_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,64);

		/* set the user data to point to the layer */
		layer->tmap->set_user_data(&m_pf_layer[laynum]);

		/* set scroll offsets */
		layer->tmap->set_scrolldx(-3 + 2 * laynum, -3 + 2 * laynum);
		layer->tmap->set_scrolldy(-128, -128);

		/* set pen 0 to transparent for all tilemaps except #4 */
		if (laynum != 3)
			layer->tmap->set_transparent_pen(0);
	}

	m_buffered_spriteram = make_unique_clear<UINT16[]>(0x1000/2);

	save_item(NAME(m_sprite_display));
	save_item(NAME(m_raster_irq_position));
	save_item(NAME(m_control));
	save_pointer(NAME(m_buffered_spriteram.get()), 0x1000/2);

	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_pf_layer[i].vram_base), i);
	}
}

/*****************************************************************************/

void m107_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *spriteram = m_buffered_spriteram.get();
	int offs;
	UINT8 *rom = m_user1_ptr;
	
	for (offs = 0;offs < 0x800;offs += 4)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i,s_ptr,pri_mask;

		pri_mask = (!(spriteram[offs+2]&0x80)) ? 2 : 0;

		y=spriteram[offs+0];
		x=spriteram[offs+3];
		x&=0x1ff;
		y&=0x1ff;

		if (x==0 || y==0) continue; /* offscreen */

		sprite=spriteram[offs+1]&0x7fff;

		x = x - 16;
		y = 384 - 16 - y;

		colour=spriteram[offs+2]&0x7f;
		fx=(spriteram[offs+2]>>8)&0x1;
		fy=(spriteram[offs+2]>>8)&0x2;
		y_multi=(spriteram[offs+0]>>11)&0x3;

		if (m_spritesystem == 0)
		{
			y_multi=1 << y_multi; /* 1, 2, 4 or 8 */

			s_ptr = 0;
			if (!fy) s_ptr+=y_multi-1;

			for (i=0; i<y_multi; i++)
			{
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
						sprite + s_ptr,
						colour,
						fx,fy,
						x,y-i*16,
						screen.priority(),pri_mask,0);

				/* wrap-around y */
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
						sprite + s_ptr,
						colour,
						fx,fy,
						x,(y-i*16) - 0x200,
						screen.priority(),pri_mask,0);

				if (fy) s_ptr++; else s_ptr--;
			}
		}
		else
		{
			int rom_offs = sprite*8;

			if (!rom)
				return;

			if (rom[rom_offs+1] || rom[rom_offs+3] || rom[rom_offs+5] || rom[rom_offs+7])
			{
				while (rom_offs < 0x40000)  /* safety check */
				{
					/*
					[1]
					x--- ---- end of block marker
					---- --x- Flip Y
					---- ---x Flip X
					[2]
					xxxx xxxx Y offs lo byte
					[3]
					---- xxx- height (1/2/4/8)
					---- ---x Y offs hi byte
					[4]
					xxxx xxxx sprite number lo byte
					[5]
					xxxx xxxx sprite number hi byte
					[6]
					xxxx xxxx X offs lo byte
					[7]
					---- ---x X offs hi byte
					*/

					int xdisp = rom[rom_offs+6]+256*rom[rom_offs+7];
					int ydisp = rom[rom_offs+2]+256*rom[rom_offs+3];
					int ffx=fx^(rom[rom_offs+1]&1);
					int ffy=fy^(rom[rom_offs+1]&2);
					sprite=rom[rom_offs+4]+256*rom[rom_offs+5];
					y_multi=1<<((rom[rom_offs+3]>>1)&0x3);
					if (fx) xdisp = -xdisp-16;
					if (fy) ydisp = -ydisp - (16*y_multi-1);
					if (!ffy) sprite+=y_multi-1;
					for (i=0; i<y_multi; i++)
					{
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								sprite+(ffy?i:-i),
								colour,
								ffx,ffy,
								(x+xdisp)&0x1ff,(y-ydisp-16*i)&0x1ff,
								screen.priority(),pri_mask,0);

						/* wrap-around y */
						m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
								sprite+(ffy?i:-i),
								colour,
								ffx,ffy,
								(x+xdisp)&0x1ff,((y-ydisp-16*i)&0x1ff)-0x200,
								screen.priority(),pri_mask,0);
					}

					if (rom[rom_offs+1]&0x80) break;    /* end of block */

					rom_offs += 8;
				}
			}
		}
	}
}

/*****************************************************************************/

void m107_state::update_scroll_positions()
{
	int laynum;
	int i;

	/*
	    rowscroll is at 0xde000 - 0xde7ff, every layer has dedicated 0x200 bytes inside this area, enabled with bit 0 of the layer video register
	    rowselect is at 0xde800 - 0xdefff, every layer has dedicated 0x200 bytes inside this area, enabled with bit 1 of the layer video register
	    Perhaps 0xdf000 - 0xdffff and bit 2-3 are respectively colscroll and colselect?
	*/

	for (laynum = 0; laynum < 4; laynum++)
	{
		pf_layer_info *layer = &m_pf_layer[laynum];

		int scrolly = m_control[0 + 2 * laynum];
		int scrollx = m_control[1 + 2 * laynum];

		if (m_control[0x08 + laynum] & 0x01) //used by World PK Soccer goal scrolling and Fire Barrel sea wave effect (stage 2) / canyon parallax effect (stage 6)
		{
			const UINT16 *scrolldata = m_vram_data + (0xe000 + 0x200 * laynum) / 2;

			layer->tmap->set_scroll_rows(512);
			for (i = 0; i < 512; i++)
				layer->tmap->set_scrollx(i, scrolldata[((i+0xff80)-(scrolly))&0x1ff] + scrollx);

		}
		else
		{
			layer->tmap->set_scroll_rows(1);
			layer->tmap->set_scrollx(0, scrollx);
		}

		layer->tmap->set_scrolly(0,scrolly);
	}
}

/*****************************************************************************/

void m107_state::tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int laynum, int category,int opaque)
{
	int line;
	rectangle clip;
	const rectangle &visarea = m_screen->visible_area();
	clip = visarea;

	if (m_control[0x08 + laynum] & 0x02)
	{
		for (line = cliprect.min_y; line <= cliprect.max_y;line++)
		{
			const UINT16 *scrolldata = m_vram_data + (0xe800 + 0x200 * laynum) / 2;
			clip.min_y = clip.max_y = line;

			m_pf_layer[laynum].tmap->set_scrollx(0,  m_control[1 + 2 * laynum]);
			m_pf_layer[laynum].tmap->set_scrolly(0,  (m_control[0 + 2 * laynum] + scrolldata[line]));

			m_pf_layer[laynum].tmap->draw(screen, bitmap, clip, category | opaque, category);
		}
	}
	else
		m_pf_layer[laynum].tmap->draw(screen, bitmap, cliprect, category | opaque, category);
}


void m107_state::screenrefresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	if ((~m_control[0x0b] >> 7) & 1)
	{
		tilemap_draw(screen, bitmap, cliprect, 3, 0,0);
		tilemap_draw(screen, bitmap, cliprect, 3, 1,0);
	}
	else
		bitmap.fill(0, cliprect);

	/* note: the opaque flag is used if layer 3 is disabled, noticeable in World PK Soccer title and gameplay screens */
	tilemap_draw(screen, bitmap, cliprect, 2, 0,(((m_control[0x0b] >> 7) & 1) ? TILEMAP_DRAW_OPAQUE : 0));
	tilemap_draw(screen, bitmap, cliprect, 1, 0,0);
	tilemap_draw(screen, bitmap, cliprect, 0, 0,0);
	tilemap_draw(screen, bitmap, cliprect, 2, 1,0);
	tilemap_draw(screen, bitmap, cliprect, 1, 1,0);
	tilemap_draw(screen, bitmap, cliprect, 0, 1,0);

	if(m_sprite_display)
		draw_sprites(screen, bitmap, cliprect);

	/* This hardware probably has more priority values - but I haven't found
	    any used yet */
}

/*****************************************************************************/

WRITE16_MEMBER(m107_state::spritebuffer_w)
{
	if (ACCESSING_BITS_0_7) {
		/*
		TODO: this register looks a lot more complex than how the game uses it. All of them seems to test various bit combinations during POST.
		*/
//      logerror("%04x: buffered spriteram\n",space.device().safe_pc());
		m_sprite_display    = (!(data & 0x1000));

		memcpy(m_buffered_spriteram.get(), m_spriteram, 0x1000);
	}
}

/*****************************************************************************/

UINT32 m107_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_scroll_positions();
	screenrefresh(screen, bitmap, cliprect);
	return 0;
}
