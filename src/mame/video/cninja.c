/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "driver.h"
#include "deco16ic.h"
#include "cninja.h"

/******************************************************************************/

static int cninja_bank_callback(const int bank)
{
	if ((bank>>4)&0xf) return 0x0000; /* Only 2 banks */
	return 0x1000;
}

static int edrandy_bank_callback(const int bank)
{
	if ((bank>>4)&0xf) return 0x0000; /* Only 2 banks */
	return 0x1000;
}

static int robocop2_bank_callback(const int bank)
{
	return (bank&0x30)<<8;
}

static int mutantf_1_bank_callback(const int bank)
{
	return ((bank>>4)&0x3)<<12;
}

static int mutantf_2_bank_callback(const int bank)
{
	return ((bank>>5)&0x1)<<14;
}

/******************************************************************************/

VIDEO_START( cninja )
{
	deco16_2_video_init(1);

	deco16_set_tilemap_bank_callback(2,cninja_bank_callback);
	deco16_set_tilemap_bank_callback(3,cninja_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( stoneage )
{
	deco16_2_video_init(1);

	deco16_set_tilemap_bank_callback(2,edrandy_bank_callback);
	deco16_set_tilemap_bank_callback(3,edrandy_bank_callback);
	deco16_set_tilemap_colour_base(3,48);

	/* The bootleg has broken scroll registers */
	tilemap_set_scrolldx(deco16_get_tilemap(3,0),-10,-10);
	tilemap_set_scrolldx(deco16_get_tilemap(1,0),-10,-10);
	tilemap_set_scrolldx(deco16_get_tilemap(0,1),2,2);
}

VIDEO_START( edrandy )
{
	deco16_2_video_init(0);

	deco16_set_tilemap_bank_callback(2,edrandy_bank_callback);
	deco16_set_tilemap_bank_callback(3,edrandy_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( robocop2 )
{
	deco16_2_video_init(0);

	deco16_set_tilemap_bank_callback(1,robocop2_bank_callback);
	deco16_set_tilemap_bank_callback(2,robocop2_bank_callback);
	deco16_set_tilemap_bank_callback(3,robocop2_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( mutantf )
{
	deco16_2_video_init(0);

	deco16_set_tilemap_bank_callback(0,mutantf_1_bank_callback);
	deco16_set_tilemap_bank_callback(1,mutantf_2_bank_callback);
	deco16_set_tilemap_bank_callback(2,mutantf_1_bank_callback);
	deco16_set_tilemap_bank_callback(3,mutantf_1_bank_callback);

	deco16_set_tilemap_colour_base(1,0x30);
	deco16_set_tilemap_colour_base(2,0x20);
	deco16_set_tilemap_colour_base(3,0x40);

	alpha_set_level(0x80);
}

/******************************************************************************/

VIDEO_EOF( cninja )
{
	deco16_raster_display_position=0;
}

static void raster_pf3_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, int pri)
{
	tilemap *tmap=deco16_get_tilemap(2,0);
	int ptr=0,start,end=0;
	rectangle clip;
	int overflow=deco16_raster_display_position;

	clip.min_x = cliprect->min_x;
	clip.max_x = cliprect->max_x;

	/* Finish list up to end of visible display */
	deco16_raster_display_list[overflow++]=255;
	deco16_raster_display_list[overflow++]=deco16_pf12_control[1];
	deco16_raster_display_list[overflow++]=deco16_pf12_control[2];
	deco16_raster_display_list[overflow++]=deco16_pf12_control[3];
	deco16_raster_display_list[overflow++]=deco16_pf12_control[4];
	deco16_raster_display_list[overflow++]=deco16_pf34_control[1];
	deco16_raster_display_list[overflow++]=deco16_pf34_control[2];
	deco16_raster_display_list[overflow++]=deco16_pf34_control[3];
	deco16_raster_display_list[overflow++]=deco16_pf34_control[4];

	while (ptr<overflow) {
		start=end;
		end=deco16_raster_display_list[ptr++];

		/* Restore state of registers before IRQ */
		deco16_pf12_control[1]=deco16_raster_display_list[ptr++];
		deco16_pf12_control[2]=deco16_raster_display_list[ptr++];
		deco16_pf12_control[3]=deco16_raster_display_list[ptr++];
		deco16_pf12_control[4]=deco16_raster_display_list[ptr++];
		deco16_pf34_control[1]=deco16_raster_display_list[ptr++];
		deco16_pf34_control[2]=deco16_raster_display_list[ptr++];
		deco16_pf34_control[3]=deco16_raster_display_list[ptr++];
		deco16_pf34_control[4]=deco16_raster_display_list[ptr++];

		clip.min_y = start;
		clip.max_y = end;

		/* Update tilemap for this register state, and draw */
		deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);
		tilemap_draw(bitmap,&clip,tmap,flags,pri);
	}
}

/******************************************************************************/

static void cninja_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0x400-4;offs >=0 ;offs -= 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult,pri=0;

		sprite = buffered_spriteram16[offs+1];
		if (!sprite) continue;

		x = buffered_spriteram16[offs+2];

		/* Sprite/playfield priority */
		switch (x&0xc000) {
		case 0x0000: pri=0; break;
		case 0x4000: pri=0xf0; break;
		case 0x8000: pri=0xf0|0xcc; break;
		case 0xc000: pri=0xf0|0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}

		y = buffered_spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;
		colour = (x >> 9) &0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0,pri);

			multi--;
		}
	}
}

