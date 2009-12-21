#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "includes/homedata.h"

/*
    video control registers:
    0 xxxxxxxx  unknown
    1 x-------  flip screen
      -xxxxxxx  unknown
    2 xxxxxxxx  unknown
    3 xxxxxxxx  related to visible area?
    4 ----xxxx  fine horiz scroll (sub-character, 8 or 12 pixels)
      xxxx----  unknown
    5 xxxxxxxx  related to visible area?
    6 xxxxxxxx  related to visible area?
    7 xxxxxxxx  unknown
    8 xxxxxxxx  unknown
    9 xxxxxxxx  unknown
    a xxxxxxxx  unknown
    b xxxxxx--  horiz scroll (character resolution)
    c xxxxxxxx  might be bg layer horiz scroll (always 0 here but used by the older games)
    d xxxxxxxx  might be fg layer horiz scroll (always 0 here but always 4 in the older games)
 */


/***************************************************************************

  Blitter

  This is probably work done by the GX61A01 custom chip found on all boards,
  however the way it works is not exactly the same in all games.

***************************************************************************/

static void mrokumei_handleblit( const address_space *space, int rom_base )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	int i;
	int dest_param;
	int source_addr;
	int dest_addr;
	int base_addr;
	int opcode, data, num_tiles;
	UINT8 *pBlitData = memory_region(space->machine, "user1") + rom_base;

	dest_param = state->blitter_param[(state->blitter_param_count - 4) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 3) & 3];

	source_addr = state->blitter_param[(state->blitter_param_count - 2) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 1) & 3];

	/*  xxx-.----.----.---- not used?
     *  ---x.----.----.---- layer
     *  ----.xxxx.xxxx.xxxx addr
     */
	base_addr= (dest_param & 0x1000);
	dest_addr= (dest_param & 0x0fff);

//  logerror( "[blit bank %02x src %04x dst %04x]\n", blitter_bank, source_addr, dest_param);

	if( state->visible_page == 0 )
	{
		base_addr += 0x2000;
	}

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx RLE incrementing
         * 01xxxxxx Raw Run
         * 1xxxxxxx RLE constant data
         */
		if( opcode == 0x00 )
		{
 			/* end-of-graphic */
 			goto finish;
		}
		data  = pBlitData[source_addr++];

		if (opcode & 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x40: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x00: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
				mrokumei_videoram_w(space, base_addr + dest_addr, data);

			if (state->vreg[1] & 0x80)	/* flip screen */
			{
				dest_addr -= 2;
				if (dest_addr < 0)
					goto finish;
			}
			else
			{
				dest_addr+=2;
				if (dest_addr >= 0x1000) goto finish;
			}
		} /* for( i=0; i<num_tiles; i++ ) */
	} /* for(;;) */

finish:
	cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, HOLD_LINE);
}

static void reikaids_handleblit( const address_space *space, int rom_base )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	int i;
	UINT16 dest_param;
	int flipx;
	int source_addr, base_addr;
	int dest_addr;
	UINT8 *pBlitData = memory_region(space->machine, "user1") + rom_base;

	int opcode, data, num_tiles;

	dest_param = state->blitter_param[(state->blitter_param_count - 4) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 3) & 3];

	source_addr = state->blitter_param[(state->blitter_param_count - 2) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 1) & 3];

	/*  x---.----.----.---- flipx
     *  -x--.----.----.---- select: attr/tile
     *  --*x.xxxx.*xxx.xxxx addr
     */
	base_addr = (dest_param & 0x4000);
	dest_addr = (dest_param & 0x3fff);
	flipx	= (dest_param & 0x8000);

//  logerror( "[blit %02x %04x %04x]\n",blitter_bank,source_addr,dest_param);

	if (state->visible_page == 0)
		base_addr += 0x2000 << 2;

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx Raw Run
         * 01xxxxxx RLE incrementing
         * 1xxxxxxx RLE constant data
         */
		if (opcode == 0x00)
		{
			/* end-of-graphic */
			goto finish;
		}

		data  = pBlitData[source_addr++];

		if ((opcode & 0xc0) == 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x00: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
			{
				int addr = base_addr + (dest_addr & 0x3fff);
				int dat = data;

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);

					if (flipx)
					{
						if ((base_addr & 0x4000) == 0)
							dat |= 0x80;
						addr ^= 0x007c;
					}

					reikaids_videoram_w(space, addr, dat);
				}
			}

			if (state->vreg[1] & 0x80)	/* flip screen */
				dest_addr-=4;
			else
				dest_addr+=4;
		} /* for( i=0; i<num_tiles; i++ ) */
	}

