/***************************************************************************
  Psychic 5
  Bombs Away

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "jalblend.h"
#include "includes/psychic5.h"

#define	BG_SCROLLX_LSB		0x308
#define	BG_SCROLLX_MSB		0x309
#define	BG_SCROLLY_LSB		0x30a
#define	BG_SCROLLY_MSB		0x30b
#define	BG_SCREEN_MODE		0x30c
#define	BG_PAL_INTENSITY_RG	0x1fe
#define	BG_PAL_INTENSITY_BU	0x1ff


/***************************************************************************
  Palette color
***************************************************************************/

static void psychic5_change_palette(running_machine &machine, int color, int offset)
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	UINT8 lo = state->m_ps5_palette_ram[offset & ~1];
	UINT8 hi = state->m_ps5_palette_ram[offset | 1];
	jal_blend_set(color, hi & 0x0f);
	palette_set_color_rgb(machine, color, pal4bit(lo >> 4), pal4bit(lo), pal4bit(hi >> 4));
}

static void psychic5_change_bg_palette(running_machine &machine, int color, int lo_offs, int hi_offs)
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	UINT8 r,g,b,lo,hi,ir,ig,ib,ix;
	rgb_t irgb;

	/* red,green,blue intensities */
	ir = pal4bit(state->m_palette_intensity >> 12);
	ig = pal4bit(state->m_palette_intensity >>  8);
	ib = pal4bit(state->m_palette_intensity >>  4);
	ix = state->m_palette_intensity & 0x0f;

	irgb = MAKE_RGB(ir,ig,ib);

	lo = state->m_ps5_palette_ram[lo_offs];
	hi = state->m_ps5_palette_ram[hi_offs];

	/* red,green,blue component */
	r = pal4bit(lo >> 4);
	g = pal4bit(lo);
	b = pal4bit(hi >> 4);

	/* Grey background enable */
	if (state->m_bg_status & 2)
	{
		UINT8 val = (r + g + b) / 3;		/* Grey */
		/* Just leave plain grey */
		palette_set_color(machine,color,jal_blend_func(MAKE_RGB(val,val,val),irgb,ix));
	}
	else
	{
		/* Seems fishy, but the title screen would be black otherwise... */
		if (!(state->m_title_screen & 1))
		{
			/* Leave the world as-is */
			palette_set_color(machine,color,jal_blend_func(MAKE_RGB(r,g,b),irgb,ix));
		}
	}
}

static void set_background_palette_intensity(running_machine &machine)
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	int i;
	state->m_palette_intensity = state->m_ps5_palette_ram[BG_PAL_INTENSITY_BU] |
						(state->m_ps5_palette_ram[BG_PAL_INTENSITY_RG]<<8);

	/* for all of the background palette */
	for (i = 0; i < 0x100; i++)
		psychic5_change_bg_palette(machine,state->m_bg_palette_base+i,state->m_bg_palette_ram_base+i*2,state->m_bg_palette_ram_base+i*2+1);
}


/***************************************************************************
  Memory handler
***************************************************************************/

READ8_MEMBER(psychic5_state::psychic5_vram_page_select_r)
{
	return m_ps5_vram_page;
}

WRITE8_MEMBER(psychic5_state::psychic5_vram_page_select_w)
{
	m_ps5_vram_page = data & 1;
}

WRITE8_MEMBER(psychic5_state::psychic5_title_screen_w)
{
	m_title_screen = data;
}

READ8_MEMBER(psychic5_state::psychic5_paged_ram_r)
{
	if (m_ps5_vram_page == 1)
	{
		switch (offset)
		{
			case 0x00: return input_port_read(machine(), "SYSTEM");
			case 0x01: return input_port_read(machine(), "P1");
			case 0x02: return input_port_read(machine(), "P2");
			case 0x03: return input_port_read(machine(), "DSW1");
			case 0x04: return input_port_read(machine(), "DSW2");
		}
	}

	return m_ps5_pagedram[m_ps5_vram_page][offset];
}

WRITE8_MEMBER(psychic5_state::psychic5_paged_ram_w)
{
	m_ps5_pagedram[m_ps5_vram_page][offset] = data;

	if (m_ps5_vram_page == 0)
	{
		if (offset <= 0xfff)
			m_bg_tilemap->mark_tile_dirty(offset >> 1);
	}
	else
	{
		if (offset == BG_SCROLLX_LSB || offset == BG_SCROLLX_MSB)
		{
			UINT16 bg_scrollx = m_ps5_io_ram[BG_SCROLLX_LSB] | (m_ps5_io_ram[BG_SCROLLX_MSB] << 8);
			m_bg_tilemap->set_scrollx(0, bg_scrollx);
		}
		else if (offset == BG_SCROLLY_LSB || offset == BG_SCROLLY_MSB)
		{
			UINT16 bg_scrolly = m_ps5_io_ram[BG_SCROLLY_LSB] | (m_ps5_io_ram[BG_SCROLLY_MSB] << 8);
			m_bg_tilemap->set_scrolly(0, bg_scrolly);
		}
		else if (offset == BG_SCREEN_MODE)
		{
			m_bg_status = m_ps5_io_ram[BG_SCREEN_MODE];
		}
		else if (offset >= 0x400 && offset <= 0x5ff)	/* Sprite color */
			psychic5_change_palette(machine(),((offset >> 1) & 0xff)+0x000,offset-0x400);
		else if (offset >= 0x800 && offset <= 0x9ff)	/* BG color */
			psychic5_change_palette(machine(),((offset >> 1) & 0xff)+0x100,offset-0x400);
		else if (offset >= 0xa00 && offset <= 0xbff)	/* Text color */
			psychic5_change_palette(machine(),((offset >> 1) & 0xff)+0x200,offset-0x400);
		else if (offset >= 0x1000)
			m_fg_tilemap->mark_tile_dirty((offset-0x1000) >> 1);
	}
}

