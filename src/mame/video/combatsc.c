/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/combatsc.h"

PALETTE_INIT( combatsc )
{
	int pal;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		int i, clut;

		switch (pal)
		{
			default:
			case 0: /* other sprites */
			case 2: /* other sprites(alt) */
			clut = 1;	/* 0 is wrong for Firing Range III targets */
			break;

			case 4: /* player sprites */
			case 6: /* player sprites(alt) */
			clut = 2;
			break;

			case 1: /* background */
			case 3: /* background(alt) */
			clut = 1;
			break;

			case 5: /* foreground tiles */
			case 7: /* foreground tiles(alt) */
			clut = 3;
			break;
		}

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
				ctabentry = 0;
			else
				ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

			colortable_entry_set_value(machine.colortable, (pal << 8) | i, ctabentry);
		}
	}
}


PALETTE_INIT( combatscb )
{
	int pal;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		int i;

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if ((pal & 1) == 0)
				/* sprites */
				ctabentry = (pal << 4) | (~color_prom[i] & 0x0f);
			else
				/* chars - no lookup? */
				ctabentry = (pal << 4) | (i & 0x0f);	/* no lookup? */

			colortable_entry_set_value(machine.colortable, (pal << 8) | i, ctabentry);
		}
	}
}


static void set_pens( running_machine &machine )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->m_paletteram[i] | (state->m_paletteram[i | 1] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	UINT8 ctrl_6 = k007121_ctrlram_r(state->m_k007121_1, 6);
	UINT8 attributes = state->m_page[0][tile_index];
	int bank = 4 * ((state->m_vreg & 0x0f) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16) + (attributes & 0x0f);

	number = state->m_page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

static TILE_GET_INFO( get_tile_info1 )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	UINT8 ctrl_6 = k007121_ctrlram_r(state->m_k007121_2, 6);
	UINT8 attributes = state->m_page[1][tile_index];
	int bank = 4 * ((state->m_vreg >> 4) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16 + 4 * 16) + (attributes & 0x0f);

	number = state->m_page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

static TILE_GET_INFO( get_text_info )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	UINT8 attributes = state->m_page[0][tile_index + 0x800];
	int number = state->m_page[0][tile_index + 0xc00];
	int color = 16 + (attributes & 0x0f);

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
}


static TILE_GET_INFO( get_tile_info0_bootleg )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	UINT8 attributes = state->m_page[0][tile_index];
	int bank = 4 * ((state->m_vreg & 0x0f) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 1 : 3;
	color = pal*16;// + (attributes & 0x0f);
	number = state->m_page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info1_bootleg )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	UINT8 attributes = state->m_page[1][tile_index];
	int bank = 4*((state->m_vreg >> 4) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 5 : 7;
	color = pal * 16;// + (attributes & 0x0f);
	number = state->m_page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
}

static TILE_GET_INFO( get_text_info_bootleg )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
//  UINT8 attributes = state->m_page[0][tile_index + 0x800];
	int number = state->m_page[0][tile_index + 0xc00];
	int color = 16;// + (attributes & 0x0f);

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( combatsc )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();

	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_textlayer =  tilemap_create(machine, get_text_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_spriteram[0] = auto_alloc_array_clear(machine, UINT8, 0x800);
	state->m_spriteram[1] = auto_alloc_array_clear(machine, UINT8, 0x800);

	state->m_bg_tilemap[0]->set_transparent_pen(0);
	state->m_bg_tilemap[1]->set_transparent_pen(0);
	state->m_textlayer->set_transparent_pen(0);

	state->m_textlayer->set_scroll_rows(32);

	state->save_pointer(NAME(state->m_spriteram[0]), 0x800);
	state->save_pointer(NAME(state->m_spriteram[1]), 0x800);
}

