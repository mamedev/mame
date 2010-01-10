/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/sidearms.h"

WRITE8_HANDLER( sidearms_videoram_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( sidearms_colorram_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( sidearms_c804_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;

	/* bits 0 and 1 are coin counters */
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	/* bit 2 and 3 lock the coin chutes */
	if (!state->gameid || state->gameid==3)
	{
		coin_lockout_w(space->machine, 0, !(data & 0x04));
		coin_lockout_w(space->machine, 1, !(data & 0x08));
	}
	else
	{
		coin_lockout_w(space->machine, 0, data & 0x04);
		coin_lockout_w(space->machine, 1, data & 0x08);
	}

	/* bit 4 resets the sound CPU */
	if (data & 0x10)
	{
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, PULSE_LINE);
	}

	/* bit 5 enables starfield */
	if (state->staron != (data & 0x20))
	{
		state->staron = data & 0x20;
		state->hflop_74a_n = 1;
		state->hcount_191 = state->vcount_191 = 0;
	}

	/* bit 6 enables char layer */
	state->charon = data & 0x40;

	/* bit 7 flips screen */
	if (state->flipon != (data & 0x80))
	{
		state->flipon = data & 0x80;
		flip_screen_set(space->machine, state->flipon);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( sidearms_gfxctrl_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;
	state->objon = data & 0x01;
	state->bgon = data & 0x02;
}

WRITE8_HANDLER( sidearms_star_scrollx_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;
	UINT32 last_state = state->hcount_191;

	state->hcount_191++;
	state->hcount_191 &= 0x1ff;

	// invert 74LS74A(flipflop) output on 74LS191(hscan counter) carry's rising edge
	if (state->hcount_191 & ~last_state & 0x100)
		state->hflop_74a_n ^= 1;
}

WRITE8_HANDLER( sidearms_star_scrolly_w )
{
	sidearms_state *state = (sidearms_state *)space->machine->driver_data;
	state->vcount_191++;
	state->vcount_191 &= 0xff;
}


static TILE_GET_INFO( get_sidearms_bg_tile_info )
{
	sidearms_state *state = (sidearms_state *)machine->driver_data;
	int code, attr, color, flags;

	code = state->tilerom[tile_index];
	attr = state->tilerom[tile_index + 1];
	code |= attr<<8 & 0x100;
	color = attr>>3 & 0x1f;
	flags = attr>>1 & 0x03;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_philko_bg_tile_info )
{
	sidearms_state *state = (sidearms_state *)machine->driver_data;
	int code, attr, color, flags;

	code = state->tilerom[tile_index];
	attr = state->tilerom[tile_index + 1];
	code |= (((attr>>6 & 0x02) | (attr & 0x01)) * 0x100);
	color = attr>>3 & 0x0f;
	flags = attr>>1 & 0x03;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	sidearms_state *state = (sidearms_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + (attr<<2 & 0x300);
	int color = attr & 0x3f;

	SET_TILE_INFO(0, code, color, 0);
}

static TILEMAP_MAPPER( sidearms_tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	int offset = ((row << 7) + col) << 1;

	/* swap bits 1-7 and 8-10 of the address to compensate for the funny layout of the ROM data */
	return ((offset & 0xf801) | ((offset & 0x0700) >> 7) | ((offset & 0x00fe) << 3)) & 0x7fff;
}

VIDEO_START( sidearms )
{
	sidearms_state *state = (sidearms_state *)machine->driver_data;
	state->tilerom = memory_region(machine, "gfx4");

	if (!state->gameid)
	{
		state->bg_tilemap = tilemap_create(machine, get_sidearms_bg_tile_info, sidearms_tilemap_scan,
			 32, 32, 128, 128);

		tilemap_set_transparent_pen(state->bg_tilemap, 15);
	}
	else
	{
		state->bg_tilemap = tilemap_create(machine, get_philko_bg_tile_info, sidearms_tilemap_scan, 32, 32, 128, 128);
	}

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);

	state->hflop_74a_n = 1;
	state->latch_374 = state->vcount_191 = state->hcount_191 = 0;

	state->flipon = state->charon = state->staron = state->objon = state->bgon = 0;
}

static void draw_sprites_region(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int start_offset, int end_offset )
{
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	const gfx_element *gfx = machine->gfx[2];
	int offs, attr, color, code, x, y, flipx, flipy;
	sidearms_state *state = (sidearms_state *)machine->driver_data;

	flipy = flipx = state->flipon;

	for (offs = end_offset - 32; offs >= start_offset; offs -= 32)
	{
		y = buffered_spriteram[offs + 2];
		if (!y || buffered_spriteram[offs + 5] == 0xc3) continue;

		attr = buffered_spriteram[offs + 1];
		color = attr & 0xf;
		code = buffered_spriteram[offs] + ((attr << 3) & 0x700);
		x = buffered_spriteram[offs + 3] + ((attr << 4) & 0x100);

		if (state->flipon)
		{
			x = (62 * 8) - x;
			y = (30 * 8) - y;
		}

		drawgfx_transpen(bitmap, cliprect,
			gfx,
			code, color,
			flipx, flipy,
			x, y, 15);
	}
}

static void sidearms_draw_starfield( running_machine *machine, bitmap_t *bitmap )
{
	int x, y, i;
	UINT32 hadd_283, vadd_283, _hflop_74a_n, _hcount_191, _vcount_191;
	UINT8 *sf_rom;
	UINT16 *lineptr;
	int pixadv, lineadv;
	sidearms_state *state = (sidearms_state *)machine->driver_data;

	// clear starfield background
	lineptr = BITMAP_ADDR16(bitmap, 16, 64);
	lineadv = bitmap->rowpixels;

	for (i=224; i; i--) { memset(lineptr, 0, 768); lineptr += lineadv; }

	// bail if not Side Arms or the starfield has been disabled
	if (state->gameid || !state->staron) return;

	// init and cache some global vars in stack frame
	hadd_283 = 0;

	_hflop_74a_n = state->hflop_74a_n;
	_vcount_191 = state->vcount_191;
	_hcount_191 = state->hcount_191 & 0xff;

	sf_rom = memory_region(machine, "user1");

#if 0 // old loop (for reference; easier to read)
	if (!flipon)
	{
		lineptr = bitmap->base;
		pixadv  = 1;
		lineadv = lineadv - 512;
	}
	else
	{
		lineptr = BITMAP_ADDR16(bitmap, 255, 512 - 1);
		pixadv  = -1;
		lineadv = -lineadv + 512;
	}

	for (y=0; y<256; y++) // 8-bit V-clock input
	{
		for (x=0; x<512; lineptr+=pixadv,x++) // 9-bit H-clock input
		{
			i = hadd_283; // store horizontal adder's previous state in i
			hadd_283 = _hcount_191 + (x & 0xff); // add lower 8 bits and preserve carry

			if (x<64 || x>447 || y<16 || y>239) continue; // clip rejection

			vadd_283 = _vcount_191 + y; // add lower 8 bits and discard carry (later)

			if (!((vadd_283 ^ (x>>3)) & 4)) continue;		// logic rejection 1
			if ((vadd_283 | (hadd_283>>1)) & 2) continue;	// logic rejection 2

			// latch data from starfield EPROM on rising edge of 74LS374's clock input
			if (!(~i & 0x1f))
			{
				i = vadd_283<<4 & 0xff0;				// to starfield EPROM A04-A11 (8 bits)
				i |= (_hflop_74a_n^(hadd_283>>8)) << 3;	// to starfield EPROM A03     (1 bit)
				i |= hadd_283>>5 & 7;					// to starfield EPROM A00-A02 (3 bits)
				latch_374 = sf_rom[i + 0x3000];			// lines A12-A13 are always high
			}

			if ((~((latch_374^hadd_283)^1) & 0x1f)) continue; // logic rejection 3

			*lineptr = (UINT16)(latch_374>>5 | 0x378); // to color mixer
		}
		lineptr += lineadv;
	}
#else // optimized loop
	if (!state->flipon)
	{
		lineptr = BITMAP_ADDR16(bitmap, 16, 64);
		pixadv  = 1;
		lineadv = lineadv - 384;
	}
	else
	{
		lineptr = BITMAP_ADDR16(bitmap, 239, 512 - 64 - 1);
		pixadv  = -1;
		lineadv = -lineadv + 384;
	}

	for (y=16; y<240; y++) // 8-bit V-clock input (clipped against vertical visible area)
	{
		// inner loop pre-entry conditioning
		hadd_283 = (_hcount_191 + 64) & ~0x1f;
		vadd_283 = _vcount_191 + y;

		i = vadd_283<<4 & 0xff0;				// to starfield EPROM A04-A11 (8 bits)
		i |= (_hflop_74a_n^(hadd_283>>8)) << 3;	// to starfield EPROM A03     (1 bit)
		i |= hadd_283>>5 & 7;					// to starfield EPROM A00-A02 (3 bits)
		state->latch_374 = sf_rom[i + 0x3000];			// lines A12-A13 are always high

		hadd_283 = _hcount_191 + 63;

		for (x=64; x<448; lineptr+=pixadv,x++) // 9-bit H-clock input (clipped against horizontal visible area)
		{
			i = hadd_283;							// store horizontal adder's previous state in i
			hadd_283 = _hcount_191 + (x & 0xff);	// add lower 8 bits and preserve carry
			vadd_283 = _vcount_191 + y;				// add lower 8 bits and discard carry (later)

			if (!((vadd_283 ^ (x>>3)) & 4)) continue;		// logic rejection 1
			if ((vadd_283 | (hadd_283>>1)) & 2) continue;	// logic rejection 2

			// latch data from starfield EPROM on rising edge of 74LS374's clock input
			if (!(~i & 0x1f))
			{
				i = vadd_283<<4 & 0xff0;				// to starfield EPROM A04-A11 (8 bits)
				i |= (_hflop_74a_n^(hadd_283>>8)) << 3;	// to starfield EPROM A03     (1 bit)
				i |= hadd_283>>5 & 7;					// to starfield EPROM A00-A02 (3 bits)
				state->latch_374 = sf_rom[i + 0x3000];			// lines A12-A13 are always high
			}

			if ((~((state->latch_374^hadd_283)^1) & 0x1f)) continue; // logic rejection 3

			*lineptr = (UINT16)(state->latch_374>>5 | 0x378); // to color mixer
		}
		lineptr += lineadv;
	}
#endif
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	sidearms_state *state = (sidearms_state *)machine->driver_data;

	if (state->gameid == 2 || state->gameid == 3) // Dyger and Whizz have simple front-to-back sprite priority
		draw_sprites_region(machine, bitmap, cliprect, 0x0000, 0x1000);
	else
	{
		draw_sprites_region(machine, bitmap, cliprect, 0x0700, 0x0800);
		draw_sprites_region(machine, bitmap, cliprect, 0x0e00, 0x1000);
		draw_sprites_region(machine, bitmap, cliprect, 0x0800, 0x0f00);
		draw_sprites_region(machine, bitmap, cliprect, 0x0000, 0x0700);
	}
}

VIDEO_UPDATE( sidearms )
{
	sidearms_state *state = (sidearms_state *)screen->machine->driver_data;

	sidearms_draw_starfield(screen->machine, bitmap);

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx[0] + (state->bg_scrollx[1] << 8 & 0xf00));
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly[0] + (state->bg_scrolly[1] << 8 & 0xf00));

	if (state->bgon)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	if (state->objon)
		draw_sprites(screen->machine, bitmap, cliprect);

	if (state->charon)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( sidearms )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
