#include "emu.h"
#include "includes/xorworld.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Xor World has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 460 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( xorworld )
{
	int i;

	for (i = 0;i < machine.total_colors();i++){
		int bit0,bit1,bit2,bit3;
		int r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e*bit0 + 0x1e*bit1 + 0x44*bit2 + 0x8f*bit3;
		/* green component */
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		g = 0x0e*bit0 + 0x1e*bit1 + 0x44*bit2 + 0x8f*bit3;
		/* blue component */
		bit0 = (color_prom[2*machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[2*machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[2*machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[2*machine.total_colors()] >> 3) & 0x01;
		b = 0x0e*bit0 + 0x1e * bit1 + 0x44*bit2 + 0x8f*bit3;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

WRITE16_MEMBER(xorworld_state::xorworld_videoram16_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

/*
    Tile format
    -----------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | ----xxxx xxxxxxxx | code
      0  | xxxx---- -------- | color
*/

static TILE_GET_INFO( get_bg_tile_info )
{
	xorworld_state *state = machine.driver_data<xorworld_state>();
	UINT16 *videoram = state->m_videoram;
	int data = videoram[tile_index];
	int code = data & 0x0fff;

	SET_TILE_INFO(0, code, data >> 12, 0);
}

VIDEO_START( xorworld )
{
	xorworld_state *state = machine.driver_data<xorworld_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | x position
      0  | xxxxxxxx -------- | y position
      1  | -------- ------xx | flipxy? (not used)
      1  | ----xxxx xxxxxx-- | sprite number
      1  | xxxx---- -------- | sprite color
*/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	xorworld_state *state = machine.driver_data<xorworld_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int i;

	for (i = 0; i < 0x40; i += 2)
	{
		int sx = spriteram16[i] & 0x00ff;
		int sy = 240 - (((spriteram16[i] & 0xff00) >> 8) & 0xff);
		int code = (spriteram16[i+1] & 0x0ffc) >> 2;
		int color = (spriteram16[i+1] & 0xf000) >> 12;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, color, 0, 0, sx, sy, 0);
	}
}

SCREEN_UPDATE_IND16( xorworld )
{
	xorworld_state *state = screen.machine().driver_data<xorworld_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
