#include "driver.h"
#include "video/taitoic.h"

#define TC0100SCN_GFX_NUM 1

UINT16 *othunder_ram;

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};
static struct tempsprite *spritelist;

static int taito_hide_pixels;



/**********************************************************/

static VIDEO_START( othunder_core )
{
	/* Up to $800/8 big sprites, requires 0x100 * sizeof(*spritelist)
       Multiply this by 32 to give room for the number of small sprites,
       which are what actually get put in the structure. */

	spritelist = auto_malloc(0x2000 * sizeof(*spritelist));

	TC0100SCN_vh_start(machine,1,TC0100SCN_GFX_NUM,taito_hide_pixels,0,0,0,0,0,0);

	if (has_TC0110PCR())
		TC0110PCR_vh_start();
}

VIDEO_START( othunder )
{
	/* There is a problem here. 4 is correct for text layer/sprite
       alignment, but the bg layers [or one of them] are wrong */

	taito_hide_pixels = 4;
	video_start_othunder_core(machine);
}


/************************************************************
            SPRITE DRAW ROUTINE

It draws a series of small tiles ("chunks") together to
create a big sprite. The spritemap rom provides the lookup
table for this. We look up the 16x8 sprite chunks from
the spritemap rom, creating each 64x64 sprite as follows:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15
    16 17 18 19
    20 21 22 23
    24 25 26 27
    28 29 30 31

The game makes heavy use of sprite zooming.

        ***

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]


        Othunder (modified table from Raine)

        Byte | Bit(s) | Description
        -----+76543210+-------------------------------------
          0  |xxxxxxx.| ZoomY (0 min, 63 max - msb unused as sprites are 64x64)
          0  |.......x| Y position (High)
          1  |xxxxxxxx| Y position (Low)
          2  |x.......| Sprite/BG Priority (0=sprites high)
          2  |.x......| Flip X
          2  |..?????.| unknown/unused ?
          2  |.......x| X position (High)
          3  |xxxxxxxx| X position (Low)
          4  |xxxxxxxx| Palette bank
          5  |?.......| unknown/unused ?
          5  |.xxxxxxx| ZoomX (0 min, 63 max - msb unused as sprites are 64x64)
          6  |x.......| Flip Y
          6  |.??.....| unknown/unused ?
          6  |...xxxxx| Sprite Tile high (2 msbs unused - 3/4 of spritemap rom empty)
          7  |xxxxxxxx| Sprite Tile low

********************************************************/


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,const int *primasks,int y_offs)
{
	UINT16 *spritemap = (UINT16 *)memory_region(REGION_USER1);
	UINT16 tile_mask = (machine->gfx[0]->total_elements) - 1;
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk,map_offset,code,j,k,px,py;
	int bad_chunks;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	struct tempsprite *sprite_ptr = spritelist;

	for (offs = (spriteram_size/2)-4;offs >=0;offs -= 4)
	{
		data = spriteram16[offs+0];
		zoomy = (data & 0xfe00) >> 9;
		y = data & 0x1ff;

		data = spriteram16[offs+1];
		flipx = (data & 0x4000) >> 14;
		priority = (data & 0x8000) >> 15;
		x = data & 0x1ff;

		data = spriteram16[offs+2];
		color = (data & 0xff00) >> 8;
		zoomx = (data & 0x7f);

		data = spriteram16[offs+3];
		tilenum = data & 0x1fff;	// $80000 spritemap rom maps up to $2000 64x64 sprites
		flipy = (data & 0x8000) >> 15;

		if (!tilenum) continue;

		map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x>0x140) x -= 0x200;
		if (y>0x140) y -= 0x200;

		bad_chunks = 0;

		for (sprite_chunk=0;sprite_chunk<32;sprite_chunk++)
		{
			k = sprite_chunk % 4;   /* 4 chunks per row */
			j = sprite_chunk / 4;   /* 8 rows */

			px = k;
			py = j;
			if (flipx)  px = 3-k;	/* pick tiles back to front for x and y flips */
			if (flipy)  py = 7-j;

			code = spritemap[map_offset + px + (py<<2)] &tile_mask;

			if (code==0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k*zoomx)/4);
			cury = y + ((j*zoomy)/8);

			zx= x + (((k+1)*zoomx)/4) - curx;
			zy= y + (((j+1)*zoomy)/8) - cury;

			if (sprites_flipscreen)
			{
				/* -zx/y is there to fix zoomed sprite coords in screenflip.
                   drawgfxzoom does not know to draw from flip-side of sprites when
                   screen is flipped; so we must correct the coords ourselves. */

				curx = 320 - curx - zx;
				cury = 256 - cury - zy;
				flipx = !flipx;
				flipy = !flipy;
			}

			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 13;

			if (primasks)
			{
				sprite_ptr->primask = primasks[priority];
				sprite_ptr++;
			}
			else
			{
				drawgfxzoom(bitmap,machine->gfx[0],
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						cliprect,TRANSPARENCY_PEN,0,
						sprite_ptr->zoomx,sprite_ptr->zoomy);
			}
		}

		if (bad_chunks)
logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}

	/* this happens only if primsks != NULL */
	while (sprite_ptr != spritelist)
	{
		sprite_ptr--;

		pdrawgfxzoom(bitmap,machine->gfx[0],
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				cliprect,TRANSPARENCY_PEN,0,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				sprite_ptr->primask);
	}
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( othunder )
{
	int layer[3];

	TC0100SCN_tilemap_update(machine);

	layer[0] = TC0100SCN_bottomlayer(0);
	layer[1] = layer[0]^1;
	layer[2] = 2;

	fillbitmap(priority_bitmap,0,cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	fillbitmap(bitmap, machine->pens[0], cliprect);

	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[0],TILEMAP_DRAW_OPAQUE,1);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[1],0,2);
	TC0100SCN_tilemap_draw(machine,bitmap,cliprect,0,layer[2],0,4);

	/* Sprites can be under/over the layer below text layer */
	{
		static const int primasks[2] = {0xf0,0xfc};
		draw_sprites(machine, bitmap,cliprect,primasks,3);
	}

	/* Draw artificial gun targets */
	{
		int rawx, rawy, centrex, centrey, screenx, screeny;

		/* calculate p1 screen co-ords by matching routine at $A932 */
		rawx = othunder_ram[0x2848/2];
		centrex = othunder_ram[0xa046/2];
		if (rawx <= centrex)
		{
			rawx = centrex - rawx;
			screenx = rawx * othunder_ram[0xa04e/2] + (((rawx * othunder_ram[0xa050/2]) & 0xffff0000) >> 16);
			screenx = 0xa0 - screenx;
			if (screenx < 0) screenx = 0;
		}
		else
		{
			if (rawx > othunder_ram[0xa028/2]) rawx = othunder_ram[0xa028/2];
			rawx -= centrex;
			screenx = rawx * othunder_ram[0xa056/2] + (((rawx * othunder_ram[0xa058/2]) & 0xffff0000) >> 16);
			screenx += 0xa0;
			if (screenx > 0x140) screenx = 0x140;
		}
		rawy = othunder_ram[0x284a/2];
		centrey = othunder_ram[0xa048/2];
		if (rawy <= centrey)
		{
			rawy = centrey - rawy;
			screeny = rawy * othunder_ram[0xa052/2] + (((rawy * othunder_ram[0xa054/2]) & 0xffff0000) >> 16);
			screeny = 0x78 - screeny;
			if (screeny < 0) screeny = 0;
		}
		else
		{
			if (rawy > othunder_ram[0xa030/2]) rawy = othunder_ram[0xa030/2];
			rawy -= centrey;
			screeny = rawy * othunder_ram[0xa05a/2] + (((rawy * othunder_ram[0xa05c/2]) & 0xffff0000) >> 16);
			screeny += 0x78;
			if (screeny > 0xf0) screeny = 0xf0;
		}

		// fudge y to show in centre of scope/hit sprite, note that screenx, screeny
		// were confirmed to match those stored by the game at $82732, $82734
		screeny += 2;

		/* calculate p2 screen co-ords by matching routine at $AA48 */
		rawx = othunder_ram[0x284c/2];
		centrex = othunder_ram[0xa04a/2];
		if (rawx <= centrex)
		{
			rawx = centrex - rawx;
			screenx = rawx * othunder_ram[0xa05e/2] + (((rawx * othunder_ram[0xa060/2]) & 0xffff0000) >> 16);
			screenx = 0xa0 - screenx;
			if (screenx < 0) screenx = 0;
		}
		else
		{
			if (rawx > othunder_ram[0xa038/2]) rawx = othunder_ram[0xa038/2];
			rawx -= centrex;
			screenx = rawx * othunder_ram[0xa066/2] + (((rawx * othunder_ram[0xa068/2]) & 0xffff0000) >> 16);
			screenx += 0xa0;
			if (screenx > 0x140) screenx = 0x140;
		}
		rawy = othunder_ram[0x284e/2];
		centrey = othunder_ram[0xa04c/2];
		if (rawy <= centrey)
		{
			rawy = centrey - rawy;
			screeny = rawy * othunder_ram[0xa062/2] + (((rawy * othunder_ram[0xa064/2]) & 0xffff0000) >> 16);
			screeny = 0x78 - screeny;
			if (screeny < 0) screeny = 0;
		}
		else
		{
			if (rawy > othunder_ram[0xa040/2]) rawy = othunder_ram[0xa040/2];
			rawy -= centrey;
			screeny = rawy * othunder_ram[0xa06a/2] + (((rawy * othunder_ram[0xa06c/2]) & 0xffff0000) >> 16);
			screeny += 0x78;
			if (screeny > 0xf0) screeny = 0xf0;
		}

		// fudge y to show in centre of scope/hit sprite, note that screenx, screeny
		// were confirmed to match those stored by the game at $82736, $82738
		screeny += 2;
	}
	return 0;
}

