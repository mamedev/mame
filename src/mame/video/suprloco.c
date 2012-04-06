/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/suprloco.h"



#define SPR_Y_TOP		0
#define SPR_Y_BOTTOM	1
#define SPR_X			2
#define SPR_COL			3
#define SPR_SKIP_LO		4
#define SPR_SKIP_HI		5
#define SPR_GFXOFS_LO	6
#define SPR_GFXOFS_HI	7


/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm not sure about the resistor values, I'm using the Galaxian ones.

***************************************************************************/
PALETTE_INIT( suprloco )
{
	int i;


	for (i = 0;i < 512;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		/* hack: generate a second bank of sprite palette with red changed to purple */
		if (i >= 256)
		{
			if ((i & 0x0f) == 0x09)
				palette_set_color(machine,i+256,MAKE_RGB(r,g,0xff));
			else
				palette_set_color(machine,i+256,MAKE_RGB(r,g,b));
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	suprloco_state *state = machine.driver_data<suprloco_state>();
	UINT8 attr = state->m_videoram[2*tile_index+1];
	SET_TILE_INFO(
			0,
			state->m_videoram[2*tile_index] | ((attr & 0x03) << 8),
			(attr & 0x1c) >> 2,
			0);
	tileinfo.category = (attr & 0x20) >> 5;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( suprloco )
{
	suprloco_state *state = machine.driver_data<suprloco_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32,32);

	state->m_bg_tilemap->set_scroll_rows(32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(suprloco_state::suprloco_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(suprloco_state::suprloco_scrollram_w)
{
	int adj = flip_screen_get(machine()) ? -8 : 8;

	m_scrollram[offset] = data;
	m_bg_tilemap->set_scrollx(offset, data - adj);
}

WRITE8_MEMBER(suprloco_state::suprloco_control_w)
{
	/* There is probably a palette select in here */

	/* Bit 0   - coin counter A */
	/* Bit 1   - coin counter B (only used if coinage differs from A) */
	/* Bit 2-3 - probably unused */
	/* Bit 4   - ??? */
	/* Bit 5   - pulsated when loco turns "super" */
	/* Bit 6   - probably unused */
	/* Bit 7   - flip screen */

	if ((m_control & 0x10) != (data & 0x10))
	{
		/*logerror("Bit 4 = %d\n", (data >> 4) & 1); */
	}

	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);

	flip_screen_set(machine(), data & 0x80);

	m_control = data;
}


READ8_MEMBER(suprloco_state::suprloco_control_r)
{
	return m_control;
}



INLINE void draw_pixel(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int color,int flip)
{
	if (flip)
	{
		x = bitmap.width() - x - 1;
		y = bitmap.height() - y - 1;
	}

	if (cliprect.contains(x, y))
		bitmap.pix16(y, x) = color;
}


static void draw_sprite(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int spr_number)
{
	suprloco_state *state = machine.driver_data<suprloco_state>();
	int flip = flip_screen_get(machine);
	int sx,sy,col,row,height,src,adjy,dy;
	UINT8 *spr_reg;
	UINT8 *gfx2;
	pen_t pen_base;
	short skip;	/* bytes to skip before drawing each row (can be negative) */


	spr_reg	= state->m_spriteram + 0x10 * spr_number;

	src = spr_reg[SPR_GFXOFS_LO] + (spr_reg[SPR_GFXOFS_HI] << 8);
	skip = spr_reg[SPR_SKIP_LO] + (spr_reg[SPR_SKIP_HI] << 8);

	height		= spr_reg[SPR_Y_BOTTOM] - spr_reg[SPR_Y_TOP];
	pen_base = 0x100 + 0x10 * (spr_reg[SPR_COL]&0x03) + ((state->m_control & 0x20)?0x100:0);
	sx = spr_reg[SPR_X];
	sy = spr_reg[SPR_Y_TOP] + 1;

	if (!flip_screen_get(machine))
	{
		adjy = sy;
		dy = 1;
	}
	else
	{
		adjy = sy + height - 1;  /* some of the sprites are still off by a pixel */
		dy = -1;
	}

	gfx2 = machine.region("gfx2")->base();
	for (row = 0;row < height;row++,adjy+=dy)
	{
		int color1,color2,flipx;
		UINT8 data;
		UINT8 *gfx;

		src += skip;

		col = 0;

		/* get pointer to packed sprite data */
		gfx = &(gfx2[src & 0x7fff]);
		flipx = src & 0x8000;   /* flip x */

		while (1)
		{
			if (flipx)	/* flip x */
			{
				data = *gfx--;
				color1 = data & 0x0f;
				color2 = data >> 4;
			}
			else
			{
				data = *gfx++;
				color1 = data >> 4;
				color2 = data & 0x0f;
			}

			if (color1 == 15) break;
			if (color1)
				draw_pixel(bitmap,cliprect,sx+col,  adjy,pen_base + color1, flip);

			if (color2 == 15) break;
			if (color2)
				draw_pixel(bitmap,cliprect,sx+col+1,adjy,pen_base + color2, flip);

			col += 2;
		}
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	suprloco_state *state = machine.driver_data<suprloco_state>();
	int spr_number;
	UINT8 *spr_reg;


	for (spr_number = 0;spr_number < (state->m_spriteram_size >> 4);spr_number++)
	{
		spr_reg = state->m_spriteram + 0x10 * spr_number;
		if (spr_reg[SPR_X] != 0xff)
			draw_sprite(machine, bitmap, cliprect, spr_number);
	}
}

SCREEN_UPDATE_IND16( suprloco )
{
	suprloco_state *state = screen.machine().driver_data<suprloco_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1,0);
	return 0;
}
