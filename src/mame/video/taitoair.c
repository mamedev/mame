// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/***************************************************************************

Functions to emulate the video hardware of the machine.

TODO
====

Harmonise draw_sprites code and offload it into taitoic.c, leaving this
as short as possible (TC0080VCO has sprites as well as tilemaps in its
address space).

Maybe wait until the Taito Air system is done - also uses TC0080VCO.

Why does syvalion draw_sprites ignore the zoomy value
(using zoomx instead) ???


Sprite ram notes
----------------

BG / chain RAM
-----------------------------------------
[0]  +0         +1
     -xxx xxxx  xxxx xxxx = tile number

[1]  +0         +1
     ---- ----  x--- ---- = flip Y
     ---- ----  -x-- ---- = flip X
     ---- ----  --xx xxxx = color


sprite RAM
--------------------------------------------------------------
 +0         +1         +2         +3
 ---x ----  ---- ----  ---- ----  ---- ---- = unknown
 ---- xx--  ---- ----  ---- ----  ---- ---- = chain y size
 ---- --xx  xxxx xxxx  ---- ----  ---- ---- = sprite x coords
 ---- ----  ---- ----  ---- --xx  xxxx xxxx = sprite y coords

 +4         +5         +6         +7
 --xx xxxx  ---- ----  ---- ----  ---- ---- = zoom x
 ---- ----  --xx xxxx  ---- ----  ---- ---- = zoom y
 ---- ----  ---- ----  ---x xxxx  xxxx xxxx = tile information offset


***************************************************************************/

#include "emu.h"
#include "includes/taitoair.h"


/* These are hand-tuned values */
static const int zoomy_conv_table[] =
{
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,   /* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,   /* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};

/***************************************************************************
  Screen refresh
***************************************************************************/

int taitoair_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return draw_sprites(bitmap,cliprect, 0x3f8 / 2);
}

/*!
 @param start_offset DMA sprite offset source
 @return value acquired by a pause flag acquisition.
 */
int taitoair_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset )
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
	   is always 64 pixels. */
	//const UINT16 stop_values[4] = { 0xc00, 0, 0, 0 };
	address_space &space = machine().driver_data()->generic_space();
	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, dy, ex, ey, zx, zy;
	int ysize;
	int j, k;
	int offs;                   /* sprite RAM offset */
	int tile_offs;              /* sprite chain offset */
	int zoomx, zoomy;           /* zoom value */

	for (offs = start_offset; offs >= 0; offs -= 0x008 / 2)
	{
		/*!
		 Starting at a particular sequence, sprite DMA seems to stop there and resume via "something",
		 effectively drawing any other sprite with better priority in the framebuffer scheme of things.

		 @todo reported sequence for DMA pause flag is 0x0c** 0x0000 0x0000 0x0000.
		       Verify how exactly via HW test. Continuing may be determined by a DMA bit write.
		 */
		if(m_tc0080vco->sprram_r(space, offs + 0, 0xffff) == 0xc00 ||
			m_tc0080vco->sprram_r(space, offs + 0, 0xffff) == 0xcff) // Air Inferno
			return offs - 8/2;

		x0        =  m_tc0080vco->sprram_r(space, offs + 1, 0xffff) & 0x3ff;
		y0        =  m_tc0080vco->sprram_r(space, offs + 0, 0xffff) & 0x3ff;
		zoomx     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x7f00) >> 8;
		zoomy     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x007f);
		tile_offs = (m_tc0080vco->sprram_r(space, offs + 3, 0xffff) & 0x1fff) << 2;
		ysize     = size[(m_tc0080vco->sprram_r(space, offs, 0xffff) & 0x0c00) >> 10];


		if (tile_offs)
		{
			/* Convert zoomy value to real value as zoomx */
			zoomy = zoomy_conv_table[zoomy];

			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (zoomy < 63)
			{
				dy = 8 + (zoomy + 2) / 8;
				ey = (zoomy + 2) % 8;
				zy = ((dy << 1) + ey) << 11;
			}
			else
			{
				dy = 16 + (zoomy - 63) / 4;
				ey = (zoomy - 63) % 4;
				zy = (dy + ey) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (m_tc0080vco->flipscreen_r())
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
				dy = -dy;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for (j = 0; j < ysize; j ++)
			{
				x = x0;
				for (k = 0; k < 4; k ++)
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = m_tc0080vco->cram_0_r(space, tile_offs, 0xffff) & 0x7fff;
						color = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x001f;
						flipx = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0040;
						flipy = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0080;

						if (m_tc0080vco->flipscreen_r())
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}


									m_gfxdecode->gfx(0)->zoom_transpen(bitmap,cliprect,
									tile,
									color,
									flipx, flipy,
									x, y,
									zx, zy, 0
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dy;
			}
		}
	}

	return 0;
}

