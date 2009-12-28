/***************************************************************************

  video.c

***************************************************************************/

#include "driver.h"
#include "includes/nemesis.h"


UINT16 *nemesis_videoram1;
UINT16 *nemesis_videoram2;
UINT16 *nemesis_colorram1;
UINT16 *nemesis_colorram2;
UINT16 *nemesis_characterram;
size_t nemesis_characterram_size;
UINT16 *nemesis_xscroll1, *nemesis_xscroll2;
UINT16 *nemesis_yscroll1, *nemesis_yscroll2;

static int spriteram_words;
static int tilemap_flip;
static int flipscreen;
static UINT8 irq_port_last;

static tilemap_t *background, *foreground;

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

static UINT8 blank_tile[8*8];


static TILE_GET_INFO( get_bg_tile_info )
{
	int code,color,flags,mask,layer;

	code = nemesis_videoram2[tile_index];
	color = nemesis_colorram2[tile_index];
	flags = 0;

	if (color & 0x80) flags |= TILE_FLIPX;
	if (code & 0x0800) flags |= TILE_FLIPY;
	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;		/* no transparency */
	if (code & 0xf800) {
		SET_TILE_INFO( 0, code & 0x7ff, color & 0x7f, flags );
	} else {
		SET_TILE_INFO( 0, 0, 0x00, 0 );
		tileinfo->pen_data = blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo->category = mask | (layer << 1);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code,color,flags,mask,layer;

	code = nemesis_videoram1[tile_index];
	color = nemesis_colorram1[tile_index];
	flags = 0;

	if (color & 0x80) flags |= TILE_FLIPX;
	if (code & 0x0800) flags |= TILE_FLIPY;
	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;		/* no transparency */
	if (code & 0xf800) {
		SET_TILE_INFO( 0, code & 0x7ff, color & 0x7f, flags );
	} else {
		SET_TILE_INFO( 0, 0, 0x00, 0 );
		tileinfo->pen_data = blank_tile;
	}

	mask = (code & 0x1000) >> 12;
	layer = (code & 0x4000) >> 14;
	if (mask && !layer)
		layer = 1;

	tileinfo->category = mask | (layer << 1);
}


WRITE16_HANDLER( nemesis_gfx_flipx_word_w )
{
	if (ACCESSING_BITS_0_7)
	{
		flipscreen = data & 0x01;

		if (data & 0x01)
			tilemap_flip |= TILEMAP_FLIPX;
		else
			tilemap_flip &= ~TILEMAP_FLIPX;

		tilemap_set_flip_all(space->machine, tilemap_flip);
	}

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0x0100)
			cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);
	}
}

WRITE16_HANDLER( nemesis_gfx_flipy_word_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x01)
			tilemap_flip |= TILEMAP_FLIPY;
		else
			tilemap_flip &= ~TILEMAP_FLIPY;

		tilemap_set_flip_all(space->machine, tilemap_flip);
	}
}


WRITE16_HANDLER( salamand_control_port_word_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 accessing_bits = data ^ irq_port_last;

		nemesis_irq_on = data & 0x01;
		nemesis_irq2_on = data & 0x02;
		flipscreen = data & 0x04;

		if (data & 0x04)
			tilemap_flip |= TILEMAP_FLIPX;
		else
			tilemap_flip &= ~TILEMAP_FLIPX;

		if (data & 0x08)
			tilemap_flip |= TILEMAP_FLIPY;
		else
			tilemap_flip &= ~TILEMAP_FLIPY;

		if (accessing_bits & 0x0c)
			tilemap_set_flip_all(space->machine, tilemap_flip);

		irq_port_last = data;
	}

	if (ACCESSING_BITS_8_15)
	{
		coin_lockout_w(space->machine, 0, data & 0x0200);
		coin_lockout_w(space->machine, 1, data & 0x0400);

		if (data & 0x0800)
			cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);

		hcrash_selected_ip = (~data & 0x1000) >> 12;		/* citybomb steering & accel */
	}
}