WRITE8_MEMBER(psychic5_state::bombsa_paged_ram_w)
{
	m_ps5_pagedram[m_ps5_vram_page][offset] = data;

	if (m_ps5_vram_page == 0)
	{
		m_bg_tilemap->mark_tile_dirty(offset >> 1);
	}
	else
	{
		if (offset == BG_SCROLLX_LSB || offset == BG_SCROLLX_MSB)
		{
			UINT16 bg_scrollx = m_ps5_io_ram[BG_SCROLLX_LSB] | (m_ps5_io_ram[BG_SCROLLX_MSB] << 8);
			m_bg_tilemap->set_scrollx(0, bg_scrollx);
		}
		else if (offset == BG_SCROLLY_LSB || offset == BG_SCROLLY_MSB)
		{
			UINT16 bg_scrolly = m_ps5_io_ram[BG_SCROLLY_LSB] | (m_ps5_io_ram[BG_SCROLLY_MSB] << 8);
			m_bg_tilemap->set_scrolly(0, bg_scrolly);
		}
		else if (offset == BG_SCREEN_MODE)
		{
			m_bg_status = m_ps5_io_ram[BG_SCREEN_MODE];
		}
		else if (offset >= 0x0800 && offset <= 0x0fff)
			m_fg_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset >= 0x1000 && offset <= 0x15ff)
			psychic5_change_palette(machine(), (offset >> 1) & 0x3ff, offset-0x1000);
	}
}

WRITE8_MEMBER(psychic5_state::bombsa_unknown_w)
{
	m_bombsa_unknown = data;
}


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	int offs = tile_index << 1;
	int attr = state->m_bg_videoram[offs + 1];
	int code = state->m_bg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	int offs = tile_index << 1;
	int attr = state->m_fg_videoram[offs + 1];
	int code = state->m_fg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(2, code, color, flags);
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

VIDEO_START( psychic5 )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	/*                          info              offset             w   h  col  row */
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,  8,  8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(15);

	state->m_ps5_pagedram[0] = auto_alloc_array(machine, UINT8, 0x2000);
	state->m_ps5_pagedram[1] = auto_alloc_array(machine, UINT8, 0x2000);

	state->m_bg_videoram  = &state->m_ps5_pagedram[0][0x0000];
	state->m_ps5_dummy_bg_ram      = &state->m_ps5_pagedram[0][0x1000];
	state->m_ps5_io_ram            = &state->m_ps5_pagedram[1][0x0000];
	state->m_ps5_palette_ram       = &state->m_ps5_pagedram[1][0x0400];
	state->m_fg_videoram  = &state->m_ps5_pagedram[1][0x1000];

	jal_blend_init(machine, 1);

	state->m_bg_palette_ram_base = 0x400;
	state->m_bg_palette_base = 0x100;
}

VIDEO_START( bombsa )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	/*                          info              offset             w   h   col  row */
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 128, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols,  8,  8,  32, 32);

	state->m_fg_tilemap->set_transparent_pen(15);

	state->m_ps5_pagedram[0] = auto_alloc_array(machine, UINT8, 0x2000);
	state->m_ps5_pagedram[1] = auto_alloc_array(machine, UINT8, 0x2000);

	state->m_bg_videoram  = &state->m_ps5_pagedram[0][0x0000];
	state->m_ps5_dummy_bg_ram      = &state->m_ps5_pagedram[0][0x1000];
	state->m_ps5_io_ram            = &state->m_ps5_pagedram[1][0x0000];
	state->m_fg_videoram  = &state->m_ps5_pagedram[1][0x0800];
	state->m_ps5_palette_ram       = &state->m_ps5_pagedram[1][0x1000];

	jal_blend_init(machine, 0);

	state->m_bg_palette_ram_base = 0x000;
	state->m_bg_palette_base = 0x000;
}

VIDEO_RESET( psychic5 )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	state->m_bg_clip_mode = 0;
	state->m_ps5_vram_page = 0;
	state->m_bg_status = 0;
	memset(state->m_ps5_pagedram[0],0,0x2000);
	memset(state->m_ps5_pagedram[1],0,0x2000);
	state->m_palette_intensity = 0;
}