VIDEO_START( combatscb )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();

	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info0_bootleg, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info1_bootleg, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_textlayer = tilemap_create(machine, get_text_info_bootleg, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_spriteram[0] = auto_alloc_array_clear(machine, UINT8, 0x800);
	state->m_spriteram[1] = auto_alloc_array_clear(machine, UINT8, 0x800);

	state->m_bg_tilemap[0]->set_transparent_pen(0);
	state->m_bg_tilemap[1]->set_transparent_pen(0);
	state->m_textlayer->set_transparent_pen(0);

	state->m_bg_tilemap[0]->set_scroll_rows(32);
	state->m_bg_tilemap[1]->set_scroll_rows(32);

	state->save_pointer(NAME(state->m_spriteram[0]), 0x800);
	state->save_pointer(NAME(state->m_spriteram[1]), 0x800);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_MEMBER(combatsc_state::combatsc_video_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(combatsc_state::combatsc_video_w)
{
	m_videoram[offset] = data;

	if (offset < 0x800)
	{
		if (m_video_circuit)
			m_bg_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
		else
			m_bg_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
	}
	else if (offset < 0x1000 && m_video_circuit == 0)
	{
		m_textlayer->mark_tile_dirty(offset & 0x3ff);
	}
}

WRITE8_MEMBER(combatsc_state::combatsc_pf_control_w)
{
	device_t *k007121 = m_video_circuit ? m_k007121_2 : m_k007121_1;
	k007121_ctrl_w(k007121, offset, data);

	if (offset == 7)
		m_bg_tilemap[m_video_circuit]->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (offset == 3)
	{
		if (data & 0x08)
			memcpy(m_spriteram[m_video_circuit], m_page[m_video_circuit] + 0x1000, 0x800);
		else
			memcpy(m_spriteram[m_video_circuit], m_page[m_video_circuit] + 0x1800, 0x800);
	}
}

READ8_MEMBER(combatsc_state::combatsc_scrollram_r)
{
	return m_scrollram[offset];
}

WRITE8_MEMBER(combatsc_state::combatsc_scrollram_w)
{
	m_scrollram[offset] = data;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit, UINT32 pri_mask )
{
	combatsc_state *state = machine.driver_data<combatsc_state>();
	device_t *k007121 = circuit ? state->m_k007121_2 : state->m_k007121_1;
	int base_color = (circuit * 4) * 16 + (k007121_ctrlram_r(k007121, 6) & 0x10) * 2;

	k007121_sprites_draw(k007121, bitmap, cliprect, machine.gfx[circuit], machine.colortable, source, base_color, 0, 0, pri_mask);
}


SCREEN_UPDATE_IND16( combatsc )
{
	combatsc_state *state = screen.machine().driver_data<combatsc_state>();
	int i;

	set_pens(screen.machine());

	if (k007121_ctrlram_r(state->m_k007121_1, 1) & 0x02)
	{
		state->m_bg_tilemap[0]->set_scroll_rows(32);
		for (i = 0; i < 32; i++)
			state->m_bg_tilemap[0]->set_scrollx(i, state->m_scrollram0[i]);
	}
	else
	{
		state->m_bg_tilemap[0]->set_scroll_rows(1);
		state->m_bg_tilemap[0]->set_scrollx(0, k007121_ctrlram_r(state->m_k007121_1, 0) | ((k007121_ctrlram_r(state->m_k007121_1, 1) & 0x01) << 8));
	}

	if (k007121_ctrlram_r(state->m_k007121_2, 1) & 0x02)
	{
		state->m_bg_tilemap[1]->set_scroll_rows(32);
		for (i = 0; i < 32; i++)
			state->m_bg_tilemap[1]->set_scrollx(i, state->m_scrollram1[i]);
	}
	else
	{
		state->m_bg_tilemap[1]->set_scroll_rows(1);
		state->m_bg_tilemap[1]->set_scrollx(0, k007121_ctrlram_r(state->m_k007121_2, 0) | ((k007121_ctrlram_r(state->m_k007121_2, 1) & 0x01) << 8));
	}

	state->m_bg_tilemap[0]->set_scrolly(0, k007121_ctrlram_r(state->m_k007121_1, 2));
	state->m_bg_tilemap[1]->set_scrolly(0, k007121_ctrlram_r(state->m_k007121_2, 2));

	screen.machine().priority_bitmap.fill(0, cliprect);

	if (state->m_priority == 0)
	{
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 4);
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 8);
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, 0, 1);
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, 1, 2);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram[1], 1, 0x0f00);
		draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram[0], 0, 0x4444);
	}
	else
	{
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 1);
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 2);
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, 1, 4);
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, 0, 8);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram[1], 1, 0x0f00);
		draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram[0], 0, 0x4444);
	}

	if (k007121_ctrlram_r(state->m_k007121_1, 1) & 0x08)
	{
		for (i = 0; i < 32; i++)
		{
			state->m_textlayer->set_scrollx(i, state->m_scrollram0[0x20 + i] ? 0 : TILE_LINE_DISABLED);
			state->m_textlayer->draw(bitmap, cliprect, 0, 0);
		}
	}

	/* chop the extreme columns if necessary */
	if (k007121_ctrlram_r(state->m_k007121_1, 3) & 0x40)
	{
		rectangle clip;

		clip = cliprect;
		clip.max_x = clip.min_x + 7;
		bitmap.fill(0, clip);

		clip = cliprect;
		clip.min_x = clip.max_x - 7;
		bitmap.fill(0, clip);
	}
	return 0;
}








