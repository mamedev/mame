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
	palette_set_color_rgb(machine(), offset & 0x3ff, pal4bit(color >> 4), pal4bit(color >> 0), pal4bit(color >> 8));
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

VIDEO_START( bigevglf )
{
	bigevglf_state *state = machine.driver_data<bigevglf_state>();

	machine.primary_screen->register_screen_bitmap(state->m_tmp_bitmap[0]);
	machine.primary_screen->register_screen_bitmap(state->m_tmp_bitmap[1]);
	machine.primary_screen->register_screen_bitmap(state->m_tmp_bitmap[2]);
	machine.primary_screen->register_screen_bitmap(state->m_tmp_bitmap[3]);
	state->save_item(NAME(state->m_tmp_bitmap[0]));
	state->save_item(NAME(state->m_tmp_bitmap[1]));
	state->save_item(NAME(state->m_tmp_bitmap[2]));
	state->save_item(NAME(state->m_tmp_bitmap[3]));

	state->m_vidram = auto_alloc_array(machine, UINT8, 0x100 * 0x100 * 4);

	state->save_pointer(NAME(state->m_vidram), 0x100 * 0x100 * 4);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bigevglf_state *state = machine.driver_data<bigevglf_state>();
	int i, j;
	for (i = 0xc0-4; i >= 0; i-= 4)
	{
		int code, sx, sy;
		code = state->m_spriteram2[i + 1];
		sx = state->m_spriteram2[i + 3];
		sy = 200 - state->m_spriteram2[i];
		for (j = 0; j < 16; j++)
			drawgfx_transpen(bitmap, cliprect, machine.gfx[0],
				state->m_spriteram1[(code << 4) + j] + ((state->m_spriteram1[0x400 + (code << 4) + j] & 0xf) << 8),
				state->m_spriteram2[i + 2] & 0xf,
				0,0,
				sx + ((j & 1) << 3), sy + ((j >> 1) << 3), 0);
	}
}

SCREEN_UPDATE_IND16( bigevglf )
{
	bigevglf_state *state = screen.machine().driver_data<bigevglf_state>();

	copybitmap(bitmap, state->m_tmp_bitmap[state->m_plane_visible], 0, 0, 0, 0, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