finish:
	cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, HOLD_LINE);
}

static void pteacher_handleblit( const address_space *space, int rom_base )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	int i;
	int dest_param;
	int source_addr;
	int dest_addr, base_addr;
	int opcode, data, num_tiles;
	UINT8 *pBlitData = memory_region(space->machine, "user1") + rom_base;

	dest_param = state->blitter_param[(state->blitter_param_count - 4) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 3) & 3];

	source_addr = state->blitter_param[(state->blitter_param_count - 2) & 3] * 256 +
		state->blitter_param[(state->blitter_param_count - 1) & 3];

	/*  x---.----.----.---- not used?
     *  -x--.----.----.---- layer   (0..1)
     *  --*x.xxxx.*xxx.xxxx addr
     */
	base_addr = (dest_param & 0x4000);
	dest_addr = (dest_param & 0x3fff);

//  logerror( "[blit %02x %04x %04x]->%d\n",blitter_bank,source_addr,dest_param,homedata_visible_page);

	if (state->visible_page == 0)
	{
		base_addr += 0x2000 << 2;
	}

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx Raw Run
         * 01xxxxxx RLE incrementing
         * 1xxxxxxx RLE constant data
         */
		if (opcode == 0x00)
		{
 			/* end-of-graphic */
 			goto finish;
		}
		data  = pBlitData[source_addr++];

		if (opcode & 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x00: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
			{
				int addr = base_addr + (dest_addr & 0x3fff);

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);
					pteacher_videoram_w(space, addr, data);
				}
			}

			if (state->vreg[1] & 0x80)	/* flip screen */
				dest_addr -= 2;
			else
				dest_addr += 2;
		} /* for( i=0; i<num_tiles; i++ ) */
	} /* for(;;) */

finish:
	cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, HOLD_LINE);
}




/***************************************************************************

  Palette setup

***************************************************************************/

PALETTE_INIT( mrokumei )
{
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxx--------x--- red
         * ----xxxx-----x-- green
         * --------xxxx--x- blue
         * ---------------x unused
         */
		r = ((color >> 11) & 0x1e) | ((color >> 3) & 1);
		g = ((color >>  7) & 0x1e) | ((color >> 2) & 1);
		b = ((color >>  3) & 0x1e) | ((color >> 1) & 1);

		palette_set_color_rgb(machine, i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT( reikaids )
{
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxx--------x--- green
         * ----xxxx-----x-- red
         * --------xxxx--x- blue
         * ---------------x unused
         */
		g = ((color >> 11) & 0x1e) | ((color >> 3) & 1);
		r = ((color >>  7) & 0x1e) | ((color >> 2) & 1);
		b = ((color >>  3) & 0x1e) | ((color >> 1) & 1);

		palette_set_color_rgb(machine, i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT( pteacher )
{
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxxx----------- green
         * -----xxxxx------ red
         * ----------xxxxx- blue
         * ---------------x unused
         */
		g = ((color >> 11) & 0x1f);
		r = ((color >>  6) & 0x1f);
		b = ((color >>  1) & 0x1f);

		palette_set_color_rgb(machine, i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void mrokumei_info0( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int gfxbank )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	int addr  = tile_index * 2 + 0x2000 * page;
	int attr  = state->videoram[addr];
	int code  = state->videoram[addr + 1] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr >> 2) + (gfxbank << 6);

	SET_TILE_INFO( 0, code, color, state->flipscreen );
}
INLINE void mrokumei_info1( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int gfxbank )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	int addr  = tile_index * 2 + 0x1000 + 0x2000 * page;
	int attr  = state->videoram[addr];
	int code  = state->videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 3) << 6);

	SET_TILE_INFO( 1, code, color, state->flipscreen );
}

static TILE_GET_INFO( mrokumei_get_info0_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	mrokumei_info0( machine, tileinfo, tile_index, 0,  state->blitter_bank & 0x03 );
}

static TILE_GET_INFO( mrokumei_get_info1_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	mrokumei_info0( machine, tileinfo, tile_index, 1,  state->blitter_bank & 0x03 );
}

static TILE_GET_INFO( mrokumei_get_info0_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	mrokumei_info1( machine, tileinfo, tile_index, 0, (state->blitter_bank & 0x38) >> 3 );
}

static TILE_GET_INFO( mrokumei_get_info1_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	mrokumei_info1( machine, tileinfo, tile_index, 1, (state->blitter_bank & 0x38) >> 3 );
}


INLINE void reikaids_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	int addr  = tile_index * 4 + layer + 0x2000 * page;
	int attr  = state->videoram[addr];
	int code  = state->videoram[addr + 0x1000] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr & 0x7c) >> 2;
	int flags = state->flipscreen;

	if (attr & 0x80) flags ^= TILE_FLIPX;

	SET_TILE_INFO( layer, code, color, flags );
}

	/* reikaids_gfx_bank[0]:
     *      -xxx.x---   layer#1
     *      ----.-xxx   layer#3
     *
     * reikaids_gfx_bank[1]:
     *      xxxx.x---   layer#0
     *      ----.-xxx   layer#2
     */