/***************************************************************************

    bootleg Combat School sprites. Each sprite has 5 bytes:

byte #0:    sprite number
byte #1:    y position
byte #2:    x position
byte #3:
    bit 0:      x position (bit 0)
    bits 1..3:  ???
    bit 4:      flip x
    bit 5:      unused?
    bit 6:      sprite bank # (bit 2)
    bit 7:      ???
byte #4:
    bits 0,1:   sprite bank # (bits 0 & 1)
    bits 2,3:   unused?
    bits 4..7:  sprite color

***************************************************************************/

static void bootleg_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	const gfx_element *gfx = machine.gfx[circuit + 2];

	int limit = circuit ? (space->read_byte(0xc2) * 256 + space->read_byte(0xc3)) : (space->read_byte(0xc0) * 256 + space->read_byte(0xc1));
	const UINT8 *finish;

	source += 0x1000;
	finish = source;
	source += 0x400;
	limit = (0x3400 - limit) / 8;
	if (limit >= 0)
		finish = source - limit * 8;
	source -= 8;

	while (source > finish)
	{
		UINT8 attributes = source[3]; /* PBxF ?xxX */
		{
			int number = source[0];
			int x = source[2] - 71 + (attributes & 0x01)*256;
			int y = 242 - source[1];
			UINT8 color = source[4]; /* CCCC xxBB */

			int bank = (color & 0x03) | ((attributes & 0x40) >> 4);

			number = ((number & 0x02) << 1) | ((number & 0x04) >> 1) | (number & (~6));
			number += 256 * bank;

			color = (circuit * 4) * 16 + (color >> 4);

			/*  hacks to select alternate palettes */
//          if(state->m_vreg == 0x40 && (attributes & 0x40)) color += 1*16;
//          if(state->m_vreg == 0x23 && (attributes & 0x02)) color += 1*16;
//          if(state->m_vreg == 0x66 ) color += 2*16;

			drawgfx_transpen(	bitmap, cliprect, gfx,
							number, color,
							attributes & 0x10,0, /* flip */
							x, y, 15 );
		}
		source -= 8;
	}
}

SCREEN_UPDATE_IND16( combatscb )
{
	combatsc_state *state = screen.machine().driver_data<combatsc_state>();
	int i;

	set_pens(screen.machine());

	for (i = 0; i < 32; i++)
	{
		state->m_bg_tilemap[0]->set_scrollx(i, state->m_io_ram[0x040 + i] + 5);
		state->m_bg_tilemap[1]->set_scrollx(i, state->m_io_ram[0x060 + i] + 3);
	}
	state->m_bg_tilemap[0]->set_scrolly(0, state->m_io_ram[0x000] + 1);
	state->m_bg_tilemap[1]->set_scrolly(0, state->m_io_ram[0x020] + 1);

	if (state->m_priority == 0)
	{
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		bootleg_draw_sprites(screen.machine(), bitmap,cliprect, state->m_page[0], 0);
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, 0 ,0);
		bootleg_draw_sprites(screen.machine(), bitmap,cliprect, state->m_page[1], 1);
	}
	else
	{
		state->m_bg_tilemap[0]->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		bootleg_draw_sprites(screen.machine(), bitmap,cliprect, state->m_page[0], 0);
		state->m_bg_tilemap[1]->draw(bitmap, cliprect, 0, 0);
		bootleg_draw_sprites(screen.machine(), bitmap,cliprect, state->m_page[1], 1);
	}

	state->m_textlayer->draw(bitmap, cliprect, 0, 0);
	return 0;
}
