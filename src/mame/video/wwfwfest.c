/*******************************************************************************
 WWF Wrestlefest (C) 1991 Technos Japan  (video/wwfwfest.c)
********************************************************************************
 driver by David Haywood

 see (drivers/wwfwfest.c) for more notes
*******************************************************************************/

#include "driver.h"
#include "includes/wwfwfest.h"

static tilemap *fg0_tilemap, *bg0_tilemap, *bg1_tilemap;
int wwfwfest_pri;
int wwfwfest_bg0_scrollx, wwfwfest_bg0_scrolly, wwfwfest_bg1_scrollx, wwfwfest_bg1_scrolly;
UINT16 *wwfwfest_fg0_videoram, *wwfwfest_bg0_videoram, *wwfwfest_bg1_videoram;
static int sprite_xoff, bg0_dx, bg1_dx[2];

/*******************************************************************************
 Write Handlers
********************************************************************************
 for writes to Video Ram
*******************************************************************************/

WRITE16_HANDLER( wwfwfest_fg0_videoram_w )
{
	/* Videoram is 8 bit, upper & lower byte writes end up in the same place */
	if (ACCESSING_MSB && ACCESSING_LSB) {
		COMBINE_DATA(&wwfwfest_fg0_videoram[offset]);
	} else if (ACCESSING_MSB) {
		wwfwfest_fg0_videoram[offset]=(data>>8)&0xff;
	} else {
		wwfwfest_fg0_videoram[offset]=data&0xff;
	}

	tilemap_mark_tile_dirty(fg0_tilemap,offset/2);
}

WRITE16_HANDLER( wwfwfest_bg0_videoram_w )
{
	COMBINE_DATA(&wwfwfest_bg0_videoram[offset]);
	tilemap_mark_tile_dirty(bg0_tilemap,offset/2);
}

WRITE16_HANDLER( wwfwfest_bg1_videoram_w )
{
	COMBINE_DATA(&wwfwfest_bg1_videoram[offset]);
	tilemap_mark_tile_dirty(bg1_tilemap,offset);
}

/*******************************************************************************
 Tilemap Related Functions
*******************************************************************************/
static TILE_GET_INFO( get_fg0_tile_info )
{
	/*- FG0 RAM Format -**

      4 bytes per tile

      ---- ----  tttt tttt  ---- ----  ???? TTTT

      C = Colour Bank (0-15)
      T = Tile Number (0 - 4095)

      other bits unknown / unused

      basically the same as WWF Superstar's FG0 Ram but
      more of it and the used bytes the other way around

    **- End of Comments -*/

	UINT16 *tilebase;
	int tileno;
	int colbank;
	tilebase =  &wwfwfest_fg0_videoram[tile_index*2];
	tileno =  (tilebase[0] & 0x00ff) | ((tilebase[1] & 0x000f) << 8);
	colbank = (tilebase[1] & 0x00f0) >> 4;
	SET_TILE_INFO(
			0,
			tileno,
			colbank,
			0);
}

static TILE_GET_INFO( get_bg0_tile_info )
{
	/*- BG0 RAM Format -**

      4 bytes per tile

      ---- ----  fF-- CCCC  ---- TTTT tttt tttt

      C = Colour Bank (0-15)
      T = Tile Number (0 - 4095)
      f = Flip Y
      F = Flip X

      other bits unknown / unused

    **- End of Comments -*/

	UINT16 *tilebase;
	int tileno,colbank;

	tilebase =  &wwfwfest_bg0_videoram[tile_index*2];
	tileno =  (tilebase[1] & 0x0fff);
	colbank = (tilebase[0] & 0x000f);
	SET_TILE_INFO(
			2,
			tileno,
			colbank,
			TILE_FLIPYX((tilebase[0] & 0x00c0) >> 6));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	/*- BG1 RAM Format -**

      2 bytes per tile

      CCCC TTTT tttt tttt

      C = Colour Bank (0-15)
      T = Tile Number (0 - 4095)

    **- End of Comments -*/

	UINT16 *tilebase;
	int tileno;
	int colbank;
	tilebase =  &wwfwfest_bg1_videoram[tile_index];
	tileno =  (tilebase[0] & 0x0fff);
	colbank = (tilebase[0] & 0xf000) >> 12;
	SET_TILE_INFO(
			3,
			tileno,
			colbank,
			0);
}