static TILE_GET_INFO( reikaids_get_info0_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 0, 0,  (state->gfx_bank[1] >> 3));
}

static TILE_GET_INFO( reikaids_get_info1_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 1, 0,  (state->gfx_bank[1] >> 3));
}

static TILE_GET_INFO( reikaids_get_info0_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 0, 1, ((state->gfx_bank[0] & 0x78) >> 3));
}

static TILE_GET_INFO( reikaids_get_info1_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 1, 1, ((state->gfx_bank[0] & 0x78) >> 3));
}

static TILE_GET_INFO( reikaids_get_info0_2 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 0, 2,  (state->gfx_bank[1] & 0x7));
}

static TILE_GET_INFO( reikaids_get_info1_2 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 1, 2,  (state->gfx_bank[1] & 0x7));
}

static TILE_GET_INFO( reikaids_get_info0_3 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 0, 3,  (state->gfx_bank[0] & 0x7));
}

static TILE_GET_INFO( reikaids_get_info1_3 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	reikaids_info(machine, tileinfo, tile_index, 1, 3,  (state->gfx_bank[0] & 0x7));
}


INLINE void pteacher_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = state->videoram[addr];
	int code  = state->videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 1) << 5);

	SET_TILE_INFO(layer, code, color, state->flipscreen);
}

static TILE_GET_INFO( pteacher_get_info0_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	pteacher_info(machine, tileinfo, tile_index, 0, 0, state->gfx_bank[0] & 0x0f);
}

static TILE_GET_INFO( pteacher_get_info1_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	pteacher_info(machine, tileinfo, tile_index, 1, 0, state->gfx_bank[0] & 0x0f);
}

static TILE_GET_INFO( pteacher_get_info0_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	pteacher_info(machine, tileinfo, tile_index, 0, 1, state->gfx_bank[0] >> 4);
}

static TILE_GET_INFO( pteacher_get_info1_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	pteacher_info(machine, tileinfo, tile_index, 1, 1, state->gfx_bank[0] >> 4);
}


INLINE void lemnangl_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = state->videoram[addr];
	int code  = state->videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = 16 * (attr >> 3) + gfxbank;

	SET_TILE_INFO(2 * layer + gfxset, code, color, state->flipscreen);
}

static TILE_GET_INFO( lemnangl_get_info0_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	lemnangl_info( machine, tileinfo, tile_index, 0, 0,  state->blitter_bank & 1, state->gfx_bank[0] & 0x0f );
}

static TILE_GET_INFO( lemnangl_get_info1_0 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	lemnangl_info( machine, tileinfo, tile_index, 1, 0,  state->blitter_bank & 1, state->gfx_bank[0] & 0x0f );
}


static TILE_GET_INFO( lemnangl_get_info0_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	lemnangl_info( machine, tileinfo, tile_index, 0, 1, (state->blitter_bank & 2) >> 1, state->gfx_bank[0] >> 4 );
}

