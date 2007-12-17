/* Funny Bubble Video hardware

todo - convert to tilemap

 */


#include "driver.h"

UINT8* funybubl_banked_videoram;
UINT8 *funybubl_paletteram;


WRITE8_HANDLER ( funybubl_paldatawrite )
{
	int colchanged ;

	UINT32 coldat;

	funybubl_paletteram[offset] = data;

	colchanged = offset >> 2;

	coldat = funybubl_paletteram[colchanged*4] | (funybubl_paletteram[colchanged*4+1] << 8) | (funybubl_paletteram[colchanged*4+2] << 16) | (funybubl_paletteram[colchanged*4+3] << 24);

	palette_set_color_rgb(Machine,colchanged,pal6bit(coldat >> 12),pal6bit(coldat >> 0),pal6bit(coldat >> 6));
}


VIDEO_START(funybubl)
{
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{


	UINT8 *source = &funybubl_banked_videoram[0x2000-0x20];
	UINT8 *finish = source - 0x1000;


	while( source>finish )
	{
		int xpos, ypos, tile;

		/* the sprites are in the sprite list twice
         the first format (in comments) appears to be a buffer, if you use
         this list you get garbage sprites in 2 player mode
         the second format (used) seems correct

         */
/*
        ypos = 0xff-source[1+0x10];
        xpos = source[2+0x10];
        tile =  source[0+0x10] | ( (source[3+0x10] & 0x0f) <<8);
        if (source[3+0x10] & 0x80) tile += 0x1000;
        if (source[3+0x10] & 0x20) xpos += 0x100;
        // bits 0x40 (not used?) and 0x10 (just set during transition period of x co-ord 0xff and 0x00) ...
        xpos -= 8;
        ypos -= 14;

*/
		ypos = source[2];
		xpos = source[3];
		tile =  source[0] | ( (source[1] & 0x0f) <<8);
		if (source[1] & 0x80) tile += 0x1000;
		if (source[1] & 0x20) {	if (xpos < 0xe0) xpos += 0x100; }
		// bits 0x40 and 0x10 not used?...

		drawgfx(bitmap,machine->gfx[1],tile,0,0,0,xpos,ypos,cliprect,TRANSPARENCY_PEN,255);

		source -= 0x20;
	}

}


VIDEO_UPDATE(funybubl)
{
	int x,y, offs;

	offs = 0;

	fillbitmap(bitmap, get_black_pen(machine), cliprect);


	/* tilemap .. convert it .. banking makes it slightly more annoying but still easy */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x< 64; x++)
		{
			int data;

			data = funybubl_banked_videoram[offs] | (funybubl_banked_videoram[offs+1] << 8);
			drawgfx(bitmap,machine->gfx[0],data&0x7fff,(data&0x8000)?2:1,0,0,x*8,y*8,cliprect,TRANSPARENCY_PEN,0);
			offs+=2;
		}
	}

	draw_sprites(machine,bitmap,cliprect);

/*
    if ( input_code_pressed_once(KEYCODE_W) )
    {
        FILE *fp;

        fp=fopen("funnybubsprites", "w+b");
        if (fp)
        {
            fwrite(&funybubl_banked_videoram[0x1000], 0x1000, 1, fp);
            fclose(fp);
        }
    }
*/
	return 0;
}
