/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/splash.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Tilemap 0: (64*32, 8x8 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | sprite code (low 8 bits)
      0  | ----xxxx -------- | sprite code (high 4 bits)
      0  | xxxx---- -------- | color

    Tilemap 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -------x | flip y
      0  | -------- ------x- | flip x
      0  | -------- xxxxxx-- | sprite code (low 6 bits)
      0  | ----xxxx -------- | sprite code (high 4 bits)
      0  | xxxx---- -------- | color
*/

static TILE_GET_INFO( get_tile_info_splash_tilemap0 )
{
	splash_state *state = machine.driver_data<splash_state>();
	int data = state->m_videoram[tile_index];
	int attr = data >> 8;
	int code = data & 0xff;

	SET_TILE_INFO(
			0,
			code + ((0x20 + (attr & 0x0f)) << 8),
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_tile_info_splash_tilemap1 )
{
	splash_state *state = machine.driver_data<splash_state>();
	int data = state->m_videoram[(0x1000/2) + tile_index];
	int attr = data >> 8;
	int code = data & 0xff;

	SET_TILE_INFO(
			1,
			(code >> 2) + ((0x30 + (attr & 0x0f)) << 6),
			(attr & 0xf0) >> 4,
			TILE_FLIPXY(code & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(splash_state::splash_vram_w)
{

	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 1);
}

static void draw_bitmap(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	splash_state *state = machine.driver_data<splash_state>();
	int sx,sy,color,count,colxor,bitswap;
	colxor = 0; /* splash and some bitmap modes in roldfrog */
	bitswap = 0;

	if (state->m_bitmap_type == 1) /* roldfrog */
	{
		if (state->m_bitmap_mode[0] == 0x0000)
		{
			colxor = 0x7f;
		}
		else if (state->m_bitmap_mode[0] == 0x0100)
		{
			bitswap = 1;
		}
		else if (state->m_bitmap_mode[0] == 0x0200)
		{
			colxor = 0x55;
		}
		else if (state->m_bitmap_mode[0] == 0x0300)
		{
			bitswap = 2;
			colxor = 0x7f;
		}
		else if (state->m_bitmap_mode[0] == 0x0400)
		{
			bitswap = 3;
		}
		else if (state->m_bitmap_mode[0] == 0x0500)
		{
			bitswap = 4;
		}
		else if (state->m_bitmap_mode[0] == 0x0600)
		{
			bitswap = 5;
			colxor = 0x7f;
		}
		else if (state->m_bitmap_mode[0] == 0x0700)
		{
			bitswap = 6;
			colxor = 0x55;
		}
	}

	count = 0;
	for (sy=0;sy<256;sy++)
	{
		for (sx=0;sx<512;sx++)
		{
			color = state->m_pixelram[count]&0xff;
			count++;

			switch( bitswap )
			{
			case 1:
				color = BITSWAP8(color,7,0,1,2,3,4,5,6);
				break;
			case 2:
				color = BITSWAP8(color,7,4,6,5,1,0,3,2);
				break;
			case 3:
				color = BITSWAP8(color,7,3,2,1,0,6,5,4);
				break;
			case 4:
				color = BITSWAP8(color,7,6,4,2,0,5,3,1);
				break;
			case 5:
				color = BITSWAP8(color,7,0,6,5,4,3,2,1);
				break;
			case 6:
				color = BITSWAP8(color,7,4,3,2,1,0,6,5);
				break;
			}

			if (sy >= cliprect.min_y && sy <= cliprect.max_y && sx-9 >= cliprect.min_x && sx-9 <= cliprect.max_x)
				bitmap.pix16(sy, sx-9) = 0x300+(color^colxor);
		}
	}

}
/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( splash )
{
	splash_state *state = machine.driver_data<splash_state>();

	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info_splash_tilemap0, tilemap_scan_rows,  8,  8, 64, 32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info_splash_tilemap1, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_bg_tilemap[0]->set_transparent_pen(0);
	state->m_bg_tilemap[1]->set_transparent_pen(0);

	state->m_bg_tilemap[0]->set_scrollx(0, 4);
}

/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | sprite number (low 8 bits)
      0  | xxxxxxxx -------- | unused
      1  | -------- xxxxxxxx | y position
      1  | xxxxxxxx -------- | unused
      2  | -------- xxxxxxxx | x position (low 8 bits)
      2  | xxxxxxxx -------- | unused
      3  | -------- ----xxxx | sprite number (high 4 bits)
      3  | -------- --xx---- | unknown
      3  | -------- -x------ | flip x
      3  | -------- x------- | flip y
      3  | xxxxxxxx -------- | unused
      400| -------- ----xxxx | sprite color
      400| -------- -xxx---- | unknown
      400| -------- x------- | x position (high bit)
      400| xxxxxxxx -------- | unused
*/

static void splash_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	splash_state *state = machine.driver_data<splash_state>();
	int i;
	const gfx_element *gfx = machine.gfx[1];

	for (i = 0; i < 0x400; i += 4){
		int sx = state->m_spriteram[i+2] & 0xff;
		int sy = (240 - (state->m_spriteram[i+1] & 0xff)) & 0xff;
		int attr = state->m_spriteram[i+3] & 0xff;
		int attr2 = state->m_spriteram[i+0x400] >> state->m_sprite_attr2_shift;
		int number = (state->m_spriteram[i] & 0xff) + (attr & 0xf)*256;

		if (attr2 & 0x80) sx += 256;

		drawgfx_transpen(bitmap,cliprect,gfx,number,
			0x10 + (attr2 & 0x0f),attr & 0x40,attr & 0x80,
			sx-8,sy,0);
	}
}

static void funystrp_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	splash_state *state = machine.driver_data<splash_state>();
	int i;
	const gfx_element *gfx = machine.gfx[1];

	for (i = 0; i < 0x400; i += 4){
		int sx = state->m_spriteram[i+2] & 0xff;
		int sy = (240 - (state->m_spriteram[i+1] & 0xff)) & 0xff;
		int attr = state->m_spriteram[i+3] & 0xff;
		int attr2 = state->m_spriteram[i+0x400] >> state->m_sprite_attr2_shift;
		int number = (state->m_spriteram[i] & 0xff) + (attr & 0xf)*256;

		if (attr2 & 0x80) sx += 256;

		drawgfx_transpen(bitmap,cliprect,gfx,number,
			(attr2 & 0x7f),attr & 0x40,attr & 0x80,
			sx-8,sy,0);
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( splash )
{
	splash_state *state = screen.machine().driver_data<splash_state>();

	/* set scroll registers */
	state->m_bg_tilemap[0]->set_scrolly(0, state->m_vregs[0]);
	state->m_bg_tilemap[1]->set_scrolly(0, state->m_vregs[1]);

	draw_bitmap(screen.machine(), bitmap, cliprect);

	state->m_bg_tilemap[1]->draw(bitmap, cliprect, 0, 0);
	splash_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap[0]->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( funystrp )
{
	splash_state *state = screen.machine().driver_data<splash_state>();

	/* set scroll registers */
	state->m_bg_tilemap[0]->set_scrolly(0, state->m_vregs[0]);
	state->m_bg_tilemap[1]->set_scrolly(0, state->m_vregs[1]);

	draw_bitmap(screen.machine(), bitmap, cliprect);

	state->m_bg_tilemap[1]->draw(bitmap, cliprect, 0, 0);
	/*Sprite chip is similar but not the same*/
	funystrp_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap[0]->draw(bitmap, cliprect, 0, 0);
	return 0;
}
