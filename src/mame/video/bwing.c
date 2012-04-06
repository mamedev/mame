/*****************************************************************************

B-Wings  (c) 1984 Data East Corporation
Zaviga   (c) 1984 Data East Corporation

drivers by Acho A. Tang

*****************************************************************************/
// Directives

#include "emu.h"
#include "includes/bwing.h"


#define BW_DEBUG 0

#define BW_NTILES_L2    7
#define BW_NTILES      (1<<BW_NTILES_L2)


//****************************************************************************
// Local Functions

static void fill_srxlat( int *xlat )
{
	unsigned base, offset, i;

	for (base = 0; base < 0x2000; base += 0x400)
	{
		for(i = 0; i < 0x100; i++)
		{
			offset = base + (i<<2 & ~0x3f) + (i & 0x0f);

			xlat[base + i] = offset;
			xlat[base + i + 0x100] = offset + 0x10;
			xlat[base + i + 0x200] = offset + 0x20;
			xlat[base + i + 0x300] = offset + 0x30;
		}
	}
}

//****************************************************************************
// Exports

const gfx_layout bwing_tilelayout =
{
	16, 16,
	BW_NTILES,
	3,
	{ 0x4000*8, 0x2000*8, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};


WRITE8_MEMBER(bwing_state::bwing_spriteram_w)
{
	m_spriteram[offset] = data;
}

WRITE8_MEMBER(bwing_state::bwing_videoram_w)
{
	m_videoram[offset] = data;
	m_charmap->mark_tile_dirty(offset);
}


READ8_MEMBER(bwing_state::bwing_scrollram_r)
{
	int offs;

	if (!m_srbank)
		offs = m_srxlat[offset];
	else
		offs = offset;

	return ((m_srbase[m_srbank])[offs]);
}


WRITE8_MEMBER(bwing_state::bwing_scrollram_w)
{
	int offs;

	if (!m_srbank)
	{
		offs = m_srxlat[offset];
		if (offs >> 12)
			m_bgmap->mark_tile_dirty(offs & 0xfff);
		else
			m_fgmap->mark_tile_dirty(offs & 0xfff);
	}
	else
	{
		offs = offset;
		if (offset < 0x1000)
			gfx_element_mark_dirty(machine().gfx[2], offset / 32);
		else
			gfx_element_mark_dirty(machine().gfx[3], offset / 32);
	}

	(m_srbase[m_srbank])[offs] = data;
}


WRITE8_MEMBER(bwing_state::bwing_scrollreg_w)
{
	m_sreg[offset] = data;

	switch (offset)
	{
		case 6: m_palatch = data; break; // one of the palette components is latched through I/O(yike)

		case 7:
			// tile graphics are decoded in RAM on the fly and tile codes are banked + interleaved(ouch)
			m_mapmask = data;
			m_srbank = data >> 6;

			#if BW_DEBUG
				logerror("(%s)%04x: w=%02x a=%04x f=%d\n", device().tag, cpu_get_pc(&space.device()), data, 0x1b00 + offset, machine().primary_screen->frame_number());
			#endif
		break;
	}

	#if BW_DEBUG
		(machine().region(REGION_CPU1)->base())[0x1b10 + offset] = data;
	#endif
}


WRITE8_MEMBER(bwing_state::bwing_paletteram_w)
{
	static const float rgb[4][3] = {
		{0.85f, 0.95f, 1.00f},
		{0.90f, 1.00f, 1.00f},
		{0.80f, 1.00f, 1.00f},
		{0.75f, 0.90f, 1.10f}
	};
	int r, g, b, i;

	m_paletteram[offset] = data;

	r = ~data & 7;
	g = ~(data >> 4) & 7;
	b = ~m_palatch & 7;

	r = ((r << 5) + (r << 2) + (r >> 1));
	g = ((g << 5) + (g << 2) + (g >> 1));
	b = ((b << 5) + (b << 2) + (b >> 1));

	if ((i = input_port_read(machine(), "EXTRA")) < 4)
	{
		r = (float)r * rgb[i][0];
		g = (float)g * rgb[i][1];
		b = (float)b * rgb[i][2];
		if (r > 0xff) r = 0xff;
		if (g > 0xff) g = 0xff;
		if (b > 0xff) b = 0xff;
	}

	palette_set_color(machine(), offset, MAKE_RGB(r, g, b));

	#if BW_DEBUG
		m_paletteram[offset + 0x40] = m_palatch;
	#endif
}

//****************************************************************************
// Initializations

static TILE_GET_INFO( get_fgtileinfo )
{
	bwing_state *state = machine.driver_data<bwing_state>();
	tileinfo.pen_data = gfx_element_get_data(machine.gfx[2], state->m_fgdata[tile_index] & (BW_NTILES - 1));
	tileinfo.palette_base = machine.gfx[2]->color_base + ((state->m_fgdata[tile_index] >> 7) << 3);
}

static TILE_GET_INFO( get_bgtileinfo )
{
	bwing_state *state = machine.driver_data<bwing_state>();
	tileinfo.pen_data = gfx_element_get_data(machine.gfx[3], state->m_bgdata[tile_index] & (BW_NTILES - 1));
	tileinfo.palette_base = machine.gfx[3]->color_base + ((state->m_bgdata[tile_index] >> 7) << 3);
}

static TILE_GET_INFO( get_charinfo )
{
	bwing_state *state = machine.driver_data<bwing_state>();
	SET_TILE_INFO(0, state->m_videoram[tile_index], 0, 0);
}

static TILEMAP_MAPPER( bwing_scan_cols )
{
	return ((col << 6) + row);
}


VIDEO_START( bwing )
{
	bwing_state *state = machine.driver_data<bwing_state>();
	UINT32 *dwptr;
	int i;

	state->m_charmap = tilemap_create(machine, get_charinfo, tilemap_scan_cols, 8, 8, 32, 32);
	state->m_fgmap = tilemap_create(machine, get_fgtileinfo, bwing_scan_cols, 16, 16, 64, 64);
	state->m_bgmap = tilemap_create(machine, get_bgtileinfo, bwing_scan_cols, 16, 16, 64, 64);

	state->m_charmap->set_transparent_pen(0);
	state->m_fgmap->set_transparent_pen(0);

	state->m_srxlat = auto_alloc_array(machine, int, 0x2000);
	state->save_pointer(NAME(state->m_srxlat), 0x2000);

	fill_srxlat(state->m_srxlat);

	state->m_fgdata = machine.region("gpu")->base();
	state->m_bgdata = state->m_fgdata + 0x1000;

	for (i = 0; i < 4; i++)
		state->m_srbase[i] = state->m_fgdata + i * 0x2000;

	for (i = 0; i < 8; i++)
		state->m_sreg[i] = 0;

//  state->m_fgfx = machine.gfx[2];
	gfx_element_set_source(machine.gfx[2], state->m_srbase[1]);

//  state->m_bgfx = machine.gfx[3];
	gfx_element_set_source(machine.gfx[3], state->m_srbase[1] + 0x1000);

	dwptr = machine.gfx[2]->pen_usage;
	if (dwptr)
	{
		dwptr[0] = 0;
		for(i = 1; i < BW_NTILES; i++)
			dwptr[i] = -1;
	}
}

//****************************************************************************
// Realtime

static void draw_sprites( running_machine &machine, bitmap_ind16 &bmp, const rectangle &clip, UINT8 *ram, int pri )
{
	bwing_state *state = machine.driver_data<bwing_state>();
	int attrib, fx, fy, code, x, y, color, i;
	gfx_element *gfx = machine.gfx[1];

	for (i = 0; i < 0x200; i += 4)
	{
		attrib = ram[i];
		code   = ram[i + 1];
		y      = ram[i + 2];
		x      = ram[i + 3];
		color  = (attrib >> 3) & 1;

		if (!(attrib & 1) || color != pri)
			continue;

		code += (attrib << 3) & 0x100;
		y -= (attrib << 1) & 0x100;
		x -= (attrib << 2) & 0x100;

		fx = attrib & 0x04;
		fy = ~attrib & 0x02;

		// normal/cocktail
		if (state->m_mapmask & 0x20)
		{
			fx = !fx;
			fy = !fy;
			x = 240 - x;
			y = 240 - y;
		}

		// single/double
		if (!(attrib & 0x10))
			drawgfx_transpen(bmp, clip, gfx, code, color, fx, fy, x, y, 0);
		else
			drawgfxzoom_transpen(bmp, clip, gfx, code, color, fx, fy, x, y, 1<<16, 2<<16, 0);
	}
}


SCREEN_UPDATE_IND16( bwing )
{
	bwing_state *state = screen.machine().driver_data<bwing_state>();
	unsigned x, y, shiftx;

	if (state->m_mapmask & 0x20)
	{
		state->m_mapflip = TILEMAP_FLIPX;
		shiftx = -8;
	}
	else
	{
		state->m_mapflip = TILEMAP_FLIPY;
		shiftx = 8;
	}

	// draw background
	if (!(state->m_mapmask & 1))
	{
		state->m_bgmap->set_flip(state->m_mapflip);
		x = ((state->m_sreg[1]<<2 & 0x300) + state->m_sreg[2] + shiftx) & 0x3ff;
		state->m_bgmap->set_scrollx(0, x);
		y = (state->m_sreg[1]<<4 & 0x300) + state->m_sreg[3];
		state->m_bgmap->set_scrolly(0, y);
		state->m_bgmap->draw(bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(get_black_pen(screen.machine()), cliprect);

	// draw low priority sprites
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram, 0);

	// draw foreground
	if (!(state->m_mapmask & 2))
	{
		state->m_fgmap->set_flip(state->m_mapflip);
		x = ((state->m_sreg[1] << 6 & 0x300) + state->m_sreg[4] + shiftx) & 0x3ff;
		state->m_fgmap->set_scrollx(0, x);
		y = (state->m_sreg[1] << 8 & 0x300) + state->m_sreg[5];
		state->m_fgmap->set_scrolly(0, y);
		state->m_fgmap->draw(bitmap, cliprect, 0, 0);
	}

	// draw high priority sprites
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram, 1);

	// draw text layer
//  if (state->m_mapmask & 4)
	{
		state->m_charmap->set_flip(state->m_mapflip);
		state->m_charmap->draw(bitmap, cliprect, 0, 0);
	}
	return 0;
}

