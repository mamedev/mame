/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

#include "emu.h"
#include "includes/centiped.h"


/*************************************
 *
 *  Tilemap callback
 *
 *************************************/

static TILE_GET_INFO( centiped_get_tile_info )
{
	centiped_state *state = machine.driver_data<centiped_state>();
	UINT8 *videoram = state->m_videoram;

	int data = videoram[tile_index];
	SET_TILE_INFO(0, (data & 0x3f) + 0x40, 0, TILE_FLIPYX(data >> 6));
}


static TILE_GET_INFO( warlords_get_tile_info )
{
	centiped_state *state = machine.driver_data<centiped_state>();
	UINT8 *videoram = state->m_videoram;
	int data = videoram[tile_index];
	int color = ((tile_index & 0x10) >> 4) | ((tile_index & 0x200) >> 8) | (state->m_flipscreen >> 5);

	SET_TILE_INFO(0, data & 0x3f, color, TILE_FLIPYX(data >> 6));
}


static TILE_GET_INFO( milliped_get_tile_info )
{
	centiped_state *state = machine.driver_data<centiped_state>();
	UINT8 *videoram = state->m_videoram;
	int data = videoram[tile_index];
	int bank = (data >> 6) & 1;
	int color = (data >> 6) & 3;
	/* Flip both x and y if flipscreen is non-zero */
	int flip_tiles = (state->m_flipscreen) ? 0x03 : 0;

	SET_TILE_INFO(0, (data & 0x3f) + 0x40 + (bank * 0x80), color, TILE_FLIPYX(flip_tiles));
}


static TILE_GET_INFO( bullsdrt_get_tile_info )
{
	centiped_state *state = machine.driver_data<centiped_state>();
	UINT8 *videoram = state->m_videoram;
	int data = videoram[tile_index];
	int bank = state->m_bullsdrt_tiles_bankram[tile_index & 0x1f] & 0x0f;

	SET_TILE_INFO(0, (data & 0x3f) + 0x40 * bank, 0, TILE_FLIPYX(data >> 6));
}



/*************************************
 *
 *  Palette init
 *
 *************************************/

