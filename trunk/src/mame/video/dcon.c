/***************************************************************************

    D-Con video hardware.

***************************************************************************/

#include "emu.h"
#include "includes/dcon.h"


/******************************************************************************/

READ16_MEMBER(dcon_state::dcon_control_r)
{
	return m_enable;
}

WRITE16_MEMBER(dcon_state::dcon_control_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_enable=data;
		if ((m_enable&4)==4)
			m_foreground_layer->enable(0);
		else
			m_foreground_layer->enable(1);

		if ((m_enable&2)==2)
			m_midground_layer->enable(0);
		else
			m_midground_layer->enable(1);

		if ((m_enable&1)==1)
			m_background_layer->enable(0);
		else
			m_background_layer->enable(1);
	}
}

WRITE16_MEMBER(dcon_state::dcon_gfxbank_w)
{
	if (data&1)
		m_gfx_bank_select=0x1000;
	else
		m_gfx_bank_select=0;
}

WRITE16_MEMBER(dcon_state::dcon_background_w)
{
	COMBINE_DATA(&m_back_data[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::dcon_foreground_w)
{
	COMBINE_DATA(&m_fore_data[offset]);
	m_foreground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::dcon_midground_w)
{
	COMBINE_DATA(&m_mid_data[offset]);
	m_midground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(dcon_state::dcon_text_w)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile=state->m_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			3,
			tile|state->m_gfx_bank_select,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	int tile = state->m_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( dcon )
{
	dcon_state *state = machine.driver_data<dcon_state>();
	state->m_background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,     16,16,32,32);
	state->m_foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	state->m_midground_layer =  tilemap_create(machine, get_mid_tile_info, tilemap_scan_rows,16,16,32,32);
	state->m_text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	state->m_midground_layer->set_transparent_pen(15);
	state->m_foreground_layer->set_transparent_pen(15);
	state->m_text_layer->set_transparent_pen(15);

	state->m_gfx_bank_select = 0;
}

static void draw_sprites(running_machine& machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	dcon_state *state = machine.driver_data<dcon_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay,inc,pri_mask = 0;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];

		switch((sprite>>14) & 3)
		{
		case 0: pri_mask = 0xf0; // above foreground layer
			break;
		case 1: pri_mask = 0xfc; // above midground layer
			break;
		case 2: pri_mask = 0xfe; // above background layer
			break;
		case 3: pri_mask = 0; // above text layer
			break;
		}

		sprite &= 0x3fff;

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		else y&=0x1ff;

		color = spriteram16[offs+0]&0x3f;
		fx = spriteram16[offs+0]&0x4000;
		fy = spriteram16[offs+0]&0x2000;
		dy=((spriteram16[offs+0]&0x0380)>>7)+1;
		dx=((spriteram16[offs+0]&0x1c00)>>10)+1;

		inc = 0;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx && !fy)
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else if (fx && !fy)
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else if (!fx && fy)
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}
				else
				{
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 + 512,
						machine.priority_bitmap,pri_mask,15);

					// wrap around y
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 - 512,
						machine.priority_bitmap,pri_mask,15);
				}

				inc++;
			}
	}
}

SCREEN_UPDATE_IND16( dcon )
{
	dcon_state *state = screen.machine().driver_data<dcon_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	/* Setup the tilemaps */
	state->m_background_layer->set_scrollx(0, state->m_scroll_ram[0] );
	state->m_background_layer->set_scrolly(0, state->m_scroll_ram[1] );
	state->m_midground_layer->set_scrollx(0, state->m_scroll_ram[2] );
	state->m_midground_layer->set_scrolly(0, state->m_scroll_ram[3] );
	state->m_foreground_layer->set_scrollx(0, state->m_scroll_ram[4] );
	state->m_foreground_layer->set_scrolly(0, state->m_scroll_ram[5] );

	if ((state->m_enable&1)!=1)
		state->m_background_layer->draw(bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	state->m_midground_layer->draw(bitmap, cliprect, 0,1);
	state->m_foreground_layer->draw(bitmap, cliprect, 0,2);
	state->m_text_layer->draw(bitmap, cliprect, 0,4);

	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( sdgndmps )
{
	dcon_state *state = screen.machine().driver_data<dcon_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	/* Gfx banking */
	if (state->m_last_gfx_bank!=state->m_gfx_bank_select)
	{
		state->m_midground_layer->mark_all_dirty();
		state->m_last_gfx_bank=state->m_gfx_bank_select;
	}

	/* Setup the tilemaps */
	state->m_background_layer->set_scrollx(0, state->m_scroll_ram[0]+128 );
	state->m_background_layer->set_scrolly(0, state->m_scroll_ram[1] );
	state->m_midground_layer->set_scrollx(0, state->m_scroll_ram[2]+128 );
	state->m_midground_layer->set_scrolly(0, state->m_scroll_ram[3] );
	state->m_foreground_layer->set_scrollx(0, state->m_scroll_ram[4]+128 );
	state->m_foreground_layer->set_scrolly(0, state->m_scroll_ram[5] );
	state->m_text_layer->set_scrollx(0, /*state->m_scroll_ram[6] + */ 128 );
	state->m_text_layer->set_scrolly(0, /*state->m_scroll_ram[7] + */ 0 );

	if ((state->m_enable&1)!=1)
		state->m_background_layer->draw(bitmap, cliprect, 0,0);
	else
		bitmap.fill(15, cliprect); /* Should always be black, not pen 15 */

	state->m_midground_layer->draw(bitmap, cliprect, 0,1);
	state->m_foreground_layer->draw(bitmap, cliprect, 0,2);
	state->m_text_layer->draw(bitmap, cliprect, 0,4);

	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}
