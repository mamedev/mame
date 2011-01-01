/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "emu.h"
#include "includes/pgm.h"

/* Sprites - These are a pain! */

/* this decodes one of the funky sprites to a bitmap so we can draw it more easily -- slow but easier to use*/
static void pgm_prepare_sprite( running_machine *machine, int wide, int high, int palt, int boffset )
{
	pgm_state *state = machine->driver_data<pgm_state>();
	UINT8 *bdata = machine->region("sprmask")->base();
	size_t  bdatasize = machine->region("sprmask")->bytes() - 1;
	UINT8 *adata = state->sprite_a_region;
	size_t  adatasize = state->sprite_a_region_size - 1;
	int xcnt, ycnt;

	UINT32 aoffset;
	UINT16 msk;

	aoffset = (bdata[(boffset + 3) & bdatasize] << 24) | (bdata[(boffset + 2) & bdatasize] << 16) |
				(bdata[(boffset + 1) & bdatasize] << 8) | (bdata[(boffset + 0) & bdatasize] << 0);
	aoffset = aoffset >> 2; aoffset *= 3;

	boffset += 4; /* because the first dword is the a data offset */

	for (ycnt = 0 ; ycnt < high ; ycnt++)
	{
		for (xcnt = 0 ; xcnt < wide ; xcnt++)
		{
			int x;

			msk = ((bdata[(boffset + 1) & bdatasize] << 8) |( bdata[(boffset + 0) & bdatasize] << 0));

			for (x = 0; x < 16; x++)
			{
				if (!(msk & 0x0001))
				{
					state->sprite_temp_render[(ycnt * (wide * 16))+(xcnt * 16 + x)] = adata[aoffset & adatasize] + palt * 32;
					aoffset++;
				}
				else
				{
					state->sprite_temp_render[(ycnt * (wide * 16)) + (xcnt * 16 + x)] = 0x8000;
				}
				msk >>= 1;
			}

			boffset += 2;
		}
	}
}


// in the dest bitmap 0x10000 is used to mark 'used pixel' and 0x8000 is used to mark 'high priority'
static void draw_sprite_line( running_machine *machine, int wide, UINT32* dest, int xzoom, int xgrow, int yoffset, int flip, int xpos, int pri )
{
	pgm_state *state = machine->driver_data<pgm_state>();
	int xcnt,xcntdraw;
	int xzoombit;
	int xoffset;
	int xdrawpos = 0;

	xcnt = 0;
	xcntdraw = 0;
	while (xcnt < wide * 16)
	{
		UINT32 srcdat;
		if (!(flip & 0x01))
			xoffset = xcnt;
		else
			xoffset = (wide * 16) - xcnt - 1;

		srcdat = state->sprite_temp_render[yoffset + xoffset];
		xzoombit = (xzoom >> (xcnt & 0x1f)) & 1;

		if (xzoombit == 1 && xgrow == 1)
		{ // double this column
			xdrawpos = xpos + xcntdraw;
			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))
				{
					if (pri)
						dest[xdrawpos] = srcdat | 0x8000 | 0x10000;
					else
						dest[xdrawpos] = srcdat | 0x10000;
				}
			}
			xcntdraw++;

			xdrawpos = xpos + xcntdraw;

			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))
				{
					if (pri)
						dest[xdrawpos] = srcdat | 0x8000 | 0x10000;
					else
						dest[xdrawpos] = srcdat | 0x10000;
				}
			}
			xcntdraw++;
		}
		else if (xzoombit == 1 && xgrow == 0)
		{
			/* skip this column */
		}
		else //normal column
		{
			xdrawpos = xpos + xcntdraw;
			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))
				{
					if (pri)
						dest[xdrawpos] = srcdat | 0x8000 | 0x10000;
					else
						dest[xdrawpos] = srcdat | 0x10000;
				}
			}
			xcntdraw++;
		}

		xcnt++;

		if (xdrawpos == 448) xcnt = wide*16;
	}
}
/* this just loops over our decoded bitmap and puts it on the screen */
static void draw_sprite_new_zoomed( running_machine *machine, int wide, int high, int xpos, int ypos, int palt, int boffset, int flip, bitmap_t* bitmap, UINT32 xzoom, int xgrow, UINT32 yzoom, int ygrow, int pri )
{
	int ycnt;
	int ydrawpos;
	UINT32 *dest;
	int yoffset;
	int ycntdraw;
	int yzoombit;

	pgm_prepare_sprite(machine, wide,high, palt, boffset);

	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;
	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt & 0x1f)) & 1;

		if (yzoombit == 1 && ygrow == 1) // double this line
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip & 0x02))
				yoffset = (ycnt * (wide * 16));
			else
				yoffset = ((high - ycnt - 1) * (wide * 16));

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR32(bitmap, ydrawpos, 0);
				draw_sprite_line(machine, wide, dest, xzoom, xgrow, yoffset, flip, xpos, pri);
			}
			ycntdraw++;

			ydrawpos = ypos + ycntdraw;
			if (!(flip & 0x02))
				yoffset = (ycnt * (wide * 16));
			else
				yoffset = ((high - ycnt - 1) * (wide * 16));

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR32(bitmap, ydrawpos, 0);
				draw_sprite_line(machine, wide, dest, xzoom, xgrow, yoffset, flip, xpos, pri);
			}
			ycntdraw++;

			if (ydrawpos ==224)
				ycnt = high;
		}
		else if (yzoombit == 1 && ygrow == 0)
		{
			/* skip this line */
			/* we should process anyway if we don't do the pre-decode.. */
		}
		else /* normal line */
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip & 0x02))
				yoffset = (ycnt * (wide * 16));
			else
				yoffset = ((high - ycnt - 1) * (wide * 16));

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR32(bitmap, ydrawpos, 0);
				draw_sprite_line(machine, wide, dest, xzoom, xgrow, yoffset, flip, xpos, pri);
			}
			ycntdraw++;

			if (ydrawpos == 224)
				ycnt = high;
		}

		ycnt++;
	}
}


