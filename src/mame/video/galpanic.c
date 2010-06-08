#include "emu.h"
#include "kan_pand.h"

UINT16 *galpanic_bgvideoram,*galpanic_fgvideoram;
size_t galpanic_fgvideoram_size;

static bitmap_t *sprites_bitmap;

VIDEO_START( galpanic )
{
	machine->generic.tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();
	sprites_bitmap = machine->primary_screen->alloc_compatible_bitmap();
}

PALETTE_INIT( galpanic )
{
	int i;

	/* first 1024 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0;i < 32768;i++)
		palette_set_color_rgb(machine,i+1024,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}



WRITE16_HANDLER( galpanic_bgvideoram_w )
{
	int sx,sy;


	data = COMBINE_DATA(&galpanic_bgvideoram[offset]);

	sy = offset / 256;
	sx = offset % 256;

	*BITMAP_ADDR16(space->machine->generic.tmpbitmap, sy, sx) = 1024 + (data >> 1);
}

WRITE16_HANDLER( galpanic_paletteram_w )
{
	data = COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	/* bit 0 seems to be a transparency flag for the front bitmap */
	palette_set_color_rgb(space->machine,offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}


static void comad_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;
	int sx=0, sy=0;

	for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 4)
	{
		int code,color,flipx,flipy;

		code = spriteram16[offs + 1] & 0x1fff;
		color = (spriteram16[offs] & 0x003c) >> 2;
		flipx = spriteram16[offs] & 0x0002;
		flipy = spriteram16[offs] & 0x0001;

		if((spriteram16[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += spriteram16[offs + 2] >> 6;
			sy += spriteram16[offs + 3] >> 6;
		}
		else
		{
			sx = spriteram16[offs + 2] >> 6;
			sy = spriteram16[offs + 3] >> 6;
		}

		sx = (sx&0x1ff) - (sx&0x200);
		sy = (sy&0x1ff) - (sy&0x200);

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

static void draw_fgbitmap(bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < galpanic_fgvideoram_size/2;offs++)
	{
		int sx,sy,color;

		sx = offs % 256;
		sy = offs / 256;
		color = galpanic_fgvideoram[offs];
		if (color)
			*BITMAP_ADDR16(bitmap, sy, sx) = color;
	}
}

VIDEO_UPDATE( galpanic )
{
	running_device *pandora = devtag_get_device(screen->machine, "pandora");

	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,screen->machine->generic.tmpbitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap, cliprect);

	pandora_update(pandora, bitmap, cliprect);

	return 0;
}

VIDEO_UPDATE( comad )
{
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,screen->machine->generic.tmpbitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap,cliprect);


//  if(galpanic_clear_sprites)
	{
		bitmap_fill(sprites_bitmap,cliprect,0);
		comad_draw_sprites(screen->machine,bitmap,cliprect);
	}
//  else
//  {
//      /* keep sprites on the bitmap without clearing them */
//      comad_draw_sprites(screen->machine,sprites_bitmap,0);
//      copybitmap_trans(bitmap,sprites_bitmap,0,0,0,0,cliprect,0);
//  }
	return 0;
}
