#include "emu.h"
#include "includes/deadang.h"


/******************************************************************************/

WRITE16_MEMBER(deadang_state::deadang_foreground_w)
{
	COMBINE_DATA(&m_video_data[offset]);
	m_pf1_layer->mark_tile_dirty(offset );
}

WRITE16_MEMBER(deadang_state::deadang_text_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_text_layer->mark_tile_dirty(offset );
}

WRITE16_MEMBER(deadang_state::deadang_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_deadangle_tilebank = data&1;
		if (m_deadangle_tilebank!=m_deadangle_oldtilebank)
		{
			m_deadangle_oldtilebank = m_deadangle_tilebank;
			m_pf1_layer->mark_all_dirty();
		}
	}
}

/******************************************************************************/

static TILEMAP_MAPPER( bg_scan )
{
	return (col&0xf) | ((row&0xf)<<4) | ((col&0x70)<<4) | ((row&0xf0)<<7);
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	const UINT16 *bgMap = (const UINT16 *)machine.region("gfx6")->base();
	int code= bgMap[tile_index];
	SET_TILE_INFO(4,code&0x7ff,code>>12,0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	const UINT16 *bgMap = (const UINT16 *)machine.region("gfx7")->base();
	int code= bgMap[tile_index];
	SET_TILE_INFO(3,code&0x7ff,code>>12,0);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	deadang_state *state = machine.driver_data<deadang_state>();
	int tile=state->m_video_data[tile_index];
	int color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(2,tile+state->m_deadangle_tilebank*0x1000,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	deadang_state *state = machine.driver_data<deadang_state>();
	UINT16 *videoram = state->m_videoram;
	int tile=(videoram[tile_index] & 0xff) | ((videoram[tile_index] >> 6) & 0x300);
	int color=(videoram[tile_index] >> 8)&0xf;

	SET_TILE_INFO(0,tile,color,0);
}

VIDEO_START( deadang )
{
	deadang_state *state = machine.driver_data<deadang_state>();
	state->m_pf3_layer = tilemap_create(machine, get_pf3_tile_info,bg_scan,               16,16,128,256);
	state->m_pf2_layer = tilemap_create(machine, get_pf2_tile_info,bg_scan,          16,16,128,256);
	state->m_pf1_layer = tilemap_create(machine, get_pf1_tile_info,tilemap_scan_cols,16,16, 32, 32);
	state->m_text_layer = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows, 8, 8, 32, 32);

	state->m_pf2_layer->set_transparent_pen(15);
	state->m_pf1_layer->set_transparent_pen(15);
	state->m_text_layer->set_transparent_pen(15);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	deadang_state *state = machine.driver_data<deadang_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs,fx,fy,x,y,color,sprite,pri;

	for (offs = 0; offs<0x800/2; offs+=4)
	{
		/* Don't draw empty sprite table entries */
		if ((spriteram16[offs+3] & 0xff00)!=0xf00) continue;

		switch (spriteram16[offs+2]&0xc000) {
		default:
		case 0xc000: pri=0; break; /* Unknown */
		case 0x8000: pri=0; break; /* Over all playfields */
		case 0x4000: pri=0xf0; break; /* Under top playfield */
		case 0x0000: pri=0xf0|0xcc; break; /* Under middle playfield */
		}

		fx= spriteram16[offs+0]&0x2000;
		fy= spriteram16[offs+0]&0x4000;
		y = spriteram16[offs+0] & 0xff;
		x = spriteram16[offs+2] & 0xff;
		if (fy) fy=0; else fy=1;
		if (spriteram16[offs+2]&0x100) x=0-(0xff-x);

		color = (spriteram16[offs+1]>>12)&0xf;
		sprite = spriteram16[offs+1]&0xfff;

		if (flip_screen_get(machine)) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite,
				color,fx,fy,x,y,
				machine.priority_bitmap,pri,15);
	}
}

SCREEN_UPDATE_IND16( deadang )
{
	deadang_state *state = screen.machine().driver_data<deadang_state>();
	/* Setup the tilemaps */
	state->m_pf3_layer->set_scrolly(0, ((state->m_scroll_ram[0x01]&0xf0)<<4)+((state->m_scroll_ram[0x02]&0x7f)<<1)+((state->m_scroll_ram[0x02]&0x80)>>7) );
	state->m_pf3_layer->set_scrollx(0, ((state->m_scroll_ram[0x09]&0xf0)<<4)+((state->m_scroll_ram[0x0a]&0x7f)<<1)+((state->m_scroll_ram[0x0a]&0x80)>>7) );
	state->m_pf1_layer->set_scrolly(0, ((state->m_scroll_ram[0x11]&0x10)<<4)+((state->m_scroll_ram[0x12]&0x7f)<<1)+((state->m_scroll_ram[0x12]&0x80)>>7) );
	state->m_pf1_layer->set_scrollx(0, ((state->m_scroll_ram[0x19]&0x10)<<4)+((state->m_scroll_ram[0x1a]&0x7f)<<1)+((state->m_scroll_ram[0x1a]&0x80)>>7) );
	state->m_pf2_layer->set_scrolly(0, ((state->m_scroll_ram[0x21]&0xf0)<<4)+((state->m_scroll_ram[0x22]&0x7f)<<1)+((state->m_scroll_ram[0x22]&0x80)>>7) );
	state->m_pf2_layer->set_scrollx(0, ((state->m_scroll_ram[0x29]&0xf0)<<4)+((state->m_scroll_ram[0x2a]&0x7f)<<1)+((state->m_scroll_ram[0x2a]&0x80)>>7) );

	/* Control byte:
        0x01: Background playfield disable
        0x02: Middle playfield disable
        0x04: Top playfield disable
        0x08: ?  Toggles at start of game
        0x10: Sprite disable
        0x20: Unused?
        0x40: Flipscreen
        0x80: Always set?
    */
	state->m_pf3_layer->enable(!(state->m_scroll_ram[0x34]&1));
	state->m_pf1_layer->enable(!(state->m_scroll_ram[0x34]&2));
	state->m_pf2_layer->enable(!(state->m_scroll_ram[0x34]&4));
	flip_screen_set(screen.machine(),  state->m_scroll_ram[0x34]&0x40 );

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);
	state->m_pf3_layer->draw(bitmap, cliprect, 0,1);
	state->m_pf1_layer->draw(bitmap, cliprect, 0,2);
	state->m_pf2_layer->draw(bitmap, cliprect, 0,4);
	if (!(state->m_scroll_ram[0x34]&0x10)) draw_sprites(screen.machine(),bitmap,cliprect);
	state->m_text_layer->draw(bitmap, cliprect, 0,0);
	return 0;
}