static void draw_sprites( running_machine *machine, bitmap_t* spritebitmap, UINT16 *sprite_source )
{
	/* ZZZZ Zxxx xxxx xxxx
       zzzz z-yy yyyy yyyy
       -ffp pppp Pvvv vvvv
       vvvv vvvv vvvv vvvv
       wwww wwwh hhhh hhhh
    */


	pgm_state *state = machine->driver_data<pgm_state>();
	const UINT16 *finish = state->spritebufferram + (0xa00 / 2);

	while (sprite_source < finish)
	{
		int xpos = sprite_source[0] & 0x07ff;
		int ypos = sprite_source[1] & 0x03ff;
		int xzom = (sprite_source[0] & 0x7800) >> 11;
		int xgrow = (sprite_source[0] & 0x8000) >> 15;
		int yzom = (sprite_source[1] & 0x7800) >> 11;
		int ygrow = (sprite_source[1] & 0x8000) >> 15;
		int palt = (sprite_source[2] & 0x1f00) >> 8;
		int flip = (sprite_source[2] & 0x6000) >> 13;
		int boff = ((sprite_source[2] & 0x007f) << 16) | (sprite_source[3] & 0xffff);
		int wide = (sprite_source[4] & 0x7e00) >> 9;
		int high = sprite_source[4] & 0x01ff;
		int pri = (sprite_source[2] & 0x0080) >>  7;

		UINT32 xzoom, yzoom;

		UINT16* sprite_zoomtable = &state->videoregs[0x1000 / 2];

		if (xgrow)
		{
		//  xzom = 0xf - xzom; // would make more sense but everything gets zoomed slightly in dragon world 2 ?!
			xzom = 0x10 - xzom; // this way it doesn't but there is a bad line when zooming after the level select?
		}

		if (ygrow)
		{
		//  yzom = 0xf - yzom; // see comment above
			yzom = 0x10 - yzom;
		}

		xzoom = (sprite_zoomtable[xzom * 2] << 16) | sprite_zoomtable[xzom * 2 + 1];
		yzoom = (sprite_zoomtable[yzom * 2] << 16) | sprite_zoomtable[yzom * 2 + 1];

		boff *= 2;
		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		if (!sprite_source[4]) break; /* is this right? */

		//if ((priority == 1) && (pri == 0)) break;

		draw_sprite_new_zoomed(machine, wide, high, xpos, ypos, palt, boff, flip, spritebitmap, xzoom, xgrow, yzoom, ygrow, pri);

		sprite_source += 5;
	}
}

/* TX Layer */
WRITE16_HANDLER( pgm_tx_videoram_w )
{
	pgm_state *state = space->machine->driver_data<pgm_state>();
	state->tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap, offset / 2);
}

