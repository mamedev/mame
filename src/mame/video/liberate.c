/*******************************************************************************

    Todo:
        There are some kind of resistors hooked up to the background colours,
        the same prom colours can change for the background but not the
        foreground.  It's rarely used (Liberation title screen only?).

    Emulation by Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"

static int background_color,background_disable;
static tilemap *background_tilemap, *fix_tilemap;
static UINT8 deco16_io_ram[16];

#if 0
void debug_print(mame_bitmap *bitmap)
{
	int i,j;
	char buf[20*16];
	char *bufptr = buf;
	for (i = 0;i < 16;i+=2)
		bufptr += sprintf(bufptr,"%04X",deco16_io_ram[i+1]|(deco16_io_ram[i]<<8));
	ui_draw_text(buf, 10, 6*6);
}
#endif

static TILEMAP_MAPPER( back_scan )
{
	/* logical (col,row) -> memory offset */
	return ((row & 0xf)) + ((15-(col &0xf))<<4) + ((row&0x10)<<5) + ((col&0x10)<<4);
}

static TILEMAP_MAPPER( fix_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((31-(col &0x1f))<<5);
}

static TILE_GET_INFO( get_back_tile_info )
{
	const UINT8 *RAM = memory_region(REGION_USER1);
	int tile,bank;

	/* Convert tile index of 512x512 to paged format */
	if (tile_index&0x100) {
		if (tile_index&0x200)
			tile_index=(tile_index&0xff)+(deco16_io_ram[5]<<8); /* Bottom right */
		else
			tile_index=(tile_index&0xff)+(deco16_io_ram[4]<<8); /* Bottom left */
	} else {
			if (tile_index&0x200)
			tile_index=(tile_index&0xff)+(deco16_io_ram[3]<<8); /* Top right */
		else
			tile_index=(tile_index&0xff)+(deco16_io_ram[2]<<8); /* Top left */
	}

	tile=RAM[tile_index];
	if (tile>0x7f) bank=3; else bank=2;
	SET_TILE_INFO(bank,tile&0x7f,background_color,0);
}

static TILE_GET_INFO( get_fix_tile_info )
{
	int tile,color;

	tile=videoram[tile_index+0x400]+((videoram[tile_index]&0x7)<<8);
	color=(videoram[tile_index]&0x70)>>4;

//if (tile&0x300) tile-=0x000;
//else if(tile&0x200) tile-=0x100;
//else if (tile&0x100) tile-=0x100;
//else tile+=0x200;

	SET_TILE_INFO(0,tile,color,0);
}

/***************************************************************************/

WRITE8_HANDLER( deco16_io_w )
{
	deco16_io_ram[offset]=data;
	if (offset>1 && offset<6)
		tilemap_mark_all_tiles_dirty(background_tilemap);

	switch (offset) {
		case 6: /* Background colour */
			if (((data>>4)&3)!=background_color) {
				background_color=(data>>4)&3;
				tilemap_mark_all_tiles_dirty(background_tilemap);
			}
			background_disable=data&0x4;
			flip_screen_set(data&0x1);
			break;
		case 7: /* Background palette resistors? */
			/* Todo */
			break;
		case 8: /* Irq ack */
			cpunum_set_input_line(0,DECO16_IRQ_LINE,CLEAR_LINE);
			break;
		case 9: /* Sound */
			soundlatch_w(0,data);
			cpunum_set_input_line(1,M6502_IRQ_LINE,HOLD_LINE);
			break;
	}
}

WRITE8_HANDLER( liberate_videoram_w )
{
	videoram[offset]=data;
	tilemap_mark_tile_dirty(fix_tilemap,offset&0x3ff);
}

/***************************************************************************/

