#include "driver.h"
#include "audio/m72.h"
#include "m72.h"



UINT16 *m72_videoram1,*m72_videoram2,*majtitle_rowscrollram;
UINT32 m72_raster_irq_position;
static UINT16 *m72_spriteram;
static tilemap *fg_tilemap,*bg_tilemap;
static INT32 scrollx1,scrolly1,scrollx2,scrolly2;
static INT32 video_off;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void m72_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,const UINT16 *vram,int gfxnum)
{
	int code,attr,color,pri;

	tile_index *= 2;

	code  = vram[tile_index] & 0xff;
	attr  = vram[tile_index] >> 8;
	color = vram[tile_index+1] & 0xff;

	if (color & 0x80) pri = 2;
	else if (color & 0x40) pri = 1;
	else pri = 0;
/* color & 0x10 is used in bchopper and hharry, more priority? */

	SET_TILE_INFO(
			gfxnum,
			code + ((attr & 0x3f) << 8),
			color & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
	tileinfo->group = pri;
}

INLINE void rtype2_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,const UINT16 *vram,int gfxnum)
{
	int code,attr,color,pri;

	tile_index *= 2;

	code  = vram[tile_index];
	color = vram[tile_index+1] & 0xff;
	attr  = vram[tile_index+1] >> 8;

	if (attr & 0x01) pri = 2;
	else if (color & 0x80) pri = 1;
	else pri = 0;

/* (vram[tile_index+2] & 0x10) is used by majtitle on the green, but it's not clear for what */
/* (vram[tile_index+3] & 0xfe) are used as well */

	SET_TILE_INFO(
			gfxnum,
			code,
			color & 0x0f,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo->group = pri;
}


static TILE_GET_INFO( m72_get_bg_tile_info )
{
	m72_get_tile_info(machine,tileinfo,tile_index,m72_videoram2,2);
}

static TILE_GET_INFO( m72_get_fg_tile_info )
{
	m72_get_tile_info(machine,tileinfo,tile_index,m72_videoram1,1);
}

static TILE_GET_INFO( hharry_get_bg_tile_info )
{
	m72_get_tile_info(machine,tileinfo,tile_index,m72_videoram2,1);
}

static TILE_GET_INFO( rtype2_get_bg_tile_info )
{
	rtype2_get_tile_info(machine,tileinfo,tile_index,m72_videoram2,1);
}

static TILE_GET_INFO( rtype2_get_fg_tile_info )
{
	rtype2_get_tile_info(machine,tileinfo,tile_index,m72_videoram1,1);
}


static TILEMAP_MAPPER( majtitle_scan_rows )
{
	/* logical (col,row) -> memory offset */
	return row*256 + col;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void register_savestate(running_machine *machine)
{
	state_save_register_global(machine, m72_raster_irq_position);
	state_save_register_global(machine, video_off);
	state_save_register_global(machine, scrollx1);
	state_save_register_global(machine, scrolly1);
	state_save_register_global(machine, scrollx2);
	state_save_register_global(machine, scrolly2);
	state_save_register_global_pointer(machine, m72_spriteram, spriteram_size/2);
}


VIDEO_START( m72 )
{
	bg_tilemap = tilemap_create(machine, m72_get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	fg_tilemap = tilemap_create(machine, m72_get_fg_tile_info,tilemap_scan_rows,8,8,64,64);

	m72_spriteram = auto_alloc_array(machine, UINT16, spriteram_size/2);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	//tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);
	tilemap_set_transmask(bg_tilemap,2,0x0007,0xfff8);

	memset(m72_spriteram,0,spriteram_size);

	tilemap_set_scrolldx(fg_tilemap,0,0);
	tilemap_set_scrolldy(fg_tilemap,-128,16);

	tilemap_set_scrolldx(bg_tilemap,0,0);
	tilemap_set_scrolldy(bg_tilemap,-128,16);

	register_savestate(machine);
}

VIDEO_START( rtype2 )
{
	bg_tilemap = tilemap_create(machine, rtype2_get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	fg_tilemap = tilemap_create(machine, rtype2_get_fg_tile_info,tilemap_scan_rows,8,8,64,64);

	m72_spriteram = auto_alloc_array(machine, UINT16, spriteram_size/2);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	tilemap_set_scrolldx(fg_tilemap,4,0);
	tilemap_set_scrolldy(fg_tilemap,-128,16);

	tilemap_set_scrolldx(bg_tilemap,4,0);
	tilemap_set_scrolldy(bg_tilemap,-128,16);

	register_savestate(machine);
}

VIDEO_START( poundfor )
{
	VIDEO_START_CALL(rtype2);

	tilemap_set_scrolldx(fg_tilemap,6,0);
	tilemap_set_scrolldx(bg_tilemap,6,0);
}


/* Major Title has a larger background RAM, and rowscroll */
VIDEO_START( majtitle )
{
// The tilemap can be 256x64, but seems to be used at 128x64 (scroll wraparound).
// The layout ramains 256x64, the right half is just not displayed.
//  bg_tilemap = tilemap_create(machine, rtype2_get_bg_tile_info,tilemap_scan_rows,8,8,256,64);
	bg_tilemap = tilemap_create(machine, rtype2_get_bg_tile_info,majtitle_scan_rows,8,8,128,64);
	fg_tilemap = tilemap_create(machine, rtype2_get_fg_tile_info,tilemap_scan_rows,8,8,64,64);

	m72_spriteram = auto_alloc_array(machine, UINT16, spriteram_size/2);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	tilemap_set_scrolldx(fg_tilemap,4,0);
	tilemap_set_scrolldy(fg_tilemap,-128,16);

	tilemap_set_scrolldx(bg_tilemap,4,0);
	tilemap_set_scrolldy(bg_tilemap,-128,16);

	register_savestate(machine);
}

VIDEO_START( hharry )
{
	bg_tilemap = tilemap_create(machine, hharry_get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	fg_tilemap = tilemap_create(machine, m72_get_fg_tile_info,   tilemap_scan_rows,8,8,64,64);

	m72_spriteram = auto_alloc_array(machine, UINT16, spriteram_size/2);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	tilemap_set_scrolldx(fg_tilemap,4,0);
	tilemap_set_scrolldy(fg_tilemap,-128,16);

	tilemap_set_scrolldx(bg_tilemap,6,0);
	tilemap_set_scrolldy(bg_tilemap,-128,16);

	register_savestate(machine);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( m72_palette1_r )
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	return paletteram16[offset] | 0xffe0;	/* only D0-D4 are connected */
}

READ16_HANDLER( m72_palette2_r )
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	return paletteram16_2[offset] | 0xffe0;	/* only D0-D4 are connected */
}

INLINE void changecolor(running_machine *machine,int color,int r,int g,int b)
{
	palette_set_color_rgb(machine,color,pal5bit(r),pal5bit(g),pal5bit(b));
}

WRITE16_HANDLER( m72_palette1_w )
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&paletteram16[offset]);
	offset &= 0x0ff;
	changecolor(space->machine,
			offset,
			paletteram16[offset + 0x000],
			paletteram16[offset + 0x200],
			paletteram16[offset + 0x400]);
}

WRITE16_HANDLER( m72_palette2_w )
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&paletteram16_2[offset]);
	offset &= 0x0ff;
	changecolor(space->machine,
			offset + 256,
			paletteram16_2[offset + 0x000],
			paletteram16_2[offset + 0x200],
			paletteram16_2[offset + 0x400]);
}

