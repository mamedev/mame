/***************************************************************************

  Video Hardware for Championship V'ball by Paul Hampson
  Generally copied from China Gate by Paul Hampson
  "Mainly copied from video of Double Dragon (bootleg) & Double Dragon II"

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/vball.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) <<6);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	vball_state *state = machine.driver_data<vball_state>();
	UINT8 code = state->m_vb_videoram[tile_index];
	UINT8 attr = state->m_vb_attribram[tile_index];
	SET_TILE_INFO(
			0,
			code + ((attr & 0x1f) << 8) + (state->m_gfxset<<8),
			(attr >> 5) & 0x7,
			0);
}


VIDEO_START( vb )
{
	vball_state *state = machine.driver_data<vball_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,background_scan, 8, 8,64,64);

	state->m_bg_tilemap->set_scroll_rows(32);
	state->m_gfxset=0;
	state->m_vb_bgprombank=0xff;
	state->m_vb_spprombank=0xff;
}

WRITE8_MEMBER(vball_state::vb_videoram_w)
{
	m_vb_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

#ifdef UNUSED_FUNCTION
READ8_MEMBER(vball_state::vb_attrib_r)
{
	return m_vb_attribram[offset];
}
#endif

WRITE8_MEMBER(vball_state::vb_attrib_w)
{
	m_vb_attribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void vb_bgprombank_w( running_machine &machine, int bank )
{
	vball_state *state = machine.driver_data<vball_state>();
	int i;
	UINT8* color_prom;

	if (bank==state->m_vb_bgprombank) return;

	color_prom = machine.region("proms")->base() + bank*0x80;
	for (i=0;i<128;i++, color_prom++) {
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
				       pal4bit(color_prom[0x800] >> 0));
	}
	state->m_vb_bgprombank=bank;
}

void vb_spprombank_w( running_machine &machine, int bank )
{
	vball_state *state = machine.driver_data<vball_state>();

	int i;
	UINT8* color_prom;

	if (bank==state->m_vb_spprombank) return;

	color_prom = machine.region("proms")->base()+0x400 + bank*0x80;
	for (i=128;i<256;i++,color_prom++)	{
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
				       pal4bit(color_prom[0x800] >> 0));
	}
	state->m_vb_spprombank=bank;
}

void vb_mark_all_dirty( running_machine &machine )
{
	vball_state *state = machine.driver_data<vball_state>();
	state->m_bg_tilemap->mark_all_dirty();
}

#define DRAW_SPRITE( order, sx, sy ) drawgfx_transpen( bitmap, \
					cliprect,gfx, \
					(which+order),color,flipx,flipy,sx,sy,0);

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	vball_state *state = machine.driver_data<vball_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT8 *src = state->m_spriteram;
	int i;

/*  240-Y    S|X|CLR|WCH WHICH    240-X
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (i = 0;i < state->m_spriteram_size;i += 4)
	{
		int attr = src[i+1];
		int which = src[i+2]+((attr & 0x07)<<8);
		int sx = ((src[i+3] + 8) & 0xff) - 7;
		int sy = 240 - src[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		switch (size)
		{
			case 0: /* normal */
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			DRAW_SPRITE(0,sx,sy + dy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE

SCREEN_UPDATE_IND16( vb )
{
	vball_state *state = screen.machine().driver_data<vball_state>();
	int i;

	state->m_bg_tilemap->set_scrolly(0,state->m_vb_scrolly_hi + *state->m_vb_scrolly_lo);

	/*To get linescrolling to work properly, we must ignore the 1st two scroll values, no idea why! -SJE */
	for (i = 2; i < 256; i++) {
		state->m_bg_tilemap->set_scrollx(i,state->m_vb_scrollx[i-2]);
		//logerror("scrollx[%d] = %d\n",i,state->m_vb_scrollx[i]);
	}
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}