VIDEO_START( prosoccr )
{
	background_tilemap = tilemap_create(get_back_tile_info,back_scan,TILEMAP_TYPE_PEN,16,16,32,32);
	fix_tilemap = tilemap_create(get_fix_tile_info,fix_scan,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(fix_tilemap,0);
}

VIDEO_START( boomrang )
{
	background_tilemap = tilemap_create(get_back_tile_info,back_scan,TILEMAP_TYPE_PEN,16,16,32,32);
	fix_tilemap = tilemap_create(get_fix_tile_info,fix_scan,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transmask(background_tilemap,0,0x0001,0x007e); /* Bottom 1 pen/Top 7 pens */
	tilemap_set_transparent_pen(fix_tilemap,0);
}

VIDEO_START( liberate )
{
	background_tilemap = tilemap_create(get_back_tile_info,back_scan,TILEMAP_TYPE_PEN,16,16,32,32);
	fix_tilemap = tilemap_create(get_fix_tile_info,fix_scan,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(fix_tilemap,0);
}

/***************************************************************************/

WRITE8_HANDLER( prosport_paletteram_w )
{
	/* RGB output is inverted */
	paletteram_BBGGGRRR_w(offset,~data);
}

PALETTE_INIT( liberate )
{
	int i,bit0,bit1,bit2,g,r,b;

	for (i = 0;i < 32;i++)
	{
		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		color_prom++;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
	palette_set_color(machine,32,MAKE_RGB(0,0,0)); /* Allocate black for when no background is displayed */
}

/***************************************************************************/

static void liberate_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Sprites */
	for (offs = 0;offs < 0x800;offs += 4)
	{
		int multi,fx,fy,sx,sy,sy2,code,color;

		code = spriteram[offs+1] + ( ( spriteram[offs+0] & 0x60 ) << 3 );
		sx = (240 - spriteram[offs+3]);
	//if (sx < -7) sx += 256;

		sy = 240-spriteram[offs+2];
		color = 0;//(spriteram[offs+1] & 0x03);// + ((spriteram[offs+1] & 0x08) >> 1);

//      if (pri==0 && color!=0) continue;
//      if (pri==1 && color==0) continue;

		fx = spriteram[offs+0] & 0x04;
		fy = spriteram[offs+0] & 0x08; // or 0x02 ?
		multi = spriteram[offs+0] & 0x10;


		if (multi) sy-=16;

		if (flip_screen) {
			sy=240-sy;
			sx=240-sx;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			sy2=sy-16;
		}
		else sy2=sy+16;

    	drawgfx(bitmap,machine->gfx[1],
        		code,
				color,
				fx,fy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
        if (multi)
    		drawgfx(bitmap,machine->gfx[1],
				code+1,
				color,
				fx,fy,
				sx,sy2,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

static void prosport_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs,multi,fx,fy,sx,sy,sy2,code,code2,color;

	for (offs = 0x000;offs < 0x800;offs += 4)
	{
	//  if ((spriteram[offs+0]&1)!=1) continue;

		code = spriteram[offs+1] + ((spriteram[offs+0]&0x3)<<8);
		code2=code+1;

		multi = spriteram[offs+0] & 0x10;

		sy=spriteram[offs+2];
		if (multi) sy+=16;
		sx = (240 - spriteram[offs+3]);
//      sy = (240-spriteram[offs+2]);//-16;
		sy = 240-sy;

		color = 1;//(spriteram[offs+0]&0x4)>>2;

		fx = 0;
		fy = spriteram[offs+0] & 0x04;
		multi = 0;// spriteram[offs+0] & 0x10;

//      if (multi) sy-=16;
		if (fy && multi) { code2=code; code++; }

		if (flip_screen) {
			sy=240-sy;
			sx=240-sx;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			sy2=sy-16;
		}
		else {
			sy2=sy+16;
		}

    	drawgfx(bitmap,machine->gfx[1],
        		code,
				color,
				fx,fy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
        if (multi)
    		drawgfx(bitmap,machine->gfx[1],
				code2,
				color,
				fx,fy,
				sx,sy2,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

static void boomrang_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri)
{
	int offs,multi,fx,fy,sx,sy,sy2,code,code2,color;

	for (offs = 0x000;offs < 0x800;offs += 4)
	{
		if ((spriteram[offs+0]&1)!=1) continue;
		if ((spriteram[offs+0]&0x8)!=pri) continue;

		code = spriteram[offs+1] + ((spriteram[offs+0]&0xe0)<<3);
		code2=code+1;

		multi = spriteram[offs+0] & 0x10;

		sy=spriteram[offs+2];
		if (multi) sy+=16;
		sx = (240 - spriteram[offs+3]);
//      sy = (240-spriteram[offs+2]);//-16;
		sy = 240-sy;

		color = (spriteram[offs+0]&0x4)>>2;

		fx = 0;
		fy = spriteram[offs+0] & 0x02;
		multi = spriteram[offs+0] & 0x10;

//      if (multi) sy-=16;
		if (fy && multi) { code2=code; code++; }

		if (flip_screen) {
			sy=240-sy;
			sx=240-sx;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			sy2=sy-16;
		}
		else {
			sy2=sy+16;
		}

    	drawgfx(bitmap,machine->gfx[1],
        		code,
				color,
				fx,fy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
        if (multi)
    		drawgfx(bitmap,machine->gfx[1],
				code2,
				color,
				fx,fy,
				sx,sy2,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

/***************************************************************************/

VIDEO_UPDATE( prosoccr )
{
	tilemap_set_scrolly(background_tilemap,0,deco16_io_ram[1]);
	tilemap_set_scrollx(background_tilemap,0,-deco16_io_ram[0]);

	if (background_disable)
		fillbitmap(bitmap,machine->pens[32],cliprect);
	else
		tilemap_draw(bitmap,cliprect,background_tilemap,0,0);
	boomrang_draw_sprites(machine,bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,fix_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( prosport )
{
	int mx,my,tile,color,offs;

	fillbitmap(bitmap,machine->pens[0],cliprect);

	prosport_draw_sprites(machine,bitmap,cliprect);

	for (offs = 0;offs < 0x400;offs++) {
		tile=videoram[offs+0x400]+((videoram[offs]&0x3)<<8);

		tile+=((deco16_io_ram[0]&0x30)<<6);

		if (!tile) continue;

		color=1;//(videoram[offs]&0x70)>>4;
		my = (offs) % 32;
		mx = (offs) / 32;

		drawgfx(bitmap,machine->gfx[0],
				tile,1,0,0,248-8*mx,8*my,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}

VIDEO_UPDATE( boomrang )
{
	tilemap_set_scrolly(background_tilemap,0, deco16_io_ram[1]);
	tilemap_set_scrollx(background_tilemap,0,-deco16_io_ram[0]);

	if (background_disable)
		fillbitmap(bitmap,machine->pens[32],cliprect);
	else
		tilemap_draw(bitmap,cliprect,background_tilemap,TILEMAP_DRAW_LAYER1,0);

	boomrang_draw_sprites(machine,bitmap,cliprect,8);
	if (!background_disable)
		tilemap_draw(bitmap,cliprect,background_tilemap,TILEMAP_DRAW_LAYER0,0);
	boomrang_draw_sprites(machine,bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,fix_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( liberate )
{
	tilemap_set_scrolly(background_tilemap,0, deco16_io_ram[1]);
	tilemap_set_scrollx(background_tilemap,0,-deco16_io_ram[0]);

	if (background_disable)
		fillbitmap(bitmap,machine->pens[32],cliprect);
	else
		tilemap_draw(bitmap,cliprect,background_tilemap,0,0);

	liberate_draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,fix_tilemap,0,0);
	return 0;
}
