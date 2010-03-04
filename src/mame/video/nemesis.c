/***************************************************************************

  video.c

***************************************************************************/

#include "emu.h"
#include "includes/nemesis.h"


static const struct
{
	UINT8 width;
	UINT8 height;
	UINT8 char_type;
}
sprite_data[8] =
{
	{ 32, 32, 4 }, { 16, 32, 5 }, { 32, 16, 2 }, { 64, 64, 7 },
	{  8,  8, 0 }, { 16,  8, 6 }, {  8, 16, 3 }, { 16, 16, 1 }
};


static TILE_GET_INFO( get_bg_tile_info )
{
	nemesis_state *state = (nemesis_state *)machine->driver_data;
	int code, color, flags, mask, layer;

	code = state->videoram2[tile_index];
	color = state->colorram2[tile_index];
	flags = 0;

	if (color & 0x80)
		flags |= TILE_FLIPX;

	if (code & 0x0800)
		flags |= TILE_FLIPY;

	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;		/* no transparency */

	if (code & 0xf800)
	{
		SET_TILE_INFO( 0, code & 0x7ff, color & 0x7f, flags );
	}
	else
	{
		SET_TILE_INFO( 0, 0, 0x00, 0 );
		tileinfo->pen_data = state->blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo->category = mask | (layer << 1);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	nemesis_state *state = (nemesis_state *)machine->driver_data;
	int code, color, flags, mask, layer;

	code = state->videoram1[tile_index];
	color = state->colorram1[tile_index];
	flags = 0;

	if (color & 0x80)
		flags |= TILE_FLIPX;

	if (code & 0x0800)
		flags |= TILE_FLIPY;

	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;		/* no transparency */

	if (code & 0xf800)
	{
		SET_TILE_INFO( 0, code & 0x7ff, color & 0x7f, flags );
	}
	else
	{
		SET_TILE_INFO( 0, 0, 0x00, 0 );
		tileinfo->pen_data = state->blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo->category = mask | (layer << 1);
}


WRITE16_HANDLER( nemesis_gfx_flipx_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		state->flipscreen = data & 0x01;

		if (data & 0x01)
			state->tilemap_flip |= TILEMAP_FLIPX;
		else
			state->tilemap_flip &= ~TILEMAP_FLIPX;

		tilemap_set_flip_all(space->machine, state->tilemap_flip);
	}

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0x0100)
			cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
	}
}

WRITE16_HANDLER( nemesis_gfx_flipy_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x01)
			state->tilemap_flip |= TILEMAP_FLIPY;
		else
			state->tilemap_flip &= ~TILEMAP_FLIPY;

		tilemap_set_flip_all(space->machine, state->tilemap_flip);
	}
}


WRITE16_HANDLER( salamand_control_port_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		UINT8 accessing_bits = data ^ state->irq_port_last;

		state->irq_on = data & 0x01;
		state->irq2_on = data & 0x02;
		state->flipscreen = data & 0x04;

		if (data & 0x04)
			state->tilemap_flip |= TILEMAP_FLIPX;
		else
			state->tilemap_flip &= ~TILEMAP_FLIPX;

		if (data & 0x08)
			state->tilemap_flip |= TILEMAP_FLIPY;
		else
			state->tilemap_flip &= ~TILEMAP_FLIPY;

		if (accessing_bits & 0x0c)
			tilemap_set_flip_all(space->machine, state->tilemap_flip);

		state->irq_port_last = data;
	}

	if (ACCESSING_BITS_8_15)
	{
		coin_lockout_w(space->machine, 0, data & 0x0200);
		coin_lockout_w(space->machine, 1, data & 0x0400);

		if (data & 0x0800)
			cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);

		state->selected_ip = (~data & 0x1000) >> 12;		/* citybomb steering & accel */
	}
}


