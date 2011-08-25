/*
*   Video Driver for Forty-Love
*/

#include "emu.h"
#include "includes/40love.h"


/*
*   color prom decoding
*/

PALETTE_INIT( fortyl )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[2*machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[2*machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[2*machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[2*machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));

		color_prom++;
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
colorram format (2 bytes per one tilemap character line, 8 pixels height):

    offset 0    x... ....   x scroll (1 MSB bit)
    offset 0    .xxx x...   tile bank (see code below for banking formula)
    offset 0    .... .xxx   tiles color (one color code per whole tilemap line)

    offset 1    xxxx xxxx   x scroll (8 LSB bits)
*/

static TILE_GET_INFO( get_bg_tile_info )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	int tile_number = state->m_videoram[tile_index];
	int tile_attrib = state->m_colorram[(tile_index / 64) * 2];
	int tile_h_bank = (tile_attrib & 0x40) << 3;	/* 0x40->0x200 */
	int tile_l_bank = (tile_attrib & 0x18) << 3;	/* 0x10->0x80, 0x08->0x40 */

	int code = tile_number;
	if ((tile_attrib & 0x20) && (code >= 0xc0))
		code = (code & 0x3f) | tile_l_bank | 0x100;
	code |= tile_h_bank;

	SET_TILE_INFO(	0,
			code,
			tile_attrib & 0x07,
			0);
}

/***************************************************************************

  State-related callbacks

***************************************************************************/

static void redraw_pixels(running_machine &machine)
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	state->m_pix_redraw = 1;
	tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( fortyl )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	state->m_pixram1 = auto_alloc_array_clear(machine, UINT8, 0x4000);
	state->m_pixram2 = auto_alloc_array_clear(machine, UINT8, 0x4000);

	state->m_tmp_bitmap1 = auto_bitmap_alloc(machine, 256, 256, machine.primary_screen->format());
	state->m_tmp_bitmap2 = auto_bitmap_alloc(machine, 256, 256, machine.primary_screen->format());

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_xoffset = 128;	// this never changes

	tilemap_set_scroll_rows(state->m_bg_tilemap, 32);
	tilemap_set_transparent_pen(state->m_bg_tilemap, 0);

	state->save_item(NAME(state->m_flipscreen));
	state->save_item(NAME(state->m_pix_color));
	state->save_pointer(NAME(state->m_pixram1), 0x4000);
	state->save_pointer(NAME(state->m_pixram2), 0x4000);
	state->save_item(NAME(*state->m_tmp_bitmap1));
	state->save_item(NAME(*state->m_tmp_bitmap2));
	state->save_item(NAME(state->m_pixram_sel));
	machine.save().register_postload(save_prepost_delegate(FUNC(redraw_pixels), &machine));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

static void fortyl_set_scroll_x( running_machine &machine, int offset )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	int i = offset & ~1;
	int x = ((state->m_colorram[i] & 0x80) << 1) | state->m_colorram[i + 1];	/* 9 bits signed */

	if (state->m_flipscreen)
		x += 0x51;
	else
		x -= 0x50;

	x &= 0x1ff;
	if (x & 0x100) x -= 0x200;				/* sign extend */

	tilemap_set_scrollx(state->m_bg_tilemap, offset / 2, x);
}

WRITE8_HANDLER( fortyl_pixram_sel_w )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	int offs;
	int f = data & 0x01;

	state->m_pixram_sel = (data & 0x04) >> 2;

	if (state->m_flipscreen != f)
	{
		state->m_flipscreen = f;
		flip_screen_set(space->machine(), state->m_flipscreen);
		state->m_pix_redraw = 1;

		for (offs = 0; offs < 32; offs++)
			fortyl_set_scroll_x(space->machine(), offs * 2);
	}
}

READ8_HANDLER( fortyl_pixram_r )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	if (state->m_pixram_sel)
		return state->m_pixram2[offset];
	else
		return state->m_pixram1[offset];
}

static void fortyl_plot_pix( running_machine &machine, int offset )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	int x, y, i, c, d1, d2;

	x = (offset & 0x1f) * 8;
	y = (offset >> 5) & 0xff;

	if (state->m_pixram_sel)
	{
		d1 = state->m_pixram2[offset];
		d2 = state->m_pixram2[offset + 0x2000];
	}
	else
	{
		d1 = state->m_pixram1[offset];
		d2 = state->m_pixram1[offset + 0x2000];
	}

	for (i = 0; i < 8; i++)
	{
		c = ((d2 >> i) & 1) + ((d1 >> i) & 1) * 2;
		if (state->m_pixram_sel)
			*BITMAP_ADDR16(state->m_tmp_bitmap2, y, x + i) = state->m_pix_color[c];
		else
			*BITMAP_ADDR16(state->m_tmp_bitmap1, y, x + i) = state->m_pix_color[c];
	}
}