static void robocop2_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0x400-4;offs >=0 ;offs -= 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult,pri=0;
		sprite = buffered_spriteram16[offs+1];
		if (!sprite) continue;

		x = buffered_spriteram16[offs+2];

		/* Sprite/playfield priority */
		switch (x&0xc000) {
		case 0x0000: pri=0; break;
		case 0x4000: pri=0xf0; break;
		case 0x8000: pri=0xf0|0xcc; break;
		case 0xc000: pri=0xf0|0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}

		y = buffered_spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;
		colour = (x >> 9) &0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		x = 304 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen) {
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0,pri);

			multi--;
		}
	}
}

static void mutantf_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, const UINT16 *spriteptr, int gfxbank)
{
	int offs,end,inc;

	/*
        Alternate format from most 16 bit games - same as Captain America

        Word 0:
            0x8000: Y flip
            0x4000: X flip
            0x2000: Flash (Sprite toggles on/off every frame)
            0x1fff: Y value
        Word 1:
            0xffff: X value
        Word 2:
            0xf000: Block height
            0x0f00: Block width
            0x00e0: Unused?
            0x001f: Colour
        Word 3:
            0xffff: Sprite value
    */

	/* This may look strange, but the alpha-blended sprite chip definitely draws end to
        front, ie, reversed from normal pdrawgfx style. */
	if (gfxbank==4) {
		offs=0;
		end=0x400;
		inc=4;
	} else {
		offs=0x3fc;
		end=-4;
		inc=-4;
	}

	while (offs!=end)
	{
		int x,y,sprite,colour,fx,fy,w,h,sx,sy,x_mult,y_mult;
		int trans=TRANSPARENCY_PEN;

		sprite = spriteptr[offs+3];
		if (!sprite) {
			offs+=inc;
			continue;
		}

		sx = spriteptr[offs+1];

		h = (spriteptr[offs+2]&0xf000)>>12;
		w = (spriteptr[offs+2]&0x0f00)>> 8;

		sy = spriteptr[offs];
		if ((sy&0x2000) && (cpu_getcurrentframe() & 1)) {
			offs+=inc;
			continue;
		}

		colour = (spriteptr[offs+2] >>0) & 0x1f;

		if (gfxbank==4) { /* Seems to be always alpha'd */
			trans=TRANSPARENCY_ALPHA;
			colour&=0xf;
		}

		fx = (spriteptr[offs+0]&0x4000);
		fy = (spriteptr[offs+0]&0x8000);

		if (flip_screen) {
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;

			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx>0x180) sx=-(0x200 - sx);
			if (sy>0x180) sy=-(0x200 - sy);

			if (fx) { x_mult=-16; sx+=16*w; } else { x_mult=16; sx-=16; }
			if (fy) { y_mult=-16; sy+=16*h; } else { y_mult=16; sy-=16; }
		} else {
			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx&0x100) sx=-(0x100 - (sx&0xff));
			if (sy&0x100) sy=-(0x100 - (sy&0xff));
			sx = 304 - sx;
			sy = 240 - sy;
			if (sx >= 432) sx -= 512;
			if (sy >= 384) sy -= 512;
			if (fx) { x_mult=-16; sx+=16; } else { x_mult=16; sx-=16*w; }
			if (fy) { y_mult=-16; sy+=16; } else { y_mult=16; sy-=16*h; }
		}

		for (x=0; x<w; x++) {
			for (y=0; y<h; y++) {
				pdrawgfx(bitmap,machine->gfx[gfxbank],
						sprite + y + h * x,
						colour,
						fx,fy,
						sx + x_mult * (w-x),sy + y_mult * (h-y),
						cliprect,trans,0,0);
			}
		}

		offs+=inc;
	}
}

