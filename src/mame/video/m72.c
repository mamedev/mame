#include "driver.h"
#include "audio/m72.h"
#include "m72.h"



UINT16 *m72_videoram1,*m72_videoram2,*majtitle_rowscrollram;
static UINT16 *m72_spriteram;
static INT32 splitline;
static tilemap *fg_tilemap,*bg_tilemap;
static int xadjust;
static int bgadjust;
static INT32 scrollx1,scrolly1,scrollx2,scrolly2;
static INT32 video_off;

static int irqbase;

MACHINE_RESET( m72 )
{
	irqbase = 0x20;
	machine_reset_m72_sound(machine);
}

MACHINE_RESET( xmultipl )
{
	irqbase = 0x08;
	machine_reset_m72_sound(machine);
}

MACHINE_RESET( kengo )
{
	irqbase = 0x18;
	machine_reset_m72_sound(machine);
}

INTERRUPT_GEN( m72_interrupt )
{
	int line = 255 - cpu_getiloops();

	if (line == 255)	/* vblank */
	{
		cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, irqbase+0);
	}
	else
	{
		if (line != splitline - 128)
			return;

		video_screen_update_partial(0, line + 128);

		/* this is used to do a raster effect and show the score display at
           the bottom of the screen or other things. The line where the
           interrupt happens is programmable (and the interrupt can be triggered
           multiple times, by changing the interrupt line register in the
           interrupt handler).
         */
		cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, irqbase+2);
	}
}



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

static void register_savestate(void)
{
	state_save_register_global(splitline);
	state_save_register_global(video_off);
	state_save_register_global(scrollx1);
	state_save_register_global(scrolly1);
	state_save_register_global(scrollx2);
	state_save_register_global(scrolly2);
	state_save_register_global_pointer(m72_spriteram, spriteram_size);
}


