#include "driver.h"

UINT8 *lsasquad_scrollram;



static void draw_layer(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8 *scrollram)
{
	int offs,scrollx,scrolly;


	scrollx = scrollram[3];
	scrolly = -scrollram[0];

	for (offs = 0;offs < 0x080;offs += 4)
	{
		int base,y,sx,sy,code,color;

		base = 64 * scrollram[offs+1];
		sx = 8*(offs/4) + scrollx;
		if (flip_screen) sx = 248 - sx;
		sx &= 0xff;

		for (y = 0;y < 32;y++)
		{
			int attr;

			sy = 8*y + scrolly;
			if (flip_screen) sy = 248 - sy;
			sy &= 0xff;

			attr = videoram[base + 2*y + 1];
			code = videoram[base + 2*y] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			drawgfx(bitmap,machine->gfx[0],
					code,
					color,
					flip_screen,flip_screen,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,15);
			if (sx > 248)	/* wraparound */
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen,flip_screen,
						sx-256,sy,
						cliprect,TRANSPARENCY_PEN,15);
		}
	}
}

static int draw_layer_daikaiju(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int offs, int  * previd, int type)
{
	int id,scrollx,scrolly,initoffs, globalscrollx;
	int stepx=0;
	initoffs=offs;

	globalscrollx=0;

	id=lsasquad_scrollram[offs+2];

	for(;offs < 0x400; offs+=4)
	{
		int base,y,sx,sy,code,color;

		 //id change
		if(id!=lsasquad_scrollram[offs+2])
		{
			*previd = id;
			return offs;
		}
		else
		{
			id=lsasquad_scrollram[offs+2];
		}

		//skip empty (??) column, potential probs with 1st column in scrollram (scroll 0, tile 0, id 0)
		if( (lsasquad_scrollram[offs+0]|lsasquad_scrollram[offs+1]|lsasquad_scrollram[offs+2]|lsasquad_scrollram[offs+3])==0)
			continue;

		//local scroll x/y
		scrolly = -lsasquad_scrollram[offs+0];
		scrollx =  lsasquad_scrollram[offs+3];

	 	//check for global x scroll used in bg layer in game (starts at offset 0 in scrollram
	 	// and game name/logo on title screen (starts in the middle of scrollram, but with different
	 	// (NOT unique )id than prev coulmn(s)

		if( *previd!=1 )
		{
			if(offs!=initoffs)
			{
				scrollx+=globalscrollx;
			}
			else
			{
				//global scroll init
				globalscrollx=scrollx;
			}
		}

		base = 64 * lsasquad_scrollram[offs+1];
		sx = scrollx+stepx;

		if (flip_screen) sx = 248 - sx;
		sx &= 0xff;

		for (y = 0;y < 32;y++)
		{
			int attr;

			sy = 8*y + scrolly;
			if (flip_screen) sy = 248 - sy;
			sy &= 0xff;

			attr = videoram[base + 2*y + 1];
			code = videoram[base + 2*y] + ((attr & 0x0f) << 8);
			color = attr >> 4;


			if((type==0 && color!=0x0d) || (type !=0 && color==0x0d))
				{
			drawgfx(bitmap,machine->gfx[0],
					code,
					color,
					flip_screen,flip_screen,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,15);
			if (sx > 248)	/* wraparound */
				drawgfx(bitmap,machine->gfx[0],
						code,
						color,
						flip_screen,flip_screen,
						sx-256,sy,
						cliprect,TRANSPARENCY_PEN,15);
					}
		}
	}
	return offs;
}

static void drawbg(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int type)
{
	int i=0;
	int id=-1;

	while(i<0x400)
	{
		if(!(lsasquad_scrollram[i+2]&1))
		{
			i=draw_layer_daikaiju(machine, bitmap, cliprect, i, &id,type);
		}
		else
		{
			id=lsasquad_scrollram[i+2];
			i+=4;
		}
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size-4;offs >= 0;offs -= 4)
	{
		int sx,sy,attr,code,color,flipx,flipy;

		sx = spriteram[offs+3];
		sy = 240 - spriteram[offs];
		attr = spriteram[offs+1];
		code = spriteram[offs+2] + ((attr & 0x30) << 4);
		color = attr & 0x0f;
		flipx = attr & 0x40;
		flipy = attr & 0x80;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,15);
		/* wraparound */
		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx-256,sy,
				cliprect,TRANSPARENCY_PEN,15);
	}
}

VIDEO_UPDATE( lsasquad )
{
	fillbitmap(bitmap,machine->pens[511],cliprect);

	draw_layer(machine,bitmap,cliprect,lsasquad_scrollram + 0x000);
	draw_layer(machine,bitmap,cliprect,lsasquad_scrollram + 0x080);
	draw_sprites(machine,bitmap,cliprect);
	draw_layer(machine,bitmap,cliprect,lsasquad_scrollram + 0x100);
	return 0;
}


VIDEO_UPDATE( daikaiju )
{
	fillbitmap(bitmap,machine->pens[511],cliprect);
	drawbg(machine,bitmap,cliprect,0); // bottom
	draw_sprites(machine,bitmap,cliprect);
	drawbg(machine,bitmap,cliprect,1);	// top = pallete $d ?
	return 0;
}
