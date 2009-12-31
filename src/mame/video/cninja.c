/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "driver.h"
#include "includes/deco16ic.h"
#include "includes/cninja.h"

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
	deco16_2_video_init(machine, 1);

	deco16_set_tilemap_bank_callback(2,cninja_bank_callback);
	deco16_set_tilemap_bank_callback(3,cninja_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( stoneage )
{
	deco16_2_video_init(machine, 1);

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
	deco16_2_video_init(machine, 0);

	deco16_set_tilemap_bank_callback(2,edrandy_bank_callback);
	deco16_set_tilemap_bank_callback(3,edrandy_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( robocop2 )
{
	deco16_2_video_init(machine, 0);

	deco16_set_tilemap_bank_callback(1,robocop2_bank_callback);
	deco16_set_tilemap_bank_callback(2,robocop2_bank_callback);
	deco16_set_tilemap_bank_callback(3,robocop2_bank_callback);
	deco16_set_tilemap_colour_base(3,48);
}

VIDEO_START( mutantf )
{
	deco16_2_video_init(machine, 0);

	deco16_set_tilemap_bank_callback(0,mutantf_1_bank_callback);
	deco16_set_tilemap_bank_callback(1,mutantf_2_bank_callback);
	deco16_set_tilemap_bank_callback(2,mutantf_1_bank_callback);
	deco16_set_tilemap_bank_callback(3,mutantf_1_bank_callback);

	deco16_set_tilemap_colour_base(1,0x30);
	deco16_set_tilemap_colour_base(2,0x20);
	deco16_set_tilemap_colour_base(3,0x40);
}

/******************************************************************************/

/******************************************************************************/

static void cninja_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
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
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;
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

		if (flip_screen_get(machine)) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}

/* The bootleg sprites are in a different format! */
static void cninjabl_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
	int offs;
	int endoffs;

	// bootleg seems to use 0x180 as an end of list marker
	// find it first, so we can use normal list processing
	endoffs = 0x400-4;
	for (offs = 0;offs <0x400-4 ;offs += 4)
	{
		int y;

		y = buffered_spriteram16[offs+1];

		if (y==0x180)
		{
			endoffs = offs;
			offs = 0x400-4;
		}
	}

	for (offs = endoffs;offs >=0 ;offs -= 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult,pri=0;

		sprite = buffered_spriteram16[offs+0]; // changed on bootleg!
		y = buffered_spriteram16[offs+1]; // changed on bootleg!

		if (!sprite) continue;

		x = buffered_spriteram16[offs+2];

		/* Sprite/playfield priority */
		switch (x&0xc000) {
		case 0x0000: pri=0; break;
		case 0x4000: pri=0xf0; break;
		case 0x8000: pri=0xf0|0xcc; break;
		case 0xc000: pri=0xf0|0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}



		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		colour = (x >> 9) &0x1f;


		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		y -= multi*16; // changed on bootleg!
		y += 4;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		//sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen_get(machine)) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}


static void robocop2_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
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
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;
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

		if (flip_screen_get(machine)) {
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}

static void mutantf_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT16 *spriteptr, int gfxbank)
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
		int alpha=0xff;

		sprite = spriteptr[offs+3];
		if (!sprite) {
			offs+=inc;
			continue;
		}

		sx = spriteptr[offs+1];

		h = (spriteptr[offs+2]&0xf000)>>12;
		w = (spriteptr[offs+2]&0x0f00)>> 8;

		sy = spriteptr[offs];
		if ((sy&0x2000) && (video_screen_get_frame_number(machine->primary_screen) & 1)) {
			offs+=inc;
			continue;
		}

		colour = (spriteptr[offs+2] >>0) & 0x1f;

		if (gfxbank==4) { /* Seems to be always alpha'd */
			alpha=0x80;
			colour&=0xf;
		}

		fx = (spriteptr[offs+0]&0x4000);
		fy = (spriteptr[offs+0]&0x8000);

		if (flip_screen_get(machine)) {
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
				pdrawgfx_alpha(bitmap,cliprect,machine->gfx[gfxbank],
						sprite + y + h * x,
						colour,
						fx,fy,
						sx + x_mult * (w-x),sy + y_mult * (h-y),
						machine->priority_bitmap,0,
						0,alpha);
			}
		}

		offs+=inc;
	}
}

/******************************************************************************/

VIDEO_UPDATE( cninja )
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,512);
	deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);
	deco16_tilemap_3_draw(screen,bitmap,cliprect,0,2);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER1,2);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0,4);
	cninja_draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( cninjabl )
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,512);
	deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);
	deco16_tilemap_3_draw(screen,bitmap,cliprect,0,2);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER1,2);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0,4);
	cninjabl_draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( edrandy )
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0);
	deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);
	deco16_tilemap_3_draw(screen,bitmap,cliprect,0,2);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,0,4);
	cninja_draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
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
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x200);
	if ((deco16_priority&4)==0)
		deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,1);

	/* Switchable priority */
	switch (deco16_priority&0x8) {
		case 8:
			deco16_tilemap_2_draw(screen,bitmap,cliprect,0,2);
			deco16_tilemap_3_draw(screen,bitmap,cliprect,0,4);
			break;
		default:
		case 0:
			deco16_tilemap_3_draw(screen,bitmap,cliprect,0,2);
			deco16_tilemap_2_draw(screen,bitmap,cliprect,0,4);
			break;
	}

	robocop2_draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

VIDEO_UPDATE( mutantf )
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(bitmap,cliprect,0x400); /* Confirmed */

	/* There is no priority prom on this board, but there is a
    priority control word, the only values used in game appear
    to be 2, 6 & 7 though:

    Bit 0:  If set sprite chip 2 above sprite chip 1 else vice versa
    Bit 1:  Always set?
    Bit 2:  Almost always set  (Sometimes not set on screen transitions)

    The other bits may control alpha blend on the 2nd sprite chip, or
    layer order.
    */
	deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
	deco16_tilemap_2_draw(screen,bitmap,cliprect,0,0);
	deco16_tilemap_3_draw(screen,bitmap,cliprect,0,0);

	/* We need to abuse the priority bitmap a little by clearing it before
        drawing each sprite layer.  This is because there is no priority
        orthogonality between sprite layers, but the alpha layer must obey
        priority between sprites in each layer.  Ie, if we didn't do this,
        then when two alpha blended shadows overlapped then they would be 25%
        transparent against the background, rather than 50% */
	if (deco16_priority&1) {
		bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
		mutantf_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram.u16,3);
		bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
		mutantf_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram2.u16,4);
	} else {
		bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
		mutantf_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram2.u16,4);
		bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
		mutantf_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram.u16,3);
	}
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}
