/*
Super Cross II (JPN Ver.)
(c)1986 GM Shoji
*/

#include "emu.h"
#include "includes/sprcros2.h"


void sprcros2_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0xb8 * bit1;
		palette_set_color(machine(),i,MAKE_RGB(r,g,b));

		colortable_palette_set_color(machine().colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* bg */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | ((color_prom[i + 0x100] & 0x0f) << 4);
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}

	/* sprites & fg */
	for (i = 0x100; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i + 0x100];
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(sprcros2_state::sprcros2_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fgtilemap->mark_tile_dirty(offset&0x3ff);
}

WRITE8_MEMBER(sprcros2_state::sprcros2_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bgtilemap->mark_tile_dirty(offset&0x3ff);
}

WRITE8_MEMBER(sprcros2_state::sprcros2_bgscrollx_w)
{
	if(m_port7&0x02)
		m_bgtilemap->set_scrollx(0, 0x100-data);
	else
		m_bgtilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(sprcros2_state::sprcros2_bgscrolly_w)
{
	m_bgtilemap->set_scrolly(0, data);
}

TILE_GET_INFO_MEMBER(sprcros2_state::get_sprcros2_bgtile_info)
{
	UINT32 tile_number = m_bgvideoram[tile_index];
	UINT8 attr = m_bgvideoram[tile_index + 0x400];

	//attr
	//76543210
	//xxxx---- colour
	//----x--- flipx
	//-----xxx tile bank

	tile_number += (attr&0x07)<<8;

	SET_TILE_INFO_MEMBER(
			0,
			tile_number,
			(attr&0xf0)>>4,
			(attr&0x08)?TILE_FLIPX:0);
}

TILE_GET_INFO_MEMBER(sprcros2_state::get_sprcros2_fgtile_info)
{
	UINT32 tile_number = m_fgvideoram[tile_index];
	UINT8 attr = m_fgvideoram[tile_index + 0x400];
	int color = (attr&0xfc)>>2;

	tileinfo.group = color;

	//attr
	//76543210
	//xxxxxx-- colour
	//------xx tile bank

	tile_number += (attr&0x03)<<8;

	SET_TILE_INFO_MEMBER(
			2,
			tile_number,
			color,
			0);
}

void sprcros2_state::video_start()
{
	m_bgtilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(sprcros2_state::get_sprcros2_bgtile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fgtilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(sprcros2_state::get_sprcros2_fgtile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	colortable_configure_tilemap_groups(machine().colortable, m_fgtilemap, machine().gfx[2], 0);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	sprcros2_state *state = machine.driver_data<sprcros2_state>();
	int offs,sx,sy,color,flipx,flipy;

	for (offs = state->m_spriteram.bytes()-4; offs >= 0; offs -= 4)
	{
		if (state->m_spriteram[offs])
		{
			//offs
			//76543210
			//x------- unused
			//-xxxxxxx sprite number
			//offs+1
			//76543210
			//xx------ unused
			//--xxx--- colour (6/7 unused and blank in prom)
			//-----x-- unused
			//------x- flipx
			//-------x unused
			//offs+2   y pos
			//offs+3   x pos

			sx = ((state->m_spriteram[offs+3]+0x10)%0x100)-0x10;
			sy = 225-(((state->m_spriteram[offs+2]+0x10)%0x100)-0x10);
			color = (state->m_spriteram[offs+1]&0x38)>>3;
			flipx = state->m_spriteram[offs+1]&0x02;
			flipy = 0;

			if (state->m_port7&0x02)
			{
				sx = 224-sx;
				sy = 224-sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap,cliprect,machine.gfx[1],
				state->m_spriteram[offs],
				color,
				flipx,flipy,
				sx,sy,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0));
		}
	}
}

UINT32 sprcros2_state::screen_update_sprcros2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bgtilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	m_fgtilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