void taitoair_state::fill_slope( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 header, INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 y1, INT32 y2, INT32 *nx1, INT32 *nx2 )
{
	if (y1 > cliprect.max_y)
		return;

	if (y2 <= cliprect.min_y)
	{
		int delta = y2 - y1;
		*nx1 = x1 + delta * sl1;
		*nx2 = x2 + delta * sl2;
		return;
	}

	if (y1 < -1000000 || y1 > 1000000)
		return;

	if (y2 > cliprect.max_y)
		y2 = cliprect.max_y + 1;

	if (y1 < cliprect.min_y)
	{
		int delta = cliprect.min_y - y1;
		x1 += delta * sl1;
		x2 += delta * sl2;
		y1 = cliprect.min_y;
	}

	if (x1 > x2 || (x1==x2 && sl1 > sl2))
	{
		INT32 t, *tp;
		t = x1;
		x1 = x2;
		x2 = t;
		t = sl1;
		sl1 = sl2;
		sl2 = t;
		tp = nx1;
		nx1 = nx2;
		nx2 = tp;
	}

	while (y1 < y2)
	{
		if (y1 >= cliprect.min_y)
		{
			int xx1 = x1 >> TAITOAIR_FRAC_SHIFT;
			int xx2 = x2 >> TAITOAIR_FRAC_SHIFT;
			int grad_col;
			int base_color;

			if (xx1 <= cliprect.max_x || xx2 >= cliprect.min_x)
			{
				if (xx1 < cliprect.min_x)
					xx1 = cliprect.min_x;
				if (xx2 > cliprect.max_x)
					xx2 = cliprect.max_x;

				if(header & 0x4000 && machine().input().code_pressed(KEYCODE_Q))
				{
					base_color = machine().rand() & 0x3fff;
					grad_col = 0;
				}
				else if(m_paletteram[(header & 0xff)+0x300] & 0x8000)
				{
					/* Terrain elements, with a gradient applied. */
					/*! @todo it's unknown if gradient color applies by global screen Y coordinate or there's a calculation to somewhere ... */
					base_color = ((header & 0x3f) * 0x80) + 0x2040;
					if(header & 0x3fe0)
						base_color = machine().rand() & 0x3fff;
					grad_col = (y1 >> 3) & 0x3f;
				}
				else
				{
					/* Non-terrain elements are colored with this. */
					base_color = (header & 0xff) + 0x300;
					grad_col = 0;
				}

				while (xx1 <= xx2)
				{
					bitmap.pix16(y1, xx1) = base_color + grad_col;
					xx1++;
				}
			}
		}

		x1 += sl1;
		x2 += sl2;
		y1++;
	}
	*nx1 = x1;
	*nx2 = x2;
}

