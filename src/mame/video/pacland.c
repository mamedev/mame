/***************************************************************************

Sprite/tile priority is quite complex in this game: it is handled both
internally to the CUS29 chip, and externally to it.

The bg tilemap is always behind everything.

The CUS29 mixes two 8-bit inputs, one from sprites and one from the fg
tilemap. 0xff is the transparent color. CUS29 also takes a PRI input, telling
which of the two color inputs has priority. Additionally, sprite pixels of
color >= 0xf0 always have priority.
The priority bit comes from the tilemap RAM, but through an additional filter:
sprite pixels of color < 0x80 act as a "cookie cut" mask, handled externally,
which overload the PRI bit, making the sprite always have priority. The external
RAM that holds this mask contains the OR of all sprite pixels drawn at a certain
position, therefore when sprites overlap, it is sufficient for one of them to
have color < 0x80 to promote priority of the frontmost sprite. This is used
to draw the light in round 19.

The CUS29 outputs an 8-bit pixel color, but only the bottom 7 bits are externally
checked to determine whether it is transparent or not; therefore, both 0xff and
0x7f are transparent. This is again used to draw the light in round 19, because
sprite color 0x7f will erase the tilemap and force it to be transparent.

***************************************************************************/

#include "driver.h"


UINT8 *pacland_videoram,*pacland_videoram2,*pacland_spriteram;

static UINT8 palette_bank;
static const UINT8 *pacland_color_prom;

static colortable *pacland_colortable;
static tilemap *bg_tilemap, *fg_tilemap;
static mame_bitmap *fg_bitmap;

static UINT32 *transmask[3];

static UINT16 scroll0,scroll1;

/***************************************************************************

  Convert the color PROMs.

  Pacland has one 1024x8 and one 1024x4 palette PROM; and three 1024x8 lookup
  table PROMs (sprites, bg tiles, fg tiles).
  The palette has 1024 colors, but it is bank switched (4 banks) and only 256
  colors are visible at a time. So, instead of creating a static palette, we
  modify it when the bank switching takes place.
  The color PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

static void switch_palette(running_machine *machine)
{
	int i;
	const UINT8 *color_prom = pacland_color_prom + 256 * palette_bank;

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3;
		int r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[1024] >> 0) & 0x01;
		bit1 = (color_prom[1024] >> 1) & 0x01;
		bit2 = (color_prom[1024] >> 2) & 0x01;
		bit3 = (color_prom[1024] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		color_prom++;

		colortable_palette_set_color(pacland_colortable,i,MAKE_RGB(r,g,b));
	}
}

PALETTE_INIT( pacland )
{
	int i;

	/* allocate the colortable */
	pacland_colortable = colortable_alloc(machine, 256);

	pacland_color_prom = color_prom;	/* we'll need this later */
	/* skip the palette data, it will be initialized later */
	color_prom += 2 * 0x400;
	/* color_prom now points to the beginning of the lookup table */

	for (i = 0;i < 0x400;i++)
		colortable_entry_set_value(pacland_colortable, machine->gfx[0]->color_base + i, *color_prom++);

	/* Background */
	for (i = 0;i < 0x400;i++)
		colortable_entry_set_value(pacland_colortable, machine->gfx[1]->color_base + i, *color_prom++);

	/* Sprites */
	for (i = 0;i < 0x400;i++)
		colortable_entry_set_value(pacland_colortable, machine->gfx[2]->color_base + i, *color_prom++);

	palette_bank = 0;
	switch_palette(machine);

	/* precalculate transparency masks for sprites */
	transmask[0] = auto_malloc(64 * sizeof(transmask[0][0]));
	transmask[1] = auto_malloc(64 * sizeof(transmask[0][0]));
	transmask[2] = auto_malloc(64 * sizeof(transmask[0][0]));
	for (i = 0; i < 64; i++)
	{
		int palentry;

		/* start with no transparency */
		transmask[0][i] = transmask[1][i] =  transmask[2][i] = 0;

		/* iterate over all palette entries except the last one */
		for (palentry = 0; palentry < 0x100; palentry++)
		{
			UINT32 mask = colortable_get_transpen_mask(pacland_colortable, machine->gfx[2], i, palentry);

			/* transmask[0] is a mask that is used to draw only high priority sprite pixels; thus, pens
               $00-$7F are opaque, and others are transparent */
			if (palentry >= 0x80)
				transmask[0][i] |= mask;

			/* transmask[1] is a normal drawing masking with palette entries $7F and $FF transparent */
			if ((palentry & 0x7f) == 0x7f)
				transmask[1][i] |= mask;

			/* transmask[2] is a mask of the topmost priority sprite pixels; thus pens $F0-$FE are
               opaque, and others are transparent */
			if (palentry < 0xf0 || palentry == 0xff)
				transmask[2][i] |= mask;
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = pacland_videoram2[offs + 1];
	int code = pacland_videoram2[offs] + ((attr & 0x01) << 8);
	int color = ((attr & 0x3e) >> 1) + ((code & 0x1c0) >> 1);
	int flags = TILE_FLIPYX(attr >> 6);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int offs = tile_index * 2;
	int attr = pacland_videoram[offs + 1];
	int code = pacland_videoram[offs] + ((attr & 0x01) << 8);
	int color = ((attr & 0x1e) >> 1) + ((code & 0x1e0) >> 1);
	int flags = TILE_FLIPYX(attr >> 6);

	tileinfo->category = (attr & 0x20) ? 1 : 0;
	tileinfo->group = color;

	SET_TILE_INFO(0, code, color, flags);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pacland )
{
	int color;

	fg_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
	fillbitmap(fg_bitmap, 0xffff, NULL);

	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);

	tilemap_set_scroll_rows(fg_tilemap, 32);

	/* create one group per color code; for each group, set the transparency mask
       to correspond to the pens that are 0x7f or 0xff */
	assert(machine->gfx[0]->total_colors <= TILEMAP_NUM_GROUPS);
	for (color = 0; color < machine->gfx[0]->total_colors; color++)
	{
		UINT32 mask = colortable_get_transpen_mask(pacland_colortable, machine->gfx[0], color, 0x7f);
		mask |= colortable_get_transpen_mask(pacland_colortable, machine->gfx[0], color, 0xff);
		tilemap_set_transmask(fg_tilemap, color, mask, 0);
	}

	spriteram = pacland_spriteram + 0x780;
	spriteram_2 = spriteram + 0x800;
	spriteram_3 = spriteram_2 + 0x800;

	state_save_register_global(palette_bank);
	state_save_register_global(scroll0);
	state_save_register_global(scroll1);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( pacland_videoram_w )
{
	pacland_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset / 2);
}