VIDEO_START( m72 )
{
	bg_tilemap = tilemap_create(m72_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
	fg_tilemap = tilemap_create(m72_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);

	m72_spriteram = auto_malloc(spriteram_size);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	xadjust = 0;
	bgadjust = 0;

	register_savestate();
}

VIDEO_START( rtype2 )
{
	bg_tilemap = tilemap_create(rtype2_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
	fg_tilemap = tilemap_create(rtype2_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);

	m72_spriteram = auto_malloc(spriteram_size);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	xadjust = -4;
	bgadjust = 0;

	register_savestate();
}

VIDEO_START( poundfor )
{
	video_start_rtype2(machine);

	xadjust = -6;
}


/* Major Title has a larger background RAM, and rowscroll */
VIDEO_START( majtitle )
{
// The tilemap can be 256x64, but seems to be used at 128x64 (scroll wraparound).
// The layout ramains 256x64, the right half is just not displayed.
//  bg_tilemap = tilemap_create(rtype2_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,256,64);
	bg_tilemap = tilemap_create(rtype2_get_bg_tile_info,majtitle_scan_rows,TILEMAP_TYPE_PEN,8,8,128,64);
	fg_tilemap = tilemap_create(rtype2_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);

	m72_spriteram = auto_malloc(spriteram_size);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	xadjust = -4;
	bgadjust = 0;

	register_savestate();
}

VIDEO_START( hharry )
{
	bg_tilemap = tilemap_create(hharry_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
	fg_tilemap = tilemap_create(m72_get_fg_tile_info,   tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);

	m72_spriteram = auto_malloc(spriteram_size);

	tilemap_set_transmask(fg_tilemap,0,0xffff,0x0001);
	tilemap_set_transmask(fg_tilemap,1,0x00ff,0xff01);
	tilemap_set_transmask(fg_tilemap,2,0x0001,0xffff);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000);
	tilemap_set_transmask(bg_tilemap,1,0x00ff,0xff00);
	tilemap_set_transmask(bg_tilemap,2,0x0001,0xfffe);

	memset(m72_spriteram,0,spriteram_size);

	xadjust = -4;
	bgadjust = -2;

	register_savestate();
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

INLINE void changecolor(int color,int r,int g,int b)
{
	palette_set_color_rgb(Machine,color,pal5bit(r),pal5bit(g),pal5bit(b));
}

WRITE16_HANDLER( m72_palette1_w )
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&paletteram16[offset]);
	offset &= 0x0ff;
	changecolor(offset,
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
	changecolor(offset + 256,
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
	COMBINE_DATA(&splitline);
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
	if (ACCESSING_LSB)
		memcpy(m72_spriteram,spriteram16,spriteram_size);
}


WRITE16_HANDLER( m72_port02_w )
{
	if (ACCESSING_LSB)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(((data & 0x04) >> 2) ^ ((~readinputport(2) >> 8) & 1));

		/* bit 3 is display disable */
		video_off = data & 0x08;

		/* bit 4 resets sound CPU (active low) */
		if (data & 0x10)
			cpunum_set_input_line(1, INPUT_LINE_RESET, CLEAR_LINE);
		else
			cpunum_set_input_line(1, INPUT_LINE_RESET, ASSERT_LINE);

		/* bit 5 = "bank"? */
	}
}

WRITE16_HANDLER( rtype2_port02_w )
{
	if (ACCESSING_LSB)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(((data & 0x04) >> 2) ^ ((~readinputport(2) >> 8) & 1));

		/* bit 3 is display disable */
		video_off = data & 0x08;

		/* other bits unknown */
	}
}


static int majtitle_rowscroll;

/* the following is mostly a kludge. This register seems to be used for something else */
WRITE16_HANDLER( majtitle_gfx_ctrl_w )
{
	if (ACCESSING_MSB)
	{
		if (data & 0xff00) majtitle_rowscroll = 1;
		else majtitle_rowscroll = 0;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void m72_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	offs = 0;
	while (offs < spriteram_size/2)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = m72_spriteram[offs+1];
		color = m72_spriteram[offs+2] & 0x0f;
		sx = -256+(m72_spriteram[offs+3] & 0x3ff);
		sy = 512-(m72_spriteram[offs+0] & 0x1ff);
		flipx = m72_spriteram[offs+2] & 0x0800;
		flipy = m72_spriteram[offs+2] & 0x0400;

		w = 1 << ((m72_spriteram[offs+2] & 0xc000) >> 14);
		h = 1 << ((m72_spriteram[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen)
		{
			sx = 512 - 16*w - sx;
			sy = 512 - 16*h - sy;
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

				drawgfx(bitmap,machine->gfx[0],
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}

		offs += w*4;
	}
}

static void majtitle_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = spriteram16_2[offs+1];
		color = spriteram16_2[offs+2] & 0x0f;
		sx = -256+(spriteram16_2[offs+3] & 0x3ff);
		sy = 512-(spriteram16_2[offs+0] & 0x1ff);
		flipx = spriteram16_2[offs+2] & 0x0800;
		flipy = spriteram16_2[offs+2] & 0x0400;

		w = 1;// << ((spriteram16_2[offs+2] & 0xc000) >> 14);
		h = 1 << ((spriteram16_2[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen)
		{
			sx = 512 - 16*w - sx;
			sy = 512 - 16*h - sy;
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

				drawgfx(bitmap,machine->gfx[2],
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

VIDEO_UPDATE( m72 )
{
	if (video_off)
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
		return 0;
	}

	tilemap_set_scrollx(fg_tilemap,0,scrollx1 + xadjust);
	tilemap_set_scrolly(fg_tilemap,0,scrolly1);

	tilemap_set_scrollx(bg_tilemap,0,scrollx2 + xadjust + bgadjust);
	tilemap_set_scrolly(bg_tilemap,0,scrolly2);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER1,0);
	m72_draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}

VIDEO_UPDATE( majtitle )
{
	int i;


	if (video_off)
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
		return 0;
	}

	tilemap_set_scrollx(fg_tilemap,0,scrollx1 + xadjust);
	tilemap_set_scrolly(fg_tilemap,0,scrolly1);

	if (majtitle_rowscroll)
	{
		tilemap_set_scroll_rows(bg_tilemap,512);
		for (i = 0;i < 512;i++)
			tilemap_set_scrollx(bg_tilemap,(i+scrolly2)&0x1ff,
					256 + majtitle_rowscrollram[i] + xadjust);
	}
	else
	{
		tilemap_set_scroll_rows(bg_tilemap,1);
		tilemap_set_scrollx(bg_tilemap,0,256 + scrollx2 + xadjust);
	}
	tilemap_set_scrolly(bg_tilemap,0,scrolly2);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER1,0);
	majtitle_draw_sprites(machine, bitmap,cliprect);
	m72_draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}