WRITE16_HANDLER( m72_videoram1_w )
{
	COMBINE_DATA(&m72_videoram1[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset/2);
}

WRITE16_HANDLER( m72_videoram2_w )
{
	COMBINE_DATA(&m72_videoram2[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}

WRITE16_HANDLER( m72_irq_line_w )
{
	COMBINE_DATA(&m72_raster_irq_position);
}

WRITE16_HANDLER( m72_scrollx1_w )
{
	COMBINE_DATA(&scrollx1);
}

WRITE16_HANDLER( m72_scrollx2_w )
{
	COMBINE_DATA(&scrollx2);
}

WRITE16_HANDLER( m72_scrolly1_w )
{
	COMBINE_DATA(&scrolly1);
}

WRITE16_HANDLER( m72_scrolly2_w )
{
	COMBINE_DATA(&scrolly2);
}

WRITE16_HANDLER( m72_dmaon_w )
{
	if (ACCESSING_BITS_0_7)
		memcpy(m72_spriteram,spriteram16,spriteram_size);
}


WRITE16_HANDLER( m72_port02_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(space->machine, ((data & 0x04) >> 2) ^ ((~input_port_read(space->machine, "DSW") >> 8) & 1));

		/* bit 3 is display disable */
		video_off = data & 0x08;

		/* bit 4 resets sound CPU (active low) */
		if (data & 0x10)
			cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_RESET, CLEAR_LINE);
		else
			cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE);

		/* bit 5 = "bank"? */
	}
}

