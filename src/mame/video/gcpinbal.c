#include "driver.h"
#include "includes/gcpinbal.h"


static tilemap *gcpinbal_tilemap[3];

UINT16 *gcpinbal_tilemapram;
UINT16 *gcpinbal_ioc_ram;

//UINT16 *gcpinbal_ctrlram;
//static UINT16 gcpinbal_ctrl_reg;

static UINT16 gcpinbal_scrollx[3],gcpinbal_scrolly[3];

static UINT16 bg0_gfxset = 0;
static UINT16 bg1_gfxset = 0;



/*******************************************************************/


static TILE_GET_INFO( get_bg0_tile_info )
{
	UINT16 tilenum = gcpinbal_tilemapram[0 + tile_index*2];
	UINT16 attr    = gcpinbal_tilemapram[1 + tile_index*2];

	SET_TILE_INFO(
			1,
			(tilenum & 0xfff) + bg0_gfxset,
			(attr & 0x1f),
			TILE_FLIPYX( (attr & 0x300) >> 8));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	UINT16 tilenum = gcpinbal_tilemapram[0x800 + tile_index*2];
	UINT16 attr    = gcpinbal_tilemapram[0x801 + tile_index*2];

	SET_TILE_INFO(
			1,
			(tilenum & 0xfff) + 0x2000 + bg1_gfxset,
			(attr & 0x1f) + 0x30,
			TILE_FLIPYX( (attr & 0x300) >> 8));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT16 tilenum = gcpinbal_tilemapram[0x1000 + tile_index];

	SET_TILE_INFO(
			2,
			(tilenum & 0xfff),
			(tilenum >> 12) | 0x70,
			0);
}

#if 0
static void dirty_tilemaps(void)	// will be used for save states
{
	tilemap_mark_all_tiles_dirty(gcpinbal_tilemap[0]);
	tilemap_mark_all_tiles_dirty(gcpinbal_tilemap[1]);
	tilemap_mark_all_tiles_dirty(gcpinbal_tilemap[2]);
}
#endif

static void gcpinbal_core_vh_start (running_machine *machine)
{
	int xoffs = 0;
	int yoffs = 0;

	gcpinbal_tilemap[0] = tilemap_create(machine, get_bg0_tile_info,tilemap_scan_rows,16,16,32,32);
	gcpinbal_tilemap[1] = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows,16,16,32,32);
	gcpinbal_tilemap[2] = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen( gcpinbal_tilemap[0],0 );
	tilemap_set_transparent_pen( gcpinbal_tilemap[1],0 );
	tilemap_set_transparent_pen( gcpinbal_tilemap[2],0 );

	/* flipscreen n/a */
	tilemap_set_scrolldx( gcpinbal_tilemap[0],-xoffs,0 );
	tilemap_set_scrolldx( gcpinbal_tilemap[1],-xoffs,0 );
	tilemap_set_scrolldx( gcpinbal_tilemap[2],-xoffs,0 );
	tilemap_set_scrolldy( gcpinbal_tilemap[0],-yoffs,0 );
	tilemap_set_scrolldy( gcpinbal_tilemap[1],-yoffs,0 );
	tilemap_set_scrolldy( gcpinbal_tilemap[2],-yoffs,0 );
}

VIDEO_START( gcpinbal )
{
	gcpinbal_core_vh_start(machine);
}


/******************************************************************
                   TILEMAP READ AND WRITE HANDLERS
*******************************************************************/

READ16_HANDLER( gcpinbal_tilemaps_word_r )
{
	return gcpinbal_tilemapram[offset];
}

WRITE16_HANDLER( gcpinbal_tilemaps_word_w )
{
	COMBINE_DATA(&gcpinbal_tilemapram[offset]);

	if (offset<0x800)	/* BG0 */
	{
		tilemap_mark_tile_dirty(gcpinbal_tilemap[0],offset/2);
	}
	else if ((offset<0x1000))	/* BG1 */
	{
		tilemap_mark_tile_dirty(gcpinbal_tilemap[1],(offset%0x800)/2);
	}
	else if ((offset<0x1800))	/* FG */
	{
		tilemap_mark_tile_dirty(gcpinbal_tilemap[2],(offset%0x800));
	}
}


#ifdef UNUSED_FUNCTION

READ16_HANDLER( gcpinbal_ctrl_word_r )
{
    // ***** NOT HOOKED UP *****

    return gcpinbal_piv_ctrlram[offset];
}


WRITE16_HANDLER( gcpinbal_ctrl_word_w )
{
    // ***** NOT HOOKED UP *****

    COMBINE_DATA(&gcpinbal_piv_ctrlram[offset]);
    data = gcpinbal_piv_ctrlram[offset];

    switch (offset)
    {
        case 0x00:
            gcpinbal_scrollx[0] = -data;
            break;

        case 0x01:
            gcpinbal_scrollx[1] = -data;
            break;

        case 0x02:
            gcpinbal_scrollx[2] = -data;
            break;

        case 0x03:
            gcpinbal_scrolly[0] = data;
            break;

        case 0x04:
            gcpinbal_scrolly[1] = data;
            break;

        case 0x05:
            gcpinbal_scrolly[2] = data;
            break;

        case 0x06:
            gcpinbal_ctrl_reg = data;
            break;
    }
}

#endif