void taitoair_state::fill_poly( bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q )
{
	INT32 sl1, sl2, cury, limy, x1, x2;
	int pmin, pmax, i, ps1, ps2;
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT * 2];
	UINT16 header = q->header;
	int pcount = q->pcount;

	for (i = 0; i < pcount; i++)
	{
		p[i].x = p[i + pcount].x = q->p[i].x << TAITOAIR_FRAC_SHIFT;
		p[i].y = p[i + pcount].y = q->p[i].y;
	}

	pmin = pmax = 0;
	for (i = 1; i < pcount; i++)
	{
		if (p[i].y < p[pmin].y)
			pmin = i;
		if (p[i].y > p[pmax].y)
			pmax = i;
	}

	cury = p[pmin].y;
	limy = p[pmax].y;

	if (cury == limy)
		return;

	if (cury > cliprect.max_y)
		return;
	if (limy <= cliprect.min_y)
		return;

	if (limy > cliprect.max_y)
		limy = cliprect.max_y;

	ps1 = pmin + pcount;
	ps2 = pmin;

	goto startup;

	for (;;)
	{
		if (p[ps1 - 1].y == p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, header, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			ps2++;

		startup:
			while (p[ps1 - 1].y == cury)
				ps1--;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x1 = p[ps1].x;
			x2 = p[ps2].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
		else if (p[ps1 - 1].y < p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, header, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			while (p[ps1 - 1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
		}
		else
		{
			fill_slope(bitmap, cliprect, header, x1, x2, sl1, sl2, cury, p[ps2 + 1].y, &x1, &x2);
			cury = p[ps2 + 1].y;
			if (cury >= limy)
				break;
			ps2++;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
	}
}

/***************************************************************************
  dsp handlers
***************************************************************************/

void taitoair_state::fb_copy_op()
{
	/*! @todo declare once */
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	/* clear screen fb */
	m_framebuffer[1]->fill(0, cliprect);
	/* copy buffer fb into screen fb (at this stage we are ready to draw) */
	copybitmap_trans(*m_framebuffer[1], *m_framebuffer[0], 0, 0, 0, 0, cliprect, 0);
	/* now clear buffer fb */
	m_framebuffer[0]->fill(0, cliprect);
}

void taitoair_state::fb_erase_op()
{
	/*! @todo declare once */
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	m_framebuffer[0]->fill(0, cliprect);
	//m_framebuffer[1]->fill(0, cliprect);
}

void taitoair_state::fb_fill_op()
{
	/*! @todo declare once */
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	if (m_line_ram[0x3fff])
	{
		int adr = 0x3fff;

		while (adr >= 0 && m_line_ram[adr] && m_line_ram[adr] != 0x4000)
		{
			int pcount = 0;
			m_q.header = m_line_ram[adr--];
			while (pcount < TAITOAIR_POLY_MAX_PT && adr >= 1 && !(m_line_ram[adr] & 0xc000))
			{
				m_q.p[pcount].y = m_line_ram[adr--] + 3 * 16;
				m_q.p[pcount].x = m_line_ram[adr--];
				pcount++;
			}
			adr--;
			m_q.pcount = pcount;
			if (!(m_line_ram[adr] & 0x8000))
			{
				m_q.header |= 0x4000;
				logerror("special poly at %04x\n", adr);
				while(adr >= 0 && !(m_line_ram[adr] & 0xc000))
					adr--;
			}
			fill_poly(*m_framebuffer[0], cliprect, &m_q);
		}
	}
}

/*!
    @todo still don't know how this works. It calls three values (0x1fff-0x5fff-0xdfff), for two or three offsets.
    In theory this should fit into framebuffer draw, display, clear and swap in some way.
*/
WRITE16_MEMBER(taitoair_state::dsp_flags_w)
{
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	{
		/* clear and copy operation if offset is 0x3001 */
		if(offset == 1)
		{
			fb_copy_op();
		}

		/* if offset 0x3001 OR 0x3002 we put data in the buffer fb */
		if(offset)
		{
			fb_fill_op();
		}
	}
}

void taitoair_state::video_start()
{
	int width, height;

	width = m_screen->width();
	height = m_screen->height();
	m_framebuffer[0] = auto_bitmap_ind16_alloc(machine(), width, height);
	m_framebuffer[1] = auto_bitmap_ind16_alloc(machine(), width, height);
	//m_buffer3d = auto_bitmap_ind16_alloc(machine(), width, height);
}

UINT32 taitoair_state::screen_update_taitoair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int sprite_ptr;
	m_tc0080vco->tilemap_update();

	UINT32 counter1 = (m_tc0430grw[0] << 16) | m_tc0430grw[1];
	UINT32 inc1x    = INT16(m_tc0430grw[2]);
	UINT32 inc1y    = INT16(m_tc0430grw[3]);
	UINT32 counter2 = (m_tc0430grw[4] << 16) | m_tc0430grw[5];
	UINT32 inc2x    = INT16(m_tc0430grw[6]);
	UINT32 inc2y    = INT16(m_tc0430grw[7]);

	// Deltas are 118/31
	int dx = cliprect.min_x      + 118;
	int dy = cliprect.min_y - 48 + 31;

	counter1 += dx*inc1x + dy*inc1y;
	counter2 += dx*inc2x + dy*inc2y;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		UINT32 c1b = counter1;
		UINT32 c2b = counter2;
		UINT16 *dest = &bitmap.pix(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			UINT16 base = 0;
			UINT32 cntr = 0;
			if(c2b & 0x800000) {
				base = 0x2040;
				cntr = c2b;
			} else if(c1b & 0x800000) {
				base = 0x2000;
				cntr = c1b;
			}
			if(m_gradbank == true)
				base|= 0x1000;

			*dest++ = base | (cntr >= 0x83f000 ? 0x3f : (cntr >> 12) & 0x3f);

			c1b += inc1x;
			c2b += inc2x;
		}
		counter1 += inc1y;
		counter2 += inc2y;
	}



	copybitmap_trans(bitmap, *m_framebuffer[1], 0, 0, 0, 0, cliprect, 0);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	sprite_ptr = draw_sprites(bitmap, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	draw_sprites(bitmap, cliprect, sprite_ptr);

	/* Hacky 3d bitmap */
	//copybitmap_trans(bitmap, m_buffer3d, 0, 0, 0, 0, cliprect, 0);
	//m_buffer3d->fill(0, cliprect);

	return 0;
}