VIDEO_RESET( bombsa )
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	state->m_ps5_vram_page = 0;
	state->m_bg_status = 0;
	state->m_title_screen = 0;
	memset(state->m_ps5_pagedram[0],0,0x2000);
	memset(state->m_ps5_pagedram[1],0,0x2000);
	state->m_palette_intensity = 0;
}


/***************************************************************************
  Screen refresh
***************************************************************************/

#define DRAW_SPRITE(code, sx, sy) jal_blend_drawgfx(bitmap, cliprect, machine.gfx[0], code, color, flipx, flipy, sx, sy, 15);

static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	/* Draw the sprites */
	for (offs = 0; offs < state->m_spriteram_size; offs += 16)
	{
		int attr  = spriteram[offs + 13];
		int code  = spriteram[offs + 14] | ((attr & 0xc0) << 2);
		int color = spriteram[offs + 15] & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[offs + 12];
		int sy = spriteram[offs + 11];
		int size = (attr & 0x08) ? 32:16;

		if (attr & 0x01) sx -= 256;
		if (attr & 0x04) sy -= 256;

		if (flip_screen_get(machine))
		{
			sx = 224 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (size == 32)
		{
			int x0,x1,y0,y1;

			if (flipx) { x0 = 2; x1 = 0; }
			else { x0 = 0; x1 = 2; }

			if (flipy) { y0 = 1; y1 = 0; }
			else { y0 = 0; y1 = 1; }

			DRAW_SPRITE(code + x0 + y0, sx, sy)
			DRAW_SPRITE(code + x0 + y1, sx, sy + 16)
			DRAW_SPRITE(code + x1 + y0, sx + 16, sy)
			DRAW_SPRITE(code + x1 + y1, sx + 16, sy + 16)
		}
		else
		{
			if (flip_screen_get(machine))
				DRAW_SPRITE(code, sx + 16, sy + 16)
			else
				DRAW_SPRITE(code, sx, sy)
		}
	}
}

static void draw_background(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	psychic5_state *state = machine.driver_data<psychic5_state>();
	UINT8 *spriteram = state->m_spriteram;

	rectangle clip = cliprect;

	set_background_palette_intensity(machine);

	if (!(state->m_title_screen & 1))
	{
		state->m_bg_clip_mode = 0;
		state->m_sx1 = state->m_sy1 = state->m_sy2 = 0;
	}
	else
	{
		int sy1_old = state->m_sy1;
		int sx1_old = state->m_sx1;
		int sy2_old = state->m_sy2;

		state->m_sy1 = spriteram[11];		/* sprite 0 */
		state->m_sx1 = spriteram[12];
		state->m_sy2 = spriteram[11+128];	/* sprite 8 */

		switch (state->m_bg_clip_mode)
		{
		case  0: case  4: if (sy1_old != state->m_sy1) state->m_bg_clip_mode++; break;
		case  2: case  6: if (sy2_old != state->m_sy2) state->m_bg_clip_mode++; break;
		case  8: case 10:
		case 12: case 14: if (sx1_old != state->m_sx1) state->m_bg_clip_mode++; break;
		case  1: case  5: if (state->m_sy1 == 0xf0) state->m_bg_clip_mode++; break;
		case  3: case  7: if (state->m_sy2 == 0xf0) state->m_bg_clip_mode++; break;
		case  9: case 11: if (state->m_sx1 == 0xf0) state->m_bg_clip_mode++; break;
		case 13: case 15: if (sx1_old == 0xf0) state->m_bg_clip_mode++;
		case 16: if (state->m_sy1 != 0x00) state->m_bg_clip_mode = 0; break;
		}

		switch (state->m_bg_clip_mode)
		{
		case  0: case  4: case  8: case 12: case 16:
			clip.set(0, 0, 0, 0);
			break;
		case  1: clip.min_y = state->m_sy1; break;
		case  3: clip.max_y = state->m_sy2; break;
		case  5: clip.max_y = state->m_sy1; break;
		case  7: clip.min_y = state->m_sy2; break;
		case  9: case 15: clip.min_x = state->m_sx1; break;
		case 11: case 13: clip.max_x = state->m_sx1; break;
		}

		if (flip_screen_get(machine))
			clip.set(255 - clip.max_x, 255 - clip.min_x, 255 - clip.max_y, 255 - clip.min_y);
	}

	state->m_bg_tilemap->draw(bitmap, clip, 0, 0);
}

SCREEN_UPDATE_RGB32( psychic5 )
{
	psychic5_state *state = screen.machine().driver_data<psychic5_state>();
	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	if (state->m_bg_status & 1)	/* Backgound enable */
		draw_background(screen.machine(), bitmap, cliprect);
	if (!(state->m_title_screen & 1))
		draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_RGB32( bombsa )
{
	psychic5_state *state = screen.machine().driver_data<psychic5_state>();
	if (state->m_bg_status & 1)	/* Backgound enable */
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		bitmap.fill(screen.machine().pens[0x0ff], cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
