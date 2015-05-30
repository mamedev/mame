// license:???
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/bigevglf.h"


WRITE8_MEMBER(bigevglf_state::bigevglf_palette_w)
{
	int color;

	m_paletteram[offset] = data;
	color = m_paletteram[offset & 0x3ff] | (m_paletteram[0x400 + (offset & 0x3ff)] << 8);
	m_palette->set_pen_color(offset & 0x3ff, pal4bit(color >> 4), pal4bit(color >> 0), pal4bit(color >> 8));
}

WRITE8_MEMBER(bigevglf_state::bigevglf_gfxcontrol_w)
{
/* bits used: 0,1,2,3
 0 and 2 select plane,
 1 and 3 select visible plane,
*/
	m_plane_selected  = ((data & 4) >> 1) | (data & 1);
	m_plane_visible = ((data & 8) >> 2) | ((data & 2) >> 1);
}

WRITE8_MEMBER(bigevglf_state::bigevglf_vidram_addr_w)
{
	m_vidram_bank = (data & 0xff) * 0x100;
}

WRITE8_MEMBER(bigevglf_state::bigevglf_vidram_w)
{
	UINT32 x, y, o;
	o = m_vidram_bank + offset;
	m_vidram[o + 0x10000 * m_plane_selected] = data;
	y = o >>8;
	x = (o & 255);
	m_tmp_bitmap[m_plane_selected].pix16(y, x) = data;
}

READ8_MEMBER(bigevglf_state::bigevglf_vidram_r)
{
	return m_vidram[0x10000 * m_plane_selected + m_vidram_bank + offset];
}

void bigevglf_state::video_start()
{
	m_screen->register_screen_bitmap(m_tmp_bitmap[0]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[1]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[2]);
	m_screen->register_screen_bitmap(m_tmp_bitmap[3]);
	save_item(NAME(m_tmp_bitmap[0]));
	save_item(NAME(m_tmp_bitmap[1]));
	save_item(NAME(m_tmp_bitmap[2]));
	save_item(NAME(m_tmp_bitmap[3]));

	m_vidram = auto_alloc_array(machine(), UINT8, 0x100 * 0x100 * 4);

	save_pointer(NAME(m_vidram), 0x100 * 0x100 * 4);
}

void bigevglf_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i, j;
	for (i = 0xc0-4; i >= 0; i-= 4)
	{
		int code, sx, sy;
		code = m_spriteram2[i + 1];
		sx = m_spriteram2[i + 3];
		sy = 200 - m_spriteram2[i];
		for (j = 0; j < 16; j++)
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				m_spriteram1[(code << 4) + j] + ((m_spriteram1[0x400 + (code << 4) + j] & 0xf) << 8),
				m_spriteram2[i + 2] & 0xf,
				0,0,
				sx + ((j & 1) << 3), sy + ((j >> 1) << 3), 0);
	}
}

UINT32 bigevglf_state::screen_update_bigevglf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmp_bitmap[m_plane_visible], 0, 0, 0, 0, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