WRITE16_HANDLER( nemesis_palette_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;
	int r, g, b, bit1, bit2, bit3, bit4, bit5;

	COMBINE_DATA(state->paletteram + offset);
	data = state->paletteram[offset];

	/* Mish, 30/11/99 - Schematics show the resistor values are:
        300 Ohms
        620 Ohms
        1200 Ohms
        2400 Ohms
        4700 Ohms

        So the correct weights per bit are 8, 17, 33, 67, 130
    */

	#define MULTIPLIER 8 * bit1 + 17 * bit2 + 33 * bit3 + 67 * bit4 + 130 * bit5

	bit1 = BIT(data, 0);
	bit2 = BIT(data, 1);
	bit3 = BIT(data, 2);
	bit4 = BIT(data, 3);
	bit5 = BIT(data, 4);
	r = MULTIPLIER;
	r = pow(r/255.0, 2)*255;
	bit1 = BIT(data, 5);
	bit2 = BIT(data, 6);
	bit3 = BIT(data, 7);
	bit4 = BIT(data, 8);
	bit5 = BIT(data, 9);
	g = MULTIPLIER;
	g = pow(g/255.0, 2)*255;
	bit1 = BIT(data, 10);
	bit2 = BIT(data, 11);
	bit3 = BIT(data, 12);
	bit4 = BIT(data, 13);
	bit5 = BIT(data, 14);
	b = MULTIPLIER;
	b = pow(b/255.0, 2)*255;

	palette_set_color(space->machine, offset, MAKE_RGB(r, g, b));
}

WRITE16_HANDLER( salamander_palette_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	COMBINE_DATA(state->paletteram + offset);
	offset &= ~1;

	data = ((state->paletteram[offset] << 8) & 0xff00) | (state->paletteram[offset + 1] & 0xff);
	palette_set_color_rgb(space->machine, offset / 2, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}


WRITE16_HANDLER( nemesis_videoram1_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	COMBINE_DATA(state->videoram1 + offset);
	tilemap_mark_tile_dirty(state->foreground, offset);
}

WRITE16_HANDLER( nemesis_videoram2_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	COMBINE_DATA(state->videoram2 + offset);
	tilemap_mark_tile_dirty(state->background, offset);
}

WRITE16_HANDLER( nemesis_colorram1_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	COMBINE_DATA(state->colorram1 + offset);
	tilemap_mark_tile_dirty(state->foreground, offset);
}

WRITE16_HANDLER( nemesis_colorram2_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;

	COMBINE_DATA(state->colorram2 + offset);
	tilemap_mark_tile_dirty(state->background, offset);
}


/* we have to straighten out the 16-bit word into bytes for gfxdecode() to work */
WRITE16_HANDLER( nemesis_charram_word_w )
{
	nemesis_state *state = (nemesis_state *)space->machine->driver_data;
	UINT16 oldword = state->charram[offset];

	COMBINE_DATA(state->charram + offset);
	data = state->charram[offset];

	if (oldword != data)
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			gfx_element_mark_dirty(space->machine->gfx[sprite_data[i].char_type], offset * 4 / (w * h));
		}
	}
}


static STATE_POSTLOAD( nemesis_postload )
{
	nemesis_state *state = (nemesis_state *)machine->driver_data;
	int i, offs;

	for (offs = 0; offs < state->charram_size; offs++)
	{
		for (i = 0; i < 8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			gfx_element_mark_dirty(machine->gfx[sprite_data[i].char_type], offs * 4 / (w * h));
		}
	}
	tilemap_mark_all_tiles_dirty(state->background);
	tilemap_mark_all_tiles_dirty(state->foreground);
}


