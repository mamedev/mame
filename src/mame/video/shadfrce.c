#include "emu.h"
#include "includes/shadfrce.h"

static TILE_GET_INFO( get_shadfrce_fgtile_info )
{

	/* ---- ----  tttt tttt  ---- ----  pppp TTTT */
	shadfrce_state *state = machine.driver_data<shadfrce_state>();
	int tileno, colour;

	tileno = (state->m_fgvideoram[tile_index *2] & 0x00ff) | ((state->m_fgvideoram[tile_index *2+1] & 0x000f) << 8);
	colour = (state->m_fgvideoram[tile_index *2+1] & 0x00f0) >>4;

	SET_TILE_INFO(0,tileno,colour*4,0);
}

WRITE16_MEMBER(shadfrce_state::shadfrce_fgvideoram_w)
{

	m_fgvideoram[offset] = data;
	m_fgtilemap->mark_tile_dirty(offset/2);
}

static TILE_GET_INFO( get_shadfrce_bg0tile_info )
{

	/* ---- ----  ---- cccc  --TT TTTT TTTT TTTT */
	shadfrce_state *state = machine.driver_data<shadfrce_state>();
	int tileno, colour,fyx;

	tileno = (state->m_bg0videoram[tile_index *2+1] & 0x3fff);
	colour = state->m_bg0videoram[tile_index *2] & 0x001f;
	if (colour & 0x10) colour ^= 0x30;	/* skip hole */
	fyx = (state->m_bg0videoram[tile_index *2] & 0x00c0) >>6;

	SET_TILE_INFO(2,tileno,colour,TILE_FLIPYX(fyx));
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg0videoram_w)
{

	m_bg0videoram[offset] = data;
	m_bg0tilemap->mark_tile_dirty(offset/2);
}

static TILE_GET_INFO( get_shadfrce_bg1tile_info )
{
	shadfrce_state *state = machine.driver_data<shadfrce_state>();
	int tileno, colour;

	tileno = (state->m_bg1videoram[tile_index] & 0x0fff);
	colour = (state->m_bg1videoram[tile_index] & 0xf000) >> 12;

	SET_TILE_INFO(2,tileno,colour+64,0);
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg1videoram_w)
{

	m_bg1videoram[offset] = data;
	m_bg1tilemap->mark_tile_dirty(offset);
}




VIDEO_START( shadfrce )
{
	shadfrce_state *state = machine.driver_data<shadfrce_state>();

	state->m_fgtilemap = tilemap_create(machine, get_shadfrce_fgtile_info,tilemap_scan_rows,    8,  8,64,32);
	state->m_fgtilemap->set_transparent_pen(0);

	state->m_bg0tilemap = tilemap_create(machine, get_shadfrce_bg0tile_info,tilemap_scan_rows, 16, 16,32,32);
	state->m_bg0tilemap->set_transparent_pen(0);

	state->m_bg1tilemap = tilemap_create(machine, get_shadfrce_bg1tile_info,tilemap_scan_rows, 16, 16,32,32);

	state->m_spvideoram_old = auto_alloc_array(machine, UINT16, state->m_spvideoram_size/2);
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg0scrollx_w)
{

	m_bg0tilemap->set_scrollx(0, data & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg0scrolly_w)
{

	m_bg0tilemap->set_scrolly(0, data  & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg1scrollx_w)
{

	m_bg1tilemap->set_scrollx(0, data  & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::shadfrce_bg1scrolly_w)
{

	m_bg1tilemap->set_scrolly(0, data & 0x1ff );
}




static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{

	/* | ---- ---- hhhf Fe-Y | ---- ---- yyyy yyyy | ---- ---- TTTT TTTT | ---- ---- tttt tttt |
       | ---- ---- -pCc cccX | ---- ---- xxxx xxxx | ---- ---- ---- ---- | ---- ---- ---- ---- | */

	/* h  = height
       f  = flipx
       F  = flipy
       e  = enable
       Yy = Y Position
       Tt = Tile No.
       Xx = X Position
       Cc = color
       P = priority
    */

	shadfrce_state *state = machine.driver_data<shadfrce_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT16 *finish = state->m_spvideoram_old;
	UINT16 *source = finish + 0x2000/2 - 8;
	int hcount;
	while( source>=finish )
	{
		int ypos = 0x100 - (((source[0] & 0x0003) << 8) | (source[1] & 0x00ff));
		int xpos = (((source[4] & 0x0001) << 8) | (source[5] & 0x00ff)) + 1;
		int tile = ((source[2] & 0x00ff) << 8) | (source[3] & 0x00ff);
		int height = (source[0] & 0x00e0) >> 5;
		int enable = ((source[0] & 0x0004));
		int flipx = ((source[0] & 0x0010) >> 4);
		int flipy = ((source[0] & 0x0008) >> 3);
		int pal = ((source[4] & 0x003e));
		int pri_mask = (source[4] & 0x0040) ? 0x02 : 0x00;

		if (pal & 0x20) pal ^= 0x60;	/* skip hole */

		height++;
		if (enable)	{
			for (hcount=0;hcount<height;hcount++) {
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16,machine.priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16,machine.priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16+0x200,machine.priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16+0x200,machine.priority_bitmap,pri_mask,0);
			}
		}
		source-=8;
	}
}

SCREEN_UPDATE_IND16( shadfrce )
{
	shadfrce_state *state = screen.machine().driver_data<shadfrce_state>();
	screen.machine().priority_bitmap.fill(0, cliprect);

	if (state->m_video_enable)
	{
		state->m_bg1tilemap->draw(bitmap, cliprect, 0,0);
		state->m_bg0tilemap->draw(bitmap, cliprect, 0,1);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_fgtilemap->draw(bitmap, cliprect, 0,0);
	}
	else
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
	}

	return 0;
}

SCREEN_VBLANK( shadfrce )
{
	// rising edge
	if (vblank_on)
	{
		shadfrce_state *state = screen.machine().driver_data<shadfrce_state>();

		/* looks like sprites are *two* frames ahead */
		memcpy(state->m_spvideoram_old, state->m_spvideoram, state->m_spvideoram_size);
	}
}
