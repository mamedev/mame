#include "emu.h"
#include "includes/silkroad.h"

/* Sprites probably need to be delayed */
/* Some scroll layers may need to be offset slightly? */
/* Check Sprite Colours after redump */
/* Clean Up */
/* is theres a bg colour register? */

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	silkroad_state *state = machine.driver_data<silkroad_state>();
	gfx_element *gfx = machine.gfx[0];
	UINT32 *source = state->m_sprram;
	UINT32 *finish = source + 0x1000/4;

	while( source < finish )
	{

		int xpos = (source[0] & 0x01ff0000) >> 16;
		int ypos = (source[0] & 0x0000ffff);
		int tileno = (source[1] & 0xffff0000) >> 16;
		int attr = (source[1] & 0x0000ffff);
		int flipx = (attr & 0x0080);
		int width = ((attr & 0x0f00) >> 8) + 1;
		int wcount;
		int color = (attr & 0x003f) ;
		int pri		 =	((attr & 0x1000)>>12);	// Priority (1 = Low)
		int pri_mask =	~((1 << (pri+1)) - 1);	// Above the first "pri" levels

		// attr & 0x2000 -> another priority bit?

		if ( (source[1] & 0xff00) == 0xff00 ) break;

		if ( (attr & 0x8000) == 0x8000 ) tileno+=0x10000;

		if (!flipx)
		{
			for (wcount=0;wcount<width;wcount++)
			{
				pdrawgfx_transpen(bitmap,cliprect,gfx,tileno+wcount,color,0,0,xpos+wcount*16+8,ypos,machine.priority_bitmap,pri_mask,0);
			}
		}
		else
		{

			for (wcount=width;wcount>0;wcount--)
			{
				pdrawgfx_transpen(bitmap,cliprect,gfx,tileno+(width-wcount),color,1,0,xpos+wcount*16-16+8,ypos,machine.priority_bitmap,pri_mask,0);
			}
		}

		source += 2;
	}
}


TILE_GET_INFO_MEMBER(silkroad_state::get_fg_tile_info)
{
	int code = ((m_vidram[tile_index] & 0xffff0000) >> 16 );
	int color = ((m_vidram[tile_index] & 0x000001f));
	int flipx =  ((m_vidram[tile_index] & 0x0000080) >> 7);

	code += 0x18000;

	SET_TILE_INFO_MEMBER(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_MEMBER(silkroad_state::silkroad_fgram_w)
{

	COMBINE_DATA(&m_vidram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(silkroad_state::get_fg2_tile_info)
{
	int code = ((m_vidram2[tile_index] & 0xffff0000) >> 16 );
	int color = ((m_vidram2[tile_index] & 0x000001f));
	int flipx =  ((m_vidram2[tile_index] & 0x0000080) >> 7);
	code += 0x18000;
	SET_TILE_INFO_MEMBER(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_MEMBER(silkroad_state::silkroad_fgram2_w)
{

	COMBINE_DATA(&m_vidram2[offset]);
	m_fg2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(silkroad_state::get_fg3_tile_info)
{
	int code = ((m_vidram3[tile_index] & 0xffff0000) >> 16 );
	int color = ((m_vidram3[tile_index] & 0x000001f));
	int flipx =  ((m_vidram3[tile_index] & 0x0000080) >> 7);
	code += 0x18000;
	SET_TILE_INFO_MEMBER(
			0,
			code,
			color,
			TILE_FLIPYX(flipx));
}



WRITE32_MEMBER(silkroad_state::silkroad_fgram3_w)
{

	COMBINE_DATA(&m_vidram3[offset]);
	m_fg3_tilemap->mark_tile_dirty(offset);
}

VIDEO_START(silkroad)
{
	silkroad_state *state = machine.driver_data<silkroad_state>();
	state->m_fg_tilemap  = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(silkroad_state::get_fg_tile_info),state),  TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	state->m_fg2_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(silkroad_state::get_fg2_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	state->m_fg3_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(silkroad_state::get_fg3_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_fg2_tilemap->set_transparent_pen(0);
	state->m_fg3_tilemap->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16(silkroad)
{
	silkroad_state *state = screen.machine().driver_data<silkroad_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0x7c0, cliprect);

	state->m_fg_tilemap->set_scrollx(0, ((state->m_regs[0] & 0xffff0000) >> 16) );
	state->m_fg_tilemap->set_scrolly(0, (state->m_regs[0] & 0x0000ffff) >> 0 );

	state->m_fg3_tilemap->set_scrolly(0, (state->m_regs[1] & 0xffff0000) >> 16 );
	state->m_fg3_tilemap->set_scrollx(0, (state->m_regs[2] & 0xffff0000) >> 16 );

	state->m_fg2_tilemap->set_scrolly(0, ((state->m_regs[5] & 0xffff0000) >> 16));
	state->m_fg2_tilemap->set_scrollx(0, (state->m_regs[2] & 0x0000ffff) >> 0 );

	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_fg2_tilemap->draw(bitmap, cliprect, 0,1);
	state->m_fg3_tilemap->draw(bitmap, cliprect, 0,2);
	draw_sprites(screen.machine(),bitmap,cliprect);

	if (0)
	{
	    popmessage ("Regs %08x %08x %08x %08x %08x",
		state->m_regs[0],
		state->m_regs[1],
		state->m_regs[2],
		state->m_regs[4],
		state->m_regs[5]);
	}

	return 0;
}
