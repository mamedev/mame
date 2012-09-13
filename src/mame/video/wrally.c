/***************************************************************************

  World Rally Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/wrally.h"


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | -------- ---xxxxx | color
      1  | -------- --x----- | priority
      1  | -------- -x------ | flip y
      1  | -------- x------- | flip x
      1  | ---xxxxx -------- | data used to handle collisions, speed, etc
      1  | xxx----- -------- | not used?
*/

TILE_GET_INFO_MEMBER(wrally_state::get_tile_info_wrally_screen0)
{
	int data = m_videoram[tile_index << 1];
	int data2 = m_videoram[(tile_index << 1) + 1];
	int code = data & 0x3fff;

	tileinfo.category = (data2 >> 5) & 0x01;

	SET_TILE_INFO_MEMBER(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

TILE_GET_INFO_MEMBER(wrally_state::get_tile_info_wrally_screen1)
{
	int data = m_videoram[(0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(0x2000/2) + (tile_index << 1) + 1];
	int code = data & 0x3fff;

	tileinfo.category = (data2 >> 5) & 0x01;

	SET_TILE_INFO_MEMBER(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void wrally_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(wrally_state::get_tile_info_wrally_screen0),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_pant[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(wrally_state::get_tile_info_wrally_screen1),this),TILEMAP_SCAN_ROWS,16,16,64,32);

	m_pant[0]->set_transmask(0,0xff01,0x00ff); /* this layer is split in two (pens 1..7, pens 8-15) */
	m_pant[1]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | unknown
      2  | ------xx xxxxxxxx | x position
      2  | --xxxx-- -------- | sprite color (low 4 bits)
      2  | -x------ -------- | shadows/highlights (see below)
      2  | x------- -------- | not used?
      3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?

    For shadows/highlights, the tile color below the sprite will be set using a
    palette (from the 8 available) based on the gfx pen of the sprite. Only pens
    in the range 0x8-0xf are used.
*/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	wrally_state *state = machine.driver_data<wrally_state>();
	int i, px, py;
	gfx_element *gfx = machine.gfx[0];

	for (i = 6/2; i < (0x1000 - 6)/2; i += 4) {
		int sx = state->m_spriteram[i+2] & 0x03ff;
		int sy = (240 - (state->m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = state->m_spriteram[i+3] & 0x3fff;
		int color = (state->m_spriteram[i+2] & 0x7c00) >> 10;
		int attr = (state->m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int color_effect = (color & 0x10) >> 4;
		int high_priority = number >= 0x3700;
		color = color & 0x0f;

		if (high_priority != priority) continue;

		if (state->flip_screen()) {
			sy = sy + 248;
		}

		if (!color_effect) {
			drawgfx_transpen(bitmap,cliprect,gfx,number,
					0x20 + color,xflip,yflip,
					sx - 0x0f,sy,0);
		} else {
			/* get a pointer to the current sprite's gfx data */
			const UINT8 *gfx_src = gfx->get_data(number % gfx->elements());

			for (py = 0; py < gfx->height(); py++){
				/* get a pointer to the current line in the screen bitmap */
				int ypos = ((sy + py) & 0x1ff);
				UINT16 *srcy = &bitmap.pix16(ypos);

				int gfx_py = yflip ? (gfx->height() - 1 - py) : py;

				if ((ypos < cliprect.min_y) || (ypos > cliprect.max_y)) continue;

				for (px = 0; px < gfx->width(); px++){
					/* get current pixel */
					int xpos = (((sx + px) & 0x3ff) - 0x0f) & 0x3ff;
					UINT16 *pixel = srcy + xpos;
					int src_color = *pixel;

					int gfx_px = xflip ? (gfx->width() - 1 - px) : px;

					/* get asociated pen for the current sprite pixel */
					int gfx_pen = gfx_src[gfx->rowbytes()*gfx_py + gfx_px];

					/* pens 8..15 are used to select a palette */
					if ((gfx_pen < 8) || (gfx_pen >= 16)) continue;

					if ((xpos < cliprect.min_x) || (xpos > cliprect.max_x)) continue;

					/* modify the color of the tile */
					*pixel = src_color + (gfx_pen-8)*1024;
				}
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( wrally )
{
	wrally_state *state = screen.machine().driver_data<wrally_state>();
	/* set scroll registers */
	if (!state->flip_screen()) {
		state->m_pant[0]->set_scrolly(0, state->m_vregs[0]);
		state->m_pant[0]->set_scrollx(0, state->m_vregs[1]+4);
		state->m_pant[1]->set_scrolly(0, state->m_vregs[2]);
		state->m_pant[1]->set_scrollx(0, state->m_vregs[3]);
	} else {
		state->m_pant[0]->set_scrolly(0, 248 - state->m_vregs[0]);
		state->m_pant[0]->set_scrollx(0, 1024 - state->m_vregs[1] - 4);
		state->m_pant[1]->set_scrolly(0, 248 - state->m_vregs[2]);
		state->m_pant[1]->set_scrollx(0, 1024 - state->m_vregs[3]);
	}

	/* draw tilemaps + sprites */
	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER0,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER1,0);

	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1),0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER0,0);

	draw_sprites(screen.machine(),bitmap,cliprect,0);

	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER1,0);

	draw_sprites(screen.machine(),bitmap,cliprect,1);

	return 0;
}