static TILE_GET_INFO( get_pgm_tx_tilemap_tile_info )
{

/* 0x904000 - 0x90ffff is the Text Overlay Ram (pgm_tx_videoram)
    each tile uses 4 bytes, the tilemap is 64x128?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA017FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	pgm_state *state = machine->driver_data<pgm_state>();
	int tileno, colour, flipyx; //,game;

	tileno = state->tx_videoram[tile_index * 2] & 0xffff;
	colour = (state->tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (state->tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

WRITE16_HANDLER( pgm_bg_videoram_w )
{
	pgm_state *state = space->machine->driver_data<pgm_state>();
	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_pgm_bg_tilemap_tile_info )
{
	/* pretty much the same as tx layer */

	pgm_state *state = machine->driver_data<pgm_state>();
	int tileno, colour, flipyx;

	tileno = state->bg_videoram[tile_index *2] & 0xffff;

	colour = (state->bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (state->bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

VIDEO_START( pgm )
{
	pgm_state *state = machine->driver_data<pgm_state>();
	int i;

	state->tx_tilemap = tilemap_create(machine, get_pgm_tx_tilemap_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	tilemap_set_transparent_pen(state->tx_tilemap, 15);

	state->bg_tilemap = tilemap_create(machine, get_pgm_bg_tilemap_tile_info, tilemap_scan_rows, 32, 32, 64, 16);
	tilemap_set_transparent_pen(state->bg_tilemap, 31);
	tilemap_set_scroll_rows(state->bg_tilemap, 16 * 32);

	state->tmppgmbitmap = auto_bitmap_alloc(machine, 448, 224, BITMAP_FORMAT_RGB32);

	for (i = 0; i < 0x1200 / 2; i++)
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));

	state->spritebufferram = auto_alloc_array(machine, UINT16, 0xa00/2);

	/* we render each sprite to a bitmap then copy the bitmap to screen bitmap with zooming */
	/* easier this way because of the funky sprite format */
	state->sprite_temp_render = auto_alloc_array(machine, UINT16, 0x400*0x200);

	state_save_register_global_pointer(machine, state->spritebufferram, 0xa00/2);
	state_save_register_global_pointer(machine, state->sprite_temp_render, 0x400*0x200);
	state_save_register_global_bitmap(machine, state->tmppgmbitmap);
}

VIDEO_UPDATE( pgm )
{
	pgm_state *state = screen->machine->driver_data<pgm_state>();
	int y;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(state->tmppgmbitmap, cliprect, 0x00000000);

	draw_sprites(screen->machine, state->tmppgmbitmap, state->spritebufferram);

	tilemap_set_scrolly(state->bg_tilemap,0, state->videoregs[0x2000/2]);

	for (y = 0; y < 224; y++)
		tilemap_set_scrollx(state->bg_tilemap, (y + state->videoregs[0x2000 / 2]) & 0x1ff, state->videoregs[0x3000 / 2] + state->rowscrollram[y]);

	{
		int y, x;

		for (y = 0; y < 224; y++)
		{
			UINT32* src = BITMAP_ADDR32(state->tmppgmbitmap, y, 0);
			UINT16* dst = BITMAP_ADDR16(bitmap, y, 0);

			for (x = 0; x < 448; x++)
			{
				if (src[x] & 0x10000)
					if ((src[x] & 0x8000) == 0x8000)
						dst[x] = src[x] & 0x7fff;
			}
		}
	}

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	{
		int y, x;

		for (y = 0; y < 224; y++)
		{
			UINT32* src = BITMAP_ADDR32(state->tmppgmbitmap, y, 0);
			UINT16* dst = BITMAP_ADDR16(bitmap, y, 0);

			for (x = 0; x < 448; x++)
			{
				if (src[x] & 0x10000)
					if ((src[x] & 0x8000) == 0x0000)
						dst[x] = src[x];
			}
		}
	}

	tilemap_set_scrolly(state->tx_tilemap, 0, state->videoregs[0x5000/2]);
	tilemap_set_scrollx(state->tx_tilemap, 0, state->videoregs[0x6000/2]); // Check
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( pgm )
{
	pgm_state *state = machine->driver_data<pgm_state>();

	/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
	memcpy(state->spritebufferram, pgm_mainram, 0xa00);
}
