#include "emu.h"
#include "includes/galspnbl.h"


void galspnbl_state::palette_init()
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine(), i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}



/* sprite format (see also Ninja Gaiden):
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | ---------------x | flip x
 *         | --------------x- | flip y
 *         | -------------x-- | enable
 *         | ----------xx---- | priority?
 *         | ---------x------ | flicker?
 *    1    | xxxxxxxxxxxxxxxx | code
 *    2    | --------xxxx---- | color
 *         | --------------xx | size: 8x8, 16x16, 32x32, 64x64
 *    3    | xxxxxxxxxxxxxxxx | y position
 *    4    | xxxxxxxxxxxxxxxx | x position
 *    5,6,7|                  | unused
 */
static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	galspnbl_state *state = machine.driver_data<galspnbl_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = (state->m_spriteram.bytes() - 16) / 2; offs >= 0; offs -= 8)
	{
		int sx, sy, code, color, size, attr, flipx, flipy;
		int col, row;

		attr = spriteram[offs];
		if ((attr & 0x0004) && ((attr & 0x0040) == 0 || (machine.primary_screen->frame_number() & 1))
//              && ((attr & 0x0030) >> 4) == priority)
				&& ((attr & 0x0020) >> 5) == priority)
		{
			code = spriteram[offs + 1];
			color = spriteram[offs + 2];
			size = 1 << (color & 0x0003); // 1,2,4,8
			color = (color & 0x00f0) >> 4;
//          sx = spriteram[offs + 4] + screenscroll;
			sx = spriteram[offs + 4];
			sy = spriteram[offs + 3];
			flipx = attr & 0x0001;
			flipy = attr & 0x0002;

			for (row = 0; row < size; row++)
			{
				for (col = 0; col < size; col++)
				{
					int x = sx + 8 * (flipx ? (size - 1 - col) : col);
					int y = sy + 8 * (flipy ? (size - 1 - row) : row);
					drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
						code + layout[row][col],
						color,
						flipx,flipy,
						x,y,0);
				}
			}
		}
	}
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

	draw_sprites(machine(), bitmap, cliprect, 0);

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

	draw_sprites(machine(), bitmap, cliprect, 1);
	return 0;
}