WRITE8_HANDLER( pacland_videoram2_w )
{
	pacland_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( pacland_scroll0_w )
{
	scroll0 = data + 256 * offset;
}

WRITE8_HANDLER( pacland_scroll1_w )
{
	scroll1 = data + 256 * offset;
}

WRITE8_HANDLER( pacland_bankswitch_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(REGION_CPU1);

	bankaddress = 0x10000 + ((data & 0x07) << 13);
	memory_set_bankptr(1,&RAM[bankaddress]);

//  pbc = data & 0x20;

	if (palette_bank != ((data & 0x18) >> 3))
	{
		palette_bank = (data & 0x18) >> 3;
		switch_palette(Machine);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* the sprite generator IC is the same as Mappy */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int whichmask)
{
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs] + ((spriteram_3[offs] & 0x80) << 1);
		int color = spriteram[offs+1] & 0x3f;
		int sx = (spriteram_2[offs+1]) + 0x100*(spriteram_3[offs+1] & 1) - 47;
		int sy = 256 - spriteram_2[offs] + 9;
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizex = (spriteram_3[offs] & 0x04) >> 2;
		int sizey = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen)
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;	// fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				if (whichmask != 0)
					drawgfx(bitmap,machine->gfx[2],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						cliprect,TRANSPARENCY_PENS,transmask[whichmask][color]);
				else
					pdrawgfx(bitmap,machine->gfx[2],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						cliprect,TRANSPARENCY_PENS,transmask[whichmask][color],0);
			}
		}
	}
}


static void draw_fg(mame_bitmap *bitmap, const rectangle *cliprect, int priority )
{
	int y, x;

	/* draw tilemap transparently over it; this will leave invalid pens (0xffff)
       anywhere where the fg_tilemap should be transparent; note that we assume
       the fg_bitmap has been pre-erased to 0xffff */
	tilemap_draw(fg_bitmap, cliprect, fg_tilemap, priority, 0);

	/* now copy the fg_bitmap to the destination wherever the sprite pixel allows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		const UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);
		UINT16 *src = BITMAP_ADDR16(fg_bitmap, y, 0);
		UINT16 *dst = BITMAP_ADDR16(bitmap, y, 0);

		/* only copy if the priority bitmap is 0 (no high priority sprite) and the
           source pixel is not the invalid pen; also clear to 0xffff when finished */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT16 pix = src[x];
			if (pix != 0xffff)
			{
				src[x] = 0xffff;
				if (pri[x] == 0)
					dst[x] = pix;
			}
		}
	}
}


VIDEO_UPDATE( pacland )
{
	int row;

	for (row = 5; row < 29; row++)
		tilemap_set_scrollx(fg_tilemap, row, flip_screen ? scroll0-7 : scroll0);
	tilemap_set_scrollx(bg_tilemap, 0, flip_screen ? scroll1-4 : scroll1-3);

	/* draw high priority sprite pixels, setting priority bitmap to non-zero
       wherever there is a high-priority pixel; note that we draw to the bitmap
       which is safe because the bg_tilemap draw will overwrite everything */
	fillbitmap(priority_bitmap, 0x00, cliprect);
	draw_sprites(machine, bitmap, cliprect, 0);

	/* draw background */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* draw low priority fg tiles */
	draw_fg(bitmap, cliprect, 0);

	/* draw sprites with regular transparency */
	draw_sprites(machine, bitmap, cliprect, 1);

	/* draw high priority fg tiles */
	draw_fg(bitmap, cliprect, 1);

	/* draw sprite pixels with colortable values >= 0xf0, which have priority over everything */
	draw_sprites(machine, bitmap, cliprect, 2);
	return 0;
}
