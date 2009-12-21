/*****************************************************************************

B-Wings  (c) 1984 Data East Corporation
Zaviga   (c) 1984 Data East Corporation

drivers by Acho A. Tang

*****************************************************************************/
// Directives

#include "driver.h"
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


WRITE8_HANDLER( bwing_spriteram_w )
{
	space->machine->generic.buffered_spriteram.u8[offset] = data;
}

WRITE8_HANDLER( bwing_videoram_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->charmap, offset);
}


READ8_HANDLER ( bwing_scrollram_r )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	int offs;

	if (!state->srbank)
		offs = state->srxlat[offset];
	else
		offs = offset;

	return ((state->srbase[state->srbank])[offs]);
}


WRITE8_HANDLER( bwing_scrollram_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	int offs;

	if (!state->srbank)
	{
		offs = state->srxlat[offset];
		if (offs >> 12)
			tilemap_mark_tile_dirty(state->bgmap, offs & 0xfff);
		else
			tilemap_mark_tile_dirty(state->fgmap, offs & 0xfff);
	}
	else
	{
		offs = offset;
		if (offset < 0x1000)
			gfx_element_mark_dirty(space->machine->gfx[2], offset / 32);
		else
			gfx_element_mark_dirty(space->machine->gfx[3], offset / 32);
	}

	(state->srbase[state->srbank])[offs] = data;
}


WRITE8_HANDLER( bwing_scrollreg_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	state->sreg[offset] = data;

	switch (offset)
	{
		case 6: state->palatch = data; break; // one of the palette components is latched through I/O(yike)

		case 7:
			// tile graphics are decoded in RAM on the fly and tile codes are banked + interleaved(ouch)
			state->mapmask = data;
			state->srbank = data >> 6;

			#if BW_DEBUG
				logerror("(%s)%04x: w=%02x a=%04x f=%d\n", space->cpu->tag, cpu_get_pc(space->cpu), data, 0x1b00 + offset, video_screen_get_frame_number(space->machine->primary_screen));
			#endif
		break;
	}

	#if BW_DEBUG
		(memory_region(space->machine, REGION_CPU1))[0x1b10 + offset] = data;
	#endif
}


WRITE8_HANDLER( bwing_paletteram_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	static const float rgb[4][3] = {{0.85f, 0.95f, 1.00f},
						{0.90f, 1.00f, 1.00f},
						{0.80f, 1.00f, 1.00f},
						{0.75f, 0.90f, 1.10f}};
	int r, g, b, i;

	state->paletteram[offset] = data;

	r = ~data & 7;
	g = ~(data >> 4) & 7;
	b = ~state->palatch & 7;

	r = ((r << 5) + (r << 2) + (r >> 1));
	g = ((g << 5) + (g << 2) + (g >> 1));
	b = ((b << 5) + (b << 2) + (b >> 1));

	if ((i = input_port_read(space->machine, "EXTRA")) < 4)
	{
		r = (float)r * rgb[i][0];
		g = (float)g * rgb[i][1];
		b = (float)b * rgb[i][2];
		if (r > 0xff) r = 0xff;
		if (g > 0xff) g = 0xff;
		if (b > 0xff) b = 0xff;
	}

	palette_set_color(space->machine, offset, MAKE_RGB(r, g, b));

	#if BW_DEBUG
		state->paletteram[offset + 0x40] = state->palatch;
	#endif
}

//****************************************************************************
// Initializations

static TILE_GET_INFO( get_fgtileinfo )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	tileinfo->pen_data = gfx_element_get_data(machine->gfx[2], state->fgdata[tile_index] & (BW_NTILES - 1));
	tileinfo->palette_base = machine->gfx[2]->color_base + ((state->fgdata[tile_index] >> 7) << 3);
}

static TILE_GET_INFO( get_bgtileinfo )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	tileinfo->pen_data = gfx_element_get_data(machine->gfx[3], state->bgdata[tile_index] & (BW_NTILES - 1));
	tileinfo->palette_base = machine->gfx[3]->color_base + ((state->bgdata[tile_index] >> 7) << 3);
}