static void init_penmask(running_machine &machine)
{
	centiped_state *state = machine.driver_data<centiped_state>();
	int i;

	for (i = 0; i < 64; i++)
	{
		UINT8 mask = 1;
		if (((i >> 0) & 3) == 0) mask |= 2;
		if (((i >> 2) & 3) == 0) mask |= 4;
		if (((i >> 4) & 3) == 0) mask |= 8;
		state->m_penmask[i] = mask;
	}
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( centiped )
{
	centiped_state *state = machine.driver_data<centiped_state>();
	state->m_bg_tilemap = tilemap_create(machine, centiped_get_tile_info, tilemap_scan_rows,  8,8, 32,32);

	init_penmask(machine);

	state->m_flipscreen = 0;

	state->save_item(NAME(state->m_flipscreen));
	state->save_item(NAME(state->m_bullsdrt_sprites_bank));
}


VIDEO_START( warlords )
{
	centiped_state *state = machine.driver_data<centiped_state>();

	state->m_bg_tilemap = tilemap_create(machine, warlords_get_tile_info, tilemap_scan_rows,  8,8, 32,32);

	state->m_flipscreen = 0;
}


VIDEO_START( milliped )
{
	centiped_state *state = machine.driver_data<centiped_state>();

	state->m_bg_tilemap = tilemap_create(machine, milliped_get_tile_info, tilemap_scan_rows,  8,8, 32,32);

	init_penmask(machine);

	state->m_flipscreen = 0;

	state->save_item(NAME(state->m_flipscreen));
}


VIDEO_START( bullsdrt )
{
	centiped_state *state = machine.driver_data<centiped_state>();

	state->m_bg_tilemap = tilemap_create(machine, bullsdrt_get_tile_info, tilemap_scan_rows,  8,8, 32,32);

	init_penmask(machine);

	state->m_flipscreen = 0;
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_MEMBER(centiped_state::centiped_videoram_w)
{
	UINT8 *videoram = m_videoram;

	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Screen flip
 *
 *************************************/

WRITE8_MEMBER(centiped_state::centiped_flip_screen_w)
{

	m_flipscreen = data >> 7;
}



/*************************************
 *
 *  Tiles bank
 *
 *************************************/

WRITE8_MEMBER(centiped_state::bullsdrt_tilesbank_w)
{

	m_bullsdrt_tiles_bankram[offset] = data;
	m_bg_tilemap->mark_all_dirty();
}



/*************************************
 *
 *  Sprites bank
 *
 *************************************/

WRITE8_MEMBER(centiped_state::bullsdrt_sprites_bank_w)
{

	m_bullsdrt_sprites_bank = data;
}



/***************************************************************************

    Centipede doesn't have a color PROM. Eight RAM locations control
    the color of characters and sprites. The meanings of the four bits are
    (all bits are inverted):

    bit 3 alternate
          blue
          green
    bit 0 red

    The alternate bit affects blue and green, not red. The way I weighted its
    effect might not be perfectly accurate, but is reasonably close.

    Centipede is unusual because the sprite color code specifies the
    colors to use one by one, instead of a combination code.
    bit 5-4 = color to use for pen 11
    bit 3-2 = color to use for pen 10
    bit 1-0 = color to use for pen 01
    pen 00 is transparent

***************************************************************************/

WRITE8_MEMBER(centiped_state::centiped_paletteram_w)
{
	m_generic_paletteram_8[offset] = data;

	/* bit 2 of the output palette RAM is always pulled high, so we ignore */
	/* any palette changes unless the write is to a palette RAM address */
	/* that is actually used */
	if (offset & 4)
	{
		rgb_t color;

		int r = 0xff * ((~data >> 0) & 1);
		int g = 0xff * ((~data >> 1) & 1);
		int b = 0xff * ((~data >> 2) & 1);

		if (~data & 0x08) /* alternate = 1 */
		{
			/* when blue component is not 0, decrease it. When blue component is 0, */
			/* decrease green component. */
			if (b) b = 0xc0;
			else if (g) g = 0xc0;
		}

		color = MAKE_RGB(r, g, b);

		/* character colors, set directly */
		if ((offset & 0x08) == 0)
			palette_set_color(machine(), offset & 0x03, color);

		/* sprite colors - set all the applicable ones */
		else
		{
			int i;

			offset = offset & 0x03;

			for (i = 0; i < 0x100; i += 4)
			{
				if (offset == ((i >> 2) & 0x03))
					palette_set_color(machine(), i + 4 + 1, color);

				if (offset == ((i >> 4) & 0x03))
					palette_set_color(machine(), i + 4 + 2, color);

				if (offset == ((i >> 6) & 0x03))
					palette_set_color(machine(), i + 4 + 3, color);
			}
		}
	}
}



/***************************************************************************

    Convert the color PROM into a more useable format.

    The palette PROM are connected to the RGB output this way:

    bit 2 -- RED
          -- GREEN
    bit 0 -- BLUE

***************************************************************************/

PALETTE_INIT( warlords )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		UINT8 pen;
		int r, g, b;

		if (i < 0x20)
			/* characters */
			pen = (((i - 0x00) & 0x1c) << 2) | (((i - 0x00) & 0x03) << 0);
		else
			/* sprites */
			pen = (((i - 0x20) & 0x1c) << 2) | (((i - 0x20) & 0x03) << 2);

		r = ((color_prom[pen] >> 2) & 0x01) * 0xff;
		g = ((color_prom[pen] >> 1) & 0x01) * 0xff;
		b = ((color_prom[pen] >> 0) & 0x01) * 0xff;

		/* colors 0x40-0x7f are converted to grey scale as it's used on the
           upright version that had an overlay */
		if (pen >= 0x40)
		{
			/* use the standard ratios: r = 30%, g = 59%, b = 11% */
			int grey = (r * 0x4d / 0xff) + (g * 0x96 / 0xff) + (b * 0x1c / 0xff);
			r = g = b = grey;
		}

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}



/***************************************************************************

    Millipede doesn't have a color PROM, it uses RAM.
    The RAM seems to be conncted to the video output this way:

    bit 7 red
          red
          red
          green
          green
          blue
          blue
    bit 0 blue

    Millipede is unusual because the sprite color code specifies the
    colors to use one by one, instead of a combination code.
    bit 7-6 = palette bank (there are 4 groups of 4 colors)
    bit 5-4 = color to use for pen 11
    bit 3-2 = color to use for pen 10
    bit 1-0 = color to use for pen 01
    pen 00 is transparent

***************************************************************************/

static void melliped_mazeinv_set_color(running_machine &machine, offs_t offset, UINT8 data)
{
	rgb_t color;
	int bit0, bit1, bit2;
	int r, g, b;

	/* red component */
	bit0 = (~data >> 5) & 0x01;
	bit1 = (~data >> 6) & 0x01;
	bit2 = (~data >> 7) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* green component */
	bit0 = 0;
	bit1 = (~data >> 3) & 0x01;
	bit2 = (~data >> 4) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* blue component */
	bit0 = (~data >> 0) & 0x01;
	bit1 = (~data >> 1) & 0x01;
	bit2 = (~data >> 2) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	color = MAKE_RGB(r, g, b);

	/* character colors, set directly */
	if (offset < 0x10)
		palette_set_color(machine, offset, color);

	/* sprite colors - set all the applicable ones */
	else
	{
		int i;

		int base = offset & 0x0c;

		offset = offset & 0x03;

		for (i = (base << 6); i < (base << 6) + 0x100; i += 4)
		{
			if (offset == ((i >> 2) & 0x03))
				palette_set_color(machine, i + 0x10 + 1, color);

			if (offset == ((i >> 4) & 0x03))
				palette_set_color(machine, i + 0x10 + 2, color);

			if (offset == ((i >> 6) & 0x03))
				palette_set_color(machine, i + 0x10 + 3, color);
		}
	}
}


WRITE8_MEMBER(centiped_state::milliped_paletteram_w)
{
	m_generic_paletteram_8[offset] = data;

	melliped_mazeinv_set_color(machine(), offset, data);
}


WRITE8_MEMBER(centiped_state::mazeinv_paletteram_w)
{
	m_generic_paletteram_8[offset] = data;

	/* the value passed in is a look-up index into the color PROM */
	melliped_mazeinv_set_color(machine(), offset, ~machine().region("proms")->base()[~data & 0x0f]);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( centiped )
{
	centiped_state *state = screen.machine().driver_data<centiped_state>();
	UINT8 *spriteram = state->m_spriteram;
	rectangle spriteclip = cliprect;
	int offs;

	/* draw the background */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* apply the sprite clip */
	if (state->m_flipscreen)
		spriteclip.min_x += 8;
	else
		spriteclip.max_x -= 8;

	/* draw the sprites */
	for (offs = 0; offs < 0x10; offs++)
	{
		int code = ((spriteram[offs] & 0x3e) >> 1) | ((spriteram[offs] & 0x01) << 6);
		int color = spriteram[offs + 0x30];
		int flipx = (spriteram[offs] >> 6) & 1;
		int flipy = (spriteram[offs] >> 7) & 1;
		int x = spriteram[offs + 0x20];
		int y = 240 - spriteram[offs + 0x10];

		drawgfx_transmask(bitmap, spriteclip, screen.machine().gfx[1], code, color, flipx, flipy, x, y, state->m_penmask[color & 0x3f]);
	}
	return 0;
}


SCREEN_UPDATE_IND16( warlords )
{
	centiped_state *state = screen.machine().driver_data<centiped_state>();
	UINT8 *spriteram = state->m_spriteram;
	int upright_mode = input_port_read(screen.machine(), "IN0") & 0x80;
	int offs;

	/* if the cocktail/upright switch flipped, force refresh */
	if (state->m_flipscreen != upright_mode)
	{
		state->m_flipscreen = upright_mode;
		state->m_bg_tilemap->set_flip(upright_mode ? TILEMAP_FLIPX : 0);
		state->m_bg_tilemap->mark_all_dirty();
	}

	/* draw the background */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0; offs < 0x10; offs++)
	{
		int code = spriteram[offs] & 0x3f;
		int flipx = (spriteram[offs] >> 6) & 1;
		int flipy = (spriteram[offs] >> 7) & 1;
		int x = spriteram[offs + 0x20];
		int y = 248 - spriteram[offs + 0x10];

		/* The four quadrants have different colors. This is not 100% accurate,
           because right on the middle the sprite could actually have two or more
           different color, but this is not noticeable, as the color that
           changes between the quadrants is mostly used on the paddle sprites */
		int color = ((y & 0x80) >> 6) | ((x & 0x80) >> 7) | (upright_mode >> 5);

		/* in upright mode, sprites are flipped */
		if (upright_mode)
		{
			x = 248 - x;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1], code, color, flipx, flipy, x, y, 0);
	}
	return 0;
}


SCREEN_UPDATE_IND16( bullsdrt )
{
	centiped_state *state = screen.machine().driver_data<centiped_state>();
	UINT8 *spriteram = state->m_spriteram;
	rectangle spriteclip = cliprect;

	int offs;

	/* draw the background */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* apply the sprite clip */
	if (state->m_flipscreen)
		spriteclip.min_x += 8;
	else
		spriteclip.max_x -= 8;

	/* draw the sprites */
	for (offs = 0; offs < 0x10; offs++)
	{
		int code = ((spriteram[offs] & 0x3e) >> 1) | ((spriteram[offs] & 0x01) << 6) | (state->m_bullsdrt_sprites_bank * 0x20);
		int color = spriteram[offs + 0x30];
		int flipy = (spriteram[offs] >> 7) & 1;
		int x = spriteram[offs + 0x20];
		int y = 240 - spriteram[offs + 0x10];

		drawgfx_transpen(bitmap, spriteclip, screen.machine().gfx[1], code, color & 0x3f, 1, flipy, x, y, 0);
	}
	return 0;
}

/*
 * This varies from Centipede, in that flipx is not in
 * the data, but is determined by VIDROT value at 0x2506.
 */
SCREEN_UPDATE_IND16( milliped )
{
	centiped_state *state = screen.machine().driver_data<centiped_state>();
	UINT8 *spriteram = state->m_spriteram;
	rectangle spriteclip = cliprect;
	int offs;

	/* draw the background */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* apply the sprite clip */
	if (state->m_flipscreen)
		spriteclip.min_x += 8;
	else
		spriteclip.max_x -= 8;

	/* draw the sprites */
	for (offs = 0; offs < 0x10; offs++)
	{
		int code = ((spriteram[offs] & 0x3e) >> 1) | ((spriteram[offs] & 0x01) << 6);
		int color = spriteram[offs + 0x30];
		int flipx = state->m_flipscreen;
		int flipy = (spriteram[offs] & 0x80);
		int x = spriteram[offs + 0x20];
		int y = 240 - spriteram[offs + 0x10];
		if (flipx) {
			flipy = !flipy;
		}

		drawgfx_transmask(bitmap, spriteclip, screen.machine().gfx[1], code, color, flipx, flipy, x, y, state->m_penmask[color & 0x3f]);
	}
	return 0;
}

