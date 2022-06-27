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
#include "taitoair.h"


/***************************************************************************
  Screen refresh
***************************************************************************/

int taitoair_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return draw_sprites(bitmap,cliprect, 0x3f8 / 2);
}

/*!
 @param start_offset DMA sprite offset source
 @return value acquired by a pause flag acquisition.
 */
int taitoair_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset)
{
	//const u16 stop_values[4] = { 0xc00, 0, 0, 0 };

	for (int offs = start_offset; offs >= 0; offs -= 0x008 / 2)
	{
		/*!
		 Starting at a particular sequence, sprite DMA seems to stop there and resume via "something",
		 effectively drawing any other sprite with better priority in the framebuffer scheme of things.

		 @todo reported sequence for DMA pause flag is 0x0c** 0x0000 0x0000 0x0000.
		       Verify how exactly via HW test. Continuing may be determined by a DMA bit write.
		 */
		if (m_tc0080vco->sprram_r(offs + 0) == 0xc00 ||
			m_tc0080vco->sprram_r(offs + 0) == 0xcff) // Air Inferno
			return offs - 8/2;

		m_tc0080vco->get_sprite_params(offs, true);

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			m_tc0080vco->draw_single_sprite(bitmap, cliprect);
		}
	}

	return 0;
}

void taitoair_state::fill_slope(bitmap_ind16 &bitmap, const rectangle &cliprect, u16 header, s32 x1, s32 x2, s32 sl1, s32 sl2, s32 y1, s32 y2, s32 *nx1, s32 *nx2)
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
		s32 t, *tp;
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

				if (header & 0x4000 && machine().input().code_pressed(KEYCODE_Q))
				{
					base_color = machine().rand() & 0x3fff;
					grad_col = 0;
				}
				else if (m_paletteram[(header & 0xff)+0x300] & 0x8000)
				{
					/* Terrain elements, with a gradient applied. */
					/*! @todo it's unknown if gradient color applies by global screen Y coordinate or there's a calculation to somewhere ... */
					base_color = ((header & 0x3f) * 0x80) + 0x2040;
					if (header & 0x3fe0)
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
					bitmap.pix(y1, xx1) = base_color + grad_col;
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

void taitoair_state::fill_poly(bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q)
{
	s32 sl1, sl2, x1, x2;
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT * 2];
	u16 header = q->header;
	int pcount = q->pcount;

	for (int i = 0; i < pcount; i++)
	{
		p[i].x = p[i + pcount].x = q->p[i].x << TAITOAIR_FRAC_SHIFT;
		p[i].y = p[i + pcount].y = q->p[i].y;
	}

	int pmin = 0, pmax = 0;
	for (int i = 1; i < pcount; i++)
	{
		if (p[i].y < p[pmin].y)
			pmin = i;
		if (p[i].y > p[pmax].y)
			pmax = i;
	}

	s32 cury = p[pmin].y;
	s32 limy = p[pmax].y;

	if (cury == limy)
		return;

	if (cury > cliprect.max_y)
		return;
	if (limy <= cliprect.min_y)
		return;

	if (limy > cliprect.max_y)
		limy = cliprect.max_y;

	int ps1 = pmin + pcount;
	int ps2 = pmin;

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
void taitoair_state::dsp_flags_w(offs_t offset, u16 data)
{
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	/* clear and copy operation if offset is 0x3001 */
	if (offset == 1)
	{
		fb_copy_op();
	}

	/* if offset 0x3001 OR 0x3002 we put data in the buffer fb */
	if (offset)
	{
		fb_fill_op();
	}
}

void taitoair_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();
	m_framebuffer[0] = std::make_unique<bitmap_ind16>(width, height);
	m_framebuffer[1] = std::make_unique<bitmap_ind16>(width, height);
	//m_buffer3d = std::make_unique<bitmap_ind16>(width, height);
}

u32 taitoair_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	// TODO: Convert TC0430GRW to device (tc0280grd.cpp)
	u32 counter1 = (m_tc0430grw[0] << 16) | m_tc0430grw[1];
	u32 inc1x    = s16(m_tc0430grw[2]);
	u32 inc1y    = s16(m_tc0430grw[3]);
	u32 counter2 = (m_tc0430grw[4] << 16) | m_tc0430grw[5];
	u32 inc2x    = s16(m_tc0430grw[6]);
	u32 inc2y    = s16(m_tc0430grw[7]);

	// Deltas are 118/31
	int dx = cliprect.min_x      + 118;
	int dy = cliprect.min_y - 48 + 31;

	counter1 += dx * inc1x + dy * inc1y;
	counter2 += dx * inc2x + dy * inc2y;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u32 c1b = counter1;
		u32 c2b = counter2;
		u16 *dest = &bitmap.pix(y, cliprect.min_x);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 base = 0;
			u32 cntr = 0;
			if (c2b & 0x800000)
			{
				base = 0x2040;
				cntr = c2b;
			}
			else if (c1b & 0x800000)
			{
				base = 0x2000;
				cntr = c1b;
			}
			if (m_gradbank == true)
				base |= 0x1000;

			*dest++ = base | (cntr >= 0x83f000 ? 0x3f : (cntr >> 12) & 0x3f);

			c1b += inc1x;
			c2b += inc2x;
		}
		counter1 += inc1y;
		counter2 += inc2y;
	}

	copybitmap_trans(bitmap, *m_framebuffer[1], 0, 0, 0, 0, cliprect, 0);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	int sprite_ptr = draw_sprites(bitmap, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	draw_sprites(bitmap, cliprect, sprite_ptr);

	/* Hacky 3d bitmap */
	//copybitmap_trans(bitmap, m_buffer3d, 0, 0, 0, 0, cliprect, 0);
	//m_buffer3d->fill(0, cliprect);

	return 0;
}