/****************************************************************
                     SPRITE DRAW ROUTINE

    Word |     Bit(s)      | Use
    -----+-----------------+-----------------
      0  |........ xxxxxxxx| X lo
      1  |........ xxxxxxxx| X hi
      2  |........ xxxxxxxx| Y lo
      3  |........ xxxxxxxx| Y hi
      4  |........ x.......| Disable
      4  |........ ...x....| Flip Y
      4  |........ ....x...| 1 = Y chain, 0 = X chain
      4  |........ .....xxx| Chain size
      5  |........ ??xxxxxx| Tile (low)
      6  |........ xxxxxxxx| Tile (high)
      7  |........ ....xxxx| Color Bank

    Modified table from Raine

****************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int y_offs)
{
	int offs,chain_pos;
	int x,y,curx,cury;
	int priority=0;
	UINT8 col,flipx,flipy,chain;
	UINT16 code;

	/* According to Raine, word in ioc_ram determines sprite/tile priority... */
	priority = (gcpinbal_ioc_ram[0x68/2] & 0x8800) ? 0 : 1;

	for (offs = spriteram_size/2-8;offs >= 0;offs -= 8)
	{
		code = ((spriteram16[offs+5])&0xff) + (((spriteram16[offs+6]) &0xff) << 8);
		code &= 0x3fff;

		if (!(spriteram16[offs+4] &0x80))	/* active sprite ? */
		{
			x = ((spriteram16[offs+0]) &0xff) + (((spriteram16[offs+1]) &0xff) << 8);
			y = ((spriteram16[offs+2]) &0xff) + (((spriteram16[offs+3]) &0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col  =   ((spriteram16[offs+7]) &0x0f) | 0x60;
			chain =   (spriteram16[offs+4]) &0x07;
			flipy =   (spriteram16[offs+4]) &0x10;
			flipx = 0;

			curx = x;
			cury = y;

			if (((spriteram16[offs+4]) &0x08) && flipy)
				cury += (chain * 16);

			for (chain_pos = chain;chain_pos >= 0;chain_pos--)
			{
				pdrawgfx_transpen(bitmap, cliprect,machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury,
						priority_bitmap,
						priority ? 0xfc : 0xf0,0);

				code++;

				if ((spriteram16[offs+4]) &0x08)	/* Y chain */
				{
					if (flipy)	cury -= 16;
					else cury += 16;
				}
				else	/* X chain */
				{
					curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf,"sprite rotate offs %04x ?",rotate);
		popmessage(buf);
	}
#endif
}




/**************************************************************
                        SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( gcpinbal )
{
	static UINT16 tile_sets = 0;
	int i;
	UINT8 layer[3];

#ifdef MAME_DEBUG
	static UINT8 dislayer[4];
#endif

#ifdef MAME_DEBUG
	if (input_code_pressed_once (KEYCODE_V))
	{
		dislayer[0] ^= 1;
		popmessage("bg0: %01x",dislayer[0]);
	}

	if (input_code_pressed_once (KEYCODE_B))
	{
		dislayer[1] ^= 1;
		popmessage("bg1: %01x",dislayer[1]);
	}

	if (input_code_pressed_once (KEYCODE_N))
	{
		dislayer[2] ^= 1;
		popmessage("fg: %01x",dislayer[2]);
	}
#endif

	gcpinbal_scrollx[0] =  gcpinbal_ioc_ram[0x14/2];
	gcpinbal_scrolly[0] =  gcpinbal_ioc_ram[0x16/2];
	gcpinbal_scrollx[1] =  gcpinbal_ioc_ram[0x18/2];
	gcpinbal_scrolly[1] =  gcpinbal_ioc_ram[0x1a/2];
	gcpinbal_scrollx[2] =  gcpinbal_ioc_ram[0x1c/2];
	gcpinbal_scrolly[2] =  gcpinbal_ioc_ram[0x1e/2];

	tile_sets = gcpinbal_ioc_ram[0x88/2];
	bg0_gfxset = (tile_sets & 0x400) ? 0x1000 : 0;
	bg1_gfxset = (tile_sets & 0x800) ? 0x1000 : 0;

	for (i=0;i<3;i++)
	{
		tilemap_set_scrollx(gcpinbal_tilemap[i], 0, gcpinbal_scrollx[i]);
		tilemap_set_scrolly(gcpinbal_tilemap[i], 0, gcpinbal_scrolly[i]);
	}

	bitmap_fill(priority_bitmap,cliprect,0);
	bitmap_fill(bitmap, cliprect, 0);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;


#ifdef MAME_DEBUG
	if (dislayer[layer[0]]==0)
#endif
	tilemap_draw(bitmap,cliprect,gcpinbal_tilemap[layer[0]],TILEMAP_DRAW_OPAQUE,1);

#ifdef MAME_DEBUG
	if (dislayer[layer[1]]==0)
#endif
	tilemap_draw(bitmap,cliprect,gcpinbal_tilemap[layer[1]],0,2);

#ifdef MAME_DEBUG
	if (dislayer[layer[2]]==0)
#endif
	tilemap_draw(bitmap,cliprect,gcpinbal_tilemap[layer[2]],0,4);


	draw_sprites(screen->machine, bitmap,cliprect,16);

#if 0
	{
//      char buf[80];
		sprintf(buf,"bg0_gfx: %04x bg1_gfx: %04x ",bg0_gfxset,bg1_gfxset);
		popmessage(buf);
	}
#endif

	return 0;
}

