#include "emu.h"
#include "includes/galspnbl.h"


void galspnbl_state::palette_init()
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine(), i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}



static void draw_background( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	galspnbl_state *state = machine.driver_data<galspnbl_state>();
	offs_t offs;

//  int screenscroll = 4 - (state->m_scroll[0] & 0xff);

	for (offs = 0; offs < 0x20000; offs++)
	{
		int y = offs >> 9;
		int x = offs & 0x1ff;

		bitmap.pix16(y, x) = 1024 + (state->m_bgvideoram[offs] >> 1);
	}
}


UINT32 galspnbl_state::screen_update_galspnbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	draw_background(machine(), bitmap, cliprect);

	galspnbl_draw_sprites(machine(), bitmap, cliprect, 0,  m_spriteram, m_spriteram.bytes());

	for (offs = 0; offs < 0x1000 / 2; offs++)
	{
		int sx, sy, code, attr, color;

		code = m_videoram[offs];
		attr = m_colorram[offs];
		color = (attr & 0x00f0) >> 4;
		sx = offs % 64;
		sy = offs / 64;

		/* What is this? A priority/half transparency marker? */
		if (!(attr & 0x0008))
		{
			drawgfx_transpen(bitmap,cliprect,machine().gfx[0],
					code,
					color,
					0,0,
//                  16*sx + screenscroll,8*sy,
					16*sx,8*sy,0);
		}
	}

	galspnbl_draw_sprites(machine(), bitmap, cliprect, 1, m_spriteram, m_spriteram.bytes());
	return 0;
}