/*******************************************************************************
 Sprite Related Functions
********************************************************************************
 sprite drawing could probably be improved a bit
*******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	/*- SPR RAM Format -**

      16 bytes per sprite

      ---- ----  yyyy yyyy  ---- ----  lllF fXYE  ---- ----  nnnn nnnn  ---- ----  NNNN NNNN
      ---- ----  ---- CCCC  ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ----

      Yy = sprite Y Position
      Xx = sprite X Position
      C  = colour bank
      f  = flip Y
      F  = flip X
      l  = chain sprite
      E  = sprite enable
      Nn = Sprite Number

      other bits unused

    **- End of Comments -*/

	const gfx_element *gfx = machine->gfx[1];
	UINT16 *source = buffered_spriteram16;
	UINT16 *finish = source + 0x2000/2;

	while( source<finish )
	{
		int xpos, ypos, colourbank, flipx, flipy, chain, enable, number, count;

		enable = (source[1] & 0x0001);

		if (enable)	{
			xpos = +(source[5] & 0x00ff) | (source[1] & 0x0004) << 6;
			if (xpos>512-16) xpos -=512;
			xpos += sprite_xoff;
			ypos = (source[0] & 0x00ff) | (source[1] & 0x0002) << 7;
			ypos = (256 - ypos) & 0x1ff;
			ypos -= 16 ;
			flipx = (source[1] & 0x0010) >> 4;
			flipy = (source[1] & 0x0008) >> 3;
			chain = (source[1] & 0x00e0) >> 5;
			chain += 1;
			number = (source[2] & 0x00ff) | (source[3] & 0x00ff) << 8;
			colourbank = (source[4] & 0x000f);

			if (flip_screen) {
				if (flipy) flipy=0; else flipy=1;
				if (flipx) flipx=0; else flipx=1;
				ypos=240-ypos-sprite_xoff;
				xpos=304-xpos;
			}

			for (count=0;count<chain;count++) {
				if (flip_screen) {
					if (!flipy) {
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos+(16*(chain-1))-(16*count),cliprect,TRANSPARENCY_PEN,0);
					} else {
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos+16*count,cliprect,TRANSPARENCY_PEN,0);
					}
				} else {
						if (flipy) {
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos-(16*(chain-1))+(16*count),cliprect,TRANSPARENCY_PEN,0);
					} else {
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos-16*count,cliprect,TRANSPARENCY_PEN,0);
					}
				}
			}
		}
	source+=8;
	}
}

/*******************************************************************************
 Video Start and Refresh Functions
********************************************************************************
 Draw Order / Priority seems to affect where the scroll values are used also.
*******************************************************************************/

VIDEO_START( wwfwfest )
{
	fg0_tilemap = tilemap_create(get_fg0_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	bg1_tilemap = tilemap_create(get_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);
	bg0_tilemap = tilemap_create(get_bg0_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);

	tilemap_set_transparent_pen(fg0_tilemap,0);
	tilemap_set_transparent_pen(bg1_tilemap,0);
	tilemap_set_transparent_pen(bg0_tilemap,0);

	sprite_xoff = bg0_dx = bg1_dx[0] = bg1_dx[1] = 0;
}

VIDEO_START( wwfwfstb )
{
	video_start_wwfwfest(machine);

	sprite_xoff = 2;
	bg0_dx = bg1_dx[0] = -4;
	bg1_dx[1] = -2;
}

VIDEO_UPDATE( wwfwfest )
{
	if (wwfwfest_pri == 0x0078) {
		tilemap_set_scrolly( bg0_tilemap, 0, wwfwfest_bg0_scrolly  );
		tilemap_set_scrollx( bg0_tilemap, 0, wwfwfest_bg0_scrollx  + bg0_dx);
		tilemap_set_scrolly( bg1_tilemap, 0, wwfwfest_bg1_scrolly  );
		tilemap_set_scrollx( bg1_tilemap, 0, wwfwfest_bg1_scrollx  + bg1_dx[0]);
	} else {
		tilemap_set_scrolly( bg1_tilemap, 0, wwfwfest_bg0_scrolly  );
		tilemap_set_scrollx( bg1_tilemap, 0, wwfwfest_bg0_scrollx  + bg1_dx[1]);
		tilemap_set_scrolly( bg0_tilemap, 0, wwfwfest_bg1_scrolly  );
		tilemap_set_scrollx( bg0_tilemap, 0, wwfwfest_bg1_scrollx  + bg0_dx);
	}

	/* todo : which bits of pri are significant to the order */

	if (wwfwfest_pri == 0x007b) {
		tilemap_draw(bitmap,cliprect,bg0_tilemap,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,fg0_tilemap,0,0);
	}

	if (wwfwfest_pri == 0x007c) {
		tilemap_draw(bitmap,cliprect,bg0_tilemap,TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,fg0_tilemap,0,0);
	}

	if (wwfwfest_pri == 0x0078) {
		tilemap_draw(bitmap,cliprect,bg1_tilemap,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,bg0_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,fg0_tilemap,0,0);
	}
	return 0;
}