/******************************************************************************/

VIDEO_UPDATE( cninja )
{
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[512],cliprect);
	deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);
	deco16_tilemap_3_draw(bitmap,cliprect,0,2);
	deco16_tilemap_2_draw(bitmap,cliprect,TILEMAP_DRAW_LAYER1,2);
	deco16_tilemap_2_draw(bitmap,cliprect,TILEMAP_DRAW_LAYER0,4);
	cninja_draw_sprites(machine,bitmap,cliprect);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( edrandy )
{
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);
	deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);
	if (deco16_raster_display_position)
		raster_pf3_draw(bitmap,cliprect,0,2);
	else
		deco16_tilemap_3_draw(bitmap,cliprect,0,2);
	deco16_tilemap_2_draw(bitmap,cliprect,0,4);
	cninja_draw_sprites(machine,bitmap,cliprect);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( robocop2 )
{
	/* One of the tilemap chips can switch between 2 tilemaps at 4bpp, or 1 at 8bpp */
	if (deco16_priority&4) {
		deco16_set_tilemap_colour_mask(2,0);
		deco16_set_tilemap_colour_mask(3,0);
		deco16_pf34_set_gfxbank(0,4);
	} else {
		deco16_set_tilemap_colour_mask(2,0xf);
		deco16_set_tilemap_colour_mask(3,0xf);
		deco16_pf34_set_gfxbank(0,2);
	}

	/* Update playfields */
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0x200],cliprect);
	if ((deco16_priority&4)==0)
		deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);

	/* Switchable priority */
	switch (deco16_priority&0x8) {
		case 8:
			deco16_tilemap_2_draw(bitmap,cliprect,0,2);
			if (deco16_raster_display_position)
				raster_pf3_draw(bitmap,cliprect,0,4);
			else
				deco16_tilemap_3_draw(bitmap,cliprect,0,4);
			break;
		default:
		case 0:
			if (deco16_raster_display_position)
				raster_pf3_draw(bitmap,cliprect,0,2);
			else
				deco16_tilemap_3_draw(bitmap,cliprect,0,2);
			deco16_tilemap_2_draw(bitmap,cliprect,0,4);
			break;
	}

	robocop2_draw_sprites(machine,bitmap,cliprect);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( mutantf )
{
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	fillbitmap(bitmap,machine->pens[0x400],cliprect); /* Confirmed */

	/* There is no priority prom on this board, but there is a
    priority control word, the only values used in game appear
    to be 2, 6 & 7 though:

    Bit 0:  If set sprite chip 2 above sprite chip 1 else vice versa
    Bit 1:  Always set?
    Bit 2:  Almost always set  (Sometimes not set on screen transitions)

    The other bits may control alpha blend on the 2nd sprite chip, or
    layer order.
    */
	deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
	deco16_tilemap_2_draw(bitmap,cliprect,0,0);
	deco16_tilemap_3_draw(bitmap,cliprect,0,0);

	/* We need to abuse the priority bitmap a little by clearing it before
        drawing each sprite layer.  This is because there is no priority
        orthogonality between sprite layers, but the alpha layer must obey
        priority between sprites in each layer.  Ie, if we didn't do this,
        then when two alpha blended shadows overlapped then they would be 25%
        transparent against the background, rather than 50% */
	if (deco16_priority&1) {
		fillbitmap(priority_bitmap,0,cliprect);
		mutantf_draw_sprites(machine,bitmap,cliprect,buffered_spriteram16,3);
		fillbitmap(priority_bitmap,0,cliprect);
		mutantf_draw_sprites(machine,bitmap,cliprect,buffered_spriteram16_2,4);
	} else {
		fillbitmap(priority_bitmap,0,cliprect);
		mutantf_draw_sprites(machine,bitmap,cliprect,buffered_spriteram16_2,4);
		fillbitmap(priority_bitmap,0,cliprect);
		mutantf_draw_sprites(machine,bitmap,cliprect,buffered_spriteram16,3);
	}
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}
