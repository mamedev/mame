#include "driver.h"
#include "snk.h"


VIDEO_START( sgladiat )
{
	tmpbitmap = auto_bitmap_alloc(512, 256, video_screen_get_format(machine->primary_screen));
}

static void sgladiat_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int scrollx, int scrolly )
{
	const gfx_element *gfx = machine->gfx[1];

	int tile_number, color, sx, sy;
	int offs, x, y;

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++)
	{
		offs = (x<<5)+y;
		tile_number = videoram[offs];

		color = 0;
		sx = x << 3;
		sy = y << 3;

		drawgfx(tmpbitmap,gfx,tile_number,color,0,0,sx,sy,0,TRANSPARENCY_NONE,0);
	}
	copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,1,&scrolly,cliprect);
}


static void tnk3_draw_text(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank, UINT8 *source )
{
	const gfx_element *gfx = machine->gfx[0];

	int tile_number, color, sx, sy;
	int x, y;

	for(x=0; x<32; x++) for(y=0; y<32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(bank == -1) color = 8;
		else
		{
			color = tile_number >> 5;
			tile_number |= bank << 8;
		}
		sx = (x+2) << 3;
		sy = (y+1) << 3;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN,15);
	}
}


static void tnk3_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xscroll, int yscroll )
{
	const gfx_element *gfx = machine->gfx[2];

	int tile_number, attributes, color, sx, sy;
	int yflip;
	int offs;

	for(offs = 0; offs < 50*4; offs+=4)
	{
		if(*(UINT32*)(spriteram+offs) == 0 || *(UINT32*)(spriteram+offs) == -1) continue;

		tile_number = spriteram[offs+1];
		attributes  = spriteram[offs+3]; /* YBFX.CCCC */
		if(attributes & 0x40) tile_number |= 256;

		color = attributes & 0xf;
		sx =  xscroll - spriteram[offs+2];
		if(!(attributes & 0x80)) sx += 256;
		sy = -yscroll + spriteram[offs];
		if(attributes & 0x10) sy += 256;
		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-16) sx -= 512;
		if (sy > 512-16) sy -= 512;
		yflip = attributes & 0x20;

		drawgfx(bitmap,gfx,tile_number,color,0,yflip,sx,sy,cliprect,TRANSPARENCY_PEN_TABLE,7);
	}
}



VIDEO_UPDATE( sgladiat )
{
	UINT8 *ram = snk_rambase - 0xd000;
	int attributes, scrollx, scrolly;

	attributes = ram[0xd300];

	scrollx = -ram[0xd700] + ((attributes & 2) ? 256:0);
	scrolly = -ram[0xd600];
	scrollx += 15;
	scrolly += 8;
	sgladiat_draw_background(screen->machine, bitmap, cliprect, scrollx, scrolly );

	scrollx = ram[0xd500] + ((attributes & 1) ? 256:0);
	scrolly = ram[0xd400];
	scrollx += 29;
	scrolly += 9;
	tnk3_draw_sprites(screen->machine, bitmap, cliprect, scrollx, scrolly );

	tnk3_draw_text(screen->machine, bitmap, cliprect, 0, &ram[0xf000] );
	return 0;
}