WRITE8_HANDLER( fortyl_pixram_w )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	if (state->m_pixram_sel)
		state->m_pixram2[offset] = data;
	else
		state->m_pixram1[offset] = data;

	fortyl_plot_pix(space->machine(), offset & 0x1fff);
}


WRITE8_HANDLER( fortyl_bg_videoram_w )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

READ8_HANDLER( fortyl_bg_videoram_r )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	return state->m_videoram[offset];
}

WRITE8_HANDLER( fortyl_bg_colorram_w )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	if (state->m_colorram[offset] != data)
	{
		int i;

		state->m_colorram[offset] = data;
		for (i = (offset / 2) * 64; i < (offset / 2) * 64 + 64; i++)
			tilemap_mark_tile_dirty(state->m_bg_tilemap, i);

		fortyl_set_scroll_x(space->machine(), offset);
	}
}

READ8_HANDLER( fortyl_bg_colorram_r )
{
	fortyl_state *state = space->machine().driver_data<fortyl_state>();
	return state->m_colorram[offset];
}

/***************************************************************************

  Display refresh

***************************************************************************/
/*
spriteram format (4 bytes per sprite):

    offset  0   xxxxxxxx    y position

    offset  1   x.......    flip Y
    offset  1   .x......    flip X
    offset  1   ..xxxxxx    gfx code (6 LSB bits)

    offset  2   ...xx...    gfx code (2 MSB bits)
    offset  2   .....xxx    color code
    offset  2   ???.....    ??? (not used, always 0)

    offset  3   xxxxxxxx    x position
*/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	UINT8 *spriteram = state->m_spriteram;
	UINT8 *spriteram_2 = state->m_spriteram2;
	int offs;

	/* spriteram #1 */
	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int code, color, sx, sy, flipx, flipy;

		sx = spriteram[offs + 3];
		sy = spriteram[offs + 0] +1;

		if (state->m_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram[offs + 1] & 0x3f) + ((spriteram[offs + 2] & 0x18) << 3);
		flipx = ((spriteram[offs + 1] & 0x40) >> 6) ^ state->m_flipscreen;
		flipy = ((spriteram[offs + 1] & 0x80) >> 7) ^ state->m_flipscreen;
		color = (spriteram[offs + 2] & 0x07) + 0x08;

		if (spriteram[offs + 2] & 0xe0)
			color = machine.rand() & 0xf;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx+state->m_xoffset,sy,0);
	}

	/* spriteram #2 */
	for (offs = 0; offs < state->m_spriteram2_size; offs += 4)
	{
		int code, color, sx, sy, flipx, flipy;

		sx = spriteram_2[offs + 3];
		sy = spriteram_2[offs + 0] +1;

		if (state->m_flipscreen)
			sx = 240 - sx;
		else
			sy = 242 - sy;

		code = (spriteram_2[offs + 1] & 0x3f) + ((spriteram_2[offs + 2] & 0x18) << 3);
		flipx = ((spriteram_2[offs + 1] & 0x40) >> 6) ^ state->m_flipscreen;
		flipy = ((spriteram_2[offs + 1] & 0x80) >> 7) ^ state->m_flipscreen;
		color = (spriteram_2[offs + 2] & 0x07) + 0x08;

		if (spriteram_2[offs + 2] & 0xe0)
			color = machine.rand() & 0xf;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx+state->m_xoffset,sy,0);
	}
}

static void draw_pixram( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	fortyl_state *state = machine.driver_data<fortyl_state>();
	int offs;
	int f = state->m_flipscreen ^ 1;

	if (state->m_pix_redraw)
	{
		state->m_pix_redraw = 0;

		for (offs = 0; offs < 0x2000; offs++)
			fortyl_plot_pix(machine, offs);
	}

	if (state->m_pixram_sel)
		copybitmap(bitmap, state->m_tmp_bitmap1, f, f, state->m_xoffset, 0, cliprect);
	else
		copybitmap(bitmap, state->m_tmp_bitmap2, f, f, state->m_xoffset, 0, cliprect);
}

SCREEN_UPDATE( fortyl )
{
	fortyl_state *state = screen->machine().driver_data<fortyl_state>();
	draw_pixram(screen->machine(), bitmap, cliprect);

	tilemap_set_scrolldy(state->m_bg_tilemap, - state->m_video_ctrl[1] + 1, - state->m_video_ctrl[1] - 1 );
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
