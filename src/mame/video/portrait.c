/***************************************************************************

  Portraits
  video hardware emulation

***************************************************************************/

#include "driver.h"

int portrait_scroll;
UINT8 *portrait_bgvideoram, *portrait_fgvideoram;
static tilemap_t *foreground, *background;

WRITE8_HANDLER( portrait_bgvideo_write )
{
	tilemap_mark_tile_dirty(background,offset/2);
	portrait_bgvideoram[offset] = data;
}

WRITE8_HANDLER( portrait_fgvideo_write )
{
	tilemap_mark_tile_dirty(foreground,offset/2);
	portrait_fgvideoram[offset] = data;
}

INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, const UINT8 *source )
{
	int attr    = source[tile_index*2+0];
	int tilenum = source[tile_index*2+1];
	int flags   = 0;
	int color   = 0;

	/* or 0x10 ? */
	if( attr & 0x20 ) flags = TILE_FLIPY;

	switch( attr & 7 )
	{
		case 1:
			tilenum += 0x200;
			break;
		case 3:
			tilenum += 0x300;
			break;
		case 5:
			tilenum += 0x100;
			break;
	}

	if (tilenum<0x100)
		color = ((tilenum&0xff)>>1)+0x00;
	else
		color = ((tilenum&0xff)>>1)+0x80;

	SET_TILE_INFO( 0, tilenum, color, flags );
}

static TILE_GET_INFO( get_bg_tile_info )
{
	get_tile_info( machine, tileinfo, tile_index, portrait_bgvideoram );
}

static TILE_GET_INFO( get_fg_tile_info )
{
	get_tile_info( machine, tileinfo, tile_index, portrait_fgvideoram );
}

VIDEO_START( portrait )
{
	background = tilemap_create( machine, get_bg_tile_info, tilemap_scan_rows,       16, 16, 32, 32 );
	foreground = tilemap_create( machine, get_fg_tile_info, tilemap_scan_rows,  16, 16, 32, 32 );

	tilemap_set_transparent_pen( foreground, 7 );
}



PALETTE_INIT( portrait )
{
	int i;
	UINT8* lookup = memory_region(machine,"tileattr");

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x40);

/*
    for (i = 0;i < 0x40;i++)
    {
        int r,g,b,data;
        data = color_prom[0];


        r = (data >> 0) & 0x7;
        g = (data >> 3) & 0x3;
        b = (data >> 5) & 0x7;

        colortable_palette_set_color(machine->colortable, i, MAKE_RGB(pal3bit(r), pal2bit(g), pal3bit(b)));

        color_prom++;
    }
*/

	for (i=0;i<0x20;i++)
	{
		int r,g,b,data;
		data = (color_prom[0]<<0) | (color_prom[0x20]<<8);

		r = (data >> 0) & 0x1f;
		g = (data >> 5) & 0x1f;
		b = (data >> 10) & 0x1f;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(pal5bit(r), pal5bit(g), pal5bit(b)));

		// ?? the lookup seems to reference 0x3f colours, unless 1 bit is priority or similar?
		colortable_palette_set_color(machine->colortable, i+0x20, MAKE_RGB(pal5bit(r>>1), pal5bit(g>>1), pal5bit(b>>1)));

		color_prom++;
	}



	for (i = 0;i < 0x800;i++)
	{
		UINT8 ctabentry = lookup[i]&0x3f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *source = machine->generic.spriteram.u8;
	UINT8 *finish = source + 0x200;

	while( source < finish )
	{
		int sy      = source[0];
		int sx      = source[1];
		int attr    = source[2];
			/* xx-x---- ?
             * --x----- flipy
             * ----x--- msb source[0]
             * -----x-- msb source[1]
             */
		int tilenum = source[3];

		int color = ((tilenum&0xff)>>1)+0x00;

		int fy = attr & 0x20;

		if(attr & 0x04) sx |= 0x100;

		if(attr & 0x08) sy |= 0x100;

		sx += (source - machine->generic.spriteram.u8) - 8;
		sx &= 0x1ff;

		sy = (512 - 64) - sy;

		/* wrong! */
		switch( attr & 0xc0 )
		{
		case 0:
			break;

		case 0x40:
			sy -= portrait_scroll;
			break;

		case 0x80:
			sy -= portrait_scroll;
			break;

		case 0xc0:
			break;

		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				tilenum,color,
				0,fy,
				sx,sy,7);

		source += 0x10;
	}
}

VIDEO_UPDATE( portrait )
{
	rectangle cliprect_scroll, cliprect_no_scroll;

	cliprect_scroll = cliprect_no_scroll = *cliprect;

	cliprect_no_scroll.min_x = cliprect_no_scroll.max_x - 111;
	cliprect_scroll.max_x    = cliprect_scroll.min_x    + 319;

	tilemap_set_scrolly(background, 0, 0);
	tilemap_set_scrolly(foreground, 0, 0);
	tilemap_draw(bitmap, &cliprect_no_scroll, background, 0, 0);
	tilemap_draw(bitmap, &cliprect_no_scroll, foreground, 0, 0);

	tilemap_set_scrolly(background, 0, portrait_scroll);
	tilemap_set_scrolly(foreground, 0, portrait_scroll);
	tilemap_draw(bitmap, &cliprect_scroll, background, 0, 0);
	tilemap_draw(bitmap, &cliprect_scroll, foreground, 0, 0);

	draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}