static TILE_GET_INFO( lemnangl_get_info1_1 )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	lemnangl_info( machine, tileinfo, tile_index, 1, 1, (state->blitter_bank & 2) >> 1, state->gfx_bank[0] >> 4 );
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mrokumei )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	state->bg_tilemap[0][0] = tilemap_create( machine, mrokumei_get_info0_0, tilemap_scan_rows, 8, 8, 64, 32 );
	state->bg_tilemap[0][1] = tilemap_create( machine, mrokumei_get_info0_1, tilemap_scan_rows, 8, 8, 64, 32 );
	state->bg_tilemap[1][0] = tilemap_create( machine, mrokumei_get_info1_0, tilemap_scan_rows, 8, 8, 64, 32 );
	state->bg_tilemap[1][1] = tilemap_create( machine, mrokumei_get_info1_1, tilemap_scan_rows, 8, 8, 64, 32 );

	tilemap_set_transparent_pen(state->bg_tilemap[0][1], 0);
	tilemap_set_transparent_pen(state->bg_tilemap[1][1], 0);
}

VIDEO_START( reikaids )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	state->bg_tilemap[0][0] = tilemap_create(machine, reikaids_get_info0_0, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[0][1] = tilemap_create(machine, reikaids_get_info0_1, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[0][2] = tilemap_create(machine, reikaids_get_info0_2, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[0][3] = tilemap_create(machine, reikaids_get_info0_3, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[1][0] = tilemap_create(machine, reikaids_get_info1_0, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[1][1] = tilemap_create(machine, reikaids_get_info1_1, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[1][2] = tilemap_create(machine, reikaids_get_info1_2, tilemap_scan_rows,  8, 8, 32, 32);
	state->bg_tilemap[1][3] = tilemap_create(machine, reikaids_get_info1_3, tilemap_scan_rows,  8, 8, 32, 32);

	tilemap_set_transparent_pen(state->bg_tilemap[0][0], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[0][1], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[0][2], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[0][3], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[1][0], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[1][1], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[1][2], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[1][3], 0xff);
}

VIDEO_START( pteacher )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	state->bg_tilemap[0][0] = tilemap_create(machine, pteacher_get_info0_0, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[0][1] = tilemap_create(machine, pteacher_get_info0_1, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[1][0] = tilemap_create(machine, pteacher_get_info1_0, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[1][1] = tilemap_create(machine, pteacher_get_info1_1, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->bg_tilemap[0][1], 0xff);
	tilemap_set_transparent_pen(state->bg_tilemap[1][1], 0xff);
}

VIDEO_START( lemnangl )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	state->bg_tilemap[0][0] = tilemap_create(machine, lemnangl_get_info0_0, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[0][1] = tilemap_create(machine, lemnangl_get_info0_1, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[1][0] = tilemap_create(machine, lemnangl_get_info1_0, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap[1][1] = tilemap_create(machine, lemnangl_get_info1_1, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->bg_tilemap[0][1], 0x0f);
	tilemap_set_transparent_pen(state->bg_tilemap[1][1], 0x0f);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mrokumei_videoram_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap[(offset & 0x2000) >> 13][(offset & 0x1000) >> 12], (offset & 0xffe) >> 1);
}

WRITE8_HANDLER( reikaids_videoram_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap[(offset & 0x2000) >> 13][offset & 3], (offset & 0xffc) >> 2);
}

WRITE8_HANDLER( pteacher_videoram_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap[(offset & 0x2000) >> 13][(offset & 0x1000) >> 12], (offset & 0xffe) >> 1);
}

WRITE8_HANDLER( reikaids_gfx_bank_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
//logerror( "%04x: [setbank %02x]\n",cpu_get_pc(space->cpu),data);

	if (state->gfx_bank[state->reikaids_which] != data)
	{
		state->gfx_bank[state->reikaids_which] = data;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	state->reikaids_which ^= 1;
}

WRITE8_HANDLER( pteacher_gfx_bank_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
//  logerror("%04x: gfxbank:=%02x\n", cpu_get_pc(space->cpu), data);
	if (state->gfx_bank[0] != data)
	{
		state->gfx_bank[0] = data;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( homedata_blitter_param_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
//logerror("%04x: blitter_param_w %02x\n", cpu_get_pc(space->cpu), data);
	state->blitter_param[state->blitter_param_count] = data;
	state->blitter_param_count++;
	state->blitter_param_count &= 3;
}

WRITE8_HANDLER( mrokumei_blitter_bank_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	/* --xxx--- layer 1 gfx bank
       -----x-- blitter ROM bank
       ------xx layer 0 gfx bank
     */

	if ((state->blitter_bank ^ data) & 0x3b)
		tilemap_mark_all_tiles_dirty_all(space->machine);

	state->blitter_bank = data;
}

WRITE8_HANDLER( reikaids_blitter_bank_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	/* xxx----- priority control
       ----x--- target page? what's this for?
       ------xx blitter ROM bank
     */
	state->blitter_bank = data;
}

WRITE8_HANDLER( pteacher_blitter_bank_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	/* xxx----- blitter ROM bank
       -----x-- pixel clock (normal/slow)
       ------x- layer #1 gfx charset (lemnangl only)
       -------x layer #0 gfx charset (lemnangl only)
     */

	if ((state->blitter_bank ^ data) & 0x03)
		tilemap_mark_all_tiles_dirty_all(space->machine);

	state->blitter_bank = data;
}

WRITE8_HANDLER( mrokumei_blitter_start_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	if (data & 0x80)
		mrokumei_handleblit(space, ((state->blitter_bank & 0x04) >> 2) * 0x10000);

	/* bit 0 = bank switch; used by hourouki to access the
       optional service mode ROM (not available in current dump) */
}

WRITE8_HANDLER( reikaids_blitter_start_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	reikaids_handleblit(space, (state->blitter_bank & 3) * 0x10000);
}

WRITE8_HANDLER( pteacher_blitter_start_w )
{
	homedata_state *state = (homedata_state *)space->machine->driver_data;
	pteacher_handleblit(space, (state->blitter_bank >> 5) * 0x10000 & (memory_region_length(space->machine, "user1") - 1));
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( mrokumei )
{
	homedata_state *state = (homedata_state *)screen->machine->driver_data;
	int flags,width;

	/* blank screen */
	if (state->vreg[0x3] == 0xc1 && state->vreg[0x4] == 0xc0 && state->vreg[0x5] == 0xff)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	flags = (state->vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != state->flipscreen)
	{
		state->flipscreen = flags;
		tilemap_mark_all_tiles_dirty_all(screen->machine);
	}

	switch (state->vreg[0x3])
	{
		case 0xb7: width = 54; break;	// mjclinic
		case 0xae: width = 52; break;	// mrokumei
		case 0x9f: width = 49; break;	// hourouki, mhgaiden, mjhokite
		case 0x96: width = 49; break;	// mjclinic
		default:
			if (state->vreg[0x3])
				popmessage("unknown video control %02x %02x %02x %02x",
					state->vreg[0x3],
					state->vreg[0x4],
					state->vreg[0x5],
					state->vreg[0x6]);
			width = 54;
			break;
	}
	video_screen_set_visarea(screen, 0*8, width*8-1, 2*8, 30*8-1);

	tilemap_set_scrollx(state->bg_tilemap[state->visible_page][0], 0, state->vreg[0xc] << 1);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][0], 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][1], 0, 0);
	return 0;
}

#ifdef UNUSED_FUNCTION
VIDEO_UPDATE( reikaids )
{
	homedata_state *state = (homedata_state *)screen->machine->driver_data;
	int flags;
	static const int pritable[8][4] =
	{
		{ 3,1,0,2 },
		{ 1,3,0,2 },
		{ 0,3,1,2 },
		{ 0,1,3,2 },
		{ 3,0,1,2 },
		{ 1,0,3,2 },
		{ 2,3,1,0 },    // (bg color should be taken from 1)
		{ 3,1,2,0 },    // (bg color should be taken from 1)
	};
	int pri, i;


	flags = (state->vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != state->flipscreen)
	{
		state->flipscreen = flags;
		tilemap_mark_all_tiles_dirty_all(screen->machine);
	}


	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	pri = (state->bank & 0x70) >> 4;
	for (i = 0; i < 4; i++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][pritable[pri][3 - i]], 0, 0);
	return 0;
}
#endif

VIDEO_UPDATE( reikaids )
{
	homedata_state *state = (homedata_state *)screen->machine->driver_data;
	int flags;
	static const int pritable[2][8][4] =	/* table of priorities derived from the PROM */
	{
	{
		{ 3,1,0,2 },
		{ 1,3,0,2 },
		{ 0,3,1,2 },
		{ 0,1,3,2 },
		{ 3,0,1,2 },
		{ 1,0,3,2 },
		{ 2,3,1,0 },	// (bg color should be taken from 1)
		{ 3,1,2,0 }	// (bg color should be taken from 1)
	},
	{
		{2,3,0,1},
		{2,0,3,1},
		{3,0,2,1},
		{0,3,2,1},
		{3,0,1,2},
		{2,1,3,0},
		{0,2,3,1},
		{3,2,0,1}
	},
	};

	int pri, i;

	flags = (state->vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != state->flipscreen)
	{
		state->flipscreen = flags;
		tilemap_mark_all_tiles_dirty_all(screen->machine);
	}


	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	pri = (state->blitter_bank & 0x70) >> 4;
	for (i = 0; i < 4; i++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][pritable[state->priority][pri][3 - i]], 0, 0);
	return 0;
}


VIDEO_UPDATE( pteacher )
{
	homedata_state *state = (homedata_state *)screen->machine->driver_data;
	int flags, scroll_low, scroll_high;


	/* blank screen */
	if (state->vreg[0x3] == 0xc1 && state->vreg[0x4] == 0xc0 && state->vreg[0x5] == 0xff)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	flags = (state->vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != state->flipscreen)
	{
		state->flipscreen = flags;
		tilemap_mark_all_tiles_dirty_all(screen->machine);
	}

	/* bit 2 of blitter_bank stretches characters horizontally by 3/2,
       so they look as if they were 12x8 instead of 8x8.

       However, the visible area can be further reduced by fudging with the video
       registers, but I haven't figured out how they work exactly.
       In most games it's like this (note that register 4 controls fine
       scrolling):

       width      3  4  5  6
       33*8    = a6 00 ef db (mjikaga)
       35*8    = bc 0b ef f0
       51*8    = a6 07 ef db (mjikaga)
       54*8    = bc 07 ef e8

       but in mjkinjas it's
       42*8    = f0 ae df cc

       note that mjkinjas has a 11MHz xtal instead of the 9MHz of all the others:
       the two things are probably related, in fact 35 * 11 / 9 = 42.7777777.

       and it also seems that
       blanked = c1 c0 ff --
      */

	if (state->blitter_bank & 0x04)
	{
		if (state->vreg[0x4] == 0xae || state->vreg[0x4] == 0xb8)
		{
			/* kludge for mjkinjas */
			video_screen_set_visarea(screen, 0*8, 42*8-1, 2*8, 30*8-1);
			scroll_low = 0;
		}
		else
		{
			if (state->vreg[0x3] == 0xa6)
				video_screen_set_visarea(screen, 0*8, 33*8-1, 2*8, 30*8-1);
			else
				video_screen_set_visarea(screen, 0*8, 35*8-1, 2*8, 30*8-1);
			scroll_low = (11 - (state->vreg[0x4] & 0x0f)) * 8 / 12;
		}
	}
	else
	{
		if (state->vreg[0x3] == 0xa6)
			video_screen_set_visarea(screen, 0*8, 51*8-1, 2*8, 30*8-1);
		else
			video_screen_set_visarea(screen, 0*8, 54*8-1, 2*8, 30*8-1);
		scroll_low = 7 - (state->vreg[0x4] & 0x0f);
	}
	scroll_high = state->vreg[0xb] >> 2;

	tilemap_set_scrollx(state->bg_tilemap[state->visible_page][0], 0, scroll_high * 8 + scroll_low);
	tilemap_set_scrollx(state->bg_tilemap[state->visible_page][1], 0, scroll_high * 8 + scroll_low);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][0], 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap[state->visible_page][1], 0, 0);
	return 0;
}


VIDEO_EOF( homedata )
{
	homedata_state *state = (homedata_state *)machine->driver_data;
	state->visible_page ^= 1;
}