static TILE_GET_INFO( get_charinfo )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	SET_TILE_INFO(0, state->videoram[tile_index], 0, 0);
}

static TILEMAP_MAPPER( bwing_scan_cols )
{
	return ((col << 6) + row);
}


VIDEO_START( bwing )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	UINT32 *dwptr;
	int i;

	state->charmap = tilemap_create(machine, get_charinfo, tilemap_scan_cols, 8, 8, 32, 32);
	state->fgmap = tilemap_create(machine, get_fgtileinfo, bwing_scan_cols, 16, 16, 64, 64);
	state->bgmap = tilemap_create(machine, get_bgtileinfo, bwing_scan_cols, 16, 16, 64, 64);

	tilemap_set_transparent_pen(state->charmap, 0);
	tilemap_set_transparent_pen(state->fgmap, 0);

	state->srxlat = auto_alloc_array(machine, int, 0x2000);
	state_save_register_global_pointer(machine, state->srxlat, 0x2000);

	fill_srxlat(state->srxlat);

	state->fgdata = memory_region(machine, "gpu");
	state->bgdata = state->fgdata + 0x1000;

	for (i = 0; i < 4; i++)
		state->srbase[i] = state->fgdata + i * 0x2000;

	for (i = 0; i < 8; i++)
		state->sreg[i] = 0;

//  state->fgfx = machine->gfx[2];
	gfx_element_set_source(machine->gfx[2], state->srbase[1]);

//  state->bgfx = machine->gfx[3];
	gfx_element_set_source(machine->gfx[3], state->srbase[1] + 0x1000);

	dwptr = machine->gfx[2]->pen_usage;
	if (dwptr)
	{
		dwptr[0] = 0;
		for(i = 1; i < BW_NTILES; i++)
			dwptr[i] = -1;
	}
}

//****************************************************************************
// Realtime

static void draw_sprites( running_machine *machine, bitmap_t *bmp, const rectangle *clip, UINT8 *ram, int pri )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	int attrib, fx, fy, code, x, y, color, i;
	gfx_element *gfx = machine->gfx[1];

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
		if (state->mapmask & 0x20)
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


VIDEO_UPDATE( bwing )
{
	bwing_state *state = (bwing_state *)screen->machine->driver_data;
	unsigned x, y, shiftx;

	if (state->mapmask & 0x20)
	{
		state->mapflip = TILEMAP_FLIPX;
		shiftx = -8;
	}
	else
	{
		state->mapflip = TILEMAP_FLIPY;
		shiftx = 8;
	}

	// draw background
	if (!(state->mapmask & 1))
	{
		tilemap_set_flip(state->bgmap, state->mapflip);
		x = ((state->sreg[1]<<2 & 0x300) + state->sreg[2] + shiftx) & 0x3ff;
		tilemap_set_scrollx(state->bgmap, 0, x);
		y = (state->sreg[1]<<4 & 0x300) + state->sreg[3];
		tilemap_set_scrolly(state->bgmap, 0, y);
		tilemap_draw(bitmap, cliprect, state->bgmap, 0, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	// draw low priority sprites
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u8, 0);

	// draw foreground
	if (!(state->mapmask & 2))
	{
		tilemap_set_flip(state->fgmap, state->mapflip);
		x = ((state->sreg[1] << 6 & 0x300) + state->sreg[4] + shiftx) & 0x3ff;
		tilemap_set_scrollx(state->fgmap, 0, x);
		y = (state->sreg[1] << 8 & 0x300) + state->sreg[5];
		tilemap_set_scrolly(state->fgmap, 0, y);
		tilemap_draw(bitmap, cliprect, state->fgmap, 0, 0);
	}

	// draw high priority sprites
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u8, 1);

	// draw text layer
//  if (state->mapmask & 4)
	{
		tilemap_set_flip(state->charmap, state->mapflip);
		tilemap_draw(bitmap, cliprect, state->charmap, 0, 0);
	}
	return 0;
}

