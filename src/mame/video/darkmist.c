#include "emu.h"
#include "includes/darkmist.h"



/* vis. flags */

#define DISPLAY_SPR		1
#define DISPLAY_FG		2 /* 2 or 8 */
#define DISPLAY_BG		4
#define DISPLAY_TXT		16


TILE_GET_INFO_MEMBER(darkmist_state::get_bgtile_info)
{
	int code,attr,pal;

	code=machine().root_device().memregion("user1")->base()[tile_index]; /* TTTTTTTT */
	attr=machine().root_device().memregion("user2")->base()[tile_index]; /* -PPP--TT - FIXED BITS (0xxx00xx) */
	code+=(attr&3)<<8;
	pal=(attr>>4);

	SET_TILE_INFO_MEMBER(
		1,
        code,
        pal,
        0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_fgtile_info)
{
	int code,attr,pal;

	code=machine().root_device().memregion("user3")->base()[tile_index]; /* TTTTTTTT */
	attr=machine().root_device().memregion("user4")->base()[tile_index]; /* -PPP--TT - FIXED BITS (0xxx00xx) */
	pal=attr>>4;

	code+=(attr&3)<<8;

	code+=0x400;

	pal+=16;

	SET_TILE_INFO_MEMBER(
		1,
        code,
        pal,
        0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_txttile_info)
{
	UINT8 *videoram = m_videoram;
	int code,attr,pal;

	code=videoram[tile_index];
	attr=videoram[tile_index+0x400];
	pal=(attr>>1);

	code+=(attr&1)<<8;

	pal+=48;

	SET_TILE_INFO_MEMBER(
		0,
        code,
        pal,
        0);
}

PALETTE_INIT(darkmist)
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x101);

	for (i = 0; i < 0x400; i++)
	{
		int ctabentry;

		if (color_prom[i] & 0x40)
			ctabentry = 0x100;
		else
		{
			ctabentry = (color_prom[i] & 0x3f);

			switch (i & 0x300)
			{
			case 0x000:  ctabentry = ctabentry | 0x80; break;
			case 0x100:  ctabentry = ctabentry | 0x00; break;
			case 0x200:  ctabentry = ctabentry | 0x40; break;
			case 0x300:  ctabentry = ctabentry | 0xc0; break;
			}
		}

		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


static void set_pens(running_machine &machine)
{
	darkmist_state *state = machine.driver_data<darkmist_state>();
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(state->m_generic_paletteram_8[i | 0x200] >> 0);
		int g = pal4bit(state->m_generic_paletteram_8[i | 0x000] >> 4);
		int b = pal4bit(state->m_generic_paletteram_8[i | 0x000] >> 0);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	colortable_palette_set_color(machine.colortable, 0x100, RGB_BLACK);
}


VIDEO_START(darkmist)
{
	darkmist_state *state = machine.driver_data<darkmist_state>();
	state->m_bgtilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(darkmist_state::get_bgtile_info),state),TILEMAP_SCAN_ROWS,16,16,512,64 );
	state->m_fgtilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(darkmist_state::get_fgtile_info),state),TILEMAP_SCAN_ROWS,16,16,64,256 );
	state->m_txtilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(darkmist_state::get_txttile_info),state),TILEMAP_SCAN_ROWS,8,8,32,32 );
	state->m_fgtilemap->set_transparent_pen(0);
	state->m_txtilemap->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16( darkmist)
{
	darkmist_state *state = screen.machine().driver_data<darkmist_state>();
	UINT8 *spriteram = state->m_spriteram;

#define DM_GETSCROLL(n) (((state->m_scroll[(n)]<<1)&0xff) + ((state->m_scroll[(n)]&0x80)?1:0) +( ((state->m_scroll[(n)-1]<<4) | (state->m_scroll[(n)-1]<<12) )&0xff00))

	set_pens(screen.machine());

	state->m_bgtilemap->set_scrollx(0, DM_GETSCROLL(0x2));
	state->m_bgtilemap->set_scrolly(0, DM_GETSCROLL(0x6));
	state->m_fgtilemap->set_scrollx(0, DM_GETSCROLL(0xa));
	state->m_fgtilemap->set_scrolly(0, DM_GETSCROLL(0xe));

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if(state->m_hw & DISPLAY_BG)
		state->m_bgtilemap->draw(bitmap, cliprect, 0,0);

	if(state->m_hw & DISPLAY_FG)
		state->m_fgtilemap->draw(bitmap, cliprect, 0,0);

	if(state->m_hw & DISPLAY_SPR)
	{
/*
    Sprites

    76543210
0 - TTTT TTTT - tile
1 - xyBP PPP? - palette (P), flips (x,y), B - use spritebank,
                ? - unknown, according to gamecode top bit of one of coords(y/x)
2 - YYYY YYYY - y coord
3 - XXXX XXXX - x coord

*/
		int i,fx,fy,tile,palette;
		for(i=0;i<state->m_spriteram.bytes();i+=32)
		{
			fy=spriteram[i+1]&0x40;
			fx=spriteram[i+1]&0x80;

			tile=spriteram[i+0];

			if(spriteram[i+1]&0x20)
				tile += (*state->m_spritebank << 8);

			palette=((spriteram[i+1])>>1)&0xf;

			if(spriteram[i+1]&0x1)
				palette=screen.machine().rand()&15;

			palette+=32;

			drawgfx_transpen(
				bitmap,cliprect,
				screen.machine().gfx[2],
				tile,
				palette,
				fx,fy,
				spriteram[i+3],spriteram[i+2],0 );
		}
	}

	if(state->m_hw & DISPLAY_TXT)
	{
		state->m_txtilemap->mark_all_dirty();
		state->m_txtilemap->draw(bitmap, cliprect, 0,0);
	}


	return 0;
}
