/***************************************************************************

  Gaelco Type 1 Video Hardware Rev B

  The video hardware it's nearly identical to the previous
  revision but it can handle more tiles and more sprites

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/thoop2.h"


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- ------xx | code (high bits)
      0  | xxxxxxxx xxxxxx-- | code (low bits)
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | --xxxxxx -------- | not used
      1  | -x------ -------- | flip x
      1  | x------- -------- | flip y
*/

static TILE_GET_INFO( get_tile_info_thoop2_screen0 )
{
	thoop2_state *state = machine.driver_data<thoop2_state>();
	int data = state->m_videoram[tile_index << 1];
	int data2 = state->m_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}


static TILE_GET_INFO( get_tile_info_thoop2_screen1 )
{
	thoop2_state *state = machine.driver_data<thoop2_state>();
	int data = state->m_videoram[(0x1000/2) + (tile_index << 1)];
	int data2 = state->m_videoram[(0x1000/2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(thoop2_state::thoop2_vram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( thoop2 )
{
	thoop2_state *state = machine.driver_data<thoop2_state>();
	int i;

	state->m_pant[0] = tilemap_create(machine, get_tile_info_thoop2_screen0,tilemap_scan_rows,16,16,32,32);
	state->m_pant[1] = tilemap_create(machine, get_tile_info_thoop2_screen1,tilemap_scan_rows,16,16,32,32);

	state->m_pant[0]->set_transmask(0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	state->m_pant[1]->set_transmask(0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */

	for (i = 0; i < 5; i++){
		state->m_sprite_table[i] = auto_alloc_array(machine, int, 512);
	}
}

/***************************************************************************

    Sprites

***************************************************************************/

static void thoop2_sort_sprites(running_machine &machine)
{
	thoop2_state *state = machine.driver_data<thoop2_state>();
	int i;

	state->m_sprite_count[0] = 0;
	state->m_sprite_count[1] = 0;
	state->m_sprite_count[2] = 0;
	state->m_sprite_count[3] = 0;
	state->m_sprite_count[4] = 0;

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int color = (state->m_spriteram[i+2] & 0x7e00) >> 9;
		int priority = (state->m_spriteram[i] & 0x3000) >> 12;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38){
			state->m_sprite_table[4][state->m_sprite_count[4]] = i;
			state->m_sprite_count[4]++;
		}

		/* save sprite number in the proper array for later */
		state->m_sprite_table[priority][state->m_sprite_count[priority]] = i;
		state->m_sprite_count[priority]++;
	}
}

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (high bits)
      3  | xxxxxxxx xxxxxx-- | sprite code (low bits)
*/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	thoop2_state *state = machine.driver_data<thoop2_state>();
	int j, x, y, ex, ey;
	const gfx_element *gfx = machine.gfx[0];

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (j = 0; j < state->m_sprite_count[pri]; j++){
		int i = state->m_sprite_table[pri][j];
		int sx = state->m_spriteram[i+2] & 0x01ff;
		int sy = (240 - (state->m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = state->m_spriteram[i+3];
		int color = (state->m_spriteram[i+2] & 0x7e00) >> 9;
		int attr = (state->m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		number |= ((number & 0x03) << 16);

		if (attr & 0x04){
			spr_size = 1;
		}
		else{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){

				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;

				drawgfx_transpen(bitmap,cliprect,gfx,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( thoop2 )
{
	thoop2_state *state = screen.machine().driver_data<thoop2_state>();
	/* set scroll registers */
	state->m_pant[0]->set_scrolly(0, state->m_vregs[0]);
	state->m_pant[0]->set_scrollx(0, state->m_vregs[1]+4);
	state->m_pant[1]->set_scrolly(0, state->m_vregs[2]);
	state->m_pant[1]->set_scrollx(0, state->m_vregs[3]);

	thoop2_sort_sprites(screen.machine());

	bitmap.fill(0, cliprect );

	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3,0);
	draw_sprites(screen.machine(), bitmap,cliprect,3);
	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3,0);

	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2,0);
	draw_sprites(screen.machine(), bitmap,cliprect,2);
	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2,0);

	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1,0);
	draw_sprites(screen.machine(), bitmap,cliprect,1);
	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1,0);

	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect,0);
	state->m_pant[1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0,0);
	state->m_pant[0]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0,0);

	draw_sprites(screen.machine(), bitmap,cliprect,4);
	return 0;
}