WRITE16_HANDLER( nemesis_palette_word_w )
{
	int r,g,b,bit1,bit2,bit3,bit4,bit5;

	COMBINE_DATA(space->machine->generic.paletteram.u16 + offset);
	data = space->machine->generic.paletteram.u16[offset];

	/* Mish, 30/11/99 - Schematics show the resistor values are:
        300 Ohms
        620 Ohms
        1200 Ohms
        2400 Ohms
        4700 Ohms

        So the correct weights per bit are 8, 17, 33, 67, 130
    */

	#define MULTIPLIER 8 * bit1 + 17 * bit2 + 33 * bit3 + 67 * bit4 + 130 * bit5

	bit1=(data >>  0)&1;
	bit2=(data >>  1)&1;
	bit3=(data >>  2)&1;
	bit4=(data >>  3)&1;
	bit5=(data >>  4)&1;
	r = MULTIPLIER;
	r = pow (r/255.0, 2)*255;
	bit1=(data >>  5)&1;
	bit2=(data >>  6)&1;
	bit3=(data >>  7)&1;
	bit4=(data >>  8)&1;
	bit5=(data >>  9)&1;
	g = MULTIPLIER;
	g = pow (g/255.0, 2)*255;
	bit1=(data >>  10)&1;
	bit2=(data >>  11)&1;
	bit3=(data >>  12)&1;
	bit4=(data >>  13)&1;
	bit5=(data >>  14)&1;
	b = MULTIPLIER;
	b = pow (b/255.0, 2)*255;

	palette_set_color(space->machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( salamander_palette_word_w )
{
	COMBINE_DATA(space->machine->generic.paletteram.u16 + offset);
	offset &= ~1;

	data = ((space->machine->generic.paletteram.u16[offset] << 8) & 0xff00) | (space->machine->generic.paletteram.u16[offset+1] & 0xff);
	palette_set_color_rgb(space->machine,offset / 2,pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
}


WRITE16_HANDLER( nemesis_videoram1_word_w )
{
	COMBINE_DATA(nemesis_videoram1 + offset);
	tilemap_mark_tile_dirty( foreground, offset );
}

WRITE16_HANDLER( nemesis_videoram2_word_w )
{
	COMBINE_DATA(nemesis_videoram2 + offset);
	tilemap_mark_tile_dirty( background, offset );
}

WRITE16_HANDLER( nemesis_colorram1_word_w )
{
	COMBINE_DATA(nemesis_colorram1 + offset);
	tilemap_mark_tile_dirty( foreground, offset );
}

WRITE16_HANDLER( nemesis_colorram2_word_w )
{
	COMBINE_DATA(nemesis_colorram2 + offset);
	tilemap_mark_tile_dirty( background, offset );
}


/* we have to straighten out the 16-bit word into bytes for gfxdecode() to work */
WRITE16_HANDLER( nemesis_characterram_word_w )
{
	UINT16 oldword = nemesis_characterram[offset];
	COMBINE_DATA(nemesis_characterram + offset);
	data = nemesis_characterram[offset];

	if (oldword != data)
	{
		int i;
		for (i=0; i<8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			gfx_element_mark_dirty(space->machine->gfx[sprite_data[i].char_type], offset * 4 / (w * h));
		}
	}
}


static STATE_POSTLOAD( nemesis_postload )
{
	int i,offs;

	for (offs=0; offs<nemesis_characterram_size; offs++)
	{
		for (i=0; i<8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			gfx_element_mark_dirty(machine->gfx[sprite_data[i].char_type], offs * 4 / (w * h));
		}
	}
	tilemap_mark_all_tiles_dirty(background);
	tilemap_mark_all_tiles_dirty(foreground);
}


/* claim a palette dirty array */
VIDEO_START( nemesis )
{
	spriteram_words = machine->generic.spriteram_size / 2;

	background = tilemap_create(machine,
		get_bg_tile_info, tilemap_scan_rows,  8,8, 64,32 );

	foreground = tilemap_create(machine,
		get_fg_tile_info, tilemap_scan_rows,  8,8, 64,32 );

	tilemap_set_transparent_pen( background, 0 );
	tilemap_set_transparent_pen( foreground, 0 );
	tilemap_set_scroll_rows( background, 256 );
	tilemap_set_scroll_rows( foreground, 256 );

	memset(nemesis_characterram, 0, nemesis_characterram_size);

	gfx_element_set_source(machine->gfx[0], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[1], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[2], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[3], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[4], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[5], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[6], (UINT8 *)nemesis_characterram);
	gfx_element_set_source(machine->gfx[7], (UINT8 *)nemesis_characterram);

	flipscreen = 0;
	tilemap_flip = 0;
	irq_port_last = 0;

	/* Set up save state */
	state_save_register_global(machine, spriteram_words);
	state_save_register_global(machine, tilemap_flip);
	state_save_register_global(machine, flipscreen);
	state_save_register_global(machine, irq_port_last);
	state_save_register_postload(machine, nemesis_postload, NULL);
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
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

	UINT16 *spriteram16 = machine->generic.spriteram.u16;
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

	for (priority=256-1; priority>=0; priority--)
	{
		for (adress = spriteram_words-8; adress >= 0; adress -= 8)
		{
			if((spriteram16[adress] & 0xff)!=priority) continue;

			zoom = spriteram16[adress+2] & 0xff;
			if (!(spriteram16[adress+2] & 0xff00) && ((spriteram16[adress+3] & 0xff00) != 0xff00))
				code = spriteram16[adress+3] + ((spriteram16[adress+4] & 0xc0) << 2);
			else
				code = (spriteram16[adress+3] & 0xff) + ((spriteram16[adress+4] & 0xc0) << 2);

			if (zoom != 0xFF || code!=0)
			{
				size = spriteram16[adress+1];
				zoom += (size & 0xc0) << 2;

				sx = spriteram16[adress+5] & 0xff;
				sy = spriteram16[adress+6] & 0xff;
				if (spriteram16[adress+4] & 0x01)
					sx-=0x100;	/* fixes left side clip */
				color = (spriteram16[adress+4] & 0x1e) >> 1;
				flipx = spriteram16[adress+1] & 0x01;
				flipy = spriteram16[adress+4] & 0x20;

				idx = (size >> 3) & 7;
				w = sprite_data[idx].width;
				h = sprite_data[idx].height;
				code = code * 8 * 16 / (w * h);
				char_type = sprite_data[idx].char_type;

				if( zoom )
				{
					zoom = ((1<<16) * 0x80 / zoom) + 0x02ab;
					if (flipscreen)
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
			} /* if sprite */
		} /* for loop */
	} /* priority */
}

/******************************************************************************/

VIDEO_UPDATE( nemesis )
{
	int offs;
	rectangle clip;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0);

	clip.min_x = 0;
	clip.max_x = 255;

	tilemap_set_scroll_cols( background, 64 );
	tilemap_set_scroll_cols( foreground, 64 );
	tilemap_set_scroll_rows( background, 1 );
	tilemap_set_scroll_rows( foreground, 1 );

	for (offs = 0; offs < 64; offs++)
	{
		int offset_x = offs;

		if (flipscreen)
			offset_x = (offs + 0x20) & 0x3f;

		tilemap_set_scrolly( background, offs, nemesis_yscroll2[offset_x] );
		tilemap_set_scrolly( foreground, offs, nemesis_yscroll1[offset_x] );
	}

	for (offs = cliprect->min_y; offs <= cliprect->max_y; offs++)
	{
		int i;
		int offset_y = offs;

		clip.min_y = offs;
		clip.max_y = offs;

		if (flipscreen)
			offset_y = 255 - offs;

		tilemap_set_scrollx( background, 0, (nemesis_xscroll2[offset_y] & 0xff) + ((nemesis_xscroll2[0x100 + offset_y] & 0x01) << 8) - (flipscreen ? 0x107 : 0) );
		tilemap_set_scrollx( foreground, 0, (nemesis_xscroll1[offset_y] & 0xff) + ((nemesis_xscroll1[0x100 + offset_y] & 0x01) << 8) - (flipscreen ? 0x107 : 0) );

		for (i=0; i<4; i+=2)
		{
			tilemap_draw(bitmap, &clip, background, TILEMAP_DRAW_CATEGORY(i+0), 1);
			tilemap_draw(bitmap, &clip, background, TILEMAP_DRAW_CATEGORY(i+1), 2);
			tilemap_draw(bitmap, &clip, foreground, TILEMAP_DRAW_CATEGORY(i+0), 1);
			tilemap_draw(bitmap, &clip, foreground, TILEMAP_DRAW_CATEGORY(i+1), 2);
		}
	}

	draw_sprites(screen->machine,bitmap,cliprect);

	return 0;
}
