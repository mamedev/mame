/***************************************************************

 Pro Yakyuu Nyuudan Test Tryout (JPN Ver.)
 video hardware emulation

****************************************************************/

#include "driver.h"

static tilemap *fg_tilemap, *bg_tilemap;
static UINT8 vram_bank;
static UINT8 *tryout_vram, *tryout_vram_gfx;
UINT8 *tryout_gfx_control;

PALETTE_INIT( tryout )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, attr;

	code = videoram[tile_index];
	attr = videoram[tile_index + 0x400];

	code |= ((attr & 0x03) << 8);

	SET_TILE_INFO(0, code, 6, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	SET_TILE_INFO(2, tryout_vram[tile_index] & 0x7f, 0, 0);
}

READ8_HANDLER( tryout_vram_r )
{
	return tryout_vram[offset]; // debug only
}

WRITE8_HANDLER( tryout_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( tryout_vram_w )
{
	/*  There are eight banks of vram - in bank 0 the first 0x400 bytes
    is reserved for the tilemap.  In banks 2, 4 and 6 the game never
    writes to the first 0x400 bytes - I suspect it's either
    unused, or it actually mirrors the tilemap ram from the first bank.

    The rest of the vram is tile data which has the bitplanes arranged
    in a very strange format.  For Mame's sake we reformat this on
    the fly for easier gfx decode.

    Bit 0 of the bank register seems special - it's kept low when uploading
    gfx data and then set high from that point onwards.

    */
	const UINT8 bank=(vram_bank>>1)&0x7;


	if ((bank==0 || bank==2 || bank==4 || bank==6) && (offset&0x7ff)<0x400) {
		int newoff=offset&0x3ff;

		switch (newoff&0x300){
		case 0x000: newoff=(newoff&0xff) | 0x100; break;
		case 0x100: newoff=(newoff&0xff) | 0x200; break;
		case 0x200: newoff=(newoff&0xff) | 0x000; break;
		case 0x300: newoff=(newoff&0xff) | 0x300; break;
		}

		tryout_vram[newoff]=data;
		tilemap_mark_tile_dirty(bg_tilemap,newoff);
		return;
	}

	/*
        Bit planes for tiles are arranged as follows within vram (split into high/low nibbles):
            0x0400 (0) + 0x0400 (4) + 0x0800(0) - tiles 0x00 to 0x0f
            0x0800 (4) + 0x0c00 (0) + 0x0c00(4) - tiles 0x10 to 0x1f
            0x1400 (0) + 0x1400 (4) + 0x1800(0) - tiles 0x20 to 0x2f
            0x1800 (4) + 0x1c00 (0) + 0x1c00(4) - tiles 0x30 to 0x3f
            etc.
    */

	offset=(offset&0x7ff) | (bank<<11);
	tryout_vram[offset]=data;

	switch (offset&0x1c00) {
	case 0x0400:
		tryout_vram_gfx[(offset&0x3ff) + 0x0000 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x2000 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	case 0x0800:
		tryout_vram_gfx[(offset&0x3ff) + 0x4000 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x4400 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	case 0x0c00:
		tryout_vram_gfx[(offset&0x3ff) + 0x0400 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x2400 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	case 0x1400:
		tryout_vram_gfx[(offset&0x3ff) + 0x0800 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x2800 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	case 0x1800:
		tryout_vram_gfx[(offset&0x3ff) + 0x4800 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x4c00 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	case 0x1c00:
		tryout_vram_gfx[(offset&0x3ff) + 0x0c00 + ((offset&0x2000)>>1)]=(data&0xf);
		tryout_vram_gfx[(offset&0x3ff) + 0x2c00 + ((offset&0x2000)>>1)]=(data&0xf0)>>4;
		break;
	}

	decodechar(Machine->gfx[2],(offset-0x400/64)&0x7f,tryout_vram_gfx,Machine->drv->gfxdecodeinfo[2].gfxlayout);
	tilemap_mark_all_tiles_dirty(bg_tilemap);
}

WRITE8_HANDLER( tryout_vram_bankswitch_w )
{
	vram_bank = data;
}

WRITE8_HANDLER( tryout_flipscreen_w )
{
	flip_screen_set(data & 1);
}

static TILEMAP_MAPPER( get_fg_memory_offset )
{
	return (row ^ 0x1f) + (col << 5);
}

static TILEMAP_MAPPER( get_bg_memory_offset )
{
	int a;
//  if (col&0x20)
//      a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  0x10 - (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);
//  else
		a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);

//  mame_printf_debug("%d %d -> %d\n",col,row, a);
	return a;
}

VIDEO_START( tryout )
{
	fg_tilemap = tilemap_create(get_fg_tile_info,get_fg_memory_offset,TILEMAP_TYPE_PEN,8,8,32,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,get_bg_memory_offset,TILEMAP_TYPE_PEN,16,16,64,16);

	tryout_vram=auto_malloc(8 * 0x800);
	tryout_vram_gfx=auto_malloc(0x6000);

	tilemap_set_transparent_pen(fg_tilemap,0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs,fx,fy,x,y,color,sprite,inc;

	for (offs = 0;offs < 0x7f;offs += 4)
	{
		if (!(spriteram[offs]&1))
			continue;

		sprite = spriteram[offs+1] + ((spriteram_2[offs]&7)<<8);
		x = spriteram[offs+3]-3;
		y = spriteram[offs+2];
		color = (spriteram[offs] & 8)>>3;
		fx = 0;
		fy = 0;
		inc = 16;

		if (flip_screen)
		{
			x = 240 - x;
			fx = !fx;

			y = 240 - y;
			fy = !fy;

			inc = -inc;
		}

		/* Double Height */
		if(spriteram[offs] & 0x10)
		{
			drawgfx(bitmap,machine->gfx[1],
				sprite,
				color,fx,fy,x,y + inc,
				cliprect,TRANSPARENCY_PEN,0);

			drawgfx(bitmap,machine->gfx[1],
				sprite+1,
				color,fx,fy,x,y,
				cliprect,TRANSPARENCY_PEN,0);
		}
		else
		{
			drawgfx(bitmap,machine->gfx[1],
				sprite,
				color,fx,fy,x,y,
				cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( tryout )
{
//  UINT8* mem=memory_region(REGION_CPU1);

	int scrollx;

	if (!flip_screen)
		tilemap_set_scrollx(fg_tilemap, 0, 16); /* Assumed hard-wired */
	else
		tilemap_set_scrollx(fg_tilemap, 0, -8); /* Assumed hard-wired */

	if(tryout_gfx_control[0] == 0xc)
		scrollx = tryout_gfx_control[1];
	if(tryout_gfx_control[0] == 0x8 && !tryout_gfx_control[1])
		scrollx = 0x100;
	else
		scrollx = tryout_gfx_control[1] | ((tryout_gfx_control[0]&1)<<8) | ((tryout_gfx_control[0]&4)<<7);

	tilemap_set_scrollx(bg_tilemap, 0, scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, -tryout_gfx_control[2]);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect);

//  popmessage("%02x %02x %02x - %04x",mem[0xe402],mem[0xe403],mem[0xe404], ((tryout_gfx_control[0]&1)<<8) | ((tryout_gfx_control[0]&4)<<7));
	return 0;
}
