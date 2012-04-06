/* Big Striker (bootleg) Video Hardware */

#include "emu.h"
#include "includes/bigstrkb.h"


/* Sprites */

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

     16 bytes per sprite

      nnnn nnnn  nnnn nnnn  aaaa aaaa  aaaa aaaa  xxxx xxxx  xxxx xxxx  yyyy yyyy  yyyy yyyy
        ( rest unused )
    **- End of Comments -*/

	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	const gfx_element *gfx = machine.gfx[2];
	UINT16 *source = state->m_spriteram;
	UINT16 *finish = source + 0x800/2;

	while( source<finish )
	{
		int xpos, ypos, num, attr;

		int flipx, col;

		xpos = source[2];
		ypos = source[3];
		num = source[0];
		attr = source[1];

		ypos = 0xffff - ypos;


		xpos -= 126;
		ypos -= 16;

		flipx = attr & 0x0100;
		col = attr & 0x000f;

		drawgfx_transpen(bitmap,cliprect,gfx,num,col,flipx,0,xpos,ypos,15);
		source+=8;
	}
}

/* Tilemaps */

static TILEMAP_MAPPER( bsb_bg_scan )
{
	int offset;

	offset = ((col&0xf)*16) + (row&0xf);
	offset += (col >> 4) * 0x100;
	offset += (row >> 4) * 0x800;

	return offset;
}

static TILE_GET_INFO( get_bsb_tile_info )
{
	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->m_videoram[tile_index] & 0x0fff;
	col=	state->m_videoram[tile_index] & 0xf000;

	SET_TILE_INFO(0,tileno,col>>12,0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bsb_tile2_info )
{
	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->m_videoram2[tile_index] & 0x0fff;
	col=	state->m_videoram2[tile_index] & 0xf000;

	SET_TILE_INFO(1,tileno,col>>12,0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram2_w)
{
	m_videoram2[offset] = data;
	m_tilemap2->mark_tile_dirty(offset);
}


static TILE_GET_INFO( get_bsb_tile3_info )
{
	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	int tileno,col;

	tileno = state->m_videoram3[tile_index] & 0x0fff;
	col=	state->m_videoram3[tile_index] & 0xf000;

	SET_TILE_INFO(1,tileno+0x2000,(col>>12)+(0x100/16),0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram3_w)
{
	m_videoram3[offset] = data;
	m_tilemap3->mark_tile_dirty(offset);
}

/* Video Start / Update */

VIDEO_START(bigstrkb)
{
	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	state->m_tilemap = tilemap_create(machine, get_bsb_tile_info,tilemap_scan_cols, 8, 8,64,32);
	state->m_tilemap2 = tilemap_create(machine, get_bsb_tile2_info,bsb_bg_scan, 16, 16,128,64);
	state->m_tilemap3 = tilemap_create(machine, get_bsb_tile3_info,bsb_bg_scan, 16, 16,128,64);

	state->m_tilemap->set_transparent_pen(15);
	//state->m_tilemap2->set_transparent_pen(15);
	state->m_tilemap3->set_transparent_pen(15);
}

SCREEN_UPDATE_IND16(bigstrkb)
{
	bigstrkb_state *state = screen.machine().driver_data<bigstrkb_state>();

//  bitmap.fill(get_black_pen(screen.machine()), cliprect);

	state->m_tilemap2->set_scrollx(0, state->m_vidreg1[0]+(256-14));
	state->m_tilemap2->set_scrolly(0, state->m_vidreg2[0]);

	state->m_tilemap3->set_scrollx(0, state->m_vidreg1[1]+(256-14));
	state->m_tilemap3->set_scrolly(0, state->m_vidreg2[1]);

	state->m_tilemap2->draw(bitmap, cliprect, 0,0);
	state->m_tilemap3->draw(bitmap, cliprect, 0,0);

	draw_sprites(screen.machine(),bitmap,cliprect);
	state->m_tilemap->draw(bitmap, cliprect, 0,0);

//  popmessage ("Regs %08x %08x %08x %08x",bsb_vidreg2[0],bsb_vidreg2[1],bsb_vidreg2[2],bsb_vidreg2[3]);
	return 0;
}