/* claim a palette dirty array */
VIDEO_START( nemesis )
{
	nemesis_state *state = (nemesis_state *)machine->driver_data;

	state->spriteram_words = state->spriteram_size / 2;

	state->background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,  8, 8, 64, 32);
	state->foreground = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8, 8, 64, 32);

	tilemap_set_transparent_pen(state->background, 0);
	tilemap_set_transparent_pen(state->foreground, 0);
	tilemap_set_scroll_rows(state->background, 256);
	tilemap_set_scroll_rows(state->foreground, 256);

	memset(state->charram, 0, state->charram_size);
	memset(state->blank_tile, 0, ARRAY_LENGTH(state->blank_tile));

	gfx_element_set_source(machine->gfx[0], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[1], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[2], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[3], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[4], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[5], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[6], (UINT8 *)state->charram);
	gfx_element_set_source(machine->gfx[7], (UINT8 *)state->charram);

	/* Set up save state */
	state_save_register_postload(machine, nemesis_postload, NULL);
}


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	/*
     *  16 bytes per sprite, in memory from 56000-56fff
     *
     *  byte    0 : relative priority.
     *  byte    2 : size (?) value #E0 means not used., bit 0x01 is flipx
                    0xc0 is upper 2 bits of zoom.
                    0x38 is size.
     *  byte    4 : zoom = 0xff
     *  byte    6 : low bits sprite code.
     *  byte    8 : color + hi bits sprite code., bit 0x20 is flipy bit. bit 0x01 is high bit of X pos.
     *  byte    A : X position.
     *  byte    C : Y position.
     *  byte    E : not used.
     */

	nemesis_state *state = (nemesis_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int adress;	/* start of sprite in spriteram */
	int sx;	/* sprite X-pos */
	int sy;	/* sprite Y-pos */
	int code;	/* start of sprite in obj RAM */
	int color;	/* color of the sprite */
	int flipx,flipy;
	int zoom;
	int char_type;
	int priority;
	int size;
	int w,h;
	int idx;

	for (priority = 256 - 1; priority >= 0; priority--)
	{
		for (adress = state->spriteram_words - 8; adress >= 0; adress -= 8)
		{
			if((spriteram[adress] & 0xff) != priority)
				continue;

			zoom = spriteram[adress + 2] & 0xff;
			if (!(spriteram[adress + 2] & 0xff00) && ((spriteram[adress + 3] & 0xff00) != 0xff00))
				code = spriteram[adress + 3] + ((spriteram[adress + 4] & 0xc0) << 2);
			else
				code = (spriteram[adress + 3] & 0xff) + ((spriteram[adress + 4] & 0xc0) << 2);

			if (zoom != 0xff || code != 0)
			{
				size = spriteram[adress + 1];
				zoom += (size & 0xc0) << 2;

				sx = spriteram[adress + 5] & 0xff;
				sy = spriteram[adress + 6] & 0xff;
				if (spriteram[adress + 4] & 0x01)
					sx-=0x100;	/* fixes left side clip */

				color = (spriteram[adress + 4] & 0x1e) >> 1;
				flipx = spriteram[adress + 1] & 0x01;
				flipy = spriteram[adress + 4] & 0x20;

				idx = (size >> 3) & 7;
				w = sprite_data[idx].width;
				h = sprite_data[idx].height;
				code = code * 8 * 16 / (w * h);
				char_type = sprite_data[idx].char_type;

				if (zoom)
				{
					zoom = ((1 << 16) * 0x80 / zoom) + 0x02ab;
					if (state->flipscreen)
					{
						sx = 256 - ((zoom * w) >> 16) - sx;
						sy = 256 - ((zoom * h) >> 16) - sy;
						flipx = !flipx;
						flipy = !flipy;
					}

					pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[char_type],
						code,
						color,
						flipx,flipy,
						sx,sy,
						zoom,zoom,
						machine->priority_bitmap,0xffcc,0 );
				}
			}
		}
	}
}

/******************************************************************************/

VIDEO_UPDATE( nemesis )
{
	nemesis_state *state = (nemesis_state *)screen->machine->driver_data;
	int offs;
	rectangle clip;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	clip.min_x = 0;
	clip.max_x = 255;

	tilemap_set_scroll_cols(state->background, 64);
	tilemap_set_scroll_cols(state->foreground, 64);
	tilemap_set_scroll_rows(state->background, 1);
	tilemap_set_scroll_rows(state->foreground, 1);

	for (offs = 0; offs < 64; offs++)
	{
		int offset_x = offs;

		if (state->flipscreen)
			offset_x = (offs + 0x20) & 0x3f;

		tilemap_set_scrolly(state->background, offs, state->yscroll2[offset_x]);
		tilemap_set_scrolly(state->foreground, offs, state->yscroll1[offset_x]);
	}

	for (offs = cliprect->min_y; offs <= cliprect->max_y; offs++)
	{
		int i;
		int offset_y = offs;

		clip.min_y = offs;
		clip.max_y = offs;

		if (state->flipscreen)
			offset_y = 255 - offs;

		tilemap_set_scrollx(state->background, 0, (state->xscroll2[offset_y] & 0xff) + ((state->xscroll2[0x100 + offset_y] & 0x01) << 8) - (state->flipscreen ? 0x107 : 0));
		tilemap_set_scrollx(state->foreground, 0, (state->xscroll1[offset_y] & 0xff) + ((state->xscroll1[0x100 + offset_y] & 0x01) << 8) - (state->flipscreen ? 0x107 : 0));

		for (i = 0; i < 4; i += 2)
		{
			tilemap_draw(bitmap, &clip, state->background, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			tilemap_draw(bitmap, &clip, state->background, TILEMAP_DRAW_CATEGORY(i + 1), 2);
			tilemap_draw(bitmap, &clip, state->foreground, TILEMAP_DRAW_CATEGORY(i + 0), 1);
			tilemap_draw(bitmap, &clip, state->foreground, TILEMAP_DRAW_CATEGORY(i + 1), 2);
		}
	}

	draw_sprites(screen->machine,bitmap,cliprect);

	return 0;
}