WRITE16_HANDLER( rtype2_port02_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(space->machine, ((data & 0x04) >> 2) ^ ((~input_port_read(space->machine, "DSW") >> 8) & 1));

		/* bit 3 is display disable */
		video_off = data & 0x08;

		/* other bits unknown */
	}
}


static int majtitle_rowscroll;

/* the following is mostly a kludge. This register seems to be used for something else */
WRITE16_HANDLER( majtitle_gfx_ctrl_w )
{
	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00) majtitle_rowscroll = 1;
		else majtitle_rowscroll = 0;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void m72_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;

	offs = 0;
	while (offs < spriteram_size/2)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = m72_spriteram[offs+1];
		color = m72_spriteram[offs+2] & 0x0f;
		sx = -256+(m72_spriteram[offs+3] & 0x3ff);
		sy = 384-(m72_spriteram[offs+0] & 0x1ff);
		flipx = m72_spriteram[offs+2] & 0x0800;
		flipy = m72_spriteram[offs+2] & 0x0400;

		w = 1 << ((m72_spriteram[offs+2] & 0xc000) >> 14);
		h = 1 << ((m72_spriteram[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen_get(machine))
		{
			sx = 512 - 16*w - sx;
			sy = 284 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (x = 0;x < w;x++)
		{
			for (y = 0;y < h;y++)
			{
				int c = code;

				if (flipx) c += 8*(w-1-x);
				else c += 8*x;
				if (flipy) c += h-1-y;
				else c += y;

				drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,0);
			}
		}

		offs += w*4;
	}
}

static void majtitle_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = spriteram16_2[offs+1];
		color = spriteram16_2[offs+2] & 0x0f;
		sx = -256+(spriteram16_2[offs+3] & 0x3ff);
		sy = 384-(spriteram16_2[offs+0] & 0x1ff);
		flipx = spriteram16_2[offs+2] & 0x0800;
		flipy = spriteram16_2[offs+2] & 0x0400;

		w = 1;// << ((spriteram16_2[offs+2] & 0xc000) >> 14);
		h = 1 << ((spriteram16_2[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen_get(machine))
		{
			sx = 512 - 16*w - sx;
			sy = 256 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (x = 0;x < w;x++)
		{
			for (y = 0;y < h;y++)
			{
				int c = code;

				if (flipx) c += 8*(w-1-x);
				else c += 8*x;
				if (flipy) c += h-1-y;
				else c += y;

				drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,0);
			}
		}
	}
}

VIDEO_UPDATE( m72 )
{
	if (video_off)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
		return 0;
	}

	tilemap_set_scrollx(fg_tilemap,0,scrollx1);
	tilemap_set_scrolly(fg_tilemap,0,scrolly1);

	tilemap_set_scrollx(bg_tilemap,0,scrollx2);
	tilemap_set_scrolly(bg_tilemap,0,scrolly2);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER1,0);
	m72_draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}

VIDEO_UPDATE( majtitle )
{
	int i;


	if (video_off)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
		return 0;
	}

	tilemap_set_scrollx(fg_tilemap,0,scrollx1);
	tilemap_set_scrolly(fg_tilemap,0,scrolly1);

	if (majtitle_rowscroll)
	{
		tilemap_set_scroll_rows(bg_tilemap,512);
		for (i = 0;i < 512;i++)
			tilemap_set_scrollx(bg_tilemap,(i+scrolly2)&0x1ff,
					256 + majtitle_rowscrollram[i]);
	}
	else
	{
		tilemap_set_scroll_rows(bg_tilemap,1);
		tilemap_set_scrollx(bg_tilemap,0,256 + scrollx2);
	}
	tilemap_set_scrolly(bg_tilemap,0,scrolly2);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER1,0);
	majtitle_draw_sprites(screen->machine, bitmap,cliprect);
	m72_draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}
